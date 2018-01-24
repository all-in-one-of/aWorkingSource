// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

// Copyright (c) 2011-2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <CubeMaker.h>
#include <FnScenegraphGenerator/plugin/FnScenegraphGenerator.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnLogging/FnLogging.h>

#include <iostream>
#include <string.h>
#include <sstream>


// Set up point data and lists of vertices and start indices
float points[] = {-0.5f,-0.5f, 0.5f,
                   0.5f,-0.5f, 0.5f,
                  -0.5f, 0.5f, 0.5f,
                   0.5f, 0.5f, 0.5f,
                  -0.5f, 0.5f,-0.5f,
                   0.5f, 0.5f,-0.5f,
                  -0.5f,-0.5f,-0.5f,
                   0.5f,-0.5f,-0.5f};
int vertexList[] = {2, 3, 1, 0,
                    4, 5, 3, 2,
                    6, 7, 5, 4,
                    0, 1, 7, 6,
                    3, 5, 7, 1,
                    4, 2, 0, 6};
int startIndex[] = {0, 4, 8, 12, 16, 20, 24};

// Set up a logger
FnLogSetup("CubeMaker")


// ----------------------------------------------------------------------------
// Cube Maker
// ----------------------------------------------------------------------------

/**
 * Constructor.
 */
CubeMaker::CubeMaker() :
    _numberOfCubes(0),
    _rotateCubes(false)
{
    // empty
}

/**
 * Destructor.
 */
CubeMaker::~CubeMaker()
{
    // empty
}

/**
 * Factory method.
 *
 * \return A new instance of the \C CubeMaker class.
 */
FnKat::ScenegraphGenerator* CubeMaker::create(void)
{
    return new CubeMaker();
}

/**
 * \return A group attribute that defines the parameters of the SGG plug-in
 *     that will appear as part of the \b ScenegraphGeneratorSetup node's
 *     parameter interface, just below the generatorType drop-down
 *     parameter.
 */
FnKat::GroupAttribute CubeMaker::getArgumentTemplate()
{
    FnKat::GroupBuilder rotateCubesHints;
    rotateCubesHints.set("widget", FnKat::StringAttribute("checkBox"));

    FnKat::GroupBuilder gb;
    gb.set("numberOfCubes", FnKat::IntAttribute(20));
    gb.set("rotateCubes", FnKat::IntAttribute(0));
    gb.set("rotateCubes__hints", rotateCubesHints.build());
    return gb.build();
}

/**
 * Applies the parameter values contained in the given argument structure
 * to the scene graph generator plug-in.
 *
 * \return \c true if the parameter values were applied successfully,
 *     otherwise \c false.
 */
bool CubeMaker::setArgs(FnKat::GroupAttribute args)
{
    if (!checkArgs(args))
        return false;

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

    // Apply the number of cubes
    FnKat::IntAttribute numberOfCubesAttr = args.getChildByName(
        "numberOfCubes");
    _numberOfCubes = numberOfCubesAttr.getValue(20, false);

    // Update the rotation flag
    FnKat::IntAttribute rotateCubesAttr = args.getChildByName("rotateCubes");
    _rotateCubes = bool(rotateCubesAttr.getValue(0, false));

    return true;
}

/**
 * Creates the root context of the scene graph portion created by the
 * plug-in.
 *
 * \return A new \C ScenegraphContext instance representing the root of the
 *     scene graph portion created by this plug-in.
 */
FnKat::ScenegraphContext* CubeMaker::getRoot()
{
    return new CubeRootContext(_numberOfCubes, _rotateCubes);
}

/**
 * Clears allocated memory and reloads data from file (if any).
 *
 * Is called when flushing Katana's caches, e.g. by clicking the <em>Flush
 * Product Caches</em> button in the Menu Bar.
 */
void CubeMaker::flush()
{
    // empty
}


// ----------------------------------------------------------------------------
// Cube Root Context
// ----------------------------------------------------------------------------

/**
 * Constructor.
 *
 * \param numberOfCubes The number of scene graph locations for polygonal
 *     cubes to create.
 * \param rotateCubes Flag that controls whether the polygonal cubes should
 *     be incrementally rotated.
 */
CubeRootContext::CubeRootContext(int numberOfCubes, bool rotateCubes) :
    _numberOfCubes(numberOfCubes),
    _rotateCubes(rotateCubes)
{
    // empty
}

/**
 * Destructor.
 */
CubeRootContext::~CubeRootContext()
{
    // empty
}

/**
 * \return An instance of the \c ScenegraphContext class that represents
 *     the first child location of the location represented by this
 *     instance, or \c 0x0 if this location does not contain any child
 *     locations.
 */
FnKat::ScenegraphContext* CubeRootContext::getFirstChild() const
{
    if (_numberOfCubes > 0)
    {
        return new CubeContext(_numberOfCubes, _rotateCubes, 0);
    }

    return 0x0;
}

/**
 * \return An instance of the \c ScenegraphContext class that represents
 *     the next sibling location of the location represented by this
 *     instance, or \c 0x0 if this location does not have any sibling
 *     locations.
 */
FnKat::ScenegraphContext* CubeRootContext::getNextSibling() const
{
    return 0x0;
}

