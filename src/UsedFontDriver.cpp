/*
 Source File : UsedFontDriver.h
 
 
 Copyright 2013 Gal Kahana HummusJS
 
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
#include "UsedFontDriver.h"
#include "PDFUsedFont.h"
#include "UnicodeString.h"
#include "ConstructorsHolder.h"
#include <list>

#include FT_SIZES_H
#include FT_GLYPH_H

using namespace v8;



UsedFontDriver::UsedFontDriver()
{
    UsedFont = NULL;
}

DEF_SUBORDINATE_INIT(UsedFontDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PDFUsedFont"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "calculateTextDimensions", CalculateTextDimensions);
    SET_PROTOTYPE_METHOD(t, "getFontMetrics", GetFontMetrics);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->UsedFont_constructor, t);       
	SET_CONSTRUCTOR_TEMPLATE(holder->UsedFont_constructor_template, t);
}

METHOD_RETURN_TYPE UsedFontDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    UsedFontDriver* usedFont = new UsedFontDriver();
    usedFont->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE UsedFontDriver::GetFontMetrics(const ARGS_TYPE& args)
{
    // There are "global" metrics of a font which are necessary to do any real
    // text formatting
    CREATE_ISOLATE_CONTEXT;
    CREATE_ESCAPABLE_SCOPE;
    long fontSize;

    if(args.Length() > 1 || (args.Length() && !args[0]->IsNumber()))
    {
        THROW_EXCEPTION("Wrong arguments, optionally provide a font size");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }

    if (args.Length())
        fontSize = TO_UINT32(args[0])->Value();
    else
        fontSize = 1;
    
    PDFUsedFont* theFont =  ObjectWrap::Unwrap<UsedFontDriver>(args.This())->UsedFont;
    FreeTypeFaceWrapper* ftWrapper = theFont->GetFreeTypeFont();
    FT_Face ftFont = (*ftWrapper).operator->();
    //FT_BBox bbox = ftFont->bbox;

    FT_Size oldSize = ftFont->size;
    FT_Size newSize;
    #define HANDLE_FTERROR(_m) { FT_Error _err = _m; if (_err) { \
        FT_Activate_Size(oldSize); \
        THROW_EXCEPTION("Unknown font error"); \
        SET_FUNCTION_RETURN_VALUE(UNDEFINED) \
    } }

    HANDLE_FTERROR(FT_New_Size(ftFont, &newSize));
    HANDLE_FTERROR(FT_Activate_Size(newSize));
    HANDLE_FTERROR(
        FT_Set_Char_Size(
            ftFont,
            0,
            64*fontSize, // units are 1/64 point, a point is 1 PDF pixel
            72, 72
        )
    );
    
    Local<Object> result = NEW_OBJECT;
    Local<Object> pixelsPerEm = NEW_OBJECT;

    pixelsPerEm->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("x"),NEW_NUMBER(newSize->metrics.x_ppem));
    pixelsPerEm->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("y"),NEW_NUMBER(newSize->metrics.y_ppem));
    pixelsPerEm->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("xScale"),NEW_NUMBER(newSize->metrics.x_scale));
    pixelsPerEm->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("yScale"),NEW_NUMBER(newSize->metrics.y_scale));

    result->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("pixelsPerEm"), pixelsPerEm);
    result->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("ascender"), NEW_NUMBER(newSize->metrics.ascender));
    result->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("descender"), NEW_NUMBER(newSize->metrics.descender));
    result->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("height"), NEW_NUMBER(newSize->metrics.height));
    result->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("max_advance"), NEW_NUMBER(newSize->metrics.max_advance));
    
    HANDLE_FTERROR(FT_Activate_Size(oldSize));
    HANDLE_FTERROR(FT_Done_Size(newSize));

    SET_FUNCTION_RETURN_VALUE(result)
    #undef HANDLE_FTERROR
}

METHOD_RETURN_TYPE UsedFontDriver::CalculateTextDimensions(const ARGS_TYPE& args)
{
    // completly copied of the freetype toturial.... :)
    // this will calculate the dimensions of a string. it uses mainly the "advance"
    // of the glyphs, which is what the plain text placement of hummus does...so it should be
    // alligned with it. right now there's no kerning calculations, simply because there's no kerning right now.
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    long fontSize;

    if(args.Length() < 1 || args.Length() > 2 ||
       (!args[0]->IsString() && !args[0]->IsArray()) ||
       (args.Length() == 2 && !args[1]->IsNumber()))
    {
		THROW_EXCEPTION("Wrong arguments, provide a string or array of glyph indexes, and optionally also a font size");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() == 2)
        fontSize = TO_UINT32(args[1])->Value();
    else
        fontSize = 1;
    
    UIntList glyphs;
    
    PDFUsedFont* theFont =  ObjectWrap::Unwrap<UsedFontDriver>(args.This())->UsedFont;
    FreeTypeFaceWrapper* ftWrapper = theFont->GetFreeTypeFont();
    FT_Face ftFont = (*ftWrapper).operator->();
    
    if(args[0]->IsString()) // if string, use freetype wrapper of the used font to get the glyph indexes
    {
        UnicodeString unicode;
        
        unicode.FromUTF8(*UTF_8_VALUE(args[0]->TO_STRING()));
        ftWrapper->GetGlyphsForUnicodeText(unicode.GetUnicodeList(),glyphs);
    }
    else // array of glyph indexes
    {
        unsigned int arrayLength = TO_UINT32(args[0]->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, v8::NEW_STRING("length")).ToLocalChecked())->Value();
        for(unsigned int i=0; i < arrayLength;++i)
            glyphs.push_back(TO_UINT32(args[0]->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, i).ToLocalChecked())->Value());
    }
    
    // now calculate the placement bounding box. using the algorithm described in the FreeType turtorial part 2, minus the kerning part, and with no scale

    // first, calculate the pen advancements
    int           pen_x, pen_y;
    std::list<FT_Vector> pos;
    pen_x = 0;   /* start at (0,0) */
    pen_y = 0;
    
    UIntList::iterator it = glyphs.begin();
    for(; it != glyphs.end();++it)
    {
        pos.push_back(FT_Vector());

        pos.back().x = pen_x;
        pos.back().y = pen_y;

        pen_x += ftWrapper->GetGlyphWidth(*it);
    }
    
    // now let's combine with the bbox, so we get the nice bbox for the whole string
    
    FT_BBox  bbox;
    FT_BBox  glyph_bbox;
    bbox.xMin = bbox.yMin =  32000;
    bbox.xMax = bbox.yMax = -32000;
    
    it = glyphs.begin();
    std::list<FT_Vector>::iterator itPos = pos.begin();
    
    for(; it != glyphs.end();++it,++itPos)
    {
        FT_Load_Glyph(ftFont,ftWrapper->GetGlyphIndexInFreeTypeIndexes(*it),FT_LOAD_NO_SCALE);
        FT_Glyph aGlyph;
        FT_Get_Glyph( ftFont->glyph,&aGlyph);
        FT_Glyph_Get_CBox(aGlyph, FT_GLYPH_BBOX_UNSCALED,&glyph_bbox);
        FT_Done_Glyph(aGlyph);
        
        glyph_bbox.xMin = ftWrapper->GetInPDFMeasurements(glyph_bbox.xMin);
        glyph_bbox.xMax = ftWrapper->GetInPDFMeasurements(glyph_bbox.xMax);
        glyph_bbox.yMin = ftWrapper->GetInPDFMeasurements(glyph_bbox.yMin);
        glyph_bbox.yMax = ftWrapper->GetInPDFMeasurements(glyph_bbox.yMax);
        
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
    
    
    Local<Object> result = NEW_OBJECT;
    // file the end object with results
    
    // adapt results to the size, and PDF font size
    
    result->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("xMin"),NEW_NUMBER((double)(bbox.xMin*fontSize)/1000));
    result->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("yMin"),NEW_NUMBER((double)(bbox.yMin*fontSize)/1000));
    result->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("xMax"),NEW_NUMBER((double)(bbox.xMax*fontSize)/1000));
    result->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("yMax"),NEW_NUMBER((double)(bbox.yMax*fontSize)/1000));
    result->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("width"),NEW_NUMBER((double)(bbox.xMax-bbox.xMin)*fontSize/1000));
    result->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("height"),NEW_NUMBER((double)(bbox.yMax-bbox.yMin)*fontSize/1000));
    
    SET_FUNCTION_RETURN_VALUE(result)
}

