/*
   Source File : DocumentContext.h


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
#include "EPDFVersion.h"
#include "IOBasicTypes.h"
#include "TrailerInformation.h"
#include "CatalogInformation.h"
#include "JPEGImageHandler.h"
#include "TIFFImageHandler.h"
#include "TiffUsageParameters.h"
#include "UsedFontsRepository.h"
#include "PDFEmbedParameterTypes.h"
#include "PDFDocumentHandler.h"
#include "ObjectsBasicTypes.h"

#include <string>
#include <set>
#include <utility>
#include <list>
#include <map>


using namespace IOBasicTypes;

typedef std::pair<ObjectIDType,bool> ObjectIDTypeAndBool;

class DictionaryContext;
class ObjectsContext;
class PDFPage;
class PageTree;
class OutputFile;
class IDocumentContextExtender;
class PageContentContext;
class ResourcesDictionary;
class PDFFormXObject;
class PDFTiledPattern;
class PDFRectangle;
class PDFImageXObject;
class PDFUsedFont;
class PageContentContext;
class PDFParser;
class PDFDictionary;
class IResourceWritingTask;
class IFormEndWritingTask;
class PDFDocumentCopyingContext;
class IPageEndWritingTask;
class ITiledPatternEndWritingTask;

typedef std::set<IDocumentContextExtender*> IDocumentContextExtenderSet;
typedef std::pair<PDFHummus::EStatusCode,ObjectIDType> EStatusCodeAndObjectIDType;
typedef std::list<ObjectIDType> ObjectIDTypeList;
typedef std::set<ObjectIDType> ObjectIDTypeSet;
typedef std::map<ObjectIDType,std::string> ObjectIDTypeToStringMap;
typedef std::set<PDFDocumentCopyingContext*> PDFDocumentCopyingContextSet;
typedef std::pair<ResourcesDictionary*,std::string> ResourcesDictionaryAndString;
typedef std::list<IResourceWritingTask*> IResourceWritingTaskList;
typedef std::map<ResourcesDictionaryAndString,IResourceWritingTaskList> ResourcesDictionaryAndStringToIResourceWritingTaskListMap;
typedef std::list<IFormEndWritingTask*> IFormEndWritingTaskList;
typedef std::map<PDFFormXObject*,IFormEndWritingTaskList> PDFFormXObjectToIFormEndWritingTaskListMap;
typedef std::list<IPageEndWritingTask*> IPageEndWritingTaskList;
typedef std::map<PDFPage*,IPageEndWritingTaskList> PDFPageToIPageEndWritingTaskListMap;
typedef std::list<ITiledPatternEndWritingTask*> ITiledPatternEndWritingTaskList;
typedef std::map<PDFTiledPattern*, ITiledPatternEndWritingTaskList> PDFTiledPatternToITiledPatternEndWritingTaskListMap;
typedef std::pair<std::string,unsigned long> StringAndULongPair;

namespace PDFHummus
{
	// image type enumeration, for images supported by hummus
	enum EHummusImageType
	{
		eUndefined,
		ePDF,
		eJPG,
		eTIFF
	};

	struct HummusImageInformation
	{
		HummusImageInformation(){writtenObjectID = 0;imageType=eUndefined;imageWidth=-1;imageHeight=-1;}
    
		ObjectIDType writtenObjectID;
		EHummusImageType imageType;
		double imageWidth;
		double imageHeight;
	};


	typedef std::map<StringAndULongPair,HummusImageInformation> StringAndULongPairToHummusImageInformationMap;


	class DocumentContext
	{
	public:
		DocumentContext();
		~DocumentContext();

		void SetObjectsContext(ObjectsContext* inObjectsContext);
		void SetOutputFileInformation(OutputFile* inOutputFile);
		PDFHummus::EStatusCode	WriteHeader(EPDFVersion inPDFVersion);
		PDFHummus::EStatusCode	FinalizeNewPDF(bool inEmbedFonts);
        PDFHummus::EStatusCode	FinalizeModifiedPDF(PDFParser* inModifiedFileParser,EPDFVersion inModifiedPDFVersion,bool inEmbedFonts);

		TrailerInformation& GetTrailerInformation();
		CatalogInformation& GetCatalogInformation();

		// Page and Page Content Writing

		// Use StartPageContentContext for creating a page content for this page.
		// using the context you can now start creating content for it.
		// if this page already has a content context, it will be returned
		PageContentContext* StartPageContentContext(PDFPage* inPage);

		// Use PausePageContentContext in order to flush a current content write, in order
		// to write some other object. A common use case is when wanting to use an image in a page.
		// first the content before the image is written, then the content is paused, a new object that represents the image
		// is written, and then the content continues showing the image with a "do" operator. This is also the cause for creating multiple content
		// streams for a page (and what will happen in this implementation as well).
		PDFHummus::EStatusCode PausePageContentContext(PageContentContext* inPageContext);

		// Finalize and release the page context. the current content stream is flushed to the PDF stream
		PDFHummus::EStatusCode EndPageContentContext(PageContentContext* inPageContext);

		// Determine whether this page already has a content context
		bool HasContentContext(PDFPage* inPage);
		
		EStatusCodeAndObjectIDType WritePage(PDFPage* inPage);
		EStatusCodeAndObjectIDType WritePageAndRelease(PDFPage* inPage);

		// Use this to add annotation references to a page. the references will be written on the next page write (see WritePage and WritePageAndRelease)
		void RegisterAnnotationReferenceForNextPageWrite(ObjectIDType inAnnotationReference);

		// Form XObject creation and finalization
		PDFFormXObject* StartFormXObject(const PDFRectangle& inBoundingBox,const double* inMatrix = NULL);
		PDFFormXObject* StartFormXObject(const PDFRectangle& inBoundingBox,ObjectIDType inFormXObjectID,const double* inMatrix = NULL);
		PDFHummus::EStatusCode EndFormXObject(PDFFormXObject* inFormXObject);
		PDFHummus::EStatusCode EndFormXObjectAndRelease(PDFFormXObject* inFormXObject);

		// no release version of ending a form XObject. owner should delete it (regular delete...nothin special)
		PDFHummus::EStatusCode EndFormXObjectNoRelease(PDFFormXObject* inFormXObject);

		// Tiled Pattern  creation and finalization
		PDFTiledPattern* StartTiledPattern(
											int inPaintType,
											int inTilingType,
											const PDFRectangle& inBoundingBox,
											double inXStep,
											double inYStep,
											const double* inMatrix = NULL);
		PDFTiledPattern* StartTiledPattern(int inPaintType,
											int inTilingType,
											const PDFRectangle& inBoundingBox,
											double inXStep,
											double inYStep,
											ObjectIDType inObjectID,
											const double* inMatrix = NULL);
		PDFHummus::EStatusCode EndTiledPattern(PDFTiledPattern* inTiledPattern);
		PDFHummus::EStatusCode EndTiledPatternAndRelease(PDFTiledPattern* inTiledPattern);


		// Image XObject creating. 
		// note that as oppose to other methods, create the image xobject also writes it, so there's no "WriteXXXXAndRelease" for image.
		// So...release the object yourself [just delete it]

		// JPEG - two variants
		
		// will return image xobject sized at 1X1
		PDFImageXObject* CreateImageXObjectFromJPGFile(const std::string& inJPGFilePath);
		PDFImageXObject* CreateImageXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream);
		PDFImageXObject* CreateImageXObjectFromJPGFile(const std::string& inJPGFilePath,ObjectIDType inImageXObjectID);
		PDFImageXObject* CreateImageXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream,ObjectIDType inImageXObjectID);

		// will return form XObject, which will include the xobject at it's size
		PDFFormXObject* CreateFormXObjectFromJPGFile(const std::string& inJPGFilePath);
		PDFFormXObject* CreateFormXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream);
		PDFFormXObject* CreateFormXObjectFromJPGFile(const std::string& inJPGFilePath,ObjectIDType inFormXObjectID);
		PDFFormXObject* CreateFormXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream,ObjectIDType inFormXObjectID);

		// TIFF
#ifndef PDFHUMMUS_NO_TIFF
		PDFFormXObject* CreateFormXObjectFromTIFFFile(	const std::string& inTIFFFilePath,
														const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters);
		PDFFormXObject* CreateFormXObjectFromTIFFStream(IByteReaderWithPosition* inTIFFStream,
														const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters);
		PDFFormXObject* CreateFormXObjectFromTIFFFile(	const std::string& inTIFFFilePath,
														ObjectIDType inFormXObjectID,
														const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters);
		PDFFormXObject* CreateFormXObjectFromTIFFStream(	IByteReaderWithPosition* inTIFFStream,
														ObjectIDType inFormXObjectID,
														const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters);
#endif
		// PDF
		// CreateFormXObjectsFromPDF is for using input PDF pages as objects in one page or more. you can used the returned IDs to place the 
		// created form xobjects
		EStatusCodeAndObjectIDTypeList CreateFormXObjectsFromPDF(const std::string& inPDFFilePath,
																 const PDFPageRange& inPageRange,
																 EPDFPageBox inPageBoxToUseAsFormBox,
																 const double* inTransformationMatrix = NULL,
																 const ObjectIDTypeList& inCopyAdditionalObjects = ObjectIDTypeList());

		EStatusCodeAndObjectIDTypeList CreateFormXObjectsFromPDF(IByteReaderWithPosition* inPDFStream,
																 const PDFPageRange& inPageRange,
																 EPDFPageBox inPageBoxToUseAsFormBox,
																 const double* inTransformationMatrix = NULL,
																 const ObjectIDTypeList& inCopyAdditionalObjects = ObjectIDTypeList());
		
		// CreateFormXObjectsFromPDF is an override to allow you to determine a custom crop for the page embed
		EStatusCodeAndObjectIDTypeList CreateFormXObjectsFromPDF(const std::string& inPDFFilePath,
																 const PDFPageRange& inPageRange,
																 const PDFRectangle& inCropBox,
																 const double* inTransformationMatrix = NULL,
																 const ObjectIDTypeList& inCopyAdditionalObjects = ObjectIDTypeList());

		EStatusCodeAndObjectIDTypeList CreateFormXObjectsFromPDF(IByteReaderWithPosition* inPDFStream,
																 const PDFPageRange& inPageRange,
																 const PDFRectangle& inCropBox,
																 const double* inTransformationMatrix = NULL,
																 const ObjectIDTypeList& inCopyAdditionalObjects = ObjectIDTypeList());

		// AppendPDFPagesFromPDF is for simple appending of the input PDF pages
		EStatusCodeAndObjectIDTypeList AppendPDFPagesFromPDF(const std::string& inPDFFilePath,
															const PDFPageRange& inPageRange,
															const ObjectIDTypeList& inCopyAdditionalObjects = ObjectIDTypeList());
		
		EStatusCodeAndObjectIDTypeList AppendPDFPagesFromPDF(IByteReaderWithPosition* inPDFStream,
															const PDFPageRange& inPageRange,
															const ObjectIDTypeList& inCopyAdditionalObjects = ObjectIDTypeList());

		// MergePDFPagesToPage, merge PDF pages content to an input page. good for single-placement of a page content, cheaper than creating
		// and XObject and later placing, when the intention is to use this graphic just once.
		PDFHummus::EStatusCode MergePDFPagesToPage(PDFPage* inPage,
										const std::string& inPDFFilePath,
										const PDFPageRange& inPageRange,
										const ObjectIDTypeList& inCopyAdditionalObjects = ObjectIDTypeList());

		PDFHummus::EStatusCode MergePDFPagesToPage(PDFPage* inPage,
										IByteReaderWithPosition* inPDFStream,
										const PDFPageRange& inPageRange,
										const ObjectIDTypeList& inCopyAdditionalObjects = ObjectIDTypeList());

		PDFDocumentCopyingContext* CreatePDFCopyingContext(const std::string& inPDFFilePath);
		PDFDocumentCopyingContext* CreatePDFCopyingContext(IByteReaderWithPosition* inPDFStream);
        PDFDocumentCopyingContext* CreatePDFCopyingContext(PDFParser* inPDFParser);

		// some public image info services, for users of hummus
		DoubleAndDoublePair GetImageDimensions(const std::string& inImageFile,unsigned long inImageIndex = 0);
		EHummusImageType GetImageType(const std::string& inImageFile,unsigned long inImageIndex);


		// Font [Text] (font index is for multi-font files. for single file fonts, pass 0)
		PDFUsedFont* GetFontForFile(const std::string& inFontFilePath,long inFontIndex);
		// second overload is for type 1, when an additional metrics file is available
		PDFUsedFont* GetFontForFile(const std::string& inFontFilePath,const std::string& inAdditionalMeticsFilePath,long inFontIndex);

		// URL should be encoded to be a valid URL, ain't gonna be checking that!
		PDFHummus::EStatusCode AttachURLLinktoCurrentPage(const std::string& inURL,const PDFRectangle& inLinkClickArea);

		// Extensibility
		void AddDocumentContextExtender(IDocumentContextExtender* inExtender);
		void RemoveDocumentContextExtender(IDocumentContextExtender* inExtender);
		void SetParserExtender(IPDFParserExtender* inParserExtender);
        
        // Extensibility option. a method of adding direct objects to a resource dictionary of a form or page.
        // resource writing tasks are one time objects (deleted when done, owned by documentcontext) that
        // are called when a certain resoruce category for a certain form is written. each task
        // is supposed to write one resource. it can write a direct object.
        // note that the task is supposed to both write the name and the content of the object.
        // returns the name of the newley added resource
        std::string AddExtendedResourceMapping(PDFFormXObject* inFormXObject,
                                          const std::string& inResourceCategoryName,
                                          IResourceWritingTask* inWritingTask);
        std::string AddExtendedResourceMapping(PDFPage* inPage,
                                          const std::string& inResourceCategoryName,
                                          IResourceWritingTask* inWritingTask);
		std::string AddExtendedResourceMapping(PDFTiledPattern* inTiledPattern,
										const std::string& inResourceCategoryName,
										IResourceWritingTask* inWritingTask);
		std::string AddExtendedResourceMapping(ResourcesDictionary* inResourceDictionary,
                                          const std::string& inResourceCategoryName,
                                          IResourceWritingTask* inWritingTask);
        
        // Extensibility option. option of writing a single time task for when a particular form ends
        void RegisterFormEndWritingTask(PDFFormXObject* inFormXObject,IFormEndWritingTask* inWritingTask);
        // Extensibility option. option of writing a single time task for when a particular page ends
        void RegisterPageEndWritingTask(PDFPage* inPageObject,IPageEndWritingTask* inWritingTask);
		// Extensibility option. option of writing a single time task for when a particular pattern ends
		void RegisterTiledPatternEndWritingTask(PDFTiledPattern* inTiledPatternObject, ITiledPatternEndWritingTask* inWritingTask);


		PDFHummus::EStatusCode WriteState(ObjectsContext* inStateWriter,ObjectIDType inObjectID);
		PDFHummus::EStatusCode ReadState(PDFParser* inStateReader,ObjectIDType inObjectID);

		void Cleanup();
        
        // modification scenario
        PDFHummus::EStatusCode SetupModifiedFile(PDFParser* inModifiedFileParser);
		
		// internal methods for copying context listeners handling
		void RegisterCopyingContext(PDFDocumentCopyingContext* inCopyingContext);
		void UnRegisterCopyingContext(PDFDocumentCopyingContext* inCopyingContext);

		// internal methods for easy image writing
		EStatusCode WriteFormForImage(const std::string& inImagePath,unsigned long inImageIndex,ObjectIDType inObjectID);
		ObjectIDTypeAndBool RegisterImageForDrawing(const std::string& inImageFile,unsigned long inImageIndex);

		// JPG images handler for retrieving JPG images information
		JPEGImageHandler& GetJPEGImageHandler();
		// tiff image handler accessor
#ifndef PDFHUMMUS_NO_TIFF
        TIFFImageHandler&  GetTIFFImageHandler();
#endif

	private:
		ObjectsContext* mObjectsContext;
		TrailerInformation mTrailerInformation;
		CatalogInformation mCatalogInformation;
		std::string mOutputFilePath;
		IDocumentContextExtenderSet mExtenders;
		JPEGImageHandler mJPEGImageHandler;
#ifndef PDFHUMMUS_NO_TIFF
		TIFFImageHandler mTIFFImageHandler;
#endif
		PDFDocumentHandler mPDFDocumentHandler;
		UsedFontsRepository mUsedFontsRepository;
		ObjectIDTypeSet mAnnotations;
		IPDFParserExtender* mParserExtender;
		PDFDocumentCopyingContextSet mCopyingContexts;
        bool mModifiedDocumentIDExists;
        std::string mModifiedDocumentID;
		ObjectIDType mCurrentPageTreeIDInState;
        ResourcesDictionaryAndStringToIResourceWritingTaskListMap mResourcesTasks;
        PDFFormXObjectToIFormEndWritingTaskListMap mFormEndTasks;
        PDFPageToIPageEndWritingTaskListMap mPageEndTasks;
		PDFTiledPatternToITiledPatternEndWritingTaskListMap mTiledPatternEndTasks;
	    StringAndULongPairToHummusImageInformationMap mImagesInformation;
		
		void WriteHeaderComment(EPDFVersion inPDFVersion);
		void Write4BinaryBytes();
		PDFHummus::EStatusCode WriteCatalogObjectOfNewPDF();
        PDFHummus::EStatusCode WriteCatalogObject(const ObjectReference& inPageTreeRootObjectReference);
		PDFHummus::EStatusCode WriteTrailerDictionary();
        PDFHummus::EStatusCode WriteTrailerDictionaryValues(DictionaryContext* inDictionaryContext);
        void WriteReferenceState(ObjectsContext* inStateWriter,
                                 DictionaryContext* inDictionaryContext, 
                                 const ObjectReference& inReference);

		void WriteXrefReference(LongFilePositionType inXrefTablePosition);
		void WriteFinalEOF();
		void WriteInfoDictionary();
		void WritePagesTree();
		int WritePageTree(PageTree* inPageTreeToWrite);
		std::string GenerateMD5IDForFile();
		PDFHummus::EStatusCode WriteResourcesDictionary(ResourcesDictionary& inResourcesDictionary);
        PDFHummus::EStatusCode WriteResourceDictionary(ResourcesDictionary* inResourcesDictionary,
                                                       DictionaryContext* inResourcesCategoryDictionary,
                                                       const std::string& inResourceDictionaryLabel,
                                                       MapIterator<ObjectIDTypeToStringMap> inMapping);
		bool IsIdentityMatrix(const double* inMatrix);
		PDFHummus::EStatusCode WriteUsedFontsDefinitions(bool inEmbedFonts);
		EStatusCodeAndObjectIDType WriteAnnotationAndLinkForURL(const std::string& inURL,const PDFRectangle& inLinkClickArea);

		void WriteTrailerState(ObjectsContext* inStateWriter,ObjectIDType inObjectID);
        void WriteReferenceState(ObjectsContext* inStateWriter,
                                 const ObjectReference& inReference);
		void WriteTrailerInfoState(ObjectsContext* inStateWriter,ObjectIDType inObjectID);
		void WriteDateState(ObjectsContext* inStateWriter,const PDFDate& inDate);
		void WriteCatalogInformationState(ObjectsContext* inStateWriter,ObjectIDType inObjectID);
		void ReadTrailerState(PDFParser* inStateReader,PDFDictionary* inTrailerState);
        ObjectReference GetReferenceFromState(PDFDictionary* inDictionary);
		void ReadTrailerInfoState(PDFParser* inStateReader,PDFDictionary* inTrailerInfoState);
		void ReadDateState(PDFDictionary* inDateState,PDFDate& inDate);
		void ReadCatalogInformationState(PDFParser* inStateReader,PDFDictionary* inCatalogInformationState);


		void WritePageTreeState(ObjectsContext* inStateWriter,ObjectIDType inObjectID,PageTree* inPageTree);
		void ReadPageTreeState(PDFParser* inStateReader,PDFDictionary* inPageTreeState,PageTree* inPageTree);
        
        PDFHummus::EStatusCode SetupTrailerFromModifiedFile(PDFParser* inModifiedFileParser);
        ObjectReference GetOriginalDocumentPageTreeRoot(PDFParser* inModifiedFileParser);
        bool DocumentHasNewPages();
        ObjectIDType WriteCombinedPageTree(PDFParser* inModifiedFileParser);
        bool IsRequiredVersionHigherThanPDFVersion(PDFParser* inModifiedFileParser,EPDFVersion inModifiedPDFVersion);
        bool DoExtendersRequireCatalogUpdate(PDFParser* inModifiedFileParser);
        bool RequiresXrefStream(PDFParser* inModifiedFileParser);
        PDFHummus::EStatusCode WriteXrefStream(LongFilePositionType& outXrefPosition);
		HummusImageInformation& GetImageInformationStructFor(const std::string& inImageFile,unsigned long inImageIndex);
	};
}