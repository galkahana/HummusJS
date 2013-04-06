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

    constructor = Persistent<Function>::New(t->GetFunction());
}

Handle<Value> DocumentCopyingContextDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    const unsigned argc = 0;
    Handle<Value> argv[argc];
    Local<Object> instance = constructor->NewInstance(argc, argv);
    
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
    
    if(!args.Length() == 2 ||
       !args[0]->IsNumber() ||
       !args[1]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. provide 2 arugments, where the first is a 0 based page index, and the second is a EPDFPageBox enumeration value")));
        return scope.Close(Undefined());
    }
    
    EStatusCodeAndObjectIDType result = copyingContextDriver->CopyingContext->CreateFormXObjectFromPDFPage(args[0]->ToNumber()->Uint32Value(),
                                                                       (EPDFPageBox)args[1]->ToNumber()->Uint32Value());
    
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


