// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDER_GLOBALSETTINGS_H
#define FNRENDER_GLOBALSETTINGS_H

#include "FnScenegraphIterator/FnScenegraphIterator.h"
#include "FnAttribute/FnAttribute.h"

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
     * @brief A utility class which provides a renderer's global settings from the Katana recipe
     * if they are used. The attributes can be found on the root scene graph location at
     * <i>[rendererName]GlobalStatements</i>.
     * This class is a placeholder and can be extended to provide renderer specific parsing and
     * access functions.
     */
    class GlobalSettings
    {
    public:
        /**
         * @param rootIterator      The root scene graph iterator
         * @param rendererNamespace The namespace used (typically the renderer name) to define
         *                          the global settings group attribute for the renderer
         *                          (<i>[rendererNamespace]GlobalStatements</i>).
         */
        GlobalSettings(FnScenegraphIterator rootIterator, const std::string& rendererNamespace);

        virtual ~GlobalSettings() {}

        /**
         * Parses the the global settings attributes
         *
         * @param rendererNamespace The namespace used (typically the renderer name) to define
         *                          the global settings group attribute for the renderer
         *                          (<i>[rendererNamespace]GlobalStatements</i>).
         * @return A zero value if successful, a non-zero value otherwise
         */
        virtual int initialise(const std::string& rendererNamespace);

        /**
         * @return The group attribute containing the global settings (<i>[rendererNamespace]GlobalStatements</i>)
         */
        GroupAttribute getGlobalSettingsAttr() const { return _globalSettingsAttr; }

        /**
         * Finds an Attribute under the global settings by name
         *
         * @param attributeName The name of a global settings attribute
         * @return The named Attribute if found (<i>[rendererNamespace]GlobalStatements.[attributeName]</i>)
         */
        Attribute getAttribute(const std::string& attributeName) const { return _globalSettingsAttr.getChildByName(attributeName); }

    protected:
        FnScenegraphIterator _rootIterator;
        GroupAttribute _globalSettingsAttr;
    };
    /**
     * @}
     */
}
}
}

namespace FnKat = Foundry::Katana;

#endif
