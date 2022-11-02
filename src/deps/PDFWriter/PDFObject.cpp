/*
   Source File : PDFObject.cpp


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
#include "PDFObject.h"
#include "IDeletable.h"

const char* PDFObject::scPDFObjectTypeLabel(int index) 
{
	static const char* labels[] =
	{
		"Boolean",
		"LiteralString",
		"HexString",
		"Null",
		"Name",
		"Integer",
		"Real",
		"Array",
		"Dictionary",
		"IndirectObjectReference",
		"Stream",
		"Symbol"
	};
	return labels[index];
};

PDFObject::PDFObject(EPDFObjectType inType)
{
	mType = inType;
}

PDFObject::PDFObject(int inType)
{
	mType = (EPDFObjectType)inType;
}


PDFObject::~PDFObject(void)
{
	StringToIDeletable::iterator it = mMetadata.begin();
	for (; it != mMetadata.end(); ++it) {
		it->second->DeleteMe();
	}
	mMetadata.clear();
}

PDFObject::EPDFObjectType PDFObject::GetType()
{
	return mType;
}

void PDFObject::SetMetadata(const std::string& inKey, IDeletable* inValue) {
	// delete old metadata
	DeleteMetadata(inKey);

	mMetadata.insert(StringToIDeletable::value_type(inKey, inValue));
}

IDeletable* PDFObject::GetMetadata(const std::string& inKey) {
	StringToIDeletable::iterator it = mMetadata.find(inKey);
	
	if (it == mMetadata.end()) 
		return NULL;
	else 
		return it->second;
}

IDeletable* PDFObject::DetachMetadata(const std::string& inKey) {
	StringToIDeletable::iterator it = mMetadata.find(inKey);

	if (it == mMetadata.end())
		return NULL;
	else {
		IDeletable* result = it->second;
		mMetadata.erase(it);
		return result;
	}
}

void PDFObject::DeleteMetadata(const std::string& inKey) {
	IDeletable* result = DetachMetadata(inKey);
	if(result)
		result->DeleteMe();
}
