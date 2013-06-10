/*
   Source File : PDFObject.h


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

#include "RefCountObject.h"

#include <string>



class PDFObject : public RefCountObject
{
public:
    
    enum EPDFObjectType
    {
        ePDFObjectBoolean,
        ePDFObjectLiteralString,
        ePDFObjectHexString,
        ePDFObjectNull,
        ePDFObjectName,
        ePDFObjectInteger,
        ePDFObjectReal,
        ePDFObjectArray,
        ePDFObjectDictionary,
        ePDFObjectIndirectObjectReference,
        ePDFObjectStream,
        ePDFObjectSymbol // symbol is parallel to unkown. mostly be a keyword, or simply a mistake in the file
    };    
    
	PDFObject(EPDFObjectType inType); 
	PDFObject(int inType); 
	virtual ~PDFObject(void);

	EPDFObjectType GetType();
    
    static const char* scPDFObjectTypeLabel[];

private:
	EPDFObjectType mType;
};
