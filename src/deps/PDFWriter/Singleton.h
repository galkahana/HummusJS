/*
   Source File : Singleton.h


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

#ifndef NULL
#define NULL 0
#endif

template <class T> 
class Singleton
{
public:
	static T* GetInstance();
	static void Reset();
	// same as reset (trying to work with different versions usage here)
	static void Release();
private:
	Singleton();
	static T* mInstance;
};

template <class T>
T* Singleton<T>::mInstance = 0;

template <class T>
T* Singleton<T>::GetInstance()
{
	if(!mInstance)
		mInstance = new T();	
	return mInstance;
}

template <class T>
void Singleton<T>::Reset()
{
	delete mInstance;
	mInstance = NULL;
}

template <class T>
void Singleton<T>::Release()
{
	Reset();
}