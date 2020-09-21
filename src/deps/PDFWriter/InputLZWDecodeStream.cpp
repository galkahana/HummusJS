/*
   Source File : InputLZWDecodeStream.cpp


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
#include "InputLZWDecodeStream.h"

#include "Trace.h"
#include "zlib.h"


InputLZWDecodeStream::InputLZWDecodeStream(int early)
{
	mSourceStream = NULL;
	mCurrentlyEncoding = false;
	mEarly = early;
	inputBuf = 0;
}

InputLZWDecodeStream::~InputLZWDecodeStream(void)
{
	if(mCurrentlyEncoding)
		FinalizeEncoding();
	if(mSourceStream)
		delete mSourceStream;	
}

void InputLZWDecodeStream::FinalizeEncoding()
{	
	mCurrentlyEncoding = false;
}

InputLZWDecodeStream::InputLZWDecodeStream(IByteReader* inSourceReader)
{	
	mSourceStream = NULL;
	mCurrentlyEncoding = false;

	Assign(inSourceReader);
}

void InputLZWDecodeStream::Assign(IByteReader* inSourceReader)
{
	mSourceStream = inSourceReader;
	if(mSourceStream)
		StartEncoding();
}

void InputLZWDecodeStream::StartEncoding()
{	
    mCurrentlyEncoding = true;

	inputBits = 0;
	ClearTable();
}

IOBasicTypes::LongBufferSizeType InputLZWDecodeStream::Read(IOBasicTypes::Byte* inBuffer, IOBasicTypes::LongBufferSizeType inBufferSize)
{
	if (mCurrentlyEncoding)
	{
		if (seqIndex >= seqLength)
		{
			if (!ProcessNextCode())
				return 0;
		}
		memcpy(inBuffer, &seqBuf[seqIndex], 1);
		seqIndex++;
		return 1;		
	}
	return 0;
}

bool InputLZWDecodeStream::ProcessNextCode() 
{
	int code;
	int nextLength;
	int i, j;

	// check for EOF
	bool ret = false;
	do
	{
		if (!mCurrentlyEncoding)
			break;
		// check for eod and clear-table codes
		do
		{
			code = GetCode();
			if (code == -1 || code == 257)
			{
				mCurrentlyEncoding = false;
				break;
			}
			if (code == 256)
			{
				ClearTable();				
			}
		} while (code == 256);

		if(!mCurrentlyEncoding)
			break;		

		if (nextCode >= 4097) 
		{
			//error(getPos(), "Bad LZW stream - expected clear-table code");
			ClearTable();
		}

		// process the next code
		nextLength = seqLength + 1;
		if (code < 256) 
		{
			seqBuf[0] = code;
			seqLength = 1;
		}
		else if (code < nextCode) 
		{
			seqLength = table[code].length;
			for (i = seqLength - 1, j = code; i > 0; --i) {
				seqBuf[i] = table[j].tail;
				j = table[j].head;
			}
			seqBuf[0] = j;
		}
		else if (code == nextCode) 
		{
			seqBuf[seqLength] = newChar;
			++seqLength;
		}
		else 
		{
			//error(getPos(), "Bad LZW stream - unexpected code");
			mCurrentlyEncoding = false;
			break;
		}
		newChar = seqBuf[0];
		if (first) {
			first = false;
		}
		else {
			table[nextCode].length = nextLength;
			table[nextCode].head = prevCode;
			table[nextCode].tail = newChar;
			++nextCode;
			if (nextCode + mEarly == 512)
				nextBits = 10;
			else if (nextCode + mEarly == 1024)
				nextBits = 11;
			else if (nextCode + mEarly == 2048)
				nextBits = 12;
		}
		prevCode = code;

		// reset buffer
		seqIndex = 0;

		ret = true;

	} while (false);
	return ret;
}


void InputLZWDecodeStream::ClearTable() {
	nextCode = 258;
	nextBits = 9;
	seqIndex = seqLength = 0;
	first = true;
}

int InputLZWDecodeStream::GetCode() {
	int c;
	int code;

	IOBasicTypes::Byte buffer;
	while (inputBits < nextBits) 
	{
		mSourceStream->Read(&buffer, 1);
		c = buffer;

		if (c == -1) return -1;
		inputBuf = (inputBuf << 8) | (c & 0xff);
		inputBits += 8;
	}
	code = (inputBuf >> (inputBits - nextBits)) & ((1 << nextBits) - 1);
	inputBits -= nextBits;
	inputBuf = inputBuf & ((1 << inputBits) - 1); // avoid overflow by limiting to the bits its supposed to use 
	return code;
}

bool InputLZWDecodeStream::NotEnded()
{
	return mCurrentlyEncoding;
}