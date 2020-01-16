#coding:utf-8
__help__ = u"调用方法函数run()实现动画发布时自动烘焙约束到场景元素上的非场景元素物体"
__author__ = 'kai.xu'

#############################################################################################################################
#类animationBakeToLocator()有异常打印，即使代码出问题，也不影响文件发布
#动画发布时自动烘焙约束到场景元素上的非场景元素物体
#方法函数是run()
#animationBakeToLocator = animationBakeToLocator()
#animationBakeToLocator.run()
#############################################################################################################################

import maya.cmds as cmds
import sys

class animationBakeToLocator(object):
    def __init__(self):
        self.constraintInfoList = []#{'parent':...,'child':...,'node':...}
        pass

    def run(self):
        try:
            self.getKeyNodes()
            #change
            #if len(self.constraintInfoList) == 0:
            #    pass
            #else:
            self.creatLocatorAndBake()
        except:
            print sys.exc_info()[0],sys.exc_info()[1] #打印异常

    def getKeyNodes(self):
        parentConstraint = cmds.ls(type='parentConstraint')
        pointConstraint = cmds.ls(type='pointConstraint')
        constraint = []

        #获取所有的约束节点
        for item in parentConstraint:
            constraint.append(item)
        for item in pointConstraint:
            constraint.append(item)

        #筛选
        for i in range(len(constraint)):
            constraintparents = cmds.listConnections(constraint[i]+'.target[0].targetTranslate',s=True,d=True)
            if not constraintparents == None:
                testList = constraintparents[0].split('_')
                for item in testList:
                    if item == 'AD' or item == 'AR':
                        constraintChildren = cmds.listConnections(constraint[i]+'.constraintTranslateX',d=True,s=True)
                        constraintInfoMap = {'parent':constraintparents[0],'child':constraintChildren[0],'node':constraint[i]}
                        self.constraintInfoList.append(constraintInfoMap)
                        break

    def creatLocatorAndBake(self):
        self.locatorList = []
        self.tmpConstraintList = []
        for constraintInfoMap in self.constraintInfoList:
            posX = cmds.getAttr(constraintInfoMap['parent']+'.translateX')
            posY = cmds.getAttr(constraintInfoMap['parent']+'.translateY')
            posZ = cmds.getAttr(constraintInfoMap['parent']+'.translateZ')
            locator = cmds.spaceLocator(name='%s_locator'%constraintInfoMap['child'])
            cmds.setAttr(locator[0]+'.translateX',posX)
            cmds.setAttr(locator[0]+'.translateY',posY)
            cmds.setAttr(locator[0]+'.translateZ',posZ)
            self.locatorList.append(locator[0])
            tmpConstraint = cmds.parentConstraint(constraintInfoMap['parent'],locator[0],maintainOffset=False,name='%s_parentConstraint'%locator[0])

            #删除原来的约束节点，创建新的父子约束，到locator
            cmds.delete(constraintInfoMap['node'])
            cmds.parentConstraint(locator[0],constraintInfoMap['child'],maintainOffset=True,name='%s_parentConstraint'%constraintInfoMap['child'])

            self.tmpConstraintList.append(tmpConstraint)

        #minTime = cmds.playbackOptions(q = True,min = True)
        #maxTime = cmds.playbackOptions(q = True,max = True)
        #fix min max frame
        import Mili.Module.utilties.ProjectConfig as ProjectConfig
        reload(ProjectConfig)
        config = ProjectConfig.ProjectConfig()
        aniCacheStart = config.get('AniStartFix', default=-8)
        aniCacheEnd = config.get('AniEndFix', default=8)
        minTime = cmds.playbackOptions(q = True,min = True) + int(aniCacheStart)
        maxTime = cmds.playbackOptions(q = True,max = True) + int(aniCacheEnd)

        #bake camera
        allNeedCamera = cmds.ls("CUT**", type = "camera")
        if allNeedCamera:
            allCamera = []
            #_log.info(self.get_cur_info() + "Introduction:" + "bake camera")
            for child in allNeedCamera:
                trans = cmds.listRelatives(child, p = True, f = True)[0]
                #allCamera.append(trans)
                self.locatorList.append(trans)
            #cmds.bakeResults(allCamera, t=(frame[0], frame[1]), simulation = True)

        if self.locatorList:
            cmds.bakeResults(self.locatorList,time=(minTime,maxTime), simulation = True)
        for trash in self.tmpConstraintList:
            cmds.delete(trash)

    def printFunc(self):
        print 'HelloWorld!'
if __name__ == '__main__':
    animationBakeToLocator = animationBakeToLocator()
    animationBakeToLocator.run()