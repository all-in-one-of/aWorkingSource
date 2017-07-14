// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <rx.h>
#include <iostream>
#include <PRManRibArchiveScenegraphLocationDelegate.h>
#include <ErrorReporting.h>
using namespace PRManProcedural;
#include <WriteRI_Util.h>
#include <WriteRI_Attributes.h>
#include <WriteRI_ObjectID.h>
#include <WriteRI_Shaders.h>
#include <WriteRI_Statements.h>
#include <FnAsset/FnDefaultAssetPlugin.h>

using namespace FnKat;

void* processRibArchive(FnScenegraphIterator sgIterator)
{
    if (!sgIterator.isValid()) return 0x0;
    if (!IsVisible(sgIterator)) return 0x0;

    FnAttribute::StringAttribute archivePathAttr = sgIterator.getAttribute("archivepath");
    if (!archivePathAttr.isValid())
    {
        archivePathAttr = sgIterator.getAttribute("geometry.filename");
    }

    if (!archivePathAttr.isValid())
    {
        Report_Error("RIB Archive did not specify the archivepath or geometry.filename attribute.", sgIterator);
        return 0x0;
    }

    std::string path = archivePathAttr.getValue();

    // Resolve asset ID
    if (DefaultAssetPlugin::isAssetId(path))
    {
        path = DefaultAssetPlugin::resolvePath(path, 0);
    }

    RiReadArchive(const_cast<char*>(path.c_str()), NULL, NULL);
    return 0x0;
}

void* PRManRibArchiveScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator, void* optionalInput)
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

    void* returnValue = processRibArchive(sgIterator);

    RiAttributeEnd();
    return returnValue;
}

PRManRibArchiveScenegraphLocationDelegate::~PRManRibArchiveScenegraphLocationDelegate()
{

}

void PRManRibArchiveScenegraphLocationDelegate::flush()
{

}

PRManRibArchiveScenegraphLocationDelegate* PRManRibArchiveScenegraphLocationDelegate::create()
{
    return new PRManRibArchiveScenegraphLocationDelegate();
}

std::string PRManRibArchiveScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("prman");
}

void PRManRibArchiveScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("ribarchive"));
    supportedLocationList.push_back(std::string("rib archive"));
}
