/*
   Source File : TIFFImageHandler.cpp


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

   
 Additional Copyright Information:

  The majority of this code is converted from tiff2pdf tool that comes with the
  tifflib library. in accordance with the code instructions the header of tiff2pdf 
  is provided here:
 
  Copyright (c) 2003 Ross Finlayson
 
  Permission to use, copy, modify, distribute, and sell this software and 
  its documentation for any purpose is hereby granted without fee, provided
  that (i) the above copyright notices and this permission notice appear in
  all copies of the software and related documentation, and (ii) the name of
  Ross Finlayson may not be used in any advertising or
  publicity relating to the software without the specific, prior written
  permission of Ross Finlayson.
  
  THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
  EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
  WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
  
  IN NO EVENT SHALL ROSS FINLAYSON BE LIABLE FOR
  ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
  OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
  WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
  LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
  OF THIS SOFTWARE.

  let's get nice libtiff copyright as well
  Copyright (c) 1991-1997 Sam Leffler
  Copyright (c) 1991-1997 Silicon Graphics, Inc.
 
  Permission to use, copy, modify, distribute, and sell this software and 
  its documentation for any purpose is hereby granted without fee, provided
  that (i) the above copyright notices and this permission notice appear in
  all copies of the software and related documentation, and (ii) the names of
  Sam Leffler and Silicon Graphics may not be used in any advertising or
  publicity relating to the software without the specific, prior written
  permission of Sam Leffler and Silicon Graphics.
  
  THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
  EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
  WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
  
  IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
  ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
  OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
  WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
  LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
  OF THIS SOFTWARE.



*/

#ifndef PDFHUMMUS_NO_TIFF

#include "TIFFImageHandler.h"
#include "Trace.h"
#include "ObjectsContext.h"
#include "DictionaryContext.h"
#include "PDFStream.h"
#include "IByteWriter.h"
#include "PDFImageXObject.h"
#include "DocumentContext.h"
#include "ProcsetResourcesConstants.h"
#include "XObjectContentContext.h"
#include "PDFFormXObject.h"
#include "SafeBufferMacrosDefs.h"
#include "IDocumentContextExtender.h"
#include "IByteReaderWithPosition.h"

// tiff lib includes
#include "tiffconf.h"
#include "tiffio.h"

#include <stdlib.h> 
#include <search.h>

using namespace PDFHummus;

#define PS_UNIT_SIZE	72.0F

/* This type is of PDF color spaces. */
typedef enum {
	T2P_CS_BILEVEL = 0x01,	/* Bilevel, black and white */
	T2P_CS_GRAY = 0x02,	/* Single channel */
	T2P_CS_RGB = 0x04,	/* Three channel tristimulus RGB */
	T2P_CS_CMYK = 0x08,	/* Four channel CMYK print inkset */
	T2P_CS_LAB = 0x10,	/* Three channel L*a*b* color space */
	T2P_CS_PALETTE = 0x1000,/* One of the above with a color map */
	T2P_CS_CALGRAY = 0x20,	/* Calibrated single channel */
	T2P_CS_CALRGB = 0x40,	/* Calibrated three channel tristimulus RGB */
	T2P_CS_ICCBASED = 0x80	/* ICC profile color specification */
} t2p_cs_t;

/* This type is of PDF compression types.  */
typedef enum{
	T2P_COMPRESS_NONE=0x00
	, T2P_COMPRESS_G4=0x01
	, T2P_COMPRESS_ZIP=0x04
} t2p_compress_t;

/* This type is whether TIFF image data can be used in PDF without transcoding. */
typedef enum{
	T2P_TRANSCODE_RAW=0x01, /* The raw data from the input can be used without recompressing */
	T2P_TRANSCODE_ENCODE=0x02 /* The data from the input is perhaps unencoded and reencoded */
} t2p_transcode_t;

/* This type is of information about the data samples of the input image. */
typedef enum{
	T2P_SAMPLE_NOTHING=0x0000, /* The unencoded samples are normal for the output colorspace */
	T2P_SAMPLE_ABGR_TO_RGB=0x0001, /* The unencoded samples are the result of ReadRGBAImage */
	T2P_SAMPLE_RGBA_TO_RGB=0x0002, /* The unencoded samples are contiguous RGBA */
	T2P_SAMPLE_RGBAA_TO_RGB=0x0004, /* The unencoded samples are RGBA with premultiplied alpha */
	T2P_SAMPLE_YCBCR_TO_RGB=0x0008, 
	T2P_SAMPLE_YCBCR_TO_LAB=0x0010, 
	T2P_SAMPLE_REALIZE_PALETTE=0x0020, /* The unencoded samples are indexes into the color map */
	T2P_SAMPLE_SIGNED_TO_UNSIGNED=0x0040, /* The unencoded samples are signed instead of unsignd */
	T2P_SAMPLE_LAB_SIGNED_TO_UNSIGNED=0x0040, /* The L*a*b* samples have a* and b* signed */
	T2P_SAMPLE_PLANAR_SEPARATE_TO_CONTIG=0x0100 /* The unencoded samples are separate instead of contiguous */
} t2p_sample_t;

/* This type is of error status of the T2P struct. */
typedef enum{
	T2P_ERR_OK = 0, /* This is the value of mT2p->t2p_error when there is no error */
	T2P_ERR_ERROR = 1 /* This is the value of mT2p->t2p_error when there was an error */
} t2p_err_t;

/* This struct defines a logical page of a TIFF. */
struct T2P_PAGE{
	tdir_t page_directory;
	uint32_t page_number;
	ttile_t page_tilecount;
	uint32_t page_extra;

	T2P_PAGE()
	{
		page_directory = 0;
		page_number = 0;
		page_tilecount = 0;
		page_extra = 0;
	}
};

/* This struct defines a PDF rectangle's coordinates. */
struct T2P_BOX{
	float x1;
	float y1;
	float x2;
	float y2;
	float mat[9];

	T2P_BOX()
	{
		x1 = 0;
		y1 = 0;
		x2 = 0;
		y2 = 0;
		mat[0] = mat[1] = mat[2] = mat[3] = mat[4] = mat[5] = mat[6] = mat[7] = mat[8] = 0;
	}
};

/* This struct defines a tile of a PDF.  */
struct T2P_TILE{
	T2P_BOX tile_box;
};

/* This struct defines information about the tiles on a PDF page. */
struct T2P_TILES{
	ttile_t tiles_tilecount;
	uint32_t tiles_tilewidth;
	uint32_t tiles_tilelength;
	uint32_t tiles_tilecountx;
	uint32_t tiles_tilecounty;
	uint32_t tiles_edgetilewidth;
	uint32_t tiles_edgetilelength;
	T2P_TILE* tiles_tiles;

	T2P_TILES()
	{
		tiles_tilecount = 0;
		tiles_tilewidth = 0;
		tiles_tilelength = 0;
		tiles_tilecountx = 0;
		tiles_tilecounty = 0;
		tiles_edgetilewidth = 0;
		tiles_edgetilelength = 0;
		tiles_tiles = 0;
	}
};

/* This struct is the context of a function to generate PDF from a TIFF. */
struct T2P{
	t2p_err_t t2p_error;
	T2P_PAGE* tiff_pages;
	T2P_TILES* tiff_tiles;
	tdir_t tiff_pagecount;
	uint16_t tiff_compression;
	uint16_t tiff_photometric;
	uint16_t tiff_fillorder;
	uint16_t tiff_bitspersample;
	uint16_t tiff_samplesperpixel;
	uint16_t tiff_planar;
	uint32_t tiff_width;
	uint32_t tiff_length;
	float tiff_xres;
	float tiff_yres;
	uint16_t tiff_orientation;
	toff_t tiff_dataoffset;
	tsize_t tiff_datasize;
	uint16_t tiff_resunit;
	uint16_t pdf_centimeters;
	uint16_t pdf_overrideres;
	uint16_t pdf_overridepagesize;
	float pdf_defaultxres;
	float pdf_defaultyres;
	float pdf_xres;
	float pdf_yres;
	float pdf_pagewidth;
	float pdf_pagelength;
	float pdf_imagewidth;
	float pdf_imagelength;
	T2P_BOX pdf_mediabox;
	T2P_BOX pdf_imagebox;
	uint32_t pdf_catalog;
	uint32_t pdf_pages;
	uint32_t pdf_info;
	ObjectIDType pdf_palettecs;
	uint16_t pdf_fitwindow;
	uint32_t pdf_startxref;
	t2p_cs_t pdf_colorspace;
	uint16_t pdf_colorspace_invert;
	uint16_t pdf_switchdecode;
	uint16_t pdf_palettesize;
	unsigned char* pdf_palette;
	int pdf_labrange[4];
	t2p_compress_t pdf_defaultcompression;
	uint16_t pdf_defaultcompressionquality;
	t2p_compress_t pdf_compression;
	uint16_t pdf_compressionquality;
	uint16_t pdf_nopassthrough;
	t2p_transcode_t pdf_transcode;
	t2p_sample_t pdf_sample;
	tdir_t pdf_page;

	float tiff_whitechromaticities[2];
	float tiff_primarychromaticities[6];
	float tiff_referenceblackwhite[2];
	float* tiff_transferfunction[3];
	int pdf_image_interpolate;	/* 0 (default) : do not interpolate,
					   1 : interpolate */
	uint16_t tiff_transferfunctioncount;
	ObjectIDType pdf_icccs;
	uint32_t tiff_iccprofilelength;
	tdata_t tiff_iccprofile;

	TIFF* input;
	TIFF* output;
	std::string inputFilePath;
	PDFStream* pdfStream;
	ObjectIDType pdf_transfer_functions_gstate;

	T2P()
	{
		t2p_error = T2P_ERR_OK;
		tiff_pages = 0;
		tiff_tiles = 0;
		tiff_pagecount = 0;
		tiff_compression = 0;
		tiff_photometric = 0;
		tiff_fillorder = 0;
		tiff_bitspersample = 0;
		tiff_samplesperpixel = 0;
		tiff_planar = 0;
		tiff_width = 0;
		tiff_length = 0;
		tiff_xres = 0;
		tiff_yres = 0;
		tiff_orientation = 0;
		tiff_dataoffset = 0;
		tiff_datasize = 0;
		tiff_resunit = 0;
		pdf_centimeters = 0;
		pdf_overrideres = 0;
		pdf_overridepagesize = 0;
		pdf_defaultxres = 0;
		pdf_defaultyres = 0;
		pdf_xres = 0;
		pdf_yres = 0;
		pdf_pagewidth = 0;
		pdf_pagelength = 0;
		pdf_imagewidth = 0;
		pdf_imagelength = 0;
		pdf_catalog = 0;
		pdf_pages = 0;
		pdf_info = 0;
		pdf_palettecs = 0;
		pdf_fitwindow = 0;
		pdf_startxref = 0;
		pdf_colorspace = T2P_CS_BILEVEL;
		pdf_colorspace_invert = 0;
		pdf_switchdecode = 0;
		pdf_palettesize = 0;
		pdf_palette = 0;
		pdf_labrange[0] = pdf_labrange[1] = pdf_labrange[2] = pdf_labrange[3] = 0;
		pdf_defaultcompression = T2P_COMPRESS_NONE;
		pdf_defaultcompressionquality = 0;
		pdf_compression = T2P_COMPRESS_NONE;
		pdf_compressionquality = 0;
		pdf_nopassthrough = 0;
		pdf_transcode = T2P_TRANSCODE_RAW;
		pdf_sample = T2P_SAMPLE_NOTHING;
		pdf_page = 0;

		tiff_whitechromaticities[0] = tiff_whitechromaticities[1] = 0;
		tiff_primarychromaticities[0] = tiff_primarychromaticities[1] = tiff_primarychromaticities[2] = tiff_primarychromaticities[3] = tiff_primarychromaticities[4] = tiff_primarychromaticities[5] = 0;
		tiff_referenceblackwhite[0] = tiff_referenceblackwhite[1] = 0;
		tiff_transferfunction[0] = tiff_transferfunction[1] = tiff_transferfunction[2] = 0 ;
		pdf_image_interpolate = 0;
		tiff_transferfunctioncount = 0;
		pdf_icccs = 0;
		tiff_iccprofilelength = 0;
		tiff_iccprofile = 0;

		input = 0;
		output = 0;
		pdfStream = 0;
		pdf_transfer_functions_gstate = 0;
	}

};

typedef	void (*TIFFErrorHandler)(const char*, const char*, va_list);

static const std::string scWarningString(": Warning, ");
static const std::string scDot(".");
static const std::string scErrorString(": Error, ");

void ReportWarning(const char* inModel, const char* inFormat, va_list inParametersList)
{
	char buffer[5001];
	std::stringstream formatter;
	formatter<<inModel<<scWarningString<<inFormat<<scDot;

	SAFE_VSPRINTF(buffer,5001,formatter.str().c_str(),inParametersList);

    Trace::DefaultTrace().TraceToLog(buffer);
}

void ReportError(const char* inModel, const char* inFormat, va_list inParametersList)
{
	char buffer[5001];
	std::stringstream formatter;
	formatter<<inModel<<scErrorString<<inFormat<<scDot;

	SAFE_VSPRINTF(buffer,5001,formatter.str().c_str(),inParametersList);

	Trace::DefaultTrace().TraceToLog(buffer);
}

TIFFImageHandler::TIFFImageHandler():mUserParameters(TIFFUsageParameters::DefaultTIFFUsageParameters())
{
	mT2p = NULL;
	mExtender = NULL;
}

TIFFImageHandler::~TIFFImageHandler(void)
{
	DestroyConversionState();
}

void TIFFImageHandler::Reset()
{
	DestroyConversionState();
	mT2p = NULL;
	mExtender = NULL;
}

void TIFFImageHandler::SetOperationsContexts(DocumentContext* inContainerDocumentContext,ObjectsContext* inObjectsContext)
{
	mObjectsContext = inObjectsContext;
	mContainerDocumentContext = inContainerDocumentContext;
}

PDFFormXObject* TIFFImageHandler::CreateFormXObjectFromTIFFFile(const std::string& inTIFFFilePath,
																const TIFFUsageParameters& inTIFFUsageParameters)
{
	if(!mObjectsContext)
	{
		TRACE_LOG("TIFFImageHandler::CreateFormXObjectFromTIFFFile. Unexpected Error, mObjectsContext not initialized with an objects context");
		return NULL;
	}

	return CreateFormXObjectFromTIFFFile(inTIFFFilePath,mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID(),inTIFFUsageParameters);
}

PDFFormXObject* TIFFImageHandler::CreateFormXObjectFromTIFFFile(const std::string& inTIFFFilePath,
																ObjectIDType inFormXObjectID,
																const TIFFUsageParameters& inTIFFUsageParameters)

{
	InputFile tiffFile;

	if(tiffFile.OpenFile(inTIFFFilePath) != PDFHummus::eSuccess)
	{
		TRACE_LOG1("TIFFImageHandler::CreateFormXObjectFromTIFFFile. cannot open file for reading - %s",inTIFFFilePath.c_str());
		return NULL;
	}
		
	return CreateFormXObjectFromTIFFStream(tiffFile.GetInputStream(),inFormXObjectID,inTIFFUsageParameters);
}

void TIFFImageHandler::InitializeConversionState(){

	mT2p = new T2P();
	mT2p->pdf_defaultxres=300.0;
	mT2p->pdf_defaultyres=300.0;
	// if does not have a compression that is either zip or ccit, this should cause it to compress in zip...like all other normal compression
	mT2p->pdf_defaultcompression = T2P_COMPRESS_ZIP;
}

void TIFFImageHandler::DestroyConversionState()
{
	int i=0;

	if(mT2p != NULL){
		if(mT2p->tiff_pages != NULL){
			_TIFFfree( (tdata_t) mT2p->tiff_pages);
		}
		for(i=0;i<mT2p->tiff_pagecount;i++){
			if(mT2p->tiff_tiles[i].tiles_tiles != NULL){
				_TIFFfree( (tdata_t) mT2p->tiff_tiles[i].tiles_tiles);
			}
		}
		if(mT2p->tiff_tiles != NULL){
			_TIFFfree( (tdata_t) mT2p->tiff_tiles);
		}
		if(mT2p->pdf_palette != NULL){
			_TIFFfree( (tdata_t) mT2p->pdf_palette);
		}
		delete mT2p;
		mT2p = NULL;

	}
}


