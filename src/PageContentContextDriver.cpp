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

using namespace v8;

PageContentContextDriver::~PageContentContextDriver()
{
    // most of the times it won't be an actual delete. only if the user forgot to write the page
    delete ContentContext;
}

PageContentContextDriver::PageContentContextDriver()
{
    // initially null, set by external pdfwriter
    ContentContext = NULL;
}

Persistent<Function> PageContentContextDriver::constructor;
Persistent<FunctionTemplate> PageContentContextDriver::constructor_template;

void PageContentContextDriver::Init()
{
    // prepare the context driver interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("PageContentContext"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);

    AbstractContentContextDriver::Init(constructor_template);
    t->PrototypeTemplate()->Set(String::NewSymbol("getCurrentPageContentStream"),FunctionTemplate::New(GetCurrentPageContentStream)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("getAssociatedPage"),FunctionTemplate::New(GetAssociatedPage)->GetFunction());
    
    constructor = Persistent<Function>::New(constructor_template->GetFunction());
}

Handle<Value> PageContentContextDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    const unsigned argc = 0;
    Local<Object> instance = constructor->NewInstance(argc, NULL);
    
    return scope.Close(instance);
}

bool PageContentContextDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}


Handle<Value> PageContentContextDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    PageContentContextDriver* pdfPage = new PageContentContextDriver();
    pdfPage->Wrap(args.This());
    
    return args.This();
}

AbstractContentContext* PageContentContextDriver::GetContext()
{
    return ContentContext;
}

Handle<Value> PageContentContextDriver::GetCurrentPageContentStream(const Arguments& args)
{
    HandleScope scope;
    
    
    PageContentContextDriver* driver = ObjectWrap::Unwrap<PageContentContextDriver>(args.This());
    
    Handle<Value> newInstance = PDFStreamDriver::NewInstance(args);
    ObjectWrap::Unwrap<PDFStreamDriver>(newInstance->ToObject())->PDFStreamInstance = driver->ContentContext->GetCurrentPageContentStream();
    return scope.Close(newInstance);
}

Handle<Value> PageContentContextDriver::GetAssociatedPage(const Arguments& args)
{
    HandleScope scope;
    
    PageContentContextDriver* driver = ObjectWrap::Unwrap<PageContentContextDriver>(args.This());
    return scope.Close(PDFPageDriver::NewInstance(driver->ContentContext->GetAssociatedPage()));
}

