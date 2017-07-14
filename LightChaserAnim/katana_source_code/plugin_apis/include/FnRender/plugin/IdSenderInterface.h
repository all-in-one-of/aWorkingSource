#ifndef FNRENDER_IDSENDERINTERFACE_H
#define FNRENDER_IDSENDERINTERFACE_H

#include <stdint.h>
#include <string>

namespace Foundry
{
namespace Katana
{
namespace Render
{
    /**
     * \ingroup RenderAPI
     */

    /**
     * @brief An interface for sending (ID, scene graph location name)
     *        pairs to Katana. This mapping is then used by the color
     *        picker to map a picked ID value in the monitor to the
     *        name of the corresponding scene graph location.
     */
    class IdSenderInterface
    {
    public:
        virtual ~IdSenderInterface() {}

        /**
         * Get the next unique ID integer value and the maximum
         * number of IDs.
         *
         * @param nextId A unique incremented ID value
         * @param maxId  The maximum number of ID values
         */
        virtual void getIds(int64_t* nextId, int64_t* maxId) = 0;

        /**
         * Send an ID and scene graph location name pair
         * for a rendered object to Katana.
         *
         * @param id         Unique ID
         * @param objectName Scene graph location name
         */
        virtual void send(int64_t id, const char* const objectName) = 0;
    };
    /**
     * @}
     */
}
}
}

namespace FnKat = Foundry::Katana;

#endif
