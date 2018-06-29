// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <rx.h>
#include <iostream>
#include <PRManSubdmeshScenegraphLocationDelegate.h>

#include <PRManProcedural.h>
#include <sstream>
#include <ErrorReporting.h>

#include <WriteRI_Util.h>

using namespace PRManProcedural;
#include <WriteRI_Attributes.h>
#include <WriteRI_ObjectID.h>
#include <WriteRI_Shaders.h>
#include <WriteRI_Statements.h>

using namespace FnKat;

// A list of RiResource handles. These need to be in memory until the
// the very last moment.
std::vector< std::string > resourceHandles;

void* processSubdmesh(FnScenegraphIterator sgIterator, PRManPluginState* sharedState)
{
    if (!sgIterator.isValid()) return 0x0;
    if (!IsVisible(sgIterator)) return 0x0;

    AttrList attrList;
    RtInt num_polys;
    std::vector<RtInt> num_vertices, vertices;
    if (!BuildPolymeshData(sgIterator, &num_polys, &num_vertices, &vertices, &attrList))
    {
        // Error already reported by BuildPolymeshData... just return
        return 0x0;
    }

    std::vector<RtInt> num_loops(num_polys, 1); // num_polys elements, initialized to 1.

    FnAttribute::GroupAttribute geometryGroup = sgIterator.getAttribute("geometry", true);
    if (geometryGroup.isValid())
    {
        AddArbitraryParameters(&attrList, geometryGroup, sgIterator);
        AddRefParameters(&attrList, geometryGroup, sgIterator);
    }

    // The rest is subdiv specific data
    std::vector<RtToken> tags;
    std::vector<RtInt>   tagArgCounts;
    std::vector<RtInt>   tagIntArgs;
    std::vector<RtFloat> tagFloatArgs;
    std::vector<RtToken> tagStringArgs;

    // Get the scheme to use for the subdivision mesh or surface
    std::string scheme = "catmull-clark";
    const std::string schemeAttrName = "prmanStatements.subdivisionMesh.scheme";
    StringAttribute schemeAttr = sgIterator.getAttribute(schemeAttrName, true);
    if (schemeAttr.isValid())
    {
        scheme = schemeAttr.getValue();
    }

    bool isHSub = false;

    FnAttribute::GroupAttribute texturesAttr = sgIterator.getAttribute("textures", true);
    if (texturesAttr.isValid())
    {
#if 1
        AddTextureAttrs(&attrList, texturesAttr, num_polys, sgIterator);
#else
        //support for per-face texture mapping via faceedits
        GroupAttr::NamedAttrVector_Type attrVector;
        texturesAttr->getChildVector(&attrVector);
        int numChildren = texturesAttr.getNumberOfChildren();

        for (int i = 0; i < numChildren; i++)
        {
            Attribute childAttr = texturesAttr.getChildByIndex(i);
            std::string childName = texturesAttr.getChildName(i);

            FnAttribute::GroupAttribute groupAttr = childAttr;
            FnAttribute::StringAttribute stringAttr;
            if (groupAttr.isValid())
            {
                FnAttribute::IntAttribute indicesAttr = groupAttr.getChildByName("indicies");
                if (!indicesAttr.isValid()) indicesAttr = groupAttr.getChildByName("indices");
                FnAttribute::StringAttribute mapsAttr = groupAttr.getChildByName("maps");

                if (indicesAttr.isValid() && mapsAttr.isValid())
                {
                    FnAttribute::IntConstVector indexData = indicesAttr->getNearestSample(0);
                    FnAttribute::StringConstVector mapData = mapsAttr->getNearestSample(0);

                    AddEntry(&attrList,  AttrListEntry("constant string " + childAttr, childAttr, FnAttribute::StringAttribute(mapData[0]), true));

                    if ( mapData.size() > 1 )
                    {
                        isHSub = true;

                        for ( int i = 1, e = (int) mapData.size(); i<e; ++i )
                        {
                            tags.push_back("faceedit");
                            tagStringArgs.push_back("set");
                            tagStringArgs.push_back( const_cast<char *>( childName.c_str() ) );
                            //tagStringArgs.push_back( const_cast<char *>( mapData[i].c_str() ) );

                            //TEMPORARY until it's proven that prman doesn't copy these on its own
                            tagStringArgs.push_back(strdup(mapData[i].c_str() ) );

                            int intLength = 0;
                            int faceIndex = 0;
                            for ( FnAttribute::IntConstVector::const_iterator I = indexData.begin(), E = indexData.end(); I!=E; ++I, ++faceIndex)
                            {

                                if ( (*I) == i )
                                {
                                    tagIntArgs.push_back(1);
                                    tagIntArgs.push_back(faceIndex);
                                    intLength+=2;
                                }
                            }


                            tagArgCounts.push_back( intLength ); // num int args
                            tagArgCounts.push_back(0); // num float args
                            tagArgCounts.push_back(3); // num string arg
                        }
                    }
                }
            }
            else
            {
                stringAttr = childAttr;
                if ( stringAttr.isValid() )
                {
                    AddEntry(&attrList,  AttrListEntry("constant string " + childName, childName, stringAttr, true));
                }
            }
        }
#endif
    }

    if (isHSub)
    {
        RiResourceBegin();
    }

    for (FnScenegraphIterator childSgIterator = sgIterator.getFirstChild(); childSgIterator.isValid(); childSgIterator = childSgIterator.getNextSibling())
    {
        if (childSgIterator.getType() != "faceset")
        {
            Report_Debug("Found a child of a subdivision surface that's not a face set", sharedState, childSgIterator);
            continue;
        }

        FnAttribute::GroupAttribute childGeometryGroup = childSgIterator.getAttribute("geometry");
        if (!childGeometryGroup.isValid())
        {
            Report_Debug("geometry group attribute missing", sharedState, childSgIterator);
            continue;
        }

        FnAttribute::IntAttribute facesAttr = childGeometryGroup.getChildByName("faces");
        if (!facesAttr.isValid())
        {
            Report_Debug("geometry.faces attribute missing", sharedState, childSgIterator);
            continue;
        }

        FnAttribute::IntConstVector facesAttrVec = facesAttr.getNearestSample(0);

        //if it's set to invisible, create a hole and skip the rest
        FnAttribute::IntAttribute visibleAttr = childSgIterator.getAttribute("visible");
        if (visibleAttr.isValid() && visibleAttr.getValue() == 0)
        {
            if (!isHSub)
            {
                RiResourceBegin();
                isHSub = true;
            }

            tags.push_back((char*)"hole");
            tagArgCounts.push_back(facesAttrVec.size()); // num int args
            tagArgCounts.push_back(0); // num float args
            tagArgCounts.push_back(0); // num string args
            tagIntArgs.insert( tagIntArgs.end(), facesAttrVec.begin(), facesAttrVec.end() );
        }
        // If some attribute is set locally, start a special subD.
        else if (childSgIterator.getAttribute( "material" ).isValid() ||
                 childSgIterator.getAttribute( "prmanStatements").isValid() ||
                 childSgIterator.getAttribute( "lightList").isValid())
        {
            if (!isHSub)
            {
                RiResourceBegin();
                isHSub = true;
            }

            RiAttributeBegin();

            WriteRI_Object_Statements(childSgIterator, sharedState);
            WriteRI_Object_Material(childSgIterator, sharedState);
            WriteRI_Object_IlluminationList(childSgIterator, sharedState);


            char * stringArray[1] = { (char*)"save" };
            resourceHandles.push_back(childSgIterator.getName());

            RiResource(const_cast<RtToken>(resourceHandles.back().c_str()), (char*)"attributes", (char*)"string operation", stringArray, NULL);
            RiAttributeEnd();

            tags.push_back((char*)"faceedit");
            tagArgCounts.push_back(facesAttrVec.size() * 2 ); // num int args
            tagArgCounts.push_back(0); // num float args
            tagArgCounts.push_back(3); // num string arg

            for ( FnAttribute::IntConstVector::const_iterator I = facesAttrVec.begin(), E = facesAttrVec.end(); I!=E;++I )
            {
                tagIntArgs.push_back( 1 );
                tagIntArgs.push_back( *I );
            }

            tagStringArgs.push_back( (char*)"attributes" );
            tagStringArgs.push_back( const_cast<RtToken>( resourceHandles.back().c_str() ) );

            tagStringArgs.push_back( (char*)"shading" );
        }
    }

    FnAttribute::GroupAttribute geometryGroupAttr = sgIterator.getAttribute("geometry");
    if (!geometryGroupAttr.isValid())
    {
        Report_Error("SubdSurface missing geometry group.", sgIterator);
        return 0x0;
    }

    // add "hole" tag
    FnAttribute::IntAttribute holeAttr = geometryGroupAttr.getChildByName("holePolyIndices");
    if (holeAttr.isValid())
    {
        tags.push_back((char*)"hole");
        tagArgCounts.push_back(holeAttr.getNumberOfTuples()); // num int args
        tagArgCounts.push_back(0); // num float args
        if (isHSub) tagArgCounts.push_back(0); // num string args

        FnAttribute::IntConstVector holeAttrValue = holeAttr.getNearestSample(0);
        for (unsigned int i=0; i<holeAttr.getNumberOfTuples(); ++i)
        {
            tagIntArgs.push_back(holeAttrValue[i]);
        }
    }

    // add "crease" tags
    FnAttribute::IntAttribute creaseLengthsAttr = geometryGroupAttr.getChildByName("creaseLengths");
    FnAttribute::IntAttribute creaseIndicesAttr = geometryGroupAttr.getChildByName("creaseIndices");
    FnAttribute::FloatAttribute creaseSharpnessAttr = geometryGroupAttr.getChildByName("creaseSharpness");
    if (creaseLengthsAttr.isValid() &&
            creaseIndicesAttr.isValid() &&
            creaseSharpnessAttr.isValid())
    {
        unsigned int totalIndices = 0;
        unsigned int totalSharpness = 0;
        FnAttribute::IntConstVector creaseLengthsAttrValue = creaseLengthsAttr.getNearestSample(0);
        for (unsigned int i=0; i<creaseLengthsAttr.getNumberOfTuples(); ++i)
        {
            tags.push_back((char*)"crease");
            tagArgCounts.push_back(creaseLengthsAttrValue[i]); // num int args
            tagArgCounts.push_back(1); // num float args
            if (isHSub) tagArgCounts.push_back(0); // num string args

            totalIndices += creaseLengthsAttrValue[i];
            totalSharpness += 1;
        }

        if (totalIndices != creaseIndicesAttr.getNumberOfTuples() ||
                totalSharpness != creaseSharpnessAttr.getNumberOfTuples())
        {
            Report_Error("Subdiv detected mismatched crease array lengths.", sgIterator);
            return 0x0;
        }

        FnAttribute::IntConstVector creaseIndicesAttrValue = creaseIndicesAttr.getNearestSample(0);
        for (unsigned int i=0; i<creaseIndicesAttr.getNumberOfTuples(); ++i)
        {
            tagIntArgs.push_back(creaseIndicesAttrValue[i]);
        }

        FnAttribute::FloatConstVector creaseSharpnessAttrValue = creaseSharpnessAttr.getNearestSample(0);
        for (unsigned int i=0; i < creaseSharpnessAttr.getNumberOfTuples(); ++i)
        {
            tagFloatArgs.push_back(creaseSharpnessAttrValue[i]);
        }
    }

    // add "corner" tags
    FnAttribute::IntAttribute cornerIndicesAttr = geometryGroupAttr.getChildByName("cornerIndices");
    FnAttribute::FloatAttribute cornerSharpnessAttr = geometryGroupAttr.getChildByName("cornerSharpness");
    if (cornerIndicesAttr.isValid() && cornerSharpnessAttr.isValid())
    {
        if (cornerIndicesAttr.getNumberOfTuples() != cornerSharpnessAttr.getNumberOfTuples())
        {
            Report_Error("Subdiv detected mismatched corner array lengths.", sgIterator);
            return 0x0;
        }

        tags.push_back((char*)"corner");
        tagArgCounts.push_back(cornerIndicesAttr.getNumberOfTuples()); // num int args
        tagArgCounts.push_back(cornerSharpnessAttr.getNumberOfTuples()); // num float args
        if (isHSub) tagArgCounts.push_back(0); // num string args

        FnAttribute::IntConstVector cornerIndicesAttrValue = cornerIndicesAttr.getNearestSample(0);
        FnAttribute::FloatConstVector cornerSharpnessAttrValue = cornerSharpnessAttr.getNearestSample(0);
        for (unsigned int i=0; i < cornerIndicesAttr.getNumberOfTuples(); ++i)
        {
            tagIntArgs.push_back(cornerIndicesAttrValue[i]);
            tagFloatArgs.push_back(cornerSharpnessAttrValue[i]);
        }
    }


    // add "facevaryinginterpolateboundary" tag
    FnAttribute::IntAttribute facevaryingInterpBoundaryAttr = geometryGroupAttr.getChildByName("facevaryinginterpolateboundary");
    if (facevaryingInterpBoundaryAttr.isValid() && facevaryingInterpBoundaryAttr.getNumberOfTuples() == 1)
    {
        tags.push_back((char*)"facevaryinginterpolateboundary");
        tagArgCounts.push_back(1); // num int args
        tagArgCounts.push_back(0); // num float args
        if (isHSub) tagArgCounts.push_back(0); // num string args

        tagIntArgs.push_back( facevaryingInterpBoundaryAttr.getValue() );
    }

    // add "facevaryingpropagatecorners" tag
    FnAttribute::IntAttribute facevaryingPropagateCornersAttr = geometryGroupAttr.getChildByName("facevaryingpropagatecorners");
    if (facevaryingPropagateCornersAttr.isValid()
            && facevaryingPropagateCornersAttr.getNumberOfTuples() == 1)
    {
        tags.push_back((char*)"facevaryingpropagatecorners");
        tagArgCounts.push_back(1); // num int args
        tagArgCounts.push_back(0); // num float args
        if (isHSub) tagArgCounts.push_back(0); // num string args

        tagIntArgs.push_back( facevaryingPropagateCornersAttr.getValue() );
    }


    // add "interpolateboundary" tag
    FnAttribute::IntAttribute interpBoundaryAttr = geometryGroupAttr.getChildByName("interpolateBoundary");
    if (interpBoundaryAttr.isValid() &&
            interpBoundaryAttr.getNumberOfTuples() == 1
            && interpBoundaryAttr.getValue()!=0)
    {
        tags.push_back((char*)"interpolateboundary");
        tagArgCounts.push_back(0); // num int args
        tagArgCounts.push_back(0); // num float args
        if (isHSub) tagArgCounts.push_back(0); // num string args
    }


    AttrList_Converter converter(attrList);
    std::vector<float>& sampleTimes = converter.getSampleTimes();
    if (sampleTimes.size() > 1)
    {
        RiMotionBeginV(sampleTimes.size(), &sampleTimes[0]);
    }

    for (std::vector<float>::const_iterator it = sampleTimes.begin(); it != sampleTimes.end(); ++it)
    {
            if ( isHSub )
            {
                RiHierarchicalSubdivisionMeshV(const_cast<RtToken>(scheme.c_str()),
                        num_polys,
                        &num_vertices[0], &vertices[0],
                        tags.size(),
                        tags.size()?(&tags[0]):(0),
                        tagArgCounts.size()?(&tagArgCounts[0]):(0),
                        tagIntArgs.size()?(&tagIntArgs[0]):(0),
                        tagFloatArgs.size()?(&tagFloatArgs[0]):(0),
                        tagStringArgs.size()?(&tagStringArgs[0]):(0),
                        converter.getSize(*it), converter.getDeclarationTokens(*it), converter.getParameters(*it));
            }
            else
            {
                RiSubdivisionMeshV(const_cast<RtToken>(scheme.c_str()),
                        num_polys,
                        &num_vertices[0], &vertices[0],
                        tags.size(),
                        tags.size()?(&tags[0]):(0),
                        tagArgCounts.size()?(&tagArgCounts[0]):(0),
                        tagIntArgs.size()?(&tagIntArgs[0]):(0),
                        tagFloatArgs.size()?(&tagFloatArgs[0]):(0),
                        converter.getSize(*it), converter.getDeclarationTokens(*it), converter.getParameters(*it));
            }
    }

    if (sampleTimes.size() > 1)
    {
        RiMotionEnd();
    }
    if (isHSub)
    {
        RiResourceEnd();
    }
    return 0x0;
}

void* PRManSubdmeshScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator, void* optionalInput)
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

    void* returnValue = processSubdmesh(sgIterator, state->sharedState);

    RiAttributeEnd();
    return returnValue;
}


PRManSubdmeshScenegraphLocationDelegate::~PRManSubdmeshScenegraphLocationDelegate()
{

}

void PRManSubdmeshScenegraphLocationDelegate::flush()
{

}

PRManSubdmeshScenegraphLocationDelegate* PRManSubdmeshScenegraphLocationDelegate::create()
{
    return new PRManSubdmeshScenegraphLocationDelegate();
}

std::string PRManSubdmeshScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("prman");
}

void PRManSubdmeshScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("subdmesh"));
}
