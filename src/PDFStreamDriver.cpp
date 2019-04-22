/*
 Source File : PDFStreamDriver.h
 
 
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
#include "PDFStreamDriver.h"
#include "PDFStream.h"
#include "ByteWriterDriver.h"
#include "ConstructorsHolder.h"

using namespace v8;



PDFStreamDriver::PDFStreamDriver()
{
    PDFStreamInstance = NULL;
	mOwns = false;
}

PDFStreamDriver::~PDFStreamDriver()
{
	if(mOwns)
		delete PDFStreamInstance;
}

DEF_SUBORDINATE_INIT(PDFStreamDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE_EXTERNAL(New);

	t->SetClassName(NEW_STRING("PDFStream"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "getWriteStream", GetWriteStream);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->PDFStream_constructor, t);   	
	SET_CONSTRUCTOR_TEMPLATE(holder->PDFStream_constructor_template,t);
}

METHOD_RETURN_TYPE PDFStreamDriver::New(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
	EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, externalHolder)

    PDFStreamDriver* driver = new PDFStreamDriver();
	driver->holder = externalHolder;
    driver->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE PDFStreamDriver::GetWriteStream(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    PDFStreamDriver* stream = ObjectWrap::Unwrap<PDFStreamDriver>(args.This());


    Local<Value> result = stream->holder->GetNewByteWriter(args);
    
    ObjectWrap::Unwrap<ByteWriterDriver>(result->TO_OBJECT())->SetStream(stream->PDFStreamInstance->GetWriteStream(), false);
    
	SET_FUNCTION_RETURN_VALUE(result)
}
