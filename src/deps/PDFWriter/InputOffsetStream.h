/*
   Source File : InputOffsetStream.h


   Copyright 2022 Gal Kahana PDFWriter

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

class InputOffsetStream : public IByteReaderWithPosition
{
public:
	/*
		default constructor
	*/
	InputOffsetStream(void);

	/*
		Does NOT destroy input stream!
	*/
	virtual ~InputOffsetStream(void);

	/*
		Constructor with assigning. see Assign for unassign instructions
	*/
	InputOffsetStream(IByteReaderWithPosition* inSourceReader);

	/*
		Assigns a reader stream for reading with offset. 
        Assignment does not transfer ownership! provided reader should still be deleted after this instance
        is deleted! (this is inline say with how the parser treats input streams)
        (null can be assigned to clear reference)

        Setting a new stream 0s the offset!
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

    // Set the reading offset
    void SetOffset(LongFilePositionType inOffset);

    // Read it back to me plz
    LongFilePositionType GetOffset();

private:
	IByteReaderWithPosition* mSourceStream;
	IOBasicTypes::LongBufferSizeType mOffset;
};
