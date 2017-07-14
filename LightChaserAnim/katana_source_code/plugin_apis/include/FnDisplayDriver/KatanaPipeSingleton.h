// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef KATANA_PIPE_SINGLETON_H
#define KATANA_PIPE_SINGLETON_H

#include "KatanaPipe.h"

namespace Foundry
{
  namespace Katana
  {
    /**
     * @ingroup DD
     * @brief The PipeSingleton provides a singleton wrapper around the
     * KatanaPipe
     *
     * @sa KatanaPipe
     */
    class PipeSingleton
    {
    public:
      /**
       * Return the KatanaPipe connected to the given hostName and portNumber
       *
       * Currently, we only support the connection back to one KCD i.e. the
       * first call to this function will define the hostName/portnumber
       * combination that will be connected to. Subsequent calls to this function
       * with different hostname portnumber combinations will continue to
       * return the original pipe.
       *
       * @param[in] hostName   the hostName to be connected to as a fully
       *                       qualified domain name.
       * @param[in] portNumber the TCP port number to the Catalog Server is
       *                       listening on.
       *
       * @return the KatanaPipe connected to the specified end point.
       */
      static KatanaPipe* Instance(const std::string &hostName, unsigned int portNumber);

    private:
      PipeSingleton();
      PipeSingleton(const PipeSingleton& rhs);
      ~PipeSingleton();

      static KatanaPipe *_pipe;
    };
  };
};

#endif
