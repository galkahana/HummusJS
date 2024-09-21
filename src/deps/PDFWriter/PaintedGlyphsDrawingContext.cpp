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

#include "PaintedGlyphsDrawingContext.h"
#include "PDFUsedFont.h"
#include "Trace.h"
#include "DocumentContext.h"
#include "ObjectsContext.h"
#include "IndirectObjectsReferenceRegistry.h"
#include "ResourcesDictionary.h"
#include "IObjectEndWritingTask.h"
#include "DictionaryContext.h"
#include "RadialGradientShadingPatternWritingTask.h"
#include "LinearGradientShadingPatternWritingTask.h"
#include "SweepGradientShadingPatternWritingTask.h"
#include <math.h>

#include FT_COLOR_H

using namespace PDFHummus;

static const double scFix16Dot16Scale = (double(1<<16));

PaintedGlyphsDrawingContext::PaintedGlyphsDrawingContext(AbstractContentContext* inContentContext, const AbstractContentContext::TextOptions& inOptions):mOptions(inOptions) {
    mContentContext = inContentContext;
    mCanDrawGlyph = false;
    mLatestAdvance = 0;
    mPalette = NULL;
}

void PaintedGlyphsDrawingContext::SetGlyph(const GlyphUnicodeMapping& inGlyph) {
    mGlyph = inGlyph;
    mBoundsStack.clear();
    mDrawnGlyphs.clear();
    mGraphicStateMatrixStack.clear();

    mCurrentOpaquePaint.p = NULL;

    FreeTypeFaceWrapper* freeTypeFace = mContentContext->GetCurrentFont()->GetFreeTypeFont();

    FT_Bool hasPaint = FT_Get_Color_Glyph_Paint(
        *freeTypeFace,
        mGlyph.mGlyphCode,
        FT_COLOR_NO_ROOT_TRANSFORM,
        &mCurrentOpaquePaint
    );

    mCanDrawGlyph = hasPaint;
    if(!hasPaint)
        return;

    FT_Error error = freeTypeFace->SelectDefaultPalette(&mPalette, &mPaletteSize);
    if (error) {
        mPalette = NULL;
        mCanDrawGlyph = false;
    }

}

bool PaintedGlyphsDrawingContext::CanDraw() {
    return mCanDrawGlyph;
}

EStatusCode PaintedGlyphsDrawingContext::Draw(double inX, double inY, bool inComputeAdvance) {
    // set initial bounds for drawing. 
    // with some tests getting the base glyph brings 0
    UIntList glyphs;
    glyphs.push_back(mGlyph.mGlyphCode);
    PDFUsedFont::TextMeasures measures = mContentContext->GetCurrentFont()->CalculateTextDimensions(glyphs, mOptions.fontSize);
    mBoundsStack.push_back(PDFRectangle(measures.xMin,measures.yMin,measures.xMax,measures.yMax));


    if(inComputeAdvance) {
        // for some reason this beings a good advance, even if dimensions bring nothing
		mLatestAdvance = mContentContext->GetCurrentFont()->CalculateTextAdvance(glyphs, mOptions.fontSize);
    }

    // rmmbr i got into this glyph already to avoid loops
    mDrawnGlyphs.push_back(mGlyph.mGlyphCode);
    

    mContentContext->q();
    // initialize a graphic context with x,y so we dont have to deal with them later
    mContentContext->cm(1,0,0,1,inX,inY);
    // till i make glyphs forms, this is required to correctly setup patterns. otherwise start with unit matrix
    mGraphicStateMatrixStack.push_back(PDFMatrix(1,0,0,1,inX,inY)); 

    // now draw the graph per it's root
    EStatusCode status = ExecuteOpaquePaint(mCurrentOpaquePaint) ? eSuccess: eFailure;
    mContentContext->Q();

    return status;
}

double PaintedGlyphsDrawingContext::GetLatestAdvance() {
	return mLatestAdvance;
}

bool PaintedGlyphsDrawingContext::ExecuteOpaquePaint(FT_OpaquePaint inOpaquePaint) {
    FreeTypeFaceWrapper* freeTypeFace = mContentContext->GetCurrentFont()->GetFreeTypeFont();

    FT_COLR_Paint paint;
    FT_Bool isOK = FT_Get_Paint(
        *freeTypeFace,
        inOpaquePaint,
        &paint
    );

    if(!isOK)
        return false;

    return ExecuteColrPaint(paint);
}

