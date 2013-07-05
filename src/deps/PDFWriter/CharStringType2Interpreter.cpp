/*
   Source File : CharStringType2Interpreter.cpp


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
#include "CharStringType2Interpreter.h"
#include "Trace.h"
#include <math.h>
#include <stdlib.h>
 
using namespace PDFHummus;


CharStringType2Interpreter::CharStringType2Interpreter(void)
{
	mImplementationHelper = NULL;
}

CharStringType2Interpreter::~CharStringType2Interpreter(void)
{
}


EStatusCode CharStringType2Interpreter::Intepret(const CharString& inCharStringToIntepret, IType2InterpreterImplementation* inImplementationHelper)
{
	Byte* charString = NULL;
	EStatusCode status;

	do
	{
		mImplementationHelper = inImplementationHelper;
		mGotEndChar = false;
		mStemsCount = 0;
		mCheckedWidth = false;
		if(!inImplementationHelper)
		{
			TRACE_LOG("CharStringType2Interpreter::Intepret, null implementation helper passed. pass a proper pointer!!");
			status = PDFHummus::eFailure;
			break;
		}

		status = mImplementationHelper->ReadCharString(inCharStringToIntepret.mStartPosition,inCharStringToIntepret.mEndPosition,&charString);	
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG2("CharStringType2Interpreter::Intepret, failed to read charstring starting in %lld and ending in %lld",inCharStringToIntepret.mStartPosition,inCharStringToIntepret.mEndPosition);
			break;
		}
		
		status = ProcessCharString(charString,inCharStringToIntepret.mEndPosition - inCharStringToIntepret.mStartPosition);

	}while(false);

	delete charString;

	return status;
}

EStatusCode CharStringType2Interpreter::ProcessCharString(Byte* inCharString,LongFilePositionType inCharStringLength)
{
	EStatusCode status = PDFHummus::eSuccess;
	Byte* pointer = inCharString;
	bool gotEndExecutionOperator = false;

	while(pointer - inCharString < inCharStringLength &&
			PDFHummus::eSuccess == status && 
			!gotEndExecutionOperator &&
			!mGotEndChar)
	{
		if(IsOperator(pointer))
		{
			pointer = InterpretOperator(pointer,gotEndExecutionOperator);
			if(!pointer)
				status = PDFHummus::eFailure;
		}
		else
		{
			pointer = InterpretNumber(pointer);
			if(!pointer)
				status = PDFHummus::eFailure;
		}
	}
	return status;
}

bool CharStringType2Interpreter::IsOperator(Byte* inProgramCounter)
{
	return  ((*inProgramCounter) <= 27) || 
			(29 <= (*inProgramCounter) && (*inProgramCounter) <= 31);
			
}


Byte* CharStringType2Interpreter::InterpretNumber(Byte* inProgramCounter)
{
	CharStringOperand operand;
	Byte* newPosition = inProgramCounter;

	if(28 == *newPosition)
	{
		operand.IsInteger = true;
		operand.IntegerValue = (short)(
									((unsigned short)(*(newPosition+1)) << 8) + (*(newPosition+2)));
		newPosition += 3;
	} 
	else if(32 <= *newPosition && *newPosition <= 246)
	{
		operand.IsInteger = true;
		operand.IntegerValue = (short)*newPosition - 139;
		++newPosition;
	}
	else if(247 <= *newPosition && *newPosition <= 250)
	{
		operand.IsInteger = true;
		operand.IntegerValue = (*newPosition - 247) * 256 + *(newPosition + 1) + 108;
		newPosition += 2;
	}
	else if(251 <= *newPosition && *newPosition <= 254)
	{
		operand.IsInteger = true;
		operand.IntegerValue = -(short)(*newPosition - 251) * 256 - *(newPosition + 1) - 108;
		newPosition += 2;
	}
	else if(255 == *newPosition)
	{
		operand.IsInteger = false;
		operand.RealValue = (short)(((unsigned short)(*(newPosition+1)) << 8) + (*(newPosition+2)));
			if(operand.RealValue > 0)
				operand.RealValue+=
					(double)(((unsigned short)(*(newPosition+3)) << 8) + (*(newPosition+4))) /
								(1 << 8);
			else
				operand.RealValue-=
					(double)(((unsigned short)(*(newPosition+3)) << 8) + (*(newPosition+4))) /
								(1 << 8);
		newPosition += 5;
	}
	else
		newPosition = NULL; // error

	if(newPosition)
	{
		mOperandStack.push_back(operand);
		EStatusCode status = mImplementationHelper->Type2InterpretNumber(operand);
		if(status != PDFHummus::eSuccess)
			return NULL;

	}

	return newPosition;
}

Byte* CharStringType2Interpreter::InterpretOperator(Byte* inProgramCounter,bool& outGotEndExecutionCommand)
{
	unsigned short operatorValue;
	Byte* newPosition = inProgramCounter;
	outGotEndExecutionCommand = false;
	
	if(12 == *newPosition)
	{
		operatorValue = 0x0c00 + *(newPosition + 1);
		newPosition+=2;
	}
	else
	{
		operatorValue = *newPosition;
		++newPosition;
	}

	switch(operatorValue)
	{
		case 1: // hstem
			CheckWidth();
			newPosition = InterpretHStem(newPosition);
			break;
		case 3: // vstem
			CheckWidth();
			newPosition = InterpretVStem(newPosition);
			break;
		case 4: // vmoveto
			CheckWidth();
			newPosition = InterpretVMoveto(newPosition);
			break;
		case 5: // rlineto
			newPosition = InterpretRLineto(newPosition);
			break;
		case 6: // hlineto
			newPosition = InterpretHLineto(newPosition);
			break;
		case 7: // vlineto
			newPosition = InterpretVLineto(newPosition);
			break;
		case 8: // rrcurveto
			newPosition = InterpretRRCurveto(newPosition);
			break;
		case 10: // callsubr
			newPosition = InterpretCallSubr(newPosition);
			break;
		case 11: // return
			newPosition = InterpretReturn(newPosition);
			outGotEndExecutionCommand = true;
			break;
		case 14: // endchar
			CheckWidth();
			newPosition = InterpretEndChar(newPosition);
			break;
		case 18: // hstemhm
			CheckWidth();
			newPosition = InterpretHStemHM(newPosition);
			break;
		case 19: // hintmask
			CheckWidth();
			newPosition = InterpretHintMask(newPosition);
			break;
		case 20: // cntrmask
			CheckWidth();
			newPosition = InterpretCntrMask(newPosition);
			break;
		case 21: // rmoveto
			CheckWidth();
			newPosition = InterpretRMoveto(newPosition);
			break;
		case 22: // hmoveto
			CheckWidth();
			newPosition = InterpretHMoveto(newPosition);
			break;
		case 23: // vstemhm
			CheckWidth();
			newPosition = InterpretVStemHM(newPosition);
			break;
		case 24: // rcurveline
			newPosition = InterpretRCurveLine(newPosition);
			break;
		case 25: // rlinecurve
			newPosition = InterpretRLineCurve(newPosition);
			break;
		case 26: // vvcurveto
			newPosition = InterpretVVCurveto(newPosition);
			break;
		case 27: // hhcurveto
			newPosition = InterpretHHCurveto(newPosition);
			break;
		case 29: // callgsubr
			newPosition = InterpretCallGSubr(newPosition);
			break;
		case 30: // vhcurveto
			newPosition = InterpretVHCurveto(newPosition);
			break;
		case 31: // hvcurveto
			newPosition = InterpretHVCurveto(newPosition);
			break;
		
		case 0x0c00: // dotsection, depracated
			// ignore
			break;
		case 0x0c03: // and
			newPosition = InterpretAnd(newPosition);
			break;
		case 0x0c04: // or
			newPosition = InterpretOr(newPosition);
			break;
		case 0x0c05: // not
			newPosition = InterpretNot(newPosition);
			break;
		case 0x0c09: // abs
			newPosition = InterpretAbs(newPosition);
			break;
		case 0x0c0a: // add
			newPosition = InterpretAdd(newPosition);
			break;
		case 0x0c0b: // sub
			newPosition = InterpretSub(newPosition);
			break;
		case 0x0c0c: // div
			newPosition = InterpretDiv(newPosition);
			break;
		case 0x0c0e: // neg
			newPosition = InterpretNeg(newPosition);
			break;
		case 0x0c0f: // eq
			newPosition = InterpretEq(newPosition);
			break;
		case 0x0c12: // drop
			newPosition = InterpretDrop(newPosition);
			break;
		case 0x0c14: // put
			newPosition = InterpretPut(newPosition);
			break;
		case 0x0c15: // get
			newPosition = InterpretGet(newPosition);
			break;
		case 0x0c16: // ifelse
			newPosition = InterpretIfelse(newPosition);
			break;
		case 0x0c17: // random
			newPosition = InterpretRandom(newPosition);
			break;
		case 0x0c18: // mul
			newPosition = InterpretMul(newPosition);
			break;
		case 0x0c1a: // sqrt
			newPosition = InterpretSqrt(newPosition);
			break;
		case 0x0c1b: // dup
			newPosition = InterpretDup(newPosition);
			break;
		case 0x0c1c: // exch
			newPosition = InterpretExch(newPosition);
			break;
		case 0x0c1d: // index
			newPosition = InterpretIndex(newPosition);
			break;
		case 0x0c1e: // roll
			newPosition = InterpretRoll(newPosition);
			break;
		case 0x0c22: // hflex
			newPosition = InterpretHFlex(newPosition);
			break;
		case 0x0c23: // flex
			newPosition = InterpretFlex(newPosition);
			break;
		case 0x0c24: // hflex1
			newPosition = InterpretHFlex1(newPosition);
			break;
		case 0x0c25: // flex1
			newPosition = InterpretFlex1(newPosition);
			break;
	}
	return newPosition;
}

void CharStringType2Interpreter::CheckWidth()
{
	if(!mCheckedWidth)
	{
		if(mOperandStack.size() % 2 != 0) // has width
			mOperandStack.pop_front();
		mCheckedWidth = true;
	}
}

Byte* CharStringType2Interpreter::InterpretHStem(Byte* inProgramCounter)
{
	mStemsCount+= (unsigned short)(mOperandStack.size() / 2);

	EStatusCode status = mImplementationHelper->Type2Hstem(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

EStatusCode CharStringType2Interpreter::ClearNFromStack(unsigned short inCount)
{
	if(mOperandStack.size() >= inCount)
	{
		for(unsigned short i=0;i<inCount;++i)
			mOperandStack.pop_back();
		return PDFHummus::eSuccess;
	}
	else
		return PDFHummus::eFailure;
}

void CharStringType2Interpreter::ClearStack()
{
	mOperandStack.clear();
}

Byte* CharStringType2Interpreter::InterpretVStem(Byte* inProgramCounter)
{
	mStemsCount+= (unsigned short)(mOperandStack.size() / 2);

	EStatusCode status = mImplementationHelper->Type2Vstem(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretVMoveto(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Vmoveto(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
	
}

Byte* CharStringType2Interpreter::InterpretRLineto(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Rlineto(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretHLineto(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Hlineto(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretVLineto(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Vlineto(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretRRCurveto(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2RRCurveto(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretCallSubr(Byte* inProgramCounter)
{
	CharString* aCharString = NULL;
	aCharString = mImplementationHelper->GetLocalSubr(mOperandStack.back().IntegerValue);
	mOperandStack.pop_back();

	if(aCharString != NULL)
	{
		Byte* charString = NULL;
		EStatusCode status = mImplementationHelper->ReadCharString(aCharString->mStartPosition,aCharString->mEndPosition,&charString);	
		
		do
		{
			if(status != PDFHummus::eSuccess)
			{
				TRACE_LOG2("CharStringType2Interpreter::InterpretCallSubr, failed to read charstring starting in %lld and ending in %lld",aCharString->mStartPosition,aCharString->mEndPosition);
				break;
			}
			
			status = ProcessCharString(charString,aCharString->mEndPosition - aCharString->mStartPosition);
		}while(false);

		delete charString;
		if(status != PDFHummus::eSuccess)
			return NULL;
		else
			return inProgramCounter;
	}
	else
	{
		return NULL;
	}
}

Byte* CharStringType2Interpreter::InterpretReturn(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Return(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretEndChar(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Endchar(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	mGotEndChar = true;
	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretHStemHM(Byte* inProgramCounter)
{
	mStemsCount+= (unsigned short)(mOperandStack.size() / 2);

	EStatusCode status = mImplementationHelper->Type2Hstemhm(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretHintMask(Byte* inProgramCounter)
{
	mStemsCount+= (unsigned short)(mOperandStack.size() / 2);

	EStatusCode status = mImplementationHelper->Type2Hintmask(mOperandStack,inProgramCounter);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter+(mStemsCount/8 + (mStemsCount % 8 != 0 ? 1:0));
}

Byte* CharStringType2Interpreter::InterpretCntrMask(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Cntrmask(mOperandStack,inProgramCounter);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter+(mStemsCount/8 + (mStemsCount % 8 != 0 ? 1:0) );
}

Byte* CharStringType2Interpreter::InterpretRMoveto(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Rmoveto(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretHMoveto(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Hmoveto(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretVStemHM(Byte* inProgramCounter)
{
	mStemsCount+= (unsigned short)(mOperandStack.size() / 2);

	EStatusCode status = mImplementationHelper->Type2Vstemhm(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretRCurveLine(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Rcurveline(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretRLineCurve(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Rlinecurve(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretVVCurveto(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Vvcurveto(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretHHCurveto(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Hhcurveto(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretCallGSubr(Byte* inProgramCounter)
{
	CharString* aCharString = NULL;
	aCharString = mImplementationHelper->GetGlobalSubr(mOperandStack.back().IntegerValue);
	mOperandStack.pop_back();

	if(aCharString != NULL)
	{
		Byte* charString = NULL;
		EStatusCode status = mImplementationHelper->ReadCharString(aCharString->mStartPosition,aCharString->mEndPosition,&charString);	
		
		do
		{
			if(status != PDFHummus::eSuccess)
			{
				TRACE_LOG2("CharStringType2Interpreter::InterpretCallSubr, failed to read charstring starting in %lld and ending in %lld",aCharString->mStartPosition,aCharString->mEndPosition);
				break;
			}
			
			status = ProcessCharString(charString,aCharString->mEndPosition - aCharString->mStartPosition);
		}while(false);

		delete charString;
		if(status != PDFHummus::eSuccess)
			return NULL;
		else
			return inProgramCounter;
	}
	else
	{
		return NULL;
	}
}

Byte* CharStringType2Interpreter::InterpretVHCurveto(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Vhcurveto(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretHVCurveto(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Hvcurveto(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretAnd(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2And(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;
	CharStringOperand newOperand;
	newOperand.IsInteger = true;

	valueB = mOperandStack.back();
	mOperandStack.pop_back();
	valueA = mOperandStack.back();
	mOperandStack.pop_back();

	newOperand.IntegerValue = (
		(valueB.IsInteger ? valueB.IntegerValue : valueB.RealValue) && 
		(valueA.IsInteger ? valueA.IntegerValue : valueA.RealValue)
		) ? 1:0;
	mOperandStack.push_back(newOperand);
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretOr(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Or(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;
	CharStringOperand newOperand;
	newOperand.IsInteger = true;

	valueB = mOperandStack.back();
	mOperandStack.pop_back();
	valueA = mOperandStack.back();
	mOperandStack.pop_back();

	newOperand.IntegerValue = (
		(valueB.IsInteger ? valueB.IntegerValue : valueB.RealValue) ||
		(valueA.IsInteger ? valueA.IntegerValue : valueA.RealValue)
		) ? 1:0;	mOperandStack.push_back(newOperand);
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretNot(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Not(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand value;
	CharStringOperand newOperand;
	newOperand.IsInteger = true;

	value = mOperandStack.back();
	mOperandStack.pop_back();

	newOperand.IntegerValue = (value.IsInteger ? value.IntegerValue : value.RealValue)  ? 1:0;
	mOperandStack.push_back(newOperand);
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretAbs(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Abs(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand value;
	CharStringOperand newOperand;

	value = mOperandStack.back();
	newOperand.IsInteger = value.IsInteger;
	mOperandStack.pop_back();

	if(value.IsInteger)
		newOperand.IntegerValue = labs(value.IntegerValue);
	else
		newOperand.RealValue = fabs(value.RealValue);
	mOperandStack.push_back(newOperand);
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretAdd(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Add(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;
	CharStringOperand newOperand;

	valueB = mOperandStack.back();
	mOperandStack.pop_back();
	valueA = mOperandStack.back();
	mOperandStack.pop_back();

	if(!valueA.IsInteger || !valueB.IsInteger)
	{
		newOperand.IsInteger = false;
		newOperand.RealValue = 
			(valueA.IsInteger ? (double)valueA.IntegerValue : valueA.RealValue)
			+
			(valueB.IsInteger ? (double)valueB.IntegerValue : valueB.RealValue);
	}
	else
	{
		newOperand.IsInteger = true;
		newOperand.IntegerValue = valueA.IntegerValue + valueB.IntegerValue;
	}
	mOperandStack.push_back(newOperand);
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretSub(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Sub(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;
	CharStringOperand newOperand;

	valueB = mOperandStack.back();
	mOperandStack.pop_back();
	valueA = mOperandStack.back();
	mOperandStack.pop_back();

	if(!valueA.IsInteger || !valueB.IsInteger)
	{
		newOperand.IsInteger = false;
		newOperand.RealValue = 
			(valueA.IsInteger ? (double)valueA.IntegerValue : valueA.RealValue)
			-
			(valueB.IsInteger ? (double)valueB.IntegerValue : valueB.RealValue);
	}
	else
	{
		newOperand.IsInteger = true;
		newOperand.IntegerValue = valueA.IntegerValue - valueB.IntegerValue;
	}
	mOperandStack.push_back(newOperand);
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretDiv(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Div(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;
	CharStringOperand newOperand;

	valueB = mOperandStack.back();
	mOperandStack.pop_back();
	valueA = mOperandStack.back();
	mOperandStack.pop_back();

	if(!valueA.IsInteger || !valueB.IsInteger)
	{
		newOperand.IsInteger = false;
		newOperand.RealValue = 
			(valueA.IsInteger ? (double)valueA.IntegerValue : valueA.RealValue)
			/
			(valueB.IsInteger ? (double)valueB.IntegerValue : valueB.RealValue);
	}
	else
	{
		newOperand.IsInteger = true;
		newOperand.IntegerValue = valueA.IntegerValue / valueB.IntegerValue;
	}
	mOperandStack.push_back(newOperand);
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretNeg(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Neg(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand value;
	CharStringOperand newOperand;

	value = mOperandStack.back();
	newOperand.IsInteger = value.IsInteger;
	mOperandStack.pop_back();

	if(value.IsInteger)
		newOperand.IntegerValue = -value.IntegerValue;
	else
		newOperand.RealValue = -value.RealValue;
	mOperandStack.push_back(newOperand);
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretEq(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Eq(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;
	CharStringOperand newOperand;

	valueB = mOperandStack.back();
	mOperandStack.pop_back();
	valueA = mOperandStack.back();
	mOperandStack.pop_back();


	newOperand.IsInteger = true;
	newOperand.IntegerValue = (
	(valueB.IsInteger ? valueB.IntegerValue : valueB.RealValue) ==
	(valueA.IsInteger ? valueA.IntegerValue : valueA.RealValue)
	) ? 1:0;	
	mOperandStack.push_back(newOperand);
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretDrop(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Drop(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	mOperandStack.pop_back();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretPut(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Put(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;

	valueB = mOperandStack.back();
	mOperandStack.pop_back();
	valueA = mOperandStack.back();
	mOperandStack.pop_back();

	mStorage[(valueB.IsInteger ? valueB.IntegerValue : (long)valueB.RealValue)] = valueA;

	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretGet(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Get(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand value;

	value = mOperandStack.back();
	mOperandStack.pop_back();
	long index = (value.IsInteger ? value.IntegerValue : (long)value.RealValue);

	if((mOperandStack.size() > (unsigned long)index) && (index >= 0))
	{
		mOperandStack.push_back(mStorage[index]);
		return inProgramCounter;
	}
	else
		return NULL;
}

Byte* CharStringType2Interpreter::InterpretIfelse(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Ifelse(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;
	CharStringOperand valueC;
	CharStringOperand valueD;

	valueD = mOperandStack.back();
	mOperandStack.pop_back();
	valueC = mOperandStack.back();
	mOperandStack.pop_back();
	valueB = mOperandStack.back();
	mOperandStack.pop_back();
	valueA = mOperandStack.back();
	mOperandStack.pop_back();


	if(!valueC.IsInteger || !valueD.IsInteger)
	{
		if((valueC.IsInteger ? (double)valueC.IntegerValue : valueC.RealValue) >
			(valueD.IsInteger ? (double)valueD.IntegerValue : valueD.RealValue))
			mOperandStack.push_back(valueB);
		else
			mOperandStack.push_back(valueA);
	}
	else
	{
		if(valueC.IntegerValue > valueD.IntegerValue)
			mOperandStack.push_back(valueB);
		else
			mOperandStack.push_back(valueA);
	}

	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretRandom(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Random(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand newOperand;

	newOperand.IsInteger = false;
	newOperand.RealValue = ((double)rand() + 1) / ((double)RAND_MAX + 1);

	mOperandStack.push_back(newOperand);
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretMul(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Mul(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;
	CharStringOperand newOperand;

	valueB = mOperandStack.back();
	mOperandStack.pop_back();
	valueA = mOperandStack.back();
	mOperandStack.pop_back();

	if(!valueA.IsInteger || !valueB.IsInteger)
	{
		newOperand.IsInteger = false;
		newOperand.RealValue = 
			(valueA.IsInteger ? (double)valueA.IntegerValue : valueA.RealValue)
			*
			(valueB.IsInteger ? (double)valueB.IntegerValue : valueB.RealValue);
	}
	else
	{
		newOperand.IsInteger = true;
		newOperand.IntegerValue = valueA.IntegerValue * valueB.IntegerValue;
	}
	mOperandStack.push_back(newOperand);
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretSqrt(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Sqrt(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand value;
	CharStringOperand newOperand;

	value = mOperandStack.back();
	mOperandStack.pop_back();

	newOperand.IsInteger = false;
	newOperand.RealValue = sqrt(value.IsInteger ? value.IntegerValue:value.RealValue);
	mOperandStack.push_back(newOperand);
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretDup(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Dup(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	mOperandStack.push_back(mOperandStack.back());
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretExch(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Exch(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;

	valueB = mOperandStack.back();
	mOperandStack.pop_back();
	valueA = mOperandStack.back();
	mOperandStack.pop_back();

	mOperandStack.push_back(valueB);
	mOperandStack.push_back(valueA);
	
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretIndex(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Index(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand value;

	value = mOperandStack.back();
	mOperandStack.pop_back();
	long index = (value.IsInteger ? value.IntegerValue : (long)value.RealValue);
	CharStringOperandList::reverse_iterator it = mOperandStack.rbegin();

	while(index > 0)
		++it;
	mOperandStack.push_back(*it);

	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretRoll(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Roll(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;

	valueB = mOperandStack.back();
	mOperandStack.pop_back();
	valueA = mOperandStack.back();
	mOperandStack.pop_back();

	long shiftAmount = (valueB.IsInteger ? valueB.IntegerValue : (long)valueB.RealValue);
	long itemsCount = (valueA.IsInteger ? valueA.IntegerValue : (long)valueA.RealValue);

	CharStringOperandList groupToShift;

	for(long i=0; i < itemsCount;++i)
	{
		groupToShift.push_front(mOperandStack.back());
		mOperandStack.pop_back();
	}

	if(shiftAmount > 0)
	{
		for(long j=0; j < shiftAmount;++j)
		{
			groupToShift.push_front(groupToShift.back());
			groupToShift.pop_back();
		}
	}
	else
	{
		for(long j=0; j < -shiftAmount;++j)
		{
			groupToShift.push_back(groupToShift.front());
			groupToShift.pop_front();
		}

	}
	
	for(long i=0; i < itemsCount;++i)
	{
		mOperandStack.push_back(mOperandStack.front());
		mOperandStack.pop_front();
	}

	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretHFlex(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Hflex(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretFlex(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Flex(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretHFlex1(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Hflex1(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;

}

Byte* CharStringType2Interpreter::InterpretFlex1(Byte* inProgramCounter)
{
	EStatusCode status = mImplementationHelper->Type2Flex1(mOperandStack);
	if(status != PDFHummus::eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}