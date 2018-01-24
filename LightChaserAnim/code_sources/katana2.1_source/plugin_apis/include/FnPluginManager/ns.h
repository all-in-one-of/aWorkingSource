#ifndef FnPluginManager_ns_h
#define FnPluginManager_ns_h

#ifdef FNGEOLIB_INTERNAL_NAMESPACE
#include "ns_internal.h"
#else

// Macros used through the code to define API namespaces
#define FNPLUGINMANAGER_NAMESPACE_ENTER namespace Foundry { \
    namespace Katana
#define FNPLUGINMANAGER_NAMESPACE_EXIT }
#define FNPLUGINMANAGER_NAMESPACE Foundry::Katana

#endif

// Create a namespace instance so that namespace aliases can be declared
FNPLUGINMANAGER_NAMESPACE_ENTER
{
}
FNPLUGINMANAGER_NAMESPACE_EXIT

// Define the FnPluginManager namespace alias.
// This is the default namespace to be used to access FnPluginManager
// classes and types
namespace FnPluginManager = FNPLUGINMANAGER_NAMESPACE;

// Define the FnKat namespace for backward compatibility
#ifndef FNGEOLIB_INTERNAL_NAMESPACE
namespace FnKat = Foundry::Katana;
#endif

#endif // FnPluginManager_ns_h
