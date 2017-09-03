/*
   Source File : OpenTypeFileInput.cpp


   Copyright 2011 Gal Kahana PDFWriter

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   
*/
#include "OpenTypeFileInput.h"
#include "Trace.h"
#include "InputFile.h"

using namespace PDFHummus;

OpenTypeFileInput::OpenTypeFileInput(void)
{
    mHeaderOffset = 0;
    mTableOffset = 0;
	mHMtx = NULL;
	mName.mNameEntries = NULL;
	mLoca = NULL;
	mGlyf = NULL;
}

OpenTypeFileInput::~OpenTypeFileInput(void)
{
	FreeTables();
}

void OpenTypeFileInput::FreeTables()
{
	delete[] mHMtx;
	mHMtx = NULL;
	if(mName.mNameEntries)
	{
		for(unsigned short i =0; i < mName.mNameEntriesCount; ++i)
			delete[] mName.mNameEntries[i].String;
	}
	delete[] mName.mNameEntries;
	mName.mNameEntries = NULL;
	delete[] mLoca;
	mLoca = NULL;
	delete[] mGlyf;
	mGlyf = NULL;

	UShortToGlyphEntryMap::iterator it = mActualGlyphs.begin();
	for(; it != mActualGlyphs.end(); ++it)
		delete it->second;
	mActualGlyphs.clear();
}

EStatusCode OpenTypeFileInput::ReadOpenTypeFile(const std::string& inFontFilePath, unsigned short inFaceIndex)
{
	InputFile fontFile;

	EStatusCode status = fontFile.OpenFile(inFontFilePath);
	if(status != PDFHummus::eSuccess)
	{
		TRACE_LOG1("OpenTypeFileInput::ReadOpenTypeFile, cannot open true type font file at %s",inFontFilePath.c_str());
		return status;
	}

	status = ReadOpenTypeFile(fontFile.GetInputStream(), inFaceIndex);
	fontFile.CloseFile();
	return status;
}

EStatusCode OpenTypeFileInput::ReadOpenTypeFile(IByteReaderWithPosition* inTrueTypeFile, unsigned short inFaceIndex)
{
	EStatusCode status;

    mFaceIndex = inFaceIndex;
    
	do
	{
		FreeTables();

		mPrimitivesReader.SetOpenTypeStream(inTrueTypeFile);

        mHeaderOffset = (unsigned long)mPrimitivesReader.GetCurrentPosition();
        mTableOffset = (unsigned long)mPrimitivesReader.GetCurrentPosition();

		status = ReadOpenTypeHeader();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("OpenTypeFileInput::ReadOpenTypeFile, failed to read true type header");
			break;
		}
		
		status = ReadHead();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("OpenTypeFileInput::ReadOpenTypeFile, failed to read head table");
			break;
		}
		
		status = ReadMaxP();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("OpenTypeFileInput::ReadOpenTypeFile, failed to read maxp table");
			break;
		}

		status = ReadHHea();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("OpenTypeFileInput::ReadOpenTypeFile, failed to read hhea table");
			break;
		}

		status = ReadHMtx();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("OpenTypeFileInput::ReadOpenTypeFile, failed to read hmtx table");
			break;
		}

		status = ReadOS2(); // Note that OS/2 is supposedly required, but some dfonts don't contain it...and it's fine
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("OpenTypeFileInput::ReadOpenTypeFile, failed to read os2 table");
			break;
		}

		status = ReadName();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("OpenTypeFileInput::ReadOpenTypeFile, failed to read name table");
			break;
		}

		if(EOpenTypeTrueType == mFontType)
		{
			// true type specifics
			status = ReadLoca();
			if(status != PDFHummus::eSuccess)
			{
				TRACE_LOG("OpenTypeFileInput::ReadOpenTypeFile, failed to read loca table");
				break;
			}

			status = ReadGlyfForDependencies();
			if(status != PDFHummus::eSuccess)
			{
				TRACE_LOG("OpenTypeFileInput::ReadOpenTypeFile, failed to read glyf table");
				break;
			}
			mCVTExists = mTables.find(GetTag("cvt ")) != mTables.end();
			mFPGMExists = mTables.find(GetTag("fpgm")) != mTables.end();
			mPREPExists = mTables.find(GetTag("prep")) != mTables.end();

			// zero cff items
			mCFF.Reset();
		}
		else
		{
			// CFF specifics
			status = ReadCFF();
			if(status != PDFHummus::eSuccess)
			{
				TRACE_LOG("OpenTypeFileInput::ReadOpenTypeFile, failed to read CFF table");
			}

			// zero true type items
			mCVTExists = false;
			mFPGMExists = false;
			mPREPExists = false;
			mGlyf = NULL;
			mLoca = NULL;
		}
	}while(false);

	return status;
}

