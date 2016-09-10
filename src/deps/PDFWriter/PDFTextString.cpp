/*
   Source File : PDFTextString.cpp


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
#include "PDFTextString.h"
#include "OutputStringBufferStream.h"
#include "PDFDocEncoding.h"
#include "UnicodeString.h"
#include <sstream>

using namespace IOBasicTypes;

PDFTextString::PDFTextString(void)
{
}

PDFTextString::PDFTextString(const std::string& inString)
{
	mTextString = inString;
}

PDFTextString::~PDFTextString(void)
{
}

const PDFTextString& PDFTextString::Empty(){
	static PDFTextString pdf_text_string_empty;
	return pdf_text_string_empty;
};

PDFTextString& PDFTextString::FromUTF8(const std::string& inString)
{
	ConvertFromUTF8(inString);
	return *this;
}

bool PDFTextString::IsEmpty() const
{
	return mTextString.size() == 0;
}

const std::string& PDFTextString::ToString() const
{
	return mTextString;
}

bool PDFTextString::operator==(const PDFTextString& inString) const
{
	return mTextString == inString.mTextString;
}

PDFTextString& PDFTextString::operator=(const PDFTextString& inString)
{
	mTextString = inString.mTextString;
	return *this;
}

void PDFTextString::ConvertFromUTF8(const std::string& inStringToConvert)
{
	OutputStringBufferStream aStringStream;

	if(!ConvertUTF8ToPDFDocEncoding(inStringToConvert,aStringStream))
	{
		// if no DOC encoding then convert to utf16BE (with BOM)
		
		UnicodeString unicodeString;
		unicodeString.FromUTF8(inStringToConvert);
		EStatusCodeAndString result = unicodeString.ToUTF16BE(true);
		
		mTextString = result.second;

	}
	else
	{
		mTextString = aStringStream.ToString();
	}
}

bool PDFTextString::ConvertUTF8ToPDFDocEncoding(const std::string& inStringToConvert,OutputStringBufferStream& refResult)
{
	BoolAndByte encodingResult;
	PDFDocEncoding pdfDocEncoding;
	bool PDFEncodingOK = true;

	UnicodeString unicodeString;
	
	unicodeString.FromUTF8(inStringToConvert);

	ULongList::const_iterator it = unicodeString.GetUnicodeList().begin();
	for(;it != unicodeString.GetUnicodeList().end() && PDFEncodingOK;++it)
	{
		encodingResult = pdfDocEncoding.Encode(*it);
		if(encodingResult.first)
			refResult.Write(&(encodingResult.second),1);
		else
			PDFEncodingOK = false;
	}

	return PDFEncodingOK;
}

PDFTextString& PDFTextString::operator=(const std::string& inString)
{
	mTextString = inString;
	return *this;
}

std::string PDFTextString::ToUTF8String() const
{
	if(mTextString.size() >= 2 && (Byte)mTextString.at(0) == 0xFE && (Byte)mTextString.at(1) == 0xFF)
		return ToUTF8FromUTF16BE();
	else
		return ToUTF8FromPDFDocEncoding();
}

std::string PDFTextString::ToUTF8FromUTF16BE() const
{
	UnicodeString unicodeString;

	unicodeString.FromUTF16(mTextString);

	EStatusCodeAndString result = unicodeString.ToUTF8();
	return result.second;
}


std::string PDFTextString::ToUTF8FromPDFDocEncoding() const
{
	ULongList unicodes;
	UnicodeString decoder;
	PDFDocEncoding pdfDocEncoding;

	std::string::const_iterator it = mTextString.begin();

	for(; it != mTextString.end();++it)
		unicodes.push_back(pdfDocEncoding.Decode(*it));

	decoder = unicodes;

	return decoder.ToUTF8().second;
}