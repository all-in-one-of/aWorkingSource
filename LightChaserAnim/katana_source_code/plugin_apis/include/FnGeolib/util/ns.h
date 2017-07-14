#ifndef FnGeolibUtil_ns_h

#define FNGEOLIBUTIL_NAMESPACE_TOKEN Util

#ifdef FNGEOLIB_INTERNAL_NAMESPACE
#include "ns_internal.h"
#else

// Macros used through the code to define API namespaces
#define FNGEOLIBUTIL_NAMESPACE_ENTER namespace Foundry { \
    namespace Katana { \
    namespace FNGEOLIBUTIL_NAMESPACE_TOKEN
#define FNGEOLIBUTIL_NAMESPACE_EXIT } }
#define FNGEOLIBUTIL_NAMESPACE Foundry::Katana::FNGEOLIBUTIL_NAMESPACE_TOKEN

#endif

// Create a namespace instance so that namespace aliases can be declared
FNGEOLIBUTIL_NAMESPACE_ENTER
{
}
FNGEOLIBUTIL_NAMESPACE_EXIT

// Define the FnGeolibUtil namespace alias.
// This is the default namespace to be used to access FnGeolibUtil
// classes and types
namespace FnGeolibUtil = FNGEOLIBUTIL_NAMESPACE;

// Define the FnKat namespace for backward compatibility
#ifndef FNGEOLIB_INTERNAL_NAMESPACE
namespace FnKat = Foundry::Katana;
#endif

#endif // FnGeolibUtil_ns_h
