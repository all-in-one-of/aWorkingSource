#ifndef FNRENDER_IDSENDERFACTORY_H
#define FNRENDER_IDSENDERFACTORY_H

#include "FnRender/plugin/IdSenderInterface.h"
#include "FnRender/FnRenderAPI.h"

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
    FNRENDER_API IdSenderInterface* getNewInstance(const std::string & host,
                                                   int64_t frameID);
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
