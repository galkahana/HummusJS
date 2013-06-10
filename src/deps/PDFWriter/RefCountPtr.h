/*
   Source File : RefCountPtr.h


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

/*
	Smart pointer implementing automatic ref count addition/reduction

	Constructors assume that the object was AddRefed, and will make sure that on Ptr destructin Release is called.
	other operators take care of various scenarios/

*/ 

#ifndef NULL
#define NULL 0
#endif


template <typename T>
class RefCountPtr
{
public:

	RefCountPtr();
	// This one will not call AddRef (assume called from outside)
	RefCountPtr(T* inValue);

	// This one will call AddRef
	RefCountPtr(const RefCountPtr<T>& inOtherPtr);

	// This one calls Release
	virtual ~RefCountPtr();

	// The next two call AddRef in insert
	RefCountPtr<T>&  operator =(T* inValue); 
	RefCountPtr<T>&  operator =(const RefCountPtr<T>& inOtherPtr); 

	// Will return the actual pointer
	T* operator->();

	// equality/inequality for included pointer
	bool operator ==(T* inOtherValue); 
	bool operator ==(RefCountPtr<T>& inOtherPtr); 

	bool operator !=(T* inOtherValue); 
	bool operator !=(RefCountPtr<T>& inOtherPtr); 

	// get pointer directly
	T* GetPtr() const;

	bool operator!() const;

private:
	T* mValue;
};

template <typename T>
RefCountPtr<T>::RefCountPtr()
{
	mValue = NULL;
}

template <typename T>
RefCountPtr<T>::RefCountPtr(T* inValue)
{
	mValue = inValue;
}

template <typename T>
RefCountPtr<T>::RefCountPtr(const RefCountPtr<T>& inOtherPtr)
{
	mValue = inOtherPtr.mValue;
	if(mValue)
		mValue->AddRef();
}


// This one calls Release
template <typename T>
RefCountPtr<T>::~RefCountPtr()
{
	if(mValue)
		mValue->Release();
}

template <typename T>
RefCountPtr<T>&  RefCountPtr<T>::operator =(T* inValue)
{
	if(mValue)
		mValue->Release();
	mValue = inValue;
	if(mValue)
		mValue->AddRef();
	return *this;
}

template <typename T>
RefCountPtr<T>&  RefCountPtr<T>::operator =(const RefCountPtr<T>& inOtherPtr)
{
	if(mValue)
		mValue->Release();
	mValue = inOtherPtr.mValue;
	if(mValue)
		mValue->AddRef();
	return *this;
}


// Will return the actual pointer
template <typename T>
T* RefCountPtr<T>::operator->()
{
	return mValue;
}

template <typename T>
bool RefCountPtr<T>::operator ==(T* inOtherValue)
{
	return mValue == inOtherValue;
}

template <typename T>
bool RefCountPtr<T>::operator ==(RefCountPtr<T>& inOtherPtr)
{
	return mValue == inOtherPtr.mValue;
}

template <typename T>
bool RefCountPtr<T>::operator !=(T* inOtherValue)
{
	return mValue != inOtherValue;
}

template <typename T>
bool RefCountPtr<T>::operator !=(RefCountPtr<T>& inOtherPtr)
{
	return mValue != inOtherPtr.mValue;
}

template <typename T>
T* RefCountPtr<T>::GetPtr() const
{
	return mValue;
}

template <typename T>
bool RefCountPtr<T>::operator!() const
{
	return !mValue;
}
