/*
 Source File : PDFObjectDriver.h
 
 
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


class PDFObject;

class PDFObjectDriver : public node::ObjectWrap
{
public:
    
    
    static void Init(v8::Handle<v8::FunctionTemplate>& ioDriverTemplate);
    
    static v8::Handle<v8::Value> CreateDriver(PDFObject* inObject);
        
    virtual PDFObject* GetObject() = 0;

private:
    
    
    static v8::Handle<v8::Value> GetType(const v8::Arguments& args);
    static v8::Handle<v8::Value> ToPDFIndirectObjectReference(const v8::Arguments& args);
    static v8::Handle<v8::Value> ToPDFArray(const v8::Arguments& args);
    static v8::Handle<v8::Value> ToPDFDictionary(const v8::Arguments& args);
    static v8::Handle<v8::Value> ToPDFStream(const v8::Arguments& args);
    static v8::Handle<v8::Value> ToPDFBoolean(const v8::Arguments& args);
    static v8::Handle<v8::Value> ToPDFLiteralString(const v8::Arguments& args);
    static v8::Handle<v8::Value> ToPDFHexString(const v8::Arguments& args);
    static v8::Handle<v8::Value> ToPDFNull(const v8::Arguments& args);
    static v8::Handle<v8::Value> ToPDFName(const v8::Arguments& args);
    static v8::Handle<v8::Value> ToPDFInteger(const v8::Arguments& args);
    static v8::Handle<v8::Value> ToPDFReal(const v8::Arguments& args);
    static v8::Handle<v8::Value> ToPDFSymbol(const v8::Arguments& args);
    // return number for pdf numbers (real and integer) and null for non numbers
    static v8::Handle<v8::Value> ToNumber(const v8::Arguments& args);
    // strings (relevant for: names, literal string, hex string, real, integer, symbol, boolean)
    static v8::Handle<v8::Value> ToString(const v8::Arguments& args);
    
};
