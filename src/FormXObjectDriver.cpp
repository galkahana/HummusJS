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
#include "ConstructorsHolder.h"

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

DEF_SUBORDINATE_INIT(FormXObjectDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE_EXTERNAL(New);

	t->SetClassName(NEW_STRING("FormXObject"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_ACCESSOR_METHOD(t,"id", GetID);
	SET_PROTOTYPE_METHOD(t, "getContentContext", GetContentContext);
    SET_PROTOTYPE_METHOD(t, "getResourcesDictinary", GetResourcesDictionary);
	SET_PROTOTYPE_METHOD(t, "getResourcesDictionary", GetResourcesDictionary);
	SET_PROTOTYPE_METHOD(t, "getContentStream", GetContentStream);



    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->FormXObject_constructor, t);  
	SET_CONSTRUCTOR_TEMPLATE(holder->FormXObject_constructor_template, t);
}

METHOD_RETURN_TYPE FormXObjectDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, externalHolder)
    
    FormXObjectDriver* form = new FormXObjectDriver();
    form->holder = externalHolder;
    form->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE FormXObjectDriver::GetID(Local<String> property, const PROPERTY_TYPE &info)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    FormXObjectDriver* form = ObjectWrap::Unwrap<FormXObjectDriver>(info.Holder());
    
    if(!form->FormXObject)
    {
		THROW_EXCEPTION("form object not initialized, create using pdfWriter.CreateFormXObject");
		SET_ACCESSOR_RETURN_VALUE(UNDEFINED)
    }
    
	SET_ACCESSOR_RETURN_VALUE(NEW_NUMBER(form->FormXObject->GetObjectID()))
}

METHOD_RETURN_TYPE FormXObjectDriver::GetContentContext(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    FormXObjectDriver* formDriver = ObjectWrap::Unwrap<FormXObjectDriver>(args.This());
  
    Local<Value> newInstance = formDriver->holder->GetNewXObjectContentContext(args);
    XObjectContentContextDriver* contentContextDriver = ObjectWrap::Unwrap<XObjectContentContextDriver>(newInstance->TO_OBJECT());
    contentContextDriver->ContentContext = formDriver->FormXObject->GetContentContext();
    contentContextDriver->FormOfContext = formDriver->FormXObject;
    contentContextDriver->SetResourcesDictionary(&(formDriver->FormXObject->GetResourcesDictionary()));
    
    SET_FUNCTION_RETURN_VALUE(newInstance)
}

METHOD_RETURN_TYPE FormXObjectDriver::GetResourcesDictionary(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    FormXObjectDriver* formDriver = ObjectWrap::Unwrap<FormXObjectDriver>(args.This());
    
    Local<Value> newInstance = formDriver->holder->GetNewResourcesDictionary(args);
    ResourcesDictionaryDriver* resourceDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(newInstance->TO_OBJECT());
    resourceDictionaryDriver->ResourcesDictionaryInstance = &(formDriver->FormXObject->GetResourcesDictionary());
    
    SET_FUNCTION_RETURN_VALUE(newInstance)
}


METHOD_RETURN_TYPE FormXObjectDriver::GetContentStream(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    FormXObjectDriver* formDriver = ObjectWrap::Unwrap<FormXObjectDriver>(args.This());
    
    Local<Value> newInstance = formDriver->holder->GetNewPDFStream(args);
    PDFStreamDriver* streamDriver = ObjectWrap::Unwrap<PDFStreamDriver>(newInstance->TO_OBJECT());
    streamDriver->PDFStreamInstance = formDriver->FormXObject->GetContentStream();
    
    SET_FUNCTION_RETURN_VALUE(newInstance)
}
