/*
 Source File : ByteReaderWithPositionDriver.h
 
 
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
#include "ByteReaderWithPositionDriver.h"
#include "IByteReaderWithPosition.h"

using namespace v8;

Persistent<Function> ByteReaderWithPositionDriver::constructor;
Persistent<FunctionTemplate> ByteReaderWithPositionDriver::constructor_template;

ByteReaderWithPositionDriver::ByteReaderWithPositionDriver()
{
    mInstance = NULL;
    mOwns = false;
}

ByteReaderWithPositionDriver::~ByteReaderWithPositionDriver()
{
    if(mOwns)
        delete mInstance;
}


void ByteReaderWithPositionDriver::Init()
{
    // prepare the page interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("ByteReaderWithPosition"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("read"),FunctionTemplate::New(Read)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("notEnded"),FunctionTemplate::New(NotEnded)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("setPosition"),FunctionTemplate::New(SetPosition)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("getCurrentPosition"),FunctionTemplate::New(GetCurrentPosition)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("setPositionFromEnd"),FunctionTemplate::New(SetPositionFromEnd)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("skip"),FunctionTemplate::New(Skip)->GetFunction());
    
    constructor = Persistent<Function>::New(constructor_template->GetFunction());
}

Handle<Value> ByteReaderWithPositionDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    Local<Object> instance = constructor->NewInstance();
    return scope.Close(instance);
}

bool ByteReaderWithPositionDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}

Handle<Value> ByteReaderWithPositionDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    ByteReaderWithPositionDriver* driver = new ByteReaderWithPositionDriver();
    driver->Wrap(args.This());
    return args.This();
}

void ByteReaderWithPositionDriver::SetStream(IByteReaderWithPosition* inReader,bool inOwns)
{
    if(mOwns)
        delete mInstance;
    mInstance = inReader;
    mOwns = inOwns;
}

IByteReaderWithPosition* ByteReaderWithPositionDriver::GetStream()
{
    return mInstance;
}

using namespace IOBasicTypes;

v8::Handle<v8::Value> ByteReaderWithPositionDriver::Read(const v8::Arguments& args)
{
    HandleScope scope;
    
    // k. i'll just read the number of bytes and return an array of them
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. pass the number of bytes to read")));
        return scope.Close(Undefined());
    }
    
    ByteReaderWithPositionDriver* element = ObjectWrap::Unwrap<ByteReaderWithPositionDriver>(args.This());
    IOBasicTypes::LongBufferSizeType bufferSize = args[0]->ToNumber()->Uint32Value();
    Byte* buffer = new Byte[bufferSize];
    
    bufferSize = element->mInstance->Read(buffer,(int)bufferSize); // reading int cause that's the maximum that can read (use should read till notended anyways)
    
    Local<Array> outBuffer = Array::New((int)bufferSize);
    
    for(LongBufferSizeType i=0;i<bufferSize;++i)
        outBuffer->Set(Number::New(i),Number::New(buffer[i]));
    
    return scope.Close(outBuffer);
}

Handle<Value> ByteReaderWithPositionDriver::NotEnded(const Arguments& args)
{
    HandleScope scope;
    
    ByteReaderWithPositionDriver* element = ObjectWrap::Unwrap<ByteReaderWithPositionDriver>(args.This());
    
    return scope.Close(Boolean::New(element->mInstance->NotEnded()));
}

Handle<Value> ByteReaderWithPositionDriver::GetCurrentPosition(const Arguments& args)
{
    HandleScope scope;
    
    ByteReaderWithPositionDriver* element = ObjectWrap::Unwrap<ByteReaderWithPositionDriver>(args.This());
    
    return scope.Close(Number::New(element->mInstance->GetCurrentPosition()));
}

Handle<Value> ByteReaderWithPositionDriver::Skip(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. pass the number of bytes to skip")));
        return scope.Close(Undefined());
    }

    
    ByteReaderWithPositionDriver* element = ObjectWrap::Unwrap<ByteReaderWithPositionDriver>(args.This());
    element->mInstance->Skip(args[0]->ToNumber()->Uint32Value());
    
    return scope.Close(args.This());
}

Handle<Value> ByteReaderWithPositionDriver::SetPosition(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. pass the position")));
        return scope.Close(Undefined());
    }
    
    
    ByteReaderWithPositionDriver* element = ObjectWrap::Unwrap<ByteReaderWithPositionDriver>(args.This());
    element->mInstance->SetPosition(args[0]->ToNumber()->Uint32Value());
    
    return scope.Close(args.This());
}

Handle<Value> ByteReaderWithPositionDriver::SetPositionFromEnd(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. pass the position")));
        return scope.Close(Undefined());
    }
    
    
    ByteReaderWithPositionDriver* element = ObjectWrap::Unwrap<ByteReaderWithPositionDriver>(args.This());
    element->mInstance->SetPositionFromEnd(args[0]->ToNumber()->Uint32Value());
    
    return scope.Close(args.This());
}
