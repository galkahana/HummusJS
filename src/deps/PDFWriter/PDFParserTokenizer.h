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


class PDFParserTokenizer
{
public:
	PDFParserTokenizer(void);
	~PDFParserTokenizer(void);


	// Assign the stream to read from (does not take ownership of the stream)
	void SetReadStream(IByteReader* inSourceStream);

	// Get the next avialable PDF token. return result returns whether
	// token retreive was successful and the token. Token retrieval may be unsuccesful if
	// the end of file was reached and no token was recorded, or if read failure occured.
	// tokens may be:
	// 1. Comments
	// 2. Strings (hex or literal)
	// 3. Dictionary start and end markers
	// 4. Array start and end markers
	// 5. postscript (type 4) function start and end delimeters (curly brackets)
	// 6. Any othr entity separated from other by space or token delimeters (or eof)
	BoolAndString GetNextToken();

	// calls this when changing underlying stream position
	void ResetReadState();
	// cll this when wanting to reset to another tokenizer state (it's a copycon, essentially)
	void ResetReadState(const PDFParserTokenizer& inExternalTokenizer);

	// Advanced option!
	// This will return the position of the stream on the first byte of the recently provided token (use after GetNextToken).
	// the implementation counts bytes as it goes, as such any external shifting of the stream will render the method invalid.
	// In other words - use only if the only position movement is through GetNextToken repeated calls.
	// Specifically "ResetReadState" resets the count
	IOBasicTypes::LongFilePositionType GetRecentTokenPosition();

	// return the current buffer size. may be 1 or 0. if 1, means that the next char for tokenizing will be taken
	// from the buffer rather from the stream and only later the stream read will be resumed. 
	// if you are trying to determine the current position reading the stream, take this size into account (substracting from the current position)
	// to get the "virtual" position from the tokenizer point of view.
	IOBasicTypes::LongFilePositionType GetReadBufferSize();
private:

	IByteReader* mStream;
	bool mHasTokenBuffer;
	IOBasicTypes::Byte mTokenBuffer;
	IOBasicTypes::LongFilePositionType mStreamPositionTracker;
	IOBasicTypes::LongFilePositionType mRecentTokenPosition;


	void SkipTillToken();

	bool CanGetNextByte();
	// failure in GetNextByteForToken actually marks a true read failure, if you checked end of file before calling it...
	PDFHummus::EStatusCode GetNextByteForToken(IOBasicTypes::Byte& outByte);

	bool IsPDFWhiteSpace(IOBasicTypes::Byte inCharacter);
	void SaveTokenBuffer(IOBasicTypes::Byte inToSave);
	bool IsPDFEntityBreaker(IOBasicTypes::Byte inCharacter);

};
