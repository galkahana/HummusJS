/*
   Source File : PSBool.h


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

#include "BoxingBase.h"

class PSBoolReader
{
public:
	void Read(const std::string& inReadFrom,bool& outValue);
	void Read(const std::wstring& inReadFrom,bool& outValue);
};

class PSBoolWriter
{
public:
	void Write(const bool& inValue,std::string& outWriteTo);
	void Write(const bool& inValue,std::wstring& outWriteTo);
};

typedef BoxingBaseWithRW<bool,PSBoolReader,PSBoolWriter> PSBool;

