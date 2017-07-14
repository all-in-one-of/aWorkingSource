// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#ifndef     SPI_PTR_SHARED_PTR_H
#define     SPI_PTR_SHARED_PTR_H

//  shared_ptr.hpp
//
//  (C) Copyright Greg Colvin and Beman Dawes 1998, 1999.
//  Copyright (c) 2001, 2002, 2003 Peter Dimov
//
//  Permission to copy, use, modify, sell and distribute this software
//  is granted provided this copyright notice appears in all copies.
//  This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//
//  See http://www.boost.org/libs/smart_ptr/shared_ptr.htm for documentation.
//


// Note: using the boost copyright notice, since this file is a simplified version of
// their shared_ptr.hpp


#include    <algorithm>
#include    <assert.h>
#include    <functional>

#include    <ptr/Version.h>

SPI_PTR_BEGIN_NAMESPACE

template<typename T>
struct Delete {
    void                    operator()(T*& iPtr) const;
};

struct NoDelete {
    void        operator()(const void* iPtr) const;
};


/////////////////////////////////////////////
//  
//  Implementation:

template<typename T>
inline void Delete<T>::operator()(T*& iPtr) const

{
//  This line will force a compiler error if T is not a complete type
//  at this point.
//  (Copied from boost's checked_delete function)
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void) sizeof(type_must_be_complete);
    
    delete iPtr;
}

inline void NoDelete::operator()(const void*) const

{
}

/*

Simple substitute for boost::shared_ptr until we get boost into Imageworks.

*/

class sp_counted_base {

    public:
    
                                sp_counted_base(void);
        virtual                 ~sp_counted_base(void);
        
        virtual void            dispose(void) = 0;
        void                    destruct(void);
        
        void                    add_ref(void);
        void                    add_ref_lock(void);
        void                    release(void);
        
        void                    weak_add_ref(void);
        void                    weak_release(void);
        
        long                    use_count(void) const;
    
    private:
    
    //  Suppressed:
                                sp_counted_base(const sp_counted_base&);
        sp_counted_base&        operator=(const sp_counted_base&);
        
        long                    mCount;
        long                    mWeakCount;

};

/*
use partial specialization to derive from D when D is a class?
(D could also be a function pointer)
*/

template <typename P, typename D>
class sp_counted_base_impl : public sp_counted_base {

    public:
    
                                sp_counted_base_impl(P iPtr,
                                                        D iDeleter);
        virtual                 ~sp_counted_base_impl(void);
        
        virtual void            dispose(void);
    
    private:
    
    //  Suppressed:
    //                          sp_counted_base_impl(void);
        
                                sp_counted_base_impl(
                                    const sp_counted_base_impl&);
        sp_counted_base_impl&   operator=(const sp_counted_base_impl&);
        
        P                       mPtr;
        D                       mDeleter;

};

/////////////////////////////////////////////
//  
//  bad_weak_ptr:

class bad_weak_ptr : public std::exception {

    public:
    
        virtual const char*     what(void)
        {
            return "SPI_ptr::bad_weak_ptr";
        }
    
    private:

};

class weak_count;

class shared_count {

    public:
    
                                shared_count(void);
                                shared_count(const shared_count& iSource);
                                shared_count(const weak_count& iSource);
        
                                template<typename P, typename D>
                                shared_count(P iPtr,
                                                D iDeleter);
                                
                                ~shared_count(void);
        
        shared_count&           operator=(const shared_count& iSource);
        
        long                    use_count(void) const;
        
        void                    swap(shared_count& iOther);
    
    private:
    
        friend bool operator<(const shared_count&,
                                const shared_count&);
        friend class weak_count;
        
        sp_counted_base*        mImpl;

};

bool operator<(const shared_count& iArg1,
                const shared_count& iArg2);

/////////////////////////////////////////////
//  
//  weak_count:

class weak_count {

    public:
    
                                weak_count(void);
                                weak_count(const weak_count& iSource);
                                weak_count(const shared_count& iSource);
                                ~weak_count(void);
        
        weak_count&             operator=(const weak_count& iSource);
        weak_count&             operator=(const shared_count& iSource);
        
        void                    swap(weak_count& ioOther);
        
        long                    use_count(void) const;
    
    private:
    
        friend class shared_count;
        
