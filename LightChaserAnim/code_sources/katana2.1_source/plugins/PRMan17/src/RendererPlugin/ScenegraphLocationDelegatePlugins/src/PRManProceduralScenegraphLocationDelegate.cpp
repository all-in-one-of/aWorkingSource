// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <rx.h>
#include <iostream>
#include <PRManProceduralScenegraphLocationDelegate.h>
#include <ErrorReporting.h>
using namespace PRManProcedural;
#include <WriteRI_Util.h>
#include <sstream>
#include <pystring/pystring.h>
#include <FnRenderOutputUtils/FnRenderOutputUtils.h>

#include <OpenEXR/ImathMatrix.h>
#ifdef USE_SPINAMESPACED_EXR
#define SPI_GEOLIB_IMATH_NAMESPACE namespace Imath = SPI::OpenEXR::Imath;
#else
#define SPI_GEOLIB_IMATH_NAMESPACE
#endif

#include <WriteRI_Attributes.h>
#include <WriteRI_ObjectID.h>
#include <WriteRI_Shaders.h>
#include <WriteRI_Statements.h>
#include <cstring>

#include <FnAttribute/FnAttributeUtils.h>

#include <FnGeolibServices/FnXFormUtil.h>
namespace FnGeolibServices = FNGEOLIBSERVICES_NAMESPACE;

using namespace FnKat;

namespace
{

    // Callback for freeing procedural data.
    RtVoid FreeProceduralData( RtPointer data )
    {
        char** stringlist = (char**) data;
        free(stringlist[0]);
        free(stringlist[1]);
        free(stringlist);
    }


    void xform_point(double* result, const Imath::M44d& xform, double x, double y, double z)
    {
        const double * m = xform.getValue();

        result[0] = x * m[0]
                    + y * m[4]
                    + z * m[8]
                    + m[12];
        result[1] = x * m[1]
                    + y * m[5]
                    + z * m[9]
                    + m[13];
        result[2] = x * m[2]
                    + y * m[6]
                    + z * m[10]
                    + m[14];
    }

    void union_point(double* bounds, const double* p)
    {
        bounds[0] = std::min(p[0], bounds[0]);
        bounds[1] = std::max(p[0], bounds[1]);
        bounds[2] = std::min(p[1], bounds[2]);
        bounds[3] = std::max(p[1], bounds[3]);
        bounds[4] = std::min(p[2], bounds[4]);
        bounds[5] = std::max(p[2], bounds[5]);
    }

    namespace
    {
        Imath::M44d computeXform(const FnAttribute::GroupAttribute & xformAttr, bool * isGlobal)
        {
            std::pair<FnAttribute::DoubleAttribute, bool> result = 
                FnGeolibServices::FnXFormUtil::CalcTransformMatrixAtTime(
                    xformAttr, 0.f);
            *isGlobal = result.second;
            if (!result.first.isValid())
                return Imath::M44d();

            FnAttribute::DoubleAttribute::array_type v = result.first.getNearestSample(0.f);
            if (v.size() < 16)
                return Imath::M44d();

            return Imath::M44d((double(*)[4]) v.data());
        }
    }

