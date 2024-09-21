/*
   Source File : PDFMatrix.cpp


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

#include "PDFMatrix.h"

PDFMatrix::PDFMatrix(void) {
    a = 1;
    b = 0;
    c = 0;
    d = 1;
    e = 0;
    f = 0;
}

PDFMatrix::~PDFMatrix(void) {

}

PDFMatrix::PDFMatrix(double inA, double inB, double inC, double inD, double inE, double inF) {
    a = inA;
    b = inB;
    c = inC;
    d = inD;
    e = inE;
    f = inF;
}

PDFMatrix::PDFMatrix(const PDFMatrix& inOther) {
    a = inOther.a;
    b = inOther.b;
    c = inOther.c;
    d = inOther.d;
    e = inOther.e;
    f = inOther.f;
}

bool PDFMatrix::operator==(const PDFMatrix& inOther) const {
    return a == inOther.a &&
        b == inOther.b &&
        c == inOther.c &&
        d == inOther.d &&
        e == inOther.e &&
        f == inOther.f;
}

bool PDFMatrix::operator!=(const PDFMatrix& inOther) const {
    return a != inOther.a ||
        b != inOther.b ||
        c != inOther.c ||
        d != inOther.d ||
        e != inOther.e ||
        f != inOther.f;

}

PDFMatrix PDFMatrix::Multiply(const PDFMatrix& inRight) const {
    return PDFMatrix(
        a*inRight.a + b*inRight.c,
        a*inRight.b + b*inRight.d,
        c*inRight.a + d*inRight.c,
        c*inRight.b + d*inRight.d,
        e*inRight.a + f*inRight.c + inRight.e,
        e*inRight.b + f*inRight.d + inRight.f
    );
}

void PDFMatrix::TransformVector(const double (&vector)[2], double (&vectorResult)[2]) const{
    vectorResult[0] = a*vector[0] + c*vector[1] + e;
    vectorResult[1] = b*vector[0] + d*vector[1] + f;
}


PDFRectangle PDFMatrix::Transform(const PDFRectangle& inRect) const{
    double t[4][2];

    double a[2] = {inRect.LowerLeftX, inRect.LowerLeftY};
    double b[2] = {inRect.LowerLeftX, inRect.UpperRightY};
    double c[2] = {inRect.UpperRightX, inRect.UpperRightY};
    double d[2] = {inRect.UpperRightX, inRect.LowerLeftY};
    TransformVector(a, t[0]);
    TransformVector(b, t[1]);
    TransformVector(c, t[2]);
    TransformVector(d, t[3]);

    double minX,minY,maxX,maxY;
    
    minX = maxX = t[0][0];
    minY = maxY = t[0][1];
    
    for(int i=1;i<4;++i)
    {
        if(minX > t[i][0])
            minX = t[i][0];
        if(maxX < t[i][0])
            maxX = t[i][0];
        if(minY > t[i][1])
            minY = t[i][1];
        if(maxY < t[i][1])
            maxY = t[i][1];
    }

    return PDFRectangle(minX,minY,maxX,maxY);

}

PDFMatrix PDFMatrix::Inverse() const {
    double det = Determinante();
    
    return PDFMatrix(
        d/det,
        -b/det,
        -c/det,
        a/det,
        (c*f-d*e)/det,
        (b*e-a*f)/det        
    );
}

double PDFMatrix::Determinante()  const{
    return a*d-b*c;
}