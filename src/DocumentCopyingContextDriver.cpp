/*
 Source File : DocumentCopyingContextDriver.cpp
 
 
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
#include "DocumentCopyingContextDriver.h"
#include "PDFDocumentCopyingContext.h"
#include "PDFPageDriver.h"
#include "FormXObjectDriver.h"
#include "PDFReaderDriver.h"
#include "PDFObjectDriver.h"
#include "BoxingBase.h"
#include "ByteReaderWithPositionDriver.h"
#include "IByteReaderWithPosition.h"

using namespace v8;
Persistent<Function> DocumentCopyingContextDriver::constructor;

DocumentCopyingContextDriver::~DocumentCopyingContextDriver()
{
    delete CopyingContext;
    delete ReadStreamProxy;
}

DocumentCopyingContextDriver::DocumentCopyingContextDriver()
{
    // initially null, set by external pdfwriter
    CopyingContext = NULL;
    ReadStreamProxy = NULL;
}

void DocumentCopyingContextDriver::Init()
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("DocumentCopyingContext"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "createFormXObjectFromPDFPage", CreateFormXObjectFromPDFPage);
	SET_PROTOTYPE_METHOD(t, "mergePDFPageToPage", MergePDFPageToPage);
	SET_PROTOTYPE_METHOD(t, "appendPDFPageFromPDF", AppendPDFPageFromPDF);
	SET_PROTOTYPE_METHOD(t, "mergePDFPageToFormXObject", MergePDFPageToFormXObject);
	SET_PROTOTYPE_METHOD(t, "getSourceDocumentParser", GetSourceDocumentParser);
	SET_PROTOTYPE_METHOD(t, "copyDirectObjectAsIs", CopyDirectObjectAsIs);
	SET_PROTOTYPE_METHOD(t, "copyObject", CopyObject);
	SET_PROTOTYPE_METHOD(t, "copyDirectObjectWithDeepCopy", CopyDirectObjectWithDeepCopy);
	SET_PROTOTYPE_METHOD(t, "copyNewObjectsForDirectObject", CopyNewObjectsForDirectObject);
	SET_PROTOTYPE_METHOD(t, "getCopiedObjectID", GetCopiedObjectID);
	SET_PROTOTYPE_METHOD(t, "getCopiedObjects", GetCopiedObjects);
	SET_PROTOTYPE_METHOD(t, "replaceSourceObjects", ReplaceSourceObjects);
	SET_PROTOTYPE_METHOD(t, "getSourceDocumentStream", GetSourceDocumentStream);
	SET_CONSTRUCTOR(constructor, t);
}

METHOD_RETURN_TYPE DocumentCopyingContextDriver::NewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	SET_FUNCTION_RETURN_VALUE(instance)
}

v8::Handle<v8::Value> DocumentCopyingContextDriver::GetNewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

METHOD_RETURN_TYPE DocumentCopyingContextDriver::New(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    DocumentCopyingContextDriver* copyingContext = new DocumentCopyingContextDriver();
    copyingContext->Wrap(args.This());
    
	SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE DocumentCopyingContextDriver::CreateFormXObjectFromPDFPage(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		THROW_EXCEPTION("copying context object not initialized, create using pdfWriter.createPDFCopyingContext");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() < 2 ||
       args.Length() > 3 ||
       !args[0]->IsNumber() ||
       (!args[1]->IsNumber() && !args[1]->IsArray()) ||
       (args.Length() == 3 && !args[2]->IsArray()))
    {
		THROW_EXCEPTION("Wrong arguments. provide 2 or 3 arugments, where the first is a 0 based page index, and the second is a EPDFPageBox enumeration value or a 4 numbers array defining an box. a 3rd parameter may be provided to deisgnate the result form matrix");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
 
    double matrixBuffer[6];
    double* transformationMatrix = NULL;
    
    if(args.Length() == 3)
    {
        Handle<Object> matrixArray = args[2]->ToObject();
        if(matrixArray->Get(v8::NEW_STRING("length"))->ToObject()->Uint32Value() != 6)
        {
            THROW_EXCEPTION("matrix array should be 6 numbers long");
            SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        }
        
        for(int i=0;i<6;++i)
            matrixBuffer[i] = TO_NUMBER(matrixArray->Get(i))->Value();
        transformationMatrix = matrixBuffer;
    }
    
    
    EStatusCodeAndObjectIDType result;
    
    if(args[0]->IsNumber())
    {
        result = copyingContextDriver->CopyingContext->CreateFormXObjectFromPDFPage(TO_NUMBER(args[0])->Uint32Value(),
                                                                       (EPDFPageBox)TO_NUMBER(args[1])->Uint32Value(),
                                                                                    transformationMatrix);
    }
    else
    {
        Handle<Object> boxArray = args[1]->ToObject();
        if(boxArray->Get(v8::NEW_STRING("length"))->ToObject()->Uint32Value() != 4)
        {
            THROW_EXCEPTION("box dimensions array should be 4 numbers long");
            SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        }
        
        PDFRectangle box(TO_NUMBER(boxArray->Get(0))->Value(),
                         TO_NUMBER(boxArray->Get(1))->Value(),
                         TO_NUMBER(boxArray->Get(2))->Value(),
                         TO_NUMBER(boxArray->Get(3))->Value());
        
        result = copyingContextDriver->CopyingContext->CreateFormXObjectFromPDFPage(TO_NUMBER(args[0])->Uint32Value(),
                                                                                    box,
                                                                                    transformationMatrix);

    }
        
    
    if(result.first != eSuccess)
    {
		THROW_EXCEPTION("Unable to create form xobject from PDF page, parhaps the page index does not fit the total pages count");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    Local<Number> idValue = NEW_NUMBER(result.second);
    
    SET_FUNCTION_RETURN_VALUE(idValue)
}

METHOD_RETURN_TYPE DocumentCopyingContextDriver::MergePDFPageToPage(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		THROW_EXCEPTION("copying context object not initialized, create using pdfWriter.createPDFCopyingContext");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() != 2 ||
       !PDFPageDriver::HasInstance(args[0]) ||
       !args[1]->IsNumber())
    {
		THROW_EXCEPTION("Wrong arguments. provide 2 arugments, where the first is a page, and the second is a page index to merge");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    EStatusCode status = copyingContextDriver->CopyingContext->MergePDFPageToPage(
                                                                                  ObjectWrap::Unwrap<PDFPageDriver>(args[0]->ToObject())->GetPage(),
                                                                                  TO_NUMBER(args[1])->Uint32Value());
    
    if(status != eSuccess)
		THROW_EXCEPTION("Unable to merge page index to page. parhaps the page index is wrong");
    SET_FUNCTION_RETURN_VALUE(UNDEFINED)

}

METHOD_RETURN_TYPE DocumentCopyingContextDriver::AppendPDFPageFromPDF(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		THROW_EXCEPTION("copying context object not initialized, create using pdfWriter.createPDFCopyingContext");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong arguments. provide a page index to append");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    EStatusCodeAndObjectIDType result = copyingContextDriver->CopyingContext->AppendPDFPageFromPDF(TO_NUMBER(args[0])->Uint32Value());
    
    if(result.first != eSuccess)
    {
		THROW_EXCEPTION("Unable to append page. parhaps the page index is wrong");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    Local<Number> idValue = NEW_NUMBER(result.second);
    SET_FUNCTION_RETURN_VALUE(idValue)
    
}

METHOD_RETURN_TYPE DocumentCopyingContextDriver::MergePDFPageToFormXObject(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		THROW_EXCEPTION("copying context object not initialized, create using pdfWriter.createPDFCopyingContext");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() != 2 ||
       !FormXObjectDriver::HasInstance(args[0]) ||
       !args[1]->IsNumber())
    {
		THROW_EXCEPTION("Wrong arguments. provide 2 arugments, where the first is a form, and the second is a page index to merge");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    EStatusCode status = copyingContextDriver->CopyingContext->MergePDFPageToFormXObject(
                                                                                  ObjectWrap::Unwrap<FormXObjectDriver>(args[0]->ToObject())->FormXObject,
                                                                                  TO_NUMBER(args[1])->Uint32Value());
    
    if(status != eSuccess)
		THROW_EXCEPTION("Unable to merge page index to form. parhaps the page index is wrong");
    SET_FUNCTION_RETURN_VALUE(UNDEFINED)

}

METHOD_RETURN_TYPE DocumentCopyingContextDriver::GetSourceDocumentParser(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    DocumentCopyingContextDriver* copyingContext = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    Handle<Value> newInstance = PDFReaderDriver::GetNewInstance(args);
    ObjectWrap::Unwrap<PDFReaderDriver>(newInstance->ToObject())->SetFromOwnedParser(copyingContext->CopyingContext->GetSourceDocumentParser());
    SET_FUNCTION_RETURN_VALUE(newInstance)
}

METHOD_RETURN_TYPE DocumentCopyingContextDriver::CopyDirectObjectAsIs(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		THROW_EXCEPTION("copying context object not initialized, create using pdfWriter.createPDFCopyingContext or PDFWriter.createPDFCopyingContextForModifiedFile");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() != 1) // need to sometimes check that this is a PDFObject
    {
		THROW_EXCEPTION("Wrong arguments. provide 1 arugment, which is PDFObject to copy");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    EStatusCode status = copyingContextDriver->CopyingContext->CopyDirectObjectAsIs(ObjectWrap::Unwrap<PDFObjectDriver>(args[0]->ToObject())->GetObject());
    if(status != eSuccess)
		THROW_EXCEPTION("Unable to merge page index to form. parhaps the page index is wrong");
    SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
}

METHOD_RETURN_TYPE DocumentCopyingContextDriver::CopyObject(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		THROW_EXCEPTION("copying context object not initialized, create using pdfWriter.createPDFCopyingContext or PDFWriter.createPDFCopyingContextForModifiedFile");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong arguments. provide 1 arugment, which is object ID of the object to copy");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    EStatusCodeAndObjectIDType result = copyingContextDriver->CopyingContext->CopyObject(TO_NUMBER(args[0])->Uint32Value());
 
     if(result.first != eSuccess)
		THROW_EXCEPTION("unable to copy the object. object id may be wrong");
     SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(result.second))
    
}

METHOD_RETURN_TYPE DocumentCopyingContextDriver::CopyDirectObjectWithDeepCopy(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		THROW_EXCEPTION("copying context object not initialized, create using pdfWriter.createPDFCopyingContext or PDFWriter.createPDFCopyingContextForModifiedFile");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() != 1) // need to sometimes check that this is a PDFObject
    {
		THROW_EXCEPTION("Wrong arguments. provide 1 arugment, which is PDFObject to copy");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    EStatusCodeAndObjectIDTypeList result = copyingContextDriver->CopyingContext->CopyDirectObjectWithDeepCopy(ObjectWrap::Unwrap<PDFObjectDriver>(args[0]->ToObject())->GetObject());
    if(result.first != eSuccess)
		THROW_EXCEPTION("Unable to copy object, parhaps the object id is wrong");

    Local<Array> resultObjectIDs = NEW_ARRAY((unsigned int)result.second.size());
    unsigned int index = 0;
    
    ObjectIDTypeList::iterator it = result.second.begin();
    for(; it != result.second.end();++it)
        resultObjectIDs->Set(NEW_NUMBER(index++),NEW_NUMBER(*it));
    
    SET_FUNCTION_RETURN_VALUE(resultObjectIDs)
}


METHOD_RETURN_TYPE DocumentCopyingContextDriver::CopyNewObjectsForDirectObject(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		THROW_EXCEPTION("copying context object not initialized, create using pdfWriter.createPDFCopyingContext or PDFWriter.createPDFCopyingContextForModifiedFile");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() != 1 ||
       !args[0]->IsArray())
    {
		THROW_EXCEPTION("Wrong arguments. provide 1 arugment, which is an array of object IDs");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    ObjectIDTypeList objectIDs;
    Handle<Object> objectIDsArray = args[0]->ToObject();

    unsigned int length = objectIDsArray->Get(v8::NEW_STRING("length"))->ToObject()->Uint32Value();
    
    for(unsigned int i=0;i <length;++i)
        objectIDs.push_back(TO_NUMBER(objectIDsArray->Get(i))->Uint32Value());
    
    EStatusCode status = copyingContextDriver->CopyingContext->CopyNewObjectsForDirectObject(objectIDs);
    if(status != eSuccess)
		THROW_EXCEPTION("Unable to copy elements");
    SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    
}



METHOD_RETURN_TYPE DocumentCopyingContextDriver::GetCopiedObjectID(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		THROW_EXCEPTION("copying context object not initialized, create using pdfWriter.createPDFCopyingContext or PDFWriter.createPDFCopyingContextForModifiedFile");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() != 1 ||
       !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong arguments. provide 1 arugment, an object ID to check");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
        
    EStatusCodeAndObjectIDType result = copyingContextDriver->CopyingContext->GetCopiedObjectID(TO_NUMBER(args[0])->Uint32Value());
    if(result.first != eSuccess)
		THROW_EXCEPTION("Unable to find element");
    SET_FUNCTION_RETURN_VALUE(NEW_NUMBER(result.second))
    
}

typedef BoxingBaseWithRW<ObjectIDType> ObjectIDTypeObject;

METHOD_RETURN_TYPE DocumentCopyingContextDriver::GetCopiedObjects(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		THROW_EXCEPTION("copying context object not initialized, create using pdfWriter.createPDFCopyingContext or PDFWriter.createPDFCopyingContextForModifiedFile");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }

    // create an object that will serve as the map

    Local<Object> result = NEW_OBJECT;
    
	MapIterator<ObjectIDTypeToObjectIDTypeMap> it = copyingContextDriver->CopyingContext->GetCopiedObjectsMappingIterator();
    
    while(it.MoveNext())
        result->Set(NEW_STRING(ObjectIDTypeObject(it.GetKey()).ToString().c_str()),NEW_NUMBER(it.GetValue()));
    
    SET_FUNCTION_RETURN_VALUE(result)
}

METHOD_RETURN_TYPE DocumentCopyingContextDriver::ReplaceSourceObjects(const ARGS_TYPE& args)
{
    // getting a dictionary mapping source to target object, translating to the C++ map...and on we go
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		THROW_EXCEPTION("copying context object not initialized, create using pdfWriter.createPDFCopyingContext or PDFWriter.createPDFCopyingContextForModifiedFile");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() != 1 ||
       !args[0]->IsObject())
    {
 		THROW_EXCEPTION("Wrong arguments. provide 1 arugment, which is an object mapping source object ids to map to target object IDs");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    // create an object that will serve as the map
    ObjectIDTypeToObjectIDTypeMap resultMap;
    
    Handle<Object> anObject = args[0]->ToObject();
    
    Handle<Array> objectKeys = anObject->GetOwnPropertyNames();
    
    for(unsigned long i=0; i < objectKeys->Length(); ++i)
    {
        Handle<String> key  = objectKeys->Get(NEW_NUMBER(0))->ToString();
        Handle<Value> value = anObject->Get(key);
        
        resultMap.insert(ObjectIDTypeToObjectIDTypeMap::value_type(ObjectIDTypeObject(*String::Utf8Value(key)),TO_NUMBER(value)->Uint32Value()));
        
    }
    
    copyingContextDriver->CopyingContext->ReplaceSourceObjects(resultMap);
    
    SET_FUNCTION_RETURN_VALUE(UNDEFINED)
}

METHOD_RETURN_TYPE DocumentCopyingContextDriver::GetSourceDocumentStream(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    DocumentCopyingContextDriver* copyingContextDriver = ObjectWrap::Unwrap<DocumentCopyingContextDriver>(args.This());
    
    if(!copyingContextDriver->CopyingContext)
    {
		THROW_EXCEPTION("copying context object not initialized, create using pdfWriter.createPDFCopyingContext or PDFWriter.createPDFCopyingContextForModifiedFile");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }

	Handle<Value> resultDriver = ByteReaderWithPositionDriver::GetNewInstance(args);
    
    ObjectWrap::Unwrap<ByteReaderWithPositionDriver>(resultDriver->ToObject())->SetStream(
        copyingContextDriver->CopyingContext->GetSourceDocumentStream(),
        false);
    
    SET_FUNCTION_RETURN_VALUE(resultDriver)
}


