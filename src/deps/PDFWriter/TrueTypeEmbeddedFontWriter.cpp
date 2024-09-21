/*
   Source File : TrueTypeEmbeddedFontWriter.cpp


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
#include "TrueTypeEmbeddedFontWriter.h"
#include "FreeTypeFaceWrapper.h"
#include "ObjectsContext.h"
#include "DictionaryContext.h"
#include "Trace.h"
#include "PDFStream.h"
#include "OutputStreamTraits.h"
#include "InputStringBufferStream.h"
#include "OpenTypeFileInput.h"
#include "FSType.h"

#include <sstream>
#include <algorithm>


using namespace PDFHummus;

TrueTypeEmbeddedFontWriter::TrueTypeEmbeddedFontWriter(void):mFontFileReaderStream(NULL)
{
}

TrueTypeEmbeddedFontWriter::~TrueTypeEmbeddedFontWriter(void)
{
}

static const std::string scLength1 = "Length1";
EStatusCode TrueTypeEmbeddedFontWriter::WriteEmbeddedFont(	
								FreeTypeFaceWrapper& inFontInfo,
								const UIntVector& inSubsetGlyphIDs,
								ObjectsContext* inObjectsContext,
								ObjectIDType& outEmbeddedFontObjectID)
{
	MyStringBuf rawFontProgram;
	bool notEmbedded;
	EStatusCode status;

	do
	{
		status = CreateTrueTypeSubset(inFontInfo,inSubsetGlyphIDs,notEmbedded,rawFontProgram);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("TrueTypeEmbeddedFontWriter::WriteEmbeddedFont, failed to write embedded font program");
			break;
		}	

		if(notEmbedded)
		{
			// can't embed. mark succesful, and go back empty
			outEmbeddedFontObjectID = 0;
			TRACE_LOG("TrueTypeEmbeddedFontWriter::WriteEmbeddedFont, font may not be embedded. so not embedding");
			return PDFHummus::eSuccess;
		}

		outEmbeddedFontObjectID = inObjectsContext->StartNewIndirectObject();
		
		DictionaryContext* fontProgramDictionaryContext = inObjectsContext->StartDictionary();

		// Length1 (decompressed true type program length)

		fontProgramDictionaryContext->WriteKey(scLength1);
		fontProgramDictionaryContext->WriteIntegerValue(rawFontProgram.GetCurrentWritePosition());
		rawFontProgram.pubseekoff(0,std::ios_base::beg);
		PDFStream* pdfStream = inObjectsContext->StartPDFStream(fontProgramDictionaryContext);


		// now copy the created font program to the output stream
		InputStringBufferStream fontProgramStream(&rawFontProgram);
		OutputStreamTraits streamCopier(pdfStream->GetWriteStream());
		status = streamCopier.CopyToOutputStream(&fontProgramStream);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("TrueTypeEmbeddedFontWriter::WriteEmbeddedFont, failed to copy font program into pdf stream");
			break;
		}


		inObjectsContext->EndPDFStream(pdfStream);
		delete pdfStream;
	}while(false);

	return status;
}

EStatusCode TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset(	FreeTypeFaceWrapper& inFontInfo, /*consider requiring only the file path...actually i don't need the whole thing*/
																const UIntVector& inSubsetGlyphIDs,
																bool& outNotEmbedded,
																MyStringBuf& outFontProgram)
{
	EStatusCode status;
	unsigned long* locaTable = NULL;

	do
	{
		UIntVector subsetGlyphIDs = inSubsetGlyphIDs;

		status = mTrueTypeFile.OpenFile(inFontInfo.GetFontFilePath());
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG1("TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset, cannot open true type font file at %s",inFontInfo.GetFontFilePath().c_str());
			break;
		}

		status = mTrueTypeInput.ReadOpenTypeFile(mTrueTypeFile.GetInputStream(),(unsigned short)inFontInfo.GetFontIndex());
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset, failed to read true type file");
			break;
		}

		if(mTrueTypeInput.GetOpenTypeFontType() != EOpenTypeTrueType)
		{
			TRACE_LOG("TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset, font file is not true type, so there is an exceptions here. expecting true types only");
			break;
		}
	
		// see if font may be embedded
		if(mTrueTypeInput.mOS2Exists && !FSType(mTrueTypeInput.mOS2.fsType).CanEmbed())
		{
			outNotEmbedded = true;
			return PDFHummus::eSuccess;
		}
		else
			outNotEmbedded = false;

		AddDependentGlyphs(subsetGlyphIDs);

		// K. this needs a bit explaining.
		// i want to leave the glyph IDs as they were in the original font.
		// this allows me to write a more comfotable font definition. something which is generic enough
		// this assumption requires that the font will contain the glyphs in their original position
		// to allow that, when the glyph count is smaller than the actual glyphs count, i'm
		// padding with 0 length glyphs (their loca entries just don't move).
		// don't worry - it's perfectly kosher.
		// so - bottom line - the glyphs count will actually be 1 more than the maxium glyph index.
		// and from here i'll just place the glyphs in their original indexes, and fill in the 
		// vacant glyphs with empties.
		unsigned short maxGlyf = subsetGlyphIDs.back();
		if(maxGlyf >= mTrueTypeInput.mMaxp.NumGlyphs)
		{
			TRACE_LOG2("TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset, error, maximum requested glyph index %ld is larger than the maximum glyph index for this font which is %ld. ",maxGlyf,mTrueTypeInput.mMaxp.NumGlyphs-1);
			status = eFailure;
			break;
		}
		mSubsetFontGlyphsCount = maxGlyf + 1;
		
		mFontFileStream.Assign(&outFontProgram);
		mPrimitivesWriter.SetOpenTypeStream(&mFontFileStream);

		// assign also to some reader streams, so i can read items for checksums calculations
		mFontFileReaderStream.Assign(&outFontProgram);
		mPrimitivesReader.SetOpenTypeStream(&mFontFileReaderStream);


		status = WriteTrueTypeHeader();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset, failed to write true type header");
			break;
		}

		status = WriteHead();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset, failed to write head table");
			break;
		}

		status = WriteHHea();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset, failed to write hhea table");
			break;
		}

		status = WriteHMtx();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset, failed to write hmtx table");
			break;
		}

		status = WriteMaxp();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset, failed to write Maxp table");
			break;
		}

		if(mTrueTypeInput.mCVTExists)
		{
			status = WriteCVT();
			if(status != PDFHummus::eSuccess)
			{
				TRACE_LOG("TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset, failed to write cvt table");
				break;
			}
		}

		if(mTrueTypeInput.mFPGMExists)
		{
			status = WriteFPGM();
			if(status != PDFHummus::eSuccess)
			{
				TRACE_LOG("TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset, failed to write fpgm table");
				break;
			}
		}

		if(mTrueTypeInput.mPREPExists)
		{
			status = WritePREP();
			if(status != PDFHummus::eSuccess)
			{
				TRACE_LOG("TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset, failed to write prep table");
				break;	
			}
		}

		status = WriteNAME();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset, failed to write name table");
			break;	
		}

        if(mTrueTypeInput.mOS2Exists)
        {
            status = WriteOS2();
            if(status != PDFHummus::eSuccess)
            {
                TRACE_LOG("TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset, failed to write os2 table");
                break;
            }
        }

		status = WriteCMAP();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset, failed to write cmap table");
			break;	
		}

		locaTable = new unsigned long[mSubsetFontGlyphsCount+1];

		status = WriteGlyf(subsetGlyphIDs,locaTable);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset, failed to write prep table");
			break;
		}

		status = WriteLoca(locaTable);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("TrueTypeEmbeddedFontWriter::CreateTrueTypeSubset, failed to write loca table");
			break;
		}

		status = CreateHeadTableCheckSumAdjustment();
	}while(false);

	delete[] locaTable;
	mTrueTypeFile.CloseFile();
	return status;
}

