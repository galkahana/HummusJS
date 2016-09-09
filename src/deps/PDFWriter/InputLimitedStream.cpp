/*
   Source File : InputLimitedStream.cpp


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
#include "InputLimitedStream.h"

using namespace IOBasicTypes;

#include <algorithm>

InputLimitedStream::InputLimitedStream(void)
{
	mStream = NULL;
	mMoreToRead = 0;
	mOwnsStream = false;
}

InputLimitedStream::~InputLimitedStream(void)
{
	if(mStream && mOwnsStream)
		delete mStream;
}

InputLimitedStream::InputLimitedStream(IByteReader* inSourceStream,LongFilePositionType inReadLimit,bool inOwnsStream)
{
	Assign(inSourceStream,inReadLimit,inOwnsStream);
}


void InputLimitedStream::Assign(IByteReader* inSourceStream,LongFilePositionType inReadLimit,bool inOwnsStream)
{
	mStream = inSourceStream;
	mMoreToRead = inReadLimit;
	mOwnsStream = inOwnsStream;
}

LongBufferSizeType InputLimitedStream::Read(Byte* inBuffer,LongBufferSizeType inBufferSize)
{
	LongBufferSizeType readBytes = mStream->Read(inBuffer,(LongBufferSizeType)std::min<LongFilePositionType>((LongFilePositionType)inBufferSize,mMoreToRead));

	mMoreToRead -= readBytes;
	return readBytes;
}

bool InputLimitedStream::NotEnded()
{
	return mStream->NotEnded() && mMoreToRead > 0;
}
