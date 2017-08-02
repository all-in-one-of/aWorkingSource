// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <FnScenegraphGenerator/plugin/FnScenegraphGenerator.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnDataBuilder.h>

#include <iostream>
#include <string.h>
#include <sstream>



class SphereMakerContext : public FnKat::ScenegraphContext
{
private:
    int _numSphereMakers;
    int _current;
public:
    SphereMakerContext(int numSphereMakers, int current) : _numSphereMakers(numSphereMakers), _current(current) {}
    virtual ~SphereMakerContext() {}

        FnKat::ScenegraphContext *getFirstChild() const {return 0x0;}

    FnKat::ScenegraphContext *getNextSibling() const
    {
        int sib = _current+1;
        if (sib < _numSphereMakers)
        {
            return new SphereMakerContext(_numSphereMakers, sib);
        }
        else
        {
            return 0x0;
        }
    }

    void getLocalAttrNames(std::vector<std::string> *names) const
    {
        names->clear();
        names->push_back("name");
        names->push_back("type");
        names->push_back("sceneGraphGenerator");
        names->push_back("xform");
    }

    FnKat::Attribute getLocalAttr(const std::string & name) const
    {
        if (name == "name")
        {
            std::ostringstream os;
            os << "sphereMaker_" << _current;
            return FnKat::StringAttribute(os.str().c_str());
        }
        if (name == "type")
        {
            return FnKat::StringAttribute("scenegraph generator");
        }
        if (name == "scenegraphGenerator")
        {
            FnKat::GroupBuilder gb;
            gb.set("generatorType", FnKat::StringAttribute("SphereMaker"));
            gb.set("args.numSpheres", FnKat::IntAttribute(5));

            // Adds resolve ids to the children scenegraphGenerators
            // Specifically adds a common 'sphereMaker' id and a 'sphere_x'
            // encoding the current index
            std::ostringstream os;
            os << "sphere_" << _current;

            std::vector<std::string> sb;
            sb.push_back("sphereMaker");
            sb.push_back(os.str());

            gb.set("resolveIds", FnKat::StringAttribute(sb));

            return gb.build();
        }
        if (name == "xform")
        {
            FnKat::GroupBuilder gb;
            double translate[] = {0, 20 * _current, 0};
            gb.set("translate", FnKat::DoubleAttribute(translate, 3, 3));
            return gb.build();
        }
        return FnKat::Attribute();
    }
};

class RootContext : public FnKat::ScenegraphContext
{
public:
    RootContext(int numSphereMakers): _numSphereMakers(numSphereMakers) {}
    virtual ~RootContext() {}

    FnKat::ScenegraphContext *getFirstChild() const
    {
        if (_numSphereMakers)
        {
            return new SphereMakerContext(_numSphereMakers, 0);
        }

        return 0x0;
    }

    FnKat::ScenegraphContext *getNextSibling() const {return 0x0;}

    void getLocalAttrNames(std::vector<std::string> *names) const
    {
        names->clear();
        names->push_back("type");
        names->push_back("xform");
    }

    FnKat::Attribute getLocalAttr(const std::string & name) const
    {
        if (name == "type") return FnKat::StringAttribute("group");
        if (name == "xform")
        {
            FnKat::GroupBuilder gb;
            double translate[] = {0, 0, 0};
            gb.set("translate", FnKat::DoubleAttribute(translate, 3, 3));
            return gb.build();
        }

        return FnKat::Attribute();
    }

private:
    int _numSphereMakers;
};

class SphereMakerMaker : public FnKat::ScenegraphGenerator
{
public:
    SphereMakerMaker(): _numSphereMakers(0) {}
    virtual ~SphereMakerMaker() {}

    static FnKat::ScenegraphGenerator* create()
    {
        return (FnKat::ScenegraphGenerator*)new SphereMakerMaker();
    }

    static void flush() {}

    static FnKat::GroupAttribute getArgumentTemplate()
    {
        FnKat::GroupBuilder gb;
        gb.set("numSphereMakers", FnKat::IntAttribute(2));
        return gb.build();
    }

    bool setArgs(FnKat::GroupAttribute args)
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
        FnKat::IntAttribute numSphereMakersAttr = args.getChildByName("numSphereMakers");
        _numSphereMakers = numSphereMakersAttr.getValue(2, false);

        return true;
    }

    FnKat::ScenegraphContext * getRoot()
    {
        return new RootContext(_numSphereMakers);
    }

private:
    int _numSphereMakers;

};


DEFINE_SGG_PLUGIN(SphereMakerMaker)

void registerPlugins()
{
    std::cout << "[LCA PLUGIN] Register SphereMakerMaker beta1.0" << std::endl;
    REGISTER_PLUGIN(SphereMakerMaker, "SphereMakerMaker", 0, 1);
}
