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

ObjectByteReader::ObjectByteReader(Local<Object> inObject)
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

	Local<Value> value = OBJECT_FROM_PERSISTENT(mObject)->Get(GET_CURRENT_CONTEXT, NEW_STRING("read")).ToLocalChecked();

    if(value->IsUndefined())
        return 0;
    Local<Function> func = Local<Function>::Cast(value);
    
    Local<Value> args[1];
    args[0] = NEW_NUMBER(inBufferSize);
    
	Local<Value> result = func->Call(GET_CURRENT_CONTEXT, OBJECT_FROM_PERSISTENT(mObject), 1, args).ToLocalChecked();
    
    if(!result->IsArray())
        return 0;
    
    IOBasicTypes::LongBufferSizeType bufferLength = result->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, v8::NEW_STRING("length")).ToLocalChecked()->TO_UINT32Value();
    for(IOBasicTypes::LongBufferSizeType i=0;i < bufferLength;++i)
        inBuffer[i] = (IOBasicTypes::Byte)(result->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, (uint32_t)i).ToLocalChecked()->TO_UINT32Value());
    
    return bufferLength;
}

bool ObjectByteReader::NotEnded()
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Value> value = OBJECT_FROM_PERSISTENT(mObject)->Get(GET_CURRENT_CONTEXT, NEW_STRING("notEnded")).ToLocalChecked();
    if(value->IsUndefined())
        return true;
    Local<Function> func = Local<Function>::Cast(value);
    
	return (func->Call(GET_CURRENT_CONTEXT,  OBJECT_FROM_PERSISTENT(mObject), 0, NULL).ToLocalChecked()->TO_BOOLEAN()->Value());
}
