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
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("ByteWriterWithPosition"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "write", Write);
	SET_PROTOTYPE_METHOD(t, "getCurrentPosition", GetCurrentPosition);
	SET_CONSTRUCTOR(constructor, t);
	SET_CONSTRUCTOR_TEMPLATE(constructor_template, t);
}

METHOD_RETURN_TYPE ByteWriterWithPositionDriver::NewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	SET_FUNCTION_RETURN_VALUE(instance);
}

Handle<Value> ByteWriterWithPositionDriver::GetNewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}


bool ByteWriterWithPositionDriver::HasInstance(Handle<Value> inObject)
{
	CREATE_ISOLATE_CONTEXT;

	return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);
}

METHOD_RETURN_TYPE ByteWriterWithPositionDriver::New(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    ByteWriterWithPositionDriver* driver = new ByteWriterWithPositionDriver();
    driver->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This());
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

METHOD_RETURN_TYPE ByteWriterWithPositionDriver::Write(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    // k. i'll just read the number of bytes and return an array of them
    if(args.Length() != 1 ||
       !args[0]->IsArray())
    {
		THROW_EXCEPTION("Wrong arguments. pass an array of bytes to write");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    ByteWriterWithPositionDriver* element = ObjectWrap::Unwrap<ByteWriterWithPositionDriver>(args.This());
    int bufferSize = args[0]->ToObject()->Get(NEW_STRING("length"))->ToObject()->Uint32Value();
    Byte* buffer = new Byte[bufferSize];
    
    for(int i=0;i<bufferSize;++i)
        buffer[i] = args[0]->ToObject()->Get(i)->ToObject()->Uint32Value();
    
    bufferSize = (int)element->mInstance->Write(buffer,bufferSize);
    
    delete[] buffer;
    
    SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(bufferSize));
}

METHOD_RETURN_TYPE ByteWriterWithPositionDriver::GetCurrentPosition(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    ByteWriterWithPositionDriver* element = ObjectWrap::Unwrap<ByteWriterWithPositionDriver>(args.This());

    SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(element->mInstance->GetCurrentPosition()));
}

