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
	CREATE_ISOLATE_CONTEXT;
	
	SET_PERSISTENT_OBJECT(mObject, Object, inObject);
}

ObjectByteReader::~ObjectByteReader()
{
	DISPOSE_PERSISTENT(mObject);
}

IOBasicTypes::LongBufferSizeType ObjectByteReader::Read(IOBasicTypes::Byte* inBuffer,IOBasicTypes::LongBufferSizeType inBufferSize)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Handle<Value> value = OBJECT_FROM_PERSISTENT(mObject)->Get(NEW_STRING("read"));

    if(value->IsUndefined())
        return 0;
    Handle<Function> func = Handle<Function>::Cast(value);
    
    Handle<Value> args[1];
    args[0] = NEW_NUMBER(inBufferSize);
    
	Handle<Value> result = func->Call(OBJECT_FROM_PERSISTENT(mObject), 1, args);
    
    if(!result->IsArray())
        return 0;
    
    IOBasicTypes::LongBufferSizeType bufferLength = result->ToObject()->Get(v8::NEW_STRING("length"))->ToObject()->Uint32Value();
    for(IOBasicTypes::LongBufferSizeType i=0;i < bufferLength;++i)
        inBuffer[i] = (IOBasicTypes::Byte)(value->ToObject()->Get((uint32_t)i)->ToNumber()->Uint32Value());
    
    return bufferLength;
}

bool ObjectByteReader::NotEnded()
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Handle<Value> value = OBJECT_FROM_PERSISTENT(mObject)->Get(NEW_STRING("notEnded"));
    if(value->IsUndefined())
        return true;
    Handle<Function> func = Handle<Function>::Cast(value);
    
	return (func->Call(OBJECT_FROM_PERSISTENT(mObject), 0, NULL)->ToBoolean()->Value());
}
