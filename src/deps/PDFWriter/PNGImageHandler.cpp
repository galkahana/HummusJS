/*
   Source File : PNGImageHandler.cpp


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
#ifndef PDFHUMMUS_NO_PNG

#include "PNGImageHandler.h"
#include "Trace.h"
#include "EStatusCode.h"
#include "PDFStream.h"
#include "ObjectsContext.h"
#include "DictionaryContext.h"
#include "ProcsetResourcesConstants.h"
#include "DocumentContext.h"
#include "XObjectContentContext.h"
#include "PDFFormXObject.h"
#include "PDFImageXObject.h"
#include "OutputStringBufferStream.h"
#include "InputStringBufferStream.h"
#include "OutputStreamTraits.h"
#include "SafeBufferMacrosDefs.h"
#include "png.h"

#include <list>
#include <stdlib.h> 

using namespace PDFHummus;
using namespace std;

typedef list<PDFImageXObject*> PDFImageXObjectList;


PNGImageHandler::PNGImageHandler(void)
{
	mObjectsContext = NULL;
	mDocumentContext = NULL;
}

PNGImageHandler::~PNGImageHandler(void)
{
}

void PNGImageHandler::SetOperationsContexts(DocumentContext* inDocumentContext,ObjectsContext* inObjectsContext)
{
	mObjectsContext = inObjectsContext;
	mDocumentContext = inDocumentContext;
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
static const std::string scBitsPerComponent = "BitsPerComponent";
static const std::string scSMask = "SMask";
PDFImageXObject* CreateImageXObjectForData(png_structp png_ptr, png_infop info_ptr, png_bytep row, ObjectsContext* inObjectsContext) {
	PDFImageXObjectList listOfImages;
	PDFImageXObject* imageXObject = NULL;
	PDFStream* imageStream = NULL;
	EStatusCode status = eSuccess;

	do
	{

		if (setjmp(png_jmpbuf(png_ptr)))
		{
			// reset failure pointer
			status = eFailure;
			break;
		}

		// get info
		png_uint_32 transformed_width = png_get_image_width(png_ptr, info_ptr);
		png_uint_32 transformed_height = png_get_image_height(png_ptr, info_ptr);
		png_byte transformed_color_type = png_get_color_type(png_ptr, info_ptr);
		png_byte transformed_bit_depth = png_get_bit_depth(png_ptr, info_ptr);
		png_byte channels_count = png_get_channels(png_ptr, info_ptr);
		ObjectIDType imageXObjectObjectId = inObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
		// this image has only the color components, use to flag to determine what to ignore
		bool isAlpha = (transformed_color_type & PNG_COLOR_MASK_ALPHA) != 0;
		png_byte colorComponents = isAlpha ? (channels_count - 1) : channels_count;
		ObjectIDType imageMaskObjectId = isAlpha ? inObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID():0;
		MyStringBuf alphaComponentsData;

		inObjectsContext->StartNewIndirectObject(imageXObjectObjectId);
		DictionaryContext* imageContext = inObjectsContext->StartDictionary();

		// type
		imageContext->WriteKey(scType);
		imageContext->WriteNameValue(scXObject);

		// subtype
		imageContext->WriteKey(scSubType);
		imageContext->WriteNameValue(scImage);

		// Width
		imageContext->WriteKey(scWidth);
		imageContext->WriteIntegerValue(transformed_width);

		// Height
		imageContext->WriteKey(scHeight);
		imageContext->WriteIntegerValue(transformed_height);

		// Bits Per Component
		imageContext->WriteKey(scBitsPerComponent);
		imageContext->WriteIntegerValue(transformed_bit_depth);

		// Color Space
		imageContext->WriteKey(scColorSpace);
		imageContext->WriteNameValue(1 == colorComponents ? scDeviceGray : scDeviceRGB);

		// Mask in case of Alpha
		if (isAlpha) {
			imageContext->WriteKey(scSMask);
			imageContext->WriteNewObjectReferenceValue(imageMaskObjectId);
		}

		// now for the image
		imageStream = inObjectsContext->StartPDFStream(imageContext);
		IByteWriter* writerStream = imageStream->GetWriteStream();
		
		png_uint_32 y = transformed_height;

		if (isAlpha) {
			OutputStringBufferStream alphaWriteStream(&alphaComponentsData);

			while (y-- > 0) {
				// read (using "rectangle" method)
				png_read_row(png_ptr, NULL, row);
				// write. iterate per sample, splitting color components and alpha
				for (png_uint_32 i = 0; i < transformed_width; ++i) {
					
					// note that we're writing by color components, but multiply by channel...that's casue we're skipping alpha
					writerStream->Write((IOBasicTypes::Byte*)(row + i*channels_count), colorComponents);
					
					// write out to alpha stream as well (hummfff i don't like this...but i like less to decode the png again....
					// alpha is the last byte, so offset by color components
					alphaWriteStream.Write((IOBasicTypes::Byte*)(row + i*channels_count + colorComponents), 1);
				}
			}
		}
		else {
			while (y-- > 0) {
				// read
				png_read_row(png_ptr, row, NULL);
				// write
				writerStream->Write((IOBasicTypes::Byte*)(row), transformed_width*colorComponents);
			}
		}

		inObjectsContext->EndPDFStream(imageStream);

		// if there's a soft mask, write it now
		if (isAlpha) {
			inObjectsContext->StartNewIndirectObject(imageMaskObjectId);
			DictionaryContext* imageMaskContext = inObjectsContext->StartDictionary();

			// type
			imageMaskContext->WriteKey(scType);
			imageMaskContext->WriteNameValue(scXObject);

			// subtype
			imageMaskContext->WriteKey(scSubType);
			imageMaskContext->WriteNameValue(scImage);

			// Width
			imageMaskContext->WriteKey(scWidth);
			imageMaskContext->WriteIntegerValue(transformed_width);

			// Height
			imageMaskContext->WriteKey(scHeight);
			imageMaskContext->WriteIntegerValue(transformed_height);

			// Bits Per Component
			imageMaskContext->WriteKey(scBitsPerComponent);
			imageMaskContext->WriteIntegerValue(transformed_bit_depth);

			// Color Space
			imageMaskContext->WriteKey(scColorSpace);
			imageMaskContext->WriteNameValue(scDeviceGray);

			PDFStream* imageMaskStream = inObjectsContext->StartPDFStream(imageMaskContext);
			IByteWriter* writerMaskStream = imageMaskStream->GetWriteStream();

			// write the alpha samples
			InputStringBufferStream alphaWriteStream(&alphaComponentsData);
			OutputStreamTraits traits(writerMaskStream);
			traits.CopyToOutputStream(&alphaWriteStream);

			inObjectsContext->EndPDFStream(imageMaskStream);
			delete imageMaskStream;
		}

		imageXObject = new PDFImageXObject(imageXObjectObjectId, 1 == colorComponents ? KProcsetImageB : KProcsetImageC);
	} while (false);

	if (eFailure == status) {
		delete imageXObject;
		imageXObject = NULL;
	}
	delete imageStream;
	return imageXObject;
}

PDFFormXObject* CreateImageFormXObjectFromImageXObject(
	const PDFImageXObjectList& inImageXObject, 
	ObjectIDType inFormXObjectID, 
	png_uint_32 transformed_width, 
	png_uint_32 transformed_height, 
	DocumentContext* inDocumentContext) {
	PDFFormXObject* formXObject = NULL;
	do
	{

		formXObject = inDocumentContext->StartFormXObject(PDFRectangle(0, 0, transformed_width, transformed_height), inFormXObjectID);
		XObjectContentContext* xobjectContentContext = formXObject->GetContentContext();

		// iterate the images in the list and place one on top of each other
		PDFImageXObjectList::const_iterator it = inImageXObject.begin();
		for (; it != inImageXObject.end(); ++it) {
			xobjectContentContext->q();
			xobjectContentContext->cm(transformed_width, 0, 0, transformed_height, 0, 0);
			xobjectContentContext->Do(formXObject->GetResourcesDictionary().AddImageXObjectMapping(*it));
			xobjectContentContext->Q();
		}

		EStatusCode status = inDocumentContext->EndFormXObjectNoRelease(formXObject);
		if (status != PDFHummus::eSuccess)
		{
			TRACE_LOG("PNGImageHandler::CreateImageFormXObjectFromImageXObject. Unexpected Error, could not create form XObject for image");
			delete formXObject;
			formXObject = NULL;
			break;
		}


	} while (false);
	return formXObject;
}

void ReadDataFromStream(png_structp png_ptr, png_bytep data, png_size_t length)
{
	if (png_ptr == NULL)
		return;

	IByteReaderWithPosition* reader = (IByteReaderWithPosition*)png_get_io_ptr(png_ptr);
	IOBasicTypes::LongFilePositionType readBytes = reader->Read((IOBasicTypes::Byte*)(data), length);

	if (readBytes != (IOBasicTypes::LongFilePositionType)length)
		png_error(png_ptr, "Read Error");
}

void HandlePngError(png_structp png_ptr, png_const_charp error_message) {
	{
		if (error_message)
			TRACE_LOG1("LibPNG Error: %s", error_message);
	}
	png_longjmp(png_ptr, 1);
}

void HandlePngWarning(png_structp png_ptr, png_const_charp warning_message) {
	if(warning_message)
		TRACE_LOG1("LibPNG Warning: %s",warning_message);
}


PDFFormXObject* CreateFormXObjectForPNGStream(IByteReaderWithPosition* inPNGStream, DocumentContext* inDocumentContext, ObjectsContext* inObjectsContext, ObjectIDType inFormXObjectID) {
	// Start reading image to get dimension. we'll then create the form, and then the image
	PDFFormXObject* formXObject = NULL;
	PDFImageXObject* imageXObject = NULL;
	PDFImageXObjectList listOfImages;
	EStatusCode status = eSuccess;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep row = NULL;

	do {
		// init structs and prep 
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, HandlePngError, HandlePngWarning);
		if (png_ptr == NULL) {
			break;
		}

		if (setjmp(png_jmpbuf(png_ptr)))
		{
			status = eFailure;
			break;
		}

		// pair png with custom IO (dont bother with writing)
		png_set_read_fn(png_ptr, (png_voidp)inPNGStream, ReadDataFromStream);

		// create info struct
		info_ptr = png_create_info_struct(png_ptr);
		if (info_ptr == NULL) {
			png_error(png_ptr, "OOM allocating info structure");
		}

		// Gal: is this important?
		png_set_keep_unknown_chunks(png_ptr, PNG_HANDLE_CHUNK_ALWAYS, NULL, 0);

		// read info from png
		png_read_info(png_ptr, info_ptr);

		png_byte color_type = png_get_color_type(png_ptr, info_ptr);
		png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

		// Let's setup some default transformations, and then reprint the png info that will
		// now adapt to post-translation

		// all them set_expand option, to bring us to a common 8 bits per component as a minimum
		if (color_type == PNG_COLOR_TYPE_PALETTE)
			png_set_palette_to_rgb(png_ptr);
		if (color_type == PNG_COLOR_TYPE_GRAY &&
			bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png_ptr);
		if (png_get_valid(png_ptr, info_ptr,
			PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);

		// now let's also avoid 16 bits for now, to stay always at 8 bits per component
		if (bit_depth == 16)
			png_set_strip_16(png_ptr);

		// and let's deal with random < 8 packing, so we're surely in 8 bits now
		if (bit_depth < 8)
			png_set_packing(png_ptr);

		// setup for potential interlace
		int passes = png_set_interlace_handling(png_ptr);

		// let's update info so now it fits the post transform data
		png_read_update_info(png_ptr, info_ptr);

		// grab updated info
		png_size_t transformed_rowbytes = png_get_rowbytes(png_ptr, info_ptr);
		png_uint_32 transformed_width = png_get_image_width(png_ptr, info_ptr);
		png_uint_32 transformed_height = png_get_image_height(png_ptr, info_ptr);

		// allocate reading info 
		row = (png_bytep)malloc(transformed_rowbytes);

		if (row == NULL)
			png_error(png_ptr, "OOM allocating row buffers");

		// K. time to start outputting something. do 1 for each pass, just in case we get


		// for each pass, create an image xobject.
		while (passes > 1) {
			// Gal: actually no.i jist need the last image.so skip the rest [till i find out otherwise...so i'm keeping the rest of the code intact]
			png_uint_32 y = transformed_height;

			while (y-- > 0) {
				// read (using "rectangle" method)
				png_read_row(png_ptr, NULL, row);
			}

			--passes;
		}

		while (passes-- > 0) {
			imageXObject = CreateImageXObjectForData(png_ptr, info_ptr, row, inObjectsContext);
			if (!imageXObject) {
				status = eFailure;
				break;
			}

			listOfImages.push_back(imageXObject);
		}

		if (eFailure == status) {
			break;
		}

		// finish reading image...no longer needed
		png_read_end(png_ptr, NULL);

		// now let's get to the form, which should just place the image and be done
		formXObject = CreateImageFormXObjectFromImageXObject(listOfImages, inFormXObjectID, transformed_width, transformed_height, inDocumentContext);
		if (!formXObject) {
			status = eFailure;
		}
	} while (false);

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	if (row != NULL) free(row);
	PDFImageXObjectList::iterator it = listOfImages.begin();
	for (; it != listOfImages.end(); ++it)
		delete *it;
	listOfImages.clear();
	if (status != eSuccess) {
		delete formXObject;
		formXObject = NULL;
	}

	return formXObject;
}

PDFFormXObject* PNGImageHandler::CreateFormXObjectFromPNGStream(IByteReaderWithPosition* inPNGStream,ObjectIDType inFormXObjectID)
{
	PDFFormXObject* imageFormXObject = NULL;

	do 
	{
		if(!mObjectsContext || !mObjectsContext)
		{
			TRACE_LOG("PNGImageHandler::CreateFormXObjectFromPNGFile. Unexpected Error, mDocumentContex or mObjectsContext not initialized");
			break;
		}

		imageFormXObject = CreateFormXObjectForPNGStream(inPNGStream, mDocumentContext, mObjectsContext, inFormXObjectID);
	} while(false);

	return imageFormXObject;  	
}


DoubleAndDoublePair PNGImageHandler::ReadImageDimensions(IByteReaderWithPosition* inPNGStream) {
	return ReadImageInfo(inPNGStream).dimensions;

}

PNGImageHandler::PNGImageInfo PNGImageHandler::ReadImageInfo(IByteReaderWithPosition* inPNGStream) {
	// reading as is set by internal reader (meaning, post transformations)

	EStatusCode status = eSuccess;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	PNGImageHandler::PNGImageInfo data;

	do {
		// init structs and prep 
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, HandlePngError, HandlePngWarning);
		if (png_ptr == NULL) {
			break;
		}

		if (setjmp(png_jmpbuf(png_ptr)))
		{
			status = eFailure;
			break;
		}

		// pair png with custom IO (dont bother with writing)
		png_set_read_fn(png_ptr, (png_voidp)inPNGStream, ReadDataFromStream);

		// create info struct
		info_ptr = png_create_info_struct(png_ptr);
		if (info_ptr == NULL) {
			png_error(png_ptr, "OOM allocating info structure");
		}

		// Gal: is this important?
		png_set_keep_unknown_chunks(png_ptr, PNG_HANDLE_CHUNK_ALWAYS, NULL, 0);

		// read info from png
		png_read_info(png_ptr, info_ptr);

		png_byte color_type = png_get_color_type(png_ptr, info_ptr);
		png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

		// Let's setup some default transformations, and then reprint the png info that will
		// now adapt to post-translation

		// all them set_expand option, to bring us to a common 8 bits per component as a minimum
		if (color_type == PNG_COLOR_TYPE_PALETTE)
			png_set_palette_to_rgb(png_ptr);
		if (color_type == PNG_COLOR_TYPE_GRAY &&
			bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png_ptr);
		if (png_get_valid(png_ptr, info_ptr,
			PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);

		// now let's also avoid 16 bits for now, to stay always at 8 bits per component
		if (bit_depth == 16)
			png_set_strip_16(png_ptr);

		// and let's deal with random < 8 packing, so we're surely in 8 bits now
		if (bit_depth < 8)
			png_set_packing(png_ptr);

		// let's update info so now it fits the post transform data
		png_read_update_info(png_ptr, info_ptr);

		// grab updated info
		png_uint_32 transformed_width = png_get_image_width(png_ptr, info_ptr);
		png_uint_32 transformed_height = png_get_image_height(png_ptr, info_ptr);
		png_byte transformed_color_type = png_get_color_type(png_ptr, info_ptr);
		png_byte channels_count = png_get_channels(png_ptr, info_ptr);
		bool isAlpha = (transformed_color_type & PNG_COLOR_MASK_ALPHA) != 0;
		png_byte colorComponents = isAlpha ? (channels_count - 1) : channels_count;


		data.colorComponents = colorComponents;
		data.hasAlpha = isAlpha;
		data.dimensions.first = transformed_width;
		data.dimensions.second = transformed_height;

	} while (false);

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	return data;
}

#endif