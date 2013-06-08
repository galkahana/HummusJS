/*
 Source File : InfoDictionaryDriver.h
 
 
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

class InfoDictionary;

class InfoDictionaryDriver : public node::ObjectWrap
{
public:
    
    static void Init();
    static v8::Handle<v8::Value> NewInstance();
    
    InfoDictionary* InfoDictionaryInstance;
    
private:
    InfoDictionaryDriver();
    
    static v8::Persistent<v8::Function> constructor;
    static v8::Handle<v8::Value> New(const v8::Arguments& args);

    static v8::Handle<v8::Value> GetTitle(v8::Local<v8::String> property,const v8::AccessorInfo &info);
    static void SetTitle(v8::Local<v8::String> property,v8::Local<v8::Value> value,const v8::AccessorInfo &info);
    static v8::Handle<v8::Value> GetAuthor(v8::Local<v8::String> property,const v8::AccessorInfo &info);
    static void SetAuthor(v8::Local<v8::String> property,v8::Local<v8::Value> value,const v8::AccessorInfo &info);
    static v8::Handle<v8::Value> GetSubject(v8::Local<v8::String> property,const v8::AccessorInfo &info);
    static void SetSubject(v8::Local<v8::String> property,v8::Local<v8::Value> value,const v8::AccessorInfo &info);
    static v8::Handle<v8::Value> GetKeywords(v8::Local<v8::String> property,const v8::AccessorInfo &info);
    static void SetKeywords(v8::Local<v8::String> property,v8::Local<v8::Value> value,const v8::AccessorInfo &info);
    static v8::Handle<v8::Value> GetCreator(v8::Local<v8::String> property,const v8::AccessorInfo &info);
    static void SetCreator(v8::Local<v8::String> property,v8::Local<v8::Value> value,const v8::AccessorInfo &info);
    static v8::Handle<v8::Value> GetProducer(v8::Local<v8::String> property,const v8::AccessorInfo &info);
    static void SetProducer(v8::Local<v8::String> property,v8::Local<v8::Value> value,const v8::AccessorInfo &info);
    static v8::Handle<v8::Value> GetTrapped(v8::Local<v8::String> property,const v8::AccessorInfo &info);
    static void SetTrapped(v8::Local<v8::String> property,v8::Local<v8::Value> value,const v8::AccessorInfo &info);

    // for dates i'm giving only setters. it's too bording to provide readers. you are the ones setting it for @#$@# sake.
    static v8::Handle<v8::Value> SetCreationDate(const v8::Arguments& args);
    static v8::Handle<v8::Value> SetModDate(const v8::Arguments& args);

    static v8::Handle<v8::Value> AddAdditionalInfoEntry(const v8::Arguments& args);
    static v8::Handle<v8::Value> RemoveAdditionalInfoEntry(const v8::Arguments& args);
    static v8::Handle<v8::Value> ClearAdditionalInfoEntries(const v8::Arguments& args);
    static v8::Handle<v8::Value> GetAdditionalInfoEntry(const v8::Arguments& args);
    static v8::Handle<v8::Value> GetAdditionalInfoEntries(const v8::Arguments& args);
};