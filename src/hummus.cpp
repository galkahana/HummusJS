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

#include "nodes.h"
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
#include "ByteReaderDriver.h"
#include "ByteWriterDriver.h"
#include "ByteReaderWithPositionDriver.h"
#include "ByteWriterWithPositionDriver.h"
#include "PDFWriter.h"
#include "PDFPageModifierDriver.h"
#include "PDFObjectParserDriver.h"
#include "ConstructorsHolder.h"

using namespace v8;
using namespace node;

METHOD_RETURN_TYPE CreateWriter(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, holder)
    Local<Value> instance = holder->GetNewPDFWriter(args);        
    
    PDFWriterDriver* driver = ObjectWrap::Unwrap<PDFWriterDriver>(instance->TO_OBJECT());

	if (args.Length() < 1 || args.Length() > 2) {
		THROW_EXCEPTION("Wrong number of arguments, Provide one argument stating the location of the output file, and an optional options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
	if (!args[0]->IsString() && !args[0]->IsObject()) {
		THROW_EXCEPTION("Wrong arguments, please provide a path to a file as the first argument or a stream object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    EPDFVersion pdfVersion = ePDFVersion14;
    PDFCreationSettings pdfCreationSettings(true,true);
    LogConfiguration logConfig = LogConfiguration::DefaultLogConfiguration();
    
    if(args.Length() == 2 && args[1]->IsObject())
    {
        Local<Object> anObject = args[1]->TO_OBJECT();
        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("version")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("version")).ToLocalChecked()->IsNumber())
        {
            long pdfVersionValue = TO_INT32(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("version")).ToLocalChecked())->Value();
            
            if(pdfVersionValue < ePDFVersion10 || ePDFVersionMax < pdfVersionValue)
            {
                THROW_EXCEPTION("Wrong argument for PDF version, please provide a valid PDF version");
                SET_FUNCTION_RETURN_VALUE(UNDEFINED)
            }
            pdfVersion = (EPDFVersion)pdfVersionValue;
        }
            
        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("compress")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("compress")).ToLocalChecked()->IsBoolean())
            pdfCreationSettings.CompressStreams = anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("compress")).ToLocalChecked()->TO_BOOLEAN()->Value();

        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("log")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("log")).ToLocalChecked()->IsString())
        {
            logConfig.ShouldLog = true;
            logConfig.LogFileLocation = *UTF_8_VALUE(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("log")).ToLocalChecked()->TO_STRING());
        }

        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("userPassword")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("userPassword")).ToLocalChecked()->IsString())
        {
            pdfCreationSettings.DocumentEncryptionOptions.ShouldEncrypt = true;
            pdfCreationSettings.DocumentEncryptionOptions.UserPassword = *UTF_8_VALUE(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("userPassword")).ToLocalChecked()->TO_STRING());
        }

        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("ownerPassword")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("ownerPassword")).ToLocalChecked()->IsString())
        {
            pdfCreationSettings.DocumentEncryptionOptions.OwnerPassword = *UTF_8_VALUE(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("ownerPassword")).ToLocalChecked()->TO_STRING());
        }

        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("userProtectionFlag")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("userProtectionFlag")).ToLocalChecked()->IsNumber())
        {
            pdfCreationSettings.DocumentEncryptionOptions.UserProtectionOptionsFlag = TO_INT32(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("userProtectionFlag")).ToLocalChecked())->Value();
        }
        else // default to print only
            pdfCreationSettings.DocumentEncryptionOptions.UserProtectionOptionsFlag = 4;
    }
    
    EStatusCode status;
    
    if(args[0]->IsObject())
    {
        status = driver->StartPDF(args[0]->TO_OBJECT(), pdfVersion,logConfig,pdfCreationSettings);
    }
    else
    {
        status = driver->StartPDF(std::string(*UTF_8_VALUE(args[0]->TO_STRING())), pdfVersion,logConfig,pdfCreationSettings);
    }
    
    if(status != PDFHummus::eSuccess)
    {
		THROW_EXCEPTION("Unable to create PDF file, make sure that output file target is available");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    SET_FUNCTION_RETURN_VALUE(instance)

}

