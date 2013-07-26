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
#include <string>

using namespace v8;

Persistent<Function> PDFTextStringDriver::constructor;

void PDFTextStringDriver::Init(Handle<Object> inExports)
{
    // prepare the page interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->SetClassName(String::NewSymbol("PDFTextString"));
    t->InstanceTemplate()->SetInternalFieldCount(1);
    t->PrototypeTemplate()->Set(String::NewSymbol("toBytesArray"),FunctionTemplate::New(ToBytesArray)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("toString"),FunctionTemplate::New(ToString)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("fromString"),FunctionTemplate::New(FromString)->GetFunction());
    
    constructor = Persistent<Function>::New(t->GetFunction());
    inExports->Set(String::NewSymbol("PDFTextString"),constructor);
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
    if(args.Length() > 0 && args[0]->IsString())
        element->mTextString.FromUTF8(*String::Utf8Value(args[0]->ToString()));
    
    element->Wrap(args.This());
    return args.This();
}

Handle<Value> PDFTextStringDriver::ToBytesArray(const Arguments& args)
{
    HandleScope scope;
    
    PDFTextStringDriver* element = ObjectWrap::Unwrap<PDFTextStringDriver>(args.This());
    
	std::string aString = element->mTextString.ToString();

	Local<Array> result = Array::New(aString.length());

	for(std::string::size_type i=0;i<aString.length();++i)
		result->Set(v8::Number::New(i),v8::Number::New(aString[i]));

	return scope.Close(result);
}

Handle<Value> PDFTextStringDriver::ToString(const Arguments& args)
{
    HandleScope scope;
    
    PDFTextStringDriver* element = ObjectWrap::Unwrap<PDFTextStringDriver>(args.This());
    
    return scope.Close(String::New(element->mTextString.ToUTF8String().c_str()));
}

Handle<Value> PDFTextStringDriver::FromString(const Arguments& args)
{
    HandleScope scope;
    
    PDFTextStringDriver* element = ObjectWrap::Unwrap<PDFTextStringDriver>(args.This());
    if(args.Length() > 0 && args[0]->IsString())
        element->mTextString.FromUTF8(*String::Utf8Value(args[0]->ToString()));
    
    return scope.Close(args.This());
}