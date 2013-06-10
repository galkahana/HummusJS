/*
   Source File : CharStringType1Tracer.h


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
#include "IType1InterpreterImplementation.h"
#include "PrimitiveObjectsWriter.h"
#include <string>

class Type1Input;
class IByteWriter;



class CharStringType1Tracer: public IType1InterpreterImplementation
{
public:
	CharStringType1Tracer(void);
	~CharStringType1Tracer(void);

	PDFHummus::EStatusCode TraceGlyphProgram(Byte inGlyphIndex, Type1Input* inType1Input, IByteWriter* inWriter);
	PDFHummus::EStatusCode TraceGlyphProgram(const std::string& inGlyphName, Type1Input* inType1Input, IByteWriter* inWriter);

	// IType1InterpreterImplementation
	virtual PDFHummus::EStatusCode Type1Hstem(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1Vstem(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1VMoveto(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1RLineto(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1HLineto(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1VLineto(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1RRCurveto(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1ClosePath(const LongList& inOperandList);
	virtual Type1CharString* GetSubr(long inSubrIndex);
	virtual PDFHummus::EStatusCode Type1Return(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1Hsbw(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1Endchar(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1RMoveto(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1HMoveto(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1VHCurveto(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1HVCurveto(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1DotSection(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1VStem3(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1HStem3(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1Seac(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1Sbw(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1Div(const LongList& inOperandList);
	virtual bool IsOtherSubrSupported(long inOtherSubrsIndex);
	virtual PDFHummus::EStatusCode CallOtherSubr(const LongList& inOperandList,LongList& outPostScriptOperandStack);
	virtual PDFHummus::EStatusCode Type1Pop(const LongList& inOperandList,const LongList& inPostScriptOperandStack);
	virtual PDFHummus::EStatusCode Type1SetCurrentPoint(const LongList& inOperandList);
	virtual PDFHummus::EStatusCode Type1InterpretNumber(long inOperand);
	virtual unsigned long GetLenIV();

private:
	IByteWriter* mWriter;
	Type1Input* mHelper;
	PrimitiveObjectsWriter mPrimitiveWriter;

};
