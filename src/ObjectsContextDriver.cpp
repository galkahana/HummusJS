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
#include "DictionaryContextDriver.h"
#include "DictionaryContext.h"
#include "ETokenSeparator.h"
#include "EStatusCode.h"
#include "PDFStreamDriver.h"
#include "ByteWriterWithPositionDriver.h"
#include <string>

using namespace v8;


void ObjectsContextDriver::Init()
{
    // prepare the form xobject driver interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->SetClassName(String::NewSymbol("ObjectsContext"));
    t->InstanceTemplate()->SetInternalFieldCount(1);
    
    t->PrototypeTemplate()->Set(String::NewSymbol("allocateNewObjectID"),FunctionTemplate::New(AllocateNewObjectID)->GetFunction());
 
    t->PrototypeTemplate()->Set(String::NewSymbol("startDictionary"),FunctionTemplate::New(StartDictionary)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("startArray"),FunctionTemplate::New(StartArray)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("writeNumber"),FunctionTemplate::New(WriteNumber)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("endArray"),FunctionTemplate::New(EndArray)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("endLine"),FunctionTemplate::New(EndLine)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("endDictionary"),FunctionTemplate::New(EndDictionary)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("endIndirectObject"),FunctionTemplate::New(EndIndirectObject)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("writeIndirectObjectReference"),FunctionTemplate::New(WriteIndirectObjectReference)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("startNewIndirectObject"),FunctionTemplate::New(StartNewIndirectObject)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("startModifiedIndirectObject"),FunctionTemplate::New(StartModifiedIndirectObject)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("deleteObject"),FunctionTemplate::New(DeleteObject)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("writeName"),FunctionTemplate::New(WriteName)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("writeLiteralString"),FunctionTemplate::New(WriteLiteralString)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("writeHexString"),FunctionTemplate::New(WriteHexString)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("writeBoolean"),FunctionTemplate::New(WriteBoolean)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("writeKeyword"),FunctionTemplate::New(WriteKeyword)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("writeComment"),FunctionTemplate::New(WriteComment)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("setCompressStreams"),FunctionTemplate::New(SetCompressStreams)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("startPDFStream"),FunctionTemplate::New(StartPDFStream)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("startUnfilteredPDFStream"),FunctionTemplate::New(StartUnfilteredPDFStream)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("endPDFStream"),FunctionTemplate::New(EndPDFStream)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("startFreeContext"),FunctionTemplate::New(StartFreeContext)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("endFreeContext"),FunctionTemplate::New(EndFreeContext)->GetFunction());
    
    constructor = Persistent<Function>::New(t->GetFunction());
}

Handle<Value> ObjectsContextDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    Local<Object> instance = constructor->NewInstance();
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

Handle<Value> ObjectsContextDriver::DeleteObject(const Arguments& args)
{
 
    HandleScope scope;
    
    if(!args.Length() == 1 ||
       !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is an object number")));
		return scope.Close(Undefined());
        
    }
    
    ObjectsContextDriver* objectsContextDriver = ObjectWrap::Unwrap<ObjectsContextDriver>(args.This());
    
    objectsContextDriver->ObjectsContextInstance->GetInDirectObjectsRegistry().DeleteObject(args[0]->ToNumber()->Uint32Value());
    
    return scope.Close(args.This());
    
}


Handle<Value> ObjectsContextDriver::StartDictionary(const Arguments& args)
{
    HandleScope scope;
    
    ObjectsContextDriver* driver = ObjectWrap::Unwrap<ObjectsContextDriver>(args.This());
    
    DictionaryContext* dictionaryContext = driver->ObjectsContextInstance->StartDictionary();
    
    Handle<Value> newInstance = DictionaryContextDriver::NewInstance(args);
    ObjectWrap::Unwrap<DictionaryContextDriver>(newInstance->ToObject())->DictionaryContextInstance = dictionaryContext;
    return scope.Close(newInstance);
}

Handle<Value> ObjectsContextDriver::StartArray(const Arguments& args)
{
    HandleScope scope;
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->StartArray();
    return scope.Close(args.This());
}

Handle<Value> ObjectsContextDriver::WriteNumber(const Arguments& args)
{
    HandleScope scope;
    
    if(!args.Length() == 1 ||
       !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is a number")));
		return scope.Close(Undefined());
        
    }
    
    if(args[0]->IsUint32())
        ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteInteger(args[0]->ToUint32()->Uint32Value());
    else if(args[0]->IsInt32())
        ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteInteger(args[0]->ToInt32()->Int32Value());
    else
        ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteDouble(args[0]->ToNumber()->Value());

    return scope.Close(args.This());
}

