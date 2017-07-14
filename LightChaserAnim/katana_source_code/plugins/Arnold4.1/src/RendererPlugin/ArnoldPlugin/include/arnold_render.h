// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#ifndef INCLUDED_ARNOLD_RENDER_H
#define INCLUDED_ARNOLD_RENDER_H

/* Katana and Arnold integration codebase roadmap.
 * 
 * The basic idea of this code is to have a program which calls the
 * Arnold library to perform rendering.  The input file format is
 * a python file which builds a geometry producer; only the code
 * involved with actually loading the producer from the file should
 * care about Python in any way.  This producer is then queried for
 * whatever global render settings are important (for example, lights).
 * Then we use Arnold procedural nodes to provide deferred expansion
 * on locations we have bounding box information for.
 * 
 * This codebase also contains a display driver to send rendered
 * pixels over a socket to a client using the really primitive
 * Katana bucket protocol.
 *
 *
 * main.cpp - contains int main(int, char*[]); argument processing and
 * renderer intialization code.
 * 
 * buildCamera.cpp - buildRenderCamera, buildCamera functions; everything needed to setup the 
 * cameras.
 * 
 * buildLights.cpp - buildLights function; everything needed to setup all
 * the lights in the scene.
 *
 * buildGeometry.cpp - buildGeometry function;currently contains everything
 * needed to pass geometry to Arnold.
 *
 * procedural.cpp - KatanaProcedural_ProcLoader and support functions;
 * everything procedural geometry related.
 *  
 */
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

#ifdef _WIN32
#include <memory>
#else
#include <tr1/memory>
#endif

#include <FnScenegraphIterator/FnScenegraphIterator.h>
#include <FnRenderOutputUtils/FnRenderOutputUtils.h>

#include <OpenEXR/ImathMatrix.h>

#include <ai.h>
#include <buildCamera.h>
typedef std::map<std::string, AtNode*> InstanceMap_t;


enum {BUCKET_INDICATOR_CORNERS, BUCKET_INDICATOR_CHECKER, BUCKET_INDICATOR_NONE};

void setUserMatrix(AtNode* node, Imath::M44d xform);

bool buildCropWindow(int* crop, std::string crop_window, 
                     std::string crop_windows_default,
                     int width, int height, ArnoldPluginState* sharedState);


// get an Arnold version in an 'int' format so 
// that it's easy to check arnold versions
unsigned int getArnoldVersion();
unsigned int quantifyArnoldVersion(const char*, const char*, const char*, const char*);

void setLogFlags(FnKat::FnScenegraphIterator rootIterator);




//set params from katana param to arnold param
//if !GroupAttribute.isValid() these function will set arnold
//param to be default value(if any)
//if AtNode is NULL these function will set param to be on options node.
void setArnoldParam_RGB(FnAttribute::GroupAttribute globals, const char* attrName, AtNode*node = 0, const char* arnoldName = 0);

void setArnoldParam_Double(FnAttribute::GroupAttribute globals, const char* attrName, AtNode*node = 0, const char* arnoldName = 0);
void setArnoldParam_String(FnAttribute::GroupAttribute globals, const char* attrName, AtNode*node = 0, const char* arnoldName = 0);
void setArnoldParam_Pointer(FnAttribute::GroupAttribute globals, const char* attrName, FnKat::FnScenegraphIterator sgIterator, ArnoldPluginState* sharedState,
        AtNode*node = 0, const char* arnoldName = 0, bool obeyMute=false);


// Apply the globals default settings
void setGlobalsDefault(FnKat::FnScenegraphIterator rootIterator, ArnoldPluginState* sharedState);
// Apply the globals settings
void buildGlobals(FnKat::FnScenegraphIterator rootIterator, ArnoldPluginState* sharedState);

//builds global coordinate systems as nodes of type "transformation" when available
void buildGlobalCoordinateSystems(FnKat::FnScenegraphIterator rootIterator, ArnoldPluginState* sharedState);

void processAssIncludes(FnKat::FnScenegraphIterator rootIterator, ArnoldPluginState* sharedState);


// Build all the geometry in the scene.
void buildGeometry(FnKat::FnScenegraphIterator rootIterator, bool forceExpand, bool nooverride, ArnoldPluginState* sharedState);

// Build all the lights in the scene.
void buildLights(FnKat::FnScenegraphIterator worldIterator, std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState);


void setLightXform(
        AtNode * lightNode, const std::map<float, Imath::M44d> & xformSamples);

void setLightGeometry(AtNode * lightNode, FnAttribute::GroupAttribute geoAttr);

void setLightVisibility(AtNode * node, const char * paramName, FnAttribute::GroupAttribute visAttr);


void buildProceduralArrayArgument( AtNode* procedural, std::string argumentName, std::string declareString,
                                   unsigned char type, int dataSize, const void* data );
void buildProceduralArguments( AtNode* procedural, FnAttribute::GroupAttribute argumentsAttr );


// Build a specific location.
void buildLocation(FnKat::FnScenegraphIterator rootIterator, std::vector<AtNode*>* childNodes, bool forceExpand, bool nooverride, ArnoldPluginState* sharedState, bool obeyInstanceId = true);

// Various geometry building commands, used by buildLocation.
AtNode* buildGroup(FnKat::FnScenegraphIterator sgIterator, std::vector<AtNode*>* childNodes,
    bool forceExpand, bool nooverride, ArnoldPluginState* sharedState, bool obeyInstanceId = true);
void buildInstanceGroup(FnKat::FnScenegraphIterator sgIterator,
        std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState);

// Is a given location visible?
bool isVisible(FnKat::FnScenegraphIterator sgIterator);




void applyShaderParams(std::string type, AtNode* shaderNode, FnAttribute::GroupAttribute shaderParamsAttr,
    FnKat::FnScenegraphIterator sgIterator, std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState);
void applyShaderParam(std::string type, AtNode* node, const std::string& attrName, FnKat::Attribute attrRef,
    FnKat::FnScenegraphIterator sgIterator, std::vector<AtNode*>* childNodes, bool doPointers, ArnoldPluginState* sharedState);



int KatanaProcedural_ProcLoader(AtProcVtable* api);
int KatanaProcedural_ProcNull(AtProcVtable* api);


void makeOSLNetworkNode(
        const std::string locationName,
        FnKat::RenderOutputUtils::ShadingNodeDescriptionMap & nodes,
        std::string nodeName,
        std::set<std::string> & createdNodeNames,
        std::vector<std::string> & commandsOut, ArnoldPluginState* sharedState);

void makeOSLMonolithicCommmandString(
        const std::string & terminalName,
        const std::string & terminalShader,
        FnAttribute::GroupAttribute paramGroup,
        std::vector<std::string> & commandsOut, ArnoldPluginState* sharedState);





FnAttribute::GroupAttribute getFlattenedMaterialAttr(FnKat::FnScenegraphIterator sgIterator, ArnoldPluginState* sharedState);


bool UserDefDeclare(AtNode* node, const char* name, const char* type);


void writeArbitrary(AtNode* node,
        const std::string & locationFullName, const std::string & locationType,
                FnAttribute::GroupAttribute geometryAttr, FnAttribute::GroupAttribute arbitraryAttr);



ByteVectorRcPtr_t GetCachedShaderIndexArray(const std::string & path, ArnoldPluginState* sharedState);

void setPerCameraDefaultAASeed(FnKat::FnScenegraphIterator rootIterator, ArnoldPluginState* sharedState);


#endif
