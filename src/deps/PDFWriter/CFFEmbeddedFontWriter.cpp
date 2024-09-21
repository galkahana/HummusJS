/*
   Source File : CFFEmbeddedFontWriter.cpp


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
#include "CFFEmbeddedFontWriter.h"
#include "ObjectsContext.h"
#include "InputStringBufferStream.h"
#include "OutputStreamTraits.h"
#include "PDFStream.h"
#include "Trace.h"
#include "FreeTypeFaceWrapper.h"
#include "DictionaryContext.h"
#include "IByteReaderWithPosition.h"
#include "CharStringType2Flattener.h"
#include "FSType.h"

#include <algorithm>
#include <utility>
#include <list>


using namespace PDFHummus;

CFFEmbeddedFontWriter::CFFEmbeddedFontWriter(void)
{
}

CFFEmbeddedFontWriter::~CFFEmbeddedFontWriter(void)
{
}

static const std::string scSubtype = "Subtype";

EStatusCode CFFEmbeddedFontWriter::WriteEmbeddedFont(	
								FreeTypeFaceWrapper& inFontInfo,
								const UIntVector& inSubsetGlyphIDs,
								const std::string& inFontFile3SubType,
								const std::string& inSubsetFontName,
								ObjectsContext* inObjectsContext,
								ObjectIDType& outEmbeddedFontObjectID)
{
	return WriteEmbeddedFont(inFontInfo,inSubsetGlyphIDs,inFontFile3SubType,inSubsetFontName,inObjectsContext,NULL,outEmbeddedFontObjectID);
}

EStatusCode CFFEmbeddedFontWriter::WriteEmbeddedFont(
	FreeTypeFaceWrapper& inFontInfo,
	const UIntVector& inSubsetGlyphIDs,
	const std::string& inFontFile3SubType,
	const std::string& inSubsetFontName,
	ObjectsContext* inObjectsContext,
	UShortVector* inCIDMapping,
	ObjectIDType& outEmbeddedFontObjectID)
{
	MyStringBuf rawFontProgram; 
	bool notEmbedded;
		// as oppose to true type, the reason for using a memory stream here is mainly peformance - i don't want to start
		// setting file pointers and move in a file stream
	EStatusCode status;

	do
	{
		status = CreateCFFSubset(inFontInfo,inSubsetGlyphIDs,inCIDMapping,inSubsetFontName,notEmbedded,rawFontProgram);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("CFFEmbeddedFontWriter::WriteEmbeddedFont, failed to write embedded font program");
			break;
		}	

		if(notEmbedded)
		{
			// can't embed. mark succesful, and go back empty
			outEmbeddedFontObjectID = 0;
			TRACE_LOG("CFFEmbeddedFontWriter::WriteEmbeddedFont, font may not be embedded. so not embedding");
			return PDFHummus::eSuccess;
		}

		outEmbeddedFontObjectID = inObjectsContext->StartNewIndirectObject();
		
		DictionaryContext* fontProgramDictionaryContext = inObjectsContext->StartDictionary();

		rawFontProgram.pubseekoff(0,std::ios_base::beg);

		fontProgramDictionaryContext->WriteKey(scSubtype);
		fontProgramDictionaryContext->WriteNameValue(inFontFile3SubType);
		PDFStream* pdfStream = inObjectsContext->StartPDFStream(fontProgramDictionaryContext);


		// now copy the created font program to the output stream
		InputStringBufferStream fontProgramStream(&rawFontProgram);
		OutputStreamTraits streamCopier(pdfStream->GetWriteStream());
		status = streamCopier.CopyToOutputStream(&fontProgramStream);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("CFFEmbeddedFontWriter::WriteEmbeddedFont, failed to copy font program into pdf stream");
			break;
		}


		inObjectsContext->EndPDFStream(pdfStream);
		delete pdfStream;
	}while(false);

	return status;	
}

static const unsigned short scROS = 0xC1E;
EStatusCode CFFEmbeddedFontWriter::CreateCFFSubset(	
									FreeTypeFaceWrapper& inFontInfo,
									const UIntVector& inSubsetGlyphIDs,
									UShortVector* inCIDMapping,
									const std::string& inSubsetFontName,
									bool& outNotEmbedded,
									MyStringBuf& outFontProgram)
{
	EStatusCode status;

	do
	{

		status = mOpenTypeFile.OpenFile(inFontInfo.GetFontFilePath());
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG1("CFFEmbeddedFontWriter::CreateCFFSubset, cannot open type font file at %s",inFontInfo.GetFontFilePath().c_str());
			break;
		}

		status = mOpenTypeInput.ReadOpenTypeFile(mOpenTypeFile.GetInputStream(),(unsigned short)inFontInfo.GetFontIndex());
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("CFFEmbeddedFontWriter::CreateCFFSubset, failed to read true type file");
			break;
		}

		if(mOpenTypeInput.GetOpenTypeFontType() != EOpenTypeCFF)
		{
			TRACE_LOG("CFFEmbeddedFontWriter::CreateCFFSubset, font file is not CFF, so there is an exceptions here. expecting CFFs only");
			break;
		}

		// see if font may be embedded
		if(mOpenTypeInput.mOS2Exists && !FSType(mOpenTypeInput.mOS2.fsType).CanEmbed())
		{
			outNotEmbedded = true;
			return PDFHummus::eSuccess;
		}
		else
			outNotEmbedded = false;

		UIntVector subsetGlyphIDs = inSubsetGlyphIDs;
		if(subsetGlyphIDs.front() != 0) // make sure 0 glyph is in
			subsetGlyphIDs.insert(subsetGlyphIDs.begin(),0);

		status = AddDependentGlyphs(subsetGlyphIDs);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("CFFEmbeddedFontWriter::CreateCFFSubset, failed to add dependent glyphs");
			break;
		}

		mSubsetFontGlyphsCount = subsetGlyphIDs.size();

		mIsCID = mOpenTypeInput.mCFF.mTopDictIndex[0].mTopDict.find(scROS) != 
					mOpenTypeInput.mCFF.mTopDictIndex[0].mTopDict.end();

		mFontFileStream.Assign(&outFontProgram);
		mPrimitivesWriter.SetStream(&mFontFileStream);

		status = WriteCFFHeader();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("CFFEmbeddedFontWriter::CreateCFFSubset, failed to write CFF header");
			break;
		}

		status = WriteName(inSubsetFontName);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("CFFEmbeddedFontWriter::CreateCFFSubset, failed to write CFF Name");
			break;
		}

		status = WriteTopIndex();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("CFFEmbeddedFontWriter::CreateCFFSubset, failed to write Top Index");
			break;
		}

		status = WriteStringIndex();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("CFFEmbeddedFontWriter::CreateCFFSubset, failed to write String Index");
			break;
		}

		status = WriteGlobalSubrsIndex();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("CFFEmbeddedFontWriter::CreateCFFSubset, failed to write global subrs index");
			break;
		}

		status = WriteEncodings(inSubsetGlyphIDs);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("CFFEmbeddedFontWriter::CreateCFFSubset, failed to write encodings");
			break;
		}

		status = WriteCharsets(inSubsetGlyphIDs,inCIDMapping);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("CFFEmbeddedFontWriter::CreateCFFSubset, failed to write charstring");
			break;
		}

		FontDictInfoToByteMap newFDIndexes;

		if(mIsCID)
		{
			DetermineFDArrayIndexes(inSubsetGlyphIDs,newFDIndexes);
			status = WriteFDSelect(inSubsetGlyphIDs,newFDIndexes);
			if(status != PDFHummus::eSuccess)
				break;
		}


		status = WriteCharStrings(inSubsetGlyphIDs);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("CFFEmbeddedFontWriter::CreateCFFSubset, failed to write charstring");
			break;
		}

		status = WritePrivateDictionary();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("CFFEmbeddedFontWriter::CreateCFFSubset, failed to write private");
			break;
		}

		if(mIsCID)
		{
			status = WriteFDArray(inSubsetGlyphIDs,newFDIndexes);
			if(status != PDFHummus::eSuccess)
				break;
		}

		status = UpdateIndexesAtTopDict();
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("CFFEmbeddedFontWriter::CreateCFFSubset, failed to update indexes");
			break;			
		}
	}while(false);

	mOpenTypeFile.CloseFile();
	return status;
}

EStatusCode CFFEmbeddedFontWriter::AddDependentGlyphs(UIntVector& ioSubsetGlyphIDs)
{
	EStatusCode status = PDFHummus::eSuccess;
	UIntSet glyphsSet;
	UIntVector::iterator it = ioSubsetGlyphIDs.begin();
	bool hasCompositeGlyphs = false;

	for(;it != ioSubsetGlyphIDs.end() && PDFHummus::eSuccess == status; ++it)
	{
		bool localHasCompositeGlyphs;
		status = AddComponentGlyphs(*it,glyphsSet,localHasCompositeGlyphs);
		hasCompositeGlyphs |= localHasCompositeGlyphs;
	}

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
	return status;
}

EStatusCode CFFEmbeddedFontWriter::AddComponentGlyphs(unsigned int inGlyphID,UIntSet& ioComponents,bool &outFoundComponents)
{
	CharString2Dependencies dependencies;
	EStatusCode status = mOpenTypeInput.mCFF.CalculateDependenciesForCharIndex(0,inGlyphID,dependencies);

	if(PDFHummus::eSuccess == status && dependencies.mCharCodes.size() !=0)
	{
		UShortSet::iterator it = dependencies.mCharCodes.begin();
		for(; it != dependencies.mCharCodes.end() && PDFHummus::eSuccess == status; ++it)
		{
			bool dummyFound;
			ioComponents.insert(*it);
			status = AddComponentGlyphs(*it,ioComponents,dummyFound);
		}
		outFoundComponents = true;
	}
	else
		outFoundComponents = false;
	return status;
}

EStatusCode CFFEmbeddedFontWriter::WriteCFFHeader()
{
	 // i'm just gonna copy the header of the original CFF
	 // content. 
	 // One thing i never got - OffSize does not seem to be important.
	 // all offeet references to (0) are dictionary items (like in Top Dict),
	 // and reading them follows the Integer operand rules. so why signify their size.
	 // it's probably even not true, cause i guess font writers write integers using the most
	 // compressed method, so if the number is small they'll use less bytes, and if large more.
	 // so i don't get it. hope it won't screw up my implementation. in any case, for the sake of a single pass.
	 // i'll probably just set it to something.
	
	OutputStreamTraits streamCopier(&mFontFileStream);
	mOpenTypeFile.GetInputStream()->SetPosition(mOpenTypeInput.mCFF.mCFFOffset);
	return streamCopier.CopyToOutputStream(mOpenTypeFile.GetInputStream(),mOpenTypeInput.mCFF.mHeader.hdrSize);
}

EStatusCode CFFEmbeddedFontWriter::WriteName(const std::string& inSubsetFontName)
{
	// get the first name from the name table, and write it here

	std::string fontName = inSubsetFontName.size() == 0 ? mOpenTypeInput.mCFF.mName.front() : inSubsetFontName;

	Byte sizeOfOffset = GetMostCompressedOffsetSize((unsigned long)fontName.size() + 1);

	mPrimitivesWriter.WriteCard16(1);
	mPrimitivesWriter.WriteOffSize(sizeOfOffset);
	mPrimitivesWriter.SetOffSize(sizeOfOffset);
	mPrimitivesWriter.WriteOffset(1);
	mPrimitivesWriter.WriteOffset((unsigned long)fontName.size() + 1);
	mPrimitivesWriter.Write((const Byte*)fontName.c_str(),fontName.size());

	return mPrimitivesWriter.GetInternalState();
}

Byte CFFEmbeddedFontWriter::GetMostCompressedOffsetSize(unsigned long inOffset)
{
	if(inOffset < 256)
		return 1;
	
	if(inOffset < 65536)
		return 2;

	if(inOffset < 1<<24)
		return 3;

	return 4;
}

EStatusCode CFFEmbeddedFontWriter::WriteTopIndex()
{
	/*
		what do i have to do:
		- write the top dictionary to a separate segment
			- make sure to write the ROS variable first, if one exists.
			- make sure to avoid writing any of the offset variables.
			- leave placeholders for any of the offset variables. make them maximum size. note
				to leave items for fdarray and fdselect only if required being a CID
			- be aware of the placeholders locations relative to the beginning of the segment
		- calculate the size of the segment
		- write the appropriate index header
		- write the segment
		- adjust the placeholders offset relative to the beginning of the file.

	*/
	EStatusCode status;
	MyStringBuf topDictSegment; 

	do
	{
		status = WriteTopDictSegment(topDictSegment);
		if(status != PDFHummus::eSuccess)
			break;


		// write index section
		Byte sizeOfOffset = GetMostCompressedOffsetSize((unsigned long)topDictSegment.GetCurrentWritePosition() + 1);

		mPrimitivesWriter.WriteCard16(1);
		mPrimitivesWriter.WriteOffSize(sizeOfOffset);
		mPrimitivesWriter.SetOffSize(sizeOfOffset);
		mPrimitivesWriter.WriteOffset(1);
		mPrimitivesWriter.WriteOffset((unsigned long)topDictSegment.GetCurrentWritePosition() + 1);

		topDictSegment.pubseekoff(0,std::ios_base::beg);

		LongFilePositionType topDictDataOffset = mFontFileStream.GetCurrentPosition();

		// Write data
		InputStringBufferStream topDictStream(&topDictSegment);
		OutputStreamTraits streamCopier(&mFontFileStream);
		status = streamCopier.CopyToOutputStream(&topDictStream);
		if(status != PDFHummus::eSuccess)
			break;

		// Adjust position locators for important placeholders
		mCharsetPlaceHolderPosition+=topDictDataOffset;
		mEncodingPlaceHolderPosition+=topDictDataOffset;
		mCharstringsPlaceHolderPosition+=topDictDataOffset;
		mPrivatePlaceHolderPosition+=topDictDataOffset;
		mFDArrayPlaceHolderPosition+=topDictDataOffset;
		mFDSelectPlaceHolderPosition+=topDictDataOffset;		

	}while(false);

	if(status != PDFHummus::eSuccess)
		return status;
	else
		return mPrimitivesWriter.GetInternalState();
	return status;
}

