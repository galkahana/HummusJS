/*
   Source File : IDescendentFontWriter.h


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
#include "WrittenFontRepresentation.h"

#include <string>
#include <vector>
#include <utility>

class FreeTypeFaceWrapper;
class ObjectsContext;
class DictionaryContext;



typedef std::pair<unsigned int, GlyphEncodingInfo> UIntAndGlyphEncodingInfo;
typedef std::vector<UIntAndGlyphEncodingInfo> UIntAndGlyphEncodingInfoVector;

class IDescendentFontWriter
{
public:
	virtual ~IDescendentFontWriter(){};

	virtual PDFHummus::EStatusCode WriteFont(	ObjectIDType inDecendentObjectID, 
									const std::string& inFontName,
									FreeTypeFaceWrapper& inFontInfo,
									const UIntAndGlyphEncodingInfoVector& inEncodedGlyphs,
									ObjectsContext* inObjectsContext,
									bool inEmbedFont) = 0;

	virtual void WriteSubTypeValue(DictionaryContext* inDescendentFontContext) = 0;

	// free writing of additional keys
	virtual void WriteAdditionalKeys(DictionaryContext* inDescendentFontContext) = 0;

	// write reference to font file in the descriptor of the descendend font (expecting that later there'll also be font file writing)
	virtual void WriteFontFileReference(DictionaryContext* inDescriptorContext,
										ObjectsContext* inObjectsContext) = 0;

};