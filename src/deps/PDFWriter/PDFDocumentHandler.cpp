/*
   Source File : PDFDocumentHandler.cpp


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
#include "PDFDocumentHandler.h"
#include "Trace.h"
#include "RefCountPtr.h"
#include "PDFObjectCast.h"
#include "PDFArray.h"
#include "PDFInteger.h"
#include "PDFReal.h"
#include "PDFDictionary.h"
#include "DocumentContext.h"
#include "PDFFormXObject.h"
#include "PDFStream.h"
#include "OutputStreamTraits.h"
#include "PDFStreamInput.h"
#include "InputFlateDecodeStream.h"
#include "ObjectsContext.h"
#include "PDFIndirectObjectReference.h"
#include "PDFBoolean.h"
#include "PDFSymbol.h"
#include "DictionaryContext.h"
#include "PDFLiteralString.h"
#include "PDFHexString.h"
#include "PrimitiveObjectsWriter.h"
#include "PageContentContext.h"
#include "PDFPage.h"
#include "PDFParserTokenizer.h"
#include "InputStreamSkipperStream.h"
#include "IResourceWritingTask.h"
#include "IFormEndWritingTask.h"
#include "PDFPageInput.h"
#include "IndirectObjectsReferenceRegistry.h"
#include "SimpleStringTokenizer.h"

using namespace PDFHummus;

PDFDocumentHandler::PDFDocumentHandler(void)
{
	mObjectsContext = NULL;
	mDocumentContext = NULL;
	mWrittenPage = NULL;
    mParser = NULL;
    mParserOwned = false;

}

PDFDocumentHandler::~PDFDocumentHandler(void)
{
    if(mParserOwned)
        delete mParser;
}

void PDFDocumentHandler::SetOperationsContexts(DocumentContext* inDocumentContext,
											   ObjectsContext* inObjectsContext)
{
	mObjectsContext = inObjectsContext;
	mDocumentContext = inDocumentContext;
}

class IPageEmbedInFormCommand
{
public:

	virtual PDFFormXObject* CreatePDFFormXObjectForPage(PDFDocumentHandler* inDocumentHandler,unsigned long i,const double* inTransformationMatrix, ObjectIDType inPredefinedFormId) = 0;
};

class PageEmbedInFormWithCropBox: public IPageEmbedInFormCommand
{
public:
	PageEmbedInFormWithCropBox(const PDFRectangle& inCropBox)
	{
		mCropBox = inCropBox;
	}

	PDFFormXObject* CreatePDFFormXObjectForPage(PDFDocumentHandler* inDocumentHandler,unsigned long i,const double* inTransformationMatrix, ObjectIDType inPredefinedFormId)
	{
		return inDocumentHandler->CreatePDFFormXObjectForPage(i,mCropBox,inTransformationMatrix,inPredefinedFormId);
	}

private:
	PDFRectangle mCropBox;

};

class PageEmbedInFormWithPageBox: public IPageEmbedInFormCommand
{
public:
	PageEmbedInFormWithPageBox(EPDFPageBox inPageBoxToUseAsFormBox)
	{
		mPageBoxToUseAsFormBox = inPageBoxToUseAsFormBox;
	}

	PDFFormXObject* CreatePDFFormXObjectForPage(PDFDocumentHandler* inDocumentHandler,unsigned long i,const double* inTransformationMatrix, ObjectIDType inPredefinedFormId)
	{
		return inDocumentHandler->CreatePDFFormXObjectForPage(i,mPageBoxToUseAsFormBox,inTransformationMatrix,inPredefinedFormId);
	}

private:
	EPDFPageBox mPageBoxToUseAsFormBox;

};


EStatusCodeAndObjectIDTypeList PDFDocumentHandler::CreateFormXObjectsFromPDF(	const std::string& inPDFFilePath,
																				const PDFParsingOptions& inParsingOptions,
																				const PDFPageRange& inPageRange,
																				IPageEmbedInFormCommand* inPageEmbedCommand,
																				const double* inTransformationMatrix,
																				const ObjectIDTypeList& inCopyAdditionalObjects,
																				const ObjectIDTypeList& inPredefinedFormIDs)
{
	if(StartFileCopyingContext(inPDFFilePath, inParsingOptions) != PDFHummus::eSuccess)
	{
		return EStatusCodeAndObjectIDTypeList(PDFHummus::eFailure,ObjectIDTypeList());
	}

	return CreateFormXObjectsFromPDFInContext(inPageRange,inPageEmbedCommand,inTransformationMatrix,inCopyAdditionalObjects, inPredefinedFormIDs);
}

EStatusCodeAndObjectIDTypeList PDFDocumentHandler::CreateFormXObjectsFromPDFInContext(
																						const PDFPageRange& inPageRange,
																						IPageEmbedInFormCommand* inPageEmbedCommand,
																						const double* inTransformationMatrix,
																						const ObjectIDTypeList& inCopyAdditionalObjects,
																						const ObjectIDTypeList& inPredefinedFormIDs)
{
	EStatusCodeAndObjectIDTypeList result;

	do
	{
		IDocumentContextExtenderSet::iterator it = mExtenders.begin();
		for(; it != mExtenders.end() && PDFHummus::eSuccess == result.first; ++it)
		{
			result.first = (*it)->OnPDFParsingComplete(mObjectsContext,mDocumentContext,this);
			if(result.first != PDFHummus::eSuccess)
				TRACE_LOG("DocumentContext::CreateFormXObjectsFromPDFInContext, unexpected failure. extender declared failure after parsing page.");
		}

		// copy additional objects prior to pages, so we have them ready at page copying
		if(inCopyAdditionalObjects.size() > 0)
		{
			result.first = WriteNewObjects(inCopyAdditionalObjects);
			if(result.first != PDFHummus::eSuccess)
			{
				TRACE_LOG("PDFDocumentHandler::CreateFormXObjectsFromPDFInContext, failed copying additional objects");
				break;
			}
		}

		PDFFormXObject* newObject;

		if(PDFPageRange::eRangeTypeAll == inPageRange.mType)
		{
			ObjectIDTypeList::const_iterator itFormIDs = inPredefinedFormIDs.begin();
			for(unsigned long i=0; i < mParser->GetPagesCount() && PDFHummus::eSuccess == result.first; ++i)
			{
				newObject = inPageEmbedCommand->CreatePDFFormXObjectForPage(this,i,inTransformationMatrix, itFormIDs == inPredefinedFormIDs.end() ? 0 : *itFormIDs);
				if(itFormIDs != inPredefinedFormIDs.end())
					++itFormIDs;
				if(newObject)
				{
					result.second.push_back(newObject->GetObjectID());
					delete newObject;
				}
				else
				{
					TRACE_LOG1("PDFDocumentHandler::CreateFormXObjectsFromPDFInContext, failed to embed page %ld", i);
					result.first = PDFHummus::eFailure;
				}
			}
		}
		else
		{
			// eRangeTypeSpecific
			ULongAndULongList::const_iterator it = inPageRange.mSpecificRanges.begin();
			ObjectIDTypeList::const_iterator itFormIDs = inPredefinedFormIDs.begin();
			for(; it != inPageRange.mSpecificRanges.end() && PDFHummus::eSuccess == result.first;++it)
			{
				if(it->first <= it->second && it->second < mParser->GetPagesCount())
				{
					for(unsigned long i=it->first; i <= it->second && PDFHummus::eSuccess == result.first; ++i)
					{
						newObject = inPageEmbedCommand->CreatePDFFormXObjectForPage(this,i,inTransformationMatrix, itFormIDs == inPredefinedFormIDs.end() ? 0 : *itFormIDs);
						if (itFormIDs != inPredefinedFormIDs.end())
							++itFormIDs;
						if(newObject)
						{
							result.second.push_back(newObject->GetObjectID());
							delete newObject;
						}
						else
						{
							TRACE_LOG1("PDFDocumentHandler::CreateFormXObjectsFromPDFInContext, failed to embed page %ld", i);
							result.first = PDFHummus::eFailure;
						}
					}
				}
				else
				{
					TRACE_LOG3("PDFDocumentHandler::CreateFormXObjectsFromPDF, range mismatch. first = %ld, second = %ld, PDF page count = %ld", 
						it->first,
						it->second,
						mParser->GetPagesCount());
					result.first = PDFHummus::eFailure;
				}
			}
		}


	}while(false);

	IDocumentContextExtenderSet::iterator it = mExtenders.begin();
	for(; it != mExtenders.end() && PDFHummus::eSuccess == result.first; ++it)
	{
		result.first = (*it)->OnPDFCopyingComplete(mObjectsContext,mDocumentContext,this);
		if(result.first != PDFHummus::eSuccess)
			TRACE_LOG("DocumentContext::CreateFormXObjectsFromPDFInContext, unexpected failure. extender declared failure before finalizing copy.");
	}


	StopCopyingContext();

	return result;

}

EStatusCodeAndObjectIDTypeList PDFDocumentHandler::CreateFormXObjectsFromPDF(	const std::string& inPDFFilePath,
																				const PDFParsingOptions& inParsingOptions,
																				const PDFPageRange& inPageRange,
																				EPDFPageBox inPageBoxToUseAsFormBox,
																				const double* inTransformationMatrix,
																				const ObjectIDTypeList& inCopyAdditionalObjects,
																				const ObjectIDTypeList& inPredefinedFormIDs)
{
	PageEmbedInFormWithPageBox embedCommand(inPageBoxToUseAsFormBox);
	return CreateFormXObjectsFromPDF(inPDFFilePath,inParsingOptions,inPageRange,&embedCommand,inTransformationMatrix,inCopyAdditionalObjects,inPredefinedFormIDs);
}

EStatusCodeAndObjectIDTypeList PDFDocumentHandler::CreateFormXObjectsFromPDF(	const std::string& inPDFFilePath,
																				const PDFParsingOptions& inParsingOptions,
																				const PDFPageRange& inPageRange,
																				const PDFRectangle& inCropBox,
																				const double* inTransformationMatrix,
																				const ObjectIDTypeList& inCopyAdditionalObjects,
																				const ObjectIDTypeList& inPredefinedFormIDs)
{
	PageEmbedInFormWithCropBox embedCommand(inCropBox);
	return CreateFormXObjectsFromPDF(inPDFFilePath, inParsingOptions,inPageRange,&embedCommand,inTransformationMatrix,inCopyAdditionalObjects, inPredefinedFormIDs);
}

PDFFormXObject* PDFDocumentHandler::CreatePDFFormXObjectForPage(unsigned long inPageIndex,
																EPDFPageBox inPageBoxToUseAsFormBox,
																const double* inTransformationMatrix,
																ObjectIDType inPredefinedFormId)
{
	RefCountPtr<PDFDictionary> pageObject = mParser->ParsePage(inPageIndex);

	if(!pageObject)
	{
		TRACE_LOG1("PDFDocumentHandler::CreatePDFFormXObjectForPage, unhexpected exception, page index does not denote a page object. page index = %ld",inPageIndex);
		return NULL;
	}
	else
		return CreatePDFFormXObjectForPage(pageObject.GetPtr(),DeterminePageBox(pageObject.GetPtr(),inPageBoxToUseAsFormBox),inTransformationMatrix, inPredefinedFormId);
}

PDFFormXObject* PDFDocumentHandler::CreatePDFFormXObjectForPage(PDFDictionary* inPageObject,
																const PDFRectangle& inFormBox,
																const double* inTransformationMatrix,
																ObjectIDType inPredefinedFormId)
{
	PDFFormXObject* result = NULL;
	EStatusCode status = PDFHummus::eSuccess;

	IDocumentContextExtenderSet::iterator it = mExtenders.begin();
	for(; it != mExtenders.end() && PDFHummus::eSuccess == status; ++it)
	{
		status = (*it)->OnBeforeCreateXObjectFromPage(inPageObject,mObjectsContext,mDocumentContext,this);
		if(status != PDFHummus::eSuccess)
			TRACE_LOG("DocumentContext::CreatePDFFormXObjectForPage, unexpected failure. extender declared failure before writing page.");
	}
	if(status != PDFHummus::eSuccess)
		return NULL;

	do
	{
		if(CopyResourcesIndirectObjects(inPageObject) != PDFHummus::eSuccess)
			break;

		// Create a new form XObject
		result = inPredefinedFormId == 0 ? mDocumentContext->StartFormXObject(inFormBox,inTransformationMatrix): mDocumentContext->StartFormXObject(inFormBox,inPredefinedFormId,inTransformationMatrix);

		// copy the page content to the target XObject stream
		if(WritePageContentToSingleStream(result->GetContentStream()->GetWriteStream(),inPageObject) != PDFHummus::eSuccess)
		{
			delete result;
			result = NULL;
			break;
		}

		// resources dictionary is gonna be empty at this point...so we can use our own code to write the dictionary, by extending.
		// which will be a simple loop. note that at this point all indirect objects should have been copied, and have mapping
		mDocumentContext->AddDocumentContextExtender(this);
		mWrittenPage = inPageObject;

		if(mDocumentContext->EndFormXObjectNoRelease(result) != PDFHummus::eSuccess)
		{
			delete result;
			result = NULL;
			break;
		}

	}while(false);

	mWrittenPage = NULL;
	mDocumentContext->RemoveDocumentContextExtender(this);

	if(result)
	{
		IDocumentContextExtenderSet::iterator it = mExtenders.begin();
		for(; it != mExtenders.end() && PDFHummus::eSuccess == status; ++it)
		{
			status = (*it)->OnAfterCreateXObjectFromPage(result,inPageObject,mObjectsContext,mDocumentContext,this);
			if(status != PDFHummus::eSuccess)
				TRACE_LOG("DocumentContext::CreatePDFFormXObjectForPage, unexpected failure. extender declared failure after writing page.");
		}
		if(status != PDFHummus::eSuccess)
		{
			delete result;
			return NULL;
		}
		else
			return result;
	}
	else
		return result;
}

PDFRectangle PDFDocumentHandler::DeterminePageBox(PDFDictionary* inDictionary,EPDFPageBox inPageBoxType)
{
	PDFRectangle result;
    inDictionary->AddRef();
    PDFPageInput pageInput(mParser,inDictionary);

	switch(inPageBoxType)
	{
		case ePDFPageBoxMediaBox:
			result = pageInput.GetMediaBox();
            break;
		case ePDFPageBoxCropBox:
		{
            result = pageInput.GetCropBox();
			break;
		}
		case ePDFPageBoxBleedBox:
		{
            result = pageInput.GetBleedBox();
			break;
		}
		case ePDFPageBoxTrimBox:
		{
            result = pageInput.GetTrimBox();
			break;
		}
		case ePDFPageBoxArtBox:
		{
            result = pageInput.GetArtBox();
			break;
		}
	}
	return result;
}

PDFFormXObject* PDFDocumentHandler::CreatePDFFormXObjectForPage(unsigned long inPageIndex,
																const PDFRectangle& inCropBox,
																const double* inTransformationMatrix,
																ObjectIDType inPredefinedFormId)
{
	RefCountPtr<PDFDictionary> pageObject = mParser->ParsePage(inPageIndex);

	if(!pageObject)
	{
		TRACE_LOG1("PDFDocumentHandler::CreatePDFFormXObjectForPage, unhexpected exception, page index does not denote a page object. page index = %ld",inPageIndex);
		return NULL;
	}
	else
		return CreatePDFFormXObjectForPage(pageObject.GetPtr(),inCropBox,inTransformationMatrix, inPredefinedFormId);
}

EStatusCode PDFDocumentHandler::WritePageContentToSingleStream(IByteWriter* inTargetStream,PDFDictionary* inPageObject)
{
	EStatusCode status = PDFHummus::eSuccess;

	RefCountPtr<PDFObject> pageContent(mParser->QueryDictionaryObject(inPageObject,"Contents"));
    
    // for empty page, simply return
    if(!pageContent)
        return status;
    
	if(pageContent->GetType() == PDFObject::ePDFObjectStream)
	{
		status = WritePDFStreamInputToStream(inTargetStream,(PDFStreamInput*)pageContent.GetPtr());
	}
	else if(pageContent->GetType() == PDFObject::ePDFObjectArray)
	{
		SingleValueContainerIterator<PDFObjectVector> it = ((PDFArray*)pageContent.GetPtr())->GetIterator();
		PDFObjectCastPtr<PDFIndirectObjectReference> refItem;
		while(it.MoveNext() && status == PDFHummus::eSuccess)
		{
			refItem = it.GetItem();
			if(!refItem)
			{
				status = PDFHummus::eFailure;
				TRACE_LOG("PDFDocumentHandler::WritePageContentToSingleStream, content stream array contains non-refs");
				break;
			}
			PDFObjectCastPtr<PDFStreamInput> contentStream(mParser->ParseNewObject(refItem->mObjectID));
			if(!contentStream)
			{
				status = PDFHummus::eFailure;
				TRACE_LOG("PDFDocumentHandler::WritePageContentToSingleStream, content stream array contains references to non streams");
				break;
			}
			status = WritePDFStreamInputToStream(inTargetStream,contentStream.GetPtr());
			if(PDFHummus::eSuccess == status)
			{
				// separate the streams with a nice endline
				PrimitiveObjectsWriter primitivesWriter;
				primitivesWriter.SetStreamForWriting(inTargetStream);
				primitivesWriter.EndLine();
			}
		}
	}
	else
	{
		TRACE_LOG1("PDFDocumentHandler::WritePageContentToSingleStream, error copying page content, expected either array or stream, getting %s",PDFObject::scPDFObjectTypeLabel(pageContent->GetType()));
		status = PDFHummus::eFailure;
	}
	

	return status;
}

EStatusCode PDFDocumentHandler::WritePDFStreamInputToStream(IByteWriter* inTargetStream,PDFStreamInput* inSourceStream)
{
	IByteReader* streamReader = mParser->CreateInputStreamReader(inSourceStream);

	if(!streamReader)
		return PDFHummus::eFailure;

	mPDFStream->SetPosition(inSourceStream->GetStreamContentStart());

	OutputStreamTraits traits(inTargetStream);
	EStatusCode status = traits.CopyToOutputStream(streamReader);
	delete streamReader;
	return status;
}

EStatusCode PDFDocumentHandler::CopyResourcesIndirectObjects(PDFDictionary* inPage)
{
	// makes sure that all indirect references are copied. those will come from the resources dictionary.
	// this is how we go about this:
	// Loop the immediate entities of the resources dictionary. for each value (which may be indirect) do this:
	// if indirect, run CopyInDirectObject on it (passing its ID and a new ID at the target PDF (just allocate as you go))
	// if direct, let go.

	PDFObjectCastPtr<PDFDictionary> resources(FindPageResources(mParser, inPage));

	// k. no resources...as wierd as that might be...or just wrong...i'll let it be
	if(!resources)
		return PDFHummus::eSuccess;

	ObjectIDTypeList newObjectsToWrite;

	RegisterInDirectObjects(resources.GetPtr(),newObjectsToWrite);
	return WriteNewObjects(newObjectsToWrite);
}

EStatusCode PDFDocumentHandler::WriteNewObjects(const ObjectIDTypeList& inSourceObjectIDs)
{
	ObjectIDTypeSet writtenObjects;
	// note that any objects in inSourceObjectIDs are trusted for not having been copied yet!
	return WriteNewObjects(inSourceObjectIDs,writtenObjects);
}


EStatusCode PDFDocumentHandler::WriteNewObjects(const ObjectIDTypeList& inSourceObjectIDs,ObjectIDTypeSet& ioCopiedObjects)
{

	ObjectIDTypeList::const_iterator itNewObjects = inSourceObjectIDs.begin();
	EStatusCode status = PDFHummus::eSuccess;

	for(; itNewObjects != inSourceObjectIDs.end() && PDFHummus::eSuccess == status; ++itNewObjects)
	{
		// theoretically speaking, it could be that while one object was copied, another one in this array is already
		// copied, so make sure to check that these objects are still required for copying
		if(ioCopiedObjects.find(*itNewObjects) == ioCopiedObjects.end())
		{
			ObjectIDTypeToObjectIDTypeMap::iterator it = mSourceToTarget.find(*itNewObjects);
			if(it == mSourceToTarget.end())
			{
				ObjectIDType newObjectID = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
				it = mSourceToTarget.insert(ObjectIDTypeToObjectIDTypeMap::value_type(*itNewObjects,newObjectID)).first;
			}
			ioCopiedObjects.insert(*itNewObjects);
			status = CopyInDirectObject(*itNewObjects,it->second,ioCopiedObjects);
		}
	}
	return status;
}

void PDFDocumentHandler::RegisterInDirectObjects(PDFDictionary* inDictionary,ObjectIDTypeList& outNewObjects)
{
	MapIterator<PDFNameToPDFObjectMap> it(inDictionary->GetIterator());

	// i'm assuming keys are directs. i can move into indirects if that's important
	while(it.MoveNext())
	{
		if(it.GetValue()->GetType() == PDFObject::ePDFObjectIndirectObjectReference)
		{
			ObjectIDTypeToObjectIDTypeMap::iterator	itObjects = mSourceToTarget.find(((PDFIndirectObjectReference*)it.GetValue())->mObjectID);
			if(itObjects == mSourceToTarget.end())
				outNewObjects.push_back(((PDFIndirectObjectReference*)it.GetValue())->mObjectID);
		} 
		else if(it.GetValue()->GetType() == PDFObject::ePDFObjectArray)
		{
			RegisterInDirectObjects((PDFArray*)it.GetValue(),outNewObjects);
		}
		else if(it.GetValue()->GetType() == PDFObject::ePDFObjectDictionary)
		{
			RegisterInDirectObjects((PDFDictionary*)it.GetValue(),outNewObjects);
		}
	}
}

void PDFDocumentHandler::RegisterInDirectObjects(PDFArray* inArray,ObjectIDTypeList& outNewObjects)
{
	SingleValueContainerIterator<PDFObjectVector> it(inArray->GetIterator());

	while(it.MoveNext())
	{
		if(it.GetItem()->GetType() == PDFObject::ePDFObjectIndirectObjectReference)
		{
			ObjectIDTypeToObjectIDTypeMap::iterator	itObjects = mSourceToTarget.find(((PDFIndirectObjectReference*)it.GetItem())->mObjectID);
			if(itObjects == mSourceToTarget.end())
				outNewObjects.push_back(((PDFIndirectObjectReference*)it.GetItem())->mObjectID);
		} 
		else if(it.GetItem()->GetType() == PDFObject::ePDFObjectArray)
		{
			RegisterInDirectObjects((PDFArray*)it.GetItem(),outNewObjects);
		}
		else if(it.GetItem()->GetType() == PDFObject::ePDFObjectDictionary)
		{
			RegisterInDirectObjects((PDFDictionary*)it.GetItem(),outNewObjects);
		}
	}
}

EStatusCode PDFDocumentHandler::CopyInDirectObject(ObjectIDType inSourceObjectID,ObjectIDType inTargetObjectID)
{
	ObjectIDTypeSet ioCopiedObjects;
	return CopyInDirectObject(inSourceObjectID,inTargetObjectID,ioCopiedObjects);
}


EStatusCode PDFDocumentHandler::CopyInDirectObject(ObjectIDType inSourceObjectID,ObjectIDType inTargetObjectID,ObjectIDTypeSet& ioCopiedObjects)
{
	// CopyInDirectObject will do this (lissen up)
	// Start a new object with the input ID
	// Write the object according to type. For arrays/dicts there might be indirect objects to copy. for them do this:
	// if you got it in the map already, just write down the new ID. if not register the ID, with a new ID already allocated at this point.
	// Once done. loop what you registered, using CopyInDirectObject in the same manner. This should maintain that each object is written separately
	EStatusCode status;
	ObjectIDTypeList newObjectsToWrite;
	OutWritingPolicy writingPolicy(this, newObjectsToWrite);

	RefCountPtr<PDFObject> sourceObject = mParser->ParseNewObject(inSourceObjectID);
	if(!sourceObject)
	{
		XrefEntryInput* xrefEntry = mParser->GetXrefEntry(inSourceObjectID);
		if ((xrefEntry != NULL) && (xrefEntry->mType == eXrefEntryDelete)) {
			// if the object is deleted, replace with a deleted object
			mObjectsContext->GetInDirectObjectsRegistry().DeleteObject(inTargetObjectID);
			return PDFHummus::eSuccess;
		}
		else {
			// fail
			TRACE_LOG1("PDFDocumentHandler::CopyInDirectObject, object not found. %ld", inSourceObjectID);
			return PDFHummus::eFailure;
		}
	}

	mObjectsContext->StartNewIndirectObject(inTargetObjectID);
	status = WriteObjectByType(sourceObject.GetPtr(),eTokenSeparatorEndLine, &writingPolicy);
	if(PDFHummus::eSuccess == status)
	{
		if (sourceObject->GetType() != PDFObject::ePDFObjectStream) // write indirect object end for non streams only...cause they take care of writing their own
			mObjectsContext->EndIndirectObject();
		return WriteNewObjects(newObjectsToWrite,ioCopiedObjects);
	}
	else
		return status;
}

EStatusCode PDFDocumentHandler::OnResourcesWrite(
						ResourcesDictionary* inResources,
						DictionaryContext* inPageResourcesDictionaryContext,
						ObjectsContext* inPDFWriterObjectContext,
						DocumentContext* inPDFWriterDocumentContext)
{
	// Writing resources dictionary. simply loop internal elements and copy. nicely enough, i can use read methods, trusting
	// that no new objects need be written
	
	PDFObjectCastPtr<PDFDictionary> resources(FindPageResources(mParser, mWrittenPage));
	ObjectIDTypeList dummyObjectList; // this one should remain empty...

	// k. no resources...as wierd as that might be...or just wrong...i'll let it be
	if(!resources)
		return PDFHummus::eSuccess;

	MapIterator<PDFNameToPDFObjectMap> it(resources->GetIterator());
	EStatusCode status = PDFHummus::eSuccess;
	OutWritingPolicy writingPolicy(this, dummyObjectList);

	while(it.MoveNext() && PDFHummus::eSuccess == status)
	{
		status = inPageResourcesDictionaryContext->WriteKey(it.GetKey()->GetValue());
		if(PDFHummus::eSuccess == status)
			status = WriteObjectByType(it.GetValue(),eTokenSeparatorEndLine, &writingPolicy);
	}
	return status;
}

EStatusCodeAndObjectIDTypeList PDFDocumentHandler::AppendPDFPagesFromPDF(const std::string& inPDFFilePath,
																		const PDFParsingOptions& inParsingOptions,
																		const PDFPageRange& inPageRange,
																		const ObjectIDTypeList& inCopyAdditionalObjects)
{
	if(StartFileCopyingContext(inPDFFilePath, inParsingOptions) != PDFHummus::eSuccess)
		return EStatusCodeAndObjectIDTypeList(PDFHummus::eFailure,ObjectIDTypeList());

	return AppendPDFPagesFromPDFInContext(inPageRange,inCopyAdditionalObjects);
}
	

EStatusCodeAndObjectIDTypeList PDFDocumentHandler::AppendPDFPagesFromPDFInContext(const PDFPageRange& inPageRange,
																					const ObjectIDTypeList& inCopyAdditionalObjects)
{
	EStatusCodeAndObjectIDTypeList result;

	do
	{

		IDocumentContextExtenderSet::iterator it = mExtenders.begin();
		for(; it != mExtenders.end() && PDFHummus::eSuccess == result.first; ++it)
		{
			result.first = (*it)->OnPDFParsingComplete(mObjectsContext,mDocumentContext,this);
			if(result.first != PDFHummus::eSuccess)
				TRACE_LOG("DocumentContext::AppendPDFPagesFromPDF, unexpected failure. extender declared failure after parsing page.");
		}

		// copy additional objects prior to pages, so we have them ready at page copying
		if(inCopyAdditionalObjects.size() > 0)
		{
			result.first = WriteNewObjects(inCopyAdditionalObjects);
			if(result.first != PDFHummus::eSuccess)
			{
				TRACE_LOG("PDFDocumentHandler::AppendPDFPagesFromPDF, failed copying additional objects");
				break;
			}
		}

		EStatusCodeAndObjectIDType newObject;

		if(PDFPageRange::eRangeTypeAll == inPageRange.mType)
		{
			for(unsigned long i=0; i < mParser->GetPagesCount() && PDFHummus::eSuccess == result.first; ++i)
			{
				newObject = CreatePDFPageForPage(i);
				if(PDFHummus::eSuccess == newObject.first)
				{
					result.second.push_back(newObject.second);
				}
				else
				{
					TRACE_LOG1("PDFDocumentHandler::AppendPDFPagesFromPDF, failed to embed page %ld", i);
					result.first = PDFHummus::eFailure;
				}
			}
		}
		else
		{
			// eRangeTypeSpecific
			ULongAndULongList::const_iterator it = inPageRange.mSpecificRanges.begin();
			for(; it != inPageRange.mSpecificRanges.end() && PDFHummus::eSuccess == result.first;++it)
			{
				if(it->first <= it->second && it->second < mParser->GetPagesCount())
				{
					for(unsigned long i=it->first; i <= it->second && PDFHummus::eSuccess == result.first; ++i)
					{
						newObject = CreatePDFPageForPage(i);
						if(PDFHummus::eSuccess == newObject.first)
						{
							result.second.push_back(newObject.second);
						}
						else
						{
							TRACE_LOG1("PDFDocumentHandler::AppendPDFPagesFromPDF, failed to embed page %ld", i);
							result.first = PDFHummus::eFailure;
						}
					}
				}
				else
				{
					TRACE_LOG3("PDFDocumentHandler::AppendPDFPagesFromPDF, range mismatch. first = %ld, second = %ld, PDF page count = %ld", 
						it->first,
						it->second,
						mParser->GetPagesCount());
					result.first = PDFHummus::eFailure;
				}
			}
		}


	}while(false);

	IDocumentContextExtenderSet::iterator it = mExtenders.begin();
	for(; it != mExtenders.end() && PDFHummus::eSuccess == result.first; ++it)
	{
		result.first = (*it)->OnPDFCopyingComplete(mObjectsContext,mDocumentContext,this);
		if(result.first != PDFHummus::eSuccess)
			TRACE_LOG("DocumentContext::AppendPDFPagesFromPDF, unexpected failure. extender declared failure before finalizing copy.");
	}

	StopCopyingContext();

	return result;
}

EStatusCodeAndObjectIDType PDFDocumentHandler::CreatePDFPageForPage(unsigned long inPageIndex)
{
	RefCountPtr<PDFDictionary> pageObject = mParser->ParsePage(inPageIndex);
	EStatusCodeAndObjectIDType result;
	result.first = PDFHummus::eFailure;
	result.second = 0;

	if(!pageObject)
	{
		TRACE_LOG1("PDFDocumentHandler::CreatePDFPageForPage, unhexpected exception, page index does not denote a page object. page index = %ld",inPageIndex);
		return result;
	}

	EStatusCode status = PDFHummus::eSuccess;

	IDocumentContextExtenderSet::iterator it = mExtenders.begin();
	for(; it != mExtenders.end() && PDFHummus::eSuccess == status; ++it)
	{
		status = (*it)->OnBeforeCreatePageFromPage(pageObject.GetPtr(),mObjectsContext,mDocumentContext,this);
		if(status != PDFHummus::eSuccess)
			TRACE_LOG("DocumentContext::CreatePDFPageForPage, unexpected failure. extender declared failure before writing page.");
	}
	if(status != PDFHummus::eSuccess)
		return result;

	// Create a new form XObject
	PDFPage newPage;

	do
	{
		if(CopyResourcesIndirectObjects(pageObject.GetPtr()) != PDFHummus::eSuccess)
			break;
 
		PDFPageInput pageInput(mParser,pageObject);
		newPage.SetMediaBox(pageInput.GetMediaBox());
		PDFRectangle cropBox = pageInput.GetCropBox();
		if(cropBox != pageInput.GetMediaBox())
			newPage.SetCropBox(pageInput.GetCropBox());
		if(cropBox != pageInput.GetBleedBox())
			newPage.SetBleedBox(pageInput.GetBleedBox());
		if(cropBox != pageInput.GetArtBox())
			newPage.SetArtBox(pageInput.GetArtBox());
		if(cropBox != pageInput.GetTrimBox())
			newPage.SetTrimBox(pageInput.GetTrimBox());
		newPage.SetRotate(pageInput.GetRotate());

		// copy the page content to the target page content
		if(CopyPageContentToTargetPagePassthrough(&newPage,pageObject.GetPtr()) != PDFHummus::eSuccess)
			break;

		// resources dictionary is gonna be empty at this point...so we can use our own code to write the dictionary, by extending.
		// which will be a simple loop. note that at this point all indirect objects should have been copied, and have mapping
		mDocumentContext->AddDocumentContextExtender(this);
		mWrittenPage = pageObject.GetPtr();

		result = mDocumentContext->WritePage(&newPage);
		if(result.first != PDFHummus::eSuccess)
			break;

	}while(false);

	mWrittenPage = NULL;
	mDocumentContext->RemoveDocumentContextExtender(this);

	if(result.first == PDFHummus::eSuccess)
	{
		it = mExtenders.begin();
		for(; it != mExtenders.end() && PDFHummus::eSuccess == result.first; ++it)
		{
			result.first = (*it)->OnAfterCreatePageFromPage(&newPage,pageObject.GetPtr(),mObjectsContext,mDocumentContext,this);
			if(result.first != PDFHummus::eSuccess)
				TRACE_LOG("DocumentContext::CreatePDFFormXObjectForPage, unexpected failure. extender declared failure after writing page.");
		}
	}

	return result;	
}

EStatusCode PDFDocumentHandler::CopyPageContentToTargetPagePassthrough(PDFPage* inPage, PDFDictionary* inPageObject) 
{
	EStatusCode status = PDFHummus::eSuccess;

	RefCountPtr<PDFObject> pageContent(mParser->QueryDictionaryObject(inPageObject, "Contents"));

	// for empty page, do nothing
	if (!pageContent)
		return status;

	if (pageContent->GetType() == PDFObject::ePDFObjectStream)
	{
		PDFObjectCastPtr<PDFIndirectObjectReference> streamReference = inPageObject->QueryDirectObject("Contents");
		EStatusCodeAndObjectIDType copyObjectStatus = CopyObject(streamReference->mObjectID);
		status = copyObjectStatus.first;
		if (PDFHummus::eSuccess == status) {
			inPage->AddContentStreamReference(copyObjectStatus.second);
		}
	}
	else if (pageContent->GetType() == PDFObject::ePDFObjectArray)
	{
		SingleValueContainerIterator<PDFObjectVector> it = ((PDFArray*)pageContent.GetPtr())->GetIterator();
		PDFObjectCastPtr<PDFIndirectObjectReference> refItem;
		while (it.MoveNext() && status == PDFHummus::eSuccess)
		{
			refItem = it.GetItem();
			if (!refItem)
			{
				status = PDFHummus::eFailure;
				TRACE_LOG("PDFDocumentHandler::CopyPageContentToTargetPagePassthrough, content stream array contains non-refs");
				break;
			}
			EStatusCodeAndObjectIDType copyObjectStatus = CopyObject(refItem->mObjectID);
			status = copyObjectStatus.first;
			if (PDFHummus::eSuccess == status) {
				inPage->AddContentStreamReference(copyObjectStatus.second);
			}
		}
	}
	else
	{
		TRACE_LOG1("PDFDocumentHandler::CopyPageContentToTargetPagePassthrough, error copying page content, expected either array or stream, getting %s", PDFObject::scPDFObjectTypeLabel(pageContent->GetType()));
		status = PDFHummus::eFailure;
	}

	return status;
}

EStatusCode PDFDocumentHandler::CopyPageContentToTargetPageRecoded(PDFPage* inPage,PDFDictionary* inPageObject)
{
	EStatusCode status = PDFHummus::eSuccess;
    
	RefCountPtr<PDFObject> pageContent(mParser->QueryDictionaryObject(inPageObject,"Contents"));
    
    // for empty page, do nothing
    if(!pageContent)
        return status;
    
	PageContentContext* pageContentContext = mDocumentContext->StartPageContentContext(inPage);
	if(pageContent->GetType() == PDFObject::ePDFObjectStream)
	{
		status = WritePDFStreamInputToContentContext(pageContentContext,(PDFStreamInput*)pageContent.GetPtr());
	}
	else if(pageContent->GetType() == PDFObject::ePDFObjectArray)
	{
		SingleValueContainerIterator<PDFObjectVector> it = ((PDFArray*)pageContent.GetPtr())->GetIterator();
		PDFObjectCastPtr<PDFIndirectObjectReference> refItem;
		while(it.MoveNext() && status == PDFHummus::eSuccess)
		{
			refItem = it.GetItem();
			if(!refItem)
			{
				status = PDFHummus::eFailure;
				TRACE_LOG("PDFDocumentHandler::CopyPageContentToTargetPageRecoded, content stream array contains non-refs");
				break;
			}
			PDFObjectCastPtr<PDFStreamInput> contentStream(mParser->ParseNewObject(refItem->mObjectID));
			if(!contentStream)
			{
				status = PDFHummus::eFailure;
				TRACE_LOG("PDFDocumentHandler::CopyPageContentToTargetPageRecoded, content stream array contains references to non streams");
				break;
			}
			status = WritePDFStreamInputToContentContext(pageContentContext,contentStream.GetPtr());
		}
	}
	else
	{
		TRACE_LOG1("PDFDocumentHandler::CopyPageContentToTargetPageRecoded, error copying page content, expected either array or stream, getting %s",PDFObject::scPDFObjectTypeLabel(pageContent->GetType()));
		status = PDFHummus::eFailure;
	}
	
	if(status != PDFHummus::eSuccess)
	{
		delete pageContentContext;
	}
	else
	{
		mDocumentContext->EndPageContentContext(pageContentContext);
	}
	return status;
}

EStatusCode PDFDocumentHandler::WritePDFStreamInputToContentContext(PageContentContext* inContentContext,PDFStreamInput* inContentSource)
{
	EStatusCode status = PDFHummus::eSuccess;
	
	do
	{
		inContentContext->StartAStreamIfRequired();

		status = WritePDFStreamInputToStream(inContentContext->GetCurrentPageContentStream()->GetWriteStream(),inContentSource);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("PDFDocumentHandler::WritePDFStreamInputToContentContext, failed to write content stream from page input to target page");
			break;
		}

		status = inContentContext->FinalizeCurrentStream();

	}while(false);

	return status;

}

EStatusCode PDFDocumentHandler::StartFileCopyingContext(const std::string& inPDFFilePath, const PDFParsingOptions& inOptions)
{
	if(mPDFFile.OpenFile(inPDFFilePath) != PDFHummus::eSuccess)
	{
		TRACE_LOG1("PDFDocumentHandler::StartFileCopyingContext, unable to open file for reading in %s",inPDFFilePath.c_str());
		return PDFHummus::eFailure;
	}

	return StartCopyingContext(mPDFFile.GetInputStream(), inOptions);
}

EStatusCode PDFDocumentHandler::StartCopyingContext(PDFParser* inPDFParser)
{
	EStatusCode status = eSuccess;
    
	do
	{
        if(mParser && mParserOwned)
            delete mParser;
        mParserOwned = false;
        mParser = inPDFParser;
		mPDFStream = inPDFParser->GetParserStream();
        
		if(mParser->IsEncrypted() && !mParser->IsEncryptionSupported())
		{
			TRACE_LOG("PDFDocumentHandler::StartCopyingContext, Document contains an unsupported encryption. Library does not support embedding of encrypted PDF that cant be decrypted");
			status = PDFHummus::eFailure;
			break;
		}
        
	}while(false);
    
	return status;    
}

EStatusCode PDFDocumentHandler::StartCopyingContext(IByteReaderWithPosition* inPDFStream, const PDFParsingOptions& inOptions)
{
	EStatusCode status;

	do
	{
        if(!mParserOwned || !mParser)
            mParser = new PDFParser();
		mPDFStream = inPDFStream;
        mParserOwned = true;

		status = mParser->StartPDFParsing(inPDFStream, inOptions);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("PDFDocumentHandler::StartCopyingContext, failure occured while parsing PDF file.");
			break;
		}

		if(mParser->IsEncrypted() && !mParser->IsEncryptionSupported())
		{
			TRACE_LOG("PDFDocumentHandler::StartCopyingContext, Cant decrypt document. make sure to provide appropriate password for this document in order to copy from it");
			status = PDFHummus::eFailure;
			break;
		}

	}while(false);

	return status;
}

EStatusCodeAndObjectIDType PDFDocumentHandler::CreateFormXObjectFromPDFPage(unsigned long inPageIndex,
																			EPDFPageBox inPageBoxToUseAsFormBox,
																			const double* inTransformationMatrix,
																			ObjectIDType inPredefinedFormId)
{
	EStatusCodeAndObjectIDType result;
	PDFFormXObject* newObject;

	if(inPageIndex < mParser->GetPagesCount())
	{
		newObject = CreatePDFFormXObjectForPage(inPageIndex,inPageBoxToUseAsFormBox,inTransformationMatrix, inPredefinedFormId);
		if(newObject)
		{
			result.first = PDFHummus::eSuccess;
			result.second = newObject->GetObjectID();
			delete newObject;
		}
		else
		{
			TRACE_LOG1("PDFDocumentHandler::CreateFormXObjectFromPDFPage, failed to embed page %ld",inPageIndex);
			result.first = PDFHummus::eFailure;
		}
	}
	else
	{
		TRACE_LOG2(
			"PDFDocumentHandler::CreateFormXObjectFromPDFPage, request object index %ld is larger than maximum page for input document = %ld", 
			inPageIndex,
			mParser->GetPagesCount()-1);
		result.first = PDFHummus::eFailure;
	}
	return result;
}

EStatusCodeAndObjectIDType PDFDocumentHandler::CreateFormXObjectFromPDFPage(unsigned long inPageIndex,
																			 const PDFRectangle& inCropBox,
																			 const double* inTransformationMatrix,
																			 ObjectIDType inPredefinedFormId)
{
	EStatusCodeAndObjectIDType result;
	PDFFormXObject* newObject;

	if(inPageIndex < mParser->GetPagesCount())
	{
		newObject = CreatePDFFormXObjectForPage(inPageIndex,inCropBox,inTransformationMatrix, inPredefinedFormId);
		if(newObject)
		{
			result.first = PDFHummus::eSuccess;
			result.second = newObject->GetObjectID();
			delete newObject;
		}
		else
		{
			TRACE_LOG1("PDFDocumentHandler::CreateFormXObjectFromPDFPage, failed to embed page %ld",inPageIndex);
			result.first = PDFHummus::eFailure;
		}
	}
	else
	{
		TRACE_LOG2(
			"PDFDocumentHandler::CreateFormXObjectFromPDFPage, request object index %ld is larger than maximum page for input document = %ld", 
			inPageIndex,
			mParser->GetPagesCount()-1);
		result.first = PDFHummus::eFailure;
	}
	return result;
}


EStatusCodeAndObjectIDType PDFDocumentHandler::AppendPDFPageFromPDF(unsigned long inPageIndex)
{
	EStatusCodeAndObjectIDType result;

	if(inPageIndex < mParser->GetPagesCount())
	{
		result = CreatePDFPageForPage(inPageIndex);
		if(result.first != PDFHummus::eSuccess)
			TRACE_LOG1("PDFDocumentHandler::AppendPDFPageFromPDF, failed to append page %ld",inPageIndex);
	}
	else
	{
		TRACE_LOG2(
			"PDFDocumentHandler::AppendPDFPageFromPDF, request object index %ld is larger than maximum page for input document = %ld", 
			inPageIndex,
			mParser->GetPagesCount()-1);
		result.first = PDFHummus::eFailure;
	}
	return result;
}

EStatusCodeAndObjectIDType PDFDocumentHandler::CopyObject(ObjectIDType inSourceObjectID)
{
	EStatusCodeAndObjectIDType result;

	ObjectIDTypeToObjectIDTypeMap::iterator it = mSourceToTarget.find(inSourceObjectID);
	if(it == mSourceToTarget.end())
	{
		ObjectIDTypeList anObjectList;
		anObjectList.push_back(inSourceObjectID);
		result.first = WriteNewObjects(anObjectList);
		result.second = mSourceToTarget[inSourceObjectID];
	}
	else
	{
		result.first = PDFHummus::eSuccess;
		result.second = it->second;
	}
	return result;
}

PDFParser* PDFDocumentHandler::GetSourceDocumentParser()
{
	return mParser;
}

EStatusCodeAndObjectIDType PDFDocumentHandler::GetCopiedObjectID(ObjectIDType inSourceObjectID)
{
	EStatusCodeAndObjectIDType result;

	ObjectIDTypeToObjectIDTypeMap::iterator it = mSourceToTarget.find(inSourceObjectID);
	if(it == mSourceToTarget.end())
	{
		result.first = PDFHummus::eFailure;
	}
	else
	{
		result.first = PDFHummus::eSuccess;
		result.second = it->second;
	}
	return result;
}

MapIterator<ObjectIDTypeToObjectIDTypeMap> PDFDocumentHandler::GetCopiedObjectsMappingIterator()
{
	return MapIterator<ObjectIDTypeToObjectIDTypeMap>(mSourceToTarget);
}

void PDFDocumentHandler::StopCopyingContext()
{
	mPDFFile.CloseFile();
	mPDFStream = NULL;
	// clearing the source to target mapping here. note that copying enjoyed sharing of objects between them
	mSourceToTarget.clear();
	if(mParserOwned)
    {
        if(mParser)
        {
            mParser->ResetParser();
            delete mParser;
        }
        mParser = NULL;
        mParserOwned = false;
    }

}

void PDFDocumentHandler::AddDocumentContextExtender(IDocumentContextExtender* inExtender)
{
	mExtenders.insert(inExtender);
}

void PDFDocumentHandler::RemoveDocumentContextExtender(IDocumentContextExtender* inExtender)
{
	mExtenders.erase(inExtender);
}


EStatusCode PDFDocumentHandler::MergePDFPagesToPage(PDFPage* inPage,
													const std::string& inPDFFilePath,
													const PDFParsingOptions& inParsingOptions,
													const PDFPageRange& inPageRange,
													const ObjectIDTypeList& inCopyAdditionalObjects)
{
	if(StartFileCopyingContext(inPDFFilePath, inParsingOptions) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	return MergePDFPagesToPageInContext(inPage,inPageRange,inCopyAdditionalObjects);
}

EStatusCode PDFDocumentHandler::MergePDFPagesToPageInContext(PDFPage* inPage,
															const PDFPageRange& inPageRange,
															const ObjectIDTypeList& inCopyAdditionalObjects)
{
	EStatusCode status = PDFHummus::eSuccess;

	do
	{
		IDocumentContextExtenderSet::iterator it = mExtenders.begin();
		for(; it != mExtenders.end() && PDFHummus::eSuccess == status; ++it)
		{
			status = (*it)->OnPDFParsingComplete(mObjectsContext,mDocumentContext,this);
			if(status != PDFHummus::eSuccess)
				TRACE_LOG("DocumentContext::MergePDFPagesToPage, unexpected failure. extender declared failure after parsing page.");
		}

	// copy additional objects prior to pages, so we have them ready at page merging
		if(inCopyAdditionalObjects.size() > 0)
		{
			status = WriteNewObjects(inCopyAdditionalObjects);
			if(status != PDFHummus::eSuccess)
			{
				TRACE_LOG("PDFDocumentHandler::MergePDFPagesToPage, failed copying additional objects");
				break;
			}
		}

		if(PDFPageRange::eRangeTypeAll == inPageRange.mType)
		{
			for(unsigned long i=0; i < mParser->GetPagesCount() && PDFHummus::eSuccess == status; ++i)
			{
				status = MergePDFPageForPage(inPage,i);
				if(status != PDFHummus::eSuccess)
					TRACE_LOG1("PDFDocumentHandler::MergePDFPagesToPage, failed to embed page %ld", i);
			}
		}
		else
		{
			// eRangeTypeSpecific
			ULongAndULongList::const_iterator it = inPageRange.mSpecificRanges.begin();
			for(; it != inPageRange.mSpecificRanges.end() && PDFHummus::eSuccess == status;++it)
			{
				if(it->first <= it->second && it->second < mParser->GetPagesCount())
				{
					for(unsigned long i=it->first; i <= it->second && PDFHummus::eSuccess == status; ++i)
					{
						status = MergePDFPageForPage(inPage,i);
						if(status != PDFHummus::eSuccess)
							TRACE_LOG1("PDFDocumentHandler::MergePDFPagesToPage, failed to embed page %ld", i);
					}
				}
				else
				{
					TRACE_LOG3("PDFDocumentHandler::MergePDFPagesToPage, range mismatch. first = %ld, second = %ld, PDF page count = %ld", 
						it->first,
						it->second,
						mParser->GetPagesCount());
					status = PDFHummus::eFailure;
				}
			}
		}


	}while(false);

	IDocumentContextExtenderSet::iterator it = mExtenders.begin();
	for(; it != mExtenders.end() && PDFHummus::eSuccess == status; ++it)
	{
		status = (*it)->OnPDFCopyingComplete(mObjectsContext,mDocumentContext,this);
		if(status != PDFHummus::eSuccess)
			TRACE_LOG("DocumentContext::MergePDFPagesToPage, unexpected failure. extender declared failure before finalizing copy.");
	}

	StopCopyingContext();

	return status;

}

EStatusCode PDFDocumentHandler::MergePDFPageForPage(PDFPage* inTargetPage,unsigned long inSourcePageIndex)
{
	RefCountPtr<PDFDictionary> pageObject = mParser->ParsePage(inSourcePageIndex);
	EStatusCode status  = PDFHummus::eSuccess;

	if(!pageObject)
	{
		TRACE_LOG1("PDFDocumentHandler::MergePDFPageForPage, unhexpected exception, page index does not denote a page object. page index = %ld",inSourcePageIndex);
		return PDFHummus::eFailure;
	}


	IDocumentContextExtenderSet::iterator it = mExtenders.begin();
	for(; it != mExtenders.end() && PDFHummus::eSuccess == status; ++it)
	{
		status = (*it)->OnBeforeMergePageFromPage(inTargetPage,pageObject.GetPtr(),mObjectsContext,mDocumentContext,this);
		if(status != PDFHummus::eSuccess)
			TRACE_LOG("DocumentContext::MergePDFPageForPage, unexpected failure. extender declared failure before writing page.");
	}
	if(status != PDFHummus::eSuccess)
		return status;

	do
	{
		StringToStringMap pageResourcesNamesMapping;

		// pause current content context on the stream, if such exists, to allow a safe and separate copying of the page content
		if(mDocumentContext->HasContentContext(inTargetPage))
		{
			status = mDocumentContext->PausePageContentContext(mDocumentContext->StartPageContentContext(inTargetPage));
			if(status != PDFHummus::eSuccess)
				break;
		}

		if(MergeResourcesToPage(inTargetPage,pageObject.GetPtr(),pageResourcesNamesMapping) != PDFHummus::eSuccess)
			break;

		// copy the page content to the target page content
		status = MergePageContentToTargetPage(inTargetPage,pageObject.GetPtr(),pageResourcesNamesMapping);

	}while(false);

	if(PDFHummus::eSuccess == status)
	{
		it = mExtenders.begin();
		for(; it != mExtenders.end() && PDFHummus::eSuccess == status; ++it)
		{
			status = (*it)->OnAfterMergePageFromPage(inTargetPage,pageObject.GetPtr(),mObjectsContext,mDocumentContext,this);
			if(status != PDFHummus::eSuccess)
				TRACE_LOG("DocumentContext::MergePDFPageForPage, unexpected failure. extender declared failure after writing page.");
		}
	}

	return status;	

}

EStatusCode PDFDocumentHandler::MergeResourcesToPage(PDFPage* inTargetPage,PDFDictionary* inPage,StringToStringMap& outMappedResourcesNames)
{
	// parse each individual resources dictionary separately and copy the resources. the output parameter should be used for old vs. new names
	
	PDFObjectCastPtr<PDFDictionary> resources(FindPageResources(mParser, inPage));

	// k. no resources...as wierd as that might be...or just wrong...i'll let it be
	if(!resources)
		return PDFHummus::eSuccess;

	EStatusCode status = PDFHummus::eSuccess;

	// ProcSet
	PDFObjectCastPtr<PDFArray> procsets(mParser->QueryDictionaryObject(resources.GetPtr(),"ProcSet"));
	if(procsets.GetPtr())
	{
		SingleValueContainerIterator<PDFObjectVector> it(procsets->GetIterator());
		while(it.MoveNext())
			inTargetPage->GetResourcesDictionary().AddProcsetResource(((PDFName*)it.GetItem())->GetValue());
	}


	do
	{

		// ExtGState
		PDFObjectCastPtr<PDFDictionary> extgstate(mParser->QueryDictionaryObject(resources.GetPtr(),"ExtGState"));
		if(extgstate.GetPtr())
		{	
			MapIterator<PDFNameToPDFObjectMap> it(extgstate->GetIterator());
			while(it.MoveNext() && PDFHummus::eSuccess == status)
			{
				// always copying to indirect object. with this method i'm forcing to write them now, not having to hold them in memory till i actually write the resource dictionary.
				EStatusCodeAndObjectIDType result = CopyObjectToIndirectObject(it.GetValue());
				if(result.first != PDFHummus::eSuccess)
					status = PDFHummus::eFailure;				
				outMappedResourcesNames.insert(
					StringToStringMap::value_type(
						AsEncodedName(it.GetKey()->GetValue()),
						inTargetPage->GetResourcesDictionary().AddExtGStateMapping(result.second)));
				
			}
			if(status != PDFHummus::eSuccess)
				break;
		}

		// ColorSpace
		PDFObjectCastPtr<PDFDictionary> colorspace(mParser->QueryDictionaryObject(resources.GetPtr(),"ColorSpace"));
		if(colorspace.GetPtr())
		{	
			MapIterator<PDFNameToPDFObjectMap> it(colorspace->GetIterator());
			while(it.MoveNext() && PDFHummus::eSuccess == status)
			{
				EStatusCodeAndObjectIDType result = CopyObjectToIndirectObject(it.GetValue());
				if(result.first != PDFHummus::eSuccess)
					status = PDFHummus::eFailure;				
				outMappedResourcesNames.insert(
					StringToStringMap::value_type(
						AsEncodedName(it.GetKey()->GetValue()),
						inTargetPage->GetResourcesDictionary().AddColorSpaceMapping(result.second)));
				
			}
			if(status != PDFHummus::eSuccess)
				break;
		}

		// Pattern
		PDFObjectCastPtr<PDFDictionary> pattern(mParser->QueryDictionaryObject(resources.GetPtr(),"Pattern"));
		if(pattern.GetPtr())
		{	
			MapIterator<PDFNameToPDFObjectMap> it(pattern->GetIterator());
			while(it.MoveNext() && PDFHummus::eSuccess == status)
			{
				EStatusCodeAndObjectIDType result = CopyObjectToIndirectObject(it.GetValue());
				if(result.first != PDFHummus::eSuccess)
					status = PDFHummus::eFailure;				
				outMappedResourcesNames.insert(
					StringToStringMap::value_type(
						AsEncodedName(it.GetKey()->GetValue()),
						inTargetPage->GetResourcesDictionary().AddPatternMapping(result.second)));
				
			}
			if(status != PDFHummus::eSuccess)
				break;
		}

		// Shading
		PDFObjectCastPtr<PDFDictionary> shading(mParser->QueryDictionaryObject(resources.GetPtr(),"Shading"));
		if(shading.GetPtr())
		{	
			MapIterator<PDFNameToPDFObjectMap> it(shading->GetIterator());
			while(it.MoveNext() && PDFHummus::eSuccess == status)
			{
				EStatusCodeAndObjectIDType result = CopyObjectToIndirectObject(it.GetValue());
				if(result.first != PDFHummus::eSuccess)
					status = PDFHummus::eFailure;				
				outMappedResourcesNames.insert(
					StringToStringMap::value_type(
						AsEncodedName(it.GetKey()->GetValue()),
						inTargetPage->GetResourcesDictionary().AddShadingMapping(result.second)));
				
			}
			if(status != PDFHummus::eSuccess)
				break;
		}

		// XObject
		PDFObjectCastPtr<PDFDictionary> xobject(mParser->QueryDictionaryObject(resources.GetPtr(),"XObject"));
		if(xobject.GetPtr())
		{	
			MapIterator<PDFNameToPDFObjectMap> it(xobject->GetIterator());
			while(it.MoveNext() && PDFHummus::eSuccess == status)
			{
				EStatusCodeAndObjectIDType result = CopyObjectToIndirectObject(it.GetValue());
				if(result.first != PDFHummus::eSuccess)
					status = PDFHummus::eFailure;				
				outMappedResourcesNames.insert(
					StringToStringMap::value_type(
						AsEncodedName(it.GetKey()->GetValue()),
						inTargetPage->GetResourcesDictionary().AddXObjectMapping(result.second)));
				
			}
			if(status != PDFHummus::eSuccess)
				break;
		}

		// Font
		PDFObjectCastPtr<PDFDictionary> font(mParser->QueryDictionaryObject(resources.GetPtr(),"Font"));
		if(font.GetPtr())
		{	
			MapIterator<PDFNameToPDFObjectMap> it(font->GetIterator());
			while(it.MoveNext() && PDFHummus::eSuccess == status)
			{
				EStatusCodeAndObjectIDType result = CopyObjectToIndirectObject(it.GetValue());
				if(result.first != PDFHummus::eSuccess)
					status = PDFHummus::eFailure;				
				outMappedResourcesNames.insert(
					StringToStringMap::value_type(
						AsEncodedName(it.GetKey()->GetValue()),
						inTargetPage->GetResourcesDictionary().AddFontMapping(result.second)));
				
			}
			if(status != PDFHummus::eSuccess)
				break;
		}

		// Properties
		PDFObjectCastPtr<PDFDictionary> properties(mParser->QueryDictionaryObject(resources.GetPtr(),"Properties"));
		if(properties.GetPtr())
		{	
			MapIterator<PDFNameToPDFObjectMap> it(properties->GetIterator());
			while(it.MoveNext() && PDFHummus::eSuccess == status)
			{
				EStatusCodeAndObjectIDType result = CopyObjectToIndirectObject(it.GetValue());
				if(result.first != PDFHummus::eSuccess)
					status = PDFHummus::eFailure;				
				outMappedResourcesNames.insert(
					StringToStringMap::value_type(
						AsEncodedName(it.GetKey()->GetValue()),
						inTargetPage->GetResourcesDictionary().AddPropertyMapping(result.second)));
				
			}
			if(status != PDFHummus::eSuccess)
				break;
		}

	}while(false);
	return status;
}

std::string PDFDocumentHandler::AsEncodedName(const std::string& inName)
{
	// for later comparisons and replacement, i'd like to have the name as it appears in a PDF stream, with all spaces encoded.
	// i know there's little chance that resource names will contain spaces...but i want to be safe.

	PrimitiveObjectsWriter primitiveWriter;
	OutputStringBufferStream aStringBuilder;

	primitiveWriter.SetStreamForWriting(&aStringBuilder);
	primitiveWriter.WriteName(inName,eTokenSeparatorNone);

	return aStringBuilder.ToString().substr(1); // return without initial forward slash
}

EStatusCodeAndObjectIDType PDFDocumentHandler::CopyObjectToIndirectObject(PDFObject* inObject)
{
	EStatusCodeAndObjectIDType result;
	if(inObject->GetType() != PDFObject::ePDFObjectIndirectObjectReference)
	{
		result.second = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
		result.first = CopyDirectObjectToIndirectObject(inObject,result.second);
	}
	else
	{
		ObjectIDTypeToObjectIDTypeMap::iterator	itObjects = mSourceToTarget.find(((PDFIndirectObjectReference*)inObject)->mObjectID);
		if(itObjects == mSourceToTarget.end())
		{
			result.second = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
			mSourceToTarget.insert(ObjectIDTypeToObjectIDTypeMap::value_type(((PDFIndirectObjectReference*)inObject)->mObjectID,result.second));
			result.first = CopyInDirectObject(((PDFIndirectObjectReference*)inObject)->mObjectID,result.second);
		}
		else
		{
			result.second = itObjects->second;
			result.first = PDFHummus::eSuccess;
		}
	}
	return result;

}

EStatusCode PDFDocumentHandler::CopyDirectObjectToIndirectObject(PDFObject* inObject,ObjectIDType inTargetObjectID)
{
	EStatusCode status;
	ObjectIDTypeList newObjectsToWrite;
	OutWritingPolicy writingPolicy(this, newObjectsToWrite);

	mObjectsContext->StartNewIndirectObject(inTargetObjectID);
	status = WriteObjectByType(inObject,eTokenSeparatorEndLine, &writingPolicy);
	if(PDFHummus::eSuccess == status)
	{
		if(inObject->GetType() != PDFObject::ePDFObjectStream) // write indirect object end for non streams only...cause they take care of writing their own
			mObjectsContext->EndIndirectObject();
		return WriteNewObjects(newObjectsToWrite);
	}
	else
		return status;
}

EStatusCode PDFDocumentHandler::MergePageContentToTargetPage(PDFPage* inTargetPage,PDFDictionary* inSourcePage,const StringToStringMap& inMappedResourcesNames)
{
	EStatusCode status = PDFHummus::eSuccess;

	RefCountPtr<PDFObject> pageContent(mParser->QueryDictionaryObject(inSourcePage,"Contents"));
    
    // for empty page, return now
    if(!pageContent)
        return status;

	bool hasAlreadyAContentContext = mDocumentContext->HasContentContext(inTargetPage);
	PageContentContext* pageContentContext = mDocumentContext->StartPageContentContext(inTargetPage);

	if(pageContent->GetType() == PDFObject::ePDFObjectStream)
	{
		status = WritePDFStreamInputToContentContext(pageContentContext,(PDFStreamInput*)pageContent.GetPtr(),inMappedResourcesNames);
	}
	else if(pageContent->GetType() == PDFObject::ePDFObjectArray)
	{
		SingleValueContainerIterator<PDFObjectVector> it = ((PDFArray*)pageContent.GetPtr())->GetIterator();
		PDFObjectCastPtr<PDFIndirectObjectReference> refItem;
		while(it.MoveNext() && status == PDFHummus::eSuccess)
		{
			refItem = it.GetItem();
			if(!refItem)
			{
				status = PDFHummus::eFailure;
				TRACE_LOG("PDFDocumentHandler::MergePageContentToTargetPage, content stream array contains non-refs");
				break;
			}
			PDFObjectCastPtr<PDFStreamInput> contentStream(mParser->ParseNewObject(refItem->mObjectID));
			if(!contentStream)
			{
				status = PDFHummus::eFailure;
				TRACE_LOG("PDFDocumentHandler::MergePageContentToTargetPage, content stream array contains references to non streams");
				break;
			}
			status = WritePDFStreamInputToContentContext(pageContentContext,contentStream.GetPtr(),inMappedResourcesNames);
		}
	}
	else
	{
		TRACE_LOG1("PDFDocumentHandler::MergePageContentToTargetPage, error copying page content, expected either array or stream, getting %s",PDFObject::scPDFObjectTypeLabel(pageContent->GetType()));
		status = PDFHummus::eFailure;
	}

	// this means that this function created the content context, in which case it owns it and should finalize it
	if(!hasAlreadyAContentContext)
		mDocumentContext->EndPageContentContext(pageContentContext);
	return status;	
}

EStatusCode PDFDocumentHandler::WritePDFStreamInputToContentContext(PageContentContext* inContentContext,PDFStreamInput* inContentSource,const StringToStringMap& inMappedResourcesNames)
{
	EStatusCode status = PDFHummus::eSuccess;
	
	do
	{
		inContentContext->StartAStreamIfRequired();

		status = WritePDFStreamInputToStream(inContentContext->GetCurrentPageContentStream()->GetWriteStream(),inContentSource,inMappedResourcesNames);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("PDFDocumentHandler::WritePDFStreamInputToContentContext, failed to write content stream from page input to target page");
			break;
		}

		status = inContentContext->FinalizeCurrentStream(); // i want to begin a new stream after this, to maintain some sort of consistency [if important] with the embedded pages

	}while(false);

	return status;
}

EStatusCode PDFDocumentHandler::WritePDFStreamInputToStream(IByteWriter* inTargetStream,PDFStreamInput* inSourceStream,const StringToStringMap& inMappedResourcesNames)
{
	// as oppose to regular copying, this copying has to replace name references that refer to mapped resources.
	// as such, the method of copying will be token by token, where each token is checked for being a resource reference.
	// the current assumption, somewhere between speed and safety compromise, is that the any name token that has a resource name is in fact a relevant
	// resource reference. 


	// in order to have minimal interferences with the stream content i'm going for two passes here.
	// the first pass will scan the stream for tokens to replace providing the tokens and their positions.
	// a second pass is then repeatedly copies the content between tokens that are to be replaced. this might be wasteful,
	// but is the safest method to get the content right.

	ResourceTokenMarkerList resourcesPositions;

	EStatusCode status = ScanStreamForResourcesTokens(inSourceStream,inMappedResourcesNames,resourcesPositions);
	if(status != PDFHummus::eSuccess)
		return status;

	return MergeAndReplaceResourcesTokens(inTargetStream,inSourceStream,inMappedResourcesNames,resourcesPositions);
}


static const char scSlash = '/';
EStatusCode PDFDocumentHandler::ScanStreamForResourcesTokens(PDFStreamInput* inSourceStream,const StringToStringMap& inMappedResourcesNames,ResourceTokenMarkerList& outResourceMarkers)
{
	IByteReader* streamReader = mParser->CreateInputStreamReader(inSourceStream);

	if(!streamReader)
		return PDFHummus::eFailure;

	mPDFStream->SetPosition(inSourceStream->GetStreamContentStart());

	// using simplestringtokenizer instead of regular pdfparsertokenizer, as content stream may contain
	// streams, which may have tokens that will be interpreted as pdf tokens (like string start), and so will cause
	// a wrong inclusion of content and so will skip content that should be replaced.
	// There's still risk here, in that there will be a string that like a resource name with forward slash which will be mistaken
	// for a resource usage. this is something to tackle still.
	SimpleStringTokenizer tokenizer;
	tokenizer.SetReadStream(streamReader);

	BoolAndString tokenizerResult;

	while(streamReader->NotEnded())
	{
		BoolAndString tokenizerResult = tokenizer.GetNextToken();

		if(!tokenizerResult.first)
			break;

		// check if this is a token that will need replacement - 1. verify that it's a name 2. verify that it's a name in the input map
		// note that here i don't have to take care of name space chars encoding, as the names are alrady encoded in the map [the new names are never containing space chars]
		if(tokenizerResult.second.at(0) == scSlash && 
			(inMappedResourcesNames.find(tokenizerResult.second.substr(1)) != inMappedResourcesNames.end()))
				outResourceMarkers.push_back(ResourceTokenMarker(tokenizerResult.second.substr(1),tokenizer.GetRecentTokenPosition()));
	}

	delete streamReader;
	return PDFHummus::eSuccess;
}

EStatusCode PDFDocumentHandler::MergeAndReplaceResourcesTokens(	IByteWriter* inTargetStream,
																PDFStreamInput* inSourceStream,
																const StringToStringMap& inMappedResourcesNames,
																const ResourceTokenMarkerList& inResourceMarkers)
{
	IByteReader* streamReader = mParser->CreateInputStreamReader(inSourceStream);

	if(!streamReader)
		return PDFHummus::eFailure;

	mPDFStream->SetPosition(inSourceStream->GetStreamContentStart());

	OutputStreamTraits traits(inTargetStream);
	PrimitiveObjectsWriter primitivesWriter;
	primitivesWriter.SetStreamForWriting(inTargetStream);
	EStatusCode status = PDFHummus::eSuccess;
	InputStreamSkipperStream skipper(streamReader);
	ResourceTokenMarkerList::const_iterator it = inResourceMarkers.begin();
	LongFilePositionType previousContentPosition = 0;

	for(; it != inResourceMarkers.end() && PDFHummus::eSuccess == status; ++it)
	{
		status = traits.CopyToOutputStream(streamReader,(LongBufferSizeType)(it->ResourceTokenPosition - previousContentPosition));
		if(status != PDFHummus::eSuccess)
			break;
		primitivesWriter.WriteName(inMappedResourcesNames.find(it->ResourceToken)->second,eTokenSeparatorNone);
		// note that i'm using SkipBy here. if i want to use SkipTo, i have to user the skipper stream as input stream for the rest
		// of the reader objects here, as SkipTo relies on information on how many bytes were read
		skipper.SkipBy(it->ResourceToken.size() + 1); // skip the resource name in the read stream [include +1 for slash]
		previousContentPosition = it->ResourceTokenPosition + it->ResourceToken.size() + 1;
	}

	// copy from last marker (or in case there are none - from the beginning), till end
	if(PDFHummus::eSuccess == status)
		status = traits.CopyToOutputStream(streamReader);
	
	skipper.Assign(NULL);
	delete streamReader;
	return status;
}


EStatusCode PDFDocumentHandler::MergePDFPageToPage(PDFPage* inTargetPage,unsigned long inSourcePageIndex)
{
	EStatusCode status;

	if(inSourcePageIndex < mParser->GetPagesCount())
	{
		status = MergePDFPageForPage(inTargetPage,inSourcePageIndex);
		if(status != PDFHummus::eSuccess)
			TRACE_LOG1("PDFDocumentHandler::MergePDFPageToPage, failed to merge page %ld",inSourcePageIndex);
	}
	else
	{
		TRACE_LOG2(
			"PDFDocumentHandler::MergePDFPageToPage, request object index %ld is larger than maximum page for input document = %ld", 
			inSourcePageIndex,
			mParser->GetPagesCount()-1);
		status = PDFHummus::eFailure;
	}
	return status;
}

EStatusCodeAndObjectIDTypeList PDFDocumentHandler::CreateFormXObjectsFromPDF(IByteReaderWithPosition* inPDFStream,
																			const PDFParsingOptions& inParsingOptions,
																			 const PDFPageRange& inPageRange,
																			 EPDFPageBox inPageBoxToUseAsFormBox,
																			 const double* inTransformationMatrix,
																			 const ObjectIDTypeList& inCopyAdditionalObjects,
																			 const ObjectIDTypeList& inPredefinedFormIDs)
{
	PageEmbedInFormWithPageBox embedCommand(inPageBoxToUseAsFormBox);
	return CreateFormXObjectsFromPDF(inPDFStream,inParsingOptions,inPageRange,&embedCommand,inTransformationMatrix,inCopyAdditionalObjects, inPredefinedFormIDs);

}

EStatusCodeAndObjectIDTypeList PDFDocumentHandler::CreateFormXObjectsFromPDF(IByteReaderWithPosition* inPDFStream,
																			const PDFParsingOptions& inParsingOptions,
																			const PDFPageRange& inPageRange,
																			const PDFRectangle& inCropBox,
																			const double* inTransformationMatrix,
																			const ObjectIDTypeList& inCopyAdditionalObjects,
																			const ObjectIDTypeList& inPredefinedFormIDs)
{
	PageEmbedInFormWithCropBox embedCommand(inCropBox);
	return CreateFormXObjectsFromPDF(inPDFStream,inParsingOptions,inPageRange,&embedCommand,inTransformationMatrix,inCopyAdditionalObjects, inPredefinedFormIDs);
}

EStatusCodeAndObjectIDTypeList PDFDocumentHandler::CreateFormXObjectsFromPDF(	IByteReaderWithPosition* inPDFStream,
																				const PDFParsingOptions& inParsingOptions,
																					const PDFPageRange& inPageRange,
																				IPageEmbedInFormCommand* inPageEmbedCommand,
																				const double* inTransformationMatrix,
																				const ObjectIDTypeList& inCopyAdditionalObjects,
																				const ObjectIDTypeList& inPredefinedFormIDs)
{
	if(StartStreamCopyingContext(inPDFStream, inParsingOptions) != PDFHummus::eSuccess)
		return EStatusCodeAndObjectIDTypeList(PDFHummus::eFailure,ObjectIDTypeList());

	return CreateFormXObjectsFromPDFInContext(inPageRange,inPageEmbedCommand,inTransformationMatrix,inCopyAdditionalObjects, inPredefinedFormIDs);
}

EStatusCodeAndObjectIDTypeList PDFDocumentHandler::AppendPDFPagesFromPDF(IByteReaderWithPosition* inPDFStream,
																		 const PDFParsingOptions& inParsingOptions,
																		 const PDFPageRange& inPageRange,
																		 const ObjectIDTypeList& inCopyAdditionalObjects)
{
	if(StartStreamCopyingContext(inPDFStream, inParsingOptions) != PDFHummus::eSuccess)
		return EStatusCodeAndObjectIDTypeList(PDFHummus::eFailure,ObjectIDTypeList());

	return AppendPDFPagesFromPDFInContext(inPageRange,inCopyAdditionalObjects);
}

EStatusCode PDFDocumentHandler::MergePDFPagesToPage(PDFPage* inPage,
													IByteReaderWithPosition* inPDFStream,
													const PDFParsingOptions& inParsingOptions,
													const PDFPageRange& inPageRange,
													const ObjectIDTypeList& inCopyAdditionalObjects)
{
	if(StartStreamCopyingContext(inPDFStream, inParsingOptions) != PDFHummus::eSuccess)
		return PDFHummus::eFailure;

	return MergePDFPagesToPageInContext(inPage,inPageRange,inCopyAdditionalObjects);
}

EStatusCode PDFDocumentHandler::StartStreamCopyingContext(IByteReaderWithPosition* inPDFStream, const PDFParsingOptions& inOptions)
{
	return StartCopyingContext(inPDFStream,inOptions);
}

PDFHummus::EStatusCode PDFDocumentHandler::StartParserCopyingContext(PDFParser* inPDFParser)
{
    return StartCopyingContext(inPDFParser);
}


EStatusCodeAndObjectIDTypeList PDFDocumentHandler::CopyDirectObjectWithDeepCopy(PDFObject* inObject)
{
	ObjectIDTypeList notCopiedReferencedObjects;
	OutWritingPolicy writingPolicy(this, notCopiedReferencedObjects);

	EStatusCode status = WriteObjectByType(inObject,eTokenSeparatorEndLine, &writingPolicy);

	return EStatusCodeAndObjectIDTypeList(status,notCopiedReferencedObjects);
}

EStatusCode PDFDocumentHandler::CopyNewObjectsForDirectObject(const ObjectIDTypeList& inReferencedObjects)
{
	return WriteNewObjects(inReferencedObjects);
}

void PDFDocumentHandler::SetParserExtender(IPDFParserExtender* inParserExtender)
{
	mParser->SetParserExtender(inParserExtender);
}

void PDFDocumentHandler::ReplaceSourceObjects(const ObjectIDTypeToObjectIDTypeMap& inSourceObjectsToNewTargetObjects)
{
	ObjectIDTypeToObjectIDTypeMap::const_iterator itReplaced = inSourceObjectsToNewTargetObjects.begin();

	for(; itReplaced != inSourceObjectsToNewTargetObjects.end(); ++itReplaced)
	{
		if(mSourceToTarget.find(itReplaced->first) == mSourceToTarget.end())
			mSourceToTarget.insert(ObjectIDTypeToObjectIDTypeMap::value_type(itReplaced->first,itReplaced->second));
	}
}

IByteReaderWithPosition* PDFDocumentHandler::GetSourceDocumentStream()
{
	return mPDFStream;
}

// for modification scenarios, no need for deep copying. the following implement this path
EStatusCode PDFDocumentHandler::CopyDirectObjectAsIs(PDFObject* inObject)
{
	InWritingPolicy writingPolicy(this);
	return WriteObjectByType(inObject,eTokenSeparatorEndLine,&writingPolicy);
}

void InWritingPolicy::WriteReference(PDFIndirectObjectReference* inReference, ETokenSeparator inSeparator) {
	mDocumentHandler->mObjectsContext->WriteIndirectObjectReference(inReference->mObjectID, inReference->mVersion, inSeparator);
}

void OutWritingPolicy::WriteReference(PDFIndirectObjectReference* inReference, ETokenSeparator inSeparator) {
	ObjectIDType sourceObjectID = inReference->mObjectID;
	ObjectIDTypeToObjectIDTypeMap::iterator	itObjects = mDocumentHandler->mSourceToTarget.find(sourceObjectID);
	if (itObjects == mDocumentHandler->mSourceToTarget.end())
	{
		ObjectIDType newObjectID = mDocumentHandler->mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
		itObjects = mDocumentHandler->mSourceToTarget.insert(ObjectIDTypeToObjectIDTypeMap::value_type(sourceObjectID, newObjectID)).first;
		mSourceObjectsToAdd.push_back(sourceObjectID);
	}
	mDocumentHandler->mObjectsContext->WriteNewIndirectObjectReference(itObjects->second, inSeparator);
}


EStatusCode PDFDocumentHandler::WriteObjectByType(PDFObject* inObject,ETokenSeparator inSeparator, IObjectWritePolicy* inWritePolicy)
{
	EStatusCode status = PDFHummus::eSuccess;
    
	switch(inObject->GetType())
	{
		case PDFObject::ePDFObjectBoolean:
		{
			mObjectsContext->WriteBoolean(((PDFBoolean*)inObject)->GetValue(),inSeparator);
			break;
		}
		case PDFObject::ePDFObjectLiteralString:
		{
			mObjectsContext->WriteLiteralString(((PDFLiteralString*)inObject)->GetValue(),inSeparator);
			break;
		}
		case PDFObject::ePDFObjectHexString:
		{
			mObjectsContext->WriteHexString(((PDFHexString*)inObject)->GetValue(),inSeparator);
			break;
		}
		case PDFObject::ePDFObjectNull:
		{
			mObjectsContext->WriteNull(eTokenSeparatorEndLine);
			break;
		}
		case PDFObject::ePDFObjectName:
		{
			mObjectsContext->WriteName(((PDFName*)inObject)->GetValue(),inSeparator);
			break;
		}
		case PDFObject::ePDFObjectInteger:
		{
			mObjectsContext->WriteInteger(((PDFInteger*)inObject)->GetValue(),inSeparator);
			break;
		}
		case PDFObject::ePDFObjectReal:
		{
			mObjectsContext->WriteDouble(((PDFReal*)inObject)->GetValue(),inSeparator);
			break;
		}
		case PDFObject::ePDFObjectSymbol:
		{
			mObjectsContext->WriteKeyword(((PDFSymbol*)inObject)->GetValue());
			break;
		}
		case PDFObject::ePDFObjectIndirectObjectReference:
		{
			inWritePolicy->WriteReference((PDFIndirectObjectReference*)inObject, inSeparator);
			break;
		}
		case PDFObject::ePDFObjectArray:
		{
			status = WriteArrayObject((PDFArray*)inObject,inSeparator, inWritePolicy);
			break;
		}
		case PDFObject::ePDFObjectDictionary:
		{
			status = WriteDictionaryObject((PDFDictionary*)inObject, inWritePolicy);
			break;
		}
		case PDFObject::ePDFObjectStream:
		{
			status = WriteStreamObject((PDFStreamInput*)inObject, inWritePolicy);
			break;
		}
	}
	return status;
}

EStatusCode PDFDocumentHandler::WriteArrayObject(PDFArray* inArray,ETokenSeparator inSeparator, IObjectWritePolicy* inWritePolicy)
{
	SingleValueContainerIterator<PDFObjectVector> it(inArray->GetIterator());
    
	EStatusCode status = PDFHummus::eSuccess;
	
	mObjectsContext->StartArray();
    
	while(it.MoveNext() && PDFHummus::eSuccess == status)
		status = WriteObjectByType(it.GetItem(),eTokenSeparatorSpace, inWritePolicy);
    
	if(PDFHummus::eSuccess == status)
		mObjectsContext->EndArray(inSeparator);
    
	return status;
}

EStatusCode PDFDocumentHandler::WriteDictionaryObject(PDFDictionary* inDictionary, IObjectWritePolicy* inWritePolicy)
{
	MapIterator<PDFNameToPDFObjectMap> it(inDictionary->GetIterator());
	EStatusCode status = PDFHummus::eSuccess;
	DictionaryContext* dictionary = mObjectsContext->StartDictionary();
    
	while(it.MoveNext() && PDFHummus::eSuccess == status)
	{
		status = dictionary->WriteKey(it.GetKey()->GetValue());
		if(PDFHummus::eSuccess == status)
			status = WriteObjectByType(it.GetValue(),eTokenSeparatorEndLine, inWritePolicy);
	}
	
	if(PDFHummus::eSuccess == status)
	{
		return mObjectsContext->EndDictionary(dictionary);
	}
	else
		return PDFHummus::eSuccess;
}

EStatusCode PDFDocumentHandler::WriteStreamObject(PDFStreamInput* inStream, IObjectWritePolicy* inWritePolicy)
{
	/*
	1. Create stream dictionary, copy all elements of input stream but Length (which may be the same...but due to internals may not)
	2. Create PDFStream with this dictionary and use its output stream to write the result
	*/
	RefCountPtr<PDFDictionary> streamDictionary(inStream->QueryStreamDictionary());
	DictionaryContext* newStreamDictionary = mObjectsContext->StartDictionary();

	MapIterator<PDFNameToPDFObjectMap> it(streamDictionary->GetIterator());
	EStatusCode status = PDFHummus::eSuccess;
	bool readingDecrypted = false;
	IByteReader* streamReader = NULL;

	/*
	*	To support unencrypted pdf output, mostly used for debugging, (and maybe i should put a general flag there),
	*	add ability here to copy by rewriting the streams...when possible.
	*/
	if(!mObjectsContext->IsCompressingStreams()) {
		streamReader = mParser->StartReadingFromStream(inStream);
		readingDecrypted = streamReader != NULL;
	}	
	if(!readingDecrypted) {
		streamReader = mParser->StartReadingFromStreamForPlainCopying(inStream);
	}

	if (streamReader == NULL) {
               status = PDFHummus::eFailure;
	}
	
	while (it.MoveNext() && PDFHummus::eSuccess == status)
	{
		if (it.GetKey()->GetValue() != "Length" && (!readingDecrypted || it.GetKey()->GetValue() != "Filter")) {
			status = newStreamDictionary->WriteKey(it.GetKey()->GetValue());
			if (PDFHummus::eSuccess == status)
				status = WriteObjectByType(it.GetValue(), eTokenSeparatorEndLine, inWritePolicy);
		}
	}

	if (status != PDFHummus::eSuccess)
	{
		TRACE_LOG("PDFDocumentHandler::WriteStreamObject, failed to write stream dictionary");
		return PDFHummus::eFailure;
	}

	PDFStream* newStream = readingDecrypted ?
		mObjectsContext->StartPDFStream(newStreamDictionary) :
		mObjectsContext->StartUnfilteredPDFStream(newStreamDictionary);
	OutputStreamTraits outputTraits(newStream->GetWriteStream());

	status = outputTraits.CopyToOutputStream(streamReader);
	if (status != PDFHummus::eSuccess)
	{
		TRACE_LOG("PDFDocumentHandler::WriteStreamObject, failed to copy stream");
		delete newStream;
		delete streamReader;
		return PDFHummus::eFailure;
	}

	mObjectsContext->EndPDFStream(newStream);
	delete newStream;
	delete streamReader;
	return status;
}

