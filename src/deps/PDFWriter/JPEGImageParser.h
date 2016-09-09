/*
   Source File : JPEGImageParser.h


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

#include "EStatusCode.h"
#include "IOBasicTypes.h"

struct JPEGImageInformation;
class IByteReaderWithPosition;

struct TwoLevelStatus
{
	PDFHummus::EStatusCode primary;
	PDFHummus::EStatusCode secondary;

	TwoLevelStatus(PDFHummus::EStatusCode inPrimary,
		PDFHummus::EStatusCode inSecondary)
	{
		primary = inPrimary;
		secondary = inSecondary;

	}

	bool eitherBad()
	{
		return primary != PDFHummus::eSuccess || secondary != PDFHummus::eSuccess;
	}
};

class JPEGImageParser
{
public:
	JPEGImageParser(void);
	~JPEGImageParser(void);

	PDFHummus::EStatusCode Parse(IByteReaderWithPosition* inImageStream,JPEGImageInformation& outImageInformation);

private:

	IByteReaderWithPosition* mImageStream;
	IOBasicTypes::Byte mReadBuffer[500];

	PDFHummus::EStatusCode ReadJPEGID();
	PDFHummus::EStatusCode ReadStreamToBuffer(unsigned long inAmountToRead);
	TwoLevelStatus ReadStreamToBuffer(unsigned long inAmountToRead, unsigned long& refReadLimit);
	PDFHummus::EStatusCode ReadJpegTag(unsigned int& outTagID);
	PDFHummus::EStatusCode ReadSOF0Data(JPEGImageInformation& outImageInformation);
	unsigned int GetIntValue(const IOBasicTypes::Byte* inBuffer,
							 bool inUseLittleEndian = false);
	void SkipStream(unsigned long inSkip);
	PDFHummus::EStatusCode SkipStream(unsigned long inSkip, unsigned long& refReadLimit);
	PDFHummus::EStatusCode ReadJFIFData(JPEGImageInformation& outImageInformation);
	PDFHummus::EStatusCode ReadPhotoshopData(JPEGImageInformation& outImageInformation,bool outPhotoshopDataOK);
	PDFHummus::EStatusCode ReadExifData(JPEGImageInformation& outImageInformation);
	PDFHummus::EStatusCode GetResolutionFromExif(	JPEGImageInformation& outImageInformation,
									   unsigned long inXResolutionOffset,
									   unsigned long inYResolutionOffset,
									   unsigned long& inoutOffset,
									   bool inUseLittleEndian);
	PDFHummus::EStatusCode ReadRationalValue(double& outDoubleValue,
								  bool inUseLittleEndian);
	PDFHummus::EStatusCode ReadExifID();
	PDFHummus::EStatusCode IsBigEndianExif(bool& outIsBigEndian);
	PDFHummus::EStatusCode ReadIntValue(	unsigned int& outIntValue,
								bool inUseLittleEndian = false);
	PDFHummus::EStatusCode SkipTillChar(IOBasicTypes::Byte inSkipUntilValue,unsigned long& refSkipLimit);
	PDFHummus::EStatusCode ReadLongValue(unsigned long& outLongValue,
		bool inUseLittleEndian);
	TwoLevelStatus ReadLongValue(
		unsigned long& refReadLimit,
		unsigned long& outLongValue,
		bool inUseLittleEndian = false);
	unsigned long GetLongValue(	const IOBasicTypes::Byte* inBuffer,
								bool inUseLittleEndian);
	double GetFractValue(const IOBasicTypes::Byte* inBuffer);
	PDFHummus::EStatusCode SkipTag();
};