    void processBounds(FnScenegraphIterator sgIterator, RtBound& adjustedBound)
    {
        FnScenegraphIterator it = sgIterator;

        // Get the bounds currently in effect... in the local coordinate system.
        FnAttribute::DoubleAttribute boundAttr = it.getAttribute("bound");
        Imath::M44d xform;

        while (it.isValid() && !boundAttr.isValid())
        {
            FnAttribute::GroupAttribute xformAttr = it.getAttribute("xform");
            if (xformAttr.isValid())
            {
                bool isAbs = false;
                Imath::M44d local = computeXform(xformAttr, &isAbs);
                xform = local * xform;  // FIXME: this ignores isAbs
            }
            it = it.getParent();
            boundAttr = it.getAttribute("bound");
        }

        if (boundAttr.isValid())
        {
            // Transform bound by xform.
            FnAttribute::DoubleConstVector oldBounds = boundAttr.getNearestSample(0);
            double newBounds[6];
            double p[3];
            xform_point(p, xform, oldBounds[0], oldBounds[2], oldBounds[4]);
            newBounds[0] = p[0];
            newBounds[1] = p[0];
            newBounds[2] = p[1];
            newBounds[3] = p[1];
            newBounds[4] = p[2];
            newBounds[5] = p[2];

            for (int i=0;i<8;++i)
            {
                xform_point(p, xform, oldBounds[0 + (i&1)], oldBounds[2 + (i & 2) / 2], oldBounds[4 + (i & 4) / 4]);
                union_point(newBounds, p);
            }

            for (int i = 0; i < 6; ++i)
            {
                adjustedBound[i] = static_cast<float>(newBounds[i]);
            }
        }

        if (adjustedBound[0] == adjustedBound[1] && adjustedBound[2] == adjustedBound[3] && adjustedBound[4] == adjustedBound[5])
        {
            Report_Warning("Empty bounding box detected (PRMan will not expand beyond this point).", sgIterator);
        }
    }

    void processCameraAsAttributes(FnScenegraphIterator sgIterator, const std::string& cameraInfoPath)
    {
        FnKat::RenderOutputUtils::CameraInfo cameraInfo = FnKat::RenderOutputUtils::getCameraInfo( sgIterator.getRoot(), cameraInfoPath );

        if ( cameraInfo.isValid() )
        {
            AttrList attrList;
            attrList.push_back(AttrListEntry("string cameraInfo_path",  "cameraInfo_path",   FnAttribute::StringAttribute(cameraInfoPath), true));
            attrList.push_back(AttrListEntry("float cameraInfo_fov",    "cameraInfo_fov",    FnAttribute::FloatAttribute(cameraInfo.getFov()), true));

            attrList.push_back(AttrListEntry("float cameraInfo_near",   "cameraInfo_near",   FnAttribute::FloatAttribute(cameraInfo.getNear()) ,true));
            attrList.push_back(AttrListEntry("float cameraInfo_far",    "cameraInfo_far",    FnAttribute::FloatAttribute(cameraInfo.getFar()) ,true));
            attrList.push_back(AttrListEntry("float cameraInfo_left",   "cameraInfo_left",   FnAttribute::FloatAttribute(cameraInfo.getLeft()) ,true));
            attrList.push_back(AttrListEntry("float cameraInfo_right",  "cameraInfo_right",  FnAttribute::FloatAttribute(cameraInfo.getRight()) , true));
            attrList.push_back(AttrListEntry("float cameraInfo_top",    "cameraInfo_top",    FnAttribute::FloatAttribute(cameraInfo.getTop()) , true));
            attrList.push_back(AttrListEntry("float cameraInfo_bottom", "cameraInfo_bottom", FnAttribute::FloatAttribute(cameraInfo.getBottom()) ,true));

            float xformValues[16];
            const double *xformFromCameraInfo = cameraInfo.getXForm();

            //TODO: review order of values
            for ( int i = 0; i < 16; i++)
            {
                xformValues[i] = (float)xformFromCameraInfo[i];

            }

            attrList.push_back(AttrListEntry("float[16] cameraInfo_xform", "cameraInfo_xform",  FnAttribute::FloatAttribute(xformValues, 16, 1), true));

            AttrList_Converter converter(attrList);
            RiAttributeV((char*)"user",
                    converter.getSize(0),
                    converter.getDeclarationTokens(0),
                    converter.getParameters(0));
        }
    }

