// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.



#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <iostream>

#include <FnAttributeModifier/plugin/FnAttributeModifier.h>
#include "FnAttribute/FnGroupBuilder.h"


// ------------------------------------ PointDisplacingAttributeModifier : a useful base class

/* PointDisplacingAttributeModifier is a class which provides helper functions 
 * to perform one particular case of attribute modifications: the displacement 
 * of points in an object's geometry. */

class PointDisplacingAttributeModifier : public FnKat::AttributeModifier
{
public:
    PointDisplacingAttributeModifier(FnKat::GroupAttribute args) : FnKat::AttributeModifier(args) {}
    virtual ~PointDisplacingAttributeModifier() {}
    virtual void cook(FnKat::AttributeModifierInput &input);
    virtual void makeNewPoint(float* dest, const float* src, unsigned int dimensions) =0;
    static void flush() {}
};

void 
PointDisplacingAttributeModifier::cook(FnKat::AttributeModifierInput &input) {
    FnKat::FloatAttribute points = input.getAttribute("geometry.point.P");

    if(points.isValid()) {

        unsigned int tuple_size = points.getTupleSize();   // i.e., number of dimensions in space
        unsigned int num_values;

        //const float* values = points.getNearestSample(0.0f, &num_values);
        FnKat::FloatConstVector oldpts = points.getNearestSample(0.0f);
        const float* values = oldpts.data();
        num_values = oldpts.size();

        float* newvalues = new float[num_values];

        for(unsigned int i=0; i<num_values; i+=tuple_size) {
            makeNewPoint(&newvalues[i], &values[i], tuple_size);
        }

        FnKat::FloatAttribute newpoints(newvalues, num_values, tuple_size);;
        input.addOverride("geometry.point.P", newpoints, false);
    } else {
        input.addOverride("type", FnKat::StringAttribute("error"), false);
        input.addOverride("errorMessage", FnKat::StringAttribute("No geometry.point.P on input"), false);
    }
}

// ------------------------------------ Messer : the plugin which messes up points

static void
makeRandomVector(float* v, unsigned int dimensions, float length) {
   float d = 0.0;
   for(unsigned int i=0; i<dimensions; i++) {
       v[i] = (float)rand()/(float)RAND_MAX;
       d += v[i]*v[i];
   }
   length /= sqrt(d);
   for(unsigned int i=0; i<dimensions; i++) {
       v[i] *= length;
   }
}

class Messer : public PointDisplacingAttributeModifier 
{
  static const float DEFAULT_DISPLACEMENT;
    float _displacement;
public:
    Messer(FnKat::GroupAttribute args)
              : PointDisplacingAttributeModifier(args), 
                _displacement(DEFAULT_DISPLACEMENT) { 
        if ( args.isValid() ) {
            FnKat::FloatAttribute displacementAttr = args.getChildByName("displacement");
            if (displacementAttr.isValid()) {
                _displacement = displacementAttr.getValue(DEFAULT_DISPLACEMENT);
            }
        }
    }
    virtual ~Messer() {}

    virtual void makeNewPoint(float* dest, const float* src, unsigned int dimensions);
    //virtual std::string getCacheId();

    static FnKat::AttributeModifier *create(FnKat::GroupAttribute args) {
        return new Messer(args);
    }
    static FnKat::GroupAttribute getArgumentTemplate() {
        //return FnGroupAttribute();
        FnKat::GroupBuilder gb;
        gb.set("displacement", FnKat::FloatAttribute(DEFAULT_DISPLACEMENT));
        return gb.build();
    }
};

void
Messer::makeNewPoint(float* dest, const float* src, unsigned int dimensions) {
    float delta[3];  // maximum number of dimensions for now; we could bump this up
    assert(dimensions<=3);  // saves reallocating memory each time
    makeRandomVector(delta, dimensions, _displacement);
    for(unsigned int dim=0; dim<dimensions; dim++) {
        dest[dim] = src[dim] + delta[dim];
    }
}

const float Messer::DEFAULT_DISPLACEMENT = 2.0f;

#if 0
std::string
Messer::getCacheId() {
   return "messer";
}
#endif


DEFINE_AMP_PLUGIN(Messer)

void registerPlugins()
{
    REGISTER_PLUGIN(Messer, "Messer", 0, 1);
}
