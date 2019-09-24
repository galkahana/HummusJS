/*
   Source File : JPEGImageHandler.cpp


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
#include "JPEGImageHandler.h"
#include "InputFile.h"
#include "JPEGImageParser.h"
#include "Trace.h"
#include "EStatusCode.h"
#include "PDFImageXObject.h"
#include "PDFStream.h"
#include "OutputStreamTraits.h"
#include "JPEGImageHandler.h"
#include "ObjectsContext.h"
#include "DictionaryContext.h"
#include "IDocumentContextExtender.h"
#include "ProcsetResourcesConstants.h"
#include "DocumentContext.h"
#include "XObjectContentContext.h"
#include "PDFFormXObject.h"

using namespace PDFHummus;


JPEGImageHandler::JPEGImageHandler(void)
{
	mObjectsContext = NULL;
	mDocumentContext = NULL;
}

JPEGImageHandler::~JPEGImageHandler(void)
{
}

void JPEGImageHandler::Reset()
{
	mImagesInformationMap.clear();
	mExtenders.clear();
}

void JPEGImageHandler::SetOperationsContexts(DocumentContext* inDocumentContext,ObjectsContext* inObjectsContext)
{
	mObjectsContext = inObjectsContext;
	mDocumentContext = inDocumentContext;
}

PDFImageXObject* JPEGImageHandler::CreateImageXObjectFromJPGFile(const std::string& inJPGFilePath,ObjectIDType inImageXObjectID)
{
	PDFImageXObject* imageXObject = NULL;

	do
	{
		// retrieve image information
		BoolAndJPEGImageInformation imageInformationResult = RetrieveImageInformation(inJPGFilePath);
		if(!imageInformationResult.first)
		{
			TRACE_LOG1("JPEGImageHandler::CreateImageXObjectJPGFile, unable to retrieve image information for %s",inJPGFilePath.c_str());
			break;
		}

		// Write Image XObject
		imageXObject = CreateAndWriteImageXObjectFromJPGInformation(inJPGFilePath,inImageXObjectID,imageInformationResult.second);

	} while(false);

	return imageXObject;

}

static const std::string scType = "Type";
static const std::string scXObject = "XObject";
static const std::string scSubType = "Subtype";

static const std::string scImage = "Image";
static const std::string scWidth = "Width";
static const std::string scHeight = "Height";
static const std::string scColorSpace = "ColorSpace";
static const std::string scDeviceGray = "DeviceGray";
static const std::string scDeviceRGB = "DeviceRGB";
static const std::string scDeviceCMYK = "DeviceCMYK";
static const std::string scDecode = "Decode";
static const std::string scBitsPerComponent = "BitsPerComponent";
static const std::string scFilter = "Filter";
static const std::string scDCTDecode = "DCTDecode";
PDFImageXObject* JPEGImageHandler::CreateAndWriteImageXObjectFromJPGInformation(const std::string& inJPGFilePath,
																				ObjectIDType inImageXObjectID,
																				const JPEGImageInformation& inJPGImageInformation)
{
	InputFile JPGFile;
	if(JPGFile.OpenFile(inJPGFilePath) != PDFHummus::eSuccess)
	{
		TRACE_LOG1("JPEGImageHandler::CreateAndWriteImageXObjectFromJPGInformation. Unable to open JPG file for reading, %s", inJPGFilePath.c_str());
		return NULL;
	}

	PDFImageXObject* imageXObject = CreateAndWriteImageXObjectFromJPGInformation(JPGFile.GetInputStream(),inImageXObjectID,inJPGImageInformation);

	JPGFile.CloseFile();

	return imageXObject;
}


PDFImageXObject* JPEGImageHandler::CreateAndWriteImageXObjectFromJPGInformation(IByteReaderWithPosition* inJPGImageStream,
																				ObjectIDType inImageXObjectID,
																				const JPEGImageInformation& inJPGImageInformation)
{
	PDFImageXObject* imageXObject = NULL;
	EStatusCode status = PDFHummus::eSuccess;

	do
	{
		if(!mObjectsContext)
		{
			TRACE_LOG("JPEGImageHandler::CreateAndWriteImageXObjectFromJPGInformation. Unexpected Error, mObjectsContext not initialized with an objects context");
			break;
		}

		mObjectsContext->StartNewIndirectObject(inImageXObjectID);
		DictionaryContext* imageContext = mObjectsContext->StartDictionary();

		// type
		imageContext->WriteKey(scType);
		imageContext->WriteNameValue(scXObject);

		// subtype
		imageContext->WriteKey(scSubType);
		imageContext->WriteNameValue(scImage);

		// Width
		imageContext->WriteKey(scWidth);
		imageContext->WriteIntegerValue(inJPGImageInformation.SamplesWidth);

		// Height
		imageContext->WriteKey(scHeight);
		imageContext->WriteIntegerValue(inJPGImageInformation.SamplesHeight);

		// Bits Per Component
		imageContext->WriteKey(scBitsPerComponent);
		imageContext->WriteIntegerValue(8);

		// Color Space and Decode Array if necessary
		imageContext->WriteKey(scColorSpace);
		switch(inJPGImageInformation.ColorComponentsCount)
		{
		case 1:
				imageContext->WriteNameValue(scDeviceGray);
				break;
		case 3:
				imageContext->WriteNameValue(scDeviceRGB);
				break;
		case 4:
				imageContext->WriteNameValue(scDeviceCMYK);

				// Decode array
				imageContext->WriteKey(scDecode);
				mObjectsContext->StartArray();
				for(int i=0;i<4;++i)
				{
					mObjectsContext->WriteDouble(1);
					mObjectsContext->WriteDouble(0);
				}
				mObjectsContext->EndArray();
				mObjectsContext->EndLine();
				break;

		default:
			TRACE_LOG1("JPEGImageHandler::CreateAndWriteImageXObjectFromJPGInformation, Unexpected Error, unfamilar color components count - %d",
				inJPGImageInformation.ColorComponentsCount);
			status = PDFHummus::eFailure;
			break;
		}
		if(status != PDFHummus::eSuccess)
			break;

		// Decoder - DCTDecode
		imageContext->WriteKey(scFilter);
		imageContext->WriteNameValue(scDCTDecode);

		IDocumentContextExtenderSet::iterator it = mExtenders.begin();
		EStatusCode status = PDFHummus::eSuccess;
		for(; it != mExtenders.end() && PDFHummus::eSuccess == status; ++it)
		{
			if((*it)->OnJPEGImageXObjectWrite(inImageXObjectID,imageContext,mObjectsContext,mDocumentContext,this) != PDFHummus::eSuccess)
			{
				TRACE_LOG("JPEGImageHandler::CreateAndWriteImageXObjectFromJPGInformation, unexpected failure. extender declared failure when writing image xobject.");
				status = PDFHummus::eFailure;
				break;
			}
		}
		if(status != PDFHummus::eSuccess)
			break;


		PDFStream* imageStream = mObjectsContext->StartUnfilteredPDFStream(imageContext);

		OutputStreamTraits outputTraits(imageStream->GetWriteStream());
		status = outputTraits.CopyToOutputStream(inJPGImageStream);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("JPEGImageHandler::CreateAndWriteImageXObjectFromJPGInformation. Unexpected Error, failed to copy jpg stream to output stream");
			delete imageStream;
			break;
		}

		mObjectsContext->EndPDFStream(imageStream);
		delete imageStream;

		imageXObject = new PDFImageXObject(inImageXObjectID,1 == inJPGImageInformation.ColorComponentsCount ? KProcsetImageB:KProcsetImageC);
	}while(false);


	return imageXObject;
}

BoolAndJPEGImageInformation JPEGImageHandler::RetrieveImageInformation(IByteReaderWithPosition* inJPGStream)
{
	BoolAndJPEGImageInformation imageInformationResult(false,mNullInformation);

	do
	{
		JPEGImageParser jpgImageParser;
		JPEGImageInformation imageInformation;

		EStatusCode status = jpgImageParser.Parse(inJPGStream,imageInformation);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("JPEGImageHandler::JPEGImageHandler. Failed to parse JPG stream");
			break;
		}

		imageInformationResult.first = true;
		imageInformationResult.second = imageInformation;
	} while(false);

	return imageInformationResult;
}

BoolAndJPEGImageInformation JPEGImageHandler::RetrieveImageInformation(const std::string& inJPGFilePath)
{
	BoolAndJPEGImageInformation imageInformationResult(false,mNullInformation);

	StringToJPEGImageInformationMap::iterator it = mImagesInformationMap.find(inJPGFilePath);
	if(it == mImagesInformationMap.end())
	{
		do
		{
			InputFile JPGFile;
			EStatusCode status = JPGFile.OpenFile(inJPGFilePath);
			if(status != PDFHummus::eSuccess)
			{
				TRACE_LOG1("JPEGImageHandler::JPEGImageHandler. Unable to open JPG file for reading, %s", inJPGFilePath.c_str());
				break;
			}

			JPEGImageParser jpgImageParser;
			JPEGImageInformation imageInformation;

			status = jpgImageParser.Parse(JPGFile.GetInputStream(),imageInformation);
			if(status != PDFHummus::eSuccess)
			{
				TRACE_LOG1("JPEGImageHandler::JPEGImageHandler. Failed to parse JPG file, %s", inJPGFilePath.c_str());
				break;
			}

			status = JPGFile.CloseFile();
			if(status != PDFHummus::eSuccess)
			{
				TRACE_LOG1("JPEGImageHandler::JPEGImageHandler. Failed to close JPG file, %s", inJPGFilePath.c_str());
				break;
			}

			mImagesInformationMap.insert(StringToJPEGImageInformationMap::value_type(inJPGFilePath,imageInformation));
			imageInformationResult.first = true;
			imageInformationResult.second = imageInformation;
		} while(false);
	}
	else
	{
		imageInformationResult.first = true;
		imageInformationResult.second = it->second;
	}

	return imageInformationResult;
}

void JPEGImageHandler::AddDocumentContextExtender(IDocumentContextExtender* inExtender)
{
	mExtenders.insert(inExtender);
}

void JPEGImageHandler::RemoveDocumentContextExtender(IDocumentContextExtender* inExtender)
{
	mExtenders.erase(inExtender);
}


PDFFormXObject* JPEGImageHandler::CreateFormXObjectFromJPGFile(const std::string& inJPGFilePath,ObjectIDType inFormXObjectID)
{
	PDFImageXObject* imageXObject = NULL;
	PDFFormXObject* imageFormXObject = NULL;

	do
	{
		if(!mObjectsContext)
		{
			TRACE_LOG("JPEGImageHandler::CreateFormXObjectFromJPGFile. Unexpected Error, mDocumentContex not initialized with a document context");
			break;
		}

		// retrieve image information
		BoolAndJPEGImageInformation imageInformationResult = RetrieveImageInformation(inJPGFilePath);
		if(!imageInformationResult.first)
		{
			TRACE_LOG1("JPEGImageHandler::CreateFormXObjectFromJPGFile, unable to retrieve image information for %s",inJPGFilePath.c_str());
			break;
		}

		// Write Image XObject
		imageXObject = CreateAndWriteImageXObjectFromJPGInformation(inJPGFilePath,mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID(),imageInformationResult.second);
		if(!imageXObject)
		{
			TRACE_LOG1("JPEGImageHandler::CreateFormXObjectFromJPGFile, unable to create image xobject for %s",inJPGFilePath.c_str());
			break;
		}

		// Write Image form XObject
		imageFormXObject = CreateImageFormXObjectFromImageXObject(imageXObject,inFormXObjectID,imageInformationResult.second);
		if(!imageFormXObject)
		{
			TRACE_LOG1("JPEGImageHandler::CreateFormXObjectFromJPGFile, unable to create form xobject for %s",inJPGFilePath.c_str());
			break;
		}


	} while(false);

	delete imageXObject;
	return imageFormXObject;
}

PDFFormXObject* JPEGImageHandler::CreateImageFormXObjectFromImageXObject(PDFImageXObject* inImageXObject,ObjectIDType inFormXObjectID, const JPEGImageInformation& inJPGImageInformation)
{
	PDFFormXObject* formXObject = NULL;
	do
	{
		if(!mObjectsContext)
		{
			TRACE_LOG("JPEGImageHandler::CreateImageFormXObjectFromImageXObject. Unexpected Error, mDocumentContex not initialized with a document context");
			break;
		}

		DoubleAndDoublePair dimensions = GetImageDimensions(inJPGImageInformation);

		formXObject = mDocumentContext->StartFormXObject(PDFRectangle(0,0,dimensions.first,dimensions.second),inFormXObjectID);
		XObjectContentContext* xobjectContentContext = formXObject->GetContentContext();

		xobjectContentContext->q();
		xobjectContentContext->cm(dimensions.first,0,0,dimensions.second,0,0);
		xobjectContentContext->Do(formXObject->GetResourcesDictionary().AddImageXObjectMapping(inImageXObject));
		xobjectContentContext->Q();

		EStatusCode status = mDocumentContext->EndFormXObjectNoRelease(formXObject);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("JPEGImageHandler::CreateImageFormXObjectFromImageXObject. Unexpected Error, could not create form XObject for image");
			delete formXObject;
			formXObject = NULL;
			break;
		}


	}while(false);
	return formXObject;
}

DoubleAndDoublePair JPEGImageHandler::GetImageDimensions(const JPEGImageInformation& inJPGImageInformation)
{
	DoubleAndDoublePair returnResult(1,1);

	do
	{
		// prefer JFIF determined resolution
		if(inJPGImageInformation.JFIFInformationExists)
		{
			double jfifXDensity = 	(0 == inJPGImageInformation.JFIFXDensity) ? 1:inJPGImageInformation.JFIFXDensity;
			double jfifYDensity = (0 == inJPGImageInformation.JFIFYDensity) ? 1:inJPGImageInformation.JFIFYDensity;

			switch(inJPGImageInformation.JFIFUnit)
			{
				case 1: // INCH
					returnResult.first = ((double)inJPGImageInformation.SamplesWidth / jfifXDensity) * 72.0;
					returnResult.second = ((double)inJPGImageInformation.SamplesHeight / jfifYDensity) * 72.0;
					break;
				case 2: // CM
					returnResult.first = ((double)inJPGImageInformation.SamplesWidth / jfifXDensity) * 72.0 / 2.54;
					returnResult.second = ((double)inJPGImageInformation.SamplesHeight / jfifYDensity) * 72.0 / 2.54;
					break;
				default: // 0 - aspect ratio
					returnResult.first = (double)inJPGImageInformation.SamplesWidth;
					returnResult.second = (double)inJPGImageInformation.SamplesHeight;
					break;
			}
			break;
		}

		// if no jfif try exif
		if(inJPGImageInformation.ExifInformationExists)
		{
			double exifXDensity = 	(0 == inJPGImageInformation.ExifXDensity) ? 1:inJPGImageInformation.ExifXDensity;
			double exifYDensity = (0 == inJPGImageInformation.ExifYDensity) ? 1:inJPGImageInformation.ExifYDensity;

			switch(inJPGImageInformation.ExifUnit)
			{
				case 1: // aspect ratio
					returnResult.first = (double)inJPGImageInformation.SamplesWidth;
					returnResult.second = (double)inJPGImageInformation.SamplesHeight;
					break;
				case 3: // CM
					returnResult.first = ((double)inJPGImageInformation.SamplesWidth / exifXDensity) * 72.0 / 2.54;
					returnResult.second = ((double)inJPGImageInformation.SamplesHeight / exifYDensity) * 72.0 / 2.54;
					break;
				default: // 2 - Inch
					returnResult.first = ((double)inJPGImageInformation.SamplesWidth / exifXDensity) * 72.0;
					returnResult.second = ((double)inJPGImageInformation.SamplesHeight / exifYDensity) * 72.0;
					break;
			}
			break;
		}

		// if no jfif, try photoshop
		if(inJPGImageInformation.PhotoshopInformationExists)
		{
			double photoshopXDensity = 	(0 == inJPGImageInformation.PhotoshopXDensity) ? 1:inJPGImageInformation.PhotoshopXDensity;
			double photoshopYDensity = (0 == inJPGImageInformation.PhotoshopYDensity) ? 1:inJPGImageInformation.PhotoshopYDensity;

			returnResult.first = ((double)inJPGImageInformation.SamplesWidth / photoshopXDensity) * 72.0;
			returnResult.second = ((double)inJPGImageInformation.SamplesHeight / photoshopYDensity) * 72.0;
			break;
		}

		// else aspect ratio
		returnResult.first = (double)inJPGImageInformation.SamplesWidth;
		returnResult.second = (double)inJPGImageInformation.SamplesHeight;
	}while(false);

	return returnResult;
}

PDFImageXObject* JPEGImageHandler::CreateImageXObjectFromJPGFile(const std::string& inJPGFilePath)
{
	if(!mObjectsContext)
	{
		TRACE_LOG("JPEGImageHandler::CreateImageXObjectFromJPGFile. Unexpected Error, mObjectsContext not initialized with an objects context");
		return NULL;
	}

	return CreateImageXObjectFromJPGFile(inJPGFilePath,mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID());
}

PDFFormXObject* JPEGImageHandler::CreateFormXObjectFromJPGFile(const std::string& inJPGFilePath)
{
	if(!mObjectsContext)
	{
		TRACE_LOG("JPEGImageHandler::CreateFormXObjectFromJPGFile. Unexpected Error, mObjectsContext not initialized with an objects context");
		return NULL;
	}

	return CreateFormXObjectFromJPGFile(inJPGFilePath,mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID());

}

PDFImageXObject* JPEGImageHandler::CreateImageXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream)
{
	if(!mObjectsContext)
	{
		TRACE_LOG("JPEGImageHandler::CreateImageXObjectFromJPGStream. Unexpected Error, mObjectsContext not initialized with an objects context");
		return NULL;
	}

	return CreateImageXObjectFromJPGStream(inJPGStream,mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID());
}

PDFImageXObject* JPEGImageHandler::CreateImageXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream,ObjectIDType inImageXObjectID)
{
	PDFImageXObject* imageXObject = NULL;

	do
	{
		if(!mObjectsContext)
		{
			TRACE_LOG("JPEGImageHandler::CreateImageXObjectFromJPGStream. Unexpected Error, mDocumentContex not initialized with a document context");
			break;
		}

		JPEGImageParser jpgImageParser;
		JPEGImageInformation imageInformation;

		LongFilePositionType recordedPosition = inJPGStream->GetCurrentPosition();

		EStatusCode status = jpgImageParser.Parse(inJPGStream,imageInformation);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("JPEGImageHandler::CreateImageXObjectFromJPGStream. Failed to parse JPG stream");
			break;
		}

		// reset image position after parsing header, for later content copying
		inJPGStream->SetPosition(recordedPosition);

		imageXObject = CreateAndWriteImageXObjectFromJPGInformation(inJPGStream,inImageXObjectID,imageInformation);

	} while(false);

	return imageXObject;

}

PDFFormXObject* JPEGImageHandler::CreateFormXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream)
{
	if(!mObjectsContext)
	{
		TRACE_LOG("JPEGImageHandler::CreateFormXObjectFromJPGStream. Unexpected Error, mObjectsContext not initialized with an objects context");
		return NULL;
	}

	return CreateFormXObjectFromJPGStream(inJPGStream,mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID());
}

PDFFormXObject* JPEGImageHandler::CreateFormXObjectFromJPGStream(IByteReaderWithPosition* inJPGStream,ObjectIDType inFormXObjectID)
{
	PDFFormXObject* imageFormXObject = NULL;
	PDFImageXObject* imageXObject = NULL;

	do
	{
		if(!mObjectsContext)
		{
			TRACE_LOG("JPEGImageHandler::CreateImageXObjectFromJPGStream. Unexpected Error, mDocumentContex not initialized with a document context");
			break;
		}

		JPEGImageParser jpgImageParser;
		JPEGImageInformation imageInformation;

		LongFilePositionType recordedPosition = inJPGStream->GetCurrentPosition();

		EStatusCode status = jpgImageParser.Parse(inJPGStream,imageInformation);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG("JPEGImageHandler::CreateImageXObjectFromJPGStream. Failed to parse JPG stream");
			break;
		}

		// reset image position after parsing header, for later content copying
		inJPGStream->SetPosition(recordedPosition);

		imageXObject = CreateAndWriteImageXObjectFromJPGInformation(inJPGStream,mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID(),imageInformation);
		if(!imageXObject)
		{
			TRACE_LOG("JPEGImageHandler::CreateFormXObjectFromJPGStream, unable to create image xobject");
			break;
		}

		// Write Image form XObject
		imageFormXObject = CreateImageFormXObjectFromImageXObject(imageXObject,inFormXObjectID,imageInformation);
		if(!imageFormXObject)
		{
			TRACE_LOG("JPEGImageHandler::CreateImageXObjectFromJPGStream, unable to create form xobject");
			break;
		}

	} while(false);

	delete imageXObject;
	return imageFormXObject;
}

int JPEGImageHandler::GetColorComponents(const JPEGImageInformation& inJPGImageInformation)
{
	return inJPGImageInformation.ColorComponentsCount;
}
