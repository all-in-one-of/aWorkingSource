#ifndef INCLUDED_FNGEOLIBUTIL_MUTEX_H
#define INCLUDED_FNGEOLIBUTIL_MUTEX_H

/*
PTEX SOFTWARE
Copyright 2009 Disney Enterprises, Inc.  All rights reserved

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.

  * The names "Disney", "Walt Disney Pictures", "Walt Disney Animation
    Studios" or the names of its contributors may NOT be used to
    endorse or promote products derived from this software without
    specific prior written permission from Walt Disney Pictures.

Disclaimer: THIS SOFTWARE IS PROVIDED BY WALT DISNEY PICTURES AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE, NONINFRINGEMENT AND TITLE ARE DISCLAIMED.
IN NO EVENT SHALL WALT DISNEY PICTURES, THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND BASED ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
*/

#include <FnPlatform/internal/Portability.h>

#include "ns.h"
#include "Platform.h"

// #define DEBUG_THREADING

FNGEOLIBUTIL_NAMESPACE_ENTER
{

#ifndef NDEBUG
    template <class T>
    class DebugLock : public T {
     public:
        FNKAT_DEPRECATED DebugLock() : _locked(0) {}
        FNKAT_DEPRECATED void lock()
        {
            T::lock();
            __sync_fetch_and_add(&_locked, 1);
        }
        FNKAT_DEPRECATED void unlock()
        {
            int currentLocked = __sync_fetch_and_sub(&_locked, 1);
            assert(currentLocked > 0);
            T::unlock();
        }
        FNKAT_DEPRECATED bool locked()
        {
            int currentLocked = __sync_fetch_and_add(&_locked, 0);
            return (currentLocked != 0);
        }
     private:
        volatile int _locked;
    };
#endif

    /** Automatically acquire and release lock within enclosing scope. */
    template <class T>
    class AutoLock {
    public:
        FNKAT_DEPRECATED AutoLock(T& m) : _m(m) { _m.lock(); }
        FNKAT_DEPRECATED ~AutoLock()            { _m.unlock(); }
    private:
        T& _m;
    };

    // Auto read/write lock
    class AutoReadLock {
    public:
        FNKAT_DEPRECATED AutoReadLock(_ReadWriteLock & rwl) : _rwl(rwl) { _rwl.rdlock(); }
        FNKAT_DEPRECATED ~AutoReadLock() { _rwl.unlock(); }
    private:
        _ReadWriteLock & _rwl;
    };
    class AutoWriteLock {
    public:
        FNKAT_DEPRECATED AutoWriteLock(_ReadWriteLock & rwl) : _rwl(rwl) { _rwl.wrlock(); }
        FNKAT_DEPRECATED ~AutoWriteLock() { _rwl.unlock(); }
    private:
        _ReadWriteLock & _rwl;
    };


#ifndef NDEBUG
    // add debug wrappers to mutex and spinlock
    typedef DebugLock<_Mutex> Mutex;
    typedef DebugLock<_SpinLock> SpinLock;
    typedef DebugLock<_RecursiveMutex> RecursiveMutex;
#else
    typedef _Mutex Mutex;
    typedef _SpinLock SpinLock;
    typedef _RecursiveMutex RecursiveMutex;
#endif


    typedef AutoLock<Mutex> AutoMutex;
    typedef AutoLock<SpinLock> AutoSpin;
    typedef AutoLock<RecursiveMutex> AutoRecursiveMutex;



    typedef _ReadWriteLock ReadWriteLock;

}
FNGEOLIBUTIL_NAMESPACE_EXIT

#endif // INCLUDED_FNGEOLIBUTIL_MUTEX_H
