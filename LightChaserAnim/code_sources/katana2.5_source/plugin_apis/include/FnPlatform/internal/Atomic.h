#ifndef KATANA_PLUGINAPI_FNPLATFORM_INTERNAL_ATOMIC_H_
#define KATANA_PLUGINAPI_FNPLATFORM_INTERNAL_ATOMIC_H_

#ifdef _MSC_VER
#include <intrin.h>
#endif  // _MSC_VER

#include <FnPlatform/FnPlatformAPI.h>
#include <FnPlatform/ns.h>

FNPLATFORM_NAMESPACE_ENTER
{
namespace internal
{
static inline void AtomicThreadFence()
{
#ifdef _MSC_VER
    _ReadWriteBarrier();
#else
    __sync_synchronize();
#endif  // _MSC_VER
}

template <typename T>
static inline bool AtomicCompareExchangeStrong(T* volatile* obj,
                                               T** expected, T* desired)
{
#ifdef _MSC_VER
    T* prev = (T*)_InterlockedCompareExchangePointer(
        (void* volatile*)obj, (void*)desired, (void*)*expected);
#else
    T* prev = __sync_val_compare_and_swap(obj, *expected, desired);
#endif  // _MSC_VER
    if (prev == *expected)
    {
        return true;
    }
    *expected = prev;
    return false;
}
}  // namespace internal
}
FNPLATFORM_NAMESPACE_EXIT

#endif  // KATANA_PLUGINAPI_FNPLATFORM_INTERNAL_ATOMIC_H_
