// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include <buildCommons.h>
#include <ArnoldCurvesScenegraphLocationDelegate.h>
#include <objectSettings.h>
#include <buildCamera.h>
#include <ArnoldPluginState.h>

using namespace FnKat;


void setOrientations( const GroupAttribute& pointAttr,
                      FnScenegraphIterator& sgIterator,
                      ArnoldPluginState* sharedState, AtNode* curvenode )
{
    FnAttribute::FloatAttribute orientAttr;
    orientAttr = pointAttr.getChildByName("orientation");

    if (orientAttr.isValid())
    {
        std::set<float> orientSampleTimes;
        std::vector<float> orientations;

        getSampleTimes(sgIterator.getRoot(), orientAttr, &orientSampleTimes, sharedState);

        for (std::set<float>::iterator it = orientSampleTimes.begin(); it != orientSampleTimes.end(); ++it)
        {
            FloatConstVector sampleData = orientAttr.getNearestSample(*it);
            orientations.insert(orientations.end(), sampleData.begin(),sampleData.end());
        }

        AiNodeSetArray(curvenode, "orientations",
                       AiArrayConvert(orientAttr.getNumberOfValues(),
                       orientSampleTimes.size(), AI_TYPE_FLOAT,
                       (void*) (&(orientations[0]))));
    }
}


