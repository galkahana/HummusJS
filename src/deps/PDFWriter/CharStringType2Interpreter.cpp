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

#define MAX_ARGUMENTS_STACK_SIZE 48
#define MAX_STEM_HINTS_SIZE 96
#define MAX_SUBR_NESTING_STACK_SIZE 10


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
		mSubrsNesting = 0;
		if(!inImplementationHelper)
		{
			TRACE_LOG("CharStringType2Interpreter::Intepret, null implementation helper passed. pass a proper pointer!!");
			status = eFailure;
			break;
		}

		status = mImplementationHelper->ReadCharString(inCharStringToIntepret.mStartPosition,inCharStringToIntepret.mEndPosition,&charString);	
		if(status != eSuccess)
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
	EStatusCode status = eSuccess;
	Byte* pointer = inCharString;
	bool gotEndExecutionOperator = false;

	while(pointer - inCharString < inCharStringLength &&
			eSuccess == status && 
			!gotEndExecutionOperator &&
			!mGotEndChar)
	{
		LongFilePositionType readLimit = inCharStringLength - (pointer - inCharString); // should be at least 1

		if(IsOperator(*pointer))
		{
			pointer = InterpretOperator(pointer,gotEndExecutionOperator, readLimit);
			if(!pointer)
				status = eFailure;
		}
		else
		{	
			pointer = InterpretNumber(pointer, readLimit);
			if(!pointer)
				status = eFailure;

			if(mOperandStack.size() > MAX_ARGUMENTS_STACK_SIZE) {
				TRACE_LOG1("CharStringType2Interpreter::ProcessCharString, reached maximum allows arguments count - %d, aborting", mOperandStack.size());
				status = eFailure;
			}
		}
	}
	return status;
}

bool CharStringType2Interpreter::IsOperator(Byte inCurrentByte)
{
	return  ((inCurrentByte) <= 27) || 
			(29 <= (inCurrentByte) && (inCurrentByte) <= 31);
			
}


Byte* CharStringType2Interpreter::InterpretNumber(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	CharStringOperand operand;
	Byte* newPosition = inProgramCounter;

	if(inReadLimit < 1)
		return NULL; // error, cant read a single byte

	if(28 == *newPosition && inReadLimit >= 3)
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
	else if(247 <= *newPosition && *newPosition <= 250  && inReadLimit >= 2)
	{
		operand.IsInteger = true;
		operand.IntegerValue = (*newPosition - 247) * 256 + *(newPosition + 1) + 108;
		newPosition += 2;
	}
	else if(251 <= *newPosition && *newPosition <= 254  && inReadLimit >= 2)
	{
		operand.IsInteger = true;
		operand.IntegerValue = -(short)(*newPosition - 251) * 256 - *(newPosition + 1) - 108;
		newPosition += 2;
	}
	else if(255 == *newPosition  && inReadLimit >= 5)
	{
		operand.IsInteger = false;
		operand.RealValue = (short)(((unsigned short)(*(newPosition+1)) << 8) + (*(newPosition+2)));
			if(operand.RealValue > 0)
				operand.RealValue+=
					(double)(((unsigned short)(*(newPosition+3)) << 8) + (*(newPosition+4))) /
								(1 << 16);
			else
				operand.RealValue-=
					(double)(((unsigned short)(*(newPosition+3)) << 8) + (*(newPosition+4))) /
								(1 << 16);
		newPosition += 5;
	}
	else
		newPosition = NULL; // error

	if(newPosition)
	{
		mOperandStack.push_back(operand);
		EStatusCode status = mImplementationHelper->Type2InterpretNumber(operand);
		if(status != eSuccess)
			return NULL;

	}

	return newPosition;
}

