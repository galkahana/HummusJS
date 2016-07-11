/*
   Source File : MD5Generator.cpp


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

   
 Additional Copyright Information:

 Gal Kahana 8/5/2010. my code is completely copied/adapted from this:

 MD5.CC - source code for the C++/object oriented translation and 
          modification of MD5.

 Translation and modification (c) 1995 by Mordechai T. Abzug 

 This translation/ modification is provided "as is," without express or 
 implied warranty of any kind.

 The translator/ modifier does not claim (1) that MD5 will do what you think 
 it does; (2) that this translation/ modification is accurate; or (3) that 
 this software is "merchantible."  (Language for this disclaimer partially 
 copied from the disclaimer below).

 the code is based on:

 MD5.H - header file for MD5C.C
 MDDRIVER.C - test driver for MD2, MD4 and MD5

 Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
 rights reserved.

 License to copy and use this software is granted provided that it
 is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 Algorithm" in all material mentioning or referencing this software
 or this function.

 License is also granted to make and use derivative works provided
 that such works are identified as "derived from the RSA Data
 Security, Inc. MD5 Message-Digest Algorithm" in all material
 mentioning or referencing the derived work.

 RSA Data Security, Inc. makes no representations concerning either
 the merchantability of this software or the suitability of this
 software for any particular purpose. It is provided "as is"
 without express or implied warranty of any kind.

 These notices must be retained in any copies of any part of this
 documentation and/or software.



*/
#include "MD5Generator.h"
#include "OutputStringBufferStream.h"
#include "SafeBufferMacrosDefs.h"

#include <string.h>

using namespace IOBasicTypes;
using namespace PDFHummus;

MD5Generator::MD5Generator(void)
{
	mIsFinalized = false;

	// Nothing counted, so count=0
	mCount[0] = 0;
	mCount[1] = 0;

	// Load magic initialization constants.
	mState[0] = 0x67452301;
	mState[1] = 0xefcdab89;
	mState[2] = 0x98badcfe;
	mState[3] = 0x10325476;
}

MD5Generator::~MD5Generator(void)
{
}

EStatusCode MD5Generator::Accumulate(const std::string& inString) {
	if (mIsFinalized)
		return PDFHummus::eFailure;

	_Accumulate((const Byte*)inString.c_str(), (unsigned long)inString.length());
	return PDFHummus::eSuccess;
}

EStatusCode MD5Generator::Accumulate(const ByteList& inString)
{
	if(mIsFinalized)
		return PDFHummus::eFailure;

	Byte* buffer = new Byte[inString.size()];
	Byte* itBuffer = buffer;
	ByteList::const_iterator it = inString.begin();
	for (; it != inString.end(); ++it,++itBuffer)
		*itBuffer = *it;
	
	_Accumulate((const uint1*)buffer,(unsigned long)inString.size());
	delete[] buffer;
	return PDFHummus::eSuccess;

}

EStatusCode MD5Generator::Accumulate(const Byte* inArray, LongBufferSizeType inLength) {
	if (mIsFinalized)
		return PDFHummus::eFailure;

	_Accumulate(inArray, (unsigned long)inLength);
	return PDFHummus::eSuccess;
}

void MD5Generator::_Accumulate(const uint1* inBlock,unsigned long inBlockSize)
{
  uint4 input_index, buffer_index;
  uint4 buffer_space;                // how much space is left in buffer

  // Compute number of bytes mod 64
  buffer_index = (unsigned int)((mCount[0] >> 3) & 0x3F);

  // Update number of bits
  if (  (mCount[0] += ((uint4)inBlockSize << 3))<((uint4) inBlockSize << 3) )
    mCount[1]++;

  mCount[1] += ((uint4)inBlockSize  >> 29);


  buffer_space = 64 - buffer_index;  // how much space is left in buffer

  // Transform as many times as possible.
  if (inBlockSize >= buffer_space) { // ie. we have enough to fill the buffer
    // fill the rest of the buffer and transform
    memcpy(mBuffer + buffer_index,inBlock, buffer_space);
    Transform(mBuffer);

    // now, transform each 64-byte piece of the input, bypassing the buffer
    for (input_index = buffer_space; input_index + 63 < inBlockSize; 
	 input_index += 64)
      Transform(inBlock+input_index);

    buffer_index = 0;  // so we can buffer remaining
  }
  else
    input_index=0;     // so we can buffer the whole input


  // and here we do the buffering:
  memcpy(mBuffer+buffer_index, inBlock+input_index, inBlockSize-input_index);
}


