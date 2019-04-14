/*
 Source File : DocumentCopyingContextDriver.h
 
 
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

class PDFDocumentCopyingContext;
class IByteReaderWithPosition;
class ConstructorsHolder;

class DocumentCopyingContextDriver : public node::ObjectWrap
{
public:
    
    virtual ~DocumentCopyingContextDriver();
    
    DEC_SUBORDINATE_INIT(Init)
    
    PDFDocumentCopyingContext* CopyingContext;
    // member holding an optional stream pointer, held by copying context for the sake of final release
    IByteReaderWithPosition* ReadStreamProxy;
    
	ConstructorsHolder* holder;
private:
    DocumentCopyingContextDriver();
    
	static METHOD_RETURN_TYPE New(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE CreateFormXObjectFromPDFPage(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE MergePDFPageToPage(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE AppendPDFPageFromPDF(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE MergePDFPageToFormXObject(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetSourceDocumentParser(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE CopyDirectObjectAsIs(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE CopyObject(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE CopyDirectObjectWithDeepCopy(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE CopyNewObjectsForDirectObject(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetCopiedObjectID(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetCopiedObjects(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE ReplaceSourceObjects(const ARGS_TYPE& args);
	static METHOD_RETURN_TYPE GetSourceDocumentStream(const ARGS_TYPE& args);
    
};
