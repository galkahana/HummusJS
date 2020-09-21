/*
   Source File : InputLZWDecodeStream.h


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
#include "IByteReader.h"

class InputLZWDecodeStream : public IByteReader
{
public:
	InputLZWDecodeStream(int early);

	// Note that assigning passes ownership on the stream, use Assign(NULL) to remove ownership
	InputLZWDecodeStream(IByteReader* inSourceReader);
	virtual ~InputLZWDecodeStream(void);

	// Assigning passes ownership of the input stream to the decoder stream. 
	// if you don't care for that, then after finishing with the decode, Assign(NULL).
	void Assign(IByteReader* inSourceReader);

	// IByteReader implementation. note that "inBufferSize" determines how many
	// bytes will be placed in the Buffer...not how many are actually read from the underlying
	// encoded stream. got it?!
	virtual IOBasicTypes::LongBufferSizeType Read(IOBasicTypes::Byte* inBuffer,IOBasicTypes::LongBufferSizeType inBufferSize);

	virtual bool NotEnded();

private:
	
	IByteReader* mSourceStream;
	
	bool mCurrentlyEncoding;
	
	int mEarly;			// early parameter
	int inputBuf;		// input buffer
	int inputBits;		// number of bits in input buffer

	struct {			// decoding table
		int length;
		int head;
		IOBasicTypes::Byte tail;
	} table[4097];
	int nextCode;			// next code to be used
	int nextBits;			// number of bits in next code word
	int prevCode;			// previous code used in stream
	int newChar;			// next char to be added to table
	IOBasicTypes::Byte		seqBuf[4097];		// buffer for current sequence
	int seqLength;		// length of current sequence
	int seqIndex;			// index into current sequence
	bool first;			// first code after a table clear

	bool ProcessNextCode();
	void ClearTable();
	int GetCode();

	void FinalizeEncoding();	
	void StartEncoding();

};
