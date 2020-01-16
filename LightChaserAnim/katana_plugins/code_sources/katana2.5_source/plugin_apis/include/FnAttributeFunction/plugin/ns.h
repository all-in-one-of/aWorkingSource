#ifndef FnAttributeFunction_ns_h
#define FnAttributeFunction_ns_h

#ifdef FNGEOLIB_INTERNAL_NAMESPACE
#include "ns_internal.h"
#else

// Macros used through the code to define API namespaces
#define FNATTRIBUTEFUNCTION_NAMESPACE_ENTER namespace Foundry { \
    namespace Katana
#define FNATTRIBUTEFUNCTION_NAMESPACE_EXIT }
#define FNATTRIBUTEFUNCTION_NAMESPACE Foundry::Katana

#endif

// Create a namespace instance so that namespace aliases can be declared
FNATTRIBUTEFUNCTION_NAMESPACE_ENTER
{
}
FNATTRIBUTEFUNCTION_NAMESPACE_EXIT

// Define the FnAttributeFunction namespace alias.
// This is the default namespace to be used to access
// FnAttributeFunction classes and types
namespace FnAttributeFunction = FNATTRIBUTEFUNCTION_NAMESPACE;

// Define the FnKat namespace for backward compatibility
#ifndef FNGEOLIB_INTERNAL_NAMESPACE
namespace FnKat = Foundry::Katana;
#endif

#endif // FnAttributeFunction_ns_h
