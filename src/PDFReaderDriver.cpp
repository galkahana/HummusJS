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
#include "PDFObjectParserDriver.h"
#include "RefCountPtr.h"
#include "PDFDictionary.h"
#include "PDFArrayDriver.h"
#include "PDFArray.h"
#include "PDFPageInput.h"
#include "PDFStreamInputDriver.h"
#include "ByteReaderDriver.h"
#include "ByteReaderWithPositionDriver.h"
#include "ObjectByteReaderWithPosition.h"
#include "ConstructorsHolder.h"

using namespace v8;



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

DEF_SUBORDINATE_INIT(PDFReaderDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE_EXTERNAL(New);

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
	SET_PROTOTYPE_METHOD(t, "startReadingFromStreamForPlainCopying", StartReadingFromStreamForPlainCopying);
	SET_PROTOTYPE_METHOD(t, "startReadingObjectsFromStream", StartReadingObjectsFromStream);
	SET_PROTOTYPE_METHOD(t, "startReadingObjectsFromStreams", StartReadingObjectsFromStreams);
	SET_PROTOTYPE_METHOD(t, "getParserStream", GetParserStream);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->PDFReader_constructor, t);    
    SET_CONSTRUCTOR_TEMPLATE(holder->PDFReader_constructor_template, t);    
}

METHOD_RETURN_TYPE PDFReaderDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, externalHolder)
    
    PDFReaderDriver* reader = new PDFReaderDriver();

    reader->holder = externalHolder; 
    reader->Wrap(args.This());
    
	SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE PDFReaderDriver::GetPDFLevel(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
    CREATE_ESCAPABLE_SCOPE;

    SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(ObjectWrap::Unwrap<PDFReaderDriver>(args.This())->mPDFReader->GetPDFLevel()))
}

METHOD_RETURN_TYPE PDFReaderDriver::GetPagesCount(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
    CREATE_ESCAPABLE_SCOPE;

	SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(ObjectWrap::Unwrap<PDFReaderDriver>(args.This())->mPDFReader->GetPagesCount()))
}


METHOD_RETURN_TYPE PDFReaderDriver::QueryDictionaryObject(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());

    if(args.Length() != 2 ||
       !reader->holder->IsPDFDictionaryInstance(args[0]) ||
       !args[1]->IsString())
    {
 		THROW_EXCEPTION("Wrong arguments. Provide a dictionary and a string");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    

    PDFDictionaryDriver* dictionary = ObjectWrap::Unwrap<PDFDictionaryDriver>(args[0]->TO_OBJECT());
    
    RefCountPtr<PDFObject> object = reader->mPDFReader->QueryDictionaryObject(dictionary->TheObject.GetPtr(),*UTF_8_VALUE(args[1]->TO_STRING()));
    if(!object)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        
    SET_FUNCTION_RETURN_VALUE(reader->holder->GetInstanceFor(object.GetPtr()))
}

METHOD_RETURN_TYPE PDFReaderDriver::QueryArrayObject(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());

    if(args.Length() != 2 ||
       !reader->holder->IsPDFArrayInstance(args[0]) ||
       !args[1]->IsNumber())
    {
 		THROW_EXCEPTION("Wrong arguments. Provide an array and an index");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    PDFArrayDriver* driver = ObjectWrap::Unwrap<PDFArrayDriver>(args[0]->TO_OBJECT());
    
    RefCountPtr<PDFObject> object = reader->mPDFReader->QueryArrayObject(driver->TheObject.GetPtr(),TO_UINT32(args[1])->Value());
    if(!object)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
    SET_FUNCTION_RETURN_VALUE(reader->holder->GetInstanceFor(object.GetPtr()))
}

METHOD_RETURN_TYPE PDFReaderDriver::GetTrailer(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
 
    PDFDictionary* trailer = reader->mPDFReader->GetTrailer();
    
    if(!trailer)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
    SET_FUNCTION_RETURN_VALUE(reader->holder->GetInstanceFor(trailer))
}

PDFHummus::EStatusCode PDFReaderDriver::StartPDFParsing(v8::Local<v8::Object> inStreamObject,const PDFParsingOptions& inParsingOptions)
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
    return mPDFReader->StartPDFParsing(mReadStreamProxy,inParsingOptions);
}

