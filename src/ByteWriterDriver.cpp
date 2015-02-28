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

	CREATE_ISOLATE_CONTEXT;

	// prepare the page interfrace template
	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("ByteWriter"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "write", Write);
	SET_CONSTRUCTOR(constructor, t);
	SET_CONSTRUCTOR_TEMPLATE(constructor_template, t);
}

METHOD_RETURN_TYPE ByteWriterDriver::NewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	SET_FUNCTION_RETURN_VALUE(instance);
}

v8::Handle<v8::Value> ByteWriterDriver::GetNewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

bool ByteWriterDriver::HasInstance(Handle<Value> inObject)
{
	CREATE_ISOLATE_CONTEXT;

	return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);
}

METHOD_RETURN_TYPE ByteWriterDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    ByteWriterDriver* driver = new ByteWriterDriver();
    driver->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This());
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

METHOD_RETURN_TYPE ByteWriterDriver::Write(const ARGS_TYPE& args)
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
    
    ByteWriterDriver* element = ObjectWrap::Unwrap<ByteWriterDriver>(args.This());
    int bufferSize = args[0]->ToObject()->Get(NEW_STRING("length"))->ToObject()->Uint32Value();
    Byte* buffer = new Byte[bufferSize];
    
    for(int i=0;i<bufferSize;++i)
        buffer[i] = args[0]->ToObject()->Get(i)->ToObject()->Uint32Value();

    bufferSize = (int)element->mInstance->Write(buffer,bufferSize);
    
    delete[] buffer;
    
	SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(bufferSize));
}


