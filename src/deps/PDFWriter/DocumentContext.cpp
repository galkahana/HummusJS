/*
   Source File : DocumentContext.cpp


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
#include "DocumentContext.h"
#include "ObjectsContext.h"
#include "IByteWriterWithPosition.h"
#include "DictionaryContext.h"
#include "PDFPage.h"
#include "PageTree.h"
#include "BoxingBase.h"
#include "InfoDictionary.h"
#include "MD5Generator.h"
#include "OutputFile.h"
#include "Trace.h"
#include "IDocumentContextExtender.h"
#include "PageContentContext.h"
#include "PDFFormXObject.h"
#include "PDFTiledPattern.h"
#include "PDFParser.h"
#include "PDFObjectCast.h"
#include "PDFDictionary.h"
#include "PDFIndirectObjectReference.h"
#include "PDFInteger.h"
#include "PDFLiteralString.h"
#include "PDFBoolean.h"
#include "PDFArray.h"
#include "PDFDocumentCopyingContext.h"
#include "Ascii7Encoding.h"
#include "PDFHexString.h"
#include "PDFName.h"
#include "IResourceWritingTask.h"
#include "IFormEndWritingTask.h"
#include "IPageEndWritingTask.h"
#include "ITiledPatternEndWritingTask.h"
#include "PDFPageInput.h"
#include "IObjectEndWritingTask.h"


using namespace PDFHummus;

DocumentContext::DocumentContext()
{
	mObjectsContext = NULL;
	mParserExtender = NULL;
    mModifiedDocumentIDExists = false;
}

DocumentContext::~DocumentContext(void)
{
    Cleanup();
}

void DocumentContext::SetObjectsContext(ObjectsContext* inObjectsContext)
{
	mObjectsContext = inObjectsContext;
	mJPEGImageHandler.SetOperationsContexts(this,mObjectsContext);
	mPDFDocumentHandler.SetOperationsContexts(this,mObjectsContext);
	mUsedFontsRepository.SetObjectsContext(mObjectsContext);
#ifndef PDFHUMMUS_NO_TIFF
	mTIFFImageHandler.SetOperationsContexts(this,mObjectsContext);
#endif
#ifndef PDFHUMMUS_NO_PNG
	mPNGImageHandler.SetOperationsContexts(this, mObjectsContext);
#endif
	mExtGStateRegistry.SetObjectsContext(mObjectsContext);
}

void DocumentContext::SetEmbedFonts(bool inEmbedFonts) {
	mUsedFontsRepository.SetEmbedFonts(inEmbedFonts);
}

void DocumentContext::SetOutputFileInformation(OutputFile* inOutputFile)
{
	// just save the output file path for the ID generation in the end
	mOutputFilePath = inOutputFile->GetFilePath();
    mModifiedDocumentIDExists = false;
}

void DocumentContext::AddDocumentContextExtender(IDocumentContextExtender* inExtender)
{
	mExtenders.insert(inExtender);
	mJPEGImageHandler.AddDocumentContextExtender(inExtender);
	mPDFDocumentHandler.AddDocumentContextExtender(inExtender);

	PDFDocumentCopyingContextSet::iterator it = mCopyingContexts.begin();
	for(; it != mCopyingContexts.end(); ++it)
		(*it)->AddDocumentContextExtender(inExtender);
}

void DocumentContext::RemoveDocumentContextExtender(IDocumentContextExtender* inExtender)
{
	mExtenders.erase(inExtender);
	mJPEGImageHandler.RemoveDocumentContextExtender(inExtender);
	mPDFDocumentHandler.RemoveDocumentContextExtender(inExtender);
	PDFDocumentCopyingContextSet::iterator it = mCopyingContexts.begin();
	for(; it != mCopyingContexts.end(); ++it)
		(*it)->RemoveDocumentContextExtender(inExtender);
}

TrailerInformation& DocumentContext::GetTrailerInformation()
{
	return mTrailerInformation;
}

EStatusCode	DocumentContext::WriteHeader(EPDFVersion inPDFVersion)
{
	if(mObjectsContext)
	{
		WriteHeaderComment(inPDFVersion);
		Write4BinaryBytes();
		return PDFHummus::eSuccess;
	}
	else
		return PDFHummus::eFailure;
}

static const std::string scPDFVersion10 = "PDF-1.0";
static const std::string scPDFVersion11 = "PDF-1.1";
static const std::string scPDFVersion12 = "PDF-1.2";
static const std::string scPDFVersion13 = "PDF-1.3";
static const std::string scPDFVersion14 = "PDF-1.4";
static const std::string scPDFVersion15 = "PDF-1.5";
static const std::string scPDFVersion16 = "PDF-1.6";
static const std::string scPDFVersion17 = "PDF-1.7";

void DocumentContext::WriteHeaderComment(EPDFVersion inPDFVersion)
{
	switch(inPDFVersion)
	{
		case ePDFVersion10:
			mObjectsContext->WriteComment(scPDFVersion10);
			break;
		case ePDFVersion11:
			mObjectsContext->WriteComment(scPDFVersion11);
			break;
		case ePDFVersion12:
			mObjectsContext->WriteComment(scPDFVersion12);
			break;
		case ePDFVersion13:
			mObjectsContext->WriteComment(scPDFVersion13);
			break;
		case ePDFVersion14:
		case ePDFVersionUndefined:
			mObjectsContext->WriteComment(scPDFVersion14);
			break;
		case ePDFVersion15:
			mObjectsContext->WriteComment(scPDFVersion15);
			break;
		case ePDFVersion16:
			mObjectsContext->WriteComment(scPDFVersion16);
			break;
		case ePDFVersion17:
        case ePDFVersionExtended:
			mObjectsContext->WriteComment(scPDFVersion17);
			break;
	}
}

static const IOBasicTypes::Byte scBinaryBytesArray[] = {'%',0xBD,0xBE,0xBC,0xB5,'\r','\n'}; // might imply that i need a newline writer here....an underlying primitives-token context

void DocumentContext::Write4BinaryBytes()
{
	IByteWriterWithPosition *freeContextOutput = mObjectsContext->StartFreeContext();
	freeContextOutput->Write(scBinaryBytesArray,7);
	mObjectsContext->EndFreeContext();
}

EStatusCode	DocumentContext::FinalizeNewPDF()
{
	EStatusCode status;
	LongFilePositionType xrefTablePosition;


	// this will finalize writing all renments of the file, like xref, trailer and whatever objects still accumulating
	do
	{
		status = WriteUsedFontsDefinitions();
		if(status != 0)
			break;

		// don't write page tree if no pages. this should allow
		// customizations to use an alternative algorithm for pages writing
		// just by avoiding using humusses
		if(DocumentHasNewPages())
			WritePagesTree();

		// don't write catalog if reference already setup
		// this would allow customization
		// to completly overwrite hummus catalog writing
		// by setting it beforehand
		if (!mTrailerInformation.GetRoot().first) {
			status = WriteCatalogObjectOfNewPDF();
			if (status != 0)
				break;
		}


		// write the info dictionary of the trailer, if has any valid entries
		WriteInfoDictionary();
		// write encryption dictionary, if encrypting
		WriteEncryptionDictionary();

		status = mObjectsContext->WriteXrefTable(xrefTablePosition);
		if(status != 0)
			break;

		status = WriteTrailerDictionary();
		if(status != 0)
			break;

		WriteXrefReference(xrefTablePosition);
		WriteFinalEOF();

	} while(false);

	return status;
}


static const std::string scStartXref = "startxref";
void DocumentContext::WriteXrefReference(LongFilePositionType inXrefTablePosition)
{
	mObjectsContext->WriteKeyword(scStartXref);
	mObjectsContext->WriteInteger(inXrefTablePosition,eTokenSeparatorEndLine);
}

static const IOBasicTypes::Byte scEOF[] = {'%','%','E','O','F'};

void DocumentContext::WriteFinalEOF()
{
	IByteWriterWithPosition *freeContextOutput = mObjectsContext->StartFreeContext();
	freeContextOutput->Write(scEOF,5);
	mObjectsContext->EndFreeContext();
}

static const std::string scTrailer = "trailer";
static const std::string scSize = "Size";
static const std::string scPrev = "Prev";
static const std::string scRoot = "Root";
static const std::string scEncrypt = "Encrypt";
static const std::string scInfo = "Info";
static const std::string scID = "ID";
EStatusCode DocumentContext::WriteTrailerDictionary()
{
	DictionaryContext* dictionaryContext;

	mObjectsContext->WriteKeyword(scTrailer);
	dictionaryContext = mObjectsContext->StartDictionary();

    EStatusCode status = WriteTrailerDictionaryValues(dictionaryContext);

    mObjectsContext->EndDictionary(dictionaryContext);

    return status;
}

EStatusCode DocumentContext::WriteTrailerDictionaryValues(DictionaryContext* inDictionaryContext)
{
    EStatusCode status = eSuccess;

	do
	{

		// size
		inDictionaryContext->WriteKey(scSize);
		inDictionaryContext->WriteIntegerValue(mObjectsContext->GetInDirectObjectsRegistry().GetObjectsCount());

		// prev reference
		BoolAndLongFilePositionType filePositionResult = mTrailerInformation.GetPrev();
		if(filePositionResult.first)
		{
			inDictionaryContext->WriteKey(scPrev);
			inDictionaryContext->WriteIntegerValue(filePositionResult.second);
		}

		// catalog reference
		BoolAndObjectReference objectIDResult = mTrailerInformation.GetRoot();
		if(objectIDResult.first)
		{
			inDictionaryContext->WriteKey(scRoot);
			inDictionaryContext->WriteObjectReferenceValue(objectIDResult.second);
		}
		else
		{
			TRACE_LOG("DocumentContext::WriteTrailerDictionaryValues, Unexpected Failure. Didn't find catalog object while writing trailer");
			status = PDFHummus::eFailure;
			break;
		}

		// encrypt dictionary reference
		objectIDResult = mTrailerInformation.GetEncrypt();
		if(objectIDResult.first)
		{
			inDictionaryContext->WriteKey(scEncrypt);
			inDictionaryContext->WriteObjectReferenceValue(objectIDResult.second);
		}

		// info reference
		objectIDResult = mTrailerInformation.GetInfoDictionaryReference();
		if(objectIDResult.first)
		{
			inDictionaryContext->WriteKey(scInfo);
			inDictionaryContext->WriteObjectReferenceValue(objectIDResult.second);
		}

		// write ID [must be unencrypted, in encrypted documents]
		mEncryptionHelper.PauseEncryption();

		if(mNewPDFID.empty()) // new pdf id is created prior to end in case of encryption
			mNewPDFID = GenerateMD5IDForFile();
		inDictionaryContext->WriteKey(scID);
		mObjectsContext->StartArray();

        // if modified file scenario use original ID, otherwise create a new one for the document created ID
        if(mModifiedDocumentIDExists)
            mObjectsContext->WriteHexString(mModifiedDocumentID);
        else
            mObjectsContext->WriteHexString(mNewPDFID);
		mObjectsContext->WriteHexString(mNewPDFID);
		mObjectsContext->EndArray();
		mObjectsContext->EndLine();

		mEncryptionHelper.ReleaseEncryption();

	}while(false);

	return status;
}

static const std::string scTitle = "Title";
static const std::string scAuthor = "Author";
static const std::string scSubject = "Subject";
static const std::string scKeywords = "Keywords";
static const std::string scCreator = "Creator";
static const std::string scProducer = "Producer";
static const std::string scCreationDate = "CreationDate";
static const std::string scModDate = "ModDate";
static const std::string scTrapped = "Trapped";
static const std::string scTrue = "True";
static const std::string scFalse = "False";

void DocumentContext::WriteInfoDictionary()
{
	InfoDictionary& infoDictionary = mTrailerInformation.GetInfo();
	if(infoDictionary.IsEmpty())
		return;

	ObjectIDType infoDictionaryID = mObjectsContext->StartNewIndirectObject();
	DictionaryContext* infoContext = mObjectsContext->StartDictionary();

	mTrailerInformation.SetInfoDictionaryReference(infoDictionaryID);

	if(!infoDictionary.Title.IsEmpty())
	{
		infoContext->WriteKey(scTitle);
		infoContext->WriteLiteralStringValue(infoDictionary.Title.ToString());
	}

	if(!infoDictionary.Author.IsEmpty())
	{
		infoContext->WriteKey(scAuthor);
		infoContext->WriteLiteralStringValue(infoDictionary.Author.ToString());
	}

	if(!infoDictionary.Subject.IsEmpty())
	{
		infoContext->WriteKey(scSubject);
		infoContext->WriteLiteralStringValue(infoDictionary.Subject.ToString());
	}

	if(!infoDictionary.Keywords.IsEmpty())
	{
		infoContext->WriteKey(scKeywords);
		infoContext->WriteLiteralStringValue(infoDictionary.Keywords.ToString());
	}

	if(!infoDictionary.Creator.IsEmpty())
	{
		infoContext->WriteKey(scCreator);
		infoContext->WriteLiteralStringValue(infoDictionary.Creator.ToString());
	}

	if(!infoDictionary.Producer.IsEmpty())
	{
		infoContext->WriteKey(scProducer);
		infoContext->WriteLiteralStringValue(infoDictionary.Producer.ToString());
	}

	if(!infoDictionary.CreationDate.IsNull())
	{
		infoContext->WriteKey(scCreationDate);
		infoContext->WriteLiteralStringValue(infoDictionary.CreationDate.ToString());
	}

	if(!infoDictionary.ModDate.IsNull())
	{
		infoContext->WriteKey(scModDate);
		infoContext->WriteLiteralStringValue(infoDictionary.ModDate.ToString());
	}

	if(EInfoTrappedUnknown != infoDictionary.Trapped)
	{
		infoContext->WriteKey(scTrapped);
		infoContext->WriteNameValue(EInfoTrappedTrue == infoDictionary.Trapped ? scTrue : scFalse);
	}

	MapIterator<StringToPDFTextString> it = infoDictionary.GetAdditionaEntriesIterator();

	while(it.MoveNext())
	{
		infoContext->WriteKey(it.GetKey());
		infoContext->WriteLiteralStringValue(it.GetValue().ToString());
	}

	mObjectsContext->EndDictionary(infoContext);
	mObjectsContext->EndIndirectObject();

}

void DocumentContext::WriteEncryptionDictionary() {
	if (!mEncryptionHelper.IsDocumentEncrypted())
		return;

	ObjectIDType encryptionDictionaryID = mObjectsContext->StartNewIndirectObject();
	mEncryptionHelper.WriteEncryptionDictionary(mObjectsContext);
	mObjectsContext->EndIndirectObject();

	mTrailerInformation.SetEncrypt(encryptionDictionaryID);
}

CatalogInformation& DocumentContext::GetCatalogInformation()
{
	return mCatalogInformation;
}

void DocumentContext::SetupEncryption(const EncryptionOptions& inEncryptionOptions,EPDFVersion inPDFVersion)
{
	mObjectsContext->SetEncryptionHelper(&mEncryptionHelper);
	if (inEncryptionOptions.ShouldEncrypt) {
		mNewPDFID = GenerateMD5IDForFile();

		mEncryptionHelper.Setup(
			inEncryptionOptions.ShouldEncrypt,
			((double)inPDFVersion) / 10.0,
			inEncryptionOptions.UserPassword,
			inEncryptionOptions.OwnerPassword,
			inEncryptionOptions.UserProtectionOptionsFlag,
			true,
			mNewPDFID
			);
	}
	else
		mEncryptionHelper.SetupNoEncryption();
}

void DocumentContext::SetupEncryption(PDFParser* inModifiedFileParser) {
	mObjectsContext->SetEncryptionHelper(&mEncryptionHelper);

	if (inModifiedFileParser->IsEncrypted() && inModifiedFileParser->IsEncryptionSupported()) {
		mEncryptionHelper.Setup(inModifiedFileParser->GetDecryptionHelper());
	}
	else
		mEncryptionHelper.SetupNoEncryption();
}

bool DocumentContext::SupportsEncryption()
{
	return mEncryptionHelper.SupportsEncryption();
}


static const std::string scType = "Type";
static const std::string scCatalog = "Catalog";
static const std::string scPages = "Pages";
EStatusCode DocumentContext::WriteCatalogObjectOfNewPDF()
{
    return WriteCatalogObject(DocumentHasNewPages() ? mCatalogInformation.GetPageTreeRoot(mObjectsContext->GetInDirectObjectsRegistry())->GetID(): ObjectReference());

}

EStatusCode DocumentContext::WriteCatalogObject(const ObjectReference& inPageTreeRootObjectReference,IDocumentContextExtender* inModifiedFileCopyContext)
{
	EStatusCode status = PDFHummus::eSuccess;
	ObjectIDType catalogID = mObjectsContext->StartNewIndirectObject();
	mTrailerInformation.SetRoot(catalogID); // set the catalog reference as root in the trailer

	DictionaryContext* catalogContext = mObjectsContext->StartDictionary();

	catalogContext->WriteKey(scType);
	catalogContext->WriteNameValue(scCatalog);

	if (inPageTreeRootObjectReference.ObjectID != 0) {
		catalogContext->WriteKey(scPages);
		catalogContext->WriteObjectReferenceValue(inPageTreeRootObjectReference);
	}

	IDocumentContextExtenderSet::iterator it = mExtenders.begin();
	for(; it != mExtenders.end() && PDFHummus::eSuccess == status; ++it)
	{
		status = (*it)->OnCatalogWrite(&mCatalogInformation,catalogContext,mObjectsContext,this);
		if(status != PDFHummus::eSuccess)
			TRACE_LOG("DocumentContext::WriteCatalogObject, unexpected failure. extender declared failure when writing catalog.");
	}

	if (inModifiedFileCopyContext){
		status = inModifiedFileCopyContext->OnCatalogWrite(&mCatalogInformation,catalogContext,mObjectsContext,this);
		if(status != PDFHummus::eSuccess)
			TRACE_LOG("DocumentContext::WriteCatalogObject, unexpected failure. Copying extender declared failure when writing catalog.");
	}

	mObjectsContext->EndDictionary(catalogContext);
	mObjectsContext->EndIndirectObject();
	return status;
}


void DocumentContext::WritePagesTree()
{
	PageTree* pageTreeRoot = mCatalogInformation.GetPageTreeRoot(mObjectsContext->GetInDirectObjectsRegistry());

	WritePageTree(pageTreeRoot);
}

static const std::string scCount = "Count";
static const std::string scKids = "Kids";
static const std::string scParent = "Parent";

// Recursion to write a page tree node. the return result is the page nodes count, for
// accumulation at higher levels
int DocumentContext::WritePageTree(PageTree* inPageTreeToWrite)
{
	DictionaryContext* pagesTreeContext;

	if(inPageTreeToWrite->IsLeafParent())
	{
		mObjectsContext->StartNewIndirectObject(inPageTreeToWrite->GetID());

		pagesTreeContext = mObjectsContext->StartDictionary();

		// type
		pagesTreeContext->WriteKey(scType);
		pagesTreeContext->WriteNameValue(scPages);

		// count
		pagesTreeContext->WriteKey(scCount);
		pagesTreeContext->WriteIntegerValue(inPageTreeToWrite->GetNodesCount());

		// kids
		pagesTreeContext->WriteKey(scKids);
		mObjectsContext->StartArray();
		for(int i=0;i<inPageTreeToWrite->GetNodesCount();++i)
			mObjectsContext->WriteNewIndirectObjectReference(inPageTreeToWrite->GetPageIDChild(i));
		mObjectsContext->EndArray();
		mObjectsContext->EndLine();

		// parent
		if(inPageTreeToWrite->GetParent())
		{
			pagesTreeContext->WriteKey(scParent);
			pagesTreeContext->WriteNewObjectReferenceValue(inPageTreeToWrite->GetParent()->GetID());
		}

		mObjectsContext->EndDictionary(pagesTreeContext);
		mObjectsContext->EndIndirectObject();

		return inPageTreeToWrite->GetNodesCount();
	}
	else
	{
		int totalPagesNodes = 0;

		// first loop the kids and write them (while at it, accumulate the children count).
		for(int i=0;i<inPageTreeToWrite->GetNodesCount();++i)
			totalPagesNodes += WritePageTree(inPageTreeToWrite->GetPageTreeChild(i));

		mObjectsContext->StartNewIndirectObject(inPageTreeToWrite->GetID());

		pagesTreeContext = mObjectsContext->StartDictionary();

		// type
		pagesTreeContext->WriteKey(scType);
		pagesTreeContext->WriteNameValue(scPages);

		// count
		pagesTreeContext->WriteKey(scCount);
		pagesTreeContext->WriteIntegerValue(totalPagesNodes);

		// kids
		pagesTreeContext->WriteKey(scKids);
		mObjectsContext->StartArray();
		for(int j=0;j<inPageTreeToWrite->GetNodesCount();++j)
			mObjectsContext->WriteNewIndirectObjectReference(inPageTreeToWrite->GetPageTreeChild(j)->GetID());
		mObjectsContext->EndArray();
		mObjectsContext->EndLine();

		// parent
		if(inPageTreeToWrite->GetParent())
		{
			pagesTreeContext->WriteKey(scParent);
			pagesTreeContext->WriteNewObjectReferenceValue(inPageTreeToWrite->GetParent()->GetID());
		}

		mObjectsContext->EndDictionary(pagesTreeContext);
		mObjectsContext->EndIndirectObject();

		return totalPagesNodes;
	}
}

static const std::string scResources = "Resources";
static const std::string scPage = "Page";
static const std::string scMediaBox = "MediaBox";
static const std::string scRotate = "Rotate";
static const std::string scCropBox = "CropBox";
static const std::string scBleedBox = "BleedBox";
static const std::string scTrimBox = "TrimBox";
static const std::string scArtBox = "ArtBox";
static const std::string scContents = "Contents";

EStatusCodeAndObjectIDType DocumentContext::WritePage(PDFPage* inPage)
{
	EStatusCodeAndObjectIDType result;

	result.first = PDFHummus::eSuccess;
	result.second = mObjectsContext->StartNewIndirectObject();

	DictionaryContext* pageContext = mObjectsContext->StartDictionary();

	// type
	pageContext->WriteKey(scType);
	pageContext->WriteNameValue(scPage);

	// parent
	pageContext->WriteKey(scParent);
	pageContext->WriteNewObjectReferenceValue(mCatalogInformation.AddPageToPageTree(result.second,mObjectsContext->GetInDirectObjectsRegistry()));

	// Media Box
	pageContext->WriteKey(scMediaBox);
	pageContext->WriteRectangleValue(inPage->GetMediaBox());

    // Rotate
    if(inPage->GetRotate().first)
    {
        pageContext->WriteKey(scRotate);
        pageContext->WriteIntegerValue(inPage->GetRotate().second);
    }

    // Crop Box
    PDFRectangle cropBox;
    if(inPage->GetCropBox().first && (inPage->GetCropBox().second != inPage->GetMediaBox()))
    {
        pageContext->WriteKey(scCropBox);
        pageContext->WriteRectangleValue(inPage->GetCropBox().second);
        cropBox = inPage->GetCropBox().second;
    }
    else
        cropBox = inPage->GetMediaBox();


    // Bleed Box
    if(inPage->GetBleedBox().first && (inPage->GetBleedBox().second != cropBox))
    {
        pageContext->WriteKey(scBleedBox);
        pageContext->WriteRectangleValue(inPage->GetBleedBox().second);
    }

    // Trim Box
    if(inPage->GetTrimBox().first && (inPage->GetTrimBox().second != cropBox))
    {
        pageContext->WriteKey(scTrimBox);
        pageContext->WriteRectangleValue(inPage->GetTrimBox().second);
    }

    // Art Box
    if(inPage->GetArtBox().first && (inPage->GetArtBox().second != cropBox))
    {
        pageContext->WriteKey(scArtBox);
        pageContext->WriteRectangleValue(inPage->GetArtBox().second);
    }


	do
	{
		// Resource dict
		pageContext->WriteKey(scResources);
		result.first = WriteResourcesDictionary(inPage->GetResourcesDictionary());
		if(result.first != PDFHummus::eSuccess)
		{
			TRACE_LOG("DocumentContext::WritePage, failed to write resources dictionary");
			break;
		}

		// Annotations
		if(mAnnotations.size() > 0)
		{
			pageContext->WriteKey("Annots");

			ObjectIDTypeSet::iterator it = mAnnotations.begin();

			mObjectsContext->StartArray();
			for(; it != mAnnotations.end(); ++it)
				mObjectsContext->WriteNewIndirectObjectReference(*it);
			mObjectsContext->EndArray(eTokenSeparatorEndLine);
		}
		mAnnotations.clear();

		// Content
		if(inPage->GetContentStreamsCount() > 0)
		{
			SingleValueContainerIterator<ObjectIDTypeList> iterator = inPage->GetContentStreamReferencesIterator();

			pageContext->WriteKey(scContents);
			if(inPage->GetContentStreamsCount() > 1)
			{
				mObjectsContext->StartArray();
				while(iterator.MoveNext())
					mObjectsContext->WriteNewIndirectObjectReference(iterator.GetItem());
				mObjectsContext->EndArray();
				mObjectsContext->EndLine();
			}
			else
			{
				iterator.MoveNext();
				pageContext->WriteNewObjectReferenceValue(iterator.GetItem());
			}
		}

		IDocumentContextExtenderSet::iterator it = mExtenders.begin();
		for(; it != mExtenders.end() && PDFHummus::eSuccess == result.first; ++it)
		{
			result.first = (*it)->OnPageWrite(inPage,pageContext,mObjectsContext,this);
			if(result.first != PDFHummus::eSuccess)
			{
				TRACE_LOG("DocumentContext::WritePage, unexpected failure. extender declared failure when writing page.");
				break;
			}
		}
		result.first = mObjectsContext->EndDictionary(pageContext);
		if(result.first != PDFHummus::eSuccess)
		{
			TRACE_LOG("DocumentContext::WritePage, unexpected failure. Failed to end dictionary in page write.");
			break;
		}
		mObjectsContext->EndIndirectObject();


        result.first = eSuccess;

        // now write writing tasks
		{
			PDFPageToIPageEndWritingTaskListMap::iterator itPageTasks= mPageEndTasks.find(inPage);
			if(itPageTasks != mPageEndTasks.end())
			{
				IPageEndWritingTaskList::iterator itTasks = itPageTasks->second.begin();

				for(; itTasks != itPageTasks->second.end() && eSuccess == result.first; ++itTasks)
					result.first = (*itTasks)->Write(inPage,mObjectsContext,this);

				// one time, so delete
				for(itTasks = itPageTasks->second.begin(); itTasks != itPageTasks->second.end(); ++itTasks)
					delete (*itTasks);
				mPageEndTasks.erase(itPageTasks);
			}

		}
		if(result.first != eSuccess)
			break;

		// more writing tasks
		{
			PDFPageToIObjectEndWritingTaskListMap::iterator itPageTasks= mMorePageEndTasks.find(inPage);
			if(itPageTasks != mMorePageEndTasks.end())
			{
				IObjectEndWritingTaskList::iterator itTasks = itPageTasks->second.begin();

				for(; itTasks != itPageTasks->second.end() && eSuccess == result.first; ++itTasks)
					result.first = (*itTasks)->Write(mObjectsContext,this);

				// one time, so delete
				for(itTasks = itPageTasks->second.begin(); itTasks != itPageTasks->second.end(); ++itTasks)
					delete (*itTasks);
				mMorePageEndTasks.erase(itPageTasks);
			}

		}

	}while(false);

	return result;
}


EStatusCodeAndObjectIDType DocumentContext::WritePageAndRelease(PDFPage* inPage)
{
	EStatusCodeAndObjectIDType status = WritePage(inPage);
	delete inPage;
	return status;
}

static const std::string scUnknown = "Unknown";
std::string DocumentContext::GenerateMD5IDForFile()
{
	MD5Generator md5;

	// encode current time
	PDFDate currentTime;
	currentTime.SetToCurrentTime();
	md5.Accumulate(currentTime.ToString());

	// file location
	md5.Accumulate(mOutputFilePath);

	md5.Accumulate(BoxingBaseWithRW<LongFilePositionType>(mObjectsContext->GetCurrentPosition()).ToString());

	// document information dictionary
	InfoDictionary& infoDictionary = mTrailerInformation.GetInfo();

	md5.Accumulate(infoDictionary.Title.ToString());
	md5.Accumulate(infoDictionary.Author.ToString());
	md5.Accumulate(infoDictionary.Subject.ToString());
	md5.Accumulate(infoDictionary.Keywords.ToString());
	md5.Accumulate(infoDictionary.Creator.ToString());
	md5.Accumulate(infoDictionary.Producer.ToString());
	md5.Accumulate(infoDictionary.CreationDate.ToString());
	md5.Accumulate(infoDictionary.ModDate.ToString());
	md5.Accumulate(EInfoTrappedUnknown == infoDictionary.Trapped ? scUnknown:(EInfoTrappedTrue == infoDictionary.Trapped ? scTrue:scFalse));

	MapIterator<StringToPDFTextString> it = infoDictionary.GetAdditionaEntriesIterator();

	while(it.MoveNext())
		md5.Accumulate(it.GetValue().ToString());

	return md5.ToStringAsString();
}

bool DocumentContext::HasContentContext(PDFPage* inPage)
{
	return inPage->GetAssociatedContentContext() != NULL;
}

PageContentContext* DocumentContext::StartPageContentContext(PDFPage* inPage)
{
	if(!inPage->GetAssociatedContentContext())
	{
		inPage->AssociateContentContext(new PageContentContext(this,inPage,mObjectsContext));
	}
	return inPage->GetAssociatedContentContext();
}

EStatusCode DocumentContext::PausePageContentContext(PageContentContext* inPageContext)
{
	return inPageContext->FinalizeCurrentStream();
}

EStatusCode DocumentContext::EndPageContentContext(PageContentContext* inPageContext)
{
	EStatusCode status = inPageContext->FinalizeCurrentStream();
	inPageContext->GetAssociatedPage()->DisassociateContentContext();
	delete inPageContext;
	return status;
}

static const std::string scPattern = "Pattern";
static const std::string scPatternType = "PatternType";
static const std::string scPaintType = "PaintType";
static const std::string scTilingType = "TilingType";
static const std::string scXStep = "XStep";
static const std::string scYStep = "YStep";
static const std::string scBBox = "BBox";
static const std::string scMatrix = "Matrix";


PDFTiledPattern* DocumentContext::StartTiledPattern(
	int inPaintType,
	int inTilingType,
	const PDFRectangle& inBoundingBox,
	double inXStep,
	double inYStep,
	ObjectIDType inObjectID,
	const double* inMatrix)
{
	PDFTiledPattern* aPatternObject = NULL;
	do
	{
		mObjectsContext->StartNewIndirectObject(inObjectID);
		DictionaryContext* context = mObjectsContext->StartDictionary();

		// type
		context->WriteKey(scType);
		context->WriteNameValue(scPattern);

		// pattern type
		context->WriteKey(scPatternType);
		context->WriteIntegerValue(1);

		// paint type
		context->WriteKey(scPaintType);
		context->WriteIntegerValue(inPaintType);

		// tiling type
		context->WriteKey(scTilingType);
		context->WriteIntegerValue(inTilingType);

		// x step
		context->WriteKey(scXStep);
		context->WriteDoubleValue(inXStep);

		// y step
		context->WriteKey(scYStep);
		context->WriteDoubleValue(inYStep);

		// bbox
		context->WriteKey(scBBox);
		context->WriteRectangleValue(inBoundingBox);

		// matrix
		if (inMatrix && !IsIdentityMatrix(inMatrix))
		{
			context->WriteKey(scMatrix);
			mObjectsContext->StartArray();
			for (int i = 0; i<6; ++i)
				mObjectsContext->WriteDouble(inMatrix[i]);
			mObjectsContext->EndArray(eTokenSeparatorEndLine);
		}

		// Resource dict
		context->WriteKey(scResources);
		// put a resources dictionary place holder
		ObjectIDType resourcesDictionaryID = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
		context->WriteNewObjectReferenceValue(resourcesDictionaryID);

		// Now start the stream and the form XObject state
		aPatternObject = new PDFTiledPattern(this, inObjectID, mObjectsContext->StartPDFStream(context), resourcesDictionaryID);
	} while (false);

	return aPatternObject;


}


PDFTiledPattern* DocumentContext::StartTiledPattern(int inPaintType,
	int inTilingType,
	const PDFRectangle& inBoundingBox,
	double inXStep,
	double inYStep,
	const double* inMatrix)
{
	ObjectIDType objectID = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
	return StartTiledPattern(inPaintType,
							 inTilingType,
							 inBoundingBox,
							 inXStep,
							 inYStep,
							 objectID,
							 inMatrix);

}

static const std::string scXObject = "XObject";
static const std::string scSubType = "Subtype";
static const std::string scForm = "Form";
static const std::string scFormType = "FormType";
static const std::string scGroup = "Group";
static const std::string scS = "S";
static const std::string scTransparency = "Transparency";

PDFFormXObject* DocumentContext::StartFormXObject(const PDFRectangle& inBoundingBox,ObjectIDType inFormXObjectID,const double* inMatrix,const bool inUseTransparencyGroup)
{
	PDFFormXObject* aFormXObject = NULL;
	do
	{
		mObjectsContext->StartNewIndirectObject(inFormXObjectID);
		DictionaryContext* xobjectContext = mObjectsContext->StartDictionary();

		// type
		xobjectContext->WriteKey(scType);
		xobjectContext->WriteNameValue(scXObject);

		// subtype
		xobjectContext->WriteKey(scSubType);
		xobjectContext->WriteNameValue(scForm);

		// form type
		xobjectContext->WriteKey(scFormType);
		xobjectContext->WriteIntegerValue(1);

		// bbox
		xobjectContext->WriteKey(scBBox);
		xobjectContext->WriteRectangleValue(inBoundingBox);

		// matrix
		if(inMatrix && !IsIdentityMatrix(inMatrix))
		{
			xobjectContext->WriteKey(scMatrix);
			mObjectsContext->StartArray();
			for(int i=0;i<6;++i)
				mObjectsContext->WriteDouble(inMatrix[i]);
			mObjectsContext->EndArray(eTokenSeparatorEndLine);
		}
        if (inUseTransparencyGroup) {
	        xobjectContext->WriteKey(scGroup);
			DictionaryContext* groupContext = mObjectsContext->StartDictionary();
			groupContext->WriteKey(scS);
			groupContext->WriteNameValue(scTransparency);
			mObjectsContext->EndDictionary(groupContext);
        }

		// Resource dict
		xobjectContext->WriteKey(scResources);
		// put a resources dictionary place holder
		ObjectIDType formXObjectResourcesDictionaryID = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
		xobjectContext->WriteNewObjectReferenceValue(formXObjectResourcesDictionaryID);

		IDocumentContextExtenderSet::iterator it = mExtenders.begin();
		EStatusCode status = PDFHummus::eSuccess;
		for(; it != mExtenders.end() && PDFHummus::eSuccess == status; ++it)
		{
			if((*it)->OnFormXObjectWrite(inFormXObjectID,formXObjectResourcesDictionaryID,xobjectContext,mObjectsContext,this) != PDFHummus::eSuccess)
			{
				TRACE_LOG("DocumentContext::StartFormXObject, unexpected failure. extender declared failure when writing form xobject.");
				status = PDFHummus::eFailure;
				break;
			}
		}
		if(status != PDFHummus::eSuccess)
			break;

		// Now start the stream and the form XObject state
		aFormXObject =  new PDFFormXObject(this,inFormXObjectID,mObjectsContext->StartPDFStream(xobjectContext),formXObjectResourcesDictionaryID);
	} while(false);

	return aFormXObject;
}


PDFFormXObject* DocumentContext::StartFormXObject(const PDFRectangle& inBoundingBox,const double* inMatrix,const bool inUseTransparencyGroup)
{
	ObjectIDType formXObjectID = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
	return StartFormXObject(inBoundingBox,formXObjectID,inMatrix,inUseTransparencyGroup);
}

EStatusCode DocumentContext::EndFormXObjectNoRelease(PDFFormXObject* inFormXObject)
{
	mObjectsContext->EndPDFStream(inFormXObject->GetContentStream());

	// now write the resources dictionary, full of all the goodness that got accumulated over the stream write
	mObjectsContext->StartNewIndirectObject(inFormXObject->GetResourcesDictionaryObjectID());
	WriteResourcesDictionary(inFormXObject->GetResourcesDictionary());
	mObjectsContext->EndIndirectObject();

	EStatusCode status = eSuccess;

    // now write writing tasks
	{
		PDFFormXObjectToIFormEndWritingTaskListMap::iterator it= mFormEndTasks.find(inFormXObject);

		if(it != mFormEndTasks.end())
		{
			IFormEndWritingTaskList::iterator itTasks = it->second.begin();

			for(; itTasks != it->second.end() && eSuccess == status; ++itTasks)
				status = (*itTasks)->Write(inFormXObject,mObjectsContext,this);

			// one time, so delete
			for(itTasks = it->second.begin(); itTasks != it->second.end(); ++itTasks)
				delete (*itTasks);
			mFormEndTasks.erase(it);
		}

	}
	if(status != eSuccess)
		return status;


	// and more writing tasks
	{
		PDFFormXObjectToIObjectEndWritingTaskListMap::iterator it= mMoreFormEndTasks.find(inFormXObject);

		if(it != mMoreFormEndTasks.end())
		{
			IObjectEndWritingTaskList::iterator itTasks = it->second.begin();

			for(; itTasks != it->second.end() && eSuccess == status; ++itTasks)
				status = (*itTasks)->Write(mObjectsContext,this);

			// one time, so delete
			for(itTasks = it->second.begin(); itTasks != it->second.end(); ++itTasks)
				delete (*itTasks);
			mMoreFormEndTasks.erase(it);
		}		
	}

	return status;
}

EStatusCode DocumentContext::EndTiledPattern(PDFTiledPattern* inTiledPattern)
{
	mObjectsContext->EndPDFStream(inTiledPattern->GetContentStream());

	// now write the resources dictionary, full of all the goodness that got accumulated over the stream write
	mObjectsContext->StartNewIndirectObject(inTiledPattern->GetResourcesDictionaryObjectID());
	WriteResourcesDictionary(inTiledPattern->GetResourcesDictionary());
	mObjectsContext->EndIndirectObject();

	EStatusCode status = eSuccess;

	// now write writing tasks
	{
		PDFTiledPatternToITiledPatternEndWritingTaskListMap::iterator it = mTiledPatternEndTasks.find(inTiledPattern);

		if (it != mTiledPatternEndTasks.end())
		{
			ITiledPatternEndWritingTaskList::iterator itTasks = it->second.begin();

			for (; itTasks != it->second.end() && eSuccess == status; ++itTasks)
				status = (*itTasks)->Write(inTiledPattern, mObjectsContext, this);

			// one time, so delete
			for (itTasks = it->second.begin(); itTasks != it->second.end(); ++itTasks)
				delete (*itTasks);
			mTiledPatternEndTasks.erase(it);
		}
	}
	if(status != eSuccess)
		return status;

	// and more writing tasks
	{
		PDFTiledPatternToIObjectEndWritingTaskListMap::iterator it= mMoreTiledPatternEndTasks.find(inTiledPattern);

		if(it != mMoreTiledPatternEndTasks.end())
		{
			IObjectEndWritingTaskList::iterator itTasks = it->second.begin();

			for(; itTasks != it->second.end() && eSuccess == status; ++itTasks)
				status = (*itTasks)->Write(mObjectsContext,this);

			// one time, so delete
			for(itTasks = it->second.begin(); itTasks != it->second.end(); ++itTasks)
				delete (*itTasks);
			mMoreTiledPatternEndTasks.erase(it);
		}		
	}		


	return status;
}

EStatusCode DocumentContext::EndTiledPatternAndRelease(PDFTiledPattern* inTiledPattern)
{
	EStatusCode status = EndTiledPattern(inTiledPattern);
	delete inTiledPattern;

	return status;

}


EStatusCode DocumentContext::EndFormXObject(PDFFormXObject* inFormXObject)
{
	return EndFormXObjectNoRelease(inFormXObject);
}

EStatusCode DocumentContext::EndFormXObjectAndRelease(PDFFormXObject* inFormXObject)
{
	EStatusCode status = EndFormXObjectNoRelease(inFormXObject);
	delete inFormXObject; // will also delete the stream becuase the form XObject owns it

	return status;
}

static const std::string scProcesets = "ProcSet";
static const std::string scXObjects = "XObject";
static const std::string scExtGStates = "ExtGState";
static const std::string scFonts = "Font";
static const std::string scColorSpaces = "ColorSpace";
static const std::string scPatterns = "Pattern";
static const std::string scShadings = "Shading";
static const std::string scProperties = "Properties";
EStatusCode DocumentContext::WriteResourcesDictionary(ResourcesDictionary& inResourcesDictionary)
{
	EStatusCode status = PDFHummus::eSuccess;

	do
	{

		DictionaryContext* resourcesContext = mObjectsContext->StartDictionary();


        //	Procsets
        SingleValueContainerIterator<StringSet> itProcesets = inResourcesDictionary.GetProcesetsIterator();
        if(itProcesets.MoveNext())
        {
            resourcesContext->WriteKey(scProcesets);
            mObjectsContext->StartArray();
            do
            {
                mObjectsContext->WriteName(itProcesets.GetItem());
            }
            while (itProcesets.MoveNext());
            mObjectsContext->EndArray();
            mObjectsContext->EndLine();
        }

        // XObjects
        status = WriteResourceDictionary(&inResourcesDictionary,resourcesContext,scXObjects,inResourcesDictionary.GetXObjectsIterator());
        if(status!=eSuccess)
            break;

		// ExtGStates
        status = WriteResourceDictionary(&inResourcesDictionary,resourcesContext,scExtGStates,inResourcesDictionary.GetExtGStatesIterator());
        if(status!=eSuccess)
            break;

		// Fonts
        status = WriteResourceDictionary(&inResourcesDictionary,resourcesContext,scFonts,inResourcesDictionary.GetFontsIterator());
        if(status!=eSuccess)
            break;

		// Color space
        status = WriteResourceDictionary(&inResourcesDictionary,resourcesContext,scColorSpaces,inResourcesDictionary.GetColorSpacesIterator());

		// Patterns
        status = WriteResourceDictionary(&inResourcesDictionary,resourcesContext,scPatterns,inResourcesDictionary.GetPatternsIterator());
        if(status!=eSuccess)
            break;

		// Shading
        status = WriteResourceDictionary(&inResourcesDictionary,resourcesContext,scShadings,inResourcesDictionary.GetShadingsIterator());
        if(status!=eSuccess)
            break;

		// Properties
        status = WriteResourceDictionary(&inResourcesDictionary,resourcesContext,scProperties,inResourcesDictionary.GetPropertiesIterator());
        if(status!=eSuccess)
            break;

		IDocumentContextExtenderSet::iterator itExtenders = mExtenders.begin();
		for(; itExtenders != mExtenders.end() && PDFHummus::eSuccess == status; ++itExtenders)
		{
			status = (*itExtenders)->OnResourcesWrite(&(inResourcesDictionary),resourcesContext,mObjectsContext,this);
			if(status != PDFHummus::eSuccess)
			{
				TRACE_LOG("DocumentContext::WriteResourcesDictionary, unexpected failure. extender declared failure when writing resources.");
				break;
			}
		}

		mObjectsContext->EndDictionary(resourcesContext);
	}while(false);

	return status;
}

EStatusCode DocumentContext::WriteResourceDictionary(ResourcesDictionary* inResourcesDictionary,
                                              DictionaryContext* inResourcesCategoryDictionary,
											const std::string& inResourceDictionaryLabel,
											MapIterator<ObjectIDTypeToStringMap> inMapping)
{
    EStatusCode status = eSuccess;

    ResourcesDictionaryAndStringToIResourceWritingTaskListMap::iterator itWriterTasks =
        mResourcesTasks.find(ResourcesDictionaryAndString(inResourcesDictionary,inResourceDictionaryLabel));

    if(inMapping.MoveNext() || itWriterTasks != mResourcesTasks.end())
    {
        do {
            inResourcesCategoryDictionary->WriteKey(inResourceDictionaryLabel);
            DictionaryContext* resourceContext = mObjectsContext->StartDictionary();

            if(!inMapping.IsFinished())
            {
                do
                {
                    resourceContext->WriteKey(inMapping.GetValue());
                    resourceContext->WriteNewObjectReferenceValue(inMapping.GetKey());
                }
                while(inMapping.MoveNext());
            }

            if(itWriterTasks != mResourcesTasks.end())
            {
                IResourceWritingTaskList::iterator itTasks = itWriterTasks->second.begin();

                for(; itTasks != itWriterTasks->second.end() && eSuccess == status; ++itTasks)
                    status = (*itTasks)->Write(inResourcesCategoryDictionary,mObjectsContext,this);

                // Discard the tasks for this category
                for(itTasks = itWriterTasks->second.begin(); itTasks != itWriterTasks->second.end(); ++itTasks)
                    delete *itTasks;
                mResourcesTasks.erase(itWriterTasks);
                if(status != eSuccess)
                    break;
            }

            IDocumentContextExtenderSet::iterator it = mExtenders.begin();
            EStatusCode status = PDFHummus::eSuccess;
            for(; it != mExtenders.end() && eSuccess == status; ++it)
            {
                status = (*it)->OnResourceDictionaryWrite(resourceContext,inResourceDictionaryLabel,mObjectsContext,this);
                if(status != PDFHummus::eSuccess)
                {
                    TRACE_LOG("DocumentContext::WriteResourceDictionary, unexpected failure. extender declared failure when writing a resource dictionary.");
                    break;
                }
            }

            mObjectsContext->EndDictionary(resourceContext);

        }
        while (false);

    }

    return status;
}



bool DocumentContext::IsIdentityMatrix(const double* inMatrix)
{
	return
		inMatrix[0] == 1 &&
		inMatrix[1] == 0 &&
		inMatrix[2] == 0 &&
		inMatrix[3] == 1 &&
		inMatrix[4] == 0 &&
		inMatrix[5] == 0;

}

PDFImageXObject* DocumentContext::CreateImageXObjectFromJPGFile(const std::string& inJPGFilePath)
{
	return mJPEGImageHandler.CreateImageXObjectFromJPGFile(inJPGFilePath);
}

PDFFormXObject* DocumentContext::CreateFormXObjectFromJPGFile(const std::string& inJPGFilePath)
{
	return mJPEGImageHandler.CreateFormXObjectFromJPGFile(inJPGFilePath);
}

#ifndef PDFHUMMUS_NO_PNG
PDFFormXObject* DocumentContext::CreateFormXObjectFromPNGStream(IByteReaderWithPosition* inPNGStream, ObjectIDType inFormXObjectId)
{
	return mPNGImageHandler.CreateFormXObjectFromPNGStream(inPNGStream, inFormXObjectId);
}
#endif

JPEGImageHandler& DocumentContext::GetJPEGImageHandler()
{
	return mJPEGImageHandler;
}

#ifndef PDFHUMMUS_NO_TIFF
TIFFImageHandler& DocumentContext::GetTIFFImageHandler()
{
	return mTIFFImageHandler;
}

PDFFormXObject* DocumentContext::CreateFormXObjectFromTIFFFile(	const std::string& inTIFFFilePath,
																const TIFFUsageParameters& inTIFFUsageParameters)
{

	return mTIFFImageHandler.CreateFormXObjectFromTIFFFile(inTIFFFilePath,inTIFFUsageParameters);
}

PDFFormXObject* DocumentContext::CreateFormXObjectFromTIFFFile(
                                                               const std::string& inTIFFFilePath,
                                                               ObjectIDType inFormXObjectID,
                                                               const TIFFUsageParameters& inTIFFUsageParameters)
{
	return mTIFFImageHandler.CreateFormXObjectFromTIFFFile(inTIFFFilePath,inFormXObjectID,inTIFFUsageParameters);
}

PDFFormXObject* DocumentContext::CreateFormXObjectFromTIFFStream(IByteReaderWithPosition* inTIFFStream,
                                                                 const TIFFUsageParameters& inTIFFUsageParameters)
{
	return mTIFFImageHandler.CreateFormXObjectFromTIFFStream(inTIFFStream,inTIFFUsageParameters);
}

PDFFormXObject* DocumentContext::CreateFormXObjectFromTIFFStream(IByteReaderWithPosition* inTIFFStream,
                                                                 ObjectIDType inFormXObjectID,
                                                                 const TIFFUsageParameters& inTIFFUsageParameters)
{
	return mTIFFImageHandler.CreateFormXObjectFromTIFFStream(inTIFFStream,inFormXObjectID,inTIFFUsageParameters);
}

#endif

PDFImageXObject* DocumentContext::CreateImageXObjectFromJPGFile(const std::string& inJPGFilePath,ObjectIDType inImageXObjectID)
{
	return mJPEGImageHandler.CreateImageXObjectFromJPGFile(inJPGFilePath,inImageXObjectID);
}

PDFFormXObject* DocumentContext::CreateFormXObjectFromJPGFile(const std::string& inJPGFilePath,ObjectIDType inFormXObjectID)
{
	return mJPEGImageHandler.CreateFormXObjectFromJPGFile(inJPGFilePath,inFormXObjectID);
}




PDFUsedFont* DocumentContext::GetFontForFile(const std::string& inFontFilePath,long inFontIndex)
{
	return mUsedFontsRepository.GetFontForFile(inFontFilePath,inFontIndex);
}

EStatusCode DocumentContext::WriteUsedFontsDefinitions()
{
	return mUsedFontsRepository.WriteUsedFontsDefinitions();
}

PDFUsedFont* DocumentContext::GetFontForFile(const std::string& inFontFilePath,const std::string& inAdditionalMeticsFilePath,long inFontIndex)
{
	return mUsedFontsRepository.GetFontForFile(inFontFilePath,inAdditionalMeticsFilePath,inFontIndex);
}

EStatusCodeAndObjectIDTypeList DocumentContext::CreateFormXObjectsFromPDF(const std::string& inPDFFilePath,
																			const PDFParsingOptions& inParsingOptions,
																			const PDFPageRange& inPageRange,
																			EPDFPageBox inPageBoxToUseAsFormBox,
																			const double* inTransformationMatrix,
																			const ObjectIDTypeList& inCopyAdditionalObjects,
																			const ObjectIDTypeList& inPredefinedFormIDs)
{
	return mPDFDocumentHandler.CreateFormXObjectsFromPDF(inPDFFilePath,inParsingOptions,inPageRange,inPageBoxToUseAsFormBox,inTransformationMatrix,inCopyAdditionalObjects,inPredefinedFormIDs);

}

EStatusCodeAndObjectIDTypeList DocumentContext::CreateFormXObjectsFromPDF(const std::string& inPDFFilePath,
																			const PDFParsingOptions& inParsingOptions,
																			const PDFPageRange& inPageRange,
																			const PDFRectangle& inCropBox,
																			const double* inTransformationMatrix,
																			const ObjectIDTypeList& inCopyAdditionalObjects,
																			const ObjectIDTypeList& inPredefinedFormIDs)
{
	return mPDFDocumentHandler.CreateFormXObjectsFromPDF(inPDFFilePath,inParsingOptions,inPageRange,inCropBox,inTransformationMatrix,inCopyAdditionalObjects, inPredefinedFormIDs);

}
EStatusCodeAndObjectIDTypeList DocumentContext::AppendPDFPagesFromPDF(const std::string& inPDFFilePath,
																	const PDFParsingOptions& inParsingOptions,
																	  const PDFPageRange& inPageRange,
																	  const ObjectIDTypeList& inCopyAdditionalObjects)
{
	return mPDFDocumentHandler.AppendPDFPagesFromPDF(inPDFFilePath,inParsingOptions,inPageRange,inCopyAdditionalObjects);
}

EStatusCode DocumentContext::WriteState(ObjectsContext* inStateWriter,ObjectIDType inObjectID)
{
	EStatusCode status;

	do
	{
		inStateWriter->StartNewIndirectObject(inObjectID);

		ObjectIDType trailerInformationID = inStateWriter->GetInDirectObjectsRegistry().AllocateNewObjectID();
		ObjectIDType catalogInformationID = inStateWriter->GetInDirectObjectsRegistry().AllocateNewObjectID();
		ObjectIDType usedFontsRepositoryID = inStateWriter->GetInDirectObjectsRegistry().AllocateNewObjectID();
		ObjectIDType encryptionHelperID = inStateWriter->GetInDirectObjectsRegistry().AllocateNewObjectID();

		DictionaryContext* documentDictionary = inStateWriter->StartDictionary();

		documentDictionary->WriteKey("Type");
		documentDictionary->WriteNameValue("DocumentContext");

		documentDictionary->WriteKey("mTrailerInformation");
		documentDictionary->WriteNewObjectReferenceValue(trailerInformationID);

		documentDictionary->WriteKey("mCatalogInformation");
		documentDictionary->WriteNewObjectReferenceValue(catalogInformationID);

		documentDictionary->WriteKey("mUsedFontsRepository");
		documentDictionary->WriteNewObjectReferenceValue(usedFontsRepositoryID);

		documentDictionary->WriteKey("mEncryptionHelper");
		documentDictionary->WriteNewObjectReferenceValue(encryptionHelperID);


        documentDictionary->WriteKey("mModifiedDocumentIDExists");
        documentDictionary->WriteBooleanValue(mModifiedDocumentIDExists);

        if(mModifiedDocumentIDExists)
        {
            documentDictionary->WriteKey("mModifiedDocumentID");
            documentDictionary->WriteHexStringValue(mModifiedDocumentID);
        }

		if (!mNewPDFID.empty())
		{
			documentDictionary->WriteKey("mNewPDFID");
			documentDictionary->WriteHexStringValue(mNewPDFID);
		}

		inStateWriter->EndDictionary(documentDictionary);
		inStateWriter->EndIndirectObject();

		WriteTrailerState(inStateWriter,trailerInformationID);
		WriteCatalogInformationState(inStateWriter,catalogInformationID);

		status = mUsedFontsRepository.WriteState(inStateWriter,usedFontsRepositoryID);
		if(status != PDFHummus::eSuccess)
			break;

		status = mEncryptionHelper.WriteState(inStateWriter, encryptionHelperID);
		if (status != PDFHummus::eSuccess)
			break;
	}while(false);

	return status;
}

void DocumentContext::WriteReferenceState(ObjectsContext* inStateWriter,
                                          const ObjectReference& inReference)
{
    DictionaryContext* referenceContext = inStateWriter->StartDictionary();

    referenceContext->WriteKey("ObjectID");
    referenceContext->WriteIntegerValue(inReference.ObjectID);

    referenceContext->WriteKey("GenerationNumber");
    referenceContext->WriteIntegerValue(inReference.GenerationNumber);

    inStateWriter->EndDictionary(referenceContext);
}

void DocumentContext::WriteTrailerState(ObjectsContext* inStateWriter,ObjectIDType inObjectID)
{
	inStateWriter->StartNewIndirectObject(inObjectID);

	DictionaryContext* trailerDictionary = inStateWriter->StartDictionary();

	trailerDictionary->WriteKey("Type");
	trailerDictionary->WriteNameValue("TrailerInformation");

	trailerDictionary->WriteKey("mPrev");
	trailerDictionary->WriteIntegerValue(mTrailerInformation.GetPrev().second);

	trailerDictionary->WriteKey("mRootReference");
	WriteReferenceState(inStateWriter,mTrailerInformation.GetRoot().second);

	trailerDictionary->WriteKey("mEncryptReference");
	WriteReferenceState(inStateWriter,mTrailerInformation.GetEncrypt().second);

	trailerDictionary->WriteKey("mInfoDictionary");
	ObjectIDType infoDictionaryID = inStateWriter->GetInDirectObjectsRegistry().AllocateNewObjectID();
	trailerDictionary->WriteNewObjectReferenceValue(infoDictionaryID);

	trailerDictionary->WriteKey("mInfoDictionaryReference");
	WriteReferenceState(inStateWriter,mTrailerInformation.GetInfoDictionaryReference().second);

	inStateWriter->EndDictionary(trailerDictionary);
	inStateWriter->EndIndirectObject();

	WriteTrailerInfoState(inStateWriter,infoDictionaryID);
}

void DocumentContext::WriteTrailerInfoState(ObjectsContext* inStateWriter,ObjectIDType inObjectID)
{
	inStateWriter->StartNewIndirectObject(inObjectID);
	DictionaryContext* infoDictionary = inStateWriter->StartDictionary();

	infoDictionary->WriteKey("Type");
	infoDictionary->WriteNameValue("InfoDictionary");

	infoDictionary->WriteKey("Title");
	infoDictionary->WriteLiteralStringValue(mTrailerInformation.GetInfo().Title.ToString());

	infoDictionary->WriteKey("Author");
	infoDictionary->WriteLiteralStringValue(mTrailerInformation.GetInfo().Author.ToString());

	infoDictionary->WriteKey("Subject");
	infoDictionary->WriteLiteralStringValue(mTrailerInformation.GetInfo().Subject.ToString());

	infoDictionary->WriteKey("Keywords");
	infoDictionary->WriteLiteralStringValue(mTrailerInformation.GetInfo().Keywords.ToString());

	infoDictionary->WriteKey("Creator");
	infoDictionary->WriteLiteralStringValue(mTrailerInformation.GetInfo().Creator.ToString());

	infoDictionary->WriteKey("Producer");
	infoDictionary->WriteLiteralStringValue(mTrailerInformation.GetInfo().Producer.ToString());

	infoDictionary->WriteKey("CreationDate");
	WriteDateState(inStateWriter,mTrailerInformation.GetInfo().CreationDate);

	infoDictionary->WriteKey("ModDate");
	WriteDateState(inStateWriter,mTrailerInformation.GetInfo().ModDate);

	infoDictionary->WriteKey("Trapped");
	infoDictionary->WriteIntegerValue(mTrailerInformation.GetInfo().Trapped);

	MapIterator<StringToPDFTextString> itAdditionalInfo = mTrailerInformation.GetInfo().GetAdditionaEntriesIterator();

	infoDictionary->WriteKey("mAdditionalInfoEntries");
	DictionaryContext* additionalInfoDictionary = inStateWriter->StartDictionary();
	while(itAdditionalInfo.MoveNext())
	{
		additionalInfoDictionary->WriteKey(itAdditionalInfo.GetKey());
		additionalInfoDictionary->WriteLiteralStringValue(itAdditionalInfo.GetValue().ToString());
	}
	inStateWriter->EndDictionary(additionalInfoDictionary);

	inStateWriter->EndDictionary(infoDictionary);
	inStateWriter->EndIndirectObject();

}

void DocumentContext::WriteDateState(ObjectsContext* inStateWriter,const PDFDate& inDate)
{
	DictionaryContext* dateDictionary = inStateWriter->StartDictionary();

	dateDictionary->WriteKey("Type");
	dateDictionary->WriteNameValue("Date");

	dateDictionary->WriteKey("Year");
	dateDictionary->WriteIntegerValue(inDate.Year);

	dateDictionary->WriteKey("Month");
	dateDictionary->WriteIntegerValue(inDate.Month);

	dateDictionary->WriteKey("Day");
	dateDictionary->WriteIntegerValue(inDate.Day);

	dateDictionary->WriteKey("Hour");
	dateDictionary->WriteIntegerValue(inDate.Hour);

	dateDictionary->WriteKey("Minute");
	dateDictionary->WriteIntegerValue(inDate.Minute);

	dateDictionary->WriteKey("Second");
	dateDictionary->WriteIntegerValue(inDate.Second);

	dateDictionary->WriteKey("UTC");
	dateDictionary->WriteIntegerValue(inDate.UTC);

	dateDictionary->WriteKey("HourFromUTC");
	dateDictionary->WriteIntegerValue(inDate.HourFromUTC);

	dateDictionary->WriteKey("MinuteFromUTC");
	dateDictionary->WriteIntegerValue(inDate.MinuteFromUTC);

	inStateWriter->EndDictionary(dateDictionary);
}

void DocumentContext::WriteCatalogInformationState(ObjectsContext* inStateWriter,ObjectIDType inObjectID)
{
	ObjectIDType rootNodeID = 0;
	if(mCatalogInformation.GetCurrentPageTreeNode())
	{
		rootNodeID = inStateWriter->GetInDirectObjectsRegistry().AllocateNewObjectID();
		WritePageTreeState(inStateWriter,rootNodeID,mCatalogInformation.GetPageTreeRoot(mObjectsContext->GetInDirectObjectsRegistry()));
	}


	inStateWriter->StartNewIndirectObject(inObjectID);
	DictionaryContext* catalogInformation = inStateWriter->StartDictionary();

	catalogInformation->WriteKey("Type");
	catalogInformation->WriteNameValue("CatalogInformation");

	if(mCatalogInformation.GetCurrentPageTreeNode())
	{
		catalogInformation->WriteKey("PageTreeRoot");
		catalogInformation->WriteNewObjectReferenceValue(rootNodeID);

		catalogInformation->WriteKey("mCurrentPageTreeNode");
		catalogInformation->WriteNewObjectReferenceValue(mCurrentPageTreeIDInState);
	}

	inStateWriter->EndDictionary(catalogInformation);
	inStateWriter->EndIndirectObject();

}

void DocumentContext::WritePageTreeState(ObjectsContext* inStateWriter,ObjectIDType inObjectID,PageTree* inPageTree)
{
	ObjectIDTypeList kidsObjectIDs;

	inStateWriter->StartNewIndirectObject(inObjectID);
	DictionaryContext* pageTreeDictionary = inStateWriter->StartDictionary();

	pageTreeDictionary->WriteKey("Type");
	pageTreeDictionary->WriteNameValue("PageTree");

	pageTreeDictionary->WriteKey("mPageTreeID");
	pageTreeDictionary->WriteIntegerValue(inPageTree->GetID());

	pageTreeDictionary->WriteKey("mIsLeafParent");
	pageTreeDictionary->WriteBooleanValue(inPageTree->IsLeafParent());

	if(inPageTree->IsLeafParent())
	{
		pageTreeDictionary->WriteKey("mKidsIDs");
		inStateWriter->StartArray();
		for(int i=0;i<inPageTree->GetNodesCount();++i)
			inStateWriter->WriteInteger(inPageTree->GetPageIDChild(i));
		inStateWriter->EndArray(eTokenSeparatorEndLine);
	}
	else
	{
		pageTreeDictionary->WriteKey("mKidsNodes");
		inStateWriter->StartArray();
		for(int i=0;i<inPageTree->GetNodesCount();++i)
		{
			ObjectIDType pageNodeObjectID = inStateWriter->GetInDirectObjectsRegistry().AllocateNewObjectID();
			inStateWriter->WriteNewIndirectObjectReference(pageNodeObjectID);
			kidsObjectIDs.push_back(pageNodeObjectID);
		}
		inStateWriter->EndArray(eTokenSeparatorEndLine);
	}

	inStateWriter->EndDictionary(pageTreeDictionary);
	inStateWriter->EndIndirectObject();

	if(kidsObjectIDs.size() > 0)
	{
		ObjectIDTypeList::iterator it = kidsObjectIDs.begin();
		int i = 0;
		for(;i < inPageTree->GetNodesCount();++i,++it)
			WritePageTreeState(inStateWriter,*it,inPageTree->GetPageTreeChild(i));
	}

	if(inPageTree == mCatalogInformation.GetCurrentPageTreeNode())
	{
		mCurrentPageTreeIDInState = inObjectID;
	}
}

EStatusCode DocumentContext::ReadState(PDFParser* inStateReader,ObjectIDType inObjectID)
{
	PDFObjectCastPtr<PDFDictionary> documentState(inStateReader->ParseNewObject(inObjectID));

    PDFObjectCastPtr<PDFBoolean> modifiedDocumentExists(documentState->QueryDirectObject("mModifiedDocumentIDExists"));
    mModifiedDocumentIDExists = modifiedDocumentExists->GetValue();

    if(mModifiedDocumentIDExists)
    {
        PDFObjectCastPtr<PDFHexString> modifiedDocumentExists(documentState->QueryDirectObject("mModifiedDocumentID"));
        mModifiedDocumentID = modifiedDocumentExists->GetValue();
    }

	PDFObjectCastPtr<PDFHexString> newPDFID(documentState->QueryDirectObject("mNewPDFID"));

	if (!!newPDFID)
		mNewPDFID = newPDFID->GetValue();

	PDFObjectCastPtr<PDFDictionary> trailerInformationState(inStateReader->QueryDictionaryObject(documentState.GetPtr(),"mTrailerInformation"));
	ReadTrailerState(inStateReader,trailerInformationState.GetPtr());

	PDFObjectCastPtr<PDFDictionary> catalogInformationState(inStateReader->QueryDictionaryObject(documentState.GetPtr(),"mCatalogInformation"));
	ReadCatalogInformationState(inStateReader,catalogInformationState.GetPtr());

	PDFObjectCastPtr<PDFIndirectObjectReference> usedFontsInformationStateID(documentState->QueryDirectObject("mUsedFontsRepository"));
	EStatusCode status =  mUsedFontsRepository.ReadState(inStateReader, usedFontsInformationStateID->mObjectID);
	if (status != eSuccess)
		return status;

	PDFObjectCastPtr<PDFIndirectObjectReference> encrytpionStateID(documentState->QueryDirectObject("mEncryptionHelper"));
	return mEncryptionHelper.ReadState(inStateReader, encrytpionStateID->mObjectID);
}

void DocumentContext::ReadTrailerState(PDFParser* inStateReader,PDFDictionary* inTrailerState)
{
	PDFObjectCastPtr<PDFInteger> prevState(inTrailerState->QueryDirectObject("mPrev"));
	mTrailerInformation.SetPrev(prevState->GetValue());

	PDFObjectCastPtr<PDFDictionary> rootReferenceState(inTrailerState->QueryDirectObject("mRootReference"));
	mTrailerInformation.SetRoot(GetReferenceFromState(rootReferenceState.GetPtr()));

	PDFObjectCastPtr<PDFDictionary> encryptReferenceState(inTrailerState->QueryDirectObject("mEncryptReference"));
	mTrailerInformation.SetEncrypt(GetReferenceFromState(encryptReferenceState.GetPtr()));

	PDFObjectCastPtr<PDFDictionary> infoDictionaryState(inStateReader->QueryDictionaryObject(inTrailerState,"mInfoDictionary"));
	ReadTrailerInfoState(inStateReader,infoDictionaryState.GetPtr());

	PDFObjectCastPtr<PDFDictionary> infoDictionaryReferenceState(inTrailerState->QueryDirectObject("mInfoDictionaryReference"));
	mTrailerInformation.SetInfoDictionaryReference(GetReferenceFromState(infoDictionaryReferenceState.GetPtr()));

}

ObjectReference DocumentContext::GetReferenceFromState(PDFDictionary* inDictionary)
{
    PDFObjectCastPtr<PDFInteger> objectID(inDictionary->QueryDirectObject("ObjectID"));
    PDFObjectCastPtr<PDFInteger> generationNumber(inDictionary->QueryDirectObject("GenerationNumber"));

    return ObjectReference((ObjectIDType)(objectID->GetValue()),(unsigned long)generationNumber->GetValue());
}

void DocumentContext::ReadTrailerInfoState(PDFParser* inStateReader,PDFDictionary* inTrailerInfoState)
{
	PDFObjectCastPtr<PDFLiteralString> titleState(inTrailerInfoState->QueryDirectObject("Title"));
	mTrailerInformation.GetInfo().Title = titleState->GetValue();

	PDFObjectCastPtr<PDFLiteralString> authorState(inTrailerInfoState->QueryDirectObject("Author"));
	mTrailerInformation.GetInfo().Author = authorState->GetValue();

	PDFObjectCastPtr<PDFLiteralString> subjectState(inTrailerInfoState->QueryDirectObject("Subject"));
	mTrailerInformation.GetInfo().Subject = subjectState->GetValue();

	PDFObjectCastPtr<PDFLiteralString> keywordsState(inTrailerInfoState->QueryDirectObject("Keywords"));
	mTrailerInformation.GetInfo().Keywords = keywordsState->GetValue();

	PDFObjectCastPtr<PDFLiteralString> creatorState(inTrailerInfoState->QueryDirectObject("Creator"));
	mTrailerInformation.GetInfo().Creator = creatorState->GetValue();

	PDFObjectCastPtr<PDFLiteralString> producerState(inTrailerInfoState->QueryDirectObject("Producer"));
	mTrailerInformation.GetInfo().Producer = producerState->GetValue();

	PDFObjectCastPtr<PDFDictionary> creationDateState(inTrailerInfoState->QueryDirectObject("CreationDate"));
	ReadDateState(creationDateState.GetPtr(),mTrailerInformation.GetInfo().CreationDate);

	PDFObjectCastPtr<PDFDictionary> modDateState(inTrailerInfoState->QueryDirectObject("ModDate"));
	ReadDateState(creationDateState.GetPtr(),mTrailerInformation.GetInfo().ModDate);

	PDFObjectCastPtr<PDFInteger> trappedState(inTrailerInfoState->QueryDirectObject("Trapped"));
	mTrailerInformation.GetInfo().Trapped = (EInfoTrapped)trappedState->GetValue();

	PDFObjectCastPtr<PDFDictionary> additionalInfoState(inTrailerInfoState->QueryDirectObject("mAdditionalInfoEntries"));

	MapIterator<PDFNameToPDFObjectMap> it = additionalInfoState->GetIterator();
	PDFObjectCastPtr<PDFName> keyState;
	PDFObjectCastPtr<PDFLiteralString> valueState;

	mTrailerInformation.GetInfo().ClearAdditionalInfoEntries();
	while(it.MoveNext())
	{
		keyState = it.GetKey();
		valueState = it.GetValue();

		mTrailerInformation.GetInfo().AddAdditionalInfoEntry(keyState->GetValue(),PDFTextString(valueState->GetValue()));
	}
}

void DocumentContext::ReadDateState(PDFDictionary* inDateState,PDFDate& inDate)
{
	PDFObjectCastPtr<PDFInteger> yearState(inDateState->QueryDirectObject("Year"));
	inDate.Year = (int)yearState->GetValue();

	PDFObjectCastPtr<PDFInteger> monthState(inDateState->QueryDirectObject("Month"));
	inDate.Month = (int)monthState->GetValue();

	PDFObjectCastPtr<PDFInteger> dayState(inDateState->QueryDirectObject("Day"));
	inDate.Day = (int)dayState->GetValue();

	PDFObjectCastPtr<PDFInteger> hourState(inDateState->QueryDirectObject("Hour"));
	inDate.Hour = (int)hourState->GetValue();

	PDFObjectCastPtr<PDFInteger> minuteState(inDateState->QueryDirectObject("Minute"));
	inDate.Minute = (int)minuteState->GetValue();

	PDFObjectCastPtr<PDFInteger> secondState(inDateState->QueryDirectObject("Second"));
	inDate.Second = (int)secondState->GetValue();

	PDFObjectCastPtr<PDFInteger> utcState(inDateState->QueryDirectObject("UTC"));
	inDate.UTC = (PDFDate::EUTCRelation)utcState->GetValue();

	PDFObjectCastPtr<PDFInteger> hourFromUTCState(inDateState->QueryDirectObject("HourFromUTC"));
	inDate.HourFromUTC = (int)hourFromUTCState->GetValue();

	PDFObjectCastPtr<PDFInteger> minuteFromUTCState(inDateState->QueryDirectObject("MinuteFromUTC"));
	inDate.MinuteFromUTC = (int)minuteFromUTCState->GetValue();
}

void DocumentContext::ReadCatalogInformationState(PDFParser* inStateReader,PDFDictionary* inCatalogInformationState)
{
	PDFObjectCastPtr<PDFIndirectObjectReference> pageTreeRootState(inCatalogInformationState->QueryDirectObject("PageTreeRoot"));

	// clear current state
	if(mCatalogInformation.GetCurrentPageTreeNode())
	{
		delete mCatalogInformation.GetPageTreeRoot(mObjectsContext->GetInDirectObjectsRegistry());
		mCatalogInformation.SetCurrentPageTreeNode(NULL);
	}


	if(!pageTreeRootState) // no page nodes yet...
		return;

	PDFObjectCastPtr<PDFIndirectObjectReference> currentPageTreeState(inCatalogInformationState->QueryDirectObject("mCurrentPageTreeNode"));
	mCurrentPageTreeIDInState = currentPageTreeState->mObjectID;

	PDFObjectCastPtr<PDFDictionary> pageTreeState(inStateReader->ParseNewObject(pageTreeRootState->mObjectID));

	PDFObjectCastPtr<PDFInteger> pageTreeIDState(pageTreeState->QueryDirectObject("mPageTreeID"));
	PageTree* rootNode = new PageTree((ObjectIDType)pageTreeIDState->GetValue());

	if(pageTreeRootState->mObjectID == mCurrentPageTreeIDInState)
		mCatalogInformation.SetCurrentPageTreeNode(rootNode);
	ReadPageTreeState(inStateReader,pageTreeState.GetPtr(),rootNode);

}

void DocumentContext::ReadPageTreeState(PDFParser* inStateReader,PDFDictionary* inPageTreeState,PageTree* inPageTree)
{
	PDFObjectCastPtr<PDFBoolean> isLeafParentState(inPageTreeState->QueryDirectObject("mIsLeafParent"));
	bool isLeafParent = isLeafParentState->GetValue();

	if(isLeafParent)
	{
		PDFObjectCastPtr<PDFArray> kidsIDsState(inPageTreeState->QueryDirectObject("mKidsIDs"));
		PDFObjectCastPtr<PDFInteger> kidID;

		SingleValueContainerIterator<PDFObjectVector> it = kidsIDsState->GetIterator();
		while(it.MoveNext())
		{
			kidID = it.GetItem();
			inPageTree->AddNodeToTree((ObjectIDType)kidID->GetValue(),mObjectsContext->GetInDirectObjectsRegistry());
		}
	}
	else
	{
		PDFObjectCastPtr<PDFArray> kidsNodesState(inPageTreeState->QueryDirectObject("mKidsNodes"));

		SingleValueContainerIterator<PDFObjectVector> it = kidsNodesState->GetIterator();
		while(it.MoveNext())
		{
			PDFObjectCastPtr<PDFDictionary> kidNodeState(inStateReader->ParseNewObject(((PDFIndirectObjectReference*)it.GetItem())->mObjectID));

			PDFObjectCastPtr<PDFInteger> pageTreeIDState(kidNodeState->QueryDirectObject("mPageTreeID"));
			PageTree* kidNode = new PageTree((ObjectIDType)pageTreeIDState->GetValue());

			if(((PDFIndirectObjectReference*)it.GetItem())->mObjectID == mCurrentPageTreeIDInState)
				mCatalogInformation.SetCurrentPageTreeNode(kidNode);
			ReadPageTreeState(inStateReader,kidNodeState.GetPtr(),kidNode);

			inPageTree->AddNodeToTree(kidNode,mObjectsContext->GetInDirectObjectsRegistry());
		}
	}
}

PDFDocumentCopyingContext* DocumentContext::CreatePDFCopyingContext(const std::string& inFilePath, const PDFParsingOptions& inOptions)
{
	PDFDocumentCopyingContext* context = new PDFDocumentCopyingContext();

	if(context->Start(inFilePath,this,mObjectsContext,inOptions,mParserExtender) != PDFHummus::eSuccess)
	{
		delete context;
		return NULL;
	}
	else
		return context;
}

EStatusCode DocumentContext::AttachURLLinktoCurrentPage(const std::string& inURL,const PDFRectangle& inLinkClickArea)
{
	EStatusCodeAndObjectIDType writeResult = WriteAnnotationAndLinkForURL(inURL,inLinkClickArea);

	if(writeResult.first != PDFHummus::eSuccess)
		return writeResult.first;

	RegisterAnnotationReferenceForNextPageWrite(writeResult.second);
	return PDFHummus::eSuccess;
}

static const std::string scAnnot = "Annot";
static const std::string scLink = "Link";
static const std::string scRect = "Rect";
static const std::string scF = "F";
static const std::string scW = "W";
static const std::string scA = "A";
static const std::string scBS = "BS";
static const std::string scAction = "Action";
static const std::string scURI = "URI";
EStatusCodeAndObjectIDType DocumentContext::WriteAnnotationAndLinkForURL(const std::string& inURL,const PDFRectangle& inLinkClickArea)
{
	EStatusCodeAndObjectIDType result(PDFHummus::eFailure,0);

	do
	{
		Ascii7Encoding encoding;

		BoolAndString encodedResult = encoding.Encode(inURL);
		if(!encodedResult.first)
		{
			TRACE_LOG1("DocumentContext::WriteAnnotationAndLinkForURL, unable to encode string to Ascii7. make sure that all charachters are valid URLs [should be ascii 7 compatible]. URL - %s",inURL.c_str());
			break;
		}

		result.second = mObjectsContext->StartNewIndirectObject();
		DictionaryContext* linkAnnotationContext = mObjectsContext->StartDictionary();

		// Type
		linkAnnotationContext->WriteKey(scType);
		linkAnnotationContext->WriteNameValue(scAnnot);

		// Subtype
		linkAnnotationContext->WriteKey(scSubType);
		linkAnnotationContext->WriteNameValue(scLink);

		// Rect
		linkAnnotationContext->WriteKey(scRect);
		linkAnnotationContext->WriteRectangleValue(inLinkClickArea);

		// F
		linkAnnotationContext->WriteKey(scF);
		linkAnnotationContext->WriteIntegerValue(4);

		// BS
		linkAnnotationContext->WriteKey(scBS);
		DictionaryContext* borderStyleContext = mObjectsContext->StartDictionary();

		borderStyleContext->WriteKey(scW);
		borderStyleContext->WriteIntegerValue(0);
		mObjectsContext->EndDictionary(borderStyleContext);

		// A
		linkAnnotationContext->WriteKey(scA);
		DictionaryContext* actionContext = mObjectsContext->StartDictionary();

		// Type
		actionContext->WriteKey(scType);
		actionContext->WriteNameValue(scAction);

		// S
		actionContext->WriteKey(scS);
		actionContext->WriteNameValue(scURI);

		// URI
		actionContext->WriteKey(scURI);
		actionContext->WriteLiteralStringValue(encodedResult.second);

		mObjectsContext->EndDictionary(actionContext);

		mObjectsContext->EndDictionary(linkAnnotationContext);
		mObjectsContext->EndIndirectObject();
		result.first = PDFHummus::eSuccess;
	}while(false);

	return result;
}

void DocumentContext::RegisterAnnotationReferenceForNextPageWrite(ObjectIDType inAnnotationReference)
{
	mAnnotations.insert(inAnnotationReference);
}

ObjectIDTypeSet& DocumentContext::GetAnnotations()
{
	return mAnnotations;
}

EStatusCode DocumentContext::MergePDFPagesToPage(PDFPage* inPage,
								const std::string& inPDFFilePath,
								const PDFParsingOptions& inParsingOptions,
								const PDFPageRange& inPageRange,
								const ObjectIDTypeList& inCopyAdditionalObjects)
{
	return mPDFDocumentHandler.MergePDFPagesToPage(inPage,
												   inPDFFilePath,
													inParsingOptions,
													inPageRange,
												   inCopyAdditionalObjects);
}

PDFImageXObject* DocumentContext::CreateImageXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream)
{
	return mJPEGImageHandler.CreateImageXObjectFromJPGStream(inJPGStream);
}

PDFImageXObject* DocumentContext::CreateImageXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream,ObjectIDType inImageXObjectID)
{
	return mJPEGImageHandler.CreateImageXObjectFromJPGStream(inJPGStream,inImageXObjectID);
}

PDFFormXObject* DocumentContext::CreateFormXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream)
{
	return mJPEGImageHandler.CreateFormXObjectFromJPGStream(inJPGStream);

}

PDFFormXObject* DocumentContext::CreateFormXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream,ObjectIDType inFormXObjectID)
{
	return mJPEGImageHandler.CreateFormXObjectFromJPGStream(inJPGStream,inFormXObjectID);
}

EStatusCodeAndObjectIDTypeList DocumentContext::CreateFormXObjectsFromPDF(IByteReaderWithPosition* inPDFStream,
																	const PDFParsingOptions& inParsingOptions,
																	const PDFPageRange& inPageRange,
																	EPDFPageBox inPageBoxToUseAsFormBox,
																	const double* inTransformationMatrix,
																	const ObjectIDTypeList& inCopyAdditionalObjects,
																	const ObjectIDTypeList& inPredefinedFormIDs)
{
	return mPDFDocumentHandler.CreateFormXObjectsFromPDF(inPDFStream, inParsingOptions,inPageRange,inPageBoxToUseAsFormBox,inTransformationMatrix,inCopyAdditionalObjects, inPredefinedFormIDs);
}

EStatusCodeAndObjectIDTypeList DocumentContext::CreateFormXObjectsFromPDF(IByteReaderWithPosition* inPDFStream,
																	const PDFParsingOptions& inParsingOptions,
																	const PDFPageRange& inPageRange,
																	const PDFRectangle& inCropBox,
																	const double* inTransformationMatrix,
																	const ObjectIDTypeList& inCopyAdditionalObjects,
																	const ObjectIDTypeList& inPredefinedFormIDs)
{
	return mPDFDocumentHandler.CreateFormXObjectsFromPDF(inPDFStream,inParsingOptions,inPageRange,inCropBox,inTransformationMatrix,inCopyAdditionalObjects, inPredefinedFormIDs);
}

EStatusCodeAndObjectIDTypeList DocumentContext::AppendPDFPagesFromPDF(IByteReaderWithPosition* inPDFStream,
																const PDFParsingOptions& inParsingOptions,
																const PDFPageRange& inPageRange,
																const ObjectIDTypeList& inCopyAdditionalObjects)
{
	return mPDFDocumentHandler.AppendPDFPagesFromPDF(inPDFStream,inParsingOptions,inPageRange,inCopyAdditionalObjects);
}

EStatusCode DocumentContext::MergePDFPagesToPage(	PDFPage* inPage,
											IByteReaderWithPosition* inPDFStream,
										const PDFParsingOptions& inParsingOptions,
										const PDFPageRange& inPageRange,
											const ObjectIDTypeList& inCopyAdditionalObjects)
{
	return mPDFDocumentHandler.MergePDFPagesToPage(inPage,inPDFStream, inParsingOptions,inPageRange,inCopyAdditionalObjects);
}

PDFDocumentCopyingContext* DocumentContext::CreatePDFCopyingContext(IByteReaderWithPosition* inPDFStream, const PDFParsingOptions& inOptions)
{
	PDFDocumentCopyingContext* context = new PDFDocumentCopyingContext();

	if(context->Start(inPDFStream,this,mObjectsContext, inOptions,mParserExtender) != PDFHummus::eSuccess)
	{
		delete context;
		return NULL;
	}
	else
		return context;
}

void DocumentContext::Cleanup()
{
	// DO NOT NULL MOBJECTSCONTEXT. EVER

	mTrailerInformation.Reset();
	mCatalogInformation.Reset();
	mJPEGImageHandler.Reset();
#ifndef PDFHUMMUS_NO_TIFF
	mTIFFImageHandler.Reset();
#endif
	mUsedFontsRepository.Reset();
	mExtGStateRegistry.Reset();
	mOutputFilePath.clear();
	mExtenders.clear();
	mAnnotations.clear();
    PDFDocumentCopyingContextSet::iterator it = mCopyingContexts.begin();
	for(; it != mCopyingContexts.end(); ++it)
		(*it)->ReleaseDocumentContextReference();
	mCopyingContexts.clear();
    mModifiedDocumentIDExists = false;

    ResourcesDictionaryAndStringToIResourceWritingTaskListMap::iterator itCategories = mResourcesTasks.begin();

    for(; itCategories != mResourcesTasks.end(); ++itCategories)
    {
        IResourceWritingTaskList::iterator itWritingTasks = itCategories->second.begin();
        for(; itWritingTasks != itCategories->second.end(); ++itWritingTasks)
            delete *itWritingTasks;
    }

    mResourcesTasks.clear();

	{
		PDFFormXObjectToIFormEndWritingTaskListMap::iterator itFormEnd = mFormEndTasks.begin();

		for(; itFormEnd != mFormEndTasks.end();++itFormEnd)
		{
			IFormEndWritingTaskList::iterator itEndWritingTasks = itFormEnd->second.begin();
			for(; itEndWritingTasks != itFormEnd->second.end(); ++itEndWritingTasks)
				delete *itEndWritingTasks;

		}
		mFormEndTasks.clear();
	}

	{
		PDFFormXObjectToIObjectEndWritingTaskListMap::iterator itEnd = mMoreFormEndTasks.begin();

		for(; itEnd != mMoreFormEndTasks.end();++itEnd)
		{
			IObjectEndWritingTaskList::iterator itEndWritingTasks = itEnd->second.begin();
			for(; itEndWritingTasks != itEnd->second.end(); ++itEndWritingTasks)
				delete *itEndWritingTasks;

		}
		mMoreFormEndTasks.clear();
	}

	{
		PDFPageToIPageEndWritingTaskListMap::iterator itPageEnd = mPageEndTasks.begin();

		for(; itPageEnd != mPageEndTasks.end();++itPageEnd)
		{
			IPageEndWritingTaskList::iterator itPageEndWritingTasks = itPageEnd->second.begin();
			for(; itPageEndWritingTasks != itPageEnd->second.end(); ++itPageEndWritingTasks)
				delete *itPageEndWritingTasks;

		}
		mPageEndTasks.clear();
	}

	{
		PDFPageToIObjectEndWritingTaskListMap::iterator itEnd = mMorePageEndTasks.begin();

		for(; itEnd != mMorePageEndTasks.end();++itEnd)
		{
			IObjectEndWritingTaskList::iterator itEndWritingTasks = itEnd->second.begin();
			for(; itEndWritingTasks != itEnd->second.end(); ++itEndWritingTasks)
				delete *itEndWritingTasks;

		}
		mMoreFormEndTasks.clear();		
	}


	{
		PDFTiledPatternToITiledPatternEndWritingTaskListMap::iterator itPatternEnd = mTiledPatternEndTasks.begin();

		for (; itPatternEnd != mTiledPatternEndTasks.end(); ++itPatternEnd)
		{
			ITiledPatternEndWritingTaskList::iterator itTiledPatternEndWritingTasks = itPatternEnd->second.begin();
			for (; itTiledPatternEndWritingTasks != itPatternEnd->second.end(); ++itTiledPatternEndWritingTasks)
				delete *itTiledPatternEndWritingTasks;

		}
		mTiledPatternEndTasks.clear();
	}

	{
		PDFTiledPatternToIObjectEndWritingTaskListMap::iterator itEnd = mMoreTiledPatternEndTasks.begin();

		for(; itEnd != mMoreTiledPatternEndTasks.end();++itEnd)
		{
			IObjectEndWritingTaskList::iterator itEndWritingTasks = itEnd->second.begin();
			for(; itEndWritingTasks != itEnd->second.end(); ++itEndWritingTasks)
				delete *itEndWritingTasks;

		}
		mMoreTiledPatternEndTasks.clear();		
	}	
}

void DocumentContext::SetParserExtender(IPDFParserExtender* inParserExtender)
{
	mParserExtender = inParserExtender;
	mPDFDocumentHandler.SetParserExtender(inParserExtender);
}

void DocumentContext::RegisterCopyingContext(PDFDocumentCopyingContext* inCopyingContext)
{
	mCopyingContexts.insert(inCopyingContext);
}

void DocumentContext::UnRegisterCopyingContext(PDFDocumentCopyingContext* inCopyingContext)
{
	mCopyingContexts.erase(inCopyingContext);
}

EStatusCode DocumentContext::SetupModifiedFile(PDFParser* inModifiedFileParser)
{
    // setup trailer and save original document ID

    if(!inModifiedFileParser->GetTrailer())
        return eFailure;

    PDFObjectCastPtr<PDFIndirectObjectReference> rootReference = inModifiedFileParser->GetTrailer()->QueryDirectObject("Root");
    if(!rootReference)
        return eFailure;

    // set catalog reference and previous reference table position
    mTrailerInformation.SetRoot(rootReference->mObjectID);
    mTrailerInformation.SetPrev(inModifiedFileParser->GetXrefPosition());

    // setup modified date to current time
    mTrailerInformation.GetInfo().ModDate.SetToCurrentTime();

    // try to get document ID. in any case use whatever was the original
    mModifiedDocumentIDExists = true;
    mModifiedDocumentID = "";
    PDFObjectCastPtr<PDFArray> idArray = inModifiedFileParser->GetTrailer()->QueryDirectObject("ID");
    if(idArray.GetPtr() && idArray->GetLength() == 2)
    {
        PDFObjectCastPtr<PDFHexString> firstID = idArray->QueryObject(0);
        if(firstID != NULL && firstID.GetPtr())
            mModifiedDocumentID = firstID->GetValue();
    }

    return eSuccess;
}

class ModifiedDocCatalogWriterExtension : public DocumentContextExtenderAdapter
{
public:
	ModifiedDocCatalogWriterExtension(
		PDFDocumentCopyingContext* inCopyingContext,
		bool inRequiredVersionUpdate,
		EPDFVersion inPDFVersion){
		mModifiedDocumentCopyingContext = inCopyingContext;
		mRequiresVersionUpdate = inRequiredVersionUpdate;
		mPDFVersion = inPDFVersion;
	}
    virtual ~ModifiedDocCatalogWriterExtension(){}

    // IDocumentContextExtender implementation
	virtual PDFHummus::EStatusCode OnCatalogWrite(
            CatalogInformation* inCatalogInformation,
            DictionaryContext* inCatalogDictionaryContext,
            ObjectsContext* inPDFWriterObjectContext,
            PDFHummus::DocumentContext* inDocumentContext)
    {

		// update version
		if (mRequiresVersionUpdate) {
			inCatalogDictionaryContext->WriteKey("Version");

			// need to write as /1.4 (name, of float value)
			inCatalogDictionaryContext->WriteNameValue(Double(((double)mPDFVersion) / 10).ToString());
		}

		// now write all info that's not overriden by this implementation
		PDFParser* modifiedDocumentParser = mModifiedDocumentCopyingContext->GetSourceDocumentParser();
		PDFObjectCastPtr<PDFDictionary> catalogDict(modifiedDocumentParser->QueryDictionaryObject(modifiedDocumentParser->GetTrailer(),"Root"));
		MapIterator<PDFNameToPDFObjectMap>  catalogDictIt = catalogDict->GetIterator();

		if (!catalogDict) {
			// no catalog. not cool but possible. call quits
			return eSuccess;
		}

		// copy all elements that were not already written. in other words - overriden
		while (catalogDictIt.MoveNext())
		{
			if (!inCatalogDictionaryContext->HasKey(catalogDictIt.GetKey()->GetValue()))
			{
				inCatalogDictionaryContext->WriteKey(catalogDictIt.GetKey()->GetValue());
				mModifiedDocumentCopyingContext->CopyDirectObjectAsIs(catalogDictIt.GetValue());
			}
		}


        return eSuccess;
    }

private:
	PDFDocumentCopyingContext* mModifiedDocumentCopyingContext;
	bool mRequiresVersionUpdate;
    EPDFVersion mPDFVersion;
};

EStatusCode	DocumentContext::FinalizeModifiedPDF(PDFParser* inModifiedFileParser, EPDFVersion inModifiedPDFVersion)
{
	EStatusCode status;
	LongFilePositionType xrefTablePosition;

	do
	{
		status = WriteUsedFontsDefinitions();
		if(status != eSuccess)
			break;

        // Page tree writing
        // k. page tree needs to be a combination of what pages are coming from the original document
        // and those from the new one. The decision whether a new page tree need to be written is simple -
        // if no pages were added...no new page tree...if yes...then we need a new page tree which will combine
        // the new pages and the old pages

        ObjectReference originalDocumentPageTreeRoot = GetOriginalDocumentPageTreeRoot(inModifiedFileParser);
        bool hasNewPageTreeRoot;
        ObjectReference finalPageRoot;

        if(DocumentHasNewPages())
        {
            if(originalDocumentPageTreeRoot.ObjectID != 0)
            {
                finalPageRoot.ObjectID = WriteCombinedPageTree(inModifiedFileParser);
                finalPageRoot.GenerationNumber = 0;

                // check for error - may fail to write combined page tree if document is protected!
                if(finalPageRoot.ObjectID == 0)
                {
                    status = eFailure;
                    break;
                }
            }
            else
            {
                WritePagesTree();
                PageTree* pageTreeRoot = mCatalogInformation.GetPageTreeRoot(mObjectsContext->GetInDirectObjectsRegistry());
                finalPageRoot.ObjectID = pageTreeRoot->GetID();
                finalPageRoot.GenerationNumber = 0;

            }
            hasNewPageTreeRoot = true;
        }
        else
        {
            hasNewPageTreeRoot = false;
            finalPageRoot = originalDocumentPageTreeRoot;
        }
        // marking if has new page root, cause this effects the decision to have a new catalog

        bool requiresVersionUpdate = IsRequiredVersionHigherThanPDFVersion(inModifiedFileParser,inModifiedPDFVersion);

        if(hasNewPageTreeRoot || requiresVersionUpdate || DoExtendersRequireCatalogUpdate(inModifiedFileParser))
        {
			// use an extender to copy original catalog elements and update version if required
			PDFDocumentCopyingContext* copyingContext = CreatePDFCopyingContext(inModifiedFileParser);
			if(!copyingContext) {
			    status = eFailure;
			    break;
			}
			ModifiedDocCatalogWriterExtension catalogUpdate(copyingContext,requiresVersionUpdate,inModifiedPDFVersion);
            status = WriteCatalogObject(finalPageRoot,&catalogUpdate);
			delete copyingContext;
            if(status != eSuccess)
                break;
        }

 		// write the info dictionary of the trailer, if has any valid entries
		WriteInfoDictionary();

		// write encryption dictionary, if encrypting
		status = CopyEncryptionDictionary(inModifiedFileParser);
        if(status != eSuccess)
            break;
        if(RequiresXrefStream(inModifiedFileParser))
        {
            status = WriteXrefStream(xrefTablePosition);
        }
        else
        {
            status = mObjectsContext->WriteXrefTable(xrefTablePosition);
            if(status != eSuccess)
                break;

            status = WriteTrailerDictionary();
            if(status != eSuccess)
                break;

        }

		WriteXrefReference(xrefTablePosition);
		WriteFinalEOF();
	} while(false);

	return status;
}

ObjectReference DocumentContext::GetOriginalDocumentPageTreeRoot(PDFParser* inModifiedFileParser)
{
	ObjectReference rootObject;


	do
	{
		// get catalogue, verify indirect reference
		PDFObjectCastPtr<PDFIndirectObjectReference> catalogReference(inModifiedFileParser->GetTrailer()->QueryDirectObject("Root"));
		if(!catalogReference)
		{
			TRACE_LOG("DocumentContext::GetOriginalDocumentPageTreeRoot, failed to read catalog reference in trailer");
			break;
		}

		PDFObjectCastPtr<PDFDictionary> catalog(inModifiedFileParser->ParseNewObject(catalogReference->mObjectID));
		if(!catalog)
		{
			TRACE_LOG("DocumentContext::GetOriginalDocumentPageTreeRoot, failed to read catalog");
			break;
		}

		// get pages, verify indirect reference
		PDFObjectCastPtr<PDFIndirectObjectReference> pagesReference(catalog->QueryDirectObject("Pages"));
		if(!pagesReference)
		{
			TRACE_LOG("PDFParser::GetOriginalDocumentPageTreeRoot, failed to read pages reference in catalog");
			break;
		}

		// check if the pages tree is not deleted. this should allow users
		// to override the page tree
		GetObjectWriteInformationResult objectRegistryData = mObjectsContext->GetInDirectObjectsRegistry().GetObjectWriteInformation(pagesReference->mObjectID);
		if (objectRegistryData.first && objectRegistryData.second.mObjectReferenceType == ObjectWriteInformation::Used)
		{
			rootObject.GenerationNumber = pagesReference->mVersion;
			rootObject.ObjectID = pagesReference->mObjectID;
		}

	}while(false);

	return rootObject;

}

bool DocumentContext::DocumentHasNewPages()
{
    // the best way to check if there are new pages created is to check if there's at least one leaf

    if(!mCatalogInformation.GetCurrentPageTreeNode())
        return false;

    // note that page tree root surely exist, so no worries about creating a new one
    PageTree* pageTreeRoot = mCatalogInformation.GetPageTreeRoot(mObjectsContext->GetInDirectObjectsRegistry());

    bool hasLeafs = false;

    while(hasLeafs == false)
    {
        hasLeafs = pageTreeRoot->IsLeafParent();
        if(pageTreeRoot->GetNodesCount() == 0)
            break;
        else {
            pageTreeRoot = pageTreeRoot->GetPageTreeChild(0);
            if (!pageTreeRoot)
                break;
        }
    }

    return hasLeafs;
}

ObjectIDType DocumentContext::WriteCombinedPageTree(PDFParser* inModifiedFileParser)
{
    // writing a combined page tree looks like this
    // first, we allocate a new root object that will contain both new and old pages
    // then, write the new pages tree with reference to the new root object as parent
    // then, write a new pages tree root to represent the old pages tree. this is a copy
    // of the old tree, but with the parent object pointing to the new root object.
    // now write the new root object with allocated ID and the old and new pages trees roots as direct children.
    // happy.


    // allocate new root object
    ObjectIDType newPageRootTreeID = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();

    PageTree* root = new PageTree(newPageRootTreeID);

    // write new pages tree
    PageTree* newPagesTree = mCatalogInformation.GetPageTreeRoot(mObjectsContext->GetInDirectObjectsRegistry());
    newPagesTree->SetParent(root);
 	long long newPagesCount = WritePageTree(newPagesTree);
    newPagesTree->SetParent(NULL);
    delete root;

    // write modified old pages root
    ObjectReference originalTreeRoot = GetOriginalDocumentPageTreeRoot(inModifiedFileParser);

    PDFObjectCastPtr<PDFDictionary> originalTreeRootObject = inModifiedFileParser->ParseNewObject(originalTreeRoot.ObjectID);

    mObjectsContext->StartModifiedIndirectObject(originalTreeRoot.ObjectID);

    DictionaryContext* pagesTreeContext = mObjectsContext->StartDictionary();

    PDFObjectCastPtr<PDFInteger> kidsCount = originalTreeRootObject->QueryDirectObject(scCount);
    long long originalPageTreeKidsCount = kidsCount.GetPtr() ? kidsCount->GetValue() : 0;

    // copy all but parent key. then add parent as the new root object

    MapIterator<PDFNameToPDFObjectMap>  pageTreeIt = originalTreeRootObject->GetIterator();
    PDFDocumentCopyingContext aCopyingContext;

    EStatusCode status = aCopyingContext.Start(inModifiedFileParser,this,mObjectsContext);

    do {

        if(status != eSuccess)
        {
            TRACE_LOG("DocumentContext::WriteCombinedPageTree, Unable to copy original page tree. this probably means that the original file is protected - and is therefore unsupported for such activity as adding pages");
            break;
        }

        while(pageTreeIt.MoveNext())
        {
            if(pageTreeIt.GetKey()->GetValue() != "Parent")
            {
                pagesTreeContext->WriteKey(pageTreeIt.GetKey()->GetValue());
                aCopyingContext.CopyDirectObjectAsIs(pageTreeIt.GetValue());
            }

        }

        aCopyingContext.End();

        // parent
        pagesTreeContext->WriteKey(scParent);
        pagesTreeContext->WriteNewObjectReferenceValue(newPageRootTreeID);

        mObjectsContext->EndDictionary(pagesTreeContext);
        mObjectsContext->EndIndirectObject();

        // now write the root page tree. 2 kids, the original pages, and new pages
        mObjectsContext->StartNewIndirectObject(newPageRootTreeID);

        pagesTreeContext = mObjectsContext->StartDictionary();

        // type
        pagesTreeContext->WriteKey(scType);
        pagesTreeContext->WriteNameValue(scPages);

        // count
        pagesTreeContext->WriteKey(scCount);
        pagesTreeContext->WriteIntegerValue(originalPageTreeKidsCount + newPagesCount);

        // kids
        pagesTreeContext->WriteKey(scKids);
        mObjectsContext->StartArray();

        mObjectsContext->WriteIndirectObjectReference(originalTreeRoot);
        mObjectsContext->WriteNewIndirectObjectReference(newPagesTree->GetID());

        mObjectsContext->EndArray();
        mObjectsContext->EndLine();

        mObjectsContext->EndDictionary(pagesTreeContext);
        mObjectsContext->EndIndirectObject();

    } while (false);



    if(status == eSuccess)
        return newPageRootTreeID;
    else
        return 0;
}

bool DocumentContext::IsRequiredVersionHigherThanPDFVersion(PDFParser* inModifiedFileParser,EPDFVersion inModifiedPDFVersion)
{
    return (EPDFVersion)((size_t)(inModifiedFileParser->GetPDFLevel() * 10)) < inModifiedPDFVersion;
}

bool DocumentContext::DoExtendersRequireCatalogUpdate(PDFParser* inModifiedFileParser)
{
    bool isUpdateRequired = false;

 	IDocumentContextExtenderSet::iterator it = mExtenders.begin();
	for(; it != mExtenders.end() && !isUpdateRequired; ++it)
		isUpdateRequired = (*it)->IsCatalogUpdateRequiredForModifiedFile(inModifiedFileParser);

    return isUpdateRequired;
}

EStatusCode DocumentContext::CopyEncryptionDictionary(PDFParser* inModifiedFileParser)
{
	// Reuse original encryption dict for new modified trailer. for sake of simplicity (with trailer using ref for encrypt), make it indirect if not already
	RefCountPtr<PDFObject> encrypt(inModifiedFileParser->GetTrailer()->QueryDirectObject("Encrypt"));
	if (encrypt.GetPtr() == NULL)
		return eSuccess;

	if (encrypt->GetType() == PDFObject::ePDFObjectIndirectObjectReference)
	{
		// just set the reference to the object
		mTrailerInformation.SetEncrypt(((PDFIndirectObjectReference*)encrypt.GetPtr())->mObjectID);
	}
	else
	{
		// copying context, write as is
		PDFDocumentCopyingContext* copyingContext = CreatePDFCopyingContext(inModifiedFileParser);
		if(!copyingContext) {
			return eFailure;
		}
		// copy to indirect object and set refrence
		mEncryptionHelper.PauseEncryption();
		ObjectIDType encryptionDictionaryID = mObjectsContext->StartNewIndirectObject();

		copyingContext->CopyDirectObjectAsIs(encrypt.GetPtr());
		delete copyingContext;
		mObjectsContext->EndIndirectObject();
		mEncryptionHelper.ReleaseEncryption();

		mTrailerInformation.SetEncrypt(encryptionDictionaryID);
	}
	return eSuccess;
}

bool DocumentContext::RequiresXrefStream(PDFParser* inModifiedFileParser)
{
    // modification requires xref stream if the original document uses one...so just ask trailer
    if(!inModifiedFileParser->GetTrailer())
        return false;

    PDFObjectCastPtr<PDFName> typeObject = inModifiedFileParser->GetTrailer()->QueryDirectObject("Type");

    if(!typeObject)
        return false;

    return typeObject->GetValue() == "XRef";


}

EStatusCode DocumentContext::WriteXrefStream(LongFilePositionType& outXrefPosition)
{
    EStatusCode status = eSuccess;

    do
    {
		mEncryptionHelper.PauseEncryption(); // don't encrypt while writing xref stream
        // get the position by accessing the free context of the underlying objects stream

        // an Xref stream is a beast that is both trailer and the xref
        // start the xref with a dictionary detailing the trailer information, then move to the
        // xref table aspects, with the lower level objects context.

        outXrefPosition = mObjectsContext->GetCurrentPosition();
        mObjectsContext->StartNewIndirectObject();

        DictionaryContext* xrefDictionary = mObjectsContext->StartDictionary();

        xrefDictionary->WriteKey("Type");
        xrefDictionary->WriteNameValue("XRef");

        status = WriteTrailerDictionaryValues(xrefDictionary);
        if(status != eSuccess)
            break;

        // k. now for the xref table itself
        status = mObjectsContext->WriteXrefStream(xrefDictionary);

		mEncryptionHelper.ReleaseEncryption();

    }
    while (false);

    return status;
}

PDFDocumentCopyingContext* DocumentContext::CreatePDFCopyingContext(PDFParser* inPDFParser)
{
	PDFDocumentCopyingContext* context = new PDFDocumentCopyingContext();

	if(context->Start(inPDFParser,this,mObjectsContext) != PDFHummus::eSuccess)
	{
		delete context;
		return NULL;
	}
	else
		return context;
}


std::string DocumentContext::AddExtendedResourceMapping(PDFPage* inPage,
                                  const std::string& inResourceCategoryName,
                                  IResourceWritingTask* inWritingTask)
{
    return AddExtendedResourceMapping(&inPage->GetResourcesDictionary(),inResourceCategoryName,inWritingTask);
}

std::string DocumentContext::AddExtendedResourceMapping(PDFTiledPattern* inPattern,
	const std::string& inResourceCategoryName,
	IResourceWritingTask* inWritingTask)
{
	return AddExtendedResourceMapping(&inPattern->GetResourcesDictionary(), inResourceCategoryName, inWritingTask);
}

std::string DocumentContext::AddExtendedResourceMapping(ResourcesDictionary* inResourceDictionary,
                                  const std::string& inResourceCategoryName,
                                  IResourceWritingTask* inWritingTask)
{
    // do two things. first is to include this writing task as part of the tasks to write
    // second is to allocate a name for this resource from the resource category in the relevant dictionary

    ResourcesDictionaryAndStringToIResourceWritingTaskListMap::iterator it =
    mResourcesTasks.find(ResourcesDictionaryAndString(inResourceDictionary,inResourceCategoryName));

    if(it == mResourcesTasks.end())
    {
        it =mResourcesTasks.insert(
                                       ResourcesDictionaryAndStringToIResourceWritingTaskListMap::value_type(
                                                                                                             ResourcesDictionaryAndString(inResourceDictionary,inResourceCategoryName),
                                                                                                             IResourceWritingTaskList())).first;
    }

    it->second.push_back(inWritingTask);

    std::string newResourceName;

    if(inResourceCategoryName == scXObjects)
        newResourceName = inResourceDictionary->AddXObjectMapping(0);
    else if(inResourceCategoryName == scExtGStates)
        newResourceName = inResourceDictionary->AddExtGStateMapping(0);
    else if(inResourceCategoryName == scFonts)
        newResourceName = inResourceDictionary->AddFontMapping(0);
    else if(inResourceCategoryName == scColorSpaces)
        newResourceName = inResourceDictionary->AddColorSpaceMapping(0);
    else if(inResourceCategoryName == scPatterns)
        newResourceName = inResourceDictionary->AddPatternMapping(0);
    else if(inResourceCategoryName == scShadings)
        newResourceName = inResourceDictionary->AddShadingMapping(0);
    else if(inResourceCategoryName == scProperties)
        newResourceName = inResourceDictionary->AddPropertyMapping(0);
    else {
        TRACE_LOG1("DocumentContext::AddExtendedResourceMapping:, unidentified category for registering a resource writer %s",inResourceCategoryName.c_str());
    }
    return newResourceName;
}

std::string DocumentContext::AddExtendedResourceMapping(PDFFormXObject* inFormXObject,
                                                   const std::string& inResourceCategoryName,
                                                   IResourceWritingTask* inWritingTask)
{
    return AddExtendedResourceMapping(&inFormXObject->GetResourcesDictionary(),inResourceCategoryName,inWritingTask);
}

void DocumentContext::RegisterFormEndWritingTask(PDFFormXObject* inFormXObject,IFormEndWritingTask* inWritingTask)
{
    PDFFormXObjectToIFormEndWritingTaskListMap::iterator it =
    mFormEndTasks.find(inFormXObject);

    if(it == mFormEndTasks.end())
    {
        it =mFormEndTasks.insert(PDFFormXObjectToIFormEndWritingTaskListMap::value_type(inFormXObject,IFormEndWritingTaskList())).first;
    }

    it->second.push_back(inWritingTask);
}

void DocumentContext::RegisterFormEndWritingTask(PDFFormXObject* inFormXObject,IObjectEndWritingTask* inWritingTask)
{
    PDFFormXObjectToIObjectEndWritingTaskListMap::iterator it =
    mMoreFormEndTasks.find(inFormXObject);

    if(it == mMoreFormEndTasks.end())
    {
        it = mMoreFormEndTasks.insert(PDFFormXObjectToIObjectEndWritingTaskListMap::value_type(inFormXObject,IObjectEndWritingTaskList())).first;
    }

    it->second.push_back(inWritingTask);
}

void DocumentContext::RegisterPageEndWritingTask(PDFPage* inPage,IPageEndWritingTask* inWritingTask)
{
    PDFPageToIPageEndWritingTaskListMap::iterator it =
    mPageEndTasks.find(inPage);

    if(it == mPageEndTasks.end())
    {
        it =mPageEndTasks.insert(PDFPageToIPageEndWritingTaskListMap::value_type(inPage,IPageEndWritingTaskList())).first;
    }

    it->second.push_back(inWritingTask);
}

void DocumentContext::RegisterPageEndWritingTask(PDFPage* inPage,IObjectEndWritingTask* inWritingTask)
{
    PDFPageToIObjectEndWritingTaskListMap::iterator it =
    mMorePageEndTasks.find(inPage);

    if(it == mMorePageEndTasks.end())
    {
        it = mMorePageEndTasks.insert(PDFPageToIObjectEndWritingTaskListMap::value_type(inPage,IObjectEndWritingTaskList())).first;
    }

    it->second.push_back(inWritingTask);
}

void DocumentContext::RegisterTiledPatternEndWritingTask(PDFTiledPattern* inPattern, ITiledPatternEndWritingTask* inWritingTask)
{
	PDFTiledPatternToITiledPatternEndWritingTaskListMap::iterator it =
		mTiledPatternEndTasks.find(inPattern);

	if (it == mTiledPatternEndTasks.end())
	{
		it = mTiledPatternEndTasks.insert(PDFTiledPatternToITiledPatternEndWritingTaskListMap::value_type(inPattern, ITiledPatternEndWritingTaskList())).first;
	}

	it->second.push_back(inWritingTask);
}

void DocumentContext::RegisterTiledPatternEndWritingTask(PDFTiledPattern* inPattern, IObjectEndWritingTask* inWritingTask)
{
    PDFTiledPatternToIObjectEndWritingTaskListMap::iterator it =
    mMoreTiledPatternEndTasks.find(inPattern);

    if(it == mMoreTiledPatternEndTasks.end())
    {
        it = mMoreTiledPatternEndTasks.insert(PDFTiledPatternToIObjectEndWritingTaskListMap::value_type(inPattern,IObjectEndWritingTaskList())).first;
    }

    it->second.push_back(inWritingTask);
}

DoubleAndDoublePair DocumentContext::GetImageDimensions(
	IByteReaderWithPosition* inImageStream,
	unsigned long inImageIndex,
	const PDFParsingOptions& inOptions)
{
  double imageWidth = 0.0;
	double imageHeight = 0.0;

  LongFilePositionType recordedPosition = inImageStream->GetCurrentPosition();

	EHummusImageType imageType = GetImageType(inImageStream,inImageIndex);

  switch(imageType)
  {
    case ePDF:
    {
      // get the dimensions via the PDF parser. will use the media rectangle to draw image
      PDFParser pdfParser;

      if(pdfParser.StartPDFParsing(inImageStream, inOptions) != eSuccess)
        break;

      PDFPageInput helper(&pdfParser,pdfParser.ParsePage(inImageIndex));

      imageWidth = helper.GetMediaBox().GetWidth();
      imageHeight = helper.GetMediaBox().GetHeight();

      break;
    }
    case eJPG:
    {
      BoolAndJPEGImageInformation jpgImageInformation = GetJPEGImageHandler().RetrieveImageInformation(inImageStream);
      if(!jpgImageInformation.first)
        break;

      DoubleAndDoublePair dimensions = GetJPEGImageHandler().GetImageDimensions(jpgImageInformation.second);

      imageWidth = dimensions.first;
      imageHeight = dimensions.second;
      break;
    }
#ifndef PDFHUMMUS_NO_TIFF
    case eTIFF:
    {
      TIFFImageHandler hummusTiffHandler;

      DoubleAndDoublePair dimensions = hummusTiffHandler.ReadImageDimensions(inImageStream,inImageIndex);

      imageWidth = dimensions.first;
      imageHeight = dimensions.second;
      break;
    }
#endif
#ifndef PDFHUMMUS_NO_PNG
    case ePNG:
    {
      PNGImageHandler hummusPngHandler;

      DoubleAndDoublePair dimensions = hummusPngHandler.ReadImageDimensions(inImageStream);

      imageWidth = dimensions.first;
      imageHeight = dimensions.second;
      break;
    }
#endif
    default:
    {
      // just avoding uninteresting compiler warnings. meaning...if you can't get the image type or unsupported, do nothing
    }
  }

  // restore stream position to initial state
  inImageStream->SetPosition(recordedPosition);

  return DoubleAndDoublePair(imageWidth,imageHeight);
}

DoubleAndDoublePair DocumentContext::GetImageDimensions(
	const std::string& inImageFile,
	unsigned long inImageIndex,
	const PDFParsingOptions& inOptions)
{
    HummusImageInformation& imageInformation = GetImageInformationStructFor(inImageFile,inImageIndex);

	if(imageInformation.imageHeight == -1 || imageInformation.imageWidth == -1)
	{

		double imageWidth = 0.0;
		double imageHeight = 0.0;

		EHummusImageType imageType = GetImageType(inImageFile,inImageIndex);

		switch(imageType)
		{
			case ePDF:
			{
				// get the dimensions via the PDF parser. will use the media rectangle to draw image
				PDFParser pdfParser;

				InputFile file;
				if(file.OpenFile(inImageFile) != eSuccess)
					break;
				if(pdfParser.StartPDFParsing(file.GetInputStream(), inOptions) != eSuccess)
					break;

				PDFPageInput helper(&pdfParser,pdfParser.ParsePage(inImageIndex));

				imageWidth = helper.GetMediaBox().GetHeight();
				imageHeight = helper.GetMediaBox().GetWidth();

				break;
			}
			case eJPG:
			{
				BoolAndJPEGImageInformation jpgImageInformation = GetJPEGImageHandler().RetrieveImageInformation(inImageFile);
				if(!jpgImageInformation.first)
					break;

				DoubleAndDoublePair dimensions = GetJPEGImageHandler().GetImageDimensions(jpgImageInformation.second);

				imageWidth = dimensions.first;
				imageHeight = dimensions.second;
				break;
			}
#ifndef PDFHUMMUS_NO_TIFF
			case eTIFF:
			{
				TIFFImageHandler hummusTiffHandler;

				InputFile file;
				if(file.OpenFile(inImageFile) != eSuccess)
				{
					break;
				}

				DoubleAndDoublePair dimensions = hummusTiffHandler.ReadImageDimensions(file.GetInputStream(),inImageIndex);

				imageWidth = dimensions.first;
				imageHeight = dimensions.second;
				break;
			}
#endif
#ifndef PDFHUMMUS_NO_PNG
			case ePNG:
			{
				PNGImageHandler hummusPngHandler;

				InputFile file;
				if (file.OpenFile(inImageFile) != eSuccess)
				{
					break;
				}

				DoubleAndDoublePair dimensions = hummusPngHandler.ReadImageDimensions(file.GetInputStream());

				imageWidth = dimensions.first;
				imageHeight = dimensions.second;
				break;
			}
#endif
			default:
			{
				// just avoding uninteresting compiler warnings. meaning...if you can't get the image type or unsupported, do nothing
			}
		}

		imageInformation.imageHeight = imageHeight;
		imageInformation.imageWidth = imageWidth;
	}

    return DoubleAndDoublePair(imageInformation.imageWidth,imageInformation.imageHeight);
}

static const Byte scPDFMagic[] = {0x25,0x50,0x44,0x46};
static const Byte scMagicJPG[] = {0xFF,0xD8};
static const Byte scMagicTIFFBigEndianTiff[] = {0x4D,0x4D,0x00,0x2A};
static const Byte scMagicTIFFBigEndianBigTiff[] = {0x4D,0x4D,0x00,0x2B};
static const Byte scMagicTIFFLittleEndianTiff[] = {0x49,0x49,0x2A,0x00};
static const Byte scMagicTIFFLittleEndianBigTiff[] = {0x49,0x49,0x2B,0x00};
static const Byte scMagicPng[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };

PDFHummus::EHummusImageType DocumentContext::GetImageType(IByteReaderWithPosition* inImageStream,unsigned long inImageIndex)
{
	// The types of images that are discovered here are those familiar to Hummus - JPG, TIFF and PDF.
	// PDF is recognized by starting with "%PDF"
	// JPG will start with "0xff,0xd8"
	// TIFF will start with "0x49,0x49" (little endian) or "0x4D,0x4D" (big endian)
	// then either 42 or 43 (tiff or big tiff respectively) written in 2 bytes, as either big or little endian
	// PNG will start with 89 50 4e 47 0d 0a 1a 0a

	// so just read the first 8 bytes and it should be enough to recognize a known format

	Byte magic[8];
	unsigned long readLength = 8;
	PDFHummus::EHummusImageType imageType;

  LongFilePositionType recordedPosition = inImageStream->GetCurrentPosition();

  inImageStream->Read(magic,readLength);

	if(readLength >= 4 && memcmp(scPDFMagic,magic,4) == 0)
		imageType =  PDFHummus::ePDF;
	else if(readLength >= 2 && memcmp(scMagicJPG,magic,2) == 0)
		imageType = PDFHummus::eJPG;
	else if(readLength >= 4 && memcmp(scMagicTIFFBigEndianTiff,magic,4) == 0)
		imageType = PDFHummus::eTIFF;
	else if(readLength >= 4 && memcmp(scMagicTIFFBigEndianBigTiff,magic,4) == 0)
		imageType = PDFHummus::eTIFF;
	else if(readLength >= 4 && memcmp(scMagicTIFFLittleEndianTiff,magic,4) == 0)
		imageType = PDFHummus::eTIFF;
	else if(readLength >= 4 && memcmp(scMagicTIFFLittleEndianBigTiff,magic,4) == 0)
		imageType = PDFHummus::eTIFF;
	else if (readLength >= 8 && memcmp(scMagicPng, magic, 8) == 0)
		imageType = PDFHummus::ePNG;
	else
		imageType = PDFHummus::eUndefined;

  inImageStream->SetPosition(recordedPosition);

  return imageType;
}

PDFHummus::EHummusImageType DocumentContext::GetImageType(const std::string& inImageFile,unsigned long inImageIndex)
{

    HummusImageInformation& imageInformation = GetImageInformationStructFor(inImageFile,inImageIndex);

	if(imageInformation.imageType == eUndefined)
	{

		// The types of images that are discovered here are those familiar to Hummus - JPG, TIFF and PDF
		// PDF is recognized by starting with "%PDF"
		// JPG will start with "0xff,0xd8"
		// TIFF will start with "0x49,0x49" (little endian) or "0x4D,0x4D" (big endian)
		// then either 42 or 43 (tiff or big tiff respectively) written in 2 bytes, as either big or little endian
		// PNG will start with 89 50 4e 47 0d 0a 1a 0a

		// so just read the first 8 bytes and it should be enough to recognize a known format

		Byte magic[8];
		unsigned long readLength = 8;
		InputFile inputFile;
		PDFHummus::EHummusImageType imageType;
		if(inputFile.OpenFile(inImageFile) == eSuccess)
		{
			inputFile.GetInputStream()->Read(magic,readLength);

			if(readLength >= 4 && memcmp(scPDFMagic,magic,4) == 0)
				imageType =  PDFHummus::ePDF;
			else if(readLength >= 2 && memcmp(scMagicJPG,magic,2) == 0)
				imageType = PDFHummus::eJPG;
			else if(readLength >= 4 && memcmp(scMagicTIFFBigEndianTiff,magic,4) == 0)
				imageType = PDFHummus::eTIFF;
			else if(readLength >= 4 && memcmp(scMagicTIFFBigEndianBigTiff,magic,4) == 0)
				imageType = PDFHummus::eTIFF;
			else if(readLength >= 4 && memcmp(scMagicTIFFLittleEndianTiff,magic,4) == 0)
				imageType = PDFHummus::eTIFF;
			else if(readLength >= 4 && memcmp(scMagicTIFFLittleEndianBigTiff,magic,4) == 0)
				imageType = PDFHummus::eTIFF;
			else if (readLength >= 8 && memcmp(scMagicPng, magic, 8) == 0)
				imageType = PDFHummus::ePNG;
			else
				imageType = PDFHummus::eUndefined;
		}
		else
			imageType = PDFHummus::eUndefined;

		imageInformation.imageType = imageType;
	}

    return imageInformation.imageType;
}

unsigned long DocumentContext::GetImagePagesCount(
	const std::string& inImageFile,
	const PDFParsingOptions& inOptions)
{
	unsigned long result = 0;

	EHummusImageType imageType = GetImageType(inImageFile,0);

	switch (imageType)
	{
		case ePDF:
		{
			// get the dimensions via the PDF parser. will use the media rectangle to draw image
			PDFParser pdfParser;

			InputFile file;
			if (file.OpenFile(inImageFile) != eSuccess)
				break;
			if (pdfParser.StartPDFParsing(file.GetInputStream(), inOptions) != eSuccess)
				break;

			result = pdfParser.GetPagesCount();
			break;
		}
#ifndef PDFHUMMUS_NO_PNG
		case ePNG:
#endif
		case eJPG:
		{
			result = 1;
		}
#ifndef PDFHUMMUS_NO_TIFF
		case eTIFF:
		{
			TIFFImageHandler hummusTiffHandler;

			InputFile file;
			if (file.OpenFile(inImageFile) != eSuccess)
				break;

			result = hummusTiffHandler.ReadImagePageCount(file.GetInputStream());
			break;
		}
#endif
		default:
		{
				   // just avoding uninteresting compiler warnings. meaning...if you can't get the image type or unsupported, do nothing
		}
	}

	return result;

}

EStatusCode DocumentContext::WriteFormForImage(
	const std::string& inImagePath,
	unsigned long inImageIndex,
	ObjectIDType inObjectID,
	const PDFParsingOptions& inParsingOptions)
{
    EStatusCode status = eFailure;
    EHummusImageType imageType = GetImageType(inImagePath,inImageIndex);

    switch(imageType)
    {
        case ePDF:
        {
			PDFPageRange singlePageRange;
			singlePageRange.mType = PDFPageRange::eRangeTypeSpecific;
			singlePageRange.mSpecificRanges.push_back(ULongAndULong(inImageIndex, inImageIndex));

			status = CreateFormXObjectsFromPDF(inImagePath, inParsingOptions, singlePageRange, ePDFPageBoxMediaBox, NULL, ObjectIDTypeList(), ObjectIDTypeList(1, inObjectID)).first;
            break;
        }
        case eJPG:
        {
            PDFFormXObject* form = CreateFormXObjectFromJPGFile(inImagePath,inObjectID);
            status = (form ? eSuccess:eFailure);
            delete form;
            break;
        }
#ifndef PDFHUMMUS_NO_TIFF
		case eTIFF:
        {
            TIFFUsageParameters params;
            params.PageIndex = (unsigned int)inImageIndex;

            PDFFormXObject* form = CreateFormXObjectFromTIFFFile(inImagePath,inObjectID,params);
            status = (form ? eSuccess:eFailure);
            delete form;
            break;
        }
#endif
#ifndef PDFHUMMUS_NO_PNG
		case ePNG:
		{
			InputFile inputFile;
			if (inputFile.OpenFile(inImagePath) != eSuccess) {
				break;
			}
			PDFFormXObject* form = CreateFormXObjectFromPNGStream(inputFile.GetInputStream(), inObjectID);
			status = (form ? eSuccess : eFailure);
			delete form;
			break;
		}

		default:
        {
            status = eFailure;
        }
#endif
	}
    return status;
}

HummusImageInformation& DocumentContext::GetImageInformationStructFor(const std::string& inImageFile,unsigned long inImageIndex)
{
    StringAndULongPairToHummusImageInformationMap::iterator it = mImagesInformation.find(StringAndULongPair(inImageFile,inImageIndex));

    if(it == mImagesInformation.end())
        it = mImagesInformation.insert(
                        StringAndULongPairToHummusImageInformationMap::value_type(
                                StringAndULongPair(inImageFile,inImageIndex),HummusImageInformation())).first;

    return it->second;
}


ObjectIDTypeAndBool DocumentContext::RegisterImageForDrawing(const std::string& inImageFile,unsigned long inImageIndex)
{
    HummusImageInformation& imageInformation = GetImageInformationStructFor(inImageFile,inImageIndex);
    bool firstTime;

    if(imageInformation.writtenObjectID == 0)
    {
        imageInformation.writtenObjectID = mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
        firstTime = true;
    }
    else
        firstTime = false;

    return ObjectIDTypeAndBool(imageInformation.writtenObjectID,firstTime);
}


ExtGStateRegistry& DocumentContext::GetExtGStateRegistry() {
	return mExtGStateRegistry;
}

ObjectsContext* DocumentContext::GetObjectsContext() {
	return mObjectsContext;
}