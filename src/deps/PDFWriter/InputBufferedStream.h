/*
   Source File : InputBufferedStream.h


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
#include "IByteReaderWithPosition.h"

#define DEFAULT_BUFFER_SIZE 256*1024

class InputBufferedStream : public IByteReaderWithPosition
{
public:
	/*
		default constructor with default buffer size
	*/
	InputBufferedStream(void);

	/*
		Destroys an owned buffer
	*/
	virtual ~InputBufferedStream(void);

	/*
		consturctor with buffer size setup
	*/
	InputBufferedStream(IOBasicTypes::LongBufferSizeType inBufferSize);

	/*
		Constructor with assigning. see Assign for unassign instructions
	*/
	InputBufferedStream(IByteReaderWithPosition* inSourceReader,IOBasicTypes::LongBufferSizeType inBufferSize = DEFAULT_BUFFER_SIZE);

	/*
		Assigns a reader stream for buffered reading. from the moment of assigning the
		buffer assumes control of the stream.
		Assign a NULL or a different reader to release ownership.
	*/
	void Assign(IByteReaderWithPosition* inReader);

	// IByteReaderWithPosition implementation
	virtual LongBufferSizeType Read(Byte* inBuffer,LongBufferSizeType inBufferSize);
	virtual bool NotEnded();
	virtual void Skip(LongBufferSizeType inSkipSize);
	virtual void SetPosition(LongFilePositionType inOffsetFromStart);
	virtual void SetPositionFromEnd(LongFilePositionType inOffsetFromEnd);
	virtual LongFilePositionType GetCurrentPosition();

	IByteReaderWithPosition* GetSourceStream();

private:
	IOBasicTypes::Byte* mBuffer;
	IOBasicTypes::LongBufferSizeType mBufferSize;
	IOBasicTypes::Byte* mCurrentBufferIndex;
	IOBasicTypes::Byte* mLastAvailableIndex;
	IByteReaderWithPosition* mSourceStream;

	void Initiate(IByteReaderWithPosition* inSourceReader,IOBasicTypes::LongBufferSizeType inBufferSize);

};
