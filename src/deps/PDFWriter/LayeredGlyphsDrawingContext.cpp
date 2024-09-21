/*
   Source File : LayeredGlyphsDrawingContext.h


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

#include "LayeredGlyphsDrawingContext.h"
#include "PDFUsedFont.h"
#include "Trace.h"

#include FT_COLOR_H

using namespace PDFHummus;

LayeredGlyphsDrawingContext::LayeredGlyphsDrawingContext(AbstractContentContext* inContentContext, const AbstractContentContext::TextOptions& inOptions):mOptions(inOptions) {
    mContentContext = inContentContext;
    mCanDrawGlyph = false;
    mLatestAdvance = 0;
}

void LayeredGlyphsDrawingContext::SetGlyph(const GlyphUnicodeMapping& inGlyph) {
    mGlyph = inGlyph;

    FreeTypeFaceWrapper* freeTypeFace = mContentContext->GetCurrentFont()->GetFreeTypeFont();

    mIterator.p = NULL;
    FT_Bool haveLayers = FT_Get_Color_Glyph_Layer(*freeTypeFace,
                                            mGlyph.mGlyphCode,
                                            &mLayerGlyphIndex,
                                            &mLayerColorIndex,
                                            &mIterator);
    mCanDrawGlyph = haveLayers;

    if(!haveLayers)
        return;
    
    // ok...got an interesting glyph. now's a good time to also bother with palette selection. using the default palette.
    // maybe can do this once per face and keep this on the face wrapper
    FT_Error error = freeTypeFace->SelectDefaultPalette(&mPalette, &mPaletteSize);
    if (error) {
        mPalette = NULL;
        mCanDrawGlyph = false;
    }
}

bool LayeredGlyphsDrawingContext::CanDraw() {
    return mCanDrawGlyph;
}

EStatusCode LayeredGlyphsDrawingContext::Draw(double inX, double inY, bool inComputeAdvance) {
    EStatusCode status = eSuccess;
    if(inComputeAdvance) {
        // for advance, use the original glyph width...i'm thinking it probably should be the same width in the end
		UIntList glyphs;
        glyphs.push_back(mGlyph.mGlyphCode);
		mLatestAdvance = mContentContext->GetCurrentFont()->CalculateTextAdvance(glyphs, mOptions.fontSize);
    }

    do {

        mContentContext->q();
    
        if(0xFFFF == mLayerColorIndex) {
            mContentContext->SetupColor(mOptions);
        } else {
            if(mLayerColorIndex >= mPaletteSize) {
                TRACE_LOG2("LayeredGlyphsDrawingContext::Draw, requsted color index %d is too high. The color palette only holds index 0 to %d", mLayerColorIndex, mPaletteSize);
                status = eFailure;
                break;
            }

            FT_Color layer_color = mPalette[mLayerColorIndex];

            mContentContext->SetOpacity(double(layer_color.alpha)/255.0);
            mContentContext->rg(double(layer_color.red)/255.0,double(layer_color.green)/255.0,double(layer_color.blue)/255.0);
        }


        mContentContext->BT();
        mContentContext->Tm(1,0,0,1,inX,inY);
        mContentContext->SetCurrentFontSize(mOptions.fontSize);
        GlyphUnicodeMappingList text;
        // at this point put the original glyph unicode value. this means it'd be multiplied. we'll try to maybe improve later
        text.push_back(GlyphUnicodeMapping(mLayerGlyphIndex, mGlyph.mUnicodeValues)); 
        mContentContext->Tj(text);
        mContentContext->ET();	        

        mContentContext->Q();
    } while (FT_Get_Color_Glyph_Layer(*(mContentContext->GetCurrentFont()->GetFreeTypeFont()),
                                        mGlyph.mGlyphCode,
                                        &mLayerGlyphIndex,
                                        &mLayerColorIndex,
                                        &mIterator));
    return status;
}

double LayeredGlyphsDrawingContext::GetLatestAdvance() {
	return mLatestAdvance;
}