        friend bool operator==(const weak_count& iArg1,
                                const weak_count& iArg2);
        friend bool operator<(const weak_count& iArg1,
                                const weak_count& iArg2);
        
        sp_counted_base*        mImpl;

};

bool operator==(const weak_count& iArg1,
                const weak_count& iArg2);
bool operator<(const weak_count& iArg1,
                const weak_count& iArg2);


//  Enable const, dynamic, static casting:
struct const_cast_tag {};
struct static_cast_tag {};
struct dynamic_cast_tag {};

template<typename T>
class weak_ptr;

template<typename T>
class shared_ptr {

    public:
    
        typedef T element_type;
        typedef T value_type;
        typedef T* pointer;
        typedef T& reference;
        
    //  Generated:
    //                          shared_ptr(const shared_ptr& iSource);
    //                          ~shared_ptr(void);
        
                                shared_ptr(void);
        
                                template<typename Y>
        explicit                shared_ptr(Y* iPtr);
        
                                template<typename Y, typename D>
                                shared_ptr(Y* iPtr,
                                            D iDeleter);
        
        //  This constructor is not explicit because it is designed to allows
        //  implicit conversions from shared_ptr<Derived> to shared_ptr<Base>
                                template<typename Y>
                                shared_ptr(shared_ptr<Y> iOther);
        
                                template<typename Y>
        explicit                shared_ptr(weak_ptr<Y> iOther);
        
                                template<typename Y>
                                shared_ptr(shared_ptr<Y> iOther,
                                            const_cast_tag);

                                template<typename Y>
                                shared_ptr(shared_ptr<Y> iOther,
                                            static_cast_tag);

                                template<typename Y>
                                shared_ptr(shared_ptr<Y> iOther,
                                            dynamic_cast_tag);
        
    //  This could be compiler-generated, but there is a comment in the
    //  original boost sources that when this operator is generated, gcc
    //  gives a warning with -Wsynth, so we define it explicitly.
        shared_ptr&             operator=(const shared_ptr& iSource);
        
        typedef T* (shared_ptr<T>::*unspecified_bool_type)() const;
        
                                operator unspecified_bool_type(void) const
                                {
                                    return mPtr == 0 ? 0 : &shared_ptr<T>::get;
                                }
        
        reference               operator*(void) const;
        pointer                 operator->(void) const;
        
        bool                    operator==(const shared_ptr<T>& iArg) const;
        bool                    operator!=(const shared_ptr<T>& iArg) const;
        bool                    operator<(const shared_ptr<T>& iArg) const;
        bool                    operator<=(const shared_ptr<T>& iArg) const;
        bool                    operator>(const shared_ptr<T>& iArg) const;
        bool                    operator>=(const shared_ptr<T>& iArg) const;
        
        void                    reset(void);
        
        template<typename Y>
        void                    reset(Y* iPtr);
        
        template<typename Y, typename D>
        void                    reset(Y* iPtr,
                                        D iDeleter);
        
        pointer                 get(void) const;
        
        long                    use_count(void) const;
        
        void                    swap(shared_ptr<T>& iOther);
    
    private:
    
        template <typename Y>
        friend class shared_ptr;
        
        template <typename Y>
        friend class weak_ptr;
        
        typedef shared_ptr<T> this_type;
        
        T*                      mPtr;
        shared_count            mCount;

};

template<class T, class U>
shared_ptr<T> const_pointer_cast(const shared_ptr<U> & r)
{
    return shared_ptr<T>(r, const_cast_tag());
}

template<class T, class U>
shared_ptr<T> dynamic_pointer_cast(const shared_ptr<U> & r)
{
    return shared_ptr<T>(r, dynamic_cast_tag());
}

template<class T, class U>
shared_ptr<T> static_pointer_cast(const shared_ptr<U> & r)
{
    return shared_ptr<T>(r, static_cast_tag());
}


/////////////////////////////////////////////
//  
//  Implementation:

//  sp_counted_base

inline sp_counted_base::sp_counted_base(void) :
    mCount(1),
    mWeakCount(1)

{
}

inline sp_counted_base::~sp_counted_base(void)

{
}

inline void sp_counted_base::destruct(void)

{
    delete this;
}

inline void sp_counted_base::add_ref(void)

{
    ++mCount;
}

inline void sp_counted_base::add_ref_lock(void)

{
    if (mCount == 0)
        throw bad_weak_ptr();
    
    ++mCount;
}

