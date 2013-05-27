/*
 Source File : PDFPageInputDriver
 
 
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
#include "PDFPageInputDriver.h"
#include "PDFPageInput.h"
#include "PDFDictionaryDriver.h"

using namespace v8;

Persistent<Function> PDFPageInputDriver::constructor;

PDFPageInputDriver::PDFPageInputDriver()
{
    PageInput = NULL;
}

PDFPageInputDriver::~PDFPageInputDriver()
{
    delete PageInput;
}

void PDFPageInputDriver::Init()
{
    // prepare the page interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->SetClassName(String::NewSymbol("PDFPageInput"));
    t->InstanceTemplate()->SetInternalFieldCount(1);
    t->PrototypeTemplate()->Set(String::NewSymbol("getDictionary"),FunctionTemplate::New(GetDictionary)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("getMediaBox"),FunctionTemplate::New(GetMediaBox)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("getCropBox"),FunctionTemplate::New(GetCropBox)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("getTrimBox"),FunctionTemplate::New(GetTrimBox)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("getBleedBox"),FunctionTemplate::New(GetBleedBox)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("getArtBox"),FunctionTemplate::New(GetArtBox)->GetFunction());
    
    constructor = Persistent<Function>::New(t->GetFunction());
}

Handle<Value> PDFPageInputDriver::NewInstance()
{
    HandleScope scope;
    
    Local<Object> instance = constructor->NewInstance();
    
    return scope.Close(instance);
}

Handle<Value> PDFPageInputDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    PDFPageInputDriver* element = new PDFPageInputDriver();
    element->Wrap(args.This());
    return args.This();
}

/*
 static v8::Handle<v8::Value> GetDictionary(const v8::Arguments& args);
 static v8::Handle<v8::Value> GetMediaBox(const v8::Arguments& args);
 static v8::Handle<v8::Value> GetCropBox(const v8::Arguments& args);
 static v8::Handle<v8::Value> GetTrimBox(const v8::Arguments& args);
 static v8::Handle<v8::Value> GetBleedBox(const v8::Arguments& args);
 static v8::Handle<v8::Value> GetArtBox(const v8::Arguments& args);
 */

Handle<Value> PDFPageInputDriver::GetDictionary(const Arguments& args)
{
    HandleScope scope;
    
    PDFPageInputDriver* element = ObjectWrap::Unwrap<PDFPageInputDriver>(args.This());
    
    if(!element->PageInput)
    {
		ThrowException(Exception::Error(String::New("page input not initialized. create one using the PDFReader.parsePage")));
        return scope.Close(Undefined());
    }

    Handle<Value> newInstance;

    newInstance = PDFDictionaryDriver::NewInstance();
    ObjectWrap::Unwrap<PDFDictionaryDriver>(newInstance->ToObject())->TheObject = element->PageInputDictionary;
    return scope.Close(newInstance);
}

Handle<Value> PDFPageInputDriver::GetMediaBox(const Arguments& args)
{
    
    PDFPageInputDriver* element = ObjectWrap::Unwrap<PDFPageInputDriver>(args.This());
    
    if(!element->PageInput)
    {
        HandleScope scope;
		ThrowException(Exception::Error(String::New("page input not initialized. create one using the PDFReader.parsePage")));
        return scope.Close(Undefined());
    }
    return GetArrayForPDFRectangle(element->PageInput->GetMediaBox());
}

Handle<Value> PDFPageInputDriver::GetArrayForPDFRectangle(const PDFRectangle& inRectangle)
{
    HandleScope scope;

    Local<Array> result = Array::New(4);
    
    result->Set(Number::New(0),Number::New(inRectangle.LowerLeftX));
    result->Set(Number::New(1),Number::New(inRectangle.LowerLeftY));
    result->Set(Number::New(2),Number::New(inRectangle.UpperRightX));
    result->Set(Number::New(3),Number::New(inRectangle.UpperRightY));
    return scope.Close(result);
    
}

Handle<Value> PDFPageInputDriver::GetCropBox(const Arguments& args)
{
    
    PDFPageInputDriver* element = ObjectWrap::Unwrap<PDFPageInputDriver>(args.This());
    
    if(!element->PageInput)
    {
        HandleScope scope;
		ThrowException(Exception::Error(String::New("page input not initialized. create one using the PDFReader.parsePage")));
        return scope.Close(Undefined());
    }
    return GetArrayForPDFRectangle(element->PageInput->GetCropBox());
}

Handle<Value> PDFPageInputDriver::GetTrimBox(const Arguments& args)
{
    
    PDFPageInputDriver* element = ObjectWrap::Unwrap<PDFPageInputDriver>(args.This());
    
    if(!element->PageInput)
    {
        HandleScope scope;
		ThrowException(Exception::Error(String::New("page input not initialized. create one using the PDFReader.parsePage")));
        return scope.Close(Undefined());
    }
    return GetArrayForPDFRectangle(element->PageInput->GetTrimBox());
}

Handle<Value> PDFPageInputDriver::GetBleedBox(const Arguments& args)
{
    
    PDFPageInputDriver* element = ObjectWrap::Unwrap<PDFPageInputDriver>(args.This());
    
    if(!element->PageInput)
    {
        HandleScope scope;
		ThrowException(Exception::Error(String::New("page input not initialized. create one using the PDFReader.parsePage")));
        return scope.Close(Undefined());
    }
    return GetArrayForPDFRectangle(element->PageInput->GetBleedBox());
}

Handle<Value> PDFPageInputDriver::GetArtBox(const Arguments& args)
{
    
    PDFPageInputDriver* element = ObjectWrap::Unwrap<PDFPageInputDriver>(args.This());
    
    if(!element->PageInput)
    {
        HandleScope scope;
		ThrowException(Exception::Error(String::New("page input not initialized. create one using the PDFReader.parsePage")));
        return scope.Close(Undefined());
    }
    return GetArrayForPDFRectangle(element->PageInput->GetArtBox());
}
