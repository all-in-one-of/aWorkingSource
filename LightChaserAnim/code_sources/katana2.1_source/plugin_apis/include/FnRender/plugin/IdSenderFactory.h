#ifndef FNRENDER_IDSENDERFACTORY_H
#define FNRENDER_IDSENDERFACTORY_H

#include "FnRender/plugin/IdSenderInterface.h"

namespace Foundry
{
namespace Katana
{
namespace Render
{
namespace IdSenderFactory
{
    /**
     * \ingroup RenderAPI
     */

    //@{
    /**
     * @name Id Sender Interface
     */
    /**
     * TODO
     */
    IdSenderInterface* getNewInstance(const std::string & host, int64_t frameID);
     //@}

    /**
     * @}
     */
}
}
}
}

namespace FnKat = Foundry::Katana;

#endif
