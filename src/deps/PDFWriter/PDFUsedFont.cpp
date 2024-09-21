/*
   Source File : PDFUsedFont.cpp


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
#include "PDFUsedFont.h"
#include "IWrittenFont.h"
#include "UnicodeString.h"
#include "ObjectsContext.h"
#include "DictionaryContext.h"
#include "PDFParser.h"
#include "PDFObjectCast.h"
#include "PDFDictionary.h"
#include "PDFIndirectObjectReference.h"

#include FT_GLYPH_H


using namespace PDFHummus;

PDFUsedFont::PDFUsedFont(FT_Face inInputFace,
						 const std::string& inFontFilePath,
						 const std::string& inAdditionalMetricsFontFilePath,
                         long inFontIndex,
						 ObjectsContext* inObjectsContext,
						 bool inEmbedFont):mFaceWrapper(inInputFace,inFontFilePath,inAdditionalMetricsFontFilePath,inFontIndex)
{
	mObjectsContext = inObjectsContext;
	mWrittenFont = NULL;
	mEmbedFont = inEmbedFont;
}

PDFUsedFont::~PDFUsedFont(void)
{
	delete mWrittenFont;
}

bool PDFUsedFont::IsValid()
{
	return mFaceWrapper.IsValid();
}

EStatusCode PDFUsedFont::EncodeStringForShowing(const GlyphUnicodeMappingList& inText,
												ObjectIDType &outFontObjectToUse,
												UShortList& outCharactersToUse,
												bool& outTreatCharactersAsCID)
{
	if (inText.empty()) {
		outFontObjectToUse = 0;
		outTreatCharactersAsCID = false;
		return PDFHummus::eSuccess;
	}

	if(!mWrittenFont)
		mWrittenFont = mFaceWrapper.CreateWrittenFontObject(mObjectsContext,mEmbedFont);
	if(!mWrittenFont)
		return PDFHummus::eFailure;
	mWrittenFont->AppendGlyphs(inText,outCharactersToUse,outTreatCharactersAsCID,outFontObjectToUse);

	return PDFHummus::eSuccess;
}

EStatusCode PDFUsedFont::TranslateStringToGlyphs(const std::string& inText,GlyphUnicodeMappingList& outGlyphsUnicodeMapping)
{
	UIntList glyphs;
	UnicodeString unicode;

	EStatusCode status = unicode.FromUTF8(inText);
	if(status != PDFHummus::eSuccess)
		return status;


	status = mFaceWrapper.GetGlyphsForUnicodeText(unicode.GetUnicodeList(),glyphs);

	ULongList::const_iterator itUnicode = unicode.GetUnicodeList().begin();
	UIntList::iterator itGlyphs = glyphs.begin();

	for(; itUnicode != unicode.GetUnicodeList().end(); ++itUnicode,++itGlyphs)
		outGlyphsUnicodeMapping.push_back(GlyphUnicodeMapping(*itGlyphs,*itUnicode));

	return status;
}

EStatusCode PDFUsedFont::EncodeStringsForShowing(const GlyphUnicodeMappingListList& inText,
												ObjectIDType &outFontObjectToUse,
												UShortListList& outCharactersToUse,
												bool& outTreatCharactersAsCID)
{
	if (inText.empty()) {
		outFontObjectToUse = 0;
		outTreatCharactersAsCID = false;
		return PDFHummus::eSuccess;
	}

	if(!mWrittenFont)
		mWrittenFont = mFaceWrapper.CreateWrittenFontObject(mObjectsContext,mEmbedFont);
	if(!mWrittenFont)
		return PDFHummus::eFailure;
	mWrittenFont->AppendGlyphs(inText,outCharactersToUse,outTreatCharactersAsCID,outFontObjectToUse);

	return PDFHummus::eSuccess;
}

EStatusCode PDFUsedFont::WriteFontDefinition()
{
    // note that written font may be empty, in case no glyphs were used for this font! in the empty case, just dont write the def
    if(!mWrittenFont)
        return eSuccess;
    else
        return mWrittenFont->WriteFontDefinition(mEmbedFont);
}

EStatusCode PDFUsedFont::WriteState(ObjectsContext* inStateWriter,ObjectIDType inObjectID)
{
	inStateWriter->StartNewIndirectObject(inObjectID);
	DictionaryContext* pdfUsedFontObject = inStateWriter->StartDictionary();

	pdfUsedFontObject->WriteKey("Type");
	pdfUsedFontObject->WriteNameValue("PDFUsedFont");

	ObjectIDType writtenFontObject = 0;

	if(mWrittenFont)
	{
		writtenFontObject = inStateWriter->GetInDirectObjectsRegistry().AllocateNewObjectID();

		pdfUsedFontObject->WriteKey("mWrittenFont");
		pdfUsedFontObject->WriteNewObjectReferenceValue(writtenFontObject);
	}

	inStateWriter->EndDictionary(pdfUsedFontObject);
	inStateWriter->EndIndirectObject();

	if(mWrittenFont)
		mWrittenFont->WriteState(inStateWriter,writtenFontObject);

	return PDFHummus::eSuccess;
}

EStatusCode PDFUsedFont::ReadState(PDFParser* inStateReader,ObjectIDType inObjectID)
{
	PDFObjectCastPtr<PDFDictionary> pdfUsedFontState(inStateReader->ParseNewObject(inObjectID));

	PDFObjectCastPtr<PDFIndirectObjectReference> writtenFontReference(pdfUsedFontState->QueryDirectObject("mWrittenFont"));

	if(!writtenFontReference)
		return PDFHummus::eSuccess;

	if(mWrittenFont)
		delete mWrittenFont;

	mWrittenFont = mFaceWrapper.CreateWrittenFontObject(mObjectsContext,mEmbedFont);
	if(!mWrittenFont)
		return PDFHummus::eFailure;

	return mWrittenFont->ReadState(inStateReader,writtenFontReference->mObjectID);
}

FreeTypeFaceWrapper* PDFUsedFont::GetFreeTypeFont()
{
    return &mFaceWrapper;
}

void PDFUsedFont::GetUnicodeGlyphs(const std::string& inText, UIntList& glyphs)
{
	UnicodeString unicode;
        
    unicode.FromUTF8(inText);
	mFaceWrapper.GetGlyphsForUnicodeText(unicode.GetUnicodeList(),glyphs);
}

PDFUsedFont::TextMeasures PDFUsedFont::CalculateTextDimensions(const std::string& inText,long inFontSize)
{
	UIntList glyphs;
    UnicodeString unicode;
        
    unicode.FromUTF8(inText);
	mFaceWrapper.GetGlyphsForUnicodeText(unicode.GetUnicodeList(),glyphs);
	return CalculateTextDimensions(glyphs,inFontSize);
}

PDFUsedFont::TextMeasures PDFUsedFont::CalculateTextDimensions(const UIntList& inGlyphsList,long inFontSize)
{
    // now calculate the placement bounding box. using the algorithm described in the FreeType turtorial part 2, minus the kerning part, and with no scale

    // first, calculate the pen advancements
    int           pen_x, pen_y;
    std::list<FT_Vector> pos;
    pen_x = 0;   /* start at (0,0) */
    pen_y = 0;
    
    UIntList::const_iterator it = inGlyphsList.begin();
    for(; it != inGlyphsList.end();++it)
    {
        pos.push_back(FT_Vector());

        pos.back().x = pen_x;
        pos.back().y = pen_y;

        pen_x += mFaceWrapper.GetGlyphWidth(*it);
    }
    
    // now let's combine with the bbox, so we get the nice bbox for the whole string
    
    FT_BBox  bbox;
    FT_BBox  glyph_bbox;
    bbox.xMin = bbox.yMin =  32000;
    bbox.xMax = bbox.yMax = -32000;
    
    it = inGlyphsList.begin();
    std::list<FT_Vector>::iterator itPos = pos.begin();
    
    for(; it != inGlyphsList.end();++it,++itPos)
    {
		mFaceWrapper.LoadGlyph(mFaceWrapper.GetGlyphIndexInFreeTypeIndexes(*it));
        FT_Glyph aGlyph;
        FT_Get_Glyph( mFaceWrapper->glyph,&aGlyph);
        FT_Glyph_Get_CBox(aGlyph, FT_GLYPH_BBOX_UNSCALED,&glyph_bbox);
        FT_Done_Glyph(aGlyph);
        
        glyph_bbox.xMin = mFaceWrapper.GetInPDFMeasurements(glyph_bbox.xMin);
        glyph_bbox.xMax = mFaceWrapper.GetInPDFMeasurements(glyph_bbox.xMax);
        glyph_bbox.yMin = mFaceWrapper.GetInPDFMeasurements(glyph_bbox.yMin);
        glyph_bbox.yMax = mFaceWrapper.GetInPDFMeasurements(glyph_bbox.yMax);
        
        glyph_bbox.xMin += itPos->x;
        glyph_bbox.xMax += itPos->x;
        glyph_bbox.yMin += itPos->y;
        glyph_bbox.yMax += itPos->y;
    
        
        if ( glyph_bbox.xMin < bbox.xMin )
            bbox.xMin = glyph_bbox.xMin;
        
        if ( glyph_bbox.yMin < bbox.yMin )
            bbox.yMin = glyph_bbox.yMin;
        
        if ( glyph_bbox.xMax > bbox.xMax )
            bbox.xMax = glyph_bbox.xMax;
        
        if ( glyph_bbox.yMax > bbox.yMax )
            bbox.yMax = glyph_bbox.yMax;
    }
    if ( bbox.xMin > bbox.xMax )
    {
        bbox.xMin = 0;
        bbox.yMin = 0;
        bbox.xMax = 0;
        bbox.yMax = 0;
    }

	PDFUsedFont::TextMeasures result;

	result.xMin = bbox.xMin * static_cast<double>(inFontSize) / 1000.0;
	result.yMin = bbox.yMin * static_cast<double>(inFontSize) / 1000.0;
	result.xMax = bbox.xMax * static_cast<double>(inFontSize) / 1000.0;
	result.yMax = bbox.yMax * static_cast<double>(inFontSize) / 1000.0;
	result.width = (bbox.xMax - bbox.xMin) * static_cast<double>(inFontSize) / 1000.0;
	result.height = (bbox.yMax - bbox.yMin) * static_cast<double>(inFontSize) / 1000.0;

	return result;
}

