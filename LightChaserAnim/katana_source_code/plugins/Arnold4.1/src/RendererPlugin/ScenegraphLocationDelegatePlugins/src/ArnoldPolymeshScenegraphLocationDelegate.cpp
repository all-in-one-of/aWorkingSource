// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include <buildCommons.h>
#include <ArnoldPolymeshScenegraphLocationDelegate.h>
#include <buildCamera.h>
#include <vector>
#include <set>
#include <list>
#include <pystring/pystring.h>

#include <FnRenderOutputUtils/FnRenderOutputUtils.h>
#include <FnAttribute/FnAttribute.h>
#include <objectSettings.h>
#include <buildCamera.h>
#include <ArnoldPluginState.h>

#include <FnGeolibServices/FnArbitraryOutputAttr.h>
namespace FnGeolibServices = FNGEOLIBSERVICES_NAMESPACE;

using namespace FnKat;

//this currently deals with geometry.arbitrary.st specifically
//TODO: generalize to handle all permutations of arbitrary attrs
//TODO: MB
void writeArnoldUVs(AtNode* meshNode, FnScenegraphIterator sgIterator, 
                    GroupAttribute geometryAttr, GroupAttribute stAttr, GroupAttribute vertexAttr,
                    GroupAttribute pointAttr, FnAttribute::IntAttribute vertexListAttr, IntConstVector & verts,
                    std::string &flipV)
{
    //new attrs?
    if (stAttr.isValid())
    {
        //make arbitraryOutputAttr and check if we gto a valid one
        FnGeolibServices::ArbitraryOutputAttr arbitraryAttr("st", stAttr, "polymesh", geometryAttr);
        if (!arbitraryAttr.isValid())
        {
            AiMsgWarning("[kat] invalid arbitrary.st format @ %s (%s)",
                sgIterator.getFullName().c_str(), arbitraryAttr.getError().c_str());
            return;
        }

        //try and cast these to our expected type(s)
        FnAttribute::IntAttribute indexAttr; 
        FnAttribute::FloatAttribute uvAttr;
        
        //need to support both forms welded-or-otherwise to be a good katana citizen
        if (arbitraryAttr.hasIndexedValueAttr())
        {
            indexAttr = arbitraryAttr.getIndexAttr(true);
            uvAttr = arbitraryAttr.getIndexedValueAttr();
        }
        else
        {
            uvAttr = arbitraryAttr.getValueAttr();
            
            //already validated above, go ahead and build the index attr manually
            std::vector<int> indexValues;
            indexValues.resize(verts.size());
            for (unsigned int i = 0; i < indexValues.size(); ++i)
            {
                indexValues[i] = i;
            }
            indexAttr = FnAttribute::IntAttribute(indexValues.data(), indexValues.size(), 1);
        }
        
        if (!indexAttr.isValid() || !uvAttr.isValid())
        {
            AiMsgWarning("[kat] unable to get arbitary.st attributes @ %s %s",
                sgIterator.getFullName().c_str(), arbitraryAttr.getError().c_str());
            return;
        }
        
        //get and check data
        IntConstVector uvidxs = indexAttr.getNearestSample(0);
        FloatConstVector uvlistConst = uvAttr.getNearestSample(0);
        
        if(verts.size() != uvidxs.size())
        {
            AiMsgWarning("[kat] uvidxs size missmatch @ %s", sgIterator.getFullName().c_str());
            return;
        }

        std::vector<float> uvlist;
        uvlist.resize(uvlistConst.size());
        for (std::vector<float>::size_type i = 0; i < uvlist.size(); i += 2)
        {
            //U
            uvlist[i] = uvlistConst[i];

            //V : we need to check if we want to flip it or not
            float v = uvlistConst[i+1];
            if(flipV == "flip")
            {
                v = 1.0 - v;
            }

            uvlist[i+1] = v;
        }
        
        //give uvlist to arnold
        AiNodeSetArray(meshNode,"uvlist", 
                    AiArrayConvert(uvlist.size(), 1, AI_TYPE_FLOAT, (void*)(&uvlist[0])));

        //need to convert the int vector to long (on m64 this causes problems otherwise)
        std::vector<unsigned int> uvidxsLong(uvidxs.size(), 0);
        for(unsigned int i = 0; i < uvidxs.size(); i++) uvidxsLong[i] = (long) uvidxs[i];

        //give idxs to arnold
        AiNodeSetArray(meshNode, "uvidxs", 
            AiArrayConvert(uvidxs.size(), 1, AI_TYPE_UINT, (void*)&(uvidxsLong[0])));
    }
    //old crap
    else
    {
        AiMsgDebug("[kat] missing geometry.arbitrary.st @ %s", sgIterator.getFullName().c_str());

        //UVs from pit, these are unshared! they create a real mess on subd's!!!
        //test for facevarying
        bool facevarying = false;
        FnAttribute::FloatAttribute uvAttr = pointAttr.getChildByName("uv");
        if (!uvAttr.isValid() && vertexAttr.isValid())
        {
            uvAttr = vertexAttr.getChildByName("uv");
            facevarying = true;
        }
        
        if (uvAttr.isValid())
        {
            //need to flip V for arnold
            FloatConstVector uvs = uvAttr.getNearestSample(0);
            std::vector<float> fixedUVs(uvs.size(), 0);
            
            for(unsigned int i = 0; i < fixedUVs.size(); i+=2)
            {
                fixedUVs[i] = uvs[i];
                fixedUVs[i+1] = 1 - uvs[i+1];
            }
            
            AiNodeSetArray(meshNode,"uvlist",
                               AiArrayConvert(fixedUVs.size(), 1, 
                               AI_TYPE_FLOAT, (void*)(&fixedUVs[0])));
            
            if (!facevarying)
            {
                std::vector<unsigned int> uvidxs(uvs.size()/2, 0);
                for (unsigned int i = 0; i < uvidxs.size(); i++) uvidxs[i] = i;
            
                //not facevarying, just use verts index
                //const IntAttr::Storage_Type& verts = vertexListAttr->getNearestSample(0);
                AiNodeSetArray(meshNode, "uvidxs", AiArrayConvert(uvidxs.size(), 1, AI_TYPE_UINT,
                    (void*)&(uvidxs[0])));
            }
            else
            {
                //facevarying, assume straight index array, make one
                std::vector<unsigned int> uvidxs(uvs.size()/2, 0);
                for (unsigned int i = 0; i < uvidxs.size(); i++) uvidxs[i] = i;

                AiNodeSetArray(meshNode, "uvidxs", AiArrayConvert(uvidxs.size(), 1, AI_TYPE_UINT,
                    (void*)&(uvidxs[0])));
            }

        }
        else
        {
            AiMsgDebug("[kat] missing uv's @ %s", sgIterator.getFullName().c_str());
        }
    }
}


