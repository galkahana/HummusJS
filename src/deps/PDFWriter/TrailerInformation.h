/*
   Source File : TrailerInformation.h


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
#include "IOBasicTypes.h"
#include "ObjectsBasicTypes.h"
#include "InfoDictionary.h"

#include <utility>

using namespace IOBasicTypes;

typedef std::pair<bool,LongFilePositionType> BoolAndLongFilePositionType;
typedef std::pair<bool,ObjectReference> BoolAndObjectReference;

class TrailerInformation
{
public:
	TrailerInformation(void);
	~TrailerInformation(void);

	void SetPrev(LongFilePositionType inPrev);
	void SetRoot(const ObjectReference& inRootReference);
	void SetEncrypt(const ObjectReference& inEncryptReference);
	void SetInfoDictionaryReference(const ObjectReference& inInfoDictionaryReference);


	// return with existance validation. for example, if Root is undefined, will return false
	// as the first parameter, otherwise true and the value
	BoolAndLongFilePositionType GetPrev();
	BoolAndObjectReference GetRoot();
	BoolAndObjectReference GetEncrypt();
	BoolAndObjectReference GetInfoDictionaryReference();

	InfoDictionary& GetInfo();

	void Reset();

private:
	LongFilePositionType mPrev;

	ObjectReference mRootReference;
	ObjectReference mEncryptReference;

	InfoDictionary mInfoDictionary;
	ObjectReference mInfoDictionaryReference;
    
};
