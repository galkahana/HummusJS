/*
 Source File : PDFPageInput.h
 
 
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

#include "PDFPageInput.h"
#include "PDFParser.h"
#include "PDFArray.h"
#include "Trace.h"
#include "PDFName.h"
#include "ParsedPrimitiveHelper.h"

PDFPageInput::PDFPageInput(PDFParser* inParser,PDFObject* inPageObject):mPageObject(inPageObject)
{
    mParser = inParser;
    AssertPageObjectValid();
}

void PDFPageInput::AssertPageObjectValid()
{
    if(!mPageObject)
        TRACE_LOG("PDFPageInput::AssertPageObjectValid, null page object or not a dictionary");
    
    PDFObjectCastPtr<PDFName> typeObject = mPageObject->QueryDirectObject("Type");
    if(!typeObject || typeObject->GetValue() != "Page")
    {
        TRACE_LOG("PDFPageInput::AssertPageObjectValid, dictionar object provided is NOT a page object");
        mPageObject = NULL;
    }
}

PDFPageInput::PDFPageInput(PDFParser* inParser,const PDFObjectCastPtr<PDFDictionary>& inPageObject)
{
    mParser = inParser;
    mPageObject = inPageObject;
    AssertPageObjectValid();
}

PDFPageInput::PDFPageInput(PDFParser* inParser,const RefCountPtr<PDFDictionary>& inPageObject)
{
    mParser = inParser;
    mPageObject = inPageObject.GetPtr();
    AssertPageObjectValid();
}

PDFPageInput::PDFPageInput(const PDFPageInput& inOtherPage)
{
    mParser = inOtherPage.mParser;
    mPageObject = inOtherPage.mPageObject;
    AssertPageObjectValid();
}

PDFPageInput::~PDFPageInput()
{
}

bool PDFPageInput::operator!()
{
    return !mPageObject;
}

PDFRectangle PDFPageInput::GetMediaBox()
{
    PDFRectangle result;
    
    PDFObjectCastPtr<PDFArray> mediaBox(QueryInheritedValue(mPageObject.GetPtr(),"MediaBox"));
    if(!mediaBox || mediaBox->GetLength() != 4)
    {
        TRACE_LOG("PDFPageInput::GetMediaBox, Exception, pdf page does not have correct media box. defaulting to A4");
        result = PDFRectangle(0,0,595,842);
    }
    else
    {
        SetPDFRectangleFromPDFArray(mediaBox.GetPtr(),result);
    }  
    
    return result;
}

PDFRectangle PDFPageInput::GetCropBox()
{
    PDFRectangle result;
    PDFObjectCastPtr<PDFArray> cropBox(QueryInheritedValue(mPageObject.GetPtr(),"CropBox"));
    
    if(!cropBox || cropBox->GetLength() != 4)
        result = GetMediaBox();
    else
        SetPDFRectangleFromPDFArray(cropBox.GetPtr(),result);
    return result;
}

PDFRectangle PDFPageInput::GetTrimBox()
{
    return GetBoxAndDefaultWithCrop("TrimBox");
}

PDFRectangle PDFPageInput::GetBoxAndDefaultWithCrop(const std::string& inBoxName)
{
    PDFRectangle result;
    PDFObjectCastPtr<PDFArray> aBox(QueryInheritedValue(mPageObject.GetPtr(),inBoxName));
    
    if(!aBox || aBox->GetLength() != 4)
        result = GetCropBox();
    else
        SetPDFRectangleFromPDFArray(aBox.GetPtr(),result);
    return result;        
}

PDFRectangle PDFPageInput::GetBleedBox()
{
    return GetBoxAndDefaultWithCrop("BleedBox");
}

PDFRectangle PDFPageInput::GetArtBox()
{
    return GetBoxAndDefaultWithCrop("ArtBox");
}


static const std::string scParent = "Parent";
PDFObject* PDFPageInput::QueryInheritedValue(PDFDictionary* inDictionary,const std::string& inName)
{
	if(inDictionary->Exists(inName))
	{
		return mParser->QueryDictionaryObject(inDictionary,inName);
	}
	else if(inDictionary->Exists(scParent))
	{
		PDFObjectCastPtr<PDFDictionary> parent(mParser->QueryDictionaryObject(inDictionary,scParent));
		if(!parent)
			return NULL;
		return QueryInheritedValue(parent.GetPtr(),inName);
	}
	else
		return NULL;
}

void PDFPageInput::SetPDFRectangleFromPDFArray(PDFArray* inPDFArray,PDFRectangle& outPDFRectangle)
{
	RefCountPtr<PDFObject> lowerLeftX(inPDFArray->QueryObject(0));
	RefCountPtr<PDFObject> lowerLeftY(inPDFArray->QueryObject(1));
	RefCountPtr<PDFObject> upperRightX(inPDFArray->QueryObject(2));
	RefCountPtr<PDFObject> upperRightY(inPDFArray->QueryObject(3));
	
	outPDFRectangle.LowerLeftX = ParsedPrimitiveHelper(lowerLeftX.GetPtr()).GetAsDouble();
	outPDFRectangle.LowerLeftY = ParsedPrimitiveHelper(lowerLeftY.GetPtr()).GetAsDouble();
	outPDFRectangle.UpperRightX = ParsedPrimitiveHelper(upperRightX.GetPtr()).GetAsDouble();
	outPDFRectangle.UpperRightY = ParsedPrimitiveHelper(upperRightY.GetPtr()).GetAsDouble();
}