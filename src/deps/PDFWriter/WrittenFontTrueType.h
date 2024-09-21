/*
   Source File : WrittenFontTrueType.h


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

class WrittenFontTrueType : public AbstractWrittenFont
{
public:
	WrittenFontTrueType(ObjectsContext* inObjectsContext, FreeTypeFaceWrapper* inFontInfo);
	~WrittenFontTrueType(void);

	virtual PDFHummus::EStatusCode WriteFontDefinition(bool inEmbedFont);

	virtual PDFHummus::EStatusCode WriteState(ObjectsContext* inStateWriter,ObjectIDType inObjectId);
	virtual PDFHummus::EStatusCode ReadState(PDFParser* inStateReader,ObjectIDType inObjectID);


private:
    bool fontSupportsWinAnsiEncoding;

	virtual bool AddToANSIRepresentation(	const GlyphUnicodeMappingList& inGlyphsList,
											UShortList& outEncodedCharacters);

	virtual bool AddToANSIRepresentation(
											const GlyphUnicodeMappingListList& inGlyphsList,
											UShortListList& outEncodedCharacters);


	virtual unsigned short EncodeCIDGlyph(unsigned int inGlyphId);

    bool AddANSICandidates(const GlyphUnicodeMappingList& inGlyphsList, UShortList& ioCandidates);



};
