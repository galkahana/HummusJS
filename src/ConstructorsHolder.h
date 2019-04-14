/*
 Source File : ConstructorsHolder.h
 
 
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
#pragma once

#include "nodes.h"

class DictionaryContext;
class PDFObject;
class PDFPage;
class ResourcesDictionary;

class ConstructorsHolder {

    public:
        ConstructorsHolder(v8::Isolate* isolate, v8::Local<v8::Object> exports);
        virtual ~ConstructorsHolder();

        // constructors
        v8::Persistent<v8::Function> PDFWriter_constructor; 
        v8::Persistent<v8::Function> ByteReader_constructor; 
        v8::Persistent<v8::Function> PDFReader_constructor; 
        v8::Persistent<v8::Function> DocumentCopyingContext_constructor;
        v8::Persistent<v8::Function> ByteWriterWithPosition_constructor;
        v8::Persistent<v8::Function> OutputFile_constructor;
        v8::Persistent<v8::Function> ObjectsContext_constructor;
        v8::Persistent<v8::Function> ByteWriter_constructor;
        v8::Persistent<v8::Function> PDFStream_constructor;
        v8::Persistent<v8::Function> FormXObject_constructor;
        v8::Persistent<v8::Function> PageContentContext_constructor;
        v8::Persistent<v8::Function> ByteReaderWithPosition_constructor;
        v8::Persistent<v8::Function> InputFile_constructor;
        v8::Persistent<v8::Function> DictionaryContext_constructor;
        v8::Persistent<v8::Function> DocumentContext_constructor;
        v8::Persistent<v8::Function> InfoDictionary_constructor;
        v8::Persistent<v8::Function> PDFDate_constructor;
        v8::Persistent<v8::Function> ImageXObject_constructor;
        v8::Persistent<v8::Function> PDFArray_constructor;
        v8::Persistent<v8::Function> PDFBoolean_constructor;
        v8::Persistent<v8::Function> PDFLiteralString_constructor;
        v8::Persistent<v8::Function> PDFHexString_constructor;
        v8::Persistent<v8::Function> PDFNull_constructor;
        v8::Persistent<v8::Function> PDFName_constructor;
        v8::Persistent<v8::Function> PDFInteger_constructor;
        v8::Persistent<v8::Function> PDFReal_constructor;
        v8::Persistent<v8::Function> PDFDictionary_constructor;
        v8::Persistent<v8::Function> PDFIndirectObjectReference_constructor;
        v8::Persistent<v8::Function> PDFStreamInput_constructor;
        v8::Persistent<v8::Function> PDFSymbol_constructor;
        v8::Persistent<v8::Function> PDFObjectParser_constructor;
        v8::Persistent<v8::Function> PDFPageInput_constructor;
        v8::Persistent<v8::Function> PDFPageModifier_constructor;
        v8::Persistent<v8::Function> ResourcesDictionary_constructor;
        v8::Persistent<v8::Function> UsedFont_constructor;
        v8::Persistent<v8::Function> XObjectContentContext_constructor;
        v8::Persistent<v8::Function> PDFTextString_constructor;
        v8::Persistent<v8::Function> PDFPage_constructor;


        // instance creation
        v8::Handle<v8::Value> GetNewPDFWriter(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewByteReader(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewPDFReader(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewDocumentCopyingContext(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewByteWriterWithPosition(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewOutputFile(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewObjectsContext(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewByteWriter(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewPDFStream(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewFormXObject(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewPageContentContext(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewByteReaderWithPosition(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewInputFile(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewDictionaryContext(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewDocumentContext(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewInfoDictionary(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewPDFDate(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewImageXObject(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewPDFArray();
        v8::Handle<v8::Value> GetNewPDFBoolean();
        v8::Handle<v8::Value> GetNewPDFLiteralString();
        v8::Handle<v8::Value> GetNewPDFHexString();
        v8::Handle<v8::Value> GetNewPDFNull();
        v8::Handle<v8::Value> GetNewPDFName();
        v8::Handle<v8::Value> GetNewPDFInteger();
        v8::Handle<v8::Value> GetNewPDFReal();
        v8::Handle<v8::Value> GetNewPDFDictionary();
        v8::Handle<v8::Value> GetNewPDFIndirectObjectReference();
        v8::Handle<v8::Value> GetNewPDFStreamInput();
        v8::Handle<v8::Value> GetNewPDFSymbol();
        v8::Handle<v8::Value> GetNewPDFObjectParser(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewPDFPageInput(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewPDFPageModifier(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewResourcesDictionary(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewUsedFont(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewXObjectContentContext(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewPDFTextString(const ARGS_TYPE& args);
        v8::Handle<v8::Value> GetNewPDFPage(const ARGS_TYPE& args);


        // instance creation for objects
        v8::Handle<v8::Value> GetInstanceFor(DictionaryContext* inDictionaryContextInstance);   
        v8::Handle<v8::Value> GetInstanceFor(PDFObject* inObject);
        v8::Handle<v8::Value> GetInstanceFor(PDFPage* inPage);
        v8::Handle<v8::Value> GetInstanceFor(ResourcesDictionary* inResourcesDictionary);
        

    private:
        static void DeleteMe(const v8::WeakCallbackInfo<ConstructorsHolder>& info);

        v8::Persistent<v8::Object> mExports;

};