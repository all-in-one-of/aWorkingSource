#ifndef KATANA_PLUGINAPI_FNPLATFORM_INTERNAL_SHAREDPTR_H_
#define KATANA_PLUGINAPI_FNPLATFORM_INTERNAL_SHAREDPTR_H_

#include <FnPlatform/ns.h>

#if defined(_MSC_VER) || __cplusplus >= 201103L
#include <memory>
#else
#include <tr1/memory>
#endif

FNPLATFORM_NAMESPACE_ENTER
{
namespace internal
{
template <typename T>
struct SharedPtr
{
#if __cplusplus >= 201103L
    typedef typename std::shared_ptr<T> type;
#else
    typedef typename std::tr1::shared_ptr<T> type;
#endif
};
}  // namespace internal
}
FNPLATFORM_NAMESPACE_EXIT

#endif  // KATANA_PLUGINAPI_FNPLATFORM_INTERNAL_SHAREDPTR_H_
