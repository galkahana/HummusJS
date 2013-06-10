/*
   Source File : Type1Input.h


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
#include "InputPFBDecodeStream.h"
#include "IType1InterpreterImplementation.h"

#include <string>
#include <map>
#include <vector>
#include <set>



enum EType1EncodingType
{
	eType1EncodingTypeStandardEncoding,
	eType1EncodingTypeCustom
};

struct Type1Encoding
{
	EType1EncodingType EncodingType;
	std::string mCustomEncoding[256];
};

struct Type1FontDictionary
{
	std::string FontName;
	int PaintType;
	int FontType;
	double FontMatrix[6];
	double FontBBox[4];
	int UniqueID;
	// Metrics ignored
	double StrokeWidth;
	bool FSTypeValid;
	unsigned short fsType;
};

struct Type1FontInfoDictionary
{
	std::string version;
	std::string Notice;
	std::string Copyright;
	std::string FullName;
	std::string FamilyName;
	std::string Weight;
	double ItalicAngle;
	bool isFixedPitch;
	double UnderlinePosition;
	double UnderlineThickness;
	bool FSTypeValid;
	unsigned short fsType;
};

struct Type1PrivateDictionary
{
	int UniqueID;
	std::vector<int> BlueValues;
	std::vector<int> OtherBlues;
	std::vector<int> FamilyBlues;
	std::vector<int> FamilyOtherBlues;
	double BlueScale;
	int BlueShift;
	int BlueFuzz;
	double StdHW;
	double StdVW;
	std::vector<double> StemSnapH;
	std::vector<double> StemSnapV;
	bool ForceBold;
	int LanguageGroup;
	int lenIV;
	bool RndStemUp;
};


typedef std::set<Byte> ByteSet;
typedef std::set<unsigned short> UShortSet;

struct CharString1Dependencies
{
	ByteSet mCharCodes; // from seac operator
	UShortSet mOtherSubrs; // from callothersubr
	UShortSet mSubrs; // from callsubr
};


typedef std::vector<Type1CharString> Type1CharStringVector;
typedef std::map<std::string,Type1CharString> StringToType1CharStringMap;
typedef std::map<std::string,Byte> StringToByteMap;

class IByteReaderWithPosition;

class Type1Input : public Type1InterpreterImplementationAdapter
{
public:
	Type1Input(void);
	~Type1Input(void);

	PDFHummus::EStatusCode ReadType1File(IByteReaderWithPosition* inType1File);
	PDFHummus::EStatusCode CalculateDependenciesForCharIndex(Byte inCharStringIndex,
												  CharString1Dependencies& ioDependenciesInfo);
	PDFHummus::EStatusCode CalculateDependenciesForCharIndex(const std::string& inCharStringName,
												  CharString1Dependencies& ioDependenciesInfo);
	void Reset();
	Type1CharString* GetGlyphCharString(const std::string& inCharStringName);
	Type1CharString* GetGlyphCharString(Byte inCharStringIndex);
	std::string GetGlyphCharStringName(Byte inCharStringIndex);
	bool IsValidGlyphIndex(Byte inCharStringIndex);
	Byte GetEncoding(const std::string& inCharStringName);
    bool IsCustomEncoding();
    
	// some structs for you all laddies and lasses
	Type1FontDictionary mFontDictionary;
	Type1FontInfoDictionary mFontInfoDictionary;
	Type1PrivateDictionary mPrivateDictionary;

	// IType1InterpreterImplementation overrides
	virtual Type1CharString* GetSubr(long inSubrIndex);
	virtual PDFHummus::EStatusCode Type1Seac(const LongList& inOperandList);
	virtual bool IsOtherSubrSupported(long inOtherSubrsIndex);
	virtual unsigned long GetLenIV();

private:
	Type1Encoding mEncoding;
	StringToByteMap mReverseEncoding;
	long mSubrsCount;
	Type1CharString* mSubrs;
	StringToType1CharStringMap mCharStrings;

	InputPFBDecodeStream mPFBDecoder;

	CharString1Dependencies* mCurrentDependencies;


	void FreeTables();
	bool IsComment(const std::string& inToken);
	PDFHummus::EStatusCode ReadFontDictionary();
	PDFHummus::EStatusCode ReadFontInfoDictionary();
	std::string FromPSName(const std::string& inPostScriptName);
	PDFHummus::EStatusCode ParseEncoding();
	PDFHummus::EStatusCode ReadPrivateDictionary();
	PDFHummus::EStatusCode ParseIntVector(std::vector<int>& inVector);
	PDFHummus::EStatusCode ParseDoubleVector(std::vector<double>& inVector);
	PDFHummus::EStatusCode ParseSubrs();
	PDFHummus::EStatusCode ParseCharstrings();
	PDFHummus::EStatusCode ParseDoubleArray(double* inArray,int inArraySize);
	std::string FromPSString(const std::string& inPSString);
	void CalculateReverseEncoding();
};
