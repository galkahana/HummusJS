/*
 Source File : PDFObjectDriver.cpp
 
 
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

#include "PDFObjectDriver.h"
#include "PDFObject.h"
#include "PDFObjectCast.h"
#include "PDFArrayDriver.h"
#include "PDFArray.h"
#include "PDFDictionaryDriver.h"
#include "PDFDictionary.h"
#include "PDFStreamInputDriver.h"
#include "PDFStreamInput.h"
#include "PDFIndirectObjectReferenceDriver.h"
#include "PDFIndirectObjectReference.h"
#include "PDFBooleanDriver.h"
#include "PDFBoolean.h"
#include "PDFLiteralStringDriver.h"
#include "PDFLiteralString.h"
#include "PDFHexStringDriver.h"
#include "PDFHexString.h"
#include "PDFNullDriver.h"
#include "PDFNull.h"
#include "PDFNameDriver.h"
#include "PDFName.h"
#include "PDFIntegerDriver.h"
#include "PDFInteger.h"
#include "PDFRealDriver.h"
#include "PDFReal.h"
#include "PDFSymbolDriver.h"
#include "PDFSymbol.h"
#include "BoxingBase.h"

using namespace v8;

void PDFObjectDriver::Init(Handle<FunctionTemplate>& ioDriverTemplate)
{
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "getType", GetType);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "toPDFIndirectObjectReference", ToPDFIndirectObjectReference);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "toPDFArray", ToPDFArray);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "toPDFDictionary", ToPDFDictionary);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "toPDFStream", ToPDFStream);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "toPDFBoolean", ToPDFBoolean);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "toPDFLiteralString", ToPDFLiteralString);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "toPDFHexString", ToPDFHexString);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "toPDFNull", ToPDFNull);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "toPDFName", ToPDFName);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "toPDFInteger", ToPDFInteger);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "toPDFReal", ToPDFReal);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "toPDFSymbol", ToPDFSymbol);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "toNumber", ToNumber);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "toString", ToString);
}

METHOD_RETURN_TYPE PDFObjectDriver::GetType(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    Local<Number> result = NEW_NUMBER((unsigned long)ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject()->GetType());
    SET_FUNCTION_RETURN_VALUE(result);
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFIndirectObjectReference(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectIndirectObjectReference)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    
	Handle<Value> newInstance = PDFIndirectObjectReferenceDriver::GetNewInstance();
    ObjectWrap::Unwrap<PDFIndirectObjectReferenceDriver>(newInstance->ToObject())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFArray(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectArray)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    
	Handle<Value> newInstance = PDFArrayDriver::GetNewInstance();
    ObjectWrap::Unwrap<PDFArrayDriver>(newInstance->ToObject())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFDictionary(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectDictionary)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    
	Handle<Value> newInstance = PDFDictionaryDriver::GetNewInstance();
    ObjectWrap::Unwrap<PDFDictionaryDriver>(newInstance->ToObject())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFStream(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectStream)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    
    Handle<Value> newInstance = PDFStreamInputDriver::GetNewInstance();
    ObjectWrap::Unwrap<PDFStreamInputDriver>(newInstance->ToObject())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFBoolean(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectBoolean)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    
	Handle<Value> newInstance = PDFBooleanDriver::GetNewInstance();
    ObjectWrap::Unwrap<PDFBooleanDriver>(newInstance->ToObject())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFLiteralString(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectLiteralString)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    
	Handle<Value> newInstance = PDFLiteralStringDriver::GetNewInstance();
    ObjectWrap::Unwrap<PDFLiteralStringDriver>(newInstance->ToObject())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFHexString(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectHexString)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    
	Handle<Value> newInstance = PDFHexStringDriver::GetNewInstance();
    ObjectWrap::Unwrap<PDFHexStringDriver>(newInstance->ToObject())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFNull(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectNull)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    
	Handle<Value> newInstance = PDFNullDriver::GetNewInstance();
    ObjectWrap::Unwrap<PDFNullDriver>(newInstance->ToObject())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFName(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectName)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    
	Handle<Value> newInstance = PDFNameDriver::GetNewInstance();
    ObjectWrap::Unwrap<PDFNameDriver>(newInstance->ToObject())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFInteger(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectInteger)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    
	Handle<Value> newInstance = PDFIntegerDriver::GetNewInstance();
    ObjectWrap::Unwrap<PDFIntegerDriver>(newInstance->ToObject())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFReal(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectReal)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    
    Handle<Value> newInstance = PDFRealDriver::GetNewInstance();
    ObjectWrap::Unwrap<PDFRealDriver>(newInstance->ToObject())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFSymbol(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectSymbol)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    
    Handle<Value> newInstance = PDFSymbolDriver::GetNewInstance();
    ObjectWrap::Unwrap<PDFSymbolDriver>(newInstance->ToObject())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

Handle<Value> PDFObjectDriver::CreateDriver(PDFObject* inObject)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
	ESCAPABLE_HANDLE(Value) newInstance;

    switch(inObject->GetType())
    {
        case PDFObject::ePDFObjectBoolean:
        {
            newInstance = PDFBooleanDriver::GetNewInstance();
            ObjectWrap::Unwrap<PDFBooleanDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectLiteralString:
        {
			newInstance = PDFLiteralStringDriver::GetNewInstance();
            ObjectWrap::Unwrap<PDFLiteralStringDriver>(newInstance->ToObject())->TheObject = inObject;
            break;            
        }
        case PDFObject::ePDFObjectHexString:
        {
			newInstance = PDFHexStringDriver::GetNewInstance();
            ObjectWrap::Unwrap<PDFHexStringDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectNull:
        {
			newInstance = PDFNullDriver::GetNewInstance();
            ObjectWrap::Unwrap<PDFNullDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectName:
        {
            newInstance = PDFNameDriver::GetNewInstance();
            ObjectWrap::Unwrap<PDFNameDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectInteger:
        {
            newInstance = PDFIntegerDriver::GetNewInstance();
            ObjectWrap::Unwrap<PDFIntegerDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectReal:
        {
            newInstance = PDFRealDriver::GetNewInstance();
            ObjectWrap::Unwrap<PDFRealDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectArray:
        {
            newInstance = PDFArrayDriver::GetNewInstance();
            ObjectWrap::Unwrap<PDFArrayDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
            
        }
        case PDFObject::ePDFObjectDictionary:
        {
            newInstance = PDFDictionaryDriver::GetNewInstance();
            ObjectWrap::Unwrap<PDFDictionaryDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectIndirectObjectReference:
        {
            newInstance = PDFIndirectObjectReferenceDriver::GetNewInstance();
            ObjectWrap::Unwrap<PDFIndirectObjectReferenceDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectStream:
        {
            newInstance = PDFStreamInputDriver::GetNewInstance();
            ObjectWrap::Unwrap<PDFStreamInputDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectSymbol:
        {
            newInstance = PDFSymbolDriver::GetNewInstance();
            ObjectWrap::Unwrap<PDFSymbolDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
            
        }
    }
	return CLOSE_SCOPE(newInstance);
}

typedef BoxingBaseWithRW<long long> LongLong;

METHOD_RETURN_TYPE PDFObjectDriver::ToNumber(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() == PDFObject::ePDFObjectInteger)
    {
        SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(((PDFInteger*)anObject)->GetValue()));
    }
    else if(anObject->GetType() == PDFObject::ePDFObjectReal)
    {
        SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(((PDFReal*)anObject)->GetValue()));
        
    }
    else
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
}

METHOD_RETURN_TYPE PDFObjectDriver::ToString(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    std::string result;
    
    switch(anObject->GetType())
    {
        case PDFObject::ePDFObjectName:
            result = ((PDFName*)anObject)->GetValue();
            break;
        case PDFObject::ePDFObjectLiteralString:
            result = ((PDFLiteralString*)anObject)->GetValue();
            break;
        case PDFObject::ePDFObjectHexString:
            result = ((PDFHexString*)anObject)->GetValue();
            break;
        case PDFObject::ePDFObjectReal:
            result = Double(((PDFReal*)anObject)->GetValue()).ToString();
            break;
        case PDFObject::ePDFObjectInteger:
            result = LongLong(((PDFInteger*)anObject)->GetValue()).ToString();
            break;
        case PDFObject::ePDFObjectSymbol:
            result = ((PDFSymbol*)anObject)->GetValue();
            break;
        case PDFObject::ePDFObjectBoolean:
            result = ((PDFBoolean*)anObject)->GetValue() ? "true":"false";
            break;
        default:
            result = PDFObject::scPDFObjectTypeLabel[anObject->GetType()];
    }
    SET_FUNCTION_RETURN_VALUE(NEW_STRING(result.c_str()));

}
