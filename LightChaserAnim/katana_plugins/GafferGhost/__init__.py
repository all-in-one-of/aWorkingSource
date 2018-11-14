import logging
log = logging.getLogger('GafferGhost')

try:
    import v1 as GafferGhost
except Exception as exception:
    log.exception('Error importing Super Tool Python package: %s' % str(exception))
else:
    PluginRegistry = [("SuperTool", 2, "GafferGhost",
                      (GafferGhost.GafferGhostNode,
                       GafferGhost.GetEditor))]

