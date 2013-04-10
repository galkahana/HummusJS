/*
 Source File : UsedFontDriver.h
 
 
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
#include "PDFTextStringDriver.h"

using namespace v8;

Persistent<Function> PDFTextStringDriver::constructor;

void PDFTextStringDriver::Init()
{
    // prepare the page interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->SetClassName(String::NewSymbol("PDFTextString"));
    t->InstanceTemplate()->SetInternalFieldCount(1);
    t->PrototypeTemplate()->Set(String::NewSymbol("toString"),FunctionTemplate::New(ToString)->GetFunction());
    
    constructor = Persistent<Function>::New(t->GetFunction());
}

Handle<Value> PDFTextStringDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 || !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. Provide 1 argument which is a string")));
        return scope.Close(Undefined());
    }
    
    const unsigned argc = 1;
    Handle<Value> argv[argc] = {args[0]};
    Local<Object> instance = constructor->NewInstance(argc, argv);
    
    return scope.Close(instance);
}

Handle<Value> PDFTextStringDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    PDFTextStringDriver* element = new PDFTextStringDriver();
    element->mTextString.FromUTF8(*String::Utf8Value(args[0]->ToString()));
    
    element->Wrap(args.This());
    return args.This();
}

Handle<Value> PDFTextStringDriver::ToString(const Arguments& args)
{
    HandleScope scope;
    
    PDFTextStringDriver* element = ObjectWrap::Unwrap<PDFTextStringDriver>(args.This());
    
    return scope.Close(String::New(element->mTextString.ToString().c_str()));
}
