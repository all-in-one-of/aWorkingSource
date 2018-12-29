#!/usr/bin/env python
# encoding: utf-8

__auther__ = "kevin tsui"

from Katana import os, QtGui, QtCore, QT4Widgets, QT4FormWidgets, QT4Color, Utils
import UI4

from ReferenceTab import ShotColorReference

PluginRegistry = [
    ("KatanaPanel", 2.0, "Light Chaser Animation/Shot Color Reference", ShotColorReference),
]