import pymel.core as pm
import sys
try:
    from Mili.Menu import mayaMenuBoot
    mayaMenuBoot.start(False)
    pm.mel.trace("Maya Root: Interface")
    sys.path.insert(0, 'D:/work/TD/RndTools/PIPE_Lib/Maya2014')
except ImportError:
    import sys
    sys.path.insert(0, 'D:/work/TD/RndTools/Pipeline/Maya/Python')
    from Mili.Menu import mayaMenuBoot 
    mayaMenuBoot.start(True)
    pm.mel.trace("Maya Root: Standalone")
finally:
    pm.mel.trace("Tool is Loading")
