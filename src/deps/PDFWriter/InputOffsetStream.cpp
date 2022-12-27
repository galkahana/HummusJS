/*
   Source File : InputOffsetStream.cpp


   Copyright 2022 Gal Kahana PDFWriter

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
#include "InputOffsetStream.h"


InputOffsetStream::InputOffsetStream() {
    Assign(NULL);
}

InputOffsetStream::~InputOffsetStream(void) {
}

InputOffsetStream::InputOffsetStream(IByteReaderWithPosition* inSourceReader) {
    Assign(inSourceReader);
}

void InputOffsetStream::Assign(IByteReaderWithPosition* inReader) {
    mSourceStream = inReader;
    mOffset = 0;
}

LongBufferSizeType InputOffsetStream::Read(Byte* inBuffer,LongBufferSizeType inBufferSize) {
    if(!mSourceStream)
        return 0;

    return mSourceStream->Read(inBuffer, inBufferSize);
}

bool InputOffsetStream::NotEnded() {
    if(!mSourceStream)
        return true;

    return mSourceStream->NotEnded();
}

void InputOffsetStream::Skip(LongBufferSizeType inSkipSize) {
    if(mSourceStream)
        mSourceStream->Skip(inSkipSize);
}

void InputOffsetStream::SetPosition(LongFilePositionType inOffsetFromStart) {
    if(mSourceStream)
        mSourceStream->SetPosition(mOffset + inOffsetFromStart);
}

void InputOffsetStream::SetPositionFromEnd(LongFilePositionType inOffsetFromEnd) {
    if(mSourceStream)
        mSourceStream->SetPositionFromEnd(inOffsetFromEnd);    
}

LongFilePositionType InputOffsetStream::GetCurrentPosition() {
    if(!mSourceStream)
        return 0;

    return mSourceStream->GetCurrentPosition() - mOffset;
}

IByteReaderWithPosition* InputOffsetStream::GetSourceStream() {
    return mSourceStream;
}

void InputOffsetStream::SetOffset(LongFilePositionType inOffset) {
    mOffset = inOffset;
}

LongFilePositionType InputOffsetStream::GetOffset() {
    return mOffset;
}
