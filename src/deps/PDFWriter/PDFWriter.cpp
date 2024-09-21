/*
   Source File : PDFWriter.cpp


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

#include "PDFWriter.h"
#include "Trace.h"
#include "Singleton.h"
#include "StateWriter.h"
#include "StateReader.h"
#include "ObjectsContext.h"
#include "DictionaryContext.h"
#include "PDFDictionary.h"
#include "PDFObjectCast.h"
#include "PDFIndirectObjectReference.h"
#include "IByteWriterWithPosition.h"
#include "OutputStreamTraits.h"
#include "PDFBoolean.h"
#include "PDFInteger.h"
#include "PDFPageInput.h"
#include "PDFDocumentCopyingContext.h"

using namespace PDFHummus;

const LogConfiguration& LogConfiguration::DefaultLogConfiguration(){
	static LogConfiguration default_log_configuration(false, false, "PDFWriterLog.txt");
	return default_log_configuration;
}

PDFWriter::PDFWriter(void)
{
	// Gal: some trick, already set reference to objects context. this does not mean that the document context can start doing something.
	// this allows the creation of copying context before starting to write the PDF, so that already
	// the first decision (level) about the PDF can be the result of parsing
	mDocumentContext.SetObjectsContext(&mObjectsContext);
    mIsModified = false;
}

PDFWriter::~PDFWriter(void)
{
}

EPDFVersion thisOrDefaultVersion(EPDFVersion inPDFVersion) {
	return ePDFVersionUndefined == inPDFVersion ? ePDFVersion14 : inPDFVersion;
}

EStatusCode PDFWriter::StartPDF(
							const std::string& inOutputFilePath,
							EPDFVersion inPDFVersion,
							const LogConfiguration& inLogConfiguration,
							const PDFCreationSettings& inPDFCreationSettings)
{
	SetupLog(inLogConfiguration);
	SetupCreationSettings(inPDFCreationSettings);

	EStatusCode status = mOutputFile.OpenFile(inOutputFilePath);
	if(status != eSuccess)
		return status;

	mObjectsContext.SetOutputStream(mOutputFile.GetOutputStream());
	mDocumentContext.SetOutputFileInformation(&mOutputFile);

	if (inPDFCreationSettings.DocumentEncryptionOptions.ShouldEncrypt) {
		mDocumentContext.SetupEncryption(inPDFCreationSettings.DocumentEncryptionOptions, thisOrDefaultVersion(inPDFVersion));
		if (!mDocumentContext.SupportsEncryption()) {
			mOutputFile.CloseFile(); // close the file, to keep things clean
			return eFailure;
		}
	}

	mIsModified = false;

	return mDocumentContext.WriteHeader(thisOrDefaultVersion(inPDFVersion));
}

EStatusCode PDFWriter::EndPDF()
{

	EStatusCode status;
	do
	{
        if(mIsModified)
            status = mDocumentContext.FinalizeModifiedPDF(&mModifiedFileParser,mModifiedFileVersion);
        else
            status = mDocumentContext.FinalizeNewPDF();
		if(status != eSuccess)
		{
			TRACE_LOG("PDFWriter::EndPDF, Could not end PDF");
			break;
		}
		status = mOutputFile.CloseFile();
        if(status != eSuccess)
        {
            TRACE_LOG("PDFWriter::EndPDF, Could not close output file");
            break;

        }
        mModifiedFileParser.ResetParser();
        status = mModifiedFile.CloseFile();
	}
	while(false);

    if(status != eSuccess)
    {
        mOutputFile.CloseFile();
        mModifiedFileParser.ResetParser();
        mModifiedFile.CloseFile();
    }
	Cleanup();
	return status;
}

void PDFWriter::Cleanup()
{
	mObjectsContext.Cleanup();
	mDocumentContext.Cleanup();
	//ReleaseLog();
}

void PDFWriter::Reset()
{
	mOutputFile.CloseFile();
    mModifiedFileParser.ResetParser();
    mModifiedFile.CloseFile();
	Cleanup();
}

EStatusCodeAndObjectIDType PDFWriter::WritePageAndReturnPageID(PDFPage* inPage)
{
	return mDocumentContext.WritePage(inPage);
}

EStatusCodeAndObjectIDType PDFWriter::WritePageReleaseAndReturnPageID(PDFPage* inPage)
{
	return mDocumentContext.WritePageAndRelease(inPage);
}

EStatusCode PDFWriter::WritePage(PDFPage* inPage)
{
	return mDocumentContext.WritePage(inPage).first;
}

EStatusCode PDFWriter::WritePageAndRelease(PDFPage* inPage)
{
	return mDocumentContext.WritePageAndRelease(inPage).first;
}


void PDFWriter::SetupLog(const LogConfiguration& inLogConfiguration)
{
	if(inLogConfiguration.LogStream)
		Trace::DefaultTrace().SetLogSettings(inLogConfiguration.LogStream,inLogConfiguration.ShouldLog);
	else
		Trace::DefaultTrace().SetLogSettings(inLogConfiguration.LogFileLocation,inLogConfiguration.ShouldLog,inLogConfiguration.StartWithBOM);
}

void PDFWriter::SetupCreationSettings(const PDFCreationSettings& inPDFCreationSettings)
{
	mObjectsContext.SetCompressStreams(inPDFCreationSettings.CompressStreams);
	mDocumentContext.SetEmbedFonts(inPDFCreationSettings.EmbedFonts);
}

void PDFWriter::ReleaseLog()
{
	//Singleton<Trace>::Reset();
}

DocumentContext& PDFWriter::GetDocumentContext()
{
	return mDocumentContext;
}

ObjectsContext& PDFWriter::GetObjectsContext()
{
	return mObjectsContext;
}

OutputFile& PDFWriter::GetOutputFile()
{
	return mOutputFile;
}

PageContentContext* PDFWriter::StartPageContentContext(PDFPage* inPage)
{
	return mDocumentContext.StartPageContentContext(inPage);
}

EStatusCode PDFWriter::PausePageContentContext(PageContentContext* inPageContext)
{
	return mDocumentContext.PausePageContentContext(inPageContext);
}

EStatusCode PDFWriter::EndPageContentContext(PageContentContext* inPageContext)
{
	return mDocumentContext.EndPageContentContext(inPageContext);
}

PDFFormXObject* PDFWriter::StartFormXObject(const PDFRectangle& inBoundingBox,const double* inMatrix)
{
	return mDocumentContext.StartFormXObject(inBoundingBox,inMatrix);
}

PDFFormXObject* PDFWriter::StartFormXObject(const PDFRectangle& inBoundingBox,ObjectIDType inFormXObjectID,const double* inMatrix)
{
	return mDocumentContext.StartFormXObject(inBoundingBox,inFormXObjectID,inMatrix);
}

EStatusCode PDFWriter::EndFormXObject(PDFFormXObject* inFormXObject)
{
	return mDocumentContext.EndFormXObject(inFormXObject);
}

EStatusCode PDFWriter::EndFormXObjectAndRelease(PDFFormXObject* inFormXObject)
{
	return mDocumentContext.EndFormXObjectAndRelease(inFormXObject);
}

PDFImageXObject* PDFWriter::CreateImageXObjectFromJPGFile(const std::string& inJPGFilePath)
{
	return mDocumentContext.CreateImageXObjectFromJPGFile(inJPGFilePath);
}

PDFFormXObject* PDFWriter::CreateFormXObjectFromJPGFile(const std::string& inJPGFilePath)
{
	return mDocumentContext.CreateFormXObjectFromJPGFile(inJPGFilePath);
}

#ifndef PDFHUMMUS_NO_TIFF
PDFFormXObject* PDFWriter::CreateFormXObjectFromTIFFFile(const std::string& inTIFFFilePath,const TIFFUsageParameters& inTIFFUsageParameters)
{
	return mDocumentContext.CreateFormXObjectFromTIFFFile(inTIFFFilePath,inTIFFUsageParameters);
}

PDFFormXObject* PDFWriter::CreateFormXObjectFromTIFFFile(const std::string& inTIFFFilePath,ObjectIDType inFormXObjectID, const TIFFUsageParameters& inTIFFUsageParameters)
{
	return mDocumentContext.CreateFormXObjectFromTIFFFile(inTIFFFilePath,inFormXObjectID,inTIFFUsageParameters);
}

PDFFormXObject* PDFWriter::CreateFormXObjectFromTIFFStream(IByteReaderWithPosition* inTIFFStream,
                                                           const TIFFUsageParameters& inTIFFUsageParameters)
{
	return mDocumentContext.CreateFormXObjectFromTIFFStream(inTIFFStream,inTIFFUsageParameters);
}

PDFFormXObject* PDFWriter::CreateFormXObjectFromTIFFStream(IByteReaderWithPosition* inTIFFStream,
                                                           ObjectIDType inFormXObjectID,
                                                           const TIFFUsageParameters& inTIFFUsageParameters)
{
	return mDocumentContext.CreateFormXObjectFromTIFFStream(inTIFFStream,inFormXObjectID,inTIFFUsageParameters);
}

#endif

#ifndef PDFHUMMUS_NO_PNG
PDFFormXObject* PDFWriter::CreateFormXObjectFromPNGFile(const std::string& inPNGFilePath) {
	return CreateFormXObjectFromPNGFile(inPNGFilePath, mObjectsContext.GetInDirectObjectsRegistry().AllocateNewObjectID());
}

PDFFormXObject* PDFWriter::CreateFormXObjectFromPNGFile(const std::string& inPNGFilePath, ObjectIDType inFormXObjectId) {
	InputFile inputFile;
	if (inputFile.OpenFile(inPNGFilePath) != eSuccess) {
		return NULL;
	}

	return CreateFormXObjectFromPNGStream(inputFile.GetInputStream(), inFormXObjectId);
}


PDFFormXObject* PDFWriter::CreateFormXObjectFromPNGStream(IByteReaderWithPosition* inPNGStream) {
	return CreateFormXObjectFromPNGStream(inPNGStream , mObjectsContext.GetInDirectObjectsRegistry().AllocateNewObjectID());
}

PDFFormXObject* PDFWriter::CreateFormXObjectFromPNGStream(IByteReaderWithPosition* inPNGStream, ObjectIDType inFormXObjectId) {
	return mDocumentContext.CreateFormXObjectFromPNGStream(inPNGStream, inFormXObjectId);
}

#endif

PDFImageXObject* PDFWriter::CreateImageXObjectFromJPGFile(const std::string& inJPGFilePath,ObjectIDType inImageXObjectID)
{
	return mDocumentContext.CreateImageXObjectFromJPGFile(inJPGFilePath,inImageXObjectID);
}

PDFFormXObject* PDFWriter::CreateFormXObjectFromJPGFile(const std::string& inJPGFilePath,ObjectIDType inImageXObjectID)
{
	return mDocumentContext.CreateFormXObjectFromJPGFile(inJPGFilePath,inImageXObjectID);
}


PDFUsedFont* PDFWriter::GetFontForFile(const std::string& inFontFilePath,long inFontIndex)
{
	return mDocumentContext.GetFontForFile(inFontFilePath,inFontIndex);
}

PDFUsedFont* PDFWriter::GetFontForFile(const std::string& inFontFilePath,const std::string& inAdditionalMeticsFilePath,long inFontIndex)
{
	return mDocumentContext.GetFontForFile(inFontFilePath,inAdditionalMeticsFilePath,inFontIndex);
}

EStatusCodeAndObjectIDTypeList PDFWriter::CreateFormXObjectsFromPDF(const std::string& inPDFFilePath,
																	  const PDFPageRange& inPageRange,
																	  EPDFPageBox inPageBoxToUseAsFormBox,
																	  const double* inTransformationMatrix,
																	  const ObjectIDTypeList& inCopyAdditionalObjects,
																	  const PDFParsingOptions& inParsingOptions)
{
	return mDocumentContext.CreateFormXObjectsFromPDF(inPDFFilePath,
														inParsingOptions,
														inPageRange,
														inPageBoxToUseAsFormBox,
														inTransformationMatrix,
														inCopyAdditionalObjects);
}

EStatusCodeAndObjectIDTypeList PDFWriter::CreateFormXObjectsFromPDF(const std::string& inPDFFilePath,
																	 const PDFPageRange& inPageRange,
																	 const PDFRectangle& inCropBox,
																	 const double* inTransformationMatrix,
																	 const ObjectIDTypeList& inCopyAdditionalObjects,
																	const PDFParsingOptions& inParsingOptions)
{
	return mDocumentContext.CreateFormXObjectsFromPDF(inPDFFilePath,
														inParsingOptions,
														inPageRange,
														inCropBox,
														inTransformationMatrix,
														inCopyAdditionalObjects);
}

EStatusCodeAndObjectIDTypeList PDFWriter::AppendPDFPagesFromPDF(const std::string& inPDFFilePath,
																const PDFPageRange& inPageRange,
																const ObjectIDTypeList& inCopyAdditionalObjects,
																const PDFParsingOptions& inParsingOptions)
{
	return mDocumentContext.AppendPDFPagesFromPDF(inPDFFilePath,
														inParsingOptions,
														inPageRange,
														inCopyAdditionalObjects);
}

EStatusCode PDFWriter::Shutdown(const std::string& inStateFilePath)
{
	EStatusCode status;

	do
	{
		StateWriter writer;

		status = writer.Start(inStateFilePath);
		if(status != eSuccess)
		{
			TRACE_LOG("PDFWriter::Shutdown, cant start state writing");
			break;
		}

		ObjectIDType rootObjectID = writer.GetObjectsWriter()->StartNewIndirectObject();
		DictionaryContext* pdfWriterDictionary = writer.GetObjectsWriter()->StartDictionary();

		pdfWriterDictionary->WriteKey("Type");
		pdfWriterDictionary->WriteNameValue("PDFWriter");

		ObjectIDType objectsContextID = writer.GetObjectsWriter()->GetInDirectObjectsRegistry().AllocateNewObjectID();
		ObjectIDType DocumentContextID = writer.GetObjectsWriter()->GetInDirectObjectsRegistry().AllocateNewObjectID();

		pdfWriterDictionary->WriteKey("mObjectsContext");
		pdfWriterDictionary->WriteNewObjectReferenceValue(objectsContextID);

		pdfWriterDictionary->WriteKey("mDocumentContext");
		pdfWriterDictionary->WriteNewObjectReferenceValue(DocumentContextID);

        pdfWriterDictionary->WriteKey("mIsModified");
        pdfWriterDictionary->WriteBooleanValue(mIsModified);


        if(mIsModified)
        {
            pdfWriterDictionary->WriteKey("mModifiedFileVersion");
            pdfWriterDictionary->WriteIntegerValue(mModifiedFileVersion);
        }

		writer.GetObjectsWriter()->EndDictionary(pdfWriterDictionary);
		writer.GetObjectsWriter()->EndIndirectObject();

		writer.SetRootObject(rootObjectID);

		status = mObjectsContext.WriteState(writer.GetObjectsWriter(),objectsContextID);
		if(status != eSuccess)
			break;

		status = mDocumentContext.WriteState(writer.GetObjectsWriter(),DocumentContextID);
		if(status != eSuccess)
			break;

		status = writer.Finish();
		if(status != eSuccess)
		{
			TRACE_LOG("PDFWriter::Shutdown, cant finish state writing");
		}

	}while(false);

	if(status != eSuccess)
	{
		mOutputFile.CloseFile();
		TRACE_LOG("PDFWriter::Shutdown, Could not end PDF");
	}
	else
		status = mOutputFile.CloseFile();
	//ReleaseLog();
	return status;
}

EStatusCode PDFWriter::ContinuePDF(const std::string& inOutputFilePath,
								   const std::string& inStateFilePath,
                                   const std::string& inOptionalModifiedFile,
								   const LogConfiguration& inLogConfiguration)
{


	SetupLog(inLogConfiguration);
	EStatusCode status = mOutputFile.OpenFile(inOutputFilePath,true);
	if(status != eSuccess)
		return status;

    if(inOptionalModifiedFile.size() != 0)
    {
        // setup parser for reading modified file
        status = mModifiedFile.OpenFile(inOptionalModifiedFile);
        if(status != eSuccess)
            return status;

        status = mModifiedFileParser.StartPDFParsing(mModifiedFile.GetInputStream());
        if(status != eSuccess)
            return status;
    }

	mObjectsContext.SetOutputStream(mOutputFile.GetOutputStream());
	mDocumentContext.SetOutputFileInformation(&mOutputFile);

	return SetupState(inStateFilePath);


}

EStatusCode PDFWriter::SetupState(const std::string& inStateFilePath)
{
	EStatusCode status;

	do
	{
		StateReader reader;

		status = reader.Start(inStateFilePath);
		if(status != eSuccess)
		{
			TRACE_LOG("PDFWriter::SetupState, cant start state readering");
			break;
		}

		PDFObjectCastPtr<PDFDictionary> pdfWriterDictionary(reader.GetObjectsReader()->ParseNewObject(reader.GetRootObjectID()));

        PDFObjectCastPtr<PDFBoolean> isModifiedObject(pdfWriterDictionary->QueryDirectObject("mIsModified"));
        mIsModified = isModifiedObject->GetValue();

        if(mIsModified)
        {
            PDFObjectCastPtr<PDFInteger> isModifiedFileVersionObject(pdfWriterDictionary->QueryDirectObject("mModifiedFileVersion"));
            mModifiedFileVersion = (EPDFVersion)(isModifiedFileVersionObject->GetValue());
        }

		PDFObjectCastPtr<PDFIndirectObjectReference> objectsContextObject(pdfWriterDictionary->QueryDirectObject("mObjectsContext"));
		status = mObjectsContext.ReadState(reader.GetObjectsReader(),objectsContextObject->mObjectID);
		if(status!= eSuccess)
			break;

		PDFObjectCastPtr<PDFIndirectObjectReference> documentContextObject(pdfWriterDictionary->QueryDirectObject("mDocumentContext"));
		status = mDocumentContext.ReadState(reader.GetObjectsReader(),documentContextObject->mObjectID);
		if(status!= eSuccess)
			break;

		reader.Finish();

	}while(false);

	return status;
}


EStatusCode PDFWriter::ContinuePDFForStream(IByteWriterWithPosition* inOutputStream,
											const std::string& inStateFilePath,
                                            IByteReaderWithPosition* inModifiedSourceStream,
			 								const LogConfiguration& inLogConfiguration)
{
	SetupLog(inLogConfiguration);

    if(inModifiedSourceStream)
        if(mModifiedFileParser.StartPDFParsing(inModifiedSourceStream) != eSuccess)
            return eFailure;

	mObjectsContext.SetOutputStream(inOutputStream);

	return SetupState(inStateFilePath);

}


PDFDocumentCopyingContext* PDFWriter::CreatePDFCopyingContext(const std::string& inPDFFilePath, const PDFParsingOptions& inOptions)
{
	return mDocumentContext.CreatePDFCopyingContext(inPDFFilePath, inOptions);
}

EStatusCode PDFWriter::AttachURLLinktoCurrentPage(const std::string& inURL,const PDFRectangle& inLinkClickArea)
{
	return mDocumentContext.AttachURLLinktoCurrentPage(inURL,inLinkClickArea);
}

EStatusCode PDFWriter::MergePDFPagesToPage(PDFPage* inPage,
								const std::string& inPDFFilePath,
								const PDFPageRange& inPageRange,
								const ObjectIDTypeList& inCopyAdditionalObjects,
								const PDFParsingOptions& inParsingOptions)
{
	return mDocumentContext.MergePDFPagesToPage(inPage,
												inPDFFilePath,
												inParsingOptions,
												inPageRange,
												inCopyAdditionalObjects);
}


EStatusCode PDFWriter::StartPDFForStream(IByteWriterWithPosition* inOutputStream,
										 EPDFVersion inPDFVersion,
										 const LogConfiguration& inLogConfiguration,
										 const PDFCreationSettings& inPDFCreationSettings)
{
	SetupLog(inLogConfiguration);
	SetupCreationSettings(inPDFCreationSettings);
	if (inPDFCreationSettings.DocumentEncryptionOptions.ShouldEncrypt) {
		mDocumentContext.SetupEncryption(inPDFCreationSettings.DocumentEncryptionOptions, thisOrDefaultVersion(inPDFVersion));
		if (!mDocumentContext.SupportsEncryption())
			return eFailure;
	}

	mObjectsContext.SetOutputStream(inOutputStream);
    mIsModified = false;

	return mDocumentContext.WriteHeader(thisOrDefaultVersion(inPDFVersion));
}
EStatusCode PDFWriter::EndPDFForStream()
{
    EStatusCode status;

    if(mIsModified)
        status = mDocumentContext.FinalizeModifiedPDF(&mModifiedFileParser,mModifiedFileVersion);
    else
        status = mDocumentContext.FinalizeNewPDF();
    mModifiedFileParser.ResetParser();
	Cleanup();
	return status;
}

PDFImageXObject* PDFWriter::CreateImageXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream)
{
	return mDocumentContext.CreateImageXObjectFromJPGStream(inJPGStream);
}

PDFImageXObject* PDFWriter::CreateImageXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream,ObjectIDType inImageXObjectID)
{
	return mDocumentContext.CreateImageXObjectFromJPGStream(inJPGStream,inImageXObjectID);
}

PDFFormXObject* PDFWriter::CreateFormXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream)
{
	return mDocumentContext.CreateFormXObjectFromJPGStream(inJPGStream);

}

PDFFormXObject* PDFWriter::CreateFormXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream,ObjectIDType inFormXObjectID)
{
	return mDocumentContext.CreateFormXObjectFromJPGStream(inJPGStream,inFormXObjectID);
}

EStatusCodeAndObjectIDTypeList PDFWriter::CreateFormXObjectsFromPDF(IByteReaderWithPosition* inPDFStream,
																	const PDFPageRange& inPageRange,
																	EPDFPageBox inPageBoxToUseAsFormBox,
																	const double* inTransformationMatrix,
																	const ObjectIDTypeList& inCopyAdditionalObjects,
																	const PDFParsingOptions& inParsingOptions)
{
	return mDocumentContext.CreateFormXObjectsFromPDF(inPDFStream,inParsingOptions,inPageRange,inPageBoxToUseAsFormBox,inTransformationMatrix,inCopyAdditionalObjects);
}

EStatusCodeAndObjectIDTypeList PDFWriter::CreateFormXObjectsFromPDF(IByteReaderWithPosition* inPDFStream,
																	const PDFPageRange& inPageRange,
																	const PDFRectangle& inCropBox,
																	const double* inTransformationMatrix,
																	const ObjectIDTypeList& inCopyAdditionalObjects,
																	const PDFParsingOptions& inParsingOptions)
{
	return mDocumentContext.CreateFormXObjectsFromPDF(inPDFStream,inParsingOptions,inPageRange,inCropBox,inTransformationMatrix,inCopyAdditionalObjects);
}

EStatusCodeAndObjectIDTypeList PDFWriter::AppendPDFPagesFromPDF(IByteReaderWithPosition* inPDFStream,
																const PDFPageRange& inPageRange,
																const ObjectIDTypeList& inCopyAdditionalObjects,
																const PDFParsingOptions& inParsingOptions)
{
	return mDocumentContext.AppendPDFPagesFromPDF(inPDFStream,inParsingOptions,inPageRange,inCopyAdditionalObjects);
}

EStatusCode PDFWriter::MergePDFPagesToPage(	PDFPage* inPage,
											IByteReaderWithPosition* inPDFStream,
											const PDFPageRange& inPageRange,
											const ObjectIDTypeList& inCopyAdditionalObjects,
											const PDFParsingOptions& inParsingOptions)
{
	return mDocumentContext.MergePDFPagesToPage(inPage,inPDFStream, inParsingOptions,inPageRange,inCopyAdditionalObjects);
}

PDFDocumentCopyingContext* PDFWriter::CreatePDFCopyingContext(IByteReaderWithPosition* inPDFStream, const PDFParsingOptions& inOptions)
{
	return mDocumentContext.CreatePDFCopyingContext(inPDFStream,inOptions);
}

EStatusCode PDFWriter::ModifyPDF(const std::string& inModifiedFile,
                                            EPDFVersion inPDFVersion,
                                            const std::string& inOptionalAlternativeOutputFile,
                                            const LogConfiguration& inLogConfiguration,
                                            const PDFCreationSettings& inPDFCreationSettings)
{
    EStatusCode status = eSuccess;

    SetupLog(inLogConfiguration);
	SetupCreationSettings(inPDFCreationSettings);

    do
    {
        // either append to original file, or create a new copy and "modify" it. depending on users choice
        if(inOptionalAlternativeOutputFile.size() == 0 || (inOptionalAlternativeOutputFile == inModifiedFile))
        {
            status = mOutputFile.OpenFile(inModifiedFile,true);
            if(status != eSuccess)
                break;
			mObjectsContext.SetOutputStream(mOutputFile.GetOutputStream());
		}
        else
        {
            status = mOutputFile.OpenFile(inOptionalAlternativeOutputFile);
            if(status != eSuccess)
               break;

            // copy original to new output file
            InputFile modifiedFileInput;
            status = modifiedFileInput.OpenFile(inModifiedFile);
            if(status != eSuccess)
                break;

            OutputStreamTraits traits(mOutputFile.GetOutputStream());
            status = traits.CopyToOutputStream(modifiedFileInput.GetInputStream());
            if(status != eSuccess)
                break;

			// add an extra newline after eof, just in case the original file didn't have it
			mObjectsContext.SetOutputStream(mOutputFile.GetOutputStream());
			mObjectsContext.WriteTokenSeparator(eTokenSeparatorEndLine);
        }

        mDocumentContext.SetOutputFileInformation(&mOutputFile);

        // do setup for modification
        mIsModified = true;
        status = SetupStateFromModifiedFile(inModifiedFile, thisOrDefaultVersion(inPDFVersion), inPDFCreationSettings);
    }
    while (false);

    return status;
}

EStatusCode PDFWriter::ModifyPDFForStream(
                                      IByteReaderWithPosition* inModifiedSourceStream,
                                      IByteWriterWithPosition* inModifiedDestinationStream,
                                      bool inAppendOnly,
                                      EPDFVersion inPDFVersion,
                                      const LogConfiguration& inLogConfiguration,
                                      const PDFCreationSettings& inPDFCreationSettings)
{
    SetupLog(inLogConfiguration);
	SetupCreationSettings(inPDFCreationSettings);

    if(!inAppendOnly)
    {
        // copy original to new output stream
        OutputStreamTraits traits(inModifiedDestinationStream);
        EStatusCode status = traits.CopyToOutputStream(inModifiedSourceStream);
        if(status != eSuccess)
            return status;
        inModifiedSourceStream->SetPosition(0);
    }

    mObjectsContext.SetOutputStream(inModifiedDestinationStream);
    mObjectsContext.WriteTokenSeparator(eTokenSeparatorEndLine);
    
    mIsModified = true;

    return SetupStateFromModifiedStream(inModifiedSourceStream, thisOrDefaultVersion(inPDFVersion), inPDFCreationSettings);
}

EStatusCode PDFWriter::SetupStateFromModifiedStream(IByteReaderWithPosition* inModifiedSourceStream,
                                                    EPDFVersion inPDFVersion,
													const PDFCreationSettings& inPDFCreationSettings)
{
    EStatusCode status;
	PDFParsingOptions parsingOptions;

	// this bit here is actually interesting. in order to modify an already encrypted document
	// and add more content to it, i just need the user password. not the owner one.
	// in fact, passing the owner password here will create the wrong encryption key.
	// interesting.
	if (inPDFCreationSettings.DocumentEncryptionOptions.ShouldEncrypt)
		parsingOptions.Password = inPDFCreationSettings.DocumentEncryptionOptions.UserPassword;

    do
    {
        status = mModifiedFileParser.StartPDFParsing(inModifiedSourceStream, parsingOptions);
        if(status != eSuccess)
            break;

        mObjectsContext.SetupModifiedFile(&mModifiedFileParser);

        status = mDocumentContext.SetupModifiedFile(&mModifiedFileParser);
        if(status != eSuccess)
            break;

		if (mModifiedFileParser.IsEncrypted() && mModifiedFileParser.IsEncryptionSupported()) {
			mDocumentContext.SetupEncryption(&mModifiedFileParser);
			if (!mDocumentContext.SupportsEncryption()) {
				status = eFailure;
				break;
			}
		}

        mModifiedFileVersion = thisOrDefaultVersion(inPDFVersion);
    }
    while (false);

    return status;
}

EStatusCode PDFWriter::SetupStateFromModifiedFile(const std::string& inModifiedFile,EPDFVersion inPDFVersion, const PDFCreationSettings& inPDFCreationSettings)
{
    EStatusCode status;

    do
    {
        status = mModifiedFile.OpenFile(inModifiedFile);
        if(status != eSuccess)
            break;

        status = SetupStateFromModifiedStream(mModifiedFile.GetInputStream(), thisOrDefaultVersion(inPDFVersion), inPDFCreationSettings);
    }
    while(false);

    return status;
}

PDFParser& PDFWriter::GetModifiedFileParser()
{
    return mModifiedFileParser;
}

InputFile& PDFWriter::GetModifiedInputFile()
{
    return mModifiedFile;
}

PDFDocumentCopyingContext* PDFWriter::CreatePDFCopyingContextForModifiedFile()
{
	return mDocumentContext.CreatePDFCopyingContext(&mModifiedFileParser);
}

DoubleAndDoublePair PDFWriter::GetImageDimensions(const std::string& inImageFile,unsigned long inImageIndex, const PDFParsingOptions& inParsingOptions)
{
	return mDocumentContext.GetImageDimensions(inImageFile,inImageIndex,inParsingOptions);
}

DoubleAndDoublePair PDFWriter::GetImageDimensions(IByteReaderWithPosition* inImageStream,unsigned long inImageIndex, const PDFParsingOptions& inParsingOptions)
{
	return mDocumentContext.GetImageDimensions(inImageStream,inImageIndex,inParsingOptions);
}

PDFHummus::EHummusImageType PDFWriter::GetImageType(const std::string& inImageFile,unsigned long inImageIndex)
{
	return mDocumentContext.GetImageType(inImageFile,inImageIndex);
}

unsigned long PDFWriter::GetImagePagesCount(const std::string& inImageFile, const PDFParsingOptions& inOptions)
{
	return mDocumentContext.GetImagePagesCount(inImageFile,inOptions);
}

PDFHummus::EStatusCode PDFWriter::RecryptPDF(
	const std::string& inOriginalPDFPath,
	const std::string& inOriginalPDFPassword,
	const std::string& inNewPDFPath,
	const LogConfiguration& inLogConfiguration,
	const PDFCreationSettings& inPDFCreationSettings,
	EPDFVersion inOveridePDFVersion) {

	InputFile originalPDF;
	OutputFile newPDF;

	EStatusCode status = originalPDF.OpenFile(inOriginalPDFPath);
	if (status != eSuccess)
		return status;

	status = newPDF.OpenFile(inNewPDFPath);
	if (status != eSuccess)
		return status;

	return PDFWriter::RecryptPDF(
		originalPDF.GetInputStream(),
		inOriginalPDFPassword,
		newPDF.GetOutputStream(),
		inLogConfiguration,
		inPDFCreationSettings,
		inOveridePDFVersion
		);
}

PDFHummus::EStatusCode PDFWriter::RecryptPDF(
	IByteReaderWithPosition* inOriginalPDFStream,
	const std::string& inOriginalPDFPassword,
	IByteWriterWithPosition* inNewPDFStream,
	const LogConfiguration& inLogConfiguration,
	const PDFCreationSettings& inPDFCreationSettings,
	EPDFVersion inOveridePDFVersion) {
	PDFWriter pdfWriter;
	EStatusCode status;
	PDFDocumentCopyingContext* copyingContext = NULL;

	/*
	How to recrypt an encrypted or plain PDF. In other words. create a new version that's decrypted, or encrypted with new passwords.
	*/

	do
	{
		// open PDF copying context for the source document (before starting a new one, so i can get the origina level and set the same)
		copyingContext = pdfWriter.CreatePDFCopyingContext(inOriginalPDFStream, PDFParsingOptions(inOriginalPDFPassword));
		if (!copyingContext)
		{
			status = PDFHummus::eFailure;
			break;
		}


		// open new PDF for writing
		status = pdfWriter.StartPDFForStream(
			inNewPDFStream,
			(ePDFVersionUndefined == inOveridePDFVersion)  ? EPDFVersion((int)(copyingContext->GetSourceDocumentParser()->GetPDFLevel() * 10)) : inOveridePDFVersion,
			inLogConfiguration,
			inPDFCreationSettings
			);
		if (status != PDFHummus::eSuccess)
		{
			break;
		}


		// get its root object ID
		PDFObjectCastPtr<PDFIndirectObjectReference> catalogRef(copyingContext->GetSourceDocumentParser()->GetTrailer()->QueryDirectObject("Root"));
		if (!catalogRef)
		{
			status = PDFHummus::eFailure;
			break;
		}

		// deep-copy the whole pdf through its root - return root object ID copy at new PDF
		EStatusCodeAndObjectIDType copyCatalogResult = copyingContext->CopyObject(catalogRef->mObjectID);
		if (copyCatalogResult.first != eSuccess)
		{
			status = PDFHummus::eFailure;
			break;

		}

		delete copyingContext;
		copyingContext = NULL;

		// set new root object ID as this document root
		pdfWriter.GetDocumentContext().GetTrailerInformation().SetRoot(copyCatalogResult.second);

		// now just end the PDF
		pdfWriter.EndPDF();
	} while (false);

	delete copyingContext;

	return status;
}
