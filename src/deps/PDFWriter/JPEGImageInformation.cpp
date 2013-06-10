/*
   Source File : JPEGImageInformation.cpp


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
#include "JPEGImageInformation.h"

JPEGImageInformation::JPEGImageInformation(void)
{

	SamplesWidth = 0;
	SamplesHeight = 0;
	ColorComponentsCount= 0;

	JFIFInformationExists = false;
	JFIFUnit = 0;
	JFIFXDensity = 0;
	JFIFYDensity = 0;

	ExifInformationExists = false;
	ExifUnit = 0;
	ExifXDensity = 0;
	ExifYDensity = 0;

	PhotoshopInformationExists = false;
	PhotoshopXDensity = 0;
	PhotoshopYDensity = 0;
}


