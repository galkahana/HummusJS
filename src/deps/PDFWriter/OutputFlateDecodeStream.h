/*
   Source File : OutputFlateDecodeStream.h


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
#include "IByteWriterWithPosition.h"

struct z_stream_s;
typedef z_stream_s z_stream;

class OutputFlateDecodeStream : public IByteWriter
{
public:
	OutputFlateDecodeStream(void);
	virtual ~OutputFlateDecodeStream(void);

	/* passing the target writer to OutputFlateDecodeStream makes it its owner. so if you don't want it to try and delete it on destructor - use Assign(NULL) */
	OutputFlateDecodeStream(IByteWriter* inTargetWriter, bool inInitiallyOn = true);

	// Assing makes OutputFlateDecodeStream the owner of inWriter, so if you don't want the class to delete it upon destructions - use Assign(NULL)
	void Assign(IByteWriter* inWriter,bool inInitiallyOn = true);

	virtual IOBasicTypes::LongBufferSizeType Write(const IOBasicTypes::Byte* inBuffer,IOBasicTypes::LongBufferSizeType inSize);

	void TurnOnEncoding();
	void TurnOffEncoding();

private:
	IOBasicTypes::Byte* mBuffer;
	IByteWriter* mTargetStream;
	bool mCurrentlyEncoding;
	z_stream* mZLibState;

	void FinalizeEncoding();
	void StartEncoding();
	IOBasicTypes::LongBufferSizeType DecodeBufferAndWrite(const IOBasicTypes::Byte* inBuffer,IOBasicTypes::LongBufferSizeType inSize);
};
