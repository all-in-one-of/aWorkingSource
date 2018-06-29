// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef _SPHERE_MAKER_H_
#define _SPHERE_MAKER_H_

#include <FnScenegraphGenerator/plugin/FnScenegraphGenerator.h>

#include <iostream>
#include <string.h>
#include <sstream>


class SphereContext : public FnKat::ScenegraphContext
{
private:
    int _numSpheres;
    int _current;
    FnKat::GroupAttribute _timeSlice;

public:
    SphereContext(int numSpheres, int current, FnKat::GroupAttribute timeSlice);
    virtual ~SphereContext();

    FnKat::ScenegraphContext *getFirstChild() const;
    FnKat::ScenegraphContext *getNextSibling() const;

    void getLocalAttrNames(std::vector<std::string> *names) const;

    FnKat::Attribute getLocalAttr(const std::string & name) const;
};


class SphereRootContext : public FnKat::ScenegraphContext
{
public:
    SphereRootContext(int numSpheres, FnKat::GroupAttribute timeSlice);
    virtual ~SphereRootContext();

    FnKat::ScenegraphContext *getFirstChild() const;

    FnKat::ScenegraphContext *getNextSibling() const;

    void getLocalAttrNames(std::vector<std::string> *names) const;

    FnKat::Attribute getLocalAttr(const std::string & name) const;

private:
    int _numSpheres;
    FnKat::GroupAttribute _timeSlice;
};

class SphereMaker : public FnKat::ScenegraphGenerator
{
public:
    SphereMaker();
    virtual ~SphereMaker();

    static FnKat::ScenegraphGenerator* create(void);

    static FnKat::GroupAttribute getArgumentTemplate();

    bool setArgs(FnKat::GroupAttribute args);

    FnKat::ScenegraphContext * getRoot();

    static void flush() {}

private:
    int _numSpheres;
    FnKat::GroupAttribute _timeSlice;

};



#endif
