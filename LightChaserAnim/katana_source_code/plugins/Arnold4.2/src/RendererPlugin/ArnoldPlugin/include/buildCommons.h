// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#ifndef BUILDCOMMONS_H
#define BUILDCOMMONS_H

#include <FnScenegraphIterator/FnScenegraphIterator.h>
#include <ai_nodes.h>
#include <ai_msg.h>
#include <set>
#include <map>
#include <OpenEXR/ImathMatrix.h>
#include <ai_universe.h>
#include <ArnoldPluginState.h>
#include <FnAttribute/FnAttribute.h>

Imath::M44d XFormAttrToIMath(const FnAttribute::DoubleAttribute & attr, float t);
void XFormAttrToIMathVec(std::vector<Imath::M44d> & vec, const FnAttribute::DoubleAttribute & attr);


void setTextureUserDataForInstance(FnKat::FnScenegraphIterator sgIterator, AtNode* node, ArnoldPluginState* sharedState);

void setBackgroundVisibility(AtNode * node, const char * paramName, FnAttribute::GroupAttribute visAttr);
void buildArbitraryStatements(FnAttribute::GroupAttribute enclosingGroup, AtNode * node,
        //optional sgIterator argument will attempt to create user attributes for
        //non-existent parameters. This is only called from /root for arnoldGlobalStatements.
        FnKat::FnScenegraphIterator sgIterator=FnKat::FnScenegraphIterator());

bool isVisible(FnKat::FnScenegraphIterator sgIterator) ;
void getSampleTimes(FnKat::FnScenegraphIterator rootIterator, FnKat::DataAttribute dataAttr, std::set<float>* samples, ArnoldPluginState* sharedState);
void writeArbitrary(AtNode* node, const std::string & locationFullName, const std::string & locationType, FnAttribute::GroupAttribute geometryAttr, FnAttribute::GroupAttribute arbitraryAttr); 
void SetUserData(AtNode* node, const char* name, int dataSize, int apiType, void* dataStart, bool forceArray=false);
bool UserDefDeclare(AtNode* node, const char* name, const char* type);
bool setTextureUserDataAsArbitrary(FnKat::FnScenegraphIterator sgIterator, AtNode* node,
        FnAttribute::GroupAttribute texturesAttr, FnAttribute::GroupAttribute geometryAttr = FnAttribute::GroupAttribute());
void setTextureUserDataForNurbspatch(FnKat::FnScenegraphIterator sgIterator, AtNode* meshNode);
AtByte getSubdivisionIterations(FnKat::FnScenegraphIterator sgIterator);
void setTextureUserDataForPolymesh(FnKat::FnScenegraphIterator sgIterator, AtNode* meshNode,
    unsigned int numPolys, FnAttribute::GroupAttribute geometryAttr = FnAttribute::GroupAttribute());
// Apply the correct transformation matrix to node.
void applyTransform(AtNode* node, FnKat::FnScenegraphIterator sgIterator,  ArnoldPluginState* sharedState, std::map<float, Imath::M44d> *returnSamples=0,
        FnAttribute::GroupAttribute * overrideXformAttr = 0);
void setArnoldParam_Bool(FnAttribute::GroupAttribute globals, const char* attrName, AtNode*node = 0, const char* arnoldName = 0);

void setArnoldParam_Float(FnAttribute::GroupAttribute globals, const char* attrName, AtNode*node = 0, const char* arnoldName = 0);
void M44d_to_AtMatrix(AtMatrix m, Imath::M44d xform);
void setArnoldParam_Int(FnAttribute::GroupAttribute globals, const char* attrName, AtNode*node = 0, const char* arnoldName = 0);

// Get the transform that applies to a location.
void getTransformSamples(std::map<float, Imath::M44d>* sampleTimes,
        FnKat::FnScenegraphIterator sgIterator, ArnoldPluginState* sharedState, bool clampToShutter=true,
                FnAttribute::GroupAttribute * overrideXformAttr = 0);
void SetGlobalNormalizedCropWindow(float c0, float c1, float c2, float c3, ArnoldPluginState* sharedState);
void GetGlobalNormalizedCropWindow(float & c0, float & c1, float & c2, float & c3, ArnoldPluginState* sharedState);
float GetFrameTime(FnKat::FnScenegraphIterator sgIterator, ArnoldPluginState* sharedState);
#endif
