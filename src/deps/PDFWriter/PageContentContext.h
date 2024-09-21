/*
   Source File : PageContentContext.h


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

class PDFPage;
class ObjectsContext;
class PDFStream;

class PageContentContext : public AbstractContentContext
{
public:
	PageContentContext(PDFHummus::DocumentContext* inDocumentContext,PDFPage* inPageOfContext,ObjectsContext* inObjectsContext);
	virtual ~PageContentContext(void);

	// Finish writing a current stream, if exists and flush to the main PDF stream
	PDFHummus::EStatusCode FinalizeCurrentStream();

	// Extensibility method, retrieves the current content stream for writing. if one does not exist - creates it.
	PDFStream* GetCurrentPageContentStream();

	// Extensibility method, forces creation of a new stream, if one does not exist now.
	void StartAStreamIfRequired();

	// Extensibility method, get the page to which this content is associated
	PDFPage* GetAssociatedPage();
    
private:
	PDFPage* mPageOfContext;
	ObjectsContext* mObjectsContext;
	PDFStream* mCurrentStream;

	PDFHummus::EStatusCode FinalizeStreamWriteAndRelease();
	void StartContentStreamDefinition();

	// AbstractContentContext implementation
	virtual ResourcesDictionary* GetResourcesDictionary();
	virtual void RenewStreamConnection();
	virtual void ScheduleImageWrite(const std::string& inImagePath,unsigned long inImageIndex,ObjectIDType inObjectID, const PDFParsingOptions& inParsingOptions);
	virtual void ScheduleObjectEndWriteTask(IObjectEndWritingTask* inObjectEndWritingTask);
};
