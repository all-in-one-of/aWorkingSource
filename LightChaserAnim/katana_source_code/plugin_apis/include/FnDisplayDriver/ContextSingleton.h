// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef CONTEXT_SINGLETON_H
#define CONTEXT_SINGLETON_H

#include <zmq.h>

#define NUM_IO_THREADS 1

namespace Foundry
{
  namespace Katana
  {
    /** @addtogroup DD
     *  @{
     */

    /**
     * @brief Provides a wrapper around the Zero MQ context ensuring that only
     * one instance is ever created.
     */
    class ContextSingleton
    {
    public:
      /**
       * @brief Static function to return the valid zero mq context.
       */
      static void* Instance();

    private:
      ContextSingleton();
      ContextSingleton(const ContextSingleton& rhs);
      ~ContextSingleton();

      static void *_context;

    };
    /**
     * @}
     */
  };
};


#endif