EStatusCode PDFDocumentHandler::MergePDFPageToFormXObject(PDFFormXObject* inTargetFormXObject,
                                                          unsigned long inSourcePageIndex)
{
	EStatusCode result;
    
	if(inSourcePageIndex < mParser->GetPagesCount())
	{
		result = MergePDFPageForXObject(inTargetFormXObject,inSourcePageIndex);
		if(result != eSuccess)
			TRACE_LOG1("PDFDocumentHandler::MergePDFPageToFormXObject, failed to merge page %ld",inSourcePageIndex);
	}
	else
	{
		TRACE_LOG2(
                   "PDFDocumentHandler::MergePDFPageToFormXObject, request object index %ld is larger than maximum page for input document = %ld", 
                   inSourcePageIndex,
                   mParser->GetPagesCount()-1);
		result = eFailure;
	}
	return result;
    
}

EStatusCode PDFDocumentHandler::MergePDFPageForXObject(
                                                        PDFFormXObject* inTargetFormXObject,
                                                        unsigned long inSourcePageIndex)
{
	RefCountPtr<PDFDictionary> pageObject = mParser->ParsePage(inSourcePageIndex);
    EStatusCode result = eSuccess;
    
    do 
    {
        if(!pageObject)
        {
            TRACE_LOG1("PDFDocumentHandler::MergePDFPageForXObject, unhexpected exception, page index does not denote a page object. page index = %ld",inSourcePageIndex);
            result = eFailure;
            break;
        }
        
        StringToStringMap pageResourcesNamesMapping;
            
        // register resources for later copying (post form xobject writing finishing)
        result = RegisterResourcesForForm(inTargetFormXObject,pageObject.GetPtr(),pageResourcesNamesMapping);
        if(result != PDFHummus::eSuccess)
            break;
            
        // copy the page content to the target page content
        result = MergePageContentToTargetXObject(inTargetFormXObject,pageObject.GetPtr(),pageResourcesNamesMapping);
            
    } 
    while (false);

	return result;	
}

