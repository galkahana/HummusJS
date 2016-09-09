/*
   Source File : InfoDictionary.h


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

#include "PDFTextString.h"
#include "PDFDate.h"
#include "MapIterator.h"
#include <string>
#include <map>

typedef std::map<std::string,PDFTextString> StringToPDFTextString;

enum EInfoTrapped
{
	EInfoTrappedTrue,
	EInfoTrappedFalse,
	EInfoTrappedUnknown
};

class InfoDictionary
{
public:
	InfoDictionary(void);
	~InfoDictionary(void);

	PDFTextString Title;
	PDFTextString Author;
	PDFTextString Subject;
	PDFTextString Keywords;
	PDFTextString Creator;
	PDFTextString Producer;
	PDFDate CreationDate;
	PDFDate ModDate;
	EInfoTrapped Trapped;

	void AddAdditionalInfoEntry(const std::string& inKey,
								const PDFTextString& inValue);
	void RemoveAdditionalInfoEntry(const std::string& inKey);
	void ClearAdditionalInfoEntries();
	PDFTextString GetAdditionalInfoEntry(const std::string& inKey);

	MapIterator<StringToPDFTextString> GetAdditionaEntriesIterator();


	bool IsEmpty();

	void Reset();
private:

	StringToPDFTextString mAdditionalInfoEntries;
};

