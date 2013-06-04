/*
 Source File : PDFHexStringDriver.cpp
 
 
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
#include "PDFHexStringDriver.h"
#include "RefCountPtr.h"

using namespace v8;

Persistent<Function> PDFHexStringDriver::constructor;
Persistent<FunctionTemplate> PDFHexStringDriver::constructor_template;

void PDFHexStringDriver::Init()
{
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("PDFHexString"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    constructor_template->InstanceTemplate()->SetAccessor(String::NewSymbol("value"),GetValue);
    
    PDFObjectDriver::Init(constructor_template);

    constructor = Persistent<Function>::New(constructor_template->GetFunction());
}

Handle<Value> PDFHexStringDriver::NewInstance()
{
    HandleScope scope;
    
    Local<Object> instance = constructor->NewInstance();
    
    return scope.Close(instance);
}

bool PDFHexStringDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}

Handle<Value> PDFHexStringDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    PDFHexStringDriver* driver = new PDFHexStringDriver();
    driver->Wrap(args.This());
    return args.This();
}

PDFObject* PDFHexStringDriver::GetObject()
{
    return TheObject.GetPtr();
}

Handle<Value> PDFHexStringDriver::GetValue(Local<String> property,const AccessorInfo &info)
{
    HandleScope scope;
    
    Handle<String> result = String::New(ObjectWrap::Unwrap<PDFHexStringDriver>(info.Holder())->TheObject->GetValue().c_str());
    return scope.Close(result);
}


