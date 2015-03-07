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
#include "PDFPageInputDriver.h"
#include "PDFDictionaryDriver.h"
#include "PDFObjectDriver.h"
#include "RefCountPtr.h"
#include "PDFDictionary.h"
#include "PDFArrayDriver.h"
#include "PDFArray.h"
#include "PDFPageInput.h"
#include "PDFStreamInputDriver.h"
#include "ByteReaderDriver.h"
#include "ByteReaderWithPositionDriver.h"
#include "ObjectByteReaderWithPosition.h"

using namespace v8;

Persistent<Function> PDFReaderDriver::constructor;
Persistent<FunctionTemplate> PDFReaderDriver::constructor_template;

PDFReaderDriver::PDFReaderDriver()
{
    mStartedWithStream = false;
    mPDFReader = NULL;
    mReadStreamProxy = NULL;
    mOwnsParser = false;
}

PDFReaderDriver::~PDFReaderDriver()
{
    delete mReadStreamProxy;
    if(mOwnsParser)
        delete mPDFReader;
}

void PDFReaderDriver::Init()
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PDFReader"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "getPDFLevel", GetPDFLevel);
	SET_PROTOTYPE_METHOD(t, "getPagesCount", GetPagesCount);
	SET_PROTOTYPE_METHOD(t, "getTrailer", GetTrailer);
	SET_PROTOTYPE_METHOD(t, "queryDictionaryObject", QueryDictionaryObject);
	SET_PROTOTYPE_METHOD(t, "queryArrayObject", QueryArrayObject);
	SET_PROTOTYPE_METHOD(t, "parseNewObject", ParseNewObject);
	SET_PROTOTYPE_METHOD(t, "getPageObjectID", GetPageObjectID);
	SET_PROTOTYPE_METHOD(t, "parsePageDictionary", ParsePageDictionary);
	SET_PROTOTYPE_METHOD(t, "parsePage", ParsePage);
	SET_PROTOTYPE_METHOD(t, "getObjectsCount", GetObjectsCount);
	SET_PROTOTYPE_METHOD(t, "isEncrypted", IsEncrypted);
	SET_PROTOTYPE_METHOD(t, "getXrefSize", GetXrefSize);
	SET_PROTOTYPE_METHOD(t, "getXrefEntry", GetXrefEntry);
	SET_PROTOTYPE_METHOD(t, "getXrefPosition", GetXrefPosition);
	SET_PROTOTYPE_METHOD(t, "startReadingFromStream", StartReadingFromStream);
	SET_PROTOTYPE_METHOD(t, "getParserStream", GetParserStream);
	SET_CONSTRUCTOR(constructor, t);
    SET_CONSTRUCTOR_TEMPLATE(constructor_template, t);    
}

METHOD_RETURN_TYPE PDFReaderDriver::NewInstance(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
    CREATE_ESCAPABLE_SCOPE;

	SET_FUNCTION_RETURN_VALUE(PDFReaderDriver::GetNewInstance(args));
}

bool PDFReaderDriver::HasInstance(Handle<Value> inObject)
{
    CREATE_ISOLATE_CONTEXT;

    return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);
}

v8::Handle<v8::Value> PDFReaderDriver::GetNewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	if (args.Length() == 1)
	{
		const unsigned argc = 1;
		Handle<Value> argv[argc] = { args[0] };
		Local<Object> instance = NEW_INSTANCE_ARGS(constructor, argc, argv);
		return CLOSE_SCOPE(instance);
	}
	else
	{
		Local<Object> instance = NEW_INSTANCE(constructor);
		return CLOSE_SCOPE(instance);
	}
}


METHOD_RETURN_TYPE PDFReaderDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFReaderDriver* reader = new PDFReaderDriver();
    reader->Wrap(args.This());
    
	SET_FUNCTION_RETURN_VALUE(args.This());
}

METHOD_RETURN_TYPE PDFReaderDriver::GetPDFLevel(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
    CREATE_ESCAPABLE_SCOPE;

    SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(ObjectWrap::Unwrap<PDFReaderDriver>(args.This())->mPDFReader->GetPDFLevel()));
}

METHOD_RETURN_TYPE PDFReaderDriver::GetPagesCount(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
    CREATE_ESCAPABLE_SCOPE;

	SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(ObjectWrap::Unwrap<PDFReaderDriver>(args.This())->mPDFReader->GetPagesCount()));
}