    void processCameraAsParameters(FnScenegraphIterator sgIterator, const std::string& cameraInfoPath, ProceduralArgs& procArgs)
    {
        FnKat::RenderOutputUtils::CameraInfo cameraInfo = FnKat::RenderOutputUtils::getCameraInfo( sgIterator.getRoot(), cameraInfoPath );

        if ( cameraInfo.isValid() )
        {
            procArgs.addArgument("cameraInfo_path",   FnAttribute::StringAttribute(cameraInfoPath));
            procArgs.addArgument("cameraInfo_fov",    FnAttribute::FloatAttribute(cameraInfo.getFov()));
            procArgs.addArgument("cameraInfo_near",   FnAttribute::FloatAttribute(cameraInfo.getNear()));
            procArgs.addArgument("cameraInfo_far",    FnAttribute::FloatAttribute(cameraInfo.getFar()));
            procArgs.addArgument("cameraInfo_left",   FnAttribute::FloatAttribute(cameraInfo.getLeft()));
            procArgs.addArgument("cameraInfo_right",  FnAttribute::FloatAttribute(cameraInfo.getRight()));
            procArgs.addArgument("cameraInfo_top",    FnAttribute::FloatAttribute(cameraInfo.getTop()));
            procArgs.addArgument("cameraInfo_bottom", FnAttribute::FloatAttribute(cameraInfo.getBottom()));

            float xformValues[16];
            const double *xformFromCameraInfo = cameraInfo.getXForm();

            //TODO: review order of values
            for ( int i = 0; i < 16; i++)
            {
                xformValues[i] = (float)xformFromCameraInfo[i];

            }
            procArgs.addArgument("cameraInfo_xform", FnAttribute::FloatAttribute(xformValues, 16, 1));
        }
    }

    void processHairOptCamera(FnScenegraphIterator sgIterator, const std::string& cameraPath, const std::string& cameraInfoPath,
            ProceduralArgs& procArgs)
    {
        FnKat::RenderOutputUtils::CameraInfo cameraInfo = FnKat::RenderOutputUtils::getCameraInfo( sgIterator.getRoot(), cameraInfoPath );

        if ( ! cameraInfo.isValid() )
        {
            std::string error = "Procedural primitive specifies 'hair_opt_camera' with a camera path of ";
            error += cameraPath;
            error += " which cannot be found or is incomplete.";

            Report_Error(error, sgIterator);
        }
        else
        {
            procArgs.addArgument("hair_opt_camera_fov",    FnAttribute::FloatAttribute(cameraInfo.getFov()));
            procArgs.addArgument("hair_opt_camera_near",   FnAttribute::FloatAttribute(cameraInfo.getNear()));
            procArgs.addArgument("hair_opt_camera_far",    FnAttribute::FloatAttribute(cameraInfo.getFar()));
            procArgs.addArgument("hair_opt_camera_left",   FnAttribute::FloatAttribute(cameraInfo.getLeft()));
            procArgs.addArgument("hair_opt_camera_right",  FnAttribute::FloatAttribute(cameraInfo.getRight()));
            procArgs.addArgument("hair_opt_camera_top",    FnAttribute::FloatAttribute(cameraInfo.getTop()));
            procArgs.addArgument("hair_opt_camera_bottom", FnAttribute::FloatAttribute(cameraInfo.getBottom()));

            //create scoped coordinate system
            RiTransformBegin();

            //todo check order
            const double* xformValues = cameraInfo.getXForm();
            RtMatrix transform = {
                {(float)xformValues[0], (float)xformValues[1], (float)xformValues[2], (float)xformValues[3]},
                {(float)xformValues[4], (float)xformValues[5], (float)xformValues[6], (float)xformValues[7]},
                {(float)xformValues[8], (float)xformValues[9], (float)xformValues[10], (float)xformValues[11]},
                {(float)xformValues[12], (float)xformValues[13], (float)xformValues[14], (float)xformValues[15]}
            };

            RiTransform( transform );
            RiScopedCoordinateSystem(const_cast<char*>(cameraPath.c_str()));
            RiTransformEnd();
        }
    }
}

