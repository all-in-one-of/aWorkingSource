// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDER_CAMERASETTINGS_H
#define FNRENDER_CAMERASETTINGS_H

#include "FnScenegraphIterator/FnScenegraphIterator.h"
#include "FnAttribute/FnAttribute.h"

#include "FnRenderOutputUtils/FnRenderOutputUtils.h"

#include <map>
#include <tr1/memory>

namespace Foundry
{
namespace Katana
{
namespace Render
{
    /**
     * \ingroup RenderAPI
     * @{
     */

    /**
     * @brief A utility class which provides Katana's camera settings by parsing
     * attribute data on camera scene graph locations.
     * This class can be extended if renderer specific processing is required.
     *
     * \note Where applicable, the getter functions refer to the camera settings
     *       attribute on the camera scene graph location.
     */
    class CameraSettings
    {
    public:

        // Alias for a shared pointer to CameraSettings
        typedef std::tr1::shared_ptr<CameraSettings> Ptr;

        /**
         * @param iterator   The root scene graph iterator
         * @param cameraName The camera scene graph location name (<i>/root/cam/[cameraName]</i>)
         */
        CameraSettings(FnScenegraphIterator iterator, const std::string & cameraName = "") : _iterator(iterator),
                                                                                             _cameraName(cameraName),
                                                                                             _projection("perspective"),
                                                                                             _fov(70.0f),
                                                                                             _orthographicWidth(30.0f) {}

        virtual ~CameraSettings() {}

        /**
         * Handles the camera attribute parsing
         *
         * @param displayWindow The display window (affects the camera's screen window)
         * @param overscan The overscan (affects the camera's screen window)
         * @param shutterClose Shutter close (used to calculate the camera's transforms)
         * @return A zero value if successful, a non-zero value otherwise
         */
        virtual int initialise(int displayWindow[4], int overscan[4], float shutterClose = 0.0f);

        /**
         * @return The camera's scene graph location name.
         */
        std::string getName() const { return _cameraName; }

        /**
         * @return The camera's projection, e.g. 'perspective' (<i>geometry.projection</i>)
         */
        std::string getProjection() const { return _projection; }

        /**
         * @return The camera's field of view (<i>geometry.fov</i>)
         */
        float getFov() const { return _fov; }

        /**
         * @return The camera's clipping plane (near, far) (<i>geometry.near</i> and <i>geometry.far</i>)
         */
        void getClipping(float clipping[2]) const;

        /**
         * @return The camera's screen window (left, right, bottom, top)
         */
        void getScreenWindow(float screenWindow[4]) const;

        /**
         * @return The camera's transform (inverse xform attribute list)
         */
        std::vector<FnKat::RenderOutputUtils::Transform> getTransforms() const { return _transforms; }

    protected:
        FnScenegraphIterator _iterator;

        std::string _cameraName;
        std::string _projection;
        float _fov;
        float _orthographicWidth;
        float _clipping[2];
        float _screenWindow[4];

        std::vector<FnKat::RenderOutputUtils::Transform> _transforms;
    };

    /**
     * @}
     */
}
}
}

namespace FnKat = Foundry::Katana;

#endif
