/*
   Source File : InputAscii85DecodeStream.h


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
#pragma once

#include "EStatusCode.h"
#include "IByteReader.h"

class InputAsciiHexDecodeStream : public IByteReader
{
public:
	InputAsciiHexDecodeStream(void);
	~InputAsciiHexDecodeStream(void);

	// Note that assigning passes ownership on the stream, use Assign(NULL) to remove ownership
	InputAsciiHexDecodeStream(IByteReader* inSourceReader);

	// Assigning passes ownership of the input stream to the decoder stream. 
	// if you don't care for that, then after finishing with the decode, Assign(NULL).
	void Assign(IByteReader* inSourceReader);

	// IByteReader implementation. note that "inBufferSize" determines how many
	// bytes will be placed in the Buffer...not how many are actually read from the underlying
	// encoded stream. got it?!
	virtual IOBasicTypes::LongBufferSizeType Read(IOBasicTypes::Byte* inBuffer,IOBasicTypes::LongBufferSizeType inBufferSize);

	virtual bool NotEnded();

private:
	IByteReader* mSourceStream;

	bool mHitEnd;
	IOBasicTypes::Byte mBuffer[4];
	int mReadBufferSize;
	int mReadBufferIndex;

	void ReadNextBuffer();

};
