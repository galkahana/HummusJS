/*
   Source File : TrailerInformation.cpp


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
#include "TrailerInformation.h"

TrailerInformation::TrailerInformation(void)
{
	mPrev = 0;
}

TrailerInformation::~TrailerInformation(void)
{
}

void TrailerInformation::SetInfoDictionaryReference(const ObjectReference& inInfoDictionaryReference)
{
	mInfoDictionaryReference = inInfoDictionaryReference;
}

void TrailerInformation::SetPrev(LongFilePositionType inPrev)
{
	mPrev = inPrev;
}

void TrailerInformation::SetRoot(const ObjectReference& inRootReference)
{
	mRootReference = inRootReference;
}

void TrailerInformation::SetEncrypt(const ObjectReference& inEncryptReference)
{
	mEncryptReference = inEncryptReference;
}

BoolAndLongFilePositionType TrailerInformation::GetPrev()
{
	return BoolAndLongFilePositionType(mPrev != 0,mPrev);
}

BoolAndObjectReference TrailerInformation::GetRoot()
{
	return BoolAndObjectReference(mRootReference.ObjectID != 0, mRootReference);
}

BoolAndObjectReference TrailerInformation::GetEncrypt()
{
	return BoolAndObjectReference(mEncryptReference.ObjectID != 0, mEncryptReference);
}

InfoDictionary& TrailerInformation::GetInfo()
{
	return mInfoDictionary;
}

BoolAndObjectReference TrailerInformation::GetInfoDictionaryReference()
{
	return BoolAndObjectReference(mInfoDictionaryReference.ObjectID != 0, mInfoDictionaryReference);
}

void TrailerInformation::Reset()
{
	mPrev = 0;
	mRootReference.ObjectID = 0;
	mEncryptReference.ObjectID = 0;
	mInfoDictionaryReference.ObjectID = 0;
	mInfoDictionary.Reset();
}