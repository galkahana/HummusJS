/*
   Source File : PDFObjectParser.cpp


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
#include "PDFObjectParser.h"
#include "PDFObject.h"
#include "PDFBoolean.h"
#include "PDFLiteralString.h"
#include "PDFHexString.h"
#include "PDFNull.h"
#include "PDFName.h"
#include "PDFReal.h"
#include "PDFInteger.h"
#include "PDFArray.h"
#include "PDFDictionary.h"
#include "PDFIndirectObjectReference.h"
#include "PDFSymbol.h"
#include "PDFStreamInput.h"
#include "Trace.h"
#include "BoxingBase.h"
#include "PDFStream.h"
#include "IByteReader.h"
#include "RefCountPtr.h"
#include "PDFObjectCast.h"
#include "IPDFParserExtender.h"
#include "DecryptionHelper.h"

#include <sstream>

using namespace PDFHummus;

#define MAX_OBJECT_DEPTH 100 // While PDF does not explicitly define arrays and dicts depth...we do, due to call stack depth limit...and to avoid potential malarky.


PDFObjectParser::PDFObjectParser(void)
{
	mParserExtender = NULL;
	mDecryptionHelper = NULL;
	mOwnsStream = false;
	mStream = NULL;
	mDepth = 0;
}

PDFObjectParser::~PDFObjectParser(void)
{
	if(mOwnsStream)
		delete mStream;
}

void PDFObjectParser::SetReadStream(IByteReader* inSourceStream,
									IReadPositionProvider* inCurrentPositionProvider,
									bool inOwnsStream)
{
	if(mOwnsStream) {
		delete mStream;
	}

	mStream = inSourceStream;
	mOwnsStream = inOwnsStream;
	mTokenizer.SetReadStream(inSourceStream);
	mCurrentPositionProvider = inCurrentPositionProvider;
	ResetReadState();
}

void PDFObjectParser::ResetReadState()
{
	mTokenBuffer.clear();
	mTokenizer.ResetReadState();
	mDepth = 0;
}

void PDFObjectParser::ResetReadState(const PDFParserTokenizer& inExternalTokenizer)
{
	mTokenBuffer.clear();
	mTokenizer.ResetReadState(inExternalTokenizer);
}

static const std::string scR = "R";
static const std::string scStream = "stream";
PDFObject* PDFObjectParser::ParseNewObject()
{
	PDFObject* pdfObject = NULL;
	std::string token;

	do
	{
		if(!GetNextToken(token))
			break;

		// based on the parsed token, and perhaps some more, determine the type of object
		// and how to parse it.

		// Boolean
		if(IsBoolean(token))
		{
			pdfObject = ParseBoolean(token);
			break;
		} 
		// Literal String
		else if(IsLiteralString(token))
		{
			pdfObject = ParseLiteralString(token);
			break;
		}
		// Hexadecimal String
		else if(IsHexadecimalString(token))
		{
			pdfObject = ParseHexadecimalString(token);
			break;
		}
		// NULL
		else if (IsNull(token))
		{
			pdfObject = new PDFNull();
			break;
		}
		// Name
		else if(IsName(token))
		{
			pdfObject = ParseName(token);
			break;
		}
		// Number (and possibly an indirect reference)
		else if(IsNumber(token))
		{	
			pdfObject = ParseNumber(token);
			
			// this could be an indirect reference in case this is a positive integer
			// and the next one is also, and then there's an "R" keyword
			if(pdfObject && 
				(pdfObject->GetType() == PDFObject::ePDFObjectInteger) && 
				((PDFInteger*)pdfObject)->GetValue() > 0)
			{
				// try parse version
				std::string numberToken;
				if(!GetNextToken(numberToken)) // k. no next token...cant be reference
					break;

				if(!IsNumber(numberToken)) // k. no number, cant be reference
				{
					SaveTokenToBuffer(numberToken);
					break;
				}

				PDFObject* versionObject = ParseNumber(numberToken); 
				bool isReference = false;
				do
				{
					if(!versionObject || 
                       (versionObject->GetType() != PDFObject::ePDFObjectInteger) ||
						((PDFInteger*)versionObject)->GetValue() < 0) // k. failure to parse number, or no non-negative, cant be reference
					{
						SaveTokenToBuffer(numberToken);
						break;
					}

					// try parse R keyword
					std::string keywordToken;
					if(!GetNextToken(keywordToken)) // k. no next token...cant be reference
						break;

					if(keywordToken != scR) // k. not R...cant be reference
					{
						SaveTokenToBuffer(numberToken);
						SaveTokenToBuffer(keywordToken);
						break;
					}

					isReference = true;
				}while(false);

				// if passed all these, then this is a reference
				if(isReference)
				{
					PDFObject* referenceObject = new PDFIndirectObjectReference(
														(ObjectIDType)((PDFInteger*)pdfObject)->GetValue(),
														(unsigned long)((PDFInteger*)versionObject)->GetValue());
					delete pdfObject;
					pdfObject = referenceObject;
				}
				delete versionObject;
			}
			break;
		}
		// Array
		else if(IsArray(token))
		{
			pdfObject = ParseArray();
			break;
		}
		// Dictionary
		else if (IsDictionary(token))
		{
			pdfObject = ParseDictionary();

			if(pdfObject)
			{
				// could be a stream. will be if the next token is the "stream" keyword
				if(!GetNextToken(token))
					break;

				if(scStream == token) 
				{
					// yes, found a stream. record current position as the position where the stream starts. 
					// remove from the current stream position the size of the tokenizer buffer, which is "read", but not used
					pdfObject = new PDFStreamInput((PDFDictionary*)pdfObject, mCurrentPositionProvider->GetCurrentPosition() - mTokenizer.GetReadBufferSize());
				}
				else
				{
					SaveTokenToBuffer(token);
				}
			}

			break;
		}
		// Symbol (legitimate keyword or error. determine if error based on semantics)
		else
			pdfObject = new PDFSymbol(token);
	}while(false);


	return pdfObject;

}

bool PDFObjectParser::GetNextToken(std::string& outToken)
{
	if(mTokenBuffer.size() > 0)
	{
		outToken = mTokenBuffer.front();
		mTokenBuffer.pop_front();
		return true;
	}
	else
	{
		// skip comments
		BoolAndString tokenizerResult;

		do
		{
			tokenizerResult = mTokenizer.GetNextToken();
			if(tokenizerResult.first && !IsComment(tokenizerResult.second))
			{
				outToken = tokenizerResult.second;
				break;
			}
		}while(tokenizerResult.first);
		return tokenizerResult.first;
	}
}

static const std::string scTrue = "true";
static const std::string scFalse = "false";
bool PDFObjectParser::IsBoolean(const std::string& inToken)
{
	return (scTrue == inToken || scFalse == inToken);	
}

PDFObject* PDFObjectParser::ParseBoolean(const std::string& inToken)
{
	return new PDFBoolean(scTrue == inToken);
}

static const char scLeftParanthesis = '(';
bool PDFObjectParser::IsLiteralString(const std::string& inToken)
{
	return inToken.size() > 0 && inToken.at(0) == scLeftParanthesis;
}

static const char scRightParanthesis = ')';
PDFObject* PDFObjectParser::ParseLiteralString(const std::string& inToken)
{
  std::stringbuf stringBuffer;
	Byte buffer;
	std::string::const_iterator it = inToken.begin();
	size_t i=1;
	++it; // skip first paranthesis
	
	// verify that last character is ')'
	if(inToken.at(inToken.size()-1) != scRightParanthesis)
	{
		TRACE_LOG1("PDFObjectParser::ParseLiteralString, exception in parsing literal string, no closing paranthesis, Expression: %s",inToken.substr(0, MAX_TRACE_SIZE - 200).c_str());
		return NULL;
	}

	for(; i < inToken.size()-1;++it,++i)
	{
		if(*it == '\\')
		{
			++it;
			++i;
			if('0' <= *it && *it <= '7')
			{
				buffer = (*it - '0');
				if (i+1 < inToken.size() && '0' <= *(it+1) && *(it+1) <= '7'){
					++it;
					++i;
					buffer = buffer << 3;
					buffer += (*it - '0');
					if (i + 1 < inToken.size() && '0' <= *(it + 1) && *(it + 1) <= '7'){
						++it;
						++i;
						buffer = buffer << 3;
						buffer += (*it - '0');
					}
				}
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

	return new PDFLiteralString(MaybeDecryptString(stringBuffer.str()));
}

/*


*/

