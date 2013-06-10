/*
 Source File : ParsedPrimitiveHelper.h
 
 
 Copyright 2012 Gal Kahana PDFWriter
 
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

/*
 Helper object for reading primitives. pass it to constructor and you can now use it to read primitives easily.
 ownership: always addrefs and releases
 */

#include <string>



class PDFObject;


class ParsedPrimitiveHelper
{
public:
    ParsedPrimitiveHelper(PDFObject* inObject);
    ~ParsedPrimitiveHelper();
    
    // numbers
    double GetAsDouble();
    long long GetAsInteger();
    bool IsNumber();
    
    // strings (relevant for: names, literal string, hex string, real, integer, symbol, boolean)
    std::string ToString();
    
private:
    
    PDFObject* mWrappedObject;
};