/*
   Source File : IType2InterpreterImplementation.h


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
#include "CharStringDefinitions.h"

class IType2InterpreterImplementation
{
public:
	virtual PDFHummus::EStatusCode ReadCharString(LongFilePositionType inCharStringStart,
							   LongFilePositionType inCharStringEnd,
							   Byte** outCharString) = 0;	

	// events in the code
	virtual PDFHummus::EStatusCode Type2InterpretNumber(const CharStringOperand& inOperand) = 0;
	virtual PDFHummus::EStatusCode Type2Hstem(const CharStringOperandList& inOperandList) = 0;
	virtual PDFHummus::EStatusCode Type2Vstem(const CharStringOperandList& inOperandList) = 0;
	virtual PDFHummus::EStatusCode Type2Vmoveto(const CharStringOperandList& inOperandList) = 0;
	virtual PDFHummus::EStatusCode Type2Rlineto(const CharStringOperandList& inOperandList) = 0;
	virtual PDFHummus::EStatusCode Type2Hlineto(const CharStringOperandList& inOperandList) = 0;
	virtual PDFHummus::EStatusCode Type2Vlineto(const CharStringOperandList& inOperandList) = 0;
	virtual PDFHummus::EStatusCode Type2RRCurveto(const CharStringOperandList& inOperandList) = 0;
	virtual PDFHummus::EStatusCode Type2Return(const CharStringOperandList& inOperandList) =0;
	virtual PDFHummus::EStatusCode Type2Endchar(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Hstemhm(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Hintmask(const CharStringOperandList& inOperandList,Byte* inProgramCounter,LongFilePositionType inReadLimit)=0;
	virtual PDFHummus::EStatusCode Type2Cntrmask(const CharStringOperandList& inOperandList,Byte* inProgramCounter,LongFilePositionType inReadLimit)=0;
	virtual PDFHummus::EStatusCode Type2Rmoveto(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Hmoveto(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Vstemhm(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Rcurveline(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Rlinecurve(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Vvcurveto(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Hvcurveto(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Hhcurveto(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Vhcurveto(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Hflex(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Hflex1(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Flex(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Flex1(const CharStringOperandList& inOperandList)=0;
	
	virtual PDFHummus::EStatusCode Type2And(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Or(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Not(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Abs(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Add(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Sub(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Div(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Neg(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Eq(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Drop(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Put(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Get(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Ifelse(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Random(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Mul(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Sqrt(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Dup(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Exch(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Index(const CharStringOperandList& inOperandList)=0;
	virtual PDFHummus::EStatusCode Type2Roll(const CharStringOperandList& inOperandList)=0;
	
	virtual CharString* GetLocalSubr(long inSubrIndex) = 0; // you should bias the index !!
	virtual CharString* GetGlobalSubr(long inSubrIndex) = 0;// you should bias the index !!
	
};

class Type2InterpreterImplementationAdapter : public IType2InterpreterImplementation
{
public:
	virtual PDFHummus::EStatusCode ReadCharString(LongFilePositionType inCharStringStart,
							   LongFilePositionType inCharStringEnd,
							   Byte** outCharString){
	  (void) inCharStringStart;
	  (void) inCharStringEnd;
	  return PDFHummus::eFailure;}	

	virtual PDFHummus::EStatusCode Type2InterpretNumber(const CharStringOperand& inOperand) {(void) inOperand; return PDFHummus::eSuccess;};
	virtual PDFHummus::EStatusCode Type2Hstem(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Vstem(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Vmoveto(const CharStringOperandList& inOperandList) {(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Rlineto(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Hlineto(const CharStringOperandList& inOperandList) {(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Vlineto(const CharStringOperandList& inOperandList) {(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2RRCurveto(const CharStringOperandList& inOperandList) {(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Return(const CharStringOperandList& inOperandList) {(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Endchar(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Hstemhm(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Hintmask(const CharStringOperandList& inOperandList,Byte* inProgramCounter,LongFilePositionType inReadLimit){(void) inOperandList; (void) (void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Cntrmask(const CharStringOperandList& inOperandList,Byte* inProgramCounter,LongFilePositionType inReadLimit){return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Rmoveto(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Hmoveto(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Vstemhm(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Rcurveline(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Rlinecurve(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Vvcurveto(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Hvcurveto(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Hhcurveto(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Vhcurveto(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Hflex(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Hflex1(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Flex(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Flex1(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}

	virtual PDFHummus::EStatusCode Type2And(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Or(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Not(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Abs(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Add(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Sub(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Div(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Neg(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Eq(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Drop(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Put(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Get(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Ifelse(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Random(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Mul(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Sqrt(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Dup(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Exch(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Index(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}
	virtual PDFHummus::EStatusCode Type2Roll(const CharStringOperandList& inOperandList){(void) inOperandList; return PDFHummus::eSuccess;}	
	
	virtual CharString* GetLocalSubr(long inSubrIndex) {(void) inSubrIndex; return NULL;}
	virtual CharString* GetGlobalSubr(long inSubrIndex){(void) inSubrIndex; return NULL;}
};