void TrueTypeEmbeddedFontWriter::AddDependentGlyphs(UIntVector& ioSubsetGlyphIDs)
{
	UIntSet glyphsSet;
	UIntVector::iterator it = ioSubsetGlyphIDs.begin();
	bool hasCompositeGlyphs = false;

	for(;it != ioSubsetGlyphIDs.end(); ++it)
		hasCompositeGlyphs |= AddComponentGlyphs(*it,glyphsSet);

	if(hasCompositeGlyphs)
	{
		UIntSet::iterator itNewGlyphs;

		for(it = ioSubsetGlyphIDs.begin();it != ioSubsetGlyphIDs.end(); ++it)
			glyphsSet.insert(*it);

		ioSubsetGlyphIDs.clear();
		for(itNewGlyphs = glyphsSet.begin(); itNewGlyphs != glyphsSet.end(); ++itNewGlyphs)
			ioSubsetGlyphIDs.push_back(*itNewGlyphs);
		
		sort(ioSubsetGlyphIDs.begin(),ioSubsetGlyphIDs.end());
	}
}

bool TrueTypeEmbeddedFontWriter::AddComponentGlyphs(unsigned int inGlyphID,UIntSet& ioComponents)
{
	GlyphEntry* glyfTableEntry;
	UIntList::iterator itComponentGlyphs;
	bool isComposite = false;

	if(inGlyphID >= mTrueTypeInput.mMaxp.NumGlyphs)
	{
		TRACE_LOG2("TrueTypeEmbeddedFontWriter::AddComponentGlyphs, error, requested glyph index %ld is larger than the maximum glyph index for this font which is %ld. ",inGlyphID,mTrueTypeInput.mMaxp.NumGlyphs-1);
		return false;
	}

	glyfTableEntry = mTrueTypeInput.mGlyf[inGlyphID];
	if(glyfTableEntry != NULL && glyfTableEntry->mComponentGlyphs.size() > 0)
	{
		isComposite = true;
		for(itComponentGlyphs = glyfTableEntry->mComponentGlyphs.begin(); 
				itComponentGlyphs != glyfTableEntry->mComponentGlyphs.end(); 
				++itComponentGlyphs)
		{
				ioComponents.insert(*itComponentGlyphs);
				AddComponentGlyphs(*itComponentGlyphs,ioComponents);
		}
	}
	return isComposite;
}

