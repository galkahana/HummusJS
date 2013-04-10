/*
 Source File : ImageXObjectDriver
 
 
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

class DictionaryContext;

class DictionaryContextDriver : public node::ObjectWrap
{
public:
    virtual ~DictionaryContextDriver();
    
    static void Init();
    static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);
    
    static bool HasInstance(v8::Handle<v8::Value> inObject);
    
    DictionaryContext* DictionaryContextInstance;
    
private:
    DictionaryContextDriver();
    
    static v8::Persistent<v8::Function> constructor;
    static v8::Persistent<v8::FunctionTemplate> constructor_template;
    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> WriteKey(const v8::Arguments& args);
    static v8::Handle<v8::Value> WriteNameValue(const v8::Arguments& args);
    static v8::Handle<v8::Value> WriteRectangleValue(const v8::Arguments& args);
    static v8::Handle<v8::Value> WriteLiteralStringValue(const v8::Arguments& args);
    static v8::Handle<v8::Value> WriteBooleanValue(const v8::Arguments& args);
    static v8::Handle<v8::Value> WriteObjectReferenceValue(const v8::Arguments& args);
};