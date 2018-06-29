// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#ifndef BUILDMATERIAL_H
#define BUILDMATERIAL_H

#include <FnScenegraphIterator/FnScenegraphIterator.h>
#include <ai.h>

AtNode* getShader(std::string type, FnKat::FnScenegraphIterator sgIterator,
    std::vector<AtNode*>* childNodes, bool hashed, bool unique, bool suppress, ArnoldPluginState* sharedState);
AtNode* getShaderByPath(const std::string& pathName, FnKat::FnScenegraphIterator sgIterator,
    std::vector<AtNode*>* childNodes, bool hashed, ArnoldPluginState* sharedState, const std::string stype="");
void applyMaterial(AtNode* node, FnKat::FnScenegraphIterator sgIterator, std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState);

#endif

