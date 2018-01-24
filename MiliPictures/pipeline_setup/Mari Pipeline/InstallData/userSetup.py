import sys
import globalSet
ROOTPATH = globalSet.ROOTPATH
try:
    from Scripts import __init__
except ImportError:
    sys.path.insert(0, '%s'%ROOTPATH)
    from Scripts import __init__

