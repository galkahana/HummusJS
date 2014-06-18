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
typedef	unsigned short uint16;	/* sizeof (uint16) must == 2 */
typedef	unsigned int uint32;	/* sizeof (uint32) must == 4 */
typedef	int int32;
typedef int32 tsize_t;          /* i/o size in bytes */
typedef void* tdata_t;          /* image data ref */

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
													const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters);
	PDFFormXObject* CreateFormXObjectFromTIFFStream(IByteReaderWithPosition* inTIFFStream,
													const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters);
	PDFFormXObject* CreateFormXObjectFromTIFFFile(	const std::string& inTIFFFilePath,
													ObjectIDType inFormXObjectID,
													const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters);
	PDFFormXObject* CreateFormXObjectFromTIFFStream(IByteReaderWithPosition* inTIFFStream,
													ObjectIDType inFormXObjectID,
													const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters);


	void SetOperationsContexts(PDFHummus::DocumentContext* inContainerDocumentContext,ObjectsContext* inObjectsContext);
	void SetDocumentContextExtender(IDocumentContextExtender* inExtender);

	void Reset();
    
    // utility for tiffs, to get what tiff dimensions hummus will use
    DoubleAndDoublePair ReadImageDimensions(IByteReaderWithPosition* inTIFFStream,unsigned long inImageIndex);
	// retrieves some image data, see struct for info
	TiffImageInfo ReadImageInfo(IByteReaderWithPosition* inTIFFStream,unsigned long inImageIndex);
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
	void ComposePDFPageOrient(T2P_BOX*, uint16);
	void ComposePDFPageOrientFlip(T2P_BOX*, uint16);
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
	tsize_t SampleRGBAToRGB(tdata_t inData, uint32 inSampleCount);
	tsize_t SampleRGBAAToRGB(tdata_t inData, uint32 inSampleCount);
	tsize_t	SampleLABSignedToUnsigned(tdata_t inBuffer, uint32 inSampleCount);
	void TileCollapseLeft(tdata_t inBuffer, tsize_t inScanWidth, 
							uint32 inTileWidth, uint32 inEdgeTileWidth, 
							uint32 inTileLength);
	PDFImageXObject* WriteUntiledImageXObject();
	void WriteCommonImageDictionaryProperties(DictionaryContext* inImageContext);
	PDFHummus::EStatusCode WriteImageData(PDFStream* inImageStream);
	void CalculateTiffSizeNoTiles();
	void SampleRealizePalette(unsigned char* inBuffer);
	tsize_t SampleABGRToRGB(tdata_t inData, uint32 inSampleCount);
	PDFHummus::EStatusCode WriteImageBufferToStream(	PDFStream* inPDFStream,
											uint32 inImageWidth,
											uint32 inImageLength,
											unsigned char* inBuffer,
											ImageSizeProc inBufferSizeFunction);
	PDFFormXObject* WriteImagesFormXObject(const PDFImageXObjectList& inImages,ObjectIDType inFormXObjectID);
	void AddImagesProcsets(PDFImageXObject* inImageXObject);
	void WriteIndexedCSForBiLevelColorMap();
};

#endif