/*
    hummus.recrypt(
        inOriginalPath/inOriginalStream,
        inNewPath/inNewStream,
        [options])

    options = 
    {
        password:
        version:
        compress:
        log:
        userPassword:
        ownerPassword: // must pass userPassword!
        userProtectionFlag: // must pass userPassword!
    }
*/

METHOD_RETURN_TYPE Recrypt(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	if (args.Length() < 2 || args.Length() > 3) {
		THROW_EXCEPTION("Wrong number of arguments, Provide one argument stating the location of the source file, a second one for the destination file, and an optional options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
	if (!args[0]->IsString() && !args[0]->IsObject()) {
		THROW_EXCEPTION("Wrong arguments, please provide a path to a file as the first argument or a stream object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}

	if (!args[1]->IsString() && !args[1]->IsObject()) {
		THROW_EXCEPTION("Wrong arguments, please provide a path to a file as the second argument or a stream object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}

    if((args[1]->IsString() && !args[0]->IsString()) || (args[1]->IsObject() && !args[0]->IsObject())) {
		THROW_EXCEPTION("Wrong arguments, please either provide two paths or two stream objects for the first two arguments");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)        
    }
    

    EPDFVersion pdfVersion = ePDFVersionUndefined;
    PDFCreationSettings pdfCreationSettings(true,true);
    LogConfiguration logConfig = LogConfiguration::DefaultLogConfiguration();
    std::string originalPassword;
    
    if(args.Length() == 3 && args[2]->IsObject())
    {
        Local<Object> anObject = args[2]->TO_OBJECT();
        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("version")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("version")).ToLocalChecked()->IsNumber())
        {
            long pdfVersionValue = TO_INT32(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("version")).ToLocalChecked())->Value();
            
            if(pdfVersionValue != ePDFVersionUndefined && (pdfVersionValue < ePDFVersion10 || ePDFVersionMax < pdfVersionValue))
            {
                THROW_EXCEPTION("Wrong argument for PDF version, please provide a valid PDF version");
                SET_FUNCTION_RETURN_VALUE(UNDEFINED)
            }
            pdfVersion = (EPDFVersion)pdfVersionValue;
        }
            
        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("compress")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("compress")).ToLocalChecked()->IsBoolean())
            pdfCreationSettings.CompressStreams = anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("compress")).ToLocalChecked()->TO_BOOLEAN()->Value();

        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("log")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("log")).ToLocalChecked()->IsString())
        {
            logConfig.ShouldLog = true;
            logConfig.LogFileLocation = *UTF_8_VALUE(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("log")).ToLocalChecked()->TO_STRING());
        }

        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("password")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("password")).ToLocalChecked()->IsString())
        {
            originalPassword = *UTF_8_VALUE(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("password")).ToLocalChecked()->TO_STRING());
        }

        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("userPassword")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("userPassword")).ToLocalChecked()->IsString())
        {
            pdfCreationSettings.DocumentEncryptionOptions.ShouldEncrypt = true;
            pdfCreationSettings.DocumentEncryptionOptions.UserPassword = *UTF_8_VALUE(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("userPassword")).ToLocalChecked()->TO_STRING());
        }

        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("ownerPassword")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("ownerPassword")).ToLocalChecked()->IsString())
        {
            pdfCreationSettings.DocumentEncryptionOptions.OwnerPassword = *UTF_8_VALUE(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("ownerPassword")).ToLocalChecked()->TO_STRING());
        }

        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("userProtectionFlag")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("userProtectionFlag")).ToLocalChecked()->IsNumber())
        {
            pdfCreationSettings.DocumentEncryptionOptions.UserProtectionOptionsFlag = TO_INT32(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("userProtectionFlag")).ToLocalChecked())->Value();
        }
        else // default to print only
            pdfCreationSettings.DocumentEncryptionOptions.UserProtectionOptionsFlag = 4;
    }
    
    EStatusCode status;
    
    if(args[0]->IsObject())
    {
        ObjectByteReaderWithPosition readStreamProxy(args[0]->TO_OBJECT());
        ObjectByteWriterWithPosition writeStreamProxy(args[1]->TO_OBJECT());
        status = PDFWriter::RecryptPDF(&readStreamProxy,
                                    originalPassword,
                                    &writeStreamProxy,
                                    logConfig,
                                    pdfCreationSettings,
                                    pdfVersion);
    }
    else
    {
        status = PDFWriter::RecryptPDF(std::string(*UTF_8_VALUE(args[0]->TO_STRING())),
                                    originalPassword,
                                    std::string(*UTF_8_VALUE(args[1]->TO_STRING())),
                                    logConfig,
                                    pdfCreationSettings,
                                    pdfVersion);
    }
    
    if(status != PDFHummus::eSuccess)
    {
		THROW_EXCEPTION("Unable to recrypt files, check that input and output files are clear and arguments are coool");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    SET_FUNCTION_RETURN_VALUE(UNDEFINED)

}

