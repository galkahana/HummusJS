/*
 Source File : ByteReaderDriver.h
 
 
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
#include "ByteReaderDriver.h"
#include "IByteReader.h"

using namespace v8;

Persistent<Function> ByteReaderDriver::constructor;
Persistent<FunctionTemplate> ByteReaderDriver::constructor_template;

ByteReaderDriver::ByteReaderDriver()
{
    mInstance = NULL;
    mOwns = false;
}

ByteReaderDriver::~ByteReaderDriver()
{
    if(mOwns)
        delete mInstance;
}


void ByteReaderDriver::Init()
{
    // prepare the page interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("ByteReader"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("read"),FunctionTemplate::New(Read)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("notEnded"),FunctionTemplate::New(NotEnded)->GetFunction());
    
    constructor = Persistent<Function>::New(constructor_template->GetFunction());
}

Handle<Value> ByteReaderDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    Local<Object> instance = constructor->NewInstance();
    return scope.Close(instance);
}

bool ByteReaderDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}

Handle<Value> ByteReaderDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    ByteReaderDriver* driver = new ByteReaderDriver();
    driver->Wrap(args.This());
    return args.This();
}

void ByteReaderDriver::SetStream(IByteReader* inReader,bool inOwns)
{
    if(mOwns)
        delete mInstance;
    mInstance = inReader;
    mOwns = inOwns;
}

IByteReader* ByteReaderDriver::GetStream()
{
    return mInstance;
}

using namespace IOBasicTypes;

v8::Handle<v8::Value> ByteReaderDriver::Read(const v8::Arguments& args)
{
    HandleScope scope;
    
    // k. i'll just read the number of bytes and return an array of them
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. pass the number of bytes to read")));
        return scope.Close(Undefined());
    }
    
    ByteReaderDriver* element = ObjectWrap::Unwrap<ByteReaderDriver>(args.This());
    IOBasicTypes::LongBufferSizeType bufferSize = args[0]->ToNumber()->Uint32Value();
    Byte* buffer = new Byte[bufferSize];
    
    bufferSize = element->mInstance->Read(buffer,(int)bufferSize); // reading int cause that's the maximum that can read (use should read till notended anyways)

    Local<Array> outBuffer = Array::New((int)bufferSize);
    
    for(LongBufferSizeType i=0;i<bufferSize;++i)
        outBuffer->Set(Number::New(i),Number::New(buffer[i]));
    
    delete[] buffer;
    
    return scope.Close(outBuffer);
}

Handle<Value> ByteReaderDriver::NotEnded(const Arguments& args)
{
    HandleScope scope;
    
    ByteReaderDriver* element = ObjectWrap::Unwrap<ByteReaderDriver>(args.This());
    
    return scope.Close(Boolean::New(element->mInstance->NotEnded()));
}


