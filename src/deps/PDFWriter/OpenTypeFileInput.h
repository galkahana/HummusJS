/*
   Source File : OpenTypeFileInput.h


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
#include "OpenTypePrimitiveReader.h"
#include "IByteReaderWithPosition.h"
#include "CFFFileInput.h"

#include <string>
#include <map>
#include <list>



struct TableEntry
{
	unsigned long CheckSum;
	unsigned long Offset;
	unsigned long Length;
};

typedef std::map<unsigned long,TableEntry> ULongToTableEntryMap;


struct HeadTable
{
	double TableVersionNumber;
	double FontRevision;
	unsigned long CheckSumAdjustment;
	unsigned long MagicNumber;
	unsigned short Flags;
	unsigned short UnitsPerEm;
	long long Created;
	long long Modified;
	short XMin;
	short YMin;
	short XMax;
	short YMax;
	unsigned short MacStyle;
	unsigned short LowerRectPPEM;
	short FontDirectionHint;
	short IndexToLocFormat;
	short GlyphDataFormat;
};

struct MaxpTable
{
	double TableVersionNumber;
	unsigned short NumGlyphs;
	unsigned short MaxPoints;
	unsigned short MaxCountours;
	unsigned short MaxCompositePoints;
	unsigned short MaxCompositeContours;
	unsigned short MaxZones;
	unsigned short MaxTwilightPoints;
	unsigned short MaxStorage;
	unsigned short MaxFunctionDefs;
	unsigned short MaxInstructionDefs;
	unsigned short MaxStackElements;
	unsigned short MaxSizeOfInstructions;
	unsigned short MaxComponentElements;
	unsigned short MaxCompontentDepth;
};

struct HHeaTable
{
	double TableVersionNumber;
	short Ascender;
	short Descender;
	short LineGap;
	unsigned short AdvanceWidthMax;
	short MinLeftSideBearing;
	short MinRightSideBearing;
	short XMaxExtent;
	short CaretSlopeRise;
	short CaretSlopeRun;
	short CaretOffset;
	short MetricDataFormat;
	unsigned short NumberOfHMetrics;
};

struct HMtxTableEntry
{
	unsigned short AdvanceWidth;
	short LeftSideBearing;
};

typedef HMtxTableEntry* HMtxTable;

struct OS2Table
{
	unsigned short Version;
	short AvgCharWidth;
	unsigned short WeightClass;
	unsigned short WidthClass;
	unsigned short fsType;
	short SubscriptXSize;
	short SubscriptYSize;
	short SubscriptXOffset;
	short SubscriptYOffset;
	short SuperscriptXSize;
	short SuperscriptYSize;
	short SuperscriptXOffset;
	short SuperscriptYOffset;
	short StrikeoutSize;
	short StrikeoutPosition;
	short FamilyClass;
	Byte Panose[10];
	unsigned long UnicodeRange1;
	unsigned long UnicodeRange2;
	unsigned long UnicodeRange3;
	unsigned long UnicodeRange4;
	char AchVendID[4];
	unsigned short FSSelection;
	unsigned short FirstCharIndex;
	unsigned short LastCharIndex;
	short TypoAscender;
	short TypoDescender;
	short TypoLineGap;
	unsigned short WinAscent;
	unsigned short WinDescent;
	unsigned long CodePageRange1;
	unsigned long CodePageRange2;
	short XHeight;
	short CapHeight;
	unsigned short DefaultChar;
	unsigned short BreakChar;
	unsigned short MaxContext;
};

struct NameTableEntry
{
	unsigned short PlatformID;
	unsigned short EncodingID;
	unsigned short LanguageID;
	unsigned short NameID;
	unsigned short Length;
	unsigned short Offset;
	char* String;
};


struct NameTable
{
	unsigned short mNameEntriesCount;
	NameTableEntry* mNameEntries;
};

typedef unsigned long* LocaTable;
typedef std::list<unsigned long> ULongList;
typedef std::list<unsigned int> UIntList;

/* this time it's gonna be just what's intersting for my subsetting purposes - which is the 
   dependencies ('n some other stuff) */
struct GlyphEntry
{
	short NumberOfContours;
	short XMin;
	short YMin;
	short XMax;
	short YMax;
	UIntList mComponentGlyphs; // will be empty for simple glyphs, and with component glyph indexes for components
};

typedef GlyphEntry** GlyfTable;

typedef std::map<unsigned short,GlyphEntry*> UShortToGlyphEntryMap;


enum EOpenTypeInputType
{
	EOpenTypeTrueType,
	EOpenTypeCFF
};

class OpenTypeFileInput
{
public:
	OpenTypeFileInput(void);
	~OpenTypeFileInput(void);

	PDFHummus::EStatusCode ReadOpenTypeFile(const std::string& inFontFilePath, unsigned short inFaceIndex);
	PDFHummus::EStatusCode ReadOpenTypeFile(IByteReaderWithPosition* inTrueTypeFile, unsigned short inFaceIndex);


	EOpenTypeInputType GetOpenTypeFontType();

	unsigned short GetGlyphsCount();

    unsigned long mHeaderOffset;
    unsigned long mTableOffset;
    
    unsigned short mFaceIndex;
    
	// read tables
	HeadTable mHead;
	MaxpTable mMaxp;
	HHeaTable mHHea;
	HMtxTable mHMtx;
	OS2Table mOS2;
	NameTable mName;
	LocaTable mLoca;
	GlyfTable mGlyf;

    // OS2 (surprise may not always exist. in dfonts for instance)
    bool mOS2Exists;
    
	// not read, but can tell if they are there
	bool mCVTExists;
	bool mFPGMExists;
	bool mPREPExists;

	TableEntry* GetTableEntry(const char* inTagName);

	CFFFileInput mCFF;
	
private:
	OpenTypePrimitiveReader mPrimitivesReader;
	EOpenTypeInputType mFontType;
	unsigned short mTablesCount;
	ULongToTableEntryMap mTables;
	UShortToGlyphEntryMap mActualGlyphs; // using actual glyphs to map the glyphs that are not empty 
										 // (yeah, when parsing subset fonts...some glyphs might just
										 // be empty, to avoid having to change the glyphs indices. some
										 // technique some producers use

	PDFHummus::EStatusCode ReadOpenTypeHeader();
	PDFHummus::EStatusCode ReadOpenTypeSFNT();
    PDFHummus::EStatusCode ReadOpenTypeSFNTFromDfont();
	PDFHummus::EStatusCode ReadHead();
	PDFHummus::EStatusCode ReadMaxP();
	PDFHummus::EStatusCode ReadHHea();
	PDFHummus::EStatusCode ReadHMtx();
	PDFHummus::EStatusCode ReadOS2();
	PDFHummus::EStatusCode ReadName();
	PDFHummus::EStatusCode ReadLoca();
	PDFHummus::EStatusCode ReadGlyfForDependencies();
	unsigned long GetTag(const char* inTagName);
	void FreeTables();
	PDFHummus::EStatusCode ReadCFF();

};
