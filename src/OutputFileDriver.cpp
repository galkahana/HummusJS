/*
 Source File : OutputFileDriver.h
 
 
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
#include "OutputFileDriver.h"
#include "OutputFile.h"
#include "ByteWriterWithPositionDriver.h"
#include "ConstructorsHolder.h"


using namespace v8;



OutputFileDriver::OutputFileDriver()
{
    mOutputFileInstance = new OutputFile();
    mOwnsInstance = true;
}

OutputFileDriver::~OutputFileDriver()
{
    if(mOwnsInstance)
        delete mOutputFileInstance;
}

PDFHummus::EStatusCode OutputFileDriver::OpenFile(const std::string& inFilePath,bool inAppend)
{
    if(!mOutputFileInstance)
        mOutputFileInstance = new OutputFile();
    mOwnsInstance = true;
    return mOutputFileInstance->OpenFile(inFilePath,inAppend);
}

void OutputFileDriver::SetFromOwnedFile(OutputFile* inFile)
{
    if(mOutputFileInstance && mOwnsInstance)
        delete mOutputFileInstance;
    mOwnsInstance = false;
    mOutputFileInstance = inFile;
}

DEF_SUBORDINATE_INIT(OutputFileDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE_EXTERNAL(New);

	t->SetClassName(NEW_STRING("OutputFile"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "openFile", OpenFile);
	SET_PROTOTYPE_METHOD(t, "closeFile", CloseFile);
	SET_PROTOTYPE_METHOD(t, "getFilePath", GetFilePath);
	SET_PROTOTYPE_METHOD(t, "getOutputStream", GetOutputStream);

	SET_CONSTRUCTOR_EXPORT("OutputFile",t);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->OutputFile_constructor, t);        
	SET_CONSTRUCTOR_TEMPLATE(holder->OutputFile_constructor_template, t);
}


METHOD_RETURN_TYPE OutputFileDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, externalHolder)
    
    OutputFileDriver* outputFile = new OutputFileDriver();
    bool append = false;
    if((args.Length() == 1 ||  args.Length() == 2) &&
        args[0]->IsString())
       
    {
        if(args.Length() == 2 && args[1]->IsBoolean())
            append = args[1]->TO_BOOLEAN()->Value();
        outputFile->OpenFile(*UTF_8_VALUE(args[0]->TO_STRING()),append);
    }

    outputFile->holder = externalHolder; 
    
    outputFile->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE OutputFileDriver::OpenFile(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if((args.Length() != 1  && args.Length() != 2) ||
       !args[0]->IsString() ||
       (args.Length() == 2 && !args[1]->IsBoolean()))
    {
		THROW_EXCEPTION("wrong arguments. please provide a string for the file path and optional boolean flag to determine whether this file is opened for appending");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    OutputFileDriver* driver = ObjectWrap::Unwrap<OutputFileDriver>(args.This());
    
    
    if(!driver)
    {
		THROW_EXCEPTION("no driver created...please create one through Hummus");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    if(driver->OpenFile(*UTF_8_VALUE(args[0]->TO_STRING()),args.Length() == 2 ? args[1]->TO_BOOLEAN()->Value() : false) != PDFHummus::eSuccess)
    {
		THROW_EXCEPTION("can't open file. make sure path is not busy");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    SET_FUNCTION_RETURN_VALUE(UNDEFINED)
}

METHOD_RETURN_TYPE OutputFileDriver::CloseFile(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    OutputFileDriver* driver = ObjectWrap::Unwrap<OutputFileDriver>(args.This());
    
    if(!driver)
    {
		THROW_EXCEPTION("no driver created...please create one through Hummus");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    if(driver->mOutputFileInstance)
        driver->mOutputFileInstance->CloseFile();
    
    SET_FUNCTION_RETURN_VALUE(UNDEFINED)
}

METHOD_RETURN_TYPE OutputFileDriver::GetFilePath(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    OutputFileDriver* driver = ObjectWrap::Unwrap<OutputFileDriver>(args.This());
    
    if(!driver)
    {
		THROW_EXCEPTION("no driver created...please create one through Hummus");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    if(driver->mOutputFileInstance && driver->mOutputFileInstance->GetOutputStream())
        SET_FUNCTION_RETURN_VALUE(NEW_STRING(driver->mOutputFileInstance->GetFilePath().c_str()))
    else
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
}

METHOD_RETURN_TYPE OutputFileDriver::GetOutputStream(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    OutputFileDriver* driver = ObjectWrap::Unwrap<OutputFileDriver>(args.This());
    
    if(!driver)
    {
		THROW_EXCEPTION("no driver created...please create one through Hummus");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    if(driver->mOutputFileInstance && driver->mOutputFileInstance->GetOutputStream())
    {
        Local<Value> result = driver->holder->GetNewByteWriterWithPosition(args);
        
        ObjectWrap::Unwrap<ByteWriterWithPositionDriver>(result->TO_OBJECT())->SetStream(driver->mOutputFileInstance->GetOutputStream(),false);
        
        SET_FUNCTION_RETURN_VALUE(result)
    }
    else
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
}

