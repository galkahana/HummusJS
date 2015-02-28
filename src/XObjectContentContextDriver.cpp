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
#include "XObjectContentContextDriver.h"
#include "XObjectContentContext.h"
#include "IFormEndWritingTask.h"
#include "PDFWriterDriver.h"
#include "DocumentContext.h"

using namespace v8;

XObjectContentContextDriver::XObjectContentContextDriver()
{
    // initially null, set by external pdfwriter
    ContentContext = NULL;
    FormOfContext = NULL;
    mPDFWriterDriver = NULL;
}

Persistent<Function> XObjectContentContextDriver::constructor;

void XObjectContentContextDriver::Init()
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("XObjectContentContext"));
	t->InstanceTemplate()->SetInternalFieldCount(1);
	AbstractContentContextDriver::Init(t);
	SET_CONSTRUCTOR(constructor, t);
}

METHOD_RETURN_TYPE XObjectContentContextDriver::NewInstance(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    Local<Object> instance = NEW_INSTANCE(constructor);
    
    SET_FUNCTION_RETURN_VALUE(instance);
}

v8::Handle<v8::Value> XObjectContentContextDriver::GetNewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

METHOD_RETURN_TYPE XObjectContentContextDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    XObjectContentContextDriver* form = new XObjectContentContextDriver();
    form->Wrap(args.This());
    
	SET_FUNCTION_RETURN_VALUE(args.This());
}

AbstractContentContext* XObjectContentContextDriver::GetContext()
{
    return ContentContext;
}

void XObjectContentContextDriver::SetPDFWriter(PDFWriterDriver* inPDFWriter)
{
    mPDFWriterDriver = inPDFWriter;
}

PDFWriterDriver* XObjectContentContextDriver::GetPDFWriter()
{
    return mPDFWriterDriver;
}

 class XObjectFormImageWritingTask : public IFormEndWritingTask
 {
 public:
 XObjectFormImageWritingTask(PDFWriterDriver* inDriver,const std::string& inImagePath,unsigned long inImageIndex,ObjectIDType inObjectID)
 {mDriver = inDriver;mImagePath = inImagePath;mImageIndex = inImageIndex;mObjectID = inObjectID;}
 
 virtual ~XObjectFormImageWritingTask(){}
 
     virtual PDFHummus::EStatusCode Write(PDFFormXObject* inFormXObject,
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


void XObjectContentContextDriver::ScheduleImageWrite(const std::string& inImagePath,unsigned long inImageIndex,ObjectIDType inObjectID)
{
    mPDFWriterDriver->GetWriter()->GetDocumentContext().RegisterFormEndWritingTask(
                                                            FormOfContext,
                                                            new XObjectFormImageWritingTask(mPDFWriterDriver,inImagePath,inImageIndex,inObjectID));

}

