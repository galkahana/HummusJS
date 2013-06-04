/*
 Source File : DocumentCopyingContextDriver.cpp
 
 
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
#include "DocumentCopyingContextDriver.h"
#include "PDFDocumentCopyingContext.h"
#include "PDFPageDriver.h"
#include "FormXObjectDriver.h"
#include "PDFReaderDriver.h"
#include "PDFObjectDriver.h"
#include "BoxingBase.h"

using namespace v8;

DocumentCopyingContextDriver::~DocumentCopyingContextDriver()
{
    delete CopyingContext;
}

DocumentCopyingContextDriver::DocumentCopyingContextDriver()
{
    // initially null, set by external pdfwriter
    CopyingContext = NULL;
}

Persistent<Function> DocumentCopyingContextDriver::constructor;

void DocumentCopyingContextDriver::Init()
{
    // prepare the context driver interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->SetClassName(String::NewSymbol("DocumentCopyingContext"));
    t->InstanceTemplate()->SetInternalFieldCount(1);
    
    t->PrototypeTemplate()->Set(String::NewSymbol("createFormXObjectFromPDFPage"),FunctionTemplate::New(CreateFormXObjectFromPDFPage)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("mergePDFPageToPage"),FunctionTemplate::New(MergePDFPageToPage)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("appendPDFPageFromPDF"),FunctionTemplate::New(AppendPDFPageFromPDF)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("mergePDFPageToFormXObject"),FunctionTemplate::New(MergePDFPageToFormXObject)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("getSourceDocumentParser"),FunctionTemplate::New(GetSourceDocumentParser)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("copyDirectObjectAsIs"),FunctionTemplate::New(CopyDirectObjectAsIs)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("copyObject"),FunctionTemplate::New(CopyObject)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("copyDirectObjectWithDeepCopy"),FunctionTemplate::New(CopyDirectObjectWithDeepCopy)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("copyNewObjectsForDirectObject"),FunctionTemplate::New(CopyNewObjectsForDirectObject)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("getCopiedObjectID"),FunctionTemplate::New(GetCopiedObjectID)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("getCopiedObjects"),FunctionTemplate::New(GetCopiedObjects)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("replaceSourceObjects"),FunctionTemplate::New(ReplaceSourceObjects)->GetFunction());

    constructor = Persistent<Function>::New(t->GetFunction());
}

Handle<Value> DocumentCopyingContextDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    Local<Object> instance = constructor->NewInstance();
    
    return scope.Close(instance);
}

Handle<Value> DocumentCopyingContextDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    DocumentCopyingContextDriver* copyingContext = new DocumentCopyingContextDriver();
    copyingContext->Wrap(args.This());
    
    return args.This();
}

v8::Handle<v8::Value> DocumentCopyingContextDriver::CreateFormXObjectFromPDFPage(const v8::Arguments& args)
{
    HandleScope scope;
    
    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		ThrowException(Exception::TypeError(String::New("copying context object not initialized, create using pdfWriter.createPDFCopyingContext")));
        return scope.Close(Undefined());
    }
    
    if(args.Length() < 2 ||
       args.Length() > 3 ||
       !args[0]->IsNumber() ||
       (!args[1]->IsNumber() && !args[1]->IsArray()) ||
       (args.Length() == 3 && !args[2]->IsArray()))
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. provide 2 or 3 arugments, where the first is a 0 based page index, and the second is a EPDFPageBox enumeration value or a 4 numbers array defining an box. a 3rd parameter may be provided to deisgnate the result form matrix")));
        return scope.Close(Undefined());
    }
 
    double matrixBuffer[6];
    double* transformationMatrix = NULL;
    
    if(args.Length() == 3)
    {
        Handle<Object> matrixArray = args[2]->ToObject();
        if(matrixArray->Get(v8::String::New("length"))->ToObject()->Uint32Value() != 6)
        {
            ThrowException(Exception::TypeError(String::New("matrix array should be 6 numbers long")));
            return scope.Close(Undefined());
        }
        
        for(int i=0;i<6;++i)
            matrixBuffer[i] = matrixArray->Get(i)->ToNumber()->Value();
        transformationMatrix = matrixBuffer;
    }
    
    
    EStatusCodeAndObjectIDType result;
    
    if(args[0]->IsNumber())
    {
        result = copyingContextDriver->CopyingContext->CreateFormXObjectFromPDFPage(args[0]->ToNumber()->Uint32Value(),
                                                                       (EPDFPageBox)args[1]->ToNumber()->Uint32Value(),
                                                                                    transformationMatrix);
    }
    else
    {
        Handle<Object> boxArray = args[1]->ToObject();
        if(boxArray->Get(v8::String::New("length"))->ToObject()->Uint32Value() != 4)
        {
            ThrowException(Exception::TypeError(String::New("box dimensions array should be 4 numbers long")));
            return scope.Close(Undefined());
        }
        
        PDFRectangle box(boxArray->Get(0)->ToNumber()->Value(),
                         boxArray->Get(1)->ToNumber()->Value(),
                         boxArray->Get(2)->ToNumber()->Value(),
                         boxArray->Get(3)->ToNumber()->Value());
        
        result = copyingContextDriver->CopyingContext->CreateFormXObjectFromPDFPage(args[0]->ToNumber()->Uint32Value(),
                                                                                    box,
                                                                                    transformationMatrix);

    }
        
    
    if(result.first != eSuccess)
    {
		ThrowException(Exception::Error(String::New("Unable to create form xobject from PDF page, parhaps the page index does not fit the total pages count")));
        return scope.Close(Undefined());
    }
    
    Local<Number> idValue = Number::New(result.second);
    
    return scope.Close(idValue);
}

v8::Handle<v8::Value> DocumentCopyingContextDriver::MergePDFPageToPage(const v8::Arguments& args)
{
    HandleScope scope;
    
    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		ThrowException(Exception::TypeError(String::New("copying context object not initialized, create using pdfWriter.createPDFCopyingContext")));
        return scope.Close(Undefined());
    }
    
    if(!args.Length() == 2 ||
       !PDFPageDriver::HasInstance(args[0]) ||
       !args[1]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. provide 2 arugments, where the first is a page, and the second is a page index to merge")));
        return scope.Close(Undefined());
    }
    
    EStatusCode status = copyingContextDriver->CopyingContext->MergePDFPageToPage(
                                                                                  ObjectWrap::Unwrap<PDFPageDriver>(args[0]->ToObject())->GetPage(),
                                                                                  args[1]->ToNumber()->Uint32Value());
    
    if(status != eSuccess)
		ThrowException(Exception::Error(String::New("Unable to merge page index to page. parhaps the page index is wrong")));
    return scope.Close(Undefined());

}

v8::Handle<v8::Value> DocumentCopyingContextDriver::AppendPDFPageFromPDF(const v8::Arguments& args)
{
    HandleScope scope;
    
    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		ThrowException(Exception::TypeError(String::New("copying context object not initialized, create using pdfWriter.createPDFCopyingContext")));
        return scope.Close(Undefined());
    }
    
    if(!args.Length() == 1 ||
       !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. provide a page index to append")));
        return scope.Close(Undefined());
    }
    
    EStatusCodeAndObjectIDType result = copyingContextDriver->CopyingContext->AppendPDFPageFromPDF(args[0]->ToNumber()->Uint32Value());
    
    if(result.first != eSuccess)
    {
		ThrowException(Exception::Error(String::New("Unable to append page. parhaps the page index is wrong")));
        return scope.Close(Undefined());
    }
    
    Local<Number> idValue = Number::New(result.second);
    return scope.Close(idValue);
    
}

Handle<Value> DocumentCopyingContextDriver::MergePDFPageToFormXObject(const Arguments& args)
{
    HandleScope scope;
    
    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		ThrowException(Exception::TypeError(String::New("copying context object not initialized, create using pdfWriter.createPDFCopyingContext")));
        return scope.Close(Undefined());
    }
    
    if(!args.Length() == 2 ||
       !FormXObjectDriver::HasInstance(args[0]) ||
       !args[1]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. provide 2 arugments, where the first is a form, and the second is a page index to merge")));
        return scope.Close(Undefined());
    }
    
    EStatusCode status = copyingContextDriver->CopyingContext->MergePDFPageToFormXObject(
                                                                                  ObjectWrap::Unwrap<FormXObjectDriver>(args[0]->ToObject())->FormXObject,
                                                                                  args[1]->ToNumber()->Uint32Value());
    
    if(status != eSuccess)
		ThrowException(Exception::Error(String::New("Unable to merge page index to form. parhaps the page index is wrong")));
    return scope.Close(Undefined());

}

Handle<Value> DocumentCopyingContextDriver::GetSourceDocumentParser(const Arguments& args)
{
    HandleScope scope;
    
    DocumentCopyingContextDriver* copyingContext = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    Handle<Value> newInstance = PDFReaderDriver::NewInstance(args);
    ObjectWrap::Unwrap<PDFReaderDriver>(newInstance->ToObject())->SetFromOwnedParser(copyingContext->CopyingContext->GetSourceDocumentParser());
    return scope.Close(newInstance);
}

Handle<Value> DocumentCopyingContextDriver::CopyDirectObjectAsIs(const Arguments& args)
{
    HandleScope scope;
    
    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		ThrowException(Exception::TypeError(String::New("copying context object not initialized, create using pdfWriter.createPDFCopyingContext or PDFWriter.createPDFCopyingContextForModifiedFile")));
        return scope.Close(Undefined());
    }
    
    if(!args.Length() == 1) // need to sometimes check that this is a PDFObject
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. provide 1 arugment, which is PDFObject to copy")));
        return scope.Close(Undefined());
    }
    
    EStatusCode status = copyingContextDriver->CopyingContext->CopyDirectObjectAsIs(ObjectWrap::Unwrap<PDFObjectDriver>(args[0]->ToObject())->GetObject());
    if(status != eSuccess)
		ThrowException(Exception::Error(String::New("Unable to merge page index to form. parhaps the page index is wrong")));
    return scope.Close(Undefined());
    
}

Handle<Value> DocumentCopyingContextDriver::CopyObject(const Arguments& args)
{
    HandleScope scope;
    
    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		ThrowException(Exception::TypeError(String::New("copying context object not initialized, create using pdfWriter.createPDFCopyingContext or PDFWriter.createPDFCopyingContextForModifiedFile")));
        return scope.Close(Undefined());
    }
    
    if(!args.Length() == 1 ||
       !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. provide 1 arugment, which is object ID of the object to copy")));
        return scope.Close(Undefined());
    }
    
    EStatusCodeAndObjectIDType result = copyingContextDriver->CopyingContext->CopyObject(args[0]->ToNumber()->Uint32Value());
 
     if(result.first != eSuccess)
		ThrowException(Exception::Error(String::New("unable to copy the object. object id may be wrong")));
     return scope.Close(Number::New(result.second));
    
}

Handle<Value> DocumentCopyingContextDriver::CopyDirectObjectWithDeepCopy(const Arguments& args)
{
    HandleScope scope;
    
    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		ThrowException(Exception::TypeError(String::New("copying context object not initialized, create using pdfWriter.createPDFCopyingContext or PDFWriter.createPDFCopyingContextForModifiedFile")));
        return scope.Close(Undefined());
    }
    
    if(!args.Length() == 1) // need to sometimes check that this is a PDFObject
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. provide 1 arugment, which is PDFObject to copy")));
        return scope.Close(Undefined());
    }
    
    EStatusCodeAndObjectIDTypeList result = copyingContextDriver->CopyingContext->CopyDirectObjectWithDeepCopy(ObjectWrap::Unwrap<PDFObjectDriver>(args[0]->ToObject())->GetObject());
    if(result.first != eSuccess)
		ThrowException(Exception::Error(String::New("Unable to copy object, parhaps the object id is wrong")));

    Local<Array> resultObjectIDs = Array::New((unsigned int)result.second.size());
    unsigned int index = 0;
    
    ObjectIDTypeList::iterator it = result.second.begin();
    for(; it != result.second.end();++it)
        resultObjectIDs->Set(Number::New(index++),Number::New(*it));
    
    return scope.Close(resultObjectIDs);
}


Handle<Value> DocumentCopyingContextDriver::CopyNewObjectsForDirectObject(const Arguments& args)
{
    HandleScope scope;
    
    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		ThrowException(Exception::TypeError(String::New("copying context object not initialized, create using pdfWriter.createPDFCopyingContext or PDFWriter.createPDFCopyingContextForModifiedFile")));
        return scope.Close(Undefined());
    }
    
    if(!args.Length() == 1 ||
       !args[0]->IsArray())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. provide 1 arugment, which is an array of object IDs")));
        return scope.Close(Undefined());
    }
    
    ObjectIDTypeList objectIDs;
    Handle<Object> objectIDsArray = args[0]->ToObject();

    unsigned int length = objectIDsArray->Get(v8::String::New("length"))->ToObject()->Uint32Value();
    
    for(unsigned int i=0;i <length;++i)
        objectIDs.push_back(objectIDsArray->Get(i)->ToNumber()->Uint32Value());
    
    EStatusCode status = copyingContextDriver->CopyingContext->CopyNewObjectsForDirectObject(objectIDs);
    if(status != eSuccess)
		ThrowException(Exception::Error(String::New("Unable to copy elements")));
    return scope.Close(Undefined());
    
}



Handle<Value> DocumentCopyingContextDriver::GetCopiedObjectID(const Arguments& args)
{
    HandleScope scope;
    
    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		ThrowException(Exception::TypeError(String::New("copying context object not initialized, create using pdfWriter.createPDFCopyingContext or PDFWriter.createPDFCopyingContextForModifiedFile")));
        return scope.Close(Undefined());
    }
    
    if(!args.Length() == 1 ||
       !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. provide 1 arugment, an object ID to check")));
        return scope.Close(Undefined());
    }
        
    EStatusCodeAndObjectIDType result = copyingContextDriver->CopyingContext->GetCopiedObjectID(args[0]->ToNumber()->Uint32Value());
    if(result.first != eSuccess)
		ThrowException(Exception::Error(String::New("Unable to find element")));
    return scope.Close(Number::New(result.second));
    
}

typedef BoxingBaseWithRW<ObjectIDType> ObjectIDTypeObject;

Handle<Value> DocumentCopyingContextDriver::GetCopiedObjects(const Arguments& args)
{
    HandleScope scope;
    
    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		ThrowException(Exception::TypeError(String::New("copying context object not initialized, create using pdfWriter.createPDFCopyingContext or PDFWriter.createPDFCopyingContextForModifiedFile")));
        return scope.Close(Undefined());
    }

    // create an object that will serve as the map

    Local<Object> result = Object::New();
    
	MapIterator<ObjectIDTypeToObjectIDTypeMap> it = copyingContextDriver->CopyingContext->GetCopiedObjectsMappingIterator();
    
    while(it.MoveNext())
        result->Set(String::New(ObjectIDTypeObject(it.GetKey()).ToString().c_str()),Number::New(it.GetValue()));
    
    return scope.Close(result);
}

Handle<Value> DocumentCopyingContextDriver::ReplaceSourceObjects(const v8::Arguments& args)
{
    // getting a dictionary mapping source to target object, translating to the C++ map...and on we go
    HandleScope scope;
    
    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		ThrowException(Exception::TypeError(String::New("copying context object not initialized, create using pdfWriter.createPDFCopyingContext or PDFWriter.createPDFCopyingContextForModifiedFile")));
        return scope.Close(Undefined());
    }
    
    if(args.Length() != 0 ||
       !args[0]->IsObject())
    {
 		ThrowException(Exception::TypeError(String::New("Wrong arguments. provide 1 arugment, which is an object mapping source object ids to map to target object IDs")));
        return scope.Close(Undefined());
    }
    
    // create an object that will serve as the map
    ObjectIDTypeToObjectIDTypeMap resultMap;
    
    Handle<Object> anObject = args[0]->ToObject();
    
    Handle<Array> objectKeys = anObject->GetOwnPropertyNames();
    
    for(unsigned long i=0; i < objectKeys->Length(); ++i)
    {
        Handle<String> key  = objectKeys->Get(Number::New(0))->ToString();
        Handle<Value> value = anObject->Get(key);
        
        resultMap.insert(ObjectIDTypeToObjectIDTypeMap::value_type(ObjectIDTypeObject(*String::Utf8Value(key)),value->ToNumber()->Uint32Value()));
        
    }
    
    copyingContextDriver->CopyingContext->ReplaceSourceObjects(resultMap);
    
    return scope.Close(Undefined());
}

