#ifndef FnConfig_ns_h
#define FnConfig_ns_h

#ifdef FNGEOLIB_INTERNAL_NAMESPACE
#include "ns_internal.h"
#else

// Macros used through the code to define API namespaces
#define FNCONFIG_NAMESPACE_ENTER namespace Foundry { \
    namespace Katana
#define FNCONFIG_NAMESPACE_EXIT }
#define FNCONFIG_NAMESPACE Foundry::Katana

#endif

// Create a namespace instance so that namespace aliases can be declared
FNCONFIG_NAMESPACE_ENTER
{
}
FNCONFIG_NAMESPACE_EXIT

// Define the FnConfig namespace alias.
// This is the default namespace to be used to access FnConfig classes and types
namespace FnConfig = FNCONFIG_NAMESPACE;

// Define the FnKat namespace for backward compatibility
#ifndef FNGEOLIB_INTERNAL_NAMESPACE
namespace FnKat = Foundry::Katana;
#endif

#endif // FnConfig_ns_h
