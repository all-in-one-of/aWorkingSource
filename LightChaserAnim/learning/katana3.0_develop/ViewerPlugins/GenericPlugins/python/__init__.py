# Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.
"""
Python package implementing Event Translators for the Viewer.
"""

import EventTranslators

PluginRegistry = [
    ("EventTranslator", 1.0, "MouseEventTranslator", EventTranslators.MouseEventTranslator),
    ("EventTranslator", 1.0, "MouseWheelTranslator", EventTranslators.MouseWheelTranslator),
    ("EventTranslator", 1.0, "KeyEventTranslator", EventTranslators.KeyEventTranslator),
    ("EventTranslator", 1.0, "EnterLeaveEventTranslator", EventTranslators.EnterLeaveEventTranslator)
]
