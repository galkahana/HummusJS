/*
 Source File : InfoDictinaryDriver.cpp
 
 
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
#include "InfoDictionaryDriver.h"
#include "InfoDictionary.h"
#include "PDFDateDriver.h"
#include "PDFDate.h"
#include "PDFTextString.h"

using namespace v8;

Persistent<Function> InfoDictionaryDriver::constructor;

InfoDictionaryDriver::InfoDictionaryDriver()
{
    InfoDictionaryInstance = NULL;
}

void InfoDictionaryDriver::Init()
{
    // prepare the page interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->SetClassName(String::NewSymbol("InfoDictionary"));
    t->InstanceTemplate()->SetInternalFieldCount(1);
    t->PrototypeTemplate()->Set(String::NewSymbol("addAdditionalInfoEntry"),FunctionTemplate::New(AddAdditionalInfoEntry)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("removeAdditionalInfoEntry"),FunctionTemplate::New(RemoveAdditionalInfoEntry)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("clearAdditionalInfoEntries"),FunctionTemplate::New(ClearAdditionalInfoEntries)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("getAdditionalInfoEntry"),FunctionTemplate::New(GetAdditionalInfoEntry)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("getAdditionalInfoEntries"),FunctionTemplate::New(GetAdditionalInfoEntries)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("setCreationDate"),FunctionTemplate::New(SetCreationDate)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("setModDate"),FunctionTemplate::New(SetModDate)->GetFunction());
    t->InstanceTemplate()->SetAccessor(String::NewSymbol("title"),GetTitle,SetTitle);
    t->InstanceTemplate()->SetAccessor(String::NewSymbol("author"),GetAuthor,SetAuthor);
    t->InstanceTemplate()->SetAccessor(String::NewSymbol("subject"),GetSubject,SetSubject);
    t->InstanceTemplate()->SetAccessor(String::NewSymbol("keywords"),GetKeywords,SetKeywords);
    t->InstanceTemplate()->SetAccessor(String::NewSymbol("creator"),GetCreator,SetCreator);
    t->InstanceTemplate()->SetAccessor(String::NewSymbol("producer"),GetProducer,SetProducer);
    t->InstanceTemplate()->SetAccessor(String::NewSymbol("trapped"),GetTrapped,SetTrapped);
    
    constructor = Persistent<Function>::New(t->GetFunction());
}

Handle<Value> InfoDictionaryDriver::NewInstance()
{
    HandleScope scope;
    
    Local<Object> instance = constructor->NewInstance();
    return scope.Close(instance);
}


Handle<Value> InfoDictionaryDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    InfoDictionaryDriver* driver = new InfoDictionaryDriver();
    driver->Wrap(args.This());
    return args.This();
}

Handle<Value> InfoDictionaryDriver::GetTitle(Local<String> property,const AccessorInfo &info)
{
    HandleScope scope;
    
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
    {
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
        return scope.Close(Undefined());
    }

    return scope.Close(String::New(infoDictionaryDriver->InfoDictionaryInstance->Title.ToUTF8String().c_str()));
}

void InfoDictionaryDriver::SetTitle(Local<String> property,Local<Value> value,const AccessorInfo &info)
{
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
    
    infoDictionaryDriver->InfoDictionaryInstance->Title.FromUTF8(*String::Utf8Value(value->ToString()));
}

Handle<Value> InfoDictionaryDriver::GetAuthor(Local<String> property,const AccessorInfo &info)
{
    HandleScope scope;
    
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
    {
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
        return scope.Close(Undefined());
    }
    
    return scope.Close(String::New(infoDictionaryDriver->InfoDictionaryInstance->Author.ToUTF8String().c_str()));
}

void InfoDictionaryDriver::SetAuthor(Local<String> property,Local<Value> value,const AccessorInfo &info)
{
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
    
    infoDictionaryDriver->InfoDictionaryInstance->Author.FromUTF8(*String::Utf8Value(value->ToString()));
}

Handle<Value> InfoDictionaryDriver::GetSubject(Local<String> property,const AccessorInfo &info)
{
    HandleScope scope;
    
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
    {
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
        return scope.Close(Undefined());
    }
    
    return scope.Close(String::New(infoDictionaryDriver->InfoDictionaryInstance->Subject.ToUTF8String().c_str()));
}

void InfoDictionaryDriver::SetSubject(Local<String> property,Local<Value> value,const AccessorInfo &info)
{
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
    
    infoDictionaryDriver->InfoDictionaryInstance->Subject.FromUTF8(*String::Utf8Value(value->ToString()));
}

Handle<Value> InfoDictionaryDriver::GetKeywords(Local<String> property,const AccessorInfo &info)
{
    HandleScope scope;
    
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
    {
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
        return scope.Close(Undefined());
    }
    
    return scope.Close(String::New(infoDictionaryDriver->InfoDictionaryInstance->Keywords.ToUTF8String().c_str()));
}

void InfoDictionaryDriver::SetKeywords(Local<String> property,Local<Value> value,const AccessorInfo &info)
{
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
    
    infoDictionaryDriver->InfoDictionaryInstance->Keywords.FromUTF8(*String::Utf8Value(value->ToString()));
}

Handle<Value> InfoDictionaryDriver::GetCreator(Local<String> property,const AccessorInfo &info)
{
    HandleScope scope;
    
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
    {
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
        return scope.Close(Undefined());
    }
    
    return scope.Close(String::New(infoDictionaryDriver->InfoDictionaryInstance->Creator.ToUTF8String().c_str()));
}

void InfoDictionaryDriver::SetCreator(Local<String> property,Local<Value> value,const AccessorInfo &info)
{
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
    
    infoDictionaryDriver->InfoDictionaryInstance->Creator.FromUTF8(*String::Utf8Value(value->ToString()));
}

Handle<Value> InfoDictionaryDriver::GetProducer(Local<String> property,const AccessorInfo &info)
{
    HandleScope scope;
    
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
    {
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
        return scope.Close(Undefined());
    }
    
    return scope.Close(String::New(infoDictionaryDriver->InfoDictionaryInstance->Producer.ToUTF8String().c_str()));
}

void InfoDictionaryDriver::SetProducer(Local<String> property,Local<Value> value,const AccessorInfo &info)
{
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
    
    infoDictionaryDriver->InfoDictionaryInstance->Producer.FromUTF8(*String::Utf8Value(value->ToString()));
}

Handle<Value> InfoDictionaryDriver::GetTrapped(Local<String> property,const AccessorInfo &info)
{
    HandleScope scope;
    
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
    {
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
        return scope.Close(Undefined());
    }
    
    return scope.Close(Number::New(infoDictionaryDriver->InfoDictionaryInstance->Trapped));
}

void InfoDictionaryDriver::SetTrapped(Local<String> property,Local<Value> value,const AccessorInfo &info)
{
    InfoDictionaryDriver* infoDictionaryDriver = ObjectWrap::Unwrap<InfoDictionaryDriver>(info.Holder());
    
    if(!infoDictionaryDriver->InfoDictionaryInstance)
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
    
    infoDictionaryDriver->InfoDictionaryInstance->Trapped = (EInfoTrapped)(value->ToNumber()->Uint32Value());
}

Handle<Value> InfoDictionaryDriver::SetCreationDate(const Arguments& args)
{
    HandleScope scope;
    
    InfoDictionaryDriver* driver = ObjectWrap::Unwrap<InfoDictionaryDriver>(args.This());
    if(!driver->InfoDictionaryInstance)
    {
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
        return scope.Close(Undefined());
    }

    
    Handle<Value> dataDriverValue = PDFDateDriver::NewInstance(args);
    PDFDateDriver* dateDriver = ObjectWrap::Unwrap<PDFDateDriver>(dataDriverValue->ToObject());

    driver->InfoDictionaryInstance->CreationDate =  *(dateDriver->getInstance());
    
    return scope.Close(Undefined());
}

Handle<Value> InfoDictionaryDriver::SetModDate(const Arguments& args)
{
    HandleScope scope;
    
    InfoDictionaryDriver* driver = ObjectWrap::Unwrap<InfoDictionaryDriver>(args.This());
    if(!driver->InfoDictionaryInstance)
    {
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
        return scope.Close(Undefined());
    }
    
    
    Handle<Value> dataDriverValue = PDFDateDriver::NewInstance(args);
    PDFDateDriver* dateDriver = ObjectWrap::Unwrap<PDFDateDriver>(dataDriverValue->ToObject());
    
    driver->InfoDictionaryInstance->ModDate =  *(dateDriver->getInstance());
    
    return scope.Close(Undefined());
}

Handle<Value> InfoDictionaryDriver::AddAdditionalInfoEntry(const Arguments& args)
{
    HandleScope scope;
    
    InfoDictionaryDriver* driver = ObjectWrap::Unwrap<InfoDictionaryDriver>(args.This());
    if(!driver->InfoDictionaryInstance)
    {
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
        return scope.Close(Undefined());
    }
    
    if(args.Length() != 2 ||
       !args[0]->IsString() ||
       !args[1]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments. please provide two strings - key and value ")));
        return scope.Close(Undefined());
    }
    
    PDFTextString textString;
    textString.FromUTF8(*String::Utf8Value(args[1]->ToString()));
    
    driver->InfoDictionaryInstance->AddAdditionalInfoEntry(*String::Utf8Value(args[0]->ToString()), textString);
    
    return scope.Close(Undefined());
}

Handle<Value> InfoDictionaryDriver::RemoveAdditionalInfoEntry(const Arguments& args)
{
    HandleScope scope;
    
    InfoDictionaryDriver* driver = ObjectWrap::Unwrap<InfoDictionaryDriver>(args.This());
    if(!driver->InfoDictionaryInstance)
    {
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
        return scope.Close(Undefined());
    }
    
    if(args.Length() != 1 ||
       !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments. please provide key of the entry to remove ")));
        return scope.Close(Undefined());
    }
    
    driver->InfoDictionaryInstance->RemoveAdditionalInfoEntry(*String::Utf8Value(args[0]->ToString()));
    
    return scope.Close(Undefined());
}

Handle<Value> InfoDictionaryDriver::ClearAdditionalInfoEntries(const Arguments& args)
{
    HandleScope scope;
    
    InfoDictionaryDriver* driver = ObjectWrap::Unwrap<InfoDictionaryDriver>(args.This());
    if(!driver->InfoDictionaryInstance)
    {
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
        return scope.Close(Undefined());
    }
    
    driver->InfoDictionaryInstance->ClearAdditionalInfoEntries();
    
    return scope.Close(Undefined());
}

Handle<Value> InfoDictionaryDriver::GetAdditionalInfoEntry(const Arguments& args)
{
    HandleScope scope;
    
    InfoDictionaryDriver* driver = ObjectWrap::Unwrap<InfoDictionaryDriver>(args.This());
    if(!driver->InfoDictionaryInstance)
    {
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
        return scope.Close(Undefined());
    }
    
    if(args.Length() != 1 ||
       !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments. please provide key of the entry to return ")));
        return scope.Close(Undefined());
    }
    
    PDFTextString testString = driver->InfoDictionaryInstance->GetAdditionalInfoEntry(*String::Utf8Value(args[0]->ToString()));
    
    return scope.Close(String::New(testString.ToUTF8String().c_str()));
}

Handle<Value> InfoDictionaryDriver::GetAdditionalInfoEntries(const Arguments& args)
{
    HandleScope scope;
    
    InfoDictionaryDriver* driver = ObjectWrap::Unwrap<InfoDictionaryDriver>(args.This());
    if(!driver->InfoDictionaryInstance)
    {
		ThrowException(Exception::Error(String::New("info dictionary driver not initialized. use the document context object to get a valid info dictionary")));
        return scope.Close(Undefined());
    }
    
    if(args.Length() != 1 ||
       !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments. please provide key of the entry to return ")));
        return scope.Close(Undefined());
    }
    
    Handle<Object> anObject = Object::New();
   
    MapIterator<StringToPDFTextString> it = driver->InfoDictionaryInstance->GetAdditionaEntriesIterator();

    while(it.MoveNext())
        anObject->Set(String::New(it.GetKey().c_str()),String::New(it.GetValue().ToUTF8String().c_str()));
    
    return scope.Close(anObject);
}


