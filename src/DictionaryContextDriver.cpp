/*
 Source File : DictionaryContextDriver.cpp
 
 
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
#include "DictionaryContextDriver.h"
#include "DictionaryContext.h"
#include "PDFRectangle.h"

using namespace v8;

DictionaryContextDriver::~DictionaryContextDriver()
{
    // Do not, under any circumstance, be tempted to delete the dictionary context. it will be deleted by "EndContext" of the owner - the objects context
    // there was a delete here which causes a bug to happen for a year. until today. 28/5/2014
}

void DictionaryContextDriver::Init()
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("DictionaryContext"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "writeKey", WriteKey);
	SET_PROTOTYPE_METHOD(t, "writeNameValue", WriteNameValue);
	SET_PROTOTYPE_METHOD(t, "writeRectangleValue", WriteRectangleValue);
	SET_PROTOTYPE_METHOD(t, "writeLiteralStringValue", WriteLiteralStringValue);
	SET_PROTOTYPE_METHOD(t, "writeBooleanValue", WriteBooleanValue);
	SET_PROTOTYPE_METHOD(t, "writeObjectReferenceValue", WriteObjectReferenceValue);
	SET_CONSTRUCTOR(constructor, t);
	SET_CONSTRUCTOR_TEMPLATE(constructor_template, t);
}

METHOD_RETURN_TYPE DictionaryContextDriver::NewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	SET_FUNCTION_RETURN_VALUE(instance);
}

v8::Handle<v8::Value> DictionaryContextDriver::GetNewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

v8::Handle<v8::Value> DictionaryContextDriver::GetInstanceFor(DictionaryContext* inDictionaryContextInstance) {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
    ObjectWrap::Unwrap<DictionaryContextDriver>(instance->ToObject())->DictionaryContextInstance = inDictionaryContextInstance;
	return CLOSE_SCOPE(instance);    
}


bool DictionaryContextDriver::HasInstance(Handle<Value> inObject)
{
	CREATE_ISOLATE_CONTEXT;

	return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);
}

DictionaryContextDriver::DictionaryContextDriver()
{
    DictionaryContextInstance = NULL;
}

Persistent<Function> DictionaryContextDriver::constructor;
Persistent<FunctionTemplate> DictionaryContextDriver::constructor_template;

METHOD_RETURN_TYPE DictionaryContextDriver::New(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    DictionaryContextDriver* driver = new DictionaryContextDriver();
    driver->Wrap(args.This());
    
	SET_FUNCTION_RETURN_VALUE(args.This());
}

METHOD_RETURN_TYPE DictionaryContextDriver::WriteKey(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    if(!(args.Length() == 1) ||
       !args[0]->IsString())
    {
		THROW_EXCEPTION("Wrong arguments, provide a string to write");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
        
    }
    
    DictionaryContextDriver* driver = ObjectWrap::Unwrap<DictionaryContextDriver>(args.This());
    
    if(!driver->DictionaryContextInstance)
    {
		THROW_EXCEPTION("dictinoarycontext object not initialized, create using objectscontext.startDictionary");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    driver->DictionaryContextInstance->WriteKey(*String::Utf8Value(args[0]->ToString()));
    
    SET_FUNCTION_RETURN_VALUE(args.This());
}

METHOD_RETURN_TYPE DictionaryContextDriver::WriteRectangleValue(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    // can accept array or four numbers
    if( (args.Length() != 1 && args.Length() != 4) ||
       (args.Length() == 1 && !args[0]->IsArray()) ||
       (args.Length() == 4 && (!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber())))
    {
		THROW_EXCEPTION("Wrong arguments, provide an array of 4 numbers, or 4 numbers");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
        
    }
    
    DictionaryContextDriver* driver = ObjectWrap::Unwrap<DictionaryContextDriver>(args.This());
    
    if(!driver->DictionaryContextInstance)
    {
		THROW_EXCEPTION("dictinoarycontext object not initialized, create using objectscontext.startDictionary");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    if(args.Length() == 1)
    {
        // array version. verify that there are 4 numbers
        if(args[0]->ToObject()->Get(NEW_STRING("length"))->ToObject()->Uint32Value() != 4)
        {
            THROW_EXCEPTION("Wrong arguments, provide an array of 4 numbers, or 4 numbers");
            SET_FUNCTION_RETURN_VALUE(UNDEFINED);
        }
        
        driver->DictionaryContextInstance->WriteRectangleValue(PDFRectangle(
                                                      args[0]->ToObject()->Get(0)->ToNumber()->Value(),
                                                      args[0]->ToObject()->Get(1)->ToNumber()->Value(),
                                                      args[0]->ToObject()->Get(2)->ToNumber()->Value(),
                                                      args[0]->ToObject()->Get(3)->ToNumber()->Value()));
        
    }
    else
    {
        // 4 numbers version
        driver->DictionaryContextInstance->WriteRectangleValue(PDFRectangle(
                                                                            args[0]->ToNumber()->Value(),
                                                                            args[1]->ToNumber()->Value(),
                                                                            args[2]->ToNumber()->Value(),
                                                                            args[3]->ToNumber()->Value()));
    }
    
    
    SET_FUNCTION_RETURN_VALUE(args.This());
}

METHOD_RETURN_TYPE DictionaryContextDriver::WriteNameValue(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    if(!(args.Length() == 1) ||
       !args[0]->IsString())
    {
		THROW_EXCEPTION("Wrong arguments, provide a string to write");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
        
    }
    
    DictionaryContextDriver* driver = ObjectWrap::Unwrap<DictionaryContextDriver>(args.This());
    
    if(!driver->DictionaryContextInstance)
    {
		THROW_EXCEPTION("dictinoarycontext object not initialized, create using objectscontext.startDictionary");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    driver->DictionaryContextInstance->WriteNameValue(*String::Utf8Value(args[0]->ToString()));
    
    SET_FUNCTION_RETURN_VALUE(args.This());
}

METHOD_RETURN_TYPE DictionaryContextDriver::WriteLiteralStringValue(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    if(!(args.Length() == 1) ||
       (!args[0]->IsString() && !args[0]->IsArray()))
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument that is a literal string (string) or an array");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
        
    }

    DictionaryContextDriver* driver = ObjectWrap::Unwrap<DictionaryContextDriver>(args.This());
    
    if(!driver->DictionaryContextInstance)
    {
		THROW_EXCEPTION("dictinoarycontext object not initialized, create using objectscontext.startDictionary");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }

	if(args[0]->IsArray())
	{
		std::string string;
		unsigned long arrayLength = (args[0]->ToObject()->Get(NEW_STRING("length")))->ToObject()->Uint32Value();
		for(unsigned long i=0;i<arrayLength;++i)
			string.push_back((unsigned char)args[0]->ToObject()->Get(i)->ToNumber()->Value());
		driver->DictionaryContextInstance->WriteLiteralStringValue(string);
	}
	else
    {
		driver->DictionaryContextInstance->WriteLiteralStringValue(*String::Utf8Value(args[0]->ToString()));
	}
    SET_FUNCTION_RETURN_VALUE(args.This());
}

METHOD_RETURN_TYPE DictionaryContextDriver::WriteBooleanValue(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    if(!(args.Length() == 1) ||
       !args[0]->IsBoolean())
    {
		THROW_EXCEPTION("Wrong arguments, provide a boolean to write");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
        
    }
    DictionaryContextDriver* driver = ObjectWrap::Unwrap<DictionaryContextDriver>(args.This());
    
    if(!driver->DictionaryContextInstance)
    {
		THROW_EXCEPTION("dictinoarycontext object not initialized, create using objectscontext.startDictionary");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    driver->DictionaryContextInstance->WriteBooleanValue(args[0]->ToBoolean()->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This());
}

METHOD_RETURN_TYPE DictionaryContextDriver::WriteObjectReferenceValue(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    if(!(args.Length() == 1) ||
       !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong arguments, provide an object id to write");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
        
    }
    DictionaryContextDriver* driver = ObjectWrap::Unwrap<DictionaryContextDriver>(args.This());
    
    if(!driver->DictionaryContextInstance)
    {
		THROW_EXCEPTION("dictinoarycontext object not initialized, create using objectscontext.startDictionary");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    driver->DictionaryContextInstance->WriteObjectReferenceValue((ObjectIDType)args[0]->ToNumber()->Uint32Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This());
}