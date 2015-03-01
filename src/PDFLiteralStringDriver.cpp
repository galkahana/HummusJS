/*
 Source File : PDFLiteralStringDriver.h
 
 
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
#include "PDFLiteralStringDriver.h"
#include "RefCountPtr.h"
#include "PDFTextString.h"

using namespace v8;

Persistent<Function> PDFLiteralStringDriver::constructor;
Persistent<FunctionTemplate> PDFLiteralStringDriver::constructor_template;

void PDFLiteralStringDriver::Init()
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PDFLiteralString"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "toText", ToText);
	SET_ACCESSOR_METHOD(t, "value", GetValue);
	PDFObjectDriver::Init(t);
	SET_CONSTRUCTOR(constructor, t);
	SET_CONSTRUCTOR_TEMPLATE(constructor_template, t);
}

METHOD_RETURN_TYPE PDFLiteralStringDriver::NewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	SET_FUNCTION_RETURN_VALUE(instance);
}

v8::Handle<v8::Value> PDFLiteralStringDriver::GetNewInstance()
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

bool PDFLiteralStringDriver::HasInstance(Handle<Value> inObject)
{
	CREATE_ISOLATE_CONTEXT;

	return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);
}

METHOD_RETURN_TYPE PDFLiteralStringDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFLiteralStringDriver* driver = new PDFLiteralStringDriver();
    driver->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This());
}

PDFObject* PDFLiteralStringDriver::GetObject()
{
    return TheObject.GetPtr();
}

METHOD_RETURN_TYPE PDFLiteralStringDriver::GetValue(Local<String> property, const PROPERTY_TYPE &info)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    
    Handle<String> result = NEW_STRING(ObjectWrap::Unwrap<PDFLiteralStringDriver>(info.Holder())->TheObject->GetValue().c_str());
    SET_ACCESSOR_RETURN_VALUE(result);
}


METHOD_RETURN_TYPE PDFLiteralStringDriver::ToText(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    PDFLiteralStringDriver* driver = ObjectWrap::Unwrap<PDFLiteralStringDriver>(args.This());

    
    Handle<String> result = NEW_STRING(PDFTextString(driver->TheObject->GetValue()).ToUTF8String().c_str());
    SET_FUNCTION_RETURN_VALUE(result);
}