void applySubdSettingsToMesh(AtNode * meshNode,
                             FnScenegraphIterator sgIterator, AtByte iterations,
                             std::vector<AtNode*>* childNodes,
                             ArnoldPluginState* sharedState)
{
    if (!meshNode) return;
    
    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(meshNode);

    GroupAttribute arnoldStatementsGroup =
        sgIterator.getAttribute("arnoldStatements", true);
    
    if (sgIterator.getType() == "subdmesh")
    {
        std::string subdivType = "catclark";

        if (arnoldStatementsGroup.isValid())
        {
            FnAttribute::StringAttribute subdivTypeAttr = arnoldStatementsGroup.getChildByName("subdiv_type");
            if(subdivTypeAttr.isValid())
            {
                subdivType = subdivTypeAttr.getValue();
            }
        }

        AiNodeSetStr(meshNode, "subdiv_type", subdivType.c_str());

        // Only set the subdiv_iterations on subdmeshes
        AiNodeSetByte(meshNode, "subdiv_iterations", iterations);

    }
    else
    {
        // We will not set subdiv_type on Polumeshes (same as setting it to "none")
    }

    if (!arnoldStatementsGroup.isValid()) return;

    FnAttribute::FloatAttribute zeroAttr = arnoldStatementsGroup.getChildByName("zero_value");
    if (zeroAttr.isValid())
    {
        float zero_value = zeroAttr.getValue();
        AiNodeSetFlt(meshNode, "disp_zero_value", zero_value);
    }

    FnAttribute::FloatAttribute dispHeightAttr = arnoldStatementsGroup.getChildByName("disp_height");
    if (dispHeightAttr.isValid())
    {
        float dispHeight = dispHeightAttr.getValue();

        const AtParamEntry* pentry = AiNodeEntryLookUpParameter(
                baseNodeEntry, "disp_height");

        if (pentry)
        {
            AiNodeSetFlt(meshNode, "disp_height", dispHeight);
        }
    }

    float dispPadding = 0.0f;
    bool hasDispPadding = false;

    FnAttribute::FloatAttribute dispPaddingAttr = arnoldStatementsGroup.getChildByName("disp_padding");
    if (dispPaddingAttr.isValid())
    {
        dispPadding = dispPaddingAttr.getValue();
        hasDispPadding = true;
    }
    else
    {
        FnAttribute::FloatAttribute displacementPaddingAttr = arnoldStatementsGroup.getChildByName("displacement_padding");
        if (displacementPaddingAttr.isValid())
        {
            dispPadding = displacementPaddingAttr.getValue();
            hasDispPadding = true;
        }
    }

    if (hasDispPadding)
    {
        const AtParamEntry* pentry = AiNodeEntryLookUpParameter( baseNodeEntry, "disp_padding");

        if (pentry)
        {
            AiNodeSetFlt(meshNode, "disp_padding", dispPadding);
        }
    }


    FnAttribute::FloatAttribute maxEdgeLengthAttr =
        arnoldStatementsGroup.getChildByName("pixel_error");
    if (maxEdgeLengthAttr.isValid())
    {
        const AtParamEntry* pentry = AiNodeEntryLookUpParameter(
                baseNodeEntry, "subdiv_pixel_error");
        if (pentry && AiParamGetType(pentry) == AI_TYPE_FLOAT)
        {
            AiNodeSetFlt(meshNode, "subdiv_pixel_error",
                    maxEdgeLengthAttr.getValue());
        }
    }
    
    FnAttribute::IntAttribute intAttr = arnoldStatementsGroup.getChildByName("subdiv_smooth_derivs");
    if (intAttr.isValid())
    {
        AiNodeSetBool(meshNode, "subdiv_smooth_derivs", intAttr.getValue() != 0);
    }

    FnAttribute::IntAttribute adaptiveMetricAttr =
            arnoldStatementsGroup.getChildByName("subdiv_adaptive_metric");
    
    //support legacy attr
    if (!adaptiveMetricAttr.isValid())
    {
        adaptiveMetricAttr =
            arnoldStatementsGroup.getChildByName("adaptive_metric");
    }
    
    if (adaptiveMetricAttr.isValid())
    {
        const AtParamEntry* pentry = AiNodeEntryLookUpParameter(
                baseNodeEntry, "subdiv_adaptive_metric");
        
        if (pentry && AiParamGetType(pentry) == AI_TYPE_ENUM)
        {
            AiNodeSetInt(meshNode, "subdiv_adaptive_metric",
                    adaptiveMetricAttr.getValue());
        }
    }
    
    FnAttribute::StringAttribute subdivCamAttr =
        arnoldStatementsGroup.getChildByName("subdiv_dicing_camera");
    if (subdivCamAttr.isValid())
    {
        std::string camPath = pystring::strip(subdivCamAttr.getValue());
        
        if (!camPath.empty())
        {
            AtNode* existingCamera = AiNodeLookUpByName(camPath.c_str());
            
            if (existingCamera)
            {
                AiNodeSetPtr(meshNode, "subdiv_dicing_camera", existingCamera);
            }
            else
            {
                AtNode* newCamera =  buildCameraLocationByPath(camPath,
                   sgIterator.getRoot(), 0, sharedState);
                
                if (newCamera)
                {
                    if (childNodes)
                    {
                        childNodes->push_back(newCamera);
                    }
                    
                    AiNodeSetPtr(meshNode, "subdiv_dicing_camera", newCamera);
                }
            }
        }
    }
}



