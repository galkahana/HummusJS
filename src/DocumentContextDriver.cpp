/*
 Source File : DocumentContextDriver.cpp
 
 
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
#include "DocumentContextDriver.h"
#include "DocumentContext.h"
#include "InfoDictionaryDriver.h"
#include "ConstructorsHolder.h"

using namespace v8;
using namespace PDFHummus;


DEF_SUBORDINATE_INIT(DocumentContextDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE_EXTERNAL(New);

	t->SetClassName(NEW_STRING("DocumentContext"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "getInfoDictionary", GetInfoDictionary);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->DocumentContext_constructor, t);   	
}

DocumentContextDriver::DocumentContextDriver()
{
    DocumentContextInstance = NULL;
}

METHOD_RETURN_TYPE DocumentContextDriver::New(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
	EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, externalHolder)

    DocumentContextDriver* objectsContext = new DocumentContextDriver();
	objectsContext->holder = externalHolder;
    objectsContext->Wrap(args.This());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE DocumentContextDriver::GetInfoDictionary(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    DocumentContextDriver* driver = ObjectWrap::Unwrap<DocumentContextDriver>(args.This());
    if(!driver->DocumentContextInstance)
    {
		THROW_EXCEPTION("document context driver not initialized. use the pdfwriter to get the current document context");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    Local<Value> infoDictionaryDriverObject = driver->holder->GetNewInfoDictionary(args);
    InfoDictionaryDriver* infoDictDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(infoDictionaryDriverObject->TO_OBJECT());
    infoDictDriver->InfoDictionaryInstance = &(driver->DocumentContextInstance->GetTrailerInformation().GetInfo());
    
	SET_FUNCTION_RETURN_VALUE(infoDictionaryDriverObject)
}


