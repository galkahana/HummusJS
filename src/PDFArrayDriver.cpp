/*
 Source File : PDFArrayDriver.h
 
 
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
#include "PDFArrayDriver.h"
#include "RefCountPtr.h"

using namespace v8;

Persistent<Function> PDFArrayDriver::constructor;
Persistent<FunctionTemplate> PDFArrayDriver::constructor_template;

void PDFArrayDriver::Init()
{
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("PDFArray"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("toJSArray"),FunctionTemplate::New(ToJSArray)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("queryObject"),FunctionTemplate::New(QueryObject)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("getLength"),FunctionTemplate::New(GetLength)->GetFunction());

    PDFObjectDriver::Init(constructor_template);
    
    constructor = Persistent<Function>::New(constructor_template->GetFunction());
}

Handle<Value> PDFArrayDriver::NewInstance()
{
    HandleScope scope;
    
    Local<Object> instance = constructor->NewInstance();
    
    return scope.Close(instance);
}

bool PDFArrayDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}

Handle<Value> PDFArrayDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    PDFArrayDriver* array = new PDFArrayDriver();
    array->Wrap(args.This());
    return args.This();
}

PDFObject* PDFArrayDriver::GetObject()
{
    return TheObject.GetPtr();
}

Handle<Value> PDFArrayDriver::ToJSArray(const Arguments& args)
{
    HandleScope scope;
    
    PDFArrayDriver* arrayDriver = ObjectWrap::Unwrap<PDFArrayDriver>(args.This());
    
    Local<Array> result = Array::New((int)arrayDriver->TheObject->GetLength());
    
    for(unsigned long i=0; i < arrayDriver->TheObject->GetLength();++i)
    {
        RefCountPtr<PDFObject> anObject(arrayDriver->TheObject->QueryObject(i));
        result->Set(Number::New(i),PDFObjectDriver::CreateDriver(anObject.GetPtr()));
    }

    return scope.Close(result);
}

Handle<Value> PDFArrayDriver::GetLength(const Arguments& args)
{
    HandleScope scope;
    
    PDFArrayDriver* arrayDriver = ObjectWrap::Unwrap<PDFArrayDriver>(args.This());
    
    Local<Number> result = Number::New(arrayDriver->TheObject->GetLength());
    
    return scope.Close(result);
}

Handle<Value> PDFArrayDriver::QueryObject(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument which is an index in the array")));
		return scope.Close(Undefined());
        
    }
    
    PDFArrayDriver* arrayDriver = ObjectWrap::Unwrap<PDFArrayDriver>(args.This());
    if(args[0]->ToNumber()->Uint32Value() >= arrayDriver->TheObject->GetLength())
    {
		ThrowException(Exception::Error(String::New("wrong arguments, pass 1 argument which is a valid index in the array")));
		return scope.Close(Undefined());
    }
    
    RefCountPtr<PDFObject> anObject = arrayDriver->TheObject->QueryObject(args[0]->ToNumber()->Uint32Value());
    Handle<Value> result = PDFObjectDriver::CreateDriver(anObject.GetPtr());
    
    return scope.Close(result);
}