#define N_STD_STRINGS 391

static const unsigned short scCharset = 15;
static const unsigned short scEncoding = 16;
static const unsigned short scCharstrings = 17;
static const unsigned short scPrivate = 18;
static const unsigned short scFDArray = 0xC24;
static const unsigned short scFDSelect = 0xC25;
static const unsigned short scEmbeddedPostscript = 0xC15;
EStatusCode CFFEmbeddedFontWriter::WriteTopDictSegment(MyStringBuf& ioTopDictSegment)
{
	OutputStringBufferStream topDictStream(&ioTopDictSegment);
	CFFPrimitiveWriter dictPrimitiveWriter;
	UShortToDictOperandListMap::iterator itROS;
	UShortToDictOperandListMap::iterator it;
	dictPrimitiveWriter.SetStream(&topDictStream);

	UShortToDictOperandListMap& originalTopDictRef = mOpenTypeInput.mCFF.mTopDictIndex[0].mTopDict;

	itROS = originalTopDictRef.find(scROS);

	 // make sure to write ROS first, if one exists
	if(mIsCID)
		dictPrimitiveWriter.WriteDictItems(itROS->first,itROS->second);

	// write all keys, excluding those that we want to write on our own
	for(it = originalTopDictRef.begin(); it != originalTopDictRef.end();++it)
	{
		if(	it->first != scROS &&
			it->first != scCharset &&
			it->first != scEncoding &&
			it->first != scCharstrings &&
			it->first != scPrivate &&
			it->first != scFDArray &&
			it->first != scFDSelect)
				dictPrimitiveWriter.WriteDictItems(it->first,it->second);
	}
	// check if it had an embedded postscript (which would normally be the FSType implementation).
	// if not...create one to implement the FSType
	if(originalTopDictRef.find(scEmbeddedPostscript) == originalTopDictRef.end() && mOpenTypeInput.mOS2Exists)
	{
		// no need for sophistication here...you can consider this as the only string to be added.
		// so can be sure that its index would be the current count 
		std::stringstream formatter;
		formatter<<"/FSType "<<mOpenTypeInput.mOS2.fsType<<" def";
		mOptionalEmbeddedPostscript = formatter.str();
		dictPrimitiveWriter.WriteIntegerOperand(mOpenTypeInput.mCFF.mStringsCount + N_STD_STRINGS);
		dictPrimitiveWriter.WriteDictOperator(scEmbeddedPostscript);
	}
	else
		mOptionalEmbeddedPostscript = "";

	// now leave placeholders, record their positions
	mCharsetPlaceHolderPosition = topDictStream.GetCurrentPosition();
	dictPrimitiveWriter.Pad5Bytes();
	dictPrimitiveWriter.WriteDictOperator(scCharset);
	mCharstringsPlaceHolderPosition = topDictStream.GetCurrentPosition();
	dictPrimitiveWriter.Pad5Bytes();
	dictPrimitiveWriter.WriteDictOperator(scCharstrings);
	if(mOpenTypeInput.mCFF.mPrivateDicts[0].mPrivateDictStart != 0)
	{
		mPrivatePlaceHolderPosition = topDictStream.GetCurrentPosition();
		dictPrimitiveWriter.Pad5Bytes(); // for private it's two places - size and position
		dictPrimitiveWriter.Pad5Bytes();
		dictPrimitiveWriter.WriteDictOperator(scPrivate);
	}
	else
	{
		mPrivatePlaceHolderPosition = 0;
	}
	if(mIsCID)
	{
		mEncodingPlaceHolderPosition = 0;
		mFDArrayPlaceHolderPosition = topDictStream.GetCurrentPosition();
		dictPrimitiveWriter.Pad5Bytes();
		dictPrimitiveWriter.WriteDictOperator(scFDArray);
		mFDSelectPlaceHolderPosition = topDictStream.GetCurrentPosition();
		dictPrimitiveWriter.Pad5Bytes();
		dictPrimitiveWriter.WriteDictOperator(scFDSelect);
	}
	else
	{
		mEncodingPlaceHolderPosition = topDictStream.GetCurrentPosition();
		dictPrimitiveWriter.Pad5Bytes();
		dictPrimitiveWriter.WriteDictOperator(scEncoding);
		mFDArrayPlaceHolderPosition = 0;
		mFDSelectPlaceHolderPosition = 0;
	}
	return dictPrimitiveWriter.GetInternalState();
}

