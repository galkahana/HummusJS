/*
   Source File : PDFModifiedPage.h


   Copyright 2013 Gal Kahana PDFWriter

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

#include "EStatusCode.h"
#include "PDFRectangle.h"
#include <vector>
#include <string>

class AbstractContentContext;
class PDFWriter;
class PDFFormXObject;
class PDFDictionary;
class PDFDocumentCopyingContext;
class ObjectsContext;
class ResourcesDictionary;
class PDFParser;
class PDFObject;

typedef std::vector<PDFFormXObject*> PDFFormXObjectVector;

#pragma once
class PDFModifiedPage
{
public:
	PDFModifiedPage(PDFWriter* inWriter,unsigned long inPageIndex,bool inEnsureContentEncapsulation = false);
	~PDFModifiedPage(void);

	AbstractContentContext* StartContentContext();
	PDFHummus::EStatusCode PauseContentContext();
	PDFHummus::EStatusCode EndContentContext();
	AbstractContentContext* GetContentContext();


	PDFHummus::EStatusCode AttachURLLinktoCurrentPage(const std::string& inURL, const PDFRectangle& inLinkClickArea);

	PDFHummus::EStatusCode WritePage();


	// advanced
	ResourcesDictionary* GetCurrentResourcesDictionary();
	PDFFormXObject* GetCurrentFormContext();
private:

	PDFWriter* mWriter;
	unsigned long mPageIndex;
	bool mEnsureContentEncapsulation;
	PDFFormXObject* mCurrentContext;
	PDFFormXObjectVector mContenxts;
	bool mIsDirty;

	std::vector<std::string> WriteModifiedResourcesDict(PDFParser* inParser,PDFDictionary* inResourcesDictionary,ObjectsContext& inObjectContext,PDFDocumentCopyingContext* inCopyingContext);
	unsigned char GetDifferentChar(unsigned char);
	std::vector<std::string> WriteNewResourcesDictionary(ObjectsContext& inObjectContext);
	PDFObject* findInheritedResources(PDFParser* inParser,PDFDictionary* inDictionary);

};