/**
 * Fills the given list of names of attributes according to the attributes
 * that should live on the scene graph location represented by this
 * instance.
 *
 * \param[out] names The list of names of attributes to clear and fill.
 */
void CubeRootContext::getLocalAttrNames(std::vector<std::string>* names) const
{
    names->clear();
    names->push_back("type");
    names->push_back("xform");
}

/**
 * \return An instance of the \c Attribute class representing the scene
 *     graph attribute with the given name, or \c Attribute() if no
 *     attribute of the given name lives on the location represented by
 *     this instance.
 */
FnKat::Attribute CubeRootContext::getLocalAttr(const std::string& name) const
{
    if (name == "type")
    {
        return FnKat::StringAttribute("group");
    }
    else if (name == "xform")
    {
        FnKat::GroupBuilder gb;
        double translate[] = {0, 0, -10};
        gb.set("translate", FnKat::DoubleAttribute(translate, 3, 3));
        gb.setGroupInherit(false);
        return gb.build();
    }

    return FnKat::Attribute();
}


// ----------------------------------------------------------------------------
// Cube Context
// ----------------------------------------------------------------------------

/**
 * Constructor.
 *
 * \param numberOfCubes The number of scene graph locations for polygonal
 *     cubes to create.
 * \param rotateCubes Flag that controls whether the polygonal cubes should
 *     be incrementally rotated.
 * \param index The index of new context underneath its parent context.
 */
CubeContext::CubeContext(int numberOfCubes, bool rotateCubes, int index) :
    _numberOfCubes(numberOfCubes),
    _rotateCubes(rotateCubes),
    _index(index)
{
    // empty
}

/**
 * Destructor.
 */
CubeContext::~CubeContext()
{
    // empty
}

/**
 * \return An instance of the \c ScenegraphContext class that represents
 *     the first child location of the location represented by this
 *     instance, or \c 0x0 if this location does not contain any child
 *     locations.
 */
FnKat::ScenegraphContext* CubeContext::getFirstChild() const
{
    return 0x0;
}

/**
 * \return An instance of the \c ScenegraphContext class that represents
 *     the next sibling location of the location represented by this
 *     instance, or \c 0x0 if this location does not have any sibling
 *     locations.
 */
FnKat::ScenegraphContext* CubeContext::getNextSibling() const
{
    if (_index < _numberOfCubes - 1)
    {
        return new CubeContext(_numberOfCubes, _rotateCubes, _index + 1);
    }

    return 0x0;
}

/**
 * Fills the given list of names of attributes according to the attributes
 * that should live on the scene graph location represented by this
 * instance.
 *
 * \param[out] names The list of names of attributes to clear and fill.
 */
void CubeContext::getLocalAttrNames(std::vector<std::string>* names) const
{
    names->clear();
    names->push_back("name");
    names->push_back("type");
    names->push_back("xform");
    names->push_back("geometry");
}

/**
 * \return An instance of the \c Attribute class representing the scene
 *     graph attribute with the given name, or \c Attribute() if no
 *     attribute of the given name lives on the location represented by
 *     this instance.
 */
FnKat::Attribute CubeContext::getLocalAttr(const std::string& name) const
{
    if (name == "name")
    {
        std::ostringstream os;
        os << "cube_" << _index;
        return FnKat::StringAttribute(os.str().c_str());
    }
    else if (name == "type")
    {
        return FnKat::StringAttribute("polymesh");
    }
    else if (name == "geometry")
    {
        FnKat::GroupBuilder gb;

        FnKat::GroupBuilder gbPoint;
        gbPoint.set("P",
                    FnKat::FloatAttribute(points,
                                          sizeof(points) / sizeof(float), 3));
        gb.set("point", gbPoint.build());

        FnKat::GroupBuilder gbPoly;
        gbPoly.set("vertexList",
                   FnKat::IntAttribute(vertexList,
                                       sizeof(vertexList) / sizeof(int), 1));
        gbPoly.set("startIndex",
                   FnKat::IntAttribute(startIndex,
                                       sizeof(startIndex) / sizeof(int), 1));
        gb.set("poly", gbPoly.build());

        return gb.build();
    }
    else if (name == "xform")
    {
        FnKat::GroupBuilder gb;

        double translateValues[] = {5.0 * _index - 15.0, 0.0, -40.0};
        gb.set("translate", FnKat::DoubleAttribute(translateValues, 3, 3));

        double rx = _rotateCubes ? 15.0 * _index : 0.0;
        double rxValues[] = {rx,  1.0, 0.0, 0.0};
        double ryValues[] = {0.0, 0.0, 1.0, 0.0};
        double rzValues[] = {0.0, 0.0, 0.0, 1.0};
        gb.set("rotateX", FnKat::DoubleAttribute(rxValues, 4, 4));
        gb.set("rotateY", FnKat::DoubleAttribute(ryValues, 4, 4));
        gb.set("rotateZ", FnKat::DoubleAttribute(rzValues, 4, 4));

        double scale = (_index + 1.0) * 0.5;
        double scaleValues[] = {scale, scale, scale};
        gb.set("scale", FnKat::DoubleAttribute(scaleValues, 3, 3));

        gb.setGroupInherit(false);
        return gb.build();
    }

    return FnKat::Attribute();
}
