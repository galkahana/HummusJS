/*
 Source File : PDFObjectDriver.h
 
 
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


class PDFObject;
class ConstructorsHolder;

class PDFObjectDriver : public node::ObjectWrap
{
public:
    
    
    static void Init(v8::Local<v8::FunctionTemplate>& ioDriverTemplate);
    
    virtual PDFObject* GetObject() = 0;

	ConstructorsHolder* holder;

private:
    
    
	static METHOD_RETURN_TYPE GetType(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE ToPDFIndirectObjectReference(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE ToPDFArray(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE ToPDFDictionary(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE ToPDFStream(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE ToPDFBoolean(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE ToPDFLiteralString(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE ToPDFHexString(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE ToPDFNull(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE ToPDFName(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE ToPDFInteger(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE ToPDFReal(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE ToPDFSymbol(const ARGS_TYPE& args);
    // return number for pdf numbers (real and integer) and null for non numbers
	static METHOD_RETURN_TYPE ToNumber(const ARGS_TYPE& args);
    // strings (relevant for: names, literal string, hex string, real, integer, symbol, boolean)
	static METHOD_RETURN_TYPE ToString(const ARGS_TYPE& args);
    
};
