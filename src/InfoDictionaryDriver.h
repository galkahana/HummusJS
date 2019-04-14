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
#include "nodes.h"

class InfoDictionary;
class ConstructorsHolder;

class InfoDictionaryDriver : public node::ObjectWrap
{
public:
    
	DEC_SUBORDINATE_INIT(Init)
    
    InfoDictionary* InfoDictionaryInstance;

	ConstructorsHolder* holder;
    
private:
    InfoDictionaryDriver();
    
	static METHOD_RETURN_TYPE New(const ARGS_TYPE& args);

	static METHOD_RETURN_TYPE GetTitle(v8::Local<v8::String> property, const PROPERTY_TYPE &info);
    static void SetTitle(v8::Local<v8::String> property,v8::Local<v8::Value> value,const PROPERTY_SETTER_TYPE &info);
	static METHOD_RETURN_TYPE GetAuthor(v8::Local<v8::String> property, const PROPERTY_TYPE &info);
	static void SetAuthor(v8::Local<v8::String> property, v8::Local<v8::Value> value, const PROPERTY_SETTER_TYPE &info);
	static METHOD_RETURN_TYPE GetSubject(v8::Local<v8::String> property, const PROPERTY_TYPE &info);
	static void SetSubject(v8::Local<v8::String> property, v8::Local<v8::Value> value, const PROPERTY_SETTER_TYPE &info);
	static METHOD_RETURN_TYPE GetKeywords(v8::Local<v8::String> property, const PROPERTY_TYPE &info);
	static void SetKeywords(v8::Local<v8::String> property, v8::Local<v8::Value> value, const PROPERTY_SETTER_TYPE &info);
	static METHOD_RETURN_TYPE GetCreator(v8::Local<v8::String> property, const PROPERTY_TYPE &info);
	static void SetCreator(v8::Local<v8::String> property, v8::Local<v8::Value> value, const PROPERTY_SETTER_TYPE &info);
	static METHOD_RETURN_TYPE GetProducer(v8::Local<v8::String> property, const PROPERTY_TYPE &info);
	static void SetProducer(v8::Local<v8::String> property, v8::Local<v8::Value> value, const PROPERTY_SETTER_TYPE &info);
	static METHOD_RETURN_TYPE GetTrapped(v8::Local<v8::String> property, const PROPERTY_TYPE &info);
	static void SetTrapped(v8::Local<v8::String> property, v8::Local<v8::Value> value, const PROPERTY_SETTER_TYPE &info);


    // for dates i'm giving only setters. it's too bording to provide readers. you are the ones setting it for @#$@# sake.
	static METHOD_RETURN_TYPE SetCreationDate(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE SetModDate(const ARGS_TYPE& args);

	static METHOD_RETURN_TYPE AddAdditionalInfoEntry(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE RemoveAdditionalInfoEntry(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE ClearAdditionalInfoEntries(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetAdditionalInfoEntry(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetAdditionalInfoEntries(const ARGS_TYPE& args);
};