PDFFormXObject* TIFFImageHandler::ConvertTiff2PDF(ObjectIDType inFormXObjectID)
{
	PDFFormXObject* imageFormXObject = NULL;
	EStatusCode status;
	PDFImageXObjectList imagesImageXObject;
	PDFImageXObject* anImage;

	do
	{
		status = ReadTopLevelTiffInformation();
		if(status != PDFHummus::eSuccess)
			break;

		if(mT2p->pdf_page >= mT2p->tiff_pagecount)
		{
			TRACE_LOG3(
				"TIFFImageHandler::ConvertTiff2PDF, Requested tiff page %u where the tiff only has %u pages. Tiff file name - %s",
				mT2p->pdf_page,
				mT2p->tiff_pagecount,
				mT2p->inputFilePath.c_str());
			status = PDFHummus::eFailure;
			break;
		}
		status = ReadTIFFPageInformation();
		if(status != PDFHummus::eSuccess)
			break;
		

		// Write Transfer functions
		if(mT2p->tiff_transferfunctioncount != 0)
		{
			ObjectIDTypeList transferFunctionsObjectIDs;
			for(int i=0; i < mT2p->tiff_transferfunctioncount; i++)
				transferFunctionsObjectIDs.push_back(WriteTransferFunction(i));
			mT2p->pdf_transfer_functions_gstate = WriteTransferFunctionsExtGState(transferFunctionsObjectIDs);
		}

		// Write color space palette if one exists
		if( (mT2p->pdf_colorspace & T2P_CS_PALETTE) != 0)
			mT2p->pdf_palettecs = WritePaletteCS();

		// Write ICC profile for ICC CS based color spaces
		if( (mT2p->pdf_colorspace & T2P_CS_ICCBASED) != 0)
			mT2p->pdf_icccs = WriteICCCS();


		// Write image XObjects. for tiles write multiple images. for untiled write a single
		// image
		if(mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilecount !=0)
		{
			for(int i=0;i < (int)mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilecount;++i)
			{
				anImage = WriteTileImageXObject(i);
				if(anImage)
					imagesImageXObject.push_back(anImage);
				else
				{
					status = PDFHummus::eFailure;
					break;
				}
			}
		} 
		else 
		{
			anImage = WriteUntiledImageXObject();
			if(anImage)
				imagesImageXObject.push_back(anImage);
			else
				status = PDFHummus::eFailure;
		}
		if(status != PDFHummus::eSuccess)
			break;

		// now...write the final XObject to posit all images in their right size
		// as well as setup the graphic state, ICC, Transfer functions...the lots
		// return this bugger as the image
		imageFormXObject = WriteImagesFormXObject(imagesImageXObject,inFormXObjectID);

	}while(false);

	PDFImageXObjectList::iterator itImages = imagesImageXObject.begin();
	for(; itImages != imagesImageXObject.end(); ++itImages)
		delete *itImages;

	return imageFormXObject;
}

/*
 * This function is used by qsort to sort a T2P_PAGE* array of page structures
 * by page number.
 */

static int t2p_cmp_t2p_page(const void* e1, const void* e2)
{
	return( ((T2P_PAGE*)e1)->page_number - ((T2P_PAGE*)e2)->page_number );
}

EStatusCode TIFFImageHandler::ReadTopLevelTiffInformation()
{
	EStatusCode status = PDFHummus::eSuccess;
	tdir_t directorycount = TIFFNumberOfDirectories(mT2p->input);
	tdir_t i=0;
	uint16_t pagen=0;
	uint16_t paged=0;
	uint16_t xuint16_t=0;

	do
	{
		// allocate tiff pages
		mT2p->tiff_pages = (T2P_PAGE*) _TIFFmalloc(directorycount * sizeof(T2P_PAGE));
		if(!mT2p->tiff_pages)
		{
			TRACE_LOG2("Can't allocate %u bytes of memory for tiff_pages array, %s",
						directorycount * sizeof(T2P_PAGE),mT2p->inputFilePath.c_str());
			status = PDFHummus::eFailure;
			break;
		}
		_TIFFmemset( mT2p->tiff_pages, 0x00, directorycount * sizeof(T2P_PAGE));
	
		// allocate tile array, one for each page
		mT2p->tiff_tiles = (T2P_TILES*) _TIFFmalloc(directorycount * sizeof(T2P_TILES));
		if(!mT2p->tiff_tiles)
		{
			TRACE_LOG2("Can't allocate %u bytes of memory for tiff_tiles array, %s",
						directorycount * sizeof(T2P_PAGE),mT2p->inputFilePath.c_str());
			status = PDFHummus::eFailure;
			break;
		}
		_TIFFmemset( mT2p->tiff_tiles, 0x00, directorycount * sizeof(T2P_TILES));

		for(i=0;i<directorycount;i++)
		{
			bool isPage = false,isPage2 = false;
			uint32_t subfiletype = 0;
			
			if(!TIFFSetDirectory(mT2p->input, i)){
				TRACE_LOG2("Can't set directory %u of input file %s",
							directorycount * sizeof(T2P_PAGE),mT2p->inputFilePath.c_str());
				status = PDFHummus::eFailure;
				break;
			}			

			if(TIFFGetField(mT2p->input, TIFFTAG_PAGENUMBER, &pagen, &paged))
			{
				mT2p->tiff_pages[mT2p->tiff_pagecount].page_number =
					((pagen>paged) && (paged != 0)) ? paged : pagen;
				isPage2 = true;
			} 
			else if(TIFFGetField(mT2p->input, TIFFTAG_SUBFILETYPE, &subfiletype))
			{
				isPage = ((subfiletype & FILETYPE_PAGE) != 0) || (subfiletype == 0);
			} 
			else if(TIFFGetField(mT2p->input, TIFFTAG_OSUBFILETYPE, &subfiletype))
			{
				isPage =	(subfiletype == OFILETYPE_IMAGE) || 
							(subfiletype == OFILETYPE_PAGE)	 || 
							(subfiletype == 0);
			}
			else
			{
				isPage = true;
				isPage2 = true;
			}

			if(isPage)
				mT2p->tiff_pages[mT2p->tiff_pagecount].page_number=mT2p->tiff_pagecount;
			if(isPage || isPage2)
			{
				mT2p->tiff_pages[mT2p->tiff_pagecount].page_directory=i;
				if(TIFFIsTiled(mT2p->input))
				{
					mT2p->tiff_pages[mT2p->tiff_pagecount].page_tilecount = 
						TIFFNumberOfTiles(mT2p->input);
				}
			}
			mT2p->tiff_pagecount++;
		}
		if(status != PDFHummus::eSuccess)
			break;

		qsort((void*) mT2p->tiff_pages, mT2p->tiff_pagecount,sizeof(T2P_PAGE), t2p_cmp_t2p_page);

		for(i=0;i<mT2p->tiff_pagecount;i++)
		{
			TIFFSetDirectory(mT2p->input, mT2p->tiff_pages[i].page_directory );
			if((TIFFGetField(mT2p->input, TIFFTAG_PHOTOMETRIC, &xuint16_t)
						&& (xuint16_t==PHOTOMETRIC_PALETTE))
			   || TIFFGetField(mT2p->input, TIFFTAG_INDEXED, &xuint16_t)) 
			{
				mT2p->tiff_pages[i].page_extra++;
			}
			if (TIFFGetField(mT2p->input, TIFFTAG_TRANSFERFUNCTION,
									 &(mT2p->tiff_transferfunction[0]),
									 &(mT2p->tiff_transferfunction[1]),
									 &(mT2p->tiff_transferfunction[2]))) 
			{
				if(mT2p->tiff_transferfunction[1] !=
				   mT2p->tiff_transferfunction[0]) 
				{
					mT2p->tiff_transferfunctioncount = 3;
					mT2p->tiff_pages[i].page_extra += 4;
				} 
				else 
				{
					mT2p->tiff_transferfunctioncount = 1;
					mT2p->tiff_pages[i].page_extra += 2;
				}
			} 
			else 
			{
				mT2p->tiff_transferfunctioncount=0;
			}
			if(TIFFGetField(
				mT2p->input, 
				TIFFTAG_ICCPROFILE, 
				&(mT2p->tiff_iccprofilelength), 
				&(mT2p->tiff_iccprofile)) != 0)
			{
				mT2p->tiff_pages[i].page_extra++;
			}
			mT2p->tiff_tiles[i].tiles_tilecount = mT2p->tiff_pages[i].page_tilecount;
			if( (TIFFGetField(mT2p->input, TIFFTAG_PLANARCONFIG, &xuint16_t) != 0)
				&& (xuint16_t == PLANARCONFIG_SEPARATE ) )
			{
					TIFFGetField(mT2p->input, TIFFTAG_SAMPLESPERPIXEL, &xuint16_t);
					mT2p->tiff_tiles[i].tiles_tilecount/= xuint16_t;
			}
			if( mT2p->tiff_tiles[i].tiles_tilecount > 0)
			{
				TIFFGetField(mT2p->input, 
					TIFFTAG_TILEWIDTH, 
					&(mT2p->tiff_tiles[i].tiles_tilewidth));
				TIFFGetField(mT2p->input, 
					TIFFTAG_TILELENGTH, 
					&(mT2p->tiff_tiles[i].tiles_tilelength) );
				mT2p->tiff_tiles[i].tiles_tiles = 
				(T2P_TILE*) _TIFFmalloc(
					mT2p->tiff_tiles[i].tiles_tilecount 
					* sizeof(T2P_TILE) );
				if(!mT2p->tiff_tiles[i].tiles_tiles)
				{

					TRACE_LOG2("Can't allocate %u bytes of memory for tiles, %s",
								mT2p->tiff_tiles[i].tiles_tilecount * sizeof(T2P_TILE),
								mT2p->inputFilePath.c_str());
					status = PDFHummus::eFailure;
					break;
				}
			}		
		}
	
	}while(false);
	return status;
}

