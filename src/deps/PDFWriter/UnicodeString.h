/*
   Source File : UnicodeString.h


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

#include <string>
#include <list>



typedef std::pair<PDFHummus::EStatusCode,std::string> EStatusCodeAndString;
typedef std::list<unsigned long> ULongList;
typedef std::list<unsigned short> UShortList;
typedef std::pair<PDFHummus::EStatusCode,UShortList> EStatusCodeAndUShortList;

class UnicodeString
{
public:
	UnicodeString(void);
	UnicodeString(const UnicodeString& inOtherString);
	UnicodeString(const ULongList& inOtherList);
	~UnicodeString(void);

	UnicodeString& operator =(const UnicodeString& inOtherString);
	UnicodeString& operator =(const ULongList& inOtherList);

	bool operator==(const UnicodeString& inOtherString) const;

	PDFHummus::EStatusCode FromUTF8(const std::string& inString);
	EStatusCodeAndString ToUTF8() const;

	// convert from UTF16 string, requires BOM
	PDFHummus::EStatusCode FromUTF16(const std::string& inString);
	PDFHummus::EStatusCode FromUTF16(const unsigned char* inString, unsigned long inLength);

	// convert from UTF16BE, do not include BOM
	PDFHummus::EStatusCode FromUTF16BE(const std::string& inString);
	PDFHummus::EStatusCode FromUTF16BE(const unsigned char* inString, unsigned long inLength);

	// convert from UTF16LE do not include BOM
	PDFHummus::EStatusCode FromUTF16LE(const std::string& inString);
	PDFHummus::EStatusCode FromUTF16LE(const unsigned char* inString, unsigned long inLength);

	// convert from unsigned shorts, does not require BOM, assuming that byte ordering is according to OS
	PDFHummus::EStatusCode FromUTF16UShort(const unsigned short* inShorts, unsigned long inLength);

	// convert to UTF16 BE
	EStatusCodeAndString ToUTF16BE(bool inPrependWithBom) const;
	
	// convert to UTF16 LE
	EStatusCodeAndString ToUTF16LE(bool inPrependWithBom) const;

	// covnert to unsigned shorts. byte ordering according to OS. not placing BOM
	EStatusCodeAndUShortList ToUTF16UShort() const;

	const ULongList& GetUnicodeList() const;
	ULongList& GetUnicodeList();
private:
	ULongList mUnicodeCharacters;
};
