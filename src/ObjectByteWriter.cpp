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
	CREATE_ISOLATE_CONTEXT;

	SET_PERSISTENT_OBJECT(mObject, Object, inObject);
}

ObjectByteWriter::~ObjectByteWriter()
{
	DISPOSE_PERSISTENT(mObject);
}

IOBasicTypes::LongBufferSizeType ObjectByteWriter::Write(const IOBasicTypes::Byte* inBuffer,IOBasicTypes::LongBufferSizeType inBufferSize)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    Handle<Object> anArray = NEW_ARRAY((int)inBufferSize);
    for(int i=0;i<(int)inBufferSize;++i)
        anArray->Set(NEW_NUMBER(i),NEW_NUMBER(inBuffer[i]));
    
	Handle<Value> value = OBJECT_FROM_PERSISTENT(mObject)->Get(NEW_STRING("write"));
    if(value->IsUndefined())
        return 0;
    Handle<Function> func = Handle<Function>::Cast(value);
    Handle<Value> result;
    
    Handle<Value> args[1];
    args[0] = anArray;
    
	return (func->Call(OBJECT_FROM_PERSISTENT(mObject), 1, args)->ToNumber()->Uint32Value());
}