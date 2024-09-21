/*
   Source File : FreeTypeFaceWrapper.cpp


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
#include "FreeTypeFaceWrapper.h"
#include "IFreeTypeFaceExtender.h"
#include "FreeTypeType1Wrapper.h"
#include "FreeTypeOpenTypeWrapper.h"
#include "Trace.h"
#include "BetweenIncluding.h"
#include "WrittenFontCFF.h"
#include "WrittenFontTrueType.h"

#include <math.h>

#include FT_XFREE86_H 
#include FT_CID_H 
#include FT_OUTLINE_H


using namespace PDFHummus;

FreeTypeFaceWrapper::FreeTypeFaceWrapper(FT_Face inFace,const std::string& inFontFilePath,long inFontIndex,bool inDoOwn)
{
	mFace = inFace;
	mFontFilePath = inFontFilePath;
	mFontIndex = inFontIndex;
	mDoesOwn = inDoOwn;
	mGlyphIsLoaded = false;
	ResetPaletteSelectionState();
	SetupFormatSpecificExtender(inFontFilePath, "");
	SelectDefaultEncoding();
}

FreeTypeFaceWrapper::FreeTypeFaceWrapper(FT_Face inFace,const std::string& inFontFilePath,const std::string& inPFMFilePath,long inFontIndex, bool inDoOwn)
{
    mFace = inFace;
	mFontFilePath = inFontFilePath;
    mFontIndex = inFontIndex;
	mDoesOwn = inDoOwn;
	mGlyphIsLoaded = false;
	ResetPaletteSelectionState();
	std::string fileExtension = GetExtension(inPFMFilePath);
	if (fileExtension == "PFM" || fileExtension == "pfm") // just don't bother if it's not PFM
		SetupFormatSpecificExtender(inFontFilePath, inPFMFilePath);
	else
		SetupFormatSpecificExtender(inFontFilePath, "");
	SelectDefaultEncoding();
}

void FreeTypeFaceWrapper::ResetPaletteSelectionState() {
	mPaletteSet = false;
	mPalette = NULL;
	mPaletteStatus = FT_Err_Ok;
}

void FreeTypeFaceWrapper::SelectDefaultEncoding() {
	mUsePUACodes = false;
	// try unicode, if doesn't work, try symbol, if that doesnt work try apple roman if exists. and give up (should take care of plain unicodes and symbols and plain latins)
	if (FT_Select_Charmap(mFace, FT_ENCODING_UNICODE) != 0) {
		if(FT_Select_Charmap(mFace, FT_ENCODING_MS_SYMBOL) != 0) {
			if (FT_Select_Charmap(mFace, FT_ENCODING_APPLE_ROMAN) != 0) {
				TRACE_LOG("inFreeTypeFaceWrapper::SelectDefaultEncoding, warning - failed to set either unicode or symbol encoding");
			}
		}
		else {
			mUsePUACodes = true; // for symbol map input chars to pua codes
		}
	}
}

std::string FreeTypeFaceWrapper::NotDefGlyphName()
{
    // for special case of fonts that have glyph names, but don't define .notdef, use one of the existing chars (found a custom type 1 with that)
    
    if (mNotDefGlyphName.length() == 0) {
		if(FT_HAS_GLYPH_NAMES(mFace))
		{
			char* aString = (char*)".notdef";
			if(FT_Get_Name_Index(mFace,aString) == 0) {
				FT_UInt   gindex;
				FT_Get_First_Char( mFace, &gindex ); 
				if (gindex != 0)
				  mNotDefGlyphName = GetGlyphName(gindex, true);
			}
		}
		if (mNotDefGlyphName == "")  mNotDefGlyphName = ".notdef";
	}
	return mNotDefGlyphName;
}

std::string FreeTypeFaceWrapper::GetExtension(const std::string& inFilePath)
{
	std::string::size_type dotPosition = inFilePath.rfind(".");

	if(inFilePath.npos == dotPosition || (inFilePath.size() - 1) == dotPosition)
		return "";
	else
		return inFilePath.substr(dotPosition + 1);
}

FreeTypeFaceWrapper::~FreeTypeFaceWrapper(void)
{
	if(mDoesOwn)
		DoneFace();
	delete mFormatParticularWrapper;
}

static const char* scType1 = "Type 1";
static const char* scTrueType = "TrueType";
static const char* scCFF = "CFF";

void FreeTypeFaceWrapper::SetupFormatSpecificExtender(const std::string& inFontFilePath,const std::string& inPFMFilePath /*pass empty if non existant or irrelevant*/)
{
	if(mFace)
	{
		const char* fontFormat = FT_Get_X11_Font_Format(mFace);

		if(strcmp(fontFormat,scType1) == 0)
			mFormatParticularWrapper = new FreeTypeType1Wrapper(mFace,inFontFilePath,inPFMFilePath);
		else if(strcmp(fontFormat,scCFF) == 0 || strcmp(fontFormat,scTrueType) == 0)
			mFormatParticularWrapper = new FreeTypeOpenTypeWrapper(mFace);
		else
		{
			mFormatParticularWrapper = NULL;
			TRACE_LOG1("Failure in FreeTypeFaceWrapper::SetupFormatSpecificExtender, could not find format specific implementation for %s",fontFormat);
		}
	}
	else
		mFormatParticularWrapper = NULL;
		
}

