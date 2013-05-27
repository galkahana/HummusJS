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
#pragma once

#include <node.h>

#include "PDFDictionary.h"
#include "PDFObjectCast.h"
#include "PDFRectangle.h"

class PDFPageInput;

class PDFPageInputDriver : public node::ObjectWrap
{
public:
    PDFPageInputDriver();
    virtual ~PDFPageInputDriver();
    
    static void Init();
    static v8::Handle<v8::Value> NewInstance();
    
    PDFPageInput* PageInput;
    PDFObjectCastPtr<PDFDictionary> PageInputDictionary;
    
private:
    
    static v8::Persistent<v8::Function> constructor;
    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> GetDictionary(const v8::Arguments& args);
    static v8::Handle<v8::Value> GetMediaBox(const v8::Arguments& args);
    static v8::Handle<v8::Value> GetCropBox(const v8::Arguments& args);
    static v8::Handle<v8::Value> GetTrimBox(const v8::Arguments& args);
    static v8::Handle<v8::Value> GetBleedBox(const v8::Arguments& args);
    static v8::Handle<v8::Value> GetArtBox(const v8::Arguments& args);
    
    static v8::Handle<v8::Value> GetArrayForPDFRectangle(const PDFRectangle& inRectangle);
    
};