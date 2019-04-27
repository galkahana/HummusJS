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
#include "ConstructorsHolder.h"

using namespace v8;



DEF_SUBORDINATE_INIT(PDFArrayDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE_EXTERNAL(New);

	t->SetClassName(NEW_STRING("PDFArray"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "toJSArray", ToJSArray);
	SET_PROTOTYPE_METHOD(t, "queryObject", QueryObject);
	SET_PROTOTYPE_METHOD(t, "getLength", GetLength);
	PDFObjectDriver::Init(t);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->PDFArray_constructor, t);        
	SET_CONSTRUCTOR_TEMPLATE(holder->PDFArray_constructor_template, t);
}

METHOD_RETURN_TYPE PDFArrayDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, externalHolder)

    PDFArrayDriver* array = new PDFArrayDriver();
    array->holder = externalHolder;
    array->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This())
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
        result->Set(GET_CURRENT_CONTEXT, NEW_NUMBER(i),arrayDriver->holder->GetInstanceFor(anObject.GetPtr()));
    }

    SET_FUNCTION_RETURN_VALUE(result)
}

METHOD_RETURN_TYPE PDFArrayDriver::GetLength(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFArrayDriver* arrayDriver = ObjectWrap::Unwrap<PDFArrayDriver>(args.This());
    
    Local<Number> result = NEW_NUMBER(arrayDriver->TheObject->GetLength());
    
    SET_FUNCTION_RETURN_VALUE(result)
}

METHOD_RETURN_TYPE PDFArrayDriver::QueryObject(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument which is an index in the array");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    PDFArrayDriver* arrayDriver = ObjectWrap::Unwrap<PDFArrayDriver>(args.This());
    if(TO_UINT32(args[0])->Value() >= arrayDriver->TheObject->GetLength())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument which is a valid index in the array");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    RefCountPtr<PDFObject> anObject = arrayDriver->TheObject->QueryObject(TO_UINT32(args[0])->Value());
    Local<Value> result = arrayDriver->holder->GetInstanceFor(anObject.GetPtr());
    
    SET_FUNCTION_RETURN_VALUE(result)
}