AtByte getSubdivisionIterations(FnScenegraphIterator sgIterator)
{
    //THESE ARE THE DEFACTO DEFAULT VALUES
    AtByte iterations=0;

    GroupAttribute arnoldStatementsGroup = sgIterator.getAttribute("arnoldStatements", true);
    
    if (arnoldStatementsGroup.isValid())
    {
        FnAttribute::IntAttribute iterationsAttr = arnoldStatementsGroup.getChildByName("iterations");
        iterations = (AtByte)iterationsAttr.getValue(iterations, false);
    }
    return iterations;
}


AtNode* buildPolymesh(FnScenegraphIterator sgIterator,
                      std::vector<AtNode*>* childNodes,
                      ArnoldPluginState* sharedState)
{
    if (!isVisible(sgIterator)) return 0;
    std::string name = sgIterator.getFullName();
   
    //TODO: do we really need this test? OPT? 
    if(AiNodeLookUpByName(name.c_str()))
    {
        AiMsgError("[kat] '%s' is already in the scene!", name.c_str());
        return 0;
    }

    //get some attrs we need      
    GroupAttribute geometryAttr = sgIterator.getAttribute("geometry");
    if (!geometryAttr.isValid())
    {
        AiMsgError("[kat] Polymesh '%s' is missing the geometry attribute.", name.c_str());
        return 0;
    }

    GroupAttribute vertexAttr = geometryAttr.getChildByName("vertex");
    GroupAttribute polyAttr = geometryAttr.getChildByName("poly");
    GroupAttribute pointAttr = geometryAttr.getChildByName("point");
    if (!polyAttr.isValid() || !pointAttr.isValid())
    {
        AiMsgError("[kat] Polymesh '%s' has an invalid geometry attribute.", name.c_str());
        return 0;
    }
    
    FnAttribute::FloatAttribute pAttr = pointAttr.getChildByName("P");
    FnAttribute::IntAttribute polyStartIndexAttr = polyAttr.getChildByName("startIndex");
    FnAttribute::IntAttribute vertexListAttr = polyAttr.getChildByName("vertexList");
    if (!pAttr.isValid() || !polyStartIndexAttr.isValid() || !vertexListAttr.isValid())
    {
        AiMsgError("[kat] Polymesh '%s' has an invalid geometry attribute.", name.c_str());
        return 0;
    }
    
    
    // Convert from polyStartIndex to num_verticesPerPoly, because that's what Arnold wants.
    unsigned int numPolys = polyStartIndexAttr.getNumberOfTuples();
    std::vector<unsigned char> num_verticesPerPoly = std::vector<unsigned char>(numPolys, 0);
    int numVerts = 0;
    IntConstVector polyStartIndexAttrValue = polyStartIndexAttr.getNearestSample(0);
    {
        unsigned int srcIndex=0;
        for(IntConstVector::size_type i = 0, n = polyStartIndexAttr.getNumberOfTuples();
            srcIndex < n; ++i, ++srcIndex)
        {
            if (srcIndex+1 < n) 
            {
                numVerts = polyStartIndexAttrValue[srcIndex+1] - polyStartIndexAttrValue[srcIndex];
            } 
            else 
            {
                numVerts = vertexListAttr.getNumberOfTuples() - polyStartIndexAttrValue[srcIndex];
            }
            
            //Arnold currently only supports 8bit numVertices per poly
            //need to check for that here
            //TODO: make arnold accept 16bit numVertices
            num_verticesPerPoly[i] = (unsigned char) numVerts;
            if(numVerts > 255)
            {
                AiMsgWarning("[kat] polymesh node has polygon(s) with more that 255 verticies. skipping  -> %s",  name.c_str());
                return NULL;
            }   

            // Shockingly, this is all that's needed to drop empty polygons.
            if(numVerts < 1 || numVerts > 255)
            {
                --i;
                --(numPolys);
            }
        }
    }

    //make sure we still have a mesh
    if(numPolys < 1)
    {
        AiMsgWarning("[kat] polymesh node has 0 valied polygons -> %s",  name.c_str());
        return NULL;
    }


    //try and make new polymesh node in Arnold
    std::string meshType("polymesh");   
    AtNode* meshNode = AiNode(meshType.c_str());
    if (!meshNode)
    {
        AiMsgError("[kat] Mesh '%s' failed to create mesh node.", name.c_str());
        return 0;
    }

    //let procedural know we made a new node
    if (childNodes) childNodes->push_back(meshNode);
    AiNodeSetStr(meshNode, "name", name.c_str());
    
    
    //get num samples for P
    std::set<float> sampleTimes;
    getSampleTimes(sgIterator.getRoot(), pAttr, &sampleTimes, sharedState);
    
    
    //reserve all P's
    std::vector<float> vlist;
    vlist.reserve(sampleTimes.size() * pAttr.getNumberOfValues());

    //concat all samples into one big .ass array 
    for(std::set<float>::iterator it = sampleTimes.begin(); it != sampleTimes.end(); ++it)
    {
        FloatConstVector sampleData = pAttr.getNearestSample(*it);
        
        //AiMsgInfo("[kat] numPolys: %d  numVerts: %d", numPolys, vlist.size()/3);
        //for(unsigned int i = 0; i < sampleData.size(); i++)
        //    if(isnan(sampleData[i])) AiMsgInfo("NAN @ %d", i);   
        
        vlist.insert(vlist.end(), sampleData.begin(), sampleData.end());
    }
    
    if (sampleTimes.size() > 1)
    {
        std::vector<float> outputSampleTimes;
        outputSampleTimes.reserve(sampleTimes.size());
        outputSampleTimes.insert(outputSampleTimes.end(), sampleTimes.begin(), sampleTimes.end());
        const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(meshNode);
        if (AiNodeEntryLookUpParameter(baseNodeEntry, "deform_time_samples"))
        {
            AiNodeSetArray(meshNode, "deform_time_samples",
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
    
    
    //send P samples to arnold
    AiNodeSetArray(meshNode, "vlist", AiArrayConvert(pAttr.getNumberOfValues(),
        sampleTimes.size(), AI_TYPE_FLOAT, (void*)(&(vlist[0]))));


    AiNodeSetArray(meshNode, "nsides", AiArrayConvert(numPolys, 1, AI_TYPE_BYTE, &(num_verticesPerPoly[0])));

    IntConstVector verts = vertexListAttr.getNearestSample(0);
    std::vector<unsigned int> vidxs(verts.size(), 0);
    for(unsigned int i = 0; i < verts.size(); i++) vidxs[i] = (long) verts[i];
    AiNodeSetArray(meshNode, "vidxs",  AiArrayConvert(vidxs.size(), 1, AI_TYPE_UINT, (void*)&vidxs[0]));

    if (sgIterator.getType() != "subdmesh")
    {
        FnAttribute::FloatAttribute pointNormalAttr = pointAttr.getChildByName("N");
        FnAttribute::FloatAttribute vertexNormalAttr = vertexAttr.getChildByName("N");
        if (pointNormalAttr.isValid())
        {
            getSampleTimes(sgIterator.getRoot(), pointNormalAttr, &sampleTimes, sharedState);
            std::vector<float> nlist;
            nlist.reserve(sampleTimes.size() * pointNormalAttr.getNumberOfValues());

            //concat all samples into one big .ass array 
            for(std::set<float>::iterator it = sampleTimes.begin(); it != sampleTimes.end(); ++it)
            {
                FloatConstVector sampleData = pointNormalAttr.getNearestSample(*it);
                nlist.insert(nlist.end(), sampleData.begin(), sampleData.end());
            }

            AiNodeSetArray(meshNode, "nlist", 
                    AiArrayConvert(
                        pointNormalAttr.getNumberOfValues(),
                        sampleTimes.size(), AI_TYPE_FLOAT, 
                        (void*)&nlist[0]));

            IntConstVector verts = vertexListAttr.getNearestSample(0);
            std::vector<unsigned int> nidxs(verts.size(), 0);
            for(unsigned int i = 0; i < verts.size(); i++) nidxs[i] = (long) verts[i];
            AiNodeSetArray(meshNode, "nidxs",  AiArrayConvert(nidxs.size(), 1, AI_TYPE_UINT, (void*)&nidxs[0]));
        }
        else if (vertexNormalAttr.isValid())
        {
            getSampleTimes(sgIterator.getRoot(), vertexNormalAttr, &sampleTimes, sharedState);
            std::vector<float> nlist;
            nlist.reserve(sampleTimes.size() * vertexNormalAttr.getNumberOfValues());

            //concat all samples into one big .ass array 
            for(std::set<float>::iterator it = sampleTimes.begin(); it != sampleTimes.end(); ++it)
            {
                FloatConstVector sampleData = vertexNormalAttr.getNearestSample(*it);
                nlist.insert(nlist.end(), sampleData.begin(), sampleData.end());
            }

            AiNodeSetArray(meshNode, "nlist", 
                    AiArrayConvert(
                        vertexNormalAttr.getNumberOfValues(),
                        sampleTimes.size(), AI_TYPE_FLOAT, 
                        (void*)&nlist[0]));

            //make new, atrificial nidx array
            std::vector<unsigned int> nidxs;
            long count = vertexNormalAttr.getNumberOfValues() / 3;
            nidxs.reserve(count);
            for(long i = 0; i < count; ++i)
                nidxs.push_back(i);
            AiNodeSetArray(meshNode, "nidxs",  AiArrayConvert(nidxs.size(), 1, AI_TYPE_UINT, &nidxs[0]));

            //AiMsgInfo("vertex normals");
            //TODO: busted
            //const IntAttr::Storage_Type& verts = vertexListAttr->getNearestSample(0);
            //AiNodeSetArray(meshNode, "nidxs",  AiArrayConvert(verts.size(), 1, AI_TYPE_LONG, (void*)&verts[0]));

            //AiNodeSetFlt(meshNode, "smoothing", 1.0f);
        }
        else
        {
            //no normals?
        }
    }
    
    //native support for pref
    FnAttribute::FloatAttribute PrefAttr = pointAttr.getChildByName("Pref");
    if (PrefAttr.isValid())
    {
        FloatConstVector pref = PrefAttr.getNearestSample(0);

        unsigned long prefSize = pref.size();

        if (pAttr.getNumberOfValues() == prefSize)
        {
            unsigned long numPoints = prefSize/3;

            std::vector<AtPoint> arnoldPrefVector;
            arnoldPrefVector.reserve(numPoints);

            for(unsigned int i = 0; i < numPoints; i++)
            {
                AtPoint p;
                int index = i*3;
                p.x = pref[index];
                p.y = pref[index+1];
                p.z = pref[index+2];
                arnoldPrefVector.push_back(p);
            }

            AiNodeDeclare( meshNode, "Pref", "varying POINT");
            AiNodeSetArray(meshNode, "Pref", 
                    AiArrayConvert(numPoints, 1, AI_TYPE_POINT,
                        (void*)&arnoldPrefVector[0]));
        }
        else
        {
            AiMsgWarning("[kat] polymesh node has Pref of invalid size");
        }
    }
   
    // Add creasing if present.
    FnAttribute::IntAttribute creaseIndices = geometryAttr.getChildByName("creaseIndices");
    if (creaseIndices.isValid())
    {
        IntConstVector creaseIndicesValue = creaseIndices.getNearestSample(0);
        std::vector<unsigned int> convertedValues;
        convertedValues.reserve(creaseIndicesValue.size());
        convertedValues.insert(convertedValues.begin(), creaseIndicesValue.begin(), creaseIndicesValue.end());
        AiNodeSetArray(meshNode, "crease_idxs", AiArrayConvert(convertedValues.size(), 1, AI_TYPE_UINT, (void*)(&convertedValues[0])));
    }

    AtByte iterations = getSubdivisionIterations(sgIterator);
    applySubdSettingsToMesh(meshNode, sgIterator, iterations, childNodes, sharedState);
    
    //get arbitrary and handle it
    GroupAttribute arbitraryAttr = sgIterator.getAttribute("geometry.arbitrary", true);
    if (arbitraryAttr.isValid())
    {
        writeArbitrary(meshNode, sgIterator.getFullName(), sgIterator.getType(),
                geometryAttr, arbitraryAttr);
    }

    //try and get geometry.g.st
    GroupAttribute stAttr = sgIterator.getAttribute("geometry.arbitrary.st", true);


    // Check if we have to flip the V coordinate
    std::string flipV = "none";
    FnAttribute::StringAttribute objFlipTAttr = sgIterator.getAttribute("arnoldStatements.flip_t", true);
    if(objFlipTAttr.isValid())
    {
        flipV = objFlipTAttr.getValue();
    }
    else
    {
        FnAttribute::StringAttribute globalFlipTAttr = sgIterator.getRoot().getAttribute("arnoldGlobalStatements.flip_t", false);
        if (globalFlipTAttr.isValid())
        {
            flipV = globalFlipTAttr.getValue();
        }
    }


    //try and write UV's - this will try to write new arbitrary stule from geometry.arbitrary.st
    //if that fails, it will take uv's from pit - they will most likely be wrong
    //TODO: do we really want the fallback to pit?
    writeArnoldUVs(meshNode, sgIterator, geometryAttr, stAttr, vertexAttr, pointAttr, vertexListAttr, verts, flipV);


    bool* visibility_array      = NULL;

    unsigned char* subdiv_face_iterations_array = NULL;


    //deal with visibility
    //TODO: if arnold start supporting crease (or any other feature) for subd
    //we may need to add handling function here.
    //face visibility feature only supported arnold 2.21.0.0 and above.
    std::list< std::string > tempStringArray;
    for (FnScenegraphIterator child = sgIterator.getFirstChild();
         child.isValid(); child = child.getNextSibling())
    {
        if (child.getType() != "faceset")
        {
            continue;
        }

        GroupAttribute childGeometryGroup = child.getAttribute("geometry");
        if (!childGeometryGroup.isValid())
        {
            AiMsgWarning("[kat] geometry group attribute missing %s", child.getFullName().c_str());
            continue;
        }

        FnAttribute::IntAttribute facesAttr = childGeometryGroup.getChildByName("faces");
        if (!facesAttr.isValid())
        {
            AiMsgWarning("[kat] geometry.faces attribute missing %s", child.getFullName().c_str());
            continue;
        }

        FnAttribute::IntAttribute visibleAttr = child.getAttribute("visible");
        if (visibleAttr.isValid() && visibleAttr.getValue() == 0)
        {
            if (visibility_array == NULL)
            {
               std::string name = "face_visibility";
               std::string userType("uniform ");
               userType += "BOOL";
               visibility_array = new bool[numPolys];
               for (unsigned int i = 0; i < numPolys; i++)
                  visibility_array[ i ] = true;


               if(!UserDefDeclare(meshNode, name.c_str(), userType.c_str()))
                  continue;
            }

            IntConstVector facesAttrValue = facesAttr.getNearestSample(0);
            for (IntConstVector::const_iterator I = facesAttrValue.begin(), E = facesAttrValue.end();
                 I!=E; ++I)
            {
                visibility_array[ *I ] = false;
            }
        }

        if (iterations > 0)
        {
            FnAttribute::IntAttribute iterationsAttr = child.getAttribute("arnoldStatements.iterations");

            if (iterationsAttr.isValid())
            {
                unsigned char localValue = iterationsAttr.getValue();

                if (subdiv_face_iterations_array == NULL
                        && localValue != iterations)
                {
                    subdiv_face_iterations_array = new unsigned char[numPolys];

                    for (unsigned int i = 0; i < numPolys; i++)
                    {
                        subdiv_face_iterations_array[i] = iterations;
                    }

                    std::string name = "subdiv_face_iterations";
                    std::string userType("uniform BYTE");

                    UserDefDeclare(meshNode, name.c_str(), userType.c_str());

                }

                if (subdiv_face_iterations_array)
                {

                    IntConstVector facesAttrValue = facesAttr.getNearestSample(0);
                    for (IntConstVector::const_iterator I = facesAttrValue.begin(), E = facesAttrValue.end();
                         I!=E; ++I)
                    {
                        if ( *I < (int)numPolys)
                        {
                            subdiv_face_iterations_array[*I] = localValue;
                        }
                    }
                }
            }
        }
    }
    
    if (visibility_array)
    {
        std::string name = "face_visibility";
        std::string userType("uniform ");
        userType += "BOOL";
        int apiType = AI_TYPE_BOOLEAN;
        SetUserData(meshNode, name.c_str(), numPolys, apiType, (void*)visibility_array);
         delete [] visibility_array;
         visibility_array = NULL;
    }

    if (subdiv_face_iterations_array)
    {
        SetUserData(meshNode, "subdiv_face_iterations", numPolys,
                AI_TYPE_BYTE, (void*)subdiv_face_iterations_array); 
        delete [] subdiv_face_iterations_array;
        subdiv_face_iterations_array = 0;
    }
    
    //deal with textures
    setTextureUserDataForPolymesh(sgIterator, meshNode, numPolys, geometryAttr); 


    return meshNode;
}

void* ArnoldPolymeshScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator,
                                                        void* optionalInput)
{
    ArnoldSceneGraphLocationDelegateInput* state = reinterpret_cast<ArnoldSceneGraphLocationDelegateInput*>(optionalInput);
    void* returnValue = buildPolymesh(sgIterator, state->childNodes, state->sharedState);
    if(returnValue){
        applyObjectSettings(sgIterator, reinterpret_cast<AtNode*>(returnValue), state->childNodes, state->sharedState);
    }
    return returnValue;
}

ArnoldPolymeshScenegraphLocationDelegate::~ArnoldPolymeshScenegraphLocationDelegate()
{

}

void ArnoldPolymeshScenegraphLocationDelegate::flush()
{

}

ArnoldPolymeshScenegraphLocationDelegate::ArnoldPolymeshScenegraphLocationDelegate()
{

}

ArnoldPolymeshScenegraphLocationDelegate* ArnoldPolymeshScenegraphLocationDelegate::create()
{
    return new ArnoldPolymeshScenegraphLocationDelegate();
}

std::string ArnoldPolymeshScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("arnold");
}

void ArnoldPolymeshScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("polymesh"));
    supportedLocationList.push_back(std::string("subdmesh"));
}
