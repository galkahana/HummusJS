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
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PDFArray"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "toJSArray", ToJSArray);
	SET_PROTOTYPE_METHOD(t, "queryObject", QueryObject);
	SET_PROTOTYPE_METHOD(t, "getLength", GetLength);
	PDFObjectDriver::Init(t);
	SET_CONSTRUCTOR(constructor, t);
	SET_CONSTRUCTOR_TEMPLATE(constructor_template, t);


}

METHOD_RETURN_TYPE PDFArrayDriver::NewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	SET_FUNCTION_RETURN_VALUE(instance);
}

v8::Handle<v8::Value> PDFArrayDriver::GetNewInstance()
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

bool PDFArrayDriver::HasInstance(Handle<Value> inObject)
{
	CREATE_ISOLATE_CONTEXT;

	return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);
}

METHOD_RETURN_TYPE PDFArrayDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFArrayDriver* array = new PDFArrayDriver();
    array->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This());
}

PDFObject* PDFArrayDriver::GetObject()
{
    return TheObject.GetPtr();
}

METHOD_RETURN_TYPE PDFArrayDriver::ToJSArray(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFArrayDriver* arrayDriver = ObjectWrap::Unwrap<PDFArrayDriver>(args.This());
    
    Local<Array> result = NEW_ARRAY((int)arrayDriver->TheObject->GetLength());
    
    for(unsigned long i=0; i < arrayDriver->TheObject->GetLength();++i)
    {
        RefCountPtr<PDFObject> anObject(arrayDriver->TheObject->QueryObject(i));
        result->Set(NEW_NUMBER(i),PDFObjectDriver::CreateDriver(anObject.GetPtr()));
    }

    SET_FUNCTION_RETURN_VALUE(result);
}

METHOD_RETURN_TYPE PDFArrayDriver::GetLength(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFArrayDriver* arrayDriver = ObjectWrap::Unwrap<PDFArrayDriver>(args.This());
    
    Local<Number> result = NEW_NUMBER(arrayDriver->TheObject->GetLength());
    
    SET_FUNCTION_RETURN_VALUE(result);
}

METHOD_RETURN_TYPE PDFArrayDriver::QueryObject(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument which is an index in the array");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
        
    }
    
    PDFArrayDriver* arrayDriver = ObjectWrap::Unwrap<PDFArrayDriver>(args.This());
    if(args[0]->ToNumber()->Uint32Value() >= arrayDriver->TheObject->GetLength())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument which is a valid index in the array");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    RefCountPtr<PDFObject> anObject = arrayDriver->TheObject->QueryObject(args[0]->ToNumber()->Uint32Value());
    Handle<Value> result = PDFObjectDriver::CreateDriver(anObject.GetPtr());
    
    SET_FUNCTION_RETURN_VALUE(result);
}