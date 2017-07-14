// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#ifndef ARNOLDSPHERESCENEGRAPHLOCATIONDELEGATE_H
#define ARNOLDSPHERESCENEGRAPHLOCATIONDELEGATE_H

#include <FnRender/plugin/ScenegraphLocationDelegate.h>


class ArnoldSphereScenegraphLocationDelegate : public FnKat::Render::ScenegraphLocationDelegate
{
public:
    ArnoldSphereScenegraphLocationDelegate();
    virtual void* process(FnKat::FnScenegraphIterator sgIterator, void* optionalInput);
    virtual ~ArnoldSphereScenegraphLocationDelegate();
    static void flush();
    static ArnoldSphereScenegraphLocationDelegate* create();
    virtual std::string getSupportedRenderer() const;
    virtual void fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const;
};

#endif

