/*
   Source File : TrueTypePrimitiveWriter.cpp


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
#include "TrueTypePrimitiveWriter.h"
#include "OutputStringBufferStream.h"

using namespace PDFHummus;

TrueTypePrimitiveWriter::TrueTypePrimitiveWriter(OutputStringBufferStream* inTrueTypeFile)
{
	SetOpenTypeStream(inTrueTypeFile);
}

TrueTypePrimitiveWriter::~TrueTypePrimitiveWriter(void)
{
}

void TrueTypePrimitiveWriter::SetOpenTypeStream(OutputStringBufferStream* inTrueTypeFile)
{
	mTrueTypeFile = inTrueTypeFile;
	mInternalState = (inTrueTypeFile ? PDFHummus::eSuccess:PDFHummus::eFailure);
}

EStatusCode TrueTypePrimitiveWriter::GetInternalState()
{
	return mInternalState;
}

EStatusCode TrueTypePrimitiveWriter::WriteBYTE(Byte inValue)
{
	if(PDFHummus::eFailure == mInternalState)
		return PDFHummus::eFailure;

	EStatusCode status = (mTrueTypeFile->Write(&inValue,1) == 1 ? PDFHummus::eSuccess : PDFHummus::eFailure);

	if(PDFHummus::eFailure == status)
		mInternalState = PDFHummus::eFailure;
	return status;	
}

EStatusCode TrueTypePrimitiveWriter::WriteULONG(unsigned long inValue)
{
	Byte byte1 = (inValue>>24) & 0xff;
	Byte byte2 = (inValue>>16) & 0xff;
	Byte byte3 = (inValue>>8) & 0xff;
	Byte byte4 = inValue & 0xff;

	if(WriteBYTE(byte1) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	if(WriteBYTE(byte2) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	if(WriteBYTE(byte3) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	if(WriteBYTE(byte4) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	return PDFHummus::eSuccess;
}

EStatusCode TrueTypePrimitiveWriter::WriteUSHORT(unsigned short inValue)
{	
	Byte byte1 = (inValue>>8) & 0xff;
	Byte byte2 = inValue & 0xff;
	
	if(WriteBYTE(byte1) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	if(WriteBYTE(byte2) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	return PDFHummus::eSuccess;
}

EStatusCode TrueTypePrimitiveWriter::WriteSHORT(short inValue)
{
	return WriteUSHORT((unsigned short)inValue);
}

EStatusCode TrueTypePrimitiveWriter::Pad(int inCount)
{

	for(int i=0; i <inCount && (mInternalState == PDFHummus::eSuccess); ++i)
		WriteBYTE(0);

	return mInternalState;
}

EStatusCode TrueTypePrimitiveWriter::PadTo4()
{
	int padSize = (4 - (mTrueTypeFile->GetCurrentPosition() % 4)) % 4;

	for(int i=0; i < padSize; ++i)
		WriteBYTE(0);
	return mInternalState;
}