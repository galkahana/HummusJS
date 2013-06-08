/*
 Source File : DocumentContextDriver.cpp
 
 
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
#include "DocumentContextDriver.h"
#include "DocumentContext.h"
#include "InfoDictionaryDriver.h"


using namespace v8;
using namespace PDFHummus;


void DocumentContextDriver::Init()
{
    // prepare the form xobject driver interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->SetClassName(String::NewSymbol("DocumentContext"));
    t->InstanceTemplate()->SetInternalFieldCount(1);
    
    t->PrototypeTemplate()->Set(String::NewSymbol("getInfoDictionary"),FunctionTemplate::New(GetInfoDictionary)->GetFunction());
    
    constructor = Persistent<Function>::New(t->GetFunction());
}

Handle<Value> DocumentContextDriver::NewInstance()
{
    HandleScope scope;
    
    Local<Object> instance = constructor->NewInstance();
    return scope.Close(instance);
}

DocumentContextDriver::DocumentContextDriver()
{
    DocumentContextInstance = NULL;
}

Persistent<Function> DocumentContextDriver::constructor;

Handle<Value> DocumentContextDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    DocumentContextDriver* objectsContext = new DocumentContextDriver();
    objectsContext->Wrap(args.This());
    
    return args.This();
}

Handle<Value> DocumentContextDriver::GetInfoDictionary(const Arguments& args)
{
    HandleScope scope;
    
    DocumentContextDriver* driver = ObjectWrap::Unwrap<DocumentContextDriver>(args.This());
    if(!driver->DocumentContextInstance)
    {
		ThrowException(Exception::Error(String::New("document context driver not initialized. use the pdfwriter to get the current document context")));
        return scope.Close(Undefined());
    }
    
    Handle<Value> infoDictionaryDriverObject = InfoDictionaryDriver::NewInstance();
    InfoDictionaryDriver* infoDictDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(infoDictionaryDriverObject->ToObject());
    infoDictDriver->InfoDictionaryInstance = &(driver->DocumentContextInstance->GetTrailerInformation().GetInfo());
    
    return scope.Close(infoDictionaryDriverObject);
}


