/*
   Source File : BoxingBase.h


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
	BoxingBase is a class for boxing primitives.

	it's BoxingBase, cause it's meant to be derived...but you can use as is
*/ 

#include <string>
#include <sstream>



template <typename T>
class BoxingBase
{
public:
	BoxingBase();
	BoxingBase(const T& inValue);
	BoxingBase(const BoxingBase<T>& inOther);

	operator T() const;
	BoxingBase<T>&  operator =(const T& inValue);

protected:
	T boxedValue;
};

template <typename T>
BoxingBase<T>::BoxingBase()
{
	// maybe do null assignment sometime
}

template <typename T>
BoxingBase<T>::BoxingBase(const T& inValue)
{
	boxedValue = inValue;
}

template <typename T>
BoxingBase<T>::BoxingBase(const BoxingBase<T>& inOther)
{
	boxedValue = inOther.boxedValue;
}

template <typename T>
BoxingBase<T>&  BoxingBase<T>::operator =(const T& inValue)
{
	boxedValue = inValue;
	return *this;
}

template <typename T>
BoxingBase<T>::operator T() const
{
	return boxedValue;
}

template <typename T>
class STDStreamsReader
{
public:
	void Read(const std::string& inReadFrom,T& outValue);
	void Read(const std::wstring& inReadFrom,T& outValue);
};

template <typename T>
void STDStreamsReader<T>::Read(const std::string& inReadFrom,T& outValue)
{
	std::stringstream stream(inReadFrom);
	stream>>outValue;
}

template <typename T>
void STDStreamsReader<T>::Read(const std::wstring& inReadFrom,T& outValue)
{
	std::wstringstream stream(inReadFrom);
	stream>>outValue;
}

template <typename T>
class STDStreamsWriter
{
public:
	void Write(const T& inValue,std::string& outWriteTo);
	void Write(const T& inValue,std::wstring& outWriteTo);
};

template <typename T>
void STDStreamsWriter<T>::Write(const T& inValue,std::string& outWriteTo)
{
	std::stringstream stream;
	stream<<inValue;
	outWriteTo = stream.str();
}

template <typename T>
void STDStreamsWriter<T>::Write(const T& inValue,std::wstring& outWriteTo)
{
	std::wstringstream stream;
	stream<<inValue;
	outWriteTo = stream.str();
}


template <typename U, class Reader=STDStreamsReader<U>, class Writer=STDStreamsWriter<U> >
class BoxingBaseWithRW : public BoxingBase<U>
{
public:
	BoxingBaseWithRW();
	BoxingBaseWithRW(const U& inValue);
	BoxingBaseWithRW(const BoxingBase<U>& inOther);
	BoxingBaseWithRW(const BoxingBaseWithRW<U,Reader,Writer>& inOther);
	BoxingBaseWithRW(const std::wstring& inReadFrom);
	BoxingBaseWithRW(const std::string& inReadFrom);

	BoxingBaseWithRW<U,Reader,Writer>&  operator =(const U& inValue);

	std::string ToString() const;
	std::wstring ToWString() const;
};

template <typename U, class Reader, class Writer>
BoxingBaseWithRW<U,Reader,Writer>::BoxingBaseWithRW():BoxingBase<U>()
{

}

template <typename U, class Reader, class Writer>
BoxingBaseWithRW<U,Reader,Writer>::BoxingBaseWithRW(const U& inValue):BoxingBase<U>(inValue)
{
	
}

template <typename U, class Reader, class Writer>
BoxingBaseWithRW<U,Reader,Writer>::BoxingBaseWithRW(const BoxingBase<U>& inOther):BoxingBase<U>(inOther)
{

}

template <typename U, class Reader, class Writer>
BoxingBaseWithRW<U,Reader,Writer>::BoxingBaseWithRW(const BoxingBaseWithRW<U,Reader,Writer>& inOther):BoxingBase<U>(inOther)
{

}

template <typename U, class Reader, class Writer>
BoxingBaseWithRW<U,Reader,Writer>::BoxingBaseWithRW(const std::wstring& inReadFrom)
{
	Reader reader;
	reader.Read(inReadFrom,this->boxedValue);
}

template <typename U, class Reader, class Writer>
BoxingBaseWithRW<U,Reader,Writer>::BoxingBaseWithRW(const std::string& inReadFrom)
{
	Reader reader;
	reader.Read(inReadFrom,this->boxedValue);
}

template <typename U, class Reader, class Writer>
BoxingBaseWithRW<U,Reader,Writer>&  BoxingBaseWithRW<U,Reader,Writer>::operator =(const U& inValue)
{
	this->boxedValue = inValue;
	return *this;
}


template <typename U, class Reader, class Writer>
std::string BoxingBaseWithRW<U,Reader,Writer>::ToString() const
{
	Writer writer;
	std::string aString;

	writer.Write(this->boxedValue,aString);

	return aString;
}

template <typename U, class Reader, class Writer>
std::wstring BoxingBaseWithRW<U,Reader,Writer>::ToWString() const
{
	Writer writer;
	std::wstring aString;

	writer.Write(this->boxedValue,aString);

	return aString;
}


typedef BoxingBaseWithRW<int> Int;
typedef BoxingBaseWithRW<double> Double;
typedef BoxingBaseWithRW<long> Long;