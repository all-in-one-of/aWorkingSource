// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include "arnold_render.h"

#include <stdlib.h>
#include <algorithm>

#include <pystring/pystring.h>

#include <buildCamera.h>
#include <buildCommons.h>
#include <buildMaterial.h>

using namespace FnKat;


void setCameraDefault(AtNode* node)
{
    setArnoldParam_Float(GroupAttribute(), "shutter_start", node);
    setArnoldParam_Float(GroupAttribute(), "shutter_end", node);
    setArnoldParam_Int(GroupAttribute(), "shutter_type", node);
    setArnoldParam_String(GroupAttribute(), "rolling_shutter", node);
    setArnoldParam_Bool(GroupAttribute(), "plane_distance", node);
    setArnoldParam_Float(GroupAttribute(), "focus_distance", node);
    setArnoldParam_Float(GroupAttribute(), "aperture_size", node);
}

AtNode *getCameraLocationByPath(const std::string &path, ArnoldPluginState* sharedState)
{
    AtNode* cameraNode = sharedState->getCameraLocationByPath(path);
    if (cameraNode)
    {
        return cameraNode;
    }
    AiMsgError("[kat] FATAL: Unable to find cached camera '%s'.", path.c_str());
    exit(-1);
}

AtNode *buildCameraLocationByPath(const std::string &path, FnScenegraphIterator rootIterator,
    int *overscan, ArnoldPluginState* sharedState, int *displayWindowSize)
{
    AtNode* cameraNode = sharedState->getCameraLocationByPath(path);
    if (cameraNode)
    {
        return cameraNode;
    }

    FnScenegraphIterator cameraIterator = rootIterator.getByPath(path);
    if (!cameraIterator.isValid())
    {
        AiMsgError("[kat] FATAL: Unable to find the camera '%s'.", path.c_str());
        exit(-1);
    }

    cameraNode = buildCameraLocation(cameraIterator, sharedState, overscan, displayWindowSize);
    if(!cameraNode)
    {
        AiMsgError("[kat] FATAL: during build of '%s'.", path.c_str());
        exit(-1);
    }

    return cameraNode;
}