unsigned short TrueTypeEmbeddedFontWriter::GetSmallerPower2(unsigned short inNumber)
{
	unsigned short comparer = inNumber > 0xff ? 0x8000:0x80; 
	// that's 1 binary at the leftmost of the short or byte (most times there are less than 255 tables)
	unsigned int i= inNumber > 0xff ? 15 : 7;

	// now basically i'm gonna move the comparer down 1 bit every time, till i hit non 0, which
	// is the highest power

	while(comparer > 0 && ((inNumber & comparer) == 0))
	{
		comparer = comparer >> 1;
		--i;
	}
	return i;
}

EStatusCode TrueTypeEmbeddedFontWriter::WriteTrueTypeHeader()
{
	// prepare space for tables to write. will write (at maximum) - 
	// cmap, cvt, fpgm, glyf, head, hhea, hmtx, loca, maxp, name, os/2, prep

	unsigned short tableCount = 
		9	// needs - cmap, glyf, head, hhea, hmtx, loca, maxp, name, OS/2
		+
		(mTrueTypeInput.mCVTExists ? 1:0) + // cvt
		(mTrueTypeInput.mPREPExists ? 1:0) + // prep
		(mTrueTypeInput.mFPGMExists ? 1:0); // fpgm

	// here we go....
	mPrimitivesWriter.WriteULONG(0x10000);
	mPrimitivesWriter.WriteUSHORT(tableCount);
	unsigned short smallerPowerTwo = GetSmallerPower2(tableCount);
	mPrimitivesWriter.WriteUSHORT(1 << (smallerPowerTwo + 4));
	mPrimitivesWriter.WriteUSHORT(smallerPowerTwo);
	mPrimitivesWriter.WriteUSHORT((tableCount - (1<<smallerPowerTwo)) << 4);

	if (mTrueTypeInput.mOS2Exists)
		WriteEmptyTableEntry("OS/2", mOS2EntryWritingOffset);
	WriteEmptyTableEntry("cmap", mCMAPEntryWritingOffset);
	if(mTrueTypeInput.mCVTExists)
		WriteEmptyTableEntry("cvt ",mCVTEntryWritingOffset);
	if(mTrueTypeInput.mFPGMExists)
		WriteEmptyTableEntry("fpgm",mFPGMEntryWritingOffset);
	WriteEmptyTableEntry("glyf",mGLYFEntryWritingOffset);
	WriteEmptyTableEntry("head",mHEADEntryWritingOffset);
	WriteEmptyTableEntry("hhea",mHHEAEntryWritingOffset);
	WriteEmptyTableEntry("hmtx",mHMTXEntryWritingOffset);
	WriteEmptyTableEntry("loca",mLOCAEntryWritingOffset);
	WriteEmptyTableEntry("maxp",mMAXPEntryWritingOffset);
	WriteEmptyTableEntry("name",mNAMEEntryWritingOffset);
	if(mTrueTypeInput.mPREPExists)
		WriteEmptyTableEntry("prep",mPREPEntryWritingOffset);

	mPrimitivesWriter.PadTo4();

	return mPrimitivesWriter.GetInternalState();
}	

