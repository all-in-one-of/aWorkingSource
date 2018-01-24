# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#//////////////////////////////////////////////////////////////////////////////
#// Imports
from Katana import Utils
import logging

log = logging.getLogger("ShadowManager.Util")


#//////////////////////////////////////////////////////////////////////////////

class undogroup(object):
    """
    Used as a decorator around node and script item methods.
    """
    def __init__(self, name):
        self.__name = name
    def __call__(self, f):
        def wrapped_f(*args):
            Utils.UndoStack.OpenGroup(self.__name)
            try:
                return f(*args)
            finally:
                Utils.UndoStack.CloseGroup()
        return wrapped_f

#//////////////////////////////////////////////////////////////////////////////

class stacktrace(object):
    def __init__(self, name):
        self.__name = name
    def __call__(self, f):
        def wrapped_f(*args):
            try:
                return f(*args)
            except Exception as exception:
                log.exception('Error calling "%s" function: %s'
                              % (self.__name, str(exception)))
        return wrapped_f

