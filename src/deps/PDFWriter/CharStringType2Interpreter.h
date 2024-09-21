/*
   Source File : CharStringType2Interpreter.h


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

#include "IType2InterpreterImplementation.h"
#include "CharStringDefinitions.h"

#include <vector>
#include <list>

typedef std::vector<CharStringOperand> CharStringOperandVector;

class CharStringType2Interpreter
{
public:
	CharStringType2Interpreter(void);
	~CharStringType2Interpreter(void);

	PDFHummus::EStatusCode Intepret(const CharString& inCharStringToIntepret, IType2InterpreterImplementation* inImplementationHelper);


private:
	CharStringOperandList mOperandStack;
	unsigned short mStemsCount;
	IType2InterpreterImplementation* mImplementationHelper;
	bool mGotEndChar;
	CharStringOperandVector mStorage;
	bool mCheckedWidth;
	unsigned short mSubrsNesting;


	PDFHummus::EStatusCode ProcessCharString(Byte* inCharString,LongFilePositionType inCharStringLength);
	bool IsOperator(Byte inCurrentByte);
	Byte* InterpretNumber(Byte* inProgramCounter,LongFilePositionType inReadLimit);
	Byte* InterpretOperator(Byte* inProgramCounter,bool& outGotEndExecutionCommand,LongFilePositionType inReadLimit);
	PDFHummus::EStatusCode AddStemsCount(unsigned short inBy);

	void ClearStack();
	void CheckWidth();

	Byte* InterpretHStem(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretVStem(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretVMoveto(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretRLineto(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretHLineto(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretVLineto(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretRRCurveto(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretCallSubr(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretReturn(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretEndChar(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretHStemHM(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretHintMask(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretCntrMask(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretRMoveto(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretHMoveto(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretVStemHM(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretRCurveLine(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretRLineCurve(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretVVCurveto(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretHHCurveto(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretCallGSubr(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretVHCurveto(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretHVCurveto(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretAnd(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretOr(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretNot(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretAbs(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretAdd(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretSub(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretDiv(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretNeg(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretEq(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretDrop(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretPut(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretGet(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretIfelse(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretRandom(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretMul(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretSqrt(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretDup(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretExch(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretIndex(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretRoll(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretHFlex(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretFlex(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretHFlex1(Byte* inProgramCounter, LongFilePositionType inReadLimit);
	Byte* InterpretFlex1(Byte* inProgramCounter, LongFilePositionType inReadLimit);
};
