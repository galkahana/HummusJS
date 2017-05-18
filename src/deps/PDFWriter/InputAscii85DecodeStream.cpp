/*
   Source File : InputAscii85DecodeStream.cpp


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
#include "InputAscii85DecodeStream.h"

#include <string.h>

#include <algorithm>

using namespace IOBasicTypes;

InputAscii85DecodeStream::InputAscii85DecodeStream(void)
{
	mSourceStream = NULL;
}

InputAscii85DecodeStream::~InputAscii85DecodeStream(void)
{
	if(mSourceStream)
		delete mSourceStream;
}

InputAscii85DecodeStream::InputAscii85DecodeStream(IByteReader* inSourceReader)
{
	Assign(inSourceReader);
}

void InputAscii85DecodeStream::Assign(IByteReader* inSourceReader)
{
	mSourceStream = inSourceReader;
	mHitEnd = false;

	mReadBufferSize = 0;
	mReadBufferIndex = 0;
}

bool InputAscii85DecodeStream::NotEnded()
{
	return mSourceStream && ((!mHitEnd && mSourceStream->NotEnded()) || mReadBufferIndex < mReadBufferSize);
}

LongBufferSizeType InputAscii85DecodeStream::Read(IOBasicTypes::Byte* inBuffer,LongBufferSizeType inBufferSize)
{
	LongBufferSizeType mCurrentIndex = 0;


	if(NotEnded() && mReadBufferSize == 0) // read buffer, in any case (don't like 0 size memcpys)
		ReadNextBuffer();


	while(NotEnded() && mCurrentIndex < inBufferSize)
	{
		// copy what's in the buffer
		int sizeToCopy = std::min<int>(mReadBufferSize-mReadBufferIndex,(int)(inBufferSize-mCurrentIndex));
		memcpy(inBuffer + mCurrentIndex,mBuffer + mReadBufferIndex ,sizeToCopy);

		mReadBufferIndex+=sizeToCopy;
		mCurrentIndex+=sizeToCopy;

		// now read the next batch (even if buffer is satisfied...so we'll have for next read)
		if(mReadBufferIndex == mReadBufferSize)
			ReadNextBuffer();
	}

	return mCurrentIndex;
}

void InputAscii85DecodeStream::ReadNextBuffer()
{
	Byte buffer[5];
	int readIndex = 0;
	Byte aByte;

	while(readIndex < 5 && mSourceStream->NotEnded())
	{
		mSourceStream->Read(&aByte,1);

		if(33 <= aByte && aByte<= 117)
		{
			buffer[readIndex] = aByte;
			++readIndex;
		}
		else if(aByte == 122)
		{
			// z - special case
			buffer[readIndex] = aByte;
			++readIndex;
			break;
		}
		else if(aByte == 126)
		{
			// EOD marker
			mSourceStream->Read(&aByte,1);
			if(aByte == 62)
			{
				mHitEnd = true;
				break;
			}
		}
		// else, continue to next character, skipping space and invalid

	}

	// if there's anything to encode
	if(readIndex > 0)
	{
		if(buffer[0] == 122)
		{
			// special case Z
			mBuffer[0] = mBuffer[1] = mBuffer[2] = mBuffer[3] = 0;
			mReadBufferIndex = 0;
			mReadBufferSize = 4;
		}
		else
		{
			unsigned long encodedValue = 0;

			int i=0;
			for(;i <readIndex;++i)
				encodedValue = encodedValue * 85 + buffer[i] - 33;
			for(;i<5;++i)
				encodedValue= encodedValue * 85 + 117 -33;

			for(int j=0;j < 4;++j)
			{
				mBuffer[3-j] = encodedValue & 0xFF;
				encodedValue = encodedValue>>8;
			}
			mReadBufferIndex = 0;
			mReadBufferSize = readIndex-1;
		}

	}
}
