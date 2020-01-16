import maya.cmds as cmds
import random



moveZ = random.uniform(1.0,2.0)
moveX = random.uniform(1.0,2.0)
rotateY = random.uniform(0,120)
scaleXZ = random.uniform(0.5,1.5)


for i in range(24):
    cmds.currentTime(i,edit=True)
    if (i % 2 == 0):
        cmds.select('pCylinder1',r = True)
        myStone = cmds.duplicate()
        cmds.parent(myStone[0],'locator1',relative=True)
        cmds.move(moveX,0,moveZ,relative=True,ls=True)
        cmds.rotate(0,rotateY,0,relative=True)
        cmds.scale(scaleXZ,1,scaleXZ)
    else:
        cmds.select('pCylinder1',r = True)
        myStone = cmds.duplicate()
        cmds.parent(myStone[0],'locator1',relative=True)
        cmds.move(moveX,0,-1 * moveZ,relative=True,ls=True)
        cmds.rotate(0,rotateY,0,relative=True)
        cmds.scale(scaleXZ,1,scaleXZ)
        
    cmds.parent(myStone[0],'pathStone_G',relative=False)