/*
   Source File : WrittenFontCFF.h


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
#include "AbstractWrittenFont.h"

#include <utility>
#include <list>



typedef std::pair<unsigned char,unsigned char> UCharAndUChar;
typedef std::list<UCharAndUChar> UCharAndUCharList;


class WrittenFontCFF : public AbstractWrittenFont
{
public:
	WrittenFontCFF(ObjectsContext* inObjectsContext, FreeTypeFaceWrapper* inFontInfo, bool inIsCID, bool inFontWillBeEmbedded);
	virtual ~WrittenFontCFF(void);


	virtual PDFHummus::EStatusCode WriteFontDefinition(bool inEmbedFont);

	virtual PDFHummus::EStatusCode WriteState(ObjectsContext* inStateWriter,ObjectIDType inObjectId);
	virtual PDFHummus::EStatusCode ReadState(PDFParser* inStateReader,ObjectIDType inObjectID);

private:
	virtual bool AddToANSIRepresentation(const GlyphUnicodeMappingList& inGlyphsList,
										 UShortList& outEncodedCharacters);

	virtual bool AddToANSIRepresentation(const GlyphUnicodeMappingListList& inGlyphsList,
										UShortListList& outEncodedCharacters);

	virtual unsigned short EncodeCIDGlyph(unsigned int inGlyphId);

	bool HasEnoughSpaceForGlyphs(const GlyphUnicodeMappingList& inGlyphsList);
	unsigned short EncodeGlyph(unsigned int inGlyph,const ULongVector& inCharacters);
	void RemoveFromFreeList(unsigned char inAllocatedPosition);
	unsigned char AllocateFromFreeList(unsigned int inGlyph);
	bool HasEnoughSpaceForGlyphs(const GlyphUnicodeMappingListList& inGlyphsList);

	unsigned char mAvailablePositionsCount;
	UCharAndUCharList mFreeList;
	bool mAssignedPositionsAvailable[256];
	unsigned int mAssignedPositions[256];
	bool mIsCID;
	bool mFontWillBeEmbedded;
};
