/*
 Source File : PDFPageModifierDriver.h
 
 
 Copyright 2016 Gal Kahana HummusJS
 
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

class PDFModifiedPage;
class PDFWriter;
class ConstructorsHolder;

class PDFPageModifierDriver : public node::ObjectWrap
{
public:
    virtual ~PDFPageModifierDriver();
    
	DEC_SUBORDINATE_INIT(Init)
    

	ConstructorsHolder* holder;
   
private:
    PDFPageModifierDriver(PDFWriter* inWriter,unsigned long inPageIndex,bool inEnsureContentEncapsulation = false);
    
    PDFModifiedPage* mModifierPageInstance;
    
    
	static METHOD_RETURN_TYPE New(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE StartContext(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetContext(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE EndContext(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE AttachURLLinktoCurrentPage(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE WritePage(const ARGS_TYPE& args);

};