std::string PDFObjectParser::MaybeDecryptString(const std::string& inString) {
	if (mDecryptionHelper && mDecryptionHelper->IsEncrypted()) {

		if (mDecryptionHelper->CanDecryptDocument())
			return mDecryptionHelper->DecryptString(inString);
		else {
			if (mParserExtender)
				return mParserExtender->DecryptString(inString);
			else
				return inString;
		}
	}
	else {
		return inString;
	}
}


static const char scLeftAngle = '<';
bool PDFObjectParser::IsHexadecimalString(const std::string& inToken)
{
	// first char should be left angle brackets, and the one next must not (otherwise it's a dictionary start)
	return inToken.size() > 0 && (inToken.at(0) == scLeftAngle) && (inToken.size() < 2 || inToken.at(1) != scLeftAngle);
}


static const char scRightAngle = '>';
PDFObject* PDFObjectParser::ParseHexadecimalString(const std::string& inToken)
{
	// verify that last character is '>'
	if(inToken.at(inToken.size()-1) != scRightAngle)
	{
		TRACE_LOG1("PDFObjectParser::ParseHexadecimalString, exception in parsing hexadecimal string, no closing angle, Expression: %s",inToken.substr(0, MAX_TRACE_SIZE - 200).c_str());
		return NULL;
	}

	return new PDFHexString(MaybeDecryptString(DecodeHexString(inToken.substr(1, inToken.size() - 2))));
}

