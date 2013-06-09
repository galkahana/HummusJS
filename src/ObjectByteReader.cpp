/*
 Source File : ObjectByteReader.h
 
 
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
#include "ObjectByteReader.h"

using namespace v8;

ObjectByteReader::ObjectByteReader(Handle<Object> inObject)
{
    mObject = Persistent<Object>::New(inObject);
}

ObjectByteReader::~ObjectByteReader()
{
    mObject.Dispose();
}

IOBasicTypes::LongBufferSizeType ObjectByteReader::Read(IOBasicTypes::Byte* inBuffer,IOBasicTypes::LongBufferSizeType inBufferSize)
{
    HandleScope handle;
    
    Handle<Value> value = mObject->Get(String::New("read"));
    if(value->IsUndefined())
        return 0;
    Handle<Function> func = Handle<Function>::Cast(value);
    
    Handle<Value> args[1];
    args[0] = Number::New(inBufferSize);
    
    Handle<Value> result = func->Call(mObject, 1, args);
    
    if(!result->IsArray())
        return 0;
    
    if(value->ToObject()->Get(v8::String::New("length"))->ToObject()->Uint32Value() != 4)
        ThrowException(Exception::TypeError(String::New("Media box is set to a value which is not a 4 numbers array")));
    
    IOBasicTypes::LongBufferSizeType bufferLength = result->ToObject()->Get(v8::String::New("length"))->ToObject()->Uint32Value();
    for(IOBasicTypes::LongBufferSizeType i=0;i < bufferLength;++i)
        inBuffer[i] = (IOBasicTypes::Byte)(value->ToObject()->Get((uint32_t)i)->ToNumber()->Uint32Value());
    
    return bufferLength;
}

bool ObjectByteReader::NotEnded()
{
    HandleScope handle;

    Handle<Value> value = mObject->Get(String::New("notEnded"));
    if(value->IsUndefined())
        return true;
    Handle<Function> func = Handle<Function>::Cast(value);
    
    return (func->Call(mObject, 0, NULL)->ToBoolean()->Value());
}