PDFHummus::EStatusCode PDFReaderDriver::StartPDFParsing(const std::string& inParsedFilePath,const PDFParsingOptions& inParsingOptions)
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
    return mPDFReader->StartPDFParsing(mPDFFile.GetInputStream(),inParsingOptions);
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
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    
    RefCountPtr<PDFObject> newObject = reader->mPDFReader->ParseNewObject(TO_UINT32(args[0])->Value());
    
    if(!newObject)
    {
 		THROW_EXCEPTION("Unable to read object. Most probably object ID is wrong (or some file read issue...but i'd first check that ID. if i were you)");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    SET_FUNCTION_RETURN_VALUE(reader->holder->GetInstanceFor(newObject.GetPtr()))
}

METHOD_RETURN_TYPE PDFReaderDriver::GetPageObjectID(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
 		THROW_EXCEPTION("Wrong arguments. Provide a page index");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    
    SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(reader->mPDFReader->GetPageObjectID(TO_UINT32(args[0])->Value())))
}


METHOD_RETURN_TYPE PDFReaderDriver::ParsePageDictionary(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
 		THROW_EXCEPTION("Wrong arguments. Provide a page index");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    
    RefCountPtr<PDFDictionary> newObject = reader->mPDFReader->ParsePage(TO_UINT32(args[0])->Value());
    
    if(!newObject)
    {
 		THROW_EXCEPTION("Unable to read page, parhaps page index is wrong");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    else {
        SET_FUNCTION_RETURN_VALUE(reader->holder->GetInstanceFor(newObject.GetPtr()))
    }
}

METHOD_RETURN_TYPE PDFReaderDriver::ParsePage(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
 		THROW_EXCEPTION("Wrong arguments. Provide a page index");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    
    RefCountPtr<PDFDictionary> newObject = reader->mPDFReader->ParsePage(TO_UINT32(args[0])->Value());
    
    if(!newObject)
    {
 		THROW_EXCEPTION("Unable to read page, page index is wrong or page is null");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    else {
        Local<Value> newInstance = reader->holder->GetNewPDFPageInput(args);
        ObjectWrap::Unwrap<PDFPageInputDriver>(newInstance->TO_OBJECT())->PageInput = new PDFPageInput(reader->mPDFReader,newObject);
        ObjectWrap::Unwrap<PDFPageInputDriver>(newInstance->TO_OBJECT())->PageInputDictionary = newObject.GetPtr();
        SET_FUNCTION_RETURN_VALUE(newInstance)
    }
}

METHOD_RETURN_TYPE PDFReaderDriver::GetObjectsCount(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    
    SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(ObjectWrap::Unwrap<PDFReaderDriver>(args.This())->mPDFReader->GetObjectsCount()))
}

METHOD_RETURN_TYPE PDFReaderDriver::IsEncrypted(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    
    SET_FUNCTION_RETURN_VALUE(NEW_BOOLEAN(ObjectWrap::Unwrap<PDFReaderDriver>(args.This())->mPDFReader->IsEncrypted()))
}

METHOD_RETURN_TYPE PDFReaderDriver::GetXrefSize(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    
    SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(ObjectWrap::Unwrap<PDFReaderDriver>(args.This())->mPDFReader->GetXrefSize()))
}

METHOD_RETURN_TYPE PDFReaderDriver::GetXrefEntry(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
 		THROW_EXCEPTION("Wrong arguments. Provide an Object ID");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());
    
    XrefEntryInput* xrefEntry = reader->mPDFReader->GetXrefEntry(TO_UINT32(args[0])->Value());
    if(!xrefEntry)
    {
 		THROW_EXCEPTION("Unable to read object xref entry, page index is wrong or page is null");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)   
    }
    
    Local<Object> anObject = NEW_OBJECT;
    
    anObject->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("objectPosition"),NEW_NUMBER(xrefEntry->mObjectPosition));
    anObject->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("revision"),NEW_NUMBER(xrefEntry->mRivision));
    anObject->Set(GET_CURRENT_CONTEXT, NEW_SYMBOL("type"),NEW_NUMBER(xrefEntry->mType));

    SET_FUNCTION_RETURN_VALUE(anObject)
}


METHOD_RETURN_TYPE PDFReaderDriver::GetXrefPosition(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    
    SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(ObjectWrap::Unwrap<PDFReaderDriver>(args.This())->mPDFReader->GetXrefPosition()))
}

