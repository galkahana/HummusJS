/*
 Source File : PDFStreamInputDriver.cpp
 
 
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
#include "PDFStreamInputDriver.h"
#include "RefCountPtr.h"
#include "PDFDictionary.h"
#include <string>

using namespace v8;

Persistent<Function> PDFStreamInputDriver::constructor;
Persistent<FunctionTemplate> PDFStreamInputDriver::constructor_template;

void PDFStreamInputDriver::Init()
{
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("PDFStreamInput"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("getDictionary"),FunctionTemplate::New(GetDictionary)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("getStreamContentStart"),FunctionTemplate::New(GetStreamContentStart)->GetFunction());

    PDFObjectDriver::Init(constructor_template);

    constructor = Persistent<Function>::New(constructor_template->GetFunction());
}

Handle<Value> PDFStreamInputDriver::NewInstance()
{
    HandleScope scope;
    
    const unsigned argc = 0;
    Local<Object> instance = constructor->NewInstance(argc, NULL);
    
    return scope.Close(instance);
}

bool PDFStreamInputDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}

Handle<Value> PDFStreamInputDriver::New(const Arguments& args)
{
    HandleScope scope;
    PDFStreamInputDriver* driver = new PDFStreamInputDriver();

    driver->Wrap(args.This());
    return args.This();
}

PDFObject* PDFStreamInputDriver::GetObject()
{
    return TheObject.GetPtr();
}

Handle<Value> PDFStreamInputDriver::GetDictionary(const Arguments& args)
{
    HandleScope scope;
    PDFStreamInputDriver* driver = ObjectWrap::Unwrap<PDFStreamInputDriver>(args.This());
    RefCountPtr<PDFDictionary> streamDict = driver->TheObject->QueryStreamDictionary();
    Handle<Value> result = PDFObjectDriver::CreateDriver(streamDict.GetPtr());

    return scope.Close(result);
}

Handle<Value> PDFStreamInputDriver::GetStreamContentStart(const Arguments& args)
{
    HandleScope scope;
    PDFStreamInputDriver* driver = ObjectWrap::Unwrap<PDFStreamInputDriver>(args.This());
    Handle<Number> result = Number::New(driver->TheObject->GetStreamContentStart());
    
    return scope.Close(result);
}
