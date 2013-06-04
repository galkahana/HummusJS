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
    // prepare the page interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("InputFile"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("openFile"),FunctionTemplate::New(OpenFile)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("closeFile"),FunctionTemplate::New(CloseFile)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("getFilePath"),FunctionTemplate::New(GetFilePath)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("getFileSize"),FunctionTemplate::New(GetFileSize)->GetFunction());
    
    constructor = Persistent<Function>::New(constructor_template->GetFunction());
    inExports->Set(String::NewSymbol("InputFile"), constructor);
}

Handle<Value> InputFileDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    Local<Object> instance = constructor->NewInstance();
    
    return scope.Close(instance);
}

bool InputFileDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}

Handle<Value> InputFileDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    InputFileDriver* inputFile = new InputFileDriver();
    
    if(args.Length() == 1 && args[0]->IsString())
        inputFile->OpenFile(*String::Utf8Value(args[0]->ToString()));
    
    inputFile->Wrap(args.This());
    return args.This();
}

Handle<Value> InputFileDriver::OpenFile(const Arguments& args)
{
    HandleScope scope;
 
    if(args.Length() != 1 || !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments. please provide a string for the file path")));
        return scope.Close(Undefined());
    }

    InputFileDriver* driver = ObjectWrap::Unwrap<InputFileDriver>(args.This());

    
    if(!driver)
    {
		ThrowException(Exception::Error(String::New("no driver created...please create one through Hummus")));
        return scope.Close(Undefined());
        
    }
    
    if(!driver->OpenFile(*String::Utf8Value(args[0]->ToString())) != PDFHummus::eSuccess)
    {
		ThrowException(Exception::Error(String::New("can't open file. make sure path exists")));
        return scope.Close(Undefined());
    }
    
    return scope.Close(Undefined());
}

Handle<Value> InputFileDriver::CloseFile(const Arguments& args)
{
    HandleScope scope;
    
    InputFileDriver* driver = ObjectWrap::Unwrap<InputFileDriver>(args.This());
    
    if(!driver)
    {
		ThrowException(Exception::Error(String::New("no driver created...please create one through Hummus")));
        return scope.Close(Undefined());
        
    }
    
    if(driver->mInputFileInstance)
        driver->mInputFileInstance->CloseFile();
    
    return scope.Close(Undefined());
}

Handle<Value> InputFileDriver::GetFilePath(const Arguments& args)
{
    HandleScope scope;
    
    InputFileDriver* driver = ObjectWrap::Unwrap<InputFileDriver>(args.This());
    
    if(!driver)
    {
		ThrowException(Exception::Error(String::New("no driver created...please create one through Hummus")));
        return scope.Close(Undefined());
        
    }
    
    if(driver->mInputFileInstance && driver->mInputFileInstance->GetInputStream())
        return scope.Close(String::New(driver->mInputFileInstance->GetFilePath().c_str()));
    else
        return scope.Close(Undefined());
}

Handle<Value> InputFileDriver::GetFileSize(const Arguments& args)
{
    HandleScope scope;
    
    InputFileDriver* driver = ObjectWrap::Unwrap<InputFileDriver>(args.This());
    
    if(!driver)
    {
		ThrowException(Exception::Error(String::New("no driver created...please create one through Hummus")));
        return scope.Close(Undefined());
        
    }
    
    if(driver->mInputFileInstance && driver->mInputFileInstance->GetInputStream())
        return scope.Close(Number::New(driver->mInputFileInstance->GetFileSize()));
    else
        return scope.Close(Undefined());

}

