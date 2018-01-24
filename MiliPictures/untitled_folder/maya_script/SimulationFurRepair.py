#coding=utf-8
__author__ = 'kai.xu'





import maya.cmds as cmds
import maya.mel as mel
import os,shutil

class simulationFurRepair():
    def __init__(self):
        if cmds.window('simulationFurRepair', q=1, exists=1):
            cmds.deleteUI('simulationFurRepair')
        Mytool = cmds.window('simulationFurRepair',title='simulationFurRepair',widthHeight=(400,500),sizeable=False)
        cmds.rowColumnLayout( numberOfColumns=1, columnWidth=[(1, 400)])
        cmds.text(label='Fur&Hair  Repairer', height=30, backgroundColor=[0.5, 0.5, 0.6])
        cmds.separator(style='out')
        cmds.text(label='')
        cmds.rowColumnLayout( numberOfColumns=2, columnWidth=[(1, 200),(2,200)])
        cmds.text(label='Enter a name of Face-Control Object')
        cmds.textField('Face_Control_Object_Name',text='ControlObject')
        cmds.setParent('..')
        cmds.text(label='')
        cmds.text(label='Select the object you want to repair')        
        cmds.button(label='Creat Face-Control Object',command=lambda *arg:self.__run__())
        cmds.text(label='')
        cmds.text(label='Delete object when you done')
        cmds.button(label='Delete Face-Control Object',command=lambda *arg:self.doDelete())
        cmds.text(label='')
        cmds.text(label='Close the window and delete Cache')
        cmds.button(label='Delete and Close',command=lambda *arg:self.doClose())
        cmds.showWindow(Mytool)

        
    def __run__(self):
        self.geometryCacheReplace()
        #self.creatGeometryCache()
        #self.importGeometryCache()
        self.doBlendShape()
        self.changeValue()
        cmds.confirmDialog(title='CREAT', message='Operation Complete',button='Gotcha')
        

    def creatGeometryCache(self):
        object_listTemp = cmds.ls(sl=True)
        global object 
        object = object_listTemp[0]
        mel.eval('''doCreateGeometryCache 6 { "2", "1", "10", "OneFile", "1", "D:\Cache","0","","0", "add", "0", "1", "1","0","1","mcc","0" } ;''')
        cmds.duplicate(name = 'dupControlObject')
        cmds.group('dupControlObject',name = 'ControlObjectGroup',world=True)
        cmds.select('dupControlObject')
        pass
        
    def importGeometryCache(self):
        mel.eval('doImportCacheArgList(0,{});')
        mel.eval('np_getPrimaryProjectFileRules 0;')
        pass
      
    
    def geometryCacheReplace(self):
        object_listTemp = cmds.ls(sl=True,long=True)
        global object 
        object = object_listTemp[0]
          
        dupObjectName = cmds.textField('Face_Control_Object_Name',q=True,tx=True)
        #print dupObjectName
        
        minTime = cmds.playbackOptions(q = True,min = True)
        maxTime = cmds.playbackOptions(q = True,max = True)
      
        attrs = cmds.listHistory(object)
        meshList= []
        for i in attrs:
            if cmds.objectType(i,isType='mesh'):
                meshList.append(i)
        cmds.cacheFile(fm='OneFile',cf='mcc',ws=True,f=dupObjectName+'GeometryCache',refresh=0,st=minTime,et=maxTime,
                       dir='D:\Cache',pts=meshList[0])
        
        cmds.currentTime(minTime,edit=True )
        
        try: 
            cmds.select(dupObjectName+'_Group')
            cmds.delete()
            cmds.select(object)
            cmds.duplicate(name = dupObjectName)
            cmds.group(dupObjectName,name = dupObjectName+'_Group',world=True)
        except:
            #print 'No element'
            cmds.select(object)               
            cmds.duplicate(name = dupObjectName)
            cmds.group(dupObjectName,name = dupObjectName+'_Group',world=True)
        
        cmds.select(dupObjectName)

        if os.path.exists('D:\Cache\TrashCache.mc') or os.path.exists('D:\Cache\TrashCache.xml'):
            os.remove('D:\Cache\TrashCache.mc')
            os.remove('D:\Cache\TrashCache.xml')       
        else:
            print 'Go ahead'

        mel.eval('''doCreateGeometryCache 6 { "2", "1", "10", "OneFile", "1", "D:/Cache","0","TrashCache","0", "add", "0", "1", "1","0","0","mcc","1" } ;''')
        cmds.currentTime(minTime,edit=True )
 
        dupObjAttrs = cmds.listHistory(dupObjectName)
        cacheNameList = []
        for i in dupObjAttrs:
            if cmds.objectType(i,isType='cacheFile'):
                cacheNameList.append(i)              
                                            
        cmds.setAttr("%s.cacheName"%cacheNameList[0],dupObjectName+'GeometryCache',type='string')
        
    def doBlendShape(self):
        dupObjectName = cmds.textField('Face_Control_Object_Name',q=True,tx=True)
        cmds.select(dupObjectName)
        cmds.select(object,add=True)
        cmds.blendShape( name=dupObjectName+'BlendShape')
        #mel.eval('''blendShape -n "tamp_BlendShape";''')
        
    def changeValue(self):
        #cmds.select(object)
        #object_list = cmds.ls(sl=True,long=True)
        dupObjectName = cmds.textField('Face_Control_Object_Name',q=True,tx=True)
        conDupShapes = cmds.listRelatives(dupObjectName)
        #print conDupShapes
        conDupInputs = cmds.listConnections(conDupShapes,s=False, d=True,t='blendShape' )
        cmds.setAttr(conDupInputs[0]+'.'+dupObjectName,1)
        #print(conDupInputs)
        conShapes = cmds.listRelatives(object,shapes=True)
        #print conShapes
        conHistory = cmds.listHistory(object)
        #print(conHistory)
        tweakList = []
        blendShapeList = []
        for i in conHistory:
            if cmds.objectType(i,isType='tweak'):
               tweakList.append(i)
               #print(i)        
        for item in tweakList:
            cmds.setAttr(item+'.envelope',0)
            #print item
        for i in conHistory:
            if cmds.objectType(i,isType='blendShape'):
               blendShapeList.append(i)
               #print i        
        for item in blendShapeList:
            cmds.setAttr(item+'.envelope',0)
            #print item
        cmds.setAttr(dupObjectName+'BlendShape'+'.envelope',1)
        cmds.setAttr(dupObjectName+'BlendShape'+'.'+dupObjectName,1)
       
    def doDelete(self):
        objectDelateList = cmds.ls(sl=True)
        objectDelate = objectDelateList[0]

        dupObjectName = cmds.textField('Face_Control_Object_Name',q=True,tx=True)
        cmds.select(dupObjectName+'BlendShape')
        cmds.delete()
        conShapes = cmds.listRelatives(objectDelate,shapes=True)
        conHistory = cmds.listHistory(objectDelate)
        #print(conHistory)
        tweakList = []
        blendShapeList = []
        for i in conHistory:
            if cmds.objectType(i,isType='tweak'):
               tweakList.append(i)
               #print(i)        
        for item in tweakList:
            cmds.setAttr(item+'.envelope',1)
            #print item
        for i in conHistory:
            if cmds.objectType(i,isType='blendShape'):
               blendShapeList.append(i)
               #print i        
        for item in blendShapeList:
            cmds.setAttr(item+'.envelope',1)
            #print item
        
        try:
            cmds.select(dupObjectName+'_Group')
            cmds.delete()
        except:
            print 'You have already deleted control group'
    
        try:
            os.remove('D:\Cache\%sGeometryCache.mc'%dupObjectName)
            os.remove('D:\Cache\%sGeometryCache.xml'%dupObjectName)
            #os.rmdir( 'D:\Cache')
        except:
            print 'The file had already deleted!'
                        
        try:
            os.remove('D:\Cache\TrashCache.mc')
            os.remove('D:\Cache\TrashCache.xml')
        except:
            print 'The trash file had already deleted!'
        #cmds.deleteUI('simulationFurRepair')
        cmds.confirmDialog(title='DELETE', message='Operation Complete',button='Gotcha')
    
    def doClose(self):
        yourChoice = cmds.confirmDialog( title='CLOSED', message='You sure you want to delete the cache?', button=['Yes','No'], defaultButton='Yes', cancelButton='No', dismissString='No' )
        if yourChoice == 'Yes':
            try:
                shutil.rmtree('D:\.mayaSwatches')
            except:
                print 'The mayaSwatches file had already deleted'
            
            try:
                shutil.rmtree('D:\Cache')
                cmds.deleteUI('simulationFurRepair')
            except WindowsError:
                if os.path.exists('D:\Cache') == False:
                    cmds.deleteUI('simulationFurRepair')
                else:
                    cmds.confirmDialog( title='ERRO', message='The cache file is being used', button='Gotcha')
   
        else:
            print 'Wise choise'            

    
    
    
if __name__ == '__main__':
    simulationFurRepair()