METHOD_RETURN_TYPE CreateWriterToContinue(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, holder)
    Local<Value> instance = holder->GetNewPDFWriter(args);    
    
    PDFWriterDriver* driver = ObjectWrap::Unwrap<PDFWriterDriver>(instance->TO_OBJECT());
    
	if ((args.Length() != 2  && args.Length() !=3)||
            (!args[0]->IsString() && !args[0]->IsObject()) ||
            !args[1]->IsString() ||
            ((args.Length() == 3) && !args[2]->IsObject())) {
		THROW_EXCEPTION("Wrong arguments, provide 2 strings - path to file to continue, and path to state file (provided to the previous shutdown call. You may also add an options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    std::string alternativePath;
    Local<Object> alternativeStream;
    LogConfiguration logConfig = LogConfiguration::DefaultLogConfiguration();
   
    if(args.Length() == 2 && args[1]->IsObject())
    {
        Local<Object> anObject = args[1]->TO_OBJECT();
        
        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("modifiedFilePath")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("modifiedFilePath")).ToLocalChecked()->IsString())
            alternativePath = *UTF_8_VALUE(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("modifiedFilePath")).ToLocalChecked()->TO_STRING());

        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("modifiedStream")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("modifiedStream")).ToLocalChecked()->IsObject())
            alternativeStream = anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("modifiedStream")).ToLocalChecked()->TO_OBJECT();
        
        
        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("log")).FromJust())
        {
            Local<Value> value = anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("log")).ToLocalChecked();
            if(value->IsString())
            {
                logConfig.ShouldLog = true;
                logConfig.LogFileLocation = *UTF_8_VALUE(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("log")).ToLocalChecked()->TO_STRING());
                logConfig.LogStream = NULL;
            }
            else if(value->IsObject())
            {
                logConfig.ShouldLog = true;
                logConfig.LogFileLocation = "";
                ObjectByteWriter proxy(value->TO_OBJECT());
                logConfig.LogStream = &proxy;
            }
        }
    }
    
    EStatusCode status;
    
    if(args[0]->IsObject())
    {
        status = driver->ContinuePDF(args[0]->TO_OBJECT(),
                                     *UTF_8_VALUE(args[1]->TO_STRING()),
                                     alternativeStream,
                                     logConfig);
    }
    else
    {
        status = driver->ContinuePDF(*UTF_8_VALUE(args[0]->TO_STRING()),
                                             *UTF_8_VALUE(args[1]->TO_STRING()),
                                             alternativePath,
                                             logConfig);
    }
    
    if(status != PDFHummus::eSuccess)
    {
		THROW_EXCEPTION("Unable to continue PDF file, make sure that output file target is available and state file exists");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    SET_FUNCTION_RETURN_VALUE(instance)
}


