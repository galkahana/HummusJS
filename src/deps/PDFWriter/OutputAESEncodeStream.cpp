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

#include "OutputAESEncodeStream.h"
#include "MD5Generator.h"
#include "PDFDate.h"

#include <string.h>

using namespace IOBasicTypes;


OutputAESEncodeStream::OutputAESEncodeStream(void)
{
	mTargetStream = NULL;
	mOwnsStream = false;
	mWroteIV = false;
}

OutputAESEncodeStream::~OutputAESEncodeStream(void)
{
	Flush();
	if(mEncryptionKey)
		delete[] mEncryptionKey;
	if (mOwnsStream)
		delete mTargetStream;
}

OutputAESEncodeStream::OutputAESEncodeStream(IByteWriterWithPosition* inTargetStream, const ByteList& inEncryptionKey, bool inOwnsStream) 
{
	mTargetStream = inTargetStream;
	mOwnsStream = inOwnsStream;

	if (!mTargetStream)
		return;

	mInIndex = mIn;

	// convert inEncryptionKey to internal rep and init encrypt [let's hope its 16...]
	mEncryptionKey = new unsigned char[inEncryptionKey.size()];
	mEncryptionKeyLength = inEncryptionKey.size();
	ByteList::const_iterator it = inEncryptionKey.begin();
	size_t i = 0;
	for (; it != inEncryptionKey.end(); ++i, ++it)
		mEncryptionKey[i] = *it;
	mEncrypt.key(mEncryptionKey, mEncryptionKeyLength);
	
	mWroteIV = false;

}

LongFilePositionType OutputAESEncodeStream::GetCurrentPosition() 
{
	if (mTargetStream)
		return mTargetStream->GetCurrentPosition();
	else
		return 0;
}

LongBufferSizeType OutputAESEncodeStream::Write(const IOBasicTypes::Byte* inBuffer, IOBasicTypes::LongBufferSizeType inSize) 
{
	if (!mTargetStream)
		return 0;

	// write IV if didn't write yet
	if (!mWroteIV) {
		// create IV and write it to output file [use existing PDFDate]
		MD5Generator md5;
		// encode current time
		PDFDate currentTime;
		currentTime.SetToCurrentTime();
		md5.Accumulate(currentTime.ToString());
		memcpy(mIV, (const unsigned char*)md5.ToStringAsString().c_str(), AES_BLOCK_SIZE); // md5 should give us the desired 16 bytes

																						   // now write mIV to the output stream
		mTargetStream->Write(mIV, AES_BLOCK_SIZE);
		mWroteIV = true;
	}


	// input and existing buffer sizes smaller than block size, so just copy and return


	IOBasicTypes::LongBufferSizeType left = inSize;

	while (left > 0) {
		// if what's left is not enough to finish block, just copy and stop
		if (left + (mInIndex - mIn) < AES_BLOCK_SIZE) {
			memcpy(mInIndex, inBuffer + inSize - left, left);
			mInIndex += left;
			left = 0;
		}
		else {
			// otherwise, enough to fill block. fill, encode and continue
			IOBasicTypes::LongBufferSizeType remainder = AES_BLOCK_SIZE - (mInIndex - mIn);
			memcpy(mInIndex, inBuffer + inSize - left, remainder);

			// encrypt
			mEncrypt.cbc_encrypt(mIn, mOut, AES_BLOCK_SIZE, mIV);
			mTargetStream->Write(mOut, AES_BLOCK_SIZE);
			mInIndex = mIn;
			left -= remainder;
		}
	}

	return inSize;
}

void OutputAESEncodeStream::Flush() {
	// if there's a full buffer waiting, write it now.
	if (mInIndex - mIn == AES_BLOCK_SIZE) {
		mEncrypt.cbc_encrypt(mIn, mOut, AES_BLOCK_SIZE, mIV);
		mTargetStream->Write(mOut, AES_BLOCK_SIZE);
		mInIndex = mIn;
	}

	// (otherwise or in addition) finish encoding by completing a full block with the block remainder size. if the remainder is AES_BLOCK_SIZE cause block is empty, fill with AES_BLOCK_SIZE
	unsigned char remainder = (unsigned char)(AES_BLOCK_SIZE - (mInIndex - mIn));
	for (size_t i = 0; i < remainder; ++i)
		mInIndex[i] = remainder;
	mEncrypt.cbc_encrypt(mIn, mOut, AES_BLOCK_SIZE, mIV);
	mTargetStream->Write(mOut, AES_BLOCK_SIZE);
}