EStatusCode CFFEmbeddedFontWriter::WriteStringIndex()
{
	// if added a new string...needs to work hard, otherwise just copy the strings.
	if(mOptionalEmbeddedPostscript.size() == 0)
	{
		// copy as is from the original file. note that the global subroutines
		// starting position is equal to the strings end position. hence length is...

		OutputStreamTraits streamCopier(&mFontFileStream);
		mOpenTypeFile.GetInputStream()->SetPosition(mOpenTypeInput.mCFF.mCFFOffset + mOpenTypeInput.mCFF.mStringIndexPosition);
		return streamCopier.CopyToOutputStream(mOpenTypeFile.GetInputStream(),
												(LongBufferSizeType)(mOpenTypeInput.mCFF.mGlobalSubrsPosition -
												mOpenTypeInput.mCFF.mStringIndexPosition));
	}
	else
	{
		// need to write the bloody strings...[remember that i'm adding one more string at the end]
		mPrimitivesWriter.WriteCard16(mOpenTypeInput.mCFF.mStringsCount + 1);
		
		// calculate the total data size to determine the required offset size
		unsigned long totalSize=0;
		for(int i=0; i < mOpenTypeInput.mCFF.mStringsCount; ++i)
			totalSize += (unsigned long)strlen(mOpenTypeInput.mCFF.mStrings[i]);
		totalSize+=(unsigned long)mOptionalEmbeddedPostscript.size();
		
		Byte sizeOfOffset = GetMostCompressedOffsetSize(totalSize + 1);
		mPrimitivesWriter.WriteOffSize(sizeOfOffset);
		mPrimitivesWriter.SetOffSize(sizeOfOffset);
	
		unsigned long currentOffset = 1;

		// write the offsets
		for(int i=0; i < mOpenTypeInput.mCFF.mStringsCount; ++i)
		{
			mPrimitivesWriter.WriteOffset(currentOffset);
			currentOffset += (unsigned long)strlen(mOpenTypeInput.mCFF.mStrings[i]);
		}
		mPrimitivesWriter.WriteOffset(currentOffset);
		currentOffset+=(unsigned long)mOptionalEmbeddedPostscript.size();
		mPrimitivesWriter.WriteOffset(currentOffset);

		// write the data
		for(int i=0; i < mOpenTypeInput.mCFF.mStringsCount; ++i)
		{
			mFontFileStream.Write((const Byte*)(mOpenTypeInput.mCFF.mStrings[i]),strlen(mOpenTypeInput.mCFF.mStrings[i]));
		}
		mFontFileStream.Write((const Byte*)(mOptionalEmbeddedPostscript.c_str()),mOptionalEmbeddedPostscript.size());
		return mPrimitivesWriter.GetInternalState();
	}
}

