/*
 Source File : ResourcesDictionaryDriver.cpp
 
 
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

#include "ResourcesDictionaryDriver.h"
#include "ResourcesDictionary.h"
#include "ImageXObjectDriver.h"
#include "ConstructorsHolder.h"

using namespace v8;


DEF_SUBORDINATE_INIT(ResourcesDictionaryDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE_EXTERNAL(New);

	t->SetClassName(NEW_STRING("ResourcesDictionary"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "addFormXObjectMapping", AddFormXObjectMapping);
	SET_PROTOTYPE_METHOD(t, "addImageXObjectMapping", AddImageXObjectMapping);
	SET_PROTOTYPE_METHOD(t, "addProcsetResource", AddProcsetResource);
	SET_PROTOTYPE_METHOD(t, "addExtGStateMapping", AddExtGStateMapping);
	SET_PROTOTYPE_METHOD(t, "addFontMapping", AddFontMapping);
	SET_PROTOTYPE_METHOD(t, "addColorSpaceMapping", AddColorSpaceMapping);
	SET_PROTOTYPE_METHOD(t, "addPatternMapping", AddPatternMapping);
	SET_PROTOTYPE_METHOD(t, "addPropertyMapping", AddPropertyMapping);
	SET_PROTOTYPE_METHOD(t, "addXObjectMapping", AddXObjectMapping);
	SET_PROTOTYPE_METHOD(t, "addShadingMapping", AddShadingMapping);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->ResourcesDictionary_constructor, t);   
}

ResourcesDictionaryDriver::ResourcesDictionaryDriver()
{
    ResourcesDictionaryInstance = NULL;
}

METHOD_RETURN_TYPE ResourcesDictionaryDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, externalHolder)
    
    ResourcesDictionaryDriver* form = new ResourcesDictionaryDriver();
    form->holder = externalHolder;
    form->Wrap(args.This());
    
	SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE ResourcesDictionaryDriver::AddFormXObjectMapping(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument which is the form xobject id");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    Local<String> name = NEW_STRING(
                                     resourcesDictionaryDriver->ResourcesDictionaryInstance->AddFormXObjectMapping(
                                                                                                        (ObjectIDType)(TO_UINT32(args[0])->Value())).c_str());
    
    SET_FUNCTION_RETURN_VALUE(name)
    
}

METHOD_RETURN_TYPE ResourcesDictionaryDriver::AddImageXObjectMapping(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1)
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument which is the image xobject or its ID");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    if(resourcesDictionaryDriver->holder->IsImageXObjectInstance(args[0]))
    {
    
        Local<String> name = NEW_STRING(
                                         resourcesDictionaryDriver->ResourcesDictionaryInstance->AddImageXObjectMapping(
                                                                                ObjectWrap::Unwrap<ImageXObjectDriver>(args[0]->TO_OBJECT())->ImageXObject
                                                                                    ).c_str());
        
        SET_FUNCTION_RETURN_VALUE(name)
    }
    else if(args[0]->IsNumber())
    {
        Local<String> name = NEW_STRING(
                                         resourcesDictionaryDriver->ResourcesDictionaryInstance->AddImageXObjectMapping(
                                                                                                                       (ObjectIDType)(TO_UINT32(args[0])->Value())).c_str());
        
        SET_FUNCTION_RETURN_VALUE(name)
    }
    else
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument which is the image xobject or its ID");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
}

METHOD_RETURN_TYPE ResourcesDictionaryDriver::AddProcsetResource(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 || !args[0]->IsString())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument which is the procset name");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    resourcesDictionaryDriver->ResourcesDictionaryInstance->AddProcsetResource(*UTF_8_VALUE(args[0]->TO_STRING()));
    
     SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
}

METHOD_RETURN_TYPE ResourcesDictionaryDriver::AddExtGStateMapping(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument which is the external graphic state object id");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    Local<String> name = NEW_STRING(
                                     resourcesDictionaryDriver->ResourcesDictionaryInstance->AddExtGStateMapping(
                                                                                                                   (ObjectIDType)(TO_UINT32(args[0])->Value())).c_str());
    
    SET_FUNCTION_RETURN_VALUE(name)
}

METHOD_RETURN_TYPE ResourcesDictionaryDriver::AddFontMapping(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument which is the font object id");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    Local<String> name = NEW_STRING(
                                     resourcesDictionaryDriver->ResourcesDictionaryInstance->AddFontMapping(
                                                                                                                 (ObjectIDType)(TO_UINT32(args[0])->Value())).c_str());
    
    SET_FUNCTION_RETURN_VALUE(name)
}

METHOD_RETURN_TYPE ResourcesDictionaryDriver::AddColorSpaceMapping(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument which is the color space id");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    Local<String> name = NEW_STRING(
                                     resourcesDictionaryDriver->ResourcesDictionaryInstance->AddColorSpaceMapping(
                                                                                                            (ObjectIDType)(TO_UINT32(args[0])->Value())).c_str());
    
    SET_FUNCTION_RETURN_VALUE(name)
}

METHOD_RETURN_TYPE ResourcesDictionaryDriver::AddPatternMapping(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument which is the pattern object id");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    Local<String> name = NEW_STRING(
                                     resourcesDictionaryDriver->ResourcesDictionaryInstance->AddPatternMapping(
                                                                                                                  (ObjectIDType)(TO_UINT32(args[0])->Value())).c_str());
    
    SET_FUNCTION_RETURN_VALUE(name)
}

METHOD_RETURN_TYPE ResourcesDictionaryDriver::AddPropertyMapping(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument which is the property object id");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    Local<String> name = NEW_STRING(
                                     resourcesDictionaryDriver->ResourcesDictionaryInstance->AddPropertyMapping(
                                                                                                               (ObjectIDType)(TO_UINT32(args[0])->Value())).c_str());
    
    SET_FUNCTION_RETURN_VALUE(name)
}

METHOD_RETURN_TYPE ResourcesDictionaryDriver::AddXObjectMapping(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument which is the xobject id");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    Local<String> name = NEW_STRING(
                                     resourcesDictionaryDriver->ResourcesDictionaryInstance->AddXObjectMapping(
                                                                                                                (ObjectIDType)(TO_UINT32(args[0])->Value())).c_str());
    
    SET_FUNCTION_RETURN_VALUE(name)
}

METHOD_RETURN_TYPE ResourcesDictionaryDriver::AddShadingMapping(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument which is the shading object id");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    Local<String> name = NEW_STRING(
                                     resourcesDictionaryDriver->ResourcesDictionaryInstance->AddShadingMapping(
                                                                                                               (ObjectIDType)(TO_UINT32(args[0])->Value())).c_str());
    
    SET_FUNCTION_RETURN_VALUE(name)
}


