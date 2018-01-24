#ifndef FnAsset_ns_h
#define FnAsset_ns_h

#ifdef FNGEOLIB_INTERNAL_NAMESPACE
#include "ns_internal.h"
#else

// Macros used through the code to define API namespaces
#define FNASSET_NAMESPACE_ENTER namespace Foundry { \
    namespace Katana
#define FNASSET_NAMESPACE_EXIT }
#define FNASSET_NAMESPACE Foundry::Katana

#endif

// Create a namespace instance so that namespace aliases can be declared
FNASSET_NAMESPACE_ENTER
{
}
FNASSET_NAMESPACE_EXIT

// Define the FnAsset namespace alias.
// This is the default namespace to be used to access FnAsset classes and types
namespace FnAsset = FNASSET_NAMESPACE;

// Define the FnKat namespace for backward compatibility
#ifndef FNGEOLIB_INTERNAL_NAMESPACE
namespace FnKat = Foundry::Katana;
#endif

#endif // FnAsset_ns_h
