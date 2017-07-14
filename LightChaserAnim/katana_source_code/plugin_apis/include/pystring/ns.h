#ifndef pystring_ns_h
#define pystring_ns_h

#define GEOLIB3_PYSTRING_NAMESPACE_TOKEN pystring_v1

#ifdef FNGEOLIB_INTERNAL_NAMESPACE
#include "ns_internal.h"
#else

// Macros used through the code to define API namespaces
#define GEOLIB3_PYSTRING_NAMESPACE_ENTER namespace Foundry { \
    namespace Katana { \
    namespace GEOLIB3_PYSTRING_NAMESPACE_TOKEN
#define GEOLIB3_PYSTRING_NAMESPACE_EXIT } }
#define GEOLIB3_PYSTRING_NAMESPACE Foundry::Katana::GEOLIB3_PYSTRING_NAMESPACE_TOKEN

#endif

// Create a namespace instance so that namespace aliases can be declared
GEOLIB3_PYSTRING_NAMESPACE_ENTER
{
}
GEOLIB3_PYSTRING_NAMESPACE_EXIT

// Define the pystring namespace alias.
// This is the default namespace to be used to access pystring classes and types
namespace pystring = GEOLIB3_PYSTRING_NAMESPACE;

#endif // pystring_ns_h