static const char* scEmpty="";
const char* FreeTypeFaceWrapper::GetTypeString()
{
	if(mFace)
	{
		const char* fontFormat = FT_Get_X11_Font_Format(mFace);
		return fontFormat;
	}
	else
	{
		return scEmpty;
	}
}


FT_Face FreeTypeFaceWrapper::operator->()
{
	return mFace;
}

FreeTypeFaceWrapper::operator FT_Face()
{
	return mFace;
}

bool FreeTypeFaceWrapper::IsValid()
{
	return mFace && mFormatParticularWrapper;
}

FT_Error FreeTypeFaceWrapper::DoneFace()
{
	if(mFace)
	{
		FT_Error status = FT_Done_Face(mFace);
		mFace = NULL;
		delete mFormatParticularWrapper;
		mFormatParticularWrapper = NULL;
		return status;
	}
	else
		return 0;
}

double FreeTypeFaceWrapper::GetItalicAngle()
{
	return mFormatParticularWrapper ? mFormatParticularWrapper->GetItalicAngle():0;
}

BoolAndFTShort FreeTypeFaceWrapper::GetCapHeightInternal()
{
	if(mFormatParticularWrapper)
	{
		BoolAndFTShort fontDependentResult = mFormatParticularWrapper->GetCapHeight();
		if(fontDependentResult.first)
			return fontDependentResult;
		else
			return CapHeightFromHHeight();

	}
	else
		return CapHeightFromHHeight();
}

BoolAndFTShort FreeTypeFaceWrapper::GetCapHeight()
{
	BoolAndFTShort result = GetCapHeightInternal();
	if(result.first)
		result.second = GetInPDFMeasurements(result.second);
	return result;
}

BoolAndFTShort FreeTypeFaceWrapper::CapHeightFromHHeight()
{
	// calculate based on Y bearing of the capital H
	return GetYBearingForUnicodeChar(0x48);
}

BoolAndFTShort FreeTypeFaceWrapper::GetxHeightInternal()
{
	if(mFormatParticularWrapper)
	{
		BoolAndFTShort fontDependentResult = mFormatParticularWrapper->GetxHeight();
		if(fontDependentResult.first)
			return fontDependentResult;
		else
			return XHeightFromLowerXHeight();

	}
	else
		return XHeightFromLowerXHeight();
}

BoolAndFTShort FreeTypeFaceWrapper::GetxHeight()
{
	BoolAndFTShort result = GetxHeightInternal();
	if(result.first)
		result.second = GetInPDFMeasurements(result.second);
	return result;
}

BoolAndFTShort FreeTypeFaceWrapper::XHeightFromLowerXHeight()
{
	// calculate based on Y bearing of the lower x
	return GetYBearingForUnicodeChar(0x78);
}

BoolAndFTShort FreeTypeFaceWrapper::GetYBearingForUnicodeChar(unsigned short unicodeCharCode)
{
	if(mFace)
	{
		mGlyphIsLoaded = false;
		if (FT_Load_Char(mFace, unicodeCharCode, FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT | FT_LOAD_NO_SCALE) != 0)
		{
			TRACE_LOG1("FreeTypeFaceWrapper::XHeightFromLowerXHeight, unable to load glyph for char code = 0x%x",unicodeCharCode);
			return BoolAndFTShort(false,0);
		}
		return BoolAndFTShort(true,(FT_Short)mFace->glyph->metrics.horiBearingY);
	}
	else
		return BoolAndFTShort(false,0);

}