double PDFUsedFont::CalculateTextAdvance(const std::string& inText,double inFontSize)
{
	UIntList glyphs;
	GetUnicodeGlyphs(inText, glyphs);
	return CalculateTextAdvance(glyphs,inFontSize);
}

double PDFUsedFont::CalculateTextAdvance(const UIntList& inGlyphsList,double inFontSize)
{
    FT_Pos pen = 0;
    UIntList::const_iterator it = inGlyphsList.begin();
    for(; it != inGlyphsList.end();++it)
    {
		FT_Pos adv;
		if (mAdvanceCache.count(*it) > 0) 
			adv = mAdvanceCache[*it];
		else {
			adv = mFaceWrapper.GetGlyphWidth(*it); //potentially very expensive!
			if (mAdvanceCache.size() <= AdvanceCacheLimit) //dumb limit should cover typical usage, implement LRU if it's a problem
				mAdvanceCache[*it] = adv;
		}
		pen += adv;
    }
	return pen * inFontSize / 1000.0;
}

bool PDFUsedFont::EnumeratePaths(IOutlineEnumerator& target, const std::string& inText,double inFontSize)
{
	UIntList glyphs;
	GetUnicodeGlyphs(inText, glyphs);
	return EnumeratePaths(target, glyphs,inFontSize);
}

