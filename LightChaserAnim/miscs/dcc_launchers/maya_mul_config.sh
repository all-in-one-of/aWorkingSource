#!/bin/sh
# .bashrc


# "------mtoa"
export MTOA_HOME=/mnt/proj/software/develop/maya/mtoa/MtoA-1.4.1.2-linux-2015
export MAYA_MODULE_PATH=$MTOA_HOME
export MAYA_RENDER_DESC_PATH=$MTOA_HOME
export PYTHONPATH=$MTOA_HOME/scripts:$PYTHONPATH
export MAYA_RENDER_DESC_PATH=$MTOA_HOME
export solidangle_LICENSE=6053@10.0.0.5
export MTOA_TEMPLATES_PATH=/mnt/proj/software/develop/maya/utility/lcaShaders/ae
export MAYA_CUSTOM_TEMPLATE_PATH=/mnt/proj/software/develop/maya/utility/lcaShaders/aexml
export ARNOLD_PLUGIN_PATH=/mnt/proj/software/develop/maya/utility/lcaShaders/bin



# "------multiverse"
export MFM_ROOT=/mnt/proj/software/develop/maya/mfm
export MTOA_ROOT=/mnt/proj/software/develop/maya/mtoa/MtoA-1.4.1.2-linux-2015
export MAYA_MODULE_PATH=$MFM_ROOT:$MAYA_MODULE_PATH
export PATH=$MFM_ROOT/bin$PATH
export LD_LIBRARY_PATH=$MFM_ROOT/lib:$MFM_ROOT/2015/procedurals:$MTOA_ROOT/bin:$LD_LIBRARY_PATH



