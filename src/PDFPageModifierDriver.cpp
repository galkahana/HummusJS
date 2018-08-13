/*
 Source File : InputFileDriver.h
 
 
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
#include "EStatusCode.h"
#include "PDFPageModifierDriver.h"
#include "PDFModifiedPage.h"
#include "PDFWriter.h"
#include "PDFWriterDriver.h"
#include "XObjectContentContextDriver.h"
#include "PDFFormXObject.h"


using namespace v8;
using namespace PDFHummus;

Persistent<Function> PDFPageModifierDriver::constructor;
Persistent<FunctionTemplate> PDFPageModifierDriver::constructor_template;

PDFPageModifierDriver::PDFPageModifierDriver(PDFWriter* inWriter,unsigned long inPageIndex,bool inEnsureContentEncapsulation)
{
    mModifierPageInstance = new PDFModifiedPage(inWriter,inPageIndex,inEnsureContentEncapsulation);
}

PDFPageModifierDriver::~PDFPageModifierDriver()
{
    delete mModifierPageInstance;
}

void PDFPageModifierDriver::Init(Handle<Object> inExports)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PDFPageModifier"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "startContext", StartContext);
	SET_PROTOTYPE_METHOD(t, "getContext", GetContext);
	SET_PROTOTYPE_METHOD(t, "endContext", EndContext);
	SET_PROTOTYPE_METHOD(t, "attachURLLinktoCurrentPage", AttachURLLinktoCurrentPage);
	SET_PROTOTYPE_METHOD(t, "writePage", WritePage);
	SET_CONSTRUCTOR(constructor, t);
	SET_CONSTRUCTOR_TEMPLATE(constructor_template, t);
	
	SET_CONSTRUCTOR_EXPORT(inExports, "PDFPageModifier", t);
}

METHOD_RETURN_TYPE PDFPageModifierDriver::NewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	SET_FUNCTION_RETURN_VALUE(instance)
}

v8::Handle<v8::Value> PDFPageModifierDriver::GetNewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

bool PDFPageModifierDriver::HasInstance(Handle<Value> inObject)
{
	CREATE_ISOLATE_CONTEXT;

	return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);
}

METHOD_RETURN_TYPE PDFPageModifierDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFWriter* writer = NULL;
    unsigned long pageIndex = 0;
    bool ensureContentEncapsulation = false;

    if(
        args.Length() < 1 || 
        !args[0]->IsObject() ||
        (args.Length() >= 2 && !args[1]->IsNumber()) ||
        (args.Length() >= 3 && !args[2]->IsBoolean())
    ) {
        THROW_EXCEPTION("Wrong arguments, perovide a: PDFWriter of the pdf that page you want modified, page index of that page, and boolean indicating if you want to ensure encapsulation. last one defaults to false");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    PDFWriterDriver* writerDriver = ObjectWrap::Unwrap<PDFWriterDriver>(args[0]->ToObject());
    if(!writerDriver)
    {
		THROW_EXCEPTION("Wrong arguments, provide a PDFWriter as the first object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    else
        writer = writerDriver->GetWriter();

    if(args.Length() >= 2)
    {
        pageIndex = (unsigned long)TO_NUMBER(args[1])->Value();
    }

    if(args.Length() >= 3)
    {
        ensureContentEncapsulation = args[2]->ToBoolean()->Value();
    }


    PDFPageModifierDriver* driver = new PDFPageModifierDriver(writer,pageIndex,ensureContentEncapsulation);
    
    driver->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE PDFPageModifierDriver::StartContext(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
 
    PDFPageModifierDriver* driver = ObjectWrap::Unwrap<PDFPageModifierDriver>(args.This());

    do {
        if(!driver)
        {
            THROW_EXCEPTION("no driver created...please create one through Hummus");
            SET_FUNCTION_RETURN_VALUE(UNDEFINED)
            break;       
        }

        if(!driver->mModifierPageInstance->StartContentContext()) {
            THROW_EXCEPTION("context not created, page index is either wrong, or page is null");
            SET_FUNCTION_RETURN_VALUE(UNDEFINED) 
            break;       
        }
        SET_FUNCTION_RETURN_VALUE(args.This())
    }while(false);
    

}

METHOD_RETURN_TYPE PDFPageModifierDriver::GetContext(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
 
    PDFPageModifierDriver* driver = ObjectWrap::Unwrap<PDFPageModifierDriver>(args.This());

    
    do {
        if(!driver)
        {
            THROW_EXCEPTION("no driver created...please create one through Hummus");
            SET_FUNCTION_RETURN_VALUE(UNDEFINED)
            break;
        }

        if(!driver->mModifierPageInstance->GetCurrentFormContext()) {
            THROW_EXCEPTION("No context created, please create one with startContext");
            SET_FUNCTION_RETURN_VALUE(UNDEFINED)        
            break;

        }

        Handle<Value> newInstance = XObjectContentContextDriver::GetNewInstance(args);
        XObjectContentContextDriver* contentContextDriver = ObjectWrap::Unwrap<XObjectContentContextDriver>(newInstance->ToObject());
        contentContextDriver->ContentContext = 
            driver->mModifierPageInstance->GetCurrentFormContext() ? 
                driver->mModifierPageInstance->GetCurrentFormContext()->GetContentContext():
                NULL;
        contentContextDriver->FormOfContext = driver->mModifierPageInstance->GetCurrentFormContext();
        contentContextDriver->SetResourcesDictionary(driver->mModifierPageInstance->GetCurrentResourcesDictionary());

        SET_FUNCTION_RETURN_VALUE(newInstance)
    }while(false);
}

METHOD_RETURN_TYPE PDFPageModifierDriver::EndContext(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
 
    PDFPageModifierDriver* driver = ObjectWrap::Unwrap<PDFPageModifierDriver>(args.This());

    
    if(!driver)
    {
		THROW_EXCEPTION("no driver created...please create one through Hummus");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }

    driver->mModifierPageInstance->EndContentContext();    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE PDFPageModifierDriver::AttachURLLinktoCurrentPage(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 5 ||
        !args[0]->IsString() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber() ||
        !args[3]->IsNumber() ||
        !args[4]->IsNumber())
    {
		THROW_EXCEPTION("wrong arguments, pass a url, and 4 numbers (left,bottom,right,top) for the rectangle valid for clicking");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    PDFPageModifierDriver* driver = ObjectWrap::Unwrap<PDFPageModifierDriver>(args.This());
    
    EStatusCode status = driver->mModifierPageInstance->AttachURLLinktoCurrentPage(*String::Utf8Value(args[0]->ToString()),
                                                                             PDFRectangle(TO_NUMBER(args[1])->Value(),
                                                                             TO_NUMBER(args[2])->Value(),
                                                                             TO_NUMBER(args[3])->Value(),
                                                                             TO_NUMBER(args[4])->Value()));
    if(status != eSuccess)
    {
		THROW_EXCEPTION("unable to attach link to current page. will happen if the input URL may not be encoded to ascii7");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE PDFPageModifierDriver::WritePage(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
 
    PDFPageModifierDriver* driver = ObjectWrap::Unwrap<PDFPageModifierDriver>(args.This());

    
    if(!driver)
    {
		THROW_EXCEPTION("no driver created...please create one through Hummus");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }

    if(driver->mModifierPageInstance->WritePage() != PDFHummus::eSuccess)
    {
		THROW_EXCEPTION("Unable to write page");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }    
      
    SET_FUNCTION_RETURN_VALUE(args.This())
}