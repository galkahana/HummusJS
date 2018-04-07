/*
   Source File : PDFDocumentHandler.h


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

#include "PDFEmbedParameterTypes.h"
#include "PDFParser.h"
#include "PDFRectangle.h"
#include "InputFile.h"
#include "ObjectsBasicTypes.h"
#include "ETokenSeparator.h"
#include "DocumentContextExtenderAdapter.h"
#include "MapIterator.h"
#include "PDFParsingOptions.h"

#include <map>
#include <list>
#include <set>

class ObjectsContext;
class IByteWriter;
class PDFDictionary;
class PDFArray;
class PDFStreamInput;
class DictionaryContext;
class PageContentContext;
class PDFPage;
class IDocumentContextExtender;
class IPageEmbedInFormCommand;
class IPDFParserExtender;
class ICategoryServicesCommand;
class PDFIndirectObjectReference;
class PDFObject;


namespace PDFHummus
{
	class DocumentContext;
}

using namespace PDFHummus;
typedef std::map<ObjectIDType,ObjectIDType> ObjectIDTypeToObjectIDTypeMap;
typedef std::map<std::string,std::string> StringToStringMap;
typedef std::set<ObjectIDType> ObjectIDTypeSet;
typedef std::set<IDocumentContextExtender*> IDocumentContextExtenderSet;

struct ResourceTokenMarker
{
	ResourceTokenMarker(std::string inResourceToken,LongFilePositionType inResourceTokenPosition)
	{
		ResourceToken = inResourceToken;
		ResourceTokenPosition = inResourceTokenPosition;
	}

	std::string ResourceToken;
	LongFilePositionType ResourceTokenPosition;
};

class IObjectWritePolicy
{
public:
	virtual void WriteReference(PDFIndirectObjectReference* inReference, ETokenSeparator inSeparator) = 0;
};

class PDFDocumentHandler;

class InWritingPolicy : public IObjectWritePolicy {
public:
	InWritingPolicy(PDFDocumentHandler* inDocumentHandler) {
		mDocumentHandler = inDocumentHandler;
	}
	virtual void WriteReference(PDFIndirectObjectReference* inReference, ETokenSeparator inSeparator);
private:
	PDFDocumentHandler* mDocumentHandler;
};

class OutWritingPolicy : public IObjectWritePolicy {
public:
	OutWritingPolicy(PDFDocumentHandler* inDocumentHandler, ObjectIDTypeList& ioSourceObjectsToAdd):mSourceObjectsToAdd(ioSourceObjectsToAdd){
		mDocumentHandler = inDocumentHandler;
	}
	virtual void WriteReference(PDFIndirectObjectReference* inReference, ETokenSeparator inSeparator);
private:
	PDFDocumentHandler* mDocumentHandler;
	ObjectIDTypeList& mSourceObjectsToAdd;
};

typedef std::list<ResourceTokenMarker> ResourceTokenMarkerList;

class PDFDocumentHandler : public DocumentContextExtenderAdapter
{
	friend class InWritingPolicy;
	friend class OutWritingPolicy;

public:
	PDFDocumentHandler(void);
	virtual ~PDFDocumentHandler(void);

	void SetOperationsContexts(PDFHummus::DocumentContext* inDocumentContext,
							   ObjectsContext* inObjectsContext);

	// Create a list of XObjects from a PDF file.
	// the list of objects can then be used to place the "pages" in various locations on the written
	// PDF page.
	EStatusCodeAndObjectIDTypeList CreateFormXObjectsFromPDF( const std::string& inPDFFilePath,
																const PDFParsingOptions& inParsingOptions,
																const PDFPageRange& inPageRange,
																EPDFPageBox inPageBoxToUseAsFormBox,
																const double* inTransformationMatrix,
																const ObjectIDTypeList& inCopyAdditionalObjects,
																const ObjectIDTypeList& inPredefinedFormIDs);

	EStatusCodeAndObjectIDTypeList CreateFormXObjectsFromPDF(IByteReaderWithPosition* inPDFStream,
															const PDFParsingOptions& inParsingOptions,
															const PDFPageRange& inPageRange,
															 EPDFPageBox inPageBoxToUseAsFormBox,
															 const double* inTransformationMatrix,
															 const ObjectIDTypeList& inCopyAdditionalObjects,
															 const ObjectIDTypeList& inPredefinedFormIDs);

	EStatusCodeAndObjectIDTypeList CreateFormXObjectsFromPDF( const std::string& inPDFFilePath,
																const PDFParsingOptions& inParsingOptions,
																const PDFPageRange& inPageRange,
																const PDFRectangle& inCropBox,
																const double* inTransformationMatrix,
																const ObjectIDTypeList& inCopyAdditionalObjects,
																const ObjectIDTypeList& inPredefinedFormIDs);

	EStatusCodeAndObjectIDTypeList CreateFormXObjectsFromPDF(IByteReaderWithPosition* inPDFStream,
															const PDFParsingOptions& inParsingOptions,
															const PDFPageRange& inPageRange,
															const PDFRectangle& inCropBox,
															 const double* inTransformationMatrix,
															 const ObjectIDTypeList& inCopyAdditionalObjects,
															 const ObjectIDTypeList& inPredefinedFormIDs);
	
	// appends pages from source PDF to the written PDF. returns object ID for the created pages
	EStatusCodeAndObjectIDTypeList AppendPDFPagesFromPDF(const std::string& inPDFFilePath,
														const PDFParsingOptions& inParsingOptions,
														const PDFPageRange& inPageRange,
														const ObjectIDTypeList& inCopyAdditionalObjects);

	EStatusCodeAndObjectIDTypeList AppendPDFPagesFromPDF(IByteReaderWithPosition* inPDFStream,
														const PDFParsingOptions& inParsingOptions,
														const PDFPageRange& inPageRange,
														const ObjectIDTypeList& inCopyAdditionalObjects);

	// MergePDFPagesToPage, merge PDF pages content to an input page. good for single-placement of a page content, cheaper than creating
	// and XObject and later placing, when the intention is to use this graphic just once.
	PDFHummus::EStatusCode MergePDFPagesToPage(PDFPage* inPage,
									const std::string& inPDFFilePath,
									const PDFParsingOptions& inParsingOptions,
									const PDFPageRange& inPageRange,
									const ObjectIDTypeList& inCopyAdditionalObjects);

	PDFHummus::EStatusCode MergePDFPagesToPage(PDFPage* inPage,
									IByteReaderWithPosition* inPDFStream,
									const PDFParsingOptions& inParsingOptions,
									const PDFPageRange& inPageRange,
									const ObjectIDTypeList& inCopyAdditionalObjects);

	// Event listeners for CreateFormXObjectsFromPDF and AppendPDFPagesFromPDF
	void AddDocumentContextExtender(IDocumentContextExtender* inExtender);
	void RemoveDocumentContextExtender(IDocumentContextExtender* inExtender);	

	// IDocumentContextExtender implementation
	virtual PDFHummus::EStatusCode OnResourcesWrite(
							ResourcesDictionary* inResources,
							DictionaryContext* inPageResourcesDictionaryContext,
							ObjectsContext* inPDFWriterObjectContext,
							PDFHummus::DocumentContext* inPDFWriterDocumentContext);


	// copying context handling
	PDFHummus::EStatusCode StartFileCopyingContext(const std::string& inPDFFilePath,const PDFParsingOptions& inOptions);
	PDFHummus::EStatusCode StartStreamCopyingContext(IByteReaderWithPosition* inPDFStream, const PDFParsingOptions& inOptions);
	PDFHummus::EStatusCode StartParserCopyingContext(PDFParser* inPDFParser);
	EStatusCodeAndObjectIDType CreateFormXObjectFromPDFPage(unsigned long inPageIndex,
														 EPDFPageBox inPageBoxToUseAsFormBox,
														 const double* inTransformationMatrix,
														 ObjectIDType inPredefinedFormId);
	EStatusCodeAndObjectIDType CreateFormXObjectFromPDFPage(unsigned long inPageIndex,
														 const PDFRectangle& inCropBox,
														 const double* inTransformationMatrix,
														ObjectIDType inPredefinedFormId);
	EStatusCodeAndObjectIDType AppendPDFPageFromPDF(unsigned long inPageIndex);
	PDFHummus::EStatusCode MergePDFPageToPage(PDFPage* inTargetPage,unsigned long inSourcePageIndex);
    PDFHummus::EStatusCode MergePDFPageToFormXObject(PDFFormXObject* inTargetFormXObject,unsigned long inSourcePageIndex);
	EStatusCodeAndObjectIDType CopyObject(ObjectIDType inSourceObjectID);
	PDFParser* GetSourceDocumentParser();
	EStatusCodeAndObjectIDType GetCopiedObjectID(ObjectIDType inSourceObjectID);
	MapIterator<ObjectIDTypeToObjectIDTypeMap> GetCopiedObjectsMappingIterator();
	EStatusCodeAndObjectIDTypeList CopyDirectObjectWithDeepCopy(PDFObject* inObject);
    PDFHummus::EStatusCode CopyDirectObjectAsIs(PDFObject* inObject);
	PDFHummus::EStatusCode CopyNewObjectsForDirectObject(const ObjectIDTypeList& inReferencedObjects);
	void StopCopyingContext();
	void ReplaceSourceObjects(const ObjectIDTypeToObjectIDTypeMap& inSourceObjectsToNewTargetObjects);
	IByteReaderWithPosition* GetSourceDocumentStream();

	// Internal implementation. do not use directly
	PDFFormXObject* CreatePDFFormXObjectForPage(unsigned long inPageIndex,
												EPDFPageBox inPageBoxToUseAsFormBox,
												const double* inTransformationMatrix,
												ObjectIDType inPredefinedObjectId);
	PDFFormXObject* CreatePDFFormXObjectForPage(unsigned long inPageIndex,
												const PDFRectangle& inCropBox,
												const double* inTransformationMatrix,
												ObjectIDType inPredefinedObjectId);
    void RegisterFormRelatedObjects(PDFFormXObject* inFormXObject,const ObjectIDTypeList& inObjectsToWrite);

	// Extendibility
	void SetParserExtender(IPDFParserExtender* inParserExtender);
private:

	ObjectsContext* mObjectsContext;
	PDFHummus::DocumentContext* mDocumentContext;
	IDocumentContextExtenderSet mExtenders;


	InputFile mPDFFile;
	IByteReaderWithPosition* mPDFStream;
	PDFParser* mParser;
    bool mParserOwned;
	ObjectIDTypeToObjectIDTypeMap mSourceToTarget;
	PDFDictionary* mWrittenPage;
	

	PDFRectangle DeterminePageBox(PDFDictionary* inDictionary,EPDFPageBox inPageBoxType);
	PDFHummus::EStatusCode WritePageContentToSingleStream(IByteWriter* inTargetStream,PDFDictionary* inPageObject);
	PDFHummus::EStatusCode WritePDFStreamInputToStream(IByteWriter* inTargetStream,PDFStreamInput* inSourceStream);
	PDFHummus::EStatusCode CopyResourcesIndirectObjects(PDFDictionary* inPage);
	void RegisterInDirectObjects(PDFDictionary* inDictionary,ObjectIDTypeList& outNewObjects);
	void RegisterInDirectObjects(PDFArray* inArray,ObjectIDTypeList& outNewObjects);
	PDFHummus::EStatusCode WriteNewObjects(const ObjectIDTypeList& inSourceObjectIDs);
	PDFHummus::EStatusCode WriteNewObjects(const ObjectIDTypeList& inSourceObjectIDs,ObjectIDTypeSet& ioCopiedObjects);
	PDFHummus::EStatusCode CopyInDirectObject(ObjectIDType inSourceObjectID,ObjectIDType inTargetObjectID,ObjectIDTypeSet& ioCopiedObjects);
	EStatusCodeAndObjectIDTypeList CreateFormXObjectsFromPDF(const std::string& inPDFFilePath,
															const PDFParsingOptions& inParsingOptions,
															const PDFPageRange& inPageRange,
															IPageEmbedInFormCommand* inPageEmbedCommand,
															const double* inTransformationMatrix,
															const ObjectIDTypeList& inCopyAdditionalObjects,
															const ObjectIDTypeList& inPredefinedFormIDs);
	PDFFormXObject* CreatePDFFormXObjectForPage(PDFDictionary* inPageObject,
												const PDFRectangle& inCropBox,
												const double* inTransformationMatrix,
												ObjectIDType inPredefinedObjectId);
	PDFHummus::EStatusCode CopyInDirectObject(ObjectIDType inSourceObjectID,ObjectIDType inTargetObjectID);

	PDFHummus::EStatusCode WriteObjectByType(PDFObject* inObject, ETokenSeparator inSeparator,IObjectWritePolicy* inWritePolicy);
	PDFHummus::EStatusCode WriteArrayObject(PDFArray* inArray, ETokenSeparator inSeparator, IObjectWritePolicy* inWritePolicy);
	PDFHummus::EStatusCode WriteDictionaryObject(PDFDictionary* inDictionary, IObjectWritePolicy* inWritePolicy);
	PDFHummus::EStatusCode WriteStreamObject(PDFStreamInput* inStream, IObjectWritePolicy* inWritePolicy);


	EStatusCodeAndObjectIDType CreatePDFPageForPage(unsigned long inPageIndex);

	PDFHummus::EStatusCode CopyPageContentToTargetPagePassthrough(PDFPage* inPage, PDFDictionary* inPageObject);
	PDFHummus::EStatusCode CopyPageContentToTargetPageRecoded(PDFPage* inPage,PDFDictionary* inPageObject);

	PDFHummus::EStatusCode WritePDFStreamInputToContentContext(PageContentContext* inContentContext,PDFStreamInput* inContentSource);
	PDFHummus::EStatusCode MergePDFPageForPage(PDFPage* inTargetPage,unsigned long inSourcePageIndex);
	PDFHummus::EStatusCode MergeResourcesToPage(PDFPage* inTargetPage,PDFDictionary* inPage,StringToStringMap& outMappedResourcesNames);
	EStatusCodeAndObjectIDType CopyObjectToIndirectObject(PDFObject* inObject);
	PDFHummus::EStatusCode CopyDirectObjectToIndirectObject(PDFObject* inObject,ObjectIDType inTargetObjectID);
	PDFHummus::EStatusCode MergePageContentToTargetPage(PDFPage* inTargetPage,PDFDictionary* inSourcePage,const StringToStringMap& inMappedResourcesNames);
	PDFHummus::EStatusCode WritePDFStreamInputToContentContext(PageContentContext* inContentContext,PDFStreamInput* inContentSource,const StringToStringMap& inMappedResourcesNames);
	PDFHummus::EStatusCode WritePDFStreamInputToStream(IByteWriter* inTargetStream,PDFStreamInput* inSourceStream,const StringToStringMap& inMappedResourcesNames);
	PDFHummus::EStatusCode ScanStreamForResourcesTokens(PDFStreamInput* inSourceStream,const StringToStringMap& inMappedResourcesNames,ResourceTokenMarkerList& outResourceMarkers);
	PDFHummus::EStatusCode MergeAndReplaceResourcesTokens(	IByteWriter* inTargetStream,
												PDFStreamInput* inSourceStream,
												const StringToStringMap& inMappedResourcesNames,
												const ResourceTokenMarkerList& inResourceMarkers);

	EStatusCodeAndObjectIDTypeList CreateFormXObjectsFromPDFInContext(
																		const PDFPageRange& inPageRange,
																		IPageEmbedInFormCommand* inPageEmbedCommand,
																		const double* inTransformationMatrix,
																		const ObjectIDTypeList& inCopyAdditionalObjects,
																		const ObjectIDTypeList& inPredefinedFormIDs);
	EStatusCodeAndObjectIDTypeList CreateFormXObjectsFromPDF(IByteReaderWithPosition* inPDFStream,
															const PDFParsingOptions& inParsingOptions,
															const PDFPageRange& inPageRange,
															IPageEmbedInFormCommand* inPageEmbedCommand,
															const double* inTransformationMatrix,
															const ObjectIDTypeList& inCopyAdditionalObjects,
															const ObjectIDTypeList& inPredefinedFormIDs);
	EStatusCodeAndObjectIDTypeList AppendPDFPagesFromPDFInContext(const PDFPageRange& inPageRange,
																  const ObjectIDTypeList& inCopyAdditionalObjects);
	PDFHummus::EStatusCode MergePDFPagesToPageInContext(PDFPage* inPage,
											const PDFPageRange& inPageRange,
											const ObjectIDTypeList& inCopyAdditionalObjects);
	PDFHummus::EStatusCode StartCopyingContext(IByteReaderWithPosition* inPDFStream,const PDFParsingOptions& inOptions);
	PDFHummus::EStatusCode StartCopyingContext(PDFParser* inPDFParser);
    EStatusCode MergePDFPageForXObject(PDFFormXObject* inTargetFormXObject,unsigned long inSourcePageIndex);
    EStatusCode RegisterResourcesForForm(PDFFormXObject* inTargetFormXObject,
                                         PDFDictionary* inPageObject,
                                         StringToStringMap& inMappedResourcesNames);
    
	std::string AsEncodedName(const std::string& inName);
    void RegisterResourcesForResourcesCategory(PDFFormXObject* inTargetFormXObject,
                                               ICategoryServicesCommand* inCommand,
                                               PDFDictionary* inResourcesDictionary,
                                               ObjectIDTypeList& ioObjectsToLaterCopy,
                                               StringToStringMap& ioMappedResourcesNames);
    PDFHummus::EStatusCode MergePageContentToTargetXObject(PDFFormXObject* inTargetFormXObject,
                                                           PDFDictionary* inSourcePage,
                                                           const StringToStringMap& inMappedResourcesNames);
	PDFObject* FindPageResources(PDFParser* inParser, PDFDictionary* inDictionary);


};
