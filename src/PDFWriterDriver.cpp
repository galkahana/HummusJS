/*
 Source File : PDFWriterDriver.cpp
 
 
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
#include "PDFWriterDriver.h"
#include "PDFPageDriver.h"
#include "PageContentContextDriver.h"
#include "FormXObjectDriver.h"
#include "UsedFontDriver.h"

using namespace v8;

Persistent<Function> PDFWriterDriver::constructor;



void PDFWriterDriver::Init()
{
    // prepare the pdfwriter interfrace template
    Local<FunctionTemplate> pdfWriterFT = FunctionTemplate::New(New);
    pdfWriterFT->SetClassName(String::NewSymbol("PDFWriter"));
    pdfWriterFT->InstanceTemplate()->SetInternalFieldCount(1);
    
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("end"),FunctionTemplate::New(End)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createPage"),FunctionTemplate::New(CreatePage)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("writePage"),FunctionTemplate::New(WritePage)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("startPageContentContext"),FunctionTemplate::New(StartPageContentContext)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("pausePageContentContext"),FunctionTemplate::New(PausePageContentContext)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createFormXObject"),FunctionTemplate::New(CreateFormXObject)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("endFormXObject"),FunctionTemplate::New(EndFormXObject)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createformXObjectFromJPGFile"),FunctionTemplate::New(CreateformXObjectFromJPGFile)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("getFontForFile"),FunctionTemplate::New(GetFontForFile)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("attachURLLinktoCurrentPage"),FunctionTemplate::New(AttachURLLinktoCurrentPage)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("shutdown"),FunctionTemplate::New(Shutdown)->GetFunction());
    
    constructor = Persistent<Function>::New(pdfWriterFT->GetFunction());
}

Handle<Value> PDFWriterDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() == 1)
    {
        const unsigned argc = 1;
        Handle<Value> argv[argc] = { args[0] };
        Local<Object> instance = constructor->NewInstance(argc, argv);
        return scope.Close(instance);
    }
    else if(args.Length() == 2)
    {
        const unsigned argc = 2;
        Handle<Value> argv[argc] = {args[0],args[1]};
        Local<Object> instance = constructor->NewInstance(argc, argv);
        return scope.Close(instance);
    }
    else
        return scope.Close(Undefined());
}


Handle<Value> PDFWriterDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    PDFWriterDriver* pdfWriter = new PDFWriterDriver();
    pdfWriter->Wrap(args.This());
    
    return args.This();
}

Handle<Value> PDFWriterDriver::End(const Arguments& args)
{
    HandleScope scope;
   
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    if(pdfWriter->mPDFWriter.EndPDF() != PDFHummus::eSuccess)
    {
		ThrowException(Exception::TypeError(String::New("Unable to end PDF")));
		return scope.Close(Undefined());
    }
    
    return scope.Close(args.This());
}

Handle<Value> PDFWriterDriver::CreatePage(const Arguments& args)
{
    HandleScope scope;
    return scope.Close(PDFPageDriver::NewInstance(args));
    
}

v8::Handle<v8::Value> PDFWriterDriver::WritePage(const v8::Arguments& args)
{
    HandleScope scope;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
	if (args.Length() != 1 || !PDFPageDriver::HasInstance(args[0])) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide a page as the single parameter")));
		return scope.Close(Undefined());
	}
    
    PDFPageDriver* pageDriver = ObjectWrap::Unwrap<PDFPageDriver>(args[0]->ToObject());
    if(!pageDriver)
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide a page as the single parameter")));
		return scope.Close(Undefined());
    }
    
    if(pageDriver->ContentContext &&
       (pdfWriter->mPDFWriter.EndPageContentContext(pageDriver->ContentContext) != PDFHummus::eSuccess))
    {
		ThrowException(Exception::TypeError(String::New("Unable to finalize page context")));
		return scope.Close(Undefined());
    }
    pageDriver->ContentContext = NULL;
    
    if(pdfWriter->mPDFWriter.WritePage(pageDriver->GetPage()) != PDFHummus::eSuccess)
    {
		ThrowException(Exception::TypeError(String::New("Unable to write page")));
		return scope.Close(Undefined());
    }
    
    return scope.Close(args.This());
    
}

v8::Handle<v8::Value> PDFWriterDriver::StartPageContentContext(const Arguments& args)
{
    HandleScope scope;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
	if (args.Length() != 1 || !PDFPageDriver::HasInstance(args[0])) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide a page as the single parameter")));
		return scope.Close(Undefined());
	}
    
    PDFPageDriver* pageDriver = ObjectWrap::Unwrap<PDFPageDriver>(args[0]->ToObject());
    if(!pageDriver)
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide a page as the single parameter")));
		return scope.Close(Undefined());
    }
    
    
    Handle<Value> newInstance = PageContentContextDriver::NewInstance(args);
    PageContentContextDriver* contentContextDriver = ObjectWrap::Unwrap<PageContentContextDriver>(newInstance->ToObject());
    contentContextDriver->ContentContext = pdfWriter->mPDFWriter.StartPageContentContext(pageDriver->GetPage());
    contentContextDriver->SetResourcesDictionary(&(pageDriver->GetPage()->GetResourcesDictionary()));

    // save it also at page driver, so we can end the context when the page is written
    pageDriver->ContentContext = contentContextDriver->ContentContext;
    
    return scope.Close(newInstance);
}

v8::Handle<v8::Value> PDFWriterDriver::PausePageContentContext(const Arguments& args)
{
    HandleScope scope;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
	if (args.Length() != 1 || !PageContentContextDriver::HasInstance(args[0])) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide a page context as the single parameter")));
		return scope.Close(Undefined());
	}
    
    PageContentContextDriver* pageContextDriver = ObjectWrap::Unwrap<PageContentContextDriver>(args[0]->ToObject());
    if(!pageContextDriver)
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide a page context as the single parameter")));
		return scope.Close(Undefined());
    }
    
    if(!pageContextDriver->ContentContext)
    {
		ThrowException(Exception::TypeError(String::New("paused context not initialized, please create one using pdfWriter.startPageContentContext")));
		return scope.Close(Undefined());
    }
    
    pdfWriter->mPDFWriter.PausePageContentContext(pageContextDriver->ContentContext);
    
    return scope.Close(args.This());
}

v8::Handle<v8::Value> PDFWriterDriver::CreateFormXObject(const v8::Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 4 || !args[0]->IsNumber() || !args[1]->IsNumber() || !args[3]->IsNumber() || !args[3]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 4 coordinates of the form rectangle")));
		return scope.Close(Undefined());
    }
     
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    Handle<Value> newInstance = FormXObjectDriver::NewInstance(args);
    FormXObjectDriver* formXObjectDriver = ObjectWrap::Unwrap<FormXObjectDriver>(newInstance->ToObject());
    formXObjectDriver->FormXObject = pdfWriter->mPDFWriter.StartFormXObject(
                                                                            PDFRectangle(args[0]->ToNumber()->Value(),
                                                                                         args[1]->ToNumber()->Value(),
                                                                                         args[2]->ToNumber()->Value(),
                                                                                         args[3]->ToNumber()->Value()));
    return scope.Close(newInstance);
}

v8::Handle<v8::Value> PDFWriterDriver::EndFormXObject(const v8::Arguments& args)
{
    HandleScope scope;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
	if (args.Length() != 1 || !FormXObjectDriver::HasInstance(args[0])) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide a form as the single parameter")));
		return scope.Close(Undefined());
	}
    
    FormXObjectDriver* formContextDriver = ObjectWrap::Unwrap<FormXObjectDriver>(args[0]->ToObject());
    if(!formContextDriver)
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide a form as the single parameter")));
		return scope.Close(Undefined());
    }
        
    pdfWriter->mPDFWriter.EndFormXObject(formContextDriver->FormXObject);
    
    return scope.Close(args.This());
    
}



v8::Handle<v8::Value> PDFWriterDriver::CreateformXObjectFromJPGFile(const v8::Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 || !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is the path to the image")));
		return scope.Close(Undefined());
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    PDFFormXObject* formXObject = pdfWriter->mPDFWriter.CreateFormXObjectFromJPGFile(*String::Utf8Value(args[0]->ToString()));
    if(!formXObject)
    {
		ThrowException(Exception::Error(String::New("unable to create form xobject. verify that the target is an existing jpg file")));
		return scope.Close(Undefined());
    }
    
    Handle<Value> newInstance = FormXObjectDriver::NewInstance(args);
    ObjectWrap::Unwrap<FormXObjectDriver>(newInstance->ToObject())->FormXObject = formXObject;
    return scope.Close(newInstance);
}

v8::Handle<v8::Value> PDFWriterDriver::GetFontForFile(const v8::Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 || !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is the path to the font file")));
		return scope.Close(Undefined());
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    PDFUsedFont* usedFont = pdfWriter->mPDFWriter.GetFontForFile(*String::Utf8Value(args[0]->ToString()));
    if(!usedFont)
    {
		ThrowException(Exception::Error(String::New("unable to create font object. verify that the target is an existing and supported font type (ttf,otf,type1,dfont,ttc)")));
		return scope.Close(Undefined());
    }
    
    Handle<Value> newInstance = UsedFontDriver::NewInstance(args);
    ObjectWrap::Unwrap<UsedFontDriver>(newInstance->ToObject())->UsedFont = usedFont;
    return scope.Close(newInstance);
}

v8::Handle<v8::Value> PDFWriterDriver::AttachURLLinktoCurrentPage(const v8::Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 5 ||
        !args[0]->IsString() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber() ||
        !args[3]->IsNumber() ||
        !args[4]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass a url, and 4 numbers (left,bottom,right,top) for the rectangle valid for clicking")));
		return scope.Close(Undefined());
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    EStatusCode status = pdfWriter->mPDFWriter.AttachURLLinktoCurrentPage(*String::Utf8Value(args[0]->ToString()),
                                                                             PDFRectangle(args[1]->ToNumber()->Value(),
                                                                             args[2]->ToNumber()->Value(),
                                                                             args[3]->ToNumber()->Value(),
                                                                             args[4]->ToNumber()->Value()));
    if(status != eSuccess)
    {
		ThrowException(Exception::Error(String::New("unable to attach link to current page. will happen if the input URL may not be encoded to ascii7")));
		return scope.Close(Undefined());
    }
    
    return scope.Close(args.This());
}


v8::Handle<v8::Value> PDFWriterDriver::Shutdown(const v8::Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 ||
       !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass a path to save the state file to")));
		return scope.Close(Undefined());
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    EStatusCode status = pdfWriter->mPDFWriter.Shutdown(*String::Utf8Value(args[0]->ToString()));
    if(status != eSuccess)
    {
		ThrowException(Exception::Error(String::New("unable to save state file. verify that path is not occupied")));
		return scope.Close(Undefined());
    }
    
    return scope.Close(args.This());
}

PDFHummus::EStatusCode PDFWriterDriver::StartPDF(const std::string& inOutputFilePath,
                                EPDFVersion inPDFVersion)
{
    return mPDFWriter.StartPDF(inOutputFilePath,inPDFVersion);
}

PDFHummus::EStatusCode PDFWriterDriver::ContinuePDF(const std::string& inOutputFilePath,
                                                    const std::string& inStateFilePath)
{
    return mPDFWriter.ContinuePDF(inOutputFilePath,inStateFilePath);
}
