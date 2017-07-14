// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include "GeoScaler.h"
#include <FnLogging/FnLogging.h>


// Set up a logger
FnLogSetup("GeoScaler")


/**
 * The default value to scale geometry by in the AMP's \c cook() function.
 */
const float GeoScaler::DEFAULT_SCALE = 0.7f;


/**
 * Constructor.
 *
 * \param args: A group attribute that defines the parameters of the AMP,
 *     used to initialize internal data structures to be used later in the
 *     \c cook() function.
 */
GeoScaler::GeoScaler(FnKat::GroupAttribute args):
    FnKat::AttributeModifier(args),
    _scale(DEFAULT_SCALE)
{
    if (args.isValid())
    {
        FnKat::FloatAttribute scaleAttr = args.getChildByName("scale");
        _scale = scaleAttr.getValue(DEFAULT_SCALE, false);
    }
}

/**
 * Destructor.
 */
GeoScaler::~GeoScaler()
{
    // empty
}

/**
 * Factory method.
 *
 * \param args: A group attribute that represents values for arguments to
 *     initialize the new AMP instance with.
 * \return A new instance of the \c GeoScaler class.
 */
FnKat::AttributeModifier* GeoScaler::create(FnKat::GroupAttribute args)
{
    return new GeoScaler(args);
}

/**
 * \return A group attribute that defines the parameters of the AMP that
 *     appear as part of the \b AttributeModifierDefine node's parameter
 *     interface, just below the \b modifierType dropdown parameter.
 */
FnKat::GroupAttribute GeoScaler::getArgumentTemplate()
{
    FnKat::GroupBuilder gb;
    gb.set("scale", FnKat::FloatAttribute(DEFAULT_SCALE));
    return gb.build();
}

/**
 * Inspects the scene graph and modifies attribute data on the location
 * that is currently being processed.
 *
 * \param input: A reference to an instance of the
 *     \c AttributeModifierInput class which provides all the information
 *     related with the location being processed, as well as read-only
 *     access to other locations in the scene graph.
 */
void GeoScaler::cook(FnKat::AttributeModifierInput& input)
{
    // Get the current scene graph location's point position attribute and
    // check if it is valid
    FnKat::FloatAttribute P = input.getAttribute("geometry.point.P");
    if (P.isValid())
    {
        FnKat::FloatBuilder fb (P.getTupleSize());

        // Reserve float vectors for time samples 0.0 and 1.0
        fb.reserve(P.getNumberOfValues());
        fb.reserve(P.getNumberOfValues(), 1.0f);

        // Read current vertex positions
        FnKat::FloatConstVector data = P.getNearestSample(0.0f);

        // Scale each of the vertices for both time samples
        for (FnKat::FloatConstVector::const_iterator it = data.begin();
             it != data.end(); ++it)
        {
            fb.push_back((*it) * _scale);
            fb.push_back((*it) * _scale * 1.5f, 1.0f);
        }

        // Update the point position attribute on the current scene graph
        // location
        input.addOverride("geometry.point.P", fb.build(), true);
    }
    else
    {
        // Change the current location into an error location to stop the
        // renderer when this location is evaluated
        input.addErrorOverride("No geometry.point.P on input", true);
    }
}

/**
 * Clears allocated memory and reloads data from file (if any).
 */
void GeoScaler::flush()
{
    // empty
}


DEFINE_AMP_PLUGIN(GeoScaler)

/**
 * Registers the plug-ins that are contained in a shared object file (compiled
 * extension) with KATANA's plug-in system.
 */
void registerPlugins()
{
    REGISTER_PLUGIN(GeoScaler, "GeoScaler", 0, 1);
}
