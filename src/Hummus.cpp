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
#include "PDFTextStringDriver.h"
#include "PDFDateDriver.h"
#include "ETokenSeparator.h"
#include "DictionaryContextDriver.h"
#include "PDFPageInputDriver.h"
#include "InputFileDriver.h"
#include "OutputFileDriver.h"
#include "DocumentContextDriver.h"
#include "InfoDictionaryDriver.h"
#include "ObjectByteWriterWithPosition.h"
#include "IByteReaderWithPosition.h"
#include "ObjectByteReaderWithPosition.h"
#include "ObjectByteWriter.h"

using namespace v8;
using namespace node;

Handle<Value> CreateWriter(const Arguments& args)
{
    HandleScope scope;
    Handle<Value> instance = PDFWriterDriver::NewInstance(args);
    
    PDFWriterDriver* driver = ObjectWrap::Unwrap<PDFWriterDriver>(instance->ToObject());

	if (args.Length() < 1 || args.Length() > 2) {
		ThrowException(Exception::TypeError(String::New("Wrong number of arguments, Provide one argument stating the location of the output file, and an optional options object")));
		return scope.Close(Undefined());
	}
    
	if (!args[0]->IsString() && !args[0]->IsObject()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, please provide a path to a file as the first argument or a stream object")));
		return scope.Close(Undefined());
	}
    
    EPDFVersion pdfVersion = ePDFVersion13;
    bool compressStreams = true;
    LogConfiguration logConfig = LogConfiguration::DefaultLogConfiguration;
    
    if(args.Length() == 2 && args[1]->IsObject())
    {
        Handle<Object> anObject = args[1]->ToObject();
        if(anObject->Has(String::New("version")) && anObject->Get(String::New("version"))->IsNumber())
        {
            long pdfVersionValue = anObject->Get(String::New("version"))->ToNumber()->Int32Value();
            
            if(pdfVersionValue < ePDFVersion10 || ePDFVersionMax < pdfVersionValue)
            {
                ThrowException(Exception::TypeError(String::New("Wrong argument for PDF version, please provide a valid PDF version")));
                return scope.Close(Undefined());
            }
            pdfVersion = (EPDFVersion)pdfVersionValue;
        }
            
        if(anObject->Has(String::New("compress")) && anObject->Get(String::New("compress"))->IsBoolean())
            compressStreams = anObject->Get(String::New("compress"))->ToBoolean()->Value();

        if(anObject->Has(String::New("log")) && anObject->Get(String::New("log"))->IsString())
        {
            logConfig.ShouldLog = true;
            logConfig.LogFileLocation = *String::Utf8Value(anObject->Get(String::New("log"))->ToString());
        }

    }
    
    EStatusCode status;
    
    if(args[0]->IsObject())
    {
        status = driver->StartPDF(args[0]->ToObject(), pdfVersion,logConfig,PDFCreationSettings(compressStreams));
    }
    else
    {
        status = driver->StartPDF(*String::Utf8Value(args[0]->ToString()), pdfVersion,logConfig,PDFCreationSettings(compressStreams));
    }
    
    if(status != PDFHummus::eSuccess)
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
    
	if ((args.Length() != 2  && args.Length() !=3)||
            (!args[0]->IsString() && !args[0]->IsObject()) ||
            !args[1]->IsString() ||
            ((args.Length() == 3) && !args[2]->IsObject())) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide 2 strings - path to file to continue, and path to state file (provided to the previous shutdown call. You may also add an options object")));
		return scope.Close(Undefined());
	}
    
    std::string alternativePath;
    Handle<Object> alternativeStream;
    LogConfiguration logConfig = LogConfiguration::DefaultLogConfiguration;
   
    if(args.Length() == 2 && args[1]->IsObject())
    {
        Handle<Object> anObject = args[1]->ToObject();
        
        if(anObject->Has(String::New("modifiedFilePath")) && anObject->Get(String::New("modifiedFilePath"))->IsString())
            alternativePath = *String::Utf8Value(anObject->Get(String::New("modifiedFilePath"))->ToString());

        if(anObject->Has(String::New("modifiedStream")) && anObject->Get(String::New("modifiedStream"))->IsObject())
            alternativeStream = anObject->Get(String::New("modifiedStream"))->ToObject();
        
        
        if(anObject->Has(String::New("log")))
        {
            Handle<Value> value = anObject->Get(String::New("log"));
            if(value->IsString())
            {
                logConfig.ShouldLog = true;
                logConfig.LogFileLocation = *String::Utf8Value(anObject->Get(String::New("log"))->ToString());
                logConfig.LogStream = NULL;
            }
            else if(value->IsObject())
            {
                logConfig.ShouldLog = true;
                logConfig.LogFileLocation = "";
                ObjectByteWriter proxy(value->ToObject());
                logConfig.LogStream = &proxy;
            }
        }
    }
    
    EStatusCode status;
    
    if(args[0]->IsObject())
    {
        status = driver->ContinuePDF(args[0]->ToObject(),
                                     *String::Utf8Value(args[1]->ToString()),
                                     alternativeStream,
                                     logConfig);
    }
    else
    {
        status = driver->ContinuePDF(*String::Utf8Value(args[0]->ToString()),
                                             *String::Utf8Value(args[1]->ToString()),
                                             alternativePath,
                                             logConfig);
    }
    
    if(status != PDFHummus::eSuccess)
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
    
    if(args.Length() < 1 ||
       (!args[0]->IsString() && !args[0]->IsObject()) ||
       (args[0]->IsString() && args.Length() > 2) ||
       (args[0]->IsObject() && (!args[1]->IsObject() || args.Length() > 3)))
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, please path a path to modified file, or a pair of stream - first for the source, and second for destination. in addition you can optionally add an options object")));
		return scope.Close(Undefined());
	}
    
    
    EPDFVersion pdfVersion = ePDFVersion10;
    bool compressStreams = true;
    std::string alternativePath;
    Handle<Value> alternativeStream;
    LogConfiguration logConfig = LogConfiguration::DefaultLogConfiguration;
    
    int optionsObjectIndex = args[0]->IsString() ? 1:2;
    
    if(args.Length() == (optionsObjectIndex+1) && args[optionsObjectIndex]->IsObject())
    {
        Handle<Object> anObject = args[optionsObjectIndex]->ToObject();
        if(anObject->Has(String::New("version")) && anObject->Get(String::New("version"))->IsString())
        {
            long pdfVersionValue = anObject->Get(String::New("version"))->ToNumber()->Int32Value();
            
            if(pdfVersionValue < ePDFVersion10 || ePDFVersionMax < pdfVersionValue)
            {
                ThrowException(Exception::TypeError(String::New("Wrong argument for PDF version, please provide a valid PDF version")));
                return scope.Close(Undefined());
            }
            pdfVersion = (EPDFVersion)pdfVersionValue;
        }
        
        if(anObject->Has(String::New("compress")) && anObject->Get(String::New("compress"))->IsBoolean())
            compressStreams = anObject->Get(String::New("compress"))->ToBoolean()->Value();
        
        if(anObject->Has(String::New("modifiedFilePath")) && anObject->Get(String::New("modifiedFilePath"))->IsString())
            alternativePath = *String::Utf8Value(anObject->Get(String::New("modifiedFilePath"))->ToString());

        if(anObject->Has(String::New("log")) && anObject->Get(String::New("log"))->IsString())
        {
            logConfig.ShouldLog = true;
            logConfig.LogFileLocation = *String::Utf8Value(anObject->Get(String::New("log"))->ToString());
        }
    }
    
    EStatusCode status;
    
    if(args[0]->IsObject())
    {
        status = driver->ModifyPDF(args[0]->ToObject(),
                                   args[1]->ToObject(),
                                   pdfVersion,
                                   logConfig,
                                   PDFCreationSettings(compressStreams));
    }
    else
    {
        status = driver->ModifyPDF(*String::Utf8Value(args[0]->ToString()),
                               pdfVersion,alternativePath,logConfig,
                               PDFCreationSettings(compressStreams));
    }
    
    if(status != PDFHummus::eSuccess)
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
    
	if (args.Length() != 1 || (!args[0]->IsString() && !args[0]->IsObject()))
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide 1 string - path to file read, or a read stream object")));
		return scope.Close(Undefined());
	}
        
    PDFHummus::EStatusCode status;
        
    if(args[0]->IsObject())
        status = driver->StartPDFParsing(args[0]->ToObject());
    else
        status = driver->StartPDFParsing(*String::Utf8Value(args[0]->ToString()));
        
    if(status != PDFHummus::eSuccess)
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
    PDFPageDriver::Init(exports);
    PageContentContextDriver::Init();
    FormXObjectDriver::Init();
    XObjectContentContextDriver::Init();
    ResourcesDictionaryDriver::Init();
    UsedFontDriver::Init();
    ImageXObjectDriver::Init();
    ObjectsContextDriver::Init();
    DocumentContextDriver::Init();
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
    PDFTextStringDriver::Init(exports);
    PDFDateDriver::Init(exports);
    DictionaryContextDriver::Init();
    PDFPageInputDriver::Init();
    InputFileDriver::Init(exports);
    OutputFileDriver::Init(exports);
    InfoDictionaryDriver::Init();
    
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
    
    // ETokenSeparator
    exports->Set(String::NewSymbol("eTokenSeparatorSpace"),Number::New(eTokenSeparatorSpace));
    exports->Set(String::NewSymbol("eTokenSeparatorEndLine"),Number::New(eTokenSeparatorEndLine));
    exports->Set(String::NewSymbol("eTokenSepratorNone"),Number::New(eTokenSepratorNone));
    
    // EXrefEntryType
    exports->Set(String::NewSymbol("eXrefEntryExisting"),Number::New(eXrefEntryExisting));
    exports->Set(String::NewSymbol("eXrefEntryDelete"),Number::New(eXrefEntryDelete));
    exports->Set(String::NewSymbol("eXrefEntryStreamObject"),Number::New(eXrefEntryStreamObject));
    exports->Set(String::NewSymbol("eXrefEntryUndefined"),Number::New(eXrefEntryUndefined));
    
    // EInfoTrapped
    exports->Set(String::NewSymbol("EInfoTrappedTrue"),Number::New(EInfoTrappedTrue));
    exports->Set(String::NewSymbol("EInfoTrappedFalse"),Number::New(EInfoTrappedFalse));
    exports->Set(String::NewSymbol("EInfoTrappedUnknown"),Number::New(EInfoTrappedUnknown));
}

NODE_MODULE(Hummus, HummusInit)