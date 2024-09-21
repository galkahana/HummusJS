/*
   Source File : RadialGradientShadingPatternWritingTask.h


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

#include "InterpretedGradientStop.h"
#include "EStatusCode.h"
#include "IObjectEndWritingTask.h"
#include "ObjectsBasicTypes.h"
#include "PDFRectangle.h"
#include "PDFMatrix.h"

class RadialGradientShadingPatternWritingTask : public IObjectEndWritingTask {
    public:
        RadialGradientShadingPatternWritingTask(
            double inX0,
            double inY0,
            double inR0,
            double inX1,
            double inY1,
            double inR1,
            InterpretedGradientStopList inColorLine,
            FT_PaintExtend inGradientExtend,
            PDFRectangle inBounds,
            PDFMatrix inMatrix,
            ObjectIDType inPatternObjectId
        );

        virtual ~RadialGradientShadingPatternWritingTask();

        virtual PDFHummus::EStatusCode Write(ObjectsContext* inObjectsContext,
                                            PDFHummus::DocumentContext* inDocumentContext);

    private:
        double x0;
        double y0;
        double r0;
        double x1;
        double y1;
        double r1;
        InterpretedGradientStopList mColorLine;
        FT_PaintExtend mGradientExtend;
        PDFRectangle mBounds;
        PDFMatrix mMatrix;
        ObjectIDType mPatternObjectId;
};
