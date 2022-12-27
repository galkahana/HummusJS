/*
   Source File : PDFWriter.h


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
/*
	Main class for PDF library. entry point for writing PDFs.


*/

#include "EPDFVersion.h"
#include "OutputFile.h"
#include "DocumentContext.h"
#include "ObjectsContext.h"
#include "PDFRectangle.h"
#include "TiffUsageParameters.h"
#include "PDFEmbedParameterTypes.h"
#include "PDFParsingOptions.h"
#include "EncryptionOptions.h"

#include <string>
#include <utility>

typedef std::pair<double,double> DoubleAndDoublePair;

struct LogConfiguration
{
	bool ShouldLog;
	bool StartWithBOM;
	std::string LogFileLocation;
	IByteWriter* LogStream;

	LogConfiguration(bool inShouldLog,bool inStartWithBOM,const std::string& inLogFileLocation){ShouldLog=inShouldLog;StartWithBOM = inStartWithBOM;
																							LogFileLocation=inLogFileLocation;LogStream = NULL;}
	LogConfiguration(bool inShouldLog,IByteWriter* inLogStream){ShouldLog = inShouldLog;LogStream = inLogStream;StartWithBOM = false;}

	static const LogConfiguration& DefaultLogConfiguration();
};

struct PDFCreationSettings
{
	bool CompressStreams;
	bool EmbedFonts;
	EncryptionOptions DocumentEncryptionOptions;

	PDFCreationSettings(bool inCompressStreams, bool inEmbedFonts,EncryptionOptions inDocumentEncryptionOptions = EncryptionOptions::DefaultEncryptionOptions()):DocumentEncryptionOptions(inDocumentEncryptionOptions){
		CompressStreams = inCompressStreams;
		EmbedFonts = inEmbedFonts;
	}

};

class PageContentContext;
class PDFFormXObject;
class PDFImageXObject;
class PDFUsedFont;
class IByteWriterWithPosition;

class PDFWriter
{
public:

	PDFWriter(void);
	~PDFWriter(void);

	// output to file
	PDFHummus::EStatusCode StartPDF(const std::string& inOutputFilePath,
							EPDFVersion inPDFVersion,
							const LogConfiguration& inLogConfiguration = LogConfiguration::DefaultLogConfiguration(),
							const PDFCreationSettings& inPDFCreationSettings = PDFCreationSettings(true,true));

	PDFHummus::EStatusCode EndPDF();

	// output to stream
	PDFHummus::EStatusCode StartPDFForStream(IByteWriterWithPosition* inOutputStream,
								  EPDFVersion inPDFVersion,
								  const LogConfiguration& inLogConfiguration = LogConfiguration::DefaultLogConfiguration(),
								  const PDFCreationSettings& inPDFCreationSettings = PDFCreationSettings(true,true));
	PDFHummus::EStatusCode EndPDFForStream();

	// in case of internal or external error, call this function to cleanup, in order to allow reuse of the PDFWriter class
	void Reset();


    // modify PDF (use EndPDF to finish)
    PDFHummus::EStatusCode ModifyPDF(const std::string& inModifiedFile,
                                     EPDFVersion inPDFVersion,
                                     const std::string& inOptionalAlternativeOutputFile,
                                     const LogConfiguration& inLogConfiguration = LogConfiguration::DefaultLogConfiguration(),
                                     const PDFCreationSettings& inPDFCreationSettings = PDFCreationSettings(true,true));
    PDFHummus::EStatusCode ModifyPDFForStream(
                                    IByteReaderWithPosition* inModifiedSourceStream,
                                    IByteWriterWithPosition* inModifiedDestinationStream,
                                    bool inAppendOnly,
                                    EPDFVersion inPDFVersion,
                                    const LogConfiguration& inLogConfiguration = LogConfiguration::DefaultLogConfiguration(),
                                    const PDFCreationSettings& inPDFCreationSettings = PDFCreationSettings(true,true)
                                    );

	// Ending and Restarting writing session (optional input file is for modification scenarios)
	PDFHummus::EStatusCode Shutdown(const std::string& inStateFilePath);
	PDFHummus::EStatusCode ContinuePDF(const std::string& inOutputFilePath,
							const std::string& inStateFilePath,
                            const std::string& inOptionalModifiedFile = "",
							const LogConfiguration& inLogConfiguration = LogConfiguration::DefaultLogConfiguration());
	// Continue PDF in output stream workflow (optional input stream is for modification scenarios)
	PDFHummus::EStatusCode ContinuePDFForStream(IByteWriterWithPosition* inOutputStream,
									 const std::string& inStateFilePath,
                                     IByteReaderWithPosition* inModifiedSourceStream = NULL,
				 					 const LogConfiguration& inLogConfiguration = LogConfiguration::DefaultLogConfiguration());

