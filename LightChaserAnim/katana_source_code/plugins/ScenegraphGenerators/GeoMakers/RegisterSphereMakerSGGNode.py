"""
RegisterSphereMakerSGGNode.py

Example demonstrating using NodeTypeBuilder to wrap a legacy Scene Graph
Generator in a standalone node called "SphereMakerSGG".
"""
from Katana import Nodes3DAPI
from Katana import FnAttribute
from Katana import FnGeolibServices

GetModifiedFrameTime = Nodes3DAPI.TimingUtils.GetModifiedFrameTime


def buildSphereMakerOpChain(node, interface):
    """
    Defines the callback function used to create the Ops chain for the
    node type being registered.

    @type node: C{Nodes3DAPI.Node3D}
    @type interface: C{Nodes3DAPI.NodeTypeBuilder.OpChainInterface}
    @param node: The node requesting the op chain.
    @param interface: The interface providing the functions needed to set
        up the Ops chain for the given node.
    """
    # Set the minimum number of input ports
    interface.setMinRequiredInputs(0)

    # Get the current graph state and frame time
    graphState = interface.getGraphState()
    frameTime = graphState.getTime()

    # Compute the actual frame time we want to use
    modifiedFrameTime = GetModifiedFrameTime(node, frameTime)
    modGraphState = graphState.edit().setTime(modifiedFrameTime).build()
    systemAttr = modGraphState.getOpSystemArgs()

    locationPath = node.getParameter('location').getValue(frameTime)
    numberOfSpheres = node.getParameter('numberOfSpheres').getValue(frameTime)

    procArgsGb = FnAttribute.GroupBuilder()
    procArgsGb.set('args.system', systemAttr)
    procArgsGb.set('generatorType', 'SphereMaker')
    procArgsGb.set('args.numberOfSpheres', numberOfSpheres)

    sscb = FnGeolibServices.OpArgsBuilders.StaticSceneCreate()
    sscb.addSubOpAtLocation(locationPath, 'ScenegraphGeneratorHost',
                            procArgsGb.build())

    interface.appendOp('StaticSceneCreate', sscb.build())

# Create a NodeTypeBuilder to register the new type
nodeTypeBuilder = Nodes3DAPI.NodeTypeBuilder('SphereMakerSGG')

# Build the node's parameters and default values. The parameter types are
# inferred from the types of the default values, so location becomes a
# "string" parameter, and numberOfSpheres becomes a "number" parameter.
paramsBuilder = FnAttribute.GroupBuilder()
paramsBuilder.set('location', '/root/world/geo/SphereMaker')
paramsBuilder.set('numberOfSpheres', 20)

# Convenience method to add the 'timing.mode' parameter and give it the
# appropriate UI hints.
nodeTypeBuilder.addTimingParameters(paramsBuilder)

# Set the parameters template
nodeTypeBuilder.setParametersTemplateAttr(paramsBuilder.build())

# Set parameter hints
UI_HINTS = {
    'location': {'widget': 'newScenegraphLocation'},
    'numberOfSpheres': {'int': True},
}

for parameterName, hintDict in UI_HINTS.iteritems():
    nodeTypeBuilder.setHintsForParameter(parameterName, hintDict)

# Set the callback responsible to build the Ops chain
nodeTypeBuilder.setBuildOpChainFnc(buildSphereMakerOpChain)

# Build the new node type
nodeTypeBuilder.build()
