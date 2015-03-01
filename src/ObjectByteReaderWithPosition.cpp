/*
 Source File : ObjectByteReaderWithPosition.h
 
 
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
#include "ObjectByteReaderWithPosition.h"

using namespace v8;

ObjectByteReaderWithPosition::ObjectByteReaderWithPosition(Handle<Object> inObject)
{
	CREATE_ISOLATE_CONTEXT;

	SET_PERSISTENT_OBJECT(mObject, Object, inObject);
}

ObjectByteReaderWithPosition::~ObjectByteReaderWithPosition()
{
	DISPOSE_PERSISTENT(mObject);
}

IOBasicTypes::LongBufferSizeType ObjectByteReaderWithPosition::Read(IOBasicTypes::Byte* inBuffer,IOBasicTypes::LongBufferSizeType inBufferSize)
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
    
    IOBasicTypes::LongBufferSizeType bufferLength = result->ToObject()->Get(NEW_STRING("length"))->ToObject()->Uint32Value();
    for(IOBasicTypes::LongBufferSizeType i=0;i < bufferLength;++i)
        inBuffer[i] = (IOBasicTypes::Byte)(result->ToObject()->Get((uint32_t)i)->ToNumber()->Uint32Value());
    
    return bufferLength;
    
}

bool ObjectByteReaderWithPosition::NotEnded()
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Handle<Value> value = OBJECT_FROM_PERSISTENT(mObject)->Get(NEW_STRING("notEnded"));
    if(value->IsUndefined())
        return true;
    Handle<Function> func = Handle<Function>::Cast(value);
    
	return (func->Call(OBJECT_FROM_PERSISTENT(mObject), 0, NULL)->ToBoolean()->Value());
}

void ObjectByteReaderWithPosition::SetPosition(LongFilePositionType inOffsetFromStart)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Handle<Value> value = OBJECT_FROM_PERSISTENT(mObject)->Get(NEW_STRING("setPosition"));
    if(value->IsUndefined())
        return;
    Handle<Function> func = Handle<Function>::Cast(value);
    
    Handle<Value> args[1];
    args[0] = NEW_NUMBER(inOffsetFromStart);
	func->Call(OBJECT_FROM_PERSISTENT(mObject), 1, args);
}

void ObjectByteReaderWithPosition::SetPositionFromEnd(LongFilePositionType inOffsetFromStart)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Handle<Value> value = OBJECT_FROM_PERSISTENT(mObject)->Get(NEW_STRING("setPositionFromEnd"));
    if(value->IsUndefined())
        return;
    Handle<Function> func = Handle<Function>::Cast(value);
    
    Handle<Value> args[1];
    args[0] = NEW_NUMBER(inOffsetFromStart);
	func->Call(OBJECT_FROM_PERSISTENT(mObject), 1, args);
}

LongFilePositionType ObjectByteReaderWithPosition::GetCurrentPosition()
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Handle<Value> value = OBJECT_FROM_PERSISTENT(mObject)->Get(NEW_STRING("getCurrentPosition"));
    if(value->IsUndefined())
        return true;
    Handle<Function> func = Handle<Function>::Cast(value);
    
	return (func->Call(OBJECT_FROM_PERSISTENT(mObject), 0, NULL)->ToNumber()->Value());
}

void ObjectByteReaderWithPosition::Skip(LongBufferSizeType inSkipSize)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Handle<Value> value = OBJECT_FROM_PERSISTENT(mObject)->Get(NEW_STRING("skip"));
    if(value->IsUndefined())
        return;
    Handle<Function> func = Handle<Function>::Cast(value);
    
    Handle<Value> args[1];
    args[0] = NEW_NUMBER(inSkipSize);
	func->Call(OBJECT_FROM_PERSISTENT(mObject), 1, args);
}
