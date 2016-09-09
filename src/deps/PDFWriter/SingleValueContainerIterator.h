/*
   Source File : SingleValueContainerIterator.h


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

#include "ContainerIterator.h"

/*
	Generic template for list single-self-contained iterator.
	useful for external loops. instead of providing two iterators (list.begin and list.end)
	provide a single iterator.
	usage:

	SingleValueContainerIterator<MyListType> SingleValueContainerIterator(myListOfListType);

	while(SingleValueContainerIterator.MoveNext())
	{
		// do something with m.GetItem()
	}
*/

template <class T>
class SingleValueContainerIterator : public ContainerIterator<T>
{
public:

	SingleValueContainerIterator(T& inContainer);
	SingleValueContainerIterator(const SingleValueContainerIterator<T>& inOtherIterator);

	typename T::value_type GetItem();
};

template <class T>
SingleValueContainerIterator<T>::SingleValueContainerIterator(T& inContainer) : ContainerIterator<T>(inContainer)
{
}

template <class T>
SingleValueContainerIterator<T>::SingleValueContainerIterator(const SingleValueContainerIterator<T>& inOtherIterator) : ContainerIterator<T>(inOtherIterator)
{
}

template <class T>
typename T::value_type SingleValueContainerIterator<T>::GetItem()
{
	return *this->mCurrentPosition;
}