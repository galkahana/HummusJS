/*
 Source File : InputFlateDecodeStream.cpp
 
 
 Copyright 2012 Gal Kahana PDFWriter
 
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
#include "InputDCTDecodeStream.h"
#include "Trace.h"

#include <string.h>

#ifndef PDFHUMMUS_NO_DCT

using namespace IOBasicTypes;
using namespace PDFHummus;

class HummusJPGException
{
};

METHODDEF(void) HummusJPGErrorExit (j_common_ptr cinfo)
{
    (*cinfo->err->output_message) (cinfo);
    throw HummusJPGException();
}

METHODDEF(void) HummusJPGOutputMessage(j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];
    
    (*cinfo->err->format_message) (cinfo, buffer);
    TRACE_LOG1("HummusJPGOutputMessage, error from jpg library: %s",buffer);
}

struct HummusSourceManager
{
    struct jpeg_source_mgr pub;	/* public fields */
    
    IByteReader *mReader;	/* source stream */
    JOCTET * buffer;		/* start of buffer */
};

#define INPUT_BUF_SIZE  4096	/* choose an efficiently fread'able size */

METHODDEF(boolean) HummusFillInputBuffer (j_decompress_ptr cinfo)
{
    HummusSourceManager* src = (HummusSourceManager*) cinfo->src;
    size_t nbytes;
    
    nbytes =  src->mReader->Read((Byte*)(src->buffer),INPUT_BUF_SIZE);
    
    if (nbytes <= 0) {
        /* Insert a fake EOI marker */
        src->buffer[0] = (JOCTET) 0xFF;
        src->buffer[1] = (JOCTET) JPEG_EOI;
        nbytes = 2;
    }
    
    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = nbytes;
    
    return TRUE;
}

METHODDEF(void) HummusSkipInputData (j_decompress_ptr cinfo, long num_bytes)
{
    
    struct jpeg_source_mgr * src = cinfo->src;
    
    if (num_bytes > 0) {
        while (num_bytes > (long) src->bytes_in_buffer) {
            num_bytes -= (long) src->bytes_in_buffer;
            (void) (*src->fill_input_buffer) (cinfo);
            /* note we assume that fill_input_buffer will never return FALSE,
             * so suspension need not be handled.
             */
        }
        src->next_input_byte += (size_t) num_bytes;
        src->bytes_in_buffer -= (size_t) num_bytes;
    }
}

METHODDEF(void) HummusNoOp (j_decompress_ptr cinfo)
{
    /* no work necessary here */
}

METHODDEF(void) HummusJPGSourceInitialization (j_decompress_ptr cinfo, IByteReader * inSourceStream)
{
    HummusSourceManager* src;
    
    /* The source object and input buffer are made permanent so that a series
     * of JPEG images can be read from the same file by calling jpeg_stdio_src
     * only before the first one.  (If we discarded the buffer at the end of
     * one image, we'd likely lose the start of the next one.)
     * This makes it unsafe to use this manager and a different source
     * manager serially with the same JPEG object.  Caveat programmer.
     */
    if (cinfo->src == NULL) {	/* first time for this JPEG object? */
        cinfo->src = (struct jpeg_source_mgr *)
        (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                    sizeof(HummusSourceManager));
        src = (HummusSourceManager*) cinfo->src;
        src->buffer = (JOCTET *)
        (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                    INPUT_BUF_SIZE * sizeof(JOCTET));
    }
    
    src = (HummusSourceManager*) cinfo->src;
    src->pub.init_source = HummusNoOp;
    src->pub.fill_input_buffer = HummusFillInputBuffer;
    src->pub.skip_input_data = HummusSkipInputData;
    src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
    src->pub.term_source = HummusNoOp;
    src->mReader = inSourceStream;
    src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
    src->pub.next_input_byte = NULL; /* until buffer loaded */
}



InputDCTDecodeStream::InputDCTDecodeStream(void)
{
    mStream = NULL;
	mIsDecoding = false;
    mIsHeaderRead = false;
    mCurrentSampleRow = 0;
    mTotalSampleRows = 0;
    mIndexInRow = 0;
}

InputDCTDecodeStream::~InputDCTDecodeStream(void)
{
	if(mIsDecoding)
		FinalizeDecoding();
	if(mStream)
		delete mStream;
}

void InputDCTDecodeStream::FinalizeDecoding()
{
    jpeg_destroy_decompress(&mJPGState);
	mIsDecoding = false;
    mSamplesBuffer = NULL;
    mIsHeaderRead = false;
}

InputDCTDecodeStream::InputDCTDecodeStream(IByteReader* inSourceReader)
{
	mStream = NULL;
	mIsDecoding = false;
    mIsHeaderRead = false;
    
	Assign(inSourceReader);
}