EStatusCode OpenTypeFileInput::ReadOpenTypeHeader()
{
	EStatusCode status;
	TableEntry tableEntry;
	unsigned long tableTag;

	do
	{
		status = ReadOpenTypeSFNT();
        
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("OpenTypeFileInput::ReaderTrueTypeHeader, SFNT header not open type");
			break;
		}

        unsigned long sfntVersion;
        
        mPrimitivesReader.SetOffset(mHeaderOffset);
        mPrimitivesReader.ReadULONG(sfntVersion);
		mPrimitivesReader.ReadUSHORT(mTablesCount);
		// skip the next 6. i don't give a rats...
		mPrimitivesReader.Skip(6);

		for(unsigned short i = 0; i < mTablesCount; ++i)
		{
			mPrimitivesReader.ReadULONG(tableTag);
			mPrimitivesReader.ReadULONG(tableEntry.CheckSum);
			mPrimitivesReader.ReadULONG(tableEntry.Offset);
			mPrimitivesReader.ReadULONG(tableEntry.Length);
            tableEntry.Offset += mTableOffset;
			mTables.insert(ULongToTableEntryMap::value_type(tableTag,tableEntry));
		}
		status = mPrimitivesReader.GetInternalState();
	}
	while(false);

	return status;
}

EStatusCode OpenTypeFileInput::ReadOpenTypeSFNT()
{
	unsigned long sfntVersion;

    mPrimitivesReader.SetOffset(mHeaderOffset);
	mPrimitivesReader.ReadULONG(sfntVersion);
    
    if(mPrimitivesReader.GetInternalState() != PDFHummus::eSuccess)
    {
        return PDFHummus::eFailure;
    }
    
    if((0x74746366 /* ttcf */ == sfntVersion))
    {
        // mgubi: a TrueType composite font, just get to the right face table 
        // for the format see http://www.microsoft.com/typography/otspec/otff.htm
        
        unsigned long ttcVersion;
        unsigned long numFonts;
        unsigned long offsetTable;
        
        mPrimitivesReader.ReadULONG(ttcVersion);
        mPrimitivesReader.ReadULONG(numFonts);
        
        if (mFaceIndex >= numFonts) 
        {
            TRACE_LOG2("OpenTypeFileInput::ReadOpenTypeSFNT, face index %d out of range. Max font count is %ld",mFaceIndex,numFonts);
            return PDFHummus::eFailure;
        }
        
        for (int i= 0; i<= mFaceIndex; ++i) {
            mPrimitivesReader.ReadULONG(offsetTable);
        }
        
        mHeaderOffset = mHeaderOffset + offsetTable;
        
        return ReadOpenTypeSFNT();
    } else if((0x10000 == sfntVersion) || (0x74727565 /* true */ == sfntVersion))
	{
		mFontType = EOpenTypeTrueType;
		return PDFHummus::eSuccess;
	}
	else if(0x4F54544F /* OTTO */ == sfntVersion)
	{
		mFontType = EOpenTypeCFF;
		return PDFHummus::eSuccess;
	}
	else if ((ReadOpenTypeSFNTFromDfont() == PDFHummus::eSuccess))
    {
		return PDFHummus::eSuccess;
    }
    else
		return PDFHummus::eFailure;
}

