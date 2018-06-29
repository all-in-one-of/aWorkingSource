#ifndef FnDefaultAttributeProducer_ns_h
#define FnDefaultAttributeProducer_ns_h

#ifdef FNGEOLIB_INTERNAL_NAMESPACE
#include "ns_internal.h"
#else

// Macros used through the code to define API namespaces
#define FNDEFAULTATTRIBUTEPRODUCER_NAMESPACE_ENTER namespace Foundry { \
    namespace Katana
#define FNDEFAULTATTRIBUTEPRODUCER_NAMESPACE_EXIT }
#define FNDEFAULTATTRIBUTEPRODUCER_NAMESPACE Foundry::Katana

#endif

// Create a namespace instance so that namespace aliases can be declared
FNDEFAULTATTRIBUTEPRODUCER_NAMESPACE_ENTER
{
}
FNDEFAULTATTRIBUTEPRODUCER_NAMESPACE_EXIT

// Define the FnDefaultAttributeProducer namespace alias.
// This is the default namespace to be used to access
// FnDefaultAttributeProducer classes and types
namespace FnDefaultAttributeProducer = FNDEFAULTATTRIBUTEPRODUCER_NAMESPACE;

// Define the FnKat namespace for backward compatibility
#ifndef FNGEOLIB_INTERNAL_NAMESPACE
namespace FnKat = Foundry::Katana;
#endif

#endif // FnDefaultAttributeProducer_ns_h
