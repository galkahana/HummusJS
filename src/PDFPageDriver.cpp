/*
 Source File : PDFPageDriver.cpp
 
 
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
#include "PDFPageDriver.h"
#include "PDFRectangle.h"
#include "ResourcesDictionaryDriver.h"

using namespace v8;

Persistent<Function> PDFPageDriver::constructor;
Persistent<FunctionTemplate> PDFPageDriver::constructor_template;

PDFPageDriver::PDFPageDriver()
{
    ContentContext = NULL;
}

void PDFPageDriver::Init()
{
    // prepare the page interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("PDFPage"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    constructor_template->InstanceTemplate()->SetAccessor(String::NewSymbol("mediaBox"),GetMediaBox,SetMediaBox);
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("getResourcesDictionary"),FunctionTemplate::New(GetResourcesDictionary)->GetFunction());
    
    constructor = Persistent<Function>::New(constructor_template->GetFunction());
}

Handle<Value> PDFPageDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() == 4)
    {
        const unsigned argc = 4;
        Handle<Value> argv[argc] = {args[0],args[1],args[2],args[3]};
        Local<Object> instance = constructor->NewInstance(argc, argv);
        
        return scope.Close(instance);
    }
    else
    {
        const unsigned argc = 0;
        Local<Object> instance = constructor->NewInstance(argc, NULL);
    
        return scope.Close(instance);
    }
}

bool PDFPageDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}

Handle<Value> PDFPageDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    PDFPageDriver* pdfPage = new PDFPageDriver();
    pdfPage->Wrap(args.This());
    
    
    if(args.Length() == 4 && args[0]->IsNumber() && args[1]->IsNumber() && args[2]->IsNumber() && args[3]->IsNumber())
    {
        pdfPage->mPDFPage.SetMediaBox(PDFRectangle(args[0]->ToNumber()->Value(),
                                                   args[1]->ToNumber()->Value(),
                                                   args[2]->ToNumber()->Value(),
                                                   args[3]->ToNumber()->Value()));
    }
    
    return args.This();
}

Handle<Value> PDFPageDriver::GetMediaBox(Local<String> property,const AccessorInfo &info)
{
    HandleScope scope;

    PDFPageDriver* pageDriver = ObjectWrap::Unwrap<PDFPageDriver>(info.Holder());
    
    Local<Array> mediaBox = Array::New(4);
    
    mediaBox->Set(v8::Number::New(0),v8::Number::New(pageDriver->mPDFPage.GetMediaBox().LowerLeftX));
    mediaBox->Set(v8::Number::New(1),v8::Number::New(pageDriver->mPDFPage.GetMediaBox().LowerLeftY));
    mediaBox->Set(v8::Number::New(2),v8::Number::New(pageDriver->mPDFPage.GetMediaBox().UpperRightX));
    mediaBox->Set(v8::Number::New(3),v8::Number::New(pageDriver->mPDFPage.GetMediaBox().UpperRightY));

    return scope.Close(mediaBox);
}

void PDFPageDriver::SetMediaBox(Local<String> property,Local<Value> value,const AccessorInfo &info)
{
    HandleScope scope;
    
    PDFPageDriver* pageDriver = ObjectWrap::Unwrap<PDFPageDriver>(info.Holder());

    if(!value->IsArray())
        ThrowException(Exception::TypeError(String::New("Media box is set to a value which is not a 4 numbers array")));
    
    if(value->ToObject()->Get(v8::String::New("length"))->ToObject()->Uint32Value() != 4)
        ThrowException(Exception::TypeError(String::New("Media box is set to a value which is not a 4 numbers array")));
    
    pageDriver->mPDFPage.SetMediaBox(PDFRectangle(value->ToObject()->Get(0)->ToNumber()->Value(),
                                                  value->ToObject()->Get(1)->ToNumber()->Value(),
                                                  value->ToObject()->Get(2)->ToNumber()->Value(),
                                                  value->ToObject()->Get(3)->ToNumber()->Value()));
    
}

Handle<Value> PDFPageDriver::GetResourcesDictionary(const Arguments& args)
{
    HandleScope scope;
    PDFPageDriver* pageDriver = ObjectWrap::Unwrap<PDFPageDriver>(args.This());
    
    Handle<Value> newInstance = ResourcesDictionaryDriver::NewInstance(args);
    ResourcesDictionaryDriver* resourceDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(newInstance->ToObject());
    resourceDictionaryDriver->ResourcesDictionaryInstance = &(pageDriver->GetPage()->GetResourcesDictionary());
    
    return scope.Close(newInstance);
}