void TrueTypeEmbeddedFontWriter::WriteEmptyTableEntry(const char* inTag,LongFilePositionType& outEntryPosition)
{
	mPrimitivesWriter.WriteULONG(GetTag(inTag));
	outEntryPosition = mFontFileStream.GetCurrentPosition();
	mPrimitivesWriter.Pad(12);
}


unsigned long TrueTypeEmbeddedFontWriter::GetTag(const char* inTagName)
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

EStatusCode TrueTypeEmbeddedFontWriter::WriteHead()
{
	// copy as is, then adjust loca table format to long (that's what i'm always writing), 
	// set the checksum
	// and store the offset to the checksum

	TableEntry* tableEntry = mTrueTypeInput.GetTableEntry("head");
	if (!tableEntry) {
		return PDFHummus::eFailure;
	}
	LongFilePositionType startTableOffset;
	OutputStreamTraits streamCopier(&mFontFileStream);
	LongFilePositionType endOfStream;

	startTableOffset = mFontFileStream.GetCurrentPosition();

	// copy and save the current position
	mTrueTypeFile.GetInputStream()->SetPosition(tableEntry->Offset);
	streamCopier.CopyToOutputStream(mTrueTypeFile.GetInputStream(),tableEntry->Length);
	mPrimitivesWriter.PadTo4();
	endOfStream = mFontFileStream.GetCurrentPosition();

	// set the checksum to 0, and save its position for later update
	mHeadCheckSumOffset = startTableOffset + 8;
	mFontFileStream.SetPosition(mHeadCheckSumOffset);
	mPrimitivesWriter.WriteULONG(0);

	// set the loca format to longs
	mFontFileStream.SetPosition(startTableOffset + 50);
	mPrimitivesWriter.WriteUSHORT(1);

	// write table entry data, which includes movement
	WriteTableEntryData(mHEADEntryWritingOffset,startTableOffset,tableEntry->Length);

	// restore position to end of stream
	mFontFileStream.SetPosition(endOfStream); 

	return mPrimitivesWriter.GetInternalState();
}

void TrueTypeEmbeddedFontWriter::WriteTableEntryData(
														LongFilePositionType inTableEntryOffset,
														LongFilePositionType inTableOffset,
														unsigned long inTableLength)
{
	unsigned long checksum = GetCheckSum(inTableOffset,inTableLength);

	mFontFileStream.SetPosition(inTableEntryOffset);
	mPrimitivesWriter.WriteULONG(checksum);
	mPrimitivesWriter.WriteULONG((unsigned long)inTableOffset);
	mPrimitivesWriter.WriteULONG(inTableLength);

}

