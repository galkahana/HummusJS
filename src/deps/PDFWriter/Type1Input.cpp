/*
   Source File : Type1Input.cpp


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
#include "Type1Input.h"
#include "IByteReaderWithPosition.h"
#include "BoxingBase.h"
#include "PSBool.h"
#include "StandardEncoding.h"
#include "Trace.h"
#include "CharStringType1Interpreter.h"
#include <sstream>

using namespace PDFHummus;

Type1Input::Type1Input(void)
{
	mSubrsCount = 0;
	mSubrs = NULL;
	mCurrentDependencies = NULL;
}

Type1Input::~Type1Input(void)
{
	FreeTables();
}

void Type1Input::FreeTables()
{
	for(long i=0;i<mSubrsCount;++i)
		delete[] mSubrs[i].Code;
	delete[] mSubrs;
	mSubrs = NULL;
	mSubrsCount = 0;

	StringToType1CharStringMap::iterator itCharStrings = mCharStrings.begin();

	for(; itCharStrings != mCharStrings.end(); ++itCharStrings)
		delete[] itCharStrings->second.Code;
	mCharStrings.clear();
}

void Type1Input::Reset()
{
	// Reset type 1 data to default values

	FreeTables();

	mFontDictionary.FontMatrix[1] = mFontDictionary.FontMatrix[2] = mFontDictionary.FontMatrix[4] = mFontDictionary.FontMatrix[5] = 0;
	mFontDictionary.FontMatrix[0] = mFontDictionary.FontMatrix[3] = 0.001;
	mFontDictionary.UniqueID = -1;
	for(int i=0;i<256;++i)
		mEncoding.mCustomEncoding[i].clear();
	mReverseEncoding.clear();
	mFontDictionary.StrokeWidth = 1;
	mFontDictionary.FSTypeValid = false;
	mFontDictionary.fsType = 0;

	mFontInfoDictionary.isFixedPitch = false;
	mFontInfoDictionary.ItalicAngle = 0;
	mFontInfoDictionary.Notice.clear();
	mFontInfoDictionary.version.clear();
	mFontInfoDictionary.Weight.clear();
	mFontInfoDictionary.Copyright.clear();
	mFontInfoDictionary.FSTypeValid = false;
	mFontInfoDictionary.fsType = 0;

	mPrivateDictionary.BlueFuzz = 1;
	mPrivateDictionary.BlueScale = 0.039625;
	mPrivateDictionary.BlueShift = 7;
	mPrivateDictionary.BlueValues.clear();
	mPrivateDictionary.OtherBlues.clear();
	mPrivateDictionary.FamilyBlues.clear();
	mPrivateDictionary.FamilyOtherBlues.clear();
	mPrivateDictionary.ForceBold = false;
	mPrivateDictionary.LanguageGroup = 0;
	mPrivateDictionary.lenIV = 4;
	mPrivateDictionary.RndStemUp = false;
	mPrivateDictionary.StdHW = -1;
	mPrivateDictionary.StdVW = -1;
	mPrivateDictionary.StemSnapH.clear();
	mPrivateDictionary.StemSnapV.clear();
	mPrivateDictionary.UniqueID = -1;

	Type1PrivateDictionary mPrivateDictionary;
}

EStatusCode Type1Input::ReadType1File(IByteReaderWithPosition* inType1)
{
	EStatusCode status = eSuccess;
	BoolAndString token;


	Reset();

	do
	{
		status = mPFBDecoder.Assign(inType1);
		if(status != eSuccess)
			break;

		// the fun about pfb decoding is that it's pretty much token based...so let's do some tokening
		while(mPFBDecoder.NotEnded() && eSuccess == status)
		{
			token = mPFBDecoder.GetNextToken();
			status = mPFBDecoder.GetInternalState();
			// token not having been read, is not necessarily bad. could be a result of segment ending. 
			// so to get whether there's an actual failure, i'm using the internal state.
			if(!token.first) 
				continue;

			// skip comments
			if(IsComment(token.second))
				continue;

			// look for "begin". at this level that would be catching the "begin"
			// of the font dictionary
			if(token.second.compare("begin") == 0)
			{
				status = ReadFontDictionary();
				if(status != eSuccess)
					break;
			}

			// parsing private. note that while charstrings are not defined under private,
			// they are defined before its "end". hence parsing for them will take place at the
			// private dictionary parsing.
			if(token.second.compare("/Private") == 0)
			{
				status = ReadPrivateDictionary();
				if(status != eSuccess)
					break;
			}
			
		}

		/*
		charstring/subrs
		*/

	}while(false);

	mPFBDecoder.Assign(NULL);
	return status;
}