AtNode* buildCameraLocation(FnScenegraphIterator cameraIterator, ArnoldPluginState* sharedState, int *overscan, int *displayWindowSize)
{
    if (!cameraIterator.isValid()) return NULL;

    FnScenegraphIterator rootIterator = cameraIterator.getRoot();
    if (!rootIterator.isValid()) return NULL;

    std::string cameraName = cameraIterator.getFullName();

    AtNode* existingCamera = AiNodeLookUpByName(cameraName.c_str());
    if (existingCamera) return existingCamera;

    GroupAttribute cameraGeometryAttr = cameraIterator.getAttribute("geometry");
    if (!cameraGeometryAttr.isValid())
    {
        AiMsgError("[kat] FATAL: Geometry attribute no found on camera '%s'.", cameraName.c_str());
        return NULL;
    }

    // set camera type
    std::string camera_type = "persp_camera";
    FnAttribute::StringAttribute cameraTypeAttr = cameraGeometryAttr.getChildByName("projection");
    if( cameraTypeAttr.isValid() )
    {
        if( cameraTypeAttr.getValue() == "orthographic" )
            camera_type = "ortho_camera";
    }

    GroupAttribute gAttr = rootIterator.getAttribute("arnoldGlobalStatements");
    AtNode* filtermapNode = NULL;
    FnAttribute::IntAttribute planeDistanceAttr;
    FnAttribute::FloatAttribute dofAttr;
    FnAttribute::FloatAttribute apertureAttr;
    
    bool foundTypeAttr = false;
    
    if (gAttr.isValid())
    {
        FnAttribute::IntAttribute typeAttr = gAttr.getChildByName("camera_type");
        if (typeAttr.isValid())
        {
            foundTypeAttr = true;
            
            int type = typeAttr.getValue();
            switch (type)
            {
                case 0:
                    camera_type = "cyl_camera";
                    break;
                case 1:
                    camera_type = "fisheye_camera";
                    break;
                case 2:
                    camera_type = "ortho_camera";
                    break;
                case 3:
                {
                    FnAttribute::StringAttribute attr = cameraGeometryAttr.getChildByName("projection");
                    if (attr.isValid())
                    {
                        if (attr.getValue() == "orthographic")
                        {
                            camera_type = "ortho_camera";
                            break;
                        }
                    }
                    // Defaults to perspective camera
                    camera_type = "persp_camera";
                    break;

                }

                case 5:
                    camera_type = "spherical_camera";
                    break;
                    
                default:
                    camera_type = "persp_camera";
                    break;
            }
        }
        
        //filtermap set?
        FnAttribute::StringAttribute attrRef = gAttr.getChildByName("filtermap");
        if (attrRef.isValid())
        {
            std::string path = attrRef.getValue();
            filtermapNode = getShaderByPath(path, cameraIterator, NULL, true, sharedState, "arnoldSurface");
        }
    
        planeDistanceAttr = gAttr.getChildByName("plane_distance");
        dofAttr = gAttr.getChildByName("focus_distance");
        apertureAttr = gAttr.getChildByName("aperture_size");
    }
    
    
    //fallback case is "from camera location"
    if (!foundTypeAttr)
    {
        FnAttribute::StringAttribute attr = cameraGeometryAttr.getChildByName("projection");
        if (attr.isValid())
        {
            if (attr.getValue() == "orthographic")
            {
                camera_type = "ortho_camera";
            }
        }
    }
    
    
    //allow geometry.filtermap to override arnoldGlobalStatements.filtermap
    {
        FnAttribute::StringAttribute attrRef = cameraGeometryAttr.getChildByName("filtermap");
        if (attrRef.isValid()) 
        {
            std::string path = attrRef.getValue();
            filtermapNode = getShaderByPath(path, cameraIterator, NULL, true, sharedState, "arnoldSurface");
        }
    }

    //create cam node
    AtNode* cameraNode = AiNode(camera_type.c_str());
    if (!cameraNode)
    {
        AiMsgError("[kat] FATAL: Unable to create camera %s", cameraName.c_str());
        return NULL;
    }
    AiNodeSetStr(cameraNode, "name", cameraName.c_str());

    //hookup filtermap if we have one
    if (filtermapNode)
    {
        AiNodeSetPtr(cameraNode, "filtermap", filtermapNode);
    }
    
    //clipping plane correction?
    if (planeDistanceAttr.isValid())
    {
        AiNodeSetBool(cameraNode, "plane_distance", planeDistanceAttr.getValue());
    }
   
    //dof
    if (dofAttr.isValid() && apertureAttr.isValid())
    {
        //TODO: MB
        float dof = dofAttr.getValue();
        float aw = apertureAttr.getValue();
        if (dof != 0.0f && aw != 0.0f) 
        {
            AiNodeSetArray(cameraNode, "focus_distance", AiArrayConvert(1, 1, AI_TYPE_FLOAT, &dof));
            AiNodeSetArray(cameraNode, "aperture_size", AiArrayConvert(1, 1, AI_TYPE_FLOAT, &aw));
            
            FnAttribute::IntAttribute intAttr;
            FnAttribute::FloatAttribute floatAttr;

            intAttr = gAttr.getChildByName("aperture_nsides");
            if (intAttr.isValid())
            {
                AiNodeSetInt(cameraNode, "aperture_nsides", intAttr.getValue());
            }
            floatAttr = gAttr.getChildByName("aperture_ratio");
            if (floatAttr.isValid())
            {
                AiNodeSetFlt(cameraNode, "aperture_ratio", floatAttr.getValue());
            }
            floatAttr = gAttr.getChildByName("aperture_relax");
            if (floatAttr.isValid())
            {
                AiNodeSetFlt(cameraNode, "aperture_relax", floatAttr.getValue());
            }
            floatAttr = gAttr.getChildByName("aperture_rotation");
            if (floatAttr.isValid())
            {
                AiNodeSetFlt(cameraNode, "aperture_rotation", floatAttr.getValue());
            }
        }
    }
    
    //screen window
    //TODO: Use FnRender::CameraSettings to obtain screen window co-ordinates
    //      wherever they are required, such as here.
    //      (CameraSettings::getScreenWindow() is based on much of this code)
    {
        FnAttribute::DoubleAttribute leftAttr = cameraGeometryAttr.getChildByName("left");
        FnAttribute::DoubleAttribute rightAttr = cameraGeometryAttr.getChildByName("right");
        FnAttribute::DoubleAttribute bottomAttr = cameraGeometryAttr.getChildByName("bottom");
        FnAttribute::DoubleAttribute topAttr = cameraGeometryAttr.getChildByName("top");
        if (!leftAttr.isValid() || !rightAttr.isValid() ||
            !bottomAttr.isValid() || !topAttr.isValid())
        {
            AiMsgError("[kat] FATAL: Missing a required attribute in the geometry group on camera '%s'.",
                cameraName.c_str());
            return NULL;
        }

        double left = leftAttr.getValue();
        double right = rightAttr.getValue();
        double bottom = bottomAttr.getValue();
        double top = topAttr.getValue();

        // Orthographic camera calculations should be done before overscan is
        // applied
        if (camera_type == "ortho_camera")
        {
            FnAttribute::DoubleAttribute orthoWidthAttr =
                cameraGeometryAttr.getChildByName("orthographicWidth");

            if (orthoWidthAttr.isValid() && orthoWidthAttr.getValue() > 0)
            {
                double w = (right-left);
                double h = (top-bottom);

                double orthoWidth = orthoWidthAttr.getValue();
                double orthoHeight = orthoWidth  * h / w;

                left *= 1.0/w * orthoWidth;
                right *= 1.0/w * orthoWidth;

                top *= 1.0/h * orthoHeight;
                bottom *= 1.0/h * orthoHeight;
            }
        }

        // adjust screen window for overscan
        if (overscan != 0 && displayWindowSize != 0)
        {
            if (displayWindowSize[0] == 0 || displayWindowSize[1] == 0)
            {
                AiMsgWarning("[kat] Ignoring overscan in screen window calcuation, since display window size is 0.");
            }

            int displayWindow[4] = {0, 0, displayWindowSize[0], displayWindowSize[1]};

            double displayWindow_w2 = (displayWindow[2] - displayWindow[0]) / 2.0;
            double displayWindow_h2 = (displayWindow[3] - displayWindow[1]) / 2.0;
            double overscanMult[4] = {(displayWindow_w2+overscan[0])/displayWindow_w2,
                                      (displayWindow_h2+overscan[1])/displayWindow_h2,
                                      (displayWindow_w2+overscan[2])/displayWindow_w2,
                                      (displayWindow_h2+overscan[3])/displayWindow_h2};
            double screen_window_cx = (left + right) / 2.0;
            double screen_window_cy = (bottom + top) / 2.0;

            left = screen_window_cx + (left-screen_window_cx)*overscanMult[0];
            bottom = screen_window_cy + (bottom-screen_window_cy)*overscanMult[1];
            right = screen_window_cx + (right-screen_window_cx)*overscanMult[2];
            top = screen_window_cy + (top-screen_window_cy)*overscanMult[3];
        }

        // In order to work correctly with a non-square pixel aspect ratio we
        // need to set the "aspect_ratio" option (currently done in
        // ArnoldRender::InitArnold()) and then adjust the height of the screen
        // window to make it square.
        float screenWidth = right - left;
        float screenHeight = top - bottom;
        float heightMultiplier = 1.0;
        if (screenHeight > 0.0) heightMultiplier = screenWidth / screenHeight;

        AiNodeSetPnt2(cameraNode, "screen_window_min", left,
                      bottom * heightMultiplier);
        AiNodeSetPnt2(cameraNode, "screen_window_max", right,
                      top * heightMultiplier);
    }

    //near
    {
        FnAttribute::DoubleAttribute nearAttr = cameraGeometryAttr.getChildByName("near");
        if (!nearAttr.isValid())
        {
            AiMsgError("[kat] FATAL: 'geometry.near' not found on camera '%s'.", cameraName.c_str());
            return NULL;
        }
        AiNodeSetFlt(cameraNode, "near_clip", nearAttr.getValue());
    }

    //far
    {
        FnAttribute::DoubleAttribute farAttr = cameraGeometryAttr.getChildByName("far");
        if (!farAttr.isValid())
        {
            AiMsgError("[kat] FATAL: 'geometry.far' not found on camera '%s'.", cameraName.c_str());
            return NULL;
        }
        AiNodeSetFlt(cameraNode, "far_clip", farAttr.getValue());
    }
    
    //shutter
    {
        
        /*FloatAttribute shutterStartAttr = rootIterator.getAttribute("arnoldGlobalStatements.shutter_start");
        if (shutterStartAttr.isValid())
        {
            AiNodeSetFlt(cameraNode, "shutter_start", shutterStartAttr.getValue());
            SetGlobalShutterOpen(shutterStartAttr.getValue());
        }
        
        FnAttribute::FloatAttribute shutterEndAttr = rootIterator.getAttribute("arnoldGlobalStatements.shutter_end");
        if (shutterEndAttr.isValid())
        {
            AiNodeSetFlt(cameraNode, "shutter_end", shutterEndAttr.getValue());
            SetGlobalShutterClose(shutterEndAttr.getValue());
        }*/
        
        //TRANSITIONAL: override with possible global shutter value
        FnAttribute::FloatAttribute shutterOpenAttr = rootIterator.getAttribute("renderSettings.shutterOpen");
        if (shutterOpenAttr.isValid())
        {
            AiNodeSetFlt(cameraNode, "shutter_start", shutterOpenAttr.getValue());
            sharedState->setShutterOpen(shutterOpenAttr.getValue());
        }
        FnAttribute::FloatAttribute shutterCloseAttr = rootIterator.getAttribute("renderSettings.shutterClose");
        if (shutterCloseAttr.isValid())
        {
            AiNodeSetFlt(cameraNode, "shutter_end", shutterCloseAttr.getValue());
            sharedState->setShutterClose(shutterCloseAttr.getValue());
        }
        
        FnAttribute::IntAttribute shutterTypeAttr = rootIterator.getAttribute("arnoldGlobalStatements.shutter_type");
        if (shutterTypeAttr.isValid())
        {
            AiNodeSetInt(cameraNode, "shutter_type", shutterTypeAttr.getValue());
        }
        
        if (AiNodeEntryLookUpParameter(AiNodeGetNodeEntry(cameraNode), "shutter_curve"))
        {
            //multi-attr form because we don't currently support dynamic arrays at the ArnoldGlobalSettings node level
            FnAttribute::IntAttribute shutterCurveIntAttr =
                    rootIterator.getAttribute("arnoldGlobalStatements.shutter_curve");
            FnAttribute::FloatAttribute shutterCurveFltAttr =
                    rootIterator.getAttribute("arnoldGlobalStatements.shutter_curve");

            if (shutterCurveIntAttr.isValid())
            {
                int numKnots = shutterCurveIntAttr.getValue(0, false);
                if (numKnots > 1)
                {
                    FnAttribute::FloatAttribute shutterCurveKnots = rootIterator.getAttribute(
                                "arnoldGlobalStatements.shutter_curve_Knots");

                    FnAttribute::FloatAttribute shutterCurveFloats = rootIterator.getAttribute(
                                "arnoldGlobalStatements.shutter_curve_Floats");

                    if (shutterCurveKnots.isValid() && shutterCurveFloats.isValid()
                                && shutterCurveKnots.getNumberOfValues() >= numKnots
                                && shutterCurveFloats.getNumberOfValues() >= numKnots)
                    {
                        FnAttribute::FloatAttribute::array_type knotsValues =
                                shutterCurveKnots.getNearestSample(0.0f);
                        FnAttribute::FloatAttribute::array_type floatsValues =
                                shutterCurveFloats.getNearestSample(0.0f);
                        
                        std::vector<float> values;
                        values.reserve(numKnots*2);
                        for (int i = 0; i < numKnots; ++i)
                        {
                            values.push_back(knotsValues[i]);
                            values.push_back(floatsValues[i]);
                        }

                        AiNodeSetArray(cameraNode, "shutter_curve",
                                AiArrayConvert(numKnots, 1, AI_TYPE_POINT2,
                                        (void*)(&values[0])));
                    }
                }
            }
            //single array form for future expansion
            else if (shutterCurveFltAttr.isValid())
            {
                int numValues = shutterCurveFltAttr.getNumberOfValues();
                if (numValues >= 4 && numValues % 2 == 0)
                {
                    FnAttribute::FloatAttribute::array_type values =
                            shutterCurveFltAttr.getNearestSample(0.0f);

                    AiNodeSetArray(cameraNode, "shutter_curve",
                                AiArrayConvert(numValues / 2, 1, AI_TYPE_POINT2,
                                        (void*)(&values[0])));
                }
            }

        }

        StringAttribute rollingShutterAttr = rootIterator.getAttribute("arnoldGlobalStatements.rolling_shutter");
        if (rollingShutterAttr.isValid())
        {
            AiNodeSetStr(cameraNode, "rolling_shutter", rollingShutterAttr.getValue("off").c_str());
        }
    }

    {
        std::map<float, Imath::M44d> xformSamples;
        applyTransform(cameraNode, cameraIterator,sharedState, &xformSamples);
        
        //fov (output at same (interpolated) time samples as xform)
        {
            FnAttribute::DoubleAttribute fovAttr = cameraGeometryAttr.getChildByName("fov");
            if (!fovAttr.isValid())
            {
                AiMsgError("[kat] FATAL: Geometry attribute missing the fov attribute on camera '%s'.",
                    cameraName.c_str());
                return NULL;
            }
            
            std::map<float, Imath::M44d>::const_iterator xformSampleIter;
            std::vector<float> fovValues;
            
            for (xformSampleIter=xformSamples.begin(); xformSampleIter != xformSamples.end(); ++xformSampleIter)
            {
                double fov = 0.0;
                fovAttr.fillInterpSample(&fov, 1, (*xformSampleIter).first, false, 0.0f);
                fovValues.push_back((float)fov);
            }

            AiNodeSetArray(cameraNode, "fov", AiArrayConvert(1, fovValues.size(), AI_TYPE_FLOAT, &fovValues[0]));
        }
    }

    sharedState->setCameraLocation(cameraName, cameraNode);

    AiMsgInfo("[kat] made camera %s", cameraName.c_str());

    return cameraNode;
}

