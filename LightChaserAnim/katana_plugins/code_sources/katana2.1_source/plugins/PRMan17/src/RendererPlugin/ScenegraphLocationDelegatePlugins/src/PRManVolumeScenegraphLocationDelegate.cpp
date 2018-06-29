// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <iostream>
#include <sstream>

#include <rx.h>

#include <FnScenegraphIterator/FnScenegraphIterator.h>

#include <ErrorReporting.h>
#include <WriteRI_Attributes.h>
#include <WriteRI_ObjectID.h>
#include <WriteRI_Shaders.h>
#include <WriteRI_Statements.h>
#include <WriteRI_Util.h>

#include <PRManVolumeScenegraphLocationDelegate.h>

using namespace PRManProcedural;
using namespace FnKat;

PRManVolumeScenegraphLocationDelegate::~PRManVolumeScenegraphLocationDelegate()
{
}

void PRManVolumeScenegraphLocationDelegate::flush()
{
}

PRManVolumeScenegraphLocationDelegate* PRManVolumeScenegraphLocationDelegate::create()
{
    return new PRManVolumeScenegraphLocationDelegate();
}

std::string PRManVolumeScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("prman");
}

void PRManVolumeScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back("volume");
}

void* PRManVolumeScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator, void* optionalInput)
{
    PRManSceneGraphLocationDelegateInput* state = reinterpret_cast<PRManSceneGraphLocationDelegateInput*>(optionalInput);

    if (!sgIterator.isValid())
    {
        Report_Error("invalid scenegraph iterator.");
        return 0x0;
    }

    // Save a reference to the scenegraph iterator
    m_sgIterator = sgIterator;

    // We check 'geometry.type' here so that we can ignore unsupported
    // location at this level
    if (!checkVolumeType())
    {
        Report_Error("unsupported volume type", m_sgIterator);
        return 0x0;
    }

    WriteRI_Object_LevelOfDetail(m_sgIterator, state->sharedState);

    RiAttributeBegin();

    WriteRI_Object_Name(m_sgIterator, state->sharedState);
    WriteRI_Object_Id(m_sgIterator, state->sharedState);
    WriteRI_Object_Transform(m_sgIterator.getAttribute("xform"), m_sgIterator, state->sharedState);
    WriteRI_Object_RelativeScopedCoordinateSystems(m_sgIterator, state->sharedState);
    WriteRI_Object_Statements(m_sgIterator, state->sharedState);
    WriteRI_Object_Material(m_sgIterator, state->sharedState);
    WriteRI_Object_IlluminationList(m_sgIterator, state->sharedState);

    void* returnValue = processVolumeLocation();

    RiAttributeEnd();

    return returnValue;
}

void* PRManVolumeScenegraphLocationDelegate::processVolumeLocation()
{
    assert(m_sgIterator.isValid());

    FnAttribute::GroupAttribute geometryAttr = m_sgIterator.getAttribute("geometry");
    if (!geometryAttr.isValid())
    {
        Report_Error("no valid 'geometry' attribute found.", m_sgIterator);
        return 0x0;
    }

    FnAttribute::StringAttribute volumeTypeAttr = geometryAttr.getChildByName("type");
    const std::string volumeType = volumeTypeAttr.getValue("", false);
    if (volumeType.empty())
    {
        Report_Error("no valid 'geometry.type' attribute found.", m_sgIterator);
        return 0x0;
    }

    // Handle the current location in a different way depending on the
    // value of the 'type' attribute
    if (volumeType == "riblobby")
    {
        return processRiBlobby(geometryAttr);
    }
    else if (volumeType == "riblobbydso")
    {
        return processRiBlobbyDso(geometryAttr);
    }
    else if (volumeType == "rivolume")
    {
        return processRiVolume(geometryAttr);
    }
    else if (volumeType == "rivolumedso")
    {
        return processRiVolumeDso(geometryAttr);
    }

    return 0x0;
}

