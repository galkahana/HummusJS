/*
   Source File : PDFUsedFont.h


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

#include "FreeTypeFaceWrapper.h"
#include "ObjectsBasicTypes.h"
#include "EStatusCode.h"
#include "GlyphUnicodeMapping.h"
#include <string>
#include <list>

#include <ft2build.h>
#include FT_FREETYPE_H



typedef std::list<unsigned short> UShortList;
typedef std::list<UShortList> UShortListList;
typedef std::list<std::string> StringList;
typedef std::list<GlyphUnicodeMappingList> GlyphUnicodeMappingListList;

class IWrittenFont;
class ObjectsContext;
class PDFParser;

class PDFUsedFont
{
public:
	PDFUsedFont(FT_Face inInputFace,
				const std::string& inFontFilePath,
				const std::string& inAdditionalMetricsFontFilePath,
                long inFontIndex,
				ObjectsContext* inObjectsContext);
	virtual ~PDFUsedFont(void);

	bool IsValid();

	// a String in the following two implementations is represented here by a list of glyphs, with each mapped to the matching
	// unicode values. to move from a wide string to such a structure, use other class methods

	/*
		This function does the work of encoding a text string to a matching font instance name, and an encoded array
		of characters. the encoded array is a list of short values (double byte for CID, one byte for regular), and an extra
		boolean value indicates whether they are CID or regular.
	*/
	PDFHummus::EStatusCode EncodeStringForShowing(const GlyphUnicodeMappingList& inText,
										ObjectIDType &outFontObjectToUse,
										UShortList& outCharactersToUse,
										bool& outTreatCharactersAsCID);

	// encode all strings. make sure that they will use the same font.
	PDFHummus::EStatusCode EncodeStringsForShowing(const GlyphUnicodeMappingListList& inText,
										ObjectIDType &outFontObjectToUse,
										UShortListList& outCharactersToUse,
										bool& outTreatCharactersAsCID);

	PDFHummus::EStatusCode WriteFontDefinition();

	// use this method to translate text to glyphs and unicode mapping, to be later used for EncodeStringForShowing
	PDFHummus::EStatusCode TranslateStringToGlyphs(const std::string& inText,GlyphUnicodeMappingList& outGlyphsUnicodeMapping);

	PDFHummus::EStatusCode WriteState(ObjectsContext* inStateWriter,ObjectIDType inObjectID);
	PDFHummus::EStatusCode ReadState(PDFParser* inStateReader,ObjectIDType inObjectID);
    
    FreeTypeFaceWrapper* GetFreeTypeFont();

private:
	FreeTypeFaceWrapper mFaceWrapper;
    IWrittenFont* mWrittenFont;
	ObjectsContext* mObjectsContext;


};
