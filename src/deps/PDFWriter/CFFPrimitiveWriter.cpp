/*
 Source File : CFFPrimitiveWriter.cpp
 
 
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
#include "CFFPrimitiveWriter.h"
#include <math.h>


using namespace PDFHummus;

CFFPrimitiveWriter::CFFPrimitiveWriter(IByteWriter* inCFFOutput)
{
	SetStream(inCFFOutput);
}

CFFPrimitiveWriter::~CFFPrimitiveWriter(void)
{
}

void CFFPrimitiveWriter::SetStream(IByteWriter* inCFFOutput)
{
	mCFFOutput = inCFFOutput;
	if(inCFFOutput)
	{
		mCurrentOffsize = 1;
		mInternalState = PDFHummus::eSuccess;
	}
	else
	{
		mInternalState = PDFHummus::eFailure;
	}
}

EStatusCode CFFPrimitiveWriter::GetInternalState()
{
	return mInternalState;
}

EStatusCode CFFPrimitiveWriter::WriteByte(Byte inValue)
{
	if(PDFHummus::eFailure == mInternalState)
		return PDFHummus::eFailure;
    
	EStatusCode status = (mCFFOutput->Write(&inValue,1) == 1 ? PDFHummus::eSuccess : PDFHummus::eFailure);
    
	if(PDFHummus::eFailure == status)
		mInternalState = PDFHummus::eFailure;
	return status;
}

EStatusCode CFFPrimitiveWriter::Write(const Byte* inBuffer,LongBufferSizeType inBufferSize)
{
	if(PDFHummus::eFailure == mInternalState)
		return PDFHummus::eFailure;
    
	EStatusCode status = (mCFFOutput->Write(inBuffer,inBufferSize) == inBufferSize ? PDFHummus::eSuccess : PDFHummus::eFailure);
    
	if(PDFHummus::eFailure == status)
		mInternalState = PDFHummus::eFailure;
	return status;
}

EStatusCode CFFPrimitiveWriter::WriteCard8(Byte inValue)
{
	return WriteByte(inValue);
}

EStatusCode CFFPrimitiveWriter::WriteCard16(unsigned short inValue)
{
	Byte byte1 = (inValue>>8) & 0xff;
	Byte byte2 = inValue & 0xff;
    
	if(WriteByte(byte1) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;
    
	if(WriteByte(byte2) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;
    
	return PDFHummus::eSuccess;
}

void CFFPrimitiveWriter::SetOffSize(Byte inOffSize)
{
	mCurrentOffsize = inOffSize;
}

EStatusCode CFFPrimitiveWriter::WriteOffset(unsigned long inValue)
{
	EStatusCode status = PDFHummus::eFailure;
    
	switch(mCurrentOffsize)
	{
		case 1:
			status = WriteCard8((Byte)inValue);
			break;
		case 2:
			status = WriteCard16((unsigned short)inValue);
			break;
		case 3:
			status = Write3ByteUnsigned(inValue);
			break;
		case 4:
			status = Write4ByteUnsigned(inValue);
			break;
            
	}
	return status;
}

EStatusCode CFFPrimitiveWriter::Write3ByteUnsigned(unsigned long inValue)
{
	Byte byte1 = (inValue>>16) & 0xff;
	Byte byte2 = (inValue>>8) & 0xff;
	Byte byte3 = inValue & 0xff;
    
	if(WriteByte(byte1) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;
    
	if(WriteByte(byte2) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;
    
	if(WriteByte(byte3) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;
    
	return PDFHummus::eSuccess;
}

EStatusCode CFFPrimitiveWriter::Write4ByteUnsigned(unsigned long inValue)
{
	Byte byte1 = (inValue>>24) & 0xff;
	Byte byte2 = (inValue>>16) & 0xff;
	Byte byte3 = (inValue>>8) & 0xff;
	Byte byte4 = inValue & 0xff;
    
	if(WriteByte(byte1) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;
    
	if(WriteByte(byte2) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;
    
	if(WriteByte(byte3) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;
    
	if(WriteByte(byte4) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;
    
	return PDFHummus::eSuccess;
}



EStatusCode CFFPrimitiveWriter::WriteOffSize(Byte inValue)
{
	return WriteCard8(inValue);
}

EStatusCode CFFPrimitiveWriter::WriteSID(unsigned short inValue)
{
	return WriteCard16(inValue);
}

EStatusCode CFFPrimitiveWriter::WriteDictOperator(unsigned short inOperator)
{
	if(((inOperator >> 8)  & 0xff) == 12)
		return WriteCard16(inOperator);
	else
		return WriteCard8((Byte)(inOperator & 0xff));
}

EStatusCode CFFPrimitiveWriter::WriteDictOperand(const DictOperand& inOperand)
{
	if(inOperand.IsInteger)
		return WriteIntegerOperand(inOperand.IntegerValue);
	else
		return WriteRealOperand(inOperand.RealValue,inOperand.RealValueFractalEnd);
}

EStatusCode CFFPrimitiveWriter::WriteDictItems(unsigned short inOperator,
											   const DictOperandList& inOperands)
{
	EStatusCode status = PDFHummus::eSuccess;
	DictOperandList::const_iterator it = inOperands.begin();
    
	for(; it != inOperands.end() && PDFHummus::eSuccess == status; ++it)
		status = WriteDictOperand(*it);
	if(PDFHummus::eSuccess == status)
		status = WriteDictOperator(inOperator);
    
	return status;
}


EStatusCode CFFPrimitiveWriter::WriteIntegerOperand(long inValue)
{
	if(-107 <= inValue && inValue <= 107)
	{
		return WriteByte((Byte)(inValue + 139));
	}
	else if(108 <= inValue && inValue <= 1131)
	{
		Byte byte0,byte1;
        
		inValue-=108;
		byte0 = ((inValue >> 8) & 0xff) + 247;
		byte1 = inValue & 0xff;
        
		if(WriteByte(byte0) != PDFHummus::eSuccess)
			return PDFHummus::eFailure;
        
		if(WriteByte(byte1) != PDFHummus::eSuccess)
			return PDFHummus::eFailure;
	}
	else if(-1131 <= inValue && inValue <= -108)
	{
		Byte byte0,byte1;
        
		inValue = -(inValue + 108);
        
		byte0 = ((inValue >> 8) & 0xff) + 251;
		byte1 = inValue & 0xff;
        
		if(WriteByte(byte0) != PDFHummus::eSuccess)
			return PDFHummus::eFailure;
        
		if(WriteByte(byte1) != PDFHummus::eSuccess)
			return PDFHummus::eFailure;
	}
	else if(-32768 <= inValue && inValue<= 32767)
	{
		Byte byte1,byte2;
        
		byte1 = (inValue >> 8) & 0xff;
		byte2 = inValue & 0xff;
        
		if(WriteByte(28) != PDFHummus::eSuccess)
			return PDFHummus::eFailure;
        
		if(WriteByte(byte1) != PDFHummus::eSuccess)
			return PDFHummus::eFailure;
        
		if(WriteByte(byte2) != PDFHummus::eSuccess)
			return PDFHummus::eFailure;
	}
	else //  -2^31 <= inValue <= 2^31 - 1
	{
		return Write5ByteDictInteger(inValue);
	}
	return PDFHummus::eSuccess;
}

EStatusCode CFFPrimitiveWriter::Write5ByteDictInteger(long inValue)
{
	Byte byte1,byte2,byte3,byte4;
    
	byte1 = (inValue >> 24) & 0xff;
	byte2 = (inValue >> 16)& 0xff;
	byte3 = (inValue >> 8) & 0xff;
	byte4 = inValue & 0xff;
    
	if(WriteByte(29) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;
    
	if(WriteByte(byte1) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;
    
	if(WriteByte(byte2) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;
    
	if(WriteByte(byte3) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;
    
	if(WriteByte(byte4) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;
    
	return PDFHummus::eSuccess;
}


EStatusCode CFFPrimitiveWriter::WriteRealOperand(double inValue,long inFractalLength)
{
	// first, calculate the proper formatting
    
	bool minusSign = inValue < 0;
	bool minusExponent = false;
	bool plusExponent = false;
	unsigned short exponentSize = 0;
    
	if(minusSign)
		inValue = -inValue;
    
	double integerValue = floor(inValue);
	double fractalValue = inValue - integerValue;
    
	if(0 == fractalValue)
	{
		if(long(integerValue) % 1000 == 0 && integerValue >= 1000) // bother only if larger than 1000
		{
			plusExponent = true;
			while(long(integerValue) % 10 == 0)
			{
				++exponentSize;
				integerValue = integerValue / 10;
			}
		}
	}
    else if(0 == integerValue)
	{
		if(fractalValue <= 0.001) // bother only if < 0.001
		{
			minusExponent = true;
			while(fractalValue < 0.1)
			{
				++exponentSize;
				fractalValue = fractalValue * 10;
			}
		}
	}
    
	// now let's get to work
	if(WriteByte(30) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;
    
	// first, take care of minus sign
	Byte buffer = minusSign ? 0xe0 : 0;
	bool usedFirst = minusSign;
    
	// Integer part
	if(integerValue != 0)
	{
	        if(WriteIntegerOfReal(integerValue,buffer,usedFirst) != PDFHummus::eSuccess)
			return PDFHummus::eFailure;
        }
	else
	{
		if(SetOrWriteNibble(0,buffer,usedFirst) != PDFHummus::eSuccess)
			return PDFHummus::eFailure;
	}

	// Fractal part (if there was an integer or not)
	if(fractalValue != 0 && inFractalLength > 0)
	{
		if(SetOrWriteNibble(0xa,buffer,usedFirst) != PDFHummus::eSuccess)
			return PDFHummus::eFailure;

		while(fractalValue != 0 && inFractalLength > 0)
		{
			if(SetOrWriteNibble((Byte)floor(fractalValue * 10),buffer,usedFirst) != PDFHummus::eSuccess)
				return PDFHummus::eFailure;
			fractalValue = fractalValue * 10 - floor(fractalValue * 10);
			--inFractalLength;
		}
	}
    
	// now, if there's any exponent, write it
	if(minusExponent)
	{
		if(SetOrWriteNibble(0xc,buffer,usedFirst) != PDFHummus::eSuccess)
			return PDFHummus::eFailure;
		if(WriteIntegerOfReal(exponentSize,buffer,usedFirst) != PDFHummus::eSuccess)
			return PDFHummus::eFailure;
	}
	if(plusExponent)
	{
		if(SetOrWriteNibble(0xb,buffer,usedFirst) != PDFHummus::eSuccess)
			return PDFHummus::eFailure;
		if(WriteIntegerOfReal(exponentSize,buffer,usedFirst) != PDFHummus::eSuccess)
			return PDFHummus::eFailure;
	}
    
	// final f or ff
	if(usedFirst)
		return SetOrWriteNibble(0xf,buffer,usedFirst);
	else
		return WriteByte(0xff);
    
}

EStatusCode CFFPrimitiveWriter::SetOrWriteNibble(Byte inValue,Byte& ioBuffer,bool& ioUsedFirst)
{
	EStatusCode status = PDFHummus::eSuccess;
	if(ioUsedFirst)
	{
		ioBuffer|= inValue;
		status = WriteByte(ioBuffer);
		ioBuffer = 0;
		ioUsedFirst = false;
	}
	else
	{
		ioBuffer = (inValue << 4) & 0xf0;
		ioUsedFirst = true;
	}
	return status;
}

EStatusCode CFFPrimitiveWriter::WriteIntegerOfReal(double inIntegerValue,
												   Byte& ioBuffer,
												   bool& ioUsedFirst)
{
	if(0 == inIntegerValue)
		return PDFHummus::eSuccess;
    
	EStatusCode status = WriteIntegerOfReal(floor(inIntegerValue/10),ioBuffer,ioUsedFirst);
	if(status != PDFHummus::eSuccess)
		return PDFHummus::eFailure;
    
	return SetOrWriteNibble((Byte)(long(inIntegerValue) % 10),ioBuffer,ioUsedFirst);
}

Byte BytesPad5[5] = {'0','0','0','0','0'};
EStatusCode CFFPrimitiveWriter::Pad5Bytes()
{
	return Write(BytesPad5,5);
}

Byte BytePad[1] = {'0'};
EStatusCode CFFPrimitiveWriter::PadNBytes(unsigned short inBytesToPad)
{
	EStatusCode status = PDFHummus::eSuccess;
    
	for(unsigned short i=0;i<inBytesToPad && PDFHummus::eSuccess == status;++i)
		Write(BytePad,1);
	return status;
}