EStatusCode OpenTypeFileInput::ReadOpenTypeSFNTFromDfont()
{
	EStatusCode status = eSuccess;
    // mac resource fork header parsing
    // see: https://developer.apple.com/legacy/mac/library/documentation/mac/pdf/MoreMacintoshToolbox.pdf

    unsigned long rdata_pos, map_pos, rdata_len, map_offset;
    (void) rdata_len; // suppress compilation warning
    // verify that the header is composed as expected
    {
        Byte head[16], head2[16];
        
        mPrimitivesReader.SetOffset(mHeaderOffset);
        
        for(unsigned short i=0; i<16 && status == eSuccess; i++)
            status = mPrimitivesReader.ReadBYTE(head[i]);

		if (status != eSuccess)
		{
			return status;
		}
        
        rdata_pos = ( head[0] << 24 )  | ( head[1] << 16 )  | ( head[2] <<  8 )  | head[3] ;
        map_pos   = ( head[4] << 24 )  | ( head[5] << 16 )  | ( head[6] <<  8 )  | head[7] ;
        rdata_len = ( head[8] << 24 )  | ( head[9] << 16 )  | ( head[10] <<  8 ) | head[11] ;
        
        /*
			if ( rdata_pos + rdata_len != map_pos || map_pos == 0 ) {
				return PDFHummus::eFailure;
			}
		*/
        
        mPrimitivesReader.SetOffset(map_pos);
        
        //head2[15] = (Byte)(head[15]+1); // make it be different

		for (unsigned short i = 0; i<16 && status == eSuccess; i++)
			status = mPrimitivesReader.ReadBYTE(head2[i]);
		if (status != eSuccess) {
			return status;
		}

		/*
			{
            
				int allzeros = 1, allmatch = 1;
				for (int i = 0; i < 16; ++i )
				{
					if ( head[i] != 0 ) allzeros = 0;
					if ( head2[i] != head[i] ) allmatch = 0;
				}
				if ( !allzeros && !allmatch ) return PDFHummus::eFailure;
			}
		*/
    }

    /* If we have reached this point then it is probably a mac resource */
    /* file.  Now, does it contain any interesting resources?           */

    mPrimitivesReader.Skip(4      /* skip handle to next resource map */
                           + 2    /* skip file resource number */
                           + 2);  /* skip attributes */
    
    unsigned short type_list;
    status = mPrimitivesReader.ReadUSHORT(type_list);
	if (status != eSuccess)
		return status;
   
    map_offset  = map_pos + type_list;
    
    mPrimitivesReader.SetOffset(map_offset);

    // read the resource type list

    unsigned short cnt;
    status = mPrimitivesReader.ReadUSHORT(cnt);
	if (status != eSuccess)
		return status;

	bool foundSfnt = false;

    for (int i = 0; i < cnt + 1 && status == eSuccess && !foundSfnt; ++i )
    {
        long tag;
        unsigned short subcnt, rpos;
        status = mPrimitivesReader.ReadLONG(tag);
		if (status != eSuccess)
			break;
		status = mPrimitivesReader.ReadUSHORT(subcnt);
		if (status != eSuccess)
			break;
		status = mPrimitivesReader.ReadUSHORT(rpos);
		if (status != eSuccess)
			break;

        if ( (unsigned long)tag == GetTag("sfnt") ) {
            
            mPrimitivesReader.SetOffset(map_offset + rpos);

            // read the reference list for the 'sfnt' resources
            // the map is used to order the references by reference id
            
            std::map<unsigned short, unsigned long> resOffsetsMap;
 
            for (int j = 0; j < subcnt + 1 && status == eSuccess; ++j )
            {
                unsigned short res_id, res_name;
                unsigned long temp, mbz, res_offset;
                status = mPrimitivesReader.ReadUSHORT(res_id);
				if (status != eSuccess)
					break;
				status = mPrimitivesReader.ReadUSHORT(res_name);
				if (status != eSuccess)
					break;
				status = mPrimitivesReader.ReadULONG(temp);
				if (status != eSuccess)
					break;
				status = mPrimitivesReader.ReadULONG(mbz);
				if (status != eSuccess)
					break;
				res_offset = temp & 0xFFFFFFL;
               resOffsetsMap.insert(std::pair<unsigned short, unsigned long>(res_id,rdata_pos + res_offset));
            }
			if (status != eSuccess)
				break;
           
            int face_index = mFaceIndex, cur_face = 0; 
           unsigned long fontOffset = 0;
             
            for (std::map<unsigned short, unsigned long>::iterator it=resOffsetsMap.begin();
                 it!=resOffsetsMap.end(); ++it, ++cur_face) {
                if (cur_face == face_index) {
                    fontOffset = it->second;
                    break;
                }
            }

            if (cur_face != face_index)
            {
                TRACE_LOG("OpenTypeFileInput::ReadOpenTypeSFNTFromDfont, could not find face inside resource");
                status = PDFHummus::eFailure;
				break;
            }
            
            
            mHeaderOffset = fontOffset + 4; // skip the size of the resource
            mTableOffset = mHeaderOffset; 

            // try to open the resource as a TrueType font specification
			foundSfnt = true;
        }
    }

	if (status == eSuccess && foundSfnt)
		return ReadOpenTypeSFNT();
	else
	    return PDFHummus::eFailure;
}
unsigned long OpenTypeFileInput::GetTag(const char* inTagName)

