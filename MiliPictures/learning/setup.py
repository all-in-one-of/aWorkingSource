from distutils.core import setup
import py2exe
import sys
from glob import glob
from distutils.filelist import findall

EXE_Path = "D:/ProcessMonitor"
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
                        #~"bundle_files": 1,
                        'dist_dir':EXE_Path,
                        #~'build_dir':"D:/test/dir",
                        #~"includes":["sip"]
                    }
                }

#datafiles = [('images', [r'F:\shotgun\images\jlwz_02.jpg'])]

datafiles = [
        ("images", glob('images/*.*'))
]

datafiles.append(('imageformats', [
                'C:\Python27\Lib\site-packages\PySide\plugins\imageformats\qjpeg4.dll'
                ]))

#datafiles.extend(matplotlib.get_py2exe_datafiles())

setup(options = options,
      data_files = datafiles,
      zipfile=None,
      #~console = [{"script":'ProcessMonitor.py'}]
      windows = [{"script":'helloworld.py'}]
    )