void* processProcedural(FnScenegraphIterator sgIterator, PRManPluginState* sharedState)
{
    if (!sgIterator.isValid()) return 0x0;
    if (!IsVisible(sgIterator)) return 0x0;

    std::string sgIteratorType = sgIterator.getType();

    std::string attrBaseName = "RiProcedural";
    std::string attrPathName = "path";

    if (sgIteratorType == "renderer procedural")
    {
        attrBaseName = "rendererProcedural";
        attrPathName = "procedural";
    }


    FnAttribute::GroupAttribute procGroupAttr = sgIterator.getAttribute(attrBaseName, true);
    if (!procGroupAttr.isValid())
    {
        Report_Error("Procedural primitive has no RiProcedural attribute.", sgIterator);
        return 0x0;
    }

    FnAttribute::StringAttribute procPath = procGroupAttr.getChildByName(attrPathName);
    if (!procPath.isValid())
    {
        // This is not an error; we just skip it.
        // I've got mail out to katana_users to decide on this for certain.
        Report_Error("Procedural primitive has no RiProcedural.path attribute.", sgIterator);
        return 0x0;
    }

    //TODO: use the asset management system to resolve the real path
    std::string dsoPath = procPath.getValue();

    //let's expand %ARCH ourselves to get better error messages on failures
    //dsoPath = GEOLIB::expandArchPath(dsoPath);

#ifndef RIBDUMP
    //now let's expand %RENDERER_VERSION for good measure too
    {
        RxInfoType_t attr_type;
        int attr_count;
        int versionCode[4];
        RxRendererInfo("version", &versionCode, 4 * sizeof(int), &attr_type, &attr_count);

        std::ostringstream buffer;
        buffer << versionCode[1] << "." << versionCode[3];

        dsoPath = pystring::replace(dsoPath, "%RENDERER_VERSION", buffer.str());
    }
#endif

    std::string cameraInfoPath;
    CameraInfoStyle cameraInfoStyle = ParseCameraInfoAttributes(sgIterator, cameraInfoPath, attrBaseName);


    // Include camera info as attributes
    if ( cameraInfoPath.size() && cameraInfoStyle == CAMERA_ATTRIBUTES)
    {
        processCameraAsAttributes(sgIterator, cameraInfoPath);
    }

    float frameNumber = GetFrameNumber(sgIterator, sharedState);


    FnAttribute::GroupAttribute argsAttr = procGroupAttr.getChildByName("args");

    std::string outputStyle = "classic";

    if (argsAttr.isValid())
    {
        FnAttribute::StringAttribute outputStyleAttr = argsAttr.getChildByName("__outputStyle");

        if ( outputStyleAttr.isValid() )
        {
            outputStyle = outputStyleAttr.getValue();
        }
    }

    // Set the bounding box to RI_INFINITY / 10.
    // If you set it to RI_INFINITY, it doesn't work.
    // I don't know why. :(
    RtBound adjustedBound = {-1e37f, 1e37f, -1e37f, 1e37f, -1e37f, 1e37f };
    FnAttribute::IntAttribute useInfiniteBoundsAttr = sgIterator.getAttribute(attrBaseName+".useInfiniteBounds");

    if (useInfiniteBoundsAttr.isValid() && useInfiniteBoundsAttr.getValue() == 0)
    {
        processBounds(sgIterator, adjustedBound);
    }

    ProceduralArgs procArgs;
    std::ostringstream os;
    os << std::setprecision(12);

    if ( outputStyle == "classic" || outputStyle == "typedArguments")
    {
        if (argsAttr.isValid())
        {
            // When arguments are typed, dsoname and bound are just arguments.
            if ( outputStyle == "typedArguments" )
            {
                procArgs.addArgument("dsoname", FnAttribute::StringAttribute(dsoPath));
                procArgs.addArgument("bound", FnAttribute::FloatAttribute(adjustedBound, 6, 6));
            }

            for (int i = 0; i < argsAttr.getNumberOfChildren(); i++)
            {
                std::string childName = argsAttr.getChildName(i);
                FnAttribute::StringAttribute stringAttr = argsAttr.getChildByIndex(i);

                if ( outputStyle == "classic" && stringAttr.isValid() &&
                        pystring::startswith( childName, "extraArgs" ) )
                {
                    // Handle special case of "extraArgs" as literal. Trust that the users
                    // know what they want here. This is not need for the typed output style.
                    FnAttribute::StringConstVector v = stringAttr.getNearestSample(0);
                    if (v.size() == 0) continue;

                    //TODO: use the asset management system to decode the true path here:
                    std::string value = v[0];
                    os << " " << value << " ";
                }
                else
                {
                    procArgs.addArgument(childName, argsAttr.getChildByIndex(i));
                }
            }

            // Special case for hair dso included here (Sony legacy)
            FnAttribute::StringAttribute cameraPathAttr = argsAttr.getChildByName("hair_opt_camera");
            if ( cameraPathAttr.isValid() )
            {
                std::string cameraPath = cameraPathAttr.getValue();
                processHairOptCamera(sgIterator, cameraPath, cameraInfoPath, procArgs);
            }
        }

        // Include camera info as parameters
        if ( cameraInfoPath.size() && cameraInfoStyle == CAMERA_PARAMETERS )
        {
            processCameraAsParameters(sgIterator, cameraInfoPath, procArgs);
        }

        // Add additional options: frame, shutter and cropwindow.
        procArgs.addArgument("frame", FnAttribute::FloatAttribute(frameNumber));
        float shutterTimes[2] = { sharedState->proceduralSettings.shutterOpenTime, sharedState->proceduralSettings.shutterCloseTime };
        procArgs.addArgument("shutter", FnAttribute::FloatAttribute(shutterTimes, 2, 2));
        procArgs.addArgument("cropwindow", FnAttribute::FloatAttribute(sharedState->proceduralSettings.cropWindow, 4, 4));

        // Append string to output stream
        if ( outputStyle == "classic" )
        {
            os << procArgs.getProcString();
        }
    }
    else if ( outputStyle == "scenegraphAttr" )
    {
        FnKat::RenderOutputUtils::ProceduralOutputContextInfo contextInfo;
        contextInfo._frameNumber = (int)frameNumber;
        contextInfo._shutterOpen = sharedState->proceduralSettings.shutterOpenTime;
        contextInfo._shutterClose = sharedState->proceduralSettings.shutterCloseTime;

        contextInfo._cropWindowXMin = sharedState->proceduralSettings.cropWindow[0];
        contextInfo._cropWindowXMax = sharedState->proceduralSettings.cropWindow[1];
        contextInfo._cropWindowYMin = sharedState->proceduralSettings.cropWindow[2];
        contextInfo._cropWindowYMax = sharedState->proceduralSettings.cropWindow[3];

        contextInfo._xres = sharedState->proceduralSettings.xres;
        contextInfo._yres = sharedState->proceduralSettings.yres;

        std::string args = FnKat::RenderOutputUtils::buildProceduralArgsString(
                sgIterator,
                FnKat::RenderOutputUtils::kProceduralArgsType_ScenegraphAttr,                
                attrBaseName,
                contextInfo);

        os << args;
    }
    else
    {
        Report_Error("unknown output style for RiProcedural  '" + outputStyle + "'.", sgIterator);
        return 0x0;
    }


    if ( outputStyle == "typedArguments" )
    {
        AttrList_Converter converter(procArgs.getAttrList());
        RiProcedural2V(RiProc2DynamicLoad, RiSimpleBound, converter.getSize(0),
                converter.getDeclarationTokens(0), converter.getParameters(0));
    }
    else
    {
        char** procdata = reinterpret_cast<char**>(malloc(sizeof(char*) * 2));
        procdata[0] = strdup(dsoPath.c_str());
        procdata[1] = strdup(os.str().c_str());
        RiProcedural(procdata, adjustedBound, RiProcDynamicLoad, FreeProceduralData);
    }

    return 0x0;
}

