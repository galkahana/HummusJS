/*
 Source File : PDFPageInputDriver
 
 
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
#include "PDFPageInputDriver.h"
#include "PDFPageInput.h"
#include "PDFDictionaryDriver.h"

using namespace v8;

Persistent<Function> PDFPageInputDriver::constructor;

PDFPageInputDriver::PDFPageInputDriver()
{
    PageInput = NULL;
}

PDFPageInputDriver::~PDFPageInputDriver()
{
    delete PageInput;
}

void PDFPageInputDriver::Init()
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PDFPageInput"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "getDictionary", GetDictionary);
	SET_PROTOTYPE_METHOD(t, "getMediaBox", GetMediaBox);
	SET_PROTOTYPE_METHOD(t, "getCropBox", GetCropBox);
	SET_PROTOTYPE_METHOD(t, "getTrimBox", GetTrimBox);
	SET_PROTOTYPE_METHOD(t, "getBleedBox", GetBleedBox);
	SET_PROTOTYPE_METHOD(t, "getArtBox", GetArtBox);
	SET_CONSTRUCTOR(constructor, t);
}

METHOD_RETURN_TYPE PDFPageInputDriver::NewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	SET_FUNCTION_RETURN_VALUE(instance);
}

v8::Handle<v8::Value> PDFPageInputDriver::GetNewInstance()
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

METHOD_RETURN_TYPE PDFPageInputDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFPageInputDriver* element = new PDFPageInputDriver();
    element->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE( args.This());
}

METHOD_RETURN_TYPE  PDFPageInputDriver::GetDictionary(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFPageInputDriver* element = ObjectWrap::Unwrap<PDFPageInputDriver>(args.This());
    
    if(!element->PageInput)
    {
		THROW_EXCEPTION("page input not initialized. create one using the PDFReader.parsePage");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }

    Handle<Value> newInstance;

    newInstance = PDFDictionaryDriver::GetNewInstance();
    ObjectWrap::Unwrap<PDFDictionaryDriver>(newInstance->ToObject())->TheObject = element->PageInputDictionary;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFPageInputDriver::GetMediaBox(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    PDFPageInputDriver* element = ObjectWrap::Unwrap<PDFPageInputDriver>(args.This());
    
    if(!element->PageInput)
    {
		THROW_EXCEPTION("page input not initialized. create one using the PDFReader.parsePage");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
	else
		SET_FUNCTION_RETURN_VALUE(GetArrayForPDFRectangle(element->PageInput->GetMediaBox()));
}

Handle<Value> PDFPageInputDriver::GetArrayForPDFRectangle(const PDFRectangle& inRectangle)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    Local<Array> result = NEW_ARRAY(4);
    
    result->Set(NEW_NUMBER(0),NEW_NUMBER(inRectangle.LowerLeftX));
    result->Set(NEW_NUMBER(1),NEW_NUMBER(inRectangle.LowerLeftY));
    result->Set(NEW_NUMBER(2),NEW_NUMBER(inRectangle.UpperRightX));
    result->Set(NEW_NUMBER(3),NEW_NUMBER(inRectangle.UpperRightY));
    return CLOSE_SCOPE(result);
    
}
METHOD_RETURN_TYPE PDFPageInputDriver::GetCropBox(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	PDFPageInputDriver* element = ObjectWrap::Unwrap<PDFPageInputDriver>(args.This());

	if (!element->PageInput)
	{
		THROW_EXCEPTION("page input not initialized. create one using the PDFReader.parsePage");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
	}
	else
		SET_FUNCTION_RETURN_VALUE(GetArrayForPDFRectangle(element->PageInput->GetCropBox()));
}

METHOD_RETURN_TYPE PDFPageInputDriver::GetTrimBox(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	PDFPageInputDriver* element = ObjectWrap::Unwrap<PDFPageInputDriver>(args.This());

	if (!element->PageInput)
	{
		THROW_EXCEPTION("page input not initialized. create one using the PDFReader.parsePage");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
	}
	else
		SET_FUNCTION_RETURN_VALUE(GetArrayForPDFRectangle(element->PageInput->GetTrimBox()));
}

METHOD_RETURN_TYPE PDFPageInputDriver::GetBleedBox(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	PDFPageInputDriver* element = ObjectWrap::Unwrap<PDFPageInputDriver>(args.This());

	if (!element->PageInput)
	{
		THROW_EXCEPTION("page input not initialized. create one using the PDFReader.parsePage");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
	}
	else
		SET_FUNCTION_RETURN_VALUE(GetArrayForPDFRectangle(element->PageInput->GetBleedBox()));
}

METHOD_RETURN_TYPE PDFPageInputDriver::GetArtBox(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	PDFPageInputDriver* element = ObjectWrap::Unwrap<PDFPageInputDriver>(args.This());

	if (!element->PageInput)
	{
		THROW_EXCEPTION("page input not initialized. create one using the PDFReader.parsePage");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
	}
	else
		SET_FUNCTION_RETURN_VALUE(GetArrayForPDFRectangle(element->PageInput->GetArtBox()));
}
