/*
	Source File : InputRC4XcodeStream.h


	Copyright 2016 Gal Kahana PDFWriter

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

#include "InputRC4XcodeStream.h"

using namespace IOBasicTypes;

InputRC4XcodeStream::InputRC4XcodeStream():mRC4()
{
	mSourceStream = NULL;
}


InputRC4XcodeStream::InputRC4XcodeStream(IByteReader* inSourceReader, const ByteList& inKey):mRC4(inKey)
{
	mSourceStream = inSourceReader;
}

InputRC4XcodeStream::~InputRC4XcodeStream(void)
{
	if (mSourceStream)
		delete mSourceStream;
}


void InputRC4XcodeStream::Assign(IByteReader* inSourceReader, const ByteList& inKey)
{
	mSourceStream = inSourceReader;
	mRC4.Reset(inKey);
}

bool InputRC4XcodeStream::NotEnded()
{
	return mSourceStream && mSourceStream->NotEnded();
}

LongBufferSizeType InputRC4XcodeStream::Read(IOBasicTypes::Byte* inBuffer, LongBufferSizeType inBufferSize)
{
	LongBufferSizeType mCurrentIndex = 0;
	Byte buffer;

	while (NotEnded() && mCurrentIndex < inBufferSize)
	{
		mSourceStream->Read(&buffer, 1);
		inBuffer[mCurrentIndex] = mRC4.DecodeNextByte(buffer);
		++mCurrentIndex;
	}

	return mCurrentIndex;
}