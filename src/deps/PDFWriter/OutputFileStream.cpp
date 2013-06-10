/*
   Source File : OutputFileStream.cpp


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
#include "OutputFileStream.h"
#include "SafeBufferMacrosDefs.h"

using namespace IOBasicTypes;
using namespace PDFHummus;

OutputFileStream::OutputFileStream(void)
{
	mStream = NULL;
}

OutputFileStream::~OutputFileStream(void)
{
	if(mStream)
		Close();
}


OutputFileStream::OutputFileStream(const std::string& inFilePath,bool inAppend)
{
	mStream = NULL;
	Open(inFilePath,inAppend);
}

EStatusCode OutputFileStream::Open(const std::string& inFilePath,bool inAppend)
{
	SAFE_FOPEN(mStream,inFilePath.c_str(),inAppend ? "ab":"wb")

	if(!mStream)
		return PDFHummus::eFailure;

	// seek to end, so position reading gets the correct file position, even before first write
	SAFE_FSEEK64(mStream,0,SEEK_END);

	return PDFHummus::eSuccess;
};

EStatusCode OutputFileStream::Close()
{
	EStatusCode result = fclose(mStream) == 0 ? PDFHummus::eSuccess:PDFHummus::eFailure;

	mStream = NULL;
	return result;
}

LongBufferSizeType OutputFileStream::Write(const Byte* inBuffer,LongBufferSizeType inSize)
{

	LongBufferSizeType writtenItems = mStream ? fwrite(static_cast<const void*>(inBuffer),1,inSize,mStream):0;
	return writtenItems;
}

LongFilePositionType OutputFileStream::GetCurrentPosition()
{
	return mStream ? SAFE_FTELL64(mStream):0;
}