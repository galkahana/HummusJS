/*
 Source File : PDFStreamInputDriver.cpp
 
 
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
#include "PDFStreamInputDriver.h"
#include "RefCountPtr.h"
#include "PDFDictionary.h"
#include <string>

using namespace v8;

Persistent<Function> PDFStreamInputDriver::constructor;
Persistent<FunctionTemplate> PDFStreamInputDriver::constructor_template;

void PDFStreamInputDriver::Init()
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PDFStreamInput"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "getDictionary", GetDictionary);
	SET_PROTOTYPE_METHOD(t, "getStreamContentStart", GetStreamContentStart);
	PDFObjectDriver::Init(t);
	SET_CONSTRUCTOR(constructor, t);
	SET_CONSTRUCTOR_TEMPLATE(constructor_template, t);
}

METHOD_RETURN_TYPE PDFStreamInputDriver::NewInstance(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
	Local<Object> instance = NEW_INSTANCE(constructor);
    
    SET_FUNCTION_RETURN_VALUE(instance);
}

v8::Handle<v8::Value> PDFStreamInputDriver::GetNewInstance()
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

bool PDFStreamInputDriver::HasInstance(Handle<Value> inObject)
{
	CREATE_ISOLATE_CONTEXT;

	return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);
}

METHOD_RETURN_TYPE PDFStreamInputDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    PDFStreamInputDriver* driver = new PDFStreamInputDriver();

    driver->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This());
}

PDFObject* PDFStreamInputDriver::GetObject()
{
    return TheObject.GetPtr();
}

METHOD_RETURN_TYPE PDFStreamInputDriver::GetDictionary(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    PDFStreamInputDriver* driver = ObjectWrap::Unwrap<PDFStreamInputDriver>(args.This());
    RefCountPtr<PDFDictionary> streamDict = driver->TheObject->QueryStreamDictionary();
    Handle<Value> result = PDFObjectDriver::CreateDriver(streamDict.GetPtr());

    SET_FUNCTION_RETURN_VALUE(result);
}

METHOD_RETURN_TYPE PDFStreamInputDriver::GetStreamContentStart(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    PDFStreamInputDriver* driver = ObjectWrap::Unwrap<PDFStreamInputDriver>(args.This());
    Handle<Number> result = NEW_NUMBER(driver->TheObject->GetStreamContentStart());
    
    SET_FUNCTION_RETURN_VALUE(result);
}
