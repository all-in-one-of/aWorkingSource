// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include "arnold_render.h"
#include "buildCommons.h"

#include <cstring>

#include <FnRenderOutputUtils/FnRenderOutputUtils.h>
#include <FnGeolibServices/FnXFormUtil.h>

using namespace FnKat;

/******************************************************************************
 *
 * buildGroup
 *
 ******************************************************************************/

AtNode* buildGroup(FnScenegraphIterator sgIterator, std::vector<AtNode*>* childNodes, bool forceExpand, bool nooverride, ArnoldPluginState* sharedState, bool obeyInstanceId)
{
    if (!forceExpand)
    {
        FnAttribute::IntAttribute forceExpandAttr = sgIterator.getAttribute("forceExpand");
        if (forceExpandAttr.isValid())
        {
            forceExpand = (forceExpandAttr.getValue() != 0);
        }
    }

    if (!nooverride)
    {
        GroupAttribute arnoldStatementsGroup = sgIterator.getAttribute("arnoldStatements", true);
        if (arnoldStatementsGroup.isValid())
        {
            FnAttribute::IntAttribute forceExpandOverrideAttr = arnoldStatementsGroup.getChildByName("forceExpandOverride");
            if (forceExpandOverrideAttr.isValid())
            {
               forceExpand = (forceExpandOverrideAttr.getValue() != 0);
            }
        }
    }

    // Build a procedural for this group.
    FnAttribute::DoubleAttribute worldBoundAttr;
    {
        FnAttribute::DoubleAttribute localBoundAttr = sgIterator.getAttribute("bound");
        if (!forceExpand && localBoundAttr.isValid())
        {
            worldBoundAttr = FnGeolibServices::FnXFormUtil::CollapseBoundsTimeSamples(
                FnGeolibServices::FnXFormUtil::CalcTransformedBoundsAtExistingTimes(
                    sgIterator.getGlobalXFormGroup(), localBoundAttr));
        }
    }

    /*
    This walking represents the main traversal of the scene graph, and as such
    is where we must explicit choose to walk the scene graph, while evicting un-
    used scene data.

    There are many plausible strategies for eviction, but the simplest and most
    obvious are to evict for each 'main-line' traversal, or, to evict on each
    getFirstChild call.

    In the interest of performance (and potential re-use of location data) we
    have chosen for the moment to only EVICT_ON_CHILD_TRAVERSAL, but future
    large-scale performance data may make us re-evaluate this default.

    We are *not* putting this in the hands of the end-user until demonstrated
    that it should be. (And would be a globalsSettings option).

    Note that under no circumstance should one disable EVICT_ON_CHILD_TRAVERSAL,
    (other than as an upper limit memory bound) as this would make scene graph
    traversal use unbounded memory.
    */

    static const bool EVICT_ON_CHILD_TRAVERSAL = true;
    static const bool EVICT_ON_SIBLING_TRAVERSAL = false;

    // Optimization: Are there any child locations beneath this location? If not
    // don't bother declaring the procedural.
    FnAttribute::StringAttribute potentialChildrenAttribute =\
        sgIterator.getPotentialChildren();
    if (!potentialChildrenAttribute.getNumberOfValues())
        return NULL;


    //if we don't have a bound, or we are asked to expand, do that now
    if (forceExpand || !worldBoundAttr.isValid())
    {
        FnScenegraphIterator forceExpandIterator =\
            sgIterator.getFirstChild(EVICT_ON_CHILD_TRAVERSAL);

        while (forceExpandIterator.isValid())
        {
            buildLocation(forceExpandIterator, childNodes, forceExpand,
                          nooverride, sharedState, obeyInstanceId);
            forceExpandIterator =\
                forceExpandIterator.getNextSibling(EVICT_ON_SIBLING_TRAVERSAL);
        }
        return NULL;
    }

    //ok, no more work on this branch, spit out a new procedural
    //or an instance and let it do the work
    std::string nameStr = sgIterator.getFullName();

    // boundData : minX, maxX, minY, maxY, minZ, maxZ
    FnAttribute::DoubleAttribute::array_type worldBounds =
        worldBoundAttr.getNearestSample(0.f);
    if (worldBounds.size() < 6 )
    {
        AiMsgError("[kat] Malformed worldBounds at %s.", nameStr.c_str());
        return 0;
    }

    // Build a single procedural with the first child; this will expand all siblings.
    AtNode* procedural = AiNode("procedural");
    if (!procedural)
    {
        AiMsgError("[kat] FATAL: Failed to create procedural at %s.", nameStr.c_str());
        exit(-1);
    }
    if (childNodes) childNodes->push_back(procedural);

    std::string procNameStr = nameStr + ":proc";

    AiNodeDeclare(procedural, "sharedState", "constant POINTER");
    AiNodeSetPtr(procedural, "sharedState", sharedState);

    AiNodeSetStr(procedural, "name", procNameStr.c_str());
    AiNodeSetPtr(procedural, "funcptr", (void*)(KatanaProcedural_ProcLoader));

    AiNodeSetPtr(procedural, "userptr", new FnScenegraphIterator(sgIterator.getRoot()));

    // We store the parent location that we want to begin child traversal and a
    // flag indicating such.
    AiNodeDeclare(procedural, "traverseFromFirstChild", "constant BOOL");
    bool traverseFromFirstChild = true;
    AiNodeSetBool(procedural, "traverseFromFirstChild", &traverseFromFirstChild);

    AiNodeDeclare(procedural, "scenegraphLocationPath", "constant STRING");
    std::string proceduralLocationParam = sgIterator.getFullName();
    AiNodeSetStr(procedural, "scenegraphLocationPath",
                 proceduralLocationParam.c_str());

    AiNodeSetPnt(procedural, "min", worldBounds[0], worldBounds[2], worldBounds[4]);
    AiNodeSetPnt(procedural, "max", worldBounds[1], worldBounds[3], worldBounds[5]);

    return procedural;
}
