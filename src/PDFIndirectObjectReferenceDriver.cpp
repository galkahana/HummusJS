/*
 Source File : PDFIndirectObjectReferenceDriver.h
 
 
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
#include "PDFIndirectObjectReferenceDriver.h"
#include "PDFIndirectObjectReference.h"
#include "RefCountPtr.h"

using namespace v8;

Persistent<Function> PDFIndirectObjectReferenceDriver::constructor;
Persistent<FunctionTemplate> PDFIndirectObjectReferenceDriver::constructor_template;

void PDFIndirectObjectReferenceDriver::Init()
{
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("PDFIndirectObjectReference"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("getObjectID"),FunctionTemplate::New(GetObjectID)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("getVersion"),FunctionTemplate::New(GetVersion)->GetFunction());
    
    PDFObjectDriver::Init(constructor_template);

    constructor = Persistent<Function>::New(constructor_template->GetFunction());
}

Handle<Value> PDFIndirectObjectReferenceDriver::NewInstance()
{
    HandleScope scope;
    
    const unsigned argc = 0;
    Handle<Value> argv[argc];
    Local<Object> instance = constructor->NewInstance(argc, argv);
    
    return scope.Close(instance);
}

bool PDFIndirectObjectReferenceDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}

Handle<Value> PDFIndirectObjectReferenceDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    PDFIndirectObjectReferenceDriver* driver = new PDFIndirectObjectReferenceDriver();
    driver->Wrap(args.This());
    return args.This();
}

PDFObject* PDFIndirectObjectReferenceDriver::GetObject()
{
    return TheObject.GetPtr();
}

Handle<Value> PDFIndirectObjectReferenceDriver::GetObjectID(const Arguments& args)
{
    HandleScope scope;
    
    Local<Number> result = Number::New(ObjectWrap::Unwrap<PDFIndirectObjectReferenceDriver>(args.This())->TheObject->mObjectID);
    return scope.Close(result);
}

Handle<Value> PDFIndirectObjectReferenceDriver::GetVersion(const Arguments& args)
{
    HandleScope scope;
    
    Local<Number> result = Number::New(ObjectWrap::Unwrap<PDFIndirectObjectReferenceDriver>(args.This())->TheObject->mVersion);
    return scope.Close(result);
}
