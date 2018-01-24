// Description: An Op that creates a recursively divided unit cube
// Author: tom
// Date: 2014-01-31

#include <math.h>
#include <iostream>
#include <string>
#include <sstream>

#include <FnGeolib/op/FnGeolibOp.h>
#include <FnPluginSystem/FnPlugin.h>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnDataBuilder.h>

namespace { //anonymous

class SubdividedSpaceOp : public Foundry::Katana::GeolibOp
{
public:
    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        // User facing attributes

        FnAttribute::IntAttribute maxDepthAttr = interface.getOpArg( "maxDepth" );
        const int maxDepth = maxDepthAttr.getValue( 4, false );

        FnAttribute::IntAttribute subdivisionsAttr = interface.getOpArg( "subdivisions" );
        const int subdivisions = subdivisionsAttr.getValue( 1, false );

        // Only set internally -  hence the underscore
        FnAttribute::IntAttribute currentDepthAttr = interface.getOpArg( "_currentDepth" );
        FnAttribute::IntAttribute currentIndexAttr = interface.getOpArg( "_currentIndex" );

        const int currentDepth = currentDepthAttr.getValue( 0, false );
        const int currentIndex = currentIndexAttr.getValue( 0, false );

        interface.setAttr( "type", FnAttribute::StringAttribute( "group" ) );

        interface.setAttr( "info.index", FnAttribute::IntAttribute( currentIndex ) );
        interface.setAttr( "info.depth", FnAttribute::IntAttribute( currentDepth ) );

        FnAttribute::DoubleBuilder boundsBuilder( 2 );
        std::vector< double > &bounds = boundsBuilder.get( 0 );
        bounds.reserve( 6 );
        bounds.push_back( -0.5 ); //minx
        bounds.push_back(  0.5 ); //maxx
        bounds.push_back( -0.5 ); //miny
        bounds.push_back(  0.5 ); //maxy
        bounds.push_back( -0.5 ); //minz
        bounds.push_back(  0.5 ); //maxz
        interface.setAttr( "bound", boundsBuilder.build() );

        // The parent Node3D implementation takes care of using a static scene
        // create to implant us at some non-root location, so all we care about
        // is the relative path we're being cooked at

        if( !interface.atRoot() )
        {
            // xform
#ifndef _WIN32
	    unsigned int repeat = static_cast<unsigned int>(pow( 2, subdivisions ));
#else
	    // Prevents error error C2668: 'pow' : ambiguous call to overloaded function
	    unsigned int repeat = static_cast<unsigned int>(pow(2.0, static_cast<double>(subdivisions) ));
#endif
	    int x = currentIndex % repeat;
#ifndef _WIN32
	    int y = (int)floor( currentIndex / repeat ) % repeat;
	    int z = (int)floor( currentIndex / ( repeat * repeat ) ) % repeat;
#else
	    int y = (int)floor( static_cast<double>(currentIndex) / static_cast<double>(repeat) ) % repeat;
	    int z = (int)floor( static_cast<double>(currentIndex) / static_cast<double>( repeat * repeat ) ) % repeat;
#endif
            double tUnit = 1.0 / repeat;
            double start = -0.5 + ( tUnit * 0.5 );
            double scale = 1.0 / repeat;

            FnAttribute::DoubleBuilder doubleBuilder( 16 );
            std::vector<double> &matrix = doubleBuilder.get(0);
            matrix.resize( 16 );

            matrix[0] = scale;             matrix[1] = 0;                 matrix[2] = 0;                 matrix[3] = 0;
            matrix[4] = 0;                 matrix[5] = scale;             matrix[6] = 0;                 matrix[7] = 0;
            matrix[8] = 0;                 matrix[9] = 0;                 matrix[10] = scale;            matrix[11] = 0;
            matrix[12] = start + x*tUnit;  matrix[13] = start + y*tUnit;  matrix[14] = start + z*tUnit;  matrix[15] = 1;

            FnAttribute::GroupBuilder group;
            group.set( "matrix", doubleBuilder.build() );

            interface.setAttr( "xform", group.build() );
        }

        if( currentDepth >= maxDepth )
        {
            interface.stopChildTraversal();
            return;
        }

        // Add our children
#ifndef _WIN32
        const unsigned int numChildren = static_cast<unsigned int>(pow( 2, subdivisions*3 ));
#else
	// Prevents error C2668: 'pow' : ambiguous call to overloaded function
	const unsigned int numChildren = static_cast<unsigned int>(pow( 2.0, static_cast<double>(subdivisions)*3.0 ));
#endif
	
        for( unsigned int i=0; i<numChildren; ++i )
        {
            // We need to replace the depth and index in the opargs
            // for the child ops
            FnAttribute::GroupBuilder args;
            args.update( interface.getOpArg() );
            args.set( "_currentDepth", FnAttribute::IntAttribute( currentDepth + 1 ) );
            args.set( "_currentIndex", FnAttribute::IntAttribute( i ) );

            std::stringstream n;
            n << "unit" << i;

            interface.createChild( n.str(), "", args.build() );
        }
    }
};

DEFINE_GEOLIBOP_PLUGIN(SubdividedSpaceOp)

} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(SubdividedSpaceOp, "SubdividedSpace", 0, 1);
}

