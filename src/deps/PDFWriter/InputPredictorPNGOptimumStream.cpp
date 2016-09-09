/*
   Source File : InputPredictorPNGOptimumStream.cpp


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
#include "InputPredictorPNGOptimumStream.h"

#include "Trace.h"
#include <stdlib.h>

using namespace IOBasicTypes;

InputPredictorPNGOptimumStream::InputPredictorPNGOptimumStream(void)
{
	mSourceStream = NULL;
	mBuffer = NULL;
	mIndex = NULL;
	mBufferSize = 0;
	mUpValues = NULL;

}

InputPredictorPNGOptimumStream::~InputPredictorPNGOptimumStream(void)
{
	delete[] mBuffer;
	delete[] mUpValues;
	delete mSourceStream;
}

InputPredictorPNGOptimumStream::InputPredictorPNGOptimumStream(IByteReader* inSourceStream,IOBasicTypes::LongBufferSizeType inColumns)
{	
	mSourceStream = NULL;
	mBuffer = NULL;
	mIndex = NULL;
	mBufferSize = 0;
	mUpValues = NULL;

	Assign(inSourceStream,inColumns);
}


LongBufferSizeType InputPredictorPNGOptimumStream::Read(Byte* inBuffer,LongBufferSizeType inBufferSize)
{
	LongBufferSizeType readBytes = 0;
	

	// exhaust what's in the buffer currently
	while(mBufferSize > (LongBufferSizeType)(mIndex - mBuffer) && readBytes < inBufferSize)
	{
		DecodeNextByte(inBuffer[readBytes]);
		++readBytes;
	}

	// now repeatedly read bytes from the input stream, and decode
	while(readBytes < inBufferSize && mSourceStream->NotEnded())
	{
		memcpy(mUpValues,mBuffer,mBufferSize);

		if(mSourceStream->Read(mBuffer,mBufferSize) != mBufferSize)
		{
			TRACE_LOG("InputPredictorPNGOptimumStream::Read, problem, expected columns number read. didn't make it");
			readBytes = 0;
			break;
		}
		mFunctionType = *mIndex;
		*mIndex = 0; // so i can use this as "left" value...we don't care about this one...it's just a tag
		mIndex = mBuffer+1; // skip the first tag

		while(mBufferSize > (LongBufferSizeType)(mIndex - mBuffer) && readBytes < inBufferSize)
		{
			DecodeNextByte(inBuffer[readBytes]);
			++readBytes;
		}
	}
	return readBytes;
}

bool InputPredictorPNGOptimumStream::NotEnded()
{
	return mSourceStream->NotEnded() || (LongBufferSizeType)(mIndex - mBuffer) < mBufferSize;
}

void InputPredictorPNGOptimumStream::DecodeNextByte(Byte& outDecodedByte)
{
	// decoding function is determined by mFunctionType
	switch(mFunctionType)
	{
		case 0:
			outDecodedByte = *mIndex;
			break;
		case 1:
			outDecodedByte = (Byte)((char)mBuffer[mIndex-mBuffer - 1] + (char)*mIndex);
			break;
		case 2:
			outDecodedByte = (Byte)((char)mUpValues[mIndex-mBuffer] + (char)*mIndex);
			break;
		case 3:
			outDecodedByte = (Byte)((char)mBuffer[mIndex-mBuffer - 1]/2 + (char)mUpValues[mIndex-mBuffer]/2 + (char)*mIndex);
			break;
		case 4:
			outDecodedByte = (Byte)(PaethPredictor(mBuffer[mIndex-mBuffer - 1],mUpValues[mIndex-mBuffer],mUpValues[mIndex-mBuffer - 1]) + (char)*mIndex);
			break;
	}

	*mIndex = outDecodedByte; // saving the encoded value back to the buffer, for later copying as "Up value", and current using as "Left" value
	++mIndex;
}

void InputPredictorPNGOptimumStream::Assign(IByteReader* inSourceStream,IOBasicTypes::LongBufferSizeType inColumns)
{
	mSourceStream = inSourceStream;

	delete[] mBuffer;
	delete[] mUpValues;
	mBufferSize = inColumns + 1;
	mBuffer = new Byte[mBufferSize];
	memset(mBuffer,0,mBufferSize);
	mUpValues = new Byte[mBufferSize];
	memset(mUpValues,0,mBufferSize); // that's less important
	mIndex = mBuffer + mBufferSize;
	mFunctionType = 0;

}

char InputPredictorPNGOptimumStream::PaethPredictor(char inLeft,char inUp,char inUpLeft)
{
	int p = inLeft + inUp - inUpLeft;
	int pLeft = abs(p - inLeft);
	int pUp = abs(p - inUp);
	int pUpLeft = abs(p - inUpLeft);

	if(pLeft <= pUp && pLeft <= pUpLeft)
	  return pLeft;
	else if(pUp <= pUpLeft) 
	  return inUp;
	else 
	  return inUpLeft;
}