// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDEROUTPUTUTILS_CAMERAINFO_H
#define FNRENDEROUTPUTUTILS_CAMERAINFO_H

// Suite:
#include "FnRenderOutputUtils/suite/FnRenderOutputUtilsSuite.h"
#include "FnRenderOutputUtils/FnRenderOutputUtilsAPI.h"

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
 * @brief CameraInfo
 */
class FNRENDEROUTPUTUTILS_API CameraInfo
{
public:

    CameraInfo(CameraInfoHandle handle);
    ~CameraInfo();

    CameraInfo(const CameraInfo& rhs);
    CameraInfo& operator=(const CameraInfo& rhs);

    /**
     * isValid
     */
    bool isValid() const {return _handle != 0x0;}

    /**
     * getFov
     */
    float getFov() const;

    /**
     * getNear
     */
    float getNear() const;

    /**
     * getFar
     */
    float getFar() const;

    /**
     * getLeft
     */
    float getLeft() const;

    /**
     * getRight
     */
    float getRight() const;

    /**
     * getTop
     */
    float getTop() const;

    /**
     * getBottom
     */
    float  getBottom() const;

    /**
     * getOrtho
     */
    int getOrtho() const;

    /**
     * getOrthoWidth
     */
    float getOrthoWidth() const;

    /**
     * getXForm
     */
    double* getXForm() const;

protected:
    void acceptHandle(const CameraInfo &rhs);

private:
    CameraInfoHandle _handle;

};
/**
 * @}
 */

}
}
}

namespace FnKat = Foundry::Katana;

#endif //RenderOutputUtilsCameraInfo_H
