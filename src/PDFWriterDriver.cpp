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
#include "ByteReaderWithPositionDriver.h"
#include "PageContentContextDriver.h"
#include "FormXObjectDriver.h"
#include "UsedFontDriver.h"
#include "ImageXObjectDriver.h"
#include "ObjectsContextDriver.h"
#include "DocumentContextExtenderAdapter.h"
#include "DocumentCopyingContextDriver.h"
#include "InputFile.h"
#include "PDFParser.h"
#include "PDFDateDriver.h"
#include "PDFTextStringDriver.h"
#include "PDFParser.h"
#include "PDFPageInput.h"
#include "PDFRectangle.h"
#include "TIFFImageHandler.h"
#include "IOBasicTypes.h"
#include "PDFDocumentCopyingContext.h"
#include "PDFFormXObject.h"
#include "PDFReaderDriver.h"
#include "InputFileDriver.h"
#include "OutputFileDriver.h"
#include "DocumentContextDriver.h"
#include "ObjectByteReaderWithPosition.h"
#include "DictionaryContextDriver.h"
#include "ResourcesDictionaryDriver.h"

using namespace v8;

Persistent<Function> PDFWriterDriver::constructor;

PDFWriterDriver::PDFWriterDriver()
{
    mWriteStreamProxy = NULL;
    mReadStreamProxy = NULL;
    mStartedWithStream = false;
    
}

PDFWriterDriver::~PDFWriterDriver()
{
    delete mWriteStreamProxy;
    delete mReadStreamProxy;
}

void PDFWriterDriver::Init(Handle<Object> inExports)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PDFWriter"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "end", End);
	SET_PROTOTYPE_METHOD(t, "createPage", CreatePage);
	SET_PROTOTYPE_METHOD(t, "writePage", WritePage);
	SET_PROTOTYPE_METHOD(t, "writePageAndReturnID", WritePageAndReturnID);
	SET_PROTOTYPE_METHOD(t, "startPageContentContext", StartPageContentContext);
	SET_PROTOTYPE_METHOD(t, "pausePageContentContext", PausePageContentContext);
	SET_PROTOTYPE_METHOD(t, "createFormXObject", CreateFormXObject);
	SET_PROTOTYPE_METHOD(t, "endFormXObject", EndFormXObject);
	SET_PROTOTYPE_METHOD(t, "createFormXObjectFromJPG", CreateformXObjectFromJPG);
	SET_PROTOTYPE_METHOD(t, "getFontForFile", GetFontForFile);
	SET_PROTOTYPE_METHOD(t, "attachURLLinktoCurrentPage", AttachURLLinktoCurrentPage);
	SET_PROTOTYPE_METHOD(t, "shutdown", Shutdown);
	SET_PROTOTYPE_METHOD(t, "createFormXObjectFromTIFF", CreateFormXObjectFromTIFF);
	SET_PROTOTYPE_METHOD(t, "createImageXObjectFromJPG", CreateImageXObjectFromJPG);
	SET_PROTOTYPE_METHOD(t, "retrieveJPGImageInformation", RetrieveJPGImageInformation);
	SET_PROTOTYPE_METHOD(t, "getObjectsContext", GetObjectsContext);
	SET_PROTOTYPE_METHOD(t, "getDocumentContext", GetDocumentContext);
	SET_PROTOTYPE_METHOD(t, "appendPDFPagesFromPDF", AppendPDFPagesFromPDF);
	SET_PROTOTYPE_METHOD(t, "mergePDFPagesToPage", MergePDFPagesToPage);
	SET_PROTOTYPE_METHOD(t, "createPDFCopyingContext", CreatePDFCopyingContext);
	SET_PROTOTYPE_METHOD(t, "createFormXObjectsFromPDF", CreateFormXObjectsFromPDF);
	SET_PROTOTYPE_METHOD(t, "createPDFCopyingContextForModifiedFile", CreatePDFCopyingContextForModifiedFile);
	SET_PROTOTYPE_METHOD(t, "createPDFTextString", CreatePDFTextString);
	SET_PROTOTYPE_METHOD(t, "createPDFDate", CreatePDFDate);
	SET_PROTOTYPE_METHOD(t, "getImageDimensions", GetImageDimensions);
	SET_PROTOTYPE_METHOD(t, "getImagePagesCount", GetImagePagesCount);
	SET_PROTOTYPE_METHOD(t, "getImageType", GetImageType);
	SET_PROTOTYPE_METHOD(t, "getModifiedFileParser", GetModifiedFileParser);
	SET_PROTOTYPE_METHOD(t, "getModifiedInputFile", GetModifiedInputFile);
	SET_PROTOTYPE_METHOD(t, "getOutputFile", GetOutputFile);
	SET_PROTOTYPE_METHOD(t, "registerAnnotationReferenceForNextPageWrite", RegisterAnnotationReferenceForNextPageWrite);

	SET_CONSTRUCTOR(constructor, t);
    SET_CONSTRUCTOR_EXPORT(inExports, "PDFWriter", t);
}

METHOD_RETURN_TYPE PDFWriterDriver::NewInstance(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    Local<Object> instance = NEW_INSTANCE(constructor);
    SET_FUNCTION_RETURN_VALUE(instance);
}

v8::Handle<v8::Value> PDFWriterDriver::GetNewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

METHOD_RETURN_TYPE PDFWriterDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFWriterDriver* pdfWriter = new PDFWriterDriver();
        
    pdfWriter->Wrap(args.This());
    
	SET_FUNCTION_RETURN_VALUE(args.This());
}

