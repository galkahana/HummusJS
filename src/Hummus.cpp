/*
 Source File : Hummus
 
 
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
#include <node.h>
#include <v8.h>

#include "PDFWriterDriver.h"
#include "PDFPageDriver.h"
#include "PageContentContextDriver.h"
#include "FormXObjectDriver.h"
#include "XObjectContentContextDriver.h"
#include "ResourcesDictionaryDriver.h"
#include "UsedFontDriver.h"
#include "ImageXObjectDriver.h"
#include "ObjectsContextDriver.h"
#include "DocumentCopyingContextDriver.h"

using namespace v8;
using namespace node;

Handle<Value> CreateWriter(const Arguments& args)
{
    HandleScope scope;
    Handle<Value> instance = PDFWriterDriver::NewInstance(args);
    
    PDFWriterDriver* driver = ObjectWrap::Unwrap<PDFWriterDriver>(instance->ToObject());

    // allow version as option, defaulting to pdf1.3
	if (args.Length() < 1 || args.Length() > 2) {
		ThrowException(Exception::TypeError(String::New("Wrong number of arguments, Provide one argument stating the location of the output file, and optionally another one for setting the PDF level")));
		return scope.Close(Undefined());
	}
    
	if (!args[0]->IsString()) {
		ThrowException(Exception::TypeError(String::New("Wrong number of arguments, Provide one argument stating the location of the output file, and optionally another one for setting the PDF level")));
		return scope.Close(Undefined());
	}
    
	Local<String> stringArg = args[0]->ToString();
	String::Utf8Value utf8Path(stringArg);
    
    
    EPDFVersion pdfVersion;
    
    if(args.Length() == 2 && args[1]->IsNumber())
    {
        long pdfVersionValue = args[1]->ToNumber()->Int32Value();
        
        if(pdfVersionValue < ePDFVersion10 || ePDFVersionMax < pdfVersionValue)
        {
            ThrowException(Exception::TypeError(String::New("Wrong argument for PDF version, please provide a valid PDF version")));
            return scope.Close(Undefined());
        }
        pdfVersion = (EPDFVersion)pdfVersionValue;
    }
    else
        pdfVersion = ePDFVersion13;
    
    if(driver->StartPDF(*utf8Path, pdfVersion) != PDFHummus::eSuccess)
    {
		ThrowException(Exception::TypeError(String::New("Unable to create PDF file, make sure that output file target is available")));
		return scope.Close(Undefined());
    }
    return scope.Close(instance);

}

Handle<Value> CreateWriterToContinue(const Arguments& args)
{
    HandleScope scope;
    Handle<Value> instance = PDFWriterDriver::NewInstance(args);
    
    PDFWriterDriver* driver = ObjectWrap::Unwrap<PDFWriterDriver>(instance->ToObject());
    
	if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsString()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide 2 strings - path to file to continue, and path to state file (provided to the previous shutdown call")));
		return scope.Close(Undefined());
	}
    
    Local<String> stringArg = args[0]->ToString();
	String::Utf8Value utf8Path(stringArg);
    
    if(driver->ContinuePDF(*String::Utf8Value(args[0]->ToString()),
                        *String::Utf8Value(args[1]->ToString())) != PDFHummus::eSuccess)
    {
		ThrowException(Exception::TypeError(String::New("Unable to continue PDF file, make sure that output file target is available and state file exists")));
		return scope.Close(Undefined());
    }
    return scope.Close(instance);
}

void HummusInit(Handle<Object> exports) {
    
    PDFWriterDriver::Init();
    PDFPageDriver::Init();
    PageContentContextDriver::Init();
    FormXObjectDriver::Init();
    XObjectContentContextDriver::Init();
    ResourcesDictionaryDriver::Init();
    UsedFontDriver::Init();
    ImageXObjectDriver::Init();
    ObjectsContextDriver::Init();
    DocumentCopyingContextDriver::Init();
    
    // createWriter will create a new pdfwriter, pass a string for the target path
    exports->Set(String::NewSymbol("createWriter"),FunctionTemplate::New(CreateWriter)->GetFunction());
    exports->Set(String::NewSymbol("createWriterToContinue"),FunctionTemplate::New(CreateWriterToContinue)->GetFunction());
}

NODE_MODULE(Hummus, HummusInit)