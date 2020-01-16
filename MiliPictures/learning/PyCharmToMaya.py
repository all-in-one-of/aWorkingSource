#coding = utf-8
__author__ = 'kai.xu'
import sys
import maya.cmds as cmds

global filePath
filePath = 'D:/MyPythonCode/Learning/MayaScriptEditor.py'

class PyCharmToMaya():
    def __init__(self):
        global filePath
        self.codePath = filePath
        self.showWindows()

    def showWindows(self):
        if cmds.window('codeTestInterface', q=1, exists=1):
            cmds.deleteUI('codeTestInterface')
        Mytool = cmds.window('codeTestInterface',title='PyCharm to Maya', width=300, sizeable=False)
        cmds.rowColumnLayout('mainNameSpace', numberOfColumns=1, columnWidth=[(1, 300)])
        cmds.text('PyCharm', height=30, backgroundColor=[0.4, 0.5, 0.5])
        cmds.separator(style='out')
        cmds.text(' ')
        cmds.rowColumnLayout('layOut', numberOfColumns=2, columnWidth=[(1, 200),(2,100)])
        cmds.textField('codeFilePath', text=self.codePath, enable=False)
        cmds.button(label='Browse code',command=lambda *arg:self.folderPathFinder())
        cmds.setParent('..')
        cmds.text(' ')
        cmds.separator(style='out')
        cmds.button(label='Run', height=50, command=lambda *arg:self.testScript())
        cmds.showWindow(Mytool)

    def folderPathFinder(self):
        filePath = cmds.fileDialog2(fileMode=True, fileFilter='*.py', caption='Find your code')
        if filePath==None:
            sys.exit()
        else:
            self.codePath = filePath[0]
            cmds.textField('codeFilePath', text=str(filePath[0]), edit=True)

    def testScript(self):
        scriptText = open(self.codePath,'r').read()
        scriptText += "\n\n\n#================Program Debugger================#"
        tmp = cmds.cmdScrollFieldExecuter(parent='mainNameSpace', text=scriptText.decode('utf-8'), execute=True, sourceType="python")
        cmds.deleteUI(tmp)

    def printEndLine(self):
        for i in range(100):
            print '-',
        print '\n'
if __name__ == '__main__':
    ptm = PyCharmToMaya()
    ptm.printEndLine()