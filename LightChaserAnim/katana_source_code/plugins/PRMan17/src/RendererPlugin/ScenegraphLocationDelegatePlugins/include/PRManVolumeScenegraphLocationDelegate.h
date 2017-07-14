// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef PRMANVOLUMESCENEGRAPHLOCATIONDELEGATE_H
#define PRMANVOLUMESCENEGRAPHLOCATIONDELEGATE_H

#include <set>
#include <vector>

#include <rx.h>

#include <FnAttribute/FnAttribute.h>
#include <FnRender/plugin/ScenegraphLocationDelegate.h>

#include <AttrList.h>


class PRManVolumeScenegraphLocationDelegate : public Foundry::Katana::Render::ScenegraphLocationDelegate
{
public:

    virtual ~PRManVolumeScenegraphLocationDelegate();

    virtual void* process(FnKat::FnScenegraphIterator sgIterator, void* optionalInput);
    virtual std::string getSupportedRenderer() const;
    virtual void fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const;

    static void flush();
    static PRManVolumeScenegraphLocationDelegate* create();

private:

    void* processVolumeLocation();

    void* processRiBlobby(FnAttribute::GroupAttribute geometryAttr);
    void* processRiBlobbyDso(FnAttribute::GroupAttribute geometryAttr);
    void* processRiVolume(FnAttribute::GroupAttribute geometryAttr);
    void* processRiVolumeDso(FnAttribute::GroupAttribute geometryAttr);

    bool checkVolumeType();
    void gatherSampleTimesFromAttr(std::set<float>& sampleTimes, FnKat::DataAttribute attr) const;

    void calculateSampleTimes(PRManProcedural::AttrList_Converter& converter,
                              const std::set<float>& extraSamples,
                              std::vector<float>& sampleTimes) const;

    void getVoxelRes(FnAttribute::IntAttribute voxelResAttr, float time, RtInt (&voxelRes)[3]) const;
    void getBound(FnAttribute::DoubleAttribute boundAttr, float time, RtBound& bound) const;

    // Reference to the current scenegraph location
    FnKat::FnScenegraphIterator m_sgIterator;
};

#endif // PRMANVOLUMESCENEGRAPHLOCATIONDELEGATE_H
