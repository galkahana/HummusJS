/*
   Source File : CharStringType1Interpreter.h


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
#include "IType1InterpreterImplementation.h"
#include "EStatusCode.h"
#include "InputCharStringDecodeStream.h"

class CharStringType1Interpreter
{
public:
	CharStringType1Interpreter(void);
	~CharStringType1Interpreter(void);

	PDFHummus::EStatusCode Intepret(const Type1CharString& inCharStringToIntepret, IType1InterpreterImplementation* inImplementationHelper);

private:

	LongList mOperandStack;
	IType1InterpreterImplementation* mImplementationHelper;
	bool mGotEndChar;
	LongList mPostScriptOperandStack;
	unsigned short mSubrsNesting;

	PDFHummus::EStatusCode ProcessCharString(InputCharStringDecodeStream* inCharStringToIntepret);
	bool IsOperator(Byte inBuffer);
	PDFHummus::EStatusCode InterpretOperator(Byte inBuffer,InputCharStringDecodeStream* inCharStringToIntepret,bool& outGotEndExecutionCommand);
	PDFHummus::EStatusCode InterpretNumber(Byte inBuffer,InputCharStringDecodeStream* inCharStringToIntepret);
	void ClearStack();
	PDFHummus::EStatusCode DefaultCallOtherSubr();
	
	PDFHummus::EStatusCode InterpretHStem();
	PDFHummus::EStatusCode InterpretVStem();
	PDFHummus::EStatusCode InterpretVMoveto();
	PDFHummus::EStatusCode InterpretRLineto();
	PDFHummus::EStatusCode InterpretHLineto();
	PDFHummus::EStatusCode InterpretVLineto();
	PDFHummus::EStatusCode InterpretRRCurveto();
	PDFHummus::EStatusCode InterpretClosePath();
	PDFHummus::EStatusCode InterpretCallSubr();
	PDFHummus::EStatusCode InterpretReturn();
	PDFHummus::EStatusCode InterpretHsbw();
	PDFHummus::EStatusCode InterpretEndChar();
	PDFHummus::EStatusCode InterpretRMoveto();
	PDFHummus::EStatusCode InterpretHMoveto();
	PDFHummus::EStatusCode InterpretVHCurveto();
	PDFHummus::EStatusCode InterpretHVCurveto();
	PDFHummus::EStatusCode InterpretDotSection();
	PDFHummus::EStatusCode InterpretVStem3();
	PDFHummus::EStatusCode InterpretHStem3();
	PDFHummus::EStatusCode InterpretSeac();
	PDFHummus::EStatusCode InterpretSbw();
	PDFHummus::EStatusCode InterpretDiv();
	PDFHummus::EStatusCode InterpretCallOtherSubr();
	PDFHummus::EStatusCode InterpretPop();
	PDFHummus::EStatusCode InterpretSetCurrentPoint();

};