bool PaintedGlyphsDrawingContext::ExecuteColrPaint(FT_COLR_Paint inColrPaint) {
    bool result = true;

    switch(inColrPaint.format) {
        case FT_COLR_PAINTFORMAT_COLR_LAYERS:
            result = ExecutePaintColrLayers(inColrPaint.u.colr_layers);
            break;
        case FT_COLR_PAINTFORMAT_GLYPH:
            result = ExecutePaintGlyph(inColrPaint.u.glyph);
            break;
        case FT_COLR_PAINTFORMAT_SOLID:
            result = ExecutePaintSolid(inColrPaint.u.solid);
            break;
        case FT_COLR_PAINTFORMAT_TRANSFORM:
            result = ExceutePaintTransform(inColrPaint.u.transform);
            break;
        case FT_COLR_PAINTFORMAT_TRANSLATE:
            result = ExceutePaintTranslate(inColrPaint.u.translate);
            break;
        case FT_COLR_PAINTFORMAT_SCALE:
            result = ExecutePaintScale(inColrPaint.u.scale);
            break;
        case FT_COLR_PAINTFORMAT_ROTATE:
            result = ExceutePaintRotate(inColrPaint.u.rotate);
            break;
        case FT_COLR_PAINTFORMAT_SKEW:
            result = ExecutePaintSkew(inColrPaint.u.skew);
            break;
        case FT_COLR_PAINTFORMAT_COLR_GLYPH:
            result = ExecutePaintColrGlyph(inColrPaint.u.colr_glyph);
            break;
        // Note: all gradients implementations are inspired/rigorously copied from Skia (https://skia.org/) implementation of shades
        // which largely coincides with how TrueType Colrv1 works, with a really minor
        // thing left out - sweep gradients are always 0..360 regardless of selected angles. Something
        // that I did not choose to improve, so it's the same.
        // Thank you Skia :) 
        case FT_COLR_PAINTFORMAT_RADIAL_GRADIENT:
            result = ExecutePaintRadialGradient(inColrPaint.u.radial_gradient);
            break;
        case FT_COLR_PAINTFORMAT_LINEAR_GRADIENT:
            result = ExecutePaintLinearGradient(inColrPaint.u.linear_gradient);
            break;
        case FT_COLR_PAINTFORMAT_SWEEP_GRADIENT:
            result = ExecutePaintSweepGradient(inColrPaint.u.sweep_gradient);
            break;
        case FT_COLR_PAINTFORMAT_COMPOSITE:
            result = ExecuetePaintComposite(inColrPaint.u.composite);
            break;
        default:
            TRACE_LOG1(
				"PaintedGlyphsDrawingContext::ExecuteColrPaint, unsupported Colrv1 paint format %d. skipping.",
				inColrPaint.format);
    }

    return result;
}

bool PaintedGlyphsDrawingContext::ExecutePaintColrLayers(FT_PaintColrLayers inColrLayers) {
    FreeTypeFaceWrapper* freeTypeFace = mContentContext->GetCurrentFont()->GetFreeTypeFont();
    bool result = true;

    FT_OpaquePaint paint;
    paint.p = NULL;

    while (result &&
        FT_Get_Paint_Layers(*freeTypeFace,
                            &inColrLayers.layer_iterator,
                            &paint)
    ) {
        result = ExecuteOpaquePaint(paint);
    }

    return result;

}

bool PaintedGlyphsDrawingContext::ExecutePaintGlyph(FT_PaintGlyph inGlyph) {
    // now lets save this state so we can clip without later artifacts
    mContentContext->q();
    mGraphicStateMatrixStack.push_back(mGraphicStateMatrixStack.back()); 

    // clip per a regular glyph
    mContentContext->BT();
    // set the rendering mode, for clipping
    mContentContext->Tr(7);

    // add the path to the clip path
    mContentContext->SetCurrentFontSize(mOptions.fontSize);
    GlyphUnicodeMappingList text;
    text.push_back(GlyphUnicodeMapping(inGlyph.glyphID, mGlyph.mUnicodeValues)); 
    mContentContext->Tj(text);

    // finish text object so path is added to clip path
    mContentContext->ET();    

    // add glyph bounds to stack, so lower levels can fill to it
    UIntList glyphs;
    glyphs.push_back(inGlyph.glyphID);
    PDFUsedFont::TextMeasures measures = mContentContext->GetCurrentFont()->CalculateTextDimensions(glyphs, mOptions.fontSize);
    mBoundsStack.push_back(PDFRectangle(measures.xMin,measures.yMin,measures.xMax,measures.yMax));


    // draw the internal glyph graph node...which should be a certain fill
    bool result = ExecuteOpaquePaint(inGlyph.paint);

    // pop previous pushed bounds...as they are about to go
    mBoundsStack.pop_back();

    // restore graphic stay, cleanup.
    mGraphicStateMatrixStack.pop_back(); 
    mContentContext->Q();

    return result;
}

