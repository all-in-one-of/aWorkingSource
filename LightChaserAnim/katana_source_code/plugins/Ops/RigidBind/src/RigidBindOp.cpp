// Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <vector>
#include <iostream>
#include <string>
#include <utility>
#include <exception>

#include <FnAttribute/FnAttribute.h>
#include <FnGeolib/op/FnGeolibOp.h>


#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathMatrix.h>


namespace // anonymous
{

using FnAttribute::StringAttribute;
using FnAttribute::DataAttribute;
using FnAttribute::DoubleAttribute;

/* In the example scene RestposeJointsPath values
 *  is "/root/reference/restposeJoints"
*/
static const std::string RESTPOSE_OP_ARG_NAME = "RestposeJointsPath";
static const std::string JOINT_INVERSE_XFORM = "jointInvXform";
static const std::string JOINT_XFORM = "jointXform";
static const std::string CHARACTER_ROOT = "character.root";
static const std::string XFORM_MATRIX = "xform.matrix";
static const std::string RIGID_BIND_VALUE =
        "geometry.arbitrary.rigidBind.value";

std::vector<float> FindTimeSamples(const FnAttribute::DataAttribute& attr)
{
    std::vector<float> timeSamples;

    if(!attr.isValid())
        return timeSamples;

    int64_t count = attr.getNumberOfTimeSamples();
    timeSamples.reserve(count);

    for (int64_t i = 0; i < count; ++i)
    {
        timeSamples.push_back(attr.getSampleTime(i));
    }
    return timeSamples;
}

Imath::M44d Matrix44FromAttr(const FnAttribute::DoubleAttribute& attr,
                             float time)
{
    if (attr.isValid())
    {
        FnAttribute::DoubleAttribute::array_type values =
                attr.getNearestSample(time);

        if (attr.getNumberOfValues() == 16)
        {
            const double * ptr = values.data();
            return Imath::M44d(reinterpret_cast<const double(*)[4]>(ptr));
        }

    }
    return Imath::M44d();
}

class RigidBindOp : public Foundry::Katana::GeolibOp
{

public:

    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(
            Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {

        StringAttribute restposePathPrefixOpArg =
            interface.getOpArg(RESTPOSE_OP_ARG_NAME);

        if (!restposePathPrefixOpArg.isValid())
        {
            Foundry::Katana::ReportError(
                interface, "Op Argument RestposeJointsPath is required.");
            return;
        }
        const std::string restposeJointsPathPrefix =
            restposePathPrefixOpArg.getValue();

        interface.prefetch(restposeJointsPathPrefix, interface.getInputIndex());

        StringAttribute rigidBindAttr = interface.getAttr(RIGID_BIND_VALUE);

        if (!rigidBindAttr.isValid())
        {
            return;
        }
        std::string rigidBindJointPath = rigidBindAttr.getValue();

        // Get the inverse xform attribute from the restpose joint
        const std::string restposeJointPath =
            restposeJointsPathPrefix + rigidBindJointPath;

        DataAttribute restposeJointInvXformAttr =
            interface.getAttr(JOINT_INVERSE_XFORM, restposeJointPath);
        if (!restposeJointInvXformAttr.isValid())
        {
            Foundry::Katana::ReportError(
                interface,
                "RigidBind: No valid \"" + JOINT_INVERSE_XFORM +
                        "\" attribute at location " + restposeJointPath);

            return;
        }

        // Get the xform attribute from the animated joint
        FnAttribute::StringAttribute charRootAttr =
            Foundry::Katana::GetGlobalAttr(interface, CHARACTER_ROOT);
        if (!charRootAttr.isValid())
        {
            Foundry::Katana::ReportError(
                interface, "Missing attribute \"" + CHARACTER_ROOT + "\"");
            return;
        }

        std::string animJointPath =
            charRootAttr.getValue() + rigidBindJointPath;

        DoubleAttribute animJointXformAttr =
            interface.getAttr(JOINT_XFORM, animJointPath);
        if (!animJointXformAttr.isValid())
        {
            Foundry::Katana::ReportError(
                interface,
                "RigidBind: No valid \"" + JOINT_XFORM +
                    "\" attribute at location " + animJointPath);
            return;
        }
        std::vector<float> timeSamples(FindTimeSamples(animJointXformAttr));

        const Imath::M44d jointInvMat =
            Matrix44FromAttr(restposeJointInvXformAttr, .0f);

        const size_t numTimeSamples = timeSamples.size();
        std::vector<float> timeSamplesCount;
        timeSamplesCount.reserve(numTimeSamples);

        std::vector<Imath::M44d> relativeJointXformValues;
        relativeJointXformValues.reserve(numTimeSamples);

        std::vector<const double*> matrixPtrs;
        matrixPtrs.reserve(numTimeSamples);

        for (std::vector<float>::const_iterator it = timeSamples.begin();
             it != timeSamples.end();
             ++it)
        {
            const float time = *it;
            Imath::M44d animJointXform =
                    Matrix44FromAttr(animJointXformAttr, time);
            Imath::M44d relativeJointXform = jointInvMat * animJointXform;

            timeSamplesCount.push_back(time);
            relativeJointXformValues.push_back(relativeJointXform);
        }

        for (size_t i = 0; i < numTimeSamples; ++i)
        {
            matrixPtrs.push_back(relativeJointXformValues[i].getValue());
        }

        const DoubleAttribute res = DoubleAttribute(timeSamplesCount.data(),
                                                    numTimeSamples,
                                                    matrixPtrs.data(), 16, 4);

        interface.setAttr(XFORM_MATRIX, res);
    }
};

DEFINE_GEOLIBOP_PLUGIN(RigidBindOp)

} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(RigidBindOp, "RigidBindOp", 0, 1);
}

