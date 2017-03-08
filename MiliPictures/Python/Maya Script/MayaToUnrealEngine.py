__author__ = 'This guy is bloody cool:kai.xu'

import maya.cmds as cmds
import os
class MayaToUnrealModule():
    def __init__(self):
        self.UnrealEngine4_folder = "ScenesAR"
        self.UnrealEngine4_path = "Game/StarterContent/Maya"
        self.Maya_path = "D:/Unreal.txt"
    def getInformation(self):
        contents = ''
        for ar in cmds.ls(type=["assemblyReference"]):
            allADs = cmds.assembly(ar+".representations",query=True,lr=True)
            tmpRpt = cmds.assembly(ar,query=True,active=True)
            if not allADs:
                continue
            if len(allADs)<4:
                continue
            if len(set(["gpu.abc","box.mb","abc.abc"]) - set([x.lower().split("_")[-1] for x in allADs])) == 0\
                or len(set(["gpu.abc","box.abc","abc.abc"]) - set([x.lower().split("_")[-1] for x in allADs])) == 0:
                temp = allADs[0].split("_")
                asset = ''
                for i in range(len(temp)):
                    if i == len(temp)-1:
                        continue
                    elif i == len(temp)-2:
                        asset  += temp[i]
                    else:
                        asset  += temp[i] + "_"
                label = ar
                cmds.select(ar)
                transform = cmds.xform(worldSpace=True,translation=True,query=True)
                rotation = cmds.xform(worldSpace=True,rotation=True,query=True)
                scale = cmds.xform(worldSpace=True,scale=True,query=True)
                contents += self.setContents(asset,label,transform,rotation,scale)
        return contents
    def setContents(self,asset,label,transform,rotation,scale):
        folder = self.UnrealEngine4_folder
        path = self.UnrealEngine4_path
        text = \
        """"""\
        """Begin Map\n"""\
        """   Begin Level\n"""\
        """      Begin Actor Class=StaticMeshActor Name={0} Archetype=StaticMeshActor'/Script/Engine.Default__StaticMeshActor'\n"""\
        """         Begin Object Class=StaticMeshComponent Name="StaticMeshComponent0" Archetype=StaticMeshComponent'Default__StaticMeshActor:StaticMeshComponent0'\n"""\
        """       End Object\n"""\
        """         Begin Object Name="StaticMeshComponent0"\n"""\
        """            StaticMesh=StaticMesh'/{12}/{0}.{0}'\n"""\
        """            RelativeLocation=(X={2},Y={3},Z={4})\n"""\
        """            RelativeRotation=(Pitch={6},Yaw=-{7},Roll={5})\n"""\
        """            RelativeScale3D=(X={8},Y={9},Z={10})\n"""\
        """         End Object\n"""\
        """         StaticMeshComponent=StaticMeshComponent0\n"""\
        """         RootComponent=StaticMeshComponent0\n"""\
        """         ActorLabel="{1}"\n"""\
        """         FolderPath="{11}"\n"""\
        """      End Actor\n"""\
        """   End Level\n"""\
        """Begin Surface\n"""\
        """End Surface\n"""\
        """End Map\n"""\
        """\n""".format(asset,label,transform[0],transform[2],transform[1],rotation[0],rotation[2],rotation[1],scale[0],scale[2],scale[1],folder,path)
        return text
    def output(self):
        path = self.Maya_path
        file = open(path,"wb")
        file.write(self.getInformation())
        file.close()
    def setUnrealEngine4Path(self,path):
        self.UnrealEngine4_path = path
    def setMayaPath(self,path):
        self.Maya_path = path
    def setUnrealEngine4Folder(self,folder):
        self.UnrealEngine4_folder = folder
    def openTXT(self):
        path = self.Maya_path
        if os.path.isfile(path):
            os.popen(path)
            print path
def main():
    app = MayaToUnrealModule()
    app.setUnrealEngine4Folder("ScenesAR")
    app.setUnrealEngine4Path("Game/StarterContent/Maya")
    app.setMayaPath("D:/Unreal.txt")
    app.output()
    app.openTXT()
if __name__ == "__main__":
    app = MayaToUnrealModule()
    app.setUnrealEngine4Folder("ScenesAR")
    app.setUnrealEngine4Path("Game/StarterContent/Maya")
    app.setMayaPath("D:/Unreal.txt")
    app.output()
    app.openTXT()