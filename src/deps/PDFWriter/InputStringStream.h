/*
Source File : InputStringStream.h


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
#pragma once

#include "IByteReaderWithPosition.h"

#include <string>

class InputStringStream : public IByteReaderWithPosition
{
public:
	InputStringStream();
	InputStringStream(const std::string& inString);
	~InputStringStream(void);

	void Assign(const std::string& inString);

	// IByteReaderWithPosition implementation
	virtual LongBufferSizeType Read(IOBasicTypes::Byte* inBuffer, LongBufferSizeType inBufferSize);
	virtual bool NotEnded();
	virtual void Skip(LongBufferSizeType inSkipSize);
	virtual void SetPosition(LongFilePositionType inOffsetFromStart);
	virtual void SetPositionFromEnd(LongFilePositionType inOffsetFromEnd);
	virtual LongFilePositionType GetCurrentPosition();

private:

	std::string::const_iterator mStartPosition;
	std::string::const_iterator mEndPosition;
	std::string::const_iterator mCurrentPosition;

};
