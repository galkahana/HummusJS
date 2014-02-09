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
#include "ImageXObjectDriver.h"

using namespace v8;


void ResourcesDictionaryDriver::Init()
{
    // prepare the form xobject driver interfrace template
    Local<FunctionTemplate> ft = FunctionTemplate::New(New);
    ft->SetClassName(String::NewSymbol("ResourcesDictionary"));
    ft->InstanceTemplate()->SetInternalFieldCount(1);
    
    ft->PrototypeTemplate()->Set(String::NewSymbol("addFormXObjectMapping"),FunctionTemplate::New(AddFormXObjectMapping)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("addImageXObjectMapping"),FunctionTemplate::New(AddImageXObjectMapping)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("addProcsetResource"),FunctionTemplate::New(AddProcsetResource)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("addExtGStateMapping"),FunctionTemplate::New(AddExtGStateMapping)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("addFontMapping"),FunctionTemplate::New(AddFontMapping)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("addColorSpaceMapping"),FunctionTemplate::New(AddColorSpaceMapping)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("addPatternMapping"),FunctionTemplate::New(AddPatternMapping)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("addPropertyMapping"),FunctionTemplate::New(AddPropertyMapping)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("addXObjectMapping"),FunctionTemplate::New(AddXObjectMapping)->GetFunction());
    ft->PrototypeTemplate()->Set(String::NewSymbol("addShadingMapping"),FunctionTemplate::New(AddShadingMapping)->GetFunction());
    
    

    
    constructor = Persistent<Function>::New(ft->GetFunction());
}

Handle<Value> ResourcesDictionaryDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    Local<Object> instance = constructor->NewInstance();
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

Handle<Value> ResourcesDictionaryDriver::AddImageXObjectMapping(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1)
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument which is the image xobject or its ID")));
		return scope.Close(Undefined());
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    if(ImageXObjectDriver::HasInstance(args[0]))
    {
    
        Local<String> name = String::New(
                                         resourcesDictionaryDriver->ResourcesDictionaryInstance->AddImageXObjectMapping(
                                                                                ObjectWrap::Unwrap<ImageXObjectDriver>(args[0]->ToObject())->ImageXObject
                                                                                    ).c_str());
        
        return scope.Close(name);
    }
    else if(args[0]->IsNumber())
    {
        Local<String> name = String::New(
                                         resourcesDictionaryDriver->ResourcesDictionaryInstance->AddImageXObjectMapping(
                                                                                                                       (ObjectIDType)(args[0]->ToUint32()->Value())).c_str());
        
        return scope.Close(name);
    }
    else
    {
		ThrowException(Exception::Error(String::New("wrong arguments, pass 1 argument which is the image xobject or its ID")));
		return scope.Close(Undefined());
    }
}

Handle<Value> ResourcesDictionaryDriver::AddProcsetResource(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 || !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument which is the procset name")));
		return scope.Close(Undefined());
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    resourcesDictionaryDriver->ResourcesDictionaryInstance->AddProcsetResource(*String::Utf8Value(args[0]->ToString()));
    
     return scope.Close(Undefined());
    
}

Handle<Value> ResourcesDictionaryDriver::AddExtGStateMapping(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument which is the external graphic state object id")));
		return scope.Close(Undefined());
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    Local<String> name = String::New(
                                     resourcesDictionaryDriver->ResourcesDictionaryInstance->AddExtGStateMapping(
                                                                                                                   (ObjectIDType)(args[0]->ToUint32()->Value())).c_str());
    
    return scope.Close(name);
}

Handle<Value> ResourcesDictionaryDriver::AddFontMapping(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument which is the font object id")));
		return scope.Close(Undefined());
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    Local<String> name = String::New(
                                     resourcesDictionaryDriver->ResourcesDictionaryInstance->AddFontMapping(
                                                                                                                 (ObjectIDType)(args[0]->ToUint32()->Value())).c_str());
    
    return scope.Close(name);
}

Handle<Value> ResourcesDictionaryDriver::AddColorSpaceMapping(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument which is the color space id")));
		return scope.Close(Undefined());
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    Local<String> name = String::New(
                                     resourcesDictionaryDriver->ResourcesDictionaryInstance->AddColorSpaceMapping(
                                                                                                            (ObjectIDType)(args[0]->ToUint32()->Value())).c_str());
    
    return scope.Close(name);
}

Handle<Value> ResourcesDictionaryDriver::AddPatternMapping(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument which is the pattern object id")));
		return scope.Close(Undefined());
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    Local<String> name = String::New(
                                     resourcesDictionaryDriver->ResourcesDictionaryInstance->AddPatternMapping(
                                                                                                                  (ObjectIDType)(args[0]->ToUint32()->Value())).c_str());
    
    return scope.Close(name);
}

Handle<Value> ResourcesDictionaryDriver::AddPropertyMapping(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument which is the property object id")));
		return scope.Close(Undefined());
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    Local<String> name = String::New(
                                     resourcesDictionaryDriver->ResourcesDictionaryInstance->AddPropertyMapping(
                                                                                                               (ObjectIDType)(args[0]->ToUint32()->Value())).c_str());
    
    return scope.Close(name);
}

Handle<Value> ResourcesDictionaryDriver::AddXObjectMapping(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument which is the xobject id")));
		return scope.Close(Undefined());
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    Local<String> name = String::New(
                                     resourcesDictionaryDriver->ResourcesDictionaryInstance->AddXObjectMapping(
                                                                                                                (ObjectIDType)(args[0]->ToUint32()->Value())).c_str());
    
    return scope.Close(name);
}

Handle<Value> ResourcesDictionaryDriver::AddShadingMapping(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument which is the shading object id")));
		return scope.Close(Undefined());
    }
    
    ResourcesDictionaryDriver* resourcesDictionaryDriver = ObjectWrap::Unwrap<ResourcesDictionaryDriver>(args.This());
    
    Local<String> name = String::New(
                                     resourcesDictionaryDriver->ResourcesDictionaryInstance->AddShadingMapping(
                                                                                                               (ObjectIDType)(args[0]->ToUint32()->Value())).c_str());
    
    return scope.Close(name);
}