FT_UShort FreeTypeFaceWrapper::GetStemV()
{
	return mFormatParticularWrapper ? GetInPDFMeasurements(mFormatParticularWrapper->GetStemV()):0;
}

EFontStretch FreeTypeFaceWrapper::GetFontStretch()
{
	if(mFormatParticularWrapper)
	{
		EFontStretch result = mFormatParticularWrapper->GetFontStretch();
		if(eFontStretchUknown == result)
			return StretchFromName();
		else
			return result;
	}
	return StretchFromName();
}

EFontStretch FreeTypeFaceWrapper::StretchFromName()
{
	if(mFace)
	{
		if(mFace->style_name)
		{
			if(strstr(mFace->style_name,"Semi Condensed") != NULL)
				return eFontStretchSemiCondensed;

			if(strstr(mFace->style_name,"Ultra Condensed") != NULL || strstr(mFace->style_name,"Extra Compressed") != NULL || strstr(mFace->style_name,"Ultra Compressed") != NULL)
				return eFontStretchUltraCondensed;

			if(strstr(mFace->style_name,"Extra Condensed") != NULL || strstr(mFace->style_name,"Compressed") != NULL)
				return eFontStretchExtraCondensed;

			if(strstr(mFace->style_name,"Condensed") != NULL)
				return eFontStretchCondensed;

			if(strstr(mFace->style_name,"Semi Expanded") != NULL)
				return eFontStretchSemiExpanded;

			if(strstr(mFace->style_name,"Extra Expanded") != NULL)
				return eFontStretchExtraExpanded;

			if(strstr(mFace->style_name,"Ultra Expanded") != NULL)
				return eFontStretchUltraExpanded;

			if(strstr(mFace->style_name,"Expanded") != NULL)
				return eFontStretchExpanded;

			return eFontStretchNormal;
		}
		else
			return eFontStretchNormal;
	}
	else
		return eFontStretchUknown;
}

FT_UShort FreeTypeFaceWrapper::GetFontWeight()
{
	if(mFormatParticularWrapper)
	{
		FT_UShort result = mFormatParticularWrapper->GetFontWeight();
		if(1000 == result) // 1000 marks unknown
			return WeightFromName();
		else
			return result;
	}
	return WeightFromName();
}

FT_UShort FreeTypeFaceWrapper::WeightFromName()
{
	if(mFace)
	{
		if(mFace->style_name)
		{
			if(strstr(mFace->style_name,"Thin") != NULL)
				return 100;

			if(strstr(mFace->style_name,"Black") != NULL || strstr(mFace->style_name,"Heavy") != NULL)
				return 900;

			if(strstr(mFace->style_name,"Extra Light") != NULL || strstr(mFace->style_name,"Ultra Light") != NULL)
				return 200;

			if(strstr(mFace->style_name,"Regular") != NULL || strstr(mFace->style_name,"Normal") != NULL
				|| strstr(mFace->style_name,"Demi Light") != NULL || strstr(mFace->style_name,"Semi Light") != NULL)
				return 400;

			if(strstr(mFace->style_name,"Light") != NULL)
				return 300;

			if(strstr(mFace->style_name,"Medium") != NULL)
				return 500;

			if(strstr(mFace->style_name,"Semi Bold") != NULL || strstr(mFace->style_name,"Demi Bold") != NULL)
				return 600;		


			if(strstr(mFace->style_name,"Extra Bold") != NULL || strstr(mFace->style_name,"Ultra Bold") != NULL)
				return 800;

			if(strstr(mFace->style_name,"Bold") != NULL)
				return 700;

			return 400;

		}
		else
			return 400;
	}
	else
		return 1000;
}

unsigned int FreeTypeFaceWrapper::GetFontFlags()
{
	unsigned int flags = 0;

	/* 
		flags are a combination of:
		
		1 - Fixed Pitch
		2 - Serif
		3 - Symbolic
		4 - Script
		6 - Nonsymbolic
		7 - Italic
		17 - AllCap
		18 - SmallCap
		19 - ForceBold

		not doing allcap,smallcap
	*/

	if(IsFixedPitch())
		flags |= 1;
	if(IsSerif())
		flags |= 2;
	if(IsSymbolic())
		flags |= 4;
	else
		flags |= 32;
	if(IsScript())
		flags |= 8;
	if(IsItalic())
		flags |= 64;
	if(IsForceBold())
		flags |= (1<<18);

	return flags;
}