METHOD_RETURN_TYPE PDFReaderDriver::StartReadingFromStream(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());

    if(args.Length() != 1 ||
       !reader->holder->IsPDFStreamInputInstance(args[0]))
    {
 		THROW_EXCEPTION("Wrong arguments. provide a PDF stream input");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    PDFStreamInputDriver* streamInput = ObjectWrap::Unwrap<PDFStreamInputDriver>(args[0]->TO_OBJECT());
    
    IByteReader* byteReader = reader->mPDFReader->StartReadingFromStream(streamInput->TheObject.GetPtr());
    
    Local<Value> driver = reader->holder->GetNewByteReader(args);  
    ObjectWrap::Unwrap<ByteReaderDriver>(driver->TO_OBJECT())->SetStream(byteReader,true);
    
    SET_FUNCTION_RETURN_VALUE(driver)
}

METHOD_RETURN_TYPE PDFReaderDriver::StartReadingFromStreamForPlainCopying(const ARGS_TYPE& args) {
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());

    if(args.Length() != 1 ||
       !reader->holder->IsPDFStreamInputInstance(args[0]))
    {
 		THROW_EXCEPTION("Wrong arguments. provide a PDF stream input");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    PDFStreamInputDriver* streamInput = ObjectWrap::Unwrap<PDFStreamInputDriver>(args[0]->TO_OBJECT());
    
    IByteReader* byteReader = reader->mPDFReader->StartReadingFromStreamForPlainCopying(streamInput->TheObject.GetPtr());
    
    Local<Value> driver = reader->holder->GetNewByteReader(args);        
    ObjectWrap::Unwrap<ByteReaderDriver>(driver->TO_OBJECT())->SetStream(byteReader,true);
    
    SET_FUNCTION_RETURN_VALUE(driver)
}
	
METHOD_RETURN_TYPE PDFReaderDriver::StartReadingObjectsFromStream(const ARGS_TYPE& args) {
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());

    if(args.Length() != 1 ||
       !reader->holder->IsPDFStreamInputInstance(args[0]))
    {
 		THROW_EXCEPTION("Wrong arguments. provide a PDF stream input");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    PDFStreamInputDriver* streamInput = ObjectWrap::Unwrap<PDFStreamInputDriver>(args[0]->TO_OBJECT());
    
    PDFObjectParser* objectReader = reader->mPDFReader->StartReadingObjectsFromStream(streamInput->TheObject.GetPtr());
    
    Local<Value> driver = reader->holder->GetNewPDFObjectParser(args);
    ObjectWrap::Unwrap<PDFObjectParserDriver>(driver->TO_OBJECT())->PDFObjectParserInstance= objectReader;
    
    SET_FUNCTION_RETURN_VALUE(driver)
}

METHOD_RETURN_TYPE PDFReaderDriver::StartReadingObjectsFromStreams(const ARGS_TYPE& args) {
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());

    if(args.Length() != 1 ||
       !reader->holder->IsPDFArrayInstance(args[0]))
    {
 		THROW_EXCEPTION("Wrong arguments. provide a PDF array");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    PDFArrayDriver* arrayInput = ObjectWrap::Unwrap<PDFArrayDriver>(args[0]->TO_OBJECT());
    
    PDFObjectParser* objectReader = reader->mPDFReader->StartReadingObjectsFromStreams(arrayInput->TheObject.GetPtr());
    
    Local<Value> driver = reader->holder->GetNewPDFObjectParser(args);
    ObjectWrap::Unwrap<PDFObjectParserDriver>(driver->TO_OBJECT())->PDFObjectParserInstance= objectReader;
    
    SET_FUNCTION_RETURN_VALUE(driver)
}

METHOD_RETURN_TYPE PDFReaderDriver::GetParserStream(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFReaderDriver* reader = ObjectWrap::Unwrap<PDFReaderDriver>(args.This());

    Local<Value> driver = reader->holder->GetNewByteReaderWithPosition(args);
    ObjectWrap::Unwrap<ByteReaderWithPositionDriver>(driver->TO_OBJECT())->SetStream(reader->mPDFReader->GetParserStream(),false);
    
    SET_FUNCTION_RETURN_VALUE(driver)
}

