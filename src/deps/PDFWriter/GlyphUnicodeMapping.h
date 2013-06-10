/*
   Source File : GlyphUnicodeMapping.h


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

#include <vector>
#include <list>



typedef std::vector<unsigned long> ULongVector;

struct GlyphUnicodeMapping
{
	GlyphUnicodeMapping(){}

	GlyphUnicodeMapping(unsigned short inGlyphCode,unsigned long inUnicodeValue)
	{
		mGlyphCode = inGlyphCode;
		mUnicodeValues.push_back(inUnicodeValue);
	}
	
	GlyphUnicodeMapping(unsigned short inGlyphCode,ULongVector inUnicodeValues)
	{
		mGlyphCode = inGlyphCode;
		mUnicodeValues = inUnicodeValues;
	}

	// and ordered list of the unicode values that this glyph represents
	ULongVector mUnicodeValues;

	// glyph code
	unsigned short mGlyphCode;	
};

typedef std::list<GlyphUnicodeMapping> GlyphUnicodeMappingList;