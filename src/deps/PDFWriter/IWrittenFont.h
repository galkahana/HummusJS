/*
   Source File : IWrittenFont.h


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
#include "GlyphUnicodeMapping.h"

#include <list>
#include <vector>



typedef std::list<unsigned short> UShortList;
typedef std::list<UShortList> UShortListList;
typedef std::list<GlyphUnicodeMappingList> GlyphUnicodeMappingListList;

class FreeTypeFaceWrapper;
class ObjectsContext;
class PDFParser;

class IWrittenFont
{
public:
	virtual ~IWrittenFont(){}	

	/*
		recieve a list of glyphs (and reference text, which can be ignored),
		and return an encoded list of charactrs to represent them. Also return if each charachter is single byte or multi (dbl) byte.
		this chap simply states whether this is non CID or CID. last but not least - return the object ID for the font, so that 
		the using content can refer to it
	*/
	virtual void AppendGlyphs(const GlyphUnicodeMappingList& inGlyphsList,
							  UShortList& outEncodedCharacters,
							  bool& outEncodingIsMultiByte,
							  ObjectIDType &outFontObjectID) = 0;

	virtual void AppendGlyphs(const GlyphUnicodeMappingListList& inGlyphsList,
							  UShortListList& outEncodedCharacters,
							  bool& outEncodingIsMultiByte,
							  ObjectIDType &outFontObjectID) = 0;

	/*
		Write a font definition using the glyphs appended.
	*/
	virtual PDFHummus::EStatusCode WriteFontDefinition(bool inEmbedFont) = 0;

	// state read and write
	virtual PDFHummus::EStatusCode WriteState(ObjectsContext* inStateWriter,ObjectIDType inObjectID) = 0;
	virtual PDFHummus::EStatusCode ReadState(PDFParser* inStateReader,ObjectIDType inObjectID) = 0;

};
