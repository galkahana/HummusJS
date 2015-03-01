/*
 Source File : PDFStreamDriver.h
 
 
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
#include "PDFStreamDriver.h"
#include "PDFStream.h"
#include "ByteWriterDriver.h"

using namespace v8;

Persistent<Function> PDFStreamDriver::constructor;
Persistent<FunctionTemplate> PDFStreamDriver::constructor_template;

PDFStreamDriver::PDFStreamDriver()
{
    PDFStreamInstance = NULL;
}

void PDFStreamDriver::Init()
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PDFStream"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "getWriteStream", GetWriteStream);
	SET_CONSTRUCTOR(constructor,t);
	SET_CONSTRUCTOR_TEMPLATE(constructor_template,t);
}

METHOD_RETURN_TYPE PDFStreamDriver::NewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
	
	Local<Object> instance = NEW_INSTANCE(constructor);
	SET_FUNCTION_RETURN_VALUE(instance);
}

v8::Handle<v8::Value> PDFStreamDriver::GetNewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

bool PDFStreamDriver::HasInstance(Handle<Value> inObject)
{
	CREATE_ISOLATE_CONTEXT;
	
	return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);
}

METHOD_RETURN_TYPE PDFStreamDriver::New(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    PDFStreamDriver* driver = new PDFStreamDriver();
    driver->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This());
}

METHOD_RETURN_TYPE PDFStreamDriver::GetWriteStream(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    Handle<Value> result = ByteWriterDriver::GetNewInstance(args);
    
    ObjectWrap::Unwrap<ByteWriterDriver>(result->ToObject())->SetStream(
                                                                        ObjectWrap::Unwrap<PDFStreamDriver>(args.This())->PDFStreamInstance->GetWriteStream(), false);
    
	SET_FUNCTION_RETURN_VALUE(result);
}
