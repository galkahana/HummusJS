/*
 Source File : DictionaryContextDriver.cpp
 
 
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
#include "DictionaryContextDriver.h"
#include "DictionaryContext.h"
#include "PDFRectangle.h"

using namespace v8;

DictionaryContextDriver::~DictionaryContextDriver()
{
    delete DictionaryContextInstance;
}

void DictionaryContextDriver::Init()
{
    // prepare the form xobject driver interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->SetClassName(String::NewSymbol("DictionaryContext"));
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("writeKey"),FunctionTemplate::New(WriteKey)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("writeNameValue"),FunctionTemplate::New(WriteNameValue)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("writeRectangleValue"),FunctionTemplate::New(WriteRectangleValue)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("writeLiteralStringValue"),FunctionTemplate::New(WriteLiteralStringValue)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("writeBooleanValue"),FunctionTemplate::New(WriteBooleanValue)->GetFunction());
    constructor_template->PrototypeTemplate()->Set(String::NewSymbol("writeObjectReferenceValue"),FunctionTemplate::New(WriteObjectReferenceValue)->GetFunction());
    constructor = Persistent<Function>::New(constructor_template->GetFunction());
}

Handle<Value> DictionaryContextDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    Local<Object> instance = constructor->NewInstance();
    return scope.Close(instance);
}

bool DictionaryContextDriver::HasInstance(Handle<Value> inObject)
{
    return inObject->IsObject() &&
    constructor_template->HasInstance(inObject->ToObject());
}

DictionaryContextDriver::DictionaryContextDriver()
{
    DictionaryContextInstance = NULL;
}

Persistent<Function> DictionaryContextDriver::constructor;
Persistent<FunctionTemplate> DictionaryContextDriver::constructor_template;

Handle<v8::Value> DictionaryContextDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    DictionaryContextDriver* driver = new DictionaryContextDriver();
    driver->Wrap(args.This());
    
    return args.This();
}

Handle<Value> DictionaryContextDriver::WriteKey(const Arguments& args)
{
    HandleScope scope;
    
    if(!args.Length() == 1 ||
       !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide a string to write")));
        return scope.Close(Undefined());
        
    }
    
    DictionaryContextDriver* driver = ObjectWrap::Unwrap<DictionaryContextDriver>(args.This());
    
    if(!driver->DictionaryContextInstance)
    {
		ThrowException(Exception::TypeError(String::New("dictinoarycontext object not initialized, create using objectscontext.startDictionary")));
        return scope.Close(Undefined());
    }
    driver->DictionaryContextInstance->WriteKey(*String::Utf8Value(args[0]->ToString()));
    
    return scope.Close(args.This());
}

Handle<Value> DictionaryContextDriver::WriteRectangleValue(const Arguments& args)
{
    HandleScope scope;
    
    // can accept array or four numbers
    if( (args.Length() != 1 && args.Length() != 4) ||
       (args.Length() == 1 && !args[0]->IsArray()) ||
       (args.Length() == 4 && (!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber())))
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide an array of 4 numbers, or 4 numbers")));
        return scope.Close(Undefined());
        
    }
    
    DictionaryContextDriver* driver = ObjectWrap::Unwrap<DictionaryContextDriver>(args.This());
    
    if(!driver->DictionaryContextInstance)
    {
		ThrowException(Exception::TypeError(String::New("dictinoarycontext object not initialized, create using objectscontext.startDictionary")));
        return scope.Close(Undefined());
    }
    
    if(args.Length() == 1)
    {
        // array version. verify that there are 4 numbers
        if(args[0]->ToObject()->Get(v8::String::New("length"))->ToObject()->Uint32Value() != 4)
        {
            ThrowException(Exception::TypeError(String::New("Wrong arguments, provide an array of 4 numbers, or 4 numbers")));
            return scope.Close(Undefined());
        }
        
        driver->DictionaryContextInstance->WriteRectangleValue(PDFRectangle(
                                                      args[0]->ToObject()->Get(0)->ToNumber()->Value(),
                                                      args[0]->ToObject()->Get(1)->ToNumber()->Value(),
                                                      args[0]->ToObject()->Get(2)->ToNumber()->Value(),
                                                      args[0]->ToObject()->Get(3)->ToNumber()->Value()));
        
    }
    else
    {
        // 4 numbers version
        driver->DictionaryContextInstance->WriteRectangleValue(PDFRectangle(
                                                                            args[0]->ToNumber()->Value(),
                                                                            args[1]->ToNumber()->Value(),
                                                                            args[2]->ToNumber()->Value(),
                                                                            args[3]->ToNumber()->Value()));
    }
    
    
    return scope.Close(args.This());
}

Handle<Value> DictionaryContextDriver::WriteNameValue(const Arguments& args)
{
    HandleScope scope;
    
    if(!args.Length() == 1 ||
       !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide a string to write")));
        return scope.Close(Undefined());
        
    }
    
    DictionaryContextDriver* driver = ObjectWrap::Unwrap<DictionaryContextDriver>(args.This());
    
    if(!driver->DictionaryContextInstance)
    {
		ThrowException(Exception::TypeError(String::New("dictinoarycontext object not initialized, create using objectscontext.startDictionary")));
        return scope.Close(Undefined());
    }
    
    driver->DictionaryContextInstance->WriteNameValue(*String::Utf8Value(args[0]->ToString()));
    
    return scope.Close(args.This());
}

Handle<Value> DictionaryContextDriver::WriteLiteralStringValue(const Arguments& args)
{
    HandleScope scope;
    
    if(!args.Length() == 1 ||
       (!args[0]->IsString() && !args[0]->IsArray()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is a literal string (string) or an array")));
		return scope.Close(Undefined());
        
    }

    DictionaryContextDriver* driver = ObjectWrap::Unwrap<DictionaryContextDriver>(args.This());
    
    if(!driver->DictionaryContextInstance)
    {
		ThrowException(Exception::TypeError(String::New("dictinoarycontext object not initialized, create using objectscontext.startDictionary")));
        return scope.Close(Undefined());
    }

	if(args[0]->IsArray())
	{
		std::string string;
		unsigned long arrayLength = (args[0]->ToObject()->Get(v8::String::New("length")))->ToObject()->Uint32Value();
		for(unsigned long i=0;i<arrayLength;++i)
			string.push_back((unsigned char)args[0]->ToObject()->Get(i)->ToNumber()->Value());
		driver->DictionaryContextInstance->WriteLiteralStringValue(string);
	}
	else
    {
		driver->DictionaryContextInstance->WriteLiteralStringValue(*String::Utf8Value(args[0]->ToString()));
	}
    return scope.Close(args.This());
}

Handle<Value> DictionaryContextDriver::WriteBooleanValue(const Arguments& args)
{
    HandleScope scope;
    
    if(!args.Length() == 1 ||
       !args[0]->IsBoolean())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide a boolean to write")));
        return scope.Close(Undefined());
        
    }
    DictionaryContextDriver* driver = ObjectWrap::Unwrap<DictionaryContextDriver>(args.This());
    
    if(!driver->DictionaryContextInstance)
    {
		ThrowException(Exception::TypeError(String::New("dictinoarycontext object not initialized, create using objectscontext.startDictionary")));
        return scope.Close(Undefined());
    }
    
    driver->DictionaryContextInstance->WriteBooleanValue(args[0]->ToBoolean()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> DictionaryContextDriver::WriteObjectReferenceValue(const Arguments& args)
{
    HandleScope scope;
    
    if(!args.Length() == 1 ||
       !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments, provide an object id to write")));
        return scope.Close(Undefined());
        
    }
    DictionaryContextDriver* driver = ObjectWrap::Unwrap<DictionaryContextDriver>(args.This());
    
    if(!driver->DictionaryContextInstance)
    {
		ThrowException(Exception::TypeError(String::New("dictinoarycontext object not initialized, create using objectscontext.startDictionary")));
        return scope.Close(Undefined());
    }
    
    driver->DictionaryContextInstance->WriteObjectReferenceValue((ObjectIDType)args[0]->ToNumber()->Uint32Value());
    
    return scope.Close(args.This());
}