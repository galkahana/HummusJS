/*
   Source File : PFMFileReader.h


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
/*	
	Hey, initially this is not a full fledged PFM file reader.
	i'm just reading the bare minimum to get some required type1 font
	values
*/

#include "EStatusCode.h"
#include "InputFile.h"


typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

struct PFMHeader
{
	WORD Version;
	DWORD Size;
	BYTE Copyright[60];
	WORD Type;
	WORD Point;
	WORD VertRes;
	WORD HorizRes;
	WORD Ascent;
	WORD InternalLeading;
	WORD ExternalLeading;
	BYTE Italic;
	BYTE Underline;
	BYTE StrikeOut;
	WORD Weight;
	BYTE CharSet;
	WORD PixWidth;
	WORD PixHeight;
	BYTE PitchAndFamily;
	WORD AvgWidth;
	WORD MaxWidth;
	BYTE FirstChar;
	BYTE LastChar;
	Byte DefaultChar;
	Byte BreakChar;
	WORD WidthBytes;
	DWORD Device;
	DWORD Face;
	DWORD BitsPinter;
	DWORD BitsOffset;

};

struct PFMExtension
{
	WORD SizeFields;
	DWORD ExtMetricsOffset;
	DWORD ExtentTable;
	DWORD OriginTable;
	DWORD PairKernTable;
	DWORD TrackKernTable;
	DWORD DriverInfo;
	DWORD Reserved;
};

struct PFMExtendedFontMetrics
{
	WORD Size;
	WORD PointSize;
	WORD Orientation;
	WORD MasterHeight;
	WORD MinScale;
	WORD MaxScale;
	WORD MasterUnits;
	WORD CapHeight;
	WORD XHeight;
	WORD LowerCaseAscent;
	WORD LowerCaseDescent;
	WORD Slant;
	WORD SuperScript;
	WORD SubScript;
	WORD UnderlineOffset;
	WORD UnderlineWidth;
	WORD DoubleUpperUnderlineOffset;
	WORD DoubleLowerUnderlineOffset;
	WORD DoubleUpperUnderlineWidth;
	WORD DoubleLowerUnderlineWidth;
	WORD StrikeOutOffset;
	WORD StrikeOutWidth;
	WORD KernPairs;
	WORD KernTracks;
};

class IByteReaderWithPosition;

class PFMFileReader
{
public:
	PFMFileReader(void);
	~PFMFileReader(void);

	PDFHummus::EStatusCode Read(const std::string& inPFMFilePath);

	PFMHeader Header;
	PFMExtension Extension;
	PFMExtendedFontMetrics ExtendedFontMetrics;

	// as i said, read just what i need

private:

	IByteReaderWithPosition* mReaderStream;
	PDFHummus::EStatusCode mInternalReadStatus;

	PDFHummus::EStatusCode ReadByte(BYTE& outByte);
	PDFHummus::EStatusCode ReadWord(WORD& outWORD);
	PDFHummus::EStatusCode ReadDWord(DWORD& outDWORD);

	PDFHummus::EStatusCode ReadHeader();
	PDFHummus::EStatusCode ReadExtension();
	PDFHummus::EStatusCode ReadExtendedFontMetrics();
};