EStatusCode TIFFImageHandler::ReadTIFFPageInformation() //t2p_read_tiff_data
{
	EStatusCode status = PDFHummus::eSuccess;
	uint16_t xuint16_t;
	uint16_t* xuint16_tp;
	float* xfloatp;

	mT2p->pdf_transcode = T2P_TRANSCODE_ENCODE;
	TIFFSetDirectory(mT2p->input, mT2p->tiff_pages[mT2p->pdf_page].page_directory);
	TIFFGetField(mT2p->input, TIFFTAG_IMAGEWIDTH, &(mT2p->tiff_width));

	do
	{
		TIFFGetField(mT2p->input, TIFFTAG_IMAGEWIDTH, &(mT2p->tiff_width));
		if(0 == mT2p->tiff_width)
		{
			TRACE_LOG1("TIFFImageHandler::ReadTIFFPageInformation: No support for %s with zero width",
						mT2p->inputFilePath.c_str());
			status = PDFHummus::eFailure;
			break;
		}

		TIFFGetField(mT2p->input, TIFFTAG_IMAGELENGTH, &(mT2p->tiff_length));
		if(0 == mT2p->tiff_length)
		{
			TRACE_LOG1("TIFFImageHandler::ReadTIFFPageInformation: No support for %s with zero length",
						mT2p->inputFilePath.c_str());
			status = PDFHummus::eFailure;
			break;
		}

        if(TIFFGetField(mT2p->input, TIFFTAG_COMPRESSION, &mT2p->tiff_compression) == 0)
		{
			TRACE_LOG1("TIFFImageHandler::ReadTIFFPageInformation: No support for %s with no compression tag",
						mT2p->inputFilePath.c_str());
			status = PDFHummus::eFailure;
			break;

        }

        if( TIFFIsCODECConfigured(mT2p->tiff_compression) == 0)
		{
			TRACE_LOG2(
				"TIFFImageHandler::ReadTIFFPageInformation: No support for %s with compression type %u:  not configured", 
				mT2p->inputFilePath.c_str(),
				mT2p->tiff_compression);				
			status = PDFHummus::eFailure;
			break;
		}

		TIFFGetFieldDefaulted(mT2p->input, TIFFTAG_BITSPERSAMPLE, &(mT2p->tiff_bitspersample));
		switch(mT2p->tiff_bitspersample)
		{
			case 1:
			case 2:
			case 4:
			case 8:
				break;
			case 0:
				TRACE_LOG1(
					"TIFFImageHandler::ReadTIFFPageInformation: Image %s has 0 bits per sample, assuming 1", 
					mT2p->inputFilePath.c_str());				
				mT2p->tiff_bitspersample = 1;
				break;
			default:
				TRACE_LOG2(
					"TIFFImageHandler::ReadTIFFPageInformation: No support for %s with %u bits per sample", 
					mT2p->inputFilePath.c_str(),
					mT2p->tiff_bitspersample);				
				status = PDFHummus::eFailure;
				break;
		}
		if(status != PDFHummus::eSuccess)
			break;

		TIFFGetFieldDefaulted(mT2p->input, TIFFTAG_SAMPLESPERPIXEL, &(mT2p->tiff_samplesperpixel));
		if(mT2p->tiff_samplesperpixel>4)
		{
			TRACE_LOG2(
				"TIFFImageHandler::ReadTIFFPageInformation: No support for %s with %u samples per pixel", 
				mT2p->inputFilePath.c_str(),
				mT2p->tiff_samplesperpixel);				
			status = PDFHummus::eFailure;
			break;
		}

		if(mT2p->tiff_samplesperpixel==0)
		{
			TRACE_LOG1(
				"TIFFImageHandler::ReadTIFFPageInformation: Image %s has 0 samples per pixel, assuming 1", 
				mT2p->inputFilePath.c_str());	
			mT2p->tiff_samplesperpixel=1;
		}
	
		if(TIFFGetField(mT2p->input, TIFFTAG_SAMPLEFORMAT, &xuint16_t) != 0 )
		{
			switch(xuint16_t)
			{
				case 0:
				case 1:
				case 4:
					break;
				default:
					TRACE_LOG2(
						"TIFFImageHandler::ReadTIFFPageInformation: No support for %s with sample format %u", 
						mT2p->inputFilePath.c_str(),
						xuint16_t);				
					status = PDFHummus::eFailure;
					break;
			}
		}
		if(status != PDFHummus::eSuccess)
				break;
		
		TIFFGetFieldDefaulted(mT2p->input, TIFFTAG_FILLORDER, &(mT2p->tiff_fillorder));
        if(TIFFGetField(mT2p->input, TIFFTAG_PHOTOMETRIC, &(mT2p->tiff_photometric)) == 0)
		{
			TRACE_LOG1("TIFFImageHandler::ReadTIFFPageInformation: No support for %s with no photometric interpretation tag",
						mT2p->inputFilePath.c_str());
			status = PDFHummus::eFailure;
			break;
        }

		switch(mT2p->tiff_photometric)
		{
			case PHOTOMETRIC_MINISWHITE:
			case PHOTOMETRIC_MINISBLACK: 
				if (mT2p->tiff_bitspersample==1){
					mT2p->pdf_colorspace=T2P_CS_BILEVEL;
					if(mT2p->tiff_photometric==PHOTOMETRIC_MINISWHITE){
						mT2p->pdf_switchdecode ^= 1;
					}
				} else {
					mT2p->pdf_colorspace=T2P_CS_GRAY;
					if(mT2p->tiff_photometric==PHOTOMETRIC_MINISWHITE){
						mT2p->pdf_switchdecode ^= 1;
					} 
				}
				break;
			case PHOTOMETRIC_RGB: 
				mT2p->pdf_colorspace=T2P_CS_RGB;
				if(mT2p->tiff_samplesperpixel == 3)
				{
					break;
				}
				if(TIFFGetField(mT2p->input, TIFFTAG_INDEXED, &xuint16_t))
				{
					if(xuint16_t==1)
						ReadPhotometricPalette();
				}
				if(mT2p->tiff_samplesperpixel > 3) 
				{
					if(mT2p->tiff_samplesperpixel == 4) 
					{
						mT2p->pdf_colorspace = T2P_CS_RGB;
						if(TIFFGetField(mT2p->input,
								TIFFTAG_EXTRASAMPLES,
								&xuint16_t, &xuint16_tp)
						   && xuint16_t == 1) 
						{
							if(xuint16_tp[0] == EXTRASAMPLE_ASSOCALPHA)
							{
								mT2p->pdf_sample=T2P_SAMPLE_RGBAA_TO_RGB;
								break;
							}
							if(xuint16_tp[0] == EXTRASAMPLE_UNASSALPHA)
							{
								mT2p->pdf_sample=T2P_SAMPLE_RGBA_TO_RGB;
								break;
							}
							TRACE_LOG1(
								"TIFFImageHandler::ReadTIFFPageInformation, RGB image %s has 4 samples per pixel, assuming RGBA",
								mT2p->inputFilePath.c_str());
							break;
						}
						mT2p->pdf_colorspace=T2P_CS_CMYK;
						mT2p->pdf_switchdecode ^= 1;
						TRACE_LOG1(
							"TIFFImageHandler::ReadTIFFPageInformation, RGB image %s has 4 samples per pixel, assuming CMYK",
							mT2p->inputFilePath.c_str());
						break;
					} 
					else 
					{
						TRACE_LOG2(
							"TIFFImageHandler::ReadTIFFPageInformation, No support for RGB image %s with %u samples per pixel",
							mT2p->inputFilePath.c_str(),
							mT2p->tiff_samplesperpixel
							);
						status = PDFHummus::eFailure;
						break;
					}
				} else {
					TRACE_LOG2(
						"TIFFImageHandler::ReadTIFFPageInformation, No support for RGB image %s with %u samples per pixel",
						mT2p->inputFilePath.c_str(),
						mT2p->tiff_samplesperpixel
						);
					status = PDFHummus::eFailure;
					break;
				}
			case PHOTOMETRIC_PALETTE: 
				status = ReadPhotometricPalette();
				break;
			case PHOTOMETRIC_SEPARATED:
				if(TIFFGetField(mT2p->input, TIFFTAG_INDEXED, &xuint16_t))
				{
					if(xuint16_t==1)
						ReadPhotometricPaletteCMYK();
				}
				if( TIFFGetField(mT2p->input, TIFFTAG_INKSET, &xuint16_t) )
				{
					if(xuint16_t != INKSET_CMYK)
					{
						TRACE_LOG1(
							"TIFFImageHandler::ReadTIFFPageInformation, No support for %s because its inkset is not CMYK",
							mT2p->inputFilePath.c_str()
							);
						status = PDFHummus::eFailure;
						break;
					}
				}
				if(mT2p->tiff_samplesperpixel==4)
				{
					mT2p->pdf_colorspace=T2P_CS_CMYK;
				} 
				else 
				{
					TRACE_LOG2(
						"TIFFImageHandler::ReadTIFFPageInformation, No support for %s because it has %u samples per pixel",
						mT2p->inputFilePath.c_str(),
						mT2p->tiff_samplesperpixel
						);
					status = PDFHummus::eFailure;
					break;
				}
				break;
			case PHOTOMETRIC_YCBCR:
				mT2p->pdf_colorspace=T2P_CS_RGB;
				if(mT2p->tiff_samplesperpixel==1)
				{
					mT2p->pdf_colorspace=T2P_CS_GRAY;
					mT2p->tiff_photometric=PHOTOMETRIC_MINISBLACK;
					break;
				}
				mT2p->pdf_sample=T2P_SAMPLE_YCBCR_TO_RGB;
				break;
			case PHOTOMETRIC_CIELAB:
				mT2p->pdf_labrange[0]= -127;
				mT2p->pdf_labrange[1]= 127;
				mT2p->pdf_labrange[2]= -127;
				mT2p->pdf_labrange[3]= 127;
				mT2p->pdf_sample=T2P_SAMPLE_LAB_SIGNED_TO_UNSIGNED;
				mT2p->pdf_colorspace=T2P_CS_LAB;
				break;
			case PHOTOMETRIC_ICCLAB:
				mT2p->pdf_labrange[0]= 0;
				mT2p->pdf_labrange[1]= 255;
				mT2p->pdf_labrange[2]= 0;
				mT2p->pdf_labrange[3]= 255;
				mT2p->pdf_colorspace=T2P_CS_LAB;
				break;
			case PHOTOMETRIC_ITULAB:
				mT2p->pdf_labrange[0]=-85;
				mT2p->pdf_labrange[1]=85;
				mT2p->pdf_labrange[2]=-75;
				mT2p->pdf_labrange[3]=124;
				mT2p->pdf_sample=T2P_SAMPLE_LAB_SIGNED_TO_UNSIGNED;
				mT2p->pdf_colorspace=T2P_CS_LAB;
				break;
			case PHOTOMETRIC_LOGL:
			case PHOTOMETRIC_LOGLUV:
				TRACE_LOG1(
					"TIFFImageHandler::ReadTIFFPageInformation, No support for %s with photometric interpretation LogL/LogLuv",
					mT2p->inputFilePath.c_str()
					);
				status = PDFHummus::eFailure;
				break;
			default:
				TRACE_LOG1(
					"TIFFImageHandler::ReadTIFFPageInformation, No support for %s with photometric interpretation %u",
					mT2p->inputFilePath.c_str()
					);
				status = PDFHummus::eFailure;
				break;
		}
		if(status != PDFHummus::eSuccess)
			break;

		if(TIFFGetField(mT2p->input, TIFFTAG_PLANARCONFIG, &(mT2p->tiff_planar)))
		{
			switch(mT2p->tiff_planar)
			{
				case 0:
					TRACE_LOG1(
						"TIFFImageHandler::ReadTIFFPageInformation, Image %s has planar configuration 0, assuming 1", 
						mT2p->inputFilePath.c_str());
					mT2p->tiff_planar=PLANARCONFIG_CONTIG;
				case PLANARCONFIG_CONTIG:
					break;
				case PLANARCONFIG_SEPARATE:
					mT2p->pdf_sample=T2P_SAMPLE_PLANAR_SEPARATE_TO_CONTIG;
					if(mT2p->tiff_bitspersample!=8)
					{
						TRACE_LOG2(
							"TIFFImageHandler::ReadTIFFPageInformation, No support for %s with separated planar configuration and %u bits per sample", 
							mT2p->inputFilePath.c_str(),
							mT2p->tiff_bitspersample);
						status = PDFHummus::eFailure;
						break;
					}
					break;
				default:
					TRACE_LOG2(
						"TIFFImageHandler::ReadTIFFPageInformation, No support for %s with planar configuration %u", 
						mT2p->inputFilePath.c_str(),
						mT2p->tiff_planar);
					status = PDFHummus::eFailure;
					break;
			}
			if(status != PDFHummus::eSuccess)
				break;
		}

        TIFFGetFieldDefaulted(mT2p->input, TIFFTAG_ORIENTATION,
                              &(mT2p->tiff_orientation));
        if(mT2p->tiff_orientation>8)
		{
                TRACE_LOG2("TIFFImageHandler::ReadTIFFPageInformation, Image %s has orientation %u, assuming 0",
                            mT2p->inputFilePath.c_str(), 
							mT2p->tiff_orientation);
                mT2p->tiff_orientation=0;
        }

        if(TIFFGetField(mT2p->input, TIFFTAG_XRESOLUTION, &(mT2p->tiff_xres) ) == 0)
		{
                mT2p->tiff_xres=0.0;
        }
        if(TIFFGetField(mT2p->input, TIFFTAG_YRESOLUTION, &(mT2p->tiff_yres) ) == 0)
		{
                mT2p->tiff_yres=0.0;
        }
		TIFFGetFieldDefaulted(mT2p->input, TIFFTAG_RESOLUTIONUNIT,&(mT2p->tiff_resunit));
		if(mT2p->tiff_resunit == RESUNIT_CENTIMETER) 
		{
			mT2p->tiff_xres *= 2.54F;
			mT2p->tiff_yres *= 2.54F;
		} 
		else if (mT2p->tiff_resunit != RESUNIT_INCH && mT2p->pdf_centimeters != 0) 
		{
			mT2p->tiff_xres *= 2.54F;
			mT2p->tiff_yres *= 2.54F;
		}

		ComposePDFPage();

		mT2p->pdf_transcode = T2P_TRANSCODE_ENCODE;
		if(mT2p->pdf_nopassthrough==0)
		{
			if(mT2p->tiff_compression==COMPRESSION_CCITTFAX4)
			{
				if(TIFFIsTiled(mT2p->input) || (TIFFNumberOfStrips(mT2p->input)==1) )
				{
					mT2p->pdf_transcode = T2P_TRANSCODE_RAW;
					mT2p->pdf_compression=T2P_COMPRESS_G4;
				}
			}
			if(mT2p->tiff_compression== COMPRESSION_ADOBE_DEFLATE 
				|| mT2p->tiff_compression==COMPRESSION_DEFLATE)
			{
				if(TIFFIsTiled(mT2p->input) || (TIFFNumberOfStrips(mT2p->input)==1) )
				{
					mT2p->pdf_transcode = T2P_TRANSCODE_RAW;
					mT2p->pdf_compression=T2P_COMPRESS_ZIP;
				}
			}
		}

		if(mT2p->pdf_transcode!=T2P_TRANSCODE_RAW)
		{
			mT2p->pdf_compression = mT2p->pdf_defaultcompression;
		}

		if(mT2p->pdf_sample & T2P_SAMPLE_REALIZE_PALETTE)
		{
			if(mT2p->pdf_colorspace & T2P_CS_CMYK)
			{
				mT2p->tiff_samplesperpixel=4;
				mT2p->tiff_photometric=PHOTOMETRIC_SEPARATED;
			} 
			else 
			{
				mT2p->tiff_samplesperpixel=3;
				mT2p->tiff_photometric=PHOTOMETRIC_RGB;
			}
		}

		if (TIFFGetField(mT2p->input, TIFFTAG_TRANSFERFUNCTION,
				 &(mT2p->tiff_transferfunction[0]),
				 &(mT2p->tiff_transferfunction[1]),
				 &(mT2p->tiff_transferfunction[2]))) 
		{
			if(mT2p->tiff_transferfunction[1] !=
			   mT2p->tiff_transferfunction[0]) 
			{
				mT2p->tiff_transferfunctioncount=3;
			} 
			else 
			{
				mT2p->tiff_transferfunctioncount=1;
			}
		} 
		else 
		{
			mT2p->tiff_transferfunctioncount=0;
		}
		if(TIFFGetField(mT2p->input, TIFFTAG_WHITEPOINT, &xfloatp)!=0)
		{
			mT2p->tiff_whitechromaticities[0]=xfloatp[0];
			mT2p->tiff_whitechromaticities[1]=xfloatp[1];
			if(mT2p->pdf_colorspace & T2P_CS_GRAY)
			{
				mT2p->pdf_colorspace = (t2p_cs_t)(mT2p->pdf_colorspace | T2P_CS_CALGRAY);
			}
			if(mT2p->pdf_colorspace & T2P_CS_RGB)
			{
				mT2p->pdf_colorspace = (t2p_cs_t)(mT2p->pdf_colorspace | T2P_CS_CALRGB);
			}
		}
		if(TIFFGetField(mT2p->input, TIFFTAG_PRIMARYCHROMATICITIES, &xfloatp)!=0)
		{
			mT2p->tiff_primarychromaticities[0]=xfloatp[0];
			mT2p->tiff_primarychromaticities[1]=xfloatp[1];
			mT2p->tiff_primarychromaticities[2]=xfloatp[2];
			mT2p->tiff_primarychromaticities[3]=xfloatp[3];
			mT2p->tiff_primarychromaticities[4]=xfloatp[4];
			mT2p->tiff_primarychromaticities[5]=xfloatp[5];
			if(mT2p->pdf_colorspace & T2P_CS_RGB)
			{
				mT2p->pdf_colorspace = (t2p_cs_t)(mT2p->pdf_colorspace | T2P_CS_CALRGB);
			}
		}
		if(mT2p->pdf_colorspace & T2P_CS_LAB)
		{
			if(TIFFGetField(mT2p->input, TIFFTAG_WHITEPOINT, &xfloatp) != 0)
			{
				mT2p->tiff_whitechromaticities[0]=xfloatp[0];
				mT2p->tiff_whitechromaticities[1]=xfloatp[1];
			} 
			else 
			{
				mT2p->tiff_whitechromaticities[0]=0.3457F; /* 0.3127F; */
				mT2p->tiff_whitechromaticities[1]=0.3585F; /* 0.3290F; */
			}
		}
		if(TIFFGetField(mT2p->input, 
			TIFFTAG_ICCPROFILE, 
			&(mT2p->tiff_iccprofilelength), 
			&(mT2p->tiff_iccprofile))!=0){
			mT2p->pdf_colorspace = (t2p_cs_t)(mT2p->pdf_colorspace | T2P_CS_ICCBASED);
		} 
		else 
		{
			mT2p->tiff_iccprofilelength=0;
			mT2p->tiff_iccprofile=NULL;
		}
		
		if( mT2p->tiff_bitspersample==1 &&
			mT2p->tiff_samplesperpixel==1)
		{
			mT2p->pdf_compression = T2P_COMPRESS_G4;
		}
	}while(false);

	return status;
}

EStatusCode TIFFImageHandler::ReadPhotometricPalette()
{
	EStatusCode status = PDFHummus::eSuccess;
	uint16_t* r;
	uint16_t* g;
	uint16_t* b;

	do
	{
		if(mT2p->tiff_samplesperpixel!=1)
		{
			TRACE_LOG1(
				"TIFFImageHandler::ReadTIFFPageInformation, No support for palettized image %s with not one sample per pixel",
				mT2p->inputFilePath.c_str());
			status = PDFHummus::eFailure;
			break;
		}
		mT2p->pdf_colorspace= (t2p_cs_t)(T2P_CS_RGB | T2P_CS_PALETTE);
		mT2p->pdf_palettesize=0x0001<<mT2p->tiff_bitspersample;
		if(!TIFFGetField(mT2p->input, TIFFTAG_COLORMAP, &r, &g, &b))
		{
			TRACE_LOG1(
				"TIFFImageHandler::ReadTIFFPageInformation, Palettized image %s has no color map",
				mT2p->inputFilePath.c_str());
			status = PDFHummus::eFailure;
			break;
		} 
		if(mT2p->pdf_palette != NULL)
		{
			_TIFFfree(mT2p->pdf_palette);
			mT2p->pdf_palette=NULL;
		}
		mT2p->pdf_palette = (unsigned char*)
			_TIFFmalloc(mT2p->pdf_palettesize*3);
		if(mT2p->pdf_palette==NULL)
		{
			TRACE_LOG2(
				"TIFFImageHandler::ReadTIFFPageInformation, Can't allocate %u bytes of memory for t2p_read_tiff_image, %s",
				mT2p->pdf_palettesize,
				mT2p->inputFilePath.c_str()
				);
			status = PDFHummus::eFailure;
			break;
		}
		for(int i=0;i<mT2p->pdf_palettesize;i++)
		{
			mT2p->pdf_palette[(i*3)]  = (unsigned char) (r[i]>>8);
			mT2p->pdf_palette[(i*3)+1]= (unsigned char) (g[i]>>8);
			mT2p->pdf_palette[(i*3)+2]= (unsigned char) (b[i]>>8);
		}
		mT2p->pdf_palettesize *= 3;
	}while(false);

	return status;
}


EStatusCode TIFFImageHandler::ReadPhotometricPaletteCMYK()
{
	EStatusCode status = PDFHummus::eSuccess;
	uint16_t* r;
	uint16_t* g;
	uint16_t* b;
	uint16_t* a;

	do
	{
		if(mT2p->tiff_samplesperpixel!=1)
		{
			TRACE_LOG1(
				"TIFFImageHandler::ReadTIFFPageInformation, No support for palettized CMYK image %s with not one sample per pixel",
				mT2p->inputFilePath.c_str()
				);
			status = PDFHummus::eFailure;
			break;
		}
		mT2p->pdf_colorspace=t2p_cs_t(T2P_CS_CMYK | T2P_CS_PALETTE);
		mT2p->pdf_palettesize=0x0001<<mT2p->tiff_bitspersample;
		if(!TIFFGetField(mT2p->input, TIFFTAG_COLORMAP, &r, &g, &b, &a))
		{
			TRACE_LOG1(
				"TIFFImageHandler::ReadTIFFPageInformation, Palettized image %s has no color map",
				mT2p->inputFilePath.c_str()
				);
			status = PDFHummus::eFailure;
			break;
		} 
		if(mT2p->pdf_palette != NULL)
		{
			_TIFFfree(mT2p->pdf_palette);
			mT2p->pdf_palette=NULL;
		}
		mT2p->pdf_palette = (unsigned char*) 
			_TIFFmalloc(mT2p->pdf_palettesize*4);
		if(mT2p->pdf_palette==NULL)
		{
			TRACE_LOG2(
				"TIFFImageHandler::ReadTIFFPageInformation, Can't allocate %u bytes of memory for t2p_read_tiff_image, %s",
				mT2p->pdf_palettesize, 
				mT2p->inputFilePath.c_str()
				);
			status = PDFHummus::eFailure;
			break;
		}
		for(int i=0;i<mT2p->pdf_palettesize;i++)
		{
			mT2p->pdf_palette[(i*4)]  = (unsigned char) (r[i]>>8);
			mT2p->pdf_palette[(i*4)+1]= (unsigned char) (g[i]>>8);
			mT2p->pdf_palette[(i*4)+2]= (unsigned char) (b[i]>>8);
			mT2p->pdf_palette[(i*4)+3]= (unsigned char) (a[i]>>8);
		}
		mT2p->pdf_palettesize *= 4;
	}while(false);

	return status;
}

/*
	This function composes the page size and image and tile locations on a page.
*/