EStatusCode CFFEmbeddedFontWriter::WriteGlobalSubrsIndex()
{
	// global subrs index is empty!. no subrs in my CFF outputs. all charstrings are flattened

	return mPrimitivesWriter.WriteCard16(0);
}


typedef std::pair<Byte,unsigned short> ByteAndUShort;
typedef std::list<ByteAndUShort> ByteAndUShortList;

EStatusCode CFFEmbeddedFontWriter::WriteEncodings(const UIntVector& inSubsetGlyphIDs)
{
	// if it's a CID. don't bother with encodings (marks as 0)
	if(mIsCID)
	{
		mEncodingPosition = 0;
		return PDFHummus::eSuccess;
	}

	// not CID, write encoding, according to encoding values from the original font
	EncodingsInfo* encodingInfo = mOpenTypeInput.mCFF.mTopDictIndex[0].mEncoding;
	if(encodingInfo->mEncodingStart <= 1)
	{
		mEncodingPosition = encodingInfo->mEncodingStart;
		return PDFHummus::eSuccess;
	}
	else
	{	
		// original font had custom encoding, let's subset it according to just the glyphs we 
		// actually have. but cause i'm lazy i'll just do the first format.

		// figure out if we got supplements
		UIntVector::const_iterator it = inSubsetGlyphIDs.begin();
		ByteAndUShortList supplements;

		for(; it != inSubsetGlyphIDs.end();++it)
		{
			// don't be confused! the supplements is by SID! not GID!
			unsigned short sid = mOpenTypeInput.mCFF.GetGlyphSID(0,*it);

			UShortToByteList::iterator itSupplements = encodingInfo->mSupplements.find(sid);
			if(itSupplements != encodingInfo->mSupplements.end())
			{
				ByteList::iterator itMoreEncoding = itSupplements->second.begin();
				for(; itMoreEncoding != itSupplements->second.end(); ++itMoreEncoding)
					supplements.push_back(ByteAndUShort(*itMoreEncoding,sid));
			}
		}

		mEncodingPosition = mFontFileStream.GetCurrentPosition();

		if(supplements.size() > 0)
			mPrimitivesWriter.WriteCard8(0x80);
		else
			mPrimitivesWriter.WriteCard8(0);

		// assuming that 0 is in the subset glyphs IDs, which does not require encoding
		// get the encodings count
		Byte encodingGlyphsCount = std::min((Byte)(inSubsetGlyphIDs.size()-1),encodingInfo->mEncodingsCount); 

		mPrimitivesWriter.WriteCard8(encodingGlyphsCount);
		for(Byte i=0; i < encodingGlyphsCount;++i)
		{
			if(inSubsetGlyphIDs[i+1] < encodingInfo->mEncodingsCount)
				mPrimitivesWriter.WriteCard8(encodingInfo->mEncoding[inSubsetGlyphIDs[i+1]-1]);
			else
				mPrimitivesWriter.WriteCard8(0);
		}

		if(supplements.size() > 0)
		{
			mPrimitivesWriter.WriteCard8(Byte(supplements.size()));
			ByteAndUShortList::iterator itCollectedSupplements = supplements.begin();

			for(; itCollectedSupplements != supplements.end(); ++itCollectedSupplements)
			{
				mPrimitivesWriter.WriteCard8(itCollectedSupplements->first);
				mPrimitivesWriter.WriteCard16(itCollectedSupplements->second);
			}
		}
	}

	return mPrimitivesWriter.GetInternalState();
}