class ICategoryServicesCommand
{
public:
    
    virtual ~ICategoryServicesCommand(){}
    
    virtual std::string GetResourcesCategoryName() = 0;
    virtual std::string RegisterInDirectResourceInFormResources(ObjectIDType inResourceToRegister) = 0;
};

class ExtGStateCategoryServices : public ICategoryServicesCommand
{
public:
    ExtGStateCategoryServices(ResourcesDictionary& inTargetResourcesDictionary):mTargetRersourcesDictionary(inTargetResourcesDictionary){}
    
    virtual std::string GetResourcesCategoryName(){return "ExtGState";}
    virtual std::string RegisterInDirectResourceInFormResources(ObjectIDType inResourceToRegister){
        return mTargetRersourcesDictionary.AddExtGStateMapping(inResourceToRegister);}

private:
    ResourcesDictionary& mTargetRersourcesDictionary;

};

class ColorSpaceCategoryServices : public ICategoryServicesCommand
{
public:
    ColorSpaceCategoryServices(ResourcesDictionary& inTargetResourcesDictionary):mTargetRersourcesDictionary(inTargetResourcesDictionary){}
    
    virtual std::string GetResourcesCategoryName(){return "ColorSpace";}
    virtual std::string RegisterInDirectResourceInFormResources(ObjectIDType inResourceToRegister){
        return mTargetRersourcesDictionary.AddColorSpaceMapping(inResourceToRegister);}
    
private:
    ResourcesDictionary& mTargetRersourcesDictionary;
    
};

