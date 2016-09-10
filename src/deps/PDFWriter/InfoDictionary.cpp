/*
   Source File : InfoDictionary.cpp


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
#include "InfoDictionary.h"

InfoDictionary::InfoDictionary(void)
{
	Trapped = EInfoTrappedUnknown;
}

InfoDictionary::~InfoDictionary(void)
{
}

bool InfoDictionary::IsEmpty()
{
	return 
		Title.IsEmpty() &&
		Author.IsEmpty() &&
		Subject.IsEmpty() &&
		Keywords.IsEmpty() &&
		Creator.IsEmpty() &&
		Producer.IsEmpty() &&
		CreationDate.IsNull() &&
		ModDate.IsNull() &&
		EInfoTrappedUnknown == Trapped &&
		mAdditionalInfoEntries.empty();

}

void InfoDictionary::Reset()
{
	mAdditionalInfoEntries.clear();
	Trapped = EInfoTrappedUnknown;
	Title = PDFTextString::Empty();
	Author = PDFTextString::Empty();
	Subject = PDFTextString::Empty();
	Keywords = PDFTextString::Empty();
	Creator = PDFTextString::Empty();
	Producer = PDFTextString::Empty();
	CreationDate.SetTime(-1);
	ModDate.SetTime(-1);
}

void InfoDictionary::AddAdditionalInfoEntry(const std::string& inKey,
											const PDFTextString& inValue)
{
	if(mAdditionalInfoEntries.find(inKey) != mAdditionalInfoEntries.end())
		mAdditionalInfoEntries[inKey] = inValue;
	else
		mAdditionalInfoEntries.insert(StringToPDFTextString::value_type(inKey,inValue));
}

void InfoDictionary::RemoveAdditionalInfoEntry(const std::string& inKey)
{
	StringToPDFTextString::iterator it = mAdditionalInfoEntries.find(inKey);
	if(it != mAdditionalInfoEntries.end())
		mAdditionalInfoEntries.erase(it);
}

void InfoDictionary::ClearAdditionalInfoEntries()
{
	mAdditionalInfoEntries.clear();
}

PDFTextString InfoDictionary::GetAdditionalInfoEntry(const std::string& inKey)
{
	StringToPDFTextString::iterator it = mAdditionalInfoEntries.find(inKey);

	if(it == mAdditionalInfoEntries.end())
		return PDFTextString::Empty();
	else
		return it->second;
}

MapIterator<StringToPDFTextString> InfoDictionary::GetAdditionaEntriesIterator()
{
	return MapIterator<StringToPDFTextString>(mAdditionalInfoEntries);
}