EStatusCode CFFEmbeddedFontWriter::WriteCharsets(const UIntVector& inSubsetGlyphIDs,
													UShortVector* inCIDMapping)
{
	// since this is a subset the chances that i'll get a defult charset are 0.
	// hence i'll always do some charset. and using format 0 !!1
	UIntVector::const_iterator it = inSubsetGlyphIDs.begin();
	++it; // skip the 0

	mCharsetPosition = mFontFileStream.GetCurrentPosition();

	mPrimitivesWriter.WriteCard8(0);
	if(mIsCID && inCIDMapping)
	{
		UShortVector::const_iterator itCIDs = inCIDMapping->begin();
		++itCIDs;
		for(; it != inSubsetGlyphIDs.end(); ++it,++itCIDs)
			mPrimitivesWriter.WriteSID(*itCIDs);

	}
	else
	{
		// note that this also works for CIDs! cause in this case the SIDs are actually
		// CIDs
		for(; it != inSubsetGlyphIDs.end(); ++it)
			mPrimitivesWriter.WriteSID(mOpenTypeInput.mCFF.GetGlyphSID(0,*it));
	}
	return mPrimitivesWriter.GetInternalState();
}

EStatusCode CFFEmbeddedFontWriter::WriteCharStrings(const UIntVector& inSubsetGlyphIDs)
{
	/*
		1. build the charstrings data, looping the glyphs charstrings and writing a flattened
		   version of each charstring
		2. write the charstring index based on offsets inside the data (size should be according to the max)
		3. copy the data into the stream
	*/


	unsigned long* offsets = new unsigned long[inSubsetGlyphIDs.size() + 1];
	MyStringBuf charStringsData;
	OutputStringBufferStream charStringsDataWriteStream(&charStringsData);
	CharStringType2Flattener charStringFlattener;
	UIntVector::const_iterator itGlyphs = inSubsetGlyphIDs.begin();
	EStatusCode status = PDFHummus::eSuccess;

	do
	{
		unsigned short i=0;
		for(; itGlyphs != inSubsetGlyphIDs.end() && PDFHummus::eSuccess == status; ++itGlyphs,++i)
		{
			offsets[i] = (unsigned long)charStringsDataWriteStream.GetCurrentPosition();
			status = charStringFlattener.WriteFlattenedGlyphProgram(	0,
																		*itGlyphs,
																		&(mOpenTypeInput.mCFF),
																		&charStringsDataWriteStream);
		}
		if(status != PDFHummus::eSuccess)
			break;

		offsets[i] = (unsigned long)charStringsDataWriteStream.GetCurrentPosition();

		charStringsData.pubseekoff(0,std::ios_base::beg);

		// write index section
		mCharStringPosition = mFontFileStream.GetCurrentPosition();
		Byte sizeOfOffset = GetMostCompressedOffsetSize(offsets[i] + 1);
		mPrimitivesWriter.WriteCard16((unsigned short)inSubsetGlyphIDs.size());
		mPrimitivesWriter.WriteOffSize(sizeOfOffset);
		mPrimitivesWriter.SetOffSize(sizeOfOffset);
		for(i=0;i<=inSubsetGlyphIDs.size();++i)
			mPrimitivesWriter.WriteOffset(offsets[i] + 1);

		// Write data
		InputStringBufferStream charStringsDataReadStream(&charStringsData);
		OutputStreamTraits streamCopier(&mFontFileStream);
		status = streamCopier.CopyToOutputStream(&charStringsDataReadStream);
		if(status != PDFHummus::eSuccess)
			break;
	}while(false);

	delete[] offsets;
	return status;
}