inline void sp_counted_base::release(void)

{
	assert(mCount > 0);
	
    if (--mCount != 0)
        return;
    
    this->dispose();
    
    this->weak_release();
}

inline void sp_counted_base::weak_add_ref(void)

{
    ++mWeakCount;
}

inline void sp_counted_base::weak_release(void)

{
	assert(mWeakCount > 0);
	
    if (--mWeakCount == 0)
        this->destruct();
}

inline long sp_counted_base::use_count(void) const

{
    return mCount;
}

//  sp_counted_base_impl

template <typename P, typename D>
inline sp_counted_base_impl<P, D>::sp_counted_base_impl(P iPtr, D iDeleter) :
    mPtr(iPtr),
    mDeleter(iDeleter)

{
}

template <typename P, typename D>
inline sp_counted_base_impl<P, D>::~sp_counted_base_impl(void)

{
}

template <typename P, typename D>
inline void sp_counted_base_impl<P, D>::dispose(void)

{
    assert(this->use_count() == 0);
    
    mDeleter(mPtr);
}

//  shared_count

inline shared_count::shared_count(void) :
    mImpl(0)

{
}

inline shared_count::shared_count(const shared_count& iSource) :
    mImpl(iSource.mImpl)

{
    if (mImpl != 0)
        mImpl->add_ref();
}

inline shared_count::shared_count(const weak_count& iSource) :
    mImpl(iSource.mImpl)

{
    if (mImpl != 0)
        mImpl->add_ref_lock();
    else
        throw bad_weak_ptr();
}

template<typename P, typename D>
inline shared_count::shared_count(P iPtr, D iDeleter)

{
    try {
        mImpl = new sp_counted_base_impl<P, D>(iPtr, iDeleter);
    }
    catch (...) {
        iDeleter(iPtr);
        throw;
    }
}

inline shared_count::~shared_count(void)

{
    if (mImpl != 0)
        mImpl->release();
}

inline shared_count& shared_count::operator=(const shared_count& iSource)

{
//  By doing the add_ref before the release, we don't have to check for
//  self-assignment.
    
    sp_counted_base*        temp = iSource.mImpl;
    
    if (temp != 0)
        temp->add_ref();
    
    if (mImpl != 0)
        mImpl->release();
    
    mImpl = temp;
    
    return *this;
}

inline long shared_count::use_count(void) const

{
    if (mImpl != 0)
        return mImpl->use_count();
    else
        return 0;
}

inline void shared_count::swap(shared_count& iOther)

{
    std::swap(mImpl, iOther.mImpl);
}

inline bool operator<(const shared_count& iArg1, const shared_count& iArg2)

{
    return std::less<sp_counted_base*>()(iArg1.mImpl, iArg2.mImpl);
}

//  weak_count

inline weak_count::weak_count(void) :
    mImpl(0)

{
}
inline weak_count::weak_count(const weak_count& iSource) :
    mImpl(iSource.mImpl)

{
    if (mImpl != 0)
        mImpl->weak_add_ref();
}

inline weak_count::weak_count(const shared_count& iSource) :
    mImpl(iSource.mImpl)

{
    if (mImpl != 0)
        mImpl->weak_add_ref();
}

inline weak_count::~weak_count(void)

{
    if (mImpl != 0)
        mImpl->weak_release();
}

inline weak_count& weak_count::operator=(const weak_count& iSource)

{
    sp_counted_base*        temp = iSource.mImpl;
    
    if (temp != 0)
        temp->weak_add_ref();
    if (mImpl != 0)
        mImpl->weak_release();
    
    mImpl = temp;
    
    return *this;
}

inline weak_count& weak_count::operator=(const shared_count& iSource)

{
    sp_counted_base*        temp = iSource.mImpl;
    
    if (temp != 0)
        temp->weak_add_ref();
    if (mImpl != 0)
        mImpl->weak_release();
    
    mImpl = temp;
    
    return *this;
}

inline void weak_count::swap(weak_count& ioOther)

{
    std::swap(mImpl, ioOther.mImpl);
}

inline long weak_count::use_count(void) const

{
    return mImpl != 0 ? mImpl->use_count() : 0;
}

inline bool operator==(const weak_count& iArg1, const weak_count& iArg2)

{
    return iArg1.mImpl == iArg2.mImpl;
}

inline bool operator<(const weak_count& iArg1, const weak_count& iArg2)

