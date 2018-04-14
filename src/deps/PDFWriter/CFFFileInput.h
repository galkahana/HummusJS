/*
   Source File : CFFFileInput.h


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
#include "IByteReaderWithPosition.h"
#include "CFFPrimitiveReader.h"
#include "IType2InterpreterImplementation.h"

#include <string.h>

#include <string>
#include <list>
#include <map>
#include <set>
#include <utility>
#include <vector>



struct CFFHeader
{
	Byte major;
	Byte minor;
	Byte hdrSize;
	Byte offSize;
};

typedef std::list<std::string> StringList;
typedef std::map<std::string,unsigned short> StringToUShort;
typedef std::pair<bool,unsigned short> BoolAndUShort;

typedef std::list<CharString*> CharStringList;

typedef CharString* CharStringsIndex;

// this time it's the font charstrings
struct CharStrings
{
	CharStrings(){mCharStringsIndex = NULL; mCharStringsType = 0; mCharStringsCount = 0;}

	Byte mCharStringsType;
	unsigned short mCharStringsCount;
	CharStringsIndex mCharStringsIndex;
};

enum ECharSetType
{
	eCharSetISOAdobe = 0,
	eCharSetExpert,
	eCharSetExpertSubset,
	eCharSetCustom
};

typedef std::map<unsigned short,CharString*> UShortToCharStringMap;

struct CharSetInfo
{	
	CharSetInfo() {mSIDs = NULL;}

	ECharSetType mType;
	UShortToCharStringMap mSIDToGlyphMap;
	unsigned short* mSIDs; // count is like glyphs count
};

typedef std::vector<CharSetInfo*> CharSetInfoVector;


enum EEncodingType
{
	eEncodingStandard = 0,
	eEncodingExpert,
	eEncodingCustom
};

typedef std::list<Byte> ByteList;
typedef std::map<unsigned short,ByteList> UShortToByteList;

typedef std::pair<Byte,unsigned short> ByteAndUShort;

struct EncodingsInfo
{
	EncodingsInfo() {mEncoding = NULL;}

	LongFilePositionType mEncodingStart;
	LongFilePositionType mEncodingEnd;

	EEncodingType mType;
	Byte mEncodingsCount;
	Byte* mEncoding;
	UShortToByteList mSupplements;
	
};

struct PrivateDictInfo
{
	PrivateDictInfo() {mPrivateDictStart=0;mPrivateDictEnd=0;mLocalSubrs=NULL;}

	LongFilePositionType mPrivateDictStart;
	LongFilePositionType mPrivateDictEnd;
	UShortToDictOperandListMap mPrivateDict;
	CharStrings* mLocalSubrs;

};

typedef std::map<LongFilePositionType,CharStrings*> LongFilePositionTypeToCharStringsMap;

struct FontDictInfo
{
	LongFilePositionType mFontDictStart;
	LongFilePositionType mFontDictEnd;
	UShortToDictOperandListMap mFontDict;
	PrivateDictInfo mPrivateDict;
};

struct TopDictInfo
{
	TopDictInfo() {
					mFDArray = NULL;
					mFDSelect = NULL;
					mCharSet = NULL;
					mEncoding = NULL;
				}

	UShortToDictOperandListMap mTopDict;
	CharSetInfo* mCharSet;
	EncodingsInfo* mEncoding;
	FontDictInfo* mFDArray;
	FontDictInfo** mFDSelect; // size is like glyphsize. each cell references the relevant FontDict
};

typedef std::vector<EncodingsInfo*> EncodingsInfoVector;




struct StringLess
{
	bool operator( ) (const char* left, 
						const char* right ) const
	{
		return strcmp(left,right) < 0;
	}
};

typedef std::set<unsigned short> UShortSet;

struct CharString2Dependencies
{
	UShortSet mCharCodes; // from seac-like endchar operator
	UShortSet mGlobalSubrs; // from callgsubr
	UShortSet mLocalSubrs; // from callsubr
};

typedef std::map<const char*,unsigned short,StringLess> CharPToUShortMap;

class CFFFileInput : public Type2InterpreterImplementationAdapter
{
public:
	CFFFileInput(void);
	~CFFFileInput(void);

	// parses the whole CFF file, with all contained fonts
	PDFHummus::EStatusCode ReadCFFFile(IByteReaderWithPosition* inCFFFile);
	// parses the CFF file just for the particular font according to index. Index should be 
	// according to how it appears in the CFF
	PDFHummus::EStatusCode ReadCFFFile(IByteReaderWithPosition* inCFFFile,unsigned short inFontIndex);
	// parses the CFF file just for the particular named font
	PDFHummus::EStatusCode ReadCFFFile(IByteReaderWithPosition* inCFFFile,const std::string& inFontName);

	// call only <i> after </i> calling the read method...got it?
	// calculate dependencies for a given charstring [it can be char, gsubr or localsubr].
	// the function is recursive, so that if the charstring has dependencies, then by the end of the
	// process their dependencies will be calculated too.
	// apart from the charstring you need to pass also the relevant localsubrs for this font, in case subrs
	// will be required. also the relevnat charstring and charset array should be passed in case
	// the seac flavor of endchar will be encountered.
	PDFHummus::EStatusCode CalculateDependenciesForCharIndex(unsigned short inFontIndex,
												  unsigned short inCharStringIndex,
												  CharString2Dependencies& ioDependenciesInfo);

	unsigned short GetFontsCount(unsigned short inFontIndex);
	unsigned short GetCharStringsCount(unsigned short inFontIndex);
	std::string GetGlyphName(unsigned short inFontIndex,unsigned short inGlyphIndex);
	unsigned short GetGlyphSID(unsigned short inFontIndex,unsigned short inGlyphIndex);
	CharString* GetGlyphCharString(unsigned short inFontIndex,
				 				   unsigned short inCharStringIndex);

	// use this wonderful little fellow when interpreting a charstring with
	// CharStringType2Intepreter. This will set the CFFFileInput with 
	// the right items so that later you can use the IType2InterpreterImplementation
	// implementation here and avoid having to mess with passing gsubrs, lsubrs etc.
	// when interpreting just call these methods instead of yours to perform the relevant actions
	PDFHummus::EStatusCode PrepareForGlyphIntepretation(unsigned short inFontIndex,
				 							unsigned short inCharStringIndex);

	void Reset();


	// IType2InterpreterImplementation overrides
	virtual PDFHummus::EStatusCode ReadCharString(LongFilePositionType inCharStringStart,
							   LongFilePositionType inCharStringEnd,
							   Byte** outCharString);
	virtual CharString* GetLocalSubr(long inSubrIndex); 
	virtual CharString* GetGlobalSubr(long inSubrIndex);
	virtual PDFHummus::EStatusCode Type2Endchar(const CharStringOperandList& inOperandList);


	// publicly available constructs
	
	// mCFFOffset should be added to any position here when referring to the beginning if the file containing this
	// segment. for instance, cff could be part of an OTF file definition, in which case the position is not 0.
	LongFilePositionType mCFFOffset;

	CFFHeader mHeader;
	unsigned short mFontsCount;
	StringList mName;
	TopDictInfo* mTopDictIndex; // count is same as fonts count
	char** mStrings;
	unsigned short mStringsCount;
	LongFilePositionType mStringIndexPosition;
	LongFilePositionType mGlobalSubrsPosition;
	PrivateDictInfo* mPrivateDicts; // private dicts are the same as fonts count. refers to the topdict related private dics, not to the FontDicts scenarios in CID.

private:
	CharStrings mGlobalSubrs;
	CharStrings* mCharStrings; // count is same as fonts count
	LongFilePositionTypeToCharStringsMap mLocalSubrs; // count is NOT the same as fonts count [some may be shared, plus there might be more because of CID usage]
	CharSetInfoVector mCharSets;// count is NOT the same as fonts count [some charsets may be shared]. consult the top dict charset pointer for the right charset
	EncodingsInfoVector mEncodings; // count is NOT the same as fonts count [some encodinds may be shared].

	CFFPrimitiveReader mPrimitivesReader;
	StringToUShort mNameToIndex;
	LongFilePositionType mNameIndexPosition;
	LongFilePositionType mTopDictIndexPosition;
	CharPToUShortMap mStringToSID;

	// for dependencies calculations using glyph interpretations. state.
	CharString2Dependencies* mCurrentDependencies;
	CharStrings* mCurrentLocalSubrs;
	CharStringList mAdditionalGlyphs;
	CharSetInfo* mCurrentCharsetInfo;

	std::string GetStringForSID(unsigned short inSID);	
	PDFHummus::EStatusCode ReadHeader();
	PDFHummus::EStatusCode ReadNameIndex();
	PDFHummus::EStatusCode ReadIndexHeader(unsigned long** outOffsets,unsigned short& outItemsCount);
	PDFHummus::EStatusCode ReadTopDictIndex();
	PDFHummus::EStatusCode ReadDict(unsigned long inReadAmount,UShortToDictOperandListMap& outDict);
	PDFHummus::EStatusCode ReadStringIndex();
	PDFHummus::EStatusCode ReadGlobalSubrs();
	PDFHummus::EStatusCode ReadCharStrings();
	PDFHummus::EStatusCode ReadCharsets();
	PDFHummus::EStatusCode ReadEncodings();
	void FreeData();
	LongFilePositionType GetCharStringsPosition(unsigned short inFontIndex);
	long GetSingleIntegerValue(unsigned short inFontIndex,unsigned short inKey,long inDefault);
	PDFHummus::EStatusCode ReadSubrsFromIndex(unsigned short& outSubrsCount,CharStringsIndex* outSubrsIndex);
	long GetCharStringType(unsigned short inFontIndex);
	PDFHummus::EStatusCode ReadPrivateDicts();
	PDFHummus::EStatusCode ReadLocalSubrs();
	long GetSingleIntegerValueFromDict(const UShortToDictOperandListMap& inDict,
												 unsigned short inKey,
												 long inDefault);
	LongFilePositionType GetCharsetPosition(unsigned short inFontIndex);
	LongFilePositionType GetEncodingPosition(unsigned short inFontIndex);
	unsigned short GetBiasedIndex(unsigned short inSubroutineCollectionSize, long inSubroutineIndex);
	PDFHummus::EStatusCode ReadFormat0Charset(bool inIsCID, UShortToCharStringMap& ioCharMap,unsigned short** inSIDArray,const CharStrings& inCharStrings);
	PDFHummus::EStatusCode ReadFormat1Charset(bool inIsCID,UShortToCharStringMap& ioCharMap,unsigned short** inSIDArray,const CharStrings& inCharStrings);
	PDFHummus::EStatusCode ReadFormat2Charset(bool inIsCID,UShortToCharStringMap& ioCharMap,unsigned short** inSIDArray,const CharStrings& inCharStrings);
	void SetupSIDToGlyphMapWithStandard(const unsigned short* inStandardCharSet,
										unsigned short inStandardCharSetLength,
										UShortToCharStringMap& ioCharMap,
										const CharStrings& inCharStrings);
	CharString* GetCharacterFromStandardEncoding(Byte inCharacterCode);
	PDFHummus::EStatusCode ReadCIDInformation();
	PDFHummus::EStatusCode ReadFDArray(unsigned short inFontIndex);
	PDFHummus::EStatusCode ReadFDSelect(unsigned short inFontIndex);
	LongFilePositionType GetFDArrayPosition(unsigned short inFontIndex);
	PDFHummus::EStatusCode ReadPrivateDict(const UShortToDictOperandListMap& inReferencingDict,PrivateDictInfo* outPrivateDict);
	PDFHummus::EStatusCode ReadLocalSubrsForPrivateDict(PrivateDictInfo* inPrivateDict,Byte inCharStringType);
	LongFilePositionType GetFDSelectPosition(unsigned short inFontIndex);
	BoolAndUShort GetIndexForFontName(const std::string& inFontName);
	PDFHummus::EStatusCode ReadTopDictIndex(unsigned short inFontIndex);
	PDFHummus::EStatusCode ReadCharStrings(unsigned short inFontIndex);
	PDFHummus::EStatusCode ReadPrivateDicts(unsigned short inFontIndex);
	PDFHummus::EStatusCode ReadLocalSubrs(unsigned short inFontIndex);
	PDFHummus::EStatusCode ReadCharsets(unsigned short inFontIndex);
	void ReadEncoding(EncodingsInfo* inEncoding,LongFilePositionType inEncodingPosition);
	PDFHummus::EStatusCode ReadEncodings(unsigned short inFontIndex);
	PDFHummus::EStatusCode ReadCIDInformation(unsigned short inFontIndex);
	PDFHummus::EStatusCode ReadCFFFileByIndexOrName(IByteReaderWithPosition* inCFFFile,const std::string& inFontName,unsigned short inFontIndex);
};

