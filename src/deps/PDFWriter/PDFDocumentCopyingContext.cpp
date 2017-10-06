/*
   Source File : PDFDocumentCopyingContext.cpp


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
#include "PDFDocumentCopyingContext.h"
#include "DocumentContext.h"

using namespace PDFHummus;

PDFDocumentCopyingContext::PDFDocumentCopyingContext(void)
{
    mDocumentContext = NULL;
}

PDFDocumentCopyingContext::~PDFDocumentCopyingContext(void)
{
	End();
}

EStatusCode PDFDocumentCopyingContext::Start(const std::string& inPDFFilePath,
											  DocumentContext* inDocumentContext,
											  ObjectsContext* inObjectsContext,
										    	const PDFParsingOptions& inOptions,
											  IPDFParserExtender* inParserExtender)
{
	mDocumentContext = inDocumentContext;
	inDocumentContext->RegisterCopyingContext(this);
	mDocumentHandler.SetOperationsContexts(inDocumentContext,inObjectsContext);
	EStatusCode status = mDocumentHandler.StartFileCopyingContext(inPDFFilePath, inOptions);
    if(eSuccess == status)
        mDocumentHandler.SetParserExtender(inParserExtender);
    return status;
}

EStatusCode PDFDocumentCopyingContext::Start(IByteReaderWithPosition* inPDFStream,
											 DocumentContext* inDocumentContext,
											 ObjectsContext* inObjectsContext,
											const PDFParsingOptions& inOptions,
											 IPDFParserExtender* inParserExtender)
{
	mDocumentContext = inDocumentContext;
	inDocumentContext->RegisterCopyingContext(this);
	mDocumentHandler.SetOperationsContexts(inDocumentContext,inObjectsContext);
	EStatusCode status = mDocumentHandler.StartStreamCopyingContext(inPDFStream, inOptions);
    if(eSuccess == status)
        mDocumentHandler.SetParserExtender(inParserExtender);
    return status;
}

PDFHummus::EStatusCode PDFDocumentCopyingContext::Start(PDFParser* inPDFParser,
                             DocumentContext* inDocumentContext,
                             ObjectsContext* inObjectsContext)
{
	mDocumentContext = inDocumentContext;
	inDocumentContext->RegisterCopyingContext(this);
	mDocumentHandler.SetOperationsContexts(inDocumentContext,inObjectsContext);
	return mDocumentHandler.StartParserCopyingContext(inPDFParser); 
}

EStatusCodeAndObjectIDType PDFDocumentCopyingContext::CreateFormXObjectFromPDFPage(unsigned long inPageIndex,
																					EPDFPageBox inPageBoxToUseAsFormBox,
																					const double* inTransformationMatrix,
																					ObjectIDType inPredefinedFormId)
{
	return mDocumentHandler.CreateFormXObjectFromPDFPage(inPageIndex,inPageBoxToUseAsFormBox,inTransformationMatrix, inPredefinedFormId);
}

EStatusCodeAndObjectIDType PDFDocumentCopyingContext::CreateFormXObjectFromPDFPage(unsigned long inPageIndex,
																					 const PDFRectangle& inCropBox,
																					 const double* inTransformationMatrix,
																					ObjectIDType inPredefinedFormId)
{
	return mDocumentHandler.CreateFormXObjectFromPDFPage(inPageIndex,inCropBox,inTransformationMatrix, inPredefinedFormId);
}


EStatusCodeAndObjectIDType PDFDocumentCopyingContext::AppendPDFPageFromPDF(unsigned long inPageIndex)
{
	return mDocumentHandler.AppendPDFPageFromPDF(inPageIndex);
}

EStatusCodeAndObjectIDType PDFDocumentCopyingContext::CopyObject(ObjectIDType inSourceObjectID)
{
	return mDocumentHandler.CopyObject(inSourceObjectID);
}

EStatusCodeAndObjectIDType PDFDocumentCopyingContext::GetCopiedObjectID(ObjectIDType inSourceObjectID)
{
	return mDocumentHandler.GetCopiedObjectID(inSourceObjectID);
}

PDFParser* PDFDocumentCopyingContext::GetSourceDocumentParser()
{
	return mDocumentHandler.GetSourceDocumentParser();
}

MapIterator<ObjectIDTypeToObjectIDTypeMap> PDFDocumentCopyingContext::GetCopiedObjectsMappingIterator()
{
	return mDocumentHandler.GetCopiedObjectsMappingIterator();
}

EStatusCode PDFDocumentCopyingContext::MergePDFPageToPage(PDFPage* inTargetPage,unsigned long inSourcePageIndex)
{
	return mDocumentHandler.MergePDFPageToPage(inTargetPage,inSourcePageIndex);
}

EStatusCode PDFDocumentCopyingContext::MergePDFPageToFormXObject(
                                            PDFFormXObject* inTargetFormXObject,
                                            unsigned long inSourcePageIndex)
{
    return mDocumentHandler.MergePDFPageToFormXObject(inTargetFormXObject,inSourcePageIndex);
}

void PDFDocumentCopyingContext::End()
{
	mDocumentHandler.StopCopyingContext();
    if(mDocumentContext)
    {
        mDocumentContext->UnRegisterCopyingContext(this);
        mDocumentContext = NULL;
    }
}

void PDFDocumentCopyingContext::ReleaseDocumentContextReference()
{
    mDocumentContext = NULL;
}

EStatusCodeAndObjectIDTypeList PDFDocumentCopyingContext::CopyDirectObjectWithDeepCopy(PDFObject* inObject)
{
	return mDocumentHandler.CopyDirectObjectWithDeepCopy(inObject);
}

EStatusCode PDFDocumentCopyingContext::CopyDirectObjectAsIs(PDFObject* inObject)
{
	return mDocumentHandler.CopyDirectObjectAsIs(inObject);    
}

EStatusCode PDFDocumentCopyingContext::CopyNewObjectsForDirectObject(const ObjectIDTypeList& inReferencedObjects)
{
	return mDocumentHandler.CopyNewObjectsForDirectObject(inReferencedObjects);
}

void PDFDocumentCopyingContext::ReplaceSourceObjects(const ObjectIDTypeToObjectIDTypeMap& inSourceObjectsToNewTargetObjects)
{
	return mDocumentHandler.ReplaceSourceObjects(inSourceObjectsToNewTargetObjects);
}

void PDFDocumentCopyingContext::AddDocumentContextExtender(IDocumentContextExtender* inExtender)
{
	mDocumentHandler.AddDocumentContextExtender(inExtender);
}

void PDFDocumentCopyingContext::RemoveDocumentContextExtender(IDocumentContextExtender* inExtender)
{
	mDocumentHandler.RemoveDocumentContextExtender(inExtender);
}

IByteReaderWithPosition* PDFDocumentCopyingContext::GetSourceDocumentStream()
{
	return mDocumentHandler.GetSourceDocumentStream();
}