AtNode* buildCurves(FnScenegraphIterator sgIterator,
                    std::vector<AtNode*>* childNodes,
                    ArnoldPluginState* sharedState)
{
    if (!isVisible(sgIterator)) return 0;

    std::string name = sgIterator.getFullName();
    
    if (AiNodeLookUpByName(name.c_str()))
    {
        AiMsgError("[kat] '%s' is already in the scene!", name.c_str());
        return 0;
    }
    
    GroupAttribute geometryAttr = sgIterator.getAttribute("geometry");
    if (!geometryAttr.isValid())
    {
        AiMsgError("[kat] Curves '%s' is missing the geometry attribute.", name.c_str());
        return 0;
    }
    
    int numCurves = 1;
    
    GroupAttribute pointAttr;
    FnAttribute::FloatAttribute pAttr;
    FnAttribute::IntAttribute numVertAttr;
    
    bool multiCurveSpec = false;
    
    FnAttribute::IntAttribute ncurvesAttr  = geometryAttr.getChildByName("curves_ncurves");
    if (ncurvesAttr.isValid())
    {
        multiCurveSpec = true;
        
        numCurves = ncurvesAttr.getValue();
        
        pAttr = geometryAttr.getChildByName("curves_pointList");
        if(!pAttr.isValid())
        {
            AiMsgError("[kat] Curves '%s' is missing the "
                       "geometry.curves_pointList attribute.", name.c_str());
            return 0;
        }
        
        numVertAttr = geometryAttr.getChildByName("curves_nvertices");
        if (!numVertAttr.isValid())
        {
            AiMsgError("[kat] Curves '%s' is missing the "
                       "geometry.curves_nvertices attribute.", name.c_str());
            return 0;
        }
    }
    else
    {
        pointAttr = geometryAttr.getChildByName("point");
        if (!pointAttr.isValid())
        {
            AiMsgError("[kat] Curves '%s' is missing the geometry.point attribute.", name.c_str());
            return 0;
        }
        
        pAttr = pointAttr.getChildByName("P");
        if (!pAttr.isValid())
        {
            AiMsgError("[kat] Curves '%s' is missing the point.P attribute.", name.c_str());
            return 0;
        }
        
        numVertAttr = geometryAttr.getChildByName("numVertices");
        if (!numVertAttr.isValid())
        {
            AiMsgError("[kat] Curves '%s' is missing the geometry.numVertices attribute.", name.c_str());
            return 0;
        }
    }
    
    IntConstVector numVert = numVertAttr.getNearestSample(0);
    
    //get num samples for P
    std::set<float> pSampleTimes;
    getSampleTimes(sgIterator.getRoot(), pAttr, &pSampleTimes, sharedState);
    
    std::vector<float> cvs;
    //concat all samples into one big .ass array
    for (std::set<float>::iterator it = pSampleTimes.begin(); it != pSampleTimes.end(); ++it)
    {
        FloatConstVector sampleData = pAttr.getNearestSample(*it);
        cvs.insert(cvs.end(), sampleData.begin(), sampleData.end());
    }

    AtNode* curvenode = AiNode("curves");
    if (!curvenode)
    {
        AiMsgError("[kat] unable to create curves primitive for %s", name.c_str());
        return NULL;
    }    

    AiNodeSetStr  (curvenode, "name",  name.c_str());
    AiNodeSetArray(curvenode, "points", AiArrayConvert(
            pAttr.getNumberOfValues(), pSampleTimes.size(),
            AI_TYPE_FLOAT, (void*)(&(cvs[0]))));
    AiNodeSetArray(curvenode, "num_points", AiArrayConvert(
            numVert.size(), 1, AI_TYPE_INT,
            (void*)&numVert[0]));


    FnAttribute::FloatAttribute widthStartAttr = geometryAttr.getChildByName("curveWidthStart");
    FnAttribute::FloatAttribute widthEndAttr = geometryAttr.getChildByName("curveWidthEnd");

    if (widthStartAttr.isValid() || widthEndAttr.isValid())
    {
        if (!widthStartAttr.isValid())
        {
            widthStartAttr = widthEndAttr;
        }
        else if (!widthEndAttr.isValid())
        {
            widthEndAttr = widthStartAttr;
        }

        std::vector<float> radius;
        int counter = 0;
        for(std::set<float>::iterator it = pSampleTimes.begin(); it != pSampleTimes.end(); ++it)
        {
            //renderman's curve width is an half size of arnold curve
            //so we adjust size here.
            float cws = widthStartAttr.getNearestSample(*it)[0]*.5f;
            float cwe = widthEndAttr.getNearestSample(*it)[0]*.5f;

            for(int i = 0; i < (int)numVert.size(); i++)
            {
                //This fomular is based on prman's basis manual
                //Since we only support b-spline basis, 
                //step is set to be 1.
                //Fomular for cubic, nonperiodic curves is following
                //nsegi = (nvertices[i]-4)/vstep+1
                //number of verying data is
                //nsegi + 1
                //value for vstep is
                //3, // bezier
                //1, // b-spline
                //1, // catmull-rom
                //2, // hermite
                //4  // power
                int nump = numVert[i] != 0 ? (numVert[i]-4)/1+1 : 0;
                nump += 1;
                double dist = 0;
                std::vector<float> seg;
                int offs = (numVert[i]-nump)/2;
                float lx = cvs[counter+offs*3];
                float ly = cvs[counter+offs*3+1];
                float lz = cvs[counter+offs*3+2];
                for(int j = offs; j < offs+nump; j++){
                    float x = cvs[counter+j*3];
                    float y = cvs[counter+j*3+1];
                    float z = cvs[counter+j*3+2];
                    lx = x-lx; ly = y-ly; lz = z-lz;
                    seg.push_back(sqrtf(lx*lx+ly*ly+lz*lz));
                    dist += seg[seg.size()-1];

                    lx = x; ly = y; lz = z;
                }

                float dw = (cwe - cws);
                float cd = 0.;
                for(int j = 0; j < nump; j++){
                    float w = ((cd+seg[j])/dist) * dw + cws;
                    cd += seg[j];
                    radius.push_back(w);
                }
            }
        }
        AiNodeSetArray(curvenode, "radius", AiArrayConvert(radius.size()/pSampleTimes.size(),
                    pSampleTimes.size(), AI_TYPE_FLOAT, &radius[0]));
    }
    else
    {
        // Try to use geometry.point.width or geometry.width attribute, if size is
        // recognized as useful.
        bool radiusSet = false;
        FnAttribute::FloatAttribute widthAttr;
        
        if (multiCurveSpec)
        {
            widthAttr = geometryAttr.getChildByName("curves_width");
        }
        else
        {
            if (pointAttr.isValid())
            {
                widthAttr = pointAttr.getChildByName("width");
            }
        }
        
        
        if (!widthAttr.isValid())
        {
            widthAttr = geometryAttr.getChildByName("width");
        }

        if (widthAttr.isValid())
        {
            unsigned int totalRadiusSamples = 0;
            unsigned int totalPointSamples = 0;
            std::vector<int> numRadiusSamples;
            numRadiusSamples.reserve(numVert.size());

            std::vector<float> radius;
            FloatConstVector width = widthAttr.getNearestSample(0);

            for (unsigned int i=0; i<numVert.size(); ++i)
            {
                int nump = numVert[i] != 0 ? (numVert[i]-4)/1+1 : 0;
                nump += 1;
                numRadiusSamples.push_back(nump);
                totalRadiusSamples += nump;
                totalPointSamples += numVert[i];
            }
            
            radius.reserve(totalRadiusSamples);

            // If width attribute is already the correct length, scale and use values directly.
            // (HoudiniGeo_In will convert open polys to b-spline curves correctly,
            //  but NURBS curves need massaging.)
            // Renderman's curve width is an half size of arnold curve
            // so we scale width value by 0.5f.
            if (width.size() == totalRadiusSamples)
            {
                for(unsigned int i=0; i<width.size(); ++i)
                {
                    radius.push_back(width[i]*0.5f);
                }

                AiNodeSetArray(curvenode, "radius", AiArrayConvert(radius.size(), 1,
                            AI_TYPE_FLOAT, &radius[0]));
                radiusSet = true;
            }
            else if (width.size() == totalPointSamples)
            {
                unsigned int widthIndex = 0;
                for(unsigned int i=0; i<numRadiusSamples.size(); ++i)
                {
                    int precull = (numVert[i]-numRadiusSamples[i]) / 2;
                    int postcull = numVert[i]-numRadiusSamples[i] - precull;

                    widthIndex += precull;

                    for (int j=0; j<numRadiusSamples[i] && widthIndex<width.size(); ++j)
                    {
                        radius.push_back(width[widthIndex]*0.5f);
                        ++widthIndex;
                    }

                    widthIndex += postcull;
                }

                AiNodeSetArray(curvenode, "radius", AiArrayConvert(radius.size(), 1,
                            AI_TYPE_FLOAT, &radius[0]));
                radiusSet = true;
            }
        }

        // If geometry.point.width attribute wasn't useful,
        // try constantWidth or (finally) hard-coded width.
        if (!radiusSet)
        {
            FnAttribute::FloatAttribute cwidthAttr = geometryAttr.getChildByName("constantWidth");
            if (cwidthAttr.isValid())
            {
                //renderman's curve width is an half size of arnold curve
                //so we adjeuct size here.
                float radius = cwidthAttr.getValue() * 0.5f;
                AiNodeSetFlt(curvenode, "radius", radius);
            }
            else
            {
                //renderman's curve width is an half size of arnold curve
                //so we adjeuct size here.
                float radius = 1.0f*.5f;
                AiNodeSetFlt(curvenode, "radius", radius);
            }
        }
    }
    
    //legacy support. Check for it first. Don't set default
    FnAttribute::StringAttribute modeAttr = geometryAttr.getChildByName("curveMode");
    if (modeAttr.isValid())
    {
        std::string mode = modeAttr.getValue();
        AiNodeSetStr  (curvenode, "mode", mode.c_str());
    }
    
    if (pSampleTimes.size() > 1)
    {
        std::vector<float> outputSampleTimes;
        outputSampleTimes.reserve(pSampleTimes.size());
        outputSampleTimes.insert(outputSampleTimes.end(), pSampleTimes.begin(), pSampleTimes.end());
        const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(curvenode);
        if (AiNodeEntryLookUpParameter(baseNodeEntry, "deform_time_samples"))
        {
            AiNodeSetArray(curvenode, "deform_time_samples",
                    AiArrayConvert(outputSampleTimes.size(), 1, AI_TYPE_FLOAT,
                            &outputSampleTimes[0]));
        }
        else if (outputSampleTimes.size() > 1 &&
                    (outputSampleTimes.front() != 0 || outputSampleTimes.back() != 1))
        {
            AiMsgWarning(
                std::string("[kat] non 0 to 1 geometry samples found on " +
                    sgIterator.getFullName() +
                            ". This version of arnold does not "
                            "support this and will likely result in "
                            "inaccurate motion blur.").c_str());
        }
    }
    
    //curve-specific settings which live in arnoldStatements
    GroupAttribute arnoldStatementsAttr =
        sgIterator.getAttribute("arnoldStatements", true);

    // The Curve basis (retrieved from arnoldStatementsAttr as "curve_basis").
    // Default to b-spline, which is not Arnold's default, but Katana's
    // default instead.
    std::string basis = "b-spline";

    if (arnoldStatementsAttr.isValid())
    {
        FnAttribute::FloatAttribute floatAttr;
        FnAttribute::IntAttribute intAttr;
        FnAttribute::StringAttribute strAttr;
        
        floatAttr = arnoldStatementsAttr.getChildByName("min_pixel_width");
        if (floatAttr.isValid())
        {
            AiNodeSetFlt(curvenode, "min_pixel_width", floatAttr.getValue());
        }
        
        // The curve mode. If it is "oriented" then the values of the "orientation"
        // attribute will be passed to the curves Arnold node as "orientations"
        strAttr = arnoldStatementsAttr.getChildByName("curve_mode");
        if (strAttr.isValid())
        {
            AiNodeSetStr(curvenode, "mode", strAttr.getValue().c_str());

            if (strAttr.getValue() == "oriented" && pointAttr.isValid())
            {
                // Read the "orientation" attribute
                setOrientations(pointAttr, sgIterator, sharedState, curvenode);
            }

        }

        strAttr = arnoldStatementsAttr.getChildByName("curve_basis");
        if (strAttr.isValid())
        {
            basis = strAttr.getValue();
        }
    }
    
    // Set the basis
    AiNodeSetStr(curvenode, "basis", basis.c_str());

    if (pSampleTimes.size() > 1)
    {
        std::vector<float> outputSampleTimes;
        outputSampleTimes.reserve(pSampleTimes.size());
        outputSampleTimes.insert(outputSampleTimes.end(), pSampleTimes.begin(), pSampleTimes.end());
        const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(curvenode);
        if (AiNodeEntryLookUpParameter(baseNodeEntry, "deform_time_samples"))
        {
            AiNodeSetArray(curvenode, "deform_time_samples",
                    AiArrayConvert(outputSampleTimes.size(), 1, AI_TYPE_FLOAT,
                                   &outputSampleTimes[0]));
        }
        else if (outputSampleTimes.size() > 1 &&
                    (outputSampleTimes.front() != 0 || outputSampleTimes.back() != 1))
        {
            AiMsgWarning(
                std::string("[kat] non 0 to 1 geometry samples found on " +
                            sgIterator.getFullName() +
                            ". This version of arnold does not "
                            "support this and will likely result in "
                            "inaccurate motion blur.").c_str());
        }
    }

    
    //get arbitrary and handle it
    GroupAttribute arbitraryAttr = sgIterator.getAttribute("geometry.arbitrary", true);
    if (arbitraryAttr.isValid()) writeArbitrary(curvenode,
            sgIterator.getFullName(), sgIterator.getType(), geometryAttr, arbitraryAttr);
    
    if(childNodes){
        childNodes->push_back(curvenode);
    }

    return curvenode;
}

void* ArnoldCurvesScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator,
                                                      void* optionalInput)
{
    ArnoldSceneGraphLocationDelegateInput* state = reinterpret_cast<ArnoldSceneGraphLocationDelegateInput*>(optionalInput);
    void* returnValue = buildCurves(sgIterator, state->childNodes, state->sharedState);
    if(returnValue){
        applyObjectSettings(sgIterator, reinterpret_cast<AtNode*>(returnValue), state->childNodes, state->sharedState);
    }
    return returnValue;
}

ArnoldCurvesScenegraphLocationDelegate::~ArnoldCurvesScenegraphLocationDelegate()
{

}

void ArnoldCurvesScenegraphLocationDelegate::flush()
{

}

ArnoldCurvesScenegraphLocationDelegate::ArnoldCurvesScenegraphLocationDelegate()
{

}

ArnoldCurvesScenegraphLocationDelegate* ArnoldCurvesScenegraphLocationDelegate::create()
{
    return new ArnoldCurvesScenegraphLocationDelegate();
}

std::string ArnoldCurvesScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("arnold");
}

void ArnoldCurvesScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("curves"));
}
