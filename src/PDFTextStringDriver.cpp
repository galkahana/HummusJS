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
#include "IOBasicTypes.h"
#include "ConstructorsHolder.h"
#include <string>

using namespace v8;

DEF_SUBORDINATE_INIT(PDFTextStringDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PDFTextString"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "toBytesArray", ToBytesArray);
	SET_PROTOTYPE_METHOD(t, "toString", ToString);
	SET_PROTOTYPE_METHOD(t, "fromString", FromString);

	SET_CONSTRUCTOR_EXPORT("PDFTextString", t);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->PDFTextString_constructor, t);   	
}

METHOD_RETURN_TYPE PDFTextStringDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFTextStringDriver* element = new PDFTextStringDriver();
	if(args.Length() > 0) {
		if(args[0]->IsString()) {
			// text string, consider a plain unicode string
	        element->mTextString.FromUTF8(*UTF_8_VALUE(args[0]->TO_STRING()));
		}
		else if(args[0]->IsArray()) {
			// bytes array, init as is
			int bufferSize = args[0]->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, NEW_STRING("length")).ToLocalChecked()->TO_UINT32Value();
			std::string buffer;

			for(int i=0;i<bufferSize;++i)
				buffer.push_back((char)(args[0]->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, i).ToLocalChecked()->TO_UINT32Value()));
			element->mTextString = buffer;
		}
	}
    
    element->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE( args.This())
}

METHOD_RETURN_TYPE PDFTextStringDriver::ToBytesArray(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFTextStringDriver* element = ObjectWrap::Unwrap<PDFTextStringDriver>(args.This());
    
	std::string aString = element->mTextString.ToString();

	Local<Array> result = NEW_ARRAY(aString.length());

	for(std::string::size_type i=0;i<aString.length();++i)
		result->Set(GET_CURRENT_CONTEXT, NEW_NUMBER(i),NEW_NUMBER((IOBasicTypes::Byte)(aString[i])));

	SET_FUNCTION_RETURN_VALUE(result)
}

METHOD_RETURN_TYPE PDFTextStringDriver::ToString(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFTextStringDriver* element = ObjectWrap::Unwrap<PDFTextStringDriver>(args.This());
    
    SET_FUNCTION_RETURN_VALUE(NEW_STRING(element->mTextString.ToUTF8String().c_str()))
}

METHOD_RETURN_TYPE PDFTextStringDriver::FromString(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFTextStringDriver* element = ObjectWrap::Unwrap<PDFTextStringDriver>(args.This());
    if(args.Length() > 0 && args[0]->IsString())
        element->mTextString.FromUTF8(*UTF_8_VALUE(args[0]->TO_STRING()));
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}