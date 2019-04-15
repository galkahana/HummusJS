/*
 Source File : PDFStreamDriver.h
 
 
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

class PDFStream;
class ConstructorsHolder;

class PDFStreamDriver : public node::ObjectWrap
{
public:
    virtual ~PDFStreamDriver();

    DEC_SUBORDINATE_INIT(Init)
    
    
    ConstructorsHolder* holder;
    PDFStream* PDFStreamInstance;
    bool mOwns;
    
private:
    PDFStreamDriver();
    
    
	
	static METHOD_RETURN_TYPE New(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetWriteStream(const ARGS_TYPE& args);
};