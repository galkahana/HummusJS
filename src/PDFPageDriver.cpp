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

using namespace v8;

Persistent<Function> PDFPageDriver::constructor;



void PDFPageDriver::Init()
{
    // prepare the pdfwriter interfrace template
    Local<FunctionTemplate> pdfPageFT = FunctionTemplate::New(New);
    pdfPageFT->SetClassName(String::NewSymbol("PDFPage"));
    pdfPageFT->InstanceTemplate()->SetInternalFieldCount(1);
    pdfPageFT->InstanceTemplate()->SetAccessor(String::NewSymbol("mediaBox"),GetMediaBox,SetMediaBox);
    
    constructor = Persistent<Function>::New(pdfPageFT->GetFunction());
}

Handle<Value> PDFPageDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    const unsigned argc = 0;
    Handle<Value> argv[argc];
    Local<Object> instance = constructor->NewInstance(argc, argv);
    
    return scope.Close(instance);
}


Handle<Value> PDFPageDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    PDFPageDriver* pdfPage = new PDFPageDriver();
    pdfPage->Wrap(args.This());
    
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


