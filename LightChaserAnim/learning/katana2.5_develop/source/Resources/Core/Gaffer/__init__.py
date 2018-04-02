# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

PluginRegistry=[]

import LightScriptItem
import RigScriptItem
import SkyDomeScriptItem
import MasterMaterialScriptItem
import AimTargetScriptItem

PluginRegistry.extend([
    ("GafferModule", "1.0.0",  "LightScriptItem", (LightScriptItem.Register, LightScriptItem.RegisterGUI)),
    ("GafferModule", "1.0.0",  "RigScriptItem", (RigScriptItem.Register, RigScriptItem.RegisterGUI)),
    ("GafferModule", "1.0.0",  "SkyDomeScriptItem", (SkyDomeScriptItem.Register, SkyDomeScriptItem.RegisterGUI)),
    ("GafferModule", "1.0.0",  "MasterMaterialScriptItem", (MasterMaterialScriptItem.Register, MasterMaterialScriptItem.RegisterGUI)),
    ("GafferModule", "1.0.0",  "AimTargetScriptItem", (AimTargetScriptItem.Register, AimTargetScriptItem.RegisterGUI)),
])
