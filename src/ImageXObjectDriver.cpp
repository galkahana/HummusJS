/*
 Source File : ImageXObjectDriver.cpp
 
 
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
#include "ImageXObjectDriver.h"
#include "PDFImageXObject.h"
#include "ConstructorsHolder.h"

using namespace v8;

ImageXObjectDriver::~ImageXObjectDriver()
{
    delete ImageXObject;
}

DEF_SUBORDINATE_INIT(ImageXObjectDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("ImageXObject"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_ACCESSOR_METHOD(t, "id", GetID);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->ImageXObject_constructor, t);    	
	SET_CONSTRUCTOR_TEMPLATE(holder->ImageXObject_constructor_template, t);
}

ImageXObjectDriver::ImageXObjectDriver()
{
    ImageXObject = NULL;
}

METHOD_RETURN_TYPE ImageXObjectDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    ImageXObjectDriver* image = new ImageXObjectDriver();
    image->Wrap(args.This());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE ImageXObjectDriver::GetID(Local<String> property, const PROPERTY_TYPE &info)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    ImageXObjectDriver* image = ObjectWrap::Unwrap<ImageXObjectDriver>(info.Holder());
    
    if(!image->ImageXObject)
    {
		THROW_EXCEPTION("image object not initialized, create using pdfWriter.createFormXObject");
        SET_ACCESSOR_RETURN_VALUE(UNDEFINED)
    }
    
	SET_ACCESSOR_RETURN_VALUE(NEW_NUMBER(image->ImageXObject->GetImageObjectID()))
}