bool PDFUsedFont::EnumeratePaths(IOutlineEnumerator& target, const UIntList& inGlyphsList,double inFontSize)
{
	bool status = true;
    target.BeginEnum(inFontSize);
	for( UIntList::const_iterator it = inGlyphsList.begin(); it != inGlyphsList.end(); ++it )
    {
		status = mFaceWrapper.GetGlyphOutline(*it, target);
		if (!status) break;

		// Keep track of glyphs' advance
		double adv = mFaceWrapper.GetGlyphWidth(*it);
		target.MoveBasepoint(adv * inFontSize / 1000.0, 0);
    }
	return status;
}

void PDFUsedFont::IOutlineEnumerator::BeginEnum(double scale)
{
	mFontScale = scale;
}

PDFUsedFont::IOutlineEnumerator::IOutlineEnumerator(double base_x, double base_y)
{
	mBase_x = base_x;
	mBase_y = base_y;
}

void PDFUsedFont::IOutlineEnumerator::MoveBasepoint(double dx, double dy)
{
	mBase_x += dx;
	mBase_y += dy;
}

bool PDFUsedFont::IOutlineEnumerator::Moveto(FT_Short x, FT_Short y)
{
	double scale = mFontScale / UPM();
	return Moveto(mBase_x + scale * x, mBase_y + scale * y);
}

bool PDFUsedFont::IOutlineEnumerator::Lineto(FT_Short x, FT_Short y)
{
	double scale = mFontScale / UPM();
	return Lineto(mBase_x + scale * x, mBase_y + scale * y);
}

bool PDFUsedFont::IOutlineEnumerator::Curveto(FT_Short x1, FT_Short y1, FT_Short x2, FT_Short y2, FT_Short x3, FT_Short y3)
{
	double scale = mFontScale / UPM();
	return Curveto(mBase_x + scale * x1, mBase_y + scale * y1,
	               mBase_x + scale * x2, mBase_y + scale * y2,
	               mBase_x + scale * x3, mBase_y + scale * y3);
}

bool PDFUsedFont::IOutlineEnumerator::Close()
{
	return Closepath();
}
