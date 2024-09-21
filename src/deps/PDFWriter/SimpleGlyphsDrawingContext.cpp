/*
   Source File : SimpleGlyphsDrawingContext.cpp


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

#include "SimpleGlyphsDrawingContext.h"
#include "PDFUsedFont.h"

SimpleGlyphsDrawingContext::SimpleGlyphsDrawingContext(AbstractContentContext* inContentContext, const AbstractContentContext::TextOptions& inOptions):mOptions(inOptions) {
	mContentContext = inContentContext;
}

void SimpleGlyphsDrawingContext::StartWriting(double inX,double inY) {
	mText.clear();
	mX = inX;
	mY = inY;
}
	
void SimpleGlyphsDrawingContext::AddGlyphMapping(const GlyphUnicodeMapping& inMapping) {
	mText.push_back(inMapping);

}

void SimpleGlyphsDrawingContext::Flush(bool inComputeAdvance) {
	// current font is expected to have been set in advance, so don't bother with it, and can
	// grab current font

	if(mText.size() == 0) {
		if(inComputeAdvance) {
			mLatestAdvance = 0;
		}
		return;
	}

    mContentContext->SetupColor(mOptions);
	mContentContext->BT();
	mContentContext->Tm(1,0,0,1,mX,mY);
	mContentContext->SetCurrentFontSize(mOptions.fontSize);
	mContentContext->Tj(mText);
	mContentContext->ET();	

	if(inComputeAdvance) {
		UIntList glyphs;

		GlyphUnicodeMappingList::iterator it = mText.begin();
		for(;it!=mText.end(); ++it) {
			glyphs.push_back(it->mGlyphCode);
		}
		
		mLatestAdvance = mContentContext->GetCurrentFont()->CalculateTextAdvance(glyphs, mOptions.fontSize);
	}

	mText.clear();
}

double SimpleGlyphsDrawingContext::GetLatestAdvance() {
	return mLatestAdvance;
}
