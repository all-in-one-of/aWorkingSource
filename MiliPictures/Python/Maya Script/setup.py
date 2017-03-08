
from distutils.core import setup  
import py2exe  
import sys,os

includes = ["encodings", "encodings.*"]    
if len(sys.argv) == 1:
    sys.argv.append("py2exe")
    sys.argv.append("-q")
#~ options = {"py2exe":   { 
                        #~ "bundle_files": 1 ,
                    #~ }    
                #~ }   
options = {"py2exe": {  "compressed": 1,
                        "optimize": 0,
                        "ascii": 0,
                        "bundle_files": 3,
                        #~ "bundle_files": 1,
                        "includes":["sip"],
                        "dist_dir": "D:/CodesUploader",
                        #~ "dist_dir": os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))+'/bin/pyUpload',
                    }
                }


setup(options = options,  
      zipfile=None,   
      #~ console = [{"script":'pipelineInstall.py'}]
      windows = [{"script":'TDsCodeUpload_PySide.py'}]
    )  

