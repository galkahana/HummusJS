/*
 Source File : AbstractContentContextDriver.h
 
 
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

#include "AbstractContentContextDriver.h"
#include "AbstractContentContext.h"

using namespace v8;


void AbstractContentContextDriver::Init(Handle<FunctionTemplate>& ioDriverTemplate)
{
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("q"),FunctionTemplate::New(q)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("k"),FunctionTemplate::New(k)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("re"),FunctionTemplate::New(re)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("f"),FunctionTemplate::New(f)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("Q"),FunctionTemplate::New(Q)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("G"),FunctionTemplate::New(G)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("w"),FunctionTemplate::New(w)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("m"),FunctionTemplate::New(m)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("l"),FunctionTemplate::New(l)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("S"),FunctionTemplate::New(S)->GetFunction());

}

Handle<Value> AbstractContentContextDriver::q(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
    else
        contentContext->GetContext()->q();
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::k(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 4 || !args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber()) {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 4 cmyk components (values should be 0-255)")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->k(args[0]->ToNumber()->Value(),
                                      args[1]->ToNumber()->Value(),
                                      args[2]->ToNumber()->Value(),
                                      args[3]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::re(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 4 || !args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber()) {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 4 parameters: 2 bottom left coordinates, and width and height measures")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->re(args[0]->ToNumber()->Value(),
                                       args[1]->ToNumber()->Value(),
                                       args[2]->ToNumber()->Value(),
                                       args[3]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::f(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
    else
        contentContext->GetContext()->f();
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::Q(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
    else
        contentContext->GetContext()->Q();
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::G(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 1 parameter, gray value (0-255)")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->G(args[0]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::w(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 1 parameter, width measure")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->w(args[0]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::m(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 2 || !args[0]->IsNumber() || !args[1]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 2 parameters, movement position")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->m(args[0]->ToNumber()->Value(),args[1]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::l(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 2 || !args[0]->IsNumber() || !args[1]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 2 parameters, line to position")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->l(args[0]->ToNumber()->Value(),args[1]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::S(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
    else
        contentContext->GetContext()->S();
    return scope.Close(args.This());
}
