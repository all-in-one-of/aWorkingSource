#coding:utf-8
__author__ = 'kai.xu'


import maya.cmds as cmds


class YourClass():
    def __init__(self):
        if cmds.window('YourClass', q=1, exists=1):
            cmds.deleteUI('YourClass')
        Mytool = cmds.window('YourClass',title='YourClass',widthHeight=(400,500),sizeable=False)
        cmds.columnLayout(adjustableColumn=True)
        cmds.text(label='Your  Class',height=30, backgroundColor=[0.5, 0.5, 0.6])
        cmds.separator(style='out')
        cmds.text(label=' ')
        cmds.rowColumnLayout( numberOfColumns=2, columnWidth=[(1, 70),(2,320)])
        cmds.button(label='text', height=50, command=lambda *arg:self.fuction())
        cmds.setParent('..')
        cmds.showWindow(Mytool)

    def something(self):
        cmds.confirmDialog( title=u'提示', message=u'请转换到 Arnold 渲染器！', button=u'了解')
        pass

if __name__ == '__main__':
    YourClass()
