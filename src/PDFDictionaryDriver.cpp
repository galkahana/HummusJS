/*
 Source File : PDFDictionaryDriver.cpp
 
 
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
#include "PDFDictionaryDriver.h"
#include "RefCountPtr.h"
#include <string>
#include "ConstructorsHolder.h"

using namespace v8;



DEF_SUBORDINATE_INIT(PDFDictionaryDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE_EXTERNAL(New);

	t->SetClassName(NEW_STRING("PDFDictionary"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "toJSObject", ToJSObject);
	SET_PROTOTYPE_METHOD(t, "queryObject", QueryObject);
	SET_PROTOTYPE_METHOD(t, "exists", Exists);

	PDFObjectDriver::Init(t);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->PDFDictionary_constructor, t);     
	SET_CONSTRUCTOR_TEMPLATE(holder->PDFDictionary_constructor_template, t);
}

METHOD_RETURN_TYPE PDFDictionaryDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, externalHolder)
    PDFDictionaryDriver* driver = new PDFDictionaryDriver();
    driver->holder = externalHolder;
    driver->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This())
}

PDFObject* PDFDictionaryDriver::GetObject()
{
    return TheObject.GetPtr();
}

METHOD_RETURN_TYPE PDFDictionaryDriver::ToJSObject(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFDictionaryDriver* driver = ObjectWrap::Unwrap<PDFDictionaryDriver>(args.This());
    
    Local<Object> result = NEW_OBJECT;
    
	MapIterator<PDFNameToPDFObjectMap> it = driver->TheObject->GetIterator();
    
    while(it.MoveNext())
        result->Set(GET_CURRENT_CONTEXT, NEW_STRING(it.GetKey()->GetValue().c_str()),driver->holder->GetInstanceFor(it.GetValue()));
    
    SET_FUNCTION_RETURN_VALUE(result)
}

METHOD_RETURN_TYPE PDFDictionaryDriver::Exists(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFDictionaryDriver* driver = ObjectWrap::Unwrap<PDFDictionaryDriver>(args.This());

    if(args.Length() != 1 || !args[0]->IsString())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument which is a string key");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    Local<Boolean> result = NEW_BOOLEAN(driver->TheObject->Exists(*UTF_8_VALUE(args[0]->TO_STRING())));
    
    SET_FUNCTION_RETURN_VALUE(result)
}

METHOD_RETURN_TYPE PDFDictionaryDriver::QueryObject(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 || !args[0]->IsString())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument which is a string key");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    std::string key = *UTF_8_VALUE(args[0]->TO_STRING());

    PDFDictionaryDriver* driver = ObjectWrap::Unwrap<PDFDictionaryDriver>(args.This());
    
    if(!driver->TheObject->Exists(key))
    {
		THROW_EXCEPTION("key not found");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    RefCountPtr<PDFObject> anObject = driver->TheObject->QueryDirectObject(key);
    Local<Value> result = driver->holder->GetInstanceFor(anObject.GetPtr());
    
    SET_FUNCTION_RETURN_VALUE(result)
}