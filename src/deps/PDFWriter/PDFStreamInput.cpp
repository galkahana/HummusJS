/*
   Source File : PDFStreamInput.cpp


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
#include "PDFStreamInput.h"
#include "PDFDictionary.h"
#include "PDFName.h"

PDFStreamInput::PDFStreamInput(PDFDictionary* inStreamDictionary,LongFilePositionType inStreamContentStart) : PDFObject(eType)
{
	mDictionary = inStreamDictionary;
	mStreamContentStart = inStreamContentStart;
}

PDFStreamInput::~PDFStreamInput(void)
{
	mDictionary->Release();
}

PDFDictionary* PDFStreamInput::QueryStreamDictionary()
{
	mDictionary->AddRef();
	return mDictionary;
}

LongFilePositionType PDFStreamInput::GetStreamContentStart()
{
	return mStreamContentStart;
}