void PaintedGlyphsDrawingContext::FillCurrentBounds() {
    PDFRectangle& currentBounds = mBoundsStack.back();
    mContentContext->re(currentBounds.LowerLeftX,currentBounds.LowerLeftY,currentBounds.GetWidth(), currentBounds.GetHeight());
    mContentContext->f();
}


bool PaintedGlyphsDrawingContext::ExecutePaintSolid(FT_PaintSolid inSolid) {
    FT_UInt16 palette_index = inSolid.color.palette_index;
    FT_F2Dot14 alpha = inSolid.color.alpha;

    // setup the color, with multiplier considered
    if(0xFFFF == palette_index) {
        mContentContext->SetupColor(AbstractContentContext::eFill,mOptions.colorValue,mOptions.colorSpace,mOptions.opacity * ( alpha / (1<<14)));
    } else {

        if(palette_index >= mPaletteSize) {
            TRACE_LOG2("LayeredGlyphsDrawingContext::Draw, requsted color index %d is too high. The color palette only holds index 0 to %d", palette_index, mPaletteSize);
            return false;
        }

        FT_Color layer_color = mPalette[palette_index];

        mContentContext->SetOpacity((double(layer_color.alpha)/255.0) * (alpha / (1<<14)));
        mContentContext->rg(double(layer_color.red)/255.0,double(layer_color.green)/255.0,double(layer_color.blue)/255.0);
    }    

    // now fill a path. use current bounds
    FillCurrentBounds();
    return true;
}

double PaintedGlyphsDrawingContext::GetFontUnitMeasurementInPDF(FT_Fixed inFixedPos) {
    return (mContentContext->GetCurrentFont()->GetFreeTypeFont()->GetInPDFMeasurements(inFixedPos) / scFix16Dot16Scale) * (mOptions.fontSize / 1000);
}

bool PaintedGlyphsDrawingContext::ExceutePaintTransform(FT_PaintTransform inTransform) {
    FreeTypeFaceWrapper* freeTypeFace = mContentContext->GetCurrentFont()->GetFreeTypeFont();

    return ApplyTransformToPaint(
        inTransform.affine.xx / scFix16Dot16Scale,
        inTransform.affine.yx / scFix16Dot16Scale,
        inTransform.affine.xy / scFix16Dot16Scale,
        inTransform.affine.yy / scFix16Dot16Scale,
        // vectors are in font space, so adjust translation to pdf measurements by em and then by font size
        GetFontUnitMeasurementInPDF(inTransform.affine.dx),
        GetFontUnitMeasurementInPDF(inTransform.affine.dy),
        inTransform.paint
    );
}

bool PaintedGlyphsDrawingContext::ApplyTransformToPaint(
    double inA,
    double inB,
    double inC,
    double inD,
    double inE,
    double inF,
    FT_OpaquePaint inOpaquePaint) {

    mContentContext->q();
    mGraphicStateMatrixStack.push_back(PDFMatrix(inA,inB,inC,inD,inE,inF).Multiply(mGraphicStateMatrixStack.back())); 

    mContentContext->cm(inA,inB,inC,inD,inE,inF);

    bool result = ExecuteOpaquePaint(inOpaquePaint);

    mGraphicStateMatrixStack.pop_back();
    mContentContext->Q();

    return result;
}


bool PaintedGlyphsDrawingContext::ExceutePaintTranslate(FT_PaintTranslate inTranslate) {
    FreeTypeFaceWrapper* freeTypeFace = mContentContext->GetCurrentFont()->GetFreeTypeFont();

    return ApplyTransformToPaint(
        1,
        0,
        0,
        1,
        // see transform
        GetFontUnitMeasurementInPDF(inTranslate.dx),
        GetFontUnitMeasurementInPDF(inTranslate.dy),
        inTranslate.paint
    );

}