class PatternCategoryServices : public ICategoryServicesCommand
{
public:
    PatternCategoryServices(ResourcesDictionary& inTargetResourcesDictionary):mTargetRersourcesDictionary(inTargetResourcesDictionary){}
    
    virtual std::string GetResourcesCategoryName(){return "Pattern";}
    virtual std::string RegisterInDirectResourceInFormResources(ObjectIDType inResourceToRegister){
        return mTargetRersourcesDictionary.AddPatternMapping(inResourceToRegister);}
    
private:
    ResourcesDictionary& mTargetRersourcesDictionary;
};

class ShadingCategoryServices : public ICategoryServicesCommand
{
public:
    ShadingCategoryServices(ResourcesDictionary& inTargetResourcesDictionary):mTargetRersourcesDictionary(inTargetResourcesDictionary){}
    
    virtual std::string GetResourcesCategoryName(){return "Shading";}
    virtual std::string RegisterInDirectResourceInFormResources(ObjectIDType inResourceToRegister){
        return mTargetRersourcesDictionary.AddShadingMapping(inResourceToRegister);}
    
private:
    ResourcesDictionary& mTargetRersourcesDictionary;
};

class XObjectCategoryServices : public ICategoryServicesCommand
{
public:
    XObjectCategoryServices(ResourcesDictionary& inTargetResourcesDictionary):mTargetRersourcesDictionary(inTargetResourcesDictionary){}
    
