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
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createFormXObjectFromJPGFile"),FunctionTemplate::New(CreateformXObjectFromJPGFile)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("getFontForFile"),FunctionTemplate::New(GetFontForFile)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("attachURLLinktoCurrentPage"),FunctionTemplate::New(AttachURLLinktoCurrentPage)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("shutdown"),FunctionTemplate::New(Shutdown)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createFormXObjectFromTIFFFile"),FunctionTemplate::New(CreateFormXObjectFromTIFFFile)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createImageXObjectFromJPGFile"),FunctionTemplate::New(CreateImageXObjectFromJPGFile)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("getObjectsContext"),FunctionTemplate::New(GetObjectsContext)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("appendPDFPagesFromPDF"),FunctionTemplate::New(AppendPDFPagesFromPDF)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("mergePDFPagesToPage"),FunctionTemplate::New(MergePDFPagesToPage)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createPDFCopyingContext"),FunctionTemplate::New(CreatePDFCopyingContext)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createFormXObjectsFromPDF"),FunctionTemplate::New(CreateFormXObjectsFromPDF)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createPDFCopyingContextForModifiedFile"),FunctionTemplate::New(CreatePDFCopyingContextForModifiedFile)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createPDFTextString"),FunctionTemplate::New(CreatePDFTextString)->GetFunction());
    pdfWriterFT->PrototypeTemplate()->Set(String::NewSymbol("createPDFDate"),FunctionTemplate::New(CreatePDFDate)->GetFunction());
    

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
    
    if((args.Length() != 1  && args.Length() != 2 ) || !args[0]->IsString() || (args.Length() == 2 && !args[1]->IsNumber()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is the path to the image. Optionally pass an object ID for a forward reference image")));
		return scope.Close(Undefined());
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    PDFFormXObject* formXObject =
        args.Length() == 2 ?
        pdfWriter->mPDFWriter.CreateFormXObjectFromJPGFile(*String::Utf8Value(args[0]->ToString()),(ObjectIDType)args[1]->ToNumber()->Int32Value()):
        pdfWriter->mPDFWriter.CreateFormXObjectFromJPGFile(*String::Utf8Value(args[0]->ToString()));
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
    
    if((args.Length() < 1  && args.Length() > 3)||
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
                                EPDFVersion inPDFVersion)
{
    return mPDFWriter.StartPDF(inOutputFilePath,inPDFVersion);
}

PDFHummus::EStatusCode PDFWriterDriver::ContinuePDF(const std::string& inOutputFilePath,
                                                    const std::string& inStateFilePath)
{
    return mPDFWriter.ContinuePDF(inOutputFilePath,inStateFilePath);
}

PDFHummus::EStatusCode PDFWriterDriver::ModifyPDF(const std::string& inSourceFile,
                                                  const std::string& inOptionalOtherOutputFile)
{
    // two phase, cause i don't want to bother the users with the level BS.
    // first, parse the source file, get the level. then modify with this level
    
    PDFHummus::EStatusCode status;
    
    do
    {
        EPDFVersion level = ePDFVersion13;
        {
            // read source file level
            PDFParser reader;
            InputFile inputFile;
            
            status = inputFile.OpenFile(inSourceFile);
            if(status != PDFHummus::eSuccess)
                break;
            status = reader.StartPDFParsing(inputFile.GetInputStream());
            if(status != PDFHummus::eSuccess)
                break;
            level = (EPDFVersion)((unsigned long)reader.GetPDFLevel() * 10);
        }
        
        // now modify
        status = mPDFWriter.ModifyPDF(inSourceFile,level,inOptionalOtherOutputFile);
        
    } while (false);
    
    return status;
}


v8::Handle<v8::Value> PDFWriterDriver::CreateFormXObjectFromTIFFFile(const v8::Arguments& args)
{
    HandleScope scope;
    
    if((args.Length() != 1 && args.Length() != 2) || !args[0]->IsString() || (args.Length() == 2 && !args[1]->IsObject() && !args[1]->IsNumber()))
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
    
    PDFFormXObject* formXObject =
        objectID == 0 ?
            pdfWriter->mPDFWriter.CreateFormXObjectFromTIFFFile(*String::Utf8Value(args[0]->ToString()),tiffUsageParameters):
            pdfWriter->mPDFWriter.CreateFormXObjectFromTIFFFile(*String::Utf8Value(args[0]->ToString()),objectID);
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

v8::Handle<v8::Value> PDFWriterDriver::CreateImageXObjectFromJPGFile(const v8::Arguments& args)
{
    HandleScope scope;
    
    if((args.Length() != 1 && args.Length() != 2) || !args[0]->IsString() || (args.Length() == 2 && !args[1]->IsNumber()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is the path to the image. pass another optional argument of a forward reference object ID")));
		return scope.Close(Undefined());
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
       
   
    PDFImageXObject* imageXObject =
       args.Length() == 2 ?
       pdfWriter->mPDFWriter.CreateImageXObjectFromJPGFile(*String::Utf8Value(args[0]->ToString()),(ObjectIDType)args[1]->ToNumber()->Int32Value()) :
       pdfWriter->mPDFWriter.CreateImageXObjectFromJPGFile(*String::Utf8Value(args[0]->ToString()));
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

v8::Handle<v8::Value> PDFWriterDriver::AppendPDFPagesFromPDF(const v8::Arguments& args)
{
    HandleScope scope;
    
    if((args.Length() != 1  &&
       args.Length() != 2) ||
       !args[0]->IsString() ||
       (args.Length() == 2 && !args[1]->IsObject()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass a path for fille to append pages from, and optionally a configuration object")));
		return scope.Close(Undefined());
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    PDFPageRange pageRange;
    
    if(args.Length() == 2)
        pageRange = ObjectToPageRange(args[1]->ToObject());
    
    EStatusCodeAndObjectIDTypeList result = pdfWriter->mPDFWriter.AppendPDFPagesFromPDF(
                                                                    *String::Utf8Value(args[0]->ToString()),
                                                                    pageRange);
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
               !anArray->Get(i)->ToObject()->Get(v8::String::New("length"))->ToNumber()->Uint32Value() == 2)
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
    
    if(args.Length() != 1 || !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument. A path to a PDF file to create copying context for")));
		return scope.Close(Undefined());
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());

    PDFDocumentCopyingContext* copyingContext = pdfWriter->mPDFWriter.CreatePDFCopyingContext(*String::Utf8Value(args[0]->ToString()));
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
    
    if((args.Length() < 1  &&
        args.Length() > 3) ||
       !args[0]->IsString() ||
       !args[1]->IsNumber() ||
       (args.Length() == 3 && !args[2]->IsObject()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass a path for fille to append pages from as xobject, an enumerator to determine the forms box size, an optionally a configuration object to choose page ranges")));
		return scope.Close(Undefined());
    }
    
    PDFWriterDriver* pdfWriter = ObjectWrap::Unwrap<PDFWriterDriver>(args.This());
    
    PDFPageRange pageRange;
    
    if(args.Length() == 3)
        pageRange = ObjectToPageRange(args[2]->ToObject());
    
    EStatusCodeAndObjectIDTypeList result = pdfWriter->mPDFWriter.CreateFormXObjectsFromPDF(
                                                                                        *String::Utf8Value(args[0]->ToString()),
                                                                                        pageRange,
                                                                                        (EPDFPageBox)args[1]->ToNumber()->Uint32Value());
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