bool PaintedGlyphsDrawingContext::ExecutePaintScale(FT_PaintScale inScale) {
    FreeTypeFaceWrapper* freeTypeFace = mContentContext->GetCurrentFont()->GetFreeTypeFont();

    return ApplyTransformToPaint(
        inScale.scale_x / scFix16Dot16Scale,
        0,
        0,
        inScale.scale_y / scFix16Dot16Scale,
        // see transform
        GetFontUnitMeasurementInPDF(inScale.center_x),
        GetFontUnitMeasurementInPDF(inScale.center_y),
        inScale.paint
    );

}

bool PaintedGlyphsDrawingContext::ExceutePaintRotate(FT_PaintRotate inRotate) {
    FreeTypeFaceWrapper* freeTypeFace = mContentContext->GetCurrentFont()->GetFreeTypeFont();

    double radianAngle = inRotate.angle / scFix16Dot16Scale;

    return ApplyTransformToPaint(
        cos(radianAngle),
        sin(radianAngle),
        -sin(radianAngle),
        cos(radianAngle),
        // see transform
        GetFontUnitMeasurementInPDF(inRotate.center_x),
        GetFontUnitMeasurementInPDF(inRotate.center_y),
        inRotate.paint
    );
}

bool PaintedGlyphsDrawingContext::ExecutePaintSkew(FT_PaintSkew inSkew) {
    FreeTypeFaceWrapper* freeTypeFace = mContentContext->GetCurrentFont()->GetFreeTypeFont();

    return ApplyTransformToPaint(
        1,
        tan(inSkew.x_skew_angle / scFix16Dot16Scale),
        tan(inSkew.y_skew_angle / scFix16Dot16Scale),
        1,
        // see transform
        GetFontUnitMeasurementInPDF(inSkew.center_x),
        GetFontUnitMeasurementInPDF(inSkew.center_y),
        inSkew.paint
    );
}

bool PaintedGlyphsDrawingContext::ExecutePaintColrGlyph(FT_PaintColrGlyph inColrGlyph) {
    FreeTypeFaceWrapper* freeTypeFace = mContentContext->GetCurrentFont()->GetFreeTypeFont();

    FT_UInt glyphID = inColrGlyph.glyphID; 

    bool didVisitGlyph = false;
    UShortList::iterator itDrawnGlyphs = mDrawnGlyphs.begin();

    for(; itDrawnGlyphs != mDrawnGlyphs.end() && !didVisitGlyph;++itDrawnGlyphs) {
        didVisitGlyph = (*itDrawnGlyphs == glyphID);
    }
    if(didVisitGlyph) {
        // orcish mischief. there's a circle in the DAG!!!!!!11. ignore per the specs
        TRACE_LOG2(
        "PaintedGlyphsDrawingContext::ExecutePaintColrGlyph, sub glyph id not found - %d while drawing glyph %d",
        glyphID,
        mGlyph.mGlyphCode);        
        return true;
    }

    FT_OpaquePaint rootPaint;
    rootPaint.p = NULL;
    FT_Bool hasPaint = FT_Get_Color_Glyph_Paint(
        *freeTypeFace,
        glyphID,
        FT_COLOR_NO_ROOT_TRANSFORM,
        &rootPaint
    );

    if(!hasPaint) {
        TRACE_LOG2(
        "PaintedGlyphsDrawingContext::ExecutePaintColrGlyph, sub glyph id not found - %d while drawing glyph %d",
        glyphID,
        mGlyph.mGlyphCode);
 
        // skip this. ignoring.
        return true;
    }

    // ok let's do this
    mDrawnGlyphs.push_back(glyphID);
    mContentContext->q();
    mGraphicStateMatrixStack.push_back(mGraphicStateMatrixStack.back());
    // now draw the graph per it's root
    bool result = ExecuteOpaquePaint(rootPaint);
    mGraphicStateMatrixStack.pop_back();
    mContentContext->Q(); 
    mDrawnGlyphs.pop_back();
    return result;   
}

 bool PaintedGlyphsDrawingContext::ReadColorStops(FT_ColorStopIterator inColorStopIterator, InterpretedGradientStopList* outColorLine) {
    FreeTypeFaceWrapper* freeTypeFace = mContentContext->GetCurrentFont()->GetFreeTypeFont();
    FT_ColorStop colorStop;
    InterpretedGradientStopList colorLine;
    bool ok = true;

    while(FT_Get_Colorline_Stops(
        *freeTypeFace,
        &colorStop,
        &inColorStopIterator)) {
            FT_Color color;

            if(0xFFFF == colorStop.color.palette_index) {
                if(mOptions.colorSpace != AbstractContentContext::eRGB) {
                    TRACE_LOG1("PaintedGlyphsDrawingContext::ReadColorStops, Error - color provided in color options is not an RGB color. For Colrv1 gradients please use RGB color. color space = %d",mOptions.colorSpace);
                    return false;
                }

                FT_Color computedColor = {
                    (Byte)(mOptions.colorValue & 0xFF), // b
                    (Byte)((mOptions.colorValue >> 8) & 0xFF), // g
                    (Byte)((mOptions.colorValue >> 16) & 0xFF), // r
                    (Byte)(mOptions.opacity*255) // a
                };

                color = computedColor;
            } else {
                if(colorStop.color.palette_index >= mPaletteSize) {
                    TRACE_LOG2("LayeredGlyphsDrawingContext::Draw, requsted color index %d is too high. The color palette only holds index 0 to %d", colorStop.color.palette_index, mPaletteSize);
                    ok = false;
                    break;
                }

                color = mPalette[colorStop.color.palette_index];
            }

            InterpretedGradientStop gradientStop = {
                colorStop.stop_offset/scFix16Dot16Scale,
                double(colorStop.color.alpha) / double(1<<14),
                color
            };

            outColorLine->push_back(
                gradientStop
            );
    };

    return ok;
}


