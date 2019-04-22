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

#include "nodes.h"

#include "GlyphUnicodeMapping.h"
#include "ObjectsBasicTypes.h"
#include <string>

class AbstractContentContext;
class ResourcesDictionary;
class PDFWriterDriver;
class ConstructorsHolder;

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
    
    
    static void Init(v8::Local<v8::FunctionTemplate>& ioDriverTemplate);
    
    
    void SetResourcesDictionary(ResourcesDictionary* inResourcesDictionary);

    ConstructorsHolder* holder;
protected:
    AbstractContentContextDriver();
        
private:
    
    virtual AbstractContentContext* GetContext() = 0;

    ResourcesDictionary* mResourcesDictionary;
    
    // simple content placements
    static METHOD_RETURN_TYPE DrawPath(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE DrawCircle(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE DrawSquare(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE DrawRectangle(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE WriteText(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE DrawImage(const ARGS_TYPE& args);
    
    // now for regular PDF operators implementation
    
    
    // path stroke/fill
    static METHOD_RETURN_TYPE b(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE B(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE bStar(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE BStar(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE s(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE S(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE f(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE F(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE fStar(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE n(const ARGS_TYPE& args);

    // path construction
    static METHOD_RETURN_TYPE m(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE l(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE c(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE v(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE y(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE h(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE re(const ARGS_TYPE& args);

    // graphic state
    static METHOD_RETURN_TYPE q(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE Q(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE cm(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE w(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE J(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE j(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE M(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE d(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE ri(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE i(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE gs(const ARGS_TYPE& args);

    // color operators
    static METHOD_RETURN_TYPE CS(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE cs(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE SC(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE SCN(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE sc(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE scn(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE G(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE g(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE RG(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE rg(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE K(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE k(const ARGS_TYPE& args);
    
 	// clip operators
    static METHOD_RETURN_TYPE W(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE WStar(const ARGS_TYPE& args);

    // XObject usage
    static METHOD_RETURN_TYPE doXObject(const ARGS_TYPE& args);

    // Text state operators
    static METHOD_RETURN_TYPE Tc(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE Tw(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE Tz(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE TL(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE Tr(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE Ts(const ARGS_TYPE& args);
    
	// Text object operators
    static METHOD_RETURN_TYPE BT(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE ET(const ARGS_TYPE& args);
   
	// Text positioning operators
    static METHOD_RETURN_TYPE Td(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE TD(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE Tm(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE TStar(const ARGS_TYPE& args);

    // Font setting
    static METHOD_RETURN_TYPE Tf(const ARGS_TYPE& args);

    // Text showing
    static METHOD_RETURN_TYPE Tj(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE Quote(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE DoubleQuote(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE TJ(const ARGS_TYPE& args);
    
    // Others
    static METHOD_RETURN_TYPE WriteFreeCode(const ARGS_TYPE& args);
    
    
    static TextPlacingOptions ObjectToOptions(const v8::Local<v8::Object>& inObject);
	static GlyphUnicodeMappingList ArrayToGlyphsList(const v8::Local<v8::Value>& inArray);
        
	void SetupColorAndLineWidth(const v8::Local<v8::Value>& inMaybeOptions);
	void SetColor(const v8::Local<v8::Value>& inMaybeOptions, bool inIsStroke);
	void FinishPath(const v8::Local<v8::Value>& inMaybeOptions);
	void SetFont(const v8::Local<v8::Value>& inMaybeOptions);
    void SetRGBColor(unsigned long inColorValue,bool inIsStroke);

};
