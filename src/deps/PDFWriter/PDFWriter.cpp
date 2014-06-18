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

using namespace PDFHummus;

const LogConfiguration LogConfiguration::DefaultLogConfiguration(false,false,"PDFWriterLog.txt");

const PDFCreationSettings PDFCreationSettings::DefaultPDFCreationSettings(true,true);

PDFWriter::PDFWriter(void)
{
	// Gal: some trick, already set reference to objects context. this does not mean that the document context can start doing something.
	// this allows the creation of copying context before starting to write the PDF, so that already
	// the first decision (level) about the PDF can be the result of parsing
	mDocumentContext.SetObjectsContext(&mObjectsContext);
    mIsModified = false;
	mEmbedFonts = true;
}

PDFWriter::~PDFWriter(void)
{
}

EStatusCode PDFWriter::StartPDF(
							const std::string& inOutputFilePath,
							EPDFVersion inPDFVersion,
							const LogConfiguration& inLogConfiguration,
							const PDFCreationSettings& inPDFCreationSettings)
{
	SetupLog(inLogConfiguration);
	SetupObjectsContext(inPDFCreationSettings);
	mEmbedFonts = inPDFCreationSettings.EmbedFonts;

	EStatusCode status = mOutputFile.OpenFile(inOutputFilePath);
	if(status != eSuccess)
		return status;

	mObjectsContext.SetOutputStream(mOutputFile.GetOutputStream());
	mDocumentContext.SetOutputFileInformation(&mOutputFile);
    
    mIsModified = false;
	
	return mDocumentContext.WriteHeader(inPDFVersion);
}

