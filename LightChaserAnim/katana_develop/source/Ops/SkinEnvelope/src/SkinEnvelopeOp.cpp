// Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <vector>
#include <iostream>
#include <string>
#include <utility>
#include <set>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathMatrix.h>
#include <pystring/pystring.h>
#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnAttributeUtils.h>
#include <FnGeolib/op/FnGeolibOp.h>


namespace // anonymous
{

using FnAttribute::StringAttribute;
using FnAttribute::DataAttribute;
using FnAttribute::DoubleAttribute;
using FnAttribute::FloatAttribute;
using FnAttribute::GroupAttribute;


/* In the example scene RestposeJointsPath values
 *  is "/root/reference/restposeJoints/"
*/
static const std::string RESTPOSE_OP_ARG_NAME = "RestposeJointsPath";
static const std::string SKIN_WEIGHT_PREFIX = "skinWeight__";
static const std::string JOINT_XFORM = "jointXform";
static const std::string JOINT_INVERSE_XFORM = "jointInvXform";
static const std::string CHARACTER_ROOT = "character.root";
static const std::string GEOMETRY_ARBITRARY = "geometry.arbitrary";
static const std::string GEOMETRY_DELETE_CHILD_PREFIX =
        "geometry.arbitrary.";
static const std::string POINT_VALUE = "geometry.point.P";
static const std::string VALUE_SUFFIX = ".value";

void AddTimeSamplesToSet(const FnAttribute::Attribute& attr,
        std::set<float>& set )
{
    if (attr.getType() == FnAttribute::GroupAttribute::getKatAttributeType())
    {
        const FnAttribute::GroupAttribute& groupAttr =
                static_cast<const FnAttribute::GroupAttribute&>(attr);

        for (size_t i = 0; i < groupAttr.getNumberOfChildren(); ++i)
        {
            AddTimeSamplesToSet(groupAttr.getChildByIndex(i), set);
        }
    }
    else
    {
        const FnAttribute::DataAttribute& dataAttr =
                static_cast<const FnAttribute::DataAttribute&>(attr);

        int64_t count = dataAttr.getNumberOfTimeSamples();
        for(int64_t i = 0; i < count; ++i)
        {
            set.insert(dataAttr.getSampleTime(i));
        }
    }
}

Imath::M44d Matrix44FromAttr(const FnAttribute::DoubleAttribute& attr,
                             float time)
{
    if (attr.isValid())
    {
        FnAttribute::DoubleAttribute::array_type values =
                attr.getNearestSample(time);

        if(attr.getNumberOfValues() == 16)
        {
            const double * ptr = values.data();
            return Imath::M44d(reinterpret_cast<const double(*)[4]>(ptr));
        }

    }
    return Imath::M44d();
}

class SkinEnvelopeOp : public Foundry::Katana::GeolibOp
{

private:

    typedef std::vector<float> TimeSamples;

    /*
     * Auxiliary data structure to hold attributes and paths
     * useful for the final skin loop.
     *
     * */
    struct TransformData
    {
        DoubleAttribute animJointXformAttr;
        DoubleAttribute restposeJointInvXformAttr;
        std::string childName;

        TransformData(const DoubleAttribute& animJoint,
                      const DoubleAttribute& restposeJointInv,
                      const std::string& cName)
            : animJointXformAttr (animJoint)
            , restposeJointInvXformAttr (restposeJointInv)
            , childName (cName)
        {}
    };


