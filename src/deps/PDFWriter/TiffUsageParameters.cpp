/*
   Source File : TiffUsageParameters.cpp


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
#include "TiffUsageParameters.h"

const TIFFBiLevelBWColorTreatment& TIFFBiLevelBWColorTreatment::DefaultTIFFBiLevelBWColorTreatment()
{
	static TIFFBiLevelBWColorTreatment default_tiff_bilevel_bwcolor_treatment(false, CMYKRGBColor::CMYKBlack());
	return default_tiff_bilevel_bwcolor_treatment;
}

const TIFFBiLevelGrayscaleColorTreatment& TIFFBiLevelGrayscaleColorTreatment::DefaultTIFFBiLevelGrayscaleColorTreatment()
{
	static TIFFBiLevelGrayscaleColorTreatment default_tiff_bilevel_grayscale_treatment(false, CMYKRGBColor::CMYKBlack(), CMYKRGBColor::CMYKWhite());
	return default_tiff_bilevel_grayscale_treatment;

}

const TIFFUsageParameters& TIFFUsageParameters::DefaultTIFFUsageParameters()
{
	static TIFFUsageParameters default_tiff_usage_parameters(0,
		TIFFBiLevelBWColorTreatment::DefaultTIFFBiLevelBWColorTreatment(),
		TIFFBiLevelGrayscaleColorTreatment::DefaultTIFFBiLevelGrayscaleColorTreatment());
	return default_tiff_usage_parameters;
}