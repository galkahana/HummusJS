/*
   Source File : JPEGImageParser.cpp


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
#include "JPEGImageParser.h"
#include "JPEGImageInformation.h"
#include "IByteReaderWithPosition.h"

#include <memory.h>

using namespace PDFHummus;

JPEGImageParser::JPEGImageParser(void)
{
}

JPEGImageParser::~JPEGImageParser(void)
{
}


const unsigned int scSOF0TagID = 0xc0; //baseline format
const unsigned int scSOF1TagID = 0xc1;
const unsigned int scSOF2TagID = 0xc2;
const unsigned int scSOF3TagID = 0xc3;
const unsigned int scSOF5TagID = 0xc5;
const unsigned int scSOF6TagID = 0xc6;
const unsigned int scSOF7TagID = 0xc7;
const unsigned int scSOF9TagID = 0xc9;
const unsigned int scSOF10TagID = 0xca;
const unsigned int scSOF11TagID = 0xcb;
const unsigned int scSOF13TagID = 0xcd;
const unsigned int scSOF14TagID = 0xce;
const unsigned int scSOF15TagID = 0xcf;
const unsigned int scAPP0TagID = 0xe0; //JFIF marker
const unsigned int scAPP1TagID = 0xe1;	//Exif marker
const unsigned int scAPP13TagID = 0xed;//Photoshop marker
const unsigned int scTagID = 0xff;
const unsigned char scJPEGID[2] = {0xff,0xd8};
const unsigned char scAPP1ID_1[6] = {0x45,0x78,0x69,0x66,0x00,0x00};
const unsigned char scAPP1ID_2[6] = {0x45,0x78,0x69,0x66,0x00,0xFF};
const unsigned char scEOS = '\0';
const unsigned char sc8Bim[4] = {'8','B','I','M'};
const unsigned char scResolutionBIMID[2] = {0x03,0xed};
const unsigned int scAPP1BigEndian = 0x4d4d;
const unsigned int scAPP1LittleEndian = 0x4949;
const unsigned int scAPP1xResolutionTagID = 0x011a;
const unsigned int scAPP1yResolutionTagID = 0x011b;
const unsigned int scAPP1ResolutionUnitTagID = 0x0128;

EStatusCode JPEGImageParser::Parse(IByteReaderWithPosition* inImageStream,JPEGImageInformation& outImageInformation)
{
	EStatusCode status = PDFHummus::eFailure;
	unsigned int tagID;
	bool PhotoshopMarkerNotFound = true;
	bool JFIFMarkerNotFound = true;
	bool SOFMarkerNotFound = true;
	bool ExifMarkerNotFound = true;

	mImageStream = inImageStream;

	do
	{
		status = ReadJPEGID();
		if(status != PDFHummus::eSuccess)
			break;

		do
		{
			if(!mImageStream->NotEnded())
				break;
			status = ReadJpegTag(tagID);
			if(status != PDFHummus::eSuccess)
				break;
			switch(tagID)
			{
				case scSOF0TagID : case scSOF1TagID : case scSOF2TagID:
				case scSOF3TagID : case scSOF5TagID : case scSOF6TagID:
				case scSOF7TagID : case scSOF9TagID : case scSOF10TagID:
				case scSOF11TagID : case scSOF13TagID : case scSOF14TagID:
				case scSOF15TagID :
					SOFMarkerNotFound = false;
					status = ReadSOF0Data(outImageInformation);
					break;
				case scAPP0TagID:
					if(JFIFMarkerNotFound)
					{
						JFIFMarkerNotFound = false;
						status = ReadJFIFData(outImageInformation);
					}
					else
						SkipTag();
					break;
				case scAPP13TagID:
                    if(PhotoshopMarkerNotFound)
                    {
                        // photoshop tags may be corrupt, so internal method will return if the 
                        // photoshop tag is OK. otherwise skip it, and wait for the next one...parhaps will be better
                        status = ReadPhotoshopData(outImageInformation,PhotoshopMarkerNotFound);
                    }
                    else
                        status = SkipTag();
					break;				
				case scAPP1TagID:
					if(ExifMarkerNotFound)
					{
						ExifMarkerNotFound = false;
						status = ReadExifData(outImageInformation);
                        if(status != eSuccess)
                        {
                            // if unable to read marker it's either XMP or unsupported version of Exif. simply ignore
                            ExifMarkerNotFound = true;
                            status = eSuccess;
                        }
					}
					else
						status = SkipTag();
					break;
				default:
					status = SkipTag();
					break;
			}
		}
		while((ExifMarkerNotFound || PhotoshopMarkerNotFound || JFIFMarkerNotFound || SOFMarkerNotFound) && (0 == status));

		if (SOFMarkerNotFound)
		{
			status = PDFHummus::eFailure;
			break;
		}
		else
			status = PDFHummus::eSuccess; 
	} 
	while(false);

	return status;
}

EStatusCode JPEGImageParser::ReadJPEGID()
{
	EStatusCode status = ReadStreamToBuffer(2);
	
	if (status != PDFHummus::eSuccess)
		return status;
	
	if (memcmp(mReadBuffer, scJPEGID, 2) != 0)
		return PDFHummus::eFailure;
	
	return PDFHummus::eSuccess;
}

EStatusCode JPEGImageParser::ReadStreamToBuffer(unsigned long inAmountToRead)
{
	if(inAmountToRead == mImageStream->Read(mReadBuffer,inAmountToRead))
		return PDFHummus::eSuccess;
	else
		return PDFHummus::eFailure;
}

EStatusCode JPEGImageParser::ReadJpegTag(unsigned int& outTagID)
{
	EStatusCode status = ReadStreamToBuffer(2);

	if(PDFHummus::eSuccess == status)
	{
		if(scTagID == (unsigned int)mReadBuffer[0])
			outTagID = (unsigned int)mReadBuffer[1];
		else 
			status = PDFHummus::eFailure;
	}
	return status;
}

EStatusCode JPEGImageParser::ReadSOF0Data(JPEGImageInformation& outImageInformation)
{
	unsigned int toSkip;
	EStatusCode status;

	status = ReadStreamToBuffer(8);
	if(PDFHummus::eSuccess == status)
	{
		toSkip = GetIntValue(mReadBuffer) - 8;
		outImageInformation.SamplesHeight = GetIntValue(mReadBuffer + 3);
		outImageInformation.SamplesWidth = GetIntValue(mReadBuffer + 5);
		outImageInformation.ColorComponentsCount = (unsigned int)mReadBuffer[7];
		SkipStream(toSkip);
	}
	return status;
}

unsigned int JPEGImageParser::GetIntValue(
							const IOBasicTypes::Byte* inBuffer,
							bool inUseLittleEndian)
{
	unsigned int value;

	if (inUseLittleEndian)
	{
		value = (unsigned int)inBuffer[0];
		value += 0x100 * (unsigned int)inBuffer[1];
	}
	else
	{
		value = (unsigned int)inBuffer[1];
		value += 0x100 * (unsigned int)inBuffer[0];
	}

	return value;
}

void JPEGImageParser::SkipStream(unsigned long inSkip)
{
	mImageStream->Skip(inSkip);
}


EStatusCode JPEGImageParser::ReadJFIFData(JPEGImageInformation& outImageInformation)
{
	unsigned int toSkip;
	EStatusCode status;

	status = ReadStreamToBuffer(14);
	if(PDFHummus::eSuccess == status)
	{
		outImageInformation.JFIFInformationExists = true;
		toSkip = GetIntValue(mReadBuffer) - 14;
		outImageInformation.JFIFUnit = (unsigned int)mReadBuffer[9];
		outImageInformation.JFIFXDensity = GetIntValue(mReadBuffer + 10);
		outImageInformation.JFIFYDensity = GetIntValue(mReadBuffer + 12);
		SkipStream(toSkip);
	}
	return status;
}

EStatusCode JPEGImageParser::ReadPhotoshopData(JPEGImageInformation& outImageInformation,bool outPhotoshopDataOK)
{
	EStatusCode status;
	unsigned int intSkip;
	unsigned long toSkip;
	unsigned int nameSkip;
	unsigned long dataLength;
	bool resolutionBimNotFound = true;

	do {
		status = ReadIntValue(intSkip);
		if(status != PDFHummus::eSuccess)
			break;
		toSkip = intSkip-2;
		status = SkipTillChar(scEOS,toSkip);
		if(status != PDFHummus::eSuccess)
			break;
		while(toSkip > 0 && resolutionBimNotFound)
		{
			status = ReadStreamToBuffer(4);
			if(status !=PDFHummus::eSuccess)
				break;
            toSkip-=4;
            if(0 != memcmp(mReadBuffer,sc8Bim,4))
                break; // k. corrupt header. stop here and just skip the next
			status = ReadStreamToBuffer(3);
			if(status !=PDFHummus::eSuccess)
				break;
			toSkip-=3;
			nameSkip = (int)mReadBuffer[2];
			if(nameSkip % 2 == 0)
				++nameSkip;
			SkipStream(nameSkip);
			toSkip-=nameSkip;
			resolutionBimNotFound = (0 != memcmp(mReadBuffer,scResolutionBIMID,2));
			status = ReadLongValue(dataLength);
			if(status != PDFHummus::eSuccess)
				break;
			toSkip-=4;
			if(resolutionBimNotFound)
			{
				if(dataLength % 2 == 1)
					++dataLength;
				toSkip-=dataLength;
				SkipStream(dataLength);
			}
			else
			{
				status = ReadStreamToBuffer(16);
				if(status !=PDFHummus::eSuccess)
					break;
				toSkip-=16;
				outImageInformation.PhotoshopInformationExists = true;
				outImageInformation.PhotoshopXDensity = GetIntValue(mReadBuffer) + GetFractValue(mReadBuffer + 2);
				outImageInformation.PhotoshopYDensity = GetIntValue(mReadBuffer + 8) + GetFractValue(mReadBuffer + 10);
			}
		}
		if(PDFHummus::eSuccess == status)
			SkipStream(toSkip);
	}while(false);
    outPhotoshopDataOK = !resolutionBimNotFound;
	return status;
}

EStatusCode JPEGImageParser::ReadExifData(JPEGImageInformation& outImageInformation)
{
	EStatusCode status;
	unsigned long ifdOffset;
	unsigned int ifdDirectorySize, tagID, toSkip;
	bool isBigEndian;
	unsigned long xResolutionOffset = 0;
	unsigned long yResolutionOffset = 0;
	unsigned int resolutionUnitValue = 0; 

	do 
	{
		//read Exif Tag size
		status = ReadIntValue(toSkip);
		if (status != PDFHummus::eSuccess)
			break;
		
		toSkip -= 2;

		//read Exif ID
		status = ReadExifID();
		toSkip -= 6;
		if (status != PDFHummus::eSuccess)
        {
            // might be wrong ID
            SkipStream(toSkip);
			break;
		}

		//read encoding
		status = IsBigEndianExif(isBigEndian);
		if (status != PDFHummus::eSuccess)
			break;

		toSkip -= 2;

		//skip 0x002a
		SkipStream(2);		
		toSkip -= 2;

		//read IFD0 offset
		status = ReadLongValue(ifdOffset, !isBigEndian);	
		if (status != PDFHummus::eSuccess)
			break;

		toSkip -= 4;

		//skip to the IFD beginning
		SkipStream(ifdOffset - 8);
		toSkip -= (ifdOffset - 8);

		//read IFD size
		status = ReadIntValue(ifdDirectorySize, !isBigEndian);
		if (status != PDFHummus::eSuccess)
			break;

		toSkip -= 2;

		for (unsigned int i = 0; i < ifdDirectorySize; i++)
		{			
			if (0 != xResolutionOffset && 0 != yResolutionOffset && 0 != resolutionUnitValue)
			{
				SkipStream(12 * (ifdDirectorySize - i));
				toSkip -= (12 * (ifdDirectorySize - i));
				break;
			}

			status = ReadIntValue(tagID, !isBigEndian);
			if (status != PDFHummus::eSuccess)
				break;

			toSkip -= 2;

			switch (tagID)
			{
				case scAPP1xResolutionTagID:
					SkipStream(6);					
					status = ReadLongValue(xResolutionOffset, !isBigEndian);
					break;
				case scAPP1yResolutionTagID:
					SkipStream(6);		
					status = ReadLongValue(yResolutionOffset, !isBigEndian);
					break;
				case scAPP1ResolutionUnitTagID:
					SkipStream(6);		
					status = ReadIntValue(resolutionUnitValue, !isBigEndian);
					SkipStream(2);
					break;
				default:
					SkipStream(10);
					break;
			}

			toSkip -= 10;
			if (status != PDFHummus::eSuccess)
				break;
		}
		
		outImageInformation.ExifInformationExists = true;
		if (resolutionUnitValue != 0) 
			outImageInformation.ExifUnit = resolutionUnitValue;		
		else
			outImageInformation.ExifUnit = 2;

		unsigned long currentOffset = ifdOffset + ifdDirectorySize * 12 + 2;
		unsigned long tempOffset = currentOffset;
		status = GetResolutionFromExif(outImageInformation, xResolutionOffset, yResolutionOffset, tempOffset, !isBigEndian);
		if (status != PDFHummus::eSuccess)
			break;

		toSkip -= (tempOffset - currentOffset);

		if (PDFHummus::eSuccess == status)
			SkipStream(toSkip);
	}
	while(false);	
	return status;
}

EStatusCode JPEGImageParser::GetResolutionFromExif(
							   JPEGImageInformation& outImageInformation,
							   unsigned long inXResolutionOffset,
							   unsigned long inYResolutionOffset,
							   unsigned long& inoutOffset,
							   bool inUseLittleEndian)
{	
	unsigned long firstOffset = 0, secondOffset = 0;
	bool xResolutionIsFirst = true;
	EStatusCode status = PDFHummus::eSuccess;

	outImageInformation.ExifXDensity = 0;
	outImageInformation.ExifYDensity = 0;

	if (inXResolutionOffset != 0 && inXResolutionOffset < inYResolutionOffset)
	{
		firstOffset = inXResolutionOffset;

		if (inYResolutionOffset != 0)
			secondOffset = inYResolutionOffset;	
	}
	else if (inYResolutionOffset != 0)
	{
		firstOffset = inYResolutionOffset;
		xResolutionIsFirst = false;

		if (inXResolutionOffset != 0)
			secondOffset = inXResolutionOffset;	
	}

	do
	{
		if (0 == firstOffset)
			break;

		SkipStream(firstOffset - inoutOffset);
		inoutOffset += (firstOffset - inoutOffset);

		status = ReadRationalValue(
			xResolutionIsFirst? outImageInformation.ExifXDensity : outImageInformation.ExifYDensity, 
			inUseLittleEndian);

		if (status != PDFHummus::eSuccess)
			break;

		inoutOffset += 8;

		if (0 == secondOffset)
			break;

		SkipStream(secondOffset - firstOffset - 8);
		inoutOffset += (secondOffset - firstOffset - 8);

		status = ReadRationalValue(
			xResolutionIsFirst? outImageInformation.ExifYDensity : outImageInformation.ExifXDensity, 
			inUseLittleEndian);
		if (status != PDFHummus::eSuccess)
			break;

		inoutOffset += 8;
	} while(false);
	return status;
}

EStatusCode JPEGImageParser::ReadRationalValue(
							   double& outDoubleValue,
							   bool inUseLittleEndian)
{
	unsigned long numerator, denominator;
	EStatusCode status = ReadLongValue(numerator, inUseLittleEndian);

	if (status != PDFHummus::eSuccess)
		return status;

	status = ReadLongValue(denominator, inUseLittleEndian);

	if (status != PDFHummus::eSuccess)
		return status;

	outDoubleValue = ((double) numerator) / ((double) denominator); 
	return status;
}

EStatusCode JPEGImageParser::ReadExifID()
{
	EStatusCode status = ReadStreamToBuffer(6);
	if (status != PDFHummus::eSuccess)
		return status;

	if (memcmp(mReadBuffer, scAPP1ID_1, 6) != 0 && memcmp(mReadBuffer, scAPP1ID_2,6) != 0)
		return PDFHummus::eFailure;

	return PDFHummus::eSuccess;
}

EStatusCode JPEGImageParser::IsBigEndianExif(bool& outIsBigEndian)
{	
	unsigned int encodingType;
	EStatusCode status = ReadIntValue(encodingType);
		
	if (status != PDFHummus::eSuccess)
		return status;
	
	if (encodingType == scAPP1BigEndian)
		outIsBigEndian = true;
	else if (encodingType == scAPP1LittleEndian)
		outIsBigEndian = false;
	else
		return PDFHummus::eFailure;

	return PDFHummus::eSuccess;
}

EStatusCode JPEGImageParser::ReadIntValue(
						unsigned int& outIntValue,
						bool inUseLittleEndian)
{
	EStatusCode status = ReadStreamToBuffer(2);

	if(PDFHummus::eSuccess == status)
		outIntValue = GetIntValue(mReadBuffer, inUseLittleEndian);
	return status;
}

EStatusCode JPEGImageParser::SkipTillChar(IOBasicTypes::Byte inSkipUntilValue,unsigned long& refSkipLimit)
{
	EStatusCode status = PDFHummus::eSuccess;
	bool charNotFound = true;
	
	while(charNotFound && (PDFHummus::eSuccess == status) && (refSkipLimit > 0))
	{
		status = ReadStreamToBuffer(1);
		if(PDFHummus::eSuccess == status)
		{
			--refSkipLimit;
			if(mReadBuffer[0] == inSkipUntilValue)
				charNotFound = false;
		}
	}
	return status;
}

EStatusCode JPEGImageParser::ReadLongValue(	unsigned long& outLongValue,
											bool inUseLittleEndian)
{
	EStatusCode status = ReadStreamToBuffer(4);

	if(PDFHummus::eSuccess == status)
		outLongValue = GetLongValue(mReadBuffer, inUseLittleEndian);
	return status;
}

unsigned long JPEGImageParser::GetLongValue(
							const IOBasicTypes::Byte* inBuffer,
							bool inUseLittleEndian)
{
	unsigned long value;

	if (inUseLittleEndian)
	{
		value = (unsigned int)inBuffer[0];
		value += 0x100 * (unsigned int)inBuffer[1];
		value += 0x10000 * (unsigned int)inBuffer[2];
		value += 0x1000000 * (unsigned int)inBuffer[3];
	}
	else
	{
		value = (unsigned int)inBuffer[3];
		value += 0x100 * (unsigned int)inBuffer[2];
		value += 0x10000 * (unsigned int)inBuffer[1];
		value += 0x1000000 * (unsigned int)inBuffer[0];
	}
	return value;
}

double JPEGImageParser::GetFractValue(const IOBasicTypes::Byte* inBuffer)
{
	double value;

	value = (double)inBuffer[0] / (double)0x100 +
			(double)inBuffer[1] / (double)0x10;
	return value;
}


EStatusCode JPEGImageParser::SkipTag()
{
	EStatusCode status;
	unsigned int toSkip;

	status = ReadIntValue(toSkip);
	// skipping -2 because int was already read
	if(PDFHummus::eSuccess == status)
		SkipStream(toSkip-2);
	return status;
}
