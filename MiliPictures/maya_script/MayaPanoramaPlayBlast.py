#coding:utf-8
__author__ = 'kai.xu'


from PySide import QtGui,QtCore
import shutil,os,time,shiboken
import maya.cmds as cmds
import maya.OpenMayaUI as omui
import maya.mel as mel
import maya.OpenMaya as om

class MayaPanoramaPlayBlast():
    def __init__(self, imgPath = '', camera='', showUI=True, playMovie=True):
        self.showUI = showUI
        self.playMovie = playMovie
        if camera=='':
            camera = cmds.ls(self.activeCam())
        if imgPath=='':
            sceneName = cmds.file(q=1,sceneName=1)
            if not os.path.isfile(sceneName):
                imgPath = os.environ['temp'].replace('\\','/')+'/tmpPanoPlayBlast/untitled_%i'%time.time()
            else:
                imgPath = os.path.splitext(sceneName)[0]
        
        #test writtable
        testPath = os.path.dirname(imgPath)+'/test.%s.txt'%time.time()
        try: 
            f = open(testPath,'w')
            f.write('')
            f.close()
            os.remove(testPath)
        except:
            imgPath = os.environ['temp'].replace('\\','/')+'/tmpPanoPlayBlast/'+os.path.basename(imgPath)
            
        print '\n\n image path:', imgPath
            
        #tool`s first UI
        if showUI:
            if cmds.window('MayaPanoramaPlayBlast', q=1, exists=1):
                cmds.deleteUI('MayaPanoramaPlayBlast')
            self.myTool = cmds.window('MayaPanoramaPlayBlast',title='Maya Panorama Playblast',widthHeight=(400,500),sizeable=False)
            cmds.rowColumnLayout( numberOfColumns=1, columnWidth=[(1, 400)])
            cmds.text(label='Panorama Playblast',height=30, backgroundColor=[0.5, 0.5, 0.6])
            cmds.separator(style='out')
            cmds.text(label=' ')
            cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1, 80),(2,300)])
            cmds.text(u'全景图片名:', align='left')
            cmds.textField('imageName',text = os.path.basename(imgPath), editable=(os.path.basename(imgPath)!=os.path.basename(os.path.splitext(cmds.file(q=1,sceneName=1))[0])) )
            cmds.setParent('..')
            cmds.text(label=' ')
            cmds.rowColumnLayout(numberOfColumns=3, columnWidth=[(1, 80),(2,260),(3,55)])
            cmds.text(u'储存路径:', align='left')
            cmds.textField('imagePath', editable=False, text=os.path.dirname(imgPath))
            cmds.button(label='Browse',command=lambda *arg: self.browse())
            cmds.setParent('..')
            cmds.text(label=' ')
            cmds.separator(style='out')
            cmds.text(label=' ')
            cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1, 300),(2,95)])
            cmds.text(label=u'请先选择动画摄像机，然后点击  ------------->',align='left')
            cmds.button(label='START', height=35, command=lambda *arg: self.doPanoramaSnap())
            cmds.setParent('..')
            cmds.text(label=' ')
            cmds.separator(style='out')
            cmds.text(label=' ')
            cmds.showWindow(self.myTool)
        else:
            self.camera = camera
            self.imageName = os.path.basename(imgPath)
            self.imagePath= os.path.dirname(imgPath)
            self.doPanoramaSnap()


    def doPanoramaSnap(self):
        self.allOrigNodes = cmds.ls()
        if self.showUI:
            cmds.scriptJob(idleEvent = lambda *args: self.doubleCleanUp(), runOnce=1)
        
        if self.showUI:
            self.imageName = cmds.textField('imageName', text=True, query=True)
            self.imagePath = cmds.textField('imagePath', text=True, query=True)
            self.camera = cmds.ls(sl=True)[0]

        if self.imageName == '' or self.imagePath == '':
            cmds.confirmDialog(title='Oops', message=u'请先输入名字和路径！', button='Gotcha')
        else:
            #~ print self.camera, cmds.objectType(cmds.listRelatives(self.camera), isType='camera')
            if not self.camera == [] and cmds.nodeType(self.camera)=='camera':
                self.lightsGrp = self.createLights(camera=self.camera)
                self.showPanoramaWindow(self.camera)
                self.screenSnap()
                self.endShow()
            else:
                cmds.confirmDialog(title='Oops', message=u'请选择动画摄像机', button='Gotcha')

    def browse(self):
        filePath = cmds.fileDialog2(fileMode=3, caption='Set Path')
        if filePath == None:
            return
        else:
            cmds.textField('imagePath', text=filePath[0], edit=True)


    def showPanoramaWindow(self,argCamera):
        #tool`s second UI,this window uesed to be snaped
        #~ if cmds.window('MayaPanoramaPlayBlast', q=1, exists=1):
            #~ cmds.deleteUI('MayaPanoramaPlayBlast')

        if cmds.window('showPanoramaWindow', q=1, exists=1):
            cmds.deleteUI('showPanoramaWindow')
        self.window = cmds.window('showPanoramaWindow', widthHeight=(1536,1024), sizeable=False, topLeftCorner=(0,0),titleBar=False)

        form = cmds.formLayout(width=1536,height=1024)
        editor1 = cmds.modelEditor()
        editor2 = cmds.modelEditor()
        editor3 = cmds.modelEditor()
        editor4 = cmds.modelEditor()
        editor5 = cmds.modelEditor()
        editor6 = cmds.modelEditor()
        self.modelEditor(editor1)
        self.modelEditor(editor2)
        self.modelEditor(editor3)
        self.modelEditor(editor4)
        self.modelEditor(editor5)
        self.modelEditor(editor6)

        #change backgoundcolor
        cmds.displayPref(displayGradient =0)

        cmds.formLayout( form, edit=True, attachForm=[(editor1, 'top', 0), (editor1, 'bottom', 512),(editor1, 'left', 0),(editor1, 'right', 1024),
                                                      (editor2, 'top', 0), (editor2, 'bottom', 512),(editor2, 'right', 512),(editor2, 'left', 512),
                                                      (editor3, 'top', 0), (editor3, 'bottom', 512),(editor3, 'right', 0),(editor3, 'left', 1024),
                                                      (editor4, 'top', 512), (editor4, 'bottom', 0),(editor4, 'left', 0),(editor4, 'right', 1024),
                                                      (editor5, 'top', 512), (editor5, 'bottom', 0),(editor5, 'right', 512),(editor5, 'left', 512),
                                                      (editor6, 'top', 512), (editor6, 'bottom', 0),(editor6, 'right', 0),(editor6, 'left', 1024)],
                         attachControl=[(editor1, 'right', 0, editor2),(editor2, 'right', 0, editor3),(editor4, 'right', 0, editor5),(editor5, 'right', 0, editor6),
                                        (editor1, 'bottom', 0, editor4),(editor2, 'bottom', 0, editor5),(editor3, 'bottom', 0, editor6)])

        cameraShape = self.camera
        if cmds.objectType(self.camera,isType='transform'):
            cameraShape = cmds.listRelatives(self.camera,children=True)[0]
        nearClipPlane = cmds.getAttr(cameraShape+'.nearClipPlane')
        farClipPlane = cmds.getAttr(cameraShape+'.farClipPlane')
        cameraFront= cmds.camera(centerOfInterest=2.450351)
        cmds.setAttr(cameraFront[0]+'.nearClipPlane',nearClipPlane)
        cmds.setAttr(cameraFront[0]+'.farClipPlane',farClipPlane)
        cmds.hide(cameraFront)
        cameraLeft= cmds.camera(centerOfInterest=2.450351,rotation=(0,90,0))
        cmds.setAttr(cameraLeft[0]+'.nearClipPlane',nearClipPlane)
        cmds.setAttr(cameraLeft[0]+'.farClipPlane',farClipPlane)
        cmds.hide(cameraLeft)
        cameraBack= cmds.camera(centerOfInterest=2.450351,rotation=(0,180,0))
        cmds.setAttr(cameraBack[0]+'.nearClipPlane',nearClipPlane)
        cmds.setAttr(cameraBack[0]+'.farClipPlane',farClipPlane)
        cmds.hide(cameraBack)
        cameraRight= cmds.camera(centerOfInterest=2.450351,rotation=(0,270,0))
        cmds.setAttr(cameraRight[0]+'.nearClipPlane',nearClipPlane)
        cmds.setAttr(cameraRight[0]+'.farClipPlane',farClipPlane)
        cmds.hide(cameraRight)
        cameraUp= cmds.camera(centerOfInterest=2.450351,rotation=(90,0,0))
        cmds.setAttr(cameraUp[0]+'.nearClipPlane',nearClipPlane)
        cmds.setAttr(cameraUp[0]+'.farClipPlane',farClipPlane)
        cmds.hide(cameraUp)
        cameraDown= cmds.camera(centerOfInterest=2.450351,rotation=(270,0,0))
        cmds.setAttr(cameraDown[0]+'.nearClipPlane',nearClipPlane)
        cmds.setAttr(cameraDown[0]+'.farClipPlane',farClipPlane)
        cmds.hide(cameraDown)

        #group = cmds.group(camera2,camera3,camera4,camera5,camera6,name='cameraFull')
        #cmds.parentConstraint(camera1,group,maintainOffset=False)
        cmds.parent(cameraFront[0],argCamera, relative=True)
        cmds.parent(cameraLeft[0],argCamera, relative=True)
        cmds.parent(cameraBack[0],argCamera, relative=True)
        cmds.parent(cameraRight[0],argCamera, relative=True)
        cmds.parent(cameraUp[0],argCamera, relative=True)
        cmds.parent(cameraDown[0],argCamera, relative=True)

        cameraList = [cameraFront[1],cameraRight[1],cameraBack[1],cameraLeft[1],cameraUp[1],cameraDown[1]]
        self.tmpCameraList = cameraList
        self.argCamera = argCamera
        for item in cameraList:
            cmds.setAttr('%s.horizontalFilmAperture'%item,1)
            cmds.setAttr('%s.verticalFilmAperture'%item,1)
            cmds.setAttr('%s.filmFit'%item,3)
            cmds.setAttr('%s.focalLength'%item,12.700)

        #~ cmds.modelEditor( editor1, edit=True, camera=cameraFront[0] )
        #~ cmds.modelEditor( editor2, edit=True, camera=cameraLeft[0] )
        #~ cmds.modelEditor( editor3, edit=True, camera=cameraBack[0] )
        #~ cmds.modelEditor( editor4, edit=True, camera=cameraRight[0] )
        #~ cmds.modelEditor( editor5, edit=True, camera=cameraUp[0] )
        #~ cmds.modelEditor( editor6, edit=True, camera=cameraDown[0] )
        cmds.modelEditor( editor1, edit=True, camera=cameraBack[0])
        cmds.modelEditor( editor2, edit=True, camera=cameraUp[0] )
        cmds.modelEditor( editor3, edit=True, camera=cameraDown[0] )
        cmds.modelEditor( editor4, edit=True, camera=cameraLeft[0] )
        cmds.modelEditor( editor5, edit=True, camera=cameraFront[0] )
        cmds.modelEditor( editor6, edit=True, camera=cameraRight[0] )

        cmds.showWindow( self.window )
        cmds.window('showPanoramaWindow', topLeftCorner=(0,0), edit=True)


    #create modelEditor
    def modelEditor(self,argEditor):
        cmds.modelEditor( argEditor, edit=True,
                        headsUpDisplay=1,
                        displayAppearance='smoothShaded',
                        displayTextures = 1,
                        shadows=0,
                        nurbsCurves=0,
                        nurbsSurfaces=0,
                        polymeshes=1,
                        subdivSurfaces=1,
                        planes=0,
                        lights=0,
                        cameras=0,
                        controlVertices=0,
                        grid=0,
                        joints=0,
                        ikHandles=0,
                        deformers=0,
                        dynamics=1,
                        fluids=1,
                        hairSystems=0,
                        follicles=0,
                        hulls=0,
                        nCloths=1,
                        nParticles=1,
                        nRigids=0,
                        dynamicConstraints=0,
                        locators=0,
                        dimensions=0,
                        handles=0,
                        pivots=0,
                        strokes=0,
                        manipulators=1,
                        greasePencils = 0,
                        objectFilterShowInHUD=0,
                        hud=False,
                        displayLights = 'selected',
                                    )


    #create lights in the scene
    def createLights(self=None,camera=None):
        sel = cmds.ls(sl=1)
        amb = cmds.createNode('ambientLight')
        dir1 = cmds.createNode('directionalLight')
        dir2 = cmds.createNode('directionalLight')
        dir3 = cmds.createNode('directionalLight')
        dir4 = cmds.createNode('directionalLight')

        cmds.setAttr(amb+'.intensity',0.6)
        cmds.setAttr(dir1+'.intensity',0.4)
        cmds.setAttr(dir2+'.intensity',0.4)
        cmds.setAttr(dir3+'.intensity',0.3)
        cmds.setAttr(dir4+'.intensity',0.3)

        cmds.setAttr(dir3+'.colorR',0.6)
        cmds.setAttr(dir3+'.colorG',0.6)

        cmds.setAttr(dir2+'.colorB',0.8)

        cmds.setAttr(dir1+'.colorR',0.8)
        cmds.setAttr(dir1+'.colorG',0.8)

        cmds.setAttr(dir4+'.colorG',0.8)
        cmds.setAttr(dir4+'.colorB',0.6)

        cmds.rotate( 65, 0, 145, dir1)
        cmds.rotate( 10, -41, -120, dir2)
        cmds.rotate( -130, -15, 30, dir3)
        cmds.rotate( 225, 485, -245, dir4)
        grp = cmds.group([amb,dir1,dir2,dir3,dir4])
        cmds.setAttr(grp+'.tx', lock=1)
        cmds.setAttr(grp+'.ty', lock=1)
        cmds.setAttr(grp+'.tz', lock=1)
        cmds.setAttr(grp+'.rx', lock=1)
        cmds.setAttr(grp+'.ry', lock=1)
        cmds.setAttr(grp+'.rz', lock=1)
        cmds.setAttr(grp+'.visibility', 0)
        if camera!=None:
            grp = cmds.parent(grp,camera)[0]
