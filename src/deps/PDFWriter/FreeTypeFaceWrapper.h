/*
   Source File : FreeTypeFaceWrapper.h


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

#include "EFontStretch.h"
#include "EStatusCode.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <utility>
#include <list>
#include <string>
#include <vector>

class IFreeTypeFaceExtender;
class IWrittenFont;
class ObjectsContext;



typedef std::pair<bool,FT_Short> BoolAndFTShort;
typedef std::list<unsigned int> UIntList;
typedef std::list<UIntList> UIntListList;
typedef std::list<std::string> StringList;
typedef std::list<unsigned long> ULongList;
typedef std::list<ULongList> ULongListList;

class FreeTypeFaceWrapper
{
public:
	// first overload - all but type ones. the file path is just given for storage (later queries may want it)
	FreeTypeFaceWrapper(FT_Face inFace,const std::string& inFontFilePath,long inFontIndex,bool inDoOwn = true);

	// second overload - type 1, to allow passing pfm file path. do not bother
	// if you don't have a PFM file. no i don't care about the godamn AFM file. just the PFM.
	// if you don't have a PFM i'll manage. again - i don't need the @#$@#$ AFM file. 
	// you see. i need to know if the font is serif, script 'n such. AFM - even if there
	// does not have that kind of info. so @#$@#$ off.
	// for any case, i'll check the file extension, and only do something about it if it has a pfm extension
	FreeTypeFaceWrapper(FT_Face inFace,const std::string& inFontFilePath,const std::string& inPFMFilePath,long inFontIndex,bool inDoOwn = true);
	~FreeTypeFaceWrapper(void);

	FT_Error DoneFace();

	FT_Face operator->();
	operator FT_Face();

	bool IsValid();

	PDFHummus::EStatusCode GetGlyphsForUnicodeText(const ULongList& inUnicodeCharacters,UIntList& outGlyphs);
	PDFHummus::EStatusCode GetGlyphsForUnicodeText(const ULongListList& inUnicodeCharacters,UIntListList& outGlyphs);

	double GetItalicAngle();
	BoolAndFTShort GetCapHeight(); // aligned to pdf metrics
	BoolAndFTShort GetxHeight(); // aligned to pdf metrics
	FT_UShort GetStemV(); // aligned to pdf metrics
	EFontStretch GetFontStretch();
	FT_UShort GetFontWeight();
	// these would be flags for the font as a whole. if subsetting, match to the character set
	unsigned int GetFontFlags();
	const char* GetTypeString();
    std::string GetGlyphName(unsigned int inGlyphIndex);
    FT_Pos GetGlyphWidth(unsigned int inGlyphIndex);

	// Create the written font object, matching to write this font in the best way.
	IWrittenFont* CreateWrittenFontObject(ObjectsContext* inObjectsContext);


	// flags determining values
	bool IsFixedPitch();
	bool IsSerif();
	bool IsScript();
	bool IsItalic();
	bool IsForceBold();

	// will be used externally to determine if font is symbolic or not
	bool IsCharachterCodeAdobeStandard(FT_ULong inCharacterCode);

	const std::string& GetFontFilePath();
    long GetFontIndex();


	// use this method to align measurements from (remember the dreaded point per EM!!!).
	// all measurements in this class are already aligned...so no need to align them
	FT_Short GetInPDFMeasurements(FT_Short inFontMeasurement);
	FT_UShort GetInPDFMeasurements(FT_UShort inFontMeasurement);
	FT_Pos GetInPDFMeasurements(FT_Pos inFontMeasurement);

    // translate to freetype glyph index. this may not always be the input, as in the case
    // of type 3 fonts with private encoding. so what actually gets in is the glyph index
    // according to hummus
    unsigned int GetGlyphIndexInFreeTypeIndexes(unsigned int inGlyphIndex);

private:

	FT_Face mFace;
	IFreeTypeFaceExtender* mFormatParticularWrapper;
	bool mHaslowercase;
	std::string mFontFilePath;
    long mFontIndex;
    std::string mNotDefGlyphName;
	bool mDoesOwn;

	BoolAndFTShort GetCapHeightInternal(); 
	BoolAndFTShort GetxHeightInternal(); 
	FT_UShort GetStemVInternal(); 


	std::string GetExtension(const std::string& inFilePath);
	void SetupFormatSpecificExtender(const std::string& inFilePath, const std::string& inPFMFilePath);
	BoolAndFTShort CapHeightFromHHeight();
	BoolAndFTShort XHeightFromLowerXHeight();
	BoolAndFTShort GetYBearingForUnicodeChar(unsigned short unicodeCharCode);
	EFontStretch StretchFromName();
	FT_UShort WeightFromName();
	bool IsSymbolic();
	bool IsDefiningCharsNotInAdobeStandardLatin();
    void SetupNotDefGlyph();
};