void* PRManVolumeScenegraphLocationDelegate::processRiBlobby(GroupAttribute geometryAttr)
{
    assert(m_sgIterator.isValid());

    FnAttribute::IntAttribute nleafAttr = geometryAttr.getChildByName("nleaf");
    if (!nleafAttr.isValid())
    {
        Report_Error("no valid 'geometry.nleaf' attribute found.", m_sgIterator);
        return 0x0;
    }

    FnAttribute::IntAttribute codeAttr = geometryAttr.getChildByName("code");
    if (!codeAttr.isValid())
    {
        Report_Error("no valid 'geometry.code' attribute found.", m_sgIterator);
        return 0x0;
    }

    // Will hold sample times from not-arbitrary attributes
    std::set<float> extraSamples;

    FnAttribute::FloatAttribute floatsAttr = geometryAttr.getChildByName("floats");
    FnAttribute::StringAttribute stringsAttr = geometryAttr.getChildByName("strings");

    gatherSampleTimesFromAttr(extraSamples, floatsAttr);
    gatherSampleTimesFromAttr(extraSamples, stringsAttr);

    // Build the attribute list for arbitrary attributes
    AttrList attrList;
    AddArbitraryParameters(&attrList, geometryAttr, m_sgIterator);

    // Extra samples are passed to the converter to ensure we interpolate
    // arbitrary attributes for all sample times
    AttrList_Converter converter(attrList, extraSamples);

    std::vector<float> sampleTimes;
    calculateSampleTimes(converter, extraSamples, sampleTimes);

    if (sampleTimes.size() > 1)
    {
        RiMotionBeginV(sampleTimes.size(), &(sampleTimes[0]));
    }

    for (std::vector<float>::const_iterator it = sampleTimes.begin(); it != sampleTimes.end(); ++it)
    {
        const float currTime = *it;

        RtInt nleaf = nleafAttr.getNearestSample(currTime).at(0);
        RtInt ninst = (RtInt)codeAttr.getNumberOfValues();
        RtInt* inst = (RtInt*)codeAttr.getNearestSample(currTime).data();

        RtInt nflt = 0;
        RtInt nstr = 0;
        RtFloat* flt = NULL;
        RtToken* str = NULL;

        if (floatsAttr.isValid())
        {
            nflt = (RtInt)floatsAttr.getNumberOfValues();
            flt = (RtFloat*)floatsAttr.getNearestSample(currTime).data();
        }

        if (stringsAttr.isValid())
        {
            nstr = (RtInt)stringsAttr.getNumberOfValues();
            str = (RtToken*)stringsAttr.getNearestSample(currTime).data();
        }

        RtInt numParams = converter.getSize(currTime);
        RtToken* tokens = converter.getDeclarationTokens(currTime);
        RtPointer* values = converter.getParameters(currTime);

        RiBlobbyV(nleaf, ninst, inst, nflt, flt, nstr, str, numParams, tokens, values);
    }

    if (sampleTimes.size() > 1)
    {
        RiMotionEnd();
    }

    return 0x0;
}

