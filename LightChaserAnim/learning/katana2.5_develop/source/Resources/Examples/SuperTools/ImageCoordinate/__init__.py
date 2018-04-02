import logging
log = logging.getLogger('ImageCoordinate')

try:
    import v1 as ImageCoordinate
except Exception as exception:
    log.exception('Error importing Super Tool Python package: %s' % str(exception))
else:
    PluginRegistry = [("SuperTool", 2, "ImageCoordinate",
                      (ImageCoordinate.ImageCoordinateNode,
                       ImageCoordinate.GetEditor))]

