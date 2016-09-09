/*
   Source File : TiffUsageParameters.h


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

#include "CMYKRGBColor.h"

// Options for TIFF image usage.
// by default the image drawn would be the first page of the input tiff (either one page or multipage)
// for bilevel images, will be drawn either as grayscale or black and white.
// for anything different, consult the parameters.


// black and white tiff parameters
struct TIFFBiLevelBWColorTreatment
{
	// AsImageMask - is this image to be drawn as regular b/w image (false), or as an image mask (true). 
	// when image mask 0 will be transparent, and 1 will be in the color of OneColor
	bool AsImageMask;

	// OneColor - When AsImageMask is true, this would be the color
	CMYKRGBColor OneColor;

	TIFFBiLevelBWColorTreatment(bool inAsImageMask,const CMYKRGBColor& inOneColor){AsImageMask=inAsImageMask;
																					OneColor=inOneColor;}

	// default treatment is AsImageMask = false
	static const TIFFBiLevelBWColorTreatment& DefaultTIFFBiLevelBWColorTreatment();
};

struct TIFFBiLevelGrayscaleColorTreatment
{
	// AsColorMap - is this image to be drawn are regular grayscale (false), or as a combination between 2 colors, where 0 
	// is one color, 1 is the other and any interim value is intepreted as their combination
	bool AsColorMap;

	// Note: make sure the OneColor and ZeroColor use the same state.

	// OneColor - if AsColorMap is true, this would be the color marking the "1" values
	CMYKRGBColor OneColor;
	// ZeroColor - if AsColorMap is true, this would be the color marking the "0" values
	CMYKRGBColor ZeroColor;

	TIFFBiLevelGrayscaleColorTreatment(bool inAsColorMap,const CMYKRGBColor& inOneColor, const CMYKRGBColor& inZeroColor){
																					AsColorMap=inAsColorMap;
																					OneColor=inOneColor;
																					ZeroColor=inZeroColor;}

	// default treatment is AsColorMap = false
	static const TIFFBiLevelGrayscaleColorTreatment& DefaultTIFFBiLevelGrayscaleColorTreatment();
};



struct TIFFUsageParameters
{
	// PageIndex - For multipage tiffs, use 0 to n-1 index to get the relevant page, where n is the total number of pages in the tiff file.
	unsigned int PageIndex;

	//	Black and white options
	TIFFBiLevelBWColorTreatment BWTreatment;

	// Grayscale options
	TIFFBiLevelGrayscaleColorTreatment GrayscaleTreatment;

	TIFFUsageParameters():BWTreatment(TIFFBiLevelBWColorTreatment::DefaultTIFFBiLevelBWColorTreatment()),GrayscaleTreatment(TIFFBiLevelGrayscaleColorTreatment::DefaultTIFFBiLevelGrayscaleColorTreatment())
						{PageIndex = 0;}

	TIFFUsageParameters(unsigned int inPageIndex,
						TIFFBiLevelBWColorTreatment inBWTreatment,
						TIFFBiLevelGrayscaleColorTreatment inGrayscaleTreatment):BWTreatment(inBWTreatment),GrayscaleTreatment(inGrayscaleTreatment)
						{PageIndex = inPageIndex;}

	// default treatment is 0 page index, and defaults for black & white and Grayscale
	static const TIFFUsageParameters& DefaultTIFFUsageParameters();

};