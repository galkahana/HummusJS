/*
 Source File : ByteWriterWithPositionDriver.h
 
 
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
#include "ByteWriterWithPositionDriver.h"
#include "IByteWriterWithPosition.h"

using namespace v8;

Persistent<Function> ByteWriterWithPositionDriver::constructor;
Persistent<FunctionTemplate> ByteWriterWithPositionDriver::constructor_template;

ByteWriterWithPositionDriver::ByteWriterWithPositionDriver()
{
    mInstance = NULL;
    mOwns = false;
}

ByteWriterWithPositionDriver::~ByteWriterWithPositionDriver()
{
    if(mOwns)
        delete mInstance;
}


void ByteWriterWithPositionDriver::Init()
{
    // prepare the page interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("ByteWriterWithPosition"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("write"),FunctionTemplate::New(Write)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("getCurrentPosition"),FunctionTemplate::New(GetCurrentPosition)->GetFunction());
    
    constructor = Persistent<Function>::New(constructor_template->GetFunction());
}

Handle<Value> ByteWriterWithPositionDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    Local<Object> instance = constructor->NewInstance();
    return scope.Close(instance);
}

bool ByteWriterWithPositionDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}

Handle<Value> ByteWriterWithPositionDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    ByteWriterWithPositionDriver* driver = new ByteWriterWithPositionDriver();
    driver->Wrap(args.This());
    return args.This();
}

void ByteWriterWithPositionDriver::SetStream(IByteWriterWithPosition* inWriterWithPosition,bool inOwns)
{
    if(mOwns)
        delete mInstance;
    mInstance = inWriterWithPosition;
    mOwns = inOwns;
}

IByteWriterWithPosition* ByteWriterWithPositionDriver::GetStream()
{
    return mInstance;
}

using namespace IOBasicTypes;

v8::Handle<v8::Value> ByteWriterWithPositionDriver::Write(const v8::Arguments& args)
{
    HandleScope scope;
    
    // k. i'll just read the number of bytes and return an array of them
    if(args.Length() != 1 ||
       !args[0]->IsArray())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. pass an array of bytes to write")));
        return scope.Close(Undefined());
    }
    
    ByteWriterWithPositionDriver* element = ObjectWrap::Unwrap<ByteWriterWithPositionDriver>(args.This());
    int bufferSize = args[0]->ToObject()->Get(v8::String::New("length"))->ToObject()->Uint32Value();
    Byte* buffer = new Byte[bufferSize];
    
    for(int i=0;i<bufferSize;++i)
        buffer[i] = args[0]->ToObject()->Get(i)->ToObject()->Uint32Value();
    
    bufferSize = (int)element->mInstance->Write(buffer,bufferSize);
    
    delete[] buffer;
    
    return scope.Close(Number::New(bufferSize));
}

v8::Handle<v8::Value> ByteWriterWithPositionDriver::GetCurrentPosition(const v8::Arguments& args)
{
    HandleScope scope;
    
    ByteWriterWithPositionDriver* element = ObjectWrap::Unwrap<ByteWriterWithPositionDriver>(args.This());

    return scope.Close(Number::New(element->mInstance->GetCurrentPosition()));
}

