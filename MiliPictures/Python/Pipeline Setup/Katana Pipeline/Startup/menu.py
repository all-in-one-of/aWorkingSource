# Copyright (c) 2016 Mili Pictures. All Rights Reserved.

#coding:utf-8
__author__ = "Kevin Tsui"

import Katana
import sys,os,json
from PyQt4 import QtGui,QtCore

from Katana import Callbacks
import logging

#~ Import pipeline path
PIPRLINE_PATH = os.environ["PIPRLINE_PATH"]
STARTUP_PATH = os.environ["STARTUP_PATH"]
sys.path.insert(0, '%s'%PIPRLINE_PATH)

#Set up custom menu element
def onStartupComplete(objectHash):
	# Hack to find the MainMenu
	layoutsMenus = [x for x in QtGui.qApp.topLevelWidgets() if type(x).__name__ == 'LayoutsMenu']
	if len(layoutsMenus) != 1:
		return
	file = open(STARTUP_PATH + "menu.json","rb")
	MenuData = json.load(file)
	file.close()

	MainMenu = layoutsMenus[0].parent()
	
	# Hang onto pointers so they do not get garbage collected
	Katana.MILI_MENUS = dict()
	MiliMenu = QtGui.QMenu('Mili Pipeline', MainMenu)
	
	for MenuMap in MenuData:
		MiliMenu.addSeparator()
		for MenuItem in MenuData[MenuMap]:
			ItemName = MenuItem
			ItemScript = MenuData[MenuMap][MenuItem]
			MiliMenu.addAction(QtGui.QAction(ItemName, MainMenu, checkable=False,triggered=lambda arg1=None,arg2=ItemScript:runScript(arg2)))	
	Katana.MILI_MENUS['/Mili'] = MiliMenu
	MainMenu.addMenu(MiliMenu)
def runScript(InScript):
	exec(InScript)
	
def buildScript(InScript):
	from KtnPyMili import KatanaSceneBuild
	reload(KatanaSceneBuild)
	print "Module KatanaSceneBuild Import Successed..."
	KatanaSceneBuild.execute()

def testScript():
	from KtnPyMili import Test
	reload(Test)
	
def NetworkMaterialInterfaceAutomatorScript():
	from KtnPyMili import NetworkMaterialInterfaceAutomator
	reload(NetworkMaterialInterfaceAutomator)
	
log = logging.getLogger("Startup Mili Pipeline")
log.info("Registering onStartupComplete callback...")
Callbacks.addCallback(Callbacks.Type.onStartupComplete, onStartupComplete)

"""
#~ Test

def onNewScene(objectHash):
	print "@onNewScene"

Callbacks.addCallback(Callbacks.Type.onNewScene, onNewScene)

def onNodeCreate(**kwargs):
	print "@OnNodeCreate"

Callbacks.addCallback(Callbacks.Type.onNodeCreate, onNodeCreate)
"""