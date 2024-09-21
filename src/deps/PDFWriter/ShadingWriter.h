/*
   Source File : ShadingWriter.h


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

// The following implementation derives a lot of its ideas and some actual code from Google's Skia 2D library (https://skia.org/).
// Specifically the PDF Shader implementation via SkGradientShader class. I'm therefore placing here below Skia's license and copyright notice.

/*
    Copyright (c) 2011 Google Inc. All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in
        the documentation and/or other materials provided with the
        distribution.

    * Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived
        from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


# pragma once

#include "EStatusCode.h"
#include "InterpretedGradientStop.h"
#include "PDFRectangle.h"
#include "PDFMatrix.h"
#include "ObjectsBasicTypes.h"

#include <string>

class ObjectsContext;
namespace PDFHummus
{
    class DocumentContext;  
};
class IByteWriter;
class PrimitiveObjectsWriter;
class PDFStream;

struct RadialShading {
    double x0;
    double y0;
    double r0;
    double x1;
    double y1;
    double r1;
    InterpretedGradientStopList colorLine;
    FT_PaintExtend gradientExtend;
};

struct LinearShading {
    double x0;
    double y0;
    double x1;
    double y1;
    InterpretedGradientStopList colorLine;
    FT_PaintExtend gradientExtend;
};

struct SweepShading {
    double cX;
    double cY;
    double startAngleRad;
    double endAngleRad;
    InterpretedGradientStopList colorLine;
    FT_PaintExtend gradientExtend;
};

class ShadingWriter {
    public:

        ShadingWriter(ObjectsContext* inObjectsContext, PDFHummus::DocumentContext* inDocumentContext);
        ~ShadingWriter();

        PDFHummus::EStatusCode WriteRadialShadingPatternObject(
            double inX0,
            double inY0,
            double inR0,
            double inX1,
            double inY1,
            double inR1,
            const InterpretedGradientStopList& inColorLine,
            FT_PaintExtend inGradientExtend,
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inPatternObjectId
        );

        PDFHummus::EStatusCode WriteLinearShadingPatternObject(
            double inX0,
            double inY0,
            double inX1,
            double inY1,
            const InterpretedGradientStopList& inColorLine,
            FT_PaintExtend inGradientExtend,
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inPatternObjectId
        );

        PDFHummus::EStatusCode WriteSweepShadingPatternObject(
            double inCX,
            double inCY,
            double inStartAngleRad,
            double inEndAngleRad,
            const InterpretedGradientStopList& inColorLine,
            FT_PaintExtend inGradientExtend,
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inPatternObjectId
        );

    private:

        PDFHummus::DocumentContext* mDocumentContext;
        ObjectsContext* mObjectsContext;

        bool ColorLineHasTransparency(const InterpretedGradientStopList& inColorLine);
        PDFHummus::EStatusCode WriteSoftmaskPatternObjectForRGBAShades(
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inPatternObjectId,
            ObjectIDType& outAlphaShadeObjectId,
            ObjectIDType& outRGBShadeObjectId
        );
        InterpretedGradientStopList CreateAlphaShadingColorLine(const InterpretedGradientStopList& inColorLine);
        PDFHummus::EStatusCode WriteAlphaSoftMaskExtGStateObject(
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inAlphaShadingPatternObjectId,
            ObjectIDType inExtGStateObjectId
        );
        PDFHummus::EStatusCode WriteAlphaSoftMaskXObjectFormObject(
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inAlphaShadingPatternObjectId,
            ObjectIDType inXObjectFormObjectId
        );
        PDFHummus::EStatusCode WriteShadingPatternObject(
            const PDFMatrix& inShadingMatrix,
            ObjectIDType inPatternObjectId,
            ObjectIDType& outShadingObjectId
        );        
        PDFHummus::EStatusCode WriteShadingPatternObjectWithPDFNativeFunctions(
            unsigned short inShadingType,
            double* inCoords,
            unsigned short inCoordsSize,
            const InterpretedGradientStopList& inColorLine,
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inPatternObjectId
        );        
        PDFStream* StartCustomFunctionShadingPatternStream(
            const PDFMatrix& inPatternProgramMatrix,
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inPatternObjectId
        );        

        PDFHummus::EStatusCode WriteShadingType1Object(
            const PDFRectangle& inBounds, 
            ObjectIDType inShadingObjectId, 
            ObjectIDType& outFunctionObjectId
        );
        void WriteStreamText(IByteWriter* inWriter, const std::string& inText);
        void WriteColorLineStepsProgram(
            IByteWriter* inWriter,
            PrimitiveObjectsWriter* inPrimitiveWriter,
            FT_PaintExtend inGradientExtend,
            const InterpretedGradientStopList& inColorLine);
        void WriteColorInterpolation(
            IByteWriter* inWriter,
            PrimitiveObjectsWriter* inPrimitiveWriter,
            FT_Byte inColorStart, 
            FT_Byte inColorEnd, 
            double inStopDiff);

        // Radial
        PDFHummus::EStatusCode WriteRGBATiledPatternObject(
            const RadialShading& inRadialShading, 
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inPatternObjectId
        );
        PDFHummus::EStatusCode WriteRGBShadingPatternObject(
            const RadialShading& inRadialShading, 
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inPatternObjectId
        );
        PDFHummus::EStatusCode WriteShadingPatternObjectWithPDFNativeFunctions(
            const RadialShading& inRadialShading, 
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inPatternObjectId
        );        
        PDFHummus::EStatusCode WriteShadingPatternObjectWithPDFCustomFunctions(
            const RadialShading& inRadialShading, 
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inPatternObjectId
        );        
        void WriteGradientCustomFunctionProgram(
            const RadialShading& inRadialShading, 
            IByteWriter* inWriter,
            PrimitiveObjectsWriter* inPrimitiveWriter);


        // Linear
        PDFHummus::EStatusCode WriteRGBATiledPatternObject(
            const LinearShading& inLinearShading, 
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inPatternObjectId
        );
        PDFHummus::EStatusCode WriteRGBShadingPatternObject(
            const LinearShading& inLinearShading, 
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inPatternObjectId
        );
        PDFHummus::EStatusCode WriteShadingPatternObjectWithPDFNativeFunctions(
            const LinearShading& inLinearShading, 
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inPatternObjectId
        );        
        PDFHummus::EStatusCode WriteShadingPatternObjectWithPDFCustomFunctions(
            const LinearShading& inLinearShading, 
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inPatternObjectId
        );        
        void WriteGradientCustomFunctionProgram(
            const LinearShading& inLinearShading, 
            IByteWriter* inWriter,
            PrimitiveObjectsWriter* inPrimitiveWriter);

        // Sweep
        PDFHummus::EStatusCode WriteRGBATiledPatternObject(
            const SweepShading& inSweepShading, 
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inPatternObjectId
        );
        PDFHummus::EStatusCode WriteRGBShadingPatternObject(
            const SweepShading& inSweepShading, 
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inPatternObjectId
        );
        PDFHummus::EStatusCode WriteShadingPatternObjectWithPDFCustomFunctions(
            const SweepShading& inSweepShading, 
            const PDFRectangle& inBounds,
            const PDFMatrix& inMatrix,
            ObjectIDType inPatternObjectId
        );        
        void WriteGradientCustomFunctionProgram(
            const SweepShading& inSweepShading, 
            IByteWriter* inWriter,
            PrimitiveObjectsWriter* inPrimitiveWriter);            
};