METHOD_RETURN_TYPE PDFReaderDriver::QueryDictionaryObject(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    if(args.Length() != 2 ||
       !PDFDictionaryDriver::HasInstance(args[0]) ||
       !args[1]->IsString())
    {
 		THROW_EXCEPTION("Wrong arguments. Provide a dictionary and a string");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    PDFDictionaryDriver* dictionary = ObjectWrap::Unwrap<PDFDictionaryDriver>(args[0]->ToObject());
    
    RefCountPtr<PDFObject> object = reader->mPDFReader->QueryDictionaryObject(dictionary->TheObject.GetPtr(),*String::Utf8Value(args[1]->ToString()));
    if(!object)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
        
    SET_FUNCTION_RETURN_VALUE(PDFObjectDriver::CreateDriver(object.GetPtr()));
}

METHOD_RETURN_TYPE PDFReaderDriver::QueryArrayObject(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    if(args.Length() != 2 ||
       !PDFArrayDriver::HasInstance(args[0]) ||
       !args[1]->IsNumber())
    {
 		THROW_EXCEPTION("Wrong arguments. Provide an array and an index");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    PDFArrayDriver* driver = ObjectWrap::Unwrap<PDFArrayDriver>(args[0]->ToObject());
    
    RefCountPtr<PDFObject> object = reader->mPDFReader->QueryArrayObject(driver->TheObject.GetPtr(),args[1]->ToNumber()->Uint32Value());
    if(!object)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    
    SET_FUNCTION_RETURN_VALUE(PDFObjectDriver::CreateDriver(object.GetPtr()));
}

METHOD_RETURN_TYPE PDFReaderDriver::GetTrailer(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
 
    PDFDictionary* trailer = reader->mPDFReader->GetTrailer();
    
    if(!trailer)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    
    SET_FUNCTION_RETURN_VALUE(PDFObjectDriver::CreateDriver(trailer));
}

PDFHummus::EStatusCode PDFReaderDriver::StartPDFParsing(v8::Handle<v8::Object> inStreamObject)
{
    if(!mPDFReader && !mOwnsParser)
    {
        mPDFReader = new PDFParser();
        mOwnsParser = true;
    }
    if(mReadStreamProxy)
    {
        delete mReadStreamProxy;
        mReadStreamProxy = NULL;
    }
    
    mStartedWithStream = true;
    mReadStreamProxy = new ObjectByteReaderWithPosition(inStreamObject);
    mPDFReader->ResetParser();
    return mPDFReader->StartPDFParsing(mReadStreamProxy);
}

PDFHummus::EStatusCode PDFReaderDriver::StartPDFParsing(const std::string& inParsedFilePath)
{
    if(!mPDFReader && !mOwnsParser)
    {
        mPDFReader = new PDFParser();
        mOwnsParser = true;
    }
    if(mReadStreamProxy)
    {
        delete mReadStreamProxy;
        mReadStreamProxy = NULL;
    }
    
    
    mStartedWithStream = false;
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
        delete mReadStreamProxy;
        mStartedWithStream = false;
        mPDFFile.CloseFile();
    }
    mPDFReader = inParser;
}

PDFParser* PDFReaderDriver::GetParser()
{
    return mPDFReader;
}

METHOD_RETURN_TYPE PDFReaderDriver::ParseNewObject(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
 		THROW_EXCEPTION("Wrong arguments. Provide an Object ID");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    
    RefCountPtr<PDFObject> newObject = reader->mPDFReader->ParseNewObject(args[0]->ToNumber()->Uint32Value());
    
    if(!newObject)
    {
 		THROW_EXCEPTION("Unable to read object. Most probably object ID is wrong (or some file read issue...but i'd first check that ID. if i were you)");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    SET_FUNCTION_RETURN_VALUE(PDFObjectDriver::CreateDriver(newObject.GetPtr()));
}

METHOD_RETURN_TYPE PDFReaderDriver::GetPageObjectID(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
 		THROW_EXCEPTION("Wrong arguments. Provide a page index");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    
    SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(reader->mPDFReader->GetPageObjectID(args[0]->ToNumber()->Uint32Value())));
}


METHOD_RETURN_TYPE PDFReaderDriver::ParsePageDictionary(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
 		THROW_EXCEPTION("Wrong arguments. Provide a page index");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    
    RefCountPtr<PDFDictionary> newObject = reader->mPDFReader->ParsePage(args[0]->ToNumber()->Uint32Value());
    
    if(!newObject)
    {
 		THROW_EXCEPTION("Unable to read page, parhaps page index is wrong");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    SET_FUNCTION_RETURN_VALUE(PDFObjectDriver::CreateDriver(newObject.GetPtr()));
}

METHOD_RETURN_TYPE PDFReaderDriver::ParsePage(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
 		THROW_EXCEPTION("Wrong arguments. Provide a page index");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    
    RefCountPtr<PDFDictionary> newObject = reader->mPDFReader->ParsePage(args[0]->ToNumber()->Uint32Value());
    
    if(!newObject)
    {
 		THROW_EXCEPTION("Unable to read page, parhaps page index is wrong");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    Handle<Value> newInstance = PDFPageInputDriver::GetNewInstance();
    ObjectWrap::Unwrap<PDFPageInputDriver>(newInstance->ToObject())->PageInput = new PDFPageInput(reader->mPDFReader,newObject);
    ObjectWrap::Unwrap<PDFPageInputDriver>(newInstance->ToObject())->PageInputDictionary = newObject.GetPtr();
    SET_FUNCTION_RETURN_VALUE(newInstance);
}

METHOD_RETURN_TYPE PDFReaderDriver::GetObjectsCount(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    
    SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(ObjectWrap::Unwrap<PDFReaderDriver>(args.This())->mPDFReader->GetObjectsCount()));
}

METHOD_RETURN_TYPE PDFReaderDriver::IsEncrypted(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    
    SET_FUNCTION_RETURN_VALUE(NEW_BOOLEAN(ObjectWrap::Unwrap<PDFReaderDriver>(args.This())->mPDFReader->IsEncrypted()));
}

METHOD_RETURN_TYPE PDFReaderDriver::GetXrefSize(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    
    SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(ObjectWrap::Unwrap<PDFReaderDriver>(args.This())->mPDFReader->GetXrefSize()));
}

METHOD_RETURN_TYPE PDFReaderDriver::GetXrefEntry(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
 		THROW_EXCEPTION("Wrong arguments. Provide an Object ID");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    
    XrefEntryInput* xrefEntry = reader->mPDFReader->GetXrefEntry(args[0]->ToNumber()->Uint32Value());
    if(!xrefEntry)
    {
 		THROW_EXCEPTION("Unable to read object xref entry, parhaps page index is wrong");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);   
    }
    
    Handle<Object> anObject = NEW_OBJECT;
    
    anObject->Set(NEW_STRING("objectPosition"),NEW_NUMBER(xrefEntry->mObjectPosition));
    anObject->Set(NEW_STRING("revision"),NEW_NUMBER(xrefEntry->mRivision));
    anObject->Set(NEW_STRING("type"),NEW_NUMBER(xrefEntry->mType));

    SET_FUNCTION_RETURN_VALUE(anObject);
}


