# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

PluginRegistry=[]

import CastingSheet
PluginRegistry.append(
    ("ImportomaticModule", "1.0.0",  "CastingSheet", (CastingSheet.Register, CastingSheet.RegisterGUI)),
)