bool PaintedGlyphsDrawingContext::ExecutePaintRadialGradient(FT_PaintRadialGradient inColrRadialGradient)
{

    double x0 = GetFontUnitMeasurementInPDF(inColrRadialGradient.c0.x);
    double y0 = GetFontUnitMeasurementInPDF(inColrRadialGradient.c0.y);
    double r0 = GetFontUnitMeasurementInPDF(inColrRadialGradient.r0);
    double x1 = GetFontUnitMeasurementInPDF(inColrRadialGradient.c1.x);
    double y1 = GetFontUnitMeasurementInPDF(inColrRadialGradient.c1.y);
    double r1 = GetFontUnitMeasurementInPDF(inColrRadialGradient.r1);

    FT_PaintExtend gradientExtend = inColrRadialGradient.colorline.extend;
    InterpretedGradientStopList colorLine;
    
    if(!ReadColorStops(inColrRadialGradient.colorline.color_stop_iterator,&colorLine))
        return false;

    // apply shading pattern
    ObjectIDType patternObjectId = mContentContext->GetDocumentContext()->GetObjectsContext()->GetInDirectObjectsRegistry().AllocateNewObjectID();
    mContentContext->cs("Pattern");
    std::string ptName = mContentContext->GetResourcesDictionary()->AddPatternMapping(patternObjectId);
    mContentContext->scn(ptName);

    // schedule task to create object for this shading pattern
    mContentContext->ScheduleObjectEndWriteTask(new RadialGradientShadingPatternWritingTask(
        x0,
        y0,
        r0,
        x1,
        y1,
        r1,
        colorLine,
        gradientExtend,
        mBoundsStack.back(),
        mGraphicStateMatrixStack.back(),
        patternObjectId
    ));

    // now fill a path. use current bounds
    FillCurrentBounds();
    return true;
}


