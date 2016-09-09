/*
   Source File : ContainerIterator.h


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
	Abstract Generic template for list single-self-contained iterator.
	useful for external loops. instead of providing two iterators (map.begin and map.end)
	provide a single iterator.
	usage:

	SingleValueContainerIterator<MyListType> SingleValueContainerIterator(myListOfListType);

	while(SingleValueContainerIterator.MoveNext())
	{
		// do something derived class item getter
	}
*/

// some minor template specialization to differ const lists from non const lists
template <class T>
class ContainerTraits
{
public:
    
    typedef typename T::iterator iteratorType;

};

template <class T>
class ContainerTraits<const T>
{
public:
    
    typedef typename T::const_iterator iteratorType;

};

// the real deal
template <class T>
class ContainerIterator
{
public:
    
	ContainerIterator(T& inContainer);
	ContainerIterator(const ContainerIterator<T>& inOtherIterator);
    
	bool MoveNext();
	bool IsFinished();
    
private:
	typename ContainerTraits<T>::iteratorType mEndPosition;
	bool mFirstMove;
    
protected:
	typename ContainerTraits<T>::iteratorType mCurrentPosition;
};

template <class T>
ContainerIterator<T>::ContainerIterator(T& inList)
{
	mCurrentPosition = inList.begin();
	mEndPosition = inList.end();
	mFirstMove = true;
}

template <class T>
ContainerIterator<T>::ContainerIterator(const ContainerIterator<T>& inOtherIterator)
{
	mCurrentPosition = inOtherIterator.mCurrentPosition;
	mEndPosition = inOtherIterator.mEndPosition;
	mFirstMove = inOtherIterator.mFirstMove;
}

template <class T>
bool ContainerIterator<T>::MoveNext()
{
	if(mCurrentPosition == mEndPosition)
		return false;
	if(mFirstMove)
	{
		mFirstMove = false;
	}
	else
	{
		if(++mCurrentPosition == mEndPosition)
			return false;
	}
	return true;
}

template <class T>
bool ContainerIterator<T>::IsFinished()
{
	return mCurrentPosition == mEndPosition;
}


