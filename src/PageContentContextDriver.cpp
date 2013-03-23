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

using namespace v8;

PageContentContextDriver::PageContentContextDriver()
{
    // initially null, set by external pdfwriter
    ContentContext = NULL;
}

Persistent<Function> PageContentContextDriver::constructor;

void PageContentContextDriver::Init()
{
    // prepare the context driver interfrace template
    Local<FunctionTemplate> ft = FunctionTemplate::New(New);
    ft->SetClassName(String::NewSymbol("PageContentContext"));
    ft->InstanceTemplate()->SetInternalFieldCount(1);
    
    ft->PrototypeTemplate()->Set(String::NewSymbol("q"),FunctionTemplate::New(q)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("k"),FunctionTemplate::New(k)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("re"),FunctionTemplate::New(re)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("f"),FunctionTemplate::New(f)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("Q"),FunctionTemplate::New(Q)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("G"),FunctionTemplate::New(G)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("w"),FunctionTemplate::New(w)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("m"),FunctionTemplate::New(m)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("l"),FunctionTemplate::New(l)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("S"),FunctionTemplate::New(S)->GetFunction());
    
    constructor = Persistent<Function>::New(ft->GetFunction());
}

Handle<Value> PageContentContextDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    const unsigned argc = 0;
    Handle<Value> argv[argc];
    Local<Object> instance = constructor->NewInstance(argc, argv);
    
    return scope.Close(instance);
}


Handle<Value> PageContentContextDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    PageContentContextDriver* pdfPage = new PageContentContextDriver();
    pdfPage->Wrap(args.This());
    
    return args.This();
}

Handle<Value> PageContentContextDriver::q(const Arguments& args)
{
    HandleScope scope;
    
    PageContentContextDriver* contentContext = ObjectWrap::Unwrap<PageContentContextDriver>(args.This());
    if(!contentContext->ContentContext)
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
    else
        contentContext->ContentContext->q();
    return scope.Close(Undefined());
}

Handle<Value> PageContentContextDriver::k(const Arguments& args)
{
    HandleScope scope;
    
    PageContentContextDriver* contentContext = ObjectWrap::Unwrap<PageContentContextDriver>(args.This());
    if(!contentContext->ContentContext)
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 4 || !args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber()) {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 4 cmyk components (values should be 0-255)")));
		return scope.Close(Undefined());
	}
    
    contentContext->ContentContext->k(args[0]->ToNumber()->Value(),
                                      args[1]->ToNumber()->Value(),
                                      args[2]->ToNumber()->Value(),
                                      args[3]->ToNumber()->Value());
    
    return scope.Close(Undefined());
}

Handle<Value> PageContentContextDriver::re(const Arguments& args)
{
    HandleScope scope;
    
    PageContentContextDriver* contentContext = ObjectWrap::Unwrap<PageContentContextDriver>(args.This());
    if(!contentContext->ContentContext)
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 4 || !args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber()) {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 4 parameters: 2 bottom left coordinates, and width and height measures")));
		return scope.Close(Undefined());
	}
    
    contentContext->ContentContext->re(args[0]->ToNumber()->Value(),
                                      args[1]->ToNumber()->Value(),
                                      args[2]->ToNumber()->Value(),
                                      args[3]->ToNumber()->Value());
    
    return scope.Close(Undefined());
}

Handle<Value> PageContentContextDriver::f(const Arguments& args)
{
    HandleScope scope;
    
    PageContentContextDriver* contentContext = ObjectWrap::Unwrap<PageContentContextDriver>(args.This());
    if(!contentContext->ContentContext)
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
    else
        contentContext->ContentContext->f();
    return scope.Close(Undefined());
}

Handle<Value> PageContentContextDriver::Q(const Arguments& args)
{
    HandleScope scope;
    
    PageContentContextDriver* contentContext = ObjectWrap::Unwrap<PageContentContextDriver>(args.This());
    if(!contentContext->ContentContext)
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
    else
        contentContext->ContentContext->Q();
    return scope.Close(Undefined());
}

Handle<Value> PageContentContextDriver::G(const Arguments& args)
{
    HandleScope scope;
    
    PageContentContextDriver* contentContext = ObjectWrap::Unwrap<PageContentContextDriver>(args.This());
    if(!contentContext->ContentContext)
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 1 parameter, gray value (0-255)")));
		return scope.Close(Undefined());
	}
    
    contentContext->ContentContext->G(args[0]->ToNumber()->Value());
    
    return scope.Close(Undefined());
}

Handle<Value> PageContentContextDriver::w(const Arguments& args)
{
    HandleScope scope;
    
    PageContentContextDriver* contentContext = ObjectWrap::Unwrap<PageContentContextDriver>(args.This());
    if(!contentContext->ContentContext)
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 1 parameter, width measure")));
		return scope.Close(Undefined());
	}
    
    contentContext->ContentContext->w(args[0]->ToNumber()->Value());
    
    return scope.Close(Undefined());    
}

Handle<Value> PageContentContextDriver::m(const Arguments& args)
{
    HandleScope scope;
    
    PageContentContextDriver* contentContext = ObjectWrap::Unwrap<PageContentContextDriver>(args.This());
    if(!contentContext->ContentContext)
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 2 || !args[0]->IsNumber() || !args[1]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 2 parameters, movement position")));
		return scope.Close(Undefined());
	}
    
    contentContext->ContentContext->m(args[0]->ToNumber()->Value(),args[1]->ToNumber()->Value());
    
    return scope.Close(Undefined());    
}

Handle<Value> PageContentContextDriver::l(const Arguments& args)
{
    HandleScope scope;
    
    PageContentContextDriver* contentContext = ObjectWrap::Unwrap<PageContentContextDriver>(args.This());
    if(!contentContext->ContentContext)
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 2 || !args[0]->IsNumber() || !args[1]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 2 parameters, line to position")));
		return scope.Close(Undefined());
	}
    
    contentContext->ContentContext->l(args[0]->ToNumber()->Value(),args[1]->ToNumber()->Value());
    
    return scope.Close(Undefined());
}

Handle<Value> PageContentContextDriver::S(const Arguments& args)
{
    HandleScope scope;
    
    PageContentContextDriver* contentContext = ObjectWrap::Unwrap<PageContentContextDriver>(args.This());
    if(!contentContext->ContentContext)
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
    else
        contentContext->ContentContext->S();
    return scope.Close(Undefined());
}


