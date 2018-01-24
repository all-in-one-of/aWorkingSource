"""
An example script that is executed when Katana starts.
"""

from Katana import Callbacks
import logging


def onStartupComplete(objectHash):
    log = logging.getLogger("Startup Example")
    log.info("Katana is now fully initialized and ready for use.")


log = logging.getLogger("Startup Example")
log.info("Registering onStartupComplete callback...")
Callbacks.addCallback(Callbacks.Type.onStartupComplete, onStartupComplete)
