/*
   Source File : SweepGradientShadingPatternWritingTask.cpp


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


#include "SweepGradientShadingPatternWritingTask.h"
#include "ShadingWriter.h"

using namespace PDFHummus;

SweepGradientShadingPatternWritingTask::SweepGradientShadingPatternWritingTask(
    double inCX,
    double inCY,
    double inStartAngleRad,
    double inEndAngleRad,
    InterpretedGradientStopList inColorLine,
    FT_PaintExtend inGradientExtend,
    PDFRectangle inBounds,
    PDFMatrix inMatrix,
    ObjectIDType inPatternObjectId
) {
    cX = inCX;
    cY = inCY;
    startAngleRad = inStartAngleRad;
    endAngleRad = inEndAngleRad;
    mColorLine = inColorLine;
    mGradientExtend = inGradientExtend;
    mBounds = inBounds;
    mMatrix = inMatrix;
    mPatternObjectId = inPatternObjectId;
}

SweepGradientShadingPatternWritingTask::~SweepGradientShadingPatternWritingTask(){
}

EStatusCode SweepGradientShadingPatternWritingTask::Write(
    ObjectsContext* inObjectsContext,
    DocumentContext* inDocumentContext) {

    ShadingWriter shadingWriter(inObjectsContext, inDocumentContext);

    return shadingWriter.WriteSweepShadingPatternObject(
        cX,
        cY,
        startAngleRad,
        endAngleRad,
        mColorLine,
        mGradientExtend,
        mBounds,
        mMatrix,
        mPatternObjectId
    );
}