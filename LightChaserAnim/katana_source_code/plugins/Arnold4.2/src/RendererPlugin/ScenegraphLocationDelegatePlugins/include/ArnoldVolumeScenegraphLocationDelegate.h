// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#ifndef ARNOLDVOLUMESCENEGRAPHLOCATIONDELEGATE_H
#define ARNOLDVOLUMESCENEGRAPHLOCATIONDELEGATE_H

#include <FnRender/plugin/ScenegraphLocationDelegate.h>

class ArnoldVolumeScenegraphLocationDelegate : public Foundry::Katana::Render::ScenegraphLocationDelegate
{
public:
    ArnoldVolumeScenegraphLocationDelegate();
    virtual void* process(FnKat::FnScenegraphIterator sgIterator, void* optionalInput);
    virtual ~ArnoldVolumeScenegraphLocationDelegate();
    static void flush();
    static ArnoldVolumeScenegraphLocationDelegate* create();
    virtual std::string getSupportedRenderer() const;
    virtual void fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const;
};

#endif

