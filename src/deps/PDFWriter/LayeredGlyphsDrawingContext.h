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

#pragma once

#include "AbstractContentContext.h"
#include "PDFUsedFont.h"

#include FT_COLOR_H

class LayeredGlyphsDrawingContext {
public:

    LayeredGlyphsDrawingContext(AbstractContentContext* inContentContext, const AbstractContentContext::TextOptions& inOptions);

    void SetGlyph(const GlyphUnicodeMapping& inGlyph);
    bool CanDraw();
    PDFHummus::EStatusCode Draw(double inX, double inY, bool inComputeAdvance = false);

    double GetLatestAdvance();

private:

	AbstractContentContext* mContentContext;
	AbstractContentContext::TextOptions mOptions;

    GlyphUnicodeMapping mGlyph;
    bool mCanDrawGlyph;
    FT_LayerIterator mIterator;
    FT_UInt  mLayerGlyphIndex;
    FT_UInt  mLayerColorIndex;
    FT_Color* mPalette;
    unsigned short mPaletteSize;

	double mLatestAdvance;   
};