// Constants for MD5Transform routine.
// Although we could use C++ style constants, defines are actually better,
// since they let us easily evade scope clashes.

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21


void MD5Generator::Transform(const uint1 *inBuffer)
{
  uint4 a = mState[0], b = mState[1], c = mState[2], d = mState[3], x[16];

  Decode (inBuffer, 64,x);

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

 /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  mState[0] += a;
  mState[1] += b;
  mState[2] += c;
  mState[3] += d;

  // Zeroize sensitive information.
  memset ( (uint1 *) x, 0, sizeof(x));	
}

// Decodes input (unsigned char) into output (UINT4). Assumes len is
// a multiple of 4.
void MD5Generator::Decode(const uint1 *inInput, uint4 inInputLen,uint4 *outOutput)
{

  unsigned int i, j;

  for (i = 0, j = 0; j < inInputLen; i++, j += 4)
    outOutput[i] = ((uint4)inInput[j]) | (((uint4)inInput[j+1]) << 8) |
      (((uint4)inInput[j+2]) << 16) | (((uint4)inInput[j+3]) << 24);
}


// ROTATE_LEFT rotates x left n bits.

unsigned int MD5Generator::RotateLeft(uint4 x, uint4 n)
{
  return (x << n) | (x >> (32-n));
}

// F, G, H and I are basic MD5 functions.

unsigned int MD5Generator::F(uint4 x, uint4 y, uint4 z)
{
  return (x & y) | (~x & z);
}

unsigned int MD5Generator::G(uint4 x, uint4 y, uint4 z)
{
  return (x & z) | (y & ~z);
}

unsigned int MD5Generator::H(uint4 x, uint4 y, uint4 z)
{
  return x ^ y ^ z;
}

unsigned int MD5Generator::I(uint4 x, uint4 y, uint4 z)
{
  return y ^ (x | ~z);
}

// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
// Rotation is separate from addition to prevent recomputation.
void MD5Generator::FF(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, 
		    uint4  s, uint4 ac)
{
 a += F(b, c, d) + x + ac;
 a = RotateLeft (a, s) +b;
}

void MD5Generator::GG(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
{
 a += G(b, c, d) + x + ac;
 a = RotateLeft (a, s) +b;
}

void MD5Generator::HH(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
{
 a += H(b, c, d) + x + ac;
 a = RotateLeft (a, s) +b;
}

void MD5Generator::II(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
{
 a += I(b, c, d) + x + ac;
 a = RotateLeft (a, s) +b;
}

const MD5Generator::uint1 MD5Generator::PADDING[64]={
0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const std::string& MD5Generator::ToHexString()
{
	Finalize();
	return MD5FinalHexString;
}

const ByteList& MD5Generator::ToString()
{
	Finalize();
	return MD5FinalString;
}

const std::string& MD5Generator::ToStringAsString()
{
	Finalize();
	return MD5FinalStringAsString;
}


void MD5Generator::Finalize() {
	if (!mIsFinalized)
	{
		unsigned char bits[8];
		unsigned int index, padLen;

		// Save number of bits
		Encode(bits, mCount, 8);

		// Pad out to 56 mod 64.
		index = (uint4)((mCount[0] >> 3) & 0x3f);
		padLen = (index < 56) ? (56 - index) : (120 - index);
		Accumulate(PADDING, padLen);

		// Append length (before padding)
		Accumulate(bits, 8);

		// Store state in digest
		Encode(mDigest, mState, 16);

		// Zeroize sensitive information
		memset(mBuffer, 0, sizeof(*mBuffer));

		mIsFinalized = true;
		PrepareFinalStrings();
	}
}

void MD5Generator::Encode(uint1 *output, uint4 *input, uint4 len) 
{

  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4) {
    output[j]   = (uint1)  (input[i] & 0xff);
    output[j+1] = (uint1) ((input[i] >> 8) & 0xff);
    output[j+2] = (uint1) ((input[i] >> 16) & 0xff);
    output[j+3] = (uint1) ((input[i] >> 24) & 0xff);
  }
}

void MD5Generator::PrepareFinalStrings()
{
	OutputStringBufferStream stringHexStream;
	char formattedHex[3];
	MD5FinalString.clear();
	
	
	for (int i=0; i<16; i++)
	{
		SAFE_SPRINTF_1(formattedHex,3,"%02x",mDigest[i]);
		stringHexStream.Write((const Byte*)formattedHex,2);
		MD5FinalString.push_back(mDigest[i]);
		MD5FinalStringAsString.push_back((char)mDigest[i]);
	}

	MD5FinalHexString = stringHexStream.ToString();
}

