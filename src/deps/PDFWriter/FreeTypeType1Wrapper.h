/*
   Source File : FreeTypeType1Wrapper.h


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
#include "PFMFileReader.h"
#include "Type1Input.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TYPE1_TABLES_H 

class FreeTypeType1Wrapper : public IFreeTypeFaceExtender
{
public:
	FreeTypeType1Wrapper(FT_Face inFace,const std::string& inFontFilePath,const std::string& inPFMFilePath);  // NEVER EVER EVER PASS NULL!!!!1 [ok to pass empty string for PFM file]
	virtual ~FreeTypeType1Wrapper(void);

	virtual	double GetItalicAngle();
	virtual BoolAndFTShort GetCapHeight();
	virtual BoolAndFTShort GetxHeight();
	virtual FT_UShort GetStemV();
	virtual EFontStretch GetFontStretch();
	virtual FT_UShort GetFontWeight();
	virtual bool HasSerifs();
	virtual bool IsScript();
	virtual bool IsForceBold();
    virtual bool HasPrivateEncoding();
    virtual unsigned int GetGlyphForUnicodeChar(unsigned long inChar);
    virtual std::string GetPrivateGlyphName(unsigned int inGlyphIndex);
    virtual unsigned int GetFreeTypeGlyphIndexFromEncodingGlyphIndex(unsigned int inGlyphIndex);
	virtual std::string GetPostscriptNameNonStandard();

private:
    FT_Face mFace;
	bool mPFMFileInfoRelevant;
	PFMFileReader mPFMReader;
	PS_FontInfoRec mPSFontInfo;
	PS_PrivateRec mPrivateInfo;
	bool mPSavailable;
	bool mPSPrivateAvailable;
    bool mIsCustomEncoding;
    Type1Input mType1File;
};
