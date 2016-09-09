/*
   Source File : GraphicStateStack.cpp


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
#include "GraphicStateStack.h"
#include "Trace.h"


using namespace PDFHummus;

GraphicStateStack::GraphicStateStack(void)
{
	mGraphicStateStack.push_back(GraphicState());
}

GraphicStateStack::~GraphicStateStack(void)
{
}

void GraphicStateStack::Push()
{
	GraphicState newState;
	
	newState = mGraphicStateStack.back(); // there's always at least one - which is the initial state
	mGraphicStateStack.push_back(newState);
}

EStatusCode GraphicStateStack::Pop()
{
	if(mGraphicStateStack.size() == 1)
	{
		TRACE_LOG("GraphicStateStack::Pop, exception. stack underflow, reached to the initial state");
		return PDFHummus::eFailure;
	}
	else
	{
		mGraphicStateStack.pop_back();
		return PDFHummus::eSuccess;
	}
}

GraphicState& GraphicStateStack::GetCurrentState()
{
	return mGraphicStateStack.back();
}