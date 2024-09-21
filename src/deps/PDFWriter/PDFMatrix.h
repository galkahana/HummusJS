/*
   Source File : PDFMatrix.h


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

#include "PDFRectangle.h"

class PDFMatrix {
public:
    /*
        x' = ax + cy + e
        y' = bx + dy + f
    
    */
	PDFMatrix(void);
	~PDFMatrix(void);

	PDFMatrix(double inA, double inB, double inC, double inD, double inE, double inF);
	PDFMatrix(const PDFMatrix& inOther);
    
    bool operator==(const PDFMatrix& inOther) const;
    bool operator!=(const PDFMatrix& inOther) const;

    PDFMatrix Multiply(const PDFMatrix& inRight) const;
    PDFRectangle Transform(const PDFRectangle& inRect) const;
    PDFMatrix Inverse() const;
    double Determinante() const;


	double a;
	double b;
	double c;
	double d;
	double e;
	double f;

    void TransformVector(const double (&vector)[2],double (&vectorResult)[2]) const; // too lazy to create PDFVector...excuse me :)

};