Byte* CharStringType2Interpreter::InterpretOperator(Byte* inProgramCounter,bool& outGotEndExecutionCommand,LongFilePositionType inReadLimit)
{
	unsigned short operatorValue;
	Byte* newPosition = inProgramCounter;
	outGotEndExecutionCommand = false;

	if(inReadLimit < 1) {
		TRACE_LOG("harStringType2Interpreter::InterpretOperator, operator identified but no chars to read. aborting");
		return NULL; // error, cant read a single byte	
	}
	
	if(12 == *newPosition)
	{
		if(inReadLimit < 2) {
			TRACE_LOG("harStringType2Interpreter::InterpretOperator, first operator char is 12. expecting another one to read whole operator, but reached read limit. aborting");
			return NULL;
		}

		operatorValue = 0x0c00 + *(newPosition + 1);
		newPosition+=2;
		inReadLimit-=2;
	}
	else
	{
		operatorValue = *newPosition;
		++newPosition;
		--inReadLimit;
	}

	switch(operatorValue)
	{
		case 1: // hstem
			CheckWidth();
			newPosition = InterpretHStem(newPosition, inReadLimit);
			break;
		case 3: // vstem
			CheckWidth();
			newPosition = InterpretVStem(newPosition, inReadLimit);
			break;
		case 4: // vmoveto
			CheckWidth();
			newPosition = InterpretVMoveto(newPosition, inReadLimit);
			break;
		case 5: // rlineto
			newPosition = InterpretRLineto(newPosition, inReadLimit);
			break;
		case 6: // hlineto
			newPosition = InterpretHLineto(newPosition, inReadLimit);
			break;
		case 7: // vlineto
			newPosition = InterpretVLineto(newPosition, inReadLimit);
			break;
		case 8: // rrcurveto
			newPosition = InterpretRRCurveto(newPosition, inReadLimit);
			break;
		case 10: // callsubr
			newPosition = InterpretCallSubr(newPosition, inReadLimit);
			break;
		case 11: // return
			newPosition = InterpretReturn(newPosition, inReadLimit);
			outGotEndExecutionCommand = true;
			break;
		case 14: // endchar
			CheckWidth();
			newPosition = InterpretEndChar(newPosition, inReadLimit);
			break;
		case 18: // hstemhm
			CheckWidth();
			newPosition = InterpretHStemHM(newPosition, inReadLimit);
			break;
		case 19: // hintmask
			CheckWidth();
			newPosition = InterpretHintMask(newPosition, inReadLimit);
			break;
		case 20: // cntrmask
			CheckWidth();
			newPosition = InterpretCntrMask(newPosition, inReadLimit);
			break;
		case 21: // rmoveto
			CheckWidth();
			newPosition = InterpretRMoveto(newPosition, inReadLimit);
			break;
		case 22: // hmoveto
			CheckWidth();
			newPosition = InterpretHMoveto(newPosition, inReadLimit);
			break;
		case 23: // vstemhm
			CheckWidth();
			newPosition = InterpretVStemHM(newPosition, inReadLimit);
			break;
		case 24: // rcurveline
			newPosition = InterpretRCurveLine(newPosition, inReadLimit);
			break;
		case 25: // rlinecurve
			newPosition = InterpretRLineCurve(newPosition, inReadLimit);
			break;
		case 26: // vvcurveto
			newPosition = InterpretVVCurveto(newPosition, inReadLimit);
			break;
		case 27: // hhcurveto
			newPosition = InterpretHHCurveto(newPosition, inReadLimit);
			break;
		case 29: // callgsubr
			newPosition = InterpretCallGSubr(newPosition, inReadLimit);
			break;
		case 30: // vhcurveto
			newPosition = InterpretVHCurveto(newPosition, inReadLimit);
			break;
		case 31: // hvcurveto
			newPosition = InterpretHVCurveto(newPosition, inReadLimit);
			break;
		
		case 0x0c00: // dotsection, depracated
			// ignore
			break;
		case 0x0c03: // and
			newPosition = InterpretAnd(newPosition, inReadLimit);
			break;
		case 0x0c04: // or
			newPosition = InterpretOr(newPosition, inReadLimit);
			break;
		case 0x0c05: // not
			newPosition = InterpretNot(newPosition, inReadLimit);
			break;
		case 0x0c09: // abs
			newPosition = InterpretAbs(newPosition, inReadLimit);
			break;
		case 0x0c0a: // add
			newPosition = InterpretAdd(newPosition, inReadLimit);
			break;
		case 0x0c0b: // sub
			newPosition = InterpretSub(newPosition, inReadLimit);
			break;
		case 0x0c0c: // div
			newPosition = InterpretDiv(newPosition, inReadLimit);
			break;
		case 0x0c0e: // neg
			newPosition = InterpretNeg(newPosition, inReadLimit);
			break;
		case 0x0c0f: // eq
			newPosition = InterpretEq(newPosition, inReadLimit);
			break;
		case 0x0c12: // drop
			newPosition = InterpretDrop(newPosition, inReadLimit);
			break;
		case 0x0c14: // put
			newPosition = InterpretPut(newPosition, inReadLimit);
			break;
		case 0x0c15: // get
			newPosition = InterpretGet(newPosition, inReadLimit);
			break;
		case 0x0c16: // ifelse
			newPosition = InterpretIfelse(newPosition, inReadLimit);
			break;
		case 0x0c17: // random
			newPosition = InterpretRandom(newPosition, inReadLimit);
			break;
		case 0x0c18: // mul
			newPosition = InterpretMul(newPosition, inReadLimit);
			break;
		case 0x0c1a: // sqrt
			newPosition = InterpretSqrt(newPosition, inReadLimit);
			break;
		case 0x0c1b: // dup
			newPosition = InterpretDup(newPosition, inReadLimit);
			break;
		case 0x0c1c: // exch
			newPosition = InterpretExch(newPosition, inReadLimit);
			break;
		case 0x0c1d: // index
			newPosition = InterpretIndex(newPosition, inReadLimit);
			break;
		case 0x0c1e: // roll
			newPosition = InterpretRoll(newPosition, inReadLimit);
			break;
		case 0x0c22: // hflex
			newPosition = InterpretHFlex(newPosition, inReadLimit);
			break;
		case 0x0c23: // flex
			newPosition = InterpretFlex(newPosition, inReadLimit);
			break;
		case 0x0c24: // hflex1
			newPosition = InterpretHFlex1(newPosition, inReadLimit);
			break;
		case 0x0c25: // flex1
			newPosition = InterpretFlex1(newPosition, inReadLimit);
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

EStatusCode CharStringType2Interpreter::AddStemsCount(unsigned short inBy) {
	if(mStemsCount + inBy > MAX_STEM_HINTS_SIZE) {
		TRACE_LOG3("CharStringType2Interpreter::AddStemsCount, about to add %d stem hintss to current %d stem hints. This will breach the upper limit of %d, aborting.", inBy, mStemsCount, MAX_STEM_HINTS_SIZE);
		return eFailure;
	}
	mStemsCount+= inBy;

	return eSuccess;
}

Byte* CharStringType2Interpreter::InterpretHStem(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = AddStemsCount((unsigned short)(mOperandStack.size() / 2));
	if(status != eSuccess)
		return NULL;

	status = mImplementationHelper->Type2Hstem(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

void CharStringType2Interpreter::ClearStack()
{
	mOperandStack.clear();
}

Byte* CharStringType2Interpreter::InterpretVStem(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = AddStemsCount((unsigned short)(mOperandStack.size() / 2));
	if(status != eSuccess)
		return NULL;	

	status = mImplementationHelper->Type2Vstem(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretVMoveto(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Vmoveto(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
	
}

Byte* CharStringType2Interpreter::InterpretRLineto(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Rlineto(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretHLineto(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Hlineto(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretVLineto(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Vlineto(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretRRCurveto(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2RRCurveto(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretCallSubr(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	CharString* aCharString = NULL;
	if(mOperandStack.size() < 1) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretCallSubr, callsubr should have at least 1 argument on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}


	aCharString = mImplementationHelper->GetLocalSubr(mOperandStack.back().IntegerValue);
	mOperandStack.pop_back();

	if(aCharString != NULL)
	{
		Byte* charString = NULL;
		EStatusCode status = mImplementationHelper->ReadCharString(aCharString->mStartPosition,aCharString->mEndPosition,&charString);	
		
		do
		{
			if(status != eSuccess)
			{
				TRACE_LOG2("CharStringType2Interpreter::InterpretCallSubr, failed to read charstring starting in %lld and ending in %lld",aCharString->mStartPosition,aCharString->mEndPosition);
				break;
			}
			
			++mSubrsNesting;
			
			if(mSubrsNesting > MAX_SUBR_NESTING_STACK_SIZE) {
				TRACE_LOG1("CharStringType2Interpreter::InterpretCallSubr, max call stack level reached at %d. aborting", MAX_SUBR_NESTING_STACK_SIZE);
				status = eFailure;
				break;
			}

			status = ProcessCharString(charString,aCharString->mEndPosition - aCharString->mStartPosition);
			--mSubrsNesting;
		}while(false);

		delete charString;
		if(status != eSuccess)
			return NULL;
		else
			return inProgramCounter;
	}
	else
	{
		return NULL;
	}
}

Byte* CharStringType2Interpreter::InterpretReturn(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Return(mOperandStack);
	if(status != eSuccess)
		return NULL;

	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretEndChar(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Endchar(mOperandStack);
	if(status != eSuccess)
		return NULL;

	mGotEndChar = true;
	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretHStemHM(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = AddStemsCount((unsigned short)(mOperandStack.size() / 2));
	if(status != eSuccess)
		return NULL;

	status = mImplementationHelper->Type2Hstemhm(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretHintMask(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = AddStemsCount((unsigned short)(mOperandStack.size() / 2)); // assuming this is a shortcut of dropping vstem if got arguments
	if(status != eSuccess)
		return NULL;	

	status = mImplementationHelper->Type2Hintmask(mOperandStack,inProgramCounter, inReadLimit);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	LongFilePositionType programCounterStemReadSize = (mStemsCount/8 + (mStemsCount % 8 != 0 ? 1:0));
	if(programCounterStemReadSize > inReadLimit)
		return NULL;
	return inProgramCounter+programCounterStemReadSize;
}

Byte* CharStringType2Interpreter::InterpretCntrMask(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = AddStemsCount((unsigned short)(mOperandStack.size() / 2)); // assuming this is a shortcut of dropping vstem if got arguments
	if(status != eSuccess)
		return NULL;	

	status = mImplementationHelper->Type2Cntrmask(mOperandStack,inProgramCounter, inReadLimit);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	LongFilePositionType programCounterStemReadSize = (mStemsCount/8 + (mStemsCount % 8 != 0 ? 1:0));
	if(programCounterStemReadSize > inReadLimit)
		return NULL;
	return inProgramCounter+programCounterStemReadSize;
}

Byte* CharStringType2Interpreter::InterpretRMoveto(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Rmoveto(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretHMoveto(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Hmoveto(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretVStemHM(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = AddStemsCount((unsigned short)(mOperandStack.size() / 2));
	if(status != eSuccess)
		return NULL;	

	status = mImplementationHelper->Type2Vstemhm(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretRCurveLine(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Rcurveline(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretRLineCurve(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Rlinecurve(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretVVCurveto(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Vvcurveto(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretHHCurveto(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Hhcurveto(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretCallGSubr(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	CharString* aCharString = NULL;
	if(mOperandStack.size() < 1) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretCallGSubr, callgsubr should have at least 1 argument on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}


	aCharString = mImplementationHelper->GetGlobalSubr(mOperandStack.back().IntegerValue);
	mOperandStack.pop_back();

	if(aCharString != NULL)
	{
		Byte* charString = NULL;
		EStatusCode status = mImplementationHelper->ReadCharString(aCharString->mStartPosition,aCharString->mEndPosition,&charString);	
		
		do
		{
			if(status != eSuccess)
			{
				TRACE_LOG2("CharStringType2Interpreter::InterpretCallGSubr, failed to read charstring starting in %lld and ending in %lld",aCharString->mStartPosition,aCharString->mEndPosition);
				break;
			}

			++mSubrsNesting;

			if(mSubrsNesting > MAX_SUBR_NESTING_STACK_SIZE) {
				TRACE_LOG1("CharStringType2Interpreter::InterpretCallGSubr, max call stack level reached at %d. aborting", MAX_SUBR_NESTING_STACK_SIZE);
				status = eFailure;
				break;
			}

			status = ProcessCharString(charString,aCharString->mEndPosition - aCharString->mStartPosition);
			--mSubrsNesting;
		}while(false);

		delete charString;
		if(status != eSuccess)
			return NULL;
		else
			return inProgramCounter;
	}
	else
	{
		return NULL;
	}
}

Byte* CharStringType2Interpreter::InterpretVHCurveto(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Vhcurveto(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretHVCurveto(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Hvcurveto(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretAnd(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2And(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;
	CharStringOperand newOperand;
	newOperand.IsInteger = true;

	if(mOperandStack.size() < 2) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretAnd, and should have at least 2 arguments on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}


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

Byte* CharStringType2Interpreter::InterpretOr(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Or(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;
	CharStringOperand newOperand;
	newOperand.IsInteger = true;

	if(mOperandStack.size() < 2) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretOr, or should have at least 2 arguments on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}


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

Byte* CharStringType2Interpreter::InterpretNot(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Not(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand value;
	CharStringOperand newOperand;
	newOperand.IsInteger = true;

	if(mOperandStack.size() < 1) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretOr, not should have at least 1 argument on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}


	value = mOperandStack.back();
	mOperandStack.pop_back();

	newOperand.IntegerValue = (value.IsInteger ? value.IntegerValue : value.RealValue)  ? 1:0;
	mOperandStack.push_back(newOperand);
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretAbs(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Abs(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand value;
	CharStringOperand newOperand;

	if(mOperandStack.size() < 1) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretAbs, abs should have at least 1 argument on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}


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

Byte* CharStringType2Interpreter::InterpretAdd(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Add(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;
	CharStringOperand newOperand;

	if(mOperandStack.size() < 2) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretAdd, add should have at least 2 arguments on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}

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

Byte* CharStringType2Interpreter::InterpretSub(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Sub(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;
	CharStringOperand newOperand;

	if(mOperandStack.size() < 2) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretSub, sub should have at least 2 arguments on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}


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

Byte* CharStringType2Interpreter::InterpretDiv(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Div(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;
	CharStringOperand newOperand;


	if(mOperandStack.size() < 2) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretDiv, div should have at least 2 arguments on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}


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

Byte* CharStringType2Interpreter::InterpretNeg(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Neg(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand value;
	CharStringOperand newOperand;

	if(mOperandStack.size() < 1) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretNeg, neg should have at least 1 argument on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}

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

Byte* CharStringType2Interpreter::InterpretEq(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Eq(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;
	CharStringOperand newOperand;

	if(mOperandStack.size() < 2) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretEq, eq should have at least 2 arguments on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}

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

Byte* CharStringType2Interpreter::InterpretDrop(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Drop(mOperandStack);
	if(status != eSuccess)
		return NULL;

	if(mOperandStack.size() < 1) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretDrop, drop should have at least 1 argument on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}

	mOperandStack.pop_back();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretPut(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Put(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;

	if(mOperandStack.size() < 2) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretPut, put should have at least 2 arguments on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}

	valueB = mOperandStack.back();
	mOperandStack.pop_back();
	valueA = mOperandStack.back();
	mOperandStack.pop_back();

	mStorage[(valueB.IsInteger ? valueB.IntegerValue : (long)valueB.RealValue)] = valueA;

	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretGet(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Get(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand value;

	if(mOperandStack.size() < 1) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretGet, get should have at least 1 argument on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}

	value = mOperandStack.back();
	mOperandStack.pop_back();
	long index = (value.IsInteger ? value.IntegerValue : (long)value.RealValue);

	if((mStorage.size() > (unsigned long)index) && (index >= 0))
	{
		mOperandStack.push_back(mStorage[index]);
		return inProgramCounter;
	}
	else {
		TRACE_LOG2("CharStringType2Interpreter::InterpretGet, input argument for get operation does not match storage size. argument value is %ld and storage size is %d. aborting", index, mStorage.size());
		return NULL;
	}
}

Byte* CharStringType2Interpreter::InterpretIfelse(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Ifelse(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;
	CharStringOperand valueC;
	CharStringOperand valueD;

	if(mOperandStack.size() < 4) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretIfelse, ifelse should have at least 4 arguments on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}		

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

Byte* CharStringType2Interpreter::InterpretRandom(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Random(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand newOperand;

	newOperand.IsInteger = false;
	newOperand.RealValue = ((double)rand() + 1) / ((double)RAND_MAX + 1);

	mOperandStack.push_back(newOperand);
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretMul(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Mul(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;
	CharStringOperand newOperand;

	if(mOperandStack.size() < 2) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretMul, mul should have at least 4 arguments on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}		

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

Byte* CharStringType2Interpreter::InterpretSqrt(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Sqrt(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand value;
	CharStringOperand newOperand;

	if(mOperandStack.size() < 1) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretSqrt, sqrt should have at least 1 argument on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}		

	value = mOperandStack.back();
	mOperandStack.pop_back();

	newOperand.IsInteger = false;
	newOperand.RealValue = sqrt(value.IsInteger ? value.IntegerValue:value.RealValue);
	mOperandStack.push_back(newOperand);
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretDup(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Dup(mOperandStack);
	if(status != eSuccess)
		return NULL;

	if(mOperandStack.size() < 1) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretDup, dup should have at least 1 argument on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}		

	mOperandStack.push_back(mOperandStack.back());
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretExch(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Exch(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;

	if(mOperandStack.size() < 2) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretExch, exch should have at least 2 arguments on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}		

	valueB = mOperandStack.back();
	mOperandStack.pop_back();
	valueA = mOperandStack.back();
	mOperandStack.pop_back();

	mOperandStack.push_back(valueB);
	mOperandStack.push_back(valueA);
	
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretIndex(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Index(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand value;

	if(mOperandStack.size() < 1) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretExch, exch should have at least 1 argument on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}		

	value = mOperandStack.back();
	mOperandStack.pop_back();
	long index = (value.IsInteger ? value.IntegerValue : (long)value.RealValue);
	CharStringOperandList::reverse_iterator it = mOperandStack.rbegin();

	while(index > 0 && it != mOperandStack.rend())
		++it;
	mOperandStack.push_back(*it);

	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretRoll(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Roll(mOperandStack);
	if(status != eSuccess)
		return NULL;

	CharStringOperand valueA;
	CharStringOperand valueB;

	if(mOperandStack.size() < 1) {
		TRACE_LOG1("CharStringType2Interpreter::InterpretRoll, roll should have at least 2 arguments on stack. got %d. aborting", mOperandStack.size());
		return NULL;
	}	

	valueB = mOperandStack.back();
	mOperandStack.pop_back();
	valueA = mOperandStack.back();
	mOperandStack.pop_back();

	long shiftAmount = (valueB.IsInteger ? valueB.IntegerValue : (long)valueB.RealValue);
	long itemsCount = (valueA.IsInteger ? valueA.IntegerValue : (long)valueA.RealValue);

	if(itemsCount > 0) {
		CharStringOperandList groupToShift;

		for(long i=0; i < itemsCount && mOperandStack.size() > 0;++i)
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
		
		// put back the rolled group
		for(long i=0; i < itemsCount;++i)
		{
			mOperandStack.push_back(groupToShift.front());
			groupToShift.pop_front();
		}
	}

	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretHFlex(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Hflex(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretFlex(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Flex(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}

Byte* CharStringType2Interpreter::InterpretHFlex1(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Hflex1(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;

}

Byte* CharStringType2Interpreter::InterpretFlex1(Byte* inProgramCounter, LongFilePositionType inReadLimit)
{
	EStatusCode status = mImplementationHelper->Type2Flex1(mOperandStack);
	if(status != eSuccess)
		return NULL;

	ClearStack();
	return inProgramCounter;
}