	// Page context, for drwaing page content
	PageContentContext* StartPageContentContext(PDFPage* inPage);
	PDFHummus::EStatusCode PausePageContentContext(PageContentContext* inPageContext);
	PDFHummus::EStatusCode EndPageContentContext(PageContentContext* inPageContext);


	// Page Writing [create a new Page by creating a new instance of PDFPage. instances may be reused.
	PDFHummus::EStatusCode WritePage(PDFPage* inPage);
	PDFHummus::EStatusCode WritePageAndRelease(PDFPage* inPage);

	// same as above page writing, but also return page ID. good for extensibility, when you want to refer to the written page form some other place
	EStatusCodeAndObjectIDType WritePageAndReturnPageID(PDFPage* inPage);
	EStatusCodeAndObjectIDType WritePageReleaseAndReturnPageID(PDFPage* inPage);


	// Form XObject creating and writing
	PDFFormXObject* StartFormXObject(const PDFRectangle& inBoundingBox,const double* inMatrix = NULL);
	PDFFormXObject* StartFormXObject(const PDFRectangle& inBoundingBox,ObjectIDType inFormXObjectID,const double* inMatrix = NULL);
    PDFHummus::EStatusCode EndFormXObject(PDFFormXObject* inFormXObject);
	PDFHummus::EStatusCode EndFormXObjectAndRelease(PDFFormXObject* inFormXObject);

	// Image XObject creating [for TIFF nad JPG files].
	// note that as oppose to other methods, create the image xobject also writes it, so there's no "WriteXXXXAndRelease" for image.
	// So...release the object yourself [just delete it]

	// jpeg - two variants
	// will return image xobject sized at 1X1
	PDFImageXObject* CreateImageXObjectFromJPGFile(const std::string& inJPGFilePath);
	PDFImageXObject* CreateImageXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream);
	PDFImageXObject* CreateImageXObjectFromJPGFile(const std::string& inJPGFilePath,ObjectIDType inImageXObjectID);
	PDFImageXObject* CreateImageXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream,ObjectIDType inImageXObjectID);

	// will return form XObject, which will include the xobject at it's size.
	// size is determined by the following order:
	// - JFIF resolution information is looked for. if found used to determine the size
	// - if not found. EXIF resolution information is looked for. if found used to determine the size
	// - if not found. Photoshop resolution information is looked for. if found used to determine the size
	// - otherwise aspect ratio is assumed, and so size is determined trivially from the samples width and height.
	PDFFormXObject* CreateFormXObjectFromJPGFile(const std::string& inJPGFilePath);
	PDFFormXObject* CreateFormXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream);
	PDFFormXObject* CreateFormXObjectFromJPGFile(const std::string& inJPGFilePath,ObjectIDType inFormXObjectID);
	PDFFormXObject* CreateFormXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream,ObjectIDType inFormXObjectID);

	// tiff
#ifndef PDFHUMMUS_NO_TIFF
	PDFFormXObject* CreateFormXObjectFromTIFFFile(	const std::string& inTIFFFilePath,
													const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters());
	PDFFormXObject* CreateFormXObjectFromTIFFStream(IByteReaderWithPosition* inTIFFStream,
													const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters());
	PDFFormXObject* CreateFormXObjectFromTIFFFile(	const std::string& inTIFFFilePath,
													ObjectIDType inFormXObjectID,
													const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters());
	PDFFormXObject* CreateFormXObjectFromTIFFStream(	IByteReaderWithPosition* inTIFFStream,
													ObjectIDType inFormXObjectID,
													const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters());
#endif

	// png
#ifndef PDFHUMMUS_NO_PNG
	PDFFormXObject* CreateFormXObjectFromPNGFile(const std::string& inPNGFilePath);
	PDFFormXObject* CreateFormXObjectFromPNGFile(const std::string& inPNGFilePath, ObjectIDType inFormXObjectID);
	PDFFormXObject* CreateFormXObjectFromPNGStream(IByteReaderWithPosition* inPNGStream);
	PDFFormXObject* CreateFormXObjectFromPNGStream(IByteReaderWithPosition* inPNGStream, ObjectIDType inFormXObjectID);
