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

#include "nodes.h"

#include "PDFParser.h"
#include "InputFile.h"

class ObjectByteReaderWithPosition;
class ConstructorsHolder;

class PDFReaderDriver : public node::ObjectWrap
{
public:
    virtual ~PDFReaderDriver();

    DEC_SUBORDINATE_INIT(Init)

	
    
    // two methods to create parser - owned, from file, or not owned, from another pointer
    PDFHummus::EStatusCode StartPDFParsing(const std::string& inParsedFilePath,const PDFParsingOptions& inParsingOptions);
    PDFHummus::EStatusCode StartPDFParsing(v8::Local<v8::Object> inStreamObject,const PDFParsingOptions& inParsingOptions);
    void SetFromOwnedParser(PDFParser* inParser);
    PDFParser* GetParser();
    

	ConstructorsHolder* holder;
private:
    PDFReaderDriver();
    
    
	static METHOD_RETURN_TYPE New(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetPDFLevel(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetPagesCount(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE QueryDictionaryObject(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE QueryArrayObject(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetTrailer(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE ParseNewObject(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetPageObjectID(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE ParsePageDictionary(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE ParsePage(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetObjectsCount(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE IsEncrypted(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetXrefSize(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetXrefEntry(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetXrefPosition(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetParserStream(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE StartReadingFromStream(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE StartReadingFromStreamForPlainCopying(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE StartReadingObjectsFromStream(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE StartReadingObjectsFromStreams(const ARGS_TYPE& args);
    
    bool mStartedWithStream;
    ObjectByteReaderWithPosition* mReadStreamProxy;
    bool mOwnsParser;
    PDFParser* mPDFReader;
    InputFile mPDFFile;
};