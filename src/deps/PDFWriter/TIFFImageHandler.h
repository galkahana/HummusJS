/*
   Source File : TIFFImageHandler.h


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
#pragma once

#ifndef PDFHUMMUS_NO_TIFF

#include "EStatusCode.h"
#include "ObjectsBasicTypes.h"
#include "TiffUsageParameters.h"

#include <string>
#include <list>
#include <utility>
#include <stdint.h>



struct T2P;
struct T2P_BOX;
class DictionaryContext;
class PDFImageXObject;
class PDFStream;
class PDFFormXObject;
class ObjectsContext;
class IDocumentContextExtender;
class IByteReaderWithPosition;

namespace PDFHummus
{
	class DocumentContext;
}

using namespace PDFHummus;


typedef std::list<ObjectIDType> ObjectIDTypeList;
typedef std::list<PDFImageXObject*> PDFImageXObjectList;
typedef std::list<std::string> StringList;
typedef std::pair<double,double> DoubleAndDoublePair;

//  tiff.h extracted defs [prefer to avoid including too much here]
typedef	unsigned short uint16_t;	/* sizeof (uint16_t) must == 2 */
typedef	unsigned int uint32_t;	/* sizeof (uint32_t) must == 4 */
typedef	int int32;
#ifdef _INCLUDE_TIFF_HEADER
/*
Since libtiff 4.0.0 tsize_t is changed from int32 to machine dependant.
'tiffconf.h' which contains the declarations for it is generated for platform.
Altough we might be able to guess tsize_t size depending on architecture,
it's not guaranteed that it will match the actual tiff headers...
*/

#include <tiffio.h>
#include <tiffvers.h>

#if TIFFLIB_VERSION >= 20120218 // tiff 4.0.1 or above
typedef uint64_t tsize_t_compat;
#else //  TIFFLIB_VERSION >= 20120218
typedef uint32_t_t tsize_t_compat;
#endif //  TIFFLIB_VERSION >= 20120218


#else //  _INCLUDE_TIFF_HEADER
/*
	if you still don't want to include tiff headers, but it's not int32,
	you can set the required definition with this macro.
	
*/
#ifndef _USE_TIFF_TSIZE_AS_FOLLOWS

typedef int64_t tsize_t;          /* i/o size in bytes */ // updating per new bundled version
typedef uint64_t tsize_t_compat;

#else // ! _USE_TIFF_TSIZE_AS_FOLLOWS

typedef _USE_TIFF_TSIZE_AS_FOLLOWS tsize_t;          /* i/o size in bytes */
/*
	we also need an unsigned type for TIFFTAG_STRIPBYTECOUNTS, TIFFTAG_STRIPOFFSETS etc
	which changed to 64bit in tiff 4.0.0
	it will be 'unsigned _USE_TIFF_TSIZE_AS_FOLLOWS' prefix by default.
	that's ugly and may not work or they are actually different size types, 
	so and you can use _USE_TIFF_TSIZE_COMPAT_AS_FOLLOWS to define that.
*/
#ifndef _USE_TIFF_TSIZE_COMPAT_AS_FOLLOWS
typedef unsigned _USE_TIFF_TSIZE_AS_FOLLOWS tsize_t_compat;
#else // ! _USE_TIFF_TSIZE_COMPAT_AS_FOLLOWS
typedef _USE_TIFF_TSIZE_COMPAT_AS_FOLLOWS tsize_t_compat;
#endif // ! _USE_TIFF_TSIZE_COMPAT_AS_FOLLOWS

#endif // ! _USE_TIFF_TSIZE_AS_FOLLOWS

typedef void* tdata_t;          /* image data ref */

#endif //  _INCLUDE_TIFF_HEADER

typedef	tsize_t (*ImageSizeProc)(T2P* inT2p);

class TIFFImageHandler
{
public:
	struct TiffImageInfo
	{
		DoubleAndDoublePair dimensions;
		int colorComponents;
	};

	TIFFImageHandler();
	virtual ~TIFFImageHandler(void);

	// create a form XObject from an image (using form for 1. tiled images 2. to setup matrix, set color space...and leave you with just placing the image object
	PDFFormXObject* CreateFormXObjectFromTIFFFile(	const std::string& inTIFFFilePath,
													const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters());
	PDFFormXObject* CreateFormXObjectFromTIFFStream(IByteReaderWithPosition* inTIFFStream,
													const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters());
	PDFFormXObject* CreateFormXObjectFromTIFFFile(	const std::string& inTIFFFilePath,
													ObjectIDType inFormXObjectID,
													const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters());
	PDFFormXObject* CreateFormXObjectFromTIFFStream(IByteReaderWithPosition* inTIFFStream,
													ObjectIDType inFormXObjectID,
													const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters());


