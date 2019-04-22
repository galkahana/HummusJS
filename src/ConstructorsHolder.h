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
        DECLARE_EXTERNAL_DE_CON_STRUCTORS(ConstructorsHolder)

        // constructors (for creating instances)
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

        // constructors templates (for instance identification)
        v8::Persistent<v8::FunctionTemplate> ByteReader_constructor_template;
        v8::Persistent<v8::FunctionTemplate> ByteReaderWithPosition_constructor_template;
        v8::Persistent<v8::FunctionTemplate> ByteWriter_constructor_template;
        v8::Persistent<v8::FunctionTemplate> ByteWriterWithPosition_constructor_template;
        v8::Persistent<v8::FunctionTemplate> DictionaryContext_constructor_template;
        v8::Persistent<v8::FunctionTemplate> FormXObject_constructor_template;
        v8::Persistent<v8::FunctionTemplate> ImageXObject_constructor_template;
        v8::Persistent<v8::FunctionTemplate> InputFile_constructor_template;
        v8::Persistent<v8::FunctionTemplate> OutputFile_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PageContentContext_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PDFArray_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PDFBoolean_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PDFDictionary_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PDFHexString_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PDFIndirectObjectReference_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PDFInteger_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PDFLiteralString_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PDFName_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PDFNull_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PDFObjectParser_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PDFPage_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PDFPageModifier_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PDFReader_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PDFReal_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PDFStream_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PDFStreamInput_constructor_template;
        v8::Persistent<v8::FunctionTemplate> PDFSymbol_constructor_template;
        v8::Persistent<v8::FunctionTemplate> UsedFont_constructor_template;        

        // instance creation
        v8::Local<v8::Value> GetNewPDFWriter(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewByteReader(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewPDFReader(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewDocumentCopyingContext(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewByteWriterWithPosition(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewOutputFile(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewObjectsContext(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewByteWriter(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewPDFStream(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewFormXObject(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewPageContentContext(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewByteReaderWithPosition(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewInputFile(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewDictionaryContext(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewDocumentContext(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewInfoDictionary(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewPDFDate(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewImageXObject(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewPDFArray();
        v8::Local<v8::Value> GetNewPDFBoolean();
        v8::Local<v8::Value> GetNewPDFLiteralString();
        v8::Local<v8::Value> GetNewPDFHexString();
        v8::Local<v8::Value> GetNewPDFNull();
        v8::Local<v8::Value> GetNewPDFName();
        v8::Local<v8::Value> GetNewPDFInteger();
        v8::Local<v8::Value> GetNewPDFReal();
        v8::Local<v8::Value> GetNewPDFDictionary();
        v8::Local<v8::Value> GetNewPDFIndirectObjectReference();
        v8::Local<v8::Value> GetNewPDFStreamInput();
        v8::Local<v8::Value> GetNewPDFSymbol();
        v8::Local<v8::Value> GetNewPDFObjectParser(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewPDFPageInput(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewPDFPageModifier(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewResourcesDictionary(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewUsedFont(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewXObjectContentContext(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewPDFTextString(const ARGS_TYPE& args);
        v8::Local<v8::Value> GetNewPDFPage(const ARGS_TYPE& args);


        // instance creation for objects
        v8::Local<v8::Value> GetInstanceFor(DictionaryContext* inDictionaryContextInstance);   
        v8::Local<v8::Value> GetInstanceFor(PDFObject* inObject);
        v8::Local<v8::Value> GetInstanceFor(PDFPage* inPage);
        v8::Local<v8::Value> GetInstanceFor(ResourcesDictionary* inResourcesDictionary);

        // instance identification
        bool IsByteReaderInstance(v8::Local<v8::Value> inObject);
        bool IsByteReaderWithPositionInstance(v8::Local<v8::Value> inObject);
        bool IsByteWriterInstance(v8::Local<v8::Value> inObject);
        bool IsByteWriterWithPositionInstance(v8::Local<v8::Value> inObject);
        bool IsDictionaryContextInstance(v8::Local<v8::Value> inObject);
        bool IsFormXObjectInstance(v8::Local<v8::Value> inObject);
        bool IsImageXObjectInstance(v8::Local<v8::Value> inObject);
        bool IsInputFileInstance(v8::Local<v8::Value> inObject);
        bool IsOutputFileInstance(v8::Local<v8::Value> inObject);
        bool IsPageContentContextInstance(v8::Local<v8::Value> inObject);
        bool IsPDFArrayInstance(v8::Local<v8::Value> inObject);
        bool IsPDFBooleanInstance(v8::Local<v8::Value> inObject);
        bool IsPDFDictionaryInstance(v8::Local<v8::Value> inObject);
        bool IsPDFHexStringInstance(v8::Local<v8::Value> inObject);
        bool IsPDFIndirectObjectReferenceInstance(v8::Local<v8::Value> inObject);
        bool IsPDFIntegerInstance(v8::Local<v8::Value> inObject);
        bool IsPDFLiteralStringInstance(v8::Local<v8::Value> inObject);
        bool IsPDFNameInstance(v8::Local<v8::Value> inObject);
        bool IsPDFNullInstance(v8::Local<v8::Value> inObject);
        bool IsPDFObjectParserInstance(v8::Local<v8::Value> inObject);
        bool IsPDFPageInstance(v8::Local<v8::Value> inObject);
        bool IsPDFPageModifierInstance(v8::Local<v8::Value> inObject);
        bool IsPDFReaderInstance(v8::Local<v8::Value> inObject);
        bool IsPDFRealInstance(v8::Local<v8::Value> inObject);
        bool IsPDFStreamInstance(v8::Local<v8::Value> inObject);
        bool IsPDFStreamInputInstance(v8::Local<v8::Value> inObject);
        bool IsPDFSymbolInstance(v8::Local<v8::Value> inObject);
        bool IsUsedFontInstance(v8::Local<v8::Value> inObject);        
        
    private:
        static bool IsInstance(v8::Local<v8::Value> inObject, const v8::Persistent<v8::FunctionTemplate>& constructor_template);
};