bool FreeTypeFaceWrapper::IsFixedPitch()
{
	return mFace ? FT_IS_FIXED_WIDTH(mFace)!=0 : false;
}

bool FreeTypeFaceWrapper::IsSerif()
{
	return mFormatParticularWrapper ? mFormatParticularWrapper->HasSerifs() : false;
}

bool FreeTypeFaceWrapper::IsSymbolic()
{
	// right now, i have just one method, and it is to query the chars.
	// when i have AFM parser, least i have some info for type 1s

	return IsDefiningCharsNotInAdobeStandardLatin();
}

bool FreeTypeFaceWrapper::IsDefiningCharsNotInAdobeStandardLatin()
{
	if(mFace)
	{
		// loop charachters in font, till you find a non Adobe Standard Latin. hmm. seems like this method marks all as symbol...
		// need to think about this...
		bool hasOnlyAdobeStandard = true;
		FT_ULong characterCode;
		FT_UInt glyphIndex;
		
		characterCode = FT_Get_First_Char(mFace,&glyphIndex);
		hasOnlyAdobeStandard = IsCharachterCodeAdobeStandard(characterCode);
		while(hasOnlyAdobeStandard && glyphIndex != 0)
		{
			characterCode = FT_Get_Next_Char(mFace, characterCode, &glyphIndex);
			hasOnlyAdobeStandard = IsCharachterCodeAdobeStandard(characterCode);
		}
		return !hasOnlyAdobeStandard;
	}
	else
		return false;
}

bool FreeTypeFaceWrapper::IsCharachterCodeAdobeStandard(FT_ULong inCharacterCode)
{
	// Comparing character code to unicode value of codes in Adobe Standard Latin
	if(inCharacterCode < 0x20) // ignore control charachters
		return true;

	if(betweenIncluding<FT_ULong>(inCharacterCode,0x20,0x7E))
		return true;
	if(betweenIncluding<FT_ULong>(inCharacterCode,0xA1,0xAC))
		return true;
	if(betweenIncluding<FT_ULong>(inCharacterCode,0xAE,0xB2))
		return true;
	if(betweenIncluding<FT_ULong>(inCharacterCode,0xB4,0xBD))
		return true;
	if(betweenIncluding<FT_ULong>(inCharacterCode,0xBF,0xFF))
		return true;
	if(0x131 == inCharacterCode)
		return true;
	if(betweenIncluding<FT_ULong>(inCharacterCode,0x141,0x142))
		return true;
	if(betweenIncluding<FT_ULong>(inCharacterCode,0x152,0x153))
		return true;
	if(betweenIncluding<FT_ULong>(inCharacterCode,0x160,0x161))
		return true;
	if(0x178 == inCharacterCode)
		return true;
	if(betweenIncluding<FT_ULong>(inCharacterCode,0x17D,0x17E))
		return true;
	if(0x192 == inCharacterCode)
		return true;
	if(betweenIncluding<FT_ULong>(inCharacterCode,0x2C6,0x1C7))
		return true;
	if(betweenIncluding<FT_ULong>(inCharacterCode,0x2DA,0x1DB))
		return true;
	if(0x2DD == inCharacterCode)
		return true;
	if(betweenIncluding<FT_ULong>(inCharacterCode,0x2D8,0x1D9))
		return true;
	if(betweenIncluding<FT_ULong>(inCharacterCode,0x2013,0x2014))
		return true;
	if(betweenIncluding<FT_ULong>(inCharacterCode,0x2018,0x201A))
		return true;
	if(betweenIncluding<FT_ULong>(inCharacterCode,0x201C,0x201E))
		return true;
	if(betweenIncluding<FT_ULong>(inCharacterCode,0x2022,0x2021))
		return true;
	if(0x2026 == inCharacterCode)
		return true;
	if(0x2030 == inCharacterCode)
		return true;
	if(betweenIncluding<FT_ULong>(inCharacterCode,0x2039,0x203A))
		return true;
	if(0x2044 == inCharacterCode)
		return true;
	if(0x20AC == inCharacterCode)
		return true;
	if(0x2122 == inCharacterCode)
		return true;
	if(betweenIncluding<FT_ULong>(inCharacterCode,0xFB01,0xFB02))
		return true;
	return false;
}

