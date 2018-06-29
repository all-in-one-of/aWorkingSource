// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef PRMANPROCEDURALSCENEGRAPHLOCATIONDELEGATE_H
#define PRMANPROCEDURALSCENEGRAPHLOCATIONDELEGATE_H

#include <FnRender/plugin/ScenegraphLocationDelegate.h>
#include <AttrList.h>


class PRManProceduralScenegraphLocationDelegate : public Foundry::Katana::Render::ScenegraphLocationDelegate
{
public:
    virtual void* process(FnKat::FnScenegraphIterator sgIterator, void* optionalInput);
    virtual ~PRManProceduralScenegraphLocationDelegate();
    static void flush();
    static PRManProceduralScenegraphLocationDelegate* create();
    virtual std::string getSupportedRenderer() const;
    virtual void fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const;
};

/*
 * Helper class to hold procedural args and output them in various format like
 * as string or typed attributes.
 *
 * We currently don't pass time samples to procedurals.
 */
class ProceduralArgs
{
public:
    template <typename T>
    void addArgument(const std::string& argName, const T attr, const bool mightBlur=false);
    std::string getProcString() const;
    const PRManProcedural::AttrList& getAttrList() const;

private:
    template <typename T>
    std::string buildNumericValueString(const T attr) const;

    PRManProcedural::AttrList m_attrList;
};

#endif

