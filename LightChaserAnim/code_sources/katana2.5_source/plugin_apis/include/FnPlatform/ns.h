#ifndef KATANA_PLUGINAPI_FNPLATFORM_NS_H_
#define KATANA_PLUGINAPI_FNPLATFORM_NS_H_

#define FNPLATFORM_NAMESPACE_TOKEN Platform

#ifdef FNGEOLIB_INTERNAL_NAMESPACE
#include <FnPlatform/ns_internal.h>
#else

// Macros used through the code to define API namespaces
#define FNPLATFORM_NAMESPACE_ENTER namespace Foundry { \
    namespace Katana { \
    namespace FNPLATFORM_NAMESPACE_TOKEN
#define FNPLATFORM_NAMESPACE_EXIT } }
#define FNPLATFORM_NAMESPACE Foundry::Katana::FNPLATFORM_NAMESPACE_TOKEN

#endif  // FNGEOLIB_INTERNAL_NAMESPACE

// Create a namespace instance so that namespace aliases can be declared
FNPLATFORM_NAMESPACE_ENTER
{
}
FNPLATFORM_NAMESPACE_EXIT

// Define the FnPlatform namespace alias.
// This is the default namespace to be used to access FnPlatform
// classes and types
namespace FnPlatform = FNPLATFORM_NAMESPACE;

#endif  // KATANA_PLUGINAPI_FNPLATFORM_NS_H_
