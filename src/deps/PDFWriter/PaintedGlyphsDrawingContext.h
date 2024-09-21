/*
   Source File : PaintedGlyphsDrawingContext.h


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

#include "AbstractContentContext.h"
#include "PDFUsedFont.h"
#include "PDFRectangle.h"
#include "PDFMatrix.h"
#include "InterpretedGradientStop.h"

#include FT_COLOR_H

#include <list>


typedef std::list<PDFRectangle> PDFRectangleList;
typedef std::list<unsigned short> UShortList;
typedef std::list<PDFMatrix> PDFMatrixList;

class PaintedGlyphsDrawingContext {
public:

    PaintedGlyphsDrawingContext(AbstractContentContext* inContentContext, const AbstractContentContext::TextOptions& inOptions);

    void SetGlyph(const GlyphUnicodeMapping& inGlyph);
    bool CanDraw();
    PDFHummus::EStatusCode Draw(double inX, double inY, bool inComputeAdvance = false);

    double GetLatestAdvance();

private:

	AbstractContentContext* mContentContext;
	AbstractContentContext::TextOptions mOptions;

    GlyphUnicodeMapping mGlyph;
    bool mCanDrawGlyph;
    FT_OpaquePaint mCurrentOpaquePaint;
    FT_Color* mPalette;
    unsigned short mPaletteSize;
    // use the stack to track bounds. can be used to apply 
    // extra bounds via clip, or consider matrix transformations
    PDFRectangleList mBoundsStack;
    // used for tracking graphic state, right now i only need matrix (cause patterns ignore it so have to create)
    PDFMatrixList mGraphicStateMatrixStack;
    // used for tracking the tree, making sure it doesnt have loops
    UShortList mDrawnGlyphs;

	double mLatestAdvance;   

    double GetFontUnitMeasurementInPDF(FT_Fixed inFixedPos);
    void FillCurrentBounds(); 
    bool ReadColorStops(FT_ColorStopIterator inColorStopIterator, InterpretedGradientStopList* outColorLine);

    bool ExecuteOpaquePaint(FT_OpaquePaint inOpaquePaint);
    bool ExecuteColrPaint(FT_COLR_Paint inColrPaint);
    bool ExecutePaintColrLayers(FT_PaintColrLayers inColrLayers);
    bool ExecutePaintGlyph(FT_PaintGlyph inGlyph);
    bool ExecutePaintSolid(FT_PaintSolid inSolid);
    bool ExceutePaintTransform(FT_PaintTransform inTransform);
    bool ApplyTransformToPaint(
        double inA,
        double inB,
        double inC,
        double inD,
        double inE,
        double inF,
        FT_OpaquePaint inOpaquePaint);
    bool ExceutePaintTranslate(FT_PaintTranslate inTranslate);
    bool ExecutePaintScale(FT_PaintScale inScale);
    bool ExceutePaintRotate(FT_PaintRotate inRotate);
    bool ExecutePaintSkew(FT_PaintSkew inSkew);
    bool ExecutePaintColrGlyph(FT_PaintColrGlyph inColrGlyph);
    bool ExecutePaintRadialGradient(FT_PaintRadialGradient inColrRadialGradient);
    bool ExecutePaintLinearGradient(FT_PaintLinearGradient inColrLinearGradient);
    bool ExecutePaintSweepGradient(FT_PaintSweepGradient inColrSweepGradient);
    bool ExecuetePaintComposite(FT_PaintComposite inColrComposite);
};
