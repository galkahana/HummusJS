/*
   Source File : PDFParserTokenizer.h


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

#include "IOBasicTypes.h"
#include "EStatusCode.h"

#include <utility>
#include <string>



class IByteReader;

typedef std::pair<bool,std::string> BoolAndString;


class SimpleStringTokenizer
{
public:
	SimpleStringTokenizer(void);
	~SimpleStringTokenizer(void);


	void SetReadStream(IByteReader* inSourceStream);
	BoolAndString GetNextToken();
	void ResetReadState();
	void ResetReadState(const SimpleStringTokenizer& inExternalTokenizer);
	IOBasicTypes::LongFilePositionType GetRecentTokenPosition();
	IOBasicTypes::LongFilePositionType GetReadBufferSize();
private:

	IByteReader* mStream;
	bool mHasTokenBuffer;
	IOBasicTypes::Byte mTokenBuffer;
	IOBasicTypes::LongFilePositionType mStreamPositionTracker;
	IOBasicTypes::LongFilePositionType mRecentTokenPosition;


	void SkipTillToken();

	// failure in GetNextByteForToken actually marks a true read failure, if you checked end of file before calling it...
	PDFHummus::EStatusCode GetNextByteForToken(IOBasicTypes::Byte& outByte);

	bool IsPDFWhiteSpace(IOBasicTypes::Byte inCharacter);
	void SaveTokenBuffer(IOBasicTypes::Byte inToSave);
	bool IsPDFEntityBreaker(IOBasicTypes::Byte inCharacter);

};
