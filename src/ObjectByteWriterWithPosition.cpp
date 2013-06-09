/*
 Source File : ObjectByteWriterWithPosition.h
 
 
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
#include "ObjectByteWriterWithPosition.h"

using namespace v8;

ObjectByteWriterWithPosition::ObjectByteWriterWithPosition(Handle<Object> inObject)
{
    mObject = Persistent<Object>::New(inObject);
}

ObjectByteWriterWithPosition::~ObjectByteWriterWithPosition()
{
    mObject.Dispose();
}

IOBasicTypes::LongBufferSizeType ObjectByteWriterWithPosition::Write(const IOBasicTypes::Byte* inBuffer,IOBasicTypes::LongBufferSizeType inBufferSize)
{
    HandleScope handle;
    
    Handle<Object> anArray = Array::New((int)inBufferSize);
    for(int i=0;i<(int)inBufferSize;++i)
        anArray->Set(Number::New(i),Number::New(inBuffer[i]));
    
    Handle<Value> value = mObject->Get(String::New("write"));
    if(value->IsUndefined() || !value->IsFunction())
    {
		ThrowException(Exception::TypeError(String::New("write is not a function, it should be you know...")));
        return 0;
    }
    Handle<Function> func = Handle<Function>::Cast(value);
    
    Handle<Value> args[1];
    args[0] = anArray;
    
    Handle<Value> result = func->Call(mObject, 1, args);
    if(result.IsEmpty())
    {
		ThrowException(Exception::TypeError(String::New("wrong return value. it's empty. return the number of written characters")));
		return 0;
    }
    else if(result->IsNumber())
    {
        return result->ToNumber()->Uint32Value();
    }
    else
    {
		ThrowException(Exception::TypeError(String::New("wrong return value. write should return the number of written characters")));
		return 0;
    }
}

IOBasicTypes::LongFilePositionType ObjectByteWriterWithPosition::GetCurrentPosition()
{
    HandleScope handle;
    
    Handle<Value> value = mObject->Get(String::New("getCurrentPosition"));
    if(value->IsUndefined())
        return true;
    Handle<Function> func = Handle<Function>::Cast(value);
    
    return (func->Call(mObject, 0, NULL)->ToNumber()->Value());
}