{
	Byte buffer[4];
	unsigned short i=0;

	for(; i<strlen(inTagName);++i)
		buffer[i] = (Byte)inTagName[i];
	for(;i<4;++i)
		buffer[i] = 0x20;

	return	((unsigned long)buffer[0]<<24) + ((unsigned long)buffer[1]<<16) + 
			((unsigned long)buffer[2]<<8) + buffer[3];
}

EStatusCode OpenTypeFileInput::ReadHead()
{
	ULongToTableEntryMap::iterator it = mTables.find(GetTag("head"));
	if(it == mTables.end())
	{
		TRACE_LOG("OpenTypeFileInput::ReadHead, could not find head table");
		return PDFHummus::eFailure;
	}

	mPrimitivesReader.SetOffset(it->second.Offset);
	mPrimitivesReader.ReadFixed(mHead.TableVersionNumber);
	mPrimitivesReader.ReadFixed(mHead.FontRevision);
	mPrimitivesReader.ReadULONG(mHead.CheckSumAdjustment);
	mPrimitivesReader.ReadULONG(mHead.MagicNumber);
	mPrimitivesReader.ReadUSHORT(mHead.Flags);
	mPrimitivesReader.ReadUSHORT(mHead.UnitsPerEm);
	mPrimitivesReader.ReadLongDateTime(mHead.Created);
	mPrimitivesReader.ReadLongDateTime(mHead.Modified);
	mPrimitivesReader.ReadSHORT(mHead.XMin);
	mPrimitivesReader.ReadSHORT(mHead.YMin);
	mPrimitivesReader.ReadSHORT(mHead.XMax);
	mPrimitivesReader.ReadSHORT(mHead.YMax);
	mPrimitivesReader.ReadUSHORT(mHead.MacStyle);
	mPrimitivesReader.ReadUSHORT(mHead.LowerRectPPEM);
	mPrimitivesReader.ReadSHORT(mHead.FontDirectionHint);
	mPrimitivesReader.ReadSHORT(mHead.IndexToLocFormat);
	mPrimitivesReader.ReadSHORT(mHead.GlyphDataFormat);

	return mPrimitivesReader.GetInternalState();	
}

