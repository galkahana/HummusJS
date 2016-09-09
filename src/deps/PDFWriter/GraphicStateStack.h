/*
   Source File : GraphicStateStack.h


   Copyright 2011 Gal Kahana PDFWriter

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

#include "GraphicState.h"
#include "EStatusCode.h"
#include <list>

typedef std::list<GraphicState> GraphicStateList;

class GraphicStateStack
{
public:
	GraphicStateStack(void);
	~GraphicStateStack(void);

	// push one level. following a "gsave" command, normally. new state copies old state variables
	void Push();

	// pop one level. following a "grestore" command. returns error if stack is underflow.
	PDFHummus::EStatusCode Pop();

	GraphicState& GetCurrentState();

private:
	GraphicStateList mGraphicStateStack;
};
