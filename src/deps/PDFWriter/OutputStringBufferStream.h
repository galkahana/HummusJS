/*
   Source File : OutputStringBufferStream.h


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
#include "MyStringBuf.h"
#include <string>



class OutputStringBufferStream : public IByteWriterWithPosition
{
public:
	OutputStringBufferStream(void);
	~OutputStringBufferStream(void);

	// override for having the stream control an external buffer. NOT taking ownership
	OutputStringBufferStream(MyStringBuf* inControlledBuffer);

	void Assign(MyStringBuf* inControlledBuffer); // can assign a new one after creation


	// IByteWriter implementation
	virtual IOBasicTypes::LongBufferSizeType Write(const IOBasicTypes::Byte* inBuffer,IOBasicTypes::LongBufferSizeType inSize);

	// IByteWriterWithPosition implementation
	virtual IOBasicTypes::LongFilePositionType GetCurrentPosition();

	std::string ToString() const;
	void Reset();
	void SetPosition(IOBasicTypes::LongFilePositionType inOffsetFromStart);


private:
	MyStringBuf* mBuffer;
	bool mOwnsBuffer;

};