static const unsigned short scSubrs = 19;
EStatusCode CFFEmbeddedFontWriter::WritePrivateDictionary()
{
	return WritePrivateDictionaryBody(mOpenTypeInput.mCFF.mPrivateDicts[0],mPrivateSize,mPrivatePosition);
}

EStatusCode CFFEmbeddedFontWriter::WritePrivateDictionaryBody(const PrivateDictInfo& inPrivateDictionary,
															  LongFilePositionType& outWriteSize,
															  LongFilePositionType& outWritePosition)
{
	// just copy the private dict, without the subrs reference
	if(inPrivateDictionary.mPrivateDictStart != 0)
	{
		UShortToDictOperandListMap::const_iterator it= inPrivateDictionary.mPrivateDict.begin();

		outWritePosition = mFontFileStream.GetCurrentPosition();
		for(; it != inPrivateDictionary.mPrivateDict.end(); ++it)
			if(it->first != scSubrs) // should get me a nice little pattern for this some time..a filter thing
				mPrimitivesWriter.WriteDictItems(it->first,it->second);

		outWriteSize = mFontFileStream.GetCurrentPosition() - outWritePosition;
		return mPrimitivesWriter.GetInternalState();
	}
	else
	{
		outWritePosition = 0;
		outWriteSize = 0;
		return PDFHummus::eSuccess;
	}
}