bool PaintedGlyphsDrawingContext::ExecutePaintLinearGradient(FT_PaintLinearGradient inColrLinearGradient) {
    
    double x0 = GetFontUnitMeasurementInPDF(inColrLinearGradient.p0.x);
    double y0 = GetFontUnitMeasurementInPDF(inColrLinearGradient.p0.y);
    double x1 = GetFontUnitMeasurementInPDF(inColrLinearGradient.p1.x);
    double y1 = GetFontUnitMeasurementInPDF(inColrLinearGradient.p1.y);
    double x2 = GetFontUnitMeasurementInPDF(inColrLinearGradient.p2.x);
    double y2 = GetFontUnitMeasurementInPDF(inColrLinearGradient.p2.y);

    // computing the projection of p0p1 on perpendicular to p0p2 to come up with 
    // p3, such that p0 and p3 are sufficient to decribe a linear gradient cause
    // the color is perpendicular to the line, as PDF likes it.
    double x3,y3;

    double dotProdP0P1Op0OnPerpc = (y2-y0)*(x1-x0) + (x0-x2)*(y1-y0);
    double lengthPerpcPow2 = (y2-y0)*(y2-y0) + (x0-x2)*(x0-x2);
    if(lengthPerpcPow2 == 0) {
        // hmm. somethings fishy. ignore, and use x1,y1 as x3,y3
        x3 = x1;
        y3 = y1;
    }
    else {
        double projPerpcOp0[2] = {(dotProdP0P1Op0OnPerpc/lengthPerpcPow2)*(y2-y0),
                                    (dotProdP0P1Op0OnPerpc/lengthPerpcPow2)*(x0-x2)};
        // now this p3 is such that it can be used instead of p1, without requiring the extra info
        // of p2. golly
        x3 = projPerpcOp0[0] + x0;
        y3 = projPerpcOp0[1] + y0;

    }

    FT_PaintExtend gradientExtend = inColrLinearGradient.colorline.extend;
    InterpretedGradientStopList colorLine;
    
    if(!ReadColorStops(inColrLinearGradient.colorline.color_stop_iterator,&colorLine))
        return false;

    // apply shading pattern
    ObjectIDType patternObjectId = mContentContext->GetDocumentContext()->GetObjectsContext()->GetInDirectObjectsRegistry().AllocateNewObjectID();
    mContentContext->cs("Pattern");
    std::string ptName = mContentContext->GetResourcesDictionary()->AddPatternMapping(patternObjectId);
    mContentContext->scn(ptName);

    // schedule task to create object for this shading pattern
    mContentContext->ScheduleObjectEndWriteTask(new LinearGradientShadingPatternWritingTask(
        x0,
        y0,
        x3,
        y3,
        colorLine,
        gradientExtend,
        mBoundsStack.back(),
        mGraphicStateMatrixStack.back(),
        patternObjectId
    ));

    // now fill a path. use current bounds
    FillCurrentBounds();
    return true;

}

bool PaintedGlyphsDrawingContext::ExecutePaintSweepGradient(FT_PaintSweepGradient inColrSweepGradient) {
    double cx = GetFontUnitMeasurementInPDF(inColrSweepGradient.center.x);
    double cy = GetFontUnitMeasurementInPDF(inColrSweepGradient.center.y);
    double radianAngleStart = inColrSweepGradient.start_angle / scFix16Dot16Scale;
    double radianAngleEnd = inColrSweepGradient.end_angle / scFix16Dot16Scale;

    FT_PaintExtend gradientExtend = inColrSweepGradient.colorline.extend;
    InterpretedGradientStopList colorLine;
    
    if(!ReadColorStops(inColrSweepGradient.colorline.color_stop_iterator,&colorLine))
        return false;

    // apply shading pattern
    ObjectIDType patternObjectId = mContentContext->GetDocumentContext()->GetObjectsContext()->GetInDirectObjectsRegistry().AllocateNewObjectID();
    mContentContext->cs("Pattern");
    std::string ptName = mContentContext->GetResourcesDictionary()->AddPatternMapping(patternObjectId);
    mContentContext->scn(ptName);

    // schedule task to create object for this shading pattern
    mContentContext->ScheduleObjectEndWriteTask(new SweepGradientShadingPatternWritingTask(
        cx,
        cy,
        radianAngleStart,
        radianAngleEnd,
        colorLine,
        gradientExtend,
        mBoundsStack.back(),
        mGraphicStateMatrixStack.back(),
        patternObjectId
    ));

    // now fill a path. use current bounds
    FillCurrentBounds();
    return true;    

}

bool PaintedGlyphsDrawingContext::ExecuetePaintComposite(FT_PaintComposite inColrComposite) {
    FT_OpaquePaint  source_paint = inColrComposite.source_paint; // "top"
    FT_OpaquePaint  backdrop_paint = inColrComposite.backdrop_paint; // "bottom"
    FT_Composite_Mode  composite_mode = inColrComposite.composite_mode;

    // Yeah well let's maybe leave this for another effort. there's quite a few, and i want to do other things. like, nothing.
    // do a default srcover and that's it for now
    bool result1 = ExecuteOpaquePaint(backdrop_paint);
    bool result2 = ExecuteOpaquePaint(source_paint);

    if(composite_mode != FT_COLR_COMPOSITE_SRC_OVER) {
        TRACE_LOG(
            "PaintedGlyphsDrawingContext::ExecuetePaintComposite, Colrv1 Composite is not supported at this point, will default FT_COLR_COMPOSITE_SRC_OVER.");    
    }

    return result1 && result2;
}