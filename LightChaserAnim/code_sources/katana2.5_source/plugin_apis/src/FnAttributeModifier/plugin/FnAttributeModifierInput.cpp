// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <FnAttributeModifier/plugin/FnAttributeModifierInput.h>

namespace Foundry
{
    namespace Katana
    {
        AttributeModifierInput::AttributeModifierInput(FnAttributeModifierHostHandle handle) :
            _handle(handle)
        {
        }

        std::string AttributeModifierInput::getName() const
        {
            return _suite->getName(_handle);
        }

        std::string AttributeModifierInput::getType() const
        {
            return _suite->getType(_handle);
        }

        std::string AttributeModifierInput::getFullName() const
        {
            return _suite->getFullName(_handle);
        }

        float AttributeModifierInput::getFrameTime() const
        {
            return _suite->getFrameTime(_handle);
        }

        std::string AttributeModifierInput::getHostString() const
        {
            return _suite->getHostString(_handle);
        }

        int AttributeModifierInput::getNumSamples() const
        {
            return _suite->getNumSamples(_handle);
        }

        float AttributeModifierInput::getShutterOpen() const
        {
            return _suite->getShutterOpen(_handle);
        }

        float AttributeModifierInput::getShutterClose() const
        {
            return _suite->getShutterClose(_handle);
        }

        Attribute AttributeModifierInput::getAttribute(const std::string &name, bool global,
            const std::string &atLocation) const
        {
            FnAttributeHandle handle = 0x0;

            if (global)
            {
                if (atLocation.empty())
                {
                    handle = _suite->getDelimitedGlobalAttribute(_handle, name.c_str());
                }
                else
                {
                    handle = _suite->getDelimitedGlobalAttributeAtLocation(_handle, atLocation.c_str(), name.c_str());
                }
            }
            else
            {
                if (atLocation.empty())
                {
                    handle = _suite->getDelimitedLocalAttribute(_handle, name.c_str());
                }
                else
                {
                    handle = _suite->getDelimitedLocalAttributeAtLocation(_handle, atLocation.c_str(), name.c_str());
                }
            }

            return Attribute::CreateAndSteal(handle);
        }

        void AttributeModifierInput::getAttributeNames(std::vector<std::string> &names, bool global) const
        {
            const char **attrNames = 0x0;
            unsigned int attrNameCount = 0;

            if (global)
            {
                attrNames = _suite->getGlobalAttributeNames(_handle, &attrNameCount);
            }
            else
            {
                attrNames = _suite->getAttributeNames(_handle, &attrNameCount);
            }

            names.clear();
            if (attrNames)
            {
                names.reserve(attrNameCount);
                names.insert(names.end(), attrNames, attrNames+attrNameCount);
            }
        }

        DoubleAttribute AttributeModifierInput::getWorldSpaceXform(bool multiSample,
            const std::string &atLocation) const
        {
            FnAttributeHandle handle = 0x0;

            if (multiSample)
            {
                if (atLocation.empty())
                {
                    handle = _suite->getMultiSampleWorldSpaceXform(_handle);
                }
                else
                {
                    handle = _suite->getMultiSampleWorldSpaceXformAtLocation(_handle, atLocation.c_str());
                }
            }
            else
            {
                if (atLocation.empty())
                {
                    handle = _suite->getWorldSpaceXform(_handle);
                }
                else
                {
                    handle = _suite->getWorldSpaceXformAtLocation(_handle, atLocation.c_str());
                }
            }

            return Attribute::CreateAndSteal(handle);
        }

        DoubleAttribute AttributeModifierInput::getWorldSpaceXformForSamples(
            const std::vector<float> &samples, const std::string &atLocation) const
        {
            FnAttributeHandle handle = 0x0;

            if (atLocation.empty())
            {
                handle = _suite->getWorldSpaceXformForSamples(_handle, samples.data(), static_cast<unsigned int>(samples.size()));
            }
            else
            {
                handle = _suite->getWorldSpaceXformAtLocationForSamples(_handle, atLocation.c_str(),
                    samples.data(),  static_cast<unsigned int>(samples.size()));
            }

            return Attribute::CreateAndSteal(handle);
        }

        // child names
        void AttributeModifierInput::getChildNames(std::vector<std::string> &names, const std::string &atLocation) const
        {
            const char *locationCStr = 0x0;
            if (!atLocation.empty())
            {
                locationCStr = atLocation.c_str();
            }

            unsigned int childNameCount = 0;
            const char **childNames = _suite->getChildNames(_handle, &childNameCount, locationCStr);

            names.clear();
            if (childNames)
            {
                names.reserve(childNameCount);
                names.insert(names.end(), childNames, childNames+childNameCount);
            }
        }

        // override
        void AttributeModifierInput::addOverride(const std::string &name, Attribute attr, bool inheritGroup)
        {
            _suite->addOverride(_handle, name.c_str(), attr.getHandle(), inheritGroup);
        }


        // HELPER FUNCTIONS, not part of C API
        void AttributeModifierInput::addErrorOverride(const std::string &errorMessage, bool isFatal)
        {
            if (isFatal)
            {
                addOverride("type", StringAttribute("error"), false);
            }
            addOverride("errorMessage", StringAttribute(errorMessage), false);
        }

        void AttributeModifierInput::setAttribute(const std::string &name, Attribute attr, bool inheritGroup)
        {
            addOverride(name, attr, inheritGroup);
        }

        void AttributeModifierInput::deleteAttribute(const std::string &name)
        {
            addOverride(name, Attribute(), false);
        }


        void AttributeModifierInput::setHost(FnPluginHost *host)
        {
            // See the comment in FnAttributeModifier::cook for details on
            // why this test/return is needed.
            if (_host && _suite) return;

            _host = host;
            _suite = (FnAttributeModifierHostSuite_v2*)host->getSuite("AttributeModifierHost", 2);
        }

        FnAttributeModifierHostSuite_v2 *AttributeModifierInput::_suite = 0x0;
        FnPluginHost *AttributeModifierInput::_host = 0x0;

    } //namespace Katana

} //namespace Foundry
