/*
   Source File : TrueTypePrimitiveWriter.h


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
#include "IOBasicTypes.h"

using namespace IOBasicTypes;

class OutputStringBufferStream;

class TrueTypePrimitiveWriter
{
public:
	TrueTypePrimitiveWriter(OutputStringBufferStream* inTrueTypeFile = NULL);
	~TrueTypePrimitiveWriter(void);

	void SetOpenTypeStream(OutputStringBufferStream* inTrueTypeFile);
	
	PDFHummus::EStatusCode GetInternalState();

	PDFHummus::EStatusCode WriteBYTE(Byte inValue);
	PDFHummus::EStatusCode WriteULONG(unsigned long inValue);
	PDFHummus::EStatusCode WriteUSHORT(unsigned short inValue);
	PDFHummus::EStatusCode WriteSHORT(short inValue);

	PDFHummus::EStatusCode Pad(int inCount);
	PDFHummus::EStatusCode PadTo4();

private:
	OutputStringBufferStream* mTrueTypeFile;
	PDFHummus::EStatusCode mInternalState;

};