EStatusCode PDFWriter::EndPDF()
{
    
	EStatusCode status;
	do
	{
        if(mIsModified)
            status = mDocumentContext.FinalizeModifiedPDF(&mModifiedFileParser,mModifiedFileVersion,mEmbedFonts);
        else    
            status = mDocumentContext.FinalizeNewPDF(mEmbedFonts);
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
		Trace::DefaultTrace.SetLogSettings(inLogConfiguration.LogStream,inLogConfiguration.ShouldLog);
	else
		Trace::DefaultTrace.SetLogSettings(inLogConfiguration.LogFileLocation,inLogConfiguration.ShouldLog,inLogConfiguration.StartWithBOM);
}

void PDFWriter::SetupObjectsContext(const PDFCreationSettings& inPDFCreationSettings)
{
	mObjectsContext.SetCompressStreams(inPDFCreationSettings.CompressStreams);
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
																	  const ObjectIDTypeList& inCopyAdditionalObjects)
{
	return mDocumentContext.CreateFormXObjectsFromPDF(inPDFFilePath,
														inPageRange,
														inPageBoxToUseAsFormBox,
														inTransformationMatrix,
														inCopyAdditionalObjects);
}

EStatusCodeAndObjectIDTypeList PDFWriter::CreateFormXObjectsFromPDF(const std::string& inPDFFilePath,
																	 const PDFPageRange& inPageRange,
																	 const PDFRectangle& inCropBox,
																	 const double* inTransformationMatrix,
																	 const ObjectIDTypeList& inCopyAdditionalObjects)
{
	return mDocumentContext.CreateFormXObjectsFromPDF(inPDFFilePath,
														inPageRange,
														inCropBox,
														inTransformationMatrix,
														inCopyAdditionalObjects);
}

EStatusCodeAndObjectIDTypeList PDFWriter::AppendPDFPagesFromPDF(const std::string& inPDFFilePath,
																const PDFPageRange& inPageRange,
																const ObjectIDTypeList& inCopyAdditionalObjects)
{
	return mDocumentContext.AppendPDFPagesFromPDF(inPDFFilePath,
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

		pdfWriterDictionary->WriteKey("mEmbedFonts");
		pdfWriterDictionary->WriteBooleanValue(mEmbedFonts);

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

		PDFObjectCastPtr<PDFBoolean> embedFontsObject(pdfWriterDictionary->QueryDirectObject("mEmbedFonts"));
		mEmbedFonts = embedFontsObject->GetValue();


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


PDFDocumentCopyingContext* PDFWriter::CreatePDFCopyingContext(const std::string& inPDFFilePath)
{
	return mDocumentContext.CreatePDFCopyingContext(inPDFFilePath);
}

EStatusCode PDFWriter::AttachURLLinktoCurrentPage(const std::string& inURL,const PDFRectangle& inLinkClickArea)
{
	return mDocumentContext.AttachURLLinktoCurrentPage(inURL,inLinkClickArea);
}

EStatusCode PDFWriter::MergePDFPagesToPage(PDFPage* inPage,
								const std::string& inPDFFilePath,
								const PDFPageRange& inPageRange,
								const ObjectIDTypeList& inCopyAdditionalObjects)
{
	return mDocumentContext.MergePDFPagesToPage(inPage,
												inPDFFilePath,
												inPageRange,
												inCopyAdditionalObjects);
}


EStatusCode PDFWriter::StartPDFForStream(IByteWriterWithPosition* inOutputStream,
										 EPDFVersion inPDFVersion,
										 const LogConfiguration& inLogConfiguration,
										 const PDFCreationSettings& inPDFCreationSettings)
{
	SetupLog(inLogConfiguration);
	SetupObjectsContext(inPDFCreationSettings);

	mObjectsContext.SetOutputStream(inOutputStream);
    mIsModified = false;
	
	return mDocumentContext.WriteHeader(inPDFVersion);
}
EStatusCode PDFWriter::EndPDFForStream()
{
    EStatusCode status;
    
    if(mIsModified)
        status = mDocumentContext.FinalizeModifiedPDF(&mModifiedFileParser,mModifiedFileVersion,mEmbedFonts);
    else    
        status = mDocumentContext.FinalizeNewPDF(mEmbedFonts);
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
																	const ObjectIDTypeList& inCopyAdditionalObjects)
{
	return mDocumentContext.CreateFormXObjectsFromPDF(inPDFStream,inPageRange,inPageBoxToUseAsFormBox,inTransformationMatrix,inCopyAdditionalObjects);
}

EStatusCodeAndObjectIDTypeList PDFWriter::CreateFormXObjectsFromPDF(IByteReaderWithPosition* inPDFStream,
																	const PDFPageRange& inPageRange,
																	const PDFRectangle& inCropBox,
																	const double* inTransformationMatrix,
																	const ObjectIDTypeList& inCopyAdditionalObjects)
{
	return mDocumentContext.CreateFormXObjectsFromPDF(inPDFStream,inPageRange,inCropBox,inTransformationMatrix,inCopyAdditionalObjects);
}

EStatusCodeAndObjectIDTypeList PDFWriter::AppendPDFPagesFromPDF(IByteReaderWithPosition* inPDFStream,
																const PDFPageRange& inPageRange,
																const ObjectIDTypeList& inCopyAdditionalObjects)
{
	return mDocumentContext.AppendPDFPagesFromPDF(inPDFStream,inPageRange,inCopyAdditionalObjects);
}

EStatusCode PDFWriter::MergePDFPagesToPage(	PDFPage* inPage,
											IByteReaderWithPosition* inPDFStream,
											const PDFPageRange& inPageRange,
											const ObjectIDTypeList& inCopyAdditionalObjects)
{
	return mDocumentContext.MergePDFPagesToPage(inPage,inPDFStream,inPageRange,inCopyAdditionalObjects);
}

PDFDocumentCopyingContext* PDFWriter::CreatePDFCopyingContext(IByteReaderWithPosition* inPDFStream)
{
	return mDocumentContext.CreatePDFCopyingContext(inPDFStream);	
}

EStatusCode PDFWriter::ModifyPDF(const std::string& inModifiedFile,
                                            EPDFVersion inPDFVersion,
                                            const std::string& inOptionalAlternativeOutputFile,
                                            const LogConfiguration& inLogConfiguration,
                                            const PDFCreationSettings& inPDFCreationSettings)
{
    EStatusCode status = eSuccess;
    
    SetupLog(inLogConfiguration);
	SetupObjectsContext(inPDFCreationSettings);
	
    do 
    {
        // either append to original file, or create a new copy and "modify" it. depending on users choice
        if(inOptionalAlternativeOutputFile.size() == 0 || (inOptionalAlternativeOutputFile == inModifiedFile))
        {
            status = mOutputFile.OpenFile(inModifiedFile,true);
            if(status != eSuccess)
                break;
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
        }
        
        mObjectsContext.SetOutputStream(mOutputFile.GetOutputStream());
        mDocumentContext.SetOutputFileInformation(&mOutputFile);
        
        // do setup for modification 
        mIsModified = true;
        status = SetupStateFromModifiedFile(inModifiedFile,inPDFVersion);
    } 
    while (false);
           
    return status;
}

EStatusCode PDFWriter::ModifyPDFForStream(
                                      IByteReaderWithPosition* inModifiedSourceStream,
                                      IByteWriterWithPosition* inModifiedDestinationStream,
                                      EPDFVersion inPDFVersion,
                                      const LogConfiguration& inLogConfiguration,
                                      const PDFCreationSettings& inPDFCreationSettings)
{    
    SetupLog(inLogConfiguration);
	SetupObjectsContext(inPDFCreationSettings);
	
    mObjectsContext.SetOutputStream(inModifiedDestinationStream);
        
    mIsModified = true;
        
    return SetupStateFromModifiedStream(inModifiedSourceStream,inPDFVersion);  
}

EStatusCode PDFWriter::SetupStateFromModifiedStream(IByteReaderWithPosition* inModifiedSourceStream,
                                                    EPDFVersion inPDFVersion)
{
    EStatusCode status;
    
    do 
    {
        status = mModifiedFileParser.StartPDFParsing(inModifiedSourceStream);
        if(status != eSuccess)
            break;    
        
        mObjectsContext.SetupModifiedFile(&mModifiedFileParser);
        
        status = mDocumentContext.SetupModifiedFile(&mModifiedFileParser);
        if(status != eSuccess)
            break;
        
        mModifiedFileVersion = inPDFVersion;
        
    } 
    while (false);
    
    return status;
}

EStatusCode PDFWriter::SetupStateFromModifiedFile(const std::string& inModifiedFile,EPDFVersion inPDFVersion)
{
    EStatusCode status;
    
    do
    {
        status = mModifiedFile.OpenFile(inModifiedFile);
        if(status != eSuccess)
            break;
        
        status = SetupStateFromModifiedStream(mModifiedFile.GetInputStream(),inPDFVersion);
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

DoubleAndDoublePair PDFWriter::GetImageDimensions(const std::string& inImageFile,unsigned long inImageIndex)
{
	return mDocumentContext.GetImageDimensions(inImageFile,inImageIndex);
}

PDFHummus::EHummusImageType PDFWriter::GetImageType(const std::string& inImageFile,unsigned long inImageIndex)
{
	return mDocumentContext.GetImageType(inImageFile,inImageIndex);
}