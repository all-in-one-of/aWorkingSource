// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#ifndef ARNOLDNURBSPATCHSCENEGRAPHLOCATIONDELEGATE_H
#define ARNOLDNURBSPATCHSCENEGRAPHLOCATIONDELEGATE_H

#include <FnRender/plugin/ScenegraphLocationDelegate.h>

class ArnoldNurbspatchScenegraphLocationDelegate : public Foundry::Katana::Render::ScenegraphLocationDelegate
{
public:
    ArnoldNurbspatchScenegraphLocationDelegate();
    virtual void* process(FnKat::FnScenegraphIterator sgIterator, void* optionalInput);
    virtual ~ArnoldNurbspatchScenegraphLocationDelegate();
    static void flush();
    static ArnoldNurbspatchScenegraphLocationDelegate* create();
    virtual std::string getSupportedRenderer() const;
    virtual void fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const;
};

#endif

