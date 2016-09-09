/*
Source File : RC4.cpp


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

#include "RC4.h"

using namespace IOBasicTypes;

RC4::RC4() {

}

RC4::RC4(const ByteList& inKey) {
	Reset(inKey);
}


void RC4::Reset(const ByteList& inKey) {
	Byte* buffer = new Byte[inKey.size()];
	Byte* itBuffer = buffer;
	ByteList::const_iterator it = inKey.begin();
	for (; it != inKey.end(); ++it, ++itBuffer)
		*itBuffer = *it;

	Init((const Byte*)buffer, inKey.size());
	delete[] buffer;
}


RC4::RC4(const Byte* inKey, LongBufferSizeType inLength)
{
	Reset(inKey, inLength);
}

void RC4::Reset(const IOBasicTypes::Byte* inKey, IOBasicTypes::LongBufferSizeType inLength) {
	Init(inKey, inLength);
}


void RC4::Init(const Byte* inKey, LongBufferSizeType inLength)
{
	for (int i = 0; i<256; ++i)
		mBuffer[i] = i;

	mJ = 0;

	for (mI = 0; mI<256; ++mI)
	{
		mJ = (mJ + mBuffer[mI] + inKey[mI % inLength]) % 256;
		Swap(mI, mJ);
	}

	mI = mJ = 0;
}

RC4::~RC4(void) {
}

void RC4::Swap(int a, int b) {
	Byte tmp = mBuffer[a];
	mBuffer[a] = mBuffer[b];
	mBuffer[b] = tmp;
}

Byte RC4::GetNextEncodingByte() {
	mI = (mI + 1) % 256;
	mJ = (mJ + mBuffer[mI]) % 256;
	Swap(mI, mJ);
	int index = (mBuffer[mI] + mBuffer[mJ]) % 256;
	return mBuffer[index];
}

Byte RC4::DecodeNextByte(Byte inByte) {
	return inByte ^ GetNextEncodingByte();
}