std::string getDefaultRenderCameraName(FnScenegraphIterator rootIterator)
{
    FnAttribute::StringAttribute cameraNameAttr = rootIterator.getAttribute("renderSettings.cameraName");
    return cameraNameAttr.getValue("/root/world/cam/camera", false);
}

void buildRenderCamera(FnScenegraphIterator rootIterator, ArnoldPluginState* sharedState, int *overscan, int *displayWindowSize)
{
    std::string cameraName = getDefaultRenderCameraName(rootIterator);

    AtNode* cameraNode = buildCameraLocationByPath(cameraName, rootIterator, overscan, sharedState, displayWindowSize);
    if(!cameraNode)
    {
        AiMsgError("[kat] FATAL: during build of '%s'.", cameraName.c_str());
        exit(-1);
    }

    // Point the option block at this camera.
    AiNodeSetPtr(AiUniverseGetOptions(), "camera", cameraNode);
    AiMsgInfo("[kat] render camera created.");
}

void buildCameras(FnScenegraphIterator worldIterator, ArnoldPluginState* sharedState)
{
    FnAttribute::StringAttribute cameraListAttr = worldIterator.getAttribute("globals.cameraList");
    if (!cameraListAttr.isValid())
    {
        AiMsgInfo("[kat] No globals.cameraList found at /root/world.  Only the render camera will be preloaded.");
        return;
    }

    StringConstVector cameraPaths = cameraListAttr.getNearestSample(0.0f);
    for (StringConstVector::const_iterator iter = cameraPaths.begin();
         iter != cameraPaths.end(); ++iter)
    {
        std::string cameraPath = (*iter);
        FnScenegraphIterator cameraIterator;
        if (pystring::startswith(cameraPath, "/root/world/"))
        {
            cameraPath = cameraPath.substr(5);
            cameraIterator = worldIterator.getByPath(cameraPath);
        }
        else
        {
            cameraIterator = worldIterator.getRoot().getByPath(cameraPath);
        }
        if (!cameraIterator.isValid())
        {
            AiMsgInfo("[kat] The camera '%s' could not be found in the scene.", cameraPath.c_str());
            continue;
        }

        buildCameraLocation(cameraIterator, sharedState);
    }
}