	void SetOperationsContexts(PDFHummus::DocumentContext* inContainerDocumentContext,ObjectsContext* inObjectsContext);
	void SetDocumentContextExtender(IDocumentContextExtender* inExtender);

	void Reset();
    
    // utility for tiffs, to get what tiff dimensions hummus will use
    DoubleAndDoublePair ReadImageDimensions(IByteReaderWithPosition* inTIFFStream,unsigned long inImageIndex);
	// retrieves some image data, see struct for info
	TiffImageInfo ReadImageInfo(IByteReaderWithPosition* inTIFFStream,unsigned long inImageIndex);
	// get number of images in the tiff collection
	unsigned long ReadImagePageCount(IByteReaderWithPosition* inTIFFStream);
private:
	PDFHummus::DocumentContext* mContainerDocumentContext;
	ObjectsContext* mObjectsContext;
	T2P* mT2p; // state for tiff->pdf
	TIFFUsageParameters mUserParameters;
	IDocumentContextExtender* mExtender;


	void InitializeConversionState();
	void DestroyConversionState();
	PDFFormXObject* ConvertTiff2PDF(ObjectIDType inFormXObjectID);
	PDFHummus::EStatusCode ReadTopLevelTiffInformation();
	PDFHummus::EStatusCode ReadTIFFPageInformation();
	PDFHummus::EStatusCode ReadPhotometricPalette();
	PDFHummus::EStatusCode ReadPhotometricPaletteCMYK();
	void ComposePDFPage();
	void ComposePDFPageOrient(T2P_BOX*, uint16_t);
	void ComposePDFPageOrientFlip(T2P_BOX*, uint16_t);
	ObjectIDType WriteTransferFunction(int i);
	ObjectIDType WriteTransferFunctionsExtGState(const ObjectIDTypeList& inTransferFunctions);
	ObjectIDType WritePaletteCS();
	ObjectIDType WriteICCCS();
	void WriteXObjectCS(DictionaryContext* inContainerDictionary);
	void WriteXObjectCALCS();
	PDFImageXObject* WriteTileImageXObject(int inTileIndex);
	bool TileIsRightEdge(int inTileIndex);
	bool TileIsBottomEdge(int inTileIndex);
	void WriteImageXObjectDecode(DictionaryContext* inImageDictionary);
	void WriteImageXObjectFilter(DictionaryContext* inImageDictionary,int inTileIndex);
	void CalculateTiffTileSize(int inTileIndex);
	PDFHummus::EStatusCode WriteImageTileData(PDFStream* inImageStream,int inTileIndex);
	void SamplePlanarSeparateToContig(unsigned char* inBuffer, 
									  unsigned char* inSamplebuffer, 
									  tsize_t inSamplebuffersize);
	tsize_t SampleRGBAToRGB(tdata_t inData, uint32_t inSampleCount);
	tsize_t SampleRGBAAToRGB(tdata_t inData, uint32_t inSampleCount);
	tsize_t	SampleLABSignedToUnsigned(tdata_t inBuffer, uint32_t inSampleCount);
	void TileCollapseLeft(tdata_t inBuffer, tsize_t inScanWidth, 
							uint32_t inTileWidth, uint32_t inEdgeTileWidth, 
							uint32_t inTileLength);
	PDFImageXObject* WriteUntiledImageXObject();
	void WriteCommonImageDictionaryProperties(DictionaryContext* inImageContext);
	PDFHummus::EStatusCode WriteImageData(PDFStream* inImageStream);
	void CalculateTiffSizeNoTiles();
	void SampleRealizePalette(unsigned char* inBuffer);
	tsize_t SampleABGRToRGB(tdata_t inData, uint32_t inSampleCount);
	PDFHummus::EStatusCode WriteImageBufferToStream(	PDFStream* inPDFStream,
											uint32_t inImageWidth,
											uint32_t inImageLength,
											unsigned char* inBuffer,
											ImageSizeProc inBufferSizeFunction);
	PDFFormXObject* WriteImagesFormXObject(const PDFImageXObjectList& inImages,ObjectIDType inFormXObjectID);
	void AddImagesProcsets(PDFImageXObject* inImageXObject);
	void WriteIndexedCSForBiLevelColorMap();
};

#endif