void TIFFImageHandler::ComposePDFPage()
{

	uint32_t i=0;
	uint32_t i2=0;
	T2P_TILE* tiles=NULL;
	T2P_BOX* boxp=NULL;
	uint32_t tilecountx=0;
	uint32_t tilecounty=0;
	uint32_t tilewidth=0;
	uint32_t tilelength=0;
	int istiled=0;
	float f=0;
	
	mT2p->pdf_xres = mT2p->tiff_xres;
	mT2p->pdf_yres = mT2p->tiff_yres;
	if(mT2p->pdf_overrideres) 
	{
		mT2p->pdf_xres = mT2p->pdf_defaultxres;
		mT2p->pdf_yres = mT2p->pdf_defaultyres;
	}
	if(mT2p->pdf_xres == 0.0)
		mT2p->pdf_xres = mT2p->pdf_defaultxres;
	if(mT2p->pdf_yres == 0.0)
		mT2p->pdf_yres = mT2p->pdf_defaultyres;
	if (mT2p->tiff_resunit != RESUNIT_CENTIMETER	/* RESUNIT_NONE and */
	    && mT2p->tiff_resunit != RESUNIT_INCH) 
	{	/* other cases */
		mT2p->pdf_imagewidth = ((float)(mT2p->tiff_width))/mT2p->pdf_xres;
		mT2p->pdf_imagelength = ((float)(mT2p->tiff_length))/mT2p->pdf_yres;
	} 
	else 
	{
		mT2p->pdf_imagewidth = 
			((float)(mT2p->tiff_width))*PS_UNIT_SIZE/mT2p->pdf_xres;
		mT2p->pdf_imagelength = 
			((float)(mT2p->tiff_length))*PS_UNIT_SIZE/mT2p->pdf_yres;
	}
	mT2p->pdf_pagewidth = mT2p->pdf_imagewidth;
	mT2p->pdf_pagelength = mT2p->pdf_imagelength;
	mT2p->pdf_mediabox.x1=0.0;
	mT2p->pdf_mediabox.y1=0.0;
	mT2p->pdf_mediabox.x2=mT2p->pdf_pagewidth;
	mT2p->pdf_mediabox.y2=mT2p->pdf_pagelength;
	mT2p->pdf_imagebox.x1=0.0;
	mT2p->pdf_imagebox.y1=0.0;
	mT2p->pdf_imagebox.x2=mT2p->pdf_imagewidth;
	mT2p->pdf_imagebox.y2=mT2p->pdf_imagelength;
	if(mT2p->tiff_orientation > 4)
	{
		f=mT2p->pdf_mediabox.x2;
		mT2p->pdf_mediabox.x2=mT2p->pdf_mediabox.y2;
		mT2p->pdf_mediabox.y2=f;
	}
	istiled=((mT2p->tiff_tiles[mT2p->pdf_page]).tiles_tilecount==0) ? 0 : 1;
	if(istiled==0)
	{
		ComposePDFPageOrient(&(mT2p->pdf_imagebox), mT2p->tiff_orientation);
		return;
	} 
	else 
	{
		tilewidth=(mT2p->tiff_tiles[mT2p->pdf_page]).tiles_tilewidth;
		tilelength=(mT2p->tiff_tiles[mT2p->pdf_page]).tiles_tilelength;
		tilecountx=(mT2p->tiff_width + 
			tilewidth -1)/ 
			tilewidth;
		(mT2p->tiff_tiles[mT2p->pdf_page]).tiles_tilecountx=tilecountx;
		tilecounty=(mT2p->tiff_length + 
			tilelength -1)/ 
			tilelength;
		(mT2p->tiff_tiles[mT2p->pdf_page]).tiles_tilecounty=tilecounty;
		(mT2p->tiff_tiles[mT2p->pdf_page]).tiles_edgetilewidth=
			mT2p->tiff_width % tilewidth;
		(mT2p->tiff_tiles[mT2p->pdf_page]).tiles_edgetilelength=
			mT2p->tiff_length % tilelength;
		tiles=(mT2p->tiff_tiles[mT2p->pdf_page]).tiles_tiles;
		for(i2=0;i2<tilecounty-1;i2++)
		{
			for(i=0;i<tilecountx-1;i++)
			{
				boxp=&(tiles[i2*tilecountx+i].tile_box);
				boxp->x1 = 
					mT2p->pdf_imagebox.x1 
					+ ((float)(mT2p->pdf_imagewidth * i * tilewidth)
					/ (float)mT2p->tiff_width);
				boxp->x2 = 
					mT2p->pdf_imagebox.x1 
					+ ((float)(mT2p->pdf_imagewidth * (i+1) * tilewidth)
					/ (float)mT2p->tiff_width);
				boxp->y1 = 
					mT2p->pdf_imagebox.y2 
					- ((float)(mT2p->pdf_imagelength * (i2+1) * tilelength)
					/ (float)mT2p->tiff_length);
				boxp->y2 = 
					mT2p->pdf_imagebox.y2 
					- ((float)(mT2p->pdf_imagelength * i2 * tilelength)
					/ (float)mT2p->tiff_length);
			}
			boxp=&(tiles[i2*tilecountx+i].tile_box);
			boxp->x1 = 
				mT2p->pdf_imagebox.x1 
				+ ((float)(mT2p->pdf_imagewidth * i * tilewidth)
				/ (float)mT2p->tiff_width);
			boxp->x2 = mT2p->pdf_imagebox.x2;
			boxp->y1 = 
				mT2p->pdf_imagebox.y2 
				- ((float)(mT2p->pdf_imagelength * (i2+1) * tilelength)
				/ (float)mT2p->tiff_length);
			boxp->y2 = 
				mT2p->pdf_imagebox.y2 
				- ((float)(mT2p->pdf_imagelength * i2 * tilelength)
				/ (float)mT2p->tiff_length);
		}
		for(i=0;i<tilecountx-1;i++)
		{
			boxp=&(tiles[i2*tilecountx+i].tile_box);
			boxp->x1 = 
				mT2p->pdf_imagebox.x1 
				+ ((float)(mT2p->pdf_imagewidth * i * tilewidth)
				/ (float)mT2p->tiff_width);
			boxp->x2 = 
				mT2p->pdf_imagebox.x1 
				+ ((float)(mT2p->pdf_imagewidth * (i+1) * tilewidth)
				/ (float)mT2p->tiff_width);
			boxp->y1 = mT2p->pdf_imagebox.y1;
			boxp->y2 = 
				mT2p->pdf_imagebox.y2 
				- ((float)(mT2p->pdf_imagelength * i2 * tilelength)
				/ (float)mT2p->tiff_length);
		}
		boxp=&(tiles[i2*tilecountx+i].tile_box);
		boxp->x1 = 
			mT2p->pdf_imagebox.x1 
			+ ((float)(mT2p->pdf_imagewidth * i * tilewidth)
			/ (float)mT2p->tiff_width);
		boxp->x2 = mT2p->pdf_imagebox.x2;
		boxp->y1 = mT2p->pdf_imagebox.y1;
		boxp->y2 = 
			mT2p->pdf_imagebox.y2 
			- ((float)(mT2p->pdf_imagelength * i2 * tilelength)
			/ (float)mT2p->tiff_length);
	}
	if(mT2p->tiff_orientation==0 || mT2p->tiff_orientation==1)
	{
		for(i=0;i<(mT2p->tiff_tiles[mT2p->pdf_page]).tiles_tilecount;i++)
		{
			ComposePDFPageOrient( &(tiles[i].tile_box) , 0); 
		}
		return;
	}
	for(i=0;i<(mT2p->tiff_tiles[mT2p->pdf_page]).tiles_tilecount;i++)
	{
		boxp=&(tiles[i].tile_box);
		boxp->x1 -= mT2p->pdf_imagebox.x1;
		boxp->x2 -= mT2p->pdf_imagebox.x1;
		boxp->y1 -= mT2p->pdf_imagebox.y1;
		boxp->y2 -= mT2p->pdf_imagebox.y1;
		if(mT2p->tiff_orientation==2 || mT2p->tiff_orientation==3)
		{
			boxp->x1 = mT2p->pdf_imagebox.x2 - mT2p->pdf_imagebox.x1 - boxp->x1;
			boxp->x2 = mT2p->pdf_imagebox.x2 - mT2p->pdf_imagebox.x1 - boxp->x2;
		}
		if(mT2p->tiff_orientation==3 || mT2p->tiff_orientation==4)
		{
			boxp->y1 = mT2p->pdf_imagebox.y2 - mT2p->pdf_imagebox.y1 - boxp->y1;
			boxp->y2 = mT2p->pdf_imagebox.y2 - mT2p->pdf_imagebox.y1 - boxp->y2;
		}
		if(mT2p->tiff_orientation==8 || mT2p->tiff_orientation==5)
		{
			boxp->y1 = mT2p->pdf_imagebox.y2 - mT2p->pdf_imagebox.y1 - boxp->y1;
			boxp->y2 = mT2p->pdf_imagebox.y2 - mT2p->pdf_imagebox.y1 - boxp->y2;
		}
		if(mT2p->tiff_orientation==5 || mT2p->tiff_orientation==6)
		{
			boxp->x1 = mT2p->pdf_imagebox.x2 - mT2p->pdf_imagebox.x1 - boxp->x1;
			boxp->x2 = mT2p->pdf_imagebox.x2 - mT2p->pdf_imagebox.x1 - boxp->x2;
		}
		if(mT2p->tiff_orientation > 4)
		{
			f=boxp->x1;
			boxp->x1 = boxp->y1;
			boxp->y1 = f;
			f=boxp->x2;
			boxp->x2 = boxp->y2;
			boxp->y2 = f; 
			ComposePDFPageOrientFlip(boxp, mT2p->tiff_orientation); 
		} 
		else 
		{
			ComposePDFPageOrient(boxp, mT2p->tiff_orientation); 
		}
		
	}

	return;
}

void TIFFImageHandler::ComposePDFPageOrient(T2P_BOX* boxp, uint16_t orientation)
{

	float m1[9];
	float f=0.0;
	
	if( boxp->x1 > boxp->x2)
	{
		f=boxp->x1;
		boxp->x1=boxp->x2;
		boxp->x2 = f;
	}
	if( boxp->y1 > boxp->y2)
	{
		f=boxp->y1;
		boxp->y1=boxp->y2;
		boxp->y2 = f;
	}
	boxp->mat[0]=m1[0]=boxp->x2-boxp->x1;
	boxp->mat[1]=m1[1]=0.0;
	boxp->mat[2]=m1[2]=0.0;
	boxp->mat[3]=m1[3]=0.0;
	boxp->mat[4]=m1[4]=boxp->y2-boxp->y1;
	boxp->mat[5]=m1[5]=0.0;
	boxp->mat[6]=m1[6]=boxp->x1;
	boxp->mat[7]=m1[7]=boxp->y1;
	boxp->mat[8]=m1[8]=1.0;
	switch(orientation)
	{
		case 0:
		case 1:
			break;
		case 2:
			boxp->mat[0]=0.0F-m1[0];
			boxp->mat[6]+=m1[0];
			break;
		case 3:
			boxp->mat[0]=0.0F-m1[0];
			boxp->mat[4]=0.0F-m1[4];
			boxp->mat[6]+=m1[0];
			boxp->mat[7]+=m1[4];
			break;
		case 4:
			boxp->mat[4]=0.0F-m1[4];
			boxp->mat[7]+=m1[4];
			break;
		case 5:
			boxp->mat[0]=0.0F;
			boxp->mat[1]=0.0F-m1[0];
			boxp->mat[3]=0.0F-m1[4];
			boxp->mat[4]=0.0F;
			boxp->mat[6]+=m1[4];
			boxp->mat[7]+=m1[0];
			break;
		case 6:
			boxp->mat[0]=0.0F;
			boxp->mat[1]=0.0F-m1[0];
			boxp->mat[3]=m1[4];
			boxp->mat[4]=0.0F;
			boxp->mat[7]+=m1[0];
			break;
		case 7:
			boxp->mat[0]=0.0F;
			boxp->mat[1]=m1[0];
			boxp->mat[3]=m1[4];
			boxp->mat[4]=0.0F;
			break;
		case 8:
			boxp->mat[0]=0.0F;
			boxp->mat[1]=m1[0];
			boxp->mat[3]=0.0F-m1[4];
			boxp->mat[4]=0.0F;
			boxp->mat[6]+=m1[4];
			break;
	}
}

void TIFFImageHandler::ComposePDFPageOrientFlip(T2P_BOX* boxp, uint16_t orientation)
{
	float m1[9];
	float f=0.0;
	
	if( boxp->x1 > boxp->x2)
	{
		f=boxp->x1;
		boxp->x1=boxp->x2;
		boxp->x2 = f;
	}
	if( boxp->y1 > boxp->y2)
	{
		f=boxp->y1;
		boxp->y1=boxp->y2;
		boxp->y2 = f;
	}
	boxp->mat[0]=m1[0]=boxp->x2-boxp->x1;
	boxp->mat[1]=m1[1]=0.0F;
	boxp->mat[2]=m1[2]=0.0F;
	boxp->mat[3]=m1[3]=0.0F;
	boxp->mat[4]=m1[4]=boxp->y2-boxp->y1;
	boxp->mat[5]=m1[5]=0.0F;
	boxp->mat[6]=m1[6]=boxp->x1;
	boxp->mat[7]=m1[7]=boxp->y1;
	boxp->mat[8]=m1[8]=1.0F;
	switch(orientation)
	{
		case 5:
			boxp->mat[0]=0.0F;
			boxp->mat[1]=0.0F-m1[4];
			boxp->mat[3]=0.0F-m1[0];
			boxp->mat[4]=0.0F;
			boxp->mat[6]+=m1[0];
			boxp->mat[7]+=m1[4];
			break;
		case 6:
			boxp->mat[0]=0.0F;
			boxp->mat[1]=0.0F-m1[4];
			boxp->mat[3]=m1[0];
			boxp->mat[4]=0.0F;
			boxp->mat[7]+=m1[4];
			break;
		case 7:
			boxp->mat[0]=0.0F;
			boxp->mat[1]=m1[4];
			boxp->mat[3]=m1[0];
			boxp->mat[4]=0.0F;
			break;
		case 8:
			boxp->mat[0]=0.0F;
			boxp->mat[1]=m1[4];
			boxp->mat[3]=0.0F-m1[0];
			boxp->mat[4]=0.0F;
			boxp->mat[6]+=m1[0];
			break;
	}
}

static const std::string scFunctionType = "FunctionType";
static const std::string scDomain = "Domain";
static const std::string scRange = "Range";
static const std::string scSize = "Size";
static const std::string scBitsPerSample = "BitsPerSample";

ObjectIDType TIFFImageHandler::WriteTransferFunction(int i)
{
	ObjectIDType transferFunctionID = mObjectsContext->StartNewIndirectObject();
	DictionaryContext* transferFunctionDictionary = mObjectsContext->StartDictionary();
	
	// Function Type
	transferFunctionDictionary->WriteKey(scFunctionType);
	transferFunctionDictionary->WriteIntegerValue(0);

	// Domain
	transferFunctionDictionary->WriteKey(scDomain);
	mObjectsContext->StartArray();
	mObjectsContext->WriteDouble(0.0);
	mObjectsContext->WriteDouble(1.0);
	mObjectsContext->EndArray(eTokenSeparatorEndLine);

	// Range
	transferFunctionDictionary->WriteKey(scRange);
	mObjectsContext->StartArray();
	mObjectsContext->WriteDouble(0.0);
	mObjectsContext->WriteDouble(1.0);
	mObjectsContext->EndArray(eTokenSeparatorEndLine);

	// Size
	transferFunctionDictionary->WriteKey(scSize);
	transferFunctionDictionary->WriteIntegerValue(1<<mT2p->tiff_bitspersample);

	// BitsPerSample
	transferFunctionDictionary->WriteKey(scBitsPerSample);
	transferFunctionDictionary->WriteIntegerValue(1<<(mT2p->tiff_bitspersample+1));

	// the stream
	PDFStream* transferFunctionStream =  mObjectsContext->StartPDFStream(transferFunctionDictionary);
	transferFunctionStream->GetWriteStream()->Write(
				(const IOBasicTypes::Byte*)mT2p->tiff_transferfunction[i],
				(1<<(mT2p->tiff_bitspersample+1)));
	mObjectsContext->EndPDFStream(transferFunctionStream);
	delete transferFunctionStream;
	return transferFunctionID;
}

static const std::string scType = "Type";
static const std::string scExtGState = "ExtGState";
static const std::string scIdentity = "Identity";
static const std::string scTR = "TR";

ObjectIDType TIFFImageHandler::WriteTransferFunctionsExtGState(const ObjectIDTypeList& inTransferFunctions)
{
	ObjectIDType gstateID = mObjectsContext->StartNewIndirectObject();
	DictionaryContext* gstateDictionary = mObjectsContext->StartDictionary();
	
	// Type
	gstateDictionary->WriteKey(scType);
	gstateDictionary->WriteNameValue(scExtGState);

	// TR
	gstateDictionary->WriteKey(scTR);
	if(inTransferFunctions.size() == 1)
	{
		gstateDictionary->WriteNewObjectReferenceValue(inTransferFunctions.back());
	}
	else
	{
		mObjectsContext->StartArray();
		ObjectIDTypeList::const_iterator it = inTransferFunctions.begin();
		for(;it != inTransferFunctions.end();++it)
			mObjectsContext->WriteNewIndirectObjectReference(*it);				
		mObjectsContext->WriteName(scIdentity);		
		mObjectsContext->EndArray(eTokenSeparatorEndLine);
	}

	mObjectsContext->EndDictionary(gstateDictionary);
	mObjectsContext->EndIndirectObject();
	return gstateID;
}

ObjectIDType TIFFImageHandler::WritePaletteCS()
{
	ObjectIDType palleteID = mObjectsContext->StartNewIndirectObject();
	PDFStream* paletteStream =  mObjectsContext->StartPDFStream();
	paletteStream->GetWriteStream()->Write(
			(const IOBasicTypes::Byte*)mT2p->pdf_palette,mT2p->pdf_palettesize);
	mObjectsContext->EndPDFStream(paletteStream);
	delete paletteStream;
	return palleteID;
}

static const std::string scN = "N";
static const std::string scAlternate = "Alternate";

