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
	CREATE_ISOLATE_CONTEXT;

	SET_PERSISTENT_OBJECT(mObject, Object, inObject);
}

ObjectByteWriterWithPosition::~ObjectByteWriterWithPosition()
{
	DISPOSE_PERSISTENT(mObject);
}

IOBasicTypes::LongBufferSizeType ObjectByteWriterWithPosition::Write(const IOBasicTypes::Byte* inBuffer,IOBasicTypes::LongBufferSizeType inBufferSize)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    Handle<Object> anArray = NEW_ARRAY((int)inBufferSize);
    for(int i=0;i<(int)inBufferSize;++i)
        anArray->Set(NEW_NUMBER(i),NEW_NUMBER(inBuffer[i]));
    
	Handle<Value> value = OBJECT_FROM_PERSISTENT(mObject)->Get(NEW_STRING("write"));
    if(value->IsUndefined() || !value->IsFunction())
    {
		THROW_EXCEPTION("write is not a function, it should be you know...");
        return 0;
    }
    Handle<Function> func = Handle<Function>::Cast(value);
    
    Handle<Value> args[1];
    args[0] = anArray;
    
	Handle<Value> result = func->Call(OBJECT_FROM_PERSISTENT(mObject), 1, args);
    if(result.IsEmpty())
    {
		THROW_EXCEPTION("wrong return value. it's empty. return the number of written characters");
		return 0;
    }
    else if(result->IsNumber())
    {
        return result->ToNumber()->Uint32Value();
    }
    else
    {
		THROW_EXCEPTION("wrong return value. write should return the number of written characters");
		return 0;
    }
}

IOBasicTypes::LongFilePositionType ObjectByteWriterWithPosition::GetCurrentPosition()
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Handle<Value> value = OBJECT_FROM_PERSISTENT(mObject)->Get(NEW_STRING("getCurrentPosition"));
    if(value->IsUndefined())
        return true;
    Handle<Function> func = Handle<Function>::Cast(value);
    
	return (func->Call(OBJECT_FROM_PERSISTENT(mObject), 0, NULL)->ToNumber()->Value());
}