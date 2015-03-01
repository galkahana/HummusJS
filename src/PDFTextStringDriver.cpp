/*
 Source File : UsedFontDriver.h
 
 
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
#include "PDFTextStringDriver.h"
#include <string>

using namespace v8;

Persistent<Function> PDFTextStringDriver::constructor;

void PDFTextStringDriver::Init(Handle<Object> inExports)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PDFTextString"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "toBytesArray", ToBytesArray);
	SET_PROTOTYPE_METHOD(t, "toString", ToString);
	SET_PROTOTYPE_METHOD(t, "fromString", FromString);
	SET_CONSTRUCTOR(constructor, t);

	SET_CONSTRUCTOR_EXPORT(inExports, "PDFTextString", t);

}

METHOD_RETURN_TYPE PDFTextStringDriver::NewInstance(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
		
	SET_FUNCTION_RETURN_VALUE(PDFTextStringDriver::GetNewInstance(args));
}

v8::Handle<v8::Value> PDFTextStringDriver::GetNewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	if (args.Length() != 1 || !args[0]->IsString())
	{
		THROW_EXCEPTION("Wrong arguments. Provide 1 argument which is a string");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
	}

	const unsigned argc = 1;
	Handle<Value> argv[argc] = { args[0] };
	Local<Object> instance = NEW_INSTANCE_ARGS(constructor, argc, argv);

	return CLOSE_SCOPE(instance);
}

METHOD_RETURN_TYPE PDFTextStringDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFTextStringDriver* element = new PDFTextStringDriver();
    if(args.Length() > 0 && args[0]->IsString())
        element->mTextString.FromUTF8(*String::Utf8Value(args[0]->ToString()));
    
    element->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE( args.This());
}

METHOD_RETURN_TYPE PDFTextStringDriver::ToBytesArray(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFTextStringDriver* element = ObjectWrap::Unwrap<PDFTextStringDriver>(args.This());
    
	std::string aString = element->mTextString.ToString();

	Local<Array> result = NEW_ARRAY(aString.length());

	for(std::string::size_type i=0;i<aString.length();++i)
		result->Set(NEW_NUMBER(i),NEW_NUMBER(aString[i]));

	SET_FUNCTION_RETURN_VALUE(result);
}

METHOD_RETURN_TYPE PDFTextStringDriver::ToString(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFTextStringDriver* element = ObjectWrap::Unwrap<PDFTextStringDriver>(args.This());
    
    SET_FUNCTION_RETURN_VALUE(NEW_STRING(element->mTextString.ToUTF8String().c_str()));
}

METHOD_RETURN_TYPE PDFTextStringDriver::FromString(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFTextStringDriver* element = ObjectWrap::Unwrap<PDFTextStringDriver>(args.This());
    if(args.Length() > 0 && args[0]->IsString())
        element->mTextString.FromUTF8(*String::Utf8Value(args[0]->ToString()));
    
    SET_FUNCTION_RETURN_VALUE(args.This());
}