    virtual std::string GetResourcesCategoryName(){return "XObject";}
    virtual std::string RegisterInDirectResourceInFormResources(ObjectIDType inResourceToRegister){
        return mTargetRersourcesDictionary.AddXObjectMapping(inResourceToRegister);}
    
private:
    ResourcesDictionary& mTargetRersourcesDictionary;
};

class FontCategoryServices : public ICategoryServicesCommand
{
public:
    FontCategoryServices(ResourcesDictionary& inTargetResourcesDictionary):mTargetRersourcesDictionary(inTargetResourcesDictionary){}
    
    virtual std::string GetResourcesCategoryName(){return "Font";}
    virtual std::string RegisterInDirectResourceInFormResources(ObjectIDType inResourceToRegister){
        return mTargetRersourcesDictionary.AddFontMapping(inResourceToRegister);}
    
private:
    ResourcesDictionary& mTargetRersourcesDictionary;
};

class PropertyCategoryServices : public ICategoryServicesCommand
{
public:
    PropertyCategoryServices(ResourcesDictionary& inTargetResourcesDictionary):mTargetRersourcesDictionary(inTargetResourcesDictionary){}
    
    virtual std::string GetResourcesCategoryName(){return "Properties";}
    virtual std::string RegisterInDirectResourceInFormResources(ObjectIDType inResourceToRegister){
        return mTargetRersourcesDictionary.AddPropertyMapping(inResourceToRegister);}
    
private:
    ResourcesDictionary& mTargetRersourcesDictionary;
};

