/*
 Source File : PageContentContextDriver.cpp
 
 
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
#include "XObjectContentContextDriver.h"
#include "XObjectContentContext.h"
#include "IFormEndWritingTask.h"
#include "PDFWriterDriver.h"
#include "DocumentContext.h"
#include "ConstructorsHolder.h"

using namespace v8;

XObjectContentContextDriver::XObjectContentContextDriver()
{
    // initially null, set by external pdfwriter
    ContentContext = NULL;
    FormOfContext = NULL;
}

DEF_SUBORDINATE_INIT(XObjectContentContextDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE_EXTERNAL(New);

	t->SetClassName(NEW_STRING("XObjectContentContext"));
	t->InstanceTemplate()->SetInternalFieldCount(1);
	AbstractContentContextDriver::Init(t);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->XObjectContentContext_constructor, t);   
}

METHOD_RETURN_TYPE XObjectContentContextDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, externalHolder)

    XObjectContentContextDriver* form = new XObjectContentContextDriver();
    form->holder = externalHolder;
    form->Wrap(args.This());
    
	SET_FUNCTION_RETURN_VALUE(args.This())
}

AbstractContentContext* XObjectContentContextDriver::GetContext()
{
    return ContentContext;
}


