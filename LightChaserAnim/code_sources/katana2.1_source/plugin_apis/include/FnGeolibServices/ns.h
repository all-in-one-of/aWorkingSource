#ifndef FnGeolibServices_ns_h
#define FnGeolibServices_ns_h

#ifdef FNGEOLIB_INTERNAL_NAMESPACE
#include "ns_internal.h"
#else

// Macros used through the code to define API namespaces
#define FNGEOLIBSERVICES_NAMESPACE_ENTER namespace Foundry { \
    namespace Katana
#define FNGEOLIBSERVICES_NAMESPACE_EXIT }
#define FNGEOLIBSERVICES_NAMESPACE Foundry::Katana

#endif

// Create a namespace instance so that namespace aliases can be declared
FNGEOLIBSERVICES_NAMESPACE_ENTER
{
}
FNGEOLIBSERVICES_NAMESPACE_EXIT

// Define the FnGeolibServices namespace alias.
// This is the default namespace to be used to access FnGeolibServices
// classes and types
namespace FnGeolibServices = FNGEOLIBSERVICES_NAMESPACE;

// Define the FnKat namespace for backward compatibility
#ifndef FNGEOLIB_INTERNAL_NAMESPACE
namespace FnKat = Foundry::Katana;
#endif

#endif // FnGeolibServices_ns_h
