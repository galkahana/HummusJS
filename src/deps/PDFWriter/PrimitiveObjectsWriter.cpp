/*
   Source File : PrimitiveObjectsWriter.cpp


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
#include "PrimitiveObjectsWriter.h"
#include "SafeBufferMacrosDefs.h"
#include "IByteWriter.h"

using namespace IOBasicTypes;

PrimitiveObjectsWriter::PrimitiveObjectsWriter(IByteWriter* inStreamForWriting)
{
	mStreamForWriting = inStreamForWriting;
}

PrimitiveObjectsWriter::~PrimitiveObjectsWriter(void)
{
}

static const IOBasicTypes::Byte scSpace[] = {' '};
void PrimitiveObjectsWriter::WriteTokenSeparator(ETokenSeparator inSeparate)
{
	if(eTokenSeparatorSpace == inSeparate)
		mStreamForWriting->Write(scSpace,1);
	else if(eTokenSeparatorEndLine == inSeparate)
		EndLine();
}

static const IOBasicTypes::Byte scNewLine[2] = {'\r','\n'};
void PrimitiveObjectsWriter::EndLine()
{
	mStreamForWriting->Write(scNewLine,2);
}

void PrimitiveObjectsWriter::WriteKeyword(const std::string& inKeyword)
{
	mStreamForWriting->Write((const IOBasicTypes::Byte *)inKeyword.c_str(),inKeyword.size());
	EndLine();
}

static const IOBasicTypes::Byte scSlash[1] = {'/'};
static const std::string scSpecialChars("()<>[]{}/%#");
void PrimitiveObjectsWriter::WriteName(const std::string& inName,ETokenSeparator inSeparate)
{
/*
from the pdf reference:
This syntax is required to represent any of the delimiter or white-space characters or the number sign character itself; 
it is recommended but not required for characters whose codes are outside the range 33 (!) to 126 (~).
*/

	mStreamForWriting->Write(scSlash,1);

	IOBasicTypes::Byte buffer[5];
	std::string::const_iterator it = inName.begin();
	for(;it != inName.end();++it)
	{
		Byte aValue = *it;

		if(aValue < 33 || aValue > 126 || scSpecialChars.find(aValue) != scSpecialChars.npos)
		{
			SAFE_SPRINTF_1((char*)buffer,5,"#%02x",aValue); 
			mStreamForWriting->Write(buffer,strlen((char*)buffer));		
		}
		else
		{
			buffer[0] = aValue;
			mStreamForWriting->Write(buffer,1);
		}
	}

	WriteTokenSeparator(inSeparate);
}

void PrimitiveObjectsWriter::WriteInteger(long long inIntegerToken,ETokenSeparator inSeparate)
{
	char buffer[512];

	SAFE_SPRINTF_1(buffer,512,"%lld",inIntegerToken);
	mStreamForWriting->Write((const IOBasicTypes::Byte *)buffer,strlen(buffer));
	WriteTokenSeparator(inSeparate);
}

static const IOBasicTypes::Byte scLeftParanthesis[1] = {'('};
static const IOBasicTypes::Byte scRightParanthesis[1] = {')'};

void PrimitiveObjectsWriter::WriteUnsafeLiteralString(const std::string& inString,ETokenSeparator inSeparate)
{
	mStreamForWriting->Write(scLeftParanthesis,1);
	mStreamForWriting->Write((const IOBasicTypes::Byte *)inString.c_str(),inString.size());
	mStreamForWriting->Write(scRightParanthesis,1);
	WriteTokenSeparator(inSeparate);
}

void PrimitiveObjectsWriter::WriteLiteralString(const std::string& inString,ETokenSeparator inSeparate)
{
	mStreamForWriting->Write(scLeftParanthesis,1);
	// doing some string conversion, so that charachters are written as safe ones.
	IOBasicTypes::Byte buffer[5];
	std::string::const_iterator it = inString.begin();
	for(;it != inString.end();++it)
	{
		Byte aValue = *it;
		if(aValue == '(' || aValue == ')' || aValue == '\\')
		{
			buffer[0] = '\\';
			buffer[1] = aValue;
			mStreamForWriting->Write(buffer,2);
		}
		else if (aValue < 32 || aValue > 126) // grabbing all nonprintable chars
		{
			SAFE_SPRINTF_1((char*)buffer,5,"\\%03o",aValue); 
			mStreamForWriting->Write(buffer,4);		
		}
		else
		{
			buffer[0] = aValue;
			mStreamForWriting->Write(buffer,1);
		}
		
	}
	mStreamForWriting->Write(scRightParanthesis,1);
	WriteTokenSeparator(inSeparate);
}

void PrimitiveObjectsWriter::WriteDouble(double inDoubleToken,ETokenSeparator inSeparate)
{
	char buffer[512];

	SAFE_SPRINTF_1(buffer,512,"%lf",inDoubleToken);

	LongBufferSizeType sizeToWrite = DetermineDoubleTrimmedLength(buffer);

	mStreamForWriting->Write((const IOBasicTypes::Byte *)buffer,sizeToWrite);
	WriteTokenSeparator(inSeparate);
}

size_t PrimitiveObjectsWriter::DetermineDoubleTrimmedLength(const char* inBufferWithDouble)
{
	size_t result = strlen(inBufferWithDouble);

	// remove all ending 0's
	while(result > 0 && inBufferWithDouble[result-1] == '0')
		--result;

	// if it's actually an integer, remove also decimal point
	if(result > 0 && inBufferWithDouble[result-1] == '.')
		--result;
	return result;
}

static const IOBasicTypes::Byte scTrue[4] = {'t','r','u','e'};
static const IOBasicTypes::Byte scFalse[5] = {'f','a','l','s','e'};

void PrimitiveObjectsWriter::WriteBoolean(bool inBoolean,ETokenSeparator inSeparate)
{
	if(inBoolean)
		mStreamForWriting->Write(scTrue,4);
	else
		mStreamForWriting->Write(scFalse,5);
	WriteTokenSeparator(inSeparate);
}

static const IOBasicTypes::Byte scNull[4] = {'n','u','l','l'};
void PrimitiveObjectsWriter::WriteNull(ETokenSeparator inSeparate)
{
	mStreamForWriting->Write(scNull,4);	
	WriteTokenSeparator(inSeparate);
}


void PrimitiveObjectsWriter::SetStreamForWriting(IByteWriter* inStreamForWriting)
{
	mStreamForWriting = inStreamForWriting;
}

static const IOBasicTypes::Byte scOpenBracketSpace[2] = {'[',' '};
void PrimitiveObjectsWriter::StartArray()
{
	mStreamForWriting->Write(scOpenBracketSpace,2);
}

static const IOBasicTypes::Byte scCloseBracket[1] = {']'};
void PrimitiveObjectsWriter::EndArray(ETokenSeparator inSeparate)
{
	mStreamForWriting->Write(scCloseBracket,1);
	WriteTokenSeparator(inSeparate);
}

static const IOBasicTypes::Byte scLeftAngle[1] = {'<'};
static const IOBasicTypes::Byte scRightAngle[1] = {'>'};
void PrimitiveObjectsWriter::WriteHexString(const std::string& inString,ETokenSeparator inSeparate)
{
	mStreamForWriting->Write(scLeftAngle,1);
	mStreamForWriting->Write((const IOBasicTypes::Byte *)inString.c_str(),inString.size());
	mStreamForWriting->Write(scRightAngle,1);
	WriteTokenSeparator(inSeparate);
}

IByteWriter* PrimitiveObjectsWriter::GetWritingStream()
{
    return mStreamForWriting;
}