ObjectIDType TIFFImageHandler::WriteICCCS()
{
	ObjectIDType ICCID = mObjectsContext->StartNewIndirectObject();
	DictionaryContext* ICCDictionary = mObjectsContext->StartDictionary();
	
	// N
	ICCDictionary->WriteKey(scN);
	ICCDictionary->WriteIntegerValue(mT2p->tiff_samplesperpixel);

	// Alternate
	ICCDictionary->WriteKey(scAlternate);
	
	// NOW Write the CS
	mT2p->pdf_colorspace = (t2p_cs_t)(mT2p->pdf_colorspace ^ T2P_CS_ICCBASED);
	WriteXObjectCS(ICCDictionary);
	mT2p->pdf_colorspace = (t2p_cs_t)(mT2p->pdf_colorspace | T2P_CS_ICCBASED);

	// the stream
	PDFStream* ICCStream =  mObjectsContext->StartPDFStream(ICCDictionary);
	ICCStream->GetWriteStream()->Write(
				(const IOBasicTypes::Byte*)mT2p->tiff_iccprofile,
				mT2p->tiff_iccprofilelength);
	mObjectsContext->EndPDFStream(ICCStream);
	delete ICCStream;
	return ICCID;	
}

static const std::string scICCBased = "ICCBased";
static const std::string scIndexed = "Indexed";
static const std::string scDeviceGray = "DeviceGray";
static const std::string scDeviceRGB = "DeviceRGB";
static const std::string scDeviceCMYK = "DeviceCMYK";
static const std::string scLab = "Lab";
static const std::string scWhitePoint = "WhitePoint";

void TIFFImageHandler::WriteXObjectCS(DictionaryContext* inContainerDictionary)
{
	float X_W=1.0;
	float Y_W=1.0;
	float Z_W=1.0;
	
	if( (mT2p->pdf_colorspace & T2P_CS_ICCBASED) != 0)
	{
		mObjectsContext->StartArray();
		mObjectsContext->WriteName(scICCBased);
		mObjectsContext->WriteNewIndirectObjectReference(mT2p->pdf_icccs);
		mObjectsContext->EndArray(eTokenSeparatorEndLine);
		return;
	}

	if( (mT2p->pdf_colorspace & T2P_CS_PALETTE) != 0)
	{
		mObjectsContext->StartArray();
		mObjectsContext->WriteName(scIndexed);
		mT2p->pdf_colorspace = (t2p_cs_t)(mT2p->pdf_colorspace ^ T2P_CS_PALETTE);
		WriteXObjectCS(NULL);
		mT2p->pdf_colorspace = (t2p_cs_t)(mT2p->pdf_colorspace | T2P_CS_PALETTE);
		mObjectsContext->WriteInteger((0x0001 << mT2p->tiff_bitspersample)-1);
		mObjectsContext->WriteNewIndirectObjectReference(mT2p->pdf_palettecs);
		mObjectsContext->EndArray(eTokenSeparatorEndLine);
		return;
	}

	if((mT2p->pdf_colorspace & T2P_CS_BILEVEL) && !mUserParameters.BWTreatment.AsImageMask)
	{
		if(inContainerDictionary)
			inContainerDictionary->WriteNameValue(scDeviceGray);
		else
			mObjectsContext->WriteName(scDeviceGray);
	}
		
	if(mT2p->pdf_colorspace & T2P_CS_GRAY)
	{
		if(mT2p->pdf_colorspace & T2P_CS_CALGRAY)
		{
			WriteXObjectCALCS();
		} 
		else 
		{
			if(mUserParameters.GrayscaleTreatment.AsColorMap)
			{
				WriteIndexedCSForBiLevelColorMap();
			}
			else
			{
				if(inContainerDictionary)
					inContainerDictionary->WriteNameValue(scDeviceGray);
				else
					mObjectsContext->WriteName(scDeviceGray);
			}
		}
	}

	if(mT2p->pdf_colorspace & T2P_CS_RGB)
	{
		if(mT2p->pdf_colorspace & T2P_CS_CALRGB)
		{
			WriteXObjectCALCS();
		} 
		else 
		{
			if(inContainerDictionary)
				inContainerDictionary->WriteNameValue(scDeviceRGB);
			else
				mObjectsContext->WriteName(scDeviceRGB);
		}
	}

	if(mT2p->pdf_colorspace & T2P_CS_CMYK)
	{
		if(inContainerDictionary)
			inContainerDictionary->WriteNameValue(scDeviceCMYK);
		else
			mObjectsContext->WriteName(scDeviceCMYK);
	}

	if(mT2p->pdf_colorspace & T2P_CS_LAB)
	{
		mObjectsContext->StartArray();
		mObjectsContext->WriteName(scLab);

		DictionaryContext* labDictionary = mObjectsContext->StartDictionary();
		labDictionary->WriteKey(scWhitePoint);
		X_W = mT2p->tiff_whitechromaticities[0];
		Y_W = mT2p->tiff_whitechromaticities[1];
		Z_W = 1.0F - (X_W + Y_W);
		X_W /= Y_W;
		Z_W /= Y_W;
		Y_W = 1.0F;
		mObjectsContext->StartArray();
		mObjectsContext->WriteDouble(X_W);
		mObjectsContext->WriteDouble(Y_W);
		mObjectsContext->WriteDouble(Z_W);
		mObjectsContext->EndArray();
		
		X_W = 0.3457F; /* 0.3127F; */ /* D50, commented D65 */
		Y_W = 0.3585F; /* 0.3290F; */
		Z_W = 1.0F - (X_W + Y_W);
		X_W /= Y_W;
		Z_W /= Y_W;
		Y_W = 1.0F;
		mObjectsContext->StartArray();
		mObjectsContext->WriteDouble(X_W);
		mObjectsContext->WriteDouble(Y_W);
		mObjectsContext->WriteDouble(Z_W);
		mObjectsContext->EndArray(eTokenSeparatorEndLine);

		labDictionary->WriteKey(scRange);
		mObjectsContext->StartArray();
		mObjectsContext->WriteInteger(mT2p->pdf_labrange[0]);
		mObjectsContext->WriteInteger(mT2p->pdf_labrange[1]);
		mObjectsContext->WriteInteger(mT2p->pdf_labrange[2]);
		mObjectsContext->WriteInteger(mT2p->pdf_labrange[3]);
		mObjectsContext->EndArray(eTokenSeparatorEndLine);
		
		mObjectsContext->EndDictionary(labDictionary);
		mObjectsContext->EndArray(eTokenSeparatorEndLine);
			
	}
}

static const std::string scCalGray = "CalGray";
static const std::string scCalRGB = "CalRGB";
static const std::string scGamma = "Gamma";
static const std::string scMatrix = "Matrix";
void TIFFImageHandler::WriteXObjectCALCS()
{

	float X_W=0.0;
	float Y_W=0.0;
	float Z_W=0.0;
	float X_R=0.0;
	float Y_R=0.0;
	float Z_R=0.0;
	float X_G=0.0;
	float Y_G=0.0;
	float Z_G=0.0;
	float X_B=0.0;
	float Y_B=0.0;
	float Z_B=0.0;
	float x_w=0.0;
	float y_w=0.0;
	float z_w=0.0;
	float x_r=0.0;
	float y_r=0.0;
	float x_g=0.0;
	float y_g=0.0;
	float x_b=0.0;
	float y_b=0.0;
	float R=1.0;
	float G=1.0;
	float B=1.0;

	mObjectsContext->StartArray();

	if(mT2p->pdf_colorspace & T2P_CS_CALGRAY)
	{
		mObjectsContext->WriteName(scCalGray);
		X_W = mT2p->tiff_whitechromaticities[0];
		Y_W = mT2p->tiff_whitechromaticities[1];
		Z_W = 1.0F - (X_W + Y_W);
		X_W /= Y_W;
		Z_W /= Y_W;
		Y_W = 1.0F;
	}

	if(mT2p->pdf_colorspace & T2P_CS_CALRGB)
	{
		mObjectsContext->WriteName(scCalRGB);
		x_w = mT2p->tiff_whitechromaticities[0];
		y_w = mT2p->tiff_whitechromaticities[1];
		x_r = mT2p->tiff_primarychromaticities[0];
		y_r = mT2p->tiff_primarychromaticities[1];
		x_g = mT2p->tiff_primarychromaticities[2];
		y_g = mT2p->tiff_primarychromaticities[3];
		x_b = mT2p->tiff_primarychromaticities[4];
		y_b = mT2p->tiff_primarychromaticities[5];
		z_w = y_w * ((x_g - x_b)*y_r - (x_r-x_b)*y_g + (x_r-x_g)*y_b);
		Y_R = (y_r/R) * ((x_g-x_b)*y_w - (x_w-x_b)*y_g + (x_w-x_g)*y_b) / z_w;
		X_R = Y_R * x_r / y_r;
		Z_R = Y_R * (((1-x_r)/y_r)-1);
		Y_G = ((0.0F-(y_g))/G) * ((x_r-x_b)*y_w - (x_w-x_b)*y_r + (x_w-x_r)*y_b) / z_w;
		X_G = Y_G * x_g / y_g;
		Z_G = Y_G * (((1-x_g)/y_g)-1);
		Y_B = (y_b/B) * ((x_r-x_g)*y_w - (x_w-x_g)*y_r + (x_w-x_r)*y_g) / z_w;
		X_B = Y_B * x_b / y_b;
		Z_B = Y_B * (((1-x_b)/y_b)-1);
		X_W = (X_R * R) + (X_G * G) + (X_B * B);
		Y_W = (Y_R * R) + (Y_G * G) + (Y_B * B);
		Z_W = (Z_R * R) + (Z_G * G) + (Z_B * B);
		X_W /= Y_W;
		Z_W /= Y_W;
		Y_W = 1.0;
	}

	DictionaryContext* calbiratedCSDictionary = mObjectsContext->StartDictionary();

	if(mT2p->pdf_colorspace & T2P_CS_CALGRAY)
	{
		calbiratedCSDictionary->WriteKey(scWhitePoint);
		mObjectsContext->StartArray();
		mObjectsContext->WriteDouble(X_W);
		mObjectsContext->WriteDouble(Y_W);
		mObjectsContext->WriteDouble(Z_W);
		mObjectsContext->EndArray(eTokenSeparatorEndLine);
		calbiratedCSDictionary->WriteKey(scGamma);
		calbiratedCSDictionary->WriteDoubleValue(2.2);
	}

	if(mT2p->pdf_colorspace & T2P_CS_CALRGB)
	{
		calbiratedCSDictionary->WriteKey(scWhitePoint);
		mObjectsContext->StartArray();
		mObjectsContext->WriteDouble(X_W);
		mObjectsContext->WriteDouble(Y_W);
		mObjectsContext->WriteDouble(Z_W);
		mObjectsContext->EndArray(eTokenSeparatorEndLine);

		calbiratedCSDictionary->WriteKey(scMatrix);
		mObjectsContext->StartArray();
		mObjectsContext->WriteDouble(X_R);
		mObjectsContext->WriteDouble(Y_R);
		mObjectsContext->WriteDouble(Z_R);
		mObjectsContext->WriteDouble(X_G);
		mObjectsContext->WriteDouble(Y_G);
		mObjectsContext->WriteDouble(Z_G);
		mObjectsContext->WriteDouble(X_B);
		mObjectsContext->WriteDouble(Y_B);
		mObjectsContext->WriteDouble(Z_B);
		mObjectsContext->EndArray(eTokenSeparatorEndLine);

		calbiratedCSDictionary->WriteKey(scGamma);
		mObjectsContext->StartArray();
		mObjectsContext->WriteDouble(2.2);
		mObjectsContext->WriteDouble(2.2);
		mObjectsContext->WriteDouble(2.2);
		mObjectsContext->EndArray(eTokenSeparatorEndLine);
	}

	mObjectsContext->EndDictionary(calbiratedCSDictionary);
	mObjectsContext->EndArray(eTokenSeparatorEndLine);
}

static const std::string scXObject = "XObject";
static const std::string scSubType = "Subtype";
static const std::string scImage = "Image";
static const std::string scWidth = "Width";
static const std::string scHeight = "Height";
static const std::string scName = "Name";
static const std::string scBitsPerComponent = "BitsPerComponent";
static const std::string scColorSpace = "ColorSpace";
static const std::string scInterpolate = "Interpolate";
PDFImageXObject* TIFFImageHandler::WriteTileImageXObject(int inTileIndex)
{
	PDFImageXObject* imageXObject = NULL;
	PDFStream* imageStream = NULL;

	do
	{
		ObjectIDType imageXObjectID = mObjectsContext->StartNewIndirectObject();
		DictionaryContext* imageContext = mObjectsContext->StartDictionary();
	
		WriteCommonImageDictionaryProperties(imageContext);

		// width
		imageContext->WriteKey(scWidth);
		imageContext->WriteIntegerValue(
			TileIsRightEdge(inTileIndex) ?
			mT2p->tiff_tiles[mT2p->pdf_page].tiles_edgetilewidth :
			mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilewidth);

		// height
		imageContext->WriteKey(scHeight);
		imageContext->WriteIntegerValue(
			TileIsBottomEdge(inTileIndex) ?
			mT2p->tiff_tiles[mT2p->pdf_page].tiles_edgetilelength :
			mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilelength);

		// filter
		WriteImageXObjectFilter(imageContext,inTileIndex);

		if(mExtender)
		{
			if(mExtender->OnTIFFImageXObjectWrite(imageXObjectID,imageContext,mObjectsContext,mContainerDocumentContext,this) != PDFHummus::eSuccess)
			{
				TRACE_LOG("TIFFImageHandler::WriteTileImageXObject, unexpected failure. extender declared failure when writing image xobject.");
				break;
			}
		}	

		imageStream = mObjectsContext->StartUnfilteredPDFStream(imageContext);

		CalculateTiffTileSize(inTileIndex);

		if(WriteImageTileData(imageStream,inTileIndex) != PDFHummus::eSuccess)
			break;

		mObjectsContext->EndPDFStream(imageStream);

		// Creating Image XObject procset value is dummy, because resources  on the 
		// container will be set externally.
		imageXObject = new PDFImageXObject(imageXObjectID);
		AddImagesProcsets(imageXObject);
	} while(false);


	delete imageStream;
	return imageXObject;
}

bool TIFFImageHandler::TileIsRightEdge(int inTileIndex)
{
	return(((inTileIndex + 1) % mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilecountx == 0)
			&&
			mT2p->tiff_tiles[mT2p->pdf_page].tiles_edgetilewidth != 0);
}


bool TIFFImageHandler::TileIsBottomEdge(int inTileIndex)
{
	return(((inTileIndex + 1)> (int)(
				mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilecount -
				mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilecountx))
			&&
			mT2p->tiff_tiles[mT2p->pdf_page].tiles_edgetilelength != 0);	
}

static const std::string scDecode = "Decode";
void TIFFImageHandler::WriteImageXObjectDecode(DictionaryContext* inImageDictionary)
{
	// Decode

	inImageDictionary->WriteKey(scDecode);
	mObjectsContext->StartArray();
	if((mT2p->pdf_colorspace & T2P_CS_GRAY) && 
		(!(mT2p->pdf_colorspace & T2P_CS_CALGRAY)) &&
		mUserParameters.GrayscaleTreatment.AsColorMap)

	{
		unsigned long colorSpaceHighValue = (0x0001 << mT2p->tiff_bitspersample) - 1;
		if(PHOTOMETRIC_MINISWHITE == mT2p->tiff_photometric)
		{
			for(int i=0;i<mT2p->tiff_samplesperpixel;++i)
			{
				mObjectsContext->WriteInteger(colorSpaceHighValue);
				mObjectsContext->WriteInteger(0);
			}
		}
		else
		{
			for(int i=0;i<mT2p->tiff_samplesperpixel;++i)
			{
				mObjectsContext->WriteInteger(0);
				mObjectsContext->WriteInteger(colorSpaceHighValue);
			}

		}
	}
	else
	{
		for(int i=0;i<mT2p->tiff_samplesperpixel;++i)
		{
			mObjectsContext->WriteInteger(1);
			mObjectsContext->WriteInteger(0);
		}
	}
	mObjectsContext->EndArray(eTokenSeparatorEndLine);
}

static const std::string scFilter = "Filter";
static const std::string scCCITTFaxDecode = "CCITTFaxDecode";
static const std::string scDecodeParms = "DecodeParms";
static const std::string scK = "K";
static const std::string scColumns = "Columns";
static const std::string scRows = "Rows";
static const std::string scBlackIs1 = "BlackIs1";
static const std::string scFlateDecode = "FlateDecode";
static const std::string scPredictor = "Predictor";
static const std::string scColors = "Colors";