void* PRManVolumeScenegraphLocationDelegate::processRiBlobbyDso(GroupAttribute geometryAttr)
{
    assert(m_sgIterator.isValid());

    FnAttribute::StringAttribute dsoAttr = geometryAttr.getChildByName("dso");
    const std::string dsoStr = dsoAttr.getValue("", false);

    if (dsoStr.empty())
    {
        Report_Error("no valid 'geometry.dso' attribute found", m_sgIterator);
        return 0x0;
    }

    // Will hold prman blobby opcodes
    std::vector<RtInt> codes;

    FnAttribute::IntAttribute volumetricAttr = geometryAttr.getChildByName("volumetric");
    const int volumetric = volumetricAttr.getValue(0, false);

    if (volumetric == 1)
    {
        // Opcode 8 tells RiBlobby to render as volume
        codes.push_back(8);
    }

    // Here we build a RiBlobby 'field plugin' call using opcode 1004.

    // Push opcode 1004 for 'field plugin'
    codes.push_back(1004);

    // Push the index in the strings array of the filename of the plugin.
    // This value is always '0'for us since we are building the string array manually.
    codes.push_back(0);

    // Will hold time samples from not-arbitrary attributes
    std::set<float> extraSamples;

    FnAttribute::FloatAttribute floatArgsAttr = geometryAttr.getChildByName("floatargs");
    FnAttribute::StringAttribute stringArgsAttr = geometryAttr.getChildByName("stringargs");

    gatherSampleTimesFromAttr(extraSamples, floatArgsAttr);
    gatherSampleTimesFromAttr(extraSamples, stringArgsAttr);

    // Build the attribute list for arbitrary attributes
    AttrList attrList;
    AddArbitraryParameters(&attrList, geometryAttr, m_sgIterator);
    AttrList_Converter converter(attrList, extraSamples);

    std::vector<float> sampleTimes;
    calculateSampleTimes(converter, extraSamples, sampleTimes);

    if (sampleTimes.size() > 1)
    {
        RiMotionBeginV(sampleTimes.size(), &(sampleTimes[0]));
    }

    for (std::vector<float>::const_iterator it = sampleTimes.begin(); it != sampleTimes.end(); ++it)
    {
        const float currTime = *it;

        RtInt nflt = 0;
        RtInt nstr = 0;
        RtFloat* flt = NULL;
        RtToken* str = NULL;

        std::vector<RtToken> strVec;

        // The first element of 'stringargs' is the dso name.
        strVec.push_back((RtToken)dsoStr.c_str());

        // Check the 'floatargs' parameter
        if (floatArgsAttr.isValid())
        {
            FnAttribute::FloatConstVector floatArgsVec = floatArgsAttr.getNearestSample(currTime);
            nflt = (RtInt)floatArgsVec.size();
            flt = (RtFloat*)floatArgsVec.data();
        }

        std::vector<RtInt> localCodes = codes;
        // Push number of floats and starting index
        localCodes.push_back(nflt);
        localCodes.push_back(0);

        // Check the 'stringargs' parameter
        if (stringArgsAttr.isValid())
        {
            FnAttribute::StringConstVector stringArgsVec = stringArgsAttr.getNearestSample(currTime);
            const char* const* stringArgs = stringArgsVec.data();
            for (size_t i = 0; i < stringArgsVec.size(); ++i)
            {
                strVec.push_back((RtToken)stringArgs[i]);
            }

            nstr = (RtInt)strVec.size();
            str = (RtToken*)&strVec[0];
        }

        // Push number of strings (dsoname excluded) and starting index (1)
        localCodes.push_back(nstr - 1);
        localCodes.push_back(1);

        RtInt numParams = converter.getSize(currTime);
        RtToken* tokens = converter.getDeclarationTokens(currTime);
        RtPointer* values = converter.getParameters(currTime);

        RiBlobbyV(1, localCodes.size(), &localCodes[0], nflt, flt, nstr, str, numParams, tokens, values);
    }

    if (sampleTimes.size() > 1)
    {
        RiMotionEnd();
    }

    return 0x0;
}

