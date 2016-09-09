/*
   Source File : CFFPrimitiveWriter.h


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
#include "IByteWriter.h"
#include "DictOperand.h"

using namespace IOBasicTypes;

class CFFPrimitiveWriter
{
public:
	CFFPrimitiveWriter(IByteWriter* inCFFOutput = NULL);
	~CFFPrimitiveWriter(void);

	void SetStream(IByteWriter* inCFFOutput);

	PDFHummus::EStatusCode GetInternalState();

	PDFHummus::EStatusCode WriteByte(Byte inValue);
	PDFHummus::EStatusCode Write(const Byte* inBuffer,LongBufferSizeType inBufferSize);

	// basic CFF values
	PDFHummus::EStatusCode WriteCard8(Byte inValue);
	PDFHummus::EStatusCode WriteCard16(unsigned short inValue);
	// set offsize to write offsets
	void SetOffSize(Byte inOffSize);
	PDFHummus::EStatusCode WriteOffset(unsigned long inValue);
	PDFHummus::EStatusCode WriteOffSize(Byte inValue);
	PDFHummus::EStatusCode WriteSID(unsigned short inValue);

	// dict data
	PDFHummus::EStatusCode WriteDictOperator(unsigned short inOperator);
	PDFHummus::EStatusCode WriteDictOperand(const DictOperand& inOperand);
	PDFHummus::EStatusCode WriteDictItems(unsigned short inOperator,const DictOperandList& inOperands);
	PDFHummus::EStatusCode WriteIntegerOperand(long inValue);
	PDFHummus::EStatusCode Write5ByteDictInteger(long inValue);
	PDFHummus::EStatusCode WriteRealOperand(double inValue,long inFractalLength=10);

	PDFHummus::EStatusCode Pad5Bytes();
	PDFHummus::EStatusCode PadNBytes(unsigned short inBytesToPad);

private:
	IByteWriter* mCFFOutput;
	PDFHummus::EStatusCode mInternalState;
	Byte mCurrentOffsize;

	PDFHummus::EStatusCode Write3ByteUnsigned(unsigned long inValue);
	PDFHummus::EStatusCode Write4ByteUnsigned(unsigned long inValue);
	PDFHummus::EStatusCode WriteIntegerOfReal(double inIntegerValue,Byte& ioBuffer,bool& ioUsedFirst);
	PDFHummus::EStatusCode SetOrWriteNibble(Byte inValue,Byte& ioBuffer,bool& ioUsedFirst);

};