void InputDCTDecodeStream::Assign(IByteReader* inSourceReader)
{
	mStream = inSourceReader;
    if(mStream)
        InitializeDecodingState();
}

void InputDCTDecodeStream::InitializeDecodingState()
{
    mJPGState.err = jpeg_std_error(&mJPGError);
    mJPGError.error_exit = HummusJPGErrorExit;
    mJPGError.output_message = HummusJPGOutputMessage;
    
    try {
        jpeg_create_decompress(&mJPGState);
        HummusJPGSourceInitialization(&mJPGState,mStream);
        mIsDecoding = true;
    }
    catch(HummusJPGException)
    {
        TRACE_LOG("InputDCTDecodeStream::InitializeDecodingState, caught exception in jpg decoding");
        
    }
}


EStatusCode InputDCTDecodeStream::StartRead()
{
    EStatusCode status = eSuccess;
    
    try
    {
        jpeg_read_header(&mJPGState, TRUE);
        jpeg_start_decompress(&mJPGState);
    
        int row_stride = mJPGState.output_width * mJPGState.output_components;
        mSamplesBuffer = (*mJPGState.mem->alloc_sarray)
            ((j_common_ptr) &mJPGState, JPOOL_IMAGE, row_stride, mJPGState.rec_outbuf_height);
        mCurrentSampleRow = 0;
        mTotalSampleRows = 0;
        mIndexInRow = 0;
        
        mIsHeaderRead = true;
    }
    catch(HummusJPGException)
    {
        TRACE_LOG("InputDCTDecodeStream::StartRead, caught exception in jpg decoding");
        status = eFailure;
    }
    return status;
    
}

LongBufferSizeType InputDCTDecodeStream::Read(
                                                Byte* inBuffer,
                                                LongBufferSizeType inBufferSize)
{
	if(!mIsDecoding)
		return 0;
    
    if(!mIsHeaderRead)
    {
        if(StartRead() != eSuccess)
            return 0;
    }

    Byte* indexInBuffer = inBuffer;
    
    // fill buffer with what remains from samples buffer
    indexInBuffer = CopySamplesArrayToBuffer(inBuffer,inBufferSize);
 
    // if not satisfied, loop by reading more samples, and filling the buffer.
    // if while reading for samples encountered end of filter, stop
    while(((LongBufferSizeType)(indexInBuffer - inBuffer) < inBufferSize) && (mJPGState.output_scanline < mJPGState.output_height))
    {
        try
        {
            mTotalSampleRows = jpeg_read_scanlines(&mJPGState, mSamplesBuffer, mJPGState.rec_outbuf_height);
        }
        catch(HummusJPGException)
        {
            TRACE_LOG("InputDCTDecodeStream::Read, caught exception in jpg decoding");
            mTotalSampleRows = 0;
        }
        mIndexInRow = 0;
        mCurrentSampleRow = 0;
        indexInBuffer = CopySamplesArrayToBuffer(indexInBuffer,inBufferSize - (indexInBuffer - inBuffer));
    }
    return indexInBuffer - inBuffer;
}

Byte* InputDCTDecodeStream::CopySamplesArrayToBuffer(Byte* inBuffer, LongBufferSizeType inBufferSize)
{
    Byte* indexInBuffer = inBuffer;
    LongBufferSizeType row_stride = mJPGState.output_width * mJPGState.output_components;

    while(mCurrentSampleRow < mTotalSampleRows && ((LongBufferSizeType)(indexInBuffer - inBuffer)) < inBufferSize)
    {
        if((inBufferSize - (indexInBuffer - inBuffer)) < (row_stride - mIndexInRow))
        {
			// buffer is smaller than what's left in row, fill buffer
            memcpy(indexInBuffer,mSamplesBuffer[mCurrentSampleRow] + mIndexInRow,inBufferSize - (indexInBuffer - inBuffer));
            mIndexInRow += inBufferSize - (indexInBuffer - inBuffer);
            indexInBuffer =  inBuffer + inBufferSize;
        }
        else
        {
			// buffer is larger than what's left in row, read into buffer whats left, mark row as finished
            memcpy(indexInBuffer,mSamplesBuffer[mCurrentSampleRow] + mIndexInRow,row_stride - mIndexInRow);
            indexInBuffer += row_stride - mIndexInRow;
            ++mCurrentSampleRow;
            mIndexInRow = 0;
        }
    }

    return indexInBuffer;
}

bool InputDCTDecodeStream::NotEnded()
{
    if(!mStream || !mIsDecoding)
        return false;
    else
        return (!mIsHeaderRead || (mCurrentSampleRow < mTotalSampleRows) || (mJPGState.output_scanline < mJPGState.output_height));
    
}

#endif
