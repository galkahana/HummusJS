/*
 Source File : AbstractContentContextDriver.h
 
 
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
#pragma once

#include <node.h>

#include "GlyphUnicodeMapping.h"
#include "ObjectsBasicTypes.h"
#include <string>

class AbstractContentContext;
class ResourcesDictionary;
class PDFWriterDriver;

struct TextPlacingOptions
{
    TextPlacingOptions()
    {
        encoding = EEncodingText;
    }
    
    enum EEncoding
    {
        EEncodingText,
        EEncodingCode,
        EEncodingHex
    };
    
    EEncoding encoding;
};

class AbstractContentContextDriver : public node::ObjectWrap
{
public:
    
    
    static void Init(v8::Handle<v8::FunctionTemplate>& ioDriverTemplate);
    
    
    void SetResourcesDictionary(ResourcesDictionary* inResourcesDictionary);

    virtual PDFWriterDriver* GetPDFWriter() = 0;
protected:
    AbstractContentContextDriver();
        
private:
    
    virtual AbstractContentContext* GetContext() = 0;
    virtual void ScheduleImageWrite(const std::string& inImagePath,unsigned long inImageIndex,ObjectIDType inObjectID) = 0;

    ResourcesDictionary* mResourcesDictionary;
    
    // simple content placements
    static v8::Handle<v8::Value> DrawPath(const v8::Arguments& args);
    static v8::Handle<v8::Value> DrawCircle(const v8::Arguments& args);
    static v8::Handle<v8::Value> DrawSquare(const v8::Arguments& args);
    static v8::Handle<v8::Value> DrawRectangle(const v8::Arguments& args);
    static v8::Handle<v8::Value> WriteText(const v8::Arguments& args);
    static v8::Handle<v8::Value> DrawImage(const v8::Arguments& args);
    
    // now for regular PDF operators implementation
    
    
    // path stroke/fill
    static v8::Handle<v8::Value> b(const v8::Arguments& args);
	static v8::Handle<v8::Value> B(const v8::Arguments& args);
	static v8::Handle<v8::Value> bStar(const v8::Arguments& args);
	static v8::Handle<v8::Value> BStar(const v8::Arguments& args);
	static v8::Handle<v8::Value> s(const v8::Arguments& args);
	static v8::Handle<v8::Value> S(const v8::Arguments& args);
	static v8::Handle<v8::Value> f(const v8::Arguments& args);
	static v8::Handle<v8::Value> F(const v8::Arguments& args);
	static v8::Handle<v8::Value> fStar(const v8::Arguments& args);
	static v8::Handle<v8::Value> n(const v8::Arguments& args);

    // path construction
    static v8::Handle<v8::Value> m(const v8::Arguments& args);
	static v8::Handle<v8::Value> l(const v8::Arguments& args);
	static v8::Handle<v8::Value> c(const v8::Arguments& args);
	static v8::Handle<v8::Value> v(const v8::Arguments& args);
	static v8::Handle<v8::Value> y(const v8::Arguments& args);
	static v8::Handle<v8::Value> h(const v8::Arguments& args);
	static v8::Handle<v8::Value> re(const v8::Arguments& args);

    // graphic state
    static v8::Handle<v8::Value> q(const v8::Arguments& args);
    static v8::Handle<v8::Value> Q(const v8::Arguments& args);
    static v8::Handle<v8::Value> cm(const v8::Arguments& args);
    static v8::Handle<v8::Value> w(const v8::Arguments& args);
    static v8::Handle<v8::Value> J(const v8::Arguments& args);
    static v8::Handle<v8::Value> j(const v8::Arguments& args);
    static v8::Handle<v8::Value> M(const v8::Arguments& args);
    static v8::Handle<v8::Value> d(const v8::Arguments& args);
    static v8::Handle<v8::Value> ri(const v8::Arguments& args);
    static v8::Handle<v8::Value> i(const v8::Arguments& args);
    static v8::Handle<v8::Value> gs(const v8::Arguments& args);

    // color operators
    static v8::Handle<v8::Value> CS(const v8::Arguments& args);
    static v8::Handle<v8::Value> cs(const v8::Arguments& args);
    static v8::Handle<v8::Value> SC(const v8::Arguments& args);
    static v8::Handle<v8::Value> SCN(const v8::Arguments& args);
    static v8::Handle<v8::Value> sc(const v8::Arguments& args);
    static v8::Handle<v8::Value> scn(const v8::Arguments& args);
    static v8::Handle<v8::Value> G(const v8::Arguments& args);
    static v8::Handle<v8::Value> g(const v8::Arguments& args);
    static v8::Handle<v8::Value> RG(const v8::Arguments& args);
    static v8::Handle<v8::Value> rg(const v8::Arguments& args);
    static v8::Handle<v8::Value> K(const v8::Arguments& args);
    static v8::Handle<v8::Value> k(const v8::Arguments& args);
    
 	// clip operators
    static v8::Handle<v8::Value> W(const v8::Arguments& args);
    static v8::Handle<v8::Value> WStar(const v8::Arguments& args);

    // XObject usage
    static v8::Handle<v8::Value> doXObject(const v8::Arguments& args);

    // Text state operators
    static v8::Handle<v8::Value> Tc(const v8::Arguments& args);
    static v8::Handle<v8::Value> Tw(const v8::Arguments& args);
    static v8::Handle<v8::Value> Tz(const v8::Arguments& args);
    static v8::Handle<v8::Value> TL(const v8::Arguments& args);
    static v8::Handle<v8::Value> Tr(const v8::Arguments& args);
    static v8::Handle<v8::Value> Ts(const v8::Arguments& args);
    
	// Text object operators
    static v8::Handle<v8::Value> BT(const v8::Arguments& args);
    static v8::Handle<v8::Value> ET(const v8::Arguments& args);
   
	// Text positioning operators
    static v8::Handle<v8::Value> Td(const v8::Arguments& args);
    static v8::Handle<v8::Value> TD(const v8::Arguments& args);
    static v8::Handle<v8::Value> Tm(const v8::Arguments& args);
    static v8::Handle<v8::Value> TStar(const v8::Arguments& args);

    // Font setting
    static v8::Handle<v8::Value> Tf(const v8::Arguments& args);

    // Text showing
    static v8::Handle<v8::Value> Tj(const v8::Arguments& args);
    static v8::Handle<v8::Value> Quote(const v8::Arguments& args);
    static v8::Handle<v8::Value> DoubleQuote(const v8::Arguments& args);
    static v8::Handle<v8::Value> TJ(const v8::Arguments& args);
    
    // Others
    static v8::Handle<v8::Value> WriteFreeCode(const v8::Arguments& args);
    
    
    static TextPlacingOptions ObjectToOptions(const v8::Handle<v8::Object>& inObject);
    static GlyphUnicodeMappingList ArrayToGlyphsList(const v8::Handle<v8::Value>& inArray);
        
    void SetupColorAndLineWidth(const v8::Handle<v8::Value>& inMaybeOptions);
    void SetColor(const v8::Handle<v8::Value>& inMaybeOptions,bool inIsStroke);
    void FinishPath(const v8::Handle<v8::Value>& inMaybeOptions);
    void SetFont(const v8::Handle<v8::Value>& inMaybeOptions);
    void SetRGBColor(unsigned long inColorValue,bool inIsStroke);

};
