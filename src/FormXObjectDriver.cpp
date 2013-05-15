/*
 Source File : FormXObjectDriver.cpp
 
 
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
#include "FormXObjectDriver.h"
#include "PDFFormXObject.h"
#include "XObjectContentContextDriver.h"
#include "ResourcesDictionaryDriver.h"

using namespace v8;

FormXObjectDriver::~FormXObjectDriver()
{
    delete FormXObject;
}

FormXObjectDriver::FormXObjectDriver()
{
    mPDFWriterDriver = NULL;
    FormXObject = NULL;
}

void FormXObjectDriver::Init()
{
    // prepare the form xobject driver interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("FormXObject"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    
    constructor_template->InstanceTemplate()->SetAccessor(String::NewSymbol("id"),GetID);
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("getContentContext"),FunctionTemplate::New(GetContentContext)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("getResourcesDictinary"),FunctionTemplate::New(GetResourcesDictionary)->GetFunction());
    constructor = Persistent<Function>::New(constructor_template->GetFunction());
}

Handle<Value> FormXObjectDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    const unsigned argc = 0;
    Local<Object> instance = constructor->NewInstance(argc, NULL);
    return scope.Close(instance);
}

bool FormXObjectDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
        constructor_template->HasInstance(inObject->ToObject());
}

Persistent<Function> FormXObjectDriver::constructor;
Persistent<FunctionTemplate> FormXObjectDriver::constructor_template;

Handle<v8::Value> FormXObjectDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    FormXObjectDriver* form = new FormXObjectDriver();
    form->Wrap(args.This());
    
    return args.This();
}

Handle<Value> FormXObjectDriver::GetID(Local<String> property,const AccessorInfo &info)
{
    HandleScope scope;
    
    FormXObjectDriver* form = ObjectWrap::Unwrap<FormXObjectDriver>(info.Holder());
    
    if(!form->FormXObject)
    {
		ThrowException(Exception::TypeError(String::New("form object not initialized, create using pdfWriter.CreateFormXObject")));
        return scope.Close(Undefined());
    }
    
    return scope.Close(Number::New(form->FormXObject->GetObjectID()));
}

Handle<Value> FormXObjectDriver::GetContentContext(const Arguments& args)
{
    HandleScope scope;
    FormXObjectDriver* formDriver = ObjectWrap::Unwrap<FormXObjectDriver>(args.This());
  
    Handle<Value> newInstance = XObjectContentContextDriver::NewInstance(args);
    XObjectContentContextDriver* contentContextDriver = ObjectWrap::Unwrap<XObjectContentContextDriver>(newInstance->ToObject());
    contentContextDriver->ContentContext = formDriver->FormXObject->GetContentContext();
    contentContextDriver->FormOfContext = formDriver->FormXObject;
    contentContextDriver->SetResourcesDictionary(&(formDriver->FormXObject->GetResourcesDictionary()));
    contentContextDriver->SetPDFWriter(formDriver->mPDFWriterDriver);
    
    return scope.Close(newInstance);
}

Handle<Value> FormXObjectDriver::GetResourcesDictionary(const Arguments& args)
{
    HandleScope scope;
    FormXObjectDriver* formDriver = ObjectWrap::Unwrap<FormXObjectDriver>(args.This());
    
    Handle<Value> newInstance = ResourcesDictionaryDriver::NewInstance(args);
    ResourcesDictionaryDriver* resourceDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(newInstance->ToObject());
    resourceDictionaryDriver->ResourcesDictionaryInstance = &(formDriver->FormXObject->GetResourcesDictionary());
    
    return scope.Close(newInstance);
}


void FormXObjectDriver::SetPDFWriter(PDFWriterDriver* inPDFWriterDriver)
{
    mPDFWriterDriver = inPDFWriterDriver;
}



