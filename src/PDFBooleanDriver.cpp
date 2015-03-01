/*
 Source File : PDFBooleanDriver.h
 
 
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
#include "PDFBooleanDriver.h"
#include "RefCountPtr.h"

using namespace v8;

Persistent<Function> PDFBooleanDriver::constructor;
Persistent<FunctionTemplate> PDFBooleanDriver::constructor_template;

void PDFBooleanDriver::Init()
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PDFBoolean"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_ACCESSOR_METHOD(t, "value", GetValue);
	PDFObjectDriver::Init(t);
	SET_CONSTRUCTOR(constructor, t);
	SET_CONSTRUCTOR_TEMPLATE(constructor_template, t);
}

METHOD_RETURN_TYPE PDFBooleanDriver::NewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	SET_FUNCTION_RETURN_VALUE(instance);
}

v8::Handle<v8::Value> PDFBooleanDriver::GetNewInstance()
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

bool PDFBooleanDriver::HasInstance(Handle<Value> inObject)
{
	CREATE_ISOLATE_CONTEXT;

	return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);
}

METHOD_RETURN_TYPE PDFBooleanDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFBooleanDriver* driver = new PDFBooleanDriver();
    driver->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This());
}

PDFObject* PDFBooleanDriver::GetObject()
{
    return TheObject.GetPtr();
}

METHOD_RETURN_TYPE PDFBooleanDriver::GetValue(Local<String> property, const PROPERTY_TYPE &info)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    Handle<Boolean> result = NEW_BOOLEAN(ObjectWrap::Unwrap<PDFBooleanDriver>(info.Holder())->TheObject->GetValue());

    
    SET_ACCESSOR_RETURN_VALUE(result);
}


