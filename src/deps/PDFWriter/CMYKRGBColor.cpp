/*
   Source File : CMYKRGBColor.cpp


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
#include "CMYKRGBColor.h"

CMYKRGBColor::CMYKRGBColor(void)
{
	UseCMYK = false;
	RGBComponents[0] = RGBComponents[1] = RGBComponents[2] = 0;
	CMYKComponents[0] = CMYKComponents[1] = CMYKComponents[2] = CMYKComponents[3] =0;
}

CMYKRGBColor::~CMYKRGBColor(void)
{
}


CMYKRGBColor::CMYKRGBColor(unsigned char inR,unsigned char inG, unsigned char inB)
{
	UseCMYK = false;
	RGBComponents[0] = inR;
	RGBComponents[1] = inG;
	RGBComponents[2] = inB;
	CMYKComponents[0] = CMYKComponents[1] = CMYKComponents[2] = CMYKComponents[3] =0;
}

CMYKRGBColor::CMYKRGBColor(unsigned char inC,unsigned char inM, unsigned char inY, unsigned char inK)
{
	UseCMYK = true;
	CMYKComponents[0] = inC;
	CMYKComponents[1] = inM;
	CMYKComponents[2] = inY;
	CMYKComponents[3] = inK;
	RGBComponents[0] = RGBComponents[1] = RGBComponents[2] = 0;
}

const CMYKRGBColor& CMYKRGBColor::CMYKBlack(){
	static CMYKRGBColor oCMYKBlack(0, 0, 0, 255);
	return oCMYKBlack;
};
const CMYKRGBColor& CMYKRGBColor::CMYKWhite(){
	static CMYKRGBColor oCMYKWhite(0, 0, 0, 0);
	return oCMYKWhite;
};
