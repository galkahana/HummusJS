/*
   Source File : TrueTypeEmbeddedFontWriter.h


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
#include "OutputStringBufferStream.h"
#include "InputFile.h"
#include "TrueTypePrimitiveWriter.h"
#include "InputStringBufferStream.h"
#include "OpenTypePrimitiveReader.h"
#include "MyStringBuf.h"

#include <vector>
#include <set>

class FreeTypeFaceWrapper;
class ObjectsContext;



typedef std::vector<unsigned int> UIntVector;
typedef std::set<unsigned int> UIntSet;

class TrueTypeEmbeddedFontWriter
{
public:
	TrueTypeEmbeddedFontWriter(void);
	~TrueTypeEmbeddedFontWriter(void);

	PDFHummus::EStatusCode WriteEmbeddedFont(	FreeTypeFaceWrapper& inFontInfo,
									const UIntVector& inSubsetGlyphIDs,
									ObjectsContext* inObjectsContext,
									ObjectIDType& outEmbeddedFontObjectID);

	// after having written the embedded font, and the actual glyph count is determined - possibly
	// including additional glyphs to inSubsetGlyphIDs list, where dependent glyphs are required - 
	// you can use this method to get the count of the actually emitted subset size
	unsigned short GetSubsetFontGlyphsCount();

private:
	OpenTypeFileInput mTrueTypeInput;
	InputFile mTrueTypeFile;
	OutputStringBufferStream mFontFileStream;
	TrueTypePrimitiveWriter mPrimitivesWriter;
	InputStringBufferStream mFontFileReaderStream; // now this might be confusing - i'm using a reader
											 // to my output buffer, so i can calculate checksums 'n such
	OpenTypePrimitiveReader mPrimitivesReader; // same here...
	unsigned short mSubsetFontGlyphsCount;

	LongFilePositionType mCVTEntryWritingOffset;
	LongFilePositionType mFPGMEntryWritingOffset;
	LongFilePositionType mGLYFEntryWritingOffset;
	LongFilePositionType mHEADEntryWritingOffset;
	LongFilePositionType mHHEAEntryWritingOffset;
	LongFilePositionType mHMTXEntryWritingOffset;
	LongFilePositionType mLOCAEntryWritingOffset;
	LongFilePositionType mMAXPEntryWritingOffset;
	LongFilePositionType mPREPEntryWritingOffset;
	LongFilePositionType mNAMEEntryWritingOffset;
	LongFilePositionType mOS2EntryWritingOffset;
	LongFilePositionType mCMAPEntryWritingOffset;

	LongFilePositionType mHeadCheckSumOffset;
	

	PDFHummus::EStatusCode CreateTrueTypeSubset(	FreeTypeFaceWrapper& inFontInfo,
										const UIntVector& inSubsetGlyphIDs,
										bool& outNotEmbedded,
										MyStringBuf& outFontProgram);

	void AddDependentGlyphs(UIntVector& ioSubsetGlyphIDs);
	bool AddComponentGlyphs(unsigned int inGlyphID,UIntSet& ioComponents);

	PDFHummus::EStatusCode WriteTrueTypeHeader();
	unsigned short GetSmallerPower2(unsigned short inNumber);
	unsigned long GetTag(const char* inTagName);
	void WriteEmptyTableEntry(const char* inTag,LongFilePositionType& outEntryPosition);
	PDFHummus::EStatusCode WriteHead();
	void WriteTableEntryData(
							LongFilePositionType inTableEntryOffset,
							LongFilePositionType inTableOffset,
							unsigned long inTableLength);
	PDFHummus::EStatusCode WriteHHea();
	PDFHummus::EStatusCode WriteHMtx();
	PDFHummus::EStatusCode WriteMaxp();
	PDFHummus::EStatusCode WriteCVT();
	PDFHummus::EStatusCode WriteFPGM();
	PDFHummus::EStatusCode WritePREP();
	PDFHummus::EStatusCode WriteNAME();
	PDFHummus::EStatusCode WriteOS2();
	PDFHummus::EStatusCode WriteGlyf(const UIntVector& inSubsetGlyphIDs,unsigned long* inLocaTable);
	PDFHummus::EStatusCode WriteLoca(unsigned long* inLocaTable);
	PDFHummus::EStatusCode WriteCMAP();
	unsigned long GetCheckSum(LongFilePositionType inOffset,unsigned long inLength);
	PDFHummus::EStatusCode CreateHeadTableCheckSumAdjustment();
	PDFHummus::EStatusCode CreateTableCopy(const char* inTableName,LongFilePositionType inTableEntryLocation);
};