#        if sel not in [[],None]:
#            mc.select(sel,noExpand=1)
#        else:
#            mc.select(cl=1)
        return grp


    #do screen snap
    def screenSnap(self):
        self.garbages = []
        minTime = cmds.playbackOptions(q = True,min = True)
        maxTime = cmds.playbackOptions(q = True,max = True)
        self.minTime = minTime
        self.maxTime = maxTime
        #~ cmds.select(cl=1)
        cmds.select(cmds.ls(self.lightsGrp,dag=1,leaf=1))
        mayaWindowParent = shiboken.wrapInstance(long(omui.MQtUtil.findWindow(self.window)), QtGui.QWidget)
        mayaWindowParent.setWindowFlags(mayaWindowParent.windowFlags() | QtCore.Qt.WindowStaysOnTopHint )
        for i in range(int(minTime),int(maxTime)+1):
            print 'frame:', i
            cmds.currentTime(i, edit=True)
            x1 = 7
            y1 = 7
            x2 = mayaWindowParent.width()
            y2 = mayaWindowParent.height()
            cmds.showWindow(self.window)
            self.originalPixmap = QtGui.QPixmap.grabWindow(QtGui.QApplication.desktop().winId(),x1,y1,x2,y2)
            if os.path.isdir(self.imagePath+'/tmp/'):
                pass
            else:
                os.makedirs(self.imagePath+'/tmp/')
            self.filePath = self.imagePath+'/tmp/'+self.imageName+'_%0.4d'%(i-int(minTime)+1)+'.png'
            self.originalPixmap.save(self.filePath, 'png')
            latLongImgPath = os.path.dirname(self.filePath)+'/latlong_'+os.path.basename(self.filePath)
            command = 'start "" "D:/Projects/_mili_common_push/pyUpload/panoViewer.exe" -cubeToLatlong "%s" "%s"'%(latLongImgPath, self.filePath)
            os.popen(command)
            self.garbages += [self.filePath, latLongImgPath]
            
        cmds.delete(self.lightsGrp)
        cmds.delete(self.tmpCameraList)
        
        #remove this window to show recording window
        if cmds.window('showPanoramaWindow', q=1, exists=1):
            cmds.deleteUI('showPanoramaWindow')
        
        self.soundCmdStr = ''
        sound = mel.eval('global string $gPlayBackSlider; $hgPlayBlastTmp =`timeControl -q -s $gPlayBackSlider`;')
        if sound !='':
            tmpWindow = cmds.window('PanormaPlayBlastRecordingWindow', w=300,h=5,title='Recording Sound')
            cmds.rowColumnLayout(nc=1)
            cmds.select(cl=1)
            cmds.modelEditor(viewSelected=1)
            cmds.showWindow(tmpWindow)
            
            soundVideo = cmds.playblast(format='avi',clearCache=1,viewer=0,showOrnaments=1,
                startTime=self.minTime, 
                endTime = self.maxTime,
                filename = self.imagePath+'/tmp/sound_'+self.imageName+'%i.avi'%time.time(),
                percent=100,
                widthHeight=[2,2],
                quality=100,
                #~ compression=codec,
                sound = sound,
                )
            self.garbages += [soundVideo]

            cmds.deleteUI(tmpWindow)
            self.soundCmdStr += '-i "%s" -c:a mp3'%soundVideo

    def endShow(self):
        cmds.displayPref(displayGradient =1)
        if cmds.window('PanormaPlayBlastConvertingWindow', q=1, exists=1):
            cmds.deleteUI('PanormaPlayBlastConvertingWindow')
        tmpWindow = cmds.window('PanormaPlayBlastConvertingWindow', w=300,h=5,title='Converting Video')
        cmds.showWindow(tmpWindow)
        inSeq = self.imagePath+'/tmp/latlong_'+self.imageName+'_%04d.png'
        
        vFrames = self.maxTime-self.minTime+1
        outVideo = self.imagePath+'/'+self.imageName+'.mov'
        aa = ('%s/Maya/Python/Mili/Module/utilties/ffmpeg/ffmpeg.exe -i "%s" %s -vframes %s -vcodec mjpeg -x264opts keyint=1 -y "%s"'%( os.environ['PIPELINE_PATH'],  inSeq, self.soundCmdStr, vFrames, outVideo))
        print aa
        os.popen(aa)
        
        if self.playMovie:
            command = 'start "" "D:/Projects/_mili_common_push/pyUpload/panoViewer.exe" "%s"'%( outVideo)
            print command
            os.popen(command)
        
        if 0:
            outVideo = self.imagePath+'/'+self.imageName+'.mp4'
            aa = ('%s/Maya/Python/Mili/Module/utilties/ffmpeg/ffmpeg.exe -i "%s" %s -vframes %i -vcodec mpeg4 -x264opts keyint=1 -y "%s"'%(os.environ['PIPELINE_PATH'],  inSeq,self.soundCmdStr, vFrames, outVideo))
            print aa
            os.popen(aa)
        
        for i in self.garbages:
            if os.path.isfile(i):
                print 'remove:',i
                try: os.remove(i)
                except: pass
        print 'Done!'
        #shutil.rmtree(self.imagePath+'/tmp/')
        #~ cmds.confirmDialog(title='JobDone', message=u'全景视频已生成\n请去相应路径查看', button='OK')
        cmds.deleteUI(tmpWindow)
    
    def doubleCleanUp(self):
        cmds.displayPref(displayGradient =1)
        for i in ['PanormaPlayBlastConvertingWindow','PanormaPlayBlastRecordingWindow', 'showPanoramaWindow']:
            if cmds.window(i, q=1, exists=1):
                cmds.deleteUI(i)
        newNodes = list(set(cmds.ls())-set(self.allOrigNodes))
        for i in newNodes[::-1]:
            try:
                cmds.delete(i)
            except:
                pass
    
    def activeCam(self):
        try:
            maya3DViewHandle = omui.M3dView()
            activeView = maya3DViewHandle.active3dView()

            cameraDP = om.MDagPath()
            maya3DViewHandle.active3dView().getCamera(cameraDP)
            return cameraDP.fullPathName()
        except:
            return '|persp|perspShape'


if __name__ == '__main__':
    #~ MayaPanoramaPlayBlast(camera='perspShape2')
    MayaPanoramaPlayBlast()
