/*
   Source File : PDFObjectCast.h


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

#include "RefCountPtr.h"
#include "PDFObject.h"

/*
	This small template function is only intended to be used for automatic casting of retrieved PDFObjects to their respective actual
	objects...and not for anything else.
*/ 

template <class T>
T* PDFObjectCast(PDFObject* inOriginal)
{
	if(!inOriginal)
		return NULL;

	if(inOriginal->GetType() == (PDFObject::EPDFObjectType)T::eType)
	{
		return (T*)inOriginal;
	}
	else
	{
		inOriginal->Release();
		return NULL;
	}
}

template <class T>
class PDFObjectCastPtr : public RefCountPtr<T>
{
public:
	PDFObjectCastPtr():RefCountPtr<T>()
	{
	}

	PDFObjectCastPtr(PDFObject* inPDFObject): RefCountPtr<T>(PDFObjectCast<T>(inPDFObject))
	{
	}

	PDFObjectCastPtr<T>&  operator =(PDFObject* inValue)
	{
		RefCountPtr<T>::operator =(PDFObjectCast<T>(inValue));
		return *this;
	}
};


