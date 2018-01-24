#ifndef FnGeolib_ns_h
#define FnGeolib_ns_h

#ifdef FNGEOLIB_INTERNAL_NAMESPACE
#include "ns_internal.h"
#else

// Macros used through the code to define API namespaces
#define FNGEOLIBOP_NAMESPACE_ENTER namespace Foundry { \
    namespace Katana
#define FNGEOLIBOP_NAMESPACE_EXIT }
#define FNGEOLIBOP_NAMESPACE Foundry::Katana

#endif

// Create a namespace instance so that namespace aliases can be declared
FNGEOLIBOP_NAMESPACE_ENTER
{
}
FNGEOLIBOP_NAMESPACE_EXIT

// Define the FnGeolibOp namespace alias.
// This is the default namespace to be used to access FnGeolibOp classes
// and types
namespace FnGeolibOp = FNGEOLIBOP_NAMESPACE;

// Define the FnKat namespace for backward compatibility
#ifndef FNGEOLIB_INTERNAL_NAMESPACE
namespace FnKat = Foundry::Katana;
#endif

#endif // FnGeolib_ns_h
