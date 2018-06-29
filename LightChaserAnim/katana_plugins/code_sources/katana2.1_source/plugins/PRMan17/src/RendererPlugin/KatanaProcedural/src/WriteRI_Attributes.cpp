// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <WriteRI_Attributes.h>
#include <ri.h>

#include <pystring/pystring.h>
#include <list>
#include <ErrorReporting.h>
#include <PRManPluginState.h>
#include <rx.h>

using namespace FnKat;

namespace PRManProcedural
{
void WriteRI_Object_Name(FnScenegraphIterator sgIterator, PRManPluginState* sharedState)
{
    Report_Debug("WriteRI_Object_Name called", sharedState, sgIterator);

    // Write the short name for this object.
    // This is used inside shaders as well as for reporting prman errors.
    // It might be nice to use the fullname, but since the shaders aren't expecting it
    // we can't send this.

    // Write the name unless there's a prmanStatements.omitIdentifierName attribute.
    FnAttribute::IntAttribute omitAttr = sgIterator.getAttribute("prmanStatements.omitIdentifierName");
    if (!(omitAttr.isValid() && omitAttr.getValue()))
    {
        //std::string name = sgIterator.getName();
        std::string name = sgIterator.getFullName();
        char* nameArray[] = { const_cast<char*>(name.c_str()), RI_NULL };
        RiAttribute((char*)"identifier", (char*)"uniform string name", nameArray, RI_NULL);
    }
}


void WriteRI_Object_LevelOfDetail(FnScenegraphIterator sgIterator, PRManPluginState* sharedState)
{
    Report_Debug("WriteRI_Object_LevelOfDetail called", sharedState, sgIterator);

    // Only items of type "level-of-detail" have LOD information.
    if (sgIterator.getType() != "level-of-detail") return;

    // writes detail range information prior to attribute begin
    FnAttribute::GroupAttribute lodRanges = sgIterator.getAttribute("lodRanges", true);

    if (!lodRanges.isValid())
    {
        Report_Fatal("lodRanges attribute missing.", sgIterator);
        return;
    }

    float d0, d1, d2, d3;
    FnAttribute::FloatAttribute floatAttr;

    floatAttr = lodRanges.getChildByName("minVisible");
    if (!floatAttr.isValid())
    {
        Report_Fatal("lodRanges attribute corrupt.", sgIterator);
    }
    d0 = floatAttr.getValue();

    floatAttr = lodRanges.getChildByName("lowerTransition");
    if (!floatAttr.isValid())
    {
        Report_Fatal("lodRanges attribute corrupt.", sgIterator);
    }
    d1 = floatAttr.getValue();

    floatAttr = lodRanges.getChildByName("upperTransition");
    if (!floatAttr.isValid())
    {
        Report_Fatal("lodRanges attribute corrupt.", sgIterator);
    }
    d2 = floatAttr.getValue();

    floatAttr = lodRanges.getChildByName("maxVisible");
    if (!floatAttr.isValid())
    {
        Report_Fatal("lodRanges attribute corrupt.", sgIterator);
    }
    d3 = floatAttr.getValue();

    RxInfoType_t valType;
    int valResultCount;

    RtFloat attrValue = 1.0;
    if (0 == RxAttribute("user:relativeDetail", &attrValue, sizeof(attrValue), &valType, &valResultCount) )
    {
        if ( attrValue > 0 && attrValue != 1 )
        {
            d0/=attrValue;
            d1/=attrValue;
            d2/=attrValue;
            d3/=attrValue;
        }
    }


    RiDetailRange(d0, d1, d2, d3);
}


void WriteRI_Object_Transform(GroupAttribute xformAttr, FnScenegraphIterator sgIterator, PRManPluginState* sharedState)
{
    Report_Debug("WriteRI_Object_Transform called", sharedState, sgIterator);
    if (!xformAttr.isValid()) return;

    int numChildren = xformAttr.getNumberOfChildren();
    for (int i = 0; i < numChildren; i++)
    {
        std::string childName = xformAttr.getChildName(i);
        Attribute childAttr = xformAttr.getChildByIndex(i);

        if(!childAttr.isValid())
        {
            continue;
        }

        // Origin is an RiIdentity
        if (pystring::startswith(childName, "origin" ))
        {
            RiIdentity();
            continue;
        }

        // A group, named anything, is traversed into.
        FnAttribute::GroupAttribute groupAttr = childAttr;
        if (groupAttr.isValid())
        {
            WriteRI_Object_Transform(groupAttr, sgIterator, sharedState);
            continue;
        }

        // The rest must be simple transform commands.
        FnAttribute::DoubleAttribute transAttr = childAttr;
        if (!transAttr.isValid())
        {
            Report_Debug("Transform is not a double", sharedState, sgIterator);
            continue;
        }

        // Get all the samples in the xform attribute
        int64_t numSampleTimes = transAttr.getNumberOfTimeSamples();
        if (numSampleTimes < 1)
        {
            Report_Debug("Transform has no samples", sharedState, sgIterator);
            continue;
        }

        // N.B. Sample times in this context are always relative to the current frame.
        //      It is the responsibility of the ScenegraphGenerator to translate
        //       sample times into "current frame" space.
        std::vector<float> sampleTimes;

        // Include any samples which may affect the current shutter period
        //  - i.e. any during the shutter period and one either side
        const float& shutterOpen = sharedState->proceduralSettings.shutterOpenTime;
        const float& shutterClose = sharedState->proceduralSettings.shutterCloseTime;

        int64_t firstSampleIx = 0;
        int64_t lastSampleIx = numSampleTimes-1;

        int64_t currSampleIx=0;

        // Find the first sample - increment until the NEXT sample is after shutterOpen.
        while((++currSampleIx) <= lastSampleIx)
        {
            float samp = transAttr.getSampleTime(currSampleIx);
            if (samp <= shutterOpen) firstSampleIx++;
            else break;
        }

        // Find the last sample - decrement until the PREV sample is before shutterClose.
        currSampleIx = lastSampleIx;
        while((--currSampleIx) >= firstSampleIx)
        {
            float samp = transAttr.getSampleTime(currSampleIx);
            if (samp >= shutterClose) lastSampleIx--;
            else break;
        }

        // Now grab all the time samples between first and last
        for (currSampleIx=firstSampleIx; currSampleIx<=lastSampleIx; currSampleIx++)
        {
            float samp = transAttr.getSampleTime(currSampleIx);
            sampleTimes.push_back( samp );
        }

        // Generate RI calls for the transform
        const unsigned int numSamples = sampleTimes.size();

        const bool hasMotion = (numSamples > 1);
        
        const bool isTranslate = pystring::startswith(childName, "translate");
        const bool isRotate    = pystring::startswith(childName, "rotate");
        const bool isScale     = pystring::startswith(childName, "scale");
        const bool isMatrix    = pystring::startswith(childName, "matrix");

        const bool isValid = isTranslate || isRotate || isScale || isMatrix;

        if (isValid)
        {
            if(hasMotion) RiMotionBeginV( sampleTimes.size(), &sampleTimes[0] );

            for (unsigned int i=0; i<numSamples; i++)
            {
                const float& timeSample = sampleTimes[i];
                DoubleConstVector values = transAttr.getNearestSample(timeSample);

                if (isTranslate)
                {
                    RiTranslate(values[0], values[1], values[2]);
                }
                else if(isRotate)
                {
                    RiRotate(values[0], values[1], values[2], values[3]);
                }
                else if(isScale)
                {
                    RiScale(values[0], values[1], values[2]);
                }
                else if(isMatrix)
                {
                    //Convert the double matrix in "values" into a float array
                    float array[4][4];
                    float *pFloat = array[0];
                    float* pEnd = &array[3][3];
                    const double* pSrc = values.data();
                    do
                    {
                        *pFloat++ = static_cast<float>(*pSrc++);
                    }
                    while (pFloat <= pEnd);
                    RiConcatTransform(array);
                }
            }

            if(hasMotion) RiMotionEnd();
        }
    }
}




void WriteRI_Object_GlobalTransform(FnScenegraphIterator sgIterator, PRManPluginState* sharedState)
{
    Report_Debug("WriteRI_Object_GlobalTransform called", sharedState, sgIterator);

    std::list<FnScenegraphIterator> parentList;
    while (sgIterator.isValid())
    {
        parentList.push_front(sgIterator);
        sgIterator = sgIterator.getParent();
    }

    while (!parentList.empty())
    {
        FnAttribute::GroupAttribute xformAttr = parentList.front().getAttribute("xform");


        WriteRI_Object_Transform(xformAttr, parentList.front(), sharedState);
        parentList.pop_front();
    }
}

void WriteRI_Object_RelativeScopedCoordinateSystems(FnScenegraphIterator sgIterator, PRManPluginState* sharedState)
{
    FnAttribute::GroupAttribute relativeScopedCoordinateSystemsGroup = sgIterator.getAttribute("relativeScopedCoordinateSystems");
    if (!relativeScopedCoordinateSystemsGroup.isValid()) return;

    int numChildren = relativeScopedCoordinateSystemsGroup.getNumberOfChildren();

    for (int i = 0; i < numChildren; i++)
    {
        FnAttribute::StringAttribute pathAttr = relativeScopedCoordinateSystemsGroup.getChildByIndex(i);
        std::string pathAttrName = relativeScopedCoordinateSystemsGroup.getChildName(i);

        if (!pathAttr.isValid()) continue;

        std::string relativePath =  pystring::lstrip(pathAttr.getValue(), "/");

        //Allow relative to path to be local to permit use of this attribute as a regular
        //scoped coordinate system. The prmanStatements.scopedCoordinateSystem attribute
        //convention only allows one scoped coordinate system per location. Because this
        //attribute convention already allows multiple, it's reasonable to repurpose it
        //for this use.
        if ( relativePath.empty() || relativePath == "." )
        {
            RiScopedCoordinateSystem(const_cast<char*>( pathAttrName.c_str() ));
            continue;
        }

        std::string path = sgIterator.getName() + "/" + relativePath;

        FnScenegraphIterator childSgIt = sgIterator.getByPath(path);
        if (!childSgIt.isValid()) continue;

        RiTransformBegin();
        RiIdentity();
        WriteRI_Object_GlobalTransform(childSgIt, sharedState);
        RiScopedCoordinateSystem(const_cast<char*>( pathAttrName.c_str() ));
        RiTransformEnd();
    }
}

}