METHOD_RETURN_TYPE PDFReaderDriver::GetXrefPosition(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    
    SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(ObjectWrap::Unwrap<PDFReaderDriver>(args.This())->mPDFReader->GetXrefPosition()));
}

METHOD_RETURN_TYPE PDFReaderDriver::StartReadingFromStream(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !PDFStreamInputDriver::HasInstance(args[0]))
    {
 		THROW_EXCEPTION("Wrong arguments. provide a PDF stream input");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    PDFStreamInputDriver* streamInput = ObjectWrap::Unwrap<PDFStreamInputDriver>(args[0]->ToObject());
    
    IByteReader* byteReader = reader->mPDFReader->StartReadingFromStream(streamInput->TheObject.GetPtr());
    
    Handle<Value> driver = ByteReaderDriver::GetNewInstance(args);
    ObjectWrap::Unwrap<ByteReaderDriver>(driver->ToObject())->SetStream(byteReader,true);
    
    SET_FUNCTION_RETURN_VALUE(driver);
}

METHOD_RETURN_TYPE PDFReaderDriver::GetParserStream(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());

    Handle<Value> driver = ByteReaderWithPositionDriver::GetNewInstance(args);
    ObjectWrap::Unwrap<ByteReaderWithPositionDriver>(driver->ToObject())->SetStream(reader->mPDFReader->GetParserStream(),false);
    
    SET_FUNCTION_RETURN_VALUE(driver);
}