EStatusCode PDFDocumentHandler::RegisterResourcesForForm(PDFFormXObject* inTargetFormXObject,
                                                         PDFDictionary* inPageObject,
                                                         StringToStringMap& outMappedResourcesNames)
{
    EStatusCode result = PDFHummus::eSuccess;
    ObjectIDTypeList objectsForDelayedWriting;
    
    do 
    {
		PDFObjectCastPtr<PDFDictionary> resources(FindPageResources(mParser, inPageObject));
        
        // k. no resources...as wierd as that might be...or just wrong...i'll let it be
        if(!resources)
            break;
        
        // ProcSet
        PDFObjectCastPtr<PDFArray> procsets(mParser->QueryDictionaryObject(resources.GetPtr(),"ProcSet"));
        if(procsets.GetPtr())
        {
            SingleValueContainerIterator<PDFObjectVector> it(procsets->GetIterator());
            while(it.MoveNext())
                inTargetFormXObject->GetResourcesDictionary().AddProcsetResource(((PDFName*)it.GetItem())->GetValue());
        }
            
        // ExtGState
        ExtGStateCategoryServices extServices(inTargetFormXObject->GetResourcesDictionary());
        RegisterResourcesForResourcesCategory(inTargetFormXObject,&extServices,resources.GetPtr(),objectsForDelayedWriting,outMappedResourcesNames);
        
           
        // ColorSpace
        ColorSpaceCategoryServices colorSpaceServices(inTargetFormXObject->GetResourcesDictionary());
        RegisterResourcesForResourcesCategory(inTargetFormXObject,&colorSpaceServices,resources.GetPtr(),objectsForDelayedWriting,outMappedResourcesNames);
        
        
        // Pattern
        PatternCategoryServices patternServices(inTargetFormXObject->GetResourcesDictionary());
        RegisterResourcesForResourcesCategory(inTargetFormXObject,&patternServices,resources.GetPtr(),objectsForDelayedWriting,outMappedResourcesNames);
        
        
        // Shading
        ShadingCategoryServices shadingServices(inTargetFormXObject->GetResourcesDictionary());
        RegisterResourcesForResourcesCategory(inTargetFormXObject,&shadingServices,resources.GetPtr(),objectsForDelayedWriting,outMappedResourcesNames);
        
         
        // XObject
        XObjectCategoryServices xobjectServices(inTargetFormXObject->GetResourcesDictionary());
        RegisterResourcesForResourcesCategory(inTargetFormXObject,&xobjectServices,resources.GetPtr(),objectsForDelayedWriting,outMappedResourcesNames);
        
        // Font
        FontCategoryServices fontServices(inTargetFormXObject->GetResourcesDictionary());
        RegisterResourcesForResourcesCategory(inTargetFormXObject,&fontServices,resources.GetPtr(),objectsForDelayedWriting,outMappedResourcesNames);
        
        
        // Properties
        PropertyCategoryServices propertyServices(inTargetFormXObject->GetResourcesDictionary());
        RegisterResourcesForResourcesCategory(inTargetFormXObject,&propertyServices,resources.GetPtr(),objectsForDelayedWriting,outMappedResourcesNames);
        
        
        RegisterFormRelatedObjects(inTargetFormXObject,objectsForDelayedWriting);
            
    } 
    while (false);

	return result;    
}

