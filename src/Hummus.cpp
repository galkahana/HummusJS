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
#include "PDFReaderDriver.h"
#include "PDFArrayDriver.h"
#include "PDFDictionaryDriver.h"
#include "PDFStreamInputDriver.h"
#include "PDFIndirectObjectReferenceDriver.h"
#include "PDFBooleanDriver.h"
#include "PDFLiteralStringDriver.h"
#include "PDFHexStringDriver.h"
#include "PDFNullDriver.h"
#include "PDFNameDriver.h"
#include "PDFIntegerDriver.h"
#include "PDFRealDriver.h"
#include "PDFSymbolDriver.h"
#include "ProcsetResourcesConstants.h"
#include "EPDFVersion.h"
#include "PDFEmbedParameterTypes.h"
#include "PDFObject.h"

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
    
    if(driver->ContinuePDF(*String::Utf8Value(args[0]->ToString()),
                        *String::Utf8Value(args[1]->ToString())) != PDFHummus::eSuccess)
    {
		ThrowException(Exception::TypeError(String::New("Unable to continue PDF file, make sure that output file target is available and state file exists")));
		return scope.Close(Undefined());
    }
    return scope.Close(instance);
}


Handle<Value> CreateWriterToModify(const Arguments& args)
{
    HandleScope scope;
    Handle<Value> instance = PDFWriterDriver::NewInstance(args);
    
    PDFWriterDriver* driver = ObjectWrap::Unwrap<PDFWriterDriver>(instance->ToObject());
    
	if ((args.Length() != 1 && args.Length() != 2) ||
        !args[0]->IsString() ||
        (args.Length() == 2 && !args[1]->IsString()))
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide 1 or 2 strings. the first string is the file to modify. the modification may be in place, or that the modified PDF is written to another PDF, in the case that a 2 parameter is provide for the target folder")));
		return scope.Close(Undefined());
	}
    
    std::string targetFilePath =
        args.Length() == 1 ? "" : *String::Utf8Value(args[1]->ToString());
    
    if(driver->ModifyPDF(*String::Utf8Value(args[0]->ToString()),
                           targetFilePath) != PDFHummus::eSuccess)
    {
		ThrowException(Exception::TypeError(String::New("Unable to modify PDF file, make sure that output file target is available and that it is not protected")));
		return scope.Close(Undefined());
    }
    return scope.Close(instance);
}


Handle<Value> CreateReader(const Arguments& args)
{
    HandleScope scope;
    Handle<Value> instance = PDFReaderDriver::NewInstance(args);
    
    PDFReaderDriver* driver = ObjectWrap::Unwrap<PDFReaderDriver>(instance->ToObject());
    
	if (args.Length() != 1 || !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide 1 string - path to file read")));
		return scope.Close(Undefined());
	}
    
    Local<String> stringArg = args[0]->ToString();
	String::Utf8Value utf8Path(stringArg);
    
    if(driver->StartPDFParsing(*String::Utf8Value(args[0]->ToString())) != PDFHummus::eSuccess)
    {
		ThrowException(Exception::Error(String::New("Unable to start parsing PDF file")));
		return scope.Close(Undefined());
    }
    return scope.Close(instance);
}

