/*
 Source File : ObjectsContextDriver.cpp
 
 
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
#include "ObjectsContextDriver.h"
#include "ObjectsContext.h"
#include "IndirectObjectsReferenceRegistry.h"

using namespace v8;


void ObjectsContextDriver::Init()
{
    // prepare the form xobject driver interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->SetClassName(String::NewSymbol("ObjectsContext"));
    t->InstanceTemplate()->SetInternalFieldCount(1);
    
    t->PrototypeTemplate()->Set(String::NewSymbol("allocateNewObjectID"),FunctionTemplate::New(AllocateNewObjectID)->GetFunction());
    constructor = Persistent<Function>::New(t->GetFunction());
}

Handle<Value> ObjectsContextDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    const unsigned argc = 0;
    Handle<Value> argv[0];
    Local<Object> instance = constructor->NewInstance(argc, argv);
    return scope.Close(instance);
}

ObjectsContextDriver::ObjectsContextDriver()
{
    ObjectsContextInstance = NULL;
}

Persistent<Function> ObjectsContextDriver::constructor;

Handle<v8::Value> ObjectsContextDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    ObjectsContextDriver* objectsContext = new ObjectsContextDriver();
    objectsContext->Wrap(args.This());
    
    return args.This();
}

Handle<Value> ObjectsContextDriver::AllocateNewObjectID(const Arguments& args)
{
    HandleScope scope;
    
    ObjectsContextDriver* objectsContextDriver = ObjectWrap::Unwrap<ObjectsContextDriver>(args.This());
    
    Local<Number> newID = Number::New(
                                     objectsContextDriver->ObjectsContextInstance->GetInDirectObjectsRegistry().AllocateNewObjectID());
    
    return scope.Close(newID);
    
}





