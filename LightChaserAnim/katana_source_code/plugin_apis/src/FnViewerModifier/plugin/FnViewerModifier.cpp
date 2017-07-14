// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnViewerModifier/plugin/FnViewerModifier.h>
#include <FnViewerModifier/plugin/FnViewerModifierInput.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnLogging/FnLogging.h>
#include <FnAttribute/FnAttribute.h>
#include <FnAsset/FnDefaultAssetPlugin.h>
#include <FnAsset/FnDefaultFileSequencePlugin.h>

#include <memory>
#include <string>
#include <iostream>


struct FnViewerModifierStruct
{
public:
  FnViewerModifierStruct(Foundry::Katana::ViewerModifier* vip);
  ~FnViewerModifierStruct();
  Foundry::Katana::ViewerModifier& getViewerModifier() { return *_viewerModifier; }

private:
  std::auto_ptr<Foundry::Katana::ViewerModifier>  _viewerModifier;
};

FnViewerModifierStruct::FnViewerModifierStruct(Foundry::Katana::ViewerModifier* vip) :
  _viewerModifier(vip)
{
  // Empty
}

FnViewerModifierStruct::~FnViewerModifierStruct()
{
  // Empty
}

void destroy(FnViewerModifierHandle handle)
{
  delete handle;
}

void deepSetup(FnViewerModifierHandle handle, FnViewerModifierHostHandle host)
{
  Foundry::Katana::ViewerModifierInput input(host);
  handle->getViewerModifier().deepSetup(input);
}

void setup(FnViewerModifierHandle handle, FnViewerModifierHostHandle host)
{
  Foundry::Katana::ViewerModifierInput input(host);
  handle->getViewerModifier().setup(input);
}

void draw(FnViewerModifierHandle handle, FnViewerModifierHostHandle host)
{
  Foundry::Katana::ViewerModifierInput input(host);
  handle->getViewerModifier().draw(input);
}

void cleanup(FnViewerModifierHandle handle, FnViewerModifierHostHandle host)
{
  Foundry::Katana::ViewerModifierInput input(host);
  handle->getViewerModifier().cleanup(input);
}

void deepCleanup(FnViewerModifierHandle handle, FnViewerModifierHostHandle host)
{
  Foundry::Katana::ViewerModifierInput input(host);
  handle->getViewerModifier().deepCleanup(input);
}


FnAttributeHandle getWorldSpaceBoundingBox(FnViewerModifierHandle handle, FnViewerModifierHostHandle host)
{
    std::cerr << "ViewerModifier getWorldSpaceBoundingBox() is deprecated. "
              << "You should use Katana 1.1 or later in order to use this plugin!"
              << std::endl;

    Foundry::Katana::NullAttribute attr;
    if (attr.isValid())
    {
      attr.getSuite()->retainAttr(attr.getHandle());
    }
    return attr.getHandle();
}


FnAttributeHandle getLocalSpaceBoundingBox(FnViewerModifierHandle handle, FnViewerModifierHostHandle host)
{
  Foundry::Katana::ViewerModifierInput input(host);
  Foundry::Katana::Attribute attr = handle->getViewerModifier().getLocalSpaceBoundingBox(input);
  if (attr.isValid())
  {
    attr.getSuite()->retainAttr(attr.getHandle());
  }
  return attr.getHandle();
}

namespace Foundry
{
  namespace Katana
  {

    /*
     * DEPRECTATION NOTE:
     * Since this function is deprecated it will not be a pure virtual, so it
     * needs a standard implementation. This returns an empty bounding box.
     */
    DoubleAttribute  ViewerModifier::getWorldSpaceBoundingBox(ViewerModifierInput& input)
    {
      return DoubleAttribute();
    }

    /* DEPRECTATION NOTE:
     * If the pugin has an implementation for getWorldSpaceBoundingBox() and
     * none for getLocalSpaceBoundingBox() then by default we will call
     * the current implementation of getWorldSpaceBoundingBox().
     */
    DoubleAttribute  ViewerModifier::getLocalSpaceBoundingBox(ViewerModifierInput& input)
    {
      return getWorldSpaceBoundingBox(input);
    }


    FnPlugStatus ViewerModifier::setHost(FnPluginHost* host)
    {
      _host = host;
      ViewerModifierInput::setHost(host);
      GroupBuilder::setHost(host);
      Attribute::setHost(host);
      DefaultAssetPlugin::setHost(host);
      DefaultFileSequencePlugin::setHost(host);
      return FnLogging::setHost(host);
    }

    FnPluginHost* ViewerModifier::getHost() { return _host; }

    FnViewerModifierHandle ViewerModifier::newViewerModifierHandle(ViewerModifier* vm)
    {
      if (!vm)
        return 0x0;

      FnViewerModifierHandle h = (FnViewerModifierHandle)new FnViewerModifierStruct(vm);
      return h;
    }

    FnViewerModifierPluginSuite_v1 ViewerModifier::createSuite(FnViewerModifierHandle (*create)(FnAttributeHandle),
                                                               FnAttributeHandle (*getArgumentTemplate)(),
                                                               const char* (*getLocationType)())
    {
      FnViewerModifierPluginSuite_v1 suite = {
        create,
        getArgumentTemplate,
        getLocationType,
        ::destroy,
        ::deepSetup,
        ::setup,
        ::draw,
        ::cleanup,
        ::deepCleanup,
        ::getWorldSpaceBoundingBox,
        ::getLocalSpaceBoundingBox,
      };
      return suite;
    }

    unsigned int  ViewerModifier::_apiVersion = 1;
    const char*   ViewerModifier::_apiName = "ViewerModifierPlugin";
    FnPluginHost* ViewerModifier::_host = 0x0;
  } // namespace Katana
} // namespace Foundry