Handle<Value> GetTypeLabel(const Arguments& args)
{
    HandleScope scope;
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide a single enumerator value of a PDF Object type")));
		return scope.Close(Undefined());
	}
    
    unsigned long value = args[0]->ToNumber()->Uint32Value();
    
    if(value > PDFObject::ePDFObjectSymbol)
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide a single enumerator value of a PDF Object type")));
		return scope.Close(Undefined());
    }
    
    Handle<Value> result = String::New(PDFObject::scPDFObjectTypeLabel[(PDFObject::EPDFObjectType)value]);
    
    return scope.Close(result);
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
    PDFReaderDriver::Init();
    PDFArrayDriver::Init();
    PDFDictionaryDriver::Init();
    PDFStreamInputDriver::Init();
    PDFIndirectObjectReferenceDriver::Init();
    PDFBooleanDriver::Init();
    PDFLiteralStringDriver::Init();
    PDFHexStringDriver::Init();
    PDFNullDriver::Init();
    PDFNameDriver::Init();
    PDFIntegerDriver::Init();
    PDFRealDriver::Init();
    PDFSymbolDriver::Init();
    
    // define methods
    exports->Set(String::NewSymbol("createWriter"),FunctionTemplate::New(CreateWriter)->GetFunction());
    exports->Set(String::NewSymbol("createWriterToContinue"),FunctionTemplate::New(CreateWriterToContinue)->GetFunction());
    exports->Set(String::NewSymbol("createWriterToModify"),FunctionTemplate::New(CreateWriterToModify)->GetFunction());
    exports->Set(String::NewSymbol("createReader"),FunctionTemplate::New(CreateReader)->GetFunction());
    
    // define pdf versions enum
    exports->Set(String::NewSymbol("ePDFVersion10"),Number::New(ePDFVersion10));
    exports->Set(String::NewSymbol("ePDFVersion11"),Number::New(ePDFVersion11));
    exports->Set(String::NewSymbol("ePDFVersion12"),Number::New(ePDFVersion12));
    exports->Set(String::NewSymbol("ePDFVersion13"),Number::New(ePDFVersion13));
    exports->Set(String::NewSymbol("ePDFVersion14"),Number::New(ePDFVersion14));
    exports->Set(String::NewSymbol("ePDFVersion15"),Number::New(ePDFVersion15));
    exports->Set(String::NewSymbol("ePDFVersion16"),Number::New(ePDFVersion16));
    exports->Set(String::NewSymbol("ePDFVersion17"),Number::New(ePDFVersion17));
    
    // procsets for resource inclusion
    exports->Set(String::NewSymbol("KProcsetImageB"),String::New(KProcsetImageB.c_str()));
    exports->Set(String::NewSymbol("KProcsetImageC"),String::New(KProcsetImageC.c_str()));
    exports->Set(String::NewSymbol("KProcsetImageI"),String::New(KProcsetImageI.c_str()));
    exports->Set(String::NewSymbol("kProcsetPDF"),String::New(KProcsetPDF.c_str()));
    exports->Set(String::NewSymbol("kProcsetText"),String::New(KProcsetText.c_str()));
    
    // page range values
    exports->Set(String::NewSymbol("eRangeTypeAll"),Number::New(PDFPageRange::eRangeTypeAll));
    exports->Set(String::NewSymbol("eRangeTypeSpecific"),Number::New(PDFPageRange::eRangeTypeSpecific));
    
    // EPDFPageBox enumerator, for embedding pages into forms
    exports->Set(String::NewSymbol("ePDFPageBoxMediaBox"),Number::New(ePDFPageBoxMediaBox));
    exports->Set(String::NewSymbol("ePDFPageBoxCropBox"),Number::New(ePDFPageBoxCropBox));
    exports->Set(String::NewSymbol("ePDFPageBoxBleedBox"),Number::New(ePDFPageBoxBleedBox));
    exports->Set(String::NewSymbol("ePDFPageBoxTrimBox"),Number::New(ePDFPageBoxTrimBox));
    exports->Set(String::NewSymbol("ePDFPageBoxArtBox"),Number::New(ePDFPageBoxArtBox));
    
    // parsed object types
    exports->Set(String::NewSymbol("ePDFObjectBoolean"),Number::New(PDFObject::ePDFObjectBoolean));
    exports->Set(String::NewSymbol("ePDFObjectLiteralString"),Number::New(PDFObject::ePDFObjectLiteralString));
    exports->Set(String::NewSymbol("ePDFObjectHexString"),Number::New(PDFObject::ePDFObjectHexString));
    exports->Set(String::NewSymbol("ePDFObjectNull"),Number::New(PDFObject::ePDFObjectNull));
    exports->Set(String::NewSymbol("ePDFObjectName"),Number::New(PDFObject::ePDFObjectName));
    exports->Set(String::NewSymbol("ePDFObjectInteger"),Number::New(PDFObject::ePDFObjectInteger));
    exports->Set(String::NewSymbol("ePDFObjectReal"),Number::New(PDFObject::ePDFObjectReal));
    exports->Set(String::NewSymbol("ePDFObjectArray"),Number::New(PDFObject::ePDFObjectArray));
    exports->Set(String::NewSymbol("ePDFObjectDictionary"),Number::New(PDFObject::ePDFObjectDictionary));
    exports->Set(String::NewSymbol("ePDFObjectIndirectObjectReference"),Number::New(PDFObject::ePDFObjectIndirectObjectReference));
    exports->Set(String::NewSymbol("ePDFObjectStream"),Number::New(PDFObject::ePDFObjectStream));
    exports->Set(String::NewSymbol("ePDFObjectSymbol"),Number::New(PDFObject::ePDFObjectSymbol));
    // getter for string represenation of type enum
    exports->Set(String::NewSymbol("getTypeLabel"),FunctionTemplate::New(GetTypeLabel)->GetFunction());
}

NODE_MODULE(Hummus, HummusInit)