    /*
     * Finds all skin weights Xform attributes under a given root attribute.
     * Returns true if at least one skin attribute is found.
     * Fill a vector of times value we need to sample. This is done by
     * accumulating for all the animated joint transforms
     * for joint that affect the skin.

     * @param getArbAttribute Arbitrary attribute inside which skin weights are
     * supposed to be stored
     *
     * @param charRootPath Base path used to find animated and restpose joints
     * @param xforms Filled with all xforms data useful to skin the mesh.
     * @param timeSamples Filled with all available time sample intervals
     * in the current frame
     * */
    static bool FindSkinWeights(
            Foundry::Katana::GeolibCookInterface &interface,
            const GroupAttribute& geoArbAttr,
            const std::string& charRootPath,
            std::vector<TransformData>& xforms,
            std::set<float>& timeSamples)
    {

        bool skinDeformationFlag = false;

        StringAttribute restposePathPrefixArg =
            interface.getOpArg(RESTPOSE_OP_ARG_NAME);

        if (!restposePathPrefixArg.isValid())
        {
            Foundry::Katana::ReportError(interface,
                "\"RestposeJointsPath\" argument is required.");
            return false;
        }
        const std::string restposeJointsPathPrefix =
            restposePathPrefixArg.getValue();

        interface.prefetch(restposeJointsPathPrefix, interface.getInputIndex());


        const size_t arbitraryAttributesNum = geoArbAttr.getNumberOfChildren();
        for (size_t i = 0; i < arbitraryAttributesNum; ++i)
        {
            const std::string childName = geoArbAttr.getChildName(i);

            if (pystring::startswith(childName, SKIN_WEIGHT_PREFIX))
            {
                skinDeformationFlag = true;

                //--harcoded path in the skin weight
                std::string jointPath = childName.substr(12);
                jointPath = pystring::replace(jointPath,"__","/");

                // get the Xform attribute for the animated joint
                std::string animJointPath = charRootPath;
                animJointPath += "/" + jointPath;

                DoubleAttribute animJointXformAttr =
                    interface.getAttr(JOINT_XFORM, animJointPath);

                std::string restposeJointPath =
                    restposeJointsPathPrefix  + jointPath;

                DoubleAttribute restposeJointInvXformAttr =
                    interface.getAttr(JOINT_INVERSE_XFORM, restposeJointPath);


                if (!animJointXformAttr.isValid())
                {
                    Foundry::Katana::ReportError(
                        interface,
                        "No valid \"" + JOINT_XFORM +
                                "\" attribute at location " + animJointPath);

                    return false;
                }
                if (!restposeJointInvXformAttr.isValid())
                {
                    Foundry::Katana::ReportError(
                        interface,
                        "No valid \"" + JOINT_INVERSE_XFORM +
                                "\" attribute at location " +
                                restposeJointPath);

                    return false;
                }
                AddTimeSamplesToSet(animJointXformAttr, timeSamples);

                xforms.push_back(TransformData(animJointXformAttr,
                                               restposeJointInvXformAttr,
                                               childName));
           }
        }
        return skinDeformationFlag;
    }


public:

    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(
            Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        // Get root location path for the character
        FnAttribute::StringAttribute charRootAttr =
            Foundry::Katana::GetGlobalAttr(interface, CHARACTER_ROOT);

        if (!charRootAttr.isValid())
        {
            Foundry::Katana::ReportError(
                interface, "Missing global attribute \"" +
                        CHARACTER_ROOT + "\"");
            return;
        }

        const std::string charRootPath = charRootAttr.getValue("", false);
        std::set<float> timeSamples;

        const GroupAttribute geoArbAttr =
                interface.getAttr(GEOMETRY_ARBITRARY);

        if (!geoArbAttr.isValid())
        {
            Foundry::Katana::ReportError(
                interface, "No valid \"" + GEOMETRY_ARBITRARY +
                            "\" attribute found.");
            return;
        }

        std::vector<TransformData > xforms;

        // Apply skin deformation only if at least a weight is found
        if (!FindSkinWeights(interface, geoArbAttr, charRootPath,
                                    xforms, timeSamples))
            return;

        // Get original undeformed vertex positions from restpose geometry
        // at time 0
        const FloatAttribute vertPosAttr =
            interface.getAttr(POINT_VALUE);
        const FloatAttribute::array_type origVertPos =
            vertPosAttr.getNearestSample(0);

        // Reserve space for the vertices to be transformed.
        const size_t timeSamplesCount = timeSamples.size();
        std::vector<float> deformedVertices(
            timeSamplesCount * origVertPos.size());

        // Iterate over the xForms data and compute deformed vertices
        // based on skin weights
        for (std::vector<TransformData>::const_iterator it = xforms.begin();
                it != xforms.end();
                ++it)
        {
            const TransformData& tData = *it;
            Imath::M44d restposeJointInvXform =
                Matrix44FromAttr(tData.restposeJointInvXformAttr, .0f);

            const DoubleAttribute jointsWeightAttr =
                geoArbAttr.getChildByName(tData.childName + VALUE_SUFFIX);

            const DoubleAttribute::array_type jointWeights =
                jointsWeightAttr.getNearestSample(0); // not animated weighs

            size_t timeSlice = 0;

            for (std::set<float>::const_iterator it = timeSamples.begin();
                    it != timeSamples.end();
                    ++it)
            {
                const Imath::M44d animJointXform =
                    Matrix44FromAttr(tData.animJointXformAttr, *it);

                const Imath::M44d relativeJointXform =
                    restposeJointInvXform * animJointXform;

                for (size_t i = 0; i < jointWeights.size(); ++i)
                {
                    float w = jointWeights[i];

                    const float* origV = &origVertPos[i * 3];
                    Imath::V3f origP =
                        Imath::V3f(origV[0], origV[1], origV[2]);

                    Imath::V3f newP = origP * relativeJointXform;

                    float* targetVert = &deformedVertices
                        [i * 3 + origVertPos.size() * timeSlice];

                    targetVert[0] += w * newP.x;
                    targetVert[1] += w * newP.y;
                    targetVert[2] += w * newP.z;

                }
                ++timeSlice;
            }
            interface.deleteAttr(GEOMETRY_DELETE_CHILD_PREFIX +
                                 tData.childName);
        }

        // build the output attribute with the transformed vertices
        std::vector<float> timeSamplesVec(timeSamples.begin(),
                                          timeSamples.end());
        std::vector<const float*> verticesStrides(timeSamplesCount);

        for (size_t i =0; i < timeSamplesCount; ++i)
        {
            verticesStrides[i] =
                &(deformedVertices[i * origVertPos.size() * 3]);
        }

        const FloatAttribute deformedAttr(
                                timeSamplesVec.data(), timeSamplesVec.size(),
                                verticesStrides.data(), origVertPos.size(), 3);
        interface.setAttr(POINT_VALUE, deformedAttr);

    }
};

DEFINE_GEOLIBOP_PLUGIN(SkinEnvelopeOp)

} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(SkinEnvelopeOp, "SkinEnvelopeOp", 0, 1);
}

