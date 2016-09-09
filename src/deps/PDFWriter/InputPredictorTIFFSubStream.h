/*
   Source File : InputPredictorTIFFSubStream.h


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

#include "IByteReader.h"

class InputPredictorTIFFSubStream : public IByteReader
{
public:
	InputPredictorTIFFSubStream(void);
	virtual ~InputPredictorTIFFSubStream(void);

	// Takes ownership (use Assign(NULL,0,0,0) to unassign)
	InputPredictorTIFFSubStream(IByteReader* inSourceStream,
								IOBasicTypes::LongBufferSizeType inColors,
								IOBasicTypes::Byte inBitsPerComponent,
								IOBasicTypes::LongBufferSizeType inColumns);

	virtual IOBasicTypes::LongBufferSizeType Read(IOBasicTypes::Byte* inBuffer,IOBasicTypes::LongBufferSizeType inBufferSize);

	virtual bool NotEnded();

	// Takes ownership (use Assign(NULL,0,0,0) to unassign)
	void Assign(IByteReader* inSourceStream,
				IOBasicTypes::LongBufferSizeType inColors,
				IOBasicTypes::Byte inBitsPerComponent,
				IOBasicTypes::LongBufferSizeType inColumns);

private:
	IByteReader* mSourceStream;
	IOBasicTypes::LongBufferSizeType mColors;
	IOBasicTypes::Byte mBitsPerComponent;
	IOBasicTypes::LongBufferSizeType mColumns;
	
	IOBasicTypes::Byte* mRowBuffer;
	IOBasicTypes::LongBufferSizeType mReadColorsCount;
	unsigned short* mReadColors;
	unsigned short* mReadColorsIndex;
	IOBasicTypes::Byte mIndexInColor;
	unsigned short mBitMask;

	void ReadByteFromColorsArray(IOBasicTypes::Byte& outBuffer);
	void DecodeBufferToColors();


};
