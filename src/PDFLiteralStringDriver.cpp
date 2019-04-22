/*
 Source File : PDFLiteralStringDriver.h
 
 
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
#include "PDFLiteralStringDriver.h"
#include "IOBasicTypes.h"
#include "RefCountPtr.h"
#include "PDFTextString.h"
#include "ConstructorsHolder.h"

using namespace v8;



DEF_SUBORDINATE_INIT(PDFLiteralStringDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE_EXTERNAL(New);

	t->SetClassName(NEW_STRING("PDFLiteralString"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "toText", ToText);
	SET_PROTOTYPE_METHOD(t, "toBytesArray", ToBytesArray);
	SET_ACCESSOR_METHOD(t, "value", GetValue);
	PDFObjectDriver::Init(t);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->PDFLiteralString_constructor, t);  	
	SET_CONSTRUCTOR_TEMPLATE(holder->PDFLiteralString_constructor_template, t);
}

METHOD_RETURN_TYPE PDFLiteralStringDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
	EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, externalHolder)
    
    PDFLiteralStringDriver* driver = new PDFLiteralStringDriver();
	driver->holder = externalHolder;
    driver->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This())
}

PDFObject* PDFLiteralStringDriver::GetObject()
{
    return TheObject.GetPtr();
}

METHOD_RETURN_TYPE PDFLiteralStringDriver::GetValue(Local<String> property, const PROPERTY_TYPE &info)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    
    Local<String> result = NEW_STRING(ObjectWrap::Unwrap<PDFLiteralStringDriver>(info.Holder())->TheObject->GetValue().c_str());
    SET_ACCESSOR_RETURN_VALUE(result)
}


METHOD_RETURN_TYPE PDFLiteralStringDriver::ToText(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    PDFLiteralStringDriver* driver = ObjectWrap::Unwrap<PDFLiteralStringDriver>(args.This());

    
    Local<String> result = NEW_STRING(PDFTextString(driver->TheObject->GetValue()).ToUTF8String().c_str());
    SET_FUNCTION_RETURN_VALUE(result)
}

METHOD_RETURN_TYPE PDFLiteralStringDriver::ToBytesArray(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
	std::string aString =  ObjectWrap::Unwrap<PDFLiteralStringDriver>(args.This())->TheObject->GetValue();

	Local<Array> result = NEW_ARRAY(aString.length());

	for(std::string::size_type i=0;i<aString.length();++i)
		result->Set(GET_CURRENT_CONTEXT, NEW_NUMBER(i),NEW_NUMBER((IOBasicTypes::Byte)(aString[i])));

	SET_FUNCTION_RETURN_VALUE(result)
}


