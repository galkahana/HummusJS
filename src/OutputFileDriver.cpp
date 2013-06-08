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


using namespace v8;

Persistent<Function> OutputFileDriver::constructor;
Persistent<FunctionTemplate> OutputFileDriver::constructor_template;

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

void OutputFileDriver::Init(Handle<Object> inExports)
{
    // prepare the page interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("OutputFile"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("openFile"),FunctionTemplate::New(OpenFile)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("closeFile"),FunctionTemplate::New(CloseFile)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("getFilePath"),FunctionTemplate::New(GetFilePath)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("getOutputStream"),FunctionTemplate::New(GetOutputStream)->GetFunction());
    
    constructor = Persistent<Function>::New(constructor_template->GetFunction());
    inExports->Set(String::NewSymbol("OutputFile"), constructor);
}

Handle<Value> OutputFileDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    Local<Object> instance = constructor->NewInstance();
    
    return scope.Close(instance);
}

bool OutputFileDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}

Handle<Value> OutputFileDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    OutputFileDriver* outputFile = new OutputFileDriver();
    bool append = false;
    if((args.Length() == 1 ||  args.Length() == 2) &&
        args[0]->IsString())
       
    {
        if(args.Length() == 2 && args[1]->IsBoolean())
            append = args[1]->ToBoolean()->Value();
        outputFile->OpenFile(*String::Utf8Value(args[0]->ToString()),append);
    }
    
    outputFile->Wrap(args.This());
    return args.This();
}

Handle<Value> OutputFileDriver::OpenFile(const Arguments& args)
{
    HandleScope scope;
    
    if((args.Length() != 1  && args.Length() != 2) ||
       !args[0]->IsString() ||
       (args.Length() == 2 && !args[1]->IsBoolean()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments. please provide a string for the file path and optional boolean flag to determine whether this file is opened for appending")));
        return scope.Close(Undefined());
    }
    
    OutputFileDriver* driver = ObjectWrap::Unwrap<OutputFileDriver>(args.This());
    
    
    if(!driver)
    {
		ThrowException(Exception::Error(String::New("no driver created...please create one through Hummus")));
        return scope.Close(Undefined());
        
    }
    
    if(!driver->OpenFile(*String::Utf8Value(args[0]->ToString()),args.Length() == 2 ? args[1]->ToBoolean()->Value() : false) != PDFHummus::eSuccess)
    {
		ThrowException(Exception::Error(String::New("can't open file. make sure path is not busy")));
        return scope.Close(Undefined());
    }
    
    return scope.Close(Undefined());
}

Handle<Value> OutputFileDriver::CloseFile(const Arguments& args)
{
    HandleScope scope;
    
    OutputFileDriver* driver = ObjectWrap::Unwrap<OutputFileDriver>(args.This());
    
    if(!driver)
    {
		ThrowException(Exception::Error(String::New("no driver created...please create one through Hummus")));
        return scope.Close(Undefined());
        
    }
    
    if(driver->mOutputFileInstance)
        driver->mOutputFileInstance->CloseFile();
    
    return scope.Close(Undefined());
}

Handle<Value> OutputFileDriver::GetFilePath(const Arguments& args)
{
    HandleScope scope;
    
    OutputFileDriver* driver = ObjectWrap::Unwrap<OutputFileDriver>(args.This());
    
    if(!driver)
    {
		ThrowException(Exception::Error(String::New("no driver created...please create one through Hummus")));
        return scope.Close(Undefined());
        
    }
    
    if(driver->mOutputFileInstance && driver->mOutputFileInstance->GetOutputStream())
        return scope.Close(String::New(driver->mOutputFileInstance->GetFilePath().c_str()));
    else
        return scope.Close(Undefined());
}

Handle<Value> OutputFileDriver::GetOutputStream(const Arguments& args)
{
    HandleScope scope;
    
    OutputFileDriver* driver = ObjectWrap::Unwrap<OutputFileDriver>(args.This());
    
    if(!driver)
    {
		ThrowException(Exception::Error(String::New("no driver created...please create one through Hummus")));
        return scope.Close(Undefined());
        
    }
    
    if(driver->mOutputFileInstance && driver->mOutputFileInstance->GetOutputStream())
    {
        Handle<Value> result = ByteWriterWithPositionDriver::NewInstance(args);
        
        ObjectWrap::Unwrap<ByteWriterWithPositionDriver>(result->ToObject())->SetStream(driver->mOutputFileInstance->GetOutputStream(),false);
        
        return scope.Close(result);
    }
    else
        return scope.Close(Undefined());
}