Handle<Value> ObjectsContextDriver::EndArray(const Arguments& args)
{
    HandleScope scope;
    
    if((!args.Length() == 0 && !args.Length() == 1) ||
        (args.Length() == 1 && !args[0]->IsNumber()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 optional argument that defiened the array ending")));
		return scope.Close(Undefined());
        
    }
    
    if(args.Length() == 0)
        ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->EndArray();
    else
        ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->EndArray((ETokenSeparator)(args[0]->ToNumber()->Uint32Value()));
    
    return scope.Close(args.This());
}

Handle<Value> ObjectsContextDriver::EndLine(const Arguments& args)
{
    HandleScope scope;
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->EndLine();
    return scope.Close(args.This());
}

Handle<Value> ObjectsContextDriver::EndDictionary(const Arguments& args)
{
    HandleScope scope;
    
    if(!args.Length() == 1 ||
       !DictionaryContextDriver::HasInstance(args[0]))
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. Please provide a dictionary to end")));
		return scope.Close(Undefined());
    }
    
    PDFHummus::EStatusCode status = ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->EndDictionary(
                                                                    ObjectWrap::Unwrap<DictionaryContextDriver>(args[0]->ToObject())->DictionaryContextInstance);
    
    if(status != PDFHummus::eSuccess)
    {
		ThrowException(Exception::TypeError(String::New("Inconsistent ending of dictionary. Wrong nesting of startDictionary and endDictionary")));
		return scope.Close(Undefined());
    }
    return scope.Close(args.This());
}

Handle<Value> ObjectsContextDriver::EndIndirectObject(const Arguments& args)
{
    HandleScope scope;
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->EndIndirectObject();
    return scope.Close(args.This());
}

Handle<Value> ObjectsContextDriver::WriteIndirectObjectReference(const Arguments& args)
{
    HandleScope scope;
    
    if( (args.Length() != 1 &&  args.Length() != 2) ||
        !args[0]->IsNumber() ||
        (args.Length() == 2 && !args[1]->IsNumber())
       )
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments. Provide object ID to write reference for and optionally a version number")));
		return scope.Close(Undefined());
        
    }

    if(args.Length() == 1)
        ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteIndirectObjectReference(args[0]->ToNumber()->Uint32Value());
    else
        ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteIndirectObjectReference(args[0]->ToNumber()->Uint32Value(),
                                                                                                                    args[1]->ToNumber()->Uint32Value());
    return scope.Close(args.This());
}

Handle<Value> ObjectsContextDriver::StartNewIndirectObject(const Arguments& args)
{
    HandleScope scope;
    
    if((!args.Length() == 0 && !args.Length() == 1) ||
       (args.Length() == 1 && !args[0]->IsNumber()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass no arguments, or pass 1 argument that is an object ID")));
		return scope.Close(Undefined());
        
    }
    
    if(args.Length() == 0)
    {
        // version that requires returning an object ID...so no chaining
        ObjectIDType result =
            ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->StartNewIndirectObject();
        return scope.Close(Number::New(result));
    }
    else
    {
        // version that writes a know indirect object id, allow chaining
    
        ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->StartNewIndirectObject(args[0]->ToUint32()->Uint32Value());
    
        return scope.Close(args.This());
    }
}

Handle<Value> ObjectsContextDriver::StartModifiedIndirectObject(const Arguments& args)
{
    HandleScope scope;
    
    if(!args.Length() == 1 ||
       !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is an object ID")));
		return scope.Close(Undefined());
        
    }
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->StartModifiedIndirectObject(args[0]->ToUint32()->Uint32Value());
    
    return scope.Close(args.This());
}

Handle<Value> ObjectsContextDriver::WriteName(const Arguments& args)
{
    HandleScope scope;
    
    if(!args.Length() == 1 ||
       !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is a name (string)")));
		return scope.Close(Undefined());
        
    }
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteName(*String::Utf8Value(args[0]->ToString()));
    
    return scope.Close(args.This());
    
}

Handle<Value> ObjectsContextDriver::WriteLiteralString(const Arguments& args)
{
    HandleScope scope;
    
    if(!args.Length() == 1 ||
       (!args[0]->IsString() && !args[0]->IsArray()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is a literal string (string) or an array")));
		return scope.Close(Undefined());
        
    }
    
	if(args[0]->IsArray())
	{
		std::string string;
		unsigned long arrayLength = (args[0]->ToObject()->Get(v8::String::New("length")))->ToObject()->Uint32Value();
		for(unsigned long i=0;i<arrayLength;++i)
			string.push_back((unsigned char)args[0]->ToObject()->Get(i)->ToNumber()->Value());
		ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteLiteralString(string);
	}
	else
    {
		ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteLiteralString(*String::Utf8Value(args[0]->ToString()));
	}
    
    return scope.Close(args.This());
    
}

Handle<Value> ObjectsContextDriver::WriteHexString(const Arguments& args)
{
    HandleScope scope;
    
    if(!args.Length() == 1 ||
       (!args[0]->IsString() && !args[0]->IsArray()))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is a literal string (string) or an array")));
		return scope.Close(Undefined());
        
    }
    
	if(args[0]->IsArray())
	{
		std::string string;
		unsigned long arrayLength = (args[0]->ToObject()->Get(v8::String::New("length")))->ToObject()->Uint32Value();
		for(unsigned long i=0;i<arrayLength;++i)
			string.push_back((unsigned char)args[0]->ToObject()->Get(i)->ToNumber()->Value());
		ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteHexString(string);
	}
	else
    {
		ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteHexString(*String::Utf8Value(args[0]->ToString()));
	}
    
    return scope.Close(args.This());
    
}

Handle<Value> ObjectsContextDriver::WriteBoolean(const Arguments& args)
{
    HandleScope scope;
    
    if(!args.Length() == 1 ||
       !args[0]->IsBoolean())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is a boolean")));
		return scope.Close(Undefined());
        
    }
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteBoolean(args[0]->ToBoolean()->Value());
    
    return scope.Close(args.This());
    
}

