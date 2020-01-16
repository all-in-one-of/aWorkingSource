// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnViewerModifier/plugin/FnViewerModifierInput.h>

namespace Foundry
{
  namespace Katana
  {
    ViewerModifierInput::ViewerModifierInput(FnViewerModifierHostHandle handle) :
      _handle(handle)
    {
      // Empty
    }

    void ViewerModifierInput::setHost(FnPluginHost* host)
    {
      _host = host;
      _suite = (FnViewerModifierHostSuite_v1*)host->getSuite("ViewerModifierHost", 1);
    }

    std::string ViewerModifierInput::getName() const
    {
      return _suite->getName(_handle);
    }

    std::string ViewerModifierInput::getType() const
    {
      return _suite->getType(_handle);
    }

    std::string ViewerModifierInput::getFullName() const
    {
      return _suite->getFullName(_handle);
    }

    bool ViewerModifierInput::isSelected() const
    {
      return _suite->isSelected(_handle);
    }

    Attribute ViewerModifierInput::getAttribute(const std::string& name, const std::string& atLocation) const
    {
      return Attribute::CreateAndSteal(
        _suite->getAttribute(_handle, name.c_str(), atLocation.c_str()));
    }

    Attribute ViewerModifierInput::getGlobalAttribute(const std::string& name, const std::string& atLocation) const
    {
      return Attribute::CreateAndSteal(
        _suite->getGlobalAttribute(_handle, name.c_str(), atLocation.c_str()));
    }

    DoubleAttribute ViewerModifierInput::getWorldSpaceXform(const std::string& atLocation) const
    {
      return Attribute::CreateAndSteal(
        _suite->getWorldSpaceXform(_handle, atLocation.c_str()));
    }

    bool ViewerModifierInput::isLiveAttribute(const std::string& attribute, const std::string& atLocation) const
    {
      return _suite->isLiveAttribute(_handle, attribute.c_str(), atLocation.c_str());
    }

    Attribute ViewerModifierInput::getLiveAttribute(const std::string& name, const std::string& atLocation) const
    {
      return Attribute::CreateAndSteal(
        _suite->getLiveAttribute(_handle, name.c_str(), atLocation.c_str()));
    }

    bool ViewerModifierInput::isGlobalLiveAttribute(const std::string& attribute, const std::string& atLocation) const
    {
      return _suite->isGlobalLiveAttribute(_handle, attribute.c_str(), atLocation.c_str());
    }

    Attribute ViewerModifierInput::getGlobalLiveAttribute(const std::string& name, const std::string& atLocation) const
    {
      return Attribute::CreateAndSteal(
        _suite->getGlobalLiveAttribute(
            _handle, name.c_str(), atLocation.c_str()));
    }


    Attribute ViewerModifierInput::getUniversalAttribute(const std::string& name, const std::string& atLocation) const
    {
        return Attribute::CreateAndSteal(
          _suite->getUniversalAttribute(
            _handle, name.c_str(), atLocation.c_str()));
    }


    bool ViewerModifierInput::isLiveWorldSpaceXform(const std::string& atLocation) const
    {
      return _suite->isLiveWorldSpaceXform(_handle, atLocation.c_str());
    }

    DoubleAttribute ViewerModifierInput::getLiveWorldSpaceXform(const std::string& atLocation) const
    {
      return Attribute::CreateAndSteal(
        _suite->getLiveWorldSpaceXform(_handle, atLocation.c_str()));
    }

    void ViewerModifierInput::getChildNames(std::vector<std::string> &names, const std::string &atLocation) const
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
        names.insert(names.end(), childNames, childNames + childNameCount);
      }
    }

    void ViewerModifierInput::setUseFixedFunctionPipeline(bool useFixedFunction)
    {
      _suite->setUseFixedFunctionPipeline(_handle, useFixedFunction);
    }

    void ViewerModifierInput::setTransparent(bool useTransparent)
    {
      _suite->setTransparent(_handle, useTransparent);
    }

    DoubleAttribute ViewerModifierInput::getDisplayMatrix(MatrixType matrix) const
    {
      return Attribute::CreateAndSteal(
          _suite->getDisplayMatrix(_handle, matrix));
    }

    DoubleAttribute ViewerModifierInput::getViewport() const
    {
      return Attribute::CreateAndSteal(_suite->getViewport(_handle));
    }

    int ViewerModifierInput::getDrawOption(const std::string& option) const
    {
      return _suite->getDrawOption(_handle, option.c_str());
    }

    float ViewerModifierInput::getDrawOptionFloat(const std::string& option) const
    {
      return _suite->getDrawOptionFloat(_handle, option.c_str());
    }

    const std::string ViewerModifierInput::getDrawOptionString(const std::string& option) const
    {
      return _suite->getDrawOptionString(_handle, option.c_str());
    }

    void ViewerModifierInput::overrideHostGeometry()
    {
      return _suite->overrideHostGeometry(_handle);
    }

    bool ViewerModifierInput::isLookedThrough(bool includeChildren)
    {
      return _suite->isLookedThrough(_handle, includeChildren);
    }

    FnViewerModifierHostSuite_v1* ViewerModifierInput::_suite = 0x0;
    FnPluginHost*                 ViewerModifierInput::_host = 0x0;
  } // namespace Katana
} // namespace Foundry
