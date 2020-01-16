#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnAttributeUtils.h>
#include <FnAttribute/FnDataBuilder.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <pystring/pystring.h>

#include <FnPluginManager/FnPluginManager.h>
#include <FnGeolib/op/FnGeolibCookInterface.h>
#include <FnGeolibServices/suite/FnGeolibCookInterfaceUtilsSuite.h>

#include <FnGeolib/util/Path.h>
#include <FnGeolib/util/AssertException.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>

FNGEOLIBOP_NAMESPACE_ENTER
{
    std::string GetInputLocationType(
        const GeolibCookInterface & interface,
        const std::string & location,
        int inputIndex)
    {
        FnAttribute::StringAttribute attr = interface.getAttr("type", location, inputIndex);
        if(!attr.isValid()) return "";
        return attr.getValue("group", false);
    }

    std::string GetAbsOutputLocationPath(
        const GeolibCookInterface & interface,
        const std::string & outputLocationPath)
    {
        if(outputLocationPath.empty())
        {
            return interface.getOutputLocationPath();
        }
        else
        {
            return FnGeolibUtil::Path::RelativeToAbsPath(
                interface.getOutputLocationPath(), outputLocationPath);
        }
    }

    std::string GetAbsInputLocationPath(
        const GeolibCookInterface & interface,
        const std::string & inputLocationPath)
    {
        if(inputLocationPath.empty())
        {
            return interface.getInputLocationPath();
        }
        else
        {
            return FnGeolibUtil::Path::RelativeToAbsPath(
                interface.getInputLocationPath(), inputLocationPath);
        }
    }

    void CreateLocation(CreateLocationInfo & createLocationInfo,
                        GeolibCookInterface & interface,
                        const std::string & location_)
    {
        std::string absLocationToCreate = GetAbsOutputLocationPath(interface, location_);
        std::string currentOutputLocationPath = interface.getOutputLocationPath();

        if(absLocationToCreate == currentOutputLocationPath)
        {
            createLocationInfo.atLeaf = true;
            createLocationInfo.canMatchChildren = false;
            return;
        }

        if(!FnGeolibUtil::Path::IsAncestorOrEqual(currentOutputLocationPath, absLocationToCreate))
        {
            createLocationInfo.atLeaf = false;
            createLocationInfo.canMatchChildren = false;
            return;
        }

        std::string remainder = FnGeolibUtil::Path::NormalizedRelativePath(
            currentOutputLocationPath, absLocationToCreate);

        std::vector<std::string> parts;
        pystring::split(remainder, parts, "/", 1);
        exception_assert(parts.size() > 0);

        interface.createChild(parts[0]);
        createLocationInfo.atLeaf = false;
        createLocationInfo.canMatchChildren = true;
    }

    void RenameChild(GeolibCookInterface &interface,
        const std::string & src, const std::string & dst)
    {
        interface.copyLocationToChild(dst, src, kFnKatGeolibDefaultInput, src);
        interface.deleteChild(src);
    }

    FnAttribute::Attribute GetGlobalAttr(
            const GeolibCookInterface & interface, const std::string & name,
            const std::string & location, int inputIndex)
    {
        static FnPluginManager::LazyHostSuite<
            FnGeolibCookInterfaceUtilsHostSuite_v2>
            suite = { "GeolibCookInterfaceUtilsHost", 2 };

        FnGeolibGlobalAttributeProviderContextDefault ctx;
        ctx.inputIndex = inputIndex;
        ctx.cookInterfaceHandle = interface.getHandle();
        ctx.cookInterfaceSuite = interface.getSuite();

        uint8_t didAbort = false;
        FnAttribute::Attribute ret = FnAttribute::Attribute::CreateAndSteal(
                suite->getGlobalAttrGeneric(
                        kFnGeolibGlobalAttributeProviderDefault, &ctx,
                        name.c_str(), static_cast<int32_t>(name.size()),
                        location.c_str(), static_cast<int32_t>(location.size()),
                        &didAbort));
        if (didAbort)
            throw FnGeolibOp::GeolibCookInterface::QueryAbortException();

        return ret;
    }

    void CopyTree(
        GeolibCookInterface & interface,
        const std::string & dstLocation_,
        const std::string & srcLocation_,
        int srcInput)
    {
        std::string dstLocationPath = GetAbsOutputLocationPath(interface, dstLocation_);
        std::string srcLocationPath = GetAbsInputLocationPath(interface, srcLocation_);
        CreateLocationInfo info;
        CreateLocation(info, interface, dstLocationPath);
        if (info.atLeaf)
        {
            interface.replaceChildren(srcLocationPath, srcInput);
            interface.replaceAttrs(srcLocationPath, srcInput);
        }
    }

    void ReportError(GeolibCookInterface & interface,
                     const std::string & message)
    {
        interface.setAttr("type", FnAttribute::StringAttribute("error"));
        interface.setAttr("errorMessage",
            FnAttribute::StringAttribute(message));
    }

    void ReportWarning(GeolibCookInterface & interface,
                       const std::string & message)

    {
        interface.setAttr("warningMessage",
            FnAttribute::StringAttribute(message));
    }

    FnAttribute::DoubleAttribute GetBoundAttr(
        const GeolibCookInterface & interface,
        const std::string & inputLocationPath,
        int inputIndex)
     {
        FnAttribute::DoubleAttribute boundAttr =
            interface.getAttr("bound", inputLocationPath, inputIndex);
        if (boundAttr.isValid() && boundAttr.getNumberOfValues() == 6)
        {
            return boundAttr;
        }

        return FnAttribute::DoubleAttribute();
    }

    void GetBoundAttrValue(
        double outMinBound[3], double outMaxBound[3],
        const FnAttribute::DoubleAttribute & boundAttr,
        float sampleTime)
    {
        if (!boundAttr.isValid())
        {
            throw std::runtime_error(
                std::string("Cannot get bounds attribute."));
        }

        double bound[6];
        boundAttr.fillInterpSample(bound, 6, sampleTime);
        outMinBound[0] = bound[0];
        outMaxBound[0] = bound[1];
        outMinBound[1] = bound[2];
        outMaxBound[1] = bound[3];
        outMinBound[2] = bound[4];
        outMaxBound[2] = bound[5];
    }

    namespace // anonymous
    {
        void _MultVecMatrix(BoundPoint & p, const double m[16])
        {
            double x = p.x*m[0] + p.y*m[4] + p.z*m[8]  + m[12];
            double y = p.x*m[1] + p.y*m[5] + p.z*m[9]  + m[13];
            double z = p.x*m[2] + p.y*m[6] + p.z*m[10] + m[14];
            double w =
                p.x*m[3] + p.y*m[7] + p.z*m[11] + m[15];

            p.x = x/w;
            p.y = y/w;
            p.z = z/w;
        }
    }

    void GetTransformedBoundAttrValue(
        std::vector<BoundPoint> & outPoints,
        const FnAttribute::DoubleAttribute & boundAttr,
        float sampleTime, const double xform[16])
    {
        double minBound[3];
        double maxBound[3];

        GetBoundAttrValue(minBound, maxBound, boundAttr, sampleTime);

        outPoints.push_back(BoundPoint(minBound[0], minBound[1], minBound[2]));
        outPoints.push_back(BoundPoint(minBound[0], minBound[1], maxBound[2]));
        outPoints.push_back(BoundPoint(minBound[0], maxBound[1], minBound[2]));
        outPoints.push_back(BoundPoint(minBound[0], maxBound[1], maxBound[2]));
        outPoints.push_back(BoundPoint(maxBound[0], minBound[1], minBound[2]));
        outPoints.push_back(BoundPoint(maxBound[0], minBound[1], maxBound[2]));
        outPoints.push_back(BoundPoint(maxBound[0], maxBound[1], minBound[2]));
        outPoints.push_back(BoundPoint(maxBound[0], maxBound[1], maxBound[2]));

        for (std::vector<BoundPoint>::iterator it = outPoints.begin();
                it != outPoints.end(); ++it)
        {
            _MultVecMatrix(*it, xform);
        }
    }

    void GetTransformedBoundAttrValue(
        double outMinBound[3], double outMaxBound[3],
        const FnAttribute::DoubleAttribute & boundAttr,
        float sampleTime, const double xform[16])
    {
        std::vector<BoundPoint> points;
        GetTransformedBoundAttrValue(points, boundAttr, sampleTime, xform);

        BoundPoint minBound = points[0];
        BoundPoint maxBound = points[0];
        for (std::vector<BoundPoint>::const_iterator it = points.begin();
             it != points.end(); ++it)
        {
            minBound.x = std::min((*it).x, minBound.x);
            minBound.y = std::min((*it).y, minBound.y);
            minBound.z = std::min((*it).z, minBound.z);

            maxBound.x = std::max((*it).x, maxBound.x);
            maxBound.y = std::max((*it).y, maxBound.y);
            maxBound.z = std::max((*it).z, maxBound.z);
        }

        outMinBound[0] = minBound.x;
        outMinBound[1] = minBound.y;
        outMinBound[2] = minBound.z;

        outMaxBound[0] = maxBound.x;
        outMaxBound[1] = maxBound.y;
        outMaxBound[2] = maxBound.z;
    }

    namespace
    {
        void _FindAllSampleTimes(
                std::set<float> & outTimes, const FnAttribute::Attribute & attr)
        {
            if (!attr.isValid()) return;

            FnAttribute::DataAttribute dataAttr = attr;
            if(dataAttr.isValid())
            {
                int64_t numSampleTimes = dataAttr.getNumberOfTimeSamples();
                for (int64_t j = 0; j < numSampleTimes; ++j)
                {
                    outTimes.insert(dataAttr.getSampleTime(j));
                }
            }
            else
            {
                FnAttribute::GroupAttribute groupAttr = attr;
                if(groupAttr.isValid())
                {
                    int64_t numChildren = groupAttr.getNumberOfChildren();
                    for (int64_t i=0; i<numChildren; ++i)
                    {
                        _FindAllSampleTimes(outTimes, groupAttr.getChildByIndex(i));
                    }
                }
            }
        }

        void FindAllSampleTimesForAttrs(
            std::set<float> & outTimes,
            const std::vector<FnAttribute::Attribute> & attrs)
        {
            outTimes.clear();

            for (std::vector<FnAttribute::Attribute>::const_iterator
                 iter=attrs.begin(); iter!=attrs.end(); ++iter)
            {
                _FindAllSampleTimes(outTimes, *iter);
            }
        }
    }

    // REMOVE THIS in favor of FnXFormUtil::MergeBounds
    FnAttribute::DoubleAttribute MergeLocalBoundAttrs(
        const std::vector<FnAttribute::DoubleAttribute> & boundAttrs)
    {
        if (boundAttrs.empty())
        {
            return FnAttribute::DoubleAttribute();
        }
        else if (boundAttrs.size() == 1)
        {
            return boundAttrs[0];
        }
        else
        {
            // assert(boundAttrs.size() > 1)
            std::set<float> sampleTimes;
            FindAllSampleTimesForAttrs(sampleTimes,
                std::vector<FnAttribute::Attribute>(
                    boundAttrs.begin(), boundAttrs.end()));

            FnAttribute::DataBuilder<FnAttribute::DoubleAttribute> builder(2);

            // For all sample times...
            for(std::set<float>::iterator sI = sampleTimes.begin(),
                sE = sampleTimes.end(); sI != sE; ++sI)
            {
                std::vector<double> & s = builder.get((*sI));

                // For all bound attrs...
                for(std::vector<FnAttribute::DoubleAttribute>::const_iterator I =
                    boundAttrs.begin(), E = boundAttrs.end(); I != E; ++I)
                {
                    if(!I->isValid() || I->getNumberOfValues() != 6) continue;

                    double bound[6];
                    (*I).fillInterpSample(bound, 6, (*sI));

                    if(s.empty())
                    {
                        s.insert(s.end(), bound, bound+6);
                    }
                    else
                    {
                        s[0] = std::min(s[0], bound[0]); // xMin
                        s[1] = std::max(s[1], bound[1]); // xMax
                        s[2] = std::min(s[2], bound[2]); // yMin
                        s[3] = std::max(s[3], bound[3]); // yMax
                        s[4] = std::min(s[4], bound[4]); // zMin
                        s[5] = std::max(s[5], bound[5]); // zMax
                    }
                }
            }

            return builder.build();
        }
    }

    FnAttribute::GroupAttribute MergeImmediateGroupChildren(
        const std::vector<FnAttribute::GroupAttribute> & attrs)
    {
        if (attrs.empty())
        {
            return FnAttribute::GroupAttribute();
        }
        else if (attrs.size() == 1)
        {
            return attrs[0];
        }

        // So we have more than one attribute. Let's make a new vector
        // and prune consecutive identical values (and invalid attrs)
        // It's possible we're merging attr A atop itself

        uint64_t lastHash = 0;
        std::vector<FnAttribute::GroupAttribute> prunedattrs;
        prunedattrs.reserve(attrs.size());

        for (size_t i=0, e=attrs.size(); i<e; ++i)
        {
            if(!attrs[i].isValid()) continue;
            if(attrs[i].getHash().uint64() == lastHash) continue;
            lastHash = attrs[i].getHash().uint64();
            prunedattrs.push_back(attrs[i]);
        }

        if (prunedattrs.empty())
        {
            return FnAttribute::GroupAttribute();
        }
        else if (prunedattrs.size() == 1)
        {
            return prunedattrs[0];
        }
        else
        {
            // We want the left-most ordering preserved, so we update a groupbuilder
            // from left->right.  But... we then want the values on the left
            // most-group builders to be preserved so we walk backwards from right
            // to left.
            //
            // Even though this involves doubly walking all children, group-builder
            // is very efficient internally so this is typically faster than
            // locally managing a std::set<std::string> for child names.

            FnAttribute::GroupBuilder gb;
            for (int i=0, e=int(prunedattrs.size()); i<e; ++i)
            {
                if(prunedattrs[i].isValid()) gb.update(prunedattrs[i]);
            }
            // -2 is so we dont doubly add the right-most element
            // Note: this loop relies on a signed int index to work
            for (int i=int(prunedattrs.size())-2, e=0; i>=e; --i)
            {
                if(prunedattrs[i].isValid()) gb.update(prunedattrs[i]);
            }
            return gb.build();
        }
    }

    FnAttribute::GroupAttribute GetGlobalXFormGroup(
        const GeolibCookInterface & interface,
        const std::string & inputLocationPath,
        int inputIndex)
    {
        std::string locationPath = GetAbsInputLocationPath(
            interface, inputLocationPath);

        std::vector<std::string> locationStack;
        FnGeolibUtil::Path::GetLocationStack(locationStack,
            locationPath);

        FnAttribute::GroupBuilder gb(FnAttribute::GroupBuilder::BuilderModeStrict);
        gb.setGroupInherit(false);

        for (int i=0, e=int(locationStack.size()); i<e; ++i)
        {
            FnAttribute::GroupAttribute xform = interface.getAttr(
                "xform", locationStack[i], inputIndex);
            if (xform.isValid())
            {
                std::ostringstream os;
                os << "i" << i;
                gb.set(os.str(), xform);
            }
        }

        return gb.build();
    }

    float GetCurrentTime(const GeolibCookInterface & interface)
    {
        return FnAttribute::FloatAttribute(
                interface.getOpArg("system.timeSlice.currentTime"))
                        .getValue(0.0f, false);
    }

    float GetShutterOpen(const GeolibCookInterface & interface)
    {
        return FnAttribute::FloatAttribute(
                interface.getOpArg("system.timeSlice.shutterOpen"))
                        .getValue(0.0f, false);
    }

    float GetShutterClose(const GeolibCookInterface & interface)
    {
        return FnAttribute::FloatAttribute(
                interface.getOpArg("system.timeSlice.shutterClose"))
                        .getValue(0.0f, false);
    }

    int GetNumSamples(const GeolibCookInterface & interface)
    {
        return FnAttribute::IntAttribute(
                interface.getOpArg("system.timeSlice.numSamples"))
                        .getValue(1, false);
    }

    FnAttribute::Attribute GetGraphStateVariable(
        const GeolibCookInterface & interface,
        const std::string & variableName)
    {

        return interface.getOpArg("system.variables."+variableName);
    }


    namespace
    {
        FnAttribute::GroupAttribute _FilterTopLevelReferentialInheritanceAttrs(
            FnAttribute::GroupAttribute & refAttr)
        {

            FnAttribute::StringAttribute inheritAttrNamesAttr =
                    refAttr.getChildByName("inheritableAttributes");

            if (!inheritAttrNamesAttr.isValid())
            {
                return refAttr;
            }

            FnAttribute::StringAttribute::array_type inheritAttrNames =
                    inheritAttrNamesAttr.getNearestSample(0.0f);

            FnAttribute::GroupBuilder gb;
            for (FnAttribute::StringAttribute::array_type::const_iterator I =
                    inheritAttrNames.begin(), E = inheritAttrNames.end();
                            I != E; ++I)
            {
                std::string nameValue(*I);

                FnAttribute::Attribute attr =
                        refAttr.getChildByName(nameValue);

                if (attr.isValid())
                {
                    gb.set(nameValue, attr);
                }
            }
            return gb.build();
        }



        FnAttribute::GroupAttribute ResolveReferentialInheritanceImpl(
            const FnGeolibOp::GeolibCookInterface & interface,
            const std::string & locationPath,
            int inputIndex,
            const std::string & keyAttrName,
            const std::string & attrName,
            bool combineLocalAttr,
            std::set<std::string> & visitedLocationPaths)
        {
            FnAttribute::GroupAttribute result;

            // only visit each reference once
            if (visitedLocationPaths.find(locationPath) !=
                    visitedLocationPaths.end())
            {
                return result;
            }

            visitedLocationPaths.insert(locationPath);

            if (combineLocalAttr)
            {
                result = GetGlobalAttr(interface, attrName, locationPath,
                        inputIndex);
            }

            FnAttribute::StringAttribute keyAttr =
                    interface.getAttr(keyAttrName, locationPath);

            if (keyAttr.isValid())
            {
                std::vector<FnAttribute::GroupAttribute> refAttrs;
                refAttrs.reserve(keyAttr.getNumberOfValues());

                FnAttribute::StringAttribute::array_type keyValues =
                        keyAttr.getNearestSample(0.0f);

                for (FnAttribute::StringAttribute::array_type::const_iterator I =
                        keyValues.begin(), E = keyValues.end(); I != E; ++I)
                {
                    std::string keyValue = (*I);
                    if (!keyValue.empty())
                    {
                        FnAttribute::GroupAttribute refAttr =
                                ResolveReferentialInheritanceImpl(
                                        interface,
                                        keyValue,
                                        inputIndex,
                                        keyAttrName,
                                        attrName,
                                        true,
                                        visitedLocationPaths);

                        if (refAttr.isValid())
                        {
                            if (attrName.empty())
                            {
                                refAttr =
                                    _FilterTopLevelReferentialInheritanceAttrs(
                                            refAttr);
                            }

                            refAttrs.push_back(refAttr);
                        }
                    }
                }

                if (!refAttrs.empty())
                {
                    if (refAttrs.size() == 1 && !result.isValid())
                    {
                        result = refAttrs[0];
                    }
                    else
                    {
                        FnAttribute::GroupBuilder gb;

                        for (std::vector<FnAttribute::GroupAttribute>::iterator I
                                = refAttrs.begin(), E = refAttrs.end(); I != E;
                                    ++I)
                        {
                            gb.deepUpdate(*I);
                        }

                        if (result.isValid())
                        {
                            gb.deepUpdate(result);
                        }

                        result = gb.build();
                    }
                }
            }

            return result;

        }

        void _RemoveNullAttributes(
            FnAttribute::GroupBuilder & gb,
            const FnAttribute::GroupAttribute & groupAttr,
            const std::string & path = "")
        {
            if (!groupAttr.isValid())
            {
                return;
            }

            for (int64_t i = 0, e = groupAttr.getNumberOfChildren(); i < e;
                 ++i)
            {
                FnAttribute::Attribute childAttr =
                        groupAttr.getChildByIndex(i);

                FnAttribute::GroupAttribute childGroup = childAttr;
                if (childGroup.isValid())
                {
                    if (path.empty())
                    {
                        _RemoveNullAttributes(gb, childGroup,
                                              groupAttr.getChildName(i));
                    }
                    else
                    {
                        _RemoveNullAttributes(gb, childGroup,
                                              path + "."
                                              + groupAttr.getChildName(i));
                    }
                }
                else
                {
                    FnAttribute::NullAttribute nullAttr = childAttr;
                    if (nullAttr.isValid())
                    {
                        if (path.empty())
                        {
                            gb.del(groupAttr.getChildName(i));
                        }
                        else
                        {
                            gb.del(path + "." + groupAttr.getChildName(i));
                        }
                    }
                }
            }
        }
    } //anonymous namespace


    FnAttribute::GroupAttribute FilterNullAttributes(
        const FnAttribute::GroupAttribute & groupAttr)
    {
        FnAttribute::GroupBuilder gb;
        gb.update(groupAttr);
        _RemoveNullAttributes(gb, groupAttr);
        return gb.build();
    }

    FnAttribute::GroupAttribute ResolveReferentialInheritance(
        const FnGeolibOp::GeolibCookInterface & interface,
        const std::string & keyAttrName,
        const std::string & attrName,
        const std::string & inputLocationPath,
        int inputIndex)
    {
        std::set<std::string> visitedLocationPaths;
        return FilterNullAttributes(ResolveReferentialInheritanceImpl(
                interface, inputLocationPath, inputIndex, keyAttrName,
                attrName, false, visitedLocationPaths));
    }

}
FNGEOLIBOP_NAMESPACE_EXIT
