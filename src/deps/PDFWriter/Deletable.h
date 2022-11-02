#include "IDeletable.h"

template <typename T>
class Deletable: public IDeletable {
public:
    Deletable(T* inData);
    virtual ~Deletable();

    virtual void DeleteMe();

    T* operator->();

	T* GetPtr();
private:
    T* mValue;
};

template <typename T>
Deletable<T>::Deletable(T* inValue)
{
	mValue = inValue;
}

template <typename T>
Deletable<T>::~Deletable()
{
	delete mValue;
}

template <typename T>
void Deletable<T>::DeleteMe()
{
	delete this;
}

template <typename T>
T* Deletable<T>::operator->()
{
	return mValue;
}

template <typename T>
T* Deletable<T>::GetPtr()
{
	return mValue;
}