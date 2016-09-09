/*
   Source File : InputPFBDecodeStream.h


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

/*
	reads a PFB stream, decoding the eexec parts. A type 1 parser can use this stream to make the parsing
	more trivial, not requiring to keep track on which area one's in.
*/

#include "EStatusCode.h"
#include "IByteReader.h"

#include <utility>
#include <string>


using namespace IOBasicTypes;

class InputPFBDecodeStream;

typedef PDFHummus::EStatusCode (*DecodeMethod)(
	InputPFBDecodeStream* inThis,
	Byte& outByte 
 );

typedef std::pair<bool,std::string> BoolAndString;

class InputPFBDecodeStream : public IByteReader
{
public:
	InputPFBDecodeStream(void);
	~InputPFBDecodeStream(void);

	// Assign will set the stream to decode. it also takes ownership of the stream. if you
	// don't want the ownership make sure to Assign(NULL) when done using the decoder.
	PDFHummus::EStatusCode Assign(IByteReader* inStreamToDecode);
	
	// IByteReader implementation
	virtual LongBufferSizeType Read(Byte* inBuffer,LongBufferSizeType inBufferSize);
	virtual bool NotEnded();

	// token actions

	// get the next avialable postscript token. return result returns whether
	// token retreive was successful and the token (if it was).
	// note that segment end automatically cuts of a token
	BoolAndString GetNextToken();

	// skip white spaces till token or EOF. note that end of segment 
	// will stop tokenizer as well
	void SkipTillToken();

	PDFHummus::EStatusCode GetInternalState();

	// internal usage.
	PDFHummus::EStatusCode ReadDecodedByte(Byte& outByte);
	PDFHummus::EStatusCode ReadRegularByte(Byte& outByte);

private:
	IByteReader* mStreamToDecode;
	LongFilePositionType mInSegmentReadIndex;
	LongFilePositionType mSegmentSize;
	Byte mCurrentType;
	DecodeMethod mDecodeMethod;
	bool mHasTokenBuffer;
	Byte mTokenBuffer;
	unsigned short mRandomizer;
	bool mFoundEOF;
	
	// error flag. if set, will not allow further reading
	PDFHummus::EStatusCode mInternalState; 

	// Starts reading a stream segment, to receive the type and length of the segment
	PDFHummus::EStatusCode InitializeStreamSegment();
	void ResetReadStatus();
	PDFHummus::EStatusCode StoreSegmentLength();
	PDFHummus::EStatusCode FlushBinarySectionTrailingCode();
	bool IsPostScriptWhiteSpace(Byte inCharacter);
	bool IsSegmentNotEnded();
	void SaveTokenBuffer(Byte inToSave);
	bool IsPostScriptEntityBreaker(Byte inCharacter);
	PDFHummus::EStatusCode InitializeBinaryDecode();
	Byte DecodeByte(Byte inByteToDecode);

	PDFHummus::EStatusCode GetNextByteForToken(Byte& outByte);

};
