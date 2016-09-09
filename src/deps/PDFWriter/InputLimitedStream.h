/*
   Source File : InputLimitedStream.h


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

#include "IByteReader.h"

class InputLimitedStream : public IByteReader
{
public:
	InputLimitedStream();
	virtual ~InputLimitedStream(void);

	// K. with this one (just cause i'm tired of this ownership thing) i'm trying something new - to flag ownership. 
	// So make good use of this last flag
	InputLimitedStream(IByteReader* inSourceStream,IOBasicTypes::LongFilePositionType inReadLimit,bool inOwnsStream);

	void Assign(IByteReader* inSourceStream,IOBasicTypes::LongFilePositionType inReadLimit,bool inOwnsStream);

	virtual IOBasicTypes::LongBufferSizeType Read(IOBasicTypes::Byte* inBuffer,IOBasicTypes::LongBufferSizeType inBufferSize);
	virtual bool NotEnded();

private:

	bool mOwnsStream;
	IByteReader* mStream;
	IOBasicTypes::LongFilePositionType mMoreToRead;
};
