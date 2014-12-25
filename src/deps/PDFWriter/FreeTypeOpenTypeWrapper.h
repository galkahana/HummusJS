/*
   Source File : FreeTypeOpenTypeWrapper.h


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
#include "IFreeTypeFaceExtender.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H 

typedef std::pair<bool,FT_UShort> BoolAndFTUShort;


class FreeTypeOpenTypeWrapper : public IFreeTypeFaceExtender
{
public:
	FreeTypeOpenTypeWrapper(FT_Face inFace); // NEVER EVER EVER PASS NULL!!!!1
	virtual ~FreeTypeOpenTypeWrapper(void);

	virtual	double GetItalicAngle();
	virtual BoolAndFTShort GetCapHeight();
	virtual BoolAndFTShort GetxHeight();
	virtual FT_UShort GetStemV();
	virtual EFontStretch GetFontStretch();
	virtual FT_UShort GetFontWeight();
	virtual bool HasSerifs();
	virtual bool IsScript();
	virtual	bool IsForceBold();
    virtual bool HasPrivateEncoding();
    virtual unsigned int GetGlyphForUnicodeChar(unsigned long inChar);
    virtual std::string GetPrivateGlyphName(unsigned int inGlyphIndex);
    virtual unsigned int GetFreeTypeGlyphIndexFromEncodingGlyphIndex(unsigned int inGlyphIndex);
	virtual std::string GetPostscriptNameNonStandard();

private:
	TT_Postscript* mPostScriptTable;
	TT_OS2* mOS2Table;
	TT_PCLT* mPCLTTable;
	FT_Face mFace;

	double SixTeenFloatToDouble(FT_Fixed inFixed16_16);
	BoolAndFTUShort StemVFromLowerLWidth();
	EFontStretch GetFontStretchForPCLTValue(FT_Char inWidthValue);
	FT_UShort GetFontWeightFromPCLTValue(FT_Char inWeightValue);
};
