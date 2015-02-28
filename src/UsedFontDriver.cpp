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
#include <list>

#include FT_GLYPH_H

using namespace v8;

Persistent<Function> UsedFontDriver::constructor;
Persistent<FunctionTemplate> UsedFontDriver::constructor_template;

UsedFontDriver::UsedFontDriver()
{
    UsedFont = NULL;
}

void UsedFontDriver::Init()
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PDFUsedFont"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "calculateTextDimensions", CalculateTextDimensions);
	SET_CONSTRUCTOR(constructor, t);
	SET_CONSTRUCTOR_TEMPLATE(constructor_template, t);
}

METHOD_RETURN_TYPE UsedFontDriver::NewInstance(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    Local<Object> instance = NEW_INSTANCE(constructor);
        
    SET_FUNCTION_RETURN_VALUE(instance);
}

v8::Handle<v8::Value> UsedFontDriver::GetNewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

bool UsedFontDriver::HasInstance(Handle<Value> inObject)
{
	CREATE_ISOLATE_CONTEXT;

	return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);
}

METHOD_RETURN_TYPE UsedFontDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    UsedFontDriver* usedFont = new UsedFontDriver();
    usedFont->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This());
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
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    if(args.Length() == 2)
        fontSize = args[1]->ToNumber()->Uint32Value();
    else
        fontSize = 1;
    
    UIntList glyphs;
    
    PDFUsedFont* theFont =  ObjectWrap::Unwrap<UsedFontDriver>(args.This())->UsedFont;
    FreeTypeFaceWrapper* ftWrapper = theFont->GetFreeTypeFont();
    FT_Face ftFont = (*ftWrapper).operator->();
    
    if(args[0]->IsString()) // if string, use freetype wrapper of the used font to get the glyph indexes
    {
        UnicodeString unicode;
        
        unicode.FromUTF8(*String::Utf8Value(args[0]->ToString()));
        ftWrapper->GetGlyphsForUnicodeText(unicode.GetUnicodeList(),glyphs);
    }
    else // array of glyph indexes
    {
        unsigned int arrayLength = args[0]->ToObject()->Get(v8::NEW_STRING("length"))->ToNumber()->Uint32Value();
        for(unsigned int i=0; i < arrayLength;++i)
            glyphs.push_back(args[0]->ToObject()->Get(i)->ToNumber()->Uint32Value());
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
    
    
    Handle<Object> result = NEW_OBJECT;
    // file the end object with results
    
    // adapt results to the size, and PDF font size
    
    result->Set(NEW_STRING("xMin"),NEW_NUMBER((double)(bbox.xMin*fontSize)/1000));
    result->Set(NEW_STRING("yMin"),NEW_NUMBER((double)(bbox.yMin*fontSize)/1000));
    result->Set(NEW_STRING("xMax"),NEW_NUMBER((double)(bbox.xMax*fontSize)/1000));
    result->Set(NEW_STRING("yMax"),NEW_NUMBER((double)(bbox.yMax*fontSize)/1000));
    result->Set(NEW_STRING("width"),NEW_NUMBER((double)(bbox.xMax-bbox.xMin)*fontSize/1000));
    result->Set(NEW_STRING("height"),NEW_NUMBER((double)(bbox.yMax-bbox.yMin)*fontSize/1000));
    
    SET_FUNCTION_RETURN_VALUE(result);
}