void* PRManVolumeScenegraphLocationDelegate::processRiVolume(FnAttribute::GroupAttribute geometryAttr)
{
    assert(m_sgIterator.isValid());

    FnAttribute::DoubleAttribute boundAttr = m_sgIterator.getAttribute("bound");
    if (!boundAttr.isValid() || boundAttr.getNumberOfValues() != 6)
    {
        Report_Warning("no valid 'bound' attribute found, unable to render volume", m_sgIterator);
        return 0x0;
    }

    FnAttribute::StringAttribute shapeAttr = geometryAttr.getChildByName("shape");
    const std::string shapeStr = shapeAttr.getValue("", false);

    if (shapeStr.empty())
    {
        Report_Error("no valid 'geometry.shape' attribute found", m_sgIterator);
        return 0x0;
    }

    // Will hold time samples from not-arbitrary attributes
    std::set<float> extraSamples;

    FnAttribute::IntAttribute voxelResAttr = geometryAttr.getChildByName("voxelResolution");

    gatherSampleTimesFromAttr(extraSamples, voxelResAttr);
    gatherSampleTimesFromAttr(extraSamples, boundAttr);

    // Build the attribute list for arbitrary attributes
    AttrList attrList;
    AddArbitraryParameters(&attrList, geometryAttr, m_sgIterator);
    AttrList_Converter converter(attrList, extraSamples);

    std::vector<float> sampleTimes;
    calculateSampleTimes(converter, extraSamples, sampleTimes);

    // Begin of prman commands
    if (sampleTimes.size() > 1)
    {
        RiMotionBeginV(sampleTimes.size(), &(sampleTimes[0]));
    }

    for (std::vector<float>::const_iterator it = sampleTimes.begin(); it != sampleTimes.end(); ++it)
    {
        const float currTime = *it;

        RtInt nvertices[] = { 0, 0, 0 };
        RtBound bound = { -1.1e30, 1.1e30, -1.1e30, 1.1e30, -1.1e30, 1.1e30 };

        getVoxelRes(voxelResAttr, currTime, nvertices);
        getBound(boundAttr, currTime, bound);

        RtInt numParams = converter.getSize(currTime);
        RtToken* tokens = converter.getDeclarationTokens(currTime);
        RtPointer* values = converter.getParameters(currTime);

        RiVolumeV((RtToken)shapeStr.c_str(), bound, nvertices, numParams, tokens, values);
    }

    if (sampleTimes.size() > 1)
    {
        RiMotionEnd();
    }

    return 0x0;
}

void* PRManVolumeScenegraphLocationDelegate::processRiVolumeDso(GroupAttribute geometryAttr)
{
    assert(m_sgIterator.isValid());

    FnAttribute::StringAttribute dsoAttr = geometryAttr.getChildByName("dso");
    const std::string dsoStr = dsoAttr.getValue("", false);

    if (dsoStr.empty())
    {
        std::cerr << "no valid 'geometry.dso' attribute found." << std::endl;
        return 0x0;
    }

    const std::string typeStr = "blobbydso:" + dsoStr;

    // Will hold prman blobby opcodes
    std::set<float> extraSamples;

    FnAttribute::IntAttribute voxelResAttr = geometryAttr.getChildByName("voxelResolution");
    FnAttribute::DoubleAttribute boundAttr = m_sgIterator.getAttribute("bound");

    gatherSampleTimesFromAttr(extraSamples, voxelResAttr);
    gatherSampleTimesFromAttr(extraSamples, boundAttr);

    // Build the attribute list for arbitrary attributes
    AttrList attrList;
    AddArbitraryParameters(&attrList, geometryAttr, m_sgIterator);

    // Check for 'floatargs' attr and add it to the arbitrary attributes list
    FnAttribute::FloatAttribute floatArgsAttr = geometryAttr.getChildByName("floatargs");
    if (floatArgsAttr.isValid())
    {
        std::stringstream strStream;
        strStream << "constant float[" << floatArgsAttr.getNumberOfValues() << "] blobbydso:floatargs";
        AddEntry(&attrList, AttrListEntry(strStream.str(), "floatargs", floatArgsAttr, true));
    }

    // Check for 'stringargs' attr and add it to the arbitrary attributes list
    FnAttribute::StringAttribute stringArgsAttr = geometryAttr.getChildByName("stringargs");
    if (stringArgsAttr.isValid())
    {
        std::stringstream strStream;
        strStream << "constant string[" << stringArgsAttr.getNumberOfValues() << "] blobbydso:stringargs";
        AddEntry(&attrList, AttrListEntry(strStream.str(), "stringargs", stringArgsAttr, true));
    }

    AttrList_Converter converter(attrList, extraSamples);

    std::vector<float> sampleTimes;
    calculateSampleTimes(converter, extraSamples, sampleTimes);

    if (sampleTimes.size() > 1)
    {
        RiMotionBeginV(sampleTimes.size(), &(sampleTimes[0]));
    }

    for (std::vector<float>::const_iterator it = sampleTimes.begin(); it != sampleTimes.end(); ++it)
    {
        const float currTime = *it;

        RtInt nvertices[] = {0, 0, 0};
        RtBound bound = { -1.1e30, 1.1e30, -1.1e30, 1.1e30, -1.1e30, 1.1e30 };

        getVoxelRes(voxelResAttr, currTime, nvertices);
        getBound(boundAttr, currTime, bound);

        RtInt numParams = converter.getSize(currTime);
        RtToken* tokens = converter.getDeclarationTokens(currTime);
        RtPointer* values = converter.getParameters(currTime);

        RiVolumeV((RtToken)typeStr.c_str(), bound, nvertices, numParams, tokens, values);
    }

    if (sampleTimes.size() > 1)
    {
        RiMotionEnd();
    }

    return 0x0;
}

