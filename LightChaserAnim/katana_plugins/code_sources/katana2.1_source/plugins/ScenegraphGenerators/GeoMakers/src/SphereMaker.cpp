// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <SphereMaker.h>
#include <FnScenegraphGenerator/plugin/FnScenegraphGenerator.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnDataBuilder.h>

#include <iostream>
#include <string.h>
#include <sstream>

SphereContext::SphereContext(int numSpheres, int current, FnKat::GroupAttribute timeSlice)
    : _numSpheres(numSpheres)
    , _current(current)
    , _timeSlice(timeSlice)
{}

SphereContext::~SphereContext() {}


FnKat::ScenegraphContext *SphereContext::getFirstChild() const
{
    return 0x0;
}


FnKat::ScenegraphContext *SphereContext::getNextSibling() const
{
    int sib = _current+1;
    if (sib < _numSpheres)
    {
        return new SphereContext(_numSpheres, sib, _timeSlice);
    }
    else
    {
        return 0x0;
    }
}


void SphereContext::getLocalAttrNames(std::vector<std::string> *names) const
{
    names->clear();
    names->push_back("name");
    names->push_back("type");
    names->push_back("xform");
    names->push_back("geometry");
}


FnKat::Attribute SphereContext::getLocalAttr(const std::string & name) const
{
    if (name == "name")
    {
        std::ostringstream os;
        os << "sphere_" << _current;
        return FnKat::StringAttribute(os.str().c_str());
    }
    if (name == "type")
    {
        return FnKat::StringAttribute("sphere");
    }
    if (name == "geometry")
    {
        FnKat::GroupBuilder gb;
        gb.set("radius", FnKat::DoubleAttribute(1.0));
        gb.set("id", FnKat::IntAttribute(_current));
        return gb.build();
    }
    if (name == "xform")
    {
        // Gets shutter information
        FnKat::IntAttribute numSamplesAttr = _timeSlice.getChildByName("numSamples");
        FnKat::FloatAttribute shutterOpenAttr = _timeSlice.getChildByName("shutterOpen");
        FnKat::FloatAttribute shutterCloseAttr = _timeSlice.getChildByName("shutterClose");
        FnKat::FloatAttribute currTimeAttr = _timeSlice.getChildByName("currentTime");

        const int numSamples = numSamplesAttr.isValid() ? numSamplesAttr.getValue() : 1;
        const double shutterOpen = shutterOpenAttr.isValid() ? shutterOpenAttr.getValue() : 0.0;
        const double shutterClose = shutterCloseAttr.isValid() ? shutterCloseAttr.getValue() : 1.0;
        const double currTime = currTimeAttr.isValid() ? currTimeAttr.getValue() : 1.0;

        const double dt = (numSamples == 1) ?
                            0.0 :
                            (shutterClose - shutterOpen) / (numSamples - 1);

        FnKat::GroupBuilder gb;
        FnKat::DoubleBuilder db(3);

        for (int i = 0; i < numSamples; ++i)
        {
            const double t = i * dt;
            
            // Here we assume that velocity = 1 unit/frame !
            db.push_back(_current * 5.0 - 15.0 + (currTime + t), t);
            db.push_back(0.0, t);
            db.push_back(-40.0, t);
        }
        gb.set("translate", db.build());

        const double val = (_current + 1.0) * 0.5;
        const double scaleValues[] = {val, val, val};
        gb.set("scale", FnKat::DoubleAttribute(scaleValues, 3, 3));

        gb.setGroupInherit(false);
        return gb.build();
    }
    return FnKat::Attribute();
}


SphereRootContext::SphereRootContext(int numSpheres, FnKat::GroupAttribute timeSlice)
    : _numSpheres(numSpheres)
    , _timeSlice(timeSlice)
{
}

SphereRootContext::~SphereRootContext() {}


FnKat::ScenegraphContext *SphereRootContext::getFirstChild() const
{
    if (_numSpheres)
    {
        return new SphereContext(_numSpheres, 0, _timeSlice);
    }

    return 0x0;
}


FnKat::ScenegraphContext *SphereRootContext::getNextSibling() const
{
    return 0x0;
}


void SphereRootContext::getLocalAttrNames(std::vector<std::string> *names) const
{
    names->clear();
    names->push_back("type");
    names->push_back("xform");
}

FnKat::Attribute SphereRootContext::getLocalAttr(const std::string & name) const
{
    if (name == "type") return FnKat::StringAttribute("group");
    if (name == "xform")
    {
        FnKat::GroupBuilder gb;
        double translate[] = {0, 0, -10};
        gb.set("translate", FnKat::DoubleAttribute(translate, 3, 3));
        gb.setGroupInherit(false);
        return gb.build();
    }

    return FnKat::Attribute();
}


SphereMaker::SphereMaker(): _numSpheres(0) {}


SphereMaker::~SphereMaker() {}

FnKat::ScenegraphGenerator* SphereMaker::create(void)
{
    return (FnKat::ScenegraphGenerator*)new SphereMaker();
}



FnKat::GroupAttribute SphereMaker::getArgumentTemplate()
{
    FnKat::GroupBuilder gb;
    gb.set("numSpheres", FnKat::IntAttribute(20));
    double testPoint[] = {1,2,3};
    gb.set("testPoint", FnKat::DoubleAttribute(testPoint, 3, 3));
    gb.set("testString", FnKat::StringAttribute(""));
    return gb.build();
}


bool SphereMaker::setArgs(FnKat::GroupAttribute args)
{
    if (!checkArgs(args)) return false;

    // Migration Notice regarding default values:
    // Versions of Katana prior to 2.0 would provide default values for all
    // SGG arguments if they had not been specified locally by the user.
    // This is no longer the case and you must now specify the default
    // value yourself.
    //
    // This can be achieved using the FnAttribute library for example:
    //
    // FnKat::IntAttribute attr = args.getChildByName("attr");
    // int value = attr.getValue(20,      // DEFAULT VALUE
    //                           false);  // THROW ERROR IF DEFAULT VALUE NOT
    //                                    // AVAILABLE OTHERWISE RETURN DEFAULT
    //                                    // VALUE

    FnKat::IntAttribute numSpheresAttr = args.getChildByName("numSpheres");
    _numSpheres = numSpheresAttr.getValue(20, false);
    _timeSlice = args.getChildByName("system.timeSlice");
    return true;
}


FnKat::ScenegraphContext * SphereMaker::getRoot()
{
    return new SphereRootContext(_numSpheres, _timeSlice);
}
