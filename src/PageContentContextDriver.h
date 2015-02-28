/*
 Source File : PageContentContextDriver.h
 
 
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

#include "AbstractContentContextDriver.h"

class PDFWriterDriver;
class PageContentContext;

class PageContentContextDriver : public AbstractContentContextDriver
{
public:
    
    virtual ~PageContentContextDriver();
    
    static void Init();
	static METHOD_RETURN_TYPE NewInstance(const ARGS_TYPE& args);
	static v8::Handle<v8::Value> GetNewInstance(const ARGS_TYPE& args);
    static bool HasInstance(v8::Handle<v8::Value> inObject);
    
    void SetPDFWriter(PDFWriterDriver* inDriver);
    
    PageContentContext* ContentContext;

    virtual PDFWriterDriver* GetPDFWriter();
private:
    PDFWriterDriver* mPDFWriterDriver;

    
    PageContentContextDriver();

    virtual AbstractContentContext* GetContext();
    virtual void ScheduleImageWrite(const std::string& inImagePath,unsigned long inImageIndex,ObjectIDType inObjectID);
 
    static v8::Persistent<v8::Function> constructor;
    static v8::Persistent<v8::FunctionTemplate> constructor_template;
	static METHOD_RETURN_TYPE New(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetCurrentPageContentStream(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetAssociatedPage(const ARGS_TYPE& args);
    
};
