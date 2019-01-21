#!/usr/bin/python

import os,textwrap

def __getMtdTemplet():
    return textwrap.dedent(
        """
        [node %s]
            desc STRING "%s"
            maya.name STRING "%s"
            maya.classification STRING "%s"
            maya.output_name STRING "%s"
            maya.output_shortname STRING "%s"
            maya.id INT %s
        """%("syToons", "shader of sytoon", "syToons", "shader/surface", "outColor", "out", "0x0012d340")
        ).strip()

def read(filepath):
    '''
    Read shader register file like .mtd for maya and houdini,AE and AEXml.
    '''
    pass

def write(filepath):
    '''
    Create regisiter file.
    '''
    f = open(filepath, "w")
    f.write(__getMtdTemplet()) 


filepath = "/home/xukai/Git/git_hub/aWorkingSource/LightChaserAnim/testing/shader_register/test.mtd"
# f = open(, "r")
# print f.read()
write(filepath)