PRManProceduralScenegraphLocationDelegate::~PRManProceduralScenegraphLocationDelegate()
{

}

void PRManProceduralScenegraphLocationDelegate::flush()
{

}

void* PRManProceduralScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator, void* optionalInput)
{
    PRManSceneGraphLocationDelegateInput* state = reinterpret_cast<PRManSceneGraphLocationDelegateInput*>(optionalInput);

    WriteRI_Object_LevelOfDetail(sgIterator, state->sharedState);
    RiAttributeBegin();
    WriteRI_Object_Name(sgIterator, state->sharedState);
    WriteRI_Object_Id(sgIterator, state->sharedState);
    FnAttribute::GroupAttribute xformAttr = sgIterator.getAttribute("xform");
    WriteRI_Object_Transform(xformAttr, sgIterator, state->sharedState);
    WriteRI_Object_RelativeScopedCoordinateSystems(sgIterator, state->sharedState);
    WriteRI_Object_Statements(sgIterator, state->sharedState);
    WriteRI_Object_Material(sgIterator, state->sharedState);
    WriteRI_Object_IlluminationList(sgIterator, state->sharedState);

    void* returnValue = processProcedural(sgIterator, state->sharedState);

    RiAttributeEnd();
    return returnValue;
}

PRManProceduralScenegraphLocationDelegate* PRManProceduralScenegraphLocationDelegate::create()
{
    return new PRManProceduralScenegraphLocationDelegate();
}

