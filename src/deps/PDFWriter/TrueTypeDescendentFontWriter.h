/*
   Source File : TrueTypeDescendentFontWriter.h


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

#include "IDescendentFontWriter.h"

class TrueTypeDescendentFontWriter: public IDescendentFontWriter
{
public:
	TrueTypeDescendentFontWriter(void);
	~TrueTypeDescendentFontWriter(void);

	// IDescendentFontWriter implementation [used also as helper for the DescendentFontWriter]
	virtual PDFHummus::EStatusCode WriteFont(	ObjectIDType inDecendentObjectID, 
									const std::string& inFontName,
									FreeTypeFaceWrapper& inFontInfo,
									const UIntAndGlyphEncodingInfoVector& inEncodedGlyphs,
									ObjectsContext* inObjectsContext,
									bool inEmbedFont);
	virtual void WriteSubTypeValue(DictionaryContext* inDescendentFontContext);
	virtual void WriteAdditionalKeys(DictionaryContext* inDescendentFontContext);
	virtual void WriteFontFileReference(DictionaryContext* inDescriptorContext,
										ObjectsContext* inObjectsContext);

private:

	ObjectIDType mEmbeddedFontFileObjectID;
};
