#ifndef FnAttribute_ns_h
#define FnAttribute_ns_h

#ifdef FNGEOLIB_INTERNAL_NAMESPACE
#include "ns_internal.h"
#else

// Macros used through the code to define API namespaces
#define FNATTRIBUTE_NAMESPACE_ENTER namespace Foundry { \
    namespace Katana
#define FNATTRIBUTE_NAMESPACE_EXIT }
#define FNATTRIBUTE_NAMESPACE Foundry::Katana

#endif

// Create a namespace instance so that namespace aliases can be declared
FNATTRIBUTE_NAMESPACE_ENTER
{
}
FNATTRIBUTE_NAMESPACE_EXIT

// Define the FnAttribute namespace alias.
// This is the default namespace to be used to access FnAttribute classes
// and types
namespace FnAttribute = FNATTRIBUTE_NAMESPACE;

#ifndef FNGEOLIB_INTERNAL_NAMESPACE
namespace FnKat = Foundry::Katana;
#endif

#endif // FnAttribute_ns_h