#endif

	// PDF

	// CreateFormXObjectsFromPDF is for using input PDF pages as objects in one page or more. you can used the returned IDs to place the
	// created form xobjects
	EStatusCodeAndObjectIDTypeList CreateFormXObjectsFromPDF(const std::string& inPDFFilePath,
															 const PDFPageRange& inPageRange,
															 EPDFPageBox inPageBoxToUseAsFormBox,
															 const double* inTransformationMatrix = NULL,
															 const ObjectIDTypeList& inCopyAdditionalObjects = ObjectIDTypeList(),
															 const PDFParsingOptions& inParsingOptions = PDFParsingOptions::DefaultPDFParsingOptions());

	EStatusCodeAndObjectIDTypeList CreateFormXObjectsFromPDF(IByteReaderWithPosition* inPDFStream,
															 const PDFPageRange& inPageRange,
															 EPDFPageBox inPageBoxToUseAsFormBox,
															 const double* inTransformationMatrix = NULL,
															 const ObjectIDTypeList& inCopyAdditionalObjects = ObjectIDTypeList(),
															 const PDFParsingOptions& inParsingOptions = PDFParsingOptions::DefaultPDFParsingOptions());

	// CreateFormXObjectsFromPDF is an override to allow you to determine a custom crop for the page embed
	EStatusCodeAndObjectIDTypeList CreateFormXObjectsFromPDF(const std::string& inPDFFilePath,
															 const PDFPageRange& inPageRange,
															 const PDFRectangle& inCropBox,
															 const double* inTransformationMatrix = NULL,
															 const ObjectIDTypeList& inCopyAdditionalObjects = ObjectIDTypeList(),
															 const PDFParsingOptions& inParsingOptions = PDFParsingOptions::DefaultPDFParsingOptions());

	EStatusCodeAndObjectIDTypeList CreateFormXObjectsFromPDF(IByteReaderWithPosition* inPDFStream,
															 const PDFPageRange& inPageRange,
															 const PDFRectangle& inCropBox,
															 const double* inTransformationMatrix = NULL,
															 const ObjectIDTypeList& inCopyAdditionalObjects = ObjectIDTypeList(),
															 const PDFParsingOptions& inParsingOptions = PDFParsingOptions::DefaultPDFParsingOptions());

	// AppendPDFPagesFromPDF is for simple appending of the input PDF pages
	EStatusCodeAndObjectIDTypeList AppendPDFPagesFromPDF(const std::string& inPDFFilePath,
														const PDFPageRange& inPageRange,
														const ObjectIDTypeList& inCopyAdditionalObjects = ObjectIDTypeList(),
														const PDFParsingOptions& inParsingOptions = PDFParsingOptions::DefaultPDFParsingOptions());

	EStatusCodeAndObjectIDTypeList AppendPDFPagesFromPDF(IByteReaderWithPosition* inPDFStream,
														const PDFPageRange& inPageRange,
														const ObjectIDTypeList& inCopyAdditionalObjects = ObjectIDTypeList(),
														const PDFParsingOptions& inParsingOptions = PDFParsingOptions::DefaultPDFParsingOptions());

	// MergePDFPagesToPage, merge PDF pages content to an input page. good for single-placement of a page content, cheaper than creating
	// and XObject and later placing, when the intention is to use this graphic just once.
	PDFHummus::EStatusCode MergePDFPagesToPage(PDFPage* inPage,
									const std::string& inPDFFilePath,
									const PDFPageRange& inPageRange,
									const ObjectIDTypeList& inCopyAdditionalObjects = ObjectIDTypeList(),
									const PDFParsingOptions& inParsingOptions = PDFParsingOptions::DefaultPDFParsingOptions());

	PDFHummus::EStatusCode MergePDFPagesToPage(PDFPage* inPage,
									IByteReaderWithPosition* inPDFStream,
									const PDFPageRange& inPageRange,
									const ObjectIDTypeList& inCopyAdditionalObjects = ObjectIDTypeList(),
									const PDFParsingOptions& inParsingOptions = PDFParsingOptions::DefaultPDFParsingOptions());


	// Copying context, allowing for a continous flow of copying from multiple sources PDFs (create one per source) to target PDF
	PDFDocumentCopyingContext* CreatePDFCopyingContext(
		const std::string& inPDFFilePath,
		const PDFParsingOptions& inOptions = PDFParsingOptions::DefaultPDFParsingOptions());
	PDFDocumentCopyingContext* CreatePDFCopyingContext(
		IByteReaderWithPosition* inPDFStream,
		const PDFParsingOptions& inOptions = PDFParsingOptions::DefaultPDFParsingOptions());

    // for modified file path, create a copying context for the modified file
    PDFDocumentCopyingContext* CreatePDFCopyingContextForModifiedFile();

	// some public image info services, for users of hummus
	DoubleAndDoublePair GetImageDimensions(const std::string& inImageFile,unsigned long inImageIndex = 0, const PDFParsingOptions& inParsingOptions = PDFParsingOptions::DefaultPDFParsingOptions());
	DoubleAndDoublePair GetImageDimensions(IByteReaderWithPosition* inImageStream,unsigned long inImageIndex = 0, const PDFParsingOptions& inParsingOptions = PDFParsingOptions::DefaultPDFParsingOptions());
	EHummusImageType GetImageType(const std::string& inImageFile,unsigned long inImageIndex);
	unsigned long GetImagePagesCount(const std::string& inImageFile, const PDFParsingOptions& inOptions = PDFParsingOptions::DefaultPDFParsingOptions());


	// fonts [text], font index is provided for multi-font file packages (such as dfont and ttc), 0 the default is
    // what should be passed for single-font files
	PDFUsedFont* GetFontForFile(const std::string& inFontFilePath,long inFontIndex = 0);
	// second overload is for type 1, when an additional metrics file is available
	PDFUsedFont* GetFontForFile(const std::string& inFontFilePath,const std::string& inAdditionalMeticsFilePath,long inFontIndex = 0);

	// URL links
	// URL should be encoded to be a valid URL, ain't gonna be checking that!
	PDFHummus::EStatusCode AttachURLLinktoCurrentPage(const std::string& inURL,const PDFRectangle& inLinkClickArea);

	// Extensibility, reaching to lower levels
	PDFHummus::DocumentContext& GetDocumentContext();
	ObjectsContext& GetObjectsContext();
	OutputFile& GetOutputFile();

    // Extensibiility, for modified files workflow
    PDFParser& GetModifiedFileParser();
    InputFile& GetModifiedInputFile();


	// Recryption statics. create new version of an existing document encrypted with new password or decrypted
	static PDFHummus::EStatusCode RecryptPDF(
		const std::string& inOriginalPDFPath,
		const std::string& inOriginalPDFPassword,
		const std::string& inNewPDFPath,
		const LogConfiguration& inLogConfiguration,
		const PDFCreationSettings& inPDFCreationSettings,
		EPDFVersion inOveridePDFVersion = ePDFVersionUndefined);

	static PDFHummus::EStatusCode RecryptPDF(
		IByteReaderWithPosition* inOriginalPDFStream,
		const std::string& inOriginalPDFPassword,
		IByteWriterWithPosition* inNewPDFStream,
		const LogConfiguration& inLogConfiguration,
		const PDFCreationSettings& inPDFCreationSettings,
		EPDFVersion inOveridePDFVersion = ePDFVersionUndefined);


private:

	ObjectsContext mObjectsContext;
	PDFHummus::DocumentContext mDocumentContext;

	// for output file workflow, this will be the valid output [stream workflow does not have a file]
	OutputFile mOutputFile;

    // for modified workflow, the next two will hold the input file data
    InputFile mModifiedFile;
    PDFParser mModifiedFileParser;
    EPDFVersion mModifiedFileVersion;
    bool mIsModified;

	void SetupLog(const LogConfiguration& inLogConfiguration);
	void SetupCreationSettings(const PDFCreationSettings& inPDFCreationSettings);
	void ReleaseLog();
	PDFHummus::EStatusCode SetupState(const std::string& inStateFilePath);
	void Cleanup();
    PDFHummus::EStatusCode SetupStateFromModifiedFile(const std::string& inModifiedFile,EPDFVersion inPDFVersion, const PDFCreationSettings& inPDFCreationSettings);
    PDFHummus::EStatusCode SetupStateFromModifiedStream(IByteReaderWithPosition* inModifiedSourceStream,EPDFVersion inPDFVersion, const PDFCreationSettings& inPDFCreationSettings);

};