EStatusCode OpenTypeFileInput::ReadMaxP()
{
	ULongToTableEntryMap::iterator it = mTables.find(GetTag("maxp"));
	if(it == mTables.end())
	{
		TRACE_LOG("OpenTypeFileInput::ReadMaxP, could not find maxp table");
		return PDFHummus::eFailure;
	}
	mPrimitivesReader.SetOffset(it->second.Offset);

	memset(&mMaxp,0,sizeof(MaxpTable)); // set all with 0's in case the table's too short, so we'll have nice lookin values

	mPrimitivesReader.ReadFixed(mMaxp.TableVersionNumber);
	mPrimitivesReader.ReadUSHORT(mMaxp.NumGlyphs);

	if(1.0 == mMaxp.TableVersionNumber)
	{
		mPrimitivesReader.ReadUSHORT(mMaxp.MaxPoints);
		mPrimitivesReader.ReadUSHORT(mMaxp.MaxCountours);
		mPrimitivesReader.ReadUSHORT(mMaxp.MaxCompositePoints);
		mPrimitivesReader.ReadUSHORT(mMaxp.MaxCompositeContours);
		mPrimitivesReader.ReadUSHORT(mMaxp.MaxZones);
		mPrimitivesReader.ReadUSHORT(mMaxp.MaxTwilightPoints);
		mPrimitivesReader.ReadUSHORT(mMaxp.MaxStorage);
		mPrimitivesReader.ReadUSHORT(mMaxp.MaxFunctionDefs);
		mPrimitivesReader.ReadUSHORT(mMaxp.MaxInstructionDefs);
		mPrimitivesReader.ReadUSHORT(mMaxp.MaxStackElements);
		mPrimitivesReader.ReadUSHORT(mMaxp.MaxSizeOfInstructions);
		mPrimitivesReader.ReadUSHORT(mMaxp.MaxComponentElements);
		mPrimitivesReader.ReadUSHORT(mMaxp.MaxCompontentDepth);
	}
	return mPrimitivesReader.GetInternalState();	

}

EStatusCode OpenTypeFileInput::ReadHHea()
{
	ULongToTableEntryMap::iterator it = mTables.find(GetTag("hhea"));
	if(it == mTables.end())
	{
		TRACE_LOG("OpenTypeFileInput::ReadHHea, could not find hhea table");
		return PDFHummus::eFailure;
	}

	mPrimitivesReader.SetOffset(it->second.Offset);

	mPrimitivesReader.ReadFixed(mHHea.TableVersionNumber);
	mPrimitivesReader.ReadSHORT(mHHea.Ascender);
	mPrimitivesReader.ReadSHORT(mHHea.Descender);
	mPrimitivesReader.ReadSHORT(mHHea.LineGap);
	mPrimitivesReader.ReadUSHORT(mHHea.AdvanceWidthMax);
	mPrimitivesReader.ReadSHORT(mHHea.MinLeftSideBearing);
	mPrimitivesReader.ReadSHORT(mHHea.MinRightSideBearing);
	mPrimitivesReader.ReadSHORT(mHHea.XMaxExtent);
	mPrimitivesReader.ReadSHORT(mHHea.CaretSlopeRise);
	mPrimitivesReader.ReadSHORT(mHHea.CaretSlopeRun);
	mPrimitivesReader.ReadSHORT(mHHea.CaretOffset);
	mPrimitivesReader.Skip(8);
	mPrimitivesReader.ReadSHORT(mHHea.MetricDataFormat);
	mPrimitivesReader.ReadUSHORT(mHHea.NumberOfHMetrics);

	return mPrimitivesReader.GetInternalState();	
}