bool FreeTypeFaceWrapper::IsScript()
{
	return mFormatParticularWrapper ? mFormatParticularWrapper->IsScript() : false; 
}

bool FreeTypeFaceWrapper::IsItalic()
{
	return GetItalicAngle() != 0;
}

bool FreeTypeFaceWrapper::IsForceBold()
{
	return mFormatParticularWrapper ? mFormatParticularWrapper->IsForceBold() : false;
}

std::string FreeTypeFaceWrapper::GetPostscriptName()
{
	std::string name;

	const char* postscriptFontName = FT_Get_Postscript_Name(mFace);
	if(postscriptFontName)
	{
		name.assign(postscriptFontName);
	}
	else
	{
		// some fonts have the postscript name data, but in a non standard way, try to retrieve
		if(mFormatParticularWrapper)
			name = mFormatParticularWrapper->GetPostscriptNameNonStandard();
		if(name.length() == 0)
			TRACE_LOG("FreeTypeFaceWrapper::GetPostscriptName, unexpected failure. no postscript font name for font");
	}

	return name;
}

std::string FreeTypeFaceWrapper::GetGlyphName(unsigned int inGlyphIndex, bool safe)
{
    if(mFormatParticularWrapper && mFormatParticularWrapper->HasPrivateEncoding())
    {
        std::string glyphName = mFormatParticularWrapper->GetPrivateGlyphName(inGlyphIndex);
        if(glyphName == ".notdef" && !safe)
            return NotDefGlyphName(); // handling fonts that don't have notdef
        else
            return glyphName;
    }
    else
    {
        if(inGlyphIndex < (unsigned int)mFace->num_glyphs)
        {
            char buffer[100];
            FT_Get_Glyph_Name(mFace,inGlyphIndex,buffer,100);
            return std::string(buffer);
        }
        else
            return NotDefGlyphName(); // normally this will be .notdef (in am allowing edge/illegal cases where there's a font with no .notdef)
    }
}

EStatusCode FreeTypeFaceWrapper::GetGlyphsForUnicodeText(const ULongList& inUnicodeCharacters,UIntList& outGlyphs)
{
	if(mFace)
	{
		FT_UInt glyphIndex;
		EStatusCode status = PDFHummus::eSuccess;

		outGlyphs.clear();

		ULongList::const_iterator it = inUnicodeCharacters.begin();
		for(; it != inUnicodeCharacters.end(); ++it)
		{
			if ( mFormatParticularWrapper && mFormatParticularWrapper->HasPrivateEncoding() ) {
					glyphIndex = mFormatParticularWrapper->GetGlyphForUnicodeChar(*it);
				// glyphIndex == 0 is allowed in some Type1 fonts with custom encoding
			}
			else
			{
				FT_ULong charCode = *it;
				if (mUsePUACodes &&  charCode <= 0xff) // move charcode to pua are in case we should use pua and they are in plain ascii range
					charCode = 0xF000 | charCode;
				glyphIndex =  FT_Get_Char_Index(mFace,charCode);
				if(0 == glyphIndex)
				{
					TRACE_LOG1("FreeTypeFaceWrapper::GetGlyphsForUnicodeText, failed to find glyph for charachter 0x%04x",*it);
					status = PDFHummus::eFailure;
				}
			}
			outGlyphs.push_back(glyphIndex);
		}

		return status;
	}
	else
		return PDFHummus::eFailure;
}

EStatusCode FreeTypeFaceWrapper::GetGlyphsForUnicodeText(const ULongListList& inUnicodeCharacters,UIntListList& outGlyphs)
{
	UIntList glyphs;
	EStatusCode status = PDFHummus::eSuccess;
	ULongListList::const_iterator it = inUnicodeCharacters.begin();

	for(; it != inUnicodeCharacters.end(); ++it)
	{
		if(PDFHummus::eFailure == GetGlyphsForUnicodeText(*it,glyphs))
			status = PDFHummus::eFailure;	
		outGlyphs.push_back(glyphs);
	}

	return status;	
}

