#ifndef KATANA_PLUGINAPI_FNPLATFORM_INTERNAL_MUTEX_H_
#define KATANA_PLUGINAPI_FNPLATFORM_INTERNAL_MUTEX_H_

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <FnPlatform/Windows.h>
#else
#include <pthread.h>
#endif  // _WIN32

#include <FnPlatform/FnPlatformAPI.h>
#include <FnPlatform/ns.h>

FNPLATFORM_NAMESPACE_ENTER
{
namespace internal
{
class Mutex
{
public:
    Mutex()
    {
#ifdef _WIN32
        // See <https://bugzilla.mozilla.org/show_bug.cgi?id=812085> for
        // justification for these parameters.
        BOOL success = InitializeCriticalSectionEx(
            &_criticalSection, SPIN_COUNT, CRITICAL_SECTION_NO_DEBUG_INFO);
        if (!success)
        {
            fprintf(stderr, "InitializeCriticalSectionEx: 0x%x\n",
                    GetLastError());
            abort();
        }
#else
        int error = pthread_mutex_init(&_mutex, NULL);
        if (error)
        {
            fprintf(stderr, "pthread_mutex_init: %d\n", error);
            abort();
        }
#endif  // _WIN32
    }

    ~Mutex()
    {
#ifdef _WIN32

        DeleteCriticalSection(&_criticalSection);
#else
        int error = pthread_mutex_destroy(&_mutex);
        if (error)
        {
            fprintf(stderr, "pthread_mutex_destroy: %d\n", error);
            abort();
        }
#endif  // _WIN32
    }

    void lock()
    {
#ifdef _WIN32
        EnterCriticalSection(&_criticalSection);
#else
        int error = pthread_mutex_lock(&_mutex);
        if (error)
        {
            fprintf(stderr, "pthread_mutex_lock: %d\n", error);
            abort();
        }
#endif  // _WIN32
    }

    void unlock()
    {
#ifdef _WIN32
        LeaveCriticalSection(&_criticalSection);
#else
        int error = pthread_mutex_unlock(&_mutex);
        if (error)
        {
            fprintf(stderr, "pthread_mutex_unlock: %d\n", error);
            abort();
        }
#endif  // _WIN32
    }

private:
    Mutex(const Mutex&) /* = delete */;
    void operator=(const Mutex&) /* = delete */;

#ifdef _WIN32
    enum
    {
        SPIN_COUNT = 1500,
    };
    CRITICAL_SECTION _criticalSection;
#else
    pthread_mutex_t _mutex;
#endif  // _WIN32
};

template <class MutexT>
class LockGuard
{
public:
    explicit LockGuard(MutexT& mutex) : _mutex(mutex) { _mutex.lock(); }
    ~LockGuard() { _mutex.unlock(); }

private:
    LockGuard(const LockGuard&) /* = delete */;
    void operator=(const LockGuard&) /* = delete */;

    MutexT& _mutex;
};
}  // namespace internal
}
FNPLATFORM_NAMESPACE_EXIT

#endif  // KATANA_PLUGINAPI_FNPLATFORM_INTERNAL_MUTEX_H_
