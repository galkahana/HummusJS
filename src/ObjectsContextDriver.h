/*
 Source File : ObjectsContextDriver
 
 
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

class ObjectsContext;

class ObjectsContextDriver : public node::ObjectWrap
{
public:
    
    static void Init();
    static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);
    
    ObjectsContext* ObjectsContextInstance;
    
private:
    ObjectsContextDriver();
    
    static v8::Persistent<v8::Function> constructor;
    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> AllocateNewObjectID(const v8::Arguments& args);
    static v8::Handle<v8::Value> StartDictionary(const v8::Arguments& args);
    static v8::Handle<v8::Value> StartArray(const v8::Arguments& args);
    static v8::Handle<v8::Value> WriteNumber(const v8::Arguments& args);
    static v8::Handle<v8::Value> EndArray(const v8::Arguments& args);
    static v8::Handle<v8::Value> EndLine(const v8::Arguments& args);
    static v8::Handle<v8::Value> EndDictionary(const v8::Arguments& args);
    static v8::Handle<v8::Value> EndIndirectObject(const v8::Arguments& args);
    static v8::Handle<v8::Value> WriteIndirectObjectReference(const v8::Arguments& args);
    static v8::Handle<v8::Value> StartNewIndirectObject(const v8::Arguments& args);
    static v8::Handle<v8::Value> StartModifiedIndirectObject(const v8::Arguments& args);
    static v8::Handle<v8::Value> DeleteObject(const v8::Arguments& args);

    static v8::Handle<v8::Value> WriteName(const v8::Arguments& args);
    static v8::Handle<v8::Value> WriteLiteralString(const v8::Arguments& args);
    static v8::Handle<v8::Value> WriteHexString(const v8::Arguments& args);
    static v8::Handle<v8::Value> WriteBoolean(const v8::Arguments& args);
    static v8::Handle<v8::Value> WriteKeyword(const v8::Arguments& args);
    static v8::Handle<v8::Value> WriteComment(const v8::Arguments& args);
    static v8::Handle<v8::Value> SetCompressStreams(const v8::Arguments& args);
    static v8::Handle<v8::Value> StartPDFStream(const v8::Arguments& args);
    static v8::Handle<v8::Value> StartUnfilteredPDFStream(const v8::Arguments& args);
    static v8::Handle<v8::Value> EndPDFStream(const v8::Arguments& args);
    
    static v8::Handle<v8::Value> StartFreeContext(const v8::Arguments& args);
    static v8::Handle<v8::Value> EndFreeContext(const v8::Arguments& args);
    
};