// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef _CUBE_MAKER_H_
#define _CUBE_MAKER_H_

#include <FnScenegraphGenerator/plugin/FnScenegraphGenerator.h>

#include <iostream>
#include <string.h>
#include <sstream>

extern float points[];
extern int vertexList[];
extern int startIndex[];



// ----------------------------------------------------------------------------
// Cube Maker
// ----------------------------------------------------------------------------

/**
 * Base class of the CubeMaker SGG plug-in.
 *
 * Inherits from \c ScenegraphGenerator and is responsible for creating the
 * root context of the scene graph portion that is created by the plug-in.
 */
class CubeMaker : public FnKat::ScenegraphGenerator
{

public:

    /**
     * Constructor.
     */
    CubeMaker();

    /**
     * Destructor.
     */
    virtual ~CubeMaker();

    /**
     * Factory method.
     *
     * \return A new instance of the \C CubeMaker class.
     */
    static FnKat::ScenegraphGenerator* create(void);

    /**
     * \return A group attribute that defines the parameters of the SGG plug-in
     *     that will appear as part of the \b ScenegraphGeneratorSetup node's
     *     parameter interface, just below the generatorType drop-down
     *     parameter.
     */
    static FnKat::GroupAttribute getArgumentTemplate();

    /**
     * Applies the parameter values contained in the given argument structure
     * to the scene graph generator plug-in.
     *
     * \return \c true if the parameter values were applied successfully,
     *     otherwise \c false.
     */
    bool setArgs(FnKat::GroupAttribute args);

    /**
     * Creates the root context of the scene graph portion created by the
     * plug-in.
     *
     * \return A new \C ScenegraphContext instance representing the root of the
     *     scene graph portion created by this plug-in.
     */
    FnKat::ScenegraphContext* getRoot();

    /**
     * Clears allocated memory and reloads data from file (if any).
     *
     * Is called when flushing Katana's caches, e.g. by clicking the <em>Flush
     * Product Caches</em> button in the Menu Bar.
     */
    static void flush();

private:

    /**
     * The number of scene graph locations for polygonal cubes to create.
     */
    int _numberOfCubes;

    /**
     * Flag that controls whether the polygonal cubes should be incrementally
     * rotated.
     */
    bool _rotateCubes;

};


// ----------------------------------------------------------------------------
// Cube Root Context
// ----------------------------------------------------------------------------

class CubeRootContext : public FnKat::ScenegraphContext
{

public:

    /**
     * Constructor.
     *
     * \param numberOfCubes The number of scene graph locations for polygonal
     *     cubes to create.
     * \param rotateCubes Flag that controls whether the polygonal cubes should
     *     be incrementally rotated.
     */
    CubeRootContext(int numberOfCubes, bool rotateCubes);

    /**
     * Destructor.
     */
    virtual ~CubeRootContext();

    /**
     * \return An instance of the \c ScenegraphContext class that represents
     *     the first child location of the location represented by this
     *     instance, or \c 0x0 if this location does not contain any child
     *     locations.
     */
    FnKat::ScenegraphContext* getFirstChild() const;

    /**
     * \return An instance of the \c ScenegraphContext class that represents
     *     the next sibling location of the location represented by this
     *     instance, or \c 0x0 if this location does not have any sibling
     *     locations.
     */
    FnKat::ScenegraphContext* getNextSibling() const;

    /**
     * Fills the given list of names of attributes according to the attributes
     * that should live on the scene graph location represented by this
     * instance.
     *
     * \param[out] names The list of names of attributes to clear and fill.
     */
    void getLocalAttrNames(std::vector<std::string>* names) const;

    /**
     * \return An instance of the \c Attribute class representing the scene
     *     graph attribute with the given name, or \c Attribute() if no
     *     attribute of the given name lives on the location represented by
     *     this instance.
     */
    FnKat::Attribute getLocalAttr(const std::string& name) const;

private:

    /**
     * The number of scene graph locations for polygonal cubes to create.
     */
    int _numberOfCubes;

    /**
     * Flag that controls whether the polygonal cubes should be incrementally
     * rotated.
     */
    bool _rotateCubes;

};


// ----------------------------------------------------------------------------
// Cube Context
// ----------------------------------------------------------------------------

class CubeContext : public FnKat::ScenegraphContext
{

public:

    /**
     * Constructor.
     *
     * \param numberOfCubes The number of scene graph locations for polygonal
     *     cubes to create.
     * \param rotateCubes Flag that controls whether the polygonal cubes should
     *     be incrementally rotated.
     * \param index The index of new context underneath its parent context.
     */
    CubeContext(int numberOfCubes, bool rotateCubes, int index);

    /**
     * Destructor.
     */
    virtual ~CubeContext();

    /**
     * \return An instance of the \c ScenegraphContext class that represents
     *     the first child location of the location represented by this
     *     instance, or \c 0x0 if this location does not contain any child
     *     locations.
     */
    FnKat::ScenegraphContext* getFirstChild() const;

    /**
     * \return An instance of the \c ScenegraphContext class that represents
     *     the next sibling location of the location represented by this
     *     instance, or \c 0x0 if this location does not have any sibling
     *     locations.
     */
    FnKat::ScenegraphContext* getNextSibling() const;

    /**
     * Fills the given list of names of attributes according to the attributes
     * that should live on the scene graph location represented by this
     * instance.
     *
     * \param[out] names The list of names of attributes to clear and fill.
     */
    void getLocalAttrNames(std::vector<std::string>* names) const;

    /**
     * \return An instance of the \c Attribute class representing the scene
     *     graph attribute with the given name, or \c Attribute() if no
     *     attribute of the given name lives on the location represented by
     *     this instance.
     */
    FnKat::Attribute getLocalAttr(const std::string& name) const;

private:

    /**
     * The number of scene graph locations for polygonal cubes to create.
     */
    int _numberOfCubes;

    /**
     * Flag that controls whether the polygonal cubes should be incrementally
     * rotated.
     */
    bool _rotateCubes;

    /**
     * The index of this context underneath its parent context.
     */
    int _index;

};


#endif  // _CUBE_MAKER_H_
