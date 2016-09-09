/*
   Source File : CFFEmbeddedFontWriter.h


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
#pragma once

#include "EStatusCode.h"
#include "ObjectsBasicTypes.h"
#include "OpenTypeFileInput.h"
#include "MyStringBuf.h"
#include "InputFile.h"
#include "CFFPrimitiveWriter.h"
#include "OutputStringBufferStream.h"
#include "IOBasicTypes.h"

#include <vector>
#include <string>
#include <set>
#include <map>


using namespace IOBasicTypes;

typedef std::vector<unsigned int> UIntVector;
typedef std::set<unsigned int> UIntSet;
typedef std::vector<unsigned short> UShortVector;
typedef std::map<FontDictInfo*,Byte> FontDictInfoToByteMap;


class FreeTypeFaceWrapper;
class ObjectsContext;

class CFFEmbeddedFontWriter
{
public:
	CFFEmbeddedFontWriter(void);
	~CFFEmbeddedFontWriter(void);

	PDFHummus::EStatusCode WriteEmbeddedFont(	FreeTypeFaceWrapper& inFontInfo,
									const UIntVector& inSubsetGlyphIDs,
									const std::string& inFontFile3SubType,
									const std::string& inSubsetFontName,
									ObjectsContext* inObjectsContext,
									ObjectIDType& outEmbeddedFontObjectID);

	// the optional inCIDMapping parameter provides a vector ordered in the same
	// way as the glyph IDs. for each position in the CID mapping vector there's the matching CID
	// for the GID in the same position in the subset glyph IDs.
	// use it when the CFF origin is from a subset font, and the GID->CID mapping is not simply
	// identity
	PDFHummus::EStatusCode WriteEmbeddedFont(	FreeTypeFaceWrapper& inFontInfo,
									const UIntVector& inSubsetGlyphIDs,
									const std::string& inFontFile3SubType,
									const std::string& inSubsetFontName,
									ObjectsContext* inObjectsContext,
									UShortVector* inCIDMapping,
									ObjectIDType& outEmbeddedFontObjectID);


private:
	OpenTypeFileInput mOpenTypeInput;
	InputFile mOpenTypeFile;
	CFFPrimitiveWriter mPrimitivesWriter;
	OutputStringBufferStream mFontFileStream;
	bool mIsCID;
	std::string mOptionalEmbeddedPostscript;

	// placeholders positions
	LongFilePositionType mCharsetPlaceHolderPosition;
	LongFilePositionType mEncodingPlaceHolderPosition;
	LongFilePositionType mCharstringsPlaceHolderPosition;
	LongFilePositionType mPrivatePlaceHolderPosition;
	LongFilePositionType mFDArrayPlaceHolderPosition;
	LongFilePositionType mFDSelectPlaceHolderPosition;
	
	LongFilePositionType mEncodingPosition;
	LongFilePositionType mCharsetPosition;
	LongFilePositionType mCharStringPosition;
	LongFilePositionType mPrivatePosition;
	LongFilePositionType mPrivateSize;
	LongFilePositionType mFDArrayPosition;
	LongFilePositionType mFDSelectPosition;

	PDFHummus::EStatusCode CreateCFFSubset(	
					FreeTypeFaceWrapper& inFontInfo,
					const UIntVector& inSubsetGlyphIDs,
					UShortVector* inCIDMapping,
					const std::string& inSubsetFontName,
					bool& outNotEmbedded,
					MyStringBuf& outFontProgram);
	PDFHummus::EStatusCode AddDependentGlyphs(UIntVector& ioSubsetGlyphIDs);
	PDFHummus::EStatusCode AddComponentGlyphs(unsigned int inGlyphID,UIntSet& ioComponents,bool &outFoundComponents);
	PDFHummus::EStatusCode WriteCFFHeader();
	PDFHummus::EStatusCode WriteName(const std::string& inSubsetFontName);
	PDFHummus::EStatusCode WriteTopIndex();
	Byte GetMostCompressedOffsetSize(unsigned long inOffset);
	PDFHummus::EStatusCode WriteTopDictSegment(MyStringBuf& ioTopDictSegment);
	PDFHummus::EStatusCode WriteStringIndex();
	PDFHummus::EStatusCode WriteGlobalSubrsIndex();
	PDFHummus::EStatusCode WriteEncodings(const UIntVector& inSubsetGlyphIDs);
	PDFHummus::EStatusCode WriteCharsets(const UIntVector& inSubsetGlyphIDs,UShortVector* inCIDMapping);
	PDFHummus::EStatusCode WriteCharStrings(const UIntVector& inSubsetGlyphIDs);
	PDFHummus::EStatusCode WritePrivateDictionary();

	PDFHummus::EStatusCode WriteFDArray(const UIntVector& inSubsetGlyphIDs,const FontDictInfoToByteMap& inNewFontDictsIndexes);
	PDFHummus::EStatusCode WriteFDSelect(const UIntVector& inSubsetGlyphIDs,const FontDictInfoToByteMap& inNewFontDictsIndexes);
	PDFHummus::EStatusCode WritePrivateDictionaryBody(	const PrivateDictInfo& inPrivateDictionary,
											LongFilePositionType& outWriteSize,
											LongFilePositionType& outWritePosition);
	PDFHummus::EStatusCode UpdateIndexesAtTopDict();

	void DetermineFDArrayIndexes(const UIntVector& inSubsetGlyphIDs,FontDictInfoToByteMap& outNewFontDictsIndexes);
	
};