// tile index will be ignored if not tiles exist for this image
void TIFFImageHandler::WriteImageXObjectFilter(DictionaryContext* inImageDictionary,int inTileIndex)
{
	DictionaryContext* decodeParmsDictionary;

	if(mT2p->pdf_compression==T2P_COMPRESS_NONE)
		return;

	// Filter
	inImageDictionary->WriteKey(scFilter);


	switch(mT2p->pdf_compression)
	{
		case T2P_COMPRESS_G4:
			inImageDictionary->WriteNameValue(scCCITTFaxDecode);

			// DecodeParms
			inImageDictionary->WriteKey(scDecodeParms);
			decodeParmsDictionary = mObjectsContext->StartDictionary();

			// K
			decodeParmsDictionary->WriteKey(scK);
			decodeParmsDictionary->WriteIntegerValue(-1);

			if(0 == mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilecount)
			{
				// Columns
				decodeParmsDictionary->WriteKey(scColumns);
				decodeParmsDictionary->WriteIntegerValue(mT2p->tiff_width);

				// Rows
				decodeParmsDictionary->WriteKey(scRows);
				decodeParmsDictionary->WriteIntegerValue(mT2p->tiff_length);
			}
			else
			{
				// Columns
				decodeParmsDictionary->WriteKey(scColumns);
				decodeParmsDictionary->WriteIntegerValue(
					TileIsRightEdge(inTileIndex) ? 
						mT2p->tiff_tiles[mT2p->pdf_page].tiles_edgetilewidth :
						mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilewidth
					);
			
				// Rows
				decodeParmsDictionary->WriteKey(scRows);
				decodeParmsDictionary->WriteIntegerValue(
					TileIsBottomEdge(inTileIndex) ? 
						mT2p->tiff_tiles[mT2p->pdf_page].tiles_edgetilelength :
						mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilelength
					);
			}

			if(mT2p->pdf_switchdecode == 0)
			{
				// BlackIs1
				decodeParmsDictionary->WriteKey(scBlackIs1);
				decodeParmsDictionary->WriteBooleanValue(true);
			}
			mObjectsContext->EndDictionary(decodeParmsDictionary);
			break;
		case T2P_COMPRESS_ZIP:
			inImageDictionary->WriteNameValue(scFlateDecode);

			if(mT2p->pdf_compressionquality%100)
			{
				// DecodeParms
				inImageDictionary->WriteKey(scDecodeParms);
				decodeParmsDictionary = mObjectsContext->StartDictionary();

				// Predictor
				decodeParmsDictionary->WriteKey(scPredictor);
				decodeParmsDictionary->WriteIntegerValue(mT2p->pdf_compressionquality%100);

				// Columns
				decodeParmsDictionary->WriteKey(scColumns);
				decodeParmsDictionary->WriteIntegerValue(mT2p->tiff_width);

				// Colors
				decodeParmsDictionary->WriteKey(scColors);
				decodeParmsDictionary->WriteIntegerValue(mT2p->tiff_samplesperpixel);

				// BitsPerComponent
				decodeParmsDictionary->WriteKey(scBitsPerComponent);
				decodeParmsDictionary->WriteIntegerValue(mT2p->tiff_bitspersample);

				mObjectsContext->EndDictionary(decodeParmsDictionary);
			}
			break;
        default:
            // do nothing
            break;
	}
}

void TIFFImageHandler::CalculateTiffTileSize(int inTileIndex)
{
	uint16_t edge=0;

	edge |= (uint16_t)TileIsRightEdge(inTileIndex);
	edge |= (uint16_t)TileIsBottomEdge(inTileIndex);
	
	if(mT2p->pdf_transcode==T2P_TRANSCODE_RAW)
	{
		if(edge)
		{
			mT2p->tiff_datasize=TIFFTileSize(mT2p->input);
		} else 
		{
			// 	TIFFTAG_TILEBYTECOUNTS changed in tiff 4.0.0;

			tsize_t_compat* tbc = NULL;
			TIFFGetField(mT2p->input, TIFFTAG_TILEBYTECOUNTS, &tbc);
			mT2p->tiff_datasize=static_cast<tsize_t>(tbc[inTileIndex]);
		}
	}
	else
	{
		mT2p->tiff_datasize=TIFFTileSize(mT2p->input);
		if(mT2p->tiff_planar==PLANARCONFIG_SEPARATE)
			mT2p->tiff_datasize*= mT2p->tiff_samplesperpixel;
	}
}


static tsize_t 
t2p_readproc(thandle_t handle, tdata_t data, tsize_t size) 
{
	(void) handle, (void) data, (void) size;
	return -1;
}

static tsize_t 
t2p_writeproc(thandle_t handle, tdata_t data, tsize_t size) 
{
	T2P *t2p = (T2P*) handle;
	if (t2p->pdfStream) 
	{
		tsize_t written = (tsize_t)t2p->pdfStream->GetWriteStream()->Write((const IOBasicTypes::Byte*)data,size);
		return written;
	}
	return size; 
}

static toff_t 
t2p_seekproc(thandle_t handle, toff_t offset, int whence) 
{ 
	(void) handle, (void) offset, (void) whence;
	return offset; 
}

static int 
t2p_closeproc(thandle_t handle)
{ 
	(void) handle;
	return 0; 
}

static toff_t 
t2p_sizeproc(thandle_t handle) 
{
	(void) handle;
	return -1;
}

static int 
t2p_mapproc(thandle_t handle, tdata_t *data, toff_t *offset) 
{ 
	(void) handle, (void) data, (void) offset;
	return -1; 
}

static void 
t2p_unmapproc(thandle_t handle, tdata_t data, toff_t offset)
{ 
	(void) handle, (void) data, (void) offset;
}

tsize_t GetSizeFromTIFFOutputStripSize(T2P* inT2p)
{
	return inT2p->output ? TIFFStripSize(inT2p->output) : 0;
	// Bug?: TIFFStripSize(output) looks like a bug to me...makes sense that strip size
	// is based on input...not output. try to see if i can check this

}

tsize_t GetSizeFromTIFFDataSize(T2P* inT2p)
{
	return inT2p->tiff_datasize;
}

EStatusCode TIFFImageHandler::WriteImageTileData(PDFStream* inImageStream,int inTileIndex)
{
	EStatusCode status = PDFHummus::eSuccess;
	uint16_t edge=0;
	unsigned char* buffer=NULL;

	tsize_t bufferoffset=0;
	unsigned char* samplebuffer=NULL;
	tsize_t samplebufferoffset=0;
	tsize_t read=0;
	uint16_t i=0;
	ttile_t tilecount=0;
	tsize_t tilesize=0;
	ttile_t septilecount=0;
	tsize_t septilesize=0;

	edge |= (uint16_t)TileIsRightEdge(inTileIndex);
	edge |= (uint16_t)TileIsBottomEdge(inTileIndex);

	do
	{
		// if recompression is not required, passthrough the image information and finish
		if((mT2p->pdf_transcode == T2P_TRANSCODE_RAW) && (edge == 0) &&
			(mT2p->pdf_compression == T2P_COMPRESS_G4 ||
				mT2p->pdf_compression == T2P_COMPRESS_ZIP))
		{
			buffer= (unsigned char*) _TIFFmalloc(mT2p->tiff_datasize);
			if(!buffer)
			{
				TRACE_LOG2(
					"TIFFImageHandler::WriteImageTileData, Can't allocate %u bytes of memory, for image %s",
					mT2p->tiff_datasize, 
					mT2p->inputFilePath.c_str());
				status = PDFHummus::eFailure;
				break;
			}
			TIFFReadRawTile(mT2p->input, inTileIndex, (tdata_t) buffer, mT2p->tiff_datasize);
			if (mT2p->tiff_fillorder==FILLORDER_LSB2MSB)
					TIFFReverseBits(buffer, mT2p->tiff_datasize);
			inImageStream->GetWriteStream()->Write(
									(const IOBasicTypes::Byte*)buffer,mT2p->tiff_datasize);
			_TIFFfree(buffer);
			break; // finish here if recompression is not required
		}

		if(mT2p->pdf_sample==T2P_SAMPLE_NOTHING)
		{
			buffer = (unsigned char*) _TIFFmalloc(mT2p->tiff_datasize);
			if(buffer==NULL)
			{
				TRACE_LOG2(
					"TIFFImageHandler::WriteImageTileData, Can't allocate %u bytes of memory, for image %s",
					mT2p->tiff_datasize, 
					mT2p->inputFilePath.c_str());
				status = PDFHummus::eFailure;
				break;
			}
			read = TIFFReadEncodedTile(
				mT2p->input, 
				inTileIndex, 
				(tdata_t) &buffer[bufferoffset], 
				mT2p->tiff_datasize);
			if(read==-1)
			{
				TRACE_LOG2(
					"TIFFImageHandler::WriteImageTileData, Error on decoding tile %u of %s",
					inTileIndex, 
					mT2p->inputFilePath.c_str());
				status = PDFHummus::eFailure;
				break;
			}
		} 
		else 
		{

			if(mT2p->pdf_sample == T2P_SAMPLE_PLANAR_SEPARATE_TO_CONTIG)
			{
				septilesize=TIFFTileSize(mT2p->input);
				septilecount=TIFFNumberOfTiles(mT2p->input);
				tilesize=septilesize*mT2p->tiff_samplesperpixel;
				tilecount=septilecount/mT2p->tiff_samplesperpixel;
				buffer = (unsigned char*) _TIFFmalloc(mT2p->tiff_datasize);
				if(buffer==NULL)
				{
					TRACE_LOG2(
						"TIFFImageHandler::WriteImageTileData, Can't allocate %u bytes of memory, for image %s",
						mT2p->tiff_datasize, 
						mT2p->inputFilePath.c_str());
					status = PDFHummus::eFailure;
					break;
				}
				samplebuffer = (unsigned char*) _TIFFmalloc(mT2p->tiff_datasize);
				if(samplebuffer==NULL)
				{
					TRACE_LOG2(
						"TIFFImageHandler::WriteImageTileData, Can't allocate %u bytes of memory, for image %s",
						mT2p->tiff_datasize, 
						mT2p->inputFilePath.c_str());
					status = PDFHummus::eFailure;
					break;
				}
				samplebufferoffset=0;
				for(i=0;i<mT2p->tiff_samplesperpixel;i++)
				{
					read = 
						TIFFReadEncodedTile(mT2p->input, 
							inTileIndex + i*tilecount, 
							(tdata_t) &(samplebuffer[samplebufferoffset]), 
							septilesize);
					if(read==-1)
					{
						TRACE_LOG2(
							"TIFFImageHandler::WriteImageTileData, Error on decoding tile %u of %s",
							inTileIndex + i*tilecount, 
							mT2p->inputFilePath.c_str());
						_TIFFfree(samplebuffer);
						_TIFFfree(buffer);
						status = PDFHummus::eFailure;
						break;
					}
					samplebufferoffset+=read;
				}
				SamplePlanarSeparateToContig(
					&(buffer[bufferoffset]),
					samplebuffer, 
					samplebufferoffset); 
				bufferoffset+=samplebufferoffset;
				_TIFFfree(samplebuffer);
			}

			if(buffer==NULL)
			{
				buffer = (unsigned char*) _TIFFmalloc(mT2p->tiff_datasize);
				if(buffer==NULL)
				{
					TRACE_LOG2(
						"TIFFImageHandler::WriteImageTileData, Can't allocate %u bytes of memory, for image %s",
						mT2p->tiff_datasize, 
						mT2p->inputFilePath.c_str());
					status = PDFHummus::eFailure;
					break;
				}
				read = TIFFReadEncodedTile(
					mT2p->input, 
					inTileIndex, 
					(tdata_t) &buffer[bufferoffset], 
					mT2p->tiff_datasize);
				if(read==-1)
				{
					TRACE_LOG2(
						"TIFFImageHandler::WriteImageTileData, Error on decoding tile %u of %s",
						inTileIndex, 
						mT2p->inputFilePath.c_str());
					_TIFFfree(buffer);
					status = PDFHummus::eFailure;
					break;
				}
			}

			if(mT2p->pdf_sample & T2P_SAMPLE_RGBA_TO_RGB)
			{
				mT2p->tiff_datasize=SampleRGBAToRGB(
					(tdata_t)buffer, 
					mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilewidth
					*mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilelength);
			}

			if(mT2p->pdf_sample & T2P_SAMPLE_RGBAA_TO_RGB)
			{
				mT2p->tiff_datasize=SampleRGBAAToRGB(
					(tdata_t)buffer, 
					mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilewidth
					*mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilelength);
			}

			if(mT2p->pdf_sample & T2P_SAMPLE_YCBCR_TO_RGB)
			{
				TRACE_LOG1("No support for YCbCr to RGB in tile for %s",mT2p->inputFilePath.c_str());
				status = PDFHummus::eFailure;
				break;
			}

			if(mT2p->pdf_sample & T2P_SAMPLE_LAB_SIGNED_TO_UNSIGNED)
			{
				mT2p->tiff_datasize=SampleLABSignedToUnsigned(
					(tdata_t)buffer, 
					mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilewidth
					*mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilelength);
			}
		}

		if(TileIsRightEdge(inTileIndex) != 0)
		{
			TileCollapseLeft(
				buffer, 
				TIFFTileRowSize(mT2p->input),
				mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilewidth,
				mT2p->tiff_tiles[mT2p->pdf_page].tiles_edgetilewidth, 
				mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilelength);
		}

		status = WriteImageBufferToStream(inImageStream,
										TileIsRightEdge(inTileIndex) ? 
											mT2p->tiff_tiles[mT2p->pdf_page].tiles_edgetilewidth :
											mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilewidth,
										TileIsBottomEdge(inTileIndex) ?
											mT2p->tiff_tiles[mT2p->pdf_page].tiles_edgetilelength :
											mT2p->tiff_tiles[mT2p->pdf_page].tiles_edgetilelength,
										 buffer,
										 GetSizeFromTIFFDataSize /*GetSizeFromTIFFOutputStripSize*/);
											
		if (buffer != NULL) 
		{
			_TIFFfree(buffer);
			buffer = NULL;
		}
	}while(false);
	return status;
}

void TIFFImageHandler::SamplePlanarSeparateToContig(unsigned char* inBuffer, 
													unsigned char* inSamplebuffer, 
													tsize_t inSamplebuffersize)
{
	tsize_t stride=0;
	tsize_t i=0;
	tsize_t j=0;
	
	stride=inSamplebuffersize/mT2p->tiff_samplesperpixel;
	for(i=0;i<stride;i++){
		for(j=0;j<mT2p->tiff_samplesperpixel;j++){
			inBuffer[i*mT2p->tiff_samplesperpixel + j] = inSamplebuffer[i + j*stride];
		}
	}
}

tsize_t TIFFImageHandler::SampleRGBAToRGB(tdata_t inData, uint32_t inSampleCount)
{
	uint32_t i = 0;
	uint8_t alpha = 0;
    uint8_t* theData = (uint8_t*)inData;
	
	for (i = 0; i < inSampleCount; i++) 
	{
        alpha = 255 - theData[i*4+3];
        
        theData[i*3] = theData[i*4] + alpha;
        theData[i*3+1] = theData[i*4+1] + alpha;
        theData[i*3+2] = theData[i*4+2] + alpha;
        
	}

	return (i * 3);
}

tsize_t TIFFImageHandler::SampleRGBAAToRGB(tdata_t inData, uint32_t inSampleCount)
{
	uint32_t i;
	
	for(i = 0; i < inSampleCount; i++)
		memcpy((uint8_t*)inData + i * 3, (uint8_t*)inData + i * 4, 3);

	return(i * 3);	
}

tsize_t	TIFFImageHandler::SampleLABSignedToUnsigned(tdata_t inBuffer, uint32_t inSampleCount)
{
	uint32_t i=0;

	for(i=0;i<inSampleCount;i++)
	{
		if( (((unsigned char*)inBuffer)[(i*3)+1] & 0x80) !=0)
		{
			((unsigned char*)inBuffer)[(i*3)+1] =
				(unsigned char)(0x80 + ((char*)inBuffer)[(i*3)+1]);
		} 
		else 
		{
			((unsigned char*)inBuffer)[(i*3)+1] |= 0x80;
		}
		if( (((unsigned char*)inBuffer)[(i*3)+2] & 0x80) !=0)
		{
			((unsigned char*)inBuffer)[(i*3)+2] =
				(unsigned char)(0x80 + ((char*)inBuffer)[(i*3)+2]);
		} 
		else 
		{
			((unsigned char*)inBuffer)[(i*3)+2] |= 0x80;
		}
	}
	return(inSampleCount*3);	
}

void TIFFImageHandler::TileCollapseLeft(tdata_t inBuffer, tsize_t inScanWidth, 
										uint32_t inTileWidth, uint32_t inEdgeTileWidth, 
										uint32_t inTileLength)
{
	tsize_t edgescanwidth=0;
	
	edgescanwidth = (inScanWidth * inEdgeTileWidth + (inTileWidth - 1))/ inTileWidth;
	for(uint32_t i=0;i<inTileLength;i++)
	{
		_TIFFmemcpy( 
			&(((char*)inBuffer)[edgescanwidth*i]), 
			&(((char*)inBuffer)[inScanWidth*i]), 
			edgescanwidth);
	}	
}

