#ifndef FRACTAL_GENERATOR_H
#define FRACTAL_GENERATOR_H

#include <FnScenegraphGenerator/plugin/FnScenegraphGenerator.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnDataBuilder.h>

#include <boost/shared_ptr.hpp>

namespace subdividedSpace
{

class SubdividedSpace : public Foundry::Katana::ScenegraphGenerator
{
public:

    struct Args
    {
        int maxdepth;
        int subdivisions;
    };
    typedef boost::shared_ptr<Args > ArgsPtr;

    SubdividedSpace();
    virtual ~SubdividedSpace();

    static Foundry::Katana::ScenegraphGenerator *create();

    static Foundry::Katana::GroupAttribute getArgumentTemplate();
    bool setArgs( Foundry::Katana::GroupAttribute args );

    Foundry::Katana::ScenegraphContext *getRoot();

    static void flush() {};

    private:

    ArgsPtr m_args;

};

class RootContext : public Foundry::Katana::ScenegraphContext
{
public:

    RootContext( SubdividedSpace::ArgsPtr args );
    RootContext( const RootContext &other );

    Foundry::Katana::ScenegraphContext *getFirstChild() const;
    Foundry::Katana::ScenegraphContext *getNextSibling() const { return 0x0; }

    void getLocalAttrNames( std::vector<std::string> *names ) const;
    Foundry::Katana::Attribute getLocalAttr( const std::string &name ) const;

    private :

    SubdividedSpace::ArgsPtr m_args;
};



class FractalContext : public Foundry::Katana::ScenegraphContext
{
public:

    FractalContext( SubdividedSpace::ArgsPtr args, unsigned int index, unsigned int depth );
    FractalContext( const FractalContext &other );

    Foundry::Katana::ScenegraphContext *getFirstChild() const;
    Foundry::Katana::ScenegraphContext *getNextSibling() const;

    void getLocalAttrNames( std::vector<std::string> *names ) const;
    Foundry::Katana::Attribute getLocalAttr( const std::string &name ) const;

    private :

    SubdividedSpace::ArgsPtr m_args;
    int m_index;
    int m_depth;
};

} /* end namespace subdividedSpace */

#endif // FRACTAL_GENERATOR_H
