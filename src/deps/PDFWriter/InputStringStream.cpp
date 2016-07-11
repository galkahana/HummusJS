/*
Source File : InputStringStream.cpp


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
#include "InputStringStream.h"

using namespace std;
using namespace IOBasicTypes;

InputStringStream::InputStringStream()
{
}

InputStringStream::InputStringStream(const string& inString)
{
	mStartPosition = inString.begin();
	mEndPosition = inString.end();
	mCurrentPosition = mStartPosition;
}

InputStringStream::~InputStringStream(void)
{
}

void InputStringStream::Assign(const string& inString)
{
	mStartPosition = inString.begin();
	mEndPosition = inString.end();
	mCurrentPosition = mStartPosition;
}

LongBufferSizeType InputStringStream::Read(Byte* inBuffer, LongBufferSizeType inBufferSize)
{
	LongBufferSizeType amountRead = 0;

	for (; amountRead < inBufferSize && mCurrentPosition != mEndPosition; ++amountRead, ++mCurrentPosition)
		inBuffer[amountRead] = (Byte)*mCurrentPosition;
	return amountRead;
}

bool InputStringStream::NotEnded()
{
	return mCurrentPosition != mEndPosition;

}

void InputStringStream::Skip(LongBufferSizeType inSkipSize)
{
	mCurrentPosition = (inSkipSize > (LongBufferSizeType)(mEndPosition - mCurrentPosition)) ? mEndPosition : (mCurrentPosition + inSkipSize);;
}

void InputStringStream::SetPosition(LongFilePositionType inOffsetFromStart)
{
	mCurrentPosition = inOffsetFromStart > (LongFilePositionType)(mEndPosition - mStartPosition) ? mEndPosition : (mStartPosition + (string::size_type)inOffsetFromStart);
}

void InputStringStream::SetPositionFromEnd(LongFilePositionType inOffsetFromEnd)
{
	mCurrentPosition = inOffsetFromEnd > (LongFilePositionType)(mEndPosition - mStartPosition) ? mStartPosition : (mStartPosition + (mEndPosition - mStartPosition - (string::size_type)inOffsetFromEnd));
}

LongFilePositionType InputStringStream::GetCurrentPosition()
{
	return mCurrentPosition - mStartPosition;
}
