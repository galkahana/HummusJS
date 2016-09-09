/*
   Source File : OpenTypePrimitiveReader.cpp


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
#include "OpenTypePrimitiveReader.h"

using namespace PDFHummus;

OpenTypePrimitiveReader::OpenTypePrimitiveReader(IByteReaderWithPosition* inOpenTypeFile)
{
	SetOpenTypeStream(inOpenTypeFile);
}

OpenTypePrimitiveReader::~OpenTypePrimitiveReader(void)
{
}

void OpenTypePrimitiveReader::SetOpenTypeStream(IByteReaderWithPosition* inOpenTypeFile)
{
	mOpenTypeFile = inOpenTypeFile;
	if(inOpenTypeFile)
	{
		mInitialPosition = inOpenTypeFile->GetCurrentPosition();
		mInternalState = PDFHummus::eSuccess;
	}
	else
	{
		mInternalState = PDFHummus::eFailure;
	}
}

EStatusCode OpenTypePrimitiveReader::ReadBYTE(unsigned char& outValue)
{
	if(PDFHummus::eFailure == mInternalState)
		return PDFHummus::eFailure;

	Byte buffer;
	EStatusCode status = (mOpenTypeFile->Read(&buffer,1) == 1 ? PDFHummus::eSuccess : PDFHummus::eFailure);

	if(PDFHummus::eFailure == status)
		mInternalState = PDFHummus::eFailure;
	outValue = buffer;
	return status;
}

EStatusCode OpenTypePrimitiveReader::ReadCHAR(char& outValue)
{
	Byte buffer;

	if(ReadBYTE(buffer) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	outValue = (char)buffer;
	return PDFHummus::eSuccess;
}

EStatusCode OpenTypePrimitiveReader::ReadUSHORT(unsigned short& outValue)
{
	Byte byte1,byte2;

	if(ReadBYTE(byte1) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	if(ReadBYTE(byte2) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;


	outValue = ((unsigned short)byte1 << 8) + byte2;

	return PDFHummus::eSuccess;

}

EStatusCode OpenTypePrimitiveReader::ReadSHORT(short& outValue)
{
	unsigned short buffer;

	if(ReadUSHORT(buffer) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	outValue = (short)buffer;
	return PDFHummus::eSuccess;

}

EStatusCode OpenTypePrimitiveReader::ReadULONG(unsigned long& outValue)
{
	Byte byte1,byte2,byte3,byte4;

	if(ReadBYTE(byte1) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	if(ReadBYTE(byte2) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	if(ReadBYTE(byte3) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	if(ReadBYTE(byte4) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	outValue = ((unsigned long)byte1 << 24) + ((unsigned long)byte2 << 16) + 
									((unsigned long)byte3 << 8) + byte4;

	return PDFHummus::eSuccess;
}

EStatusCode OpenTypePrimitiveReader::ReadLONG(long& outValue)
{
	unsigned long buffer;

	if(ReadULONG(buffer) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	outValue = (long)buffer;
	return PDFHummus::eSuccess;
}

EStatusCode OpenTypePrimitiveReader::ReadLongDateTime(long long& outValue)
{
	Byte byte1,byte2,byte3,byte4,byte5,byte6,byte7,byte8;

	if(ReadBYTE(byte1) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	if(ReadBYTE(byte2) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	if(ReadBYTE(byte3) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	if(ReadBYTE(byte4) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	if(ReadBYTE(byte5) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	if(ReadBYTE(byte6) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	if(ReadBYTE(byte7) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	if(ReadBYTE(byte8) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	outValue =(long long)(	((unsigned long long)byte1 << 56) + ((unsigned long long)byte2 << 48) + 
							((unsigned long long)byte3 << 40) + ((unsigned long long)byte4 << 32) + 
							((unsigned long long)byte5 << 24) + ((unsigned long long)byte6 << 16) + 
							((unsigned long long)byte7 << 8) + byte8);
	return PDFHummus::eSuccess;	
}

void OpenTypePrimitiveReader::SetOffset(LongFilePositionType inNewOffset)
{
	if(mInternalState != PDFHummus::eFailure)
		mOpenTypeFile->SetPosition(mInitialPosition + inNewOffset);
}	

void OpenTypePrimitiveReader::Skip(LongBufferSizeType inToSkip)
{
	if(mInternalState != PDFHummus::eFailure)
		mOpenTypeFile->Skip(inToSkip);
}

EStatusCode OpenTypePrimitiveReader::GetInternalState()
{
	return mInternalState;
}

EStatusCode OpenTypePrimitiveReader::ReadFixed(double& outValue)
{
	unsigned short integer,fraction;

	if(ReadUSHORT(integer) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	if(ReadUSHORT(fraction) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	outValue = (double)integer + ((double)fraction) / (1<<16);

	return PDFHummus::eSuccess;
}

LongFilePositionType OpenTypePrimitiveReader::GetCurrentPosition()
{
	if(mInternalState != PDFHummus::eFailure)
		return mOpenTypeFile->GetCurrentPosition() - mInitialPosition;	
	else
		return 0;
}

EStatusCode OpenTypePrimitiveReader::Read(Byte* inBuffer,LongBufferSizeType inBufferSize)
{
	if(PDFHummus::eFailure == mInternalState)
		return PDFHummus::eFailure;

	EStatusCode status = (mOpenTypeFile->Read(inBuffer,inBufferSize) == inBufferSize ? PDFHummus::eSuccess : PDFHummus::eFailure);

	if(PDFHummus::eFailure == status)
		mInternalState = PDFHummus::eFailure;
	return status;	
}

IByteReaderWithPosition* OpenTypePrimitiveReader::GetReadStream()
{
	return mOpenTypeFile;
}