std::string PDFObjectParser::DecodeHexString(const std::string inStringToDecode) {
	std::stringbuf stringBuffer;
	std::string content = inStringToDecode;


	std::string::const_iterator it = content.begin();
	BoolAndByte buffer(false, 0); // bool part = 'is first char in buffer?', Byte part = 'the first hex-decoded char'
	for (; it != content.end(); ++it)
	{
		BoolAndByte parse = GetHexValue(*it);
		if (parse.first){
			if (buffer.first){
				Byte hexbyte = (buffer.second << 4) | parse.second;
				buffer.first = false;
				stringBuffer.sputn((const char*)&hexbyte, 1);
			}
			else{
				buffer.first = true;
				buffer.second = parse.second;
			}
		}
	}

	// pad with ending 0
	if (buffer.first){
		Byte hexbyte = buffer.second << 4;
		stringBuffer.sputn((const char*)&hexbyte, 1);
	}

	// decode utf16 here?
	// Gal: absolutely not! this is plain hex decode. doesn't necesserily mean text. in fact most of the times it doesn't. keep this low level

	return stringBuffer.str();

}

static const std::string scNull = "null";
bool PDFObjectParser::IsNull(const std::string& inToken)
{
	return scNull == inToken;
}

static const char scSlash = '/';
bool PDFObjectParser::IsName(const std::string& inToken)
{
	return inToken.size() > 0 && inToken.at(0) == scSlash;
}

static const char scSharp = '#';
PDFObject* PDFObjectParser::ParseName(const std::string& inToken)
{
	EStatusCode status = PDFHummus::eSuccess;
	std::stringbuf stringBuffer;
	BoolAndByte hexResult;
	Byte buffer;
	std::string::const_iterator it = inToken.begin();
	++it; // skip initial slash

	for(; it != inToken.end() && PDFHummus::eSuccess == status; ++it)
	{
		if(*it == scSharp)
		{
			// hex representation
			++it;
			if(it == inToken.end())
			{
				TRACE_LOG1("PDFObjectParser::ParseName, exception in parsing hex value for a name token. token = %s",inToken.substr(0, MAX_TRACE_SIZE - 200).c_str());
				status = PDFHummus::eFailure;
				break;
			}
			hexResult = GetHexValue(*it);
			if(!hexResult.first)
			{
				TRACE_LOG1("PDFObjectParser::ParseName, exception in parsing hex value for a name token. token = %s",inToken.substr(0, MAX_TRACE_SIZE - 200).c_str());
				status = PDFHummus::eFailure;
				break;
			}
			buffer=(hexResult.second << 4);
			++it;
			if(it == inToken.end())
			{
				TRACE_LOG1("PDFObjectParser::ParseName, exception in parsing hex value for a name token. token = %s",inToken.substr(0, MAX_TRACE_SIZE - 200).c_str());
				status = PDFHummus::eFailure;
				break;
			}
			hexResult = GetHexValue(*it);
			if(!hexResult.first)
			{
				TRACE_LOG1("PDFObjectParser::ParseName, exception in parsing hex value for a name token. token = %s",inToken.substr(0, MAX_TRACE_SIZE - 200).c_str());
				status = PDFHummus::eFailure;
				break;
			}
			buffer+=hexResult.second;
		}
		else
		{
			buffer = *it;
		}
		stringBuffer.sputn((const char*)&buffer,1);
	}
	
	if(PDFHummus::eSuccess == status)
		return new PDFName(stringBuffer.str());
	else
		return NULL;
}

