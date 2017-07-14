// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#ifndef SPI_PTR_REF_PTR_H
#define SPI_PTR_REF_PTR_H

#include    <ptr/Version.h>

SPI_PTR_BEGIN_NAMESPACE

class ref_base 
{
public:
    ref_base() 
        : m_refCount(0) 
        {}

    ref_base(const ref_base&) 
        : m_refCount(0) 
        {}

    ref_base& operator= (const ref_base&)
    { return *this; }

    virtual ~ref_base() 
    {}

    void addReference() const
    { 
        ++m_refCount; 
    }

    void releaseReference() const
    {
        --m_refCount;
        if (m_refCount <= 0)
            delete this;
    }

private:
    mutable long m_refCount;
};


template <typename T>
class ref_ptr
{
public:
    ref_ptr() : m_ptr(0) {}

    ref_ptr(T* p) : m_ptr(p) 
    {
        if (m_ptr) m_ptr->addReference();
    }

    ref_ptr(const ref_ptr<T>& p) : m_ptr(p.m_ptr) 
    { 
        if (m_ptr) m_ptr->addReference(); 
    }
        
    //  This constructor is not explicit because it is designed to allows
    //  implicit conversions from ref_ptr<Derived> to ref_ptr<Base>
    template<typename T2>
    ref_ptr(const ref_ptr<T2>& p) : m_ptr(p.get())
    {
        if (m_ptr) m_ptr->addReference();
    }

    ref_ptr& operator= (const ref_ptr<T>& p)
    {
        T* old_ptr = m_ptr;
        m_ptr = p.m_ptr;
        if (m_ptr)
            m_ptr->addReference();
        if (old_ptr)
            old_ptr->releaseReference();
        return *this;
    }

    template<typename T2>
    ref_ptr& operator= (const ref_ptr<T2>& p)
    {
        T* old_ptr = m_ptr;
        m_ptr = p.get();
        if (m_ptr)
            m_ptr->addReference();
        if (old_ptr)
            old_ptr->releaseReference();
        return *this;
    }

    ~ref_ptr()
    {
        if (m_ptr)
            m_ptr->releaseReference();
    }

    operator ref_ptr<const T> () const
    { 
        return ref_ptr<const T>(m_ptr); 
    }

    operator bool() const { return m_ptr; }

    T* get() const { return m_ptr; }

    T& operator*() const { return *m_ptr; }

    T* operator->() const { return m_ptr; }

private:
    T* m_ptr;
};

// Comparison.

template <class T1, class T2>
bool operator== (const ref_ptr<T1>& p1, const ref_ptr<T2>& p2)
{
    return p1.get() == p2.get();
}

template <class T1, class T2>
bool operator!= (const ref_ptr<T1>& p1, const ref_ptr<T2>& p2)
{
    return p1.get() != p2.get();
}

template <class T1, class T2>
bool operator== (const ref_ptr<T1>& p1, T2* p2)
{
    return p1.get() == p2;
}

template <class T1, class T2>
bool operator!= (const ref_ptr<T1>& p1, T2* p2)
{
    return p1.get() != p2;
}

template <class T1, class T2>
bool operator== (T1* p1, const ref_ptr<T2>& p2)
{
    return p1 == p2.get();
}

template <class T1, class T2>
bool operator!= (T1* p1, const ref_ptr<T2>& p2)
{
    return p1 != p2.get();
}



/// @brief Performs dynamic_cast of internal pointer and returns ref_ptr to 
/// casted type (might be null).
template <class T1, class T2>
inline ref_ptr<T1> dynamic_pointer_cast(const ref_ptr<T2>& p)
{
    T1 *foo = dynamic_cast<T1 *>(p.get());
    return ref_ptr<T1>(foo);
}

/// @brief Performs const_cast of internal pointer and returns ref_ptr to 
/// casted type.
template <class T1, class T2>
inline ref_ptr<T1> const_pointer_cast(const ref_ptr<T2>& p)
{
    T1 *foo = const_cast<T1 *>(p.get());
    return ref_ptr<T1>(foo);
}

/// @brief Performs static_cast of internal pointer and returns ref_ptr to 
/// casted type.
template <class T1, class T2>
inline ref_ptr<T1> static_pointer_cast(const ref_ptr<T2>& p)
{
    T1 *foo = static_cast<T1 *>(p.get());
    return ref_ptr<T1>(foo);
}

SPI_PTR_END_NAMESPACE

#endif
