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
#include "PDFStreamDriver.h"

using namespace v8;
using namespace node;

METHOD_RETURN_TYPE CreateWriter(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    Handle<Value> instance = PDFWriterDriver::GetNewInstance(args);
    
    PDFWriterDriver* driver = ObjectWrap::Unwrap<PDFWriterDriver>(instance->ToObject());

	if (args.Length() < 1 || args.Length() > 2) {
		THROW_EXCEPTION("Wrong number of arguments, Provide one argument stating the location of the output file, and an optional options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
	}
    
	if (!args[0]->IsString() && !args[0]->IsObject()) {
		THROW_EXCEPTION("Wrong arguments, please provide a path to a file as the first argument or a stream object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
	}
    
    EPDFVersion pdfVersion = ePDFVersion13;
    bool compressStreams = true;
    LogConfiguration logConfig = LogConfiguration::DefaultLogConfiguration;
    
    if(args.Length() == 2 && args[1]->IsObject())
    {
        Handle<Object> anObject = args[1]->ToObject();
        if(anObject->Has(NEW_STRING("version")) && anObject->Get(NEW_STRING("version"))->IsNumber())
        {
            long pdfVersionValue = anObject->Get(NEW_STRING("version"))->ToNumber()->Int32Value();
            
            if(pdfVersionValue < ePDFVersion10 || ePDFVersionMax < pdfVersionValue)
            {
                THROW_EXCEPTION("Wrong argument for PDF version, please provide a valid PDF version");
                SET_FUNCTION_RETURN_VALUE(UNDEFINED);
            }
            pdfVersion = (EPDFVersion)pdfVersionValue;
        }
            
        if(anObject->Has(NEW_STRING("compress")) && anObject->Get(NEW_STRING("compress"))->IsBoolean())
            compressStreams = anObject->Get(NEW_STRING("compress"))->ToBoolean()->Value();

        if(anObject->Has(NEW_STRING("log")) && anObject->Get(NEW_STRING("log"))->IsString())
        {
            logConfig.ShouldLog = true;
            logConfig.LogFileLocation = *String::Utf8Value(anObject->Get(NEW_STRING("log"))->ToString());
        }

    }
    
    EStatusCode status;
    
    if(args[0]->IsObject())
    {
        status = driver->StartPDF(args[0]->ToObject(), pdfVersion,logConfig,PDFCreationSettings(compressStreams,true));
    }
    else
    {
        status = driver->StartPDF(*String::Utf8Value(args[0]->ToString()), pdfVersion,logConfig,PDFCreationSettings(compressStreams,true));
    }
    
    if(status != PDFHummus::eSuccess)
    {
		THROW_EXCEPTION("Unable to create PDF file, make sure that output file target is available");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    SET_FUNCTION_RETURN_VALUE(instance);

}

METHOD_RETURN_TYPE CreateWriterToContinue(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    Handle<Value> instance = PDFWriterDriver::GetNewInstance(args);
    
    PDFWriterDriver* driver = ObjectWrap::Unwrap<PDFWriterDriver>(instance->ToObject());
    
	if ((args.Length() != 2  && args.Length() !=3)||
            (!args[0]->IsString() && !args[0]->IsObject()) ||
            !args[1]->IsString() ||
            ((args.Length() == 3) && !args[2]->IsObject())) {
		THROW_EXCEPTION("Wrong arguments, provide 2 strings - path to file to continue, and path to state file (provided to the previous shutdown call. You may also add an options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
	}
    
    std::string alternativePath;
    Handle<Object> alternativeStream;
    LogConfiguration logConfig = LogConfiguration::DefaultLogConfiguration;
   
    if(args.Length() == 2 && args[1]->IsObject())
    {
        Handle<Object> anObject = args[1]->ToObject();
        
        if(anObject->Has(NEW_STRING("modifiedFilePath")) && anObject->Get(NEW_STRING("modifiedFilePath"))->IsString())
            alternativePath = *String::Utf8Value(anObject->Get(NEW_STRING("modifiedFilePath"))->ToString());

        if(anObject->Has(NEW_STRING("modifiedStream")) && anObject->Get(NEW_STRING("modifiedStream"))->IsObject())
            alternativeStream = anObject->Get(NEW_STRING("modifiedStream"))->ToObject();
        
        
        if(anObject->Has(NEW_STRING("log")))
        {
            Handle<Value> value = anObject->Get(NEW_STRING("log"));
            if(value->IsString())
            {
                logConfig.ShouldLog = true;
                logConfig.LogFileLocation = *String::Utf8Value(anObject->Get(NEW_STRING("log"))->ToString());
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
		THROW_EXCEPTION("Unable to continue PDF file, make sure that output file target is available and state file exists");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    SET_FUNCTION_RETURN_VALUE(instance);
}


METHOD_RETURN_TYPE CreateWriterToModify(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    Handle<Value> instance = PDFWriterDriver::GetNewInstance(args);
    
    PDFWriterDriver* driver = ObjectWrap::Unwrap<PDFWriterDriver>(instance->ToObject());
    
    if(args.Length() < 1 ||
       (!args[0]->IsString() && !args[0]->IsObject()) ||
       (args[0]->IsString() && args.Length() > 2) ||
       (args[0]->IsObject() && (!args[1]->IsObject() || args.Length() > 3)))
    {
		THROW_EXCEPTION("Wrong arguments, please path a path to modified file, or a pair of stream - first for the source, and second for destination. in addition you can optionally add an options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
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
        if(anObject->Has(NEW_STRING("version")) && anObject->Get(NEW_STRING("version"))->IsString())
        {
            long pdfVersionValue = anObject->Get(NEW_STRING("version"))->ToNumber()->Int32Value();
            
            if(pdfVersionValue < ePDFVersion10 || ePDFVersionMax < pdfVersionValue)
            {
                THROW_EXCEPTION("Wrong argument for PDF version, please provide a valid PDF version");
                SET_FUNCTION_RETURN_VALUE(UNDEFINED);
            }
            pdfVersion = (EPDFVersion)pdfVersionValue;
        }
        
        if(anObject->Has(NEW_STRING("compress")) && anObject->Get(NEW_STRING("compress"))->IsBoolean())
            compressStreams = anObject->Get(NEW_STRING("compress"))->ToBoolean()->Value();
        
        if(anObject->Has(NEW_STRING("modifiedFilePath")) && anObject->Get(NEW_STRING("modifiedFilePath"))->IsString())
            alternativePath = *String::Utf8Value(anObject->Get(NEW_STRING("modifiedFilePath"))->ToString());

        if(anObject->Has(NEW_STRING("log")) && anObject->Get(NEW_STRING("log"))->IsString())
        {
            logConfig.ShouldLog = true;
            logConfig.LogFileLocation = *String::Utf8Value(anObject->Get(NEW_STRING("log"))->ToString());
        }
    }
    
    EStatusCode status;
    
    if(args[0]->IsObject())
    {
        status = driver->ModifyPDF(args[0]->ToObject(),
                                   args[1]->ToObject(),
                                   pdfVersion,
                                   logConfig,
                                   PDFCreationSettings(compressStreams,true));
    }
    else
    {
        status = driver->ModifyPDF(*String::Utf8Value(args[0]->ToString()),
                               pdfVersion,alternativePath,logConfig,
                               PDFCreationSettings(compressStreams,true));
    }
    
    if(status != PDFHummus::eSuccess)
    {
		THROW_EXCEPTION("Unable to modify PDF file, make sure that output file target is available and that it is not protected");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    SET_FUNCTION_RETURN_VALUE(instance);
}


METHOD_RETURN_TYPE CreateReader(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    Handle<Value> instance = PDFReaderDriver::GetNewInstance(args);
    
    PDFReaderDriver* driver = ObjectWrap::Unwrap<PDFReaderDriver>(instance->ToObject());
    
	if (args.Length() != 1 || (!args[0]->IsString() && !args[0]->IsObject()))
    {
		THROW_EXCEPTION("Wrong arguments, provide 1 string - path to file read, or a read stream object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
	}
        
    PDFHummus::EStatusCode status;
        
    if(args[0]->IsObject())
        status = driver->StartPDFParsing(args[0]->ToObject());
    else
        status = driver->StartPDFParsing(*String::Utf8Value(args[0]->ToString()));
        
    if(status != PDFHummus::eSuccess)
    {
		THROW_EXCEPTION("Unable to start parsing PDF file");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    SET_FUNCTION_RETURN_VALUE(instance);
}

METHOD_RETURN_TYPE GetTypeLabel(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong arguments, provide a single enumerator value of a PDF Object type");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
	}
    
    unsigned long value = args[0]->ToNumber()->Uint32Value();
    
    if(value > PDFObject::ePDFObjectSymbol)
    {
		THROW_EXCEPTION("Wrong arguments, provide a single enumerator value of a PDF Object type");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    Handle<Value> result = NEW_STRING(PDFObject::scPDFObjectTypeLabel[(PDFObject::EPDFObjectType)value]);
    
    SET_FUNCTION_RETURN_VALUE(result);
}

void HummusInit(Handle<Object> exports) {
	CREATE_ISOLATE_CONTEXT;

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
    PDFStreamDriver::Init();
    
    // define methods
	exports->Set(NEW_SYMBOL("createWriter"), NEW_FUNCTION_TEMPLATE(CreateWriter)->GetFunction());
	exports->Set(NEW_SYMBOL("createWriterToContinue"), NEW_FUNCTION_TEMPLATE(CreateWriterToContinue)->GetFunction());
	exports->Set(NEW_SYMBOL("createWriterToModify"), NEW_FUNCTION_TEMPLATE(CreateWriterToModify)->GetFunction());
	exports->Set(NEW_SYMBOL("createReader"), NEW_FUNCTION_TEMPLATE(CreateReader)->GetFunction());
    
    // define pdf versions enum
    exports->Set(NEW_SYMBOL("ePDFVersion10"),NEW_NUMBER(ePDFVersion10));
    exports->Set(NEW_SYMBOL("ePDFVersion11"),NEW_NUMBER(ePDFVersion11));
    exports->Set(NEW_SYMBOL("ePDFVersion12"),NEW_NUMBER(ePDFVersion12));
    exports->Set(NEW_SYMBOL("ePDFVersion13"),NEW_NUMBER(ePDFVersion13));
    exports->Set(NEW_SYMBOL("ePDFVersion14"),NEW_NUMBER(ePDFVersion14));
    exports->Set(NEW_SYMBOL("ePDFVersion15"),NEW_NUMBER(ePDFVersion15));
    exports->Set(NEW_SYMBOL("ePDFVersion16"),NEW_NUMBER(ePDFVersion16));
    exports->Set(NEW_SYMBOL("ePDFVersion17"),NEW_NUMBER(ePDFVersion17));
    
    // procsets for resource inclusion
    exports->Set(NEW_SYMBOL("KProcsetImageB"),NEW_STRING(KProcsetImageB.c_str()));
    exports->Set(NEW_SYMBOL("KProcsetImageC"),NEW_STRING(KProcsetImageC.c_str()));
    exports->Set(NEW_SYMBOL("KProcsetImageI"),NEW_STRING(KProcsetImageI.c_str()));
    exports->Set(NEW_SYMBOL("kProcsetPDF"),NEW_STRING(KProcsetPDF.c_str()));
    exports->Set(NEW_SYMBOL("kProcsetText"),NEW_STRING(KProcsetText.c_str()));
    
    // page range values
    exports->Set(NEW_SYMBOL("eRangeTypeAll"),NEW_NUMBER(PDFPageRange::eRangeTypeAll));
    exports->Set(NEW_SYMBOL("eRangeTypeSpecific"),NEW_NUMBER(PDFPageRange::eRangeTypeSpecific));
    
    // EPDFPageBox enumerator, for embedding pages into forms
    exports->Set(NEW_SYMBOL("ePDFPageBoxMediaBox"),NEW_NUMBER(ePDFPageBoxMediaBox));
    exports->Set(NEW_SYMBOL("ePDFPageBoxCropBox"),NEW_NUMBER(ePDFPageBoxCropBox));
    exports->Set(NEW_SYMBOL("ePDFPageBoxBleedBox"),NEW_NUMBER(ePDFPageBoxBleedBox));
    exports->Set(NEW_SYMBOL("ePDFPageBoxTrimBox"),NEW_NUMBER(ePDFPageBoxTrimBox));
    exports->Set(NEW_SYMBOL("ePDFPageBoxArtBox"),NEW_NUMBER(ePDFPageBoxArtBox));
    
    // parsed object types
    exports->Set(NEW_SYMBOL("ePDFObjectBoolean"),NEW_NUMBER(PDFObject::ePDFObjectBoolean));
    exports->Set(NEW_SYMBOL("ePDFObjectLiteralString"),NEW_NUMBER(PDFObject::ePDFObjectLiteralString));
    exports->Set(NEW_SYMBOL("ePDFObjectHexString"),NEW_NUMBER(PDFObject::ePDFObjectHexString));
    exports->Set(NEW_SYMBOL("ePDFObjectNull"),NEW_NUMBER(PDFObject::ePDFObjectNull));
    exports->Set(NEW_SYMBOL("ePDFObjectName"),NEW_NUMBER(PDFObject::ePDFObjectName));
    exports->Set(NEW_SYMBOL("ePDFObjectInteger"),NEW_NUMBER(PDFObject::ePDFObjectInteger));
    exports->Set(NEW_SYMBOL("ePDFObjectReal"),NEW_NUMBER(PDFObject::ePDFObjectReal));
    exports->Set(NEW_SYMBOL("ePDFObjectArray"),NEW_NUMBER(PDFObject::ePDFObjectArray));
    exports->Set(NEW_SYMBOL("ePDFObjectDictionary"),NEW_NUMBER(PDFObject::ePDFObjectDictionary));
    exports->Set(NEW_SYMBOL("ePDFObjectIndirectObjectReference"),NEW_NUMBER(PDFObject::ePDFObjectIndirectObjectReference));
    exports->Set(NEW_SYMBOL("ePDFObjectStream"),NEW_NUMBER(PDFObject::ePDFObjectStream));
    exports->Set(NEW_SYMBOL("ePDFObjectSymbol"),NEW_NUMBER(PDFObject::ePDFObjectSymbol));
    // getter for string represenation of type enum
	exports->Set(NEW_SYMBOL("getTypeLabel"), NEW_FUNCTION_TEMPLATE(GetTypeLabel)->GetFunction());
    
    // ETokenSeparator
    exports->Set(NEW_SYMBOL("eTokenSeparatorSpace"),NEW_NUMBER(eTokenSeparatorSpace));
    exports->Set(NEW_SYMBOL("eTokenSeparatorEndLine"),NEW_NUMBER(eTokenSeparatorEndLine));
    exports->Set(NEW_SYMBOL("eTokenSepratorNone"),NEW_NUMBER(eTokenSepratorNone));
    
    // EXrefEntryType
    exports->Set(NEW_SYMBOL("eXrefEntryExisting"),NEW_NUMBER(eXrefEntryExisting));
    exports->Set(NEW_SYMBOL("eXrefEntryDelete"),NEW_NUMBER(eXrefEntryDelete));
    exports->Set(NEW_SYMBOL("eXrefEntryStreamObject"),NEW_NUMBER(eXrefEntryStreamObject));
    exports->Set(NEW_SYMBOL("eXrefEntryUndefined"),NEW_NUMBER(eXrefEntryUndefined));
    
    // EInfoTrapped
    exports->Set(NEW_SYMBOL("EInfoTrappedTrue"),NEW_NUMBER(EInfoTrappedTrue));
    exports->Set(NEW_SYMBOL("EInfoTrappedFalse"),NEW_NUMBER(EInfoTrappedFalse));
    exports->Set(NEW_SYMBOL("EInfoTrappedUnknown"),NEW_NUMBER(EInfoTrappedUnknown));
}

NODE_MODULE(hummus, HummusInit)