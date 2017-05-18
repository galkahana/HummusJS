/*
   Source File : InputBufferedStream.cpp


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
#include "ArrayOfInputStreamsStream.h"
#include "PDFArray.h"
#include "PDFParser.h"
#include "PDFObjectCast.h"
#include "PDFStreamInput.h"

using namespace IOBasicTypes;


ArrayOfInputStreamsStream::ArrayOfInputStreamsStream(PDFArray* inArrayOfStreams, PDFParser* inParser)
{
	mArray = inArrayOfStreams;
	mParser = inParser;
	mCurrentStream = NULL;
	mCurrentIndex = 0;
}

ArrayOfInputStreamsStream::~ArrayOfInputStreamsStream(void)
{
	delete mCurrentStream;
}

IByteReader* ArrayOfInputStreamsStream::GetActiveStream() {
	
	if (mCurrentStream && mCurrentStream->NotEnded())
		return mCurrentStream;

	// delete current stream, done with it
	if (mCurrentStream) {
		++mCurrentIndex;
	}
	delete mCurrentStream;
	mCurrentStream = NULL;

	// get next stream in array
	PDFObjectCastPtr<PDFStreamInput> aStream;

	while (!mCurrentStream && mCurrentIndex < mArray->GetLength()) {
		PDFObjectCastPtr<PDFStreamInput> aStream = mParser->QueryArrayObject(mArray.GetPtr(), mCurrentIndex);
		if (!!aStream) {
			mCurrentStream = mParser->StartReadingFromStream(aStream.GetPtr());
			// couldn't start, try with next array object
			if (!mCurrentStream)
				++mCurrentIndex;
		}
		else {
			// Not stream, try again with next array object
			++mCurrentIndex;
		}
	}

	return mCurrentStream;
}

LongBufferSizeType ArrayOfInputStreamsStream::Read(Byte* inBuffer,LongBufferSizeType inBufferSize)
{
	if (!NotEnded())
		return 0;

	LongBufferSizeType readAmount = 0;

	while (NotEnded() && readAmount < inBufferSize) {
		// read from current stream
		IByteReader* reader = GetActiveStream();
		if (reader && reader->NotEnded()) {
			readAmount+= reader->Read(inBuffer + readAmount, inBufferSize - readAmount);
		}
	}
	return readAmount;
}

bool ArrayOfInputStreamsStream::NotEnded()
{
	return !!mArray &&  mCurrentIndex < mArray->GetLength();
}