std::string PRManProceduralScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("prman");
}

void PRManProceduralScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("RendermanProcedural"));
    supportedLocationList.push_back(std::string("renderman procedural"));
    supportedLocationList.push_back(std::string("renderer procedural"));
}


template <typename T>
void ProceduralArgs::addArgument(const std::string& argName, const T attr, const bool mightBlur)
{
    std::string declaration = PRManProcedural::BuildDeclarationFromAttribute(argName, attr);
    if (!declaration.empty())
    {
        m_attrList.push_back(AttrListEntry(declaration, argName, attr, mightBlur));
    }
}

template <typename T>
std::string ProceduralArgs::buildNumericValueString(const T attr) const
{
    T typedAttr = attr;
    std::ostringstream os;
    const unsigned int numValues = typedAttr.getNearestSample(0).size();
    for (unsigned int i = 0; i < numValues; i++)
    {
        //TODO: very inefficient... we need something like a base class for the ConstVectors
        os << typedAttr.getNearestSample(0)[i] << " ";
    }
    return os.str();
}

std::string ProceduralArgs::getProcString() const
{
    std::ostringstream os;

    for (AttrList::const_iterator it = m_attrList.begin(); it != m_attrList.end(); ++it)
    {
        os << " -" << it->_name << " ";

        DataAttribute attr = it->_value;
        const FnKatAttributeType type = attr.getType();
        if (type == kFnKatAttributeTypeString)
        {
            FnAttribute::StringAttribute strAttr = it->_value;
            FnAttribute::StringConstVector v = strAttr.getNearestSample(0);
            if (v.size() == 0) continue;

            if (v.size() == 1)
            {
                std::string value = v[0];

                if (value.empty() || value.find(' ') != std::string::npos)
                {
                    os << "'" << value << "'";
                }
                else
                {
                    os << value;
                }
            }
            else
            {
                int vSize = v.size();
                os << "'";
                for (int i = 0; i < vSize; i++)
                {
                    if(i > 0)
                        os << " ";

                    //TODO: use Asset Management System to resolve the assetId
                    std::string value = v[i];
                    os << value;
                }
                os << "'";
            }
        }
        else
        {
            unsigned int numValues = ((DataAttribute)attr).getNumberOfValues();
            if (numValues == 0) continue;

            if (type == kFnKatAttributeTypeFloat)
            {
                os << buildNumericValueString<FloatAttribute>(attr);
            }
            else if (type == kFnKatAttributeTypeDouble)
            {
                os << buildNumericValueString<DoubleAttribute>(attr);
            }
            else if (type == kFnKatAttributeTypeInt)
            {
                os << buildNumericValueString<IntAttribute>(attr);
            }
        }
    }

    return os.str();
}

const AttrList& ProceduralArgs::getAttrList() const
{
    return m_attrList;
}
