// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef WRITERI_UTIL_H
#define WRITERI_UTIL_H

#include <FnScenegraphIterator/FnScenegraphIterator.h>
#include <AttrList.h>

namespace PRManProcedural
{
    void AddTextureAttrs(AttrList* attrList, FnAttribute::GroupAttribute texturesAttr, int uniformCount, FnKat::FnScenegraphIterator sgIterator);
    void AddAttributesFromTextureAttrs(FnAttribute::GroupAttribute texturesAttr);
    void AddRefParameters(AttrList* attrList, FnAttribute::GroupAttribute geometryAttr, FnKat::FnScenegraphIterator sgIterator);
    void AddArbitraryParameters(AttrList* attrList, FnAttribute::GroupAttribute geometryAttr, FnKat::FnScenegraphIterator sgIterator);
    void FillBoundFromAttr(RtBound * bound, FnAttribute::DoubleAttribute boundAttr, PRManPluginState* sharedState);
    bool BuildPolymeshData(FnKat::FnScenegraphIterator sgIterator,
        RtInt *num_polys, std::vector<RtInt> *num_vertices, std::vector<RtInt> *vertices,
        AttrList * attrList);

    bool IsVisible(FnKat::FnScenegraphIterator sgIterator);
    FnKat::Attribute createConstantAttrByIndex(FnKat::DataAttribute attr, int index);
    void HandleObjectInstance(const ProducerPacket& producerPacket);

    enum CameraInfoStyle
    {
        CAMERA_NONE,
        CAMERA_PARAMETERS,
        CAMERA_ATTRIBUTES,
    };

    CameraInfoStyle ParseCameraInfoAttributes(FnKat::FnScenegraphIterator sgIterator,
                                              std::string & outputCameraPath,
                                              const std::string & attrBaseName);

    float GetFrameNumber( FnKat::FnScenegraphIterator sgIterator, PRManPluginState* sharedState );
    std::string BuildDeclarationFromAttribute(const std::string& argName, const FnKat::Attribute attr);
}

#endif
