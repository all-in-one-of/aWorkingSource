// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FN_DISPLAY_DRIVER_H
#define FN_DISPLAY_DRIVER_H

/**
 * @defgroup DD Display Driver Host API
 * @{
 *
 * @brief The Display Driver Host API allows to initialize the host.
 */

#include <string>

#include <FnDisplayDriver/suite/FnDisplayDriverSuite.h>
#include "ns.h"

FNDISPLAYDRIVER_NAMESPACE_ENTER
{
namespace FnDisplayDriver
{
  /**
   * Bootstraps the API without having to link against the internal libraries
   * that implement the Display Driver host. This can be useful when
   * implementing an executable that needs to use the Display Driver API via
   * the plug-in system, without having to link against any internal library.
   *
   * Returns true if the bootstrap succeeds.
   */
  bool Bootstrap(const std::string& katanaPath);

  /**
   * Initializes the API with the given Display Driver Host suite.
   */
  void Initialize(const FnDisplayDriverHostSuite_v1 *);
} // namespace FnDisplayDriver
}
FNDISPLAYDRIVER_NAMESPACE_EXIT

/**
 * @}
 */
#endif
