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

using namespace v8;

XObjectContentContextDriver::XObjectContentContextDriver()
{
    // initially null, set by external pdfwriter
    ContentContext = NULL;
}

Persistent<Function> XObjectContentContextDriver::constructor;

void XObjectContentContextDriver::Init()
{
    // prepare the context driver interfrace template
    Local<FunctionTemplate> ft = FunctionTemplate::New(New);
    ft->SetClassName(String::NewSymbol("XObjectContentContext"));
    ft->InstanceTemplate()->SetInternalFieldCount(1);
    
    AbstractContentContextDriver::Init(ft);
    
    constructor = Persistent<Function>::New(ft->GetFunction());
}

Handle<Value> XObjectContentContextDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    const unsigned argc = 0;
    Local<Object> instance = constructor->NewInstance(argc, NULL);
    
    return scope.Close(instance);
}


Handle<Value> XObjectContentContextDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    XObjectContentContextDriver* form = new XObjectContentContextDriver();
    form->Wrap(args.This());
    
    return args.This();
}

AbstractContentContext* XObjectContentContextDriver::GetContext()
{
    return ContentContext;
}



