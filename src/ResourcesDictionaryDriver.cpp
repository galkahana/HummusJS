/*
 Source File : ResourcesDictionaryDriver.cpp
 
 
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

#include "ResourcesDictionaryDriver.h"
#include "ResourcesDictionary.h"

using namespace v8;


static void Init();

void ResourcesDictionaryDriver::Init()
{
    // prepare the form xobject driver interfrace template
    Local<FunctionTemplate> ft = FunctionTemplate::New(New);
    ft->SetClassName(String::NewSymbol("ResourcesDictionary"));
    ft->InstanceTemplate()->SetInternalFieldCount(1);
    
    ft->PrototypeTemplate()->Set(String::NewSymbol("addFormXObjectMapping"),FunctionTemplate::New(AddFormXObjectMapping)->GetFunction());
    constructor = Persistent<Function>::New(ft->GetFunction());
}

Handle<Value> ResourcesDictionaryDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    const unsigned argc = 0;
    Handle<Value> argv[0];
    Local<Object> instance = constructor->NewInstance(argc, argv);
    return scope.Close(instance);
}

ResourcesDictionaryDriver::ResourcesDictionaryDriver()
{
    ResourcesDictionaryInstance = NULL;
}

Persistent<Function> ResourcesDictionaryDriver::constructor;

Handle<Value> ResourcesDictionaryDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    ResourcesDictionaryDriver* form = new ResourcesDictionaryDriver();
    form->Wrap(args.This());
    
    return args.This();
}

Handle<Value> ResourcesDictionaryDriver::AddFormXObjectMapping(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument which is the form xobject id")));
		return scope.Close(Undefined());
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    Local<String> name = String::New(
                                     resourcesDictionaryDriver->ResourcesDictionaryInstance->AddFormXObjectMapping(
                                                                                                        (ObjectIDType)(args[0]->ToUint32()->Value())).c_str());
    
    return scope.Close(name);
    
}