PDFImageXObject* TIFFImageHandler::WriteUntiledImageXObject()
{
	PDFImageXObject* imageXObject = NULL;
	PDFStream* imageStream = NULL;

	do
	{
		ObjectIDType imageXObjectID = mObjectsContext->StartNewIndirectObject();
		DictionaryContext* imageContext = mObjectsContext->StartDictionary();
	
		WriteCommonImageDictionaryProperties(imageContext);

		// width
		imageContext->WriteKey(scWidth);
		imageContext->WriteIntegerValue(mT2p->tiff_width);

		// height
		imageContext->WriteKey(scHeight);
		imageContext->WriteIntegerValue(mT2p->tiff_length);

		// filter
		WriteImageXObjectFilter(imageContext,0);


		if(mExtender)
		{
			if(mExtender->OnTIFFImageXObjectWrite(imageXObjectID,imageContext,mObjectsContext,mContainerDocumentContext,this) != PDFHummus::eSuccess)
			{
				TRACE_LOG("TIFFImageHandler::WriteTileImageXObject, unexpected failure. extender declared failure when writing image xobject.");
				break;
			}
		}	

		imageStream = mObjectsContext->StartUnfilteredPDFStream(imageContext);

		CalculateTiffSizeNoTiles();

		if(WriteImageData(imageStream) != PDFHummus::eSuccess)
			break;

		mObjectsContext->EndPDFStream(imageStream);

		// Creating Image XObject procset value is dummy, because resources  on the 
		// container will be set externally.
		imageXObject = new PDFImageXObject(imageXObjectID);
		AddImagesProcsets(imageXObject);
	} while(false);


	delete imageStream;
	return imageXObject;
	
}

static const std::string scImageMask = "ImageMask";
void TIFFImageHandler::WriteCommonImageDictionaryProperties(DictionaryContext* inImageContext)
{
	// type
	inImageContext->WriteKey(scType);
	inImageContext->WriteNameValue(scXObject);

	// subtype
	inImageContext->WriteKey(scSubType);
	inImageContext->WriteNameValue(scImage);

	// bits per component
	inImageContext->WriteKey(scBitsPerComponent);
	inImageContext->WriteIntegerValue(mT2p->tiff_bitspersample);

	// color space
	if(!(mT2p->pdf_colorspace & T2P_CS_BILEVEL) || !mUserParameters.BWTreatment.AsImageMask)
	{
		inImageContext->WriteKey(scColorSpace);
		WriteXObjectCS(inImageContext);
	}

	// interpolate
	if(mT2p->pdf_image_interpolate)
	{
		inImageContext->WriteKey(scInterpolate);
		inImageContext->WriteBooleanValue(true);
	}

	// for BW image masks, make this an image mask
	if(mUserParameters.BWTreatment.AsImageMask &&
		mT2p->pdf_colorspace == T2P_CS_BILEVEL)
	{
		inImageContext->WriteKey(scImageMask);
		inImageContext->WriteBooleanValue(true);
	}

	// decode
	if( (mT2p->pdf_switchdecode != 0)
		&& ! (mT2p->pdf_colorspace == T2P_CS_BILEVEL 
		&& mT2p->pdf_compression == T2P_COMPRESS_G4)
		)
	{
		WriteImageXObjectDecode(inImageContext);
	}
}

void TIFFImageHandler::CalculateTiffSizeNoTiles()
{
	if(mT2p->pdf_transcode == T2P_TRANSCODE_RAW && 
		(mT2p->pdf_compression == T2P_COMPRESS_G4 || mT2p->pdf_compression == T2P_COMPRESS_ZIP))
	{
		// TIFFTAG_STRIPBYTECOUNTS size changed in tiff 4.0.0
		tsize_t_compat * sbc = NULL;
		TIFFGetField(mT2p->input, TIFFTAG_STRIPBYTECOUNTS, &sbc);
		mT2p->tiff_datasize = static_cast<tsize_t>(sbc[0]);
	}
	else
	{
		mT2p->tiff_datasize=TIFFScanlineSize(mT2p->input) * mT2p->tiff_length;
		if(mT2p->tiff_planar==PLANARCONFIG_SEPARATE)
			mT2p->tiff_datasize*= mT2p->tiff_samplesperpixel;
	}
}




EStatusCode TIFFImageHandler::WriteImageData(PDFStream* inImageStream)
{
	unsigned char* buffer=NULL;
	unsigned char* samplebuffer=NULL;
	tsize_t bufferoffset=0;
	tsize_t samplebufferoffset=0;
	tsize_t read=0;
	tstrip_t i=0;
	tstrip_t j=0;
	tstrip_t stripcount=0;
	tsize_t stripsize=0;
	tsize_t sepstripcount=0;
	tsize_t sepstripsize=0;

	EStatusCode status = PDFHummus::eSuccess;

	do
	{
		if(mT2p->pdf_transcode == T2P_TRANSCODE_RAW)
		{
			if(mT2p->pdf_compression == T2P_COMPRESS_G4 ||
				mT2p->pdf_compression == T2P_COMPRESS_ZIP)
			{
				buffer = (unsigned char*)_TIFFmalloc(mT2p->tiff_datasize);
				if (!buffer) 
				{
					TRACE_LOG2( 
						"Can't allocate %u bytes of memory for t2p_readwrite_pdf_image, %s", 
						mT2p->tiff_datasize, 
						mT2p->inputFilePath.c_str());
					status = PDFHummus::eFailure;
					break;
				}
				TIFFReadRawStrip(mT2p->input, 0, (tdata_t)buffer,mT2p->tiff_datasize);
				if (mT2p->tiff_fillorder==FILLORDER_LSB2MSB)
					TIFFReverseBits(buffer,mT2p->tiff_datasize);
				inImageStream->GetWriteStream()->Write(
										(const IOBasicTypes::Byte*)buffer,mT2p->tiff_datasize);
				_TIFFfree(buffer);
				break; // stop here if can write directly with no recompression
			}
		}

		if(mT2p->pdf_sample==T2P_SAMPLE_NOTHING)
		{
			buffer = (unsigned char*) _TIFFmalloc(mT2p->tiff_datasize);
			if(buffer==NULL)
			{
				TRACE_LOG2( 
					"Can't allocate %u bytes of memory for t2p_readwrite_pdf_image, %s", 
					mT2p->tiff_datasize, 
					mT2p->inputFilePath.c_str());
				status = PDFHummus::eFailure;
				break;
			}
			memset(buffer, 0, mT2p->tiff_datasize);
			stripsize=TIFFStripSize(mT2p->input);
			stripcount=TIFFNumberOfStrips(mT2p->input);
			for(i=0;i<stripcount;i++)
			{
				read = 
					TIFFReadEncodedStrip(mT2p->input, 
					i, 
					(tdata_t) &buffer[bufferoffset], 
					stripsize);
				if(read==-1)
				{
					TRACE_LOG2( 
						"Error on decoding strip %u of %s", 
						i, 
						mT2p->inputFilePath.c_str());
					_TIFFfree(buffer);
					status = PDFHummus::eFailure;
					break;
				}
				bufferoffset+=read;
				stripsize = (stripsize > mT2p->tiff_datasize - bufferoffset) ? (mT2p->tiff_datasize - bufferoffset) : stripsize;
			}
			if(status != PDFHummus::eSuccess)
				break;
		} 
		else 
		{
			if(mT2p->pdf_sample & T2P_SAMPLE_PLANAR_SEPARATE_TO_CONTIG)
			{
			
				sepstripsize=TIFFStripSize(mT2p->input);
				sepstripcount=TIFFNumberOfStrips(mT2p->input);
			
				stripsize=sepstripsize*mT2p->tiff_samplesperpixel;
				stripcount=sepstripcount/mT2p->tiff_samplesperpixel;
				
				buffer = (unsigned char*) _TIFFmalloc(mT2p->tiff_datasize);
				if(buffer==NULL)
				{
					TRACE_LOG2( 
						"Can't allocate %u bytes of memory for t2p_readwrite_pdf_image, %s", 
						mT2p->tiff_datasize, 
						mT2p->inputFilePath.c_str());
					status = PDFHummus::eFailure;
					break;
				}
				memset(buffer, 0, mT2p->tiff_datasize);
				samplebuffer = (unsigned char*) _TIFFmalloc(stripsize);
				if(samplebuffer==NULL)
				{
					TRACE_LOG2( 
						"Can't allocate %u bytes of memory for t2p_readwrite_pdf_image, %s", 
						mT2p->tiff_datasize, 
						mT2p->inputFilePath.c_str());
					status = PDFHummus::eFailure;
					break;
				}
				for(i=0;i<stripcount;i++)
				{
					samplebufferoffset=0;
					for(j=0;j<mT2p->tiff_samplesperpixel;j++)
					{
						read = 
							TIFFReadEncodedStrip(mT2p->input, 
								i + j*stripcount, 
								(tdata_t) &(samplebuffer[samplebufferoffset]), 
								sepstripsize);
						if(read==-1)
						{
							TRACE_LOG2( 
								"Error on decoding strip %u of %s", 
								i + j*stripcount, 
								mT2p->inputFilePath.c_str());
								_TIFFfree(buffer);
							status = PDFHummus::eFailure;
							break;
						}
						samplebufferoffset+=read;
					}
					if(status != PDFHummus::eSuccess)
						break;
					SamplePlanarSeparateToContig(
						&(buffer[bufferoffset]),
						samplebuffer, 
						samplebufferoffset); 
					bufferoffset+=samplebufferoffset;
				}
				_TIFFfree(samplebuffer);
				if(status!= PDFHummus::eSuccess)
					break;
				status = WriteImageBufferToStream(inImageStream,
										 mT2p->tiff_width,
										 mT2p->tiff_length,
										 buffer,
										 GetSizeFromTIFFDataSize);
				if (buffer != NULL) 
				{
					_TIFFfree(buffer);
					buffer=NULL;
				}
				break; // finish here, hopefully successfully
			}

			buffer = (unsigned char*) _TIFFmalloc(mT2p->tiff_datasize);
			if(buffer==NULL)
			{
				TRACE_LOG2( 
					"Can't allocate %u bytes of memory for t2p_readwrite_pdf_image, %s", 
					mT2p->tiff_datasize, 
					mT2p->inputFilePath.c_str());
				status = PDFHummus::eFailure;
				break;
			}
			memset(buffer, 0, mT2p->tiff_datasize);
			stripsize=TIFFStripSize(mT2p->input);
			stripcount=TIFFNumberOfStrips(mT2p->input);
			for(i=0;i<stripcount;i++)
			{
				read = 
					TIFFReadEncodedStrip(mT2p->input, 
					i, 
					(tdata_t) &buffer[bufferoffset], 
					stripsize);
				if(read==-1)
				{
					TRACE_LOG2( 
						"Error on decoding strip %u of %s", 
						i, 
						mT2p->inputFilePath.c_str());
					_TIFFfree(samplebuffer);
					_TIFFfree(buffer);
					status = PDFHummus::eFailure;
					break;
				}
				bufferoffset+=read;
				stripsize = (stripsize > mT2p->tiff_datasize - bufferoffset) ? (mT2p->tiff_datasize - bufferoffset) : stripsize;
			}
			if(status != PDFHummus::eSuccess)
				break;

			if(mT2p->pdf_sample & T2P_SAMPLE_REALIZE_PALETTE)
			{
				samplebuffer=(unsigned char*)_TIFFrealloc( 
					(tdata_t) buffer, 
					mT2p->tiff_datasize * mT2p->tiff_samplesperpixel);
				if(samplebuffer==NULL)
				{
					TRACE_LOG2( 
						"Can't allocate %u bytes of memory for t2p_readwrite_pdf_image, %s", 
						mT2p->tiff_datasize, 
						mT2p->inputFilePath.c_str());
					status = PDFHummus::eFailure;
				  _TIFFfree(buffer);
				} 
				else 
				{
					buffer=samplebuffer;
					mT2p->tiff_datasize *= mT2p->tiff_samplesperpixel;
				}
				SampleRealizePalette(buffer);
			}

			if(mT2p->pdf_sample & T2P_SAMPLE_RGBA_TO_RGB)
			{
				mT2p->tiff_datasize=SampleRGBAToRGB(
					(tdata_t)buffer, 
					mT2p->tiff_width*mT2p->tiff_length);
			}

			if(mT2p->pdf_sample & T2P_SAMPLE_RGBAA_TO_RGB)
			{
				mT2p->tiff_datasize=SampleRGBAAToRGB(
					(tdata_t)buffer, 
					mT2p->tiff_width*mT2p->tiff_length);
			}

			if(mT2p->pdf_sample & T2P_SAMPLE_YCBCR_TO_RGB)
			{
				samplebuffer=(unsigned char*)_TIFFrealloc(
					(tdata_t)buffer, 
					mT2p->tiff_width*mT2p->tiff_length*4);
				if(samplebuffer==NULL)
				{
					TRACE_LOG2( 
						"Can't allocate %u bytes of memory for t2p_readwrite_pdf_image, %s", 
						mT2p->tiff_datasize, 
						mT2p->inputFilePath.c_str());
					status = PDFHummus::eFailure;
					_TIFFfree(buffer);
					break;
				} 
				else 
				{
					buffer=samplebuffer;
				}
				if(!TIFFReadRGBAImageOriented(
					mT2p->input, 
					mT2p->tiff_width, 
					mT2p->tiff_length, 
					(uint32_t*)buffer, 
					ORIENTATION_TOPLEFT,
					0))
				{
					TRACE_LOG1( 
						"Can't use TIFFReadRGBAImageOriented to extract RGB image from %s", 
						mT2p->inputFilePath.c_str());
					status = PDFHummus::eFailure;
					break;
				}
				mT2p->tiff_datasize=SampleABGRToRGB(
					(tdata_t) buffer, 
					mT2p->tiff_width*mT2p->tiff_length);

			}

			if(mT2p->pdf_sample & T2P_SAMPLE_LAB_SIGNED_TO_UNSIGNED)
			{
				mT2p->tiff_datasize=SampleLABSignedToUnsigned(
					(tdata_t)buffer, 
					mT2p->tiff_width*mT2p->tiff_length);
			}
		}

		status = WriteImageBufferToStream(inImageStream,
								 mT2p->tiff_width,
								 mT2p->tiff_length,
								 buffer,
								 GetSizeFromTIFFDataSize);
		if (buffer != NULL) 
		{
			_TIFFfree(buffer);
			buffer=NULL;
		}
	}while(false);
	return status;
}

void TIFFImageHandler::SampleRealizePalette(unsigned char* inBuffer)
{
	uint32_t sample_count=0;
	uint16_t component_count=0;
	uint32_t palette_offset=0;
	uint32_t sample_offset=0;
	uint32_t i=0;
	uint32_t j=0;
	sample_count=mT2p->tiff_width*mT2p->tiff_length;
	component_count=mT2p->tiff_samplesperpixel;
	
	for(i=sample_count;i>0;i--)
	{
		palette_offset=inBuffer[i-1] * component_count;
		sample_offset= (i-1) * component_count;
		for(j=0;j<component_count;j++)
			inBuffer[sample_offset+j]=mT2p->pdf_palette[palette_offset+j];
	}
}

tsize_t TIFFImageHandler::SampleABGRToRGB(tdata_t inData, uint32_t inSampleCount)
{
	uint32_t i=0;
	uint32_t sample=0;
	
	for(i=0;i<inSampleCount;i++)
	{
		sample=((uint32_t*)inData)[i];
		((char*)inData)[i*3]= (char) (sample & 0xff);
		((char*)inData)[i*3+1]= (char) ((sample>>8) & 0xff);
		((char*)inData)[i*3+2]= (char) ((sample>>16) & 0xff);
	}

	return(i*3);	
}

