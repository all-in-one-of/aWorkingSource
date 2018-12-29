import logging
log = logging.getLogger('ChraBoundingSet')

try:
    import v1 as ChraBoundingSet
except Exception as exception:
    log.exception('Error importing Super Tool Python package: %s' % str(exception))
else:
    PluginRegistry = [("SuperTool", 2, "ChraBoundingSet",
                      (ChraBoundingSet.ChraBoundingSetNode,
                       ChraBoundingSet.GetEditor))]

