// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDEROUTPUTUTILS_SHADINGNODECONNECTIONDESCRIPTION_H
#define FNRENDEROUTPUTUTILS_SHADINGNODECONNECTIONDESCRIPTION_H

// Suite:
#include "FnRenderOutputUtils/suite/FnRenderOutputUtilsSuite.h"
#include "FnRenderOutputUtils/FnRenderOutputUtilsAPI.h"

#include <iostream>
#include <vector>


namespace Foundry
{
namespace Katana
{
namespace RenderOutputUtils
{

    /**
     * \ingroup RenderAPI
     */
    /**
     * @brief ShadingNodeConnectionDescription
     */
    class FNRENDEROUTPUTUTILS_API ShadingNodeConnectionDescription
    {
    public:

        ShadingNodeConnectionDescription(ShadingNodeConnectionDescriptionHandle handle);
        ~ShadingNodeConnectionDescription();

        ShadingNodeConnectionDescription(const ShadingNodeConnectionDescription& rhs);
        ShadingNodeConnectionDescription& operator=(const ShadingNodeConnectionDescription& rhs);

        /**
         * isValid
         */
        bool isValid() const {return _handle != 0x0;}

        /**
         * getName
         */
        std::string getName() const;

        /**
         * getConnectedNodeName
         */
        std::string getConnectedNodeName() const;

        /**
         * getConnectedPortName
         */
        std::string getConnectedPortName() const;

    protected:
        void acceptHandle(const ShadingNodeConnectionDescription &rhs);

    private:
        ShadingNodeConnectionDescriptionHandle _handle;
    };
    /**
     * @}
     */

}
}
}

namespace FnKat = Foundry::Katana;

#endif