class ResourceCopierTask : public IResourceWritingTask
{
public:
    ResourceCopierTask(PDFFormXObject* inFormXObject,PDFDocumentHandler* inCopier,PDFObject* inObjectToCopy)
    {
        mCopier = inCopier;
        mObjectToCopy = inObjectToCopy;
        mObjectToCopy->AddRef();
        mFormXObject = inFormXObject;
    }
    
    void SetResourceName(const std::string& inResourceName)
    {
        mResourceName = inResourceName;
    }
    
    virtual EStatusCode Write(DictionaryContext* inResoruceCategoryContext,
                              ObjectsContext* inObjectsContext,
                              PDFHummus::DocumentContext* inDocumentContext)
    {
        // write key
        inResoruceCategoryContext->WriteKey(mResourceName);
        
        // write object
        EStatusCodeAndObjectIDTypeList result = mCopier->CopyDirectObjectWithDeepCopy(mObjectToCopy);
        mObjectToCopy->Release();
        
        // register indirect objects for later writing
        
        if(result.first == eSuccess)
            mCopier->RegisterFormRelatedObjects(mFormXObject,result.second);
        
        return result.first;
    }
    
private:
    PDFDocumentHandler* mCopier;
    PDFObject* mObjectToCopy;
    std::string mResourceName;
    PDFFormXObject* mFormXObject;

};

