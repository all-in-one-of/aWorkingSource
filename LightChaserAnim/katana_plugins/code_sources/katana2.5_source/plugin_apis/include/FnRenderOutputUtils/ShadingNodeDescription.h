// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDEROUTPUTUTILS_SHADINGNODEDESCRIPTION_H
#define FNRENDEROUTPUTUTILS_SHADINGNODEDESCRIPTION_H

// Suite:
#include "FnRenderOutputUtils/suite/FnRenderOutputUtilsSuite.h"
#include "FnRenderOutputUtils/FnRenderOutputUtilsAPI.h"

#include "FnAttribute/FnAttribute.h"

#include <iostream>
#include <vector>


namespace Foundry
{
namespace Katana
{
namespace RenderOutputUtils
{

    // Forward declaration
    class ShadingNodeConnectionDescription;

    /**
     * \ingroup RenderAPI
     */
    /**
     * @brief ShadingNodeDescription
     */
    class FNRENDEROUTPUTUTILS_API ShadingNodeDescription
    {
    public:

        ShadingNodeDescription(ShadingNodeDescriptionHandle handle);
        ~ShadingNodeDescription();

        ShadingNodeDescription(const ShadingNodeDescription& rhs);
        ShadingNodeDescription& operator=(const ShadingNodeDescription& rhs);

        /**
         * isValid
         */
        bool isValid() const {return _handle != 0x0;}

        /**
         * getName
         */
        std::string getName() const;

        /**
         * getType
         */
        std::string getType() const;

        /**
         * getNumberOfParameterNames
         */
        unsigned int getNumberOfParameterNames() const;

        /**
         * getParameterName
         */
        std::string getParameterName(unsigned int index) const;

        /**
         * getParameter
         */
        FnAttribute::Attribute getParameter(const std::string &name) const;

        /**
         * getNumberOfConnectionNames
         */
        unsigned int getNumberOfConnectionNames() const;

        /**
         * getConnectionName
         */
        std::string getConnectionName(unsigned int index) const;

        /**
         * getConnection
         */
        ShadingNodeConnectionDescription getConnection(const std::string &name) const;

    protected:
        void acceptHandle(const ShadingNodeDescription &rhs);

    private:
        ShadingNodeDescriptionHandle _handle;
    };
    /**
     * @}
     */

}
}
}

namespace FnKat = Foundry::Katana;

#endif //RenderOutputUtilsShadingNodeDescription_H
