/*
 Source File : FormXObjectDriver.cpp
 
 
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
#include "FormXObjectDriver.h"
#include "PDFFormXObject.h"
#include "XObjectContentContextDriver.h"
#include "ResourcesDictionaryDriver.h"
#include "PDFStreamDriver.h"

using namespace v8;

FormXObjectDriver::~FormXObjectDriver()
{
    delete FormXObject;
}

FormXObjectDriver::FormXObjectDriver()
{
    mPDFWriterDriver = NULL;
    FormXObject = NULL;
}

void FormXObjectDriver::Init()
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("FormXObject"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_ACCESSOR_METHOD(t,"id", GetID);
	SET_PROTOTYPE_METHOD(t, "getContentContext", GetContentContext);
	SET_PROTOTYPE_METHOD(t, "getResourcesDictinary", GetResourcesDictionary);
	SET_PROTOTYPE_METHOD(t, "getContentStream", GetContentStream);

	SET_CONSTRUCTOR(constructor, t);
	SET_CONSTRUCTOR_TEMPLATE(constructor_template, t);
}

METHOD_RETURN_TYPE FormXObjectDriver::NewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	SET_FUNCTION_RETURN_VALUE(instance);
}

v8::Handle<v8::Value> FormXObjectDriver::GetNewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

bool FormXObjectDriver::HasInstance(Handle<Value> inObject)
{
	CREATE_ISOLATE_CONTEXT;

	return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);

}

Persistent<Function> FormXObjectDriver::constructor;
Persistent<FunctionTemplate> FormXObjectDriver::constructor_template;

METHOD_RETURN_TYPE FormXObjectDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    FormXObjectDriver* form = new FormXObjectDriver();
    form->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This());
}

METHOD_RETURN_TYPE FormXObjectDriver::GetID(Local<String> property, const PROPERTY_TYPE &info)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    FormXObjectDriver* form = ObjectWrap::Unwrap<FormXObjectDriver>(info.Holder());
    
    if(!form->FormXObject)
    {
		THROW_EXCEPTION("form object not initialized, create using pdfWriter.CreateFormXObject");
		SET_ACCESSOR_RETURN_VALUE(UNDEFINED);
    }
    
	SET_ACCESSOR_RETURN_VALUE(NEW_NUMBER(form->FormXObject->GetObjectID()));
}

METHOD_RETURN_TYPE FormXObjectDriver::GetContentContext(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    FormXObjectDriver* formDriver = ObjectWrap::Unwrap<FormXObjectDriver>(args.This());
  
    Handle<Value> newInstance = XObjectContentContextDriver::GetNewInstance(args);
    XObjectContentContextDriver* contentContextDriver = ObjectWrap::Unwrap<XObjectContentContextDriver>(newInstance->ToObject());
    contentContextDriver->ContentContext = formDriver->FormXObject->GetContentContext();
    contentContextDriver->FormOfContext = formDriver->FormXObject;
    contentContextDriver->SetResourcesDictionary(&(formDriver->FormXObject->GetResourcesDictionary()));
    contentContextDriver->SetPDFWriter(formDriver->mPDFWriterDriver);
    
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE FormXObjectDriver::GetResourcesDictionary(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    FormXObjectDriver* formDriver = ObjectWrap::Unwrap<FormXObjectDriver>(args.This());
    
    Handle<Value> newInstance = ResourcesDictionaryDriver::GetNewInstance(args);
    ResourcesDictionaryDriver* resourceDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(newInstance->ToObject());
    resourceDictionaryDriver->ResourcesDictionaryInstance = &(formDriver->FormXObject->GetResourcesDictionary());
    
    SET_FUNCTION_RETURN_VALUE(newInstance);
}


void FormXObjectDriver::SetPDFWriter(PDFWriterDriver* inPDFWriterDriver)
{
    mPDFWriterDriver = inPDFWriterDriver;
}

METHOD_RETURN_TYPE FormXObjectDriver::GetContentStream(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    FormXObjectDriver* formDriver = ObjectWrap::Unwrap<FormXObjectDriver>(args.This());
    
    Handle<Value> newInstance = PDFStreamDriver::GetNewInstance(args);
    PDFStreamDriver* streamDriver = ObjectWrap::Unwrap<PDFStreamDriver>(newInstance->ToObject());
    streamDriver->PDFStreamInstance = formDriver->FormXObject->GetContentStream();
    
    SET_FUNCTION_RETURN_VALUE(newInstance);
}
