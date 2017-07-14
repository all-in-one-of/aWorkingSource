// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef INCLUDED_INSTANCE_SCENEGRAPHXML_IN_H
#define INCLUDED_INSTANCE_SCENEGRAPHXML_IN_H

#include <FnScenegraphGenerator/plugin/FnScenegraphGenerator.h>

#include <Scene.h>
#include <ErrorReport.h>

//#include <ptr/shared_ptr.h>
#include <boost/smart_ptr/shared_ptr.hpp>


namespace instance_scene
{

struct ProcArgs
{
    ProcArgs()
    : frame(1)
    , animated(false)
    {
    }

    /// the current frame of the scene
    int frame;
    /// if the system is asking for more than one sample -- i.e. within a render
    /// rather than an interactive session, we'll want to give the iterator
    /// contexts a heads-up that they should provide the next frame as a sample
    /// also.
    bool animated;
};

/// no ownership or copying issues if we always send this along as a shared_ptr
typedef boost::shared_ptr<ProcArgs> ProcArgsRcPtr;


class ScenegraphXml_In : public FnKat::ScenegraphGenerator
{
public:
    ScenegraphXml_In();
    virtual ~ScenegraphXml_In();

    static FnKat::ScenegraphGenerator* create();

    static FnKat::GroupAttribute getArgumentTemplate();
    bool setArgs(FnKat::GroupAttribute args);
    FnKat::ScenegraphContext * getRoot();

    static void flush();

private:
    ProcArgsRcPtr shared_args;
    std::string filepath;
};


///////////////////////////////////////////////////////////////////////////////
/// Root Context
///
class RootIteratorContext : public FnKat::ScenegraphContext
{
public:
    RootIteratorContext(ProcArgsRcPtr args, const std::string & filepath);

    /// All iterator contexts must be able to clone themselves.
    RootIteratorContext(const RootIteratorContext &rhs);

    /// Will return a SceneIteratorContext built from m_filepath
    FnKat::ScenegraphContext *getFirstChild() const;

    /// The top-level iterator cannot build siblings so we return 0
    FnKat::ScenegraphContext *getNextSibling() const {return 0x0;}

    void getLocalAttrNames(std::vector<std::string> *names) const;
    FnKat::Attribute getLocalAttr(const std::string & name) const;

private:
    ProcArgsRcPtr m_args;
    std::string m_filepath;
};

///////////////////////////////////////////////////////////////////////////////
/// Scene Context
///
typedef boost::shared_ptr<FnKat::ScenegraphContext> IteratorContextRcPtr;

class SceneIteratorContext : public FnKat::ScenegraphContext
{
public:
    /// Takes in path to a Scene file and loads it upon construction
    /// Since we're called over and over, we need to cache the data. In this
    /// case, the cache is handled in the Scene class.
    SceneIteratorContext(ProcArgsRcPtr  args, const std::string & filepath);

    /// simple copy constructor and clone
    SceneIteratorContext(const SceneIteratorContext &rhs);

    /// its immediate children are instances or instance groups
    FnKat::ScenegraphContext *getFirstChild() const;

    /// Scenes don't themselves have siblings as their either just below
    /// the root or the child of an instance
    FnKat::ScenegraphContext *getNextSibling() const;

    /// scenes optionally have bounds -- which we advertise and provide
    /// as a "bound" attribute. In this example, scenes are always named
    /// "scene" but could have a field for their name. They are built
    /// here as scenegraph locations of type "assembly"
    void getLocalAttrNames(std::vector<std::string> *names) const;
    FnKat::Attribute getLocalAttr(const std::string & name) const;

private:
    ProcArgsRcPtr m_args;
    std::string m_filepath;
    SceneRcPtr m_scene;
    IteratorContextRcPtr m_curLocationIterator;
};


///////////////////////////////////////////////////////////////////////////////
/// Abstract base class for the three types of instances:
/// (scene, group and geometry)
/// Since they all have names; InstanceGroup parents; are responsible for providing
/// their next sibling; and can return optional "xform", 'bound', 'proxies' and
/// 'scenegraphXmlAttrs', this functionality is shared in a base class
class InstanceBaseIteratorContext : public FnKat::ScenegraphContext
{
public:
    /// every instance has a parent InstanceGroup and knows its own index
    /// within its list of siblings
    InstanceBaseIteratorContext(ProcArgsRcPtr  args,
            InstanceBaseRcPtr baseInstance, InstanceGroupRcPtr parentGroup,
                    size_t index);

    /// standard copy constructor and clone
    InstanceBaseIteratorContext(const InstanceBaseIteratorContext &rhs);

    /// firstChild differs among the subclass so it's undefined here
    FnKat::ScenegraphContext *getFirstChild() const = 0;

