/*
 Source File : PDFPageDriver.h
 
 
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

#include "PDFPage.h"

class PageContentContext;

class PDFPageDriver : public node::ObjectWrap
{
public:
    virtual ~PDFPageDriver();
    
    
    static void Init(v8::Handle<v8::Object> inExports);
	static METHOD_RETURN_TYPE NewInstance(const ARGS_TYPE& args);
	static v8::Handle<v8::Value> GetNewInstance(const ARGS_TYPE& args);
	static v8::Handle<v8::Value> GetNewInstance(PDFPage* inPage);
    static bool HasInstance(v8::Handle<v8::Value> inObject);
    
    PDFPage* GetPage(){return mPDFPage;}
    
    PageContentContext* ContentContext;
    
    
private:
    PDFPageDriver();
    
    
    static v8::Persistent<v8::Function> constructor;
    static v8::Persistent<v8::FunctionTemplate> constructor_template;
	static METHOD_RETURN_TYPE New(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetMediaBox(v8::Local<v8::String> property, const PROPERTY_TYPE &info);
    static void SetMediaBox(v8::Local<v8::String> property,v8::Local<v8::Value> value,const PROPERTY_SETTER_TYPE &info);
	static METHOD_RETURN_TYPE GetBleedBox(v8::Local<v8::String> property, const PROPERTY_TYPE &info);
    static void SetBleedBox(v8::Local<v8::String> property,v8::Local<v8::Value> value,const PROPERTY_SETTER_TYPE &info);
	static METHOD_RETURN_TYPE GetCropBox(v8::Local<v8::String> property, const PROPERTY_TYPE &info);
    static void SetCropBox(v8::Local<v8::String> property,v8::Local<v8::Value> value,const PROPERTY_SETTER_TYPE &info);
	static METHOD_RETURN_TYPE GetTrimBox(v8::Local<v8::String> property, const PROPERTY_TYPE &info);
    static void SetTrimBox(v8::Local<v8::String> property,v8::Local<v8::Value> value,const PROPERTY_SETTER_TYPE &info);
	static METHOD_RETURN_TYPE GetArtBox(v8::Local<v8::String> property, const PROPERTY_TYPE &info);
    static void SetArtBox(v8::Local<v8::String> property,v8::Local<v8::Value> value,const PROPERTY_SETTER_TYPE &info);
	static METHOD_RETURN_TYPE GetRotate(v8::Local<v8::String> property, const PROPERTY_TYPE &info);
    static void SetRotate(v8::Local<v8::String> property,v8::Local<v8::Value> value,const PROPERTY_SETTER_TYPE &info);
	static METHOD_RETURN_TYPE GetResourcesDictionary(const ARGS_TYPE& args);
    
    PDFPage* mPDFPage;
    bool mOwnsPage;
};