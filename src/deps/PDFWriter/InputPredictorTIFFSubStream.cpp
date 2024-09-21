/*
   Source File : InputPredictorTIFFSubStream.cpp


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
#include "InputPredictorTIFFSubStream.h"
#include "Trace.h"

using namespace IOBasicTypes;

InputPredictorTIFFSubStream::InputPredictorTIFFSubStream(void)
{
	mSourceStream = NULL;
	mRowBuffer = NULL;
	mReadColors = NULL;
	mReadColorsCount = 0;
}

InputPredictorTIFFSubStream::InputPredictorTIFFSubStream(IByteReader* inSourceStream,
														LongBufferSizeType inColors,
														Byte inBitsPerComponent,
														LongBufferSizeType inColumns)
{
	mSourceStream = NULL;
	mRowBuffer = NULL;
	mReadColors = NULL;

	Assign(inSourceStream,inColors,inBitsPerComponent,inColumns);
}

InputPredictorTIFFSubStream::~InputPredictorTIFFSubStream(void)
{
	delete mSourceStream;
	delete[] mRowBuffer;
	delete[] mReadColors;
}

LongBufferSizeType InputPredictorTIFFSubStream::Read(Byte* inBuffer,LongBufferSizeType inBufferSize)
{
	LongBufferSizeType readBytes = 0;
	

	// exhaust what's in the buffer currently
	while(mReadColorsCount > (LongBufferSizeType)(mReadColorsIndex - mReadColors) && readBytes < inBufferSize)
	{
		ReadByteFromColorsArray(inBuffer[readBytes]);
		++readBytes;
	}

	// now repeatedly read bytes from the input stream, and decode
	while(readBytes < inBufferSize && mSourceStream->NotEnded())
	{
		if(mSourceStream->Read(mRowBuffer,(mColumns*mColors*mBitsPerComponent)/8) != (mColumns*mColors*mBitsPerComponent)/8)
		{
			TRACE_LOG("InputPredictorPNGSubStream::Read, problem, expected columns*colors*bitspercomponent/8 number read. didn't make it");
			readBytes = 0;
			break;
		}
		DecodeBufferToColors();

		while(mReadColorsCount > (LongBufferSizeType)(mReadColorsIndex - mReadColors) && readBytes < inBufferSize)
		{
			ReadByteFromColorsArray(inBuffer[readBytes]);
			++readBytes;
		}
	}
	return readBytes;	
}

bool InputPredictorTIFFSubStream::NotEnded()
{
	return mSourceStream->NotEnded() || (LongBufferSizeType)(mReadColorsIndex - mReadColors) < mReadColorsCount;
}

void InputPredictorTIFFSubStream::Assign(IByteReader* inSourceStream,
										LongBufferSizeType inColors,
										Byte inBitsPerComponent,
										LongBufferSizeType inColumns)
{
	mSourceStream = inSourceStream;
	mColors = inColors;
	mBitsPerComponent = inBitsPerComponent;
	mColumns = inColumns;
	
	delete mRowBuffer;
	IOBasicTypes::LongBufferSizeType bufferSize = (inColumns*inColors*inBitsPerComponent)/8;
	mRowBuffer = new Byte[bufferSize];

	mReadColorsCount = inColumns * inColors;
	mReadColors = new unsigned short[mReadColorsCount];
	mReadColorsIndex = mReadColors + mReadColorsCount; // assign to end of array so will know that should read new buffer
	mIndexInColor = 0;

	mBitMask = 0;
	for(Byte i=0;i<inBitsPerComponent;++i)
		mBitMask = (mBitMask<<1) + 1;
}

void InputPredictorTIFFSubStream::ReadByteFromColorsArray(Byte& outBuffer)
{
	if(8 == mBitsPerComponent)
	{
		outBuffer = (Byte)(*mReadColorsIndex);
		++mReadColorsIndex;
	}
	else if(8 > mBitsPerComponent)
	{
		outBuffer = 0;
		for(Byte i=0;i<(8/mBitsPerComponent);++i)
		{
			outBuffer = (outBuffer<<mBitsPerComponent) + (Byte)(*mReadColorsIndex);
			++mReadColorsIndex;
		}
	}
	else // 8 < mBitsPerComponent [which is just 16 for now]
	{
		outBuffer =(Byte)(((*mReadColorsIndex)>>(mBitsPerComponent - mIndexInColor*8)) & 0xff);
		++mIndexInColor;
		if(mBitsPerComponent/8 == mIndexInColor)
		{
			++mReadColorsIndex;
			mIndexInColor = 0;
		}
	}
}

void InputPredictorTIFFSubStream::DecodeBufferToColors()
{	
	//1. Split to colors. Use array of colors (should be columns * colors). Each time take BitsPerComponent of the buffer
	//2. Once you got the "colors", loop the array, setting values after diffs (use modulo of bit mask for "sign" computing)
	//3. Now you have the colors array. 
	LongBufferSizeType i = 0;

	// read the colors differences according to bits per component
	if(8 == mBitsPerComponent)
	{
		for(; i < mReadColorsCount;++i)
			mReadColors[i] = mRowBuffer[i];
	}
	else if(8 > mBitsPerComponent)
	{
		for(; i < (mReadColorsCount*mBitsPerComponent/8);++i)
		{
			for(LongBufferSizeType j=0; j < (LongBufferSizeType)(8/mBitsPerComponent); ++j)
			{
				mReadColors[(i+1)*8/mBitsPerComponent - j - 1] = mRowBuffer[i] & mBitMask;
				mRowBuffer[i] = mRowBuffer[i]>>mBitsPerComponent;
			}
		}
	}
	else // mBitesPerComponent > 8
	{
		for(; i < mReadColorsCount;++i)
		{
			mReadColors[i] = 0;
			for(Byte j=0;j<mBitsPerComponent/8;++j)
				mReadColors[i] = (mReadColors[i]<<mBitsPerComponent) + mRowBuffer[i*mBitsPerComponent/8 + j];
		}
	}

	// calculate color values according to diffs
	for(i = mColors; i < mReadColorsCount; ++i)
		mReadColors[i] = (mReadColors[i] + mReadColors[i-mColors]) & mBitMask;

	mReadColorsIndex = mReadColors;
	mIndexInColor = 0;

}