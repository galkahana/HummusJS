/*
   Source File : PDFStream.cpp


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
#include "PDFStream.h"
#include "IObjectsContextExtender.h"
#include "InputStringBufferStream.h"
#include "OutputStreamTraits.h"
#include "EncryptionHelper.h"

PDFStream::PDFStream(bool inCompressStream,
					 IByteWriterWithPosition* inOutputStream,
					 EncryptionHelper* inEncryptionHelper,
					 ObjectIDType inExtentObjectID,
					 IObjectsContextExtender* inObjectsContextExtender)
{
	mExtender = inObjectsContextExtender;
	mCompressStream = inCompressStream;
	mExtendObjectID = inExtentObjectID;	
	mStreamStartPosition = inOutputStream->GetCurrentPosition();
	mOutputStream = inOutputStream;
	if (inEncryptionHelper && inEncryptionHelper->IsEncrypting()) {
		mEncryptionStream = inEncryptionHelper->CreateEncryptionStream(inOutputStream);
	}
	else {
		mEncryptionStream = NULL;
	}

	mStreamLength = 0;
    mStreamDictionaryContextForDirectExtentStream = NULL;


	if(mCompressStream)
	{
		if(mExtender && mExtender->OverridesStreamCompression())
		{
			mWriteStream = mExtender->GetCompressionWriteStream(mEncryptionStream ? mEncryptionStream:inOutputStream);
		}
		else
		{
			mFlateEncodingStream.Assign(mEncryptionStream ? mEncryptionStream : inOutputStream);
			mWriteStream = &mFlateEncodingStream;
		}
	}
	else
		mWriteStream = mEncryptionStream ? mEncryptionStream : inOutputStream;

}

PDFStream::PDFStream(
          bool inCompressStream,
          IByteWriterWithPosition* inOutputStream,
			EncryptionHelper* inEncryptionHelper,
			DictionaryContext* inStreamDictionaryContextForDirectExtentStream,
          IObjectsContextExtender* inObjectsContextExtender)
{
	mExtender = inObjectsContextExtender;
	mCompressStream = inCompressStream;
	mExtendObjectID = 0;	
	mStreamStartPosition = 0;
	mOutputStream = inOutputStream;
	mStreamLength = 0;
    mStreamDictionaryContextForDirectExtentStream = inStreamDictionaryContextForDirectExtentStream;
    
    mTemporaryOutputStream.Assign(&mTemporaryStream);
	if (inEncryptionHelper && inEncryptionHelper->IsEncrypting()) {
		mEncryptionStream = inEncryptionHelper->CreateEncryptionStream(&mTemporaryOutputStream);
	}
	else {
		mEncryptionStream = NULL;
	}


    
	if(mCompressStream)
	{
		if(mExtender && mExtender->OverridesStreamCompression())
		{
			mWriteStream = mExtender->GetCompressionWriteStream(mEncryptionStream ? mEncryptionStream : &mTemporaryOutputStream);
		}
		else
		{
			mFlateEncodingStream.Assign(mEncryptionStream ? mEncryptionStream : &mTemporaryOutputStream);
			mWriteStream = &mFlateEncodingStream;
		}
	}
	else
		mWriteStream = mEncryptionStream ? mEncryptionStream : &mTemporaryOutputStream;
    
}


PDFStream::~PDFStream(void)
{
    
}

IByteWriter* PDFStream::GetWriteStream()
{
	return mWriteStream;
}

void PDFStream::FinalizeStreamWrite()
{
	if(mExtender && mExtender->OverridesStreamCompression() && mCompressStream)
		mExtender->FinalizeCompressedStreamWrite(mWriteStream);
	mWriteStream = NULL;
	if(mCompressStream)
		mFlateEncodingStream.Assign(NULL);  // this both finished encoding any left buffers and releases ownership from mFlateEncodingStream

	if (mEncryptionStream) {
		// safe to delete. encryption stream is not supposed to own the underlying stream in any case. make sure
		// to delete before measuring output, as flushing may occur at this point
		delete mEncryptionStream;
		mEncryptionStream = NULL;
	}

    // different endings, depending if direct stream writing or not
    if(mExtendObjectID == 0)
    {
        mStreamLength = mTemporaryStream.GetCurrentWritePosition();
    }
    else 
    {
        mStreamLength = mOutputStream->GetCurrentPosition()-mStreamStartPosition;
        mOutputStream = NULL;
    }
}

LongFilePositionType PDFStream::GetLength()
{
	return mStreamLength;
}

bool PDFStream::IsStreamCompressed()
{
	return mCompressStream;
}

ObjectIDType PDFStream::GetExtentObjectID()
{
	return mExtendObjectID;
}

DictionaryContext* PDFStream::GetStreamDictionaryForDirectExtentStream()
{
    return mStreamDictionaryContextForDirectExtentStream;
}

void PDFStream::FlushStreamContentForDirectExtentStream()
{
    mTemporaryStream.pubseekoff(0,std::ios_base::beg);
    
    // copy internal temporary stream to output
    InputStringBufferStream inputStreamForWrite(&mTemporaryStream);
    OutputStreamTraits streamCopier(mOutputStream);
    streamCopier.CopyToOutputStream(&inputStreamForWrite);
    
    mTemporaryStream.str();
    mOutputStream = NULL;
}