static const char scPlus = '+';
static const char scMinus = '-';
static const char scNine = '9';
static const char scZero = '0';
static const char scDot = '.';
bool PDFObjectParser::IsNumber(const std::string& inToken)
{
	if(inToken.size() < 1)
		return false;

	// it's a number if the first char is either a sign or digit, or an initial decimal dot, and the rest is 
	// digits, with the exception of a dot which can appear just once.
	if(inToken.at(0) != scPlus && inToken.at(0) != scMinus && inToken.at(0) != scDot && (inToken.at(0) > scNine || inToken.at(0) < scZero))
		return false;

	bool isNumber = true;
	bool dotEncountered = (inToken.at(0) == scDot);
	std::string::const_iterator it = inToken.begin();
	++it; //verified the first char already

	// only sign is not a number
	if((inToken.at(0) == scPlus || inToken.at(0) == scMinus) && it == inToken.end())
		return false;

	for(; it != inToken.end() && isNumber;++it)
	{
		if(*it == scDot)
		{
			if(dotEncountered)
			{
				isNumber = false;
			}
			dotEncountered = true;
		}
		else
			isNumber = (scZero <= *it && *it <= scNine);
	}
	return isNumber;
}

typedef BoxingBaseWithRW<long long> LongLong;

// maximum allowed PDF int value = 2^31 − 1.
#define MAX_PDF_INT 2147483647L 
// minimum allowed PDF int value = -2^31
#define MIN_PDF_INT ((-MAX_PDF_INT)-1) 

PDFObject* PDFObjectParser::ParseNumber(const std::string& inToken)
{
	// once we know this is a number, then parsing is easy. just determine if it's a real or integer, so as to separate classes for better accuracy
	if(inToken.find(scDot) != inToken.npos) {
		return new PDFReal(Double(inToken));
	} else {
		long long integerValue = LongLong(inToken);

		// validate int value according to PDF limits. ignore if outside of range
		if((integerValue > MAX_PDF_INT) || (integerValue < MIN_PDF_INT)) {
			TRACE_LOG3("PDFObjectParser::ParseNumber, parsed integer %lld is outside of the allowed range for PDF integers - %ld to %ld", integerValue, MIN_PDF_INT, MAX_PDF_INT);
			return NULL;
		}

		return new PDFInteger(integerValue);
	}
}

static const std::string scLeftSquare = "[";
bool PDFObjectParser::IsArray(const std::string& inToken)
{
	return scLeftSquare == inToken;
}

EStatusCode PDFObjectParser::IncreaseAndCheckDepth() {
	++mDepth;
	if(mDepth > MAX_OBJECT_DEPTH) {
		TRACE_LOG1("PDFObjectParser::IncreaseAndCeckDepth, reached maximum allowed depth of %d", MAX_OBJECT_DEPTH);
		return eFailure;
	}

	return eSuccess;
}

EStatusCode PDFObjectParser::DecreaseAndCheckDepth() {
	--mDepth;
	if(mDepth < 0) {
		TRACE_LOG("PDFObjectParser::DecreaseAndCheckDepth, anomaly. managed to get to negative depth");
		return eFailure;
	}

	return eSuccess;
}


static const std::string scRightSquare = "]";
PDFObject* PDFObjectParser::ParseArray()
{
	PDFArray* anArray;
	bool arrayEndEncountered = false;
	std::string token;
	EStatusCode status = PDFHummus::eSuccess;

	if(IncreaseAndCheckDepth() != eSuccess)
		return NULL;

	anArray = new PDFArray();

	// easy one. just loop till you get to a closing bracket token and recurse
	while(GetNextToken(token) && PDFHummus::eSuccess == status)
	{
		arrayEndEncountered = (scRightSquare == token);
		if(arrayEndEncountered)
			break;

		ReturnTokenToBuffer(token);
		RefCountPtr<PDFObject> anObject(ParseNewObject());
		if(!anObject)
		{
			status = PDFHummus::eFailure;
			TRACE_LOG1("PDFObjectParser::ParseArray, failure to parse array, failed to parse a member object. token = %s",token.substr(0, MAX_TRACE_SIZE - 200).c_str());
		}
		else
		{
			anArray->AppendObject(anObject.GetPtr());
		}
	}

	if(DecreaseAndCheckDepth() != eSuccess)
		status = eFailure;

	if(arrayEndEncountered && PDFHummus::eSuccess == status)
	{
		return anArray;
	}
	else
	{
		delete anArray;
		TRACE_LOG1("PDFObjectParser::ParseArray, failure to parse array, didn't find end of array or failure to parse array member object. token = %s",token.substr(0, MAX_TRACE_SIZE - 200).c_str());
		return NULL;
	}
}

