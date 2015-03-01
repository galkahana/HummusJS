/*
 Source File : PDFIndirectObjectReferenceDriver.h
 
 
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
#include "PDFIndirectObjectReferenceDriver.h"
#include "RefCountPtr.h"

using namespace v8;

Persistent<Function> PDFIndirectObjectReferenceDriver::constructor;
Persistent<FunctionTemplate> PDFIndirectObjectReferenceDriver::constructor_template;

void PDFIndirectObjectReferenceDriver::Init()
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PDFIndirectObjectReference"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "getObjectID", GetObjectID);
	SET_PROTOTYPE_METHOD(t, "getVersion", GetVersion);
	PDFObjectDriver::Init(t);
	SET_CONSTRUCTOR(constructor, t);
	SET_CONSTRUCTOR_TEMPLATE(constructor_template, t);
}

METHOD_RETURN_TYPE PDFIndirectObjectReferenceDriver::NewInstance(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
	Local<Object> instance = NEW_INSTANCE(constructor);
    
    SET_FUNCTION_RETURN_VALUE(instance);
}

v8::Handle<v8::Value> PDFIndirectObjectReferenceDriver::GetNewInstance()
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

bool PDFIndirectObjectReferenceDriver::HasInstance(Handle<Value> inObject)
{
	CREATE_ISOLATE_CONTEXT;

	return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);
}

METHOD_RETURN_TYPE PDFIndirectObjectReferenceDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFIndirectObjectReferenceDriver* driver = new PDFIndirectObjectReferenceDriver();
    driver->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE( args.This());
}

PDFObject* PDFIndirectObjectReferenceDriver::GetObject()
{
    return TheObject.GetPtr();
}

METHOD_RETURN_TYPE PDFIndirectObjectReferenceDriver::GetObjectID(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    Local<Number> result = NEW_NUMBER(ObjectWrap::Unwrap<PDFIndirectObjectReferenceDriver>(args.This())->TheObject->mObjectID);
    SET_FUNCTION_RETURN_VALUE(result);
}

METHOD_RETURN_TYPE PDFIndirectObjectReferenceDriver::GetVersion(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    Local<Number> result = NEW_NUMBER(ObjectWrap::Unwrap<PDFIndirectObjectReferenceDriver>(args.This())->TheObject->mVersion);
    SET_FUNCTION_RETURN_VALUE(result);
}