EStatusCode TIFFImageHandler::WriteImageBufferToStream(PDFStream* inPDFStream,
														uint32_t inImageWidth,
														uint32_t inImageLength,
														unsigned char* inBuffer,
														ImageSizeProc inBufferSizeFunction)
{
	EStatusCode status = PDFHummus::eSuccess;
	do
	{
		mT2p->pdfStream = NULL;

		/* hopefully here is good enough, and that dummy is good. 
			basically the only allowed action is to write */
		TIFF* output = TIFFClientOpen("dummy.txt", "w", (thandle_t)mT2p,
					t2p_readproc, t2p_writeproc, t2p_seekproc, 
					t2p_closeproc, t2p_sizeproc, 
					t2p_mapproc, t2p_unmapproc );

		TIFFSetField(output, TIFFTAG_PHOTOMETRIC, mT2p->tiff_photometric);
		TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, mT2p->tiff_bitspersample);
		TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, mT2p->tiff_samplesperpixel);
		TIFFSetField(output, TIFFTAG_IMAGEWIDTH, inImageWidth);
		TIFFSetField(output, TIFFTAG_IMAGELENGTH, inImageLength);
		TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, inImageLength); // equal to image length in both usages...
		TIFFSetField(output, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(output, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);

		switch(mT2p->pdf_compression)
		{
		case T2P_COMPRESS_NONE:
			TIFFSetField(output, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
			break;
		case T2P_COMPRESS_G4:
			TIFFSetField(output, TIFFTAG_COMPRESSION, COMPRESSION_CCITTFAX4);
			break;
		case T2P_COMPRESS_ZIP:
			TIFFSetField(output, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
			if(mT2p->pdf_defaultcompressionquality%100 != 0)
			{
				TIFFSetField(output, 
					TIFFTAG_PREDICTOR, 
					mT2p->pdf_defaultcompressionquality % 100);
			}
			if(mT2p->pdf_defaultcompressionquality/100 != 0)
			{
				TIFFSetField(output, 
					TIFFTAG_ZIPQUALITY, 
					(mT2p->pdf_defaultcompressionquality / 100));
			}
			break;
		}

		mT2p->pdfStream = inPDFStream;
		mT2p->output = output; // dirty trick so i can use the inBufferSizeFunction function by getting info from output (sometimes)

		tsize_t bufferoffset = TIFFWriteEncodedStrip(output, (tstrip_t)0,inBuffer,inBufferSizeFunction(mT2p)); 

		mT2p->output = NULL;
		mT2p->pdfStream = NULL;
		if (output != NULL)
			TIFFClose(output);

		if (bufferoffset == (tsize_t)-1) 
		{
			TRACE_LOG1("Error writing encoded strip to output PDF %s",mT2p->inputFilePath.c_str());
			status = PDFHummus::eFailure;
			break;
		}		
	}while(false);

	return status;
}

PDFFormXObject* TIFFImageHandler::WriteImagesFormXObject(const PDFImageXObjectList& inImages,ObjectIDType inFormXObjectID)
{
	EStatusCode status = PDFHummus::eSuccess;
	PDFImageXObjectList::const_iterator it = inImages.begin();
	ttile_t i=0;
	T2P_BOX box;
	PDFFormXObject* xobjectForm = mContainerDocumentContext->StartFormXObject(
																	PDFRectangle(
																		mT2p->pdf_mediabox.x1,
																		mT2p->pdf_mediabox.y1,
																		mT2p->pdf_mediabox.x2,
																		mT2p->pdf_mediabox.y2),
																		inFormXObjectID);
	do
	{

		XObjectContentContext* xobjectContentContext = xobjectForm->GetContentContext();

		if(mT2p->tiff_transferfunctioncount != 0)
		{
			xobjectContentContext->q();
			xobjectContentContext->gs(xobjectForm->GetResourcesDictionary().AddExtGStateMapping(
																	mT2p->pdf_transfer_functions_gstate));
		}

		// BiLevel tiff image handling, set the color to the "1" color 
		if((mT2p->pdf_colorspace & T2P_CS_BILEVEL) && mUserParameters.BWTreatment.AsImageMask)
		{
			xobjectContentContext->q();
			if(mUserParameters.BWTreatment.OneColor.UseCMYK)
				xobjectContentContext->k((double)mUserParameters.BWTreatment.OneColor.CMYKComponents[0]/255,
										 (double)mUserParameters.BWTreatment.OneColor.CMYKComponents[1]/255,
										 (double)mUserParameters.BWTreatment.OneColor.CMYKComponents[2]/255,
										 (double)mUserParameters.BWTreatment.OneColor.CMYKComponents[3]/255);
			else
				xobjectContentContext->rg((double)mUserParameters.BWTreatment.OneColor.RGBComponents[0]/255,
										 (double)mUserParameters.BWTreatment.OneColor.RGBComponents[1]/255,
										 (double)mUserParameters.BWTreatment.OneColor.RGBComponents[2]/255);
		}

		if(mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilecount > 0)
		{
			for(; it != inImages.end() && i <mT2p->tiff_tiles[mT2p->pdf_page].tiles_tilecount; ++it,++i)
			{
				box = mT2p->tiff_tiles[mT2p->pdf_page].tiles_tiles[i].tile_box;
				xobjectContentContext->q();
				xobjectContentContext->cm(	box.mat[0],
											box.mat[1],
											box.mat[3],
											box.mat[4],
											box.mat[6],
											box.mat[7]);
				xobjectContentContext->Do(
					xobjectForm->GetResourcesDictionary().AddImageXObjectMapping(*it));			
				xobjectContentContext->Q();
			}
		}
		else
		{
			box = mT2p->pdf_imagebox;
			xobjectContentContext->q();
			xobjectContentContext->cm(	box.mat[0],
										box.mat[1],
										box.mat[3],
										box.mat[4],
										box.mat[6],
										box.mat[7]);
			xobjectContentContext->Do(
				xobjectForm->GetResourcesDictionary().AddImageXObjectMapping(*it));			
			xobjectContentContext->Q();
		}
		if(mT2p->tiff_transferfunctioncount != 0 || 
			((mT2p->pdf_colorspace & T2P_CS_BILEVEL) && mUserParameters.BWTreatment.AsImageMask))
			xobjectContentContext->Q();

		status = mContainerDocumentContext->EndFormXObjectNoRelease(xobjectForm);
		if(status != PDFHummus::eSuccess)
		{
			TRACE_LOG1(
				"TIFFImageHandler::WriteImagesFormXObject, Error in writing form XObject for image %s",
				mT2p->inputFilePath.c_str());
			break;
		}	

	}while(false);

	if(status != PDFHummus::eSuccess)
	{
		delete xobjectForm;
		xobjectForm = NULL;
	}

	return xobjectForm;
}

void TIFFImageHandler::AddImagesProcsets(PDFImageXObject* inImageXObject)
{
	if(mT2p->pdf_colorspace == T2P_CS_BILEVEL 
		|| mT2p->pdf_colorspace == T2P_CS_GRAY
		)
	{
		inImageXObject->AddRequiredProcset(KProcsetImageB);
	} 
	else 
	{
		inImageXObject->AddRequiredProcset(KProcsetImageC);
		if(mT2p->pdf_colorspace & T2P_CS_PALETTE)
			inImageXObject->AddRequiredProcset(KProcsetImageI);
	}	
}

static const char scSingleColorCMYKFormat[] = "%02X%02X%02X%02X ";
static const char scSingleColorRGBFormat[] = "%02X%02X%02X ";
void TIFFImageHandler::WriteIndexedCSForBiLevelColorMap()
{
	unsigned long colorValuesPerComponent = 0x0001 << mT2p->tiff_bitspersample;
	unsigned long colorSpaceHighValue = colorValuesPerComponent - 1;
	int colorComponents = mUserParameters.GrayscaleTreatment.OneColor.UseCMYK? 4:3;
	double max[4],min[4];
	double maxStep[4],minStep[4];
	char colorsBuffer[10];
	std::stringstream mPalleteStream;

	mObjectsContext->StartArray();
	mObjectsContext->WriteName(scIndexed);

	// i'm allowing just CMYK and RGB...so i'll keep it simple
	if(4 == colorComponents)
	{
		for(int i=0;i<colorComponents;++i)
		{
			max[i] = mUserParameters.GrayscaleTreatment.OneColor.CMYKComponents[i];
			min[i] = 0;
			maxStep[i] = (double)mUserParameters.GrayscaleTreatment.OneColor.CMYKComponents[i]/255;
			minStep[i] = (double)mUserParameters.GrayscaleTreatment.ZeroColor.CMYKComponents[i]/255;

		}
		mObjectsContext->WriteName(scDeviceCMYK);
	}
	else
	{
		for(int i=0;i<colorComponents;++i)
		{
			max[i] = mUserParameters.GrayscaleTreatment.OneColor.RGBComponents[i];
			min[i] = 0;
			maxStep[i] = (double)mUserParameters.GrayscaleTreatment.OneColor.RGBComponents[i]/255;
			minStep[i] = (double)mUserParameters.GrayscaleTreatment.ZeroColor.RGBComponents[i]/255;

		}
		mObjectsContext->WriteName(scDeviceRGB);
	}

	// should be consistent with the above color space...
	mObjectsContext->WriteInteger(colorSpaceHighValue);

	// now write the pallete
	
	for(unsigned long j = 0;j < colorValuesPerComponent; ++j)
	{
		if(4 == colorComponents)
		{
			SAFE_SPRINTF_4(colorsBuffer,10,scSingleColorCMYKFormat,
				(unsigned char)max[0]+ (unsigned char)min[0],
				(unsigned char)max[1] + (unsigned char)min[1],
				(unsigned char)max[2] + (unsigned char)min[2],
				(unsigned char)max[3] + (unsigned char)min[3]);
		}
		else
		{
			SAFE_SPRINTF_3(colorsBuffer,10,scSingleColorRGBFormat,
				(unsigned char)max[0]+ (unsigned char)min[0],
				(unsigned char)max[1] + (unsigned char)min[1],
				(unsigned char)max[2] + (unsigned char)min[2]);
		}

		for(int i=0;i<colorComponents;++i)
		{
			max[i] -= maxStep[i];
			min[i] += minStep[i];
			
		}
		mPalleteStream<<colorsBuffer;
	}
	mObjectsContext->WriteEncodedHexString(mPalleteStream.str());
	mObjectsContext->EndArray(eTokenSeparatorEndLine);
}

void TIFFImageHandler::SetDocumentContextExtender(IDocumentContextExtender* inExtender)
{
	mExtender = inExtender;
}

PDFFormXObject* TIFFImageHandler::CreateFormXObjectFromTIFFStream(	IByteReaderWithPosition* inTIFFStream,
																	const TIFFUsageParameters& inTIFFUsageParameters)
{
	if(!mObjectsContext)
	{
		TRACE_LOG("TIFFImageHandler::CreateFormXObjectFromTIFFFile. Unexpected Error, mObjectsContext not initialized with an objects context");
		return NULL;
	}

	return CreateFormXObjectFromTIFFStream(inTIFFStream,mObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID(),inTIFFUsageParameters);
}

struct StreamWithPos
{
	IByteReaderWithPosition* mStream;
	LongFilePositionType mOriginalPosition;
};

static tsize_t STATIC_streamRead(thandle_t inData,tdata_t inBuffer,tsize_t inBufferSize)
{
	return (tsize_t)(((StreamWithPos*)inData)->mStream)->Read((Byte*)inBuffer,inBufferSize);
}

static tsize_t STATIC_streamWrite(thandle_t inData,tdata_t inBuffer,tsize_t inBufferSize)
{
	return 0; // not writing...just reading
}

static toff_t STATIC_streamSeek(thandle_t inData, toff_t inOffset, int inDirection)
{

    switch (inDirection) {
    case 0: // set
      (((StreamWithPos*)inData)->mStream)->SetPosition(inOffset);
      break;
    case 1: // current
	      (((StreamWithPos*)inData)->mStream)->Skip(inOffset);
      break;
    case 2: // from end
      ((StreamWithPos*)inData)->mStream->SetPositionFromEnd(inOffset);
      break;
    }	

	return (toff_t)((((StreamWithPos*)inData)->mStream)->GetCurrentPosition() - ((StreamWithPos*)inData)->mOriginalPosition);
}

static int STATIC_streamClose(thandle_t inData)
{
	return 0;
}

toff_t STATIC_tiffSize(thandle_t inData) 
{ 
	LongFilePositionType currentPosition = ((StreamWithPos*)inData)->mStream->GetCurrentPosition();

	((StreamWithPos*)inData)->mStream->SetPositionFromEnd(0);

	LongFilePositionType size = ((StreamWithPos*)inData)->mStream->GetCurrentPosition() - ((StreamWithPos*)inData)->mOriginalPosition;

	((StreamWithPos*)inData)->mStream->SetPosition(currentPosition);

    return  (toff_t)size; 
}; 

int STATIC_tiffMap(thandle_t, tdata_t*, toff_t*) 
{ 
 return 0; 
}; 

void STATIC_tiffUnmap(thandle_t, tdata_t, toff_t) 
{ 
 return; 
}; 

PDFFormXObject* TIFFImageHandler::CreateFormXObjectFromTIFFStream(	IByteReaderWithPosition* inTIFFStream,
																	ObjectIDType inFormXObjectID,
																	const TIFFUsageParameters& inTIFFUsageParameters)
{

	PDFFormXObject* imageFormXObject = NULL;
	TIFF* input = NULL;

	do
	{
		TIFFSetErrorHandler(ReportError);
		TIFFSetWarningHandler(ReportWarning);

		if(!mObjectsContext || !mContainerDocumentContext)
		{
			TRACE_LOG("TIFFImageHandler::CreateFormXObjectFromTIFFFile. Unexpected Error, mObjectsContext or mContainerDocumentContext not initialized");
			break;
		}

		StreamWithPos streamInfo;
		streamInfo.mStream = inTIFFStream;
		streamInfo.mOriginalPosition = inTIFFStream->GetCurrentPosition();
		
		input = TIFFClientOpen("Stream","r",(thandle_t)&streamInfo,STATIC_streamRead,
																	STATIC_streamWrite,
																	STATIC_streamSeek,
																	STATIC_streamClose,
																	STATIC_tiffSize,
																	STATIC_tiffMap,
																	STATIC_tiffUnmap);
		if(!input)
		{
			TRACE_LOG("TIFFImageHandler::CreateFormXObjectFromTIFFFile. cannot open stream for reading");
			break;
		}


		InitializeConversionState();
		mT2p->input = input;
		mT2p->inputFilePath = "";
		mT2p->pdf_page = inTIFFUsageParameters.PageIndex;
		mUserParameters = inTIFFUsageParameters;

		imageFormXObject = ConvertTiff2PDF(inFormXObjectID);

	}while(false);

	DestroyConversionState();
	if(input != NULL)
		TIFFClose(input);

	return imageFormXObject;
}

DoubleAndDoublePair TIFFImageHandler::ReadImageDimensions(IByteReaderWithPosition* inTIFFStream,unsigned long inImageIndex)
{
    return ReadImageInfo(inTIFFStream,inImageIndex).dimensions;
}

TIFFImageHandler::TiffImageInfo TIFFImageHandler::ReadImageInfo(IByteReaderWithPosition* inTIFFStream,unsigned long inImageIndex)
{
	TIFF* input = NULL;
    TiffImageInfo imageInfo;
	
	imageInfo.dimensions.first = -1;
	imageInfo.dimensions.second = -1;
	imageInfo.colorComponents = 0;
    EStatusCode status;
    
	do
	{
		TIFFSetErrorHandler(ReportError);
		TIFFSetWarningHandler(ReportWarning);
        
		StreamWithPos streamInfo;
		streamInfo.mStream = inTIFFStream;
		streamInfo.mOriginalPosition = inTIFFStream->GetCurrentPosition();
		
		input = TIFFClientOpen("Stream","r",(thandle_t)&streamInfo,STATIC_streamRead,
                               STATIC_streamWrite,
                               STATIC_streamSeek,
                               STATIC_streamClose,
                               STATIC_tiffSize,
                               STATIC_tiffMap,
                               STATIC_tiffUnmap);
		if(!input)
		{
			TRACE_LOG("TIFFImageHandler::ReadImageDimensions. cannot open stream for reading");
			break;
		}
        
        
		InitializeConversionState();
		mT2p->input = input;
		mT2p->inputFilePath = "";
		mT2p->pdf_page = (tdir_t)inImageIndex;
        
        
        status = ReadTopLevelTiffInformation();
        if(status != PDFHummus::eSuccess)
            break;
            
        if(mT2p->pdf_page >= mT2p->tiff_pagecount)
        {
            TRACE_LOG3(
                           "TIFFImageHandler::ReadImageDimensions, Requested tiff page %u where the tiff only has %u pages. Tiff file name - %s",
                           mT2p->pdf_page,
                           mT2p->tiff_pagecount,
                           mT2p->inputFilePath.c_str());
            status = PDFHummus::eFailure;
            break;
        }
        
        status = ReadTIFFPageInformation();
        if(status != PDFHummus::eSuccess)
            break;
        imageInfo.dimensions.first = mT2p->pdf_mediabox.x2 - mT2p->pdf_mediabox.x1;
        imageInfo.dimensions.second = mT2p->pdf_mediabox.y2 - mT2p->pdf_mediabox.y1;
		imageInfo.colorComponents = mT2p->tiff_samplesperpixel;
		      
	}while(false);
    
	DestroyConversionState();
	if(input != NULL)
		TIFFClose(input);
    
    return imageInfo;
}

unsigned long TIFFImageHandler::ReadImagePageCount(IByteReaderWithPosition* inTIFFStream)
{
	TIFF* input = NULL;
	unsigned long result = 0;
	EStatusCode status;

	do
	{
		TIFFSetErrorHandler(ReportError);
		TIFFSetWarningHandler(ReportWarning);

		StreamWithPos streamInfo;
		streamInfo.mStream = inTIFFStream;
		streamInfo.mOriginalPosition = inTIFFStream->GetCurrentPosition();

		input = TIFFClientOpen("Stream", "r", (thandle_t)&streamInfo, STATIC_streamRead,
			STATIC_streamWrite,
			STATIC_streamSeek,
			STATIC_streamClose,
			STATIC_tiffSize,
			STATIC_tiffMap,
			STATIC_tiffUnmap);
		if (!input)
		{
			TRACE_LOG("TIFFImageHandler::ReadImagePageCount. cannot open stream for reading");
			break;
		}


		InitializeConversionState();
		mT2p->input = input;
		mT2p->inputFilePath = "";

		status = ReadTopLevelTiffInformation();
		if (status != PDFHummus::eSuccess)
			break;

		result = mT2p->tiff_pagecount;
	} while (false);

	DestroyConversionState();
	if (input != NULL)
		TIFFClose(input);

	return result;
}

#endif