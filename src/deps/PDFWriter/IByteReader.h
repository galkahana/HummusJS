/*
   Source File : IByteReader.h


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
/*
	IByteReader. interface for reading bytes


*/

#include "IOBasicTypes.h"


class IByteReader 
{
public:
	virtual ~IByteReader(void){};

	/*
		Read inSize bytes of inBuffer, returning the number of bytes actually read
	*/
	virtual IOBasicTypes::LongBufferSizeType Read(IOBasicTypes::Byte* inBuffer,IOBasicTypes::LongBufferSizeType inBufferSize) = 0;

	/*
		boolean to determine whether reading will get more content or not. opposite of IsEOF. cause i'm sick of !IsEOF()s
	*/
	virtual bool NotEnded() = 0;

};