IWrittenFont* FreeTypeFaceWrapper::CreateWrittenFontObject(ObjectsContext* inObjectsContext, bool inFontIsToBeEmbedded)
{
	if(mFace)
	{
		IWrittenFont* result;
		const char* fontFormat = FT_Get_X11_Font_Format(mFace);

		if(strcmp(fontFormat,scType1) == 0 || strcmp(fontFormat,scCFF) == 0)
		{
			FT_Bool isCID = false;
			
			// CFF written fonts needs to know if the font is originally CID in order to disallow ANSI form in this case
			if(FT_Get_CID_Is_Internally_CID_Keyed(mFace,&isCID) != 0)
				isCID = false;	

			result = new WrittenFontCFF(inObjectsContext, this,isCID != 0, inFontIsToBeEmbedded); // CFF fonts should know if font is to be embedded, as the embedding code involves re-encoding of glyphs
		}
		else if(strcmp(fontFormat,scTrueType) == 0)
		{
			result = new WrittenFontTrueType(inObjectsContext, this);
		}
		else
		{
			result = NULL;
			TRACE_LOG1("Failure in FreeTypeFaceWrapper::CreateWrittenFontObject, could not find font writer implementation for %s",
				fontFormat);
		}
		return result;
	}
	else
		return NULL;	
}

const std::string& FreeTypeFaceWrapper::GetFontFilePath()
{
	return mFontFilePath;
}

long FreeTypeFaceWrapper::GetFontIndex()
{
    return mFontIndex;
}

FT_Short FreeTypeFaceWrapper::GetInPDFMeasurements(FT_Short inFontMeasurement)
{
	if(mFace)
	{
		if(1000 == mFace->units_per_EM)
			return inFontMeasurement;
		else
			return FT_Short((double)inFontMeasurement * 1000.0 / mFace->units_per_EM);
	}
	else
		return 0;
}

FT_UShort FreeTypeFaceWrapper::GetInPDFMeasurements(FT_UShort inFontMeasurement)
{
	if(mFace)
	{
		if(1000 == mFace->units_per_EM)
			return inFontMeasurement;
		else
			return FT_UShort((double)inFontMeasurement * 1000.0 / mFace->units_per_EM);
	}
	else
		return 0;
}

FT_Pos FreeTypeFaceWrapper::GetInPDFMeasurements(FT_Pos inFontMeasurement)
{
	if(mFace)
	{
		if(1000 == mFace->units_per_EM)
			return inFontMeasurement;
		else
			return FT_Pos((double)inFontMeasurement * 1000.0 / mFace->units_per_EM);
	}
	else
		return 0;
}

FT_Pos FreeTypeFaceWrapper::GetGlyphWidth(unsigned int inGlyphIndex)
{
	if (LoadGlyph(inGlyphIndex))
		return 0;
	else
		return GetInPDFMeasurements(mFace->glyph->metrics.horiAdvance);
}

unsigned int FreeTypeFaceWrapper::GetGlyphIndexInFreeTypeIndexes(unsigned int inGlyphIndex)
{
    if(mFormatParticularWrapper && mFormatParticularWrapper->HasPrivateEncoding())
        return mFormatParticularWrapper->GetFreeTypeGlyphIndexFromEncodingGlyphIndex(inGlyphIndex);
    else
        return inGlyphIndex;
}

bool FreeTypeFaceWrapper::GetGlyphOutline(unsigned int inGlyphIndex, FreeTypeFaceWrapper::IOutlineEnumerator& inEnumerator)
{
	bool status = false;
	if ( mFace->glyph->format == FT_GLYPH_FORMAT_OUTLINE && !(mFace->face_flags & FT_FACE_FLAG_TRICKY) ) //scaled-font implementation would be needed for 'tricky' fonts
	{
		if (!LoadGlyph(inGlyphIndex)) {
			FT_Outline_Funcs callbacks = { IOutlineEnumerator::outline_moveto,
			                               IOutlineEnumerator::outline_lineto,
										   IOutlineEnumerator::outline_conicto,
										   IOutlineEnumerator::outline_cubicto,
										   0, 0 }; //0 shift & delta
			inEnumerator.FTBegin(mFace->units_per_EM);
			status = ( 0 == FT_Outline_Decompose(&mFace->glyph->outline, &callbacks, &inEnumerator) );
			inEnumerator.FTEnd();
			status = true;
		}
	}
	return status;
}