Handle<Value> ObjectsContextDriver::WriteKeyword(const Arguments& args)
{
    HandleScope scope;
    
    if(!args.Length() == 1 ||
       !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is a keyword (string)")));
		return scope.Close(Undefined());
        
    }
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteKeyword(*String::Utf8Value(args[0]->ToString()));
    
    return scope.Close(args.This());
    
}

Handle<Value> ObjectsContextDriver::WriteComment(const Arguments& args)
{
    HandleScope scope;
    
    if(!args.Length() == 1 ||
       !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is a comment (string)")));
		return scope.Close(Undefined());
        
    }
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteComment(*String::Utf8Value(args[0]->ToString()));
    
    return scope.Close(args.This());
    
}

Handle<Value> ObjectsContextDriver::SetCompressStreams(const Arguments& args)
{
    HandleScope scope;
    
    if(!args.Length() == 1 ||
       !args[0]->IsBoolean())
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, pass 1 argument that is a boolean, determining whether streams are to be compressed")));
		return scope.Close(Undefined());
        
    }
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->SetCompressStreams(args[0]->ToBoolean()->Value());
    
    return scope.Close(args.This());
    
}

Handle<Value> ObjectsContextDriver::EndPDFStream(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 ||
       !PDFStreamDriver::HasInstance(args[0]))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, provide a stream to end")));
		return scope.Close(Undefined());
    }
    
    PDFStreamDriver* driver = ObjectWrap::Unwrap<PDFStreamDriver>(args[0]->ToObject());
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->EndPDFStream(driver->PDFStreamInstance);
    
    return scope.Close(args.This());
    
}


Handle<Value> ObjectsContextDriver::StartPDFStream(const Arguments& args)
{
    HandleScope scope;
    
    if((args.Length() != 0 && args.Length() != 1) ||
       (args.Length() == 1 && !DictionaryContextDriver::HasInstance(args[0])))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, please provide no arguments or an optional stream dictionary")));
		return scope.Close(Undefined());
    }
    
    PDFStream* aStream;
    
    if(args.Length() == 1)
    {
        DictionaryContextDriver* driver = ObjectWrap::Unwrap<DictionaryContextDriver>(args[0]->ToObject());
        aStream = ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->StartPDFStream(driver->DictionaryContextInstance);
    }
    else
    {
        aStream = ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->StartPDFStream();
    }
    
    Handle<Value> newInstance = PDFStreamDriver::NewInstance(args);
    PDFStreamDriver* streamDriver = ObjectWrap::Unwrap<PDFStreamDriver>(newInstance->ToObject());
    streamDriver->PDFStreamInstance = aStream;

    
    return scope.Close(newInstance);
}

Handle<Value> ObjectsContextDriver::StartUnfilteredPDFStream(const Arguments& args)
{
    HandleScope scope;
    
    if((args.Length() != 0 && args.Length() != 1) ||
       (args.Length() == 1 && !DictionaryContextDriver::HasInstance(args[0])))
    {
		ThrowException(Exception::TypeError(String::New("wrong arguments, please provide no arguments or an optional stream dictionary")));
		return scope.Close(Undefined());
    }
    
    PDFStream* aStream;
    
    if(args.Length() == 1)
    {
        DictionaryContextDriver* driver = ObjectWrap::Unwrap<DictionaryContextDriver>(args[0]->ToObject());
        aStream = ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->StartUnfilteredPDFStream(driver->DictionaryContextInstance);
    }
    else
    {
        aStream = ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->StartUnfilteredPDFStream();
    }
    
    Handle<Value> newInstance = PDFStreamDriver::NewInstance(args);
    PDFStreamDriver* streamDriver = ObjectWrap::Unwrap<PDFStreamDriver>(newInstance->ToObject());
    streamDriver->PDFStreamInstance = aStream;

    
    return scope.Close(newInstance);
}


Handle<Value> ObjectsContextDriver::StartFreeContext(const Arguments& args)
{
    HandleScope scope;
    
    ObjectsContextDriver* driver = ObjectWrap::Unwrap<ObjectsContextDriver>(args.This());
    
    Handle<Value> result = ByteWriterWithPositionDriver::NewInstance(args);
    
    ObjectWrap::Unwrap<ByteWriterWithPositionDriver>(result->ToObject())->SetStream(driver->ObjectsContextInstance->StartFreeContext(), false);
    
    return scope.Close(result);
}

Handle<Value> ObjectsContextDriver::EndFreeContext(const Arguments& args)
{
    HandleScope scope;
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->EndFreeContext();
    
    return scope.Close(args.This());
}