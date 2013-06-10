/*
   Source File : PFMFileReader.cpp


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
#include "PFMFileReader.h"
#include "Trace.h"
#include "InputFile.h"
#include "IByteReaderWithPosition.h"

using namespace PDFHummus;

PFMFileReader::PFMFileReader(void)
{
}

PFMFileReader::~PFMFileReader(void)
{
}

EStatusCode PFMFileReader::Read(const std::string& inPFMFilePath)
{
	EStatusCode status = PDFHummus::eSuccess;
	mInternalReadStatus = PDFHummus::eSuccess;
	InputFile pfmFile;

	status = pfmFile.OpenFile(inPFMFilePath);
	if(status != PDFHummus::eSuccess)
	{
		TRACE_LOG1("PFMFileReader::Read, unable to open PFM file in %s",inPFMFilePath.c_str());
		return status;
	}

	do
	{
		mReaderStream = pfmFile.GetInputStream();

		status = ReadHeader();
		if(status != PDFHummus::eSuccess)
			break;

		status = ReadExtension();
		if(status != PDFHummus::eSuccess)
			break;

		status = ReadExtendedFontMetrics();
		if(status != PDFHummus::eSuccess)
			break;
		
	}while(false);

	pfmFile.CloseFile();
	return status;
}

EStatusCode PFMFileReader::ReadHeader()
{
	ReadWord(Header.Version);
	ReadDWord(Header.Size);
	for(int i=0;i<60;++i)
		ReadByte(*(Header.Copyright+ i));
	ReadWord(Header.Type);
	ReadWord(Header.Point);
	ReadWord(Header.VertRes);
	ReadWord(Header.HorizRes);
	ReadWord(Header.Ascent);
	ReadWord(Header.InternalLeading);
	ReadWord(Header.ExternalLeading);
	ReadByte(Header.Italic);
	ReadByte(Header.Underline);
	ReadByte(Header.StrikeOut);
	ReadWord(Header.Weight);
	ReadByte(Header.CharSet);
	ReadWord(Header.PixWidth);
	ReadWord(Header.PixHeight);
	ReadByte(Header.PitchAndFamily);
	ReadWord(Header.AvgWidth);
	ReadWord(Header.MaxWidth);
	ReadByte(Header.FirstChar);
	ReadByte(Header.LastChar);
	ReadByte(Header.DefaultChar);
	ReadByte(Header.BreakChar);
	ReadWord(Header.WidthBytes);
	ReadDWord(Header.Device);
	ReadDWord(Header.Face);
	ReadDWord(Header.BitsPinter);
	ReadDWord(Header.BitsOffset);

	return mInternalReadStatus;
}

EStatusCode PFMFileReader::ReadByte(BYTE& outByte)
{
	IOBasicTypes::Byte buffer;

	if(mInternalReadStatus != PDFHummus::eFailure)
	{
		if(mReaderStream->Read(&buffer,1) != 1)
			mInternalReadStatus = PDFHummus::eFailure;
		else
			outByte = buffer;
	}
	return mInternalReadStatus;
}

EStatusCode PFMFileReader::ReadWord(WORD& outWord)
{
	IOBasicTypes::Byte buffer;
	outWord = 0;

	if(mInternalReadStatus != PDFHummus::eFailure)
	{
		if(mReaderStream->Read(&buffer,1) != 1)
		{
			mInternalReadStatus = PDFHummus::eFailure;
			return PDFHummus::eFailure;
		}

		outWord = buffer;

		if(mReaderStream->Read(&buffer,1) != 1)
		{
			mInternalReadStatus = PDFHummus::eFailure;
			return PDFHummus::eFailure;
		}

		outWord |= (((WORD)buffer) << 8);
	}

	return mInternalReadStatus;
}

EStatusCode PFMFileReader::ReadDWord(DWORD& outDWORD)
{
	IOBasicTypes::Byte buffer;
	outDWORD = 0;

	if(mInternalReadStatus != PDFHummus::eFailure)
	{
		if(mReaderStream->Read(&buffer,1) != 1)
		{
			mInternalReadStatus = PDFHummus::eFailure;
			return PDFHummus::eFailure;
		}

		outDWORD = buffer;

		if(mReaderStream->Read(&buffer,1) != 1)
		{
			mInternalReadStatus = PDFHummus::eFailure;
			return PDFHummus::eFailure;
		}

		outDWORD |= (((DWORD)buffer) << 8);

		if(mReaderStream->Read(&buffer,1) != 1)
		{
			mInternalReadStatus = PDFHummus::eFailure;
			return PDFHummus::eFailure;
		}

		outDWORD |= (((DWORD)buffer) << 16);

		if(mReaderStream->Read(&buffer,1) != 1)
		{
			mInternalReadStatus = PDFHummus::eFailure;
			return PDFHummus::eFailure;
		}

		outDWORD |= (((DWORD)buffer) << 24);
	}

	return mInternalReadStatus;
}

EStatusCode PFMFileReader::ReadExtension()
{
	ReadWord(Extension.SizeFields);
	ReadDWord(Extension.ExtMetricsOffset);
	ReadDWord(Extension.ExtentTable);
	ReadDWord(Extension.OriginTable);
	ReadDWord(Extension.PairKernTable);
	ReadDWord(Extension.TrackKernTable);
	ReadDWord(Extension.DriverInfo);
	ReadDWord(Extension.Reserved);

	return mInternalReadStatus;
}

EStatusCode PFMFileReader::ReadExtendedFontMetrics()
{
	ReadWord(ExtendedFontMetrics.Size);
	ReadWord(ExtendedFontMetrics.PointSize);
	ReadWord(ExtendedFontMetrics.Orientation);
	ReadWord(ExtendedFontMetrics.MasterHeight);
	ReadWord(ExtendedFontMetrics.MinScale);
	ReadWord(ExtendedFontMetrics.MaxScale);
	ReadWord(ExtendedFontMetrics.MasterUnits);
	ReadWord(ExtendedFontMetrics.CapHeight);
	ReadWord(ExtendedFontMetrics.XHeight);
	ReadWord(ExtendedFontMetrics.LowerCaseAscent);
	ReadWord(ExtendedFontMetrics.LowerCaseDescent);
	ReadWord(ExtendedFontMetrics.Slant);
	ReadWord(ExtendedFontMetrics.SuperScript);
	ReadWord(ExtendedFontMetrics.SubScript);
	ReadWord(ExtendedFontMetrics.UnderlineOffset);
	ReadWord(ExtendedFontMetrics.UnderlineWidth);
	ReadWord(ExtendedFontMetrics.DoubleUpperUnderlineOffset);
	ReadWord(ExtendedFontMetrics.DoubleLowerUnderlineOffset);
	ReadWord(ExtendedFontMetrics.DoubleUpperUnderlineWidth);
	ReadWord(ExtendedFontMetrics.DoubleLowerUnderlineWidth);
	ReadWord(ExtendedFontMetrics.StrikeOutOffset);
	ReadWord(ExtendedFontMetrics.StrikeOutWidth);
	ReadWord(ExtendedFontMetrics.KernPairs);
	ReadWord(ExtendedFontMetrics.KernTracks);

	return mInternalReadStatus;
}