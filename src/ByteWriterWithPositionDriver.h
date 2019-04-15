/*
 Source File : ByteWriterWithPositionDriver.h
 
 
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

class IByteWriterWithPosition;

class ByteWriterWithPositionDriver : public node::ObjectWrap
{
public:
    virtual ~ByteWriterWithPositionDriver();
    
    DEC_SUBORDINATE_INIT(Init)
    
    void SetStream(IByteWriterWithPosition* inReader,bool inOwns);
    IByteWriterWithPosition* GetStream();
    
    
private:
    ByteWriterWithPositionDriver();
    
    IByteWriterWithPosition* mInstance;
    bool mOwns;
    
    
	static METHOD_RETURN_TYPE New(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE Write(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetCurrentPosition(const ARGS_TYPE& args);
};