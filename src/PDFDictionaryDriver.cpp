/*
 Source File : PDFDictionaryDriver.cpp
 
 
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
#include "PDFDictionaryDriver.h"
#include "RefCountPtr.h"
#include <string>

using namespace v8;

Persistent<Function> PDFDictionaryDriver::constructor;
Persistent<FunctionTemplate> PDFDictionaryDriver::constructor_template;

void PDFDictionaryDriver::Init()
{
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("PDFDictionary"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("toJSObject"),FunctionTemplate::New(ToJSObject)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("queryObject"),FunctionTemplate::New(QueryObject)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("exists"),FunctionTemplate::New(Exists)->GetFunction());

    PDFObjectDriver::Init(constructor_template);

    constructor = Persistent<Function>::New(constructor_template->GetFunction());
}

Handle<Value> PDFDictionaryDriver::NewInstance()
{
    HandleScope scope;
    
    const unsigned argc = 0;
    Local<Object> instance = constructor->NewInstance(argc, NULL);
    
    return scope.Close(instance);
}

bool PDFDictionaryDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}

Handle<Value> PDFDictionaryDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    PDFDictionaryDriver* driver = new PDFDictionaryDriver();
    driver->Wrap(args.This());
    return args.This();
}

PDFObject* PDFDictionaryDriver::GetObject()
{
    return TheObject.GetPtr();
}

Handle<Value> PDFDictionaryDriver::ToJSObject(const Arguments& args)
{
    HandleScope scope;
    
    PDFDictionaryDriver* driver = ObjectWrap::Unwrap<PDFDictionaryDriver>(args.This());
    
    Local<Object> result = Object::New();
    
	MapIterator<PDFNameToPDFObjectMap> it = driver->TheObject->GetIterator();
    
    while(it.MoveNext())
        result->Set(String::New(it.GetKey()->GetValue().c_str()),PDFObjectDriver::CreateDriver(it.GetValue()));
    
    return scope.Close(result);
}

Handle<Value> PDFDictionaryDriver::Exists(const Arguments& args)
{
    HandleScope scope;
    
    PDFDictionaryDriver* driver = ObjectWrap::Unwrap<PDFDictionaryDriver>(args.This());

    if(args.Length() != 1 || !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument which is a string key")));
		return scope.Close(Undefined());
        
    }
    
    Handle<Boolean> result = Boolean::New(driver->TheObject->Exists(*String::Utf8Value(args[0]->ToString())));
    
    return scope.Close(result);
}

Handle<Value> PDFDictionaryDriver::QueryObject(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 || !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument which is a string key")));
		return scope.Close(Undefined());
        
    }
    
    std::string key = *String::Utf8Value(args[0]->ToString());

    PDFDictionaryDriver* driver = ObjectWrap::Unwrap<PDFDictionaryDriver>(args.This());
    
    if(!driver->TheObject->Exists(key))
    {
		ThrowException(Exception::Error(String::New("key not found")));
		return scope.Close(Undefined());
    }
    
    RefCountPtr<PDFObject> anObject = driver->TheObject->QueryDirectObject(key);
    Handle<Value> result = PDFObjectDriver::CreateDriver(anObject.GetPtr());
    
    return scope.Close(result);
}