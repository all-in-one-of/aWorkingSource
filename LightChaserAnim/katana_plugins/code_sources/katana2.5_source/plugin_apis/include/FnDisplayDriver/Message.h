// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef MESSAGE_H
#define MESSAGE_H

#include <FnDisplayDriver/suite/FnDisplayDriverSuite.h>
#include <FnPluginSystem/FnPluginSystem.h>

namespace Foundry
{
  namespace Katana
  {
    class KatanaPipe;

    /** @addtogroup DD
     *  @{
     */

    /** @brief This is the base class against which all Messages sent to Katana
     *  should inherit from.
     */
    class Message
    {
    public:
      Message() : _handle(0) {}
      virtual ~Message();

      /** @brief Check if the message has a valid handle to the host suite.
       *
       * @return true if the message handle can be used to get/set data and send
       * through the communication pipe.
       */
      bool isValid() const
      {
        return _handle != 0;
      }

      /** @brief Set the Display Driver Host to delegate message initialization
       * and protocol details to the suite.
       *
       * The KatanaPipeSingleton bootstraps and sets the host suite, and is
       * recommended to initialize the message suite.
       */
      static FnPlugStatus setHost(FnPluginHost *host);

      static void setSuite(const FnDisplayDriverHostSuite_v1 *suite);

      /** @brief Get the Display Driver host suite for messages. If invalid, an
       * error message is output to the standard error stream.
       *
       * @return the Display Driver host suite.
       */
      static const FnDisplayDriverHostSuite_v1 *getSuite();

    protected:
      friend class KatanaPipe;
      FnMessageHandle _handle;

      static const FnDisplayDriverHostSuite_v1 *_suite;
    };
    /**
     * @}
     */
  };
};

#endif
