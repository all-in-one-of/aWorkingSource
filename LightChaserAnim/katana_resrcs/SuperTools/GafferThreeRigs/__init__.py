import logging
log = logging.getLogger('GafferThreeRigs')

try:
    import v1 as GafferThreeRigs
except Exception as exception:
    log.exception('Error importing Super Tool Python package: %s' % str(exception))
else:
    PluginRegistry = [("SuperTool", 2, "GafferThreeRigs",
                      (GafferThreeRigs.GafferThreeRigsNode,
                       GafferThreeRigs.GetEditor))]

