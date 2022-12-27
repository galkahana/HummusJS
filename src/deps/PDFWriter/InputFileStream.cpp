/*
   Source File : InputFileStream.cpp


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
#include "InputFileStream.h"
#include "SafeBufferMacrosDefs.h"

using namespace PDFHummus;

InputFileStream::InputFileStream(void)
{
	mStream = NULL;
}

InputFileStream::~InputFileStream(void)
{
	if(mStream)
		Close();
}


InputFileStream::InputFileStream(const std::string& inFilePath)
{
	mStream = NULL;
	Open(inFilePath);
}

EStatusCode InputFileStream::Open(const std::string& inFilePath)
{
	SAFE_FOPEN(mStream,inFilePath.c_str(),"rb");
	return NULL == mStream ? PDFHummus::eFailure:PDFHummus::eSuccess;
}

EStatusCode InputFileStream::Close()
{
	EStatusCode result = fclose(mStream) == 0 ? PDFHummus::eSuccess:PDFHummus::eFailure;

	mStream = NULL;
	return result;
}

LongBufferSizeType InputFileStream::Read(Byte* inBuffer,LongBufferSizeType inBufferSize)
{
	LongBufferSizeType readItems = mStream ? fread(static_cast<void*>(inBuffer),1,inBufferSize,mStream):0;
	return readItems;
}

bool InputFileStream::NotEnded()
{
	if(mStream)
		return !feof(mStream);
	else
		return false;
}

void InputFileStream::Skip(LongBufferSizeType inSkipSize)
{
	if(mStream)
		SAFE_FSEEK64(mStream,inSkipSize,SEEK_CUR);
}

void InputFileStream::SetPosition(LongFilePositionType inOffsetFromStart)
{
	if(mStream)
		SAFE_FSEEK64(mStream,inOffsetFromStart,SEEK_SET);
}

LongFilePositionType InputFileStream::GetCurrentPosition()
{
	if(mStream)
		return SAFE_FTELL64(mStream);
	else
		return 0;
}

LongFilePositionType InputFileStream::GetFileSize()
{
	if(mStream)
	{
		// very messy...prefer a different means sometime
		LongFilePositionType currentPosition = SAFE_FTELL64(mStream);
		LongFilePositionType result;

		SAFE_FSEEK64(mStream,0,SEEK_END);
		result = SAFE_FTELL64(mStream);
		SAFE_FSEEK64(mStream,currentPosition,SEEK_SET);
		return result;
	}
	else
		return 0;
}

void InputFileStream::SetPositionFromEnd(LongFilePositionType inOffsetFromEnd)
{
	if(mStream)
	{
		// if failed, probaby means that seeks too much, so place at file begin.
		if(SAFE_FSEEK64(mStream,-inOffsetFromEnd,SEEK_END) != 0)
			SAFE_FSEEK64(mStream,0,SEEK_SET);
	}
}
