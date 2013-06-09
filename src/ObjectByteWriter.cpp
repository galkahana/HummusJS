/*
 Source File : ObjectByteWriter.h
 
 
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
#include "ObjectByteWriter.h"

using namespace v8;

ObjectByteWriter::ObjectByteWriter(Handle<Object> inObject)
{
    mObject = Persistent<Object>::New(inObject);
}

ObjectByteWriter::~ObjectByteWriter()
{
    mObject.Dispose();
}

IOBasicTypes::LongBufferSizeType ObjectByteWriter::Write(const IOBasicTypes::Byte* inBuffer,IOBasicTypes::LongBufferSizeType inBufferSize)
{
    HandleScope handle;
    
    Handle<Object> anArray = Array::New((int)inBufferSize);
    for(int i=0;i<(int)inBufferSize;++i)
        anArray->Set(Number::New(i),Number::New(inBuffer[i]));
    
    Handle<Value> value = mObject->Get(String::New("write"));
    if(value->IsUndefined())
        return 0;
    Handle<Function> func = Handle<Function>::Cast(value);
    Handle<Value> result;
    
    Handle<Value> args[1];
    args[0] = anArray;
    
    return (func->Call(mObject, 1, args)->ToNumber()->Uint32Value());
}