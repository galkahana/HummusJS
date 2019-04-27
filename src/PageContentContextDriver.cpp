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
#include "ConstructorsHolder.h"

using namespace v8;

PageContentContextDriver::~PageContentContextDriver()
{
}

PageContentContextDriver::PageContentContextDriver()
{
    // initially null, set by external pdfwriter
    ContentContext = NULL;
}



DEF_SUBORDINATE_INIT(PageContentContextDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE_EXTERNAL(New);

	t->SetClassName(NEW_STRING("PageContentContext"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "getCurrentPageContentStream", GetCurrentPageContentStream);
	SET_PROTOTYPE_METHOD(t, "getAssociatedPage", GetAssociatedPage);
	AbstractContentContextDriver::Init(t);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->PageContentContext_constructor, t);   	
	SET_CONSTRUCTOR_TEMPLATE(holder->PageContentContext_constructor_template, t);
}

METHOD_RETURN_TYPE PageContentContextDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
	EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, externalHolder)
    
    PageContentContextDriver* pdfPage = new PageContentContextDriver();
	pdfPage->holder = externalHolder;
    pdfPage->Wrap(args.This());
    
	SET_FUNCTION_RETURN_VALUE(args.This())
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
    
    Local<Value> newInstance = driver->holder->GetNewPDFStream(args);
    ObjectWrap::Unwrap<PDFStreamDriver>(newInstance->TO_OBJECT())->PDFStreamInstance = driver->ContentContext->GetCurrentPageContentStream();
    SET_FUNCTION_RETURN_VALUE(newInstance)
}

METHOD_RETURN_TYPE PageContentContextDriver::GetAssociatedPage(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PageContentContextDriver* driver = ObjectWrap::Unwrap<PageContentContextDriver>(args.This());
    SET_FUNCTION_RETURN_VALUE(driver->holder->GetInstanceFor(driver->ContentContext->GetAssociatedPage()))
}
