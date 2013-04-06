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

using namespace v8;

void PDFObjectDriver::Init(Handle<FunctionTemplate>& ioDriverTemplate)
{
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("getType"),FunctionTemplate::New(GetType)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("toPDFIndirectObjectReference"),FunctionTemplate::New(ToPDFIndirectObjectReference)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("toPDFArray"),FunctionTemplate::New(ToPDFArray)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("toPDFDictionary"),FunctionTemplate::New(ToPDFDictionary)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("toPDFStream"),FunctionTemplate::New(ToPDFStream)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("toPDFBoolean"),FunctionTemplate::New(ToPDFBoolean)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("toPDFLiteralString"),FunctionTemplate::New(ToPDFLiteralString)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("toPDFHexString"),FunctionTemplate::New(ToPDFHexString)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("toPDFNull"),FunctionTemplate::New(ToPDFNull)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("toPDFName"),FunctionTemplate::New(ToPDFName)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("toPDFInteger"),FunctionTemplate::New(ToPDFInteger)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("toPDFReal"),FunctionTemplate::New(ToPDFReal)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("toPDFSymbol"),FunctionTemplate::New(ToPDFSymbol)->GetFunction());
}

Handle<Value> PDFObjectDriver::GetType(const Arguments& args)
{
    HandleScope scope;
    Local<Number> result = Number::New((unsigned long)ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject()->GetType());
    return scope.Close(result);
}

Handle<Value> PDFObjectDriver::ToPDFIndirectObjectReference(const Arguments& args)
{
    HandleScope scope;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectIndirectObjectReference)
        return scope.Close(Undefined());
    
    Handle<Value> newInstance = PDFIndirectObjectReferenceDriver::NewInstance();
    ObjectWrap::Unwrap<PDFIndirectObjectReferenceDriver>(newInstance->ToObject())->TheObject = anObject;
    return scope.Close(newInstance);
}

Handle<Value> PDFObjectDriver::ToPDFArray(const Arguments& args)
{
    HandleScope scope;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectArray)
        return scope.Close(Undefined());
    
    Handle<Value> newInstance = PDFArrayDriver::NewInstance();
    ObjectWrap::Unwrap<PDFArrayDriver>(newInstance->ToObject())->TheObject = anObject;
    return scope.Close(newInstance);
}

Handle<Value> PDFObjectDriver::ToPDFDictionary(const Arguments& args)
{
    HandleScope scope;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectDictionary)
        return scope.Close(Undefined());
    
    Handle<Value> newInstance = PDFDictionaryDriver::NewInstance();
    ObjectWrap::Unwrap<PDFDictionaryDriver>(newInstance->ToObject())->TheObject = anObject;
    return scope.Close(newInstance);
}

Handle<Value> PDFObjectDriver::ToPDFStream(const Arguments& args)
{
    HandleScope scope;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectStream)
        return scope.Close(Undefined());
    
    Handle<Value> newInstance = PDFStreamInputDriver::NewInstance();
    ObjectWrap::Unwrap<PDFStreamInputDriver>(newInstance->ToObject())->TheObject = anObject;
    return scope.Close(newInstance);
}

Handle<Value> PDFObjectDriver::ToPDFBoolean(const Arguments& args)
{
    HandleScope scope;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectBoolean)
        return scope.Close(Undefined());
    
    Handle<Value> newInstance = PDFBooleanDriver::NewInstance();
    ObjectWrap::Unwrap<PDFBooleanDriver>(newInstance->ToObject())->TheObject = anObject;
    return scope.Close(newInstance);
}

Handle<Value> PDFObjectDriver::ToPDFLiteralString(const Arguments& args)
{
    HandleScope scope;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectLiteralString)
        return scope.Close(Undefined());
    
    Handle<Value> newInstance = PDFLiteralStringDriver::NewInstance();
    ObjectWrap::Unwrap<PDFLiteralStringDriver>(newInstance->ToObject())->TheObject = anObject;
    return scope.Close(newInstance);
}

Handle<Value> PDFObjectDriver::ToPDFHexString(const Arguments& args)
{
    HandleScope scope;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectHexString)
        return scope.Close(Undefined());
    
    Handle<Value> newInstance = PDFHexStringDriver::NewInstance();
    ObjectWrap::Unwrap<PDFHexStringDriver>(newInstance->ToObject())->TheObject = anObject;
    return scope.Close(newInstance);
}

