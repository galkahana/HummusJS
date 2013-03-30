/*
 Source File : DocumentCopyingContextDriver.h
 
 
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

#include <node.h>

class PDFDocumentCopyingContext;

class DocumentCopyingContextDriver : public node::ObjectWrap
{
public:
    
    virtual ~DocumentCopyingContextDriver();
    
    static void Init();
    static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);
    
    PDFDocumentCopyingContext* CopyingContext;
    
private:
    DocumentCopyingContextDriver();
    
    static v8::Persistent<v8::Function> constructor;
    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> CreateFormXObjectFromPDFPage(const v8::Arguments& args);
    static v8::Handle<v8::Value> MergePDFPageToPage(const v8::Arguments& args);
    static v8::Handle<v8::Value> AppendPDFPageFromPDF(const v8::Arguments& args);
    static v8::Handle<v8::Value> MergePDFPageToFormXObject(const v8::Arguments& args);
    
    
};
