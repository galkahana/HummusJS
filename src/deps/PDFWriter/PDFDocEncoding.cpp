/*
   Source File : PDFDocEncoding.cpp


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
#include "PDFDocEncoding.h"
#include "BetweenIncluding.h"

static const char* scGlyphNames[256] =
{
		".notdef", ".notdef", ".notdef", ".notdef",".notdef", ".notdef", ".notdef", ".notdef",
		".notdef", ".notdef", ".notdef", ".notdef",".notdef", ".notdef", ".notdef", ".notdef",
		".notdef", ".notdef", ".notdef", ".notdef",".notdef", ".notdef", ".notdef", ".notdef",
		"breve", "caron", "circumflex", "dotaccent","hungarumlaut", "ogonek", "ring", ".notdef",
		"space", "exclam", "quotedbl", "numbersign","dollar", "percent", "ampersand", "quotesingle",
		"parenleft", "parenright", "asterisk", "plus","comma", "hyphen", "period", "slash",
		"zero", "one", "two", "three","four", "five", "six", "seven",
		"eight", "nine", "colon", "semicolon","less", "equal", "greater", "question",
		"at", "A", "B", "C","D", "E", "F", "G",
		"H", "I", "J", "K","L", "M", "N", "O",
		"P", "Q", "R", "S","T", "U", "V", "W",
		"X", "Y", "Z", "bracketleft","backslash", "bracketright", "asciicircum", "underscore",
		"grave", "a", "b", "c","d", "e", "f", "g",
		"h", "i", "j", "k","l", "m", "n", "o",
		"p", "q", "r", "s","t", "u", "v", "w",
		"x", "y", "z", "braceleft","bar", "braceright", "asciitilde", ".notdef",
		"bullet", "dagger", "daggerdbl", "ellipsis","emdash", "endash", "florin", "fraction",
		"guilsinglleft", "guilsinglright", "minus", "perthousand","quotedblbase", "quotedblleft", "quotedblright", "quoteleft",
		"quoteright", "quotesinglbase", "trademark", "fi","fl", "Lslash", "OE", "Scaron",
		"Ydieresis", "Zcaron", "dotlessi", "lslash","oe", "scaron", "zcaron", ".notdef",
		"Euro", "exclamdown", "cent", "sterling","currency", "yen", "brokenbar", "section",
		"dieresis", "copyright", "ordfeminine", "guillemotleft","logicalnot", ".notdef", "registered", "macron",
		"degree", "plusminus", "twosuperior", "threesuperior","acute", "mu", "paragraph", "periodcentered",
		"cedilla", "onesuperior", "ordmasculine", "guillemotright","onequarter", "onehalf", "threequarters", "questiondown",
		"Agrave", "Aacute", "Acircumflex", "Atilde","Adieresis", "Aring", "AE", "Ccedilla",
		"Egrave", "Eacute", "Ecircumflex", "Edieresis","Igrave", "Iacute", "Icircumflex", "Idieresis",
		"Eth", "Ntilde", "Ograve", "Oacute","Ocircumflex", "Otilde", "Odieresis", "multiply",
		"Oslash", "Ugrave", "Uacute", "Ucircumflex","Udieresis", "Yacute", "Thorn", "germandbls",
		"agrave", "aacute", "acircumflex", "atilde","adieresis", "aring", "ae", "ccedilla",
		"egrave", "eacute", "ecircumflex", "edieresis","igrave", "iacute", "icircumflex", "idieresis",
		"eth", "ntilde", "ograve", "oacute","ocircumflex", "otilde", "odieresis", "divide",
		"oslash", "ugrave", "uacute", "ucircumflex","udieresis", "yacute", "thorn", "ydieresis"
};


PDFDocEncoding::PDFDocEncoding(void)
{
}

PDFDocEncoding::~PDFDocEncoding(void)
{
}

BoolAndByte PDFDocEncoding::Encode(unsigned long inUnicodeCharacter)
{
	BoolAndByte result(true,0);

	if(	betweenIncluding<unsigned long>(inUnicodeCharacter,0x00,0x17) ||
		betweenIncluding<unsigned long>(inUnicodeCharacter,0x20,0x7E) ||
		betweenIncluding<unsigned long>(inUnicodeCharacter,0xA1,0xFF))
	{
		result.second = (char)inUnicodeCharacter;
	}
	else 
	{
		switch(inUnicodeCharacter)
		{
			case 0x02D8:
				result.second = 0x18;
				break;
			case 0x02C7:
				result.second = 0x19;
				break;
			case 0x02C6:
				result.second = 0x1a;
				break;
			case 0x02D9:
				result.second = 0x1b;
				break;
			case 0x02DD:
				result.second = 0x1c;
				break;
			case 0x02DB:
				result.second = 0x1d;
				break;
			case 0x02DA:
				result.second = 0x1e;
				break;
			case 0x02DC:
				result.second = 0x1f;
				break;
			case 0x2022:
				result.second = 0x80;
				break;
			case 0x2020:
				result.second = 0x81;
				break;
			case 0x2021:
				result.second = 0x82;
				break;
			case 0x2026:
				result.second = 0x83;
				break;
			case 0x2014:
				result.second = 0x84;
				break;
			case 0x2013:
				result.second = 0x85;
				break;
			case 0x192:
				result.second = 0x86;
				break;
			case 0x2044:
				result.second = 0x87;
				break;
			case 0x2039:
				result.second = 0x88;
				break;
			case 0x203A:
				result.second = 0x89;
				break;
			case 0x2212:
				result.second = 0x8a;
				break;
			case 0x2030:
				result.second = 0x8b;
				break;
			case 0x201E:
				result.second = 0x8c;
				break;
			case 0x201C:
				result.second = 0x8d;
				break;
			case 0x201D:
				result.second = 0x8e;
				break;
			case 0x2018:
				result.second = 0x8f;
				break;
			case 0x2019:
				result.second = 0x90;
				break;
			case 0x201A:
				result.second = 0x91;
				break;
			case 0x2122:
				result.second = 0x92;
				break;
			case 0xFB01:
				result.second = 0x93;
				break;
			case 0xFB02:
				result.second = 0x94;
				break;
			case 0x141:
				result.second = 0x95;
				break;
			case 0x152:
				result.second = 0x96;
				break;
			case 0x160:
				result.second = 0x97;
				break;
			case 0x178:
				result.second = 0x98;
				break;
			case 0x17D:
				result.second = 0x99;
				break;
			case 0x131:
				result.second = 0x9a;
				break;
			case 0x142:
				result.second = 0x9b;
				break;
			case 0x153:
				result.second = 0x9c;
				break;
			case 0x161:
				result.second = 0x9d;
				break;
			case 0x17E:
				result.second = 0x9e;
				break;
			case 0x20AC:
				result.second = 0xa0;
				break;
			default:
				result.first = false;
		}
	}
	return result;
}

const char* PDFDocEncoding::GetEncodedGlyphName(IOBasicTypes::Byte inEncodedCharacter)
{
	return scGlyphNames[inEncodedCharacter];
}

unsigned long PDFDocEncoding::Decode(IOBasicTypes::Byte inEncodedCharacter)
{
	unsigned long result;

	if(	betweenIncluding<IOBasicTypes::Byte>(inEncodedCharacter,0x00,0x17) ||
		betweenIncluding<IOBasicTypes::Byte>(inEncodedCharacter,0x20,0x7E) ||
		betweenIncluding<IOBasicTypes::Byte>(inEncodedCharacter,0xA1,0xFF))
	{
		result = (unsigned long)inEncodedCharacter;
	}
	else 
	{
		switch(inEncodedCharacter)
		{
			case 0x18:
				result = 0x02D8;
				break;
			case 0x19:
				result = 0x02C7;
				break;
			case 0x1a:
				result = 0x02C6;
				break;
			case 0x1b:
				result = 0x02D9;
				break;
			case 0x1c:
				result = 0x02DD;
				break;
			case 0x1d:
				result = 0x02DB;
				break;
			case 0x1e:
				result = 0x02DA;
				break;
			case 0x1f:
				result = 0x02DC;
				break;
			case 0x80:
				result = 0x2022;
				break;
			case 0x81:
				result = 0x2020;
				break;
			case 0x82:
				result = 0x2021;
				break;
			case 0x83:
				result = 0x2026;
				break;
			case 0x84:
				result = 0x2014;
				break;
			case 0x85:
				result = 0x2013;
				break;
			case 0x86:
				result = 0x192;
				break;
			case 0x87:
				result = 0x2044;
				break;
			case 0x88:
				result = 0x2039;
				break;
			case 0x89:
				result = 0x203A;
				break;
			case 0x8a:
				result = 0x2212;
				break;
			case 0x8b:
				result = 0x2030;
				break;
			case 0x8c:
				result = 0x201E;
				break;
			case 0x8d:
				result = 0x201C;
				break;
			case 0x8e:
				result = 0x201D;
				break;
			case 0x8f:
				result = 0x2018;
				break;
			case 0x90:
				result = 0x2019;
				break;
			case 0x91:
				result = 0x201A;
				break;
			case 0x92:
				result = 0x2122;
				break;
			case 0x93:
				result = 0xFB01;
				break;
			case 0x94:
				result = 0xFB02;
				break;
			case 0x95:
				result = 0x141;
				break;
			case 0x96:
				result = 0x152;
				break;
			case 0x97:
				result = 0x160;
				break;
			case 0x98:
				result = 0x178;
				break;
			case 0x99:
				result = 0x17D;
				break;
			case 0x9a:
				result = 0x131;
				break;
			case 0x9b:
				result = 0x142;
				break;
			case 0x9c:
				result = 0x153;
				break;
			case 0x9d:
				result = 0x161;
				break;
			case 0x9e:
				result = 0x17E;
				break;
			case 0xa0:
				result = 0x20AC;
				break;
			default:
				result = 0;
		}
	}
	return result;
}
