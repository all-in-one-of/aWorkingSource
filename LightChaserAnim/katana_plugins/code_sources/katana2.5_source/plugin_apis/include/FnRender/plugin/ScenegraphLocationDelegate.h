// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDER_SCENEGRAPHLOCATIONDELEGATE_H
#define FNRENDER_SCENEGRAPHLOCATIONDELEGATE_H

#include <vector>

#include "FnPluginSystem/FnPlugin.h"

#include "FnRender/FnRenderAPI.h"
#include "FnRender/suite/FnScenegraphLocationDelegateSuite.h"

#include "FnScenegraphIterator/FnScenegraphIterator.h"
#include "FnScenegraphIterator/suite/FnScenegraphIteratorSuite.h"

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
     *
     * @brief Super-Class that enables the creation of user-defined sub-classes and plug-ins responsible for processing/rendering different location types.
     *
     * For each location type a class associated with a plug-in can be defined that processes/renders information related to the location into something else. A common cases are to retrieve location
     * information from the scene graph iterator, render its geometry, or output a file. A ScenegraphLocationDelegate can be used as a generic way to handle locations and perform any operation
     *
     * Using the macros DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN and REGISTER_PLUGIN a location plug-in is defined and it is detected by Katana at runtime.
     *
     * For example: <br>
     *    DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(ArnoldSphereScenegraphLocationDelegate) <br>
     *    REGISTER_PLUGIN(ArnoldSphereScenegraphLocationDelegate, "ArnoldSphereScenegraphLocationDelegate", 0, 1)
     *
     *    associates the ScenegraphLocationDelegate sub-class ArnoldSphereScenegraphLocationDelegate to be responsible for processing/rendering a location type named "sphere" for the "arnold" renderer.
     *    The second  parameter must a be a Katana-wide unique string. The registration gives priority to the plugins that are discovered first in the directory hierarchy.
     *
     * Another requirement for the sub-classes is to provide 4 additional methods e.g.: <br>
     *      static void flush(); reset/update the state of the object<br>
     *      static XXX* create(); creates an instance of the sub-class XXX <br>
     *      std::string getSupportedRenderer() returns a string of the supported renderer name. If the string is empty then all renderers are supported. If there is a delegate that supports a specific renderer for a given location it is given priority over any delegate that supports all renderers.
     *      void fillSupportedLocationList(std::vector<std::string>& supportedLocationList) populates a vector of strings naming all the supported locations, e.g. a "polymesh"
     */
    class FNRENDER_API ScenegraphLocationDelegate
    {
    public:
        virtual ~ScenegraphLocationDelegate() {}
        void bootstrap(void* ptr) {};

        /**
         * @brief Process/renders a given location, must be implemented by sub-classes.
         *
         * Method responsible for processing/rendering a given location, passed as a scene graph iterator.
         * Return is implementation depended.
         *
         * @param sgIterator: the iterator referring to a location. Its expected value matches the iterator type used when then subclass was registered with the REGISTER_PLUGIN macro.
         * @param optionalInput: optional input, its use and value depends on the implementation.
         */
        virtual void* process(FnScenegraphIterator sgIterator, void* optionalInput) = 0;
        virtual std::string getSupportedRenderer() const { return std::string(); };
        virtual void fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const = 0;

        static FnScenegraphLocationDelegatePluginSuite_v1 createSuite(FnLocationHandle(*create)());
        static FnPlugStatus setHost(FnPluginHost* host);
        static FnLocationHandle newLocationHandle(ScenegraphLocationDelegate* location);

        static unsigned int _apiVersion;
        static const char* _apiName;

    private:
        static FnPluginHost * _host;
    };

    /**
     * @}
     */
}
}
}

namespace FnKat = Foundry::Katana;

#define DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(LOCATION_CLASS)            \
                                                                              \
    FnPlugin LOCATION_CLASS##_plugin;                                         \
                                                                              \
    FnLocationHandle LOCATION_CLASS##_create()                                \
    {                                                                         \
        return Foundry::Katana::Render::ScenegraphLocationDelegate::          \
                newLocationHandle(LOCATION_CLASS::create());                  \
    }                                                                         \
                                                                              \
    FnScenegraphLocationDelegatePluginSuite_v1 LOCATION_CLASS##_suite =       \
            Foundry::Katana::Render::ScenegraphLocationDelegate::createSuite( \
                    LOCATION_CLASS##_create);                                 \
                                                                              \
    const void* LOCATION_CLASS##_getSuite()                                   \
    {                                                                         \
        return &LOCATION_CLASS##_suite;                                       \
    }

#endif
