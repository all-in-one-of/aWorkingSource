// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <PRManProcedural.h>


#include <pystring/pystring.h>

#include <iomanip>
#include <map>
#include <set>
#include <stdlib.h>
#include <string.h>

#include <OpenEXR/ImathMatrix.h>
SPI_GEOLIB_IMATH_NAMESPACE
using namespace Imath;

#include <FnAttribute/FnDataBuilder.h>
#include <ErrorReporting.h>

using namespace FnKat;

namespace PRManProcedural
{
static void save_visibility_attribute(const char* prmanAttrName, const char* katanaAttrName)
{
    // 'visibility' attributes are all ints, however RxAttribute always returns floats.
    // Must read as floats, then cast to int before writing out.
    RxInfoType_t valType;
    int valResultCount;
    RtFloat floatValue = 0.0f;
    RtInt intValue = 0;

    if (0 == RxAttribute(prmanAttrName, &floatValue, sizeof(floatValue), &valType, &valResultCount))
    {
        intValue = (RtInt)floatValue;
        RtToken tokens[] = { const_cast<char*>(katanaAttrName), 0 };
        RtPointer values[] = { &intValue, 0 };
        RiAttributeV((char*)"user", 1, tokens, values);
    }
}

static void setup_group_attributes(bool saveAndResetVisibility,
        const ProducerPacket & producerPacket)
{
    if (saveAndResetVisibility)
    {
        if( producerPacket.ribDump )
        {
            RiArchiveRecord( "comment", "All visibility attributes are set here to 1 to ensure the "
                                        "procedural is seen and expanded when rendering." );
            RiArchiveRecord( "comment", "The original values are stored in user attributes." );
        }
        else
        {
            // Save visibility to user attrs
            save_visibility_attribute("visibility:camera", "int katana_camera");
            save_visibility_attribute("visibility:diffuse", "int katana_diffuse");
            save_visibility_attribute("visibility:specular", "int katana_specular");
            save_visibility_attribute("visibility:transmission", "int katana_transmission");
            save_visibility_attribute("visibility:photon", "int katana_photon");
            save_visibility_attribute("visibility:midpoint", "int katana_midpoint");

            RtInt trueValue = 1;
            RtToken tokens[] = { (char*)"int katana_visibilitySet", 0 };
            RtPointer values[] = { &trueValue, 0 };
            RiAttributeV((char*)"user", 1, tokens, values);

            // Reset the visibility to all on.
            // This has potential memory implications
            // on subsequence Procedural calls...
            {
                //RtInt zero = 0.0f;
                RtInt one = 1;
                RtToken tokens[] = { (char*)"int camera",
                                     (char*)"int diffuse",
                                     (char*)"int specular",
                                     (char*)"int transmission",
                                     (char*)"int photon",
                                     (char*)"int midpoint",
                                     0 };
                RtPointer values[] = { &one, &one, &one, &one, &one, &one, 0};
                RiAttributeV((char*)"visibility",  6, tokens, values);
            }
        }
    }

    if ( producerPacket.grouping && producerPacket.groupHasBounds )
    {
        std::ostringstream buffer;
        bool foundOne = false;

        for ( ProducerPacket::GroupingMap::const_iterator I = producerPacket.grouping->begin();
                    I!=producerPacket.grouping->end(); ++I)
        {
            if (!((*I).second))
            {
                buffer << (foundOne ?  "," : "+") << (*I).first;
                foundOne = true;
            }
        }

        if (foundOne)
        {
            RiArchiveRecord ("comment", "The below suspect looking membership groups are for bounded procedurals.");

            RtToken tokens[] = { (char*)"uniform string membership", 0 };
            std::string groupNames = buffer.str();


            const char * stringValue[] = {groupNames.c_str(), 0};
            RtPointer values[] = { stringValue, 0 };
            RiAttributeV((char*)"grouping", 1, tokens, values);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
//
// WriteRI_Group
//
void WriteRI_Group(const ProducerPacket& producerPacket, PRManPluginState* sharedState)
{
    Report_Debug("WriteRI_Group called", producerPacket, sharedState);

    FnScenegraphIterator sgIterator = producerPacket.sgIterator;
    if (!sgIterator.isValid()) return;


    /*
    This walking represents the main traversal of the scene graph, and as such
    is where we must explicit choose to walk the scene graph, while evicting un-
    used scene data.

    There are many plausible strategies for eviction, but the simplest and most
    obvious are to evict for each 'main-line' traversal, or, to evict on each
    getFirstChild call.

    In the interest of performance (and potential re-use of location data) we
    have chosen for the moment to only evict when walking down (getFirstChild)
    but further evidence may make it preferable to also evict when walking
    peers, (getNextSibling()), or walking other lists (such as the light-list,
    at regular intervals, etc.
    */

    static const bool EVICT_ON_CHILD_TRAVERSAL = true;

    // Optimization: Are there any child locations beneath this location? If not
    // don't bother declaring the procedural.
    FnAttribute::StringAttribute potentialChildrenAttribute =\
        sgIterator.getPotentialChildren();
    if (!potentialChildrenAttribute.getNumberOfValues())
        return;

    // Are we in full recursion mode?
    bool fullRecurse = producerPacket.fullRecurse;
    FnAttribute::IntAttribute forceExpandAttr = sgIterator.getAttribute("forceExpand");
    if (forceExpandAttr.isValid())
    {
        fullRecurse = (forceExpandAttr.getValue() == 1);
    }

    // Is strict visibility testing enabled?
    bool enableStrictVisibilityTesting = producerPacket.enableStrictVisibilityTesting;
    FnAttribute::IntAttribute enableStrictVisibilityAttr = sgIterator.getAttribute("prmanStatements.enableStrictVisibilityTesting");
    if (enableStrictVisibilityAttr.isValid())
    {
        if(enableStrictVisibilityAttr.getValue())
        {
            enableStrictVisibilityTesting = true;
        }
        else if(enableStrictVisibilityTesting)
        {
            Report_Warning("enableStrictVisibilityTesting cannot be disabled below a previously enabled location.  Ignoring directive", sgIterator);
        }
    }

    // We don't explore the attributes of the iterator beyond the bounding box.
    // We only care about the bounding box, because PRMan can use it as an
    // optimization.
    //
    // QUESTION: Shouldn't this be child's bound attr, and not producer's bound
    // attr?
    //
    // ANSWER : No, the reason is that when we 'Subdivide' this bounded
    //          location, we also traverse through all of the child siblings.
    //          Thus, the appropriate bounds to use is the union of the bounds
    //          for all the children, which is most easily queried as the
    //          parents bounding box. If we were to naievely use the first child
    //          producer's bounds to represent the bounds for the entire scene
    //          graph level, then any siblings of first child which have a
    //          different (larger) bounds would be incorrectly clipped.

    FnAttribute::DoubleAttribute boundAttr = sgIterator.getAttribute("bound");
    RtBound bound;
    if(boundAttr.isValid())
    {
        FillBoundFromAttr( &bound, boundAttr, sharedState );
        if (bound[0] == bound[1] && bound[2] == bound[3] && bound[4] == bound[5])
        {
            Report_Warning("Empty bounding box detected (prman will not expand beyond this point).", sgIterator);
            return;
        }

        std::stringstream msg;
        msg << "bounds: [ ";
        for(int i = 0; i < 6; ++i)
        {
            msg << (float)bound[i] << " ";
        }
        msg << "]";
        Report_Debug(msg.str(), sharedState, sgIterator);
    }

    if (fullRecurse || !boundAttr.isValid() || producerPacket.ribDump)
    {
        // As we are doing a full immediate recurse, there's no reason not to
        // directly send down the producer. It's more efficient, and there is no
        // chance that prman will hang onto it and keep it alive anyways...
        FnScenegraphIterator childSgIterator;
        if (sharedState->proceduralSettings.printTimerInfo)
        {
            // Step the iterator down a level, and add this time to the
            // iteration counter.
            UTIL::Timer timer("ScenegraphTraversal.getFirstChild",
                              &globalScenegraphTraversalCounter);
            childSgIterator =\
                sgIterator.getFirstChild(EVICT_ON_CHILD_TRAVERSAL);
        }
        else
        {
            childSgIterator = sgIterator.getFirstChild(EVICT_ON_CHILD_TRAVERSAL);
        }

        ProducerPacket childSgIteratorPacket;
        childSgIteratorPacket.fullRecurse = fullRecurse;
        childSgIteratorPacket.enableStrictVisibilityTesting =\
            enableStrictVisibilityTesting;
        childSgIteratorPacket.sgIterator = childSgIterator;
        childSgIteratorPacket.ribDump = producerPacket.ribDump;
        childSgIteratorPacket.fromImmediateExpansion = true;

        childSgIteratorPacket.copyGroupingFromParent(producerPacket.grouping,
                                                     sgIterator);

        // This is so that we will show membership grouping in
        // RIB dump for bounded groups.
        childSgIteratorPacket.groupHasBounds = boundAttr.isValid();

        // If we have valid bounds without a forced expansion during a RIB dump,
        // we would like to simulate a bounded procedural to track the setting
        // and restoring of visibility attributes and make sure the dumped RIB
        // and the captured RIB are as similar as possible. We force
        // setup_group_attributes to add the visibility attributes as we know
        // the bounds are valid and we are in RIB dump mode where all we do is
        // replace the attributes with a comment. The fromImmediateExpansion
        // flag is then set to false to make sure the visibility restoration
        // takes place in the procedural.
        const bool pseudoProcedural = boundAttr.isValid() &&
                                      forceExpandAttr.getValue(0, false) == 0 &&
                                      producerPacket.ribDump;
        if( pseudoProcedural )
        {
            RiAttributeBegin();
            setup_group_attributes(true, childSgIteratorPacket);
            childSgIteratorPacket.fromImmediateExpansion = false;
        }

        PRManProcedural::Subdivide(&childSgIteratorPacket, 0);

        if( pseudoProcedural )
        {
            RiAttributeEnd();
        }
        return;
    }

    // Create a new subdivide data packet and fill in the details to allow it to
    // pick up where it left off.
    ProducerPacket* newProducerPacket =  new ProducerPacket;
    newProducerPacket->fullRecurse = fullRecurse;
    newProducerPacket->enableStrictVisibilityTesting = enableStrictVisibilityTesting;
    newProducerPacket->groupHasBounds = true;

    newProducerPacket->scenegraphLocation = sgIterator.getFullName();
    newProducerPacket->traverseFromFirstChild = true;

    RiAttributeBegin();
    setup_group_attributes(!newProducerPacket->enableStrictVisibilityTesting, *newProducerPacket);

    RiProcedural(newProducerPacket, bound, PRManProcedural::Subdivide, PRManProcedural::Free);
    RiAttributeEnd();
}

float GetLodRangeMultiplier( FnScenegraphIterator sgIterator )
{
    std::string cameraPath( "/root/world/cam/camera"), altCameraPath;

    FnAttribute::StringAttribute altCameraAttr = sgIterator.getRoot().getAttribute("prmanGlobalStatements.levelOfDetail.alternateLodCamera");
    if ( ! altCameraAttr.isValid() ) return 1.0;

    altCameraPath = altCameraAttr.getValue();

    FnAttribute::StringAttribute cameraNameAttr = sgIterator.getRoot().getAttribute("renderSettings.cameraName");

    if ( cameraNameAttr.isValid() )
    {
        cameraPath = cameraNameAttr.getValue();
    }

    std::string resolution = "square_512";

    FnAttribute::StringAttribute resolutionAttr = sgIterator.getRoot().getAttribute("renderSettings.resolution");
    if ( resolutionAttr.isValid() )
    {
        resolution = resolutionAttr.getValue();
    }

    std::string altResolution = resolution;
    FnAttribute::StringAttribute altResAttr = sgIterator.getRoot().getAttribute("prmanGlobalStatements.levelOfDetail.alternateLodResolution");

    if ( altResAttr.isValid() )
    {
        altResolution = altResAttr.getValue();
        if ( altResolution.empty() )
        {
            altResolution = resolution;
        }
    }

    int renderCameraPixelArea = FnKat::RenderOutputUtils::getPixelAreaThroughCamera( sgIterator, cameraPath, resolution );
    int altCameraPixelArea = FnKat::RenderOutputUtils::getPixelAreaThroughCamera( sgIterator, altCameraPath, altResolution );

    if ( altCameraPixelArea == 0 || renderCameraPixelArea == 0 ) return 1.0;

    return ((float)altCameraPixelArea)/((float) renderCameraPixelArea);
}

/////////////////////////////////////////////////////////////////////////////
//
// WriteRI_LevelOfDetailGroup
//
void WriteRI_LevelOfDetailGroup(const ProducerPacket& producerPacket, PRManPluginState* sharedState )
{
    Report_Debug("WriteRI_LevelOfDetailGroup called", producerPacket, sharedState);

    FnScenegraphIterator sgIterator = producerPacket.sgIterator;
    if (!sgIterator.isValid()) return;

    // writes detail group information from bounds
    FnAttribute::DoubleAttribute boundAttr = sgIterator.getAttribute("bound");
    if (boundAttr.isValid())
    {
        float relativeDetail = GetLodRangeMultiplier( sgIterator );

        if ( relativeDetail != 1 )
        {
            RtFloat values[] = { relativeDetail };
            RiAttribute((char*)"user", (char*)"uniform float relativeDetail", values, RI_NULL);
        }


        RtBound mybound;
        const double* boundValues = boundAttr.getNearestSample(0).data();
        for (int i=0;i<6;++i)
            mybound[i] = static_cast<float>(boundValues[i]);
        RiDetail(mybound);

    }

    WriteRI_Group(producerPacket, sharedState);
}

namespace
{
    float GetLodRangeMultiplier( FnScenegraphIterator sgIterator );
}

} // namespace PRManProcedural

