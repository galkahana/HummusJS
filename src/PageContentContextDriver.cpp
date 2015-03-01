/*
 Source File : PageContentContextDriver.cpp
 
 
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
#include "PageContentContextDriver.h"
#include "PageContentContext.h"
#include "PDFStreamDriver.h"
#include "PDFPageDriver.h"
#include "IPageEndWritingTask.h"
#include "PDFWriterDriver.h"

using namespace v8;

PageContentContextDriver::~PageContentContextDriver()
{
}

PageContentContextDriver::PageContentContextDriver()
{
    // initially null, set by external pdfwriter
    ContentContext = NULL;
    
    
    mPDFWriterDriver = NULL;
}

void PageContentContextDriver::SetPDFWriter(PDFWriterDriver* inDriver)
{
    mPDFWriterDriver = inDriver;
};

Persistent<Function> PageContentContextDriver::constructor;
Persistent<FunctionTemplate> PageContentContextDriver::constructor_template;

void PageContentContextDriver::Init()
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PageContentContext"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "getCurrentPageContentStream", GetCurrentPageContentStream);
	SET_PROTOTYPE_METHOD(t, "getAssociatedPage", GetAssociatedPage);
	AbstractContentContextDriver::Init(t);
	SET_CONSTRUCTOR(constructor, t);
	SET_CONSTRUCTOR_TEMPLATE(constructor_template, t);

}

METHOD_RETURN_TYPE PageContentContextDriver::NewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	SET_FUNCTION_RETURN_VALUE(instance);
}

v8::Handle<v8::Value> PageContentContextDriver::GetNewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

bool PageContentContextDriver::HasInstance(Handle<Value> inObject)
{
	CREATE_ISOLATE_CONTEXT;

	return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);
}


METHOD_RETURN_TYPE PageContentContextDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PageContentContextDriver* pdfPage = new PageContentContextDriver();
    pdfPage->Wrap(args.This());
    
	SET_FUNCTION_RETURN_VALUE(args.This());
}

AbstractContentContext* PageContentContextDriver::GetContext()
{
    return ContentContext;
}

METHOD_RETURN_TYPE PageContentContextDriver::GetCurrentPageContentStream(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    
    PageContentContextDriver* driver = ObjectWrap::Unwrap<PageContentContextDriver>(args.This());
    
    Handle<Value> newInstance = PDFStreamDriver::GetNewInstance(args);
    ObjectWrap::Unwrap<PDFStreamDriver>(newInstance->ToObject())->PDFStreamInstance = driver->ContentContext->GetCurrentPageContentStream();
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PageContentContextDriver::GetAssociatedPage(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PageContentContextDriver* driver = ObjectWrap::Unwrap<PageContentContextDriver>(args.This());
    SET_FUNCTION_RETURN_VALUE(PDFPageDriver::GetNewInstance(driver->ContentContext->GetAssociatedPage()));
}

PDFWriterDriver* PageContentContextDriver::GetPDFWriter()
{
    return mPDFWriterDriver;
}


class PageContentImageWritingTask : public IPageEndWritingTask
{
public:
    PageContentImageWritingTask(PDFWriterDriver* inDriver,const std::string& inImagePath,unsigned long inImageIndex,ObjectIDType inObjectID)
    {mDriver = inDriver;mImagePath = inImagePath;mImageIndex = inImageIndex;mObjectID = inObjectID;}
    
    virtual ~PageContentImageWritingTask(){}
    
    virtual PDFHummus::EStatusCode Write(PDFPage* inPageObject,
                                         ObjectsContext* inObjectsContext,
                                         PDFHummus::DocumentContext* inDocumentContext)
    {
        return mDriver->WriteFormForImage(mImagePath,mImageIndex,mObjectID);
    }
    
private:
    PDFWriterDriver* mDriver;
    std::string mImagePath;
    unsigned long mImageIndex;
    ObjectIDType mObjectID;
};


void PageContentContextDriver::ScheduleImageWrite(const std::string& inImagePath,unsigned long inImageIndex,ObjectIDType inObjectID)
{
    mPDFWriterDriver->GetWriter()->GetDocumentContext().RegisterPageEndWritingTask(
                                                                                   ContentContext->GetAssociatedPage(),
                                                                                   new PageContentImageWritingTask(mPDFWriterDriver,inImagePath,inImageIndex,inObjectID));
    
}
