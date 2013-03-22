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
    
    constructor = Persistent<Function>::New(pdfWriterFT->GetFunction());
}

Handle<Value> PDFWriterDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    const unsigned argc = 1;
    Handle<Value> argv[argc] = { args[0] };
    Local<Object> instance = constructor->NewInstance(argc, argv);
    
    return scope.Close(instance);
}


Handle<Value> PDFWriterDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    PDFWriterDriver* pdfWriter = new PDFWriterDriver();
    pdfWriter->Wrap(args.This());
    
    // start a new pdf based on input parameters
	if (args.Length() != 1) {
		ThrowException(Exception::TypeError(String::New("Wrong number of arguments, Provide one argument stating the location of the output file")));
		return scope.Close(Undefined());
	}
    
	if (!args[0]->IsString()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, Provide one argument stating the location of the output file")));
		return scope.Close(Undefined());
	}
    
	Local<String> stringArg = args[0]->ToString();
	String::Utf8Value utf8Path(stringArg);
    
    if(pdfWriter->mPDFWriter.StartPDF(*utf8Path, ePDFVersion13) != PDFHummus::eSuccess)
    {
		ThrowException(Exception::TypeError(String::New("Unable to create PDF file, make sure that output file target is available")));
		return scope.Close(Undefined());
    }
    
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
    
    return scope.Close(Undefined());
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
    
	if (args.Length() != 1) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide a page as the single parameter")));
		return scope.Close(Undefined());
	}
    
    PDFPageDriver* pageDriver = ObjectWrap::Unwrap<PDFPageDriver>(args[0]->ToObject());
    if(!pageDriver)
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide a page as the single parameter")));
		return scope.Close(Undefined());
    }
    
    if(pdfWriter->mPDFWriter.WritePage(pageDriver->GetPage()) != PDFHummus::eSuccess)
    {
		ThrowException(Exception::TypeError(String::New("Unable to write page")));
		return scope.Close(Undefined());
    }
    
    return scope.Close(Undefined());
    
}
