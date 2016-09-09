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

#include "ParsedPrimitiveHelper.h"
#include "PDFObject.h"
#include "BoxingBase.h"
#include "PDFReal.h"
#include "PDFInteger.h"
#include "PDFName.h"
#include "PDFSymbol.h"
#include "PDFLiteralString.h"
#include "PDFHexString.h"
#include "PDFBoolean.h"

ParsedPrimitiveHelper::ParsedPrimitiveHelper(PDFObject* inObject)
{
    mWrappedObject = inObject;
    mWrappedObject->AddRef();
}

ParsedPrimitiveHelper::~ParsedPrimitiveHelper()
{
    mWrappedObject->Release();
}

double ParsedPrimitiveHelper::GetAsDouble()
{
	if(mWrappedObject->GetType() == PDFObject::ePDFObjectInteger)
	{
		PDFInteger* anInteger = (PDFInteger*)mWrappedObject;
		return (double)anInteger->GetValue();
	}
	else if(mWrappedObject->GetType() == PDFObject::ePDFObjectReal)
	{
		PDFReal* aReal = (PDFReal*)mWrappedObject;
		return aReal->GetValue();
	}
	else
		return 0;
}

long long ParsedPrimitiveHelper::GetAsInteger()
{
	if(mWrappedObject->GetType() == PDFObject::ePDFObjectInteger)
	{
		PDFInteger* anInteger = (PDFInteger*)mWrappedObject;
		return anInteger->GetValue();
	}
	else if(mWrappedObject->GetType() == PDFObject::ePDFObjectReal)
	{
		PDFReal* aReal = (PDFReal*)mWrappedObject;
		return (long long)aReal->GetValue();
	}
	else
		return 0;
}

bool ParsedPrimitiveHelper::IsNumber()
{
    return 
        mWrappedObject->GetType() == PDFObject::ePDFObjectInteger ||
        mWrappedObject->GetType() == PDFObject::ePDFObjectReal;
    
}

typedef BoxingBaseWithRW<long long> LongLong;

// strings (relevant for: names, literal string, hex string, real, integer, symbol, boolean)
std::string ParsedPrimitiveHelper::ToString()
{
    std::string result;
    
    switch(mWrappedObject->GetType())
    {
        case PDFObject::ePDFObjectName:
            result = ((PDFName*)mWrappedObject)->GetValue();
            break;
        case PDFObject::ePDFObjectLiteralString:
            result = ((PDFLiteralString*)mWrappedObject)->GetValue();
            break;
        case PDFObject::ePDFObjectHexString:
            result = ((PDFHexString*)mWrappedObject)->GetValue();
            break;
        case PDFObject::ePDFObjectReal:
            result = Double(((PDFReal*)mWrappedObject)->GetValue()).ToString();
            break;
        case PDFObject::ePDFObjectInteger:
            result = LongLong(((PDFInteger*)mWrappedObject)->GetValue()).ToString();
            break;
        case PDFObject::ePDFObjectSymbol:
            result = ((PDFSymbol*)mWrappedObject)->GetValue();
            break;
        case PDFObject::ePDFObjectBoolean:
            result = ((PDFBoolean*)mWrappedObject)->GetValue() ? "true":"false";
            break;
        default:
            result = PDFObject::scPDFObjectTypeLabel(mWrappedObject->GetType());
    }
    return result;
}

