/*
   Source File : FreeTypeType1Wrapper.cpp


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
#include "FreeTypeType1Wrapper.h"
#include "InputFile.h"
#include "Trace.h"



FreeTypeType1Wrapper::FreeTypeType1Wrapper(FT_Face inFace,const std::string& inFontFilePath,const std::string& inPFMFilePath)
{
	if(FT_Get_PS_Font_Info(inFace,&mPSFontInfo) != 0)
	{
		TRACE_LOG("Unexpected failure in FreeTypeType1Wrapper::FreeTypeType1Wrapper. could not retrieve PS font info");
		mPSavailable = false; // this is obviousy an exception
	}
	else
		mPSavailable = true; 


	if(FT_Get_PS_Font_Private(inFace,&mPrivateInfo) != 0)
	{
		TRACE_LOG("Unexpected failure in FreeTypeType1Wrapper::FreeTypeType1Wrapper. could not retrieve PS private font info");
		mPSPrivateAvailable = false; // this is obviousy an exception
	}
	else
		mPSPrivateAvailable = true;
    
    T1_EncodingType encodingType;
    FT_Get_PS_Font_Value(inFace, PS_DICT_ENCODING_TYPE, 0, (void*)&encodingType, sizeof(encodingType));
    mIsCustomEncoding = encodingType == T1_ENCODING_TYPE_ARRAY;

	mPFMFileInfoRelevant = 
		(inPFMFilePath.size() != 0 && mPFMReader.Read(inPFMFilePath) != PDFHummus::eFailure);
    
    // parse type 1 input file (my own parsing), to get extra info about encoding
    if(inFontFilePath.size() != 0)
    {
        InputFile type1File;
    
        type1File.OpenFile(inFontFilePath);
        mType1File.ReadType1File(type1File.GetInputStream());
    
        type1File.CloseFile();
    }
    
    mFace = inFace;
}

FreeTypeType1Wrapper::~FreeTypeType1Wrapper(void)
{
}

double FreeTypeType1Wrapper::GetItalicAngle()
{
	return mPSavailable ? (double)mPSFontInfo.italic_angle : 0;
}

BoolAndFTShort FreeTypeType1Wrapper::GetCapHeight()
{
	if(mPFMFileInfoRelevant)
		return BoolAndFTShort(true,mPFMReader.ExtendedFontMetrics.CapHeight);
	else
		return BoolAndFTShort(false,0);
}

BoolAndFTShort FreeTypeType1Wrapper::GetxHeight()
{
	if(mPFMFileInfoRelevant)
		return BoolAndFTShort(true,mPFMReader.ExtendedFontMetrics.XHeight);
	else
		return BoolAndFTShort(false,0);
}

FT_UShort FreeTypeType1Wrapper::GetStemV()
{
	// StemV of PDF matches the StdWV of type 1...so piece of cake here
	return mPSPrivateAvailable ? mPrivateInfo.standard_width[0]:0;
}

EFontStretch FreeTypeType1Wrapper::GetFontStretch()
{
	return eFontStretchUknown;
}

FT_UShort FreeTypeType1Wrapper::GetFontWeight()
{
	// ahhh. using the weight factor is as good as analyzing the name...whatever. so just return the default value
	return 1000;
}

bool FreeTypeType1Wrapper::HasSerifs()
{
	if(mPFMFileInfoRelevant)
		return (mPFMReader.Header.PitchAndFamily & 16) != 0;
	else
		return false;

}

bool FreeTypeType1Wrapper::IsScript()
{
	if(mPFMFileInfoRelevant)
		return (mPFMReader.Header.PitchAndFamily & 64) != 0;
	else
		return false;	
}

bool FreeTypeType1Wrapper::IsForceBold()
{
	return mPSPrivateAvailable ? (1 == mPrivateInfo.force_bold) : false;
}

bool FreeTypeType1Wrapper::HasPrivateEncoding()
{
    return mIsCustomEncoding;
}

unsigned int FreeTypeType1Wrapper::GetGlyphForUnicodeChar(unsigned long inChar)
{
    return (unsigned int)inChar; // will run only if custom encoding, in which case input value should be output value
}


std::string FreeTypeType1Wrapper::GetPrivateGlyphName(unsigned int inGlyphIndex)
{
    return mType1File.GetGlyphCharStringName(inGlyphIndex);
}

unsigned int FreeTypeType1Wrapper::GetFreeTypeGlyphIndexFromEncodingGlyphIndex(unsigned int inGlyphIndex)
{
    return FT_Get_Name_Index(mFace,(FT_String*)(GetPrivateGlyphName(inGlyphIndex).c_str()));
}

std::string FreeTypeType1Wrapper::GetPostscriptNameNonStandard()
{
	// only way to go for type1 is standard...so return empty
	return std::string();
}