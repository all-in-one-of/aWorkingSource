#include "SubdividedSpace.h"

#include <math.h>
#include <sstream>

#include <FnAttribute/FnGroupBuilder.h>

using namespace std;
using namespace Foundry::Katana;

namespace subdividedSpace
{

SubdividedSpace::SubdividedSpace()
{
}

SubdividedSpace::~SubdividedSpace()
{
}

ScenegraphGenerator *SubdividedSpace::create()
{
    return (ScenegraphGenerator *)new SubdividedSpace();
}

GroupAttribute SubdividedSpace::getArgumentTemplate()
{
    GroupBuilder groupBuilder;
    groupBuilder.set( "maxdepth", IntAttribute( 10 ) );
    groupBuilder.set( "subdivisions", IntAttribute( 1 ) );
    return groupBuilder.build();
}

bool SubdividedSpace::setArgs( GroupAttribute args )
{
    if ( !checkArgs(args) ) { return false; }

    m_args = ArgsPtr( new Args );

    IntAttribute depthAttr =  args.getChildByName( "maxdepth" );
    m_args->maxdepth = depthAttr.getValue(10, false);

    IntAttribute divisionsAttr =  args.getChildByName( "subdivisions" );
    m_args->subdivisions = divisionsAttr.getValue(1, false);

    return true;
}

ScenegraphContext *SubdividedSpace::getRoot()
{
    return new RootContext( m_args );
}


// RootContext

RootContext::RootContext( SubdividedSpace::ArgsPtr args )
    : m_args( args )
{
}

RootContext::RootContext( const RootContext &other )
{
    m_args = other.m_args;
}

void RootContext::getLocalAttrNames( std::vector<std::string> *names ) const
{
    names->clear();
    names->push_back( "type" );
    names->push_back( "bound" );
}

Foundry::Katana::Attribute RootContext::getLocalAttr( const std::string &name ) const
{
    if ( name == "type" )
    {
        return StringAttribute( "group" );
    }
    else if ( name == "bound" )
    {
        DoubleBuilder doubleBuilder(2);
        std::vector< double > &values = doubleBuilder.get(0);
        values.reserve( 6 );
        values.push_back( -0.5 ); //minx
        values.push_back(  0.5 ); //maxx
        values.push_back( -0.5 ); //miny
        values.push_back(  0.5 ); //maxy
        values.push_back( -0.5 ); //minz
        values.push_back(  0.5 ); //maxz
        return doubleBuilder.build();
    }
    else if ( name == "name" )
    {
        return StringAttribute( "fractal" );
    }
    return Foundry::Katana::Attribute();
}

ScenegraphContext *RootContext::getFirstChild() const
{
    return new FractalContext( m_args, 0, 0 );
}

// FractalContext

FractalContext::FractalContext( SubdividedSpace::ArgsPtr args, unsigned int index, unsigned int depth )
    : m_args( args )
    , m_index( index )
    , m_depth( depth )
{
}

FractalContext::FractalContext( const FractalContext &other )
{
    m_args = other.m_args;
    m_index = other.m_index;
    m_depth = other.m_depth;
}

void FractalContext::getLocalAttrNames( std::vector<std::string> *names ) const
{
    names->clear();
    names->push_back( "type" );
    names->push_back( "xform" );
    names->push_back( "bound" );
    names->push_back( "depth" );
    names->push_back( "index" );
}

Foundry::Katana::Attribute FractalContext::getLocalAttr( const string &name ) const
{
    if ( name == "type" )
    {
        return StringAttribute( "group" );
    }
    else if ( name == "xform" )
    {
        unsigned int repeat = (unsigned int)pow( 2, m_args->subdivisions ) ;
        int x = m_index % repeat;
        int y = (int)floor( m_index / repeat ) % repeat;
        int z = (int)floor( m_index / ( repeat * repeat ) ) % repeat;

        double tUnit = 1.0 / repeat;
        double start = -0.5 + ( tUnit * 0.5 );
        double scale = 1.0 / repeat;

        DoubleBuilder doubleBuilder( 4 );
        vector<double> &values = doubleBuilder.get(0);
        values.resize( 16 );

        values[0] = scale;         values[1] = 0;         values[2] = 0;         values[3] = 0;
        values[4] = 0;         values[5] = scale;         values[6] = 0;         values[7] = 0;
        values[8] = 0;         values[9] = 0;         values[10] = scale;        values[11] = 0;
        values[12] = start + x*tUnit;  values[13] = start + y*tUnit;  values[14] = start + z*tUnit;  values[15] = 1;

        GroupBuilder group;
        group.set( "matrix", doubleBuilder.build() );

        return group.build();
    }
    else if ( name == "bound" )
    {
        DoubleBuilder doubleBuilder(2);
        std::vector< double > &values = doubleBuilder.get(0);
        values.reserve( 6 );

        values.push_back( -0.5 ); // minx
        values.push_back(  0.5 ); // maxx
        values.push_back( -0.5 ); // miny
        values.push_back(  0.5 ); // maxy
        values.push_back( -0.5 ); // minz
        values.push_back(  0.5 ); // maxz

        return doubleBuilder.build();
    }
    else if ( name == "name" )
    {
        std::stringstream n;
        n << m_index;
        return StringAttribute( n.str() );
    }
    else if ( name == "index" )
    {
        return IntAttribute( m_index );
    }
    else if ( name == "depth" )
    {
        return IntAttribute( m_depth );
    }

    return Foundry::Katana::Attribute();
}

ScenegraphContext *FractalContext::getFirstChild() const
{
    if ( m_depth < m_args->maxdepth )
    {
        return new FractalContext( m_args, 0, m_depth+1 );
    }
    return 0x0;
}

ScenegraphContext *FractalContext::getNextSibling() const
{
    if ( (int)m_index < pow(2,m_args->subdivisions*3)-1 )
    {
        return new FractalContext( m_args, m_index+1, m_depth );
    }
    return 0x0;
}

DEFINE_SGG_PLUGIN( SubdividedSpace )

} /* end namespace subdividedSpace */


void registerPlugins()
{
    REGISTER_PLUGIN( subdividedSpace::SubdividedSpace, "SubdividedSpaceSGG", 1, 0 );
}
