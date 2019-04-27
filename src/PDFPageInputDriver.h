/*
 Source File : UsedFontDriver.h
 
 
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

#include "PDFDictionary.h"
#include "PDFObjectCast.h"
#include "PDFRectangle.h"

class PDFPageInput;
class ConstructorsHolder;

class PDFPageInputDriver : public node::ObjectWrap
{
public:
    PDFPageInputDriver();
    virtual ~PDFPageInputDriver();
    
    DEC_SUBORDINATE_INIT(Init)
    
    PDFPageInput* PageInput;
    PDFObjectCastPtr<PDFDictionary> PageInputDictionary;

    ConstructorsHolder* holder;    
private:
    
	static METHOD_RETURN_TYPE New(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetDictionary(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetMediaBox(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetCropBox(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetTrimBox(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetBleedBox(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetArtBox(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE GetRotate(const ARGS_TYPE& args);
    
    static v8::Local<v8::Value> GetArrayForPDFRectangle(const PDFRectangle& inRectangle);
    
};