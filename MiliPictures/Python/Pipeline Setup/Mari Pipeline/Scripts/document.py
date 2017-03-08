#coding:utf-8
__author__ = 'kai.xu'

import mari
import sys
import webbrowser

def main():
	url = "http://wiki.mili.com/%E9%A6%96%E9%A1%B5"

	webbrowser.open(url,new=0,autoraise=True)

SCTIPT = \
		"import userSetup\n"\
		"from Scripts import document\n"\
		"document.main()\n"

	
def _registerAction():
    menu_path = "MainWindow/Mili"
    mari.menus.addAction(mari.actions.create('Documentation', "%s"%SCTIPT), menu_path)

if mari.app.isRunning():
    _registerAction()
