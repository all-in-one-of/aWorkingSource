#ifndef FnRendererInfo_ns_h
#define FnRendererInfo_ns_h

#ifdef FNGEOLIB_INTERNAL_NAMESPACE
#include "ns_internal.h"
#else

// Macros used through the code to define API namespaces
#define FNRENDERERINFO_NAMESPACE_ENTER namespace Foundry { \
    namespace Katana
#define FNRENDERERINFO_NAMESPACE_EXIT }
#define FNRENDERERINFO_NAMESPACE Foundry::Katana

#endif

// Create a namespace instance so that namespace aliases can be declared
FNRENDERERINFO_NAMESPACE_ENTER
{
}
FNRENDERERINFO_NAMESPACE_EXIT

// Define the FnRendererInfo namespace alias.
// This is the default namespace to be used to access FnRendererInfo
// classes and types
namespace FnRendererInfo = FNRENDERERINFO_NAMESPACE;

// Define the FnKat namespace for backward compatibility
#ifndef FNGEOLIB_INTERNAL_NAMESPACE
namespace FnKat = Foundry::Katana;
#endif

#endif // FnRendererInfo_ns_h