unsigned long TrueTypeEmbeddedFontWriter::GetCheckSum(LongFilePositionType inOffset,
														unsigned long inLength)
{
	unsigned long sum = 0L;
	unsigned long endPosition = (unsigned long)(inOffset +((inLength+3) & ~3) / 4);
	unsigned long position = (unsigned long)inOffset;
	unsigned long value;

	mFontFileStream.SetPosition(inOffset);

	while (position < endPosition)
	{
		mPrimitivesReader.ReadULONG(value);
		sum += value;
		position+=4;
	}
	return sum;
}

EStatusCode TrueTypeEmbeddedFontWriter::WriteHHea()
{
	// copy as is, then possibly adjust the hmtx NumberOfHMetrics field, if the glyphs
	// count is lower

	TableEntry* tableEntry = mTrueTypeInput.GetTableEntry("hhea");
	if (!tableEntry) {
		return PDFHummus::eFailure;
	}
	LongFilePositionType startTableOffset;
	OutputStreamTraits streamCopier(&mFontFileStream);
	LongFilePositionType endOfStream;

	startTableOffset = mFontFileStream.GetCurrentPosition();

	// copy and save the current position
	mTrueTypeFile.GetInputStream()->SetPosition(tableEntry->Offset);
	streamCopier.CopyToOutputStream(mTrueTypeFile.GetInputStream(),tableEntry->Length);
	mPrimitivesWriter.PadTo4();
	endOfStream = mFontFileStream.GetCurrentPosition();

	// adjust the NumberOfHMetrics if necessary
	if(mTrueTypeInput.mHHea.NumberOfHMetrics > mSubsetFontGlyphsCount)
	{
		mFontFileStream.SetPosition(startTableOffset + tableEntry->Length - 2);
		mPrimitivesWriter.WriteUSHORT(mSubsetFontGlyphsCount);
	}

	// write table entry data, which includes movement
	WriteTableEntryData(mHHEAEntryWritingOffset,startTableOffset,tableEntry->Length);

	// restore position to end of stream
	mFontFileStream.SetPosition(endOfStream); 

	return mPrimitivesWriter.GetInternalState();	
}

EStatusCode TrueTypeEmbeddedFontWriter::WriteHMtx()
{
	// k. basically i'm supposed to fill this up till the max glyph count.
	// so i'll just use the original table (keeping an eye on the NumberOfHMetrics)
	// filling with the original values (doesn't really matter for empty glyphs) till the 
	// glyph count

	LongFilePositionType startTableOffset;

	startTableOffset = mFontFileStream.GetCurrentPosition();

	// write the table. write pairs until min(numberofhmetrics,mSubsetFontGlyphsCount)
	// then if mSubsetFontGlyphsCount > numberofhmetrics writh the width metrics as well
	unsigned numberOfHMetrics = std::min(mTrueTypeInput.mHHea.NumberOfHMetrics,mSubsetFontGlyphsCount);
	unsigned short i=0;
	for(;i<numberOfHMetrics;++i)
	{
		mPrimitivesWriter.WriteUSHORT(mTrueTypeInput.mHMtx[i].AdvanceWidth);
		mPrimitivesWriter.WriteSHORT(mTrueTypeInput.mHMtx[i].LeftSideBearing);
	}
	for(;i<mSubsetFontGlyphsCount;++i)
		mPrimitivesWriter.WriteSHORT(mTrueTypeInput.mHMtx[i].LeftSideBearing);

	LongFilePositionType endOfTable = mFontFileStream.GetCurrentPosition();
	mPrimitivesWriter.PadTo4();
	LongFilePositionType endOfStream = mFontFileStream.GetCurrentPosition();

	// write table entry data, which includes movement
	WriteTableEntryData(mHMTXEntryWritingOffset,
						startTableOffset,
						(unsigned long)(endOfTable - startTableOffset));

	// restore position to end of stream
	mFontFileStream.SetPosition(endOfStream); 

	return mPrimitivesWriter.GetInternalState();	
}

