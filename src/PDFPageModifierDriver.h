/*
 Source File : PDFPageModifierDriver.h
 
 
 Copyright 2016 Gal Kahana HummusJS
 
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

class PDFModifiedPage;
class PDFWriter;

class PDFPageModifierDriver : public node::ObjectWrap
{
public:
    virtual ~PDFPageModifierDriver();
    
    static void Init(v8::Handle<v8::Object> inExports); // to allow instantiation from the hummus object
	static METHOD_RETURN_TYPE NewInstance(const ARGS_TYPE& args);
	static v8::Handle<v8::Value> GetNewInstance(const ARGS_TYPE& args);
    static bool HasInstance(v8::Handle<v8::Value> inObject);
    
   
private:
    PDFPageModifierDriver(PDFWriter* inWriter,unsigned long inPageIndex,bool inEnsureContentEncapsulation = false);
    
    PDFModifiedPage* mModifierPageInstance;
    
    static v8::Persistent<v8::Function> constructor;
    static v8::Persistent<v8::FunctionTemplate> constructor_template;
	static METHOD_RETURN_TYPE New(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE StartContext(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetContext(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE EndContext(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE AttachURLLinktoCurrentPage(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE WritePage(const ARGS_TYPE& args);

};