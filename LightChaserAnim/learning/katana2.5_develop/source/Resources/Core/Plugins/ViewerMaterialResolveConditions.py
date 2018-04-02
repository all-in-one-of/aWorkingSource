# Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.
"""
This module is used to determine under which conditions the implicit MaterialResolve
is run in the Katana's B{Viewer} tab. As a material resolve can be an expensive
process the viewer will perform certain tests to determine if the resolve is
necessary. This is done by testing the scene graph location for the presence of
certain attributes (specified here) ond only continuing to resolve if a match is
made. If a specified attribute starts with "material." a partial material resolve
will be performed to test for the presence of the attribute in the material.

The attributes specified here are stored at /root/world under "viewer.materialResolveAttrs"
and only if that attribute does not exist, allowing it to be overridden as desired.

There are two ways to specify a match:
    - Match by name
    - Match by value

Match by name:
    This is when you simply want to test if an attribute with a specified name
    exists, but do not care about the value. This can be done by using the
    attribute name as the key and a C{NullAttribute} as the value, eg.

    C{requiredAttrs.set("material.lightShader", FnAttribute.NullAttribute())}

Match by value:
    This is when you want to ensure that the value of the specified attribute
    matches your criteria. In this case use the attribute name as the key and
    an attribute with the value that you desire as the value, eg.

    C{requiredAttrs.set("material.style", FnAttribute.StringAttribute("network"))}

Optimization Possibilities
--------------------------
The attributes specified here are a default range which should ensure that
everything that should be resolved, is resolved. This may however result in
a number of false positives, resolving locations that have no effect in the viewer.
Speed improvements can be gained by removing some of the options here and
inserting your own implicit resolver Op that will determine if a location
should be resolved.

Side effects of unresolved locations include:
    - Materials not displaying on geometry
    - Light manipulators not being available
    - Incorrect light representation by the light viewer modifier

"""
from Katana import (
   Nodes3DAPI,
   FnAttribute,
   RenderingAPI,
)

requiredAttrs = FnAttribute.GroupBuilder()

#-------------------------------------------------------------------------------
# Match by name - Specify which attribute names to search for.
#-------------------------------------------------------------------------------

requiredAttrs.set("material.resolveInViewer", FnAttribute.IntAttribute(1))
requiredAttrs.set("material.lightShader", FnAttribute.NullAttribute())
requiredAttrs.set("material.viewerSurfaceShader", FnAttribute.NullAttribute())
requiredAttrs.set("material.parameters", FnAttribute.NullAttribute())
requiredAttrs.set("material.reference", FnAttribute.NullAttribute())

# Get the names of all known renderer plug-ins for light shaders
rendererPluginNames = RenderingAPI.RenderPlugins.GetRendererPluginNames()
for rendererPluginName in rendererPluginNames:
    requiredAttrs.set("material.%sLightShader" % rendererPluginName,
                      FnAttribute.NullAttribute())


#-------------------------------------------------------------------------------
# Match by value - Specify attributes whose value must also match those provided
#-------------------------------------------------------------------------------
requiredAttrs.set("material.style", FnAttribute.StringAttribute("network"))
requiredAttrs.set("viewer.default.resolveMaterialInViewer", FnAttribute.StringAttribute("always"))

#-------------------------------------------------------------------------------
# Generate Op args
#-------------------------------------------------------------------------------
opArgsBuilder = FnAttribute.GroupBuilder()
opArgsBuilder.set("locationPaths", FnAttribute.StringAttribute('/root/world'))
opArgsBuilder.set('setAttrs.i0.name', FnAttribute.StringAttribute("viewer.materialResolveAttrs"))
opArgsBuilder.set('setAttrs.i0.attr', requiredAttrs.build())
opArgsBuilder.set('setAttrs.i0.inherit', FnAttribute.IntAttribute(0))

# Register this op with the viewer implicit resolvers
Nodes3DAPI.RegisterImplicitResolver(
        Nodes3DAPI.ImplicitResolverStage.BeforeViewerResolvers,
        'AttributeSetIfNotPresent',
        opArgsBuilder.build(),
        True)