typedef std::set<FontDictInfo*> FontDictInfoSet;
typedef std::pair<LongFilePositionType,LongFilePositionType> LongFilePositionTypePair;
typedef std::map<FontDictInfo*,LongFilePositionTypePair> FontDictInfoToLongFilePositionTypePairMap;

void CFFEmbeddedFontWriter::DetermineFDArrayIndexes(const UIntVector& inSubsetGlyphIDs,FontDictInfoToByteMap& outNewFontDictsIndexes)
{
	UIntVector::const_iterator itGlyphs = inSubsetGlyphIDs.begin();
	FontDictInfoSet fontDictInfos;

	for(; itGlyphs != inSubsetGlyphIDs.end(); ++itGlyphs)
		if(mOpenTypeInput.mCFF.mTopDictIndex[0].mFDSelect[*itGlyphs])
			fontDictInfos.insert(mOpenTypeInput.mCFF.mTopDictIndex[0].mFDSelect[*itGlyphs]);

	FontDictInfoSet::iterator itFontInfos;
	Byte i=0;

	for(itFontInfos = fontDictInfos.begin(); itFontInfos != fontDictInfos.end(); ++itFontInfos,++i)
		outNewFontDictsIndexes.insert(FontDictInfoToByteMap::value_type(*itFontInfos,i));
}


EStatusCode CFFEmbeddedFontWriter::WriteFDArray(const UIntVector& inSubsetGlyphIDs,const FontDictInfoToByteMap& inNewFontDictsIndexes)
{
	// loop the glyphs IDs, for each get their respective dictionary. put them in a set.
	// now itereate them, and write each private dictionary [no need for index]. save the private dictionary position.
	// now write the FDArray. remember it's an index, so first write into a separate, maintain the offsets and only then write the actual buffer.
	// save a mapping between the original pointer and a new index.

	FontDictInfoToLongFilePositionTypePairMap privateDictionaries;
	EStatusCode status = PDFHummus::eSuccess;
	unsigned long* offsets = NULL;

	do
	{
		if(inNewFontDictsIndexes.size() == 0)
		{
			// if no valid font infos, write an empty index and finish
			mFDArrayPosition = mFontFileStream.GetCurrentPosition();
			status = mPrimitivesWriter.WriteCard16(0);
			break;
		}

		// loop the font infos, and write the private dictionaries
		LongFilePositionType privatePosition,privateSize;
		FontDictInfoToByteMap::const_iterator itFontInfos = inNewFontDictsIndexes.begin();
		for(; itFontInfos != inNewFontDictsIndexes.end() && PDFHummus::eSuccess == status; ++itFontInfos)
		{
			status = WritePrivateDictionaryBody(itFontInfos->first->mPrivateDict,privateSize,privatePosition);
			privateDictionaries.insert(
				FontDictInfoToLongFilePositionTypePairMap::value_type(itFontInfos->first,
																	LongFilePositionTypePair(privateSize,privatePosition)));
		}
		if(status != PDFHummus::eSuccess)
			break;

		// write FDArray segment
		offsets = new unsigned long[inNewFontDictsIndexes.size() + 1];
		MyStringBuf fontDictsInfoData;
		OutputStringBufferStream fontDictDataWriteStream(&fontDictsInfoData);
		CFFPrimitiveWriter fontDictPrimitiveWriter;
		Byte i=0;

		fontDictPrimitiveWriter.SetStream(&fontDictDataWriteStream);

		for(itFontInfos = inNewFontDictsIndexes.begin(); itFontInfos != inNewFontDictsIndexes.end() && PDFHummus::eSuccess == status; ++itFontInfos,++i)
		{
			offsets[i] = (unsigned long)fontDictDataWriteStream.GetCurrentPosition();

			UShortToDictOperandListMap::const_iterator itDict= itFontInfos->first->mFontDict.begin();

			for(; itDict != itFontInfos->first->mFontDict.end() && PDFHummus::eSuccess == status; ++itDict)
				if(itDict->first != scPrivate) // should get me a nice little pattern for this some time..a filter thing
					status = fontDictPrimitiveWriter.WriteDictItems(itDict->first,itDict->second);
			
			// now add the private key
			if(PDFHummus::eSuccess == status && privateDictionaries[itFontInfos->first].first != 0)
			{
				fontDictPrimitiveWriter.WriteIntegerOperand(long(privateDictionaries[itFontInfos->first].first));
				fontDictPrimitiveWriter.WriteIntegerOperand(long(privateDictionaries[itFontInfos->first].second));
				fontDictPrimitiveWriter.WriteDictOperator(scPrivate);
				status = fontDictPrimitiveWriter.GetInternalState();
			}
		}
		if(status != PDFHummus::eSuccess)
			break;

		offsets[i] = (unsigned long)fontDictDataWriteStream.GetCurrentPosition();

		fontDictsInfoData.pubseekoff(0,std::ios_base::beg);

		// write index section
		mFDArrayPosition = mFontFileStream.GetCurrentPosition();
		Byte sizeOfOffset = GetMostCompressedOffsetSize(offsets[i] + 1);
		mPrimitivesWriter.WriteCard16((unsigned short)inNewFontDictsIndexes.size());
		mPrimitivesWriter.WriteOffSize(sizeOfOffset);
		mPrimitivesWriter.SetOffSize(sizeOfOffset);
		for(i=0;i<=inNewFontDictsIndexes.size();++i)
			mPrimitivesWriter.WriteOffset(offsets[i] + 1);

		// Write data
		InputStringBufferStream fontDictDataReadStream(&fontDictsInfoData);
		OutputStreamTraits streamCopier(&mFontFileStream);
		status = streamCopier.CopyToOutputStream(&fontDictDataReadStream);
		if(status != PDFHummus::eSuccess)
			break;

	}while(false);

	delete[] offsets;
	if(status != PDFHummus::eSuccess)
		return status;
	else
		return mPrimitivesWriter.GetInternalState();
}