    /// All instances have the same nextSibling behavior -- get index+1
    /// from their parent group
    FnKat::ScenegraphContext *getNextSibling() const;

    /// provides name and optional xform. Subclasses provide "type"
    void getLocalAttrNames(std::vector<std::string> *names) const;
    FnKat::Attribute getLocalAttr(const std::string & name) const;

protected:
    InstanceBaseRcPtr getInstance();
    ProcArgsRcPtr  m_args;

private:
    InstanceGroupRcPtr m_parentGroup;
    InstanceBaseRcPtr m_baseInstance;
    size_t m_index;
};


///////////////////////////////////////////////////////////////////////////////
/// represents an InstanceGroup. Extends InstanceBaseIteratorContext to
/// override "type"; provide and a first instance child
/// Creates a scenegraph location of type "instance group"
class InstanceGroupIteratorContext : public InstanceBaseIteratorContext
{
public:
    InstanceGroupIteratorContext(ProcArgsRcPtr  args,
            InstanceGroupRcPtr instanceGroup, InstanceGroupRcPtr parentGroup,
                    size_t index);

    InstanceGroupIteratorContext(const InstanceGroupIteratorContext &rhs);

    FnKat::ScenegraphContext *getFirstChild() const;

private:
    InstanceGroupRcPtr m_instanceGroup;
};

///////////////////////////////////////////////////////////////////////////////
/// An instance that will return a SceneIteratorContext or GeometryIteratorContext
// as its first child.  Create a scenegraph location of type "assembly instance"
// or "component instance"
class InstanceReferenceIteratorContext : public InstanceBaseIteratorContext
{
public:
    InstanceReferenceIteratorContext(ProcArgsRcPtr  args,
            InstanceReferenceRcPtr instanceScene, InstanceGroupRcPtr parentGroup,
                    size_t index);

    InstanceReferenceIteratorContext(const InstanceReferenceIteratorContext &rhs);
    FnKat::ScenegraphContext *getFirstChild() const;

private:
    InstanceReferenceRcPtr m_instanceReference;
};

///////////////////////////////////////////////////////////////////////////////
/// Defines a katana location of type "procedural" and fills in the argument
/// attributes to instantiate an HDF_In procedural. Katana will recursively
/// resolve locations of type "procedural" as generated by other procedurals.
/// This lets us keep format-specific code out of this procedural.
/// It's very cool.
class HDFIteratorContext : public FnKat::ScenegraphContext
{
public:
    HDFIteratorContext(ProcArgsRcPtr  args, const std::string & filepath);
    HDFIteratorContext(const HDFIteratorContext &rhs);
    FnKat::ScenegraphContext *getFirstChild() const {return 0x0;}
    FnKat::ScenegraphContext *getNextSibling() const {return 0x0;}

    void getLocalAttrNames(std::vector<std::string> *names) const;
    FnKat::Attribute getLocalAttr(const std::string & name) const;

private:
    ProcArgsRcPtr m_args;
    std::string m_filepath;
};

//////////////////////////////////////////////////////////////////////////
/// Procedural for Alembic
///
class ABCIteratorContext : public FnKat::ScenegraphContext
{
public:
    ABCIteratorContext(ProcArgsRcPtr  args, const std::string & filepath);
    ABCIteratorContext(const ABCIteratorContext &rhs);
    FnKat::ScenegraphContext *getFirstChild() const {return 0x0;}
    FnKat::ScenegraphContext *getNextSibling() const {return 0x0;}

    void getLocalAttrNames(std::vector<std::string> *names) const;
    FnKat::Attribute getLocalAttr(const std::string & name) const;

private:
    ProcArgsRcPtr m_args;
    std::string m_filepath;
    mutable FnKat::GroupAttribute m_sgAttr;
};

class ProceduralIteratorContext : public FnKat::ScenegraphContext
{
public:
    ProceduralIteratorContext(ProcArgsRcPtr  args, const std::string & filepath,
                            std::vector<ProceduralArgumentRcPtr> procArgs);
    ProceduralIteratorContext(const ProceduralIteratorContext &rhs);
    FnKat::ScenegraphContext *getFirstChild() const {return 0x0;}
    FnKat::ScenegraphContext *getNextSibling() const {return 0x0;}

    void getLocalAttrNames(std::vector<std::string> *names) const;
    FnKat::Attribute getLocalAttr(const std::string & name) const;

private:
    ProcArgsRcPtr m_args;
    std::string m_filepath;
    std::vector<ProceduralArgumentRcPtr> m_procArgs;
};

}

#endif /* INCLUDED_INSTANCE_SCENEGRAPHXML_IN_H */
