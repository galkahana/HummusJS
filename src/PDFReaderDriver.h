/*
 Source File : PDFReaderDriver
 
 
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

#include "PDFParser.h"
#include "InputFile.h"


class PDFReaderDriver : public node::ObjectWrap
{
public:
    virtual ~PDFReaderDriver();

    static void Init();
    static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);
    
    // two methods to create parser - owned, from file, or not owned, from another pointer
    PDFHummus::EStatusCode StartPDFParsing(const std::string& inParsedFilePath);
    void SetFromOwnedParser(PDFParser* inParser);
    
private:
    PDFReaderDriver();
    
    static v8::Persistent<v8::Function> constructor;
    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> GetPDFLevel(const v8::Arguments& args);
    static v8::Handle<v8::Value> GetPagesCount(const v8::Arguments& args);
    static v8::Handle<v8::Value> QueryDictionaryObject(const v8::Arguments& args);
    static v8::Handle<v8::Value> QueryArrayObject(const v8::Arguments& args);
    static v8::Handle<v8::Value> GetTrailer(const v8::Arguments& args);
    static v8::Handle<v8::Value> ParseNewObject(const v8::Arguments& args);
    static v8::Handle<v8::Value> GetPageObjectID(const v8::Arguments& args);
    static v8::Handle<v8::Value> ParsePage(const v8::Arguments& args);
	
	
    
    
    bool mOwnsParser;
    PDFParser* mPDFReader;
    InputFile mPDFFile;
};