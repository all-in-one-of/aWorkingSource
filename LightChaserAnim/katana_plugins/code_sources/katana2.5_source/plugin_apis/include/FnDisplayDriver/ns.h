// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnDisplayDriver_ns_h
#define FnDisplayDriver_ns_h

// Macros used through the code to define API namespaces
#define FNDISPLAYDRIVER_NAMESPACE_ENTER namespace Foundry { \
    namespace Katana
#define FNDISPLAYDRIVER_NAMESPACE_EXIT }
#define FNDISPLAYDRIVER_NAMESPACE Foundry::Katana

// Create a namespace instance so that namespace aliases can be declared
FNDISPLAYDRIVER_NAMESPACE_ENTER
{
}
FNDISPLAYDRIVER_NAMESPACE_EXIT

// Define the FnDisplayDriver namespace alias.
// This is the default namespace to be used to access FnDisplayDriver classes
// and types
namespace FnDisplayDriver = FNDISPLAYDRIVER_NAMESPACE;

#endif // FnDisplayDriver_ns_h