EStatusCode TrueTypeEmbeddedFontWriter::WriteMaxp()
{
	// copy as is, then adjust the glyphs count

	TableEntry* tableEntry = mTrueTypeInput.GetTableEntry("maxp");
	if (!tableEntry) {
		return PDFHummus::eFailure;
	}
	LongFilePositionType startTableOffset;
	OutputStreamTraits streamCopier(&mFontFileStream);
	LongFilePositionType endOfStream;

	startTableOffset = mFontFileStream.GetCurrentPosition();

	// copy and save the current position
	mTrueTypeFile.GetInputStream()->SetPosition(tableEntry->Offset);
	streamCopier.CopyToOutputStream(mTrueTypeFile.GetInputStream(),tableEntry->Length);
	mPrimitivesWriter.PadTo4();
	endOfStream = mFontFileStream.GetCurrentPosition();

	// adjust the glyphs count if necessary
	mFontFileStream.SetPosition(startTableOffset + 4);
	mPrimitivesWriter.WriteUSHORT(mSubsetFontGlyphsCount);

	// write table entry data, which includes movement
	WriteTableEntryData(mMAXPEntryWritingOffset,startTableOffset,tableEntry->Length);

	// restore position to end of stream
	mFontFileStream.SetPosition(endOfStream); 

	return mPrimitivesWriter.GetInternalState();
}

EStatusCode TrueTypeEmbeddedFontWriter::WriteCVT()
{
	return CreateTableCopy("cvt ",mCVTEntryWritingOffset);
}

EStatusCode TrueTypeEmbeddedFontWriter::WriteFPGM()
{
	return CreateTableCopy("fpgm",mFPGMEntryWritingOffset);
}

EStatusCode TrueTypeEmbeddedFontWriter::WritePREP()
{
	return CreateTableCopy("prep",mPREPEntryWritingOffset);
}

EStatusCode TrueTypeEmbeddedFontWriter::WriteGlyf(const UIntVector& inSubsetGlyphIDs,unsigned long* inLocaTable)
{
	// k. write the glyphs table. you only need to write the glyphs you are actually using.
	// while at it...update the locaTable

	TableEntry* tableEntry = mTrueTypeInput.GetTableEntry("glyf");
	if (!tableEntry) {
		return PDFHummus::eFailure;
	}
	LongFilePositionType startTableOffset = mFontFileStream.GetCurrentPosition();
	UIntVector::const_iterator it = inSubsetGlyphIDs.begin();
	OutputStreamTraits streamCopier(&mFontFileStream);
	unsigned short glyphIndex,previousGlyphIndexEnd = 0;
	inLocaTable[0] = 0;
	EStatusCode status = eSuccess;

	for(;it != inSubsetGlyphIDs.end() && eSuccess == status; ++it)
	{
		glyphIndex = *it;
		if(glyphIndex >= mTrueTypeInput.mMaxp.NumGlyphs)
		{
			TRACE_LOG2("TrueTypeEmbeddedFontWriter::WriteGlyf, error, requested glyph index %ld is larger than the maximum glyph index for this font which is %ld. ",glyphIndex,mTrueTypeInput.mMaxp.NumGlyphs-1);
			status = eFailure;
			break;
		}

		for(unsigned short i= previousGlyphIndexEnd + 1; i<=glyphIndex;++i)
			inLocaTable[i] = inLocaTable[previousGlyphIndexEnd];
		if(mTrueTypeInput.mGlyf[glyphIndex] != NULL)
		{
			mTrueTypeFile.GetInputStream()->SetPosition(tableEntry->Offset + 
															mTrueTypeInput.mLoca[glyphIndex]);
			streamCopier.CopyToOutputStream(mTrueTypeFile.GetInputStream(),
				mTrueTypeInput.mLoca[(glyphIndex) + 1] - mTrueTypeInput.mLoca[glyphIndex]);
		}
		inLocaTable[glyphIndex + 1] = (unsigned long)(mFontFileStream.GetCurrentPosition() - startTableOffset);
		previousGlyphIndexEnd = glyphIndex + 1;
	}

	LongFilePositionType endOfTable = mFontFileStream.GetCurrentPosition();
	mPrimitivesWriter.PadTo4();
	LongFilePositionType endOfStream = mFontFileStream.GetCurrentPosition();

	// write table entry data, which includes movement
	WriteTableEntryData(mGLYFEntryWritingOffset,
						startTableOffset,
						(unsigned long)(endOfTable - startTableOffset));

	// restore position to end of stream
	mFontFileStream.SetPosition(endOfStream); 

	return mPrimitivesWriter.GetInternalState();	
}

