// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef PRMANCURVESSCENEGRAPHLOCATIONDELEGATE_H
#define PRMANCURVESSCENEGRAPHLOCATIONDELEGATE_H

#include <FnRender/plugin/ScenegraphLocationDelegate.h>

class PRManCurvesScenegraphLocationDelegate : public Foundry::Katana::Render::ScenegraphLocationDelegate
{
public:
    virtual void* process(FnKat::FnScenegraphIterator sgIterator, void* optionalInput);
    virtual ~PRManCurvesScenegraphLocationDelegate();
    static void flush();
    static PRManCurvesScenegraphLocationDelegate* create();
    virtual std::string getSupportedRenderer() const;
    virtual void fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const;
};

#endif

