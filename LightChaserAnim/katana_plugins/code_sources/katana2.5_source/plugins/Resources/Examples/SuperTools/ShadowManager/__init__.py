# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

import Katana
import v1 as ShadowManager

# Register the SuperTool
PluginRegistry = [
    ("SuperTool", 2, "ShadowManager",
            (ShadowManager.ShadowManagerNode, ShadowManager.GetEditor)),
]