bool Type1Input::IsComment(const std::string& inToken)
{
	return inToken.at(0) == '%';
}

EStatusCode Type1Input::ReadFontDictionary()
{
	EStatusCode status = eSuccess;
	BoolAndString token;

	while(mPFBDecoder.NotEnded() && eSuccess == status)
	{
		token = mPFBDecoder.GetNextToken();
		status = mPFBDecoder.GetInternalState();
		if(!token.first) 
			continue;

		// skip comments
		if(IsComment(token.second))
			continue;

		// found end, done with dictionary
		if(token.second.compare("end") == 0)
			break;

		if(token.second.compare("/FontInfo") == 0)
		{
			status = ReadFontInfoDictionary();
			continue;
		}
		if(token.second.compare("/FontName") == 0)
		{
			mFontDictionary.FontName = FromPSName(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/PaintType") == 0)
		{
			mFontDictionary.PaintType = Int(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/FontType") == 0)
		{
			mFontDictionary.FontType = Int(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/FontMatrix") == 0)
		{
			status = ParseDoubleArray(mFontDictionary.FontMatrix,6);
			continue;
		}

		if(token.second.compare("/FontBBox") == 0)
		{
			status = ParseDoubleArray(mFontDictionary.FontBBox,4);
			continue;
		}

		if(token.second.compare("/UniqueID") == 0)
		{
			mFontDictionary.UniqueID = Int(mPFBDecoder.GetNextToken().second);
			continue;
		}

		if(token.second.compare("/StrokeWidth") == 0)
		{
			mFontDictionary.StrokeWidth = Double(mPFBDecoder.GetNextToken().second);
			continue;
		}

		if(token.second.compare("/Encoding") == 0)
		{
			status = ParseEncoding();
			if(eSuccess == status)
				CalculateReverseEncoding();
			continue;
		}

		if(token.second.compare("/FSType") == 0)
		{
			mFontInfoDictionary.fsType = (unsigned short)Int(mPFBDecoder.GetNextToken().second);
			mFontInfoDictionary.FSTypeValid = true;
		}
	}
	return status;
}

EStatusCode Type1Input::ReadFontInfoDictionary()
{
	EStatusCode status = eSuccess;
	BoolAndString token;

  // initialize some values to defaults
  mFontInfoDictionary.ItalicAngle = 0.0;
  mFontInfoDictionary.UnderlinePosition = 0.0;
  mFontInfoDictionary.UnderlineThickness = 0.0;
  
	while(mPFBDecoder.NotEnded() && eSuccess == status)
	{
		token = mPFBDecoder.GetNextToken();
		status = mPFBDecoder.GetInternalState();
		if(!token.first) 
			continue;

		// skip comments
		if(IsComment(token.second))
			continue;

		// "end" encountered, dictionary finished, return.
		if(token.second.compare("end") == 0) 
			break;

		if(token.second.compare("/version") == 0)
		{
			mFontInfoDictionary.version = FromPSString(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/Notice") == 0)
		{
			mFontInfoDictionary.Notice = FromPSString(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/Copyright") == 0)
		{
			mFontInfoDictionary.Copyright = FromPSString(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/FullName") == 0)
		{
			mFontInfoDictionary.FullName = FromPSString(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/FamilyName") == 0)
		{
			mFontInfoDictionary.FamilyName = FromPSString(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/Weight") == 0)
		{
			mFontInfoDictionary.Weight = FromPSString(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/ItalicAngle") == 0)
		{
			mFontInfoDictionary.ItalicAngle = 
				Double(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/isFixedPitch") == 0)
		{
			mFontInfoDictionary.isFixedPitch = 
				PSBool(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/UnderlinePosition") == 0)
		{
			mFontInfoDictionary.UnderlinePosition = 
				Double(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/UnderlineThickness") == 0)
		{
			mFontInfoDictionary.UnderlineThickness = 
				Double(mPFBDecoder.GetNextToken().second);
			continue;
		}

		if(token.second.compare("/FSType") == 0)
		{
			mFontInfoDictionary.fsType = (unsigned short)Int(mPFBDecoder.GetNextToken().second);
			mFontInfoDictionary.FSTypeValid = true;
		}
	}
	return status;	
}

std::string Type1Input::FromPSName(const std::string& inPostScriptName)
{
	return inPostScriptName.substr(1);
}

EStatusCode Type1Input::ParseDoubleArray(double* inArray,int inArraySize)
{
	EStatusCode status = eSuccess;

	// skip the [ or {
	BoolAndString token = mPFBDecoder.GetNextToken();
	if(!token.first)
		return eFailure;

	for(int i=0; i < inArraySize && eSuccess == status;++i)
	{
		token = mPFBDecoder.GetNextToken();
		status = token.first ? eSuccess:eFailure;
		inArray[i] = Double(token.second);
	}

	// skip the last ] or }
	token = mPFBDecoder.GetNextToken();
	if(!token.first)
		return eFailure;

	return status;

}

EStatusCode Type1Input::ParseEncoding()
{
	BoolAndString token = mPFBDecoder.GetNextToken();
	EStatusCode status = eSuccess;
	int encodingIndex = 0;

	if(!token.first)
		return eFailure;

	// checking for standard encoding
	if(token.second.compare("StandardEncoding") == 0)
	{
		mEncoding.EncodingType = eType1EncodingTypeStandardEncoding;

		// skip the def
		BoolAndString token = mPFBDecoder.GetNextToken();
		if(!token.first)
			return eFailure;
		return eSuccess;
	}

	// not standard encoding, parse custom encoding
	mEncoding.EncodingType = eType1EncodingTypeCustom;

	// hop hop...skip to the first dup
	while(token.first)
	{
		token = mPFBDecoder.GetNextToken();
		if(token.second.compare("dup") == 0)
			break;
	}
	if(!token.first)
		return eFailure;

	// k. now parse the repeats of "dup index charactername put"
	// till the first occurence of "readonly" or "def".
	while(token.first)
	{
		if(token.second.compare("readonly") == 0 || token.second.compare("def") == 0)
			break;

		// get the index from the next token
		token = mPFBDecoder.GetNextToken();
		if(!token.first)
			break;
		encodingIndex = Int(token.second);
		if(encodingIndex < 0 || encodingIndex > 255)
		{
			status = eFailure;
			break;
		}
		
		// get the glyph name
		token = mPFBDecoder.GetNextToken();
		if(!token.first)
			break;
		mEncoding.mCustomEncoding[encodingIndex] = FromPSName(token.second);

		// skip the put
		token = mPFBDecoder.GetNextToken();
		if(!token.first)
			break;

		// get next row first token [dup or end]
		token = mPFBDecoder.GetNextToken();
	}
	if(!token.first || status != eSuccess)
		return eFailure;

	return status;
}

bool Type1Input::IsCustomEncoding()
{
    return eType1EncodingTypeCustom == mEncoding.EncodingType;
}

void Type1Input::CalculateReverseEncoding()
{
	StringToByteMap::iterator it;

	mReverseEncoding.insert(StringToByteMap::value_type("",0));
	mReverseEncoding.insert(StringToByteMap::value_type(".notdef",0));
	if(eType1EncodingTypeCustom == mEncoding.EncodingType)
	{
		for(int i=0; i <256;++i)
		{
			it = mReverseEncoding.find(mEncoding.mCustomEncoding[i]);
			if(it == mReverseEncoding.end())
				mReverseEncoding.insert(StringToByteMap::value_type(mEncoding.mCustomEncoding[i],i));
			
		}
	}
	else
	{
		StandardEncoding standardEncoding;

		for(int i=0; i <256;++i)
		{
			it = mReverseEncoding.find(standardEncoding.GetEncodedGlyphName(i));
			if(it == mReverseEncoding.end())
				mReverseEncoding.insert(StringToByteMap::value_type(standardEncoding.GetEncodedGlyphName(i),i));
			
		}
	}
}

EStatusCode Type1Input::ReadPrivateDictionary()
{

	EStatusCode status = eSuccess;
    bool readCharString = false; // don't leave before you read CharStrings. so i'm having a little flag
	BoolAndString token;

	while(mPFBDecoder.NotEnded() && eSuccess == status)
	{
		token = mPFBDecoder.GetNextToken();
		status = mPFBDecoder.GetInternalState();
		if(!token.first) 
			continue;

		// skip comments
		if(IsComment(token.second))
			continue;

		// "end" encountered, dictionary finished, return.
		if(token.second.compare("end") == 0 && readCharString)
			break;

		if(token.second.compare("/UniqueID") == 0)
		{
			mPrivateDictionary.UniqueID = Int(mPFBDecoder.GetNextToken().second);
			continue;
		}

		if(token.second.compare("/BlueValues") == 0)
		{
			status = ParseIntVector(mPrivateDictionary.BlueValues);
			continue;
		}
		if(token.second.compare("/OtherBlues") == 0)
		{
			status = ParseIntVector(mPrivateDictionary.OtherBlues);
			continue;
		}
		if(token.second.compare("/FamilyBlues") == 0)
		{
			status = ParseIntVector(mPrivateDictionary.FamilyBlues);
			continue;
		}
		if(token.second.compare("/FamilyOtherBlues") == 0)
		{
			status = ParseIntVector(mPrivateDictionary.FamilyOtherBlues);
			continue;
		}
		if(token.second.compare("/BlueScale") == 0)
		{
			mPrivateDictionary.BlueScale = Double(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/BlueShift") == 0)
		{
			mPrivateDictionary.BlueShift = Int(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/BlueFuzz") == 0)
		{
			mPrivateDictionary.BlueFuzz = Int(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/StdHW") == 0)
		{
			mPFBDecoder.GetNextToken(); // skip [
			mPrivateDictionary.StdHW = Double(mPFBDecoder.GetNextToken().second);
			mPFBDecoder.GetNextToken(); // skip ]
			continue;
		}
		if(token.second.compare("/StdVW") == 0)
		{
			mPFBDecoder.GetNextToken(); // skip [
			mPrivateDictionary.StdVW = Double(mPFBDecoder.GetNextToken().second);
			mPFBDecoder.GetNextToken(); // skip ]
			continue;
		}
		if(token.second.compare("/StemSnapH") == 0)
		{
			status = ParseDoubleVector(mPrivateDictionary.StemSnapH);
			continue;
		}
		if(token.second.compare("/StemSnapV") == 0)
		{
			status = ParseDoubleVector(mPrivateDictionary.StemSnapV);
			continue;
		}
		if(token.second.compare("/ForceBold") == 0)
		{
			mPrivateDictionary.ForceBold = PSBool(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/LanguageGroup") == 0)
		{
			mPrivateDictionary.LanguageGroup = Int(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/lenIV") == 0)
		{
			mPrivateDictionary.lenIV = Int(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/RndStemUp") == 0)
		{
			mPrivateDictionary.RndStemUp = PSBool(mPFBDecoder.GetNextToken().second);
			continue;
		}
		if(token.second.compare("/Subrs") == 0)
		{
			status = ParseSubrs();
			continue;
		}

		if(token.second.compare("/CharStrings") == 0)
		{
			status = ParseCharstrings();
            readCharString = true;
			continue;

		}
	}
	return status;	
}

EStatusCode Type1Input::ParseIntVector(std::vector<int>& inVector)
{
	// skip the [ or {
	BoolAndString token = mPFBDecoder.GetNextToken();
	if(!token.first)
		return eFailure;


	while(token.first)
	{
		token = mPFBDecoder.GetNextToken();
		if(token.second.compare("]") == 0 || token.second.compare("}") == 0)
			break;

		inVector.push_back(Int(token.second));
	}
	return token.first ? eSuccess:eFailure;
}

EStatusCode Type1Input::ParseDoubleVector(std::vector<double>& inVector)
{
	// skip the [ or {
	BoolAndString token = mPFBDecoder.GetNextToken();
	if(!token.first)
		return eFailure;


	while(token.first)
	{
		token = mPFBDecoder.GetNextToken();
		if(token.second.compare("]") == 0 || token.second.compare("}") == 0)
			break;

		inVector.push_back(Double(token.second));
	}
	return token.first ? eSuccess:eFailure;
}

EStatusCode Type1Input::ParseSubrs()
{
	int subrIndex;

	// get the subrs count
	BoolAndString token = mPFBDecoder.GetNextToken();
	if(!token.first)
		return eFailure;

	mSubrsCount = Long(token.second);
    if(mSubrsCount == 0)
    {
        mSubrs = NULL;
        return eSuccess;
    }
    else
        mSubrs = new Type1CharString[mSubrsCount];

	// parse the subrs. they look like this: 	
	// dup index nbytes RD ~n~binary~bytes~ NP

	// skip till the first dup
	while(token.first)
	{
		token = mPFBDecoder.GetNextToken();
		if(token.second.compare("dup") == 0)
			break;
	}
	if(!token.first)
		return eFailure;

	for(long i=0;i<mSubrsCount && token.first;++i)
	{
		token = mPFBDecoder.GetNextToken();
		if(!token.first)
			break;
				
		subrIndex = Int(token.second);
		token = mPFBDecoder.GetNextToken();
		if(!token.first)
			break;

		mSubrs[subrIndex].CodeLength = Int(token.second);
		mSubrs[subrIndex].Code = new Byte[mSubrs[subrIndex].CodeLength];

		// skip the RD token (will also skip space)
		mPFBDecoder.GetNextToken();

		mPFBDecoder.Read(mSubrs[subrIndex].Code,mSubrs[subrIndex].CodeLength);

		// skip till next line or array end
		while ( token.first )
		{
			token = mPFBDecoder.GetNextToken();

			if ( 0 == token.second.compare("dup") )
				break;
			if ( 0 == token.second.compare("ND") )
				break;
			if ( 0 == token.second.compare("|-") ) // synonym for "ND"
				break;
			if ( 0 == token.second.compare("def") )
				break;
		}
	}
	if(!token.first)
		return eFailure;

	return mPFBDecoder.GetInternalState();
}

EStatusCode Type1Input::ParseCharstrings()
{
	BoolAndString token;
	std::string characterName;
	Type1CharString charString;

	// skip till "begin"
	while(mPFBDecoder.NotEnded())
	{
		token = mPFBDecoder.GetNextToken();
		if(!token.first || token.second.compare("begin") == 0)
			break;
	}
	if(!token.first)
		return eFailure;

	// Charstrings look like this:
	// charactername nbytes RD ~n~binary~bytes~ ND
	while(token.first && mPFBDecoder.GetInternalState() == eSuccess)
	{
		token = mPFBDecoder.GetNextToken();

		if("end" == token.second)
			break;

		characterName = FromPSName(token.second);

		charString.CodeLength = Int(mPFBDecoder.GetNextToken().second);

		charString.Code = new Byte[charString.CodeLength];

		// skip the RD token (will also skip space)
		mPFBDecoder.GetNextToken();


		mPFBDecoder.Read(charString.Code,charString.CodeLength);

		mCharStrings.insert(StringToType1CharStringMap::value_type(characterName,charString));

		// skip ND token
		mPFBDecoder.GetNextToken();
	}

	return mPFBDecoder.GetInternalState();
}

Type1CharString* Type1Input::GetGlyphCharString(Byte inCharStringIndex)
{
	std::string characterName;

	if(eType1EncodingTypeCustom == mEncoding.EncodingType)
	{
		if(mEncoding.mCustomEncoding[inCharStringIndex].size() == 0)
			characterName = ".notdef";
		else
			characterName = mEncoding.mCustomEncoding[inCharStringIndex];
			
	}
	else
	{
		StandardEncoding standardEncoding;
		
		characterName = standardEncoding.GetEncodedGlyphName(inCharStringIndex);
	}
	
	StringToType1CharStringMap::iterator it = mCharStrings.find(characterName);
	if(it == mCharStrings.end())
		return NULL;
	else
		return &(it->second);
}

Type1CharString* Type1Input::GetGlyphCharString(const std::string& inCharStringName)
{
	StringToType1CharStringMap::iterator it = mCharStrings.find(inCharStringName);
	if(it == mCharStrings.end())
		return NULL;
	else
		return &(it->second);
}


EStatusCode Type1Input::CalculateDependenciesForCharIndex(	Byte inCharStringIndex,
															CharString1Dependencies& ioDependenciesInfo)
{
	CharStringType1Interpreter interpreter;

	Type1CharString* charString = GetGlyphCharString(inCharStringIndex);
	if(!charString)
	{
		TRACE_LOG("Type1Input::CalculateDependenciesForCharIndex, Exception, cannot find glyph index");
		return eFailure;
	}

	mCurrentDependencies = &ioDependenciesInfo;
	EStatusCode status = interpreter.Intepret(*charString,this);
	mCurrentDependencies = NULL;
	return status;
}

EStatusCode Type1Input::CalculateDependenciesForCharIndex(const std::string& inCharStringName,
											  CharString1Dependencies& ioDependenciesInfo)
{
	CharStringType1Interpreter interpreter;
	StringToType1CharStringMap::iterator it = mCharStrings.find(inCharStringName);

	if(it == mCharStrings.end())
	{
		TRACE_LOG("Type1Input::CalculateDependenciesForCharIndex, Exception, cannot find glyph from name");
		return eFailure;
	}

	mCurrentDependencies = &ioDependenciesInfo;
	EStatusCode status = interpreter.Intepret(it->second,this);
	mCurrentDependencies = NULL;
	return status;
}


Type1CharString* Type1Input::GetSubr(long inSubrIndex)
{
	if(mCurrentDependencies)
		mCurrentDependencies->mSubrs.insert((unsigned short)inSubrIndex);

	if(inSubrIndex >= mSubrsCount)
	{
		TRACE_LOG2("CharStringType1Tracer::GetLocalSubr exception, asked for %ld and there are only %ld count subrs",inSubrIndex,mSubrsCount);
		return NULL;
	}
	else
		return mSubrs+inSubrIndex;
}

EStatusCode Type1Input::Type1Seac(const LongList& inOperandList)
{
	if(inOperandList.size() < 2) {
		TRACE_LOG1("Type1Input::Type1Seac exception, there should be 2 parameters provided for seac operation but only %d provided",inOperandList.size());
		return eFailure;		
	}

	LongList::const_reverse_iterator it = inOperandList.rbegin();

	mCurrentDependencies->mCharCodes.insert((Byte)*it);
	++it;
	mCurrentDependencies->mCharCodes.insert((Byte)*it);
	return eSuccess;
}

bool Type1Input::IsOtherSubrSupported(long inOtherSubrsIndex)
{
	mCurrentDependencies->mOtherSubrs.insert((unsigned short)inOtherSubrsIndex);
	return false;
}

unsigned long Type1Input::GetLenIV()
{
	return mPrivateDictionary.lenIV;
}

bool Type1Input::IsValidGlyphIndex(Byte inCharStringIndex)
{
	std::string characterName;

	if(eType1EncodingTypeCustom == mEncoding.EncodingType)
	{
		if(mEncoding.mCustomEncoding[inCharStringIndex].size() == 0)
			characterName = ".notdef";
		else
			characterName = mEncoding.mCustomEncoding[inCharStringIndex];
			
	}
	else
	{
		StandardEncoding standardEncoding;
		
		characterName = standardEncoding.GetEncodedGlyphName(inCharStringIndex);
	}
	
	return mCharStrings.find(characterName) != mCharStrings.end();
}

std::string Type1Input::GetGlyphCharStringName(Byte inCharStringIndex)
{
	if(eType1EncodingTypeCustom == mEncoding.EncodingType)
	{
		if(mEncoding.mCustomEncoding[inCharStringIndex].size() == 0)
			return ".notdef";
		else
			return mEncoding.mCustomEncoding[inCharStringIndex];
	}
	else
	{
		StandardEncoding standardEncoding;

		return standardEncoding.GetEncodedGlyphName(inCharStringIndex);
	}
}

std::string Type1Input::FromPSString(const std::string& inPSString)
{
	std::stringbuf stringBuffer;
	Byte buffer;
	std::string::const_iterator it = inPSString.begin();
	size_t i=1;
	++it; // skip first paranthesis
	
	for(; i < inPSString.size()-1;++it,++i)
	{
		if(*it == '\\')
		{
			++it;
			if('0' <= *it && *it <= '7')
			{
				buffer = (*it - '0') * 64;
				++it;
				buffer += (*it - '0') * 8;
				++it;
				buffer += (*it - '0');
			}
			else
			{
				switch(*it)
				{
					case 'n':
						buffer = '\n';
						break;
					case 'r':
						buffer = '\r';
						break;
					case 't':
						buffer = '\t';
						break;
					case 'b':
						buffer = '\b';
						break;
					case 'f':
						buffer = '\f';
						break;
					case '\\':
						buffer = '\\';
						break;
					case '(':
						buffer = '(';
						break;
					case ')':
						buffer = ')';
						break;
					default:
						// error!
						buffer = 0;
						break;
				}
			}
		}
		else
		{
			buffer = *it;
		}
		stringBuffer.sputn((const char*)&buffer,1);
	}
	return stringBuffer.str();
}

Byte Type1Input::GetEncoding(const std::string& inCharStringName)
{	
	StringToByteMap::iterator it = mReverseEncoding.find(inCharStringName);
	if(it == mReverseEncoding.end())
		return 0;
	else
		return it->second;
}