import logging
log = logging.getLogger('CharBoundingSet')

try:
    import v1 as CharBoundingSet
except Exception as exception:
    log.exception('Error importing Super Tool Python package: %s' % str(exception))
else:
    PluginRegistry = [("SuperTool", 2, "CharBoundingSet",
                      (CharBoundingSet.CharBoundingSetNode,
                       CharBoundingSet.GetEditor))]

