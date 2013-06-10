/*
   Source File : PDFBoolean.cpp


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
#include "PDFBoolean.h"

PDFBoolean::PDFBoolean(bool inValue):PDFObject(eType)
{
	mValue = inValue;
}

PDFBoolean::~PDFBoolean(void)
{
}

bool PDFBoolean::GetValue() const
{
	return mValue;
}

PDFBoolean::operator bool() const
{
	return mValue;
}