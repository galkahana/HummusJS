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
#include "ConstructorsHolder.h"

using namespace v8;

void PDFObjectDriver::Init(Local<FunctionTemplate>& ioDriverTemplate)
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
    SET_FUNCTION_RETURN_VALUE(result)
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFIndirectObjectReference(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObjectDriver* driver = ObjectWrap::Unwrap<PDFObjectDriver>(args.This());
    PDFObject* anObject = driver->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectIndirectObjectReference)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
	Local<Value> newInstance = driver->holder->GetNewPDFIndirectObjectReference();
    ObjectWrap::Unwrap<PDFIndirectObjectReferenceDriver>(newInstance->TO_OBJECT())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance)
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFArray(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObjectDriver* driver = ObjectWrap::Unwrap<PDFObjectDriver>(args.This());
    PDFObject* anObject = driver->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectArray)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
	Local<Value> newInstance = driver->holder->GetNewPDFArray();
    ObjectWrap::Unwrap<PDFArrayDriver>(newInstance->TO_OBJECT())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance)
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFDictionary(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObjectDriver* driver = ObjectWrap::Unwrap<PDFObjectDriver>(args.This());
    PDFObject* anObject = driver->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectDictionary)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
	Local<Value> newInstance = driver->holder->GetNewPDFDictionary();
    ObjectWrap::Unwrap<PDFDictionaryDriver>(newInstance->TO_OBJECT())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance)
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFStream(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObjectDriver* driver = ObjectWrap::Unwrap<PDFObjectDriver>(args.This());
    PDFObject* anObject = driver->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectStream)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
    Local<Value> newInstance = driver->holder->GetNewPDFStreamInput();
    ObjectWrap::Unwrap<PDFStreamInputDriver>(newInstance->TO_OBJECT())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance)
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFBoolean(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObjectDriver* driver = ObjectWrap::Unwrap<PDFObjectDriver>(args.This());
    PDFObject* anObject = driver->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectBoolean)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
	Local<Value> newInstance = driver->holder->GetNewPDFBoolean();
    ObjectWrap::Unwrap<PDFBooleanDriver>(newInstance->TO_OBJECT())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance)
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFLiteralString(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObjectDriver* driver = ObjectWrap::Unwrap<PDFObjectDriver>(args.This());
    PDFObject* anObject = driver->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectLiteralString)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
	Local<Value> newInstance = driver->holder->GetNewPDFLiteralString();
    ObjectWrap::Unwrap<PDFLiteralStringDriver>(newInstance->TO_OBJECT())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance)
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFHexString(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObjectDriver* driver = ObjectWrap::Unwrap<PDFObjectDriver>(args.This());
    PDFObject* anObject = driver->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectHexString)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
	Local<Value> newInstance = driver->holder->GetNewPDFHexString();
    ObjectWrap::Unwrap<PDFHexStringDriver>(newInstance->TO_OBJECT())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance)
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFNull(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObjectDriver* driver = ObjectWrap::Unwrap<PDFObjectDriver>(args.This());
    PDFObject* anObject = driver->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectNull)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
	Local<Value> newInstance = driver->holder->GetNewPDFNull();
    ObjectWrap::Unwrap<PDFNullDriver>(newInstance->TO_OBJECT())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance)
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFName(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObjectDriver* driver = ObjectWrap::Unwrap<PDFObjectDriver>(args.This());
    PDFObject* anObject = driver->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectName)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
	Local<Value> newInstance = driver->holder->GetNewPDFName();
    ObjectWrap::Unwrap<PDFNameDriver>(newInstance->TO_OBJECT())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance)
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFInteger(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObjectDriver* driver = ObjectWrap::Unwrap<PDFObjectDriver>(args.This());
    PDFObject* anObject = driver->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectInteger)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
	Local<Value> newInstance = driver->holder->GetNewPDFInteger();
    ObjectWrap::Unwrap<PDFIntegerDriver>(newInstance->TO_OBJECT())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance)
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFReal(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObjectDriver* driver = ObjectWrap::Unwrap<PDFObjectDriver>(args.This());
    PDFObject* anObject = driver->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectReal)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
    Local<Value> newInstance = driver->holder->GetNewPDFReal();
    ObjectWrap::Unwrap<PDFRealDriver>(newInstance->TO_OBJECT())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance)
}

METHOD_RETURN_TYPE PDFObjectDriver::ToPDFSymbol(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObjectDriver* driver = ObjectWrap::Unwrap<PDFObjectDriver>(args.This());
    PDFObject* anObject = driver->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectSymbol)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
    Local<Value> newInstance = driver->holder->GetNewPDFSymbol();
    ObjectWrap::Unwrap<PDFSymbolDriver>(newInstance->TO_OBJECT())->TheObject = anObject;
    SET_FUNCTION_RETURN_VALUE(newInstance)
}

typedef BoxingBaseWithRW<long long> LongLong;

METHOD_RETURN_TYPE PDFObjectDriver::ToNumber(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() == PDFObject::ePDFObjectInteger)
    {
        SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(((PDFInteger*)anObject)->GetValue()))
    }
    else if(anObject->GetType() == PDFObject::ePDFObjectReal)
    {
        SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(((PDFReal*)anObject)->GetValue()))
        
    }
    else
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
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
            result = PDFObject::scPDFObjectTypeLabel(anObject->GetType());
    }
    SET_FUNCTION_RETURN_VALUE(NEW_STRING(result.c_str()))

}
