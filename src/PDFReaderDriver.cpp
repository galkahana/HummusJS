/*
 Source File : PDFWriterDriver.cpp
 
 
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
#include "PDFReaderDriver.h"
#include "PDFDictionaryDriver.h"
#include "PDFObjectDriver.h"
#include "RefCountPtr.h"
#include "PDFDictionary.h"
#include "PDFArrayDriver.h"
#include "PDFArray.h"




using namespace v8;

Persistent<Function> PDFReaderDriver::constructor;

PDFReaderDriver::PDFReaderDriver()
{
    mPDFReader = NULL;
    mOwnsParser = false;
}

PDFReaderDriver::~PDFReaderDriver()
{
    if(mOwnsParser)
        delete mPDFReader;
}

void PDFReaderDriver::Init()
{
    // prepare the pdfwriter interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->SetClassName(String::NewSymbol("PDFReader"));
    t->InstanceTemplate()->SetInternalFieldCount(1);
    
    t->PrototypeTemplate()->Set(String::NewSymbol("getPDFLevel"),FunctionTemplate::New(GetPDFLevel)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("getPagesCount"),FunctionTemplate::New(GetPagesCount)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("getTrailer"),FunctionTemplate::New(GetTrailer)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("queryDictionaryObject"),FunctionTemplate::New(QueryDictionaryObject)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("queryArrayObject"),FunctionTemplate::New(QueryArrayObject)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("parseNewObject"),FunctionTemplate::New(ParseNewObject)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("getPageObjectID"),FunctionTemplate::New(GetPageObjectID)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("parsePage"),FunctionTemplate::New(ParsePage)->GetFunction());
    constructor = Persistent<Function>::New(t->GetFunction());
}

Handle<Value> PDFReaderDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() == 1)
    {
        const unsigned argc = 1;
        Handle<Value> argv[argc] = { args[0] };
        Local<Object> instance = constructor->NewInstance(argc, argv);
        return scope.Close(instance);
    }
    else
    {
        const unsigned argc = 0;
        Handle<Value> argv[0];
        Local<Object> instance = constructor->NewInstance(argc, argv);
        return scope.Close(instance);
    }
}


Handle<Value> PDFReaderDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    PDFReaderDriver* reader = new PDFReaderDriver();
    reader->Wrap(args.This());
    
    return args.This();
}

Handle<Value> PDFReaderDriver::GetPDFLevel(const Arguments& args)
{
    return Number::New(ObjectWrap::Unwrap<PDFReaderDriver>(args.This())->mPDFReader->GetPDFLevel());
}

Handle<Value> PDFReaderDriver::GetPagesCount(const Arguments& args)
{
    return Number::New(ObjectWrap::Unwrap<PDFReaderDriver>(args.This())->mPDFReader->GetPagesCount());
}


Handle<Value> PDFReaderDriver::QueryDictionaryObject(const Arguments& args)
{
    HandleScope scope;
    if(args.Length() != 2 ||
       !PDFDictionaryDriver::HasInstance(args[0]) ||
       !args[1]->IsString())
    {
 		ThrowException(Exception::TypeError(String::New("Wrong arguments. Provide a dictionary and a string")));
        return scope.Close(Undefined());
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    PDFDictionaryDriver* dictionary = ObjectWrap::Unwrap<PDFDictionaryDriver>(args[0]->ToObject());
    
    RefCountPtr<PDFObject> object = reader->mPDFReader->QueryDictionaryObject(dictionary->TheObject.GetPtr(),*String::Utf8Value(args[1]->ToString()));
    if(!object)
        return scope.Close(Undefined());
        
    return scope.Close(PDFObjectDriver::CreateDriver(object.GetPtr()));
}

Handle<Value> PDFReaderDriver::QueryArrayObject(const Arguments& args)
{
    HandleScope scope;
    if(args.Length() != 2 ||
       !PDFArrayDriver::HasInstance(args[0]) ||
       !args[1]->IsNumber())
    {
 		ThrowException(Exception::TypeError(String::New("Wrong arguments. Provide an array and an index")));
        return scope.Close(Undefined());
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    PDFArrayDriver* driver = ObjectWrap::Unwrap<PDFArrayDriver>(args[0]->ToObject());
    
    RefCountPtr<PDFObject> object = reader->mPDFReader->QueryArrayObject(driver->TheObject.GetPtr(),args[1]->ToNumber()->Uint32Value());
    if(!object)
        return scope.Close(Undefined());
    
    return scope.Close(PDFObjectDriver::CreateDriver(object.GetPtr()));
}

Handle<Value> PDFReaderDriver::GetTrailer(const Arguments& args)
{
    HandleScope scope;
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
 
    PDFDictionary* trailer = reader->mPDFReader->GetTrailer();
    
    if(!trailer)
        return scope.Close(Undefined());
    
    return scope.Close(PDFObjectDriver::CreateDriver(trailer));
}

PDFHummus::EStatusCode PDFReaderDriver::StartPDFParsing(const std::string& inParsedFilePath)
{
    if(!mPDFReader && !mOwnsParser)
    {
        mPDFReader = new PDFParser();
        mOwnsParser = true;
    }
    
    mPDFReader->ResetParser();
    if(mPDFFile.OpenFile(inParsedFilePath) != PDFHummus::eSuccess)
        return PDFHummus::eFailure;
    return mPDFReader->StartPDFParsing(mPDFFile.GetInputStream());
}

void PDFReaderDriver::SetFromOwnedParser(PDFParser* inParser)
{
    if(mOwnsParser)
    {
        delete mPDFReader;
        mOwnsParser = false;
        mPDFFile.CloseFile();
    }
    mPDFReader = inParser;
}

Handle<Value> PDFReaderDriver::ParseNewObject(const Arguments& args)
{
    HandleScope scope;

    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
 		ThrowException(Exception::TypeError(String::New("Wrong arguments. Provide an Object ID")));
        return scope.Close(Undefined());
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    
    RefCountPtr<PDFObject> newObject = reader->mPDFReader->ParseNewObject(args[0]->ToNumber()->Uint32Value());
    
    if(!newObject)
    {
 		ThrowException(Exception::TypeError(String::New("Unable to read object. Most probably object ID is wrong (or some file read issue...but i'd first check that ID. if i were you)")));
        return scope.Close(Undefined());
    }
    
    return scope.Close(PDFObjectDriver::CreateDriver(newObject.GetPtr()));
}

Handle<Value> PDFReaderDriver::GetPageObjectID(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
 		ThrowException(Exception::TypeError(String::New("Wrong arguments. Provide a page index")));
        return scope.Close(Undefined());
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    
    return scope.Close(Number::New(reader->mPDFReader->GetPageObjectID(args[0]->ToNumber()->Uint32Value())));
}


Handle<Value> PDFReaderDriver::ParsePage(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
 		ThrowException(Exception::TypeError(String::New("Wrong arguments. Provide a page index")));
        return scope.Close(Undefined());
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    
    RefCountPtr<PDFDictionary> newObject = reader->mPDFReader->ParsePage(args[0]->ToNumber()->Uint32Value());
    
    if(!newObject)
    {
 		ThrowException(Exception::TypeError(String::New("Unable to read page, parhaps page index is wrong")));
        return scope.Close(Undefined());
    }
    
    return scope.Close(PDFObjectDriver::CreateDriver(newObject.GetPtr()));
}
