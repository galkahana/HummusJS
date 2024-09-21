/*
   Source File : XObjectContentContext.h


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
#include "AbstractContentContext.h"

class PDFFormXObject;

class XObjectContentContext : public AbstractContentContext
{
public:
	XObjectContentContext(PDFHummus::DocumentContext* inDocumentContext,PDFFormXObject* inFormXObject);
	virtual ~XObjectContentContext(void);

private:

	// AbstractContentContext implementation
	virtual ResourcesDictionary* GetResourcesDictionary();
	virtual void ScheduleImageWrite(const std::string& inImagePath,unsigned long inImageIndex,ObjectIDType inObjectID, const PDFParsingOptions& inParsingOptions);
    virtual void ScheduleObjectEndWriteTask(IObjectEndWritingTask* inObjectEndWritingTask);

	PDFFormXObject* mPDFFormXObjectOfContext;
};
