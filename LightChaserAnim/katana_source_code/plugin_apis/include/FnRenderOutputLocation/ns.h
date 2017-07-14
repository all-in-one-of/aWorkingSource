#ifndef FnRenderOutputLocation_ns_h
#define FnRenderOutputLocation_ns_h

#ifdef FNGEOLIB_INTERNAL_NAMESPACE
#include "ns_internal.h"
#else

// Macros used through the code to define API namespaces
#define FNRENDEROUTPUTLOCATION_NAMESPACE_ENTER namespace Foundry { \
    namespace Katana
#define FNRENDEROUTPUTLOCATION_NAMESPACE_EXIT }
#define FNRENDEROUTPUTLOCATION_NAMESPACE Foundry::Katana

#endif

// Create a namespace instance so that namespace aliases can be declared
FNRENDEROUTPUTLOCATION_NAMESPACE_ENTER
{
}
FNRENDEROUTPUTLOCATION_NAMESPACE_EXIT

// Define the FnRenderOutputLocation namespace alias.
// This is the default namespace to be used to access FnRenderOutputLocation
// classes and types
namespace FnRenderOutputLocation = FNRENDEROUTPUTLOCATION_NAMESPACE;

// Define the FnKat namespace for backward compatibility
#ifndef FNGEOLIB_INTERNAL_NAMESPACE
namespace FnKat = Foundry::Katana;
#endif

#endif // FnRenderOutputLocation_ns_h
