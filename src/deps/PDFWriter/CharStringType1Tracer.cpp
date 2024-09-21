/*
   Source File : CharStringType1Tracer.cpp


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
#include "CharStringType1Tracer.h"
#include "CharStringType1Interpreter.h"
#include "Type1Input.h"
#include "Trace.h"


using namespace PDFHummus;

CharStringType1Tracer::CharStringType1Tracer(void)
{
}

CharStringType1Tracer::~CharStringType1Tracer(void)
{
}

EStatusCode CharStringType1Tracer::TraceGlyphProgram(Byte inGlyphIndex, Type1Input* inType1Input, IByteWriter* inWriter)
{
	CharStringType1Interpreter interpreter;

	mWriter = inWriter;
	mHelper = inType1Input;
	mPrimitiveWriter.SetStreamForWriting(inWriter);

	Type1CharString* charString = inType1Input->GetGlyphCharString(inGlyphIndex);
	if(!charString)
	{
		TRACE_LOG("CharStringType1Tracer::TraceGlyphProgram, Exception, cannot find glyph index");
		return PDFHummus::eFailure;
	}

	return interpreter.Intepret(*charString,this);
}

EStatusCode CharStringType1Tracer::TraceGlyphProgram(const std::string& inGlyphName, Type1Input* inType1Input, IByteWriter* inWriter)
{
	CharStringType1Interpreter interpreter;

	mWriter = inWriter;
	mHelper = inType1Input;
	mPrimitiveWriter.SetStreamForWriting(inWriter);

	Type1CharString* charString = inType1Input->GetGlyphCharString(inGlyphName);
	if(!charString)
	{
		TRACE_LOG("CharStringType1Tracer::TraceGlyphProgram, Exception, cannot find glyph name");
		return PDFHummus::eFailure;
	}

	return interpreter.Intepret(*charString,this);
}

EStatusCode CharStringType1Tracer::Type1Hstem(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("hstem");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1Vstem(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("vstem");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1VMoveto(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("vmoveto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1RLineto(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("rlineto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1HLineto(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("hlineto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1VLineto(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("vlineto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1RRCurveto(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("rrcurveto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1ClosePath(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("closepath");
	return PDFHummus::eSuccess;
}

Type1CharString* CharStringType1Tracer::GetSubr(long inSubrIndex)
{
	mPrimitiveWriter.WriteKeyword("callsubr");
	return mHelper->GetSubr(inSubrIndex);
}

EStatusCode CharStringType1Tracer::Type1Return(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("return");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1Hsbw(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("hsbw");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1Endchar(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("endchar");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1RMoveto(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("rmoveto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1HMoveto(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("hmoveto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1VHCurveto(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("vhcurveto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1HVCurveto(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("hvcurveto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1DotSection(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("dotsection");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1VStem3(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("vstem3");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1HStem3(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("hstem3");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1Seac(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("seac");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1Sbw(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("sbw");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1Div(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("div");
	return PDFHummus::eSuccess;
}

bool CharStringType1Tracer::IsOtherSubrSupported(long inOtherSubrsIndex)
{
	mPrimitiveWriter.WriteKeyword("callothersubr");
	return false;
}

EStatusCode CharStringType1Tracer::CallOtherSubr(const LongList& inOperandList,LongList& outPostScriptOperandStack)
{
	// not looking to get here, due to IsOtherSubrSupported returning false
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1Pop(const LongList& inOperandList,const LongList& inPostScriptOperandStack)
{
	mPrimitiveWriter.WriteKeyword("pop");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1SetCurrentPoint(const LongList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("setcurrentpoint");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType1Tracer::Type1InterpretNumber(long inOperand)
{
	mPrimitiveWriter.WriteInteger(inOperand);
	return PDFHummus::eSuccess;
}

unsigned long CharStringType1Tracer::GetLenIV()
{
	return mHelper->GetLenIV();
}