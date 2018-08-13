/*
 Source File : InputFileDriver.h
 
 
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
#include "InputFileDriver.h"
#include "InputFile.h"
#include "ByteReaderWithPositionDriver.h"


using namespace v8;

Persistent<Function> InputFileDriver::constructor;
Persistent<FunctionTemplate> InputFileDriver::constructor_template;

InputFileDriver::InputFileDriver()
{
    mInputFileInstance = new InputFile();
    mOwnsInstance = true;
}

InputFileDriver::~InputFileDriver()
{
    if(mOwnsInstance)
        delete mInputFileInstance;
}

PDFHummus::EStatusCode InputFileDriver::OpenFile(const std::string& inFilePath)
{
    if(!mInputFileInstance)
        mInputFileInstance = new InputFile();
    mOwnsInstance = true;
    return mInputFileInstance->OpenFile(inFilePath);
}

void InputFileDriver::SetFromOwnedFile(InputFile* inFile)
{
    if(mInputFileInstance && mOwnsInstance)
        delete mInputFileInstance;
    mOwnsInstance = false;
    mInputFileInstance = inFile;
}

void InputFileDriver::Init(Handle<Object> inExports)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("InputFile"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "openFile", OpenFile);
	SET_PROTOTYPE_METHOD(t, "closeFile", CloseFile);
	SET_PROTOTYPE_METHOD(t, "getFilePath", GetFilePath);
	SET_PROTOTYPE_METHOD(t, "getFileSize", GetFileSize);
	SET_PROTOTYPE_METHOD(t, "getInputStream", GetInputStream);
	SET_CONSTRUCTOR(constructor, t);
	SET_CONSTRUCTOR_TEMPLATE(constructor_template, t);
	
	SET_CONSTRUCTOR_EXPORT(inExports, "InputFile", t);
}

METHOD_RETURN_TYPE InputFileDriver::NewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	SET_FUNCTION_RETURN_VALUE(instance)
}

v8::Handle<v8::Value> InputFileDriver::GetNewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

bool InputFileDriver::HasInstance(Handle<Value> inObject)
{
	CREATE_ISOLATE_CONTEXT;

	return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);
}

METHOD_RETURN_TYPE InputFileDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InputFileDriver* inputFile = new InputFileDriver();
    
    if(args.Length() == 1 && args[0]->IsString())
        inputFile->OpenFile(*String::Utf8Value(args[0]->ToString()));
    
    inputFile->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE InputFileDriver::OpenFile(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
 
    if(args.Length() != 1 || !args[0]->IsString())
    {
		THROW_EXCEPTION("wrong arguments. please provide a string for the file path");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }

    InputFileDriver* driver = ObjectWrap::Unwrap<InputFileDriver>(args.This());

    
    if(!driver)
    {
		THROW_EXCEPTION("no driver created...please create one through Hummus");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    if(driver->OpenFile(*String::Utf8Value(args[0]->ToString())) != PDFHummus::eSuccess)
    {
		THROW_EXCEPTION("can't open file. make sure path exists");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    SET_FUNCTION_RETURN_VALUE(UNDEFINED)
}

METHOD_RETURN_TYPE InputFileDriver::CloseFile(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InputFileDriver* driver = ObjectWrap::Unwrap<InputFileDriver>(args.This());
    
    if(!driver)
    {
		THROW_EXCEPTION("no driver created...please create one through Hummus");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    if(driver->mInputFileInstance)
        driver->mInputFileInstance->CloseFile();
    
    SET_FUNCTION_RETURN_VALUE(UNDEFINED)
}

METHOD_RETURN_TYPE InputFileDriver::GetFilePath(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InputFileDriver* driver = ObjectWrap::Unwrap<InputFileDriver>(args.This());
    
    if(!driver)
    {
		THROW_EXCEPTION("no driver created...please create one through Hummus");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    if(driver->mInputFileInstance && driver->mInputFileInstance->GetInputStream())
        SET_FUNCTION_RETURN_VALUE(NEW_STRING(driver->mInputFileInstance->GetFilePath().c_str()))
    else
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
}

METHOD_RETURN_TYPE InputFileDriver::GetFileSize(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InputFileDriver* driver = ObjectWrap::Unwrap<InputFileDriver>(args.This());
    
    if(!driver)
    {
		THROW_EXCEPTION("no driver created...please create one through Hummus");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    if(driver->mInputFileInstance && driver->mInputFileInstance->GetInputStream())
        SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(driver->mInputFileInstance->GetFileSize()))
    else
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)

}

METHOD_RETURN_TYPE InputFileDriver::GetInputStream(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    InputFileDriver* driver = ObjectWrap::Unwrap<InputFileDriver>(args.This());
    
    if(!driver)
    {
		THROW_EXCEPTION("no driver created...please create one through Hummus");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    if(driver->mInputFileInstance && driver->mInputFileInstance->GetInputStream())
    {
        Handle<Value> result = ByteReaderWithPositionDriver::GetNewInstance(args);
        
        ObjectWrap::Unwrap<ByteReaderWithPositionDriver>(result->ToObject())->SetStream(driver->mInputFileInstance->GetInputStream(), false);
        
        SET_FUNCTION_RETURN_VALUE(result)
    }
    else
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
}

