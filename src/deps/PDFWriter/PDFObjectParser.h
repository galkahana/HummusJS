/*
   Source File : PDFObjectParser.h


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

#include "PDFParserTokenizer.h"
#include "EStatusCode.h"
#include "IOBasicTypes.h"
#include "IReadPositionProvider.h"

#include <list>
#include <string>
#include <utility>

class PDFObject;
class IByteReader;
class IPDFParserExtender;



typedef std::pair<bool,IOBasicTypes::Byte> BoolAndByte;
typedef std::list<std::string> StringList;

class PDFObjectParser
{
public:
	PDFObjectParser(void);
	~PDFObjectParser(void);

	
	// Assign the stream to read from (does not take ownership of the stream)
	void SetReadStream(IByteReader* inSourceStream,IReadPositionProvider* inCurrentPositionProvider);

	PDFObject* ParseNewObject(IPDFParserExtender* inParserExtender);

	// calls this when changing underlying stream position
	void ResetReadState();

private:
	PDFParserTokenizer mTokenizer;
	StringList mTokenBuffer;
	IByteReader* mStream;
	IReadPositionProvider* mCurrentPositionProvider;

	bool GetNextToken(std::string& outToken);
	void SaveTokenToBuffer(std::string& inToken);
	void ReturnTokenToBuffer(std::string& inToken);

	bool IsBoolean(const std::string& inToken);
	PDFObject* ParseBoolean(const std::string& inToken);

	bool IsLiteralString(const std::string& inToken);
	PDFObject* ParseLiteralString(const std::string& inToken,IPDFParserExtender* inParserExtender);

	bool IsHexadecimalString(const std::string& inToken);
	PDFObject* ParseHexadecimalString(const std::string& inToken,IPDFParserExtender* inParserExtender);

	bool IsNull(const std::string& inToken);

	bool IsName(const std::string& inToken);
	PDFObject* ParseName(const std::string& inToken);

	bool IsNumber(const std::string& inToken);
	PDFObject* ParseNumber(const std::string& inToken);

	bool IsArray(const std::string& inToken);
	PDFObject* ParseArray(IPDFParserExtender* inParserExtender);

	bool IsDictionary(const std::string& inToken);
	PDFObject* ParseDictionary(IPDFParserExtender* inParserExtender);

	bool IsComment(const std::string& inToken);

	BoolAndByte GetHexValue(IOBasicTypes::Byte inValue);


};
