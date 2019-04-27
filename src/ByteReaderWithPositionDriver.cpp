/*
 Source File : ByteReaderWithPositionDriver.h
 
 
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
#include "ByteReaderWithPositionDriver.h"
#include "IByteReaderWithPosition.h"
#include "ConstructorsHolder.h"

using namespace v8;

ByteReaderWithPositionDriver::ByteReaderWithPositionDriver()
{
    mInstance = NULL;
    mOwns = false;
}

ByteReaderWithPositionDriver::~ByteReaderWithPositionDriver()
{
    if(mOwns)
        delete mInstance;
}

DEF_SUBORDINATE_INIT(ByteReaderWithPositionDriver::Init)
{

	CREATE_ISOLATE_CONTEXT;

	// prepare the page interfrace template
	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("ByteReaderWithPosition"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	// prepare the page interfrace template
	SET_PROTOTYPE_METHOD(t, "read", Read);
	SET_PROTOTYPE_METHOD(t, "notEnded", NotEnded);
	SET_PROTOTYPE_METHOD(t, "setPosition", SetPosition);
	SET_PROTOTYPE_METHOD(t, "getCurrentPosition", GetCurrentPosition);
	SET_PROTOTYPE_METHOD(t, "setPositionFromEnd", SetPositionFromEnd);
	SET_PROTOTYPE_METHOD(t, "skip", Skip);



    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->ByteReaderWithPosition_constructor, t);  
	SET_CONSTRUCTOR_TEMPLATE(holder->ByteReaderWithPosition_constructor_template, t);
}

METHOD_RETURN_TYPE ByteReaderWithPositionDriver::New(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    ByteReaderWithPositionDriver* driver = new ByteReaderWithPositionDriver();
    driver->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This())
}

void ByteReaderWithPositionDriver::SetStream(IByteReaderWithPosition* inReader,bool inOwns)
{
    if(mOwns)
        delete mInstance;
    mInstance = inReader;
    mOwns = inOwns;
}

IByteReaderWithPosition* ByteReaderWithPositionDriver::GetStream()
{
    return mInstance;
}

using namespace IOBasicTypes;

METHOD_RETURN_TYPE ByteReaderWithPositionDriver::Read(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    // k. i'll just read the number of bytes and return an array of them
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong arguments. pass the number of bytes to read");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    ByteReaderWithPositionDriver* element = ObjectWrap::Unwrap<ByteReaderWithPositionDriver>(args.This());
    IOBasicTypes::LongBufferSizeType bufferSize = TO_UINT32(args[0])->Value();
    IOBasicTypes::Byte* buffer = new IOBasicTypes::Byte[bufferSize];
    
    bufferSize = element->mInstance->Read(buffer,(int)bufferSize); // reading int cause that's the maximum that can read (use should read till notended anyways)
    
    Local<Array> outBuffer = NEW_ARRAY((int)bufferSize);
    
    for(LongBufferSizeType i=0;i<bufferSize;++i)
        outBuffer->Set(GET_CURRENT_CONTEXT, NEW_NUMBER(i),NEW_NUMBER(buffer[i]));
    
    SET_FUNCTION_RETURN_VALUE(outBuffer)
}

METHOD_RETURN_TYPE ByteReaderWithPositionDriver::NotEnded(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    ByteReaderWithPositionDriver* element = ObjectWrap::Unwrap<ByteReaderWithPositionDriver>(args.This());
    
    SET_FUNCTION_RETURN_VALUE(NEW_BOOLEAN(element->mInstance->NotEnded()))
}

METHOD_RETURN_TYPE ByteReaderWithPositionDriver::GetCurrentPosition(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    ByteReaderWithPositionDriver* element = ObjectWrap::Unwrap<ByteReaderWithPositionDriver>(args.This());
    
    SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(element->mInstance->GetCurrentPosition()))
}

METHOD_RETURN_TYPE ByteReaderWithPositionDriver::Skip(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong arguments. pass the number of bytes to skip");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }

    
    ByteReaderWithPositionDriver* element = ObjectWrap::Unwrap<ByteReaderWithPositionDriver>(args.This());
    element->mInstance->Skip(TO_UINT32(args[0])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE ByteReaderWithPositionDriver::SetPosition(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong arguments. pass the position");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    
    ByteReaderWithPositionDriver* element = ObjectWrap::Unwrap<ByteReaderWithPositionDriver>(args.This());
    element->mInstance->SetPosition(TO_UINT32(args[0])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE ByteReaderWithPositionDriver::SetPositionFromEnd(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong arguments. pass the position");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    
    ByteReaderWithPositionDriver* element = ObjectWrap::Unwrap<ByteReaderWithPositionDriver>(args.This());
    element->mInstance->SetPositionFromEnd(TO_UINT32(args[0])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}
