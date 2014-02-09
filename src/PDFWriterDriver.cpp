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

void PDFWriterDriver::Init()
{
    // prepare the pdfwriter interfrace template
    Local<FunctionTemplate> pdfWriterFT = FunctionTemplate::New(New);
    pdfWriterFT->SetClassName(String::NewSymbol("PDFWriter"));
    pdfWriterFT->InstanceTemplate()->SetInternalFieldCount(1);
    
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("end"),FunctionTemplate::New(End)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createPage"),FunctionTemplate::New(CreatePage)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("writePage"),FunctionTemplate::New(WritePage)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("writePageAndReturnID"),FunctionTemplate::New(WritePageAndReturnID)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("startPageContentContext"),FunctionTemplate::New(StartPageContentContext)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("pausePageContentContext"),FunctionTemplate::New(PausePageContentContext)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createFormXObject"),FunctionTemplate::New(CreateFormXObject)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("endFormXObject"),FunctionTemplate::New(EndFormXObject)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createFormXObjectFromJPG"),FunctionTemplate::New(CreateformXObjectFromJPG)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("getFontForFile"),FunctionTemplate::New(GetFontForFile)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("attachURLLinktoCurrentPage"),FunctionTemplate::New(AttachURLLinktoCurrentPage)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("shutdown"),FunctionTemplate::New(Shutdown)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createFormXObjectFromTIFF"),FunctionTemplate::New(CreateFormXObjectFromTIFF)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createImageXObjectFromJPG"),FunctionTemplate::New(CreateImageXObjectFromJPG)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("retrieveJPGImageInformation"),FunctionTemplate::New(RetrieveJPGImageInformation)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("getObjectsContext"),FunctionTemplate::New(GetObjectsContext)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("getDocumentContext"),FunctionTemplate::New(GetDocumentContext)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("appendPDFPagesFromPDF"),FunctionTemplate::New(AppendPDFPagesFromPDF)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("mergePDFPagesToPage"),FunctionTemplate::New(MergePDFPagesToPage)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createPDFCopyingContext"),FunctionTemplate::New(CreatePDFCopyingContext)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createFormXObjectsFromPDF"),FunctionTemplate::New(CreateFormXObjectsFromPDF)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createPDFCopyingContextForModifiedFile"),FunctionTemplate::New(CreatePDFCopyingContextForModifiedFile)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createPDFTextString"),FunctionTemplate::New(CreatePDFTextString)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createPDFDate"),FunctionTemplate::New(CreatePDFDate)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("getImageDimensions"),FunctionTemplate::New(SGetImageDimensions)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("getModifiedFileParser"),FunctionTemplate::New(GetModifiedFileParser)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("getModifiedInputFile"),FunctionTemplate::New(GetModifiedInputFile)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("getOutputFile"),FunctionTemplate::New(GetOutputFile)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("registerAnnotationReferenceForNextPageWrite"),FunctionTemplate::New(RegisterAnnotationReferenceForNextPageWrite)->GetFunction());

    constructor = Persistent<Function>::New(pdfWriterFT->GetFunction());
}

Handle<Value> PDFWriterDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    Local<Object> instance = constructor->NewInstance();
    return scope.Close(instance);
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
    
    EStatusCode status;
    
    if(pdfWriter->mStartedWithStream)
        status = pdfWriter->mPDFWriter.EndPDFForStream();
    else
        status = pdfWriter->mPDFWriter.EndPDF();
    
    if(status != PDFHummus::eSuccess)
    {
		ThrowException(Exception::TypeError(String::New("Unable to end PDF")));
		return scope.Close(Undefined());
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
    
    WritePageAndReturnID(args);
    
    return scope.Close(args.This());
    
}

