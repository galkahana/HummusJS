/*
   Source File : PDFDictionary.cpp


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
#include "PDFDictionary.h"

PDFDictionary::PDFDictionary(void) : PDFObject(eType)
{ 
}

PDFDictionary::~PDFDictionary(void)
{
	PDFNameToPDFObjectMap::iterator	it = mValues.begin();

	for(; it != mValues.end(); ++it)
	{
		it->first->Release();
		it->second->Release();
	}
}

PDFObject* PDFDictionary::QueryDirectObject(std::string inName)
{
	PDFName key(inName);
	PDFNameToPDFObjectMap::iterator it = mValues.find(&key);

	if(it == mValues.end())
	{
		return NULL;
	}
	else
	{
		it->second->AddRef();
		return it->second;
	}
}

void PDFDictionary::Insert(PDFName* inKeyObject, PDFObject* inValueObject)
{
	inKeyObject->AddRef();
	inValueObject->AddRef();

	mValues.insert(PDFNameToPDFObjectMap::value_type(inKeyObject,inValueObject));
}


bool PDFDictionary::Exists(std::string inName)
{
	PDFName key(inName);
	return mValues.find(&key) != mValues.end();
}

MapIterator<PDFNameToPDFObjectMap> PDFDictionary::GetIterator()
{
	return MapIterator<PDFNameToPDFObjectMap>(mValues);
}