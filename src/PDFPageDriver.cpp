/*
 Source File : PDFPageDriver.cpp
 
 
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
#include "PDFPageDriver.h"
#include "PDFRectangle.h"
#include "ResourcesDictionaryDriver.h"

using namespace v8;

Persistent<Function> PDFPageDriver::constructor;
Persistent<FunctionTemplate> PDFPageDriver::constructor_template;

PDFPageDriver::~PDFPageDriver()
{
    if(mOwnsPage)
        delete mPDFPage;
}

PDFPageDriver::PDFPageDriver()
{
    ContentContext = NULL;
    mPDFPage = NULL;
    mOwnsPage = false;
}

void PDFPageDriver::Init(Handle<Object> inExports)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PDFPage"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_ACCESSOR_METHODS(t, "mediaBox", GetMediaBox, SetMediaBox);
	SET_PROTOTYPE_METHOD(t, "getResourcesDictionary", GetResourcesDictionary);
	SET_CONSTRUCTOR(constructor, t);
	SET_CONSTRUCTOR_TEMPLATE(constructor_template, t);
    
	SET_CONSTRUCTOR_EXPORT(inExports, "PDFPage", t);
}


METHOD_RETURN_TYPE PDFPageDriver::NewInstance(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
    CREATE_ESCAPABLE_SCOPE;    
	SET_FUNCTION_RETURN_VALUE(GetNewInstance(args));
}


v8::Handle<v8::Value> PDFPageDriver::GetNewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Handle<Value> argv[1] = { NEW_BOOLEAN(false) };
	Local<Object> instance = NEW_INSTANCE_ARGS(constructor, 1, argv);

	// this version creates an instance of a page, and owns it. this is the one used by javascript

	PDFPageDriver* driver = ObjectWrap::Unwrap<PDFPageDriver>(instance->ToObject());
	driver->mPDFPage = new PDFPage();
	driver->mOwnsPage = true;

	if (args.Length() == 4 && args[0]->IsNumber() && args[1]->IsNumber() && args[2]->IsNumber() && args[3]->IsNumber())
	{
		driver->mPDFPage->SetMediaBox(PDFRectangle(args[0]->ToNumber()->Value(),
			args[1]->ToNumber()->Value(),
			args[2]->ToNumber()->Value(),
			args[3]->ToNumber()->Value()));
	}

	return CLOSE_SCOPE(instance);
}

v8::Handle<v8::Value> PDFPageDriver::GetNewInstance(PDFPage* inPage)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Handle<Value> argv[1] = { NEW_BOOLEAN(false) };
	Local<Object> instance = NEW_INSTANCE_ARGS(constructor, 1, argv);

	// this version links to a page and does not own it!

	PDFPageDriver* driver = ObjectWrap::Unwrap<PDFPageDriver>(instance->ToObject());
	driver->mPDFPage = inPage;
	driver->mOwnsPage = false;

	return CLOSE_SCOPE(instance);

}

bool PDFPageDriver::HasInstance(Handle<Value> inObject)
{
	CREATE_ISOLATE_CONTEXT;

	return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);
}

METHOD_RETURN_TYPE PDFPageDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFPageDriver* pdfPage = new PDFPageDriver();
    if(args.Length() != 1) // which would signify that this didn't came from one of the "newinstance" here...so from user "new"
    {
        pdfPage->mPDFPage = new PDFPage();
        pdfPage->mOwnsPage = true;
        
        if(args.Length() == 4 && args[0]->IsNumber() && args[1]->IsNumber() && args[2]->IsNumber() && args[3]->IsNumber())
        {
            pdfPage->mPDFPage->SetMediaBox(PDFRectangle(args[0]->ToNumber()->Value(),
                                                       args[1]->ToNumber()->Value(),
                                                       args[2]->ToNumber()->Value(),
                                                       args[3]->ToNumber()->Value()));
        }
        
    }
    
    pdfPage->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This());
}

METHOD_RETURN_TYPE PDFPageDriver::GetMediaBox(Local<String> property, const PROPERTY_TYPE &info)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    PDFPageDriver* pageDriver = ObjectWrap::Unwrap<PDFPageDriver>(info.Holder());
    
    Local<Array> mediaBox = NEW_ARRAY(4);
    
    mediaBox->Set(v8::NEW_NUMBER(0),v8::NEW_NUMBER(pageDriver->mPDFPage->GetMediaBox().LowerLeftX));
    mediaBox->Set(v8::NEW_NUMBER(1),v8::NEW_NUMBER(pageDriver->mPDFPage->GetMediaBox().LowerLeftY));
    mediaBox->Set(v8::NEW_NUMBER(2),v8::NEW_NUMBER(pageDriver->mPDFPage->GetMediaBox().UpperRightX));
    mediaBox->Set(v8::NEW_NUMBER(3),v8::NEW_NUMBER(pageDriver->mPDFPage->GetMediaBox().UpperRightY));

    SET_ACCESSOR_RETURN_VALUE(mediaBox);
}

void PDFPageDriver::SetMediaBox(Local<String> property, Local<Value> value, const PROPERTY_SETTER_TYPE &info)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFPageDriver* pageDriver = ObjectWrap::Unwrap<PDFPageDriver>(info.Holder());

    if(!value->IsArray())
        THROW_EXCEPTION("Media box is set to a value which is not a 4 numbers array");
    
    if(value->ToObject()->Get(v8::NEW_STRING("length"))->ToObject()->Uint32Value() != 4)
        THROW_EXCEPTION("Media box is set to a value which is not a 4 numbers array");
    
    pageDriver->mPDFPage->SetMediaBox(PDFRectangle(value->ToObject()->Get(0)->ToNumber()->Value(),
                                                  value->ToObject()->Get(1)->ToNumber()->Value(),
                                                  value->ToObject()->Get(2)->ToNumber()->Value(),
                                                  value->ToObject()->Get(3)->ToNumber()->Value()));
    
}

METHOD_RETURN_TYPE PDFPageDriver::GetResourcesDictionary(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    PDFPageDriver* pageDriver = ObjectWrap::Unwrap<PDFPageDriver>(args.This());
    
    Handle<Value> newInstance = ResourcesDictionaryDriver::GetNewInstance(args);
    ResourcesDictionaryDriver* resourceDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(newInstance->ToObject());
    resourceDictionaryDriver->ResourcesDictionaryInstance = &(pageDriver->GetPage()->GetResourcesDictionary());
    
    SET_FUNCTION_RETURN_VALUE(newInstance);
}