v8::Handle<v8::Value> PDFWriterDriver::WritePageAndReturnID(const v8::Arguments& args)
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
    
    EStatusCodeAndObjectIDType result = pdfWriter->mPDFWriter.WritePageAndReturnPageID(pageDriver->GetPage());
    
    if(result.first != PDFHummus::eSuccess)
    {
		ThrowException(Exception::TypeError(String::New("Unable to write page")));
		return scope.Close(Undefined());
    }
    
    return scope.Close(Number::New(result.second));
    
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
    
    // set pdf driver, so context can use it for central registry of the PDF
    contentContextDriver->SetPDFWriter(pdfWriter);

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
    
    if((args.Length() != 4  && args.Length() != 5) || !args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber()
        || (args.Length() == 5 && !args[4]->IsNumber()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 4 coordinates of the form rectangle and an optional 5th agument which is the forward reference ID")));
		return scope.Close(Undefined());
    }
     
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    Handle<Value> newInstance = FormXObjectDriver::NewInstance(args);
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
    // set pdf driver, so context can use it for central registry of the PDF
    formXObjectDriver->SetPDFWriter(pdfWriter);
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



v8::Handle<v8::Value> PDFWriterDriver::CreateformXObjectFromJPG(const v8::Arguments& args)
{
    HandleScope scope;
    
    if((args.Length() != 1  && args.Length() != 2 ) || (!args[0]->IsString() && !args[0]->IsObject()) || (args.Length() == 2 && !args[1]->IsNumber()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is the path to the image. Optionally pass an object ID for a forward reference image")));
		return scope.Close(Undefined());
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
		ThrowException(Exception::Error(String::New("unable to create form xobject. verify that the target is an existing jpg file/stream")));
		return scope.Close(Undefined());
    }
    
    Handle<Value> newInstance = FormXObjectDriver::NewInstance(args);
    ObjectWrap::Unwrap<FormXObjectDriver>(newInstance->ToObject())->FormXObject = formXObject;
    return scope.Close(newInstance);
}

v8::Handle<v8::Value> PDFWriterDriver::RetrieveJPGImageInformation(const v8::Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1  ||
       !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is the path to the image")));
		return scope.Close(Undefined());
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    BoolAndJPEGImageInformation info = pdfWriter->mPDFWriter.GetDocumentContext().GetJPEGImageHandler().RetrieveImageInformation(*String::Utf8Value(args[0]->ToString()));
    
    if(!info.first)
    {
		ThrowException(Exception::Error(String::New("unable to retrieve image information")));
		return scope.Close(Undefined());
    }
    
    Handle<Object> result = Object::New();
    
    result->Set(String::NewSymbol("samplesWidth"),Integer::New((int)info.second.SamplesWidth));
    result->Set(String::NewSymbol("samplesHeight"),Integer::New((int)info.second.SamplesHeight));
    result->Set(String::NewSymbol("colorComponentsCount"),Integer::New(info.second.ColorComponentsCount));
    result->Set(String::NewSymbol("JFIFInformationExists"),Boolean::New(info.second.JFIFInformationExists));
    if(info.second.JFIFInformationExists)
    {
        result->Set(String::NewSymbol("JFIFUnit"),Integer::New(info.second.JFIFUnit));
        result->Set(String::NewSymbol("JFIFXDensity"),Number::New(info.second.JFIFXDensity));
        result->Set(String::NewSymbol("JFIFYDensity"),Number::New(info.second.JFIFYDensity));
    }
    result->Set(String::NewSymbol("ExifInformationExists"),Boolean::New(info.second.ExifInformationExists));
    if(info.second.ExifInformationExists)
    {
        result->Set(String::NewSymbol("ExifUnit"),Integer::New(info.second.ExifUnit));
        result->Set(String::NewSymbol("ExifXDensity"),Number::New(info.second.ExifXDensity));
        result->Set(String::NewSymbol("ExifYDensity"),Number::New(info.second.ExifYDensity));
    }
    result->Set(String::NewSymbol("PhotoshopInformationExists"),Boolean::New(info.second.PhotoshopInformationExists));
    if(info.second.PhotoshopInformationExists)
    {
        result->Set(String::NewSymbol("PhotoshopXDensity"),Number::New(info.second.PhotoshopXDensity));
        result->Set(String::NewSymbol("PhotoshopYDensity"),Number::New(info.second.PhotoshopYDensity));
    }

    return scope.Close(result);
}

v8::Handle<v8::Value> PDFWriterDriver::GetFontForFile(const v8::Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() < 1 ||
        !args[0]->IsString() ||
                (args.Length() == 2 && !args[1]->IsString() && !args[1]->IsNumber()) ||
                (args.Length() == 3 && !args[1]->IsString() && !args[2]->IsNumber()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is the path to the font file, with option to a 2nd parameter for another path in case of type 1 font. another optional argument may follow with font index in case of font packages (TTC, DFont)")));
		return scope.Close(Undefined());
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
                                                 EPDFVersion inPDFVersion,
                                                 const LogConfiguration& inLogConfiguration,
                                                 const PDFCreationSettings& inCreationSettings)
{
    mStartedWithStream = false;
    
    return mPDFWriter.StartPDF(inOutputFilePath,inPDFVersion,inLogConfiguration,inCreationSettings);
}

PDFHummus::EStatusCode PDFWriterDriver::StartPDF(Handle<Object> inWriteStream,
                                                 EPDFVersion inPDFVersion,
                                                 const LogConfiguration& inLogConfiguration,
                                                 const PDFCreationSettings& inCreationSettings)
{

    mWriteStreamProxy = new ObjectByteWriterWithPosition(inWriteStream);
    mStartedWithStream = true;
    return mPDFWriter.StartPDFForStream(mWriteStreamProxy,inPDFVersion,inLogConfiguration,inCreationSettings);
}



PDFHummus::EStatusCode PDFWriterDriver::ContinuePDF(const std::string& inOutputFilePath,
                                                    const std::string& inStateFilePath,
                                                    const std::string& inOptionalOtherOutputFile,
                                                    const LogConfiguration& inLogConfiguration)
{
    mStartedWithStream = false;
   return mPDFWriter.ContinuePDF(inOutputFilePath,inStateFilePath,inOptionalOtherOutputFile,inLogConfiguration);
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
    
    
    return mPDFWriter.ContinuePDFForStream(mWriteStreamProxy,inStateFilePath,inModifiedSourceStream.IsEmpty() ? NULL : mReadStreamProxy,inLogConfiguration);
}


PDFHummus::EStatusCode PDFWriterDriver::ModifyPDF(const std::string& inSourceFile,
                                                  EPDFVersion inPDFVersion,
                                                  const std::string& inOptionalOtherOutputFile,
                                                  const LogConfiguration& inLogConfiguration,
                                                  const PDFCreationSettings& inCreationSettings)
{
    // two phase, cause i don't want to bother the users with the level BS.
    // first, parse the source file, get the level. then modify with this level
    
    PDFHummus::EStatusCode status;
    mStartedWithStream = false;
    
    status = mPDFWriter.ModifyPDF(inSourceFile,inPDFVersion,inOptionalOtherOutputFile,inLogConfiguration,inCreationSettings);
    
    return status;
}

PDFHummus::EStatusCode PDFWriterDriver::ModifyPDF(Handle<Object> inSourceStream,
                                                  Handle<Object> inDestinationStream,
                                                  EPDFVersion inPDFVersion,
                                                  const LogConfiguration& inLogConfiguration,
                                                  const PDFCreationSettings& inCreationSettings)
{
    PDFHummus::EStatusCode status;
    mStartedWithStream = true;
   
    mWriteStreamProxy = new ObjectByteWriterWithPosition(inDestinationStream);
    mReadStreamProxy = new ObjectByteReaderWithPosition(inSourceStream);
    
    // use minimal leve ePDFVersion10 to use the modified file level (cause i don't care
    status = mPDFWriter.ModifyPDFForStream(mReadStreamProxy,mWriteStreamProxy,inPDFVersion,inLogConfiguration,inCreationSettings);
    
    return status;
}

v8::Handle<v8::Value> PDFWriterDriver::CreateFormXObjectFromTIFF(const v8::Arguments& args)
{
    HandleScope scope;
    
    if((args.Length() != 1 && args.Length() != 2) || (!args[0]->IsString() && !args[0]->IsObject()) || (args.Length() == 2 && !args[1]->IsObject() && !args[1]->IsNumber()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is the path to the image, and optionally an options object or object ID")));
		return scope.Close(Undefined());
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    TIFFUsageParameters tiffUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters;
    ObjectIDType objectID = 0;
    
    if(args.Length() == 2)
    {
        if(args[1]->IsObject())
        {
            Local<Object> anObject = args[1]->ToObject();
            
            // page index parameters
            if(anObject->Has(String::New("pageIndex")) && anObject->Get(String::New("pageIndex"))->IsNumber())
                tiffUsageParameters.PageIndex = (unsigned int)anObject->Get(String::New("pageIndex"))->ToNumber()->Value();
            
            if(anObject->Has(String::New("bwTreatment")) && anObject->Get(String::New("bwTreatment"))->IsObject())
            {
                // special black and white treatment
                Local<Object> bwObject = anObject->Get(String::New("bwTreatment"))->ToObject();
                if(bwObject->Has(String::New("asImageMask")) && bwObject->Get(String::New("asImageMask"))->IsBoolean())
                    tiffUsageParameters.BWTreatment.AsImageMask = bwObject->Get(String::New("asImageMask"))->ToBoolean()->Value();
                if(bwObject->Has(String::New("oneColor")) && bwObject->Get(String::New("oneColor"))->IsArray())
                    tiffUsageParameters.BWTreatment.OneColor = colorFromArray(bwObject->Get(String::New("oneColor")));
            }

            if(anObject->Has(String::New("grayscaleTreatment")) && anObject->Get(String::New("grayscaleTreatment"))->IsObject())
            {
                // special black and white treatment
                Local<Object> colormapObject = anObject->Get(String::New("grayscaleTreatment"))->ToObject();
                if(colormapObject->Has(String::New("asColorMap")) && colormapObject->Get(String::New("asColorMap"))->IsBoolean())
                    tiffUsageParameters.GrayscaleTreatment.AsColorMap = colormapObject->Get(String::New("asColorMap"))->ToBoolean()->Value();
                if(colormapObject->Has(String::New("oneColor")) && colormapObject->Get(String::New("oneColor"))->IsArray())
                    tiffUsageParameters.GrayscaleTreatment.OneColor = colorFromArray(colormapObject->Get(String::New("oneColor")));
                if(colormapObject->Has(String::New("zeroColor")) && colormapObject->Get(String::New("zeroColor"))->IsArray())
                    tiffUsageParameters.GrayscaleTreatment.ZeroColor = colorFromArray(colormapObject->Get(String::New("zeroColor")));
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
		ThrowException(Exception::Error(String::New("unable to create form xobject. verify that the target is an existing tiff file")));
		return scope.Close(Undefined());
    }
    
    Handle<Value> newInstance = FormXObjectDriver::NewInstance(args);
    ObjectWrap::Unwrap<FormXObjectDriver>(newInstance->ToObject())->FormXObject = formXObject;
    return scope.Close(newInstance);
}

CMYKRGBColor PDFWriterDriver::colorFromArray(v8::Handle<v8::Value> inArray)
{
    if(inArray->ToObject()->Get(v8::String::New("length"))->ToObject()->Uint32Value() == 4)
    {
        // cmyk color
        return CMYKRGBColor((unsigned char)inArray->ToObject()->Get(0)->ToNumber()->Value(),
                            (unsigned char)inArray->ToObject()->Get(1)->ToNumber()->Value(),
                            (unsigned char)inArray->ToObject()->Get(2)->ToNumber()->Value(),
                            (unsigned char)inArray->ToObject()->Get(3)->ToNumber()->Value());
        
    }
    else if(inArray->ToObject()->Get(v8::String::New("length"))->ToObject()->Uint32Value() == 3)
    {
        // rgb color
        return CMYKRGBColor((unsigned char)inArray->ToObject()->Get(0)->ToNumber()->Value(),
                            (unsigned char)inArray->ToObject()->Get(1)->ToNumber()->Value(),
                            (unsigned char)inArray->ToObject()->Get(2)->ToNumber()->Value());
    }
    else
    {
        ThrowException(Exception::Error(String::New("wrong input for color values. should be array of either 3 or 4 colors")));
        return CMYKRGBColor::CMYKBlack;
    }
}

v8::Handle<v8::Value> PDFWriterDriver::CreateImageXObjectFromJPG(const v8::Arguments& args)
{
    HandleScope scope;
    
    if((args.Length() != 1 && args.Length() != 2) || (!args[0]->IsString() && !args[0]->IsObject()) || (args.Length() == 2 && !args[1]->IsNumber()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is the path to the image. pass another optional argument of a forward reference object ID")));
		return scope.Close(Undefined());
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
		ThrowException(Exception::Error(String::New("unable to create image xobject. verify that the target is an existing jpg file")));
		return scope.Close(Undefined());
    }
    
    Handle<Value> newInstance = ImageXObjectDriver::NewInstance(args);
    ObjectWrap::Unwrap<ImageXObjectDriver>(newInstance->ToObject())->ImageXObject = imageXObject;
    return scope.Close(newInstance);
}

v8::Handle<v8::Value> PDFWriterDriver::GetObjectsContext(const v8::Arguments& args)
{
    HandleScope scope;

    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());

    Handle<Value> newInstance = ObjectsContextDriver::NewInstance(args);
    ObjectsContextDriver* objectsContextDriver = ObjectWrap::Unwrap<ObjectsContextDriver>(newInstance->ToObject());
    objectsContextDriver->ObjectsContextInstance = &(pdfWriter->mPDFWriter.GetObjectsContext());
 
    return scope.Close(newInstance);
}

v8::Handle<v8::Value> PDFWriterDriver::GetDocumentContext(const v8::Arguments& args)
{
    HandleScope scope;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    Handle<Value> newInstance = DocumentContextDriver::NewInstance();
    DocumentContextDriver* documentContextDriver = ObjectWrap::Unwrap<DocumentContextDriver>(newInstance->ToObject());
    documentContextDriver->DocumentContextInstance = &(pdfWriter->mPDFWriter.GetDocumentContext());
    
    return scope.Close(newInstance);
}


v8::Handle<v8::Value> PDFWriterDriver::AppendPDFPagesFromPDF(const v8::Arguments& args)
{
    HandleScope scope;
    
    if((args.Length() != 1  &&
       args.Length() != 2) ||
       (!args[0]->IsString() && !args[0]->IsObject())||
       (args.Length() == 2 && !args[1]->IsObject()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass a path for file to append pages from or a stream object, and optionally a configuration object")));
		return scope.Close(Undefined());
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    PDFPageRange pageRange;
    
    if(args.Length() == 2)
        pageRange = ObjectToPageRange(args[1]->ToObject());
    
    EStatusCodeAndObjectIDTypeList result;
    
    if(args[0]->IsObject())
    {
        ObjectByteReaderWithPosition proxy(args[0]->ToObject());
        result = pdfWriter->mPDFWriter.AppendPDFPagesFromPDF(
                                                             &proxy,
                                                             pageRange);
    }
    else
    {
        result = pdfWriter->mPDFWriter.AppendPDFPagesFromPDF(
                                                        *String::Utf8Value(args[0]->ToString()),
                                                        pageRange);
    }
    
    if(result.first != eSuccess)
    {
		ThrowException(Exception::Error(String::New("unable to append page, make sure it's fine")));
		return scope.Close(Undefined());
    }
    
    Local<Array> resultPageIDs = Array::New((unsigned int)result.second.size());
    unsigned int index = 0;
    
    ObjectIDTypeList::iterator it = result.second.begin();
    for(; it != result.second.end();++it)
        resultPageIDs->Set(Number::New(index++),Number::New(*it));
    
    return scope.Close(resultPageIDs);
}

PDFPageRange PDFWriterDriver::ObjectToPageRange(Handle<Object> inObject)
{
    PDFPageRange pageRange;
        
    if(inObject->Has(String::New("type")) && inObject->Get(String::New("type"))->IsNumber())
    {
        pageRange.mType = (PDFPageRange::ERangeType)(inObject->Get(String::New("type"))->ToNumber()->Uint32Value());
    }

    if(inObject->Has(String::New("specificRanges")) && inObject->Get(String::New("specificRanges"))->IsArray())
    {
        Local<Object> anArray = inObject->Get(String::New("specificRanges"))->ToObject();
        unsigned int length = anArray->Get(v8::String::New("length"))->ToNumber()->Uint32Value();
        for(unsigned int i=0; i < length; ++i)
        {
            if(!anArray->Get(i)->IsArray() ||
               anArray->Get(i)->ToObject()->Get(v8::String::New("length"))->ToNumber()->Uint32Value() != 2)
            {
                ThrowException(Exception::TypeError(String::New("wrong argument for specificRanges. it should be an array of arrays. each subarray should be of the length of 2, signifying begining page and ending page numbers")));
                break;
            }
            Local<Object> item = anArray->Get(i)->ToObject();
            if(!item->Get(0)->IsNumber() || !item->Get(1)->IsNumber())
            {
                ThrowException(Exception::TypeError(String::New("wrong argument for specificRanges. it should be an array of arrays. each subarray should be of the length of 2, signifying begining page and ending page numbers")));
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
            callback->Call(Context::GetCurrent()->Global(), argc, NULL);
        }
		return PDFHummus::eSuccess;
	}
    
    bool IsValid(){return !callback.IsEmpty();}

    Local<Function> callback;
};

v8::Handle<v8::Value> PDFWriterDriver::MergePDFPagesToPage(const v8::Arguments& args)
{
    HandleScope scope;
    
    /*
        parameters are:
            target page
            file path to pdf to merge pages to
            optional 1: options object
            optional 2: callback function to call after each page merge
     */
    
    if(args.Length() < 2 ||
       !PDFPageDriver::HasInstance(args[0]) ||
       !args[1]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, pass a page object, a path to pages source file, and two optional: configuration object and callback function that will be called between pages merging")));
		return scope.Close(Undefined());
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    PDFPageDriver* page = ObjectWrap::Unwrap<PDFPageDriver>(args[0]->ToObject());
    
    PDFPageRange pageRange;
    
    // get page range
    if(args.Length() > 2 && args[2]->IsObject())
        pageRange = ObjectToPageRange(args[2]->ToObject());
    else if(args.Length() > 3 && args[3]->IsObject())
        pageRange = ObjectToPageRange(args[3]->ToObject());
    
    // now see if there's a need for activating the callback. will do that using the document extensibility option of the lib
    MergeInterpageCallbackCaller caller;
    if((args.Length() > 2 && args[2]->IsFunction()) ||
       (args.Length() > 3 && args[3]->IsFunction()))
        caller.callback = Local<Function>::Cast(args[2]->IsFunction() ? args[2] : args[3]);
    if(caller.IsValid())
        pdfWriter->mPDFWriter.GetDocumentContext().AddDocumentContextExtender(&caller);
    
    EStatusCode status = pdfWriter->mPDFWriter.MergePDFPagesToPage(page->GetPage(),
                                                                    *String::Utf8Value(args[1]->ToString()),
                                                                    pageRange);
    if(caller.IsValid())
        pdfWriter->mPDFWriter.GetDocumentContext().RemoveDocumentContextExtender(&caller);

    if(status != eSuccess)
    {
		ThrowException(Exception::Error(String::New("unable to append to page, make sure source file exists")));
		return scope.Close(Undefined());
    }
    return scope.Close(args.This());
}

Handle<Value> PDFWriterDriver::CreatePDFCopyingContext(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 || (!args[0]->IsString() && !args[0]->IsObject()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument. A path to a PDF file to create copying context for or a stream object")));
		return scope.Close(Undefined());
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());

    PDFDocumentCopyingContext* copyingContext;
    
    if(args[0]->IsObject())
    {
        ObjectByteReaderWithPosition proxy(args[0]->ToObject());
        copyingContext = pdfWriter->mPDFWriter.CreatePDFCopyingContext(&proxy);
    }
    else
        copyingContext = pdfWriter->mPDFWriter.CreatePDFCopyingContext(*String::Utf8Value(args[0]->ToString()));
    
    if(!copyingContext)
    {
		ThrowException(Exception::Error(String::New("unable to create copying context. verify that the target is an existing PDF file")));
		return scope.Close(Undefined());
    }
    
    Handle<Value> newInstance = DocumentCopyingContextDriver::NewInstance(args);
    ObjectWrap::Unwrap<DocumentCopyingContextDriver>(newInstance->ToObject())->CopyingContext = copyingContext;
    return scope.Close(newInstance);
}

Handle<Value> PDFWriterDriver::CreateFormXObjectsFromPDF(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() < 1  ||
       args.Length() > 5 ||
       !args[0]->IsString() ||
       (args.Length() >= 2 && (!args[1]->IsNumber() && !args[1]->IsArray())) ||
       (args.Length() >= 3 && !args[2]->IsObject()) ||
       (args.Length() >= 4 && !args[3]->IsArray()) ||
       (args.Length() == 5 && !args[4]->IsArray())
       )
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass a path to the file, and optionals - a box enumerator or actual 4 numbers box, a range object, a matrix for the form, and array of object ids to copy in addition")));
		return scope.Close(Undefined());
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    PDFPageRange pageRange;
    
    if(args.Length() >= 3)
        pageRange = ObjectToPageRange(args[2]->ToObject());
    
    EStatusCodeAndObjectIDTypeList result;
    double matrixBuffer[6];
    double* transformationMatrix = NULL;
    
    if(args.Length() >= 4)
    {
        Handle<Object> matrixArray = args[3]->ToObject();
        if(matrixArray->Get(v8::String::New("length"))->ToObject()->Uint32Value() != 6)
        {
            ThrowException(Exception::TypeError(String::New("matrix array should be 6 numbers long")));
            return scope.Close(Undefined());
        }
        
        for(int i=0;i<6;++i)
            matrixBuffer[i] = matrixArray->Get(i)->ToNumber()->Value();
        transformationMatrix = matrixBuffer;
    }
    
    ObjectIDTypeList extraObjectsList;
    if(args.Length() == 5)
    {
        Handle<Object> objectsIDsArray = args[4]->ToObject();
        unsigned int arrayLength = objectsIDsArray->Get(v8::String::New("length"))->ToObject()->Uint32Value();
        for(unsigned int i=0;i<arrayLength;++i)
            extraObjectsList.push_back((ObjectIDType)(objectsIDsArray->Get(i)->ToNumber()->Uint32Value()));
            
    }
    
    
    if(args[1]->IsArray())
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
        
        result = pdfWriter->mPDFWriter.CreateFormXObjectsFromPDF(
                                                                 *String::Utf8Value(args[0]->ToString()),
                                                                 pageRange,
                                                                 box,
                                                                 transformationMatrix,
                                                                 extraObjectsList);
    }
    else
    {
        result = pdfWriter->mPDFWriter.CreateFormXObjectsFromPDF(
                                                                *String::Utf8Value(args[0]->ToString()),
                                                                pageRange,
                                                                (EPDFPageBox)args[1]->ToNumber()->Uint32Value(),
                                                                 transformationMatrix,
                                                                 extraObjectsList);
    }
    
    if(result.first != eSuccess)
    {
		ThrowException(Exception::Error(String::New("unable to create forms from file. make sure the file exists, and that the input page range is valid (well, if you provided one..m'k?")));
		return scope.Close(Undefined());
    }
    
    Local<Array> resultFormIDs = Array::New((unsigned int)result.second.size());
    unsigned int index = 0;
    
    ObjectIDTypeList::iterator it = result.second.begin();
    for(; it != result.second.end();++it)
        resultFormIDs->Set(Number::New(index++),Number::New(*it));
    
    return scope.Close(resultFormIDs);
}
 
Handle<Value> PDFWriterDriver::CreatePDFCopyingContextForModifiedFile(const Arguments& args)
{
    HandleScope scope;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    PDFDocumentCopyingContext* copyingContext = pdfWriter->mPDFWriter.CreatePDFCopyingContextForModifiedFile();
    if(!copyingContext)
    {
		ThrowException(Exception::Error(String::New("unable to create copying context for modified file...possibly a file is not being modified by this writer...")));
		return scope.Close(Undefined());
    }
    
    Handle<Value> newInstance = DocumentCopyingContextDriver::NewInstance(args);
    ObjectWrap::Unwrap<DocumentCopyingContextDriver>(newInstance->ToObject())->CopyingContext = copyingContext;
    return scope.Close(newInstance);
}

Handle<Value> PDFWriterDriver::CreatePDFTextString(const Arguments& args)
{
    HandleScope scope;
    return scope.Close(PDFTextStringDriver::NewInstance(args));
    
}

Handle<Value> PDFWriterDriver::CreatePDFDate(const Arguments& args)
{
    HandleScope scope;
    return scope.Close(PDFDateDriver::NewInstance(args));
    
}

HummusImageInformation& PDFWriterDriver::GetImageInformationStructFor(const std::string& inImageFile,unsigned long inImageIndex)
{
    StringAndULongPairToHummusImageInformationMap::iterator it = mImagesInformation.find(StringAndULongPair(inImageFile,inImageIndex));
    
    if(it == mImagesInformation.end())
        it = mImagesInformation.insert(
                        StringAndULongPairToHummusImageInformationMap::value_type(
                                StringAndULongPair(inImageFile,inImageIndex),HummusImageInformation())).first;
    
    return it->second;
}

DoubleAndDoublePair PDFWriterDriver::GetImageDimensions(const std::string& inImageFile,unsigned long inImageIndex)
{
    HummusImageInformation& imageInformation = GetImageInformationStructFor(inImageFile,inImageIndex);
    
    if(-1 == imageInformation.imageWidth)
    {
        HummusImageInformation::EHummusImageType imageType = GetImageType(inImageFile,inImageIndex);
        
        switch(imageType)
        {
            case HummusImageInformation::ePDF:
            {
                // get the dimensions via the PDF parser. will use the media rectangle to draw image
                PDFParser pdfParser;
                
                InputFile file;
                if(file.OpenFile(inImageFile) != eSuccess)
                    break;
                if(pdfParser.StartPDFParsing(file.GetInputStream()) != eSuccess)
                    break;
                
                PDFPageInput helper(&pdfParser,pdfParser.ParsePage(inImageIndex));
                
                imageInformation.imageWidth = helper.GetMediaBox().UpperRightX - helper.GetMediaBox().LowerLeftX;
                imageInformation.imageHeight = helper.GetMediaBox().UpperRightY - helper.GetMediaBox().LowerLeftY;
                
                break;
            }
            case HummusImageInformation::eJPG:
            {
                BoolAndJPEGImageInformation jpgImageInformation = mPDFWriter.GetDocumentContext().GetJPEGImageHandler().RetrieveImageInformation(inImageFile);
                if(!jpgImageInformation.first)
                    break;
                
                DoubleAndDoublePair dimensions = mPDFWriter.GetDocumentContext().GetJPEGImageHandler().GetImageDimensions(jpgImageInformation.second);
                
                imageInformation.imageWidth = dimensions.first;
                imageInformation.imageHeight = dimensions.second;
                break;
            }
            case HummusImageInformation::eTIFF:
            {
                TIFFImageHandler hummusTiffHandler;
                
                InputFile file;
                if(file.OpenFile(inImageFile) != eSuccess)
                    break;
                
                DoubleAndDoublePair dimensions = hummusTiffHandler.ReadImageDimensions(file.GetInputStream(),inImageIndex);

                imageInformation.imageWidth = dimensions.first;
                imageInformation.imageHeight = dimensions.second;
                break;
            }
            default:
            {
                // just avoding uninteresting compiler warnings. meaning...if you can't get the image type or unsupported, do nothing
            }
        }
    }
    
    return DoubleAndDoublePair(imageInformation.imageWidth,imageInformation.imageHeight);
}

static const Byte scPDFMagic[] = {0x25,0x50,0x44,0x46};
static const Byte scMagicJPG[] = {0xFF,0xD8};
static const Byte scMagicTIFFBigEndianTiff[] = {0x4D,0x4D,0x00,0x2A};
static const Byte scMagicTIFFBigEndianBigTiff[] = {0x4D,0x4D,0x00,0x2B};
static const Byte scMagicTIFFLittleEndianTiff[] = {0x49,0x49,0x2A,0x00};
static const Byte scMagicTIFFLittleEndianBigTiff[] = {0x49,0x49,0x2B,0x00};


HummusImageInformation::EHummusImageType PDFWriterDriver::GetImageType(const std::string& inImageFile,unsigned long inImageIndex)
{
    HummusImageInformation& imageInformation = GetImageInformationStructFor(inImageFile,inImageIndex);
    
    if(imageInformation.imageType == HummusImageInformation::eUndefined)
    {
        // The types of images that are discovered here are those familiar to Hummus - JPG, TIFF and PDF
        // PDF is recognized by starting with "%PDF"
        // JPG will start with "0xff,0xd8"
        // TIFF will start with "0x49,0x49" (little endian) or "0x4D,0x4D" (big endian)
        // then either 42 or 43 (tiff or big tiff respectively) written in 2 bytes, as either big or little endian
        
        // so just read the first 4 bytes and it should be enough to recognize a known format
        
        Byte magic[4];
        unsigned long readLength = 4;
        InputFile inputFile;
        if(inputFile.OpenFile(inImageFile) == eSuccess)
        {
            inputFile.GetInputStream()->Read(magic,readLength);
        
            if(readLength >= 4 && memcmp(scPDFMagic,magic,4) == 0)
                imageInformation.imageType =  HummusImageInformation::ePDF;
            else if(readLength >= 2 && memcmp(scMagicJPG,magic,2) == 0)
                imageInformation.imageType = HummusImageInformation::eJPG;
            else if(readLength >= 4 && memcmp(scMagicTIFFBigEndianTiff,magic,4) == 0)
                imageInformation.imageType = HummusImageInformation::eTIFF;
            else if(readLength >= 4 && memcmp(scMagicTIFFBigEndianBigTiff,magic,4) == 0)
                imageInformation.imageType = HummusImageInformation::eTIFF;
            else if(readLength >= 4 && memcmp(scMagicTIFFLittleEndianTiff,magic,4) == 0)
                imageInformation.imageType = HummusImageInformation::eTIFF;
            else if(readLength >= 4 && memcmp(scMagicTIFFLittleEndianBigTiff,magic,4) == 0)
                imageInformation.imageType = HummusImageInformation::eTIFF;
            else
                imageInformation.imageType = HummusImageInformation::eUndefined;
        }
        
    }

    
    return imageInformation.imageType;
}

ObjectIDTypeAndBool PDFWriterDriver::RegisterImageForDrawing(const std::string& inImageFile,unsigned long inImageIndex)
{
    HummusImageInformation& imageInformation = GetImageInformationStructFor(inImageFile,inImageIndex);
    bool firstTime;
    
    if(imageInformation.writtenObjectID == 0)
    {
        imageInformation.writtenObjectID = mPDFWriter.GetObjectsContext().GetInDirectObjectsRegistry().AllocateNewObjectID();
        firstTime = true;
    }
    else
        firstTime = false;
    
    return ObjectIDTypeAndBool(imageInformation.writtenObjectID,firstTime);
}

PDFWriter* PDFWriterDriver::GetWriter()
{
    return &mPDFWriter;
}

EStatusCode PDFWriterDriver::WriteFormForImage(const std::string& inImagePath,unsigned long inImageIndex,ObjectIDType inObjectID)
{
    EStatusCode status;
    HummusImageInformation::EHummusImageType imageType = GetImageType(inImagePath,inImageIndex);
        
    switch(imageType)
    {
        case HummusImageInformation::ePDF:
        {
            PDFDocumentCopyingContext* copyingContext = NULL;
            PDFFormXObject* formXObject = NULL;
            do {
                // hmm...pdf merging doesn't have an innate method to force an object id. so i'll create a form, and merge into it
                copyingContext = mPDFWriter.CreatePDFCopyingContext(inImagePath);
                if(!copyingContext)
                {
                    status = eFailure;
                    break;
                }
                
                // PDFPageInput(PDFParser* inParser,PDFObject* inPageObject);
                PDFPageInput pageInput(copyingContext->GetSourceDocumentParser(),
                                       copyingContext->GetSourceDocumentParser()->ParsePage(inImageIndex));
                
                formXObject = mPDFWriter.StartFormXObject(pageInput.GetMediaBox(),inObjectID);
                if(!formXObject)
                {
                    status = eFailure;
                    break;
                }
                
                status = copyingContext->MergePDFPageToFormXObject(formXObject,inImageIndex);
                if(status != eSuccess)
                    break;
                
                status = mPDFWriter.EndFormXObject(formXObject);
            }while(false);

            delete formXObject;
            delete copyingContext;
            break;
        }
        case HummusImageInformation::eJPG:
        {
            PDFFormXObject* form = mPDFWriter.CreateFormXObjectFromJPGFile(inImagePath,inObjectID);
            status = (form ? eSuccess:eFailure);
            delete form;
            break;
        }
        case HummusImageInformation::eTIFF:
        {
            TIFFUsageParameters params;
            params.PageIndex = (unsigned int)inImageIndex;
            
            PDFFormXObject* form = mPDFWriter.CreateFormXObjectFromTIFFFile(inImagePath,inObjectID,params);
            status = (form ? eSuccess:eFailure);
            delete form;
            break;
        }
        default:
        {
            status = eFailure;
        }
    }
    return status;
}


Handle<Value> PDFWriterDriver::SGetImageDimensions(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() < 1 || args.Length() > 2 ||
       !args[0]->IsString() ||
       (args.Length()==2 && !args[1]->IsNumber()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 or 2 argument. a path to an image, and optional image index (for multi-image files)")));
		return scope.Close(Undefined());
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());

    DoubleAndDoublePair dimensions = pdfWriter->GetImageDimensions(
                                  *String::Utf8Value(args[0]->ToString()),
                                  args.Length() == 2 ? args[1]->ToNumber()->Uint32Value() : 0);
    
    Handle<Object> newObject = Object::New();
    
    newObject->Set(String::New("width"),Number::New(dimensions.first));
    newObject->Set(String::New("height"),Number::New(dimensions.second));
    return scope.Close(newObject);
};

Handle<Value> PDFWriterDriver::GetModifiedFileParser(const Arguments& args)
{
    HandleScope scope;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    PDFParser* parser = &(pdfWriter->mPDFWriter.GetModifiedFileParser());
    if(!parser->GetTrailer()) // checking for the trailer should be a good indication to whether this parser is relevant
    {
		ThrowException(Exception::Error(String::New("unable to create modified parser...possibly a file is not being modified by this writer...")));
		return scope.Close(Undefined());
    }
    
    Handle<Value> newInstance = PDFReaderDriver::NewInstance(args);
    ObjectWrap::Unwrap<PDFReaderDriver>(newInstance->ToObject())->SetFromOwnedParser(parser);
    return scope.Close(newInstance);
}

Handle<Value> PDFWriterDriver::GetModifiedInputFile(const Arguments& args)
{
    HandleScope scope;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    InputFile* inputFile = &(pdfWriter->mPDFWriter.GetModifiedInputFile());
    if(!inputFile->GetInputStream()) 
    {
		ThrowException(Exception::Error(String::New("unable to create modified input file...possibly a file is not being modified by this writer...")));
		return scope.Close(Undefined());
    }
    
    Handle<Value> newInstance = InputFileDriver::NewInstance(args);
    ObjectWrap::Unwrap<InputFileDriver>(newInstance->ToObject())->SetFromOwnedFile(inputFile);
    return scope.Close(newInstance);
}

Handle<Value> PDFWriterDriver::GetOutputFile(const Arguments& args)
{
    HandleScope scope;
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    OutputFile* outputFile = &(pdfWriter->mPDFWriter.GetOutputFile());
    if(!outputFile->GetOutputStream())
    {
		ThrowException(Exception::Error(String::New("unable to get output file. probably pdf writing hasn't started, or the output is not to a file")));
		return scope.Close(Undefined());
    }
    
    Handle<Value> newInstance = OutputFileDriver::NewInstance(args);
    ObjectWrap::Unwrap<OutputFileDriver>(newInstance->ToObject())->SetFromOwnedFile(outputFile);
    return scope.Close(newInstance);
}

Handle<Value> PDFWriterDriver::RegisterAnnotationReferenceForNextPageWrite(const Arguments& args)
{
    HandleScope scope;

    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
        ThrowException(Exception::TypeError(String::New("wrong arguments,  pass an object ID for an annotation to register")));
        return scope.Close(Undefined());
    }

    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    pdfWriter->mPDFWriter.GetDocumentContext().RegisterAnnotationReferenceForNextPageWrite(args[0]->ToNumber()->Uint32Value());

    return scope.Close(args.This());
}