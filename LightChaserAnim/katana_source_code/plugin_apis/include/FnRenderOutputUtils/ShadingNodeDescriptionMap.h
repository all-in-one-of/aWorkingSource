// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDEROUTPUTUTILS_SHADINGNODEDESCRIPTIONMAP_H
#define FNRENDEROUTPUTUTILS_SHADINGNODEDESCRIPTIONMAP_H

// Suite:
#include "FnRenderOutputUtils/suite/FnRenderOutputUtilsSuite.h"

#include "FnAttribute/FnAttribute.h"
#include "FnScenegraphIterator/FnScenegraphIterator.h"

#include <iostream>

#ifdef _WIN32
  #define RENDEROUTPUTUTILSAPI
#else
  #define RENDEROUTPUTUTILSAPI __attribute__ ((visibility("default")))
#endif

namespace Foundry
{
namespace Katana
{
namespace RenderOutputUtils
{
    // Forward declaration
    class ShadingNodeDescription;

    /**
     * \ingroup RenderAPI
     */
    /**
     * @brief ShadingNodeDescriptionMap
     */
    class RENDEROUTPUTUTILSAPI ShadingNodeDescriptionMap
    {
    public:
        /**
         * ShadingNodeDescriptionMap
         */
        ShadingNodeDescriptionMap(FnAttribute::GroupAttribute materialAttr);

        /**
         * ShadingNodeDescriptionMap
         */
        ShadingNodeDescriptionMap(FnScenegraphIterator sgIterator);

        ~ShadingNodeDescriptionMap();

        /**
         * isValid
         */
        bool isValid() const {return _handle != 0x0;}

        /**
         * getShadingNodeDescriptionByName
         */
        ShadingNodeDescription getShadingNodeDescriptionByName(const std::string &name) const;

    private:
        FnShadingNodeDescriptionMapHandle _handle;

        // no copy/assign
        ShadingNodeDescriptionMap(const ShadingNodeDescriptionMap& rhs);
        ShadingNodeDescriptionMap& operator=(const ShadingNodeDescriptionMap& rhs);
    };
    /**
     * @}
     */

}
}
}

namespace FnKat = Foundry::Katana;

#endif
