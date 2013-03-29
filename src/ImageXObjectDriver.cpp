/*
 Source File : ImageXObjectDriver.cpp
 
 
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
#include "ImageXObjectDriver.h"
#include "PDFImageXObject.h"

using namespace v8;

ImageXObjectDriver::~ImageXObjectDriver()
{
    delete ImageXObject;
}

void ImageXObjectDriver::Init()
{
    // prepare the form xobject driver interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("ImageXObject"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    
    constructor_template->InstanceTemplate()->SetAccessor(String::NewSymbol("id"),GetID);
    constructor = Persistent<Function>::New(constructor_template->GetFunction());
}

Handle<Value> ImageXObjectDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    const unsigned argc = 0;
    Handle<Value> argv[0];
    Local<Object> instance = constructor->NewInstance(argc, argv);
    return scope.Close(instance);
}

bool ImageXObjectDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}

ImageXObjectDriver::ImageXObjectDriver()
{
    ImageXObject = NULL;
}

Persistent<Function> ImageXObjectDriver::constructor;
Persistent<FunctionTemplate> ImageXObjectDriver::constructor_template;

Handle<v8::Value> ImageXObjectDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    ImageXObjectDriver* image = new ImageXObjectDriver();
    image->Wrap(args.This());
    
    return args.This();
}

Handle<Value> ImageXObjectDriver::GetID(Local<String> property,const AccessorInfo &info)
{
    HandleScope scope;
    
    ImageXObjectDriver* image = ObjectWrap::Unwrap<ImageXObjectDriver>(info.Holder());
    
    if(!image->ImageXObject)
    {
		ThrowException(Exception::TypeError(String::New("image object not initialized, create using pdfWriter.createFormXObject")));
        return scope.Close(Undefined());
    }
    
    return scope.Close(Number::New(image->ImageXObject->GetImageObjectID()));
}