{
    return std::less<sp_counted_base*>()(iArg1.mImpl, iArg2.mImpl);
}

//  shared_ptr

template<typename T>
shared_ptr<T>::shared_ptr(void) :
    mPtr(0),
    mCount()

{
}

template<typename T>
template<typename Y>
shared_ptr<T>::shared_ptr(Y* iPtr) :
    mPtr(iPtr),
    mCount(shared_count(iPtr, Delete<Y>()))

{
}

template<typename T>
template<typename Y, typename D>
shared_ptr<T>::shared_ptr(Y* iPtr, D iDeleter) :
    mPtr(iPtr),
    mCount(shared_count(iPtr, iDeleter))

{
}

template<typename T>
template<typename Y>
shared_ptr<T>::shared_ptr(shared_ptr<Y> iOther) :
    mPtr(iOther.mPtr),
    mCount(iOther.mCount)

{
}

template<typename T>
template<typename Y>
shared_ptr<T>::shared_ptr(weak_ptr<Y> iOther) :
    mCount(iOther.mCount)

{
//  It is now safe to copy mPtr, as mCount did not throw.
    mPtr = iOther.mPtr;
}

template<typename T>
template<typename Y>
shared_ptr<T>::shared_ptr(shared_ptr<Y> iOther, const_cast_tag) :
    mPtr(const_cast<element_type*>(iOther.mPtr)),
    mCount(iOther.mCount)

{
}

template<typename T>
template<typename Y>
shared_ptr<T>::shared_ptr(shared_ptr<Y> iOther, dynamic_cast_tag) :
    mPtr(dynamic_cast<element_type*>(iOther.mPtr)),
    mCount(iOther.mCount)

{
}

template<typename T>
template<typename Y>
shared_ptr<T>::shared_ptr(shared_ptr<Y> iOther, static_cast_tag) :
    mPtr(static_cast<element_type*>(iOther.mPtr)),
    mCount(iOther.mCount)

{
}

template<typename T>
shared_ptr<T>& shared_ptr<T>::operator=(const shared_ptr& iSource)

{
    mPtr = iSource.mPtr;
    mCount = iSource.mCount;
    
    return *this;
}

template<typename T>
inline typename shared_ptr<T>::reference shared_ptr<T>::operator*(void) const

{
    return *mPtr;
}

template<typename T>
inline typename shared_ptr<T>::pointer shared_ptr<T>::operator->(void) const

{
    return mPtr;
}

template<typename T>
inline bool shared_ptr<T>::operator==(const shared_ptr<T>& iArg) const

{
    return mPtr == iArg.mPtr;
}

template<typename T>
inline bool shared_ptr<T>::operator!=(const shared_ptr<T>& iArg) const

{
    return mPtr != iArg.mPtr;
}

template<typename T>
inline bool shared_ptr<T>::operator<(const shared_ptr<T>& iArg) const

{
    return mCount < iArg.mCount;
}

template<typename T>
inline bool shared_ptr<T>::operator<=(const shared_ptr<T>& iArg) const

{
    return !(iArg.mCount < mCount);
}

template<typename T>
inline bool shared_ptr<T>::operator>(const shared_ptr<T>& iArg) const

{
    return iArg.mCount < mCount;
}

template<typename T>
inline bool shared_ptr<T>::operator>=(const shared_ptr<T>& iArg) const

{
    return !(mCount < iArg.mCount);
}

template<typename T>
inline typename shared_ptr<T>::pointer shared_ptr<T>::get(void) const

{
    return mPtr;
}

template<typename T>
inline void shared_ptr<T>::reset(void)

{
    this_type().swap(*this);
}

template<typename T>
template<typename Y>
inline void shared_ptr<T>::reset(Y* iPtr)

{
    this_type(iPtr).swap(*this);
}

template<typename T>
template<typename Y, typename D>
inline void shared_ptr<T>::reset(Y* iPtr, D iDeleter)

{
    this_type(iPtr, iDeleter).swap(*this);
}

template<typename T>
long shared_ptr<T>::use_count(void) const

{
    return mCount.use_count();
}

template<typename T>
void shared_ptr<T>::swap(shared_ptr<T>& iOther)

{
    std::swap(mPtr, iOther.mPtr);
    mCount.swap(iOther.mCount);
}

SPI_PTR_END_NAMESPACE

#endif
