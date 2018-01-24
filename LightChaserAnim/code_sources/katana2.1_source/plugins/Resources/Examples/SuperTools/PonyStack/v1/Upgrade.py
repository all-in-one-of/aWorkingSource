# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

__all__ = ['Upgrade']


from Katana import NodegraphAPI, Utils
import logging

log = logging.getLogger("PonyStack.Upgrade")


def Upgrade(node):
    Utils.UndoStack.DisableCapture()
    try:
        pass
        # This is where you would detect an out-of-date version:
        #    node.getParameter('version')
        # and upgrade the internal network.
    except Exception as exception:
        log.exception('Error upgrading PonyStack node "%s": %s'
                      % (node.getName(), str(exception)))
    finally:
        Utils.UndoStack.EnableCapture()
