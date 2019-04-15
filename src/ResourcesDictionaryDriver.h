/*
 Source File : ResourcesDictionaryDriver.h
 
 
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

class ResourcesDictionary;
class ConstructorsHolder;

class ResourcesDictionaryDriver : public node::ObjectWrap
{
public:
    DEC_SUBORDINATE_INIT(Init)
    
    // set externally
    ResourcesDictionary* ResourcesDictionaryInstance;
    
    ConstructorsHolder* holder;
private:
    ResourcesDictionaryDriver();
    
    static METHOD_RETURN_TYPE New(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE AddFormXObjectMapping(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE AddImageXObjectMapping(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE AddProcsetResource(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE AddExtGStateMapping(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE AddFontMapping(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE AddColorSpaceMapping(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE AddPatternMapping(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE AddPropertyMapping(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE AddXObjectMapping(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE AddShadingMapping(const ARGS_TYPE& args);

    
    
    
    
    
    

};