EStatusCode OpenTypeFileInput::ReadHMtx()
{
	ULongToTableEntryMap::iterator it = mTables.find(GetTag("hmtx"));
	if(it == mTables.end())
	{
		TRACE_LOG("OpenTypeFileInput::ReadHMtx, could not find hmtx table");
		return PDFHummus::eFailure;
	}

	mPrimitivesReader.SetOffset(it->second.Offset);

	mHMtx = new HMtxTableEntry[mMaxp.NumGlyphs];

	unsigned int i=0;

	for(; i < mHHea.NumberOfHMetrics;++i)
	{
		mPrimitivesReader.ReadUSHORT(mHMtx[i].AdvanceWidth);
		mPrimitivesReader.ReadSHORT(mHMtx[i].LeftSideBearing);
	}

	for(; i < mMaxp.NumGlyphs; ++i)
	{
		mHMtx[i].AdvanceWidth = mHMtx[mHHea.NumberOfHMetrics-1].AdvanceWidth;
		mPrimitivesReader.ReadSHORT(mHMtx[i].LeftSideBearing);
	}

	return mPrimitivesReader.GetInternalState();	
}

EStatusCode OpenTypeFileInput::ReadOS2()
{
	memset(&mOS2,0,sizeof(OS2Table));

	ULongToTableEntryMap::iterator it = mTables.find(GetTag("OS/2"));
	if(it == mTables.end())
	{
        mOS2Exists = false;
        return eSuccess;
	}
    
    mOS2Exists = true;

	mPrimitivesReader.SetOffset(it->second.Offset);


	mPrimitivesReader.ReadUSHORT(mOS2.Version);
	mPrimitivesReader.ReadSHORT(mOS2.AvgCharWidth);
	mPrimitivesReader.ReadUSHORT(mOS2.WeightClass);
	mPrimitivesReader.ReadUSHORT(mOS2.WidthClass);
	mPrimitivesReader.ReadUSHORT(mOS2.fsType);

	mPrimitivesReader.ReadSHORT(mOS2.SubscriptXSize);
	mPrimitivesReader.ReadSHORT(mOS2.SubscriptYSize);
	mPrimitivesReader.ReadSHORT(mOS2.SubscriptXOffset);
	mPrimitivesReader.ReadSHORT(mOS2.SubscriptYOffset);
	mPrimitivesReader.ReadSHORT(mOS2.SuperscriptXSize);
	mPrimitivesReader.ReadSHORT(mOS2.SuperscriptYSize);
	mPrimitivesReader.ReadSHORT(mOS2.SuperscriptXOffset);
	mPrimitivesReader.ReadSHORT(mOS2.SuperscriptYOffset);
	mPrimitivesReader.ReadSHORT(mOS2.StrikeoutSize);
	mPrimitivesReader.ReadSHORT(mOS2.StrikeoutPosition);
	mPrimitivesReader.ReadSHORT(mOS2.FamilyClass);
	for(int i=0; i <10; ++i)
		mPrimitivesReader.ReadBYTE(mOS2.Panose[i]);
	mPrimitivesReader.ReadULONG(mOS2.UnicodeRange1);
	mPrimitivesReader.ReadULONG(mOS2.UnicodeRange2);
	mPrimitivesReader.ReadULONG(mOS2.UnicodeRange3);
	mPrimitivesReader.ReadULONG(mOS2.UnicodeRange4);
	for(int i=0; i <4; ++i)
		mPrimitivesReader.ReadCHAR(mOS2.AchVendID[i]);
	mPrimitivesReader.ReadUSHORT(mOS2.FSSelection);
	mPrimitivesReader.ReadUSHORT(mOS2.FirstCharIndex);
	mPrimitivesReader.ReadUSHORT(mOS2.LastCharIndex);
	mPrimitivesReader.ReadSHORT(mOS2.TypoAscender);
	mPrimitivesReader.ReadSHORT(mOS2.TypoDescender);
	mPrimitivesReader.ReadSHORT(mOS2.TypoLineGap);
	mPrimitivesReader.ReadUSHORT(mOS2.WinAscent);
	mPrimitivesReader.ReadUSHORT(mOS2.WinDescent);

	// version 1 OS/2 table may end here [see that there's enough to continue]
	if(it->second.Length >= (mPrimitivesReader.GetCurrentPosition() - it->second.Offset) + 18)
	{
		mPrimitivesReader.ReadULONG(mOS2.CodePageRange1);
		mPrimitivesReader.ReadULONG(mOS2.CodePageRange2);
		mPrimitivesReader.ReadSHORT(mOS2.XHeight);
		mPrimitivesReader.ReadSHORT(mOS2.CapHeight);
		mPrimitivesReader.ReadUSHORT(mOS2.DefaultChar);
		mPrimitivesReader.ReadUSHORT(mOS2.BreakChar);
		mPrimitivesReader.ReadUSHORT(mOS2.MaxContext);
	}
	return mPrimitivesReader.GetInternalState();	
}

