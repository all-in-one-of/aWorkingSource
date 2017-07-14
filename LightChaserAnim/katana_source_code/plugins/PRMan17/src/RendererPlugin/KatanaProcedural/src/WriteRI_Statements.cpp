// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <PRManProcedural.h>
#include <pystring/pystring.h>

#include <ErrorReporting.h>
#include <WriteRI_Statements.h>
#include <WriteRI_Shaders.h>
#include <FnAsset/FnDefaultAssetPlugin.h>

// Using pthread library here to keep boost library dependencies to a minimum.
#include <pthread.h>

using namespace FnKat;

namespace PRManProcedural
{
    namespace
    {
        /* Global dictionary for storing attribute name-to-prman-type conversion */
        std::map<std::string, std::string> PRManTypeMap;

        pthread_once_t PRManTypeMapInitalizedFlag;

        void RegisterPRManTypes();
        std::string LookupPRManType(const std::string &lookupName);

        //this is to ease the 11.5->13.5 redefinition of visibility.transmission
        //and be removed when all kstds using the old definition are gone. (i.e. never)
        Attribute GetProcessedValueAttr( const std::string & lookupName, Attribute valueAttr)
        {
            if (lookupName == "attributes.visibility.transmission")
            {
                FnAttribute::StringAttribute stringAttr = valueAttr;
                if (stringAttr.isValid())
                {
                    const std::string value = stringAttr.getValue();
                    if (value == "No" || value == "transparent")
                    {
                        return FnAttribute::IntAttribute(0);
                    }
                    else
                    {
                        return FnAttribute::IntAttribute(1);
                    }
                }
            }
            else if(lookupName == "attributes.volume.refinementstrategies" )
            {
                FnAttribute::StringAttribute stringAttr = valueAttr;
                if (stringAttr.isValid())
                {
                    const std::string value = stringAttr.getValue();

                    std::vector<std::string> strategies;
                    pystring::split(value, strategies, ",");
                    int noStrategies = strategies.size();

                    if( noStrategies > 0 )
                    {
                        return FnAttribute::StringAttribute( (const char **) strategies.data(), noStrategies, noStrategies );
                    }
                }
            }

            return valueAttr;
        }


