# Copyright (c) 2016 Mili Pictures. All Rights Reserved.

#coding:utf-8
__author__ = "Kevin Tsui"

import sys,os

os.environ["PIPRLINE_PATH"] = "D:/work/TD/branches/RndTools_dev/Pipeline/Katana"
os.environ["STARTUP_PATH"] = "D:/work/TD/branches/RndTools_dev/Pipeline/Katana/Startup/"

SERVER_PATH = "D:/work/TD/branches/RndTools_dev/Pipeline/Katana"

try:
    import Startup
except ImportError:
    sys.path.insert(0, '%s'%SERVER_PATH)
    import Startup

