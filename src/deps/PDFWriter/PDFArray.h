/*
   Source File : PDFArray.h


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
#include "PDFObject.h"
#include "SingleValueContainerIterator.h"

#include <vector>



typedef std::vector<PDFObject*> PDFObjectVector;

class PDFArray : public PDFObject
{
public:

	enum EType
	{
		eType = ePDFObjectArray
	};

	PDFArray(void);
	virtual ~PDFArray(void);

	// Will add to end, calls AddRef
	void AppendObject(PDFObject* inObject);
	
	// Returns an object for iterating the array
	SingleValueContainerIterator<PDFObjectVector> GetIterator();

	// Returns object at a given index, calls AddRef
	PDFObject* QueryObject(unsigned long i);
	unsigned long GetLength();

private:
	PDFObjectVector mValues;
};