EStatusCode CFFEmbeddedFontWriter::WriteFDSelect(const UIntVector& inSubsetGlyphIDs,const FontDictInfoToByteMap& inNewFontDictsIndexes)
{
	// always write format 3. cause at most cases the FD dicts count will be so low that it'd
	// take a bloody mircale for no repeats to occur.
	UIntVector::const_iterator itGlyphs = inSubsetGlyphIDs.begin();


	mFDSelectPosition = mFontFileStream.GetCurrentPosition();
	mPrimitivesWriter.WriteCard8(3);

	LongFilePositionType rangesCountPosition = mFontFileStream.GetCurrentPosition();
	mPrimitivesWriter.WriteCard16(1); // temporary. will get back to this later

	unsigned short rangesCount = 1;
	Byte currentFD,newFD;
	unsigned short glyphIndex = 1;
	FontDictInfoToByteMap::const_iterator itNewIndex = 
		inNewFontDictsIndexes.find(mOpenTypeInput.mCFF.mTopDictIndex[0].mFDSelect[*itGlyphs]);
	
	// k. seems like i probably just imagine exceptions here. i guess there must
	// be a proper FDSelect with FDs for all...so i'm defaulting to some 0
	currentFD = (itNewIndex == inNewFontDictsIndexes.end() ? 0:itNewIndex->second);
	mPrimitivesWriter.WriteCard16(0);
	mPrimitivesWriter.WriteCard8(currentFD);
	++itGlyphs;

	for(; itGlyphs != inSubsetGlyphIDs.end(); ++itGlyphs,++glyphIndex)
	{
		itNewIndex = 
				inNewFontDictsIndexes.find(mOpenTypeInput.mCFF.mTopDictIndex[0].mFDSelect[*itGlyphs]);
		newFD = (itNewIndex == inNewFontDictsIndexes.end() ? 0:itNewIndex->second);
		if(newFD != currentFD)
		{
			currentFD = newFD;
			mPrimitivesWriter.WriteCard16(glyphIndex);
			mPrimitivesWriter.WriteCard8(currentFD);
			++rangesCount;
		}
	}
	mPrimitivesWriter.WriteCard16((unsigned short)inSubsetGlyphIDs.size());
	// go back to ranges count if not equal to what's already written
	if(rangesCount != 1)
	{
		LongFilePositionType currentPosition = mFontFileStream.GetCurrentPosition();
		mFontFileStream.SetPosition(rangesCountPosition);
		mPrimitivesWriter.WriteCard16(rangesCount);
		mFontFileStream.SetPosition(currentPosition);
	}
	return mPrimitivesWriter.GetInternalState();
}

EStatusCode CFFEmbeddedFontWriter::UpdateIndexesAtTopDict()
{
	mFontFileStream.SetPosition(mCharsetPlaceHolderPosition);
	mPrimitivesWriter.Write5ByteDictInteger((long)mCharsetPosition);

	mFontFileStream.SetPosition(mCharstringsPlaceHolderPosition);
	mPrimitivesWriter.Write5ByteDictInteger((long)mCharStringPosition);

	if(mOpenTypeInput.mCFF.mPrivateDicts[0].mPrivateDictStart != 0)
	{
		mFontFileStream.SetPosition(mPrivatePlaceHolderPosition);
		mPrimitivesWriter.Write5ByteDictInteger((long)mPrivateSize);
		mPrimitivesWriter.Write5ByteDictInteger((long)mPrivatePosition);
		
	}
	
	if(mIsCID)
	{
		mFontFileStream.SetPosition(mFDArrayPlaceHolderPosition);
		mPrimitivesWriter.Write5ByteDictInteger((long)mFDArrayPosition);
		mFontFileStream.SetPosition(mFDSelectPlaceHolderPosition);
		mPrimitivesWriter.Write5ByteDictInteger((long)mFDSelectPosition);
	}
	else
	{
		mFontFileStream.SetPosition(mEncodingPlaceHolderPosition);
		mPrimitivesWriter.Write5ByteDictInteger((long)mEncodingPosition);
	}

	return mPrimitivesWriter.GetInternalState();
}

unsigned short CFFEmbeddedFontWriter::GetSubsetFontGlyphsCount() {
	return mSubsetFontGlyphsCount;
}
