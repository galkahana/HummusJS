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


	PDFHummus::EStatusCode ProcessCharString(Byte* inCharString,LongFilePositionType inCharStringLength);
	bool IsOperator(Byte* inProgramCounter);
	Byte* InterpretNumber(Byte* inProgramCounter);
	Byte* InterpretOperator(Byte* inProgramCounter,bool& outGotEndExecutionCommand);

	PDFHummus::EStatusCode ClearNFromStack(unsigned short inCount);
	void ClearStack();
	void CheckWidth();

	Byte* InterpretHStem(Byte* inProgramCounter);
	Byte* InterpretVStem(Byte* inProgramCounter);
	Byte* InterpretVMoveto(Byte* inProgramCounter);
	Byte* InterpretRLineto(Byte* inProgramCounter);
	Byte* InterpretHLineto(Byte* inProgramCounter);
	Byte* InterpretVLineto(Byte* inProgramCounter);
	Byte* InterpretRRCurveto(Byte* inProgramCounter);
	Byte* InterpretCallSubr(Byte* inProgramCounter);
	Byte* InterpretReturn(Byte* inProgramCounter);
	Byte* InterpretEndChar(Byte* inProgramCounter);
	Byte* InterpretHStemHM(Byte* inProgramCounter);
	Byte* InterpretHintMask(Byte* inProgramCounter);
	Byte* InterpretCntrMask(Byte* inProgramCounter);
	Byte* InterpretRMoveto(Byte* inProgramCounter);
	Byte* InterpretHMoveto(Byte* inProgramCounter);
	Byte* InterpretVStemHM(Byte* inProgramCounter);
	Byte* InterpretRCurveLine(Byte* inProgramCounter);
	Byte* InterpretRLineCurve(Byte* inProgramCounter);
	Byte* InterpretVVCurveto(Byte* inProgramCounter);
	Byte* InterpretHHCurveto(Byte* inProgramCounter);
	Byte* InterpretCallGSubr(Byte* inProgramCounter);
	Byte* InterpretVHCurveto(Byte* inProgramCounter);
	Byte* InterpretHVCurveto(Byte* inProgramCounter);
	Byte* InterpretAnd(Byte* inProgramCounter);
	Byte* InterpretOr(Byte* inProgramCounter);
	Byte* InterpretNot(Byte* inProgramCounter);
	Byte* InterpretAbs(Byte* inProgramCounter);
	Byte* InterpretAdd(Byte* inProgramCounter);
	Byte* InterpretSub(Byte* inProgramCounter);
	Byte* InterpretDiv(Byte* inProgramCounter);
	Byte* InterpretNeg(Byte* inProgramCounter);
	Byte* InterpretEq(Byte* inProgramCounter);
	Byte* InterpretDrop(Byte* inProgramCounter);
	Byte* InterpretPut(Byte* inProgramCounter);
	Byte* InterpretGet(Byte* inProgramCounter);
	Byte* InterpretIfelse(Byte* inProgramCounter);
	Byte* InterpretRandom(Byte* inProgramCounter);
	Byte* InterpretMul(Byte* inProgramCounter);
	Byte* InterpretSqrt(Byte* inProgramCounter);
	Byte* InterpretDup(Byte* inProgramCounter);
	Byte* InterpretExch(Byte* inProgramCounter);
	Byte* InterpretIndex(Byte* inProgramCounter);
	Byte* InterpretRoll(Byte* inProgramCounter);
	Byte* InterpretHFlex(Byte* inProgramCounter);
	Byte* InterpretFlex(Byte* inProgramCounter);
	Byte* InterpretHFlex1(Byte* inProgramCounter);
	Byte* InterpretFlex1(Byte* inProgramCounter);
};
