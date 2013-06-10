/*
   Source File : CharStringDefinitions.h


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

#include "IOBasicTypes.h"
#include <list>

using namespace IOBasicTypes;


struct IndexElement
{
	IndexElement() {mStartPosition=0; mEndPosition=0; mIndex=0;}

	LongFilePositionType mStartPosition;
	LongFilePositionType mEndPosition;
	unsigned short mIndex;
};

typedef IndexElement CharString;

struct CharStringOperand
{
	bool IsInteger;
	union
	{
		long IntegerValue;
		double RealValue;
	};
};

typedef std::list<CharStringOperand> CharStringOperandList;