EStatusCode TrueTypeEmbeddedFontWriter::WriteLoca(unsigned long* inLocaTable)
{
	// k. just write the input locatable. in longs format

	LongFilePositionType startTableOffset = mFontFileStream.GetCurrentPosition();

	// write the table. write pairs until min(numberofhmetrics,mSubsetFontGlyphsCount)
	// then if mSubsetFontGlyphsCount > numberofhmetrics writh the width metrics as well
	for(unsigned short i=0;i<mSubsetFontGlyphsCount + 1;++i)
		mPrimitivesWriter.WriteULONG(inLocaTable[i]);

	LongFilePositionType endOfTable = mFontFileStream.GetCurrentPosition();
	mPrimitivesWriter.PadTo4();
	LongFilePositionType endOfStream = mFontFileStream.GetCurrentPosition();

	// write table entry data, which includes movement
	WriteTableEntryData(mLOCAEntryWritingOffset,
						startTableOffset,
						(unsigned long)(endOfTable - startTableOffset));

	// restore position to end of stream
	mFontFileStream.SetPosition(endOfStream); 

	return mPrimitivesWriter.GetInternalState();	
}

EStatusCode TrueTypeEmbeddedFontWriter::CreateHeadTableCheckSumAdjustment()
{
	LongFilePositionType endStream = mFontFileStream.GetCurrentPosition();
    unsigned long checkSum = 0xb1b0afba - GetCheckSum(0, (unsigned long)endStream);

	mFontFileStream.SetPosition(mHeadCheckSumOffset); 
	mPrimitivesWriter.WriteULONG(checkSum);
	mFontFileStream.SetPosition(endStream); // restoring position just for kicks

	return mPrimitivesWriter.GetInternalState();
}

EStatusCode TrueTypeEmbeddedFontWriter::WriteNAME()
{
	return CreateTableCopy("name",mNAMEEntryWritingOffset);
}

EStatusCode TrueTypeEmbeddedFontWriter::WriteOS2()
{
	return CreateTableCopy("OS/2",mOS2EntryWritingOffset);
}


EStatusCode TrueTypeEmbeddedFontWriter::WriteCMAP()
{
	return CreateTableCopy("cmap",mCMAPEntryWritingOffset);
}

EStatusCode TrueTypeEmbeddedFontWriter::CreateTableCopy(const char* inTableName,LongFilePositionType inTableEntryLocation)
{
	// copy as is, no adjustments required

	TableEntry* tableEntry = mTrueTypeInput.GetTableEntry(inTableName);
	if (!tableEntry) {
		return PDFHummus::eFailure;
	}
	LongFilePositionType startTableOffset;
	OutputStreamTraits streamCopier(&mFontFileStream);
	LongFilePositionType endOfStream;

	startTableOffset = mFontFileStream.GetCurrentPosition();

	// copy and save the current position
	mTrueTypeFile.GetInputStream()->SetPosition(tableEntry->Offset);
	streamCopier.CopyToOutputStream(mTrueTypeFile.GetInputStream(),tableEntry->Length);
	mPrimitivesWriter.PadTo4();
	endOfStream = mFontFileStream.GetCurrentPosition();

	// write table entry data, which includes movement
	WriteTableEntryData(inTableEntryLocation,startTableOffset,tableEntry->Length);

	// restore position to end of stream
	mFontFileStream.SetPosition(endOfStream); 

	return mPrimitivesWriter.GetInternalState();

}

unsigned short TrueTypeEmbeddedFontWriter::GetSubsetFontGlyphsCount() {
	return mSubsetFontGlyphsCount;
}