void PDFDocumentHandler::RegisterResourcesForResourcesCategory(PDFFormXObject* inTargetFormXObject,
                                                               ICategoryServicesCommand* inCommand,
                                                               PDFDictionary* inResourcesDictionary,
                                                               ObjectIDTypeList& ioObjectsToLaterCopy,
                                                               StringToStringMap& ioMappedResourcesNames)
{
    PDFObjectCastPtr<PDFDictionary> resourcesCategoryDictionary(mParser->QueryDictionaryObject(
                                                                                    inResourcesDictionary,inCommand->GetResourcesCategoryName()));
    if(resourcesCategoryDictionary.GetPtr())
    {	
        MapIterator<PDFNameToPDFObjectMap> it(resourcesCategoryDictionary->GetIterator());
        while(it.MoveNext())
        {
            if(it.GetValue()->GetType() == PDFObject::ePDFObjectIndirectObjectReference)
            {
                PDFIndirectObjectReference* indirectReference = (PDFIndirectObjectReference*)(it.GetValue());
                ObjectIDTypeToObjectIDTypeMap::iterator	itObjects = mSourceToTarget.find(indirectReference->mObjectID);
                ObjectIDType targetObjectID;
                if(itObjects == mSourceToTarget.end())
                {
                    targetObjectID = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
                    mSourceToTarget.insert(ObjectIDTypeToObjectIDTypeMap::value_type(indirectReference->mObjectID,targetObjectID));
                    ioObjectsToLaterCopy.push_back(indirectReference->mObjectID);
                }
                else
                {
                    targetObjectID = itObjects->second;
                }
                ioMappedResourcesNames.insert(StringToStringMap::value_type(AsEncodedName(it.GetKey()->GetValue()),
                                                                            inCommand->RegisterInDirectResourceInFormResources(targetObjectID)));
            }
            else 
            {
                
                ResourceCopierTask* task = new ResourceCopierTask(inTargetFormXObject,this,it.GetValue());
                StringToStringMap::iterator itInsert = ioMappedResourcesNames.insert(StringToStringMap::value_type(AsEncodedName(it.GetKey()->GetValue()),
                                                                            mDocumentContext->AddExtendedResourceMapping(inTargetFormXObject, inCommand->GetResourcesCategoryName(),
                                                                                task))).first;
                task->SetResourceName(itInsert->second);
            }
        }
    }
}

EStatusCode PDFDocumentHandler::MergePageContentToTargetXObject(PDFFormXObject* inTargetFormXObject,
                                                       PDFDictionary* inSourcePage,
                                                       const StringToStringMap& inMappedResourcesNames)
{
 	EStatusCode status = PDFHummus::eSuccess;
	RefCountPtr<PDFObject> pageContent(mParser->QueryDictionaryObject(inSourcePage,"Contents"));
    
    // for empty page, do nothing
    if(!pageContent)
        return status;

    PrimitiveObjectsWriter primitivesWriter;
    primitivesWriter.SetStreamForWriting(inTargetFormXObject->GetContentStream()->GetWriteStream());
    
	if(pageContent->GetType() == PDFObject::ePDFObjectStream)
	{
		status = WritePDFStreamInputToStream(inTargetFormXObject->GetContentStream()->GetWriteStream(),(PDFStreamInput*)pageContent.GetPtr(),inMappedResourcesNames);
        primitivesWriter.EndLine();
	}
	else if(pageContent->GetType() == PDFObject::ePDFObjectArray)
	{
		SingleValueContainerIterator<PDFObjectVector> it = ((PDFArray*)pageContent.GetPtr())->GetIterator();
		PDFObjectCastPtr<PDFIndirectObjectReference> refItem;
		while(it.MoveNext() && status == PDFHummus::eSuccess)
		{
			refItem = it.GetItem();
			if(!refItem)
			{
				status = PDFHummus::eFailure;
				TRACE_LOG("PDFDocumentHandler::MergePageContentToTargetXObject, content stream array contains non-refs");
				break;
			}
			PDFObjectCastPtr<PDFStreamInput> contentStream(mParser->ParseNewObject(refItem->mObjectID));
			if(!contentStream)
			{
				status = PDFHummus::eFailure;
				TRACE_LOG("PDFDocumentHandler::MergePageContentToTargetXObject, content stream array contains references to non streams");
				break;
			}
			status = WritePDFStreamInputToStream(inTargetFormXObject->GetContentStream()->GetWriteStream(),contentStream.GetPtr(),inMappedResourcesNames);
            primitivesWriter.EndLine();
		}
	}
	else
	{
		TRACE_LOG1("PDFDocumentHandler::MergePageContentToTargetXObject, error copying page content, expected either array or stream, getting %s",PDFObject::scPDFObjectTypeLabel(pageContent->GetType()));
		status = PDFHummus::eFailure;
	}

	return status;	   
}

class ObjectsCopyingTask : public IFormEndWritingTask
{
public:
    ObjectsCopyingTask(PDFDocumentHandler* inCopier,const ObjectIDTypeList& inObjectsToWrite)
    {mCopier = inCopier;mObjectsToWrite = inObjectsToWrite;}
    
    virtual ~ObjectsCopyingTask(){}
    
    virtual EStatusCode Write(PDFFormXObject* inFormXObject,
                              ObjectsContext* inObjectsContext,
                              PDFHummus::DocumentContext* inDocumentContext)
    {
        return mCopier->CopyNewObjectsForDirectObject(mObjectsToWrite);
    }
    
private:
    PDFDocumentHandler* mCopier;
    ObjectIDTypeList mObjectsToWrite;
};



void PDFDocumentHandler::RegisterFormRelatedObjects(PDFFormXObject* inFormXObject,const ObjectIDTypeList& inObjectsToWrite)
{
    mDocumentContext->RegisterFormEndWritingTask(inFormXObject,new ObjectsCopyingTask(this,inObjectsToWrite));
}

PDFObject* PDFDocumentHandler::FindPageResources(PDFParser* inParser, PDFDictionary* inDictionary) {
	if(inDictionary->Exists("Resources")) {
		return inParser->QueryDictionaryObject(inDictionary, "Resources");
	}
	else {
		PDFObjectCastPtr<PDFDictionary> parentDict(
			inDictionary->Exists("Parent") ? 
				inParser->QueryDictionaryObject(inDictionary, "Parent"): 
				NULL);
		if(!parentDict) {
			return NULL;
		}
		else {
			return FindPageResources(inParser,parentDict.GetPtr());
		}
		
	}	
}