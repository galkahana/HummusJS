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
	StringToVoidP::iterator it = mMetadata.begin();
	for (; it != mMetadata.end(); ++it) {
		delete it->second;
	}
	mMetadata.clear();
}

PDFObject::EPDFObjectType PDFObject::GetType()
{
	return mType;
}

void PDFObject::SetMetadata(const std::string& inKey, void* inValue) {
	// delete old metadata
	DeleteMetadata(inKey);

	mMetadata.insert(StringToVoidP::value_type(inKey, inValue));
}

void* PDFObject::GetMetadata(const std::string& inKey) {
	StringToVoidP::iterator it = mMetadata.find(inKey);
	
	if (it == mMetadata.end()) 
		return NULL;
	else 
		return it->second;
}

void* PDFObject::DetachMetadata(const std::string& inKey) {
	StringToVoidP::iterator it = mMetadata.find(inKey);

	if (it == mMetadata.end())
		return NULL;
	else {
		void* result = it->second;
		mMetadata.erase(it);
		return result;
	}
}

void PDFObject::DeleteMetadata(const std::string& inKey) {
	void* result = DetachMetadata(inKey);
	delete result;
}