METHOD_RETURN_TYPE CreateWriterToModify(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, holder)
    Local<Value> instance = holder->GetNewPDFWriter(args);  
    
    PDFWriterDriver* driver = ObjectWrap::Unwrap<PDFWriterDriver>(instance->TO_OBJECT());
    
    if(args.Length() < 1 ||
       (!args[0]->IsString() && !args[0]->IsObject()) ||
       (args[0]->IsString() && args.Length() > 2) ||
       (args[0]->IsObject() && (!args[1]->IsObject() || args.Length() > 3)))
    {
		THROW_EXCEPTION("Wrong arguments, please path a path to modified file, or a pair of stream - first for the source, and second for destination. in addition you can optionally add an options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    
    EPDFVersion pdfVersion = ePDFVersion10;
    std::string alternativePath;
    Local<Value> alternativeStream;
    PDFCreationSettings pdfCreationSettings(true,true);
    LogConfiguration logConfig = LogConfiguration::DefaultLogConfiguration();
    
    int optionsObjectIndex = args[0]->IsString() ? 1:2;
    
    if(args.Length() == (optionsObjectIndex+1) && args[optionsObjectIndex]->IsObject())
    {
        Local<Object> anObject = args[optionsObjectIndex]->TO_OBJECT();
        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("version")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("version")).ToLocalChecked()->IsString())
        {
            long pdfVersionValue = TO_INT32(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("version")).ToLocalChecked())->Value();
            
            if(pdfVersionValue < ePDFVersion10 || ePDFVersionMax < pdfVersionValue)
            {
                THROW_EXCEPTION("Wrong argument for PDF version, please provide a valid PDF version");
                SET_FUNCTION_RETURN_VALUE(UNDEFINED)
            }
            pdfVersion = (EPDFVersion)pdfVersionValue;
        }
        
        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("compress")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("compress")).ToLocalChecked()->IsBoolean())
            pdfCreationSettings.CompressStreams = anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("compress")).ToLocalChecked()->TO_BOOLEAN()->Value();
        
        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("modifiedFilePath")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("modifiedFilePath")).ToLocalChecked()->IsString())
            alternativePath = *UTF_8_VALUE(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("modifiedFilePath")).ToLocalChecked()->TO_STRING());

        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("log")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("log")).ToLocalChecked()->IsString())
        {
            logConfig.ShouldLog = true;
            logConfig.LogFileLocation = *UTF_8_VALUE(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("log")).ToLocalChecked()->TO_STRING());
        }

        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("userPassword")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("userPassword")).ToLocalChecked()->IsString())
        {
            pdfCreationSettings.DocumentEncryptionOptions.ShouldEncrypt = true;
            pdfCreationSettings.DocumentEncryptionOptions.UserPassword = *UTF_8_VALUE(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("userPassword")).ToLocalChecked()->TO_STRING());
        }

        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("ownerPassword")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("ownerPassword")).ToLocalChecked()->IsString())
        {
            pdfCreationSettings.DocumentEncryptionOptions.OwnerPassword = *UTF_8_VALUE(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("ownerPassword")).ToLocalChecked()->TO_STRING());
        }

        if(anObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("userProtectionFlag")).FromJust() && anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("userProtectionFlag")).ToLocalChecked()->IsNumber())
        {
            pdfCreationSettings.DocumentEncryptionOptions.UserProtectionOptionsFlag = TO_INT32(anObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("userProtectionFlag")).ToLocalChecked())->Value();
        }
        else // default to print only
            pdfCreationSettings.DocumentEncryptionOptions.UserProtectionOptionsFlag = 4;  
    }
    
    EStatusCode status;
    
    if(args[0]->IsObject())
    {
        status = driver->ModifyPDF(args[0]->TO_OBJECT(),
                                   args[1]->TO_OBJECT(),
                                   pdfVersion,
                                   logConfig,
                                   pdfCreationSettings);
    }
    else
    {
        status = driver->ModifyPDF(*UTF_8_VALUE(args[0]->TO_STRING()),
                               pdfVersion,alternativePath,logConfig,
                               pdfCreationSettings);
    }
    
    if(status != PDFHummus::eSuccess)
    {
		THROW_EXCEPTION("Unable to modify PDF file, make sure that output file target is available and that it is not protected");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    SET_FUNCTION_RETURN_VALUE(instance)
}


