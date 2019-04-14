/*
 Source File : ObjectsContextDriver
 
 
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

class ObjectsContext;
class ConstructorsHolder;

class ObjectsContextDriver : public node::ObjectWrap
{
public:
    
    DEC_SUBORDINATE_INIT(Init)
    
    ObjectsContext* ObjectsContextInstance;

    ConstructorsHolder* holder;
    
private:
    ObjectsContextDriver();
    
    static METHOD_RETURN_TYPE New(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE AllocateNewObjectID(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE StartDictionary(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE StartArray(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE WriteNumber(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE EndArray(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE EndLine(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE EndDictionary(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE EndIndirectObject(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE WriteIndirectObjectReference(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE StartNewIndirectObject(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE StartModifiedIndirectObject(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE DeleteObject(const ARGS_TYPE& args);

    static METHOD_RETURN_TYPE WriteName(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE WriteLiteralString(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE WriteHexString(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE WriteBoolean(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE WriteKeyword(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE WriteComment(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE SetCompressStreams(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE StartPDFStream(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE StartUnfilteredPDFStream(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE EndPDFStream(const ARGS_TYPE& args);
    
    static METHOD_RETURN_TYPE StartFreeContext(const ARGS_TYPE& args);
    static METHOD_RETURN_TYPE EndFreeContext(const ARGS_TYPE& args);
    
};