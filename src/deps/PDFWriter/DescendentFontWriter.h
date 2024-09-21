/*
   Source File : DescendentFontWriter.h


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
#include "WrittenFontRepresentation.h"
#include "ObjectsBasicTypes.h"
#include "IFontDescriptorHelper.h"
#include "IDescendentFontWriter.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <list>
#include <string>

class FreeTypeFaceWrapper;
class ObjectsContext;
class DictionaryContext;




typedef std::list<FT_Pos> FTPosList;

class DescendentFontWriter : public IFontDescriptorHelper
{
public:
	DescendentFontWriter(void);
	~DescendentFontWriter(void);

	PDFHummus::EStatusCode WriteFont(	ObjectIDType inDecendentObjectID, 
									const std::string& inFontName,
									FreeTypeFaceWrapper& inFontInfo,
									const UIntAndGlyphEncodingInfoVector& inEncodedGlyphs,
									ObjectsContext* inObjectsContext,
									IDescendentFontWriter* inDescendentFontWriterHelper,
									// the next one is for writing CID set. there's an assumption
									// that all CIDs up to the max glyph are included in the font program,
									// so that CIDset will include all up to (and including) CID glyph
									unsigned int inMaxCIDGlyph);

	// IFontDescriptorHelper implementation [would probably evolve at some point to IDescriptorWriterHelper...
	virtual void WriteCharSet(	DictionaryContext* inDescriptorContext,
								ObjectsContext* inObjectsContext,
								FreeTypeFaceWrapper* inFontInfo,
								const UIntAndGlyphEncodingInfoVector& inEncodedGlyphs);
	virtual void WriteFontFileReference(DictionaryContext* inDescriptorContext,
										ObjectsContext* inObjectsContext);
private:

	FreeTypeFaceWrapper* mFontInfo;
	ObjectsContext* mObjectsContext;
	ObjectIDType mCIDSetObjectID;
	IDescendentFontWriter* mWriterHelper;


	void WriteWidths(const UIntAndGlyphEncodingInfoVector& inEncodedGlyphs,
						DictionaryContext* inFontContext);
	void WriteCIDSystemInfo(ObjectIDType inCIDSystemInfoObjectID);
	void WriteWidthsItem(bool inAllWidthsSame,const FTPosList& inWidths,unsigned short inFirstCID, unsigned short inLastCID);
	void WriteCIDSet(unsigned int cidSetMaxGlyph);
};
