/*
 Source File : ByteWriterDriver.h
 
 
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
#include "ByteWriterDriver.h"
#include "IByteWriter.h"

using namespace v8;

Persistent<Function> ByteWriterDriver::constructor;
Persistent<FunctionTemplate> ByteWriterDriver::constructor_template;

ByteWriterDriver::ByteWriterDriver()
{
    mInstance = NULL;
    mOwns = false;
}

ByteWriterDriver::~ByteWriterDriver()
{
    if(mOwns)
        delete mInstance;
}


void ByteWriterDriver::Init()
{
    // prepare the page interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("ByteWriter"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("write"),FunctionTemplate::New(Write)->GetFunction());
    
    constructor = Persistent<Function>::New(constructor_template->GetFunction());
}

Handle<Value> ByteWriterDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    Local<Object> instance = constructor->NewInstance();
    return scope.Close(instance);
}

bool ByteWriterDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}

Handle<Value> ByteWriterDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    ByteWriterDriver* driver = new ByteWriterDriver();
    driver->Wrap(args.This());
    return args.This();
}

void ByteWriterDriver::SetStream(IByteWriter* inWriter,bool inOwns)
{
    if(mOwns)
        delete mInstance;
    mInstance = inWriter;
    mOwns = inOwns;
}

IByteWriter* ByteWriterDriver::GetStream()
{
    return mInstance;
}

using namespace IOBasicTypes;

v8::Handle<v8::Value> ByteWriterDriver::Write(const v8::Arguments& args)
{
    HandleScope scope;
    
    // k. i'll just read the number of bytes and return an array of them
    if(args.Length() != 1 ||
       !args[0]->IsArray())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. pass an array of bytes to write")));
        return scope.Close(Undefined());
    }
    
    ByteWriterDriver* element = ObjectWrap::Unwrap<ByteWriterDriver>(args.This());
    int bufferSize = args[0]->ToObject()->Get(v8::String::New("length"))->ToObject()->Uint32Value();
    Byte* buffer = new Byte[bufferSize];
    
    for(int i=0;i<bufferSize;++i)
        buffer[i] = args[0]->ToObject()->Get(i)->ToObject()->Uint32Value();

    bufferSize = (int)element->mInstance->Write(buffer,bufferSize);
    
    delete[] buffer;
    
    return scope.Close(Number::New(bufferSize));
}