EStatusCode OpenTypeFileInput::ReadName()
{
	ULongToTableEntryMap::iterator it = mTables.find(GetTag("name"));
	if(it == mTables.end())
	{
		TRACE_LOG("OpenTypeFileInput::ReadName, could not find name table");
		return PDFHummus::eFailure;
	}

	mPrimitivesReader.SetOffset(it->second.Offset);	
	mPrimitivesReader.Skip(2);
	mPrimitivesReader.ReadUSHORT(mName.mNameEntriesCount);
	mName.mNameEntries = new NameTableEntry[mName.mNameEntriesCount];
	
	unsigned short stringOffset;
	
	mPrimitivesReader.ReadUSHORT(stringOffset);

	for(unsigned short i=0;i<mName.mNameEntriesCount;++i)
	{
		mPrimitivesReader.ReadUSHORT(mName.mNameEntries[i].PlatformID);
		mPrimitivesReader.ReadUSHORT(mName.mNameEntries[i].EncodingID);
		mPrimitivesReader.ReadUSHORT(mName.mNameEntries[i].LanguageID);
		mPrimitivesReader.ReadUSHORT(mName.mNameEntries[i].NameID);
		mPrimitivesReader.ReadUSHORT(mName.mNameEntries[i].Length);
		mPrimitivesReader.ReadUSHORT(mName.mNameEntries[i].Offset);
	}

	for(unsigned short i=0;i<mName.mNameEntriesCount;++i)
	{
		mName.mNameEntries[i].String = new char[mName.mNameEntries[i].Length];
		mPrimitivesReader.SetOffset(it->second.Offset + stringOffset + mName.mNameEntries[i].Offset);
		mPrimitivesReader.Read((Byte*)(mName.mNameEntries[i].String),mName.mNameEntries[i].Length);
	}

	return mPrimitivesReader.GetInternalState();	
}

EStatusCode OpenTypeFileInput::ReadLoca()
{
	ULongToTableEntryMap::iterator it = mTables.find(GetTag("loca"));
	if(it == mTables.end())
	{
		TRACE_LOG("OpenTypeFileInput::ReadLoca, could not find loca table");
		return PDFHummus::eFailure;
	}
	mPrimitivesReader.SetOffset(it->second.Offset);	

	mLoca = new unsigned long[mMaxp.NumGlyphs+1];

	if(0 == mHead.IndexToLocFormat)
	{
		unsigned short buffer;
		for(unsigned short i=0; i < mMaxp.NumGlyphs+1; ++i)
		{
			mPrimitivesReader.ReadUSHORT(buffer);
			mLoca[i] = buffer << 1;
		}
	}
	else
	{
		for(unsigned short i=0; i < mMaxp.NumGlyphs+1; ++i)
			mPrimitivesReader.ReadULONG(mLoca[i]);
	}
	return mPrimitivesReader.GetInternalState();	
}

