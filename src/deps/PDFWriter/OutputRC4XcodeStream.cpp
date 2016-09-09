/*
Source File : InputRC4XcodeStream.cpp


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

#include "OutputRC4XcodeStream.h"

using namespace IOBasicTypes;


OutputRC4XcodeStream::OutputRC4XcodeStream(void) 
{
	mTargetStream = NULL;
	mOwnsStream = false;
}

OutputRC4XcodeStream::~OutputRC4XcodeStream(void) 
{
	if (mOwnsStream)
		delete mTargetStream;
}

OutputRC4XcodeStream::OutputRC4XcodeStream(IByteWriterWithPosition* inTargetStream, const ByteList& inEncryptionKey, bool inOwnsStream):mRC4(inEncryptionKey) 
{
	mTargetStream = inTargetStream;
	mOwnsStream = inOwnsStream;
}

LongBufferSizeType OutputRC4XcodeStream::Write(const Byte* inBuffer, LongBufferSizeType inSize)
{
	if (!mTargetStream)
		return 0;

	LongBufferSizeType mCurrentIndex = 0;
	Byte buffer;

	while (mCurrentIndex < inSize)
	{
		buffer = mRC4.DecodeNextByte(inBuffer[mCurrentIndex]);
		mTargetStream->Write(&buffer, 1);
		++mCurrentIndex;
	}

	return mCurrentIndex;
}

LongFilePositionType OutputRC4XcodeStream::GetCurrentPosition()
{
	if (!mTargetStream)
		return 0;

	return mTargetStream->GetCurrentPosition();

}
