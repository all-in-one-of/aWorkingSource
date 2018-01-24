// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef _GEOSCALER_H_
#define _GEOSCALER_H_

#include <FnAttributeModifier/plugin/FnAttributeModifier.h>
#include <FnAttribute/FnDataBuilder.h>
#include <FnAttribute/FnGroupBuilder.h>



// Class definitions ----------------------------------------------------------

/**
 * Class representing an attribute modifier plug-in for scaling geometry point
 * data of selected scene graph locations.
 */
class GeoScaler : public FnKat::AttributeModifier
{

public: // constructors and destructors ---------------------------------------

    /**
     * Constructor.
     *
     * \param args: A group attribute that defines the parameters of the AMP,
     *     used to initialize internal data structures to be used later in the
     *     \c cook() function.
     */
    GeoScaler(FnKat::GroupAttribute args);

    /**
     * Destructor.
     */
    virtual ~GeoScaler();

public: // static functions ---------------------------------------------------

    /**
     * Factory method.
     *
     * \param args: A group attribute that represents values for arguments to
     *     initialize the new AMP instance with.
     * \return A new instance of the \c GeoScaler class.
     */
    static FnKat::AttributeModifier* create(FnKat::GroupAttribute args);

    /**
     * \return A group attribute that defines the parameters of the AMP that
     *     appear as part of the \b AttributeModifierDefine node's parameter
     *     interface, just below the \b modifierType dropdown parameter.
     */
    static FnKat::GroupAttribute getArgumentTemplate();

    /**
     * Clears allocated memory and reloads data from file (if any).
     */
    static void flush();

public: // instance functions -------------------------------------------------

    /**
     * Inspects the scene graph and modifies attribute data on the location
     * that is currently being processed.
     *
     * \param input: A reference to an instance of the
     *     \c AttributeModifierInput class which provides all the information
     *     related with the location being processed, as well as read-only
     *     access to other locations in the scene graph.
     */
    virtual void cook(FnKat::AttributeModifierInput& input);

public: // instance functions -------------------------------------------------

    /**
     * The default value to scale geometry by in the AMP's \c cook() function.
     */
    static const float DEFAULT_SCALE;

protected: // instance variables ----------------------------------------------

    /**
     * The value to scale geometry by in the AMP's \c cook() function.
     */
    float _scale;

};


#endif
