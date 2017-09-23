/*
   Source File : OutputStreamTraits.cpp


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
#include "OutputStreamTraits.h"
#include "IByteWriter.h"
#include "IByteReaderWithPosition.h"

using namespace PDFHummus;

OutputStreamTraits::OutputStreamTraits(IByteWriter* inOutputStream)
{
	mOutputStream = inOutputStream;
}

OutputStreamTraits::~OutputStreamTraits(void)
{
}

#define TENMEGS 10*1024*1024
EStatusCode OutputStreamTraits::CopyToOutputStream(IByteReader* inInputStream)
{
	Byte* buffer = new Byte[TENMEGS];
	LongBufferSizeType readBytes,writeBytes;
	EStatusCode status = PDFHummus::eSuccess;

	while(inInputStream->NotEnded() && PDFHummus::eSuccess == status)
	{
		readBytes = inInputStream->Read(buffer,TENMEGS);
		writeBytes = mOutputStream->Write(buffer,readBytes);
		status = (readBytes == writeBytes) ? PDFHummus::eSuccess:PDFHummus::eFailure;
		if (readBytes == 0) {
			break; // for whatever reason notEnded is not reached...dont want this to interfere
		}
	}
	delete[] buffer;
	return status;
}

EStatusCode OutputStreamTraits::CopyToOutputStream(IByteReader* inInputStream,LongBufferSizeType inLength)
{
	Byte* buffer = new Byte[inLength];
	LongBufferSizeType readBytes,writeBytes;

	readBytes = inInputStream->Read(buffer,inLength);
	writeBytes = mOutputStream->Write(buffer,readBytes);
	EStatusCode status = (readBytes == writeBytes) ? PDFHummus::eSuccess:PDFHummus::eFailure;
	delete[] buffer;
	return status;
}