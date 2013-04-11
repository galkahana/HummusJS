/*
 Source File : PDFNameDriver.cpp
 
 
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
#include "PDFNameDriver.h"
#include "RefCountPtr.h"

using namespace v8;

Persistent<Function> PDFNameDriver::constructor;
Persistent<FunctionTemplate> PDFNameDriver::constructor_template;

void PDFNameDriver::Init()
{
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("PDFName"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    constructor_template->InstanceTemplate()->SetAccessor(String::NewSymbol("value"),GetValue);
    
    PDFObjectDriver::Init(constructor_template);

    constructor = Persistent<Function>::New(constructor_template->GetFunction());
}

Handle<Value> PDFNameDriver::NewInstance()
{
    HandleScope scope;
    
    const unsigned argc = 0;
    Local<Object> instance = constructor->NewInstance(argc, NULL);
    
    return scope.Close(instance);
}

bool PDFNameDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}

Handle<Value> PDFNameDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    PDFNameDriver* driver = new PDFNameDriver();
    driver->Wrap(args.This());
    return args.This();
}

PDFObject* PDFNameDriver::GetObject()
{
    return TheObject.GetPtr();
}

Handle<Value> PDFNameDriver::GetValue(Local<String> property,const AccessorInfo &info)
{
    HandleScope scope;
    
    Handle<String> result = String::New(ObjectWrap::Unwrap<PDFNameDriver>(info.Holder())->TheObject->GetValue().c_str());
    return scope.Close(result);
}


