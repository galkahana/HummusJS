/*
Source File : RC4.h


Copyright 2016 Gal Kahana PDFWriter

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

#include "IOBasicTypes.h"
#include <list>

typedef std::list<IOBasicTypes::Byte> ByteList;

class RC4
{
public:
	RC4();
	RC4(const ByteList& inKey);
	RC4(const IOBasicTypes::Byte* inKey,IOBasicTypes::LongBufferSizeType inLength);
	~RC4(void);

	void Reset(const ByteList& inKey);
	void Reset(const IOBasicTypes::Byte* inKey, IOBasicTypes::LongBufferSizeType inLength);


	IOBasicTypes::Byte DecodeNextByte(IOBasicTypes::Byte inByte);
	IOBasicTypes::Byte GetNextEncodingByte();

private:

	IOBasicTypes::Byte mBuffer[256];
	int mI;
	int mJ;

	void Init(const IOBasicTypes::Byte* inKey, IOBasicTypes::LongBufferSizeType inLength);
	void Swap(int a, int b);

};