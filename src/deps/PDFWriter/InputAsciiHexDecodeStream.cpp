/*
   Source File : InputAsciiHexDecodeStream.cpp


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
#include "InputAsciiHexDecodeStream.h"

#include <string.h>

#include <algorithm>

using namespace IOBasicTypes;

InputAsciiHexDecodeStream::InputAsciiHexDecodeStream(void)
{
	mSourceStream = NULL;
}

InputAsciiHexDecodeStream::~InputAsciiHexDecodeStream(void)
{
	if(mSourceStream)
		delete mSourceStream;
}

InputAsciiHexDecodeStream::InputAsciiHexDecodeStream(IByteReader* inSourceReader)
{
	Assign(inSourceReader);
}

void InputAsciiHexDecodeStream::Assign(IByteReader* inSourceReader)
{
	mSourceStream = inSourceReader;
	mHitEnd = false;

	mReadBufferSize = 0;
	mReadBufferIndex = 0;
}

bool InputAsciiHexDecodeStream::NotEnded()
{
	return mSourceStream && ((!mHitEnd && mSourceStream->NotEnded()) || mReadBufferIndex < mReadBufferSize);
}

LongBufferSizeType InputAsciiHexDecodeStream::Read(IOBasicTypes::Byte* inBuffer, LongBufferSizeType inBufferSize)
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

void InputAsciiHexDecodeStream::ReadNextBuffer()
{
	Byte buffer[2];
	Byte aByte = 0;
	int readIndex = 0;
	
	while(readIndex < 2 && mSourceStream->NotEnded())
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

	// if there's anything to decode
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

			if (buffer[0] >= '0' && buffer[0] <= '9') {
				encodedValue = (buffer[0] - '0') << 4;
			}
			else if (buffer[0] >= 'A' && buffer[0] <= 'F') {
				encodedValue = (buffer[0] - 'A' + 10) << 4;
			}
			else if (buffer[0] >= 'a' && buffer[0] <= 'f') {
				encodedValue = (buffer[0] - 'a' + 10) << 4;
			}

			if (buffer[1] >= '0' && buffer[1] <= '9') {
				encodedValue += buffer[1] - '0';
			}
			else if (buffer[1] >= 'A' && buffer[1] <= 'F') {
				encodedValue += buffer[1] - 'A' + 10;
			}
			else if (buffer[1] >= 'a' && buffer[1] <= 'f') {
				encodedValue += buffer[1] - 'a' + 10;
			}

			memcpy(mBuffer, &encodedValue,4);
			mReadBufferIndex = 0;
			mReadBufferSize = readIndex-1;
		}

	}
}
