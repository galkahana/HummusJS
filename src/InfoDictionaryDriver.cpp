/*
 Source File : InfoDictinaryDriver.cpp
 
 
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
#include "InfoDictionaryDriver.h"
#include "InfoDictionary.h"
#include "PDFDateDriver.h"
#include "PDFDate.h"
#include "PDFTextString.h"

using namespace v8;

Persistent<Function> InfoDictionaryDriver::constructor;

InfoDictionaryDriver::InfoDictionaryDriver()
{
    InfoDictionaryInstance = NULL;
}

void InfoDictionaryDriver::Init()
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("InfoDictionary"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "addAdditionalInfoEntry", AddAdditionalInfoEntry);
	SET_PROTOTYPE_METHOD(t, "removeAdditionalInfoEntry", RemoveAdditionalInfoEntry);
	SET_PROTOTYPE_METHOD(t, "clearAdditionalInfoEntries", ClearAdditionalInfoEntries);
	SET_PROTOTYPE_METHOD(t, "getAdditionalInfoEntry", GetAdditionalInfoEntry);
	SET_PROTOTYPE_METHOD(t, "getAdditionalInfoEntries", GetAdditionalInfoEntries);
	SET_PROTOTYPE_METHOD(t, "setCreationDate", SetCreationDate);
	SET_PROTOTYPE_METHOD(t, "setModDate", SetModDate);
	SET_ACCESSOR_METHODS(t, "title", GetTitle, SetTitle);
	SET_ACCESSOR_METHODS(t, "author", GetAuthor, SetAuthor);
	SET_ACCESSOR_METHODS(t, "subject", GetSubject, SetSubject);
	SET_ACCESSOR_METHODS(t, "keywords", GetKeywords, SetKeywords);
	SET_ACCESSOR_METHODS(t, "creator", GetCreator, SetCreator);
	SET_ACCESSOR_METHODS(t, "producer", GetProducer, SetProducer);
	SET_ACCESSOR_METHODS(t, "trapped", GetTrapped, SetTrapped);


	SET_CONSTRUCTOR(constructor, t);
}

METHOD_RETURN_TYPE InfoDictionaryDriver::NewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	SET_FUNCTION_RETURN_VALUE(instance);
}

v8::Handle<v8::Value> InfoDictionaryDriver::GetNewInstance()
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

METHOD_RETURN_TYPE InfoDictionaryDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InfoDictionaryDriver* driver = new InfoDictionaryDriver();
    driver->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This());
}

METHOD_RETURN_TYPE InfoDictionaryDriver::GetTitle(Local<String> property, const PROPERTY_TYPE &info)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
    {
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
        SET_ACCESSOR_RETURN_VALUE(UNDEFINED);
    }

	SET_ACCESSOR_RETURN_VALUE(NEW_STRING(infoDictionaryDriver->InfoDictionaryInstance->Title.ToUTF8String().c_str()));
}

void InfoDictionaryDriver::SetTitle(Local<String> property, Local<Value> value, const PROPERTY_SETTER_TYPE &info)
{
	CREATE_ISOLATE_CONTEXT;
	
	InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
    
    infoDictionaryDriver->InfoDictionaryInstance->Title.FromUTF8(*String::Utf8Value(value->ToString()));
}

METHOD_RETURN_TYPE InfoDictionaryDriver::GetAuthor(Local<String> property, const PROPERTY_TYPE &info)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
    {
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
		SET_ACCESSOR_RETURN_VALUE(UNDEFINED);
    }
    
	SET_ACCESSOR_RETURN_VALUE(NEW_STRING(infoDictionaryDriver->InfoDictionaryInstance->Author.ToUTF8String().c_str()));
}

void InfoDictionaryDriver::SetAuthor(Local<String> property, Local<Value> value, const PROPERTY_SETTER_TYPE &info)
{
	CREATE_ISOLATE_CONTEXT;
	
	InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
    
    infoDictionaryDriver->InfoDictionaryInstance->Author.FromUTF8(*String::Utf8Value(value->ToString()));
}

METHOD_RETURN_TYPE InfoDictionaryDriver::GetSubject(Local<String> property, const PROPERTY_TYPE &info)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
    {
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
		SET_ACCESSOR_RETURN_VALUE(UNDEFINED);
    }
    
	SET_ACCESSOR_RETURN_VALUE(NEW_STRING(infoDictionaryDriver->InfoDictionaryInstance->Subject.ToUTF8String().c_str()));
}

void InfoDictionaryDriver::SetSubject(Local<String> property, Local<Value> value, const PROPERTY_SETTER_TYPE &info)
{
	CREATE_ISOLATE_CONTEXT;
	InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
    
    infoDictionaryDriver->InfoDictionaryInstance->Subject.FromUTF8(*String::Utf8Value(value->ToString()));
}

METHOD_RETURN_TYPE InfoDictionaryDriver::GetKeywords(Local<String> property, const PROPERTY_TYPE &info)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
    {
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
		SET_ACCESSOR_RETURN_VALUE(UNDEFINED);
    }
    
	SET_ACCESSOR_RETURN_VALUE(NEW_STRING(infoDictionaryDriver->InfoDictionaryInstance->Keywords.ToUTF8String().c_str()));
}

void InfoDictionaryDriver::SetKeywords(Local<String> property, Local<Value> value, const PROPERTY_SETTER_TYPE &info)
{
	CREATE_ISOLATE_CONTEXT;
	InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
    
    infoDictionaryDriver->InfoDictionaryInstance->Keywords.FromUTF8(*String::Utf8Value(value->ToString()));
}

METHOD_RETURN_TYPE InfoDictionaryDriver::GetCreator(Local<String> property, const PROPERTY_TYPE &info)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
    {
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
		SET_ACCESSOR_RETURN_VALUE(UNDEFINED);
    }
    
	SET_ACCESSOR_RETURN_VALUE(NEW_STRING(infoDictionaryDriver->InfoDictionaryInstance->Creator.ToUTF8String().c_str()));
}

void InfoDictionaryDriver::SetCreator(Local<String> property, Local<Value> value, const PROPERTY_SETTER_TYPE &info)
{
	CREATE_ISOLATE_CONTEXT;
	InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
    
    infoDictionaryDriver->InfoDictionaryInstance->Creator.FromUTF8(*String::Utf8Value(value->ToString()));
}

METHOD_RETURN_TYPE InfoDictionaryDriver::GetProducer(Local<String> property, const PROPERTY_TYPE &info)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
   
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
    {
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
		SET_ACCESSOR_RETURN_VALUE(UNDEFINED);
    }
    
	SET_ACCESSOR_RETURN_VALUE(NEW_STRING(infoDictionaryDriver->InfoDictionaryInstance->Producer.ToUTF8String().c_str()));
}

void InfoDictionaryDriver::SetProducer(Local<String> property, Local<Value> value, const PROPERTY_SETTER_TYPE &info)
{
	CREATE_ISOLATE_CONTEXT;
	InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
    
    infoDictionaryDriver->InfoDictionaryInstance->Producer.FromUTF8(*String::Utf8Value(value->ToString()));
}

METHOD_RETURN_TYPE InfoDictionaryDriver::GetTrapped(Local<String> property, const PROPERTY_TYPE &info)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
    {
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
		SET_ACCESSOR_RETURN_VALUE(UNDEFINED);
    }
    
	SET_ACCESSOR_RETURN_VALUE(NEW_NUMBER(infoDictionaryDriver->InfoDictionaryInstance->Trapped));
}

void InfoDictionaryDriver::SetTrapped(Local<String> property, Local<Value> value, const PROPERTY_SETTER_TYPE &info)
{
	CREATE_ISOLATE_CONTEXT;
	InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
    
    infoDictionaryDriver->InfoDictionaryInstance->Trapped = (EInfoTrapped)(value->ToNumber()->Uint32Value());
}

METHOD_RETURN_TYPE InfoDictionaryDriver::SetCreationDate(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InfoDictionaryDriver* driver = ObjectWrap::Unwrap<InfoDictionaryDriver>(args.This());
    if(!driver->InfoDictionaryInstance)
    {
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }

    
    Handle<Value> dataDriverValue = PDFDateDriver::GetNewInstance(args);
    PDFDateDriver* dateDriver = ObjectWrap::Unwrap<PDFDateDriver>(dataDriverValue->ToObject());

    driver->InfoDictionaryInstance->CreationDate =  *(dateDriver->getInstance());
    
    SET_FUNCTION_RETURN_VALUE(UNDEFINED);
}

METHOD_RETURN_TYPE InfoDictionaryDriver::SetModDate(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InfoDictionaryDriver* driver = ObjectWrap::Unwrap<InfoDictionaryDriver>(args.This());
    if(!driver->InfoDictionaryInstance)
    {
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    
    Handle<Value> dataDriverValue = PDFDateDriver::GetNewInstance(args);
    PDFDateDriver* dateDriver = ObjectWrap::Unwrap<PDFDateDriver>(dataDriverValue->ToObject());
    
    driver->InfoDictionaryInstance->ModDate =  *(dateDriver->getInstance());
    
    SET_FUNCTION_RETURN_VALUE(UNDEFINED);
}

METHOD_RETURN_TYPE InfoDictionaryDriver::AddAdditionalInfoEntry(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InfoDictionaryDriver* driver = ObjectWrap::Unwrap<InfoDictionaryDriver>(args.This());
    if(!driver->InfoDictionaryInstance)
    {
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    if(args.Length() != 2 ||
       !args[0]->IsString() ||
       !args[1]->IsString())
    {
		THROW_EXCEPTION("wrong arguments. please provide two strings - key and value ");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFTextString textString;
    textString.FromUTF8(*String::Utf8Value(args[1]->ToString()));
    
    driver->InfoDictionaryInstance->AddAdditionalInfoEntry(*String::Utf8Value(args[0]->ToString()), textString);
    
    SET_FUNCTION_RETURN_VALUE(UNDEFINED);
}

METHOD_RETURN_TYPE InfoDictionaryDriver::RemoveAdditionalInfoEntry(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InfoDictionaryDriver* driver = ObjectWrap::Unwrap<InfoDictionaryDriver>(args.This());
    if(!driver->InfoDictionaryInstance)
    {
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    if(args.Length() != 1 ||
       !args[0]->IsString())
    {
		THROW_EXCEPTION("wrong arguments. please provide key of the entry to remove ");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    driver->InfoDictionaryInstance->RemoveAdditionalInfoEntry(*String::Utf8Value(args[0]->ToString()));
    
    SET_FUNCTION_RETURN_VALUE(UNDEFINED);
}

METHOD_RETURN_TYPE InfoDictionaryDriver::ClearAdditionalInfoEntries(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InfoDictionaryDriver* driver = ObjectWrap::Unwrap<InfoDictionaryDriver>(args.This());
    if(!driver->InfoDictionaryInstance)
    {
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    driver->InfoDictionaryInstance->ClearAdditionalInfoEntries();
    
    SET_FUNCTION_RETURN_VALUE(UNDEFINED);
}

METHOD_RETURN_TYPE InfoDictionaryDriver::GetAdditionalInfoEntry(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InfoDictionaryDriver* driver = ObjectWrap::Unwrap<InfoDictionaryDriver>(args.This());
    if(!driver->InfoDictionaryInstance)
    {
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    if(args.Length() != 1 ||
       !args[0]->IsString())
    {
		THROW_EXCEPTION("wrong arguments. please provide key of the entry to return ");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFTextString testString = driver->InfoDictionaryInstance->GetAdditionalInfoEntry(*String::Utf8Value(args[0]->ToString()));
    
    SET_FUNCTION_RETURN_VALUE(NEW_STRING(testString.ToUTF8String().c_str()));
}

METHOD_RETURN_TYPE InfoDictionaryDriver::GetAdditionalInfoEntries(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InfoDictionaryDriver* driver = ObjectWrap::Unwrap<InfoDictionaryDriver>(args.This());
    if(!driver->InfoDictionaryInstance)
    {
		THROW_EXCEPTION("info dictionary driver not initialized. use the document context object to get a valid info dictionary");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    if(args.Length() != 1 ||
       !args[0]->IsString())
    {
		THROW_EXCEPTION("wrong arguments. please provide key of the entry to return ");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    Handle<Object> anObject = NEW_OBJECT;
   
    MapIterator<StringToPDFTextString> it = driver->InfoDictionaryInstance->GetAdditionaEntriesIterator();

    while(it.MoveNext())
        anObject->Set(NEW_STRING(it.GetKey().c_str()),NEW_STRING(it.GetValue().ToUTF8String().c_str()));
    
    SET_FUNCTION_RETURN_VALUE(anObject);
}


