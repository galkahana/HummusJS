/*
   Source File : CFFPrimitiveReader.h


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
#include "DictOperand.h"

class CFFPrimitiveReader
{
public:
	CFFPrimitiveReader(IByteReaderWithPosition* inCFFFile = NULL);
	~CFFPrimitiveReader(void);


	void SetStream(IByteReaderWithPosition* inCFFFile);
	void SetOffset(LongFilePositionType inNewOffset);
	void Skip(LongBufferSizeType inToSkip);
	LongFilePositionType GetCurrentPosition();
	PDFHummus::EStatusCode GetInternalState();

	PDFHummus::EStatusCode ReadByte(Byte& outValue);
	PDFHummus::EStatusCode Read(Byte* ioBuffer,LongBufferSizeType inBufferSize);

	// basic CFF values
	PDFHummus::EStatusCode ReadCard8(Byte& outValue);
	PDFHummus::EStatusCode ReadCard16(unsigned short& outValue);
	// set offsize to read offsets
	void SetOffSize(Byte inOffSize);
	PDFHummus::EStatusCode ReadOffset(unsigned long& outValue);
	PDFHummus::EStatusCode ReadOffSize(Byte& outValue);
	PDFHummus::EStatusCode ReadSID(unsigned short& outValue);

	// dict data
	bool IsDictOperator(Byte inCandidate);
	PDFHummus::EStatusCode ReadDictOperator(Byte inFirstByte,unsigned short& outOperator);
	PDFHummus::EStatusCode ReadDictOperand(Byte inFirstByte,DictOperand& outOperand);

private:
	IByteReaderWithPosition* mCFFFile;
	LongFilePositionType mInitialPosition;
	PDFHummus::EStatusCode mInternalState;
	Byte mCurrentOffsize;

	PDFHummus::EStatusCode Read3ByteUnsigned(unsigned long& outValue);
	PDFHummus::EStatusCode Read4ByteUnsigned(unsigned long& outValue);
	PDFHummus::EStatusCode Read4ByteSigned(long& outValue);
	PDFHummus::EStatusCode Read2ByteSigned(short& outValue);
	PDFHummus::EStatusCode ReadIntegerOperand(Byte inFirstByte,long& outValue);
	// make sure you get here after discarding the initial byte, 30.
	PDFHummus::EStatusCode ReadRealOperand(double& outValue,long& outRealValueFractalEnd);
};