METHOD_RETURN_TYPE PDFWriterDriver::End(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
   
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    EStatusCode status;
    
    if(pdfWriter->mStartedWithStream)
        status = pdfWriter->mPDFWriter.EndPDFForStream();
    else
        status = pdfWriter->mPDFWriter.EndPDF();
    
    // now remove event listener
    pdfWriter->mPDFWriter.GetDocumentContext().AddDocumentContextExtender(pdfWriter);

    
    if(status != PDFHummus::eSuccess)
    {
		THROW_EXCEPTION("Unable to end PDF");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    if(pdfWriter->mWriteStreamProxy)
    {
        delete pdfWriter->mWriteStreamProxy;
        pdfWriter->mWriteStreamProxy = NULL;
    }
    
    if(pdfWriter->mReadStreamProxy)
    {
        delete pdfWriter->mReadStreamProxy;
        pdfWriter->mReadStreamProxy = NULL;
    }
    
    SET_FUNCTION_RETURN_VALUE(args.This());
}

METHOD_RETURN_TYPE PDFWriterDriver::CreatePage(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    SET_FUNCTION_RETURN_VALUE(PDFPageDriver::GetNewInstance(args));
    
}

METHOD_RETURN_TYPE PDFWriterDriver::WritePage(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    WritePageAndReturnID(args);
    
    SET_FUNCTION_RETURN_VALUE(args.This());
    
}

METHOD_RETURN_TYPE PDFWriterDriver::WritePageAndReturnID(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
	if (args.Length() != 1 || !PDFPageDriver::HasInstance(args[0])) {
		THROW_EXCEPTION("Wrong arguments, provide a page as the single parameter");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
	}
    
    PDFPageDriver* pageDriver = ObjectWrap::Unwrap<PDFPageDriver>(args[0]->ToObject());
    if(!pageDriver)
    {
		THROW_EXCEPTION("Wrong arguments, provide a page as the single parameter");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    if(pageDriver->ContentContext &&
       (pdfWriter->mPDFWriter.EndPageContentContext(pageDriver->ContentContext) != PDFHummus::eSuccess))
    {
		THROW_EXCEPTION("Unable to finalize page context");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    pageDriver->ContentContext = NULL;
    
    EStatusCodeAndObjectIDType result = pdfWriter->mPDFWriter.WritePageAndReturnPageID(pageDriver->GetPage());
    
    if(result.first != PDFHummus::eSuccess)
    {
		THROW_EXCEPTION("Unable to write page");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(result.second));
    
}

METHOD_RETURN_TYPE PDFWriterDriver::StartPageContentContext(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
	if (args.Length() != 1 || !PDFPageDriver::HasInstance(args[0])) {
		THROW_EXCEPTION("Wrong arguments, provide a page as the single parameter");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
	}
    
    PDFPageDriver* pageDriver = ObjectWrap::Unwrap<PDFPageDriver>(args[0]->ToObject());
    if(!pageDriver)
    {
		THROW_EXCEPTION("Wrong arguments, provide a page as the single parameter");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    
    Handle<Value> newInstance = PageContentContextDriver::GetNewInstance(args);
    PageContentContextDriver* contentContextDriver = ObjectWrap::Unwrap<PageContentContextDriver>(newInstance->ToObject());
    contentContextDriver->ContentContext = pdfWriter->mPDFWriter.StartPageContentContext(pageDriver->GetPage());
    contentContextDriver->SetResourcesDictionary(&(pageDriver->GetPage()->GetResourcesDictionary()));
    
    // save it also at page driver, so we can end the context when the page is written
    pageDriver->ContentContext = contentContextDriver->ContentContext;
    
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFWriterDriver::PausePageContentContext(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
	if (args.Length() != 1 || !PageContentContextDriver::HasInstance(args[0])) {
		THROW_EXCEPTION("Wrong arguments, provide a page context as the single parameter");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
	}
    
    PageContentContextDriver* pageContextDriver = ObjectWrap::Unwrap<PageContentContextDriver>(args[0]->ToObject());
    if(!pageContextDriver)
    {
		THROW_EXCEPTION("Wrong arguments, provide a page context as the single parameter");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    if(!pageContextDriver->ContentContext)
    {
		THROW_EXCEPTION("paused context not initialized, please create one using pdfWriter.startPageContentContext");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    pdfWriter->mPDFWriter.PausePageContentContext(pageContextDriver->ContentContext);
    
    SET_FUNCTION_RETURN_VALUE(args.This());
}

METHOD_RETURN_TYPE PDFWriterDriver::CreateFormXObject(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if((args.Length() != 4  && args.Length() != 5) || !args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber()
        || (args.Length() == 5 && !args[4]->IsNumber()))
    {
		THROW_EXCEPTION("wrong arguments, pass 4 coordinates of the form rectangle and an optional 5th agument which is the forward reference ID");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
     
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    Handle<Value> newInstance = FormXObjectDriver::GetNewInstance(args);
    FormXObjectDriver* formXObjectDriver = ObjectWrap::Unwrap<FormXObjectDriver>(newInstance->ToObject());
    formXObjectDriver->FormXObject =
                        args.Length() == 5 ?
                                            pdfWriter->mPDFWriter.StartFormXObject(
                                                                            PDFRectangle(args[0]->ToNumber()->Value(),
                                                                                         args[1]->ToNumber()->Value(),
                                                                                         args[2]->ToNumber()->Value(),
                                                                                         args[3]->ToNumber()->Value()),
                                                                                        (ObjectIDType)args[4]->ToNumber()->Value()):
                                            pdfWriter->mPDFWriter.StartFormXObject(
                                                                            PDFRectangle(args[0]->ToNumber()->Value(),
                                                                                         args[1]->ToNumber()->Value(),
                                                                                         args[2]->ToNumber()->Value(),
                                                                                         args[3]->ToNumber()->Value()));
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFWriterDriver::EndFormXObject(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
	if (args.Length() != 1 || !FormXObjectDriver::HasInstance(args[0])) {
		THROW_EXCEPTION("Wrong arguments, provide a form as the single parameter");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
	}
    
    FormXObjectDriver* formContextDriver = ObjectWrap::Unwrap<FormXObjectDriver>(args[0]->ToObject());
    if(!formContextDriver)
    {
		THROW_EXCEPTION("Wrong arguments, provide a form as the single parameter");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
        
    pdfWriter->mPDFWriter.EndFormXObject(formContextDriver->FormXObject);
    
    SET_FUNCTION_RETURN_VALUE(args.This());
    
}



METHOD_RETURN_TYPE PDFWriterDriver::CreateformXObjectFromJPG(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if((args.Length() != 1  && args.Length() != 2 ) || (!args[0]->IsString() && !args[0]->IsObject()) || (args.Length() == 2 && !args[1]->IsNumber()))
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument that is the path to the image. Optionally pass an object ID for a forward reference image");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    PDFFormXObject* formXObject;
    
    if(args[0]->IsObject())
    {
        ObjectByteReaderWithPosition proxy(args[0]->ToObject());
        
        formXObject =
        args.Length() == 2 ?
        pdfWriter->mPDFWriter.CreateFormXObjectFromJPGStream(&proxy,(ObjectIDType)args[1]->ToNumber()->Int32Value()):
        pdfWriter->mPDFWriter.CreateFormXObjectFromJPGStream(&proxy);
        
    }
    else
    {
        formXObject =
            args.Length() == 2 ?
            pdfWriter->mPDFWriter.CreateFormXObjectFromJPGFile(*String::Utf8Value(args[0]->ToString()),(ObjectIDType)args[1]->ToNumber()->Int32Value()):
            pdfWriter->mPDFWriter.CreateFormXObjectFromJPGFile(*String::Utf8Value(args[0]->ToString()));
    }
    if(!formXObject)
    {
		THROW_EXCEPTION("unable to create form xobject. verify that the target is an existing jpg file/stream");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    Handle<Value> newInstance = FormXObjectDriver::GetNewInstance(args);
    ObjectWrap::Unwrap<FormXObjectDriver>(newInstance->ToObject())->FormXObject = formXObject;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFWriterDriver::RetrieveJPGImageInformation(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1  ||
       !args[0]->IsString())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument that is the path to the image");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    BoolAndJPEGImageInformation info = pdfWriter->mPDFWriter.GetDocumentContext().GetJPEGImageHandler().RetrieveImageInformation(*String::Utf8Value(args[0]->ToString()));
    
    if(!info.first)
    {
		THROW_EXCEPTION("unable to retrieve image information");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    Handle<Object> result = NEW_OBJECT;
    
	result->Set(NEW_SYMBOL("samplesWidth"), NEW_INTEGER((int)info.second.SamplesWidth));
	result->Set(NEW_SYMBOL("samplesHeight"), NEW_INTEGER((int)info.second.SamplesHeight));
	result->Set(NEW_SYMBOL("colorComponentsCount"), NEW_INTEGER(info.second.ColorComponentsCount));
	result->Set(NEW_SYMBOL("JFIFInformationExists"), NEW_BOOLEAN(info.second.JFIFInformationExists));
    if(info.second.JFIFInformationExists)
    {
		result->Set(NEW_SYMBOL("JFIFUnit"), NEW_INTEGER(info.second.JFIFUnit));
		result->Set(NEW_SYMBOL("JFIFXDensity"), NEW_NUMBER(info.second.JFIFXDensity));
		result->Set(NEW_SYMBOL("JFIFYDensity"), NEW_NUMBER(info.second.JFIFYDensity));
    }
	result->Set(NEW_SYMBOL("ExifInformationExists"), NEW_BOOLEAN(info.second.ExifInformationExists));
    if(info.second.ExifInformationExists)
    {
		result->Set(NEW_SYMBOL("ExifUnit"), NEW_INTEGER(info.second.ExifUnit));
		result->Set(NEW_SYMBOL("ExifXDensity"), NEW_NUMBER(info.second.ExifXDensity));
		result->Set(NEW_SYMBOL("ExifYDensity"), NEW_NUMBER(info.second.ExifYDensity));
    }
	result->Set(NEW_SYMBOL("PhotoshopInformationExists"), NEW_BOOLEAN(info.second.PhotoshopInformationExists));
    if(info.second.PhotoshopInformationExists)
    {
		result->Set(NEW_SYMBOL("PhotoshopXDensity"), NEW_NUMBER(info.second.PhotoshopXDensity));
		result->Set(NEW_SYMBOL("PhotoshopYDensity"), NEW_NUMBER(info.second.PhotoshopYDensity));
    }

    SET_FUNCTION_RETURN_VALUE(result);
}

METHOD_RETURN_TYPE PDFWriterDriver::GetFontForFile(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() < 1 ||
        !args[0]->IsString() ||
                (args.Length() == 2 && !args[1]->IsString() && !args[1]->IsNumber()) ||
                (args.Length() == 3 && !args[1]->IsString() && !args[2]->IsNumber()))
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument that is the path to the font file, with option to a 2nd parameter for another path in case of type 1 font. another optional argument may follow with font index in case of font packages (TTC, DFont)");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    PDFUsedFont* usedFont;
    
    if(args.Length() == 3)
    {
        usedFont = pdfWriter->mPDFWriter.GetFontForFile(*String::Utf8Value(args[0]->ToString()),
                                                        *String::Utf8Value(args[1]->ToString()),
                                                        args[0]->ToNumber()->Uint32Value());
    }
    else if(args.Length() == 2)
    {
        if(args[1]->IsString())
            usedFont = pdfWriter->mPDFWriter.GetFontForFile(*String::Utf8Value(args[0]->ToString()),
                                                            *String::Utf8Value(args[1]->ToString()));
        else
            usedFont = pdfWriter->mPDFWriter.GetFontForFile(*String::Utf8Value(args[0]->ToString()),
                                                            args[1]->ToNumber()->Uint32Value());
    }
    else // length is 1
    {
        usedFont = pdfWriter->mPDFWriter.GetFontForFile(*String::Utf8Value(args[0]->ToString()));
    }
    
    if(!usedFont)
    {
		THROW_EXCEPTION("unable to create font object. verify that the target is an existing and supported font type (ttf,otf,type1,dfont,ttc)");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    Handle<Value> newInstance = UsedFontDriver::GetNewInstance(args);
    ObjectWrap::Unwrap<UsedFontDriver>(newInstance->ToObject())->UsedFont = usedFont;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFWriterDriver::AttachURLLinktoCurrentPage(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 5 ||
        !args[0]->IsString() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber() ||
        !args[3]->IsNumber() ||
        !args[4]->IsNumber())
    {
		THROW_EXCEPTION("wrong arguments, pass a url, and 4 numbers (left,bottom,right,top) for the rectangle valid for clicking");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    EStatusCode status = pdfWriter->mPDFWriter.AttachURLLinktoCurrentPage(*String::Utf8Value(args[0]->ToString()),
                                                                             PDFRectangle(args[1]->ToNumber()->Value(),
                                                                             args[2]->ToNumber()->Value(),
                                                                             args[3]->ToNumber()->Value(),
                                                                             args[4]->ToNumber()->Value()));
    if(status != eSuccess)
    {
		THROW_EXCEPTION("unable to attach link to current page. will happen if the input URL may not be encoded to ascii7");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    SET_FUNCTION_RETURN_VALUE(args.This());
}


METHOD_RETURN_TYPE PDFWriterDriver::Shutdown(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !args[0]->IsString())
    {
		THROW_EXCEPTION("wrong arguments, pass a path to save the state file to");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    EStatusCode status = pdfWriter->mPDFWriter.Shutdown(*String::Utf8Value(args[0]->ToString()));
    if(status != eSuccess)
    {
		THROW_EXCEPTION("unable to save state file. verify that path is not occupied");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    SET_FUNCTION_RETURN_VALUE(args.This());
}

PDFHummus::EStatusCode PDFWriterDriver::StartPDF(const std::string& inOutputFilePath,
                                                 EPDFVersion inPDFVersion,
                                                 const LogConfiguration& inLogConfiguration,
                                                 const PDFCreationSettings& inCreationSettings)
{
    mStartedWithStream = false;
    
    return setupListenerIfOK(mPDFWriter.StartPDF(inOutputFilePath,inPDFVersion,inLogConfiguration,inCreationSettings));
}

PDFHummus::EStatusCode PDFWriterDriver::StartPDF(Handle<Object> inWriteStream,
                                                 EPDFVersion inPDFVersion,
                                                 const LogConfiguration& inLogConfiguration,
                                                 const PDFCreationSettings& inCreationSettings)
{

    mWriteStreamProxy = new ObjectByteWriterWithPosition(inWriteStream);
    mStartedWithStream = true;
    return setupListenerIfOK(mPDFWriter.StartPDFForStream(mWriteStreamProxy,inPDFVersion,inLogConfiguration,inCreationSettings));
}



PDFHummus::EStatusCode PDFWriterDriver::ContinuePDF(const std::string& inOutputFilePath,
                                                    const std::string& inStateFilePath,
                                                    const std::string& inOptionalOtherOutputFile,
                                                    const LogConfiguration& inLogConfiguration)
{
    mStartedWithStream = false;
   return setupListenerIfOK(mPDFWriter.ContinuePDF(inOutputFilePath,inStateFilePath,inOptionalOtherOutputFile,inLogConfiguration));
}

PDFHummus::EStatusCode PDFWriterDriver::ContinuePDF(Handle<Object> inOutputStream,
                                                    const std::string& inStateFilePath,
                                                    Handle<Object> inModifiedSourceStream,
                                                    const LogConfiguration& inLogConfiguration)
{
   mStartedWithStream = true;
   mWriteStreamProxy = new ObjectByteWriterWithPosition(inOutputStream);
    if(!inModifiedSourceStream.IsEmpty())
        mReadStreamProxy = new ObjectByteReaderWithPosition(inModifiedSourceStream);
    
    
    return setupListenerIfOK(mPDFWriter.ContinuePDFForStream(mWriteStreamProxy,inStateFilePath,inModifiedSourceStream.IsEmpty() ? NULL : mReadStreamProxy,inLogConfiguration));
}


PDFHummus::EStatusCode PDFWriterDriver::ModifyPDF(const std::string& inSourceFile,
                                                  EPDFVersion inPDFVersion,
                                                  const std::string& inOptionalOtherOutputFile,
                                                  const LogConfiguration& inLogConfiguration,
                                                  const PDFCreationSettings& inCreationSettings)
{
    // two phase, cause i don't want to bother the users with the level BS.
    // first, parse the source file, get the level. then modify with this level
    
    mStartedWithStream = false;
    return setupListenerIfOK(mPDFWriter.ModifyPDF(inSourceFile,inPDFVersion,inOptionalOtherOutputFile,inLogConfiguration,inCreationSettings));
}

PDFHummus::EStatusCode PDFWriterDriver::ModifyPDF(Handle<Object> inSourceStream,
                                                  Handle<Object> inDestinationStream,
                                                  EPDFVersion inPDFVersion,
                                                  const LogConfiguration& inLogConfiguration,
                                                  const PDFCreationSettings& inCreationSettings)
{
    mStartedWithStream = true;
   
    mWriteStreamProxy = new ObjectByteWriterWithPosition(inDestinationStream);
    mReadStreamProxy = new ObjectByteReaderWithPosition(inSourceStream);
    
    // use minimal leve ePDFVersion10 to use the modified file level (cause i don't care
    return setupListenerIfOK(mPDFWriter.ModifyPDFForStream(mReadStreamProxy,mWriteStreamProxy,false,inPDFVersion,inLogConfiguration,inCreationSettings));
}

METHOD_RETURN_TYPE PDFWriterDriver::CreateFormXObjectFromTIFF(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if((args.Length() != 1 && args.Length() != 2) || (!args[0]->IsString() && !args[0]->IsObject()) || (args.Length() == 2 && !args[1]->IsObject() && !args[1]->IsNumber()))
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument that is the path to the image, and optionally an options object or object ID");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    TIFFUsageParameters tiffUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters();
    ObjectIDType objectID = 0;
    
    if(args.Length() == 2)
    {
        if(args[1]->IsObject())
        {
            Local<Object> anObject = args[1]->ToObject();
            
            // page index parameters
            if(anObject->Has(NEW_STRING("pageIndex")) && anObject->Get(NEW_STRING("pageIndex"))->IsNumber())
                tiffUsageParameters.PageIndex = (unsigned int)anObject->Get(NEW_STRING("pageIndex"))->ToNumber()->Value();
            
            if(anObject->Has(NEW_STRING("bwTreatment")) && anObject->Get(NEW_STRING("bwTreatment"))->IsObject())
            {
                // special black and white treatment
                Local<Object> bwObject = anObject->Get(NEW_STRING("bwTreatment"))->ToObject();
                if(bwObject->Has(NEW_STRING("asImageMask")) && bwObject->Get(NEW_STRING("asImageMask"))->IsBoolean())
                    tiffUsageParameters.BWTreatment.AsImageMask = bwObject->Get(NEW_STRING("asImageMask"))->ToBoolean()->Value();
                if(bwObject->Has(NEW_STRING("oneColor")) && bwObject->Get(NEW_STRING("oneColor"))->IsArray())
                    tiffUsageParameters.BWTreatment.OneColor = colorFromArray(bwObject->Get(NEW_STRING("oneColor")));
            }

            if(anObject->Has(NEW_STRING("grayscaleTreatment")) && anObject->Get(NEW_STRING("grayscaleTreatment"))->IsObject())
            {
                // special black and white treatment
                Local<Object> colormapObject = anObject->Get(NEW_STRING("grayscaleTreatment"))->ToObject();
                if(colormapObject->Has(NEW_STRING("asColorMap")) && colormapObject->Get(NEW_STRING("asColorMap"))->IsBoolean())
                    tiffUsageParameters.GrayscaleTreatment.AsColorMap = colormapObject->Get(NEW_STRING("asColorMap"))->ToBoolean()->Value();
                if(colormapObject->Has(NEW_STRING("oneColor")) && colormapObject->Get(NEW_STRING("oneColor"))->IsArray())
                    tiffUsageParameters.GrayscaleTreatment.OneColor = colorFromArray(colormapObject->Get(NEW_STRING("oneColor")));
                if(colormapObject->Has(NEW_STRING("zeroColor")) && colormapObject->Get(NEW_STRING("zeroColor"))->IsArray())
                    tiffUsageParameters.GrayscaleTreatment.ZeroColor = colorFromArray(colormapObject->Get(NEW_STRING("zeroColor")));
            }
        }
        else // number
        {
            objectID = args[1]->ToNumber()->Int32Value();
        }

    }
    
    PDFFormXObject* formXObject;
    
    if(args[0]->IsObject())
    {
        ObjectByteReaderWithPosition proxy(args[0]->ToObject());
        
        formXObject =
            objectID == 0 ?
                pdfWriter->mPDFWriter.CreateFormXObjectFromTIFFStream(&proxy,tiffUsageParameters):
                pdfWriter->mPDFWriter.CreateFormXObjectFromTIFFStream(&proxy,objectID,tiffUsageParameters);
        
    }
    else
    {
        formXObject =
            objectID == 0 ?
                pdfWriter->mPDFWriter.CreateFormXObjectFromTIFFFile(*String::Utf8Value(args[0]->ToString()),tiffUsageParameters):
                pdfWriter->mPDFWriter.CreateFormXObjectFromTIFFFile(*String::Utf8Value(args[0]->ToString()),objectID,tiffUsageParameters);
    }
    if(!formXObject)
    {
		THROW_EXCEPTION("unable to create form xobject. verify that the target is an existing tiff file");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    Handle<Value> newInstance = FormXObjectDriver::GetNewInstance(args);
    ObjectWrap::Unwrap<FormXObjectDriver>(newInstance->ToObject())->FormXObject = formXObject;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

CMYKRGBColor PDFWriterDriver::colorFromArray(v8::Handle<v8::Value> inArray)
{
	CREATE_ISOLATE_CONTEXT;

    if(inArray->ToObject()->Get(NEW_STRING("length"))->ToObject()->Uint32Value() == 4)
    {
        // cmyk color
        return CMYKRGBColor((unsigned char)inArray->ToObject()->Get(0)->ToNumber()->Value(),
                            (unsigned char)inArray->ToObject()->Get(1)->ToNumber()->Value(),
                            (unsigned char)inArray->ToObject()->Get(2)->ToNumber()->Value(),
                            (unsigned char)inArray->ToObject()->Get(3)->ToNumber()->Value());
        
    }
    else if(inArray->ToObject()->Get(v8::NEW_STRING("length"))->ToObject()->Uint32Value() == 3)
    {
        // rgb color
        return CMYKRGBColor((unsigned char)inArray->ToObject()->Get(0)->ToNumber()->Value(),
                            (unsigned char)inArray->ToObject()->Get(1)->ToNumber()->Value(),
                            (unsigned char)inArray->ToObject()->Get(2)->ToNumber()->Value());
    }
    else
    {
        THROW_EXCEPTION("wrong input for color values. should be array of either 3 or 4 colors");
        return CMYKRGBColor::CMYKBlack();
    }
}

METHOD_RETURN_TYPE PDFWriterDriver::CreateImageXObjectFromJPG(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if((args.Length() != 1 && args.Length() != 2) || (!args[0]->IsString() && !args[0]->IsObject()) || (args.Length() == 2 && !args[1]->IsNumber()))
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument that is the path to the image. pass another optional argument of a forward reference object ID");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
       
   
    PDFImageXObject* imageXObject;
    
    if(args[0]->IsObject())
    {
        ObjectByteReaderWithPosition proxy(args[0]->ToObject());
        
        imageXObject =
            args.Length() == 2 ?
            pdfWriter->mPDFWriter.CreateImageXObjectFromJPGStream(&proxy,(ObjectIDType)args[1]->ToNumber()->Int32Value()) :
            pdfWriter->mPDFWriter.CreateImageXObjectFromJPGStream(&proxy);
    }
    else
    {
        imageXObject =
            args.Length() == 2 ?
            pdfWriter->mPDFWriter.CreateImageXObjectFromJPGFile(*String::Utf8Value(args[0]->ToString()),(ObjectIDType)args[1]->ToNumber()->Int32Value()) :
            pdfWriter->mPDFWriter.CreateImageXObjectFromJPGFile(*String::Utf8Value(args[0]->ToString()));
    }
    if(!imageXObject)
    {
		THROW_EXCEPTION("unable to create image xobject. verify that the target is an existing jpg file");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    Handle<Value> newInstance = ImageXObjectDriver::GetNewInstance(args);
    ObjectWrap::Unwrap<ImageXObjectDriver>(newInstance->ToObject())->ImageXObject = imageXObject;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFWriterDriver::GetObjectsContext(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());

    Handle<Value> newInstance = ObjectsContextDriver::GetNewInstance(args);
    ObjectsContextDriver* objectsContextDriver = ObjectWrap::Unwrap<ObjectsContextDriver>(newInstance->ToObject());
    objectsContextDriver->ObjectsContextInstance = &(pdfWriter->mPDFWriter.GetObjectsContext());
 
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFWriterDriver::GetDocumentContext(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    Handle<Value> newInstance = DocumentContextDriver::GetNewInstance(args);
    DocumentContextDriver* documentContextDriver = ObjectWrap::Unwrap<DocumentContextDriver>(newInstance->ToObject());
    documentContextDriver->DocumentContextInstance = &(pdfWriter->mPDFWriter.GetDocumentContext());
    
    SET_FUNCTION_RETURN_VALUE(newInstance);
}


METHOD_RETURN_TYPE PDFWriterDriver::AppendPDFPagesFromPDF(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if( (args.Length() < 1  && args.Length() > 2) ||
        (!args[0]->IsString() && !args[0]->IsObject()) ||
        (args.Length() >= 2 && !args[1]->IsObject())
       )
    {
		THROW_EXCEPTION("wrong arguments, pass a path for file to append pages from or a stream object, optionally an options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    PDFPageRange pageRange;
    PDFParsingOptions parsingOptions;
    
    if(args.Length() >= 2) {
        Handle<Object> options = args[1]->ToObject();
        if(options->Has(NEW_STRING("password")) && options->Get(NEW_STRING("password"))->IsString())
        {
            parsingOptions.Password = *String::Utf8Value(options->Get(NEW_STRING("password"))->ToString());
        }        
        pageRange = ObjectToPageRange(options);
    }

    EStatusCodeAndObjectIDTypeList result;
    
    if(args[0]->IsObject())
    {
        ObjectByteReaderWithPosition proxy(args[0]->ToObject());
        result = pdfWriter->mPDFWriter.AppendPDFPagesFromPDF(
                                                             &proxy,
                                                             pageRange,
                                                             ObjectIDTypeList(),
                                                             parsingOptions);
    }
    else
    {
        result = pdfWriter->mPDFWriter.AppendPDFPagesFromPDF(
                                                        *String::Utf8Value(args[0]->ToString()),
                                                        pageRange,
                                                        ObjectIDTypeList(),
                                                        parsingOptions);
    }
    
    if(result.first != eSuccess)
    {
		THROW_EXCEPTION("unable to append page, make sure it's fine");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    Local<Array> resultPageIDs = NEW_ARRAY((unsigned int)result.second.size());
    unsigned int index = 0;
    
    ObjectIDTypeList::iterator it = result.second.begin();
    for(; it != result.second.end();++it)
        resultPageIDs->Set(NEW_NUMBER(index++),NEW_NUMBER(*it));
    
    SET_FUNCTION_RETURN_VALUE(resultPageIDs);
}

PDFPageRange PDFWriterDriver::ObjectToPageRange(Handle<Object> inObject)
{
	CREATE_ISOLATE_CONTEXT;
	PDFPageRange pageRange;
        
    if(inObject->Has(NEW_STRING("type")) && inObject->Get(NEW_STRING("type"))->IsNumber())
    {
        pageRange.mType = (PDFPageRange::ERangeType)(inObject->Get(NEW_STRING("type"))->ToNumber()->Uint32Value());
    }

    if(inObject->Has(NEW_STRING("specificRanges")) && inObject->Get(NEW_STRING("specificRanges"))->IsArray())
    {
        Local<Object> anArray = inObject->Get(NEW_STRING("specificRanges"))->ToObject();
        unsigned int length = anArray->Get(v8::NEW_STRING("length"))->ToNumber()->Uint32Value();
        for(unsigned int i=0; i < length; ++i)
        {
            if(!anArray->Get(i)->IsArray() ||
               anArray->Get(i)->ToObject()->Get(v8::NEW_STRING("length"))->ToNumber()->Uint32Value() != 2)
            {
                THROW_EXCEPTION("wrong argument for specificRanges. it should be an array of arrays. each subarray should be of the length of 2, signifying begining page and ending page numbers");
                break;
            }
            Local<Object> item = anArray->Get(i)->ToObject();
            if(!item->Get(0)->IsNumber() || !item->Get(1)->IsNumber())
            {
                THROW_EXCEPTION("wrong argument for specificRanges. it should be an array of arrays. each subarray should be of the length of 2, signifying begining page and ending page numbers");
                break;
            }
            pageRange.mSpecificRanges.push_back(ULongAndULong(
                                                              item->Get(0)->ToNumber()->Uint32Value(),
                                                              item->Get(1)->ToNumber()->Uint32Value()));
            
        }
    }
    
    return pageRange;
}

class MergeInterpageCallbackCaller : public DocumentContextExtenderAdapter
{
public:
	EStatusCode OnAfterMergePageFromPage(
                                         PDFPage* inTargetPage,
                                         PDFDictionary* inPageObjectDictionary,
                                         ObjectsContext* inPDFWriterObjectContext,
                                         DocumentContext* inPDFWriterDocumentContext,
                                         PDFDocumentHandler* inPDFDocumentHandler)
	{
        if(!callback.IsEmpty())
        {
            const unsigned argc = 0;
            callback->Call(GET_CURRENT_CONTEXT->Global(), argc, NULL);
        }
		return PDFHummus::eSuccess;
	}
    
    bool IsValid(){return !callback.IsEmpty();}

    Local<Function> callback;
};

METHOD_RETURN_TYPE PDFWriterDriver::MergePDFPagesToPage(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    /*
        parameters are:
            target page
            file path to pdf to merge pages from OR stream of pdf to merge pages from
            optional 1: options object
            optional 2: callback function to call after each page merge
     */
    
    if(args.Length() < 2)
    {
		THROW_EXCEPTION("Too few arguments. Pass a page object, a path to pages source file or an IByteReaderWithPosition, and two optional: configuration object and callback function that will be called between pages merging");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    if(!PDFPageDriver::HasInstance(args[0]))
    {
		THROW_EXCEPTION("Invalid arguments. First argument must be a page object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);        
    }
    
    if(!args[1]->IsString() && 
       !args[1]->IsObject())
    {
		THROW_EXCEPTION("Invalid arguments. Second argument must be either an input stream or a path to a pages source file.");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    PDFPageDriver* page = ObjectWrap::Unwrap<PDFPageDriver>(args[0]->ToObject());
    
    PDFPageRange pageRange;
    PDFParsingOptions parsingOptions; 
    
    // get page range
    if(args.Length() > 2 && args[2]->IsObject()) {
        Handle<Object> options = args[2]->ToObject();
        if(options->Has(NEW_STRING("password")) && options->Get(NEW_STRING("password"))->IsString())
        {
            parsingOptions.Password = *String::Utf8Value(options->Get(NEW_STRING("password"))->ToString());
        }        
        pageRange = ObjectToPageRange(options);
    } 
    else if(args.Length() > 3 && args[3]->IsObject()) {
        Handle<Object> options = args[3]->ToObject();
        if(options->Has(NEW_STRING("password")) && options->Get(NEW_STRING("password"))->IsString())
        {
            parsingOptions.Password = *String::Utf8Value(options->Get(NEW_STRING("password"))->ToString());
        }        
        pageRange = ObjectToPageRange(options);
    }
    
    // now see if there's a need for activating the callback. will do that using the document extensibility option of the lib
    MergeInterpageCallbackCaller caller;
    if((args.Length() > 2 && args[2]->IsFunction()) ||
       (args.Length() > 3 && args[3]->IsFunction()))
        caller.callback = Local<Function>::Cast(args[2]->IsFunction() ? args[2] : args[3]);
    if(caller.IsValid())
        pdfWriter->mPDFWriter.GetDocumentContext().AddDocumentContextExtender(&caller);
    
    EStatusCode status;
    if(args[1]->IsString()) 
    {
        status = pdfWriter->mPDFWriter.MergePDFPagesToPage(page->GetPage(),
                                                           *String::Utf8Value(args[1]->ToString()),
                                                           pageRange,
                                                           ObjectIDTypeList(),
                                                           parsingOptions);
    }
    else 
    {
		ObjectByteReaderWithPosition proxy(args[1]->ToObject());
        status = pdfWriter->mPDFWriter.MergePDFPagesToPage(page->GetPage(),
                                                           &proxy,
                                                           pageRange,
                                                           ObjectIDTypeList(),
                                                           parsingOptions);
    }
	
    if(caller.IsValid())
        pdfWriter->mPDFWriter.GetDocumentContext().RemoveDocumentContextExtender(&caller);

    if(status != eSuccess)
    {
		THROW_EXCEPTION("unable to append to page, make sure source file exists");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    SET_FUNCTION_RETURN_VALUE(args.This());
}

METHOD_RETURN_TYPE PDFWriterDriver::CreatePDFCopyingContext(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if( (args.Length() < 1  && args.Length() > 2) ||
        (!args[0]->IsString() && !args[0]->IsObject()) ||
        (args.Length() >= 2 && !args[1]->IsObject())
       )
    {
		THROW_EXCEPTION("wrong arguments, pass a path to a PDF file to create copying context for or a stream object, and then an optional options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());

    PDFDocumentCopyingContext* copyingContext;

    ObjectByteReaderWithPosition* proxy = NULL;
    PDFParsingOptions parsingOptions;

    if(args.Length() >= 2) {
        Handle<Object> options = args[1]->ToObject();
        if(options->Has(NEW_STRING("password")) && options->Get(NEW_STRING("password"))->IsString())
        {
            parsingOptions.Password = *String::Utf8Value(options->Get(NEW_STRING("password"))->ToString());
        }        
    }

    
    if(args[0]->IsObject())
    {
        if(PDFReaderDriver::HasInstance(args[0]))
        {
            // parser based copying context  [note that here parsingOptions doesn't matter as the parser creation already took it into account]

            PDFParser* theParser = ObjectWrap::Unwrap<PDFReaderDriver>(args[0]->ToObject())->GetParser();
            copyingContext = pdfWriter->mPDFWriter.GetDocumentContext().CreatePDFCopyingContext(theParser);
        }
        else
        {
            // stream based copying context

            proxy = new ObjectByteReaderWithPosition(args[0]->ToObject());
            copyingContext = pdfWriter->mPDFWriter.CreatePDFCopyingContext(proxy,parsingOptions);
        }
    }
    else
    {
        // file path based copying context
        copyingContext = pdfWriter->mPDFWriter.CreatePDFCopyingContext(*String::Utf8Value(args[0]->ToString()),parsingOptions);
    }
    
    if(!copyingContext)
    {
		THROW_EXCEPTION("unable to create copying context. verify that the target is an existing PDF file");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    Handle<Value> newInstance = DocumentCopyingContextDriver::GetNewInstance(args);
    ObjectWrap::Unwrap<DocumentCopyingContextDriver>(newInstance->ToObject())->CopyingContext = copyingContext;
    ObjectWrap::Unwrap<DocumentCopyingContextDriver>(newInstance->ToObject())->ReadStreamProxy = proxy;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFWriterDriver::CreateFormXObjectsFromPDF(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() < 1  ||
       args.Length() > 5 ||
       !args[0]->IsString() ||
       (args.Length() >= 2 && (!args[1]->IsNumber() && !args[1]->IsArray())) ||
       (args.Length() >= 3 && !args[2]->IsObject()) ||
       (args.Length() >= 4 && !args[3]->IsArray()) ||
       (args.Length() == 5 && !args[4]->IsArray())
       )
    {
		THROW_EXCEPTION("wrong arguments, pass a path to the file, and optionals - a box enumerator or actual 4 numbers box, a range object, a matrix for the form, array of object ids to copy in addition");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    PDFPageRange pageRange;
    PDFParsingOptions parsingOptions;
    
    if(args.Length() >= 3) {
        Handle<Object> options = args[2]->ToObject();
        if(options->Has(NEW_STRING("password")) && options->Get(NEW_STRING("password"))->IsString())
        {
            parsingOptions.Password = *String::Utf8Value(options->Get(NEW_STRING("password"))->ToString());
        }
        pageRange = ObjectToPageRange(options);
    }
    
    EStatusCodeAndObjectIDTypeList result;
    double matrixBuffer[6];
    double* transformationMatrix = NULL;
    
    if(args.Length() >= 4)
    {
        Handle<Object> matrixArray = args[3]->ToObject();
        if(matrixArray->Get(v8::NEW_STRING("length"))->ToObject()->Uint32Value() != 6)
        {
            THROW_EXCEPTION("matrix array should be 6 numbers long");
            SET_FUNCTION_RETURN_VALUE(UNDEFINED);
        }
        
        for(int i=0;i<6;++i)
            matrixBuffer[i] = matrixArray->Get(i)->ToNumber()->Value();
        transformationMatrix = matrixBuffer;
    }
    
    ObjectIDTypeList extraObjectsList;
    if(args.Length() >= 5)
    {
        Handle<Object> objectsIDsArray = args[4]->ToObject();
        unsigned int arrayLength = objectsIDsArray->Get(v8::NEW_STRING("length"))->ToObject()->Uint32Value();
        for(unsigned int i=0;i<arrayLength;++i)
            extraObjectsList.push_back((ObjectIDType)(objectsIDsArray->Get(i)->ToNumber()->Uint32Value()));
            
    }
    
    if(args[1]->IsArray())
    {
        Handle<Object> boxArray = args[1]->ToObject();
        if(boxArray->Get(v8::NEW_STRING("length"))->ToObject()->Uint32Value() != 4)
        {
            THROW_EXCEPTION("box dimensions array should be 4 numbers long");
            SET_FUNCTION_RETURN_VALUE(UNDEFINED);
        }
        
        PDFRectangle box(boxArray->Get(0)->ToNumber()->Value(),
                            boxArray->Get(1)->ToNumber()->Value(),
                            boxArray->Get(2)->ToNumber()->Value(),
                            boxArray->Get(3)->ToNumber()->Value());
        
        result = pdfWriter->mPDFWriter.CreateFormXObjectsFromPDF(
                                                                 *String::Utf8Value(args[0]->ToString()),
                                                                 pageRange,
                                                                 box,
                                                                 transformationMatrix,
                                                                 extraObjectsList,
                                                                 parsingOptions);
    }
    else
    {
        result = pdfWriter->mPDFWriter.CreateFormXObjectsFromPDF(
                                                                *String::Utf8Value(args[0]->ToString()),
                                                                pageRange,
                                                                (EPDFPageBox)args[1]->ToNumber()->Uint32Value(),
                                                                 transformationMatrix,
                                                                 extraObjectsList,
                                                                 parsingOptions);
    }
    
    if(result.first != eSuccess)
    {
		THROW_EXCEPTION("unable to create forms from file. make sure the file exists, and that the input page range is valid (well, if you provided one..m'k?");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    Local<Array> resultFormIDs = NEW_ARRAY((unsigned int)result.second.size());
    unsigned int index = 0;
    
    ObjectIDTypeList::iterator it = result.second.begin();
    for(; it != result.second.end();++it)
        resultFormIDs->Set(NEW_NUMBER(index++),NEW_NUMBER(*it));
    
    SET_FUNCTION_RETURN_VALUE(resultFormIDs);
}
 
METHOD_RETURN_TYPE PDFWriterDriver::CreatePDFCopyingContextForModifiedFile(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    PDFDocumentCopyingContext* copyingContext = pdfWriter->mPDFWriter.CreatePDFCopyingContextForModifiedFile();
    if(!copyingContext)
    {
		THROW_EXCEPTION("unable to create copying context for modified file...possibly a file is not being modified by this writer...");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    Handle<Value> newInstance = DocumentCopyingContextDriver::GetNewInstance(args);
    ObjectWrap::Unwrap<DocumentCopyingContextDriver>(newInstance->ToObject())->CopyingContext = copyingContext;
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFWriterDriver::CreatePDFTextString(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    SET_FUNCTION_RETURN_VALUE(PDFTextStringDriver::GetNewInstance(args));
    
}

METHOD_RETURN_TYPE PDFWriterDriver::CreatePDFDate(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    SET_FUNCTION_RETURN_VALUE(PDFDateDriver::GetNewInstance(args));
    
}

PDFWriter* PDFWriterDriver::GetWriter()
{
    return &mPDFWriter;
}

METHOD_RETURN_TYPE PDFWriterDriver::GetImageDimensions(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() < 1 || args.Length() > 3 ||
       !args[0]->IsString() ||
       (args.Length()>=2 && !args[1]->IsNumber()) ||
       (args.Length() >= 3 && !args[2]->IsObject())
       )
    {
		THROW_EXCEPTION("wrong arguments, pass 1 to 3 arguments. a path to an image, an optional image index (for multi-image files), and an options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());

    PDFParsingOptions parsingOptions;

    if(args.Length() >= 3) {
        Handle<Object> options = args[2]->ToObject();
        if(options->Has(NEW_STRING("password")) && options->Get(NEW_STRING("password"))->IsString())
        {
            parsingOptions.Password = *String::Utf8Value(options->Get(NEW_STRING("password"))->ToString());
        }        
    }

    DoubleAndDoublePair dimensions = pdfWriter->mPDFWriter.GetImageDimensions(
                                  *String::Utf8Value(args[0]->ToString()),
                                  args.Length() >= 2 ? args[1]->ToNumber()->Uint32Value() : 0,
                                  parsingOptions);
    
    Handle<Object> newObject = NEW_OBJECT;
    
    newObject->Set(NEW_STRING("width"),NEW_NUMBER(dimensions.first));
    newObject->Set(NEW_STRING("height"),NEW_NUMBER(dimensions.second));
    SET_FUNCTION_RETURN_VALUE(newObject);
};


METHOD_RETURN_TYPE PDFWriterDriver::GetImagePagesCount(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    if(args.Length() < 1 || args.Length() > 2 ||
       !args[0]->IsString() ||
       (args.Length() >= 2 && !args[1]->IsObject())
       )
	{
		THROW_EXCEPTION("wrong arguments, pass 1 argument and an optional one. a path to an image, and an options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
	}

	PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    PDFParsingOptions parsingOptions;

    if(args.Length() >= 2) {
        Handle<Object> options = args[1]->ToObject();
        if(options->Has(NEW_STRING("password")) && options->Get(NEW_STRING("password"))->IsString())
        {
            parsingOptions.Password = *String::Utf8Value(options->Get(NEW_STRING("password"))->ToString());
        }        
    }    

	unsigned long result = pdfWriter->mPDFWriter.GetImagePagesCount(*String::Utf8Value(args[0]->ToString()),parsingOptions);

	SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(result));
}

METHOD_RETURN_TYPE PDFWriterDriver::GetImageType(const ARGS_TYPE& args) {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	if (args.Length() != 1 ||
		!args[0]->IsString())
	{
		THROW_EXCEPTION("wrong arguments, pass 1 argument. a path to an imag");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
	}

	PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());

    PDFHummus::EHummusImageType imageType = pdfWriter->mPDFWriter.GetImageType(*String::Utf8Value(args[0]->ToString()),0);
        
    switch(imageType)
    {
        case PDFHummus::ePDF:
        {
            SET_FUNCTION_RETURN_VALUE(NEW_STRING("PDF"));
            break;
        }
        case PDFHummus::eJPG:
        {
            SET_FUNCTION_RETURN_VALUE(NEW_STRING("JPEG"));
            break;
        }
        case PDFHummus::eTIFF:
        {
            SET_FUNCTION_RETURN_VALUE(NEW_STRING("TIFF"));
            break;
        }
        default:
        {
            SET_FUNCTION_RETURN_VALUE(UNDEFINED);    
        }
    }
}

METHOD_RETURN_TYPE PDFWriterDriver::GetModifiedFileParser(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    PDFParser* parser = &(pdfWriter->mPDFWriter.GetModifiedFileParser());
    if(!parser->GetTrailer()) // checking for the trailer should be a good indication to whether this parser is relevant
    {
		THROW_EXCEPTION("unable to create modified parser...possibly a file is not being modified by this writer...");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    Handle<Value> newInstance = PDFReaderDriver::GetNewInstance(args);
    ObjectWrap::Unwrap<PDFReaderDriver>(newInstance->ToObject())->SetFromOwnedParser(parser);
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFWriterDriver::GetModifiedInputFile(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    InputFile* inputFile = &(pdfWriter->mPDFWriter.GetModifiedInputFile());
    if(!inputFile->GetInputStream()) 
    {
		THROW_EXCEPTION("unable to create modified input file...possibly a file is not being modified by this writer...");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    Handle<Value> newInstance = InputFileDriver::GetNewInstance(args);
    ObjectWrap::Unwrap<InputFileDriver>(newInstance->ToObject())->SetFromOwnedFile(inputFile);
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFWriterDriver::GetOutputFile(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    OutputFile* outputFile = &(pdfWriter->mPDFWriter.GetOutputFile());
    if(!outputFile->GetOutputStream())
    {
		THROW_EXCEPTION("unable to get output file. probably pdf writing hasn't started, or the output is not to a file");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    Handle<Value> newInstance = OutputFileDriver::GetNewInstance(args);
    ObjectWrap::Unwrap<OutputFileDriver>(newInstance->ToObject())->SetFromOwnedFile(outputFile);
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFWriterDriver::RegisterAnnotationReferenceForNextPageWrite(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
        THROW_EXCEPTION("wrong arguments,  pass an object ID for an annotation to register");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }

    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    pdfWriter->mPDFWriter.GetDocumentContext().RegisterAnnotationReferenceForNextPageWrite(args[0]->ToNumber()->Uint32Value());

    SET_FUNCTION_RETURN_VALUE(args.This());
}

/*
    From now on, extensions event triggers.
    got the following events for now:
    
    OnPageWrite: {
                    page:PDFPage,
                    pageDictionaryContext:DictionaryContext
                }
    OnResourcesWrite {
                resources: ResourcesDictionary
                pageResourcesDictionaryContext: DictionaryContext
    }
    OnResourceDictionaryWrite {
                resourceDictionaryName: string
                resourceDictionary: DictionaryContext	
    }
    OnCatalogWrite {
                catalogDictionaryContext: DictionaryContext
    }
*/

PDFHummus::EStatusCode PDFWriterDriver::OnPageWrite(
                        PDFPage* inPage,
                        DictionaryContext* inPageDictionaryContext,
                        ObjectsContext* inPDFWriterObjectContext,
                        PDFHummus::DocumentContext* inDocumentContext) {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    Handle<Object> params = NEW_OBJECT;

	params->Set(NEW_SYMBOL("page"),PDFPageDriver::GetNewInstance(inPage));
	params->Set(NEW_SYMBOL("pageDictionaryContext"),DictionaryContextDriver::GetInstanceFor(inPageDictionaryContext));
    return triggerEvent("OnPageWrite",params);        
}
PDFHummus::EStatusCode PDFWriterDriver::OnResourcesWrite(
                        ResourcesDictionary* inResources,
                        DictionaryContext* inPageResourcesDictionaryContext,
                        ObjectsContext* inPDFWriterObjectContext,
                        PDFHummus::DocumentContext* inDocumentContext) {
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    Handle<Object> params = NEW_OBJECT;

	params->Set(NEW_SYMBOL("resources"),ResourcesDictionaryDriver::GetInstanceFor(inResources));
	params->Set(NEW_SYMBOL("pageResourcesDictionaryContext"),DictionaryContextDriver::GetInstanceFor(inPageResourcesDictionaryContext));
    return triggerEvent("OnResourcesWrite",params);        
}

PDFHummus::EStatusCode PDFWriterDriver::OnResourceDictionaryWrite(
                        DictionaryContext* inResourceDictionary,
                        const std::string& inResourceDictionaryName,
                        ObjectsContext* inPDFWriterObjectContext,
                        PDFHummus::DocumentContext* inDocumentContext) {
     CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    Handle<Object> params = NEW_OBJECT;

	params->Set(NEW_SYMBOL("resourceDictionaryName"),NEW_STRING(inResourceDictionaryName.c_str()));
	params->Set(NEW_SYMBOL("resourceDictionary"),DictionaryContextDriver::GetInstanceFor(inResourceDictionary));
    return triggerEvent("OnResourceDictionaryWrite",params);        
}

PDFHummus::EStatusCode PDFWriterDriver::OnFormXObjectWrite(
                        ObjectIDType inFormXObjectID,
                        ObjectIDType inFormXObjectResourcesDictionaryID,
                        DictionaryContext* inFormDictionaryContext,
                        ObjectsContext* inPDFWriterObjectContext,
                        PDFHummus::DocumentContext* inDocumentContext) {
                            
    return PDFHummus::eSuccess;                               
}
PDFHummus::EStatusCode PDFWriterDriver::OnJPEGImageXObjectWrite(
                        ObjectIDType inImageXObjectID,
                        DictionaryContext* inImageDictionaryContext,
                        ObjectsContext* inPDFWriterObjectContext,
                        PDFHummus::DocumentContext* inDocumentContext,
                        JPEGImageHandler* inJPGImageHandler) {
                            
    return PDFHummus::eSuccess;                               
}
PDFHummus::EStatusCode PDFWriterDriver::OnTIFFImageXObjectWrite(
                        ObjectIDType inImageXObjectID,
                        DictionaryContext* inImageDictionaryContext,
                        ObjectsContext* inPDFWriterObjectContext,
                        PDFHummus::DocumentContext* inDocumentContext,
                        TIFFImageHandler* inTIFFImageHandler) {
                            
    return PDFHummus::eSuccess;                               
}

PDFHummus::EStatusCode PDFWriterDriver::triggerEvent(const std::string& inEventName, v8::Handle<v8::Object> inParams) {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Handle<Value> value = THIS_HANDLE->Get(NEW_STRING("triggerDocumentExtensionEvent"));
    if(value->IsUndefined())
        return PDFHummus::eFailure;
    Handle<Function> func = Handle<Function>::Cast(value);
    Handle<Value> args[2];
    args[0] = NEW_STRING(inEventName.c_str());
    args[1] = inParams;
	func->Call(THIS_HANDLE, 2, args);                
    return PDFHummus::eSuccess; 
}


PDFHummus::EStatusCode PDFWriterDriver::OnCatalogWrite(
                        CatalogInformation* inCatalogInformation,
                        DictionaryContext* inCatalogDictionaryContext,
                        ObjectsContext* inPDFWriterObjectContext,
                        PDFHummus::DocumentContext* inDocumentContext) {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    Handle<Object> params = NEW_OBJECT;

    // this is the only important one
	params->Set(NEW_SYMBOL("catalogDictionaryContext"),DictionaryContextDriver::GetInstanceFor(inCatalogDictionaryContext));
    return triggerEvent("OnCatalogWrite",params);                               
}


PDFHummus::EStatusCode PDFWriterDriver::OnPDFParsingComplete(
                        ObjectsContext* inPDFWriterObjectContext,
                        PDFHummus::DocumentContext* inDocumentContext,
                        PDFDocumentHandler* inPDFDocumentHandler) {
                            
    return PDFHummus::eSuccess;                               
}
PDFHummus::EStatusCode PDFWriterDriver::OnBeforeCreateXObjectFromPage(
                        PDFDictionary* inPageObjectDictionary,
                        ObjectsContext* inPDFWriterObjectContext,
                        PDFHummus::DocumentContext* inDocumentContext,
                        PDFDocumentHandler* inPDFDocumentHandler) {
                            
    return PDFHummus::eSuccess;                               
}
PDFHummus::EStatusCode PDFWriterDriver::OnAfterCreateXObjectFromPage(
                        PDFFormXObject* iPageObjectResultXObject,
                        PDFDictionary* inPageObjectDictionary,
                        ObjectsContext* inPDFWriterObjectContext,
                        PDFHummus::DocumentContext* inDocumentContext,
                        PDFDocumentHandler* inPDFDocumentHandler) {
                            
    return PDFHummus::eSuccess;                               
}
PDFHummus::EStatusCode PDFWriterDriver::OnBeforeCreatePageFromPage(
                        PDFDictionary* inPageObjectDictionary,
                        ObjectsContext* inPDFWriterObjectContext,
                        PDFHummus::DocumentContext* inDocumentContext,
                        PDFDocumentHandler* inPDFDocumentHandler) {
                            
    return PDFHummus::eSuccess;                               
}
PDFHummus::EStatusCode PDFWriterDriver::OnAfterCreatePageFromPage(
                        PDFPage* iPageObjectResultPage,
                        PDFDictionary* inPageObjectDictionary,
                        ObjectsContext* inPDFWriterObjectContext,
                        PDFHummus::DocumentContext* inDocumentContext,
                        PDFDocumentHandler* inPDFDocumentHandler) {
                            
    return PDFHummus::eSuccess;                               
}
PDFHummus::EStatusCode PDFWriterDriver::OnBeforeMergePageFromPage(
                        PDFPage* inTargetPage,
                        PDFDictionary* inPageObjectDictionary,
                        ObjectsContext* inPDFWriterObjectContext,
                        PDFHummus::DocumentContext* inDocumentContext,
                        PDFDocumentHandler* inPDFDocumentHandler) {
                            
    return PDFHummus::eSuccess;                               
}
PDFHummus::EStatusCode PDFWriterDriver::OnAfterMergePageFromPage(
                        PDFPage* inTargetPage,
                        PDFDictionary* inPageObjectDictionary,
                        ObjectsContext* inPDFWriterObjectContext,
                        PDFHummus::DocumentContext* inDocumentContext,
                        PDFDocumentHandler* inPDFDocumentHandler) {
                            
    return PDFHummus::eSuccess;                               
}
PDFHummus::EStatusCode PDFWriterDriver::OnPDFCopyingComplete(
                        ObjectsContext* inPDFWriterObjectContext,
                        PDFHummus::DocumentContext* inDocumentContext,
                        PDFDocumentHandler* inPDFDocumentHandler) {
                 
    return PDFHummus::eSuccess;                               
}
bool PDFWriterDriver::IsCatalogUpdateRequiredForModifiedFile(PDFParser* inModifiderFileParser) {
    
    return false;
}

PDFHummus::EStatusCode PDFWriterDriver::setupListenerIfOK(PDFHummus::EStatusCode inCode) {
    if(inCode == PDFHummus::eSuccess)
        mPDFWriter.GetDocumentContext().AddDocumentContextExtender(this);
    return inCode;
}