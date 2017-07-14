// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#ifndef ARNOLDGENERICSCENEGRAPHLOCATIONDELEGATE_H
#define ARNOLDGENERICSCENEGRAPHLOCATIONDELEGATE_H

#include <FnRender/plugin/ScenegraphLocationDelegate.h>

class ArnoldGenericScenegraphLocationDelegate : public Foundry::Katana::Render::ScenegraphLocationDelegate
{
public:
    ArnoldGenericScenegraphLocationDelegate();
    virtual void* process(FnKat::FnScenegraphIterator sgIterator, void* optionalInput);
    virtual ~ArnoldGenericScenegraphLocationDelegate();
    static void flush();
    static ArnoldGenericScenegraphLocationDelegate* create();
    virtual std::string getSupportedRenderer() const;
    virtual void fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const;
};

#endif

