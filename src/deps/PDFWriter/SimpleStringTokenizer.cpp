/*
   Source File : SimpleStringTokenizer.cpp


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
#include "SimpleStringTokenizer.h"
#include "IByteReader.h"
#include "OutputStringBufferStream.h"

using namespace PDFHummus;
using namespace IOBasicTypes;

SimpleStringTokenizer::SimpleStringTokenizer(void)
{
	mStream = NULL;
	ResetReadState();
}

SimpleStringTokenizer::~SimpleStringTokenizer(void)
{
}

void SimpleStringTokenizer::SetReadStream(IByteReader* inSourceStream)
{
	mStream = inSourceStream;
	ResetReadState();
}

void SimpleStringTokenizer::ResetReadState()
{
	mHasTokenBuffer = false;
	mStreamPositionTracker = 0;
	mRecentTokenPosition = 0;
}


void SimpleStringTokenizer::ResetReadState(const SimpleStringTokenizer& inExternalTokenizer)
{
	mTokenBuffer = inExternalTokenizer.mTokenBuffer;
	mHasTokenBuffer = inExternalTokenizer.mHasTokenBuffer;
	mStreamPositionTracker = inExternalTokenizer.mStreamPositionTracker;
	mRecentTokenPosition = inExternalTokenizer.mRecentTokenPosition;
}

//static const Byte scBackSlash[] = {'\\'};
static const std::string scStream = "stream";
//static const char scCR = '\r';
//static const char scLF = '\n';
BoolAndString SimpleStringTokenizer::GetNextToken()
{
	BoolAndString result;
	Byte buffer;
	OutputStringBufferStream tokenBuffer;
	
	if(!mStream || (!mStream->NotEnded() && !mHasTokenBuffer))
	{
		result.first = false;
		return result;
	}

	do
	{
		SkipTillToken();
		if(!mStream->NotEnded())
		{
			result.first = false;
			break;
		}

		// before reading the first byte save the token position, for external queries
		mRecentTokenPosition = mStreamPositionTracker;

		// get the first byte of the token
		if(GetNextByteForToken(buffer) != PDFHummus::eSuccess)
		{
			result.first = false;
			break;
		}
		tokenBuffer.Write(&buffer,1);

		result.first = true; // will only be changed to false in case of read error

		while(mStream->NotEnded())
		{
			if(GetNextByteForToken(buffer) != PDFHummus::eSuccess)
			{	
				result.first = false;
				break;
			}
			if(IsPDFWhiteSpace(buffer))
			{
				break;
			}
			else if(IsPDFEntityBreaker(buffer))
			{
				SaveTokenBuffer(buffer); // for a non-space breaker, save the token for next token read
				break;
			}
			else
				tokenBuffer.Write(&buffer,1);
		}
		result.second = tokenBuffer.ToString();
	}while(false);

	return result;
}

void SimpleStringTokenizer::SkipTillToken()
{
	Byte buffer = 0;

	if(!mStream)
		return;

	// skip till hitting first non space, or segment end
	while(mStream->NotEnded())
	{
		if(GetNextByteForToken(buffer) != PDFHummus::eSuccess)
			break;

		if(!IsPDFWhiteSpace(buffer))
		{
			SaveTokenBuffer(buffer);
			break;
		}
	}
}

EStatusCode SimpleStringTokenizer::GetNextByteForToken(Byte& outByte)
{
	++mStreamPositionTracker; // advance position tracker, because we are reading the next byte.
	if(mHasTokenBuffer)
	{
		outByte = mTokenBuffer;
		mHasTokenBuffer = false;
		return PDFHummus::eSuccess;
	}
	else
		return (mStream->Read(&outByte,1) != 1) ? PDFHummus::eFailure:PDFHummus::eSuccess;
}

static const Byte scWhiteSpaces[] = {0,0x9,0xA,0xC,0xD,0x20};
bool SimpleStringTokenizer::IsPDFWhiteSpace(Byte inCharacter)
{
	bool isWhiteSpace = false;
	for(int i=0; i < 6 && !isWhiteSpace; ++i)
		isWhiteSpace =  (scWhiteSpaces[i] == inCharacter);
	return isWhiteSpace;
}

void SimpleStringTokenizer::SaveTokenBuffer(Byte inToSave)
{
	mHasTokenBuffer = true;
	mTokenBuffer = inToSave;
	--mStreamPositionTracker; // decreasing position trakcer, because it is as if the byte is put back in the stream
}

IOBasicTypes::LongFilePositionType SimpleStringTokenizer::GetReadBufferSize()
{
	return mHasTokenBuffer ? 1 : 0;
}

static const Byte scEntityBreakers[] = {'(',')','<','>',']','[','{','}','/','%'};
bool SimpleStringTokenizer::IsPDFEntityBreaker(Byte inCharacter)
{
	bool isEntityBreak = false;
	for(int i=0; i < 10 && !isEntityBreak; ++i)
		isEntityBreak =  (scEntityBreakers[i] == inCharacter);
	return isEntityBreak;
}

LongFilePositionType SimpleStringTokenizer::GetRecentTokenPosition()
{
	return mRecentTokenPosition;
}