Handle<Value> PDFObjectDriver::ToPDFNull(const Arguments& args)
{
    HandleScope scope;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectNull)
        return scope.Close(Undefined());
    
    Handle<Value> newInstance = PDFNullDriver::NewInstance();
    ObjectWrap::Unwrap<PDFNullDriver>(newInstance->ToObject())->TheObject = anObject;
    return scope.Close(newInstance);
}

Handle<Value> PDFObjectDriver::ToPDFName(const Arguments& args)
{
    HandleScope scope;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectName)
        return scope.Close(Undefined());
    
    Handle<Value> newInstance = PDFNameDriver::NewInstance();
    ObjectWrap::Unwrap<PDFNameDriver>(newInstance->ToObject())->TheObject = anObject;
    return scope.Close(newInstance);
}

Handle<Value> PDFObjectDriver::ToPDFInteger(const Arguments& args)
{
    HandleScope scope;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectInteger)
        return scope.Close(Undefined());
    
    Handle<Value> newInstance = PDFIntegerDriver::NewInstance();
    ObjectWrap::Unwrap<PDFIntegerDriver>(newInstance->ToObject())->TheObject = anObject;
    return scope.Close(newInstance);
}

Handle<Value> PDFObjectDriver::ToPDFReal(const Arguments& args)
{
    HandleScope scope;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectReal)
        return scope.Close(Undefined());
    
    Handle<Value> newInstance = PDFRealDriver::NewInstance();
    ObjectWrap::Unwrap<PDFRealDriver>(newInstance->ToObject())->TheObject = anObject;
    return scope.Close(newInstance);
}

Handle<Value> PDFObjectDriver::ToPDFSymbol(const Arguments& args)
{
    HandleScope scope;
    
    PDFObject* anObject = ObjectWrap::Unwrap<PDFObjectDriver>(args.This())->GetObject();
    if(anObject->GetType() != PDFObject::ePDFObjectSymbol)
        return scope.Close(Undefined());
    
    Handle<Value> newInstance = PDFSymbolDriver::NewInstance();
    ObjectWrap::Unwrap<PDFSymbolDriver>(newInstance->ToObject())->TheObject = anObject;
    return scope.Close(newInstance);
}

Handle<Value> PDFObjectDriver::CreateDriver(PDFObject* inObject)
{
    HandleScope scope;
    Handle<Value> newInstance;
    
    switch(inObject->GetType())
    {
        case PDFObject::ePDFObjectBoolean:
        {
            newInstance = PDFBooleanDriver::NewInstance();
            ObjectWrap::Unwrap<PDFBooleanDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectLiteralString:
        {
            newInstance = PDFLiteralStringDriver::NewInstance();
            ObjectWrap::Unwrap<PDFLiteralStringDriver>(newInstance->ToObject())->TheObject = inObject;
            break;            
        }
        case PDFObject::ePDFObjectHexString:
        {
            newInstance = PDFHexStringDriver::NewInstance();
            ObjectWrap::Unwrap<PDFHexStringDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectNull:
        {
            newInstance = PDFNullDriver::NewInstance();
            ObjectWrap::Unwrap<PDFNullDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectName:
        {
            newInstance = PDFNameDriver::NewInstance();
            ObjectWrap::Unwrap<PDFNameDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectInteger:
        {
            newInstance = PDFIntegerDriver::NewInstance();
            ObjectWrap::Unwrap<PDFIntegerDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectReal:
        {
            newInstance = PDFRealDriver::NewInstance();
            ObjectWrap::Unwrap<PDFRealDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectArray:
        {
            newInstance = PDFArrayDriver::NewInstance();
            ObjectWrap::Unwrap<PDFArrayDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
            
        }
        case PDFObject::ePDFObjectDictionary:
        {
            newInstance = PDFDictionaryDriver::NewInstance();
            ObjectWrap::Unwrap<PDFDictionaryDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectIndirectObjectReference:
        {
            newInstance = PDFIndirectObjectReferenceDriver::NewInstance();
            ObjectWrap::Unwrap<PDFIndirectObjectReferenceDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectStream:
        {
            newInstance = PDFStreamInputDriver::NewInstance();
            ObjectWrap::Unwrap<PDFStreamInputDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectSymbol:
        {
            newInstance = PDFSymbolDriver::NewInstance();
            ObjectWrap::Unwrap<PDFSymbolDriver>(newInstance->ToObject())->TheObject = inObject;
            break;
            
        }
    }
    return scope.Close(newInstance);
}
