/*
   Source File : CharStringType2Tracer.cpp


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
#include "CharStringType2Tracer.h"
#include "CFFFileInput.h"
#include "IByteWriter.h"
#include "CharStringType2Interpreter.h"
#include "Trace.h"
#include "BoxingBase.h"
#include "SafeBufferMacrosDefs.h"

using namespace PDFHummus;

CharStringType2Tracer::CharStringType2Tracer(void)
{
}

CharStringType2Tracer::~CharStringType2Tracer(void)
{
}


EStatusCode CharStringType2Tracer::TraceGlyphProgram(unsigned short inFontIndex, 
													unsigned short inGlyphIndex, 
													CFFFileInput* inCFFFileInput, 
													IByteWriter* inWriter)
{
	CharStringType2Interpreter interpreter;
	EStatusCode status = inCFFFileInput->PrepareForGlyphIntepretation(inFontIndex,inGlyphIndex);

	mWriter = inWriter;
	mHelper = inCFFFileInput;
	mPrimitiveWriter.SetStreamForWriting(inWriter);
	mStemsCount = 0;

	do
	{
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("CharStringType2Tracer::Trace, Exception, cannot prepare for glyph interpretation");
			break;
		}
		
		CharString* charString = inCFFFileInput->GetGlyphCharString(inFontIndex,inGlyphIndex);
		if(!charString)
		{
			TRACE_LOG("CharStringType2Tracer::Trace, Exception, cannot find glyph index");
			break;
		}

		status = interpreter.Intepret(*charString,this);

	}while(false);
	return status;
}

EStatusCode CharStringType2Tracer::ReadCharString(LongFilePositionType inCharStringStart,
						   LongFilePositionType inCharStringEnd,
						   Byte** outCharString)
{
	return mHelper->ReadCharString(inCharStringStart,inCharStringEnd,outCharString);
}

EStatusCode CharStringType2Tracer::Type2InterpretNumber(const CharStringOperand& inOperand)
{
	if(inOperand.IsInteger)
		mPrimitiveWriter.WriteInteger(inOperand.IntegerValue);
	else
		mPrimitiveWriter.WriteDouble(inOperand.RealValue);
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Hstem(const CharStringOperandList& inOperandList)
{
	mStemsCount+= (unsigned short)(inOperandList.size() / 2);

	mPrimitiveWriter.WriteKeyword("hstem");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Vstem(const CharStringOperandList& inOperandList)
{
	mStemsCount+= (unsigned short)(inOperandList.size() / 2);

	mPrimitiveWriter.WriteKeyword("vstem");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Vmoveto(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("vstem");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Rlineto(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("rlineto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Hlineto(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("hlineto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Vlineto(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("vlineto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2RRCurveto(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("rrcurveto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Return(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("return");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Endchar(const CharStringOperandList& inOperandList)
{
	// no need to call the CFFFileInput endchar here. that call is used for dependencies check alone
	// and provides for CFFFileInput own intepreter implementation.

	mPrimitiveWriter.WriteKeyword("endchar");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Hstemhm(const CharStringOperandList& inOperandList)
{
	mStemsCount+= (unsigned short)(inOperandList.size() / 2);

	mPrimitiveWriter.WriteKeyword("hstemhm");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Hintmask(const CharStringOperandList& inOperandList,Byte* inProgramCounter)
{
	mStemsCount+= (unsigned short)(inOperandList.size() / 2);

	WriteStemMask(inProgramCounter);
	mPrimitiveWriter.WriteKeyword("hintmask");
	return PDFHummus::eSuccess;
}

void CharStringType2Tracer::WriteStemMask(Byte* inProgramCounter)
{
	unsigned short maskSize = mStemsCount/8 + (mStemsCount % 8 != 0 ? 1:0);
	char buffer[3];

	mWriter->Write((const Byte*)"(0x",1);
	for(unsigned short i=0;i<maskSize;++i)
	{
		SAFE_SPRINTF_1(buffer,3,"%X",inProgramCounter[i]);
		mWriter->Write((const Byte*)buffer,2);
	}

	mWriter->Write((const Byte*)")",1);
}

EStatusCode CharStringType2Tracer::Type2Cntrmask(const CharStringOperandList& inOperandList,Byte* inProgramCounter)
{
	WriteStemMask(inProgramCounter);
	mPrimitiveWriter.WriteKeyword("cntrmask");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Rmoveto(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("rmoveto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Hmoveto(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("hmoveto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Vstemhm(const CharStringOperandList& inOperandList)
{
	mStemsCount+= (unsigned short)(inOperandList.size() / 2);

	mPrimitiveWriter.WriteKeyword("vstemhm");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Rcurveline(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("rcurveline");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Rlinecurve(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("rlinecurve");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Vvcurveto(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("vvcurveto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Hvcurveto(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("hvcurveto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Hhcurveto(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("hhcurveto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Vhcurveto(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("vhcurveto");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Hflex(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("hflex");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Hflex1(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("hflex1");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Flex(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("flex");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Flex1(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("flex1");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2And(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("and");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Or(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("or");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Not(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("not");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Abs(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("abs");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Add(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("add");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Sub(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("sub");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Div(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("div");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Neg(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("neg");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Eq(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("eq");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Drop(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("drop");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Put(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("put");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Get(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("get");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Ifelse(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("ifelse");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Random(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("random");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Mul(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("mul");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Sqrt(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("sqrt");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Dup(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("dup");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Exch(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("exch");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Index(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("index");
	return PDFHummus::eSuccess;
}

EStatusCode CharStringType2Tracer::Type2Roll(const CharStringOperandList& inOperandList)
{
	mPrimitiveWriter.WriteKeyword("roll");
	return PDFHummus::eSuccess;
}

CharString* CharStringType2Tracer::GetLocalSubr(long inSubrIndex)
{
	mPrimitiveWriter.WriteKeyword("callsubr");

	return mHelper->GetLocalSubr(inSubrIndex);
}

CharString* CharStringType2Tracer::GetGlobalSubr(long inSubrIndex)
{
	mPrimitiveWriter.WriteKeyword("callgsubr");

	return mHelper->GetGlobalSubr(inSubrIndex);
}
