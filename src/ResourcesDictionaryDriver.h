/*
 Source File : ResourcesDictionaryDriver.h
 
 
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

class ResourcesDictionary;

class ResourcesDictionaryDriver : public node::ObjectWrap
{
public:
    static void Init();
    static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);

    // set externally
    ResourcesDictionary* ResourcesDictionaryInstance;
    
private:
    ResourcesDictionaryDriver();
    
    static v8::Persistent<v8::Function> constructor;
    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> AddFormXObjectMapping(const v8::Arguments& args);
    static v8::Handle<v8::Value> AddImageXObjectMapping(const v8::Arguments& args);
    static v8::Handle<v8::Value> AddProcsetResource(const v8::Arguments& args);
    static v8::Handle<v8::Value> AddExtGStateMapping(const v8::Arguments& args);
    static v8::Handle<v8::Value> AddFontMapping(const v8::Arguments& args);
    static v8::Handle<v8::Value> AddColorSpaceMapping(const v8::Arguments& args);
    static v8::Handle<v8::Value> AddPatternMapping(const v8::Arguments& args);
    static v8::Handle<v8::Value> AddPropertyMapping(const v8::Arguments& args);
    static v8::Handle<v8::Value> AddXObjectMapping(const v8::Arguments& args);
    static v8::Handle<v8::Value> AddShadingMapping(const v8::Arguments& args);

    
    
    
    
    
    

};