# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

"""
An example script that black-lists a ViewportManipulator for locations that
contain certain attributes.
"""

import ViewerManipulators
from Katana import Callbacks

def addToBlackList(objectHash):
    # Suppress the Scale manipulator for all locations that contain the 
    # attribute 'geometry.fov'
    ViewerManipulators.AddToBlacklist('Scale', ('geometry.fov',) )

Callbacks.addCallback(Callbacks.Type.onStartupComplete, addToBlackList)