        void RegisterPRManTypes()
        {
            PRManTypeMap["attributes.cull.backfacing"] = "constant int";
            PRManTypeMap["attributes.cull.hidden"] = "constant int";

            PRManTypeMap["attributes.derivatives.centered"] = "constant int";
            PRManTypeMap["attributes.derivatives.extrapolate"] = "constant int";

            PRManTypeMap["attributes.dice.binary"] = "constant int";
            PRManTypeMap["attributes.dice.hair"] = "constant int";
            PRManTypeMap["attributes.dice.rasterorient"] = "constant int";
            PRManTypeMap["attributes.dice.strategy"] = "constant string";
            PRManTypeMap["attributes.dice.referencecamera"] = "constant string";
            PRManTypeMap["attributes.dice.preservecv"] = "constant int";
            PRManTypeMap["attributes.dice.roundcurve"] = "constant int";
            PRManTypeMap["attributes.dice.offscreenstrategy"] = "constant string";
            PRManTypeMap["attributes.dice.minlength"] = "constant float";
            PRManTypeMap["attributes.dice.minlengthspace"] = "constant string";

            PRManTypeMap["attributes.displacementbound.sphere"] = "constant float";
            PRManTypeMap["attributes.displacementbound.coordinatesystem"] = "constant string";
            PRManTypeMap["attributes.irradiance.handle"] = "uniform string";
            PRManTypeMap["attributes.irradiance.filemode"] = "uniform string";
            PRManTypeMap["attributes.irradiance.maxerror"] = "uniform float";
            PRManTypeMap["attributes.irradiance.maxpixeldist"] = "uniform float";
            PRManTypeMap["attributes.limits.eyesplits"] = "constant int";
            PRManTypeMap["attributes.photon.estimator"] = "uniform int";
            PRManTypeMap["attributes.photon.causticmap"] = "uniform string";
            PRManTypeMap["attributes.photon.globalmap"] = "uniform string";
            PRManTypeMap["attributes.photon.shadingmodel"] = "uniform string";
            PRManTypeMap["attributes.photon.maxdiffusedepth"] = "uniform int";
            PRManTypeMap["attributes.photon.maxspeculardepth"] = "uniform int";
            PRManTypeMap["attributes.photon.minstoredepth"] = "uniform int";

            PRManTypeMap["attributes.sides.backfacetolerance"] = "constant float";
            PRManTypeMap["attributes.trace.maxdiffusedepth"] = "uniform int";
            PRManTypeMap["attributes.trace.maxspeculardepth"] = "uniform int";
            PRManTypeMap["attributes.trace.displacements"] = "uniform int";
            PRManTypeMap["attributes.trace.bias"] = "uniform float";
            PRManTypeMap["attributes.trace.samplemotion"] = "uniform int";
            PRManTypeMap["attributes.trace.importancethreshold"] = "uniform float";
            PRManTypeMap["attributes.trace.decimationrate"] = "uniform int";

            PRManTypeMap["attributes.user.visibility"] = "constant int";

            // 'trace' deprecated - kept for backward compatibility
            PRManTypeMap["attributes.visibility.trace"] = "uniform int";

            PRManTypeMap["attributes.visibility.camera"] = "uniform int";
            PRManTypeMap["attributes.visibility.diffuse"] = "uniform int";
            PRManTypeMap["attributes.visibility.photon"] = "uniform int";
            PRManTypeMap["attributes.visibility.specular"] = "uniform int";
            PRManTypeMap["attributes.visibility.transmission"] = "uniform int";
            PRManTypeMap["attributes.visibility.midpoint"] = "uniform int";

            PRManTypeMap["attributes.volume.depthrelativeshadingrate"] = "uniform float";
            PRManTypeMap["attributes.volume.depthresetrelativeshadingrate"] = "uniform float";
            PRManTypeMap["attributes.volume.depthinterpolation"] = "uniform string";
            PRManTypeMap["attributes.volume.deptherror"] = "uniform float";
            PRManTypeMap["attributes.volume.refinementstrategies"] = "uniform string[X]";

            PRManTypeMap["attributes.grouping.membership"] = "uniform string";
            // 'tracesubset' deprecated - kept for backward compatibility
            PRManTypeMap["attributes.grouping.tracesubset"] = "uniform string";

            PRManTypeMap["attributes.shade.strategy"] = "uniform string";
            PRManTypeMap["attributes.shade.volumeintersectionstrategy"] = "uniform string";
            PRManTypeMap["attributes.shade.volumeintersectionpriority"] = "uniform float";
            PRManTypeMap["attributes.shade.diffusehitmode"] = "uniform string";
            PRManTypeMap["attributes.shade.specularhitmode"] = "uniform string";
            PRManTypeMap["attributes.shade.transmissionhitmode"] = "uniform string";
            PRManTypeMap["attributes.shade.camerahitmode"] = "uniform string";

            PRManTypeMap["attributes.shade.shadingrate"] = "uniform float[2]";
            PRManTypeMap["attributes.shade.relativeshadingrate"] = "uniform float";
            PRManTypeMap["attributes.shade.resetrelativeshadingrate"] = "uniform float";

            PRManTypeMap["attributes.shade.diffusehitcache"] = "uniform string";
            PRManTypeMap["attributes.shade.transmissionhitcache"] = "uniform string";
            PRManTypeMap["attributes.shade.diffusehitcolorchannel"] = "uniform string";
            PRManTypeMap["attributes.shade.diffusehitopacitychannel"] = "uniform string";
            PRManTypeMap["attributes.shade.frequency"] = "uniform string";

            PRManTypeMap["attributes.stitch.enable"] = "int";
            PRManTypeMap["attributes.stitch.traceenable"] = "int";
            PRManTypeMap["attributes.stitch.newgroup"] = "int";

            //PRManTypeMap["attributes.stochastic.sigma"] = "uniform float";
            PRManTypeMap["attributes.stochastic.sigma"] = "uniform int";
            PRManTypeMap["attributes.stochastic.pointfalloff"] = "uniform int";

            PRManTypeMap["attributes.shadegroups.attributecombining"] = "constant string";
            PRManTypeMap["attributes.shadegroups.objectspacecombining"] = "constant string";

            PRManTypeMap["attributes.trimcurve.sense"] = "constant string";
        }

