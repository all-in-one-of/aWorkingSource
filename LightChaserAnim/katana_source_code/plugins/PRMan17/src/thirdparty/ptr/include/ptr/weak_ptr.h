// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef     SPI_PTR_WEAK_PTR_H
#define     SPI_PTR_WEAK_PTR_H

//
//  weak_ptr.hpp
//
//  Copyright (c) 2001, 2002, 2003 Peter Dimov
//
//  Permission to copy, use, modify, sell and distribute this software
//  is granted provided this copyright notice appears in all copies.
//  This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//
//  See http://www.boost.org/libs/smart_ptr/weak_ptr.htm for documentation.
//


// Note: using the boost copyright notice, since this file is a simplified version of
// their weak_ptr.hpp


#include    "shared_ptr.h"

#include    <algorithm>
#include    <exception>
#include    <ptr/Version.h>

SPI_PTR_BEGIN_NAMESPACE

/////////////////////////////////////////////
//  
//  weak_ptr:

template<typename T>
class weak_ptr {

    public:
    
    //  Generated:
    //                          weak_ptr(const weak_ptr& iSource);
    //                          ~weak_ptr(void);
    //  weak_ptr&               operator=(const weak_ptr& iSource);
        
        typedef T element_type;
        
                                weak_ptr(void);
        
                                template<typename Y>
                                weak_ptr(const weak_ptr<Y>& iSource);
        
                                template<typename Y>
                                weak_ptr(const shared_ptr<Y>& iSource);
        
        template<typename Y>
        weak_ptr&               operator=(const weak_ptr<Y>& iSource);
        
        template<typename Y>
        weak_ptr&               operator=(const shared_ptr<Y>& iSource);
        
        long                    use_count(void) const;
        bool                    expired(void) const;
        
        shared_ptr<T>           lock(void) const;
        
        void                    reset(void);
        void                    swap(weak_ptr<T>& ioOther);
        
        bool                    operator<(const weak_ptr& iOther) const;
    
    private:
    
        template <typename Y>
        friend class shared_ptr;
        
        template <typename Y>
        friend class weak_ptr;
        
        T*                      mPtr;
        weak_count              mCount;

};



/////////////////////////////////////////////
//  
//  Implementation:

template<typename T>
weak_ptr<T>::weak_ptr(void) :
    mPtr(0),
    mCount()

{
}

template<typename T>
template<typename Y>
weak_ptr<T>::weak_ptr(const weak_ptr<Y>& iSource) :
    mCount(iSource.mCount)

{
    mPtr = iSource.lock().get();
}

template<typename T>
template<typename Y>
weak_ptr<T>::weak_ptr(const shared_ptr<Y>& iSource) :
    mPtr(iSource.mPtr),
    mCount(iSource.mCount)

{
}

template<typename T>
template<typename Y>
weak_ptr<T>& weak_ptr<T>::operator=(const weak_ptr<Y>& iSource)

{
    mPtr = iSource.lock().get();
    mCount = iSource.mCount;
    
    return *this;
}

template<typename T>
template<typename Y>
weak_ptr<T>& weak_ptr<T>::operator=(const shared_ptr<Y>& iSource)

{
    mPtr = iSource.mPtr;
    mCount = iSource.mCount;
    
    return *this;
}

template<typename T>
long weak_ptr<T>::use_count(void) const

{
    return mCount.use_count();
}

template<typename T>
bool weak_ptr<T>::expired(void) const

{
    return mCount.use_count() == 0;
}

template<typename T>
shared_ptr<T> weak_ptr<T>::lock(void) const

{
    return this->expired() ?
            shared_ptr<T>() :
            shared_ptr<T>(*this);
}

template<typename T>
void weak_ptr<T>::reset(void)

{
}

template<typename T>
void weak_ptr<T>::swap(weak_ptr<T>& ioOther)

{
    std::swap(mPtr, ioOther.mPtr);
    mCount.swap(ioOther.mCount);
}

template<typename T>
bool weak_ptr<T>::operator<(const weak_ptr& iOther) const

{
    return mCount < iOther.mCount;
}

SPI_PTR_END_NAMESPACE

#endif
