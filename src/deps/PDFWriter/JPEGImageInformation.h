/*
   Source File : JPEGImageInformation.h


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

struct JPEGImageInformation
{
	JPEGImageInformation(void);

	// basic image information
	long long SamplesWidth;
	long long SamplesHeight;
	int ColorComponentsCount;

	// JFIF Information
	bool JFIFInformationExists;
	unsigned int JFIFUnit;
	double JFIFXDensity;
	double JFIFYDensity;

	// Exif Information
	bool ExifInformationExists;
	unsigned int ExifUnit;
	double ExifXDensity;
	double ExifYDensity;

	// Photoshop Information
	bool PhotoshopInformationExists;
	// photoshop density is always pixel per inch
	double PhotoshopXDensity;
	double PhotoshopYDensity;

};