EStatusCode OpenTypeFileInput::ReadGlyfForDependencies()
{
	ULongToTableEntryMap::iterator it = mTables.find(GetTag("glyf"));
	if(it == mTables.end())
	{
		TRACE_LOG("OpenTypeFileInput::ReadGlyfForDependencies, could not find glyf table");
		return PDFHummus::eFailure;
	}

	// it->second.Offset, is the offset to the beginning of the table
	mGlyf = new GlyphEntry*[mMaxp.NumGlyphs];

	for(unsigned short i=0; i < mMaxp.NumGlyphs; ++i)
	{
		if(mLoca[i+1] == mLoca[i])
		{
			mGlyf[i] = NULL;
		}
		else
		{
			mGlyf[i] = new GlyphEntry;

			mPrimitivesReader.SetOffset(it->second.Offset + mLoca[i]);
			mPrimitivesReader.ReadSHORT(mGlyf[i]->NumberOfContours);
			mPrimitivesReader.ReadSHORT(mGlyf[i]->XMin);
			mPrimitivesReader.ReadSHORT(mGlyf[i]->YMin);
			mPrimitivesReader.ReadSHORT(mGlyf[i]->XMax);
			mPrimitivesReader.ReadSHORT(mGlyf[i]->YMax);

			// Now look for dependencies
			if(mGlyf[i]->NumberOfContours < 0)
			{
				bool hasMoreComponents;
				unsigned short flags;
				unsigned short glyphIndex;

				do
				{
					mPrimitivesReader.ReadUSHORT(flags);
					mPrimitivesReader.ReadUSHORT(glyphIndex);
                    
                    if (glyphIndex >= mMaxp.NumGlyphs) {
                        TRACE_LOG("OpenTypeFileInput::ReadGlyfForDependencies, dependent glyph out of range");
                        return PDFHummus::eFailure;
                    }
                    
					mGlyf[i]->mComponentGlyphs.push_back(glyphIndex);
					if((flags & 1) != 0) // 
						mPrimitivesReader.Skip(4); // skip 2 shorts, ARG_1_AND_2_ARE_WORDS
					else
						mPrimitivesReader.Skip(2); // skip 1 short, nah - they are bytes

					if((flags & 8) != 0)
						mPrimitivesReader.Skip(2); // WE_HAVE_SCALE
					else if ((flags & 64) != 0)
						mPrimitivesReader.Skip(4); // WE_HAVE_AN_X_AND_Y_SCALE
					else if ((flags & 128) != 0)
						mPrimitivesReader.Skip(8); // WE_HAVE_A_TWO_BY_TWO

					hasMoreComponents = ((flags & 32) != 0);
				}while(hasMoreComponents);

			}

			mActualGlyphs.insert(UShortToGlyphEntryMap::value_type(i,mGlyf[i]));
		}
	}	


	return mPrimitivesReader.GetInternalState();	
}

unsigned short OpenTypeFileInput::GetGlyphsCount()
{
	return mMaxp.NumGlyphs;
}

TableEntry* OpenTypeFileInput::GetTableEntry(const char* inTagName)
{
	ULongToTableEntryMap::iterator it = mTables.find(GetTag(inTagName));
	
	if(it == mTables.end())	
		return NULL;
	else
		return &(it->second);
}

EStatusCode OpenTypeFileInput::ReadCFF()
{
	ULongToTableEntryMap::iterator it = mTables.find(GetTag("CFF "));
	if(it == mTables.end())
	{
		TRACE_LOG("OpenTypeFileInput::ReadCFF, could not find cff table entry");
		return PDFHummus::eFailure;
	}

	mPrimitivesReader.SetOffset(it->second.Offset);

	return mCFF.ReadCFFFile(mPrimitivesReader.GetReadStream());
}

EOpenTypeInputType OpenTypeFileInput::GetOpenTypeFontType()
{
	return mFontType;
}







