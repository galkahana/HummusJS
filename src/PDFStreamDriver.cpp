/*
 Source File : PDFStreamDriver.h
 
 
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
#include "PDFStreamDriver.h"
#include "PDFStream.h"
#include "ByteWriterDriver.h"

using namespace v8;

Persistent<Function> PDFStreamDriver::constructor;
Persistent<FunctionTemplate> PDFStreamDriver::constructor_template;

PDFStreamDriver::PDFStreamDriver()
{
    PDFStreamInstance = NULL;
}

void PDFStreamDriver::Init()
{
    // prepare the page interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("PDFStream"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("getWriteStream"),FunctionTemplate::New(GetWriteStream)->GetFunction());
    
    constructor = Persistent<Function>::New(constructor_template->GetFunction());
}

Handle<Value> PDFStreamDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    Local<Object> instance = constructor->NewInstance();
    return scope.Close(instance);
}

bool PDFStreamDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}

Handle<Value> PDFStreamDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    PDFStreamDriver* driver = new PDFStreamDriver();
    driver->Wrap(args.This());
    return args.This();
}

Handle<Value> PDFStreamDriver::GetWriteStream(const Arguments& args)
{
    HandleScope scope;
    
    Handle<Value> result = ByteWriterDriver::NewInstance(args);
    
    ObjectWrap::Unwrap<ByteWriterDriver>(result->ToObject())->SetStream(
                                                                        ObjectWrap::Unwrap<PDFStreamDriver>(args.This())->PDFStreamInstance->GetWriteStream(), false);
    
    return scope.Close(result);
}
