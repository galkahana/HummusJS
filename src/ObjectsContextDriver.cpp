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
#include "ConstructorsHolder.h"
#include <string>

using namespace v8;

DEF_SUBORDINATE_INIT(ObjectsContextDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE_EXTERNAL(New);

	t->SetClassName(NEW_STRING("ObjectsContext"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "allocateNewObjectID", AllocateNewObjectID);
	SET_PROTOTYPE_METHOD(t, "startDictionary", StartDictionary);
	SET_PROTOTYPE_METHOD(t, "startArray", StartArray);
	SET_PROTOTYPE_METHOD(t, "writeNumber", WriteNumber);
	SET_PROTOTYPE_METHOD(t, "endArray", EndArray);
	SET_PROTOTYPE_METHOD(t, "endLine", EndLine);
	SET_PROTOTYPE_METHOD(t, "endDictionary", EndDictionary);
	SET_PROTOTYPE_METHOD(t, "endIndirectObject", EndIndirectObject);
	SET_PROTOTYPE_METHOD(t, "writeIndirectObjectReference", WriteIndirectObjectReference);
	SET_PROTOTYPE_METHOD(t, "startNewIndirectObject", StartNewIndirectObject);
	SET_PROTOTYPE_METHOD(t, "startModifiedIndirectObject", StartModifiedIndirectObject);
	SET_PROTOTYPE_METHOD(t, "deleteObject", DeleteObject);
	SET_PROTOTYPE_METHOD(t, "writeName", WriteName);
	SET_PROTOTYPE_METHOD(t, "writeLiteralString", WriteLiteralString);
	SET_PROTOTYPE_METHOD(t, "writeHexString", WriteHexString);
	SET_PROTOTYPE_METHOD(t, "writeBoolean", WriteBoolean);
	SET_PROTOTYPE_METHOD(t, "writeKeyword", WriteKeyword);
	SET_PROTOTYPE_METHOD(t, "writeComment", WriteComment);
	SET_PROTOTYPE_METHOD(t, "setCompressStreams", SetCompressStreams);
	SET_PROTOTYPE_METHOD(t, "startPDFStream", StartPDFStream);
	SET_PROTOTYPE_METHOD(t, "startUnfilteredPDFStream", StartUnfilteredPDFStream);
	SET_PROTOTYPE_METHOD(t, "endPDFStream", EndPDFStream);
	SET_PROTOTYPE_METHOD(t, "startFreeContext", StartFreeContext);
	SET_PROTOTYPE_METHOD(t, "endFreeContext", EndFreeContext);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->ObjectsContext_constructor, t);       
}

ObjectsContextDriver::ObjectsContextDriver()
{
    ObjectsContextInstance = NULL;
}

METHOD_RETURN_TYPE ObjectsContextDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, externalHolder)
    
    ObjectsContextDriver* objectsContext = new ObjectsContextDriver();
    objectsContext->holder = externalHolder;
    objectsContext->Wrap(args.This());
    
	SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE ObjectsContextDriver::AllocateNewObjectID(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    ObjectsContextDriver* objectsContextDriver = ObjectWrap::Unwrap<ObjectsContextDriver>(args.This());
    
    Local<Number> newID = NEW_NUMBER(
                                     objectsContextDriver->ObjectsContextInstance->GetInDirectObjectsRegistry().AllocateNewObjectID());
    
    SET_FUNCTION_RETURN_VALUE(newID)
    
}

