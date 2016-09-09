/*
   Source File : OutputFlateDecodeStream.cpp


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
#include "OutputFlateDecodeStream.h"
#ifndef NO_TRACE
#include "Trace.h"
#endif
#include "zlib.h"

#define BUFFER_SIZE 256*1024

using namespace IOBasicTypes;

OutputFlateDecodeStream::OutputFlateDecodeStream(void)
{
	mBuffer = new IOBasicTypes::Byte[BUFFER_SIZE];
	mZLibState = new z_stream;
	mTargetStream = NULL;
	mCurrentlyEncoding = false;
}

OutputFlateDecodeStream::~OutputFlateDecodeStream(void)
{
	if(mCurrentlyEncoding)
		FinalizeEncoding();
	if(mTargetStream)
		delete mTargetStream;
	delete[] mBuffer;
	delete mZLibState;
}

void OutputFlateDecodeStream::FinalizeEncoding()
{
	// no need for flushing here, there's no notion of Z_FINISH. so just end the library work
	inflateEnd(mZLibState);
	mCurrentlyEncoding = false;
}

OutputFlateDecodeStream::OutputFlateDecodeStream(IByteWriter* inTargetWriter, bool inInitiallyOn)
{
	mBuffer = new IOBasicTypes::Byte[BUFFER_SIZE];
	mZLibState = new z_stream;
	mTargetStream = NULL;
	mCurrentlyEncoding = false;

	Assign(inTargetWriter,inInitiallyOn);
}

void OutputFlateDecodeStream::StartEncoding()
{
	mZLibState->zalloc = Z_NULL;
    mZLibState->zfree = Z_NULL;
    mZLibState->opaque = Z_NULL;
	mZLibState->avail_in = 0;
	mZLibState->next_in = Z_NULL;

    int inflateStatus = inflateInit(mZLibState);
#ifndef NO_TRACE
    if (inflateStatus != Z_OK)
		TRACE_LOG1("OutputFlateDecodeStream::StartEncoding, Unexpected failure in initializating flate library. status code = %d",inflateStatus);
	else
		mCurrentlyEncoding = true;
#else
    if (Z_OK == inflateStatus)
		mCurrentlyEncoding = true;

#endif
}


void OutputFlateDecodeStream::Assign(IByteWriter* inWriter,bool inInitiallyOn)
{	
	mTargetStream = inWriter;
	if(inInitiallyOn && mTargetStream)
		StartEncoding();

}


LongBufferSizeType OutputFlateDecodeStream::Write(const IOBasicTypes::Byte* inBuffer,LongBufferSizeType inSize)
{
	if(mCurrentlyEncoding)
		return DecodeBufferAndWrite(inBuffer,inSize);
	else if(mTargetStream)
		return mTargetStream->Write(inBuffer,inSize);
	else
		return 0;
}

LongBufferSizeType OutputFlateDecodeStream::DecodeBufferAndWrite(const IOBasicTypes::Byte* inBuffer,LongBufferSizeType inSize)
{
	if(0 == inSize)
		return 0; // inflate kinda touchy about getting 0 lengths

	int inflateResult;

	mZLibState->avail_in = (uInt)inSize; // hmm, caveat here...should take care of this sometime.
	mZLibState->next_in = (Bytef*)inBuffer;

	do
	{
		mZLibState->avail_out = BUFFER_SIZE;
		mZLibState->next_out = mBuffer;
		inflateResult = inflate(mZLibState,Z_NO_FLUSH);
		if(Z_STREAM_ERROR == inflateResult ||
		   Z_NEED_DICT == inflateResult ||
		   Z_DATA_ERROR == inflateResult ||
		   Z_MEM_ERROR == inflateResult)
		{
#ifndef NO_TRACE
			TRACE_LOG1("OutputFlateDecodeStream::DecodeBufferAndWrite, failed to write zlib information. returned error code = %d",inflateResult);
#endif
			inflateEnd(mZLibState);
			break;
		}
		else
		{
			LongBufferSizeType writtenBytes;
			writtenBytes = mTargetStream->Write(mBuffer,BUFFER_SIZE-mZLibState->avail_out);
			if(writtenBytes != BUFFER_SIZE-mZLibState->avail_out)
			{
#ifndef NO_TRACE
				TRACE_LOG2("OutputFlateDecodeStream::DecodeBufferAndWrite, Failed to write the desired amount of zlib bytes to underlying stream. supposed to write %lld, wrote %lld",
								BUFFER_SIZE-mZLibState->avail_out,writtenBytes);
#endif
				inflateEnd(mZLibState);
				inflateResult = Z_STREAM_ERROR;
				mCurrentlyEncoding = false;
				break;
			}
		}
	}while(mZLibState->avail_out == 0); // waiting for either no more writes

	// should be that at the last buffer we'll get here a nice Z_STREAM_END
	if(Z_OK == inflateResult || Z_STREAM_END == inflateResult)
		return inSize;
	else
		return 0;
}

void OutputFlateDecodeStream::TurnOnEncoding()
{
	if(!mCurrentlyEncoding)
		StartEncoding();
}

void OutputFlateDecodeStream::TurnOffEncoding()
{
	if(mCurrentlyEncoding)
		FinalizeEncoding();
}
