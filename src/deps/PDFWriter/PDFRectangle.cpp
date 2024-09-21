/*
   Source File : PDFRectangle.cpp


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
#include "PDFRectangle.h"

PDFRectangle::PDFRectangle(void)
{
	LowerLeftX = 0;
	LowerLeftY = 0;
	UpperRightX = 0;
	UpperRightY = 0;
}

PDFRectangle::~PDFRectangle(void)
{
}

PDFRectangle::PDFRectangle(double inLowerLeftX, double inLowerLeftY, double inUpperRightX, double inUpperRightY)
{
	LowerLeftX = inLowerLeftX;
	LowerLeftY = inLowerLeftY;
	UpperRightX = inUpperRightX;
	UpperRightY = inUpperRightY;
}

PDFRectangle::PDFRectangle(const PDFRectangle& inOther)
{
	LowerLeftX = inOther.LowerLeftX;
	LowerLeftY = inOther.LowerLeftY;
	UpperRightX = inOther.UpperRightX;
	UpperRightY = inOther.UpperRightY;
}

bool PDFRectangle::operator==(const PDFRectangle& inOther) const
{
    return  LowerLeftX == inOther.LowerLeftX &&
            LowerLeftY == inOther.LowerLeftY &&
            UpperRightX == inOther.UpperRightX &&
            UpperRightY == inOther.UpperRightY;    
}

bool PDFRectangle::operator!=(const PDFRectangle& inOther) const
{
    return  LowerLeftX != inOther.LowerLeftX ||
            LowerLeftY != inOther.LowerLeftY ||
            UpperRightX != inOther.UpperRightX ||
            UpperRightY != inOther.UpperRightY;   
}

double PDFRectangle::GetWidth() {
    return UpperRightX - LowerLeftX;
}

double PDFRectangle::GetHeight() {
    return UpperRightY - LowerLeftY;
}