METHOD_RETURN_TYPE ObjectsContextDriver::DeleteObject(const ARGS_TYPE& args)
{
 
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument that is an object number");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    ObjectsContextDriver* objectsContextDriver = ObjectWrap::Unwrap<ObjectsContextDriver>(args.This());
    
    objectsContextDriver->ObjectsContextInstance->GetInDirectObjectsRegistry().DeleteObject(TO_UINT32(args[0])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}


METHOD_RETURN_TYPE ObjectsContextDriver::StartDictionary(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    ObjectsContextDriver* driver = ObjectWrap::Unwrap<ObjectsContextDriver>(args.This());
    
    DictionaryContext* dictionaryContext = driver->ObjectsContextInstance->StartDictionary();
    
    Local<Value> newInstance = driver->holder->GetNewDictionaryContext(args);
    ObjectWrap::Unwrap<DictionaryContextDriver>(newInstance->TO_OBJECT())->DictionaryContextInstance = dictionaryContext;
    SET_FUNCTION_RETURN_VALUE(newInstance)
}

METHOD_RETURN_TYPE ObjectsContextDriver::StartArray(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->StartArray();
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE ObjectsContextDriver::WriteNumber(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument that is a number");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    if(args[0]->IsUint32())
        ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteInteger(TO_UINT32(args[0])->Value());
    else if(args[0]->IsInt32())
        ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteInteger(TO_INT32(args[0])->Value());
    else
        ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteDouble(TO_NUMBER(args[0])->Value());

    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE ObjectsContextDriver::EndArray(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if((args.Length() != 0 && args.Length() != 1) ||
        (args.Length() == 1 && !args[0]->IsNumber()))
    {
		THROW_EXCEPTION("wrong arguments, pass 1 optional argument that defiened the array ending");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    if(args.Length() == 0)
        ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->EndArray();
    else
        ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->EndArray((ETokenSeparator)(TO_UINT32(args[0])->Value()));
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE ObjectsContextDriver::EndLine(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->EndLine();
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE ObjectsContextDriver::EndDictionary(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    ObjectsContextDriver* driver = ObjectWrap::Unwrap<ObjectsContextDriver>(args.This());
    
    if(args.Length() != 1 ||
       !driver->holder->IsDictionaryContextInstance(args[0]))
    {
		THROW_EXCEPTION("Wrong arguments. Please provide a dictionary to end");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    PDFHummus::EStatusCode status = driver->ObjectsContextInstance->EndDictionary(
                                        ObjectWrap::Unwrap<DictionaryContextDriver>(args[0]->TO_OBJECT())->DictionaryContextInstance);
    
    if(status != PDFHummus::eSuccess)
    {
		THROW_EXCEPTION("Inconsistent ending of dictionary. Wrong nesting of startDictionary and endDictionary");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE ObjectsContextDriver::EndIndirectObject(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->EndIndirectObject();
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE ObjectsContextDriver::WriteIndirectObjectReference(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if( (args.Length() != 1 &&  args.Length() != 2) ||
        !args[0]->IsNumber() ||
        (args.Length() == 2 && !args[1]->IsNumber())
       )
    {
		THROW_EXCEPTION("wrong arguments. Provide object ID to write reference for and optionally a version number");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }

    if(args.Length() == 1)
        ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteIndirectObjectReference(TO_UINT32(args[0])->Value());
    else
        ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteIndirectObjectReference(TO_UINT32(args[0])->Value(),
                                                                                                                    TO_UINT32(args[1])->Value());
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE ObjectsContextDriver::StartNewIndirectObject(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if((args.Length() != 0 && args.Length() != 1) ||
       (args.Length() == 1 && !args[0]->IsNumber()))
    {
		THROW_EXCEPTION("wrong arguments, pass no arguments, or pass 1 argument that is an object ID");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    if(args.Length() == 0)
    {
        // version that requires returning an object ID...so no chaining
        ObjectIDType result =
            ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->StartNewIndirectObject();
        SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(result))
    }
    else
    {
        // version that writes a know indirect object id, allow chaining
    
        ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->StartNewIndirectObject(TO_UINT32(args[0])->Value());
    
        SET_FUNCTION_RETURN_VALUE(args.This())
    }
}

METHOD_RETURN_TYPE ObjectsContextDriver::StartModifiedIndirectObject(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument that is an object ID");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->StartModifiedIndirectObject(TO_UINT32(args[0])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE ObjectsContextDriver::WriteName(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !args[0]->IsString())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument that is a name (string)");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteName(*UTF_8_VALUE(args[0]->TO_STRING()));
    
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}

METHOD_RETURN_TYPE ObjectsContextDriver::WriteLiteralString(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       (!args[0]->IsString() && !args[0]->IsArray()))
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument that is a literal string (string) or an array");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
	if(args[0]->IsArray())
	{
		std::string string;
		unsigned long arrayLength = (args[0]->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, v8::NEW_STRING("length")).ToLocalChecked())->TO_UINT32Value();
		for(unsigned long i=0;i<arrayLength;++i)
			string.push_back((unsigned char)TO_NUMBER(args[0]->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, i).ToLocalChecked())->Value());
		ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteLiteralString(string);
	}
	else
    {
		ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteLiteralString(*UTF_8_VALUE(args[0]->TO_STRING()));
	}
    
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}

METHOD_RETURN_TYPE ObjectsContextDriver::WriteHexString(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       (!args[0]->IsString() && !args[0]->IsArray()))
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument that is a literal string (string) or an array");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
	if(args[0]->IsArray())
	{
		std::string string;
		unsigned long arrayLength = (args[0]->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, v8::NEW_STRING("length")).ToLocalChecked())->TO_UINT32Value();
		for(unsigned long i=0;i<arrayLength;++i)
			string.push_back((unsigned char)TO_NUMBER(args[0]->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, i).ToLocalChecked())->Value());
		ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteHexString(string);
	}
	else
    {
		ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteHexString(*UTF_8_VALUE(args[0]->TO_STRING()));
	}
    
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}

METHOD_RETURN_TYPE ObjectsContextDriver::WriteBoolean(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !args[0]->IsBoolean())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument that is a boolean");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteBoolean(args[0]->TO_BOOLEAN()->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}

METHOD_RETURN_TYPE ObjectsContextDriver::WriteKeyword(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !args[0]->IsString())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument that is a keyword (string)");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteKeyword(*UTF_8_VALUE(args[0]->TO_STRING()));
    
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}

METHOD_RETURN_TYPE ObjectsContextDriver::WriteComment(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !args[0]->IsString())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument that is a comment (string)");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->WriteComment(*UTF_8_VALUE(args[0]->TO_STRING()));
    
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}

METHOD_RETURN_TYPE ObjectsContextDriver::SetCompressStreams(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !args[0]->IsBoolean())
    {
		THROW_EXCEPTION("wrong arguments, pass 1 argument that is a boolean, determining whether streams are to be compressed");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    }
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->SetCompressStreams(args[0]->TO_BOOLEAN()->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}

METHOD_RETURN_TYPE ObjectsContextDriver::EndPDFStream(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    ObjectsContextDriver* driver = ObjectWrap::Unwrap<ObjectsContextDriver>(args.This());
    
    if(args.Length() != 1 ||
       !driver->holder->IsPDFStreamInstance(args[0]))
    {
		THROW_EXCEPTION("wrong arguments, provide a stream to end");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    PDFStreamDriver* streamDriver = ObjectWrap::Unwrap<PDFStreamDriver>(args[0]->TO_OBJECT());
    
    driver->ObjectsContextInstance->EndPDFStream(streamDriver->PDFStreamInstance);
    
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}


METHOD_RETURN_TYPE ObjectsContextDriver::StartPDFStream(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    ObjectsContextDriver* objectsContext = ObjectWrap::Unwrap<ObjectsContextDriver>(args.This());
    
    if((args.Length() != 0 && args.Length() != 1) ||
       (args.Length() == 1 && !objectsContext->holder->IsDictionaryContextInstance(args[0])))
    {
		THROW_EXCEPTION("wrong arguments, please provide no arguments or an optional stream dictionary");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    PDFStream* aStream;

    
    
    if(args.Length() == 1)
    {
        DictionaryContextDriver* driver = ObjectWrap::Unwrap<DictionaryContextDriver>(args[0]->TO_OBJECT());
        aStream = objectsContext->ObjectsContextInstance->StartPDFStream(driver->DictionaryContextInstance);
    }
    else
    {
        aStream = objectsContext->ObjectsContextInstance->StartPDFStream();
    }
    
    Local<Value> newInstance = objectsContext->holder->GetNewPDFStream(args);
    PDFStreamDriver* streamDriver = ObjectWrap::Unwrap<PDFStreamDriver>(newInstance->TO_OBJECT());
    streamDriver->PDFStreamInstance = aStream;
    streamDriver->mOwns = true;

    
    SET_FUNCTION_RETURN_VALUE(newInstance)
}

METHOD_RETURN_TYPE ObjectsContextDriver::StartUnfilteredPDFStream(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    ObjectsContextDriver* objectsContext = ObjectWrap::Unwrap<ObjectsContextDriver>(args.This());

    if((args.Length() != 0 && args.Length() != 1) ||
       (args.Length() == 1 && !objectsContext->holder->IsDictionaryContextInstance(args[0])))
    {
		THROW_EXCEPTION("wrong arguments, please provide no arguments or an optional stream dictionary");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    PDFStream* aStream;
    

    if(args.Length() == 1)
    {
        DictionaryContextDriver* driver = ObjectWrap::Unwrap<DictionaryContextDriver>(args[0]->TO_OBJECT());
        aStream = objectsContext->ObjectsContextInstance->StartUnfilteredPDFStream(driver->DictionaryContextInstance);
    }
    else
    {
        aStream = objectsContext->ObjectsContextInstance->StartUnfilteredPDFStream();
    }
    
    Local<Value> newInstance = objectsContext->holder->GetNewPDFStream(args);
    PDFStreamDriver* streamDriver = ObjectWrap::Unwrap<PDFStreamDriver>(newInstance->TO_OBJECT());
    streamDriver->PDFStreamInstance = aStream;
    streamDriver->mOwns = true;

    
    SET_FUNCTION_RETURN_VALUE(newInstance)
}


METHOD_RETURN_TYPE ObjectsContextDriver::StartFreeContext(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    ObjectsContextDriver* driver = ObjectWrap::Unwrap<ObjectsContextDriver>(args.This());
    
    Local<Value> result = driver->holder->GetNewByteWriterWithPosition(args);
    
    ObjectWrap::Unwrap<ByteWriterWithPositionDriver>(result->TO_OBJECT())->SetStream(driver->ObjectsContextInstance->StartFreeContext(), false);
    
    SET_FUNCTION_RETURN_VALUE(result)
}

METHOD_RETURN_TYPE ObjectsContextDriver::EndFreeContext(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    ObjectWrap::Unwrap<ObjectsContextDriver>(args.This())->ObjectsContextInstance->EndFreeContext();
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}