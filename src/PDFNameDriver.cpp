/*
 Source File : PDFNameDriver.cpp
 
 
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
#include "PDFNameDriver.h"
#include "RefCountPtr.h"
#include "ConstructorsHolder.h"
using namespace v8;



DEF_SUBORDINATE_INIT(PDFNameDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE_EXTERNAL(New);

	t->SetClassName(NEW_STRING("PDFName"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_ACCESSOR_METHOD(t, "value", GetValue);
	PDFObjectDriver::Init(t);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->PDFName_constructor, t);   
	SET_CONSTRUCTOR_TEMPLATE(holder->PDFName_constructor_template, t);
}

METHOD_RETURN_TYPE PDFNameDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
	EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, externalHolder)
    
    PDFNameDriver* driver = new PDFNameDriver();
	driver->holder = externalHolder;
    driver->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This())
}

PDFObject* PDFNameDriver::GetObject()
{
    return TheObject.GetPtr();
}

METHOD_RETURN_TYPE PDFNameDriver::GetValue(Local<String> property, const PROPERTY_TYPE &info)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    Local<String> result = NEW_STRING(ObjectWrap::Unwrap<PDFNameDriver>(info.Holder())->TheObject->GetValue().c_str());
    SET_ACCESSOR_RETURN_VALUE(result)
}


