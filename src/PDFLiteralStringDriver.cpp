/*
 Source File : PDFLiteralStringDriver.h
 
 
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
#include "PDFLiteralStringDriver.h"
#include "PDFLiteralString.h"
#include "RefCountPtr.h"

using namespace v8;

Persistent<Function> PDFLiteralStringDriver::constructor;
Persistent<FunctionTemplate> PDFLiteralStringDriver::constructor_template;

void PDFLiteralStringDriver::Init()
{
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("PDFLiteralString"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    constructor_template->InstanceTemplate()->SetAccessor(String::NewSymbol("value"),GetValue);
 
    PDFObjectDriver::Init(constructor_template);

    constructor = Persistent<Function>::New(constructor_template->GetFunction());
}

Handle<Value> PDFLiteralStringDriver::NewInstance()
{
    HandleScope scope;
    
    const unsigned argc = 0;
    Handle<Value> argv[argc];
    Local<Object> instance = constructor->NewInstance(argc, argv);
    
    return scope.Close(instance);
}

bool PDFLiteralStringDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}

Handle<Value> PDFLiteralStringDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    PDFLiteralStringDriver* driver = new PDFLiteralStringDriver();
    driver->Wrap(args.This());
    return args.This();
}

PDFObject* PDFLiteralStringDriver::GetObject()
{
    return TheObject.GetPtr();
}

Handle<Value> PDFLiteralStringDriver::GetValue(Local<String> property,const AccessorInfo &info)
{
    HandleScope scope;
    
    Handle<String> result = String::New(ObjectWrap::Unwrap<PDFLiteralStringDriver>(info.Holder())->TheObject->GetValue().c_str());
    return scope.Close(result);
}


