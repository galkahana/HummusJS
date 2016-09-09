/*
   Source File : MapIterator.h


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
	Generic template for maps single-self-contained iterator.
	useful for external loops. instead of providing two iterators (map.begin and map.end)
	provide a single iterator.
	usage:

	MapIterator<MyMapType> mapIterator(myMapOfMapType);

	while(mapIterator.MoveNext())
	{
		// do something with m.GetKey() and m.GetValue(), which point to the current iterated key and value
	}
*/

template <class T>
class MapIterator : public ContainerIterator<T>
{
public:

	MapIterator(T& inMap);
	MapIterator(const MapIterator<T>& inOtherIterator);

	typename T::key_type GetKey();
	typename T::mapped_type GetValue();

};

template <class T>
MapIterator<T>::MapIterator(T& inMap):ContainerIterator<T>(inMap)
{
}

template <class T>
MapIterator<T>::MapIterator(const MapIterator<T>& inOtherIterator):ContainerIterator<T>(inOtherIterator)
{
}

template <class T>
typename T::key_type MapIterator<T>::GetKey()
{
	return this->mCurrentPosition->first;
}

template <class T>
typename T::mapped_type MapIterator<T>::GetValue()
{
	return this->mCurrentPosition->second;
}