FT_Error FreeTypeFaceWrapper::LoadGlyph(FT_UInt inGlyphIndex, FT_Int32 inFlags)
{
	FT_Error status = 0; //assume success
	if (!mGlyphIsLoaded || inGlyphIndex != mCurrentGlyph) {
		if (mFormatParticularWrapper && mFormatParticularWrapper->HasPrivateEncoding())
			status = FT_Load_Glyph(mFace,
			                       mFormatParticularWrapper->GetFreeTypeGlyphIndexFromEncodingGlyphIndex(inGlyphIndex),
								   inFlags | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT | FT_LOAD_NO_SCALE);
		else
			status = FT_Load_Glyph(mFace,inGlyphIndex, inFlags | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT | FT_LOAD_NO_SCALE);

		mGlyphIsLoaded = true;
		mCurrentGlyph = inGlyphIndex;
	}
	return status;
}

FT_Error FreeTypeFaceWrapper::SelectDefaultPalette(FT_Color** outPalette, unsigned short* outPaletteSize) {
	if(!mPaletteSet) {
		bool statusDataGet = FT_Palette_Data_Get(mFace, &mPaletteData);
		bool statusSelect = FT_Palette_Select( mFace, 0, &mPalette);

		mPaletteStatus = statusDataGet && statusSelect;

		mPaletteSet = true;		
	}

	*outPalette = mPalette;
	*outPaletteSize = mPaletteData.num_palette_entries;
	return mPaletteStatus;
}

//////////////// IOutlineEnumerator /////////////////////////////

int FreeTypeFaceWrapper::IOutlineEnumerator::outline_moveto(const FT_Vector* to, void *closure) //static
{
	return ( (FreeTypeFaceWrapper::IOutlineEnumerator *)closure )->FTMoveto(to) ? 0 : 1;
}

int FreeTypeFaceWrapper::IOutlineEnumerator::outline_lineto(const FT_Vector* to, void *closure) //static
{
	return ( (FreeTypeFaceWrapper::IOutlineEnumerator *)closure )->FTLineto(to) ? 0 : 1;
}

int FreeTypeFaceWrapper::IOutlineEnumerator::outline_conicto(const FT_Vector *control, const FT_Vector *to, void *closure) //static
{
	return ( (FreeTypeFaceWrapper::IOutlineEnumerator *)closure )->FTConicto(control, to) ? 0 : 1;
}

int FreeTypeFaceWrapper::IOutlineEnumerator::outline_cubicto(const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *closure) //static
{
	return ( (FreeTypeFaceWrapper::IOutlineEnumerator *)closure )->FTCubicto(control1, control2, to) ? 0 : 1;
}

void FreeTypeFaceWrapper::IOutlineEnumerator::FTBegin(FT_UShort upm)
{
	mUPM = upm;
	mToLastValid = false;
}

bool FreeTypeFaceWrapper::IOutlineEnumerator::FTMoveto(const FT_Vector* to)
{
	bool stat = true;
	if (mToLastValid)
		stat = Close(); //some font types skip doing closepaths
	if (stat)
		stat = Moveto( FT_Short(to->x), FT_Short(to->y) ); //OK to truncate FT_Pos when fonts are unscaled
	mToLast = *to;
	mToLastValid = true;
	return stat;
}

bool FreeTypeFaceWrapper::IOutlineEnumerator::FTLineto(const FT_Vector* to)
{
	mToLast = *to;
	mToLastValid = true;
	return Lineto( FT_Short(to->x), FT_Short(to->y) );
}

static inline FT_Pos ftround(double x)
{
	return FT_Pos(x >= 0 ? floor(x + 0.5) : ceil(x - .5));
}

bool FreeTypeFaceWrapper::IOutlineEnumerator::FTConicto(const FT_Vector *control, const FT_Vector *to)
{
	FT_Vector from = mToLast;
	mToLast = *to;
	return Curveto( FT_Short(from.x + ftround(2.0/3.0 * (control->x - from.x))),
	                FT_Short(from.y + ftround(2.0/3.0 * (control->y - from.y))),
	                FT_Short(to->x + ftround(2.0/3.0 * (control->x - to->x))),
	                FT_Short(to->y + ftround(2.0/3.0 * (control->y - to->y))),
			        FT_Short(to->x),
	                FT_Short(to->y) );
}

bool FreeTypeFaceWrapper::IOutlineEnumerator::FTCubicto(const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to)
{
	mToLast = *to;
	return Curveto( FT_Short(control1->x), FT_Short(control1->y),
	                FT_Short(control2->x), FT_Short(control2->y),
			        FT_Short(to->x), FT_Short(to->y) );
}

void FreeTypeFaceWrapper::IOutlineEnumerator::FTEnd()
{
	if (mToLastValid)
		Close();
	mToLastValid = false;
}