bool PRManVolumeScenegraphLocationDelegate::checkVolumeType()
{
    assert(m_sgIterator.isValid());

    FnAttribute::GroupAttribute geometryAttr = m_sgIterator.getAttribute("geometry");
    if (!geometryAttr.isValid())
    {
        return false;
    }

    FnAttribute::StringAttribute volumeTypeAttr = geometryAttr.getChildByName("type");
    const std::string volumeType = volumeTypeAttr.getValue("", false);

    bool valid = false;

    valid |= (volumeType == "riblobby");
    valid |= (volumeType == "riblobbydso");
    valid |= (volumeType == "rivolume");
    valid |= (volumeType == "rivolumedso");

    return valid;
}

void PRManVolumeScenegraphLocationDelegate::gatherSampleTimesFromAttr(std::set<float>& sampleTimes, FnKat::DataAttribute attr) const
{
    if (!attr.isValid())
    {
        return;
    }

    int64_t numSampleTimes = attr.getNumberOfTimeSamples();

    // Only add time samples if there are more than 1 sample.
    if (numSampleTimes > 1)
    {
        for (int64_t i = 0; i < numSampleTimes; ++i)
        {
            sampleTimes.insert(attr.getSampleTime(i));
        }
    }
}

void PRManVolumeScenegraphLocationDelegate::calculateSampleTimes(AttrList_Converter& converter,
                                                                 const std::set<float>& extraSamples,
                                                                 std::vector<float>& sampleTimes) const
{
    // If converter has less than 2 samples it could mean that:
    //  - there are no arbitrary attributes
    //  - there are no multi-sample arbitrary attributes
    //
    // In both cases we could still have multiple sample times
    // in not-arbitrary attributes, so we use extraSamples instead

    if (converter.getNumSampleTimes() <= 1 && extraSamples.size() > 0)
    {
        sampleTimes.clear();
        sampleTimes.resize(extraSamples.size());
        std::copy(extraSamples.begin(), extraSamples.end(), sampleTimes.begin());
    }
    else
    {
        sampleTimes = converter.getSampleTimes();
    }
}

void PRManVolumeScenegraphLocationDelegate::getVoxelRes(FnAttribute::IntAttribute voxelResAttr, float time, RtInt (&voxelRes)[3]) const
{
    if (voxelResAttr.isValid())
    {
        if (voxelResAttr.getNumberOfValues() == 3)
        {
            const int* voxelResData = voxelResAttr.getNearestSample(time).data();
            for (int i = 0; i < 3; ++i)
            {
                voxelRes[i] = voxelResData[i];
            }
        }
        else
        {
            Report_Warning("'voxelResolution' attribute has an invalid number of elements", m_sgIterator);
        }
    }
    else
    {
        Report_Warning("no valid 'voxelResolution' attribute found, using default value", m_sgIterator);
    }
}

void PRManVolumeScenegraphLocationDelegate::getBound(FnAttribute::DoubleAttribute boundAttr, float time, RtBound& bound) const
{
    if (boundAttr.isValid())
    {
        if (boundAttr.getNumberOfValues() == 6)
        {
            FnAttribute::DoubleConstVector boundData = boundAttr.getNearestSample(time);
            for (int i = 0; i < 6; ++i)
            {
                bound[i] = static_cast<RtFloat>(boundData[i]);
            }
        }
        else
        {
            Report_Warning("'bound' attribute has an invalid number of elements", m_sgIterator);
        }
    }
    else
    {
        Report_Warning("no valid 'bound' attribute found, using default value", m_sgIterator);
    }
}