void PDFObjectParser::SaveTokenToBuffer(std::string& inToken)
{
	mTokenBuffer.push_back(inToken);
}

void PDFObjectParser::ReturnTokenToBuffer(std::string& inToken)
{
	mTokenBuffer.push_front(inToken);
}

static const std::string scDoubleLeftAngle = "<<";
bool PDFObjectParser::IsDictionary(const std::string& inToken)
{
	return scDoubleLeftAngle == inToken;
}

static const std::string scDoubleRightAngle = ">>";
PDFObject* PDFObjectParser::ParseDictionary()
{
	PDFDictionary* aDictionary;
	bool dictionaryEndEncountered = false;
	std::string token;
	EStatusCode status = PDFHummus::eSuccess;

	if(IncreaseAndCheckDepth() != eSuccess)
		return NULL;

	aDictionary = new PDFDictionary();

	while(GetNextToken(token) && PDFHummus::eSuccess == status)
	{
		dictionaryEndEncountered = (scDoubleRightAngle == token);
		if(dictionaryEndEncountered)
			break;

		ReturnTokenToBuffer(token);

		// Parse Key
		PDFObjectCastPtr<PDFName> aKey(ParseNewObject());
		if(!aKey)
		{
			status = PDFHummus::eFailure;
			TRACE_LOG1("PDFObjectParser::ParseDictionary, failure to parse key for a dictionary. token = %s",token.substr(0, MAX_TRACE_SIZE - 200).c_str());
			break;
		}


		// Parse Value
		RefCountPtr<PDFObject> aValue = ParseNewObject();
		if(!aValue)
		{
			status = PDFHummus::eFailure;
			TRACE_LOG1("PDFObjectParser::ParseDictionary, failure to parse value for a dictionary. token = %s",token.substr(0, MAX_TRACE_SIZE - 200).c_str());
			break;
		}
	

		// all good. i'm gonna be forgiving here and allow skipping duplicate keys. cause it happens
		if(!aDictionary->Exists(aKey->GetValue()))
			aDictionary->Insert(aKey.GetPtr(),aValue.GetPtr());
	}

	if(DecreaseAndCheckDepth() != eSuccess)
		status = eFailure;
		
	if(dictionaryEndEncountered && PDFHummus::eSuccess == status)
	{
		return aDictionary;
	}
	else
	{
		delete aDictionary;
		TRACE_LOG1("PDFObjectParser::ParseDictionary, failure to parse dictionary, didn't find end of array or failure to parse dictionary member object. token = %s",token.substr(0, MAX_TRACE_SIZE - 200).c_str());
		return NULL;
	}
}

static const char scCommentStart = '%';
bool PDFObjectParser::IsComment(const std::string& inToken)
{
	return inToken.size() > 0 && inToken.at(0) == scCommentStart;
}

BoolAndByte PDFObjectParser::GetHexValue(Byte inValue)
{
	if('0' <= inValue && inValue <='9')
		return BoolAndByte(true,inValue - '0');
	else if('A' <= inValue && inValue <= 'F')
		return BoolAndByte(true,inValue - 'A' + 10);
	else if('a' <= inValue && inValue <= 'f')
		return BoolAndByte(true,inValue - 'a' + 10);
	else
	{
		if (!isspace(inValue)){
			TRACE_LOG1("PDFObjectParser::GetHexValue, unrecongnized hex value - %c", inValue);
		}
		return BoolAndByte(false, inValue);
	}
}

void PDFObjectParser::SetDecryptionHelper(DecryptionHelper* inDecryptionHelper) {
	mDecryptionHelper = inDecryptionHelper;
}

void PDFObjectParser::SetParserExtender(IPDFParserExtender* inParserExtender)
{
	mParserExtender = inParserExtender;
}

IByteReader* PDFObjectParser::StartExternalRead() {
	return mStream;
}

void PDFObjectParser::EndExternalRead() {
	ResetReadState();
}