METHOD_RETURN_TYPE CreateReader(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, holder)
    Local<Value> instance = holder->GetNewPDFReader(args);      
    
    PDFReaderDriver* driver = ObjectWrap::Unwrap<PDFReaderDriver>(instance->TO_OBJECT());
    
	if (args.Length() < 1 ||
        args.Length() > 2 ||
        (!args[0]->IsString() && !args[0]->IsObject()) ||
        (args.Length() >=2 && !args[1]->IsObject()))
    {
		THROW_EXCEPTION("Wrong arguments, provide 1 string - path to file read, or a read stream object, and optionally an options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
        
    PDFHummus::EStatusCode status;
    PDFParsingOptions parsingOptions;

    if(args.Length() >= 2) 
    {
        Local<Object> options = args[1]->TO_OBJECT();
        if(options->Has(GET_CURRENT_CONTEXT, NEW_STRING("password")).FromJust() && options->Get(GET_CURRENT_CONTEXT, NEW_STRING("password")).ToLocalChecked()->IsString())
        {
            parsingOptions.Password = *UTF_8_VALUE(options->Get(GET_CURRENT_CONTEXT, NEW_STRING("password")).ToLocalChecked()->TO_STRING());
        }
    }
        
        
    if(args[0]->IsObject())
        status = driver->StartPDFParsing(args[0]->TO_OBJECT(),parsingOptions);
    else
        
        status = driver->StartPDFParsing(std::string(*UTF_8_VALUE(args[0]->TO_STRING())),parsingOptions);
    if(status != PDFHummus::eSuccess)
    {
		THROW_EXCEPTION("Unable to start parsing PDF file");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    SET_FUNCTION_RETURN_VALUE(instance)
}

METHOD_RETURN_TYPE GetTypeLabel(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong arguments, provide a single enumerator value of a PDF Object type");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    unsigned long value = TO_NUMBER(args[0])->Value();
    
    if(value > PDFObject::ePDFObjectSymbol)
    {
		THROW_EXCEPTION("Wrong arguments, provide a single enumerator value of a PDF Object type");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    Local<Value> result = NEW_STRING(PDFObject::scPDFObjectTypeLabel((PDFObject::EPDFObjectType)value));
    
    SET_FUNCTION_RETURN_VALUE(result)
}

DEF_INIT(HummusInit) {
	CREATE_ISOLATE_CONTEXT;
    
    DECLARE_EXTERNAL(ConstructorsHolder)   

    CALL_INIT_WITH_EXPORTS(PDFWriterDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFPageDriver::Init)
    CALL_INIT_WITH_EXPORTS(PageContentContextDriver::Init)
    CALL_INIT_WITH_EXPORTS(FormXObjectDriver::Init)
    CALL_INIT_WITH_EXPORTS(ObjectsContextDriver::Init);
    CALL_INIT_WITH_EXPORTS(DocumentCopyingContextDriver::Init);
    CALL_INIT_WITH_EXPORTS(PDFReaderDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFTextStringDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFDateDriver::Init)
    CALL_INIT_WITH_EXPORTS(InputFileDriver::Init)
    CALL_INIT_WITH_EXPORTS(OutputFileDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFStreamDriver::Init)
    CALL_INIT_WITH_EXPORTS(ByteReaderDriver::Init)
	CALL_INIT_WITH_EXPORTS(ByteWriterDriver::Init)
	CALL_INIT_WITH_EXPORTS(ByteWriterWithPositionDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFPageModifierDriver::Init)
    CALL_INIT_WITH_EXPORTS(ByteReaderWithPositionDriver::Init)
    CALL_INIT_WITH_EXPORTS(DictionaryContextDriver::Init)
    CALL_INIT_WITH_EXPORTS(DocumentContextDriver::Init)
    CALL_INIT_WITH_EXPORTS(InfoDictionaryDriver::Init)
    CALL_INIT_WITH_EXPORTS(ImageXObjectDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFArrayDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFDictionaryDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFStreamInputDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFIndirectObjectReferenceDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFBooleanDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFLiteralStringDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFHexStringDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFNullDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFNameDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFIntegerDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFRealDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFSymbolDriver::Init)    
    CALL_INIT_WITH_EXPORTS(PDFPageInputDriver::Init)
    CALL_INIT_WITH_EXPORTS(PDFObjectParserDriver::Init)
    CALL_INIT_WITH_EXPORTS(XObjectContentContextDriver::Init)
    CALL_INIT_WITH_EXPORTS(ResourcesDictionaryDriver::Init)
    CALL_INIT_WITH_EXPORTS(UsedFontDriver::Init)

    
    // define methods
	EXPORTS_SET(exports,NEW_SYMBOL("createWriter"), NEW_FUNCTION_TEMPLATE_EXTERNAL(CreateWriter)->GetFunction(GET_CURRENT_CONTEXT).ToLocalChecked())
	EXPORTS_SET(exports,NEW_SYMBOL("createWriterToContinue"), NEW_FUNCTION_TEMPLATE_EXTERNAL(CreateWriterToContinue)->GetFunction(GET_CURRENT_CONTEXT).ToLocalChecked())
	EXPORTS_SET(exports,NEW_SYMBOL("createWriterToModify"), NEW_FUNCTION_TEMPLATE_EXTERNAL(CreateWriterToModify)->GetFunction(GET_CURRENT_CONTEXT).ToLocalChecked())
	EXPORTS_SET(exports,NEW_SYMBOL("createReader"), NEW_FUNCTION_TEMPLATE_EXTERNAL(CreateReader)->GetFunction(GET_CURRENT_CONTEXT).ToLocalChecked())
    EXPORTS_SET(exports,NEW_SYMBOL("recrypt"), NEW_FUNCTION_TEMPLATE(Recrypt)->GetFunction(GET_CURRENT_CONTEXT).ToLocalChecked())
    
    // define pdf versions enum
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFVersion10"),NEW_NUMBER(ePDFVersion10))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFVersion11"),NEW_NUMBER(ePDFVersion11))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFVersion12"),NEW_NUMBER(ePDFVersion12))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFVersion13"),NEW_NUMBER(ePDFVersion13))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFVersion14"),NEW_NUMBER(ePDFVersion14))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFVersion15"),NEW_NUMBER(ePDFVersion15))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFVersion16"),NEW_NUMBER(ePDFVersion16))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFVersion17"),NEW_NUMBER(ePDFVersion17))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFVersionUndefined"),NEW_NUMBER(ePDFVersionUndefined))
    
    // procsets for resource inclusion
    EXPORTS_SET(exports,NEW_SYMBOL("KProcsetImageB"),NEW_STRING(KProcsetImageB.c_str()))
    EXPORTS_SET(exports,NEW_SYMBOL("KProcsetImageC"),NEW_STRING(KProcsetImageC.c_str()))
    EXPORTS_SET(exports,NEW_SYMBOL("KProcsetImageI"),NEW_STRING(KProcsetImageI.c_str()))
    EXPORTS_SET(exports,NEW_SYMBOL("kProcsetPDF"),NEW_STRING(KProcsetPDF.c_str()))
    EXPORTS_SET(exports,NEW_SYMBOL("kProcsetText"),NEW_STRING(KProcsetText.c_str()))
    
    // page range values
    EXPORTS_SET(exports,NEW_SYMBOL("eRangeTypeAll"),NEW_NUMBER(PDFPageRange::eRangeTypeAll))
    EXPORTS_SET(exports,NEW_SYMBOL("eRangeTypeSpecific"),NEW_NUMBER(PDFPageRange::eRangeTypeSpecific))
    
    // EPDFPageBox enumerator, for embedding pages into forms
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFPageBoxMediaBox"),NEW_NUMBER(ePDFPageBoxMediaBox))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFPageBoxCropBox"),NEW_NUMBER(ePDFPageBoxCropBox))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFPageBoxBleedBox"),NEW_NUMBER(ePDFPageBoxBleedBox))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFPageBoxTrimBox"),NEW_NUMBER(ePDFPageBoxTrimBox))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFPageBoxArtBox"),NEW_NUMBER(ePDFPageBoxArtBox))
    
    // parsed object types
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFObjectBoolean"),NEW_NUMBER(PDFObject::ePDFObjectBoolean))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFObjectLiteralString"),NEW_NUMBER(PDFObject::ePDFObjectLiteralString))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFObjectHexString"),NEW_NUMBER(PDFObject::ePDFObjectHexString))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFObjectNull"),NEW_NUMBER(PDFObject::ePDFObjectNull))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFObjectName"),NEW_NUMBER(PDFObject::ePDFObjectName))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFObjectInteger"),NEW_NUMBER(PDFObject::ePDFObjectInteger))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFObjectReal"),NEW_NUMBER(PDFObject::ePDFObjectReal))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFObjectArray"),NEW_NUMBER(PDFObject::ePDFObjectArray))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFObjectDictionary"),NEW_NUMBER(PDFObject::ePDFObjectDictionary))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFObjectIndirectObjectReference"),NEW_NUMBER(PDFObject::ePDFObjectIndirectObjectReference))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFObjectStream"),NEW_NUMBER(PDFObject::ePDFObjectStream))
    EXPORTS_SET(exports,NEW_SYMBOL("ePDFObjectSymbol"),NEW_NUMBER(PDFObject::ePDFObjectSymbol))
    // getter for string represenation of type enum
	EXPORTS_SET(exports,NEW_SYMBOL("getTypeLabel"), NEW_FUNCTION_TEMPLATE(GetTypeLabel)->GetFunction(GET_CURRENT_CONTEXT).ToLocalChecked())
    
    // ETokenSeparator
    EXPORTS_SET(exports,NEW_SYMBOL("eTokenSeparatorSpace"),NEW_NUMBER(eTokenSeparatorSpace))
    EXPORTS_SET(exports,NEW_SYMBOL("eTokenSeparatorEndLine"),NEW_NUMBER(eTokenSeparatorEndLine))
    EXPORTS_SET(exports,NEW_SYMBOL("eTokenSepratorNone"),NEW_NUMBER(eTokenSepratorNone))
    
    // EXrefEntryType
    EXPORTS_SET(exports,NEW_SYMBOL("eXrefEntryExisting"),NEW_NUMBER(eXrefEntryExisting))
    EXPORTS_SET(exports,NEW_SYMBOL("eXrefEntryDelete"),NEW_NUMBER(eXrefEntryDelete))
    EXPORTS_SET(exports,NEW_SYMBOL("eXrefEntryStreamObject"),NEW_NUMBER(eXrefEntryStreamObject))
    EXPORTS_SET(exports,NEW_SYMBOL("eXrefEntryUndefined"),NEW_NUMBER(eXrefEntryUndefined))
    
    // EInfoTrapped
    EXPORTS_SET(exports,NEW_SYMBOL("EInfoTrappedTrue"),NEW_NUMBER(EInfoTrappedTrue))
    EXPORTS_SET(exports,NEW_SYMBOL("EInfoTrappedFalse"),NEW_NUMBER(EInfoTrappedFalse))
    EXPORTS_SET(exports,NEW_SYMBOL("EInfoTrappedUnknown"),NEW_NUMBER(EInfoTrappedUnknown))
}

NODES_MODULE(hummus, HummusInit)