        std::string LookupPRManType(const std::string &lookupName)
        {
            // Initialize the type map.
            pthread_once(&PRManTypeMapInitalizedFlag, RegisterPRManTypes);

            std::map<std::string, std::string>::const_iterator iter = PRManTypeMap.find(lookupName);
            if (iter != PRManTypeMap.end())
                return iter->second;
            return "";
        }
    }

/////////////////////////////////////////////////////////////////////////////
//
// WriteRI_Object_AttributeStatements
//
// Traverse through children of group attr and output RiAttribute calls
void WriteRI_Object_AttributeStatements(const std::string & errorIdentifier, FnAttribute::GroupAttribute stmtAttr, PRManPluginState* sharedState)
{

    if (!stmtAttr.isValid())
    {
        Report_Error("WriteRI_Object_AttributeStatements called with null attr.", errorIdentifier);
        return;
    }

    int numStmtAttrChildren = stmtAttr.getNumberOfChildren();

    // each child is a group containg the params for the RiAttribute call.  the RiAttribute
    //   "name" arg is the name of the child.
    for (int i = 0; i < numStmtAttrChildren; i++)
    {
        FnAttribute::GroupAttribute groupAttr = stmtAttr.getChildByIndex(i);
        std::string groupAttrName = stmtAttr.getChildName(i);

        if (!groupAttr.isValid())
        {
            Report_Debug(std::string("Found a non group attribute in the RiAttribute group named '") + groupAttrName + "'", errorIdentifier, sharedState);
            continue;
        }

        std::string groupName = "attributes.";
        groupName += groupAttrName;

        int numGroupAttrChildren = groupAttr.getNumberOfChildren();
        AttrList attrList;
        // Each child is a param in the RiAttribute call
        for (int j = 0; j < numGroupAttrChildren; j++)
        {
            Attribute valueAttr = groupAttr.getChildByIndex(j);
            std::string valueAttrName = groupAttr.getChildName(j);

            std::string lookupName = groupName + "." + valueAttrName;
            std::string decStr = LookupPRManType(lookupName);

            if (decStr.empty())
            {
                FnAttribute::StringAttribute stringAttr = valueAttr;
                if (stringAttr.isValid() && stringAttr.getNearestSample(0).size() == 1)
                {
                    decStr = "uniform string";
                }
                else
                {
                    FnAttribute::FloatAttribute floatAttr = valueAttr;
                    if (floatAttr.isValid() && floatAttr.getNearestSample(0).size() == 1)
                    {
                        decStr = "constant float";
                    }
                    else
                    {
                        FnAttribute::IntAttribute intAttr = valueAttr;
                        if (intAttr.isValid() && intAttr.getNearestSample(0).size() == 1)
                        {
                            decStr = "constant int";
                        }
                        else
                        {
                            FnAttribute::GroupAttribute groupAttr = valueAttr;
                            if (groupAttr.isValid())
                            {
                                FnAttribute::StringAttribute typeAttr = groupAttr.getChildByName( "type" );
                                valueAttr = groupAttr.getChildByName( "value" );

                                if (!(typeAttr.isValid() && valueAttr.isValid()))
                                {
                                    Report_Error("Unrecognized arbitrary parameter '" + valueAttrName + "'.", errorIdentifier);
                                    continue;
                                }

                                decStr = typeAttr.getValue();
                            }
                            else
                            {
                                Report_Error("Unknown prman type for '" + lookupName + "' while outputting prmanStatement attr group.", errorIdentifier);
                                continue;
                            }
                        }
                    }
                }
            }

            // Allow attributes to overwrite their native behaviour
            valueAttr = GetProcessedValueAttr( lookupName, valueAttr );
            // Check for dynamic arrays
            DataAttribute valueData = valueAttr;
            if( valueData.isValid() )
            {
                char noValues[4];
                sprintf( noValues, "%i", (int) valueData.getNumberOfValues() );
                if( pystring::endswith( decStr, "[X]" ) )
                    decStr = pystring::slice( decStr, 0, -3 ) + "[" + noValues + "]";
            }

            attrList.push_back(AttrListEntry(decStr + " " + valueAttrName, valueAttrName,
                    valueAttr, false));
        }

        WriteRI_Object_ConvertAttrListToStatements(attrList, groupAttrName);
    }
}

/////////////////////////////////////////////////////////////////////////////
//
// WriteRI_Object_ConvertAttrListToStatements
//
// Convert an attribute list to RiAttribute statements
void WriteRI_Object_ConvertAttrListToStatements(const AttrList& attrList, const std::string& groupAttrName)
{
    // Output "user" attributes with separate RiAttribute calls,
    // for readability, but keep others together for correctness
    // (e.g. "displacementbounds" values must be specified together)
    AttrList_Converter converter(attrList);
    if (groupAttrName == "user")
    {
        RtToken* tokens = converter.getDeclarationTokens(0);
        RtPointer* parameters = converter.getParameters(0);
        for (size_t i = 0; i < converter.getSize(0); ++i)
        {
            RiAttribute(const_cast<char*>(groupAttrName.c_str()),
                        tokens[i], parameters[i], RI_NULL);
        }
    }
    else
    {
        RiAttributeV(const_cast<char*>(groupAttrName.c_str()),
                     converter.getSize(0),
                     converter.getDeclarationTokens(0),
                     converter.getParameters(0));
    }
}


/////////////////////////////////////////////////////////////////////////////
//
// WriteRI_Object_Statements
//
// Write out statements stored in the 'prmanStatements' attr group
void WriteRI_Object_Statements(FnScenegraphIterator sgIterator, PRManPluginState* sharedState)
{
    Report_Debug("WriteRI_Object_Statements called", sharedState, sgIterator);

    FnAttribute::GroupAttribute trimCurvesAttr = sgIterator.getAttribute( "geometry.trimCurves");

    if (trimCurvesAttr.isValid())
    {
        FnAttribute::DoubleAttribute knotAttr = trimCurvesAttr.getChildByName("trim_knot");
        FnAttribute::DoubleAttribute minAttr = trimCurvesAttr.getChildByName("trim_min");
        FnAttribute::DoubleAttribute maxAttr = trimCurvesAttr.getChildByName("trim_max");
        FnAttribute::DoubleAttribute uAttr = trimCurvesAttr.getChildByName("trim_u");
        FnAttribute::DoubleAttribute vAttr = trimCurvesAttr.getChildByName("trim_v");
        FnAttribute::DoubleAttribute wAttr = trimCurvesAttr.getChildByName("trim_w");

        FnAttribute::IntAttribute nAttr = trimCurvesAttr.getChildByName("trim_n" );
        FnAttribute::IntAttribute ncurvesAttr = trimCurvesAttr.getChildByName("trim_ncurves" );
        FnAttribute::IntAttribute orderAttr = trimCurvesAttr.getChildByName("trim_order" );

        if ( knotAttr.isValid() && maxAttr.isValid() && minAttr.isValid() &&
                uAttr.isValid() && vAttr.isValid() && wAttr.isValid() &&
                nAttr.isValid() && ncurvesAttr.isValid() && orderAttr.isValid() )
        {
            int numloops = (int) ncurvesAttr.getNearestSample(0).size();

            RtInt * ncurves = const_cast<RtInt *>(ncurvesAttr.getNearestSample(0).data());

            RtInt * order = const_cast<RtInt *>( orderAttr.getNearestSample(0).data());

            FnAttribute::DoubleConstVector knotAttrVec = knotAttr.getNearestSample(0);
            std::vector< float > knotData(knotAttrVec.begin(), knotAttrVec.end());
            RtFloat * knot = &knotData[0];

            FnAttribute::DoubleConstVector minAttrVec = minAttr.getNearestSample(0);
            std::vector< float > minData( minAttrVec.begin(), minAttrVec.end());
            RtFloat * min = &minData[0];

            FnAttribute::DoubleConstVector maxAttrVec = maxAttr.getNearestSample(0);
            std::vector< float > maxData(maxAttrVec.begin(), maxAttrVec.end());
            RtFloat * max = &maxData[0];

            RtInt * n = const_cast<RtInt *>(nAttr.getNearestSample(0).data());

            FnAttribute::DoubleConstVector uAttrVec = uAttr.getNearestSample(0);
            std::vector< float > uData(uAttrVec.begin(), uAttrVec.end());
            RtFloat * u = &uData[0];

            FnAttribute::DoubleConstVector vAttrVec = vAttr.getNearestSample(0);
            std::vector< float > vData(vAttrVec.begin(), vAttrVec.end());
            RtFloat * v = &vData[0];

            FnAttribute::DoubleConstVector wAttrVec = wAttr.getNearestSample(0);
            std::vector< float > wData(wAttrVec.begin(), wAttrVec.end());
            RtFloat * w = &wData[0];

            RiTrimCurve( numloops, ncurves, order, knot, min, max, n, u, v, w );
        }
    }

    //special case handling of textures as attributes useful for hair
    FnAttribute::IntAttribute applyTexturesAsRiAttributes =
            sgIterator.getAttribute("textureAssign.applyResultsAsRiAttributes");

    if (applyTexturesAsRiAttributes.isValid() && applyTexturesAsRiAttributes.getValue())
    {
        AddAttributesFromTextureAttrs( sgIterator.getAttribute("textures", true) );
    }

    WriteRI_Object_PRManStatements(sgIterator.getFullName(), sgIterator.getAttribute("prmanStatements"), sharedState);

}

void WriteRI_Object_PRManStatements(const std::string & errorIdentifier, FnAttribute::GroupAttribute prmanStatementsAttr, PRManPluginState* sharedState)
{
    if (!prmanStatementsAttr.isValid()) return;

    // RiAttribute
    FnAttribute::GroupAttribute attributesAttr = prmanStatementsAttr.getChildByName("attributes");
    if (attributesAttr.isValid())
    {
        WriteRI_Object_AttributeStatements(errorIdentifier, attributesAttr, sharedState);
    }

    // RiGeometricApproximation
    FnAttribute::GroupAttribute geoApproxAttr = prmanStatementsAttr.getChildByName("geometricApproximation");
    if (geoApproxAttr.isValid())
    {
        FnAttribute::FloatAttribute motionFactorAttr = geoApproxAttr.getChildByName("motionfactor");
        if (motionFactorAttr.isValid())
        {
            RiGeometricApproximation((char*)"motionfactor", motionFactorAttr.getValue());
        }

        FnAttribute::FloatAttribute focusFactorAttr = geoApproxAttr.getChildByName("focusfactor");
        if (focusFactorAttr.isValid())
        {
            RiGeometricApproximation((char*)"focusfactor", focusFactorAttr.getValue());
        }

        FnAttribute::FloatAttribute flatnessAttr = geoApproxAttr.getChildByName("flatness");
        if (flatnessAttr.isValid())
        {
            RiGeometricApproximation((char*)"flatness", flatnessAttr.getValue());
        }
    }

    FnAttribute::GroupAttribute basisAttr = prmanStatementsAttr.getChildByName("basis");
    if (basisAttr.isValid())
    {
        RtBasis *uBasis = &RiBSplineBasis;
        RtInt uStep = RI_BSPLINESTEP;
        RtBasis *vBasis = &RiBSplineBasis;
        RtInt vStep = RI_BSPLINESTEP;

        FnAttribute::StringAttribute uAttr = basisAttr.getChildByName("u");
        if (uAttr.isValid())
        {
            std::string value = uAttr.getValue();
            if (value == "bezier") {uBasis = &RiBezierBasis; uStep = RI_BEZIERSTEP;}
            else if (value == "b-spline") {uBasis = &RiBSplineBasis; uStep = RI_BSPLINESTEP;}
            else if (value == "catmull-rom") {uBasis = &RiCatmullRomBasis;
                                              uStep = RI_CATMULLROMSTEP;}
            else if (value == "hermite") {uBasis = &RiHermiteBasis; uStep = RI_HERMITESTEP;}
            else if (value == "power") {uBasis = &RiPowerBasis; uStep = RI_POWERSTEP;}
        }

        FnAttribute::StringAttribute vAttr = basisAttr.getChildByName("v");
        if (vAttr.isValid())
        {
            std::string value = vAttr.getValue();
            if (value == "bezier"){vBasis = &RiBezierBasis; vStep = RI_BEZIERSTEP;}
            else if (value == "b-spline") {vBasis = &RiBSplineBasis; vStep = RI_BSPLINESTEP;}
            else if (value == "catmull-rom") {vBasis = &RiCatmullRomBasis;
                                              vStep = RI_CATMULLROMSTEP;}
            else if (value == "hermite") {vBasis = &RiHermiteBasis; vStep = RI_HERMITESTEP;}
            else if (value == "power") {vBasis = &RiPowerBasis; vStep = RI_POWERSTEP;}
        }

        RiBasis(*uBasis, uStep, *vBasis, vStep);
    }

    // RiOrientation and RiReverseOrientation
    FnAttribute::StringAttribute orientationAttr = prmanStatementsAttr.getChildByName("orientation");
    if (orientationAttr.isValid())
    {
        std::string orientation = orientationAttr.getValue();
        if (orientation == "reverse")
        {
            RiReverseOrientation();
        }
        else
        {
            RiOrientation(const_cast<char*>(orientation.c_str()));
        }
    }

    // RiShadingRate
    FnAttribute::FloatAttribute shadingRateAttr = prmanStatementsAttr.getChildByName("shadingRate");
    if (shadingRateAttr.isValid())
    {
        RiShadingRate(shadingRateAttr.getValue());
    }

    // RiSides
    FnAttribute::IntAttribute sidesAttr = prmanStatementsAttr.getChildByName("sides");
    if (sidesAttr.isValid())
    {
        RiSides(sidesAttr.getValue());
    }

    // RiShadingInterpolation
    FnAttribute::StringAttribute shadingInterpAttr = prmanStatementsAttr.getChildByName("shadingInterpolation");
    if (shadingInterpAttr.isValid())
    {
        RiShadingInterpolation(const_cast<char*>(shadingInterpAttr.getValue().c_str()));
    }

    // ScopedCoordinateSystem
    FnAttribute::StringAttribute scopedCoordinateSystemAttr = prmanStatementsAttr.getChildByName("scopedCoordinateSystem");
    if (scopedCoordinateSystemAttr.isValid())
    {
        const std::string& value = scopedCoordinateSystemAttr.getValue();
        if (!value.empty())
        {
            RiScopedCoordinateSystem(const_cast<char*>(value.c_str()));
        }
    }

    // RiMatte
    FnAttribute::StringAttribute matteAttr = prmanStatementsAttr.getChildByName("matte");
    if ( matteAttr.isValid() )
    {
        if (matteAttr.getValue() == "Yes")
            RiMatte(RI_TRUE);
        else
            RiMatte(RI_FALSE);
    }


    // RiColor
    FnAttribute::FloatAttribute colorAttr = prmanStatementsAttr.getChildByName("color");
    if (colorAttr.isValid())
    {
        FnAttribute::FloatConstVector data = colorAttr.getNearestSample(0);
        if (data.size() == 3)
        {
            RtColor color = { data[0], data[1], data[2] };
            RiColor(color);
        }
    }

    // RiOpacity
    FnAttribute::FloatAttribute opacityAttr = prmanStatementsAttr.getChildByName("opacity");
    if (opacityAttr.isValid())
    {
        FnAttribute::FloatConstVector data = opacityAttr.getNearestSample(0);
        if (data.size() == 3)
        {
            RtColor color = { data[0], data[1], data[2] };
            RiOpacity( color );
        }
    }

    // RiArchive calls; inline RIB code.
    FnAttribute::StringAttribute ribIncludeAttr = prmanStatementsAttr.getChildByName("ribInclude");
    if (ribIncludeAttr.isValid())
    {
        std::string data = ribIncludeAttr.getValue();
        RiArchiveRecord((char*)"comment", (char*)"begin user-defined rib include" );

        // Resolve asset ID
        if (DefaultAssetPlugin::isAssetId(data))
        {
            data = DefaultAssetPlugin::resolvePath(data, 0);
        }

        if (pystring::endswith(data, ".rib" ))
        {
            RiReadArchive (const_cast<RtToken>(data.c_str()) , NULL, RI_NULL);
        }
        else
        {
            RtArchiveHandle arHandle = RiArchiveBegin((char*)"inline", RI_NULL);
            RiArchiveRecord((char*)"verbatim", (char*)"%s\n", ribIncludeAttr.getValue().c_str());
            RiArchiveEnd();
            RiReadArchive( arHandle, NULL, RI_NULL);
        }
        RiArchiveRecord((char*)"comment", (char*)"end user-defined rib include" );
    }

    FnAttribute::GroupAttribute inlineArchivesAttr = prmanStatementsAttr.getChildByName("inlineArchives");

    if (inlineArchivesAttr.isValid())
    {
        int numChildren = inlineArchivesAttr.getNumberOfChildren();

        for (int i = 0; i < numChildren; ++i)
        {
            FnAttribute::GroupAttribute groupAttr = inlineArchivesAttr.getChildByIndex(i);
            if (!groupAttr.isValid())
            {
                continue;
            }

            std::string archiveName = inlineArchivesAttr.getChildName(i);

            std::string archiveIdentifier = errorIdentifier + ":archive="+ archiveName;

            RiArchiveBegin((char*)archiveName.c_str(), RI_NULL);

            WriteRI_Object_PRManStatements(archiveIdentifier,
                    groupAttr.getChildByName("prmanStatements"), sharedState);

            FnAttribute::GroupAttribute material_attr = groupAttr.getChildByName("material");
            FnAttribute::GroupAttribute info_attr = groupAttr.getChildByName("info");
            WriteRI_Object_Material(archiveIdentifier, material_attr, true /*flipZ?*/, info_attr, sharedState);

            RiArchiveEnd();
        }

    }
}

}
