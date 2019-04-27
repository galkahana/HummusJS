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
#include "ConstructorsHolder.h"

using namespace v8;

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


DEF_SUBORDINATE_INIT(ByteWriterDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	// prepare the page interfrace template
	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("ByteWriter"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "write", Write);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->ByteWriter_constructor, t);   
	SET_CONSTRUCTOR_TEMPLATE(holder->ByteWriter_constructor_template, t);
}

METHOD_RETURN_TYPE ByteWriterDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    ByteWriterDriver* driver = new ByteWriterDriver();
    driver->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This())
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
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    ByteWriterDriver* element = ObjectWrap::Unwrap<ByteWriterDriver>(args.This());
    int bufferSize = args[0]->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, NEW_STRING("length")).ToLocalChecked()->TO_UINT32Value();
    IOBasicTypes::Byte* buffer = new IOBasicTypes::Byte[bufferSize];
    
    for(int i=0;i<bufferSize;++i)
        buffer[i] = args[0]->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, i).ToLocalChecked()->TO_UINT32Value();

    bufferSize = (int)element->mInstance->Write(buffer,bufferSize);
    
    delete[] buffer;
    
	SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(bufferSize))
}


