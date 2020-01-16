#!/usr/bin/env bash

#!/usr/bin/env bash

export USD_INSTALL_ROOT=/home/xukai/Documents/TestAppFactory/usd_develop/usd_try_to_complie/install
export USD_DEPEND_LIB=/home/xukai/Documents/TestProgramFactory/usd_develop/usd_compile/out
export PATH=$USD_INSTALL_ROOT/bin:$PATH

export PYSIDE=${USD_DEPEND_LIB}/py2.7-qt4.8.6-pyside1.2.4-64bit-release
export PYTHONPATH=$USD_INSTALL_ROOT/lib/python:\
${USD_DEPEND_LIB}/numpy/lib64/python2.7/site-packages/numpy-1.11.1-py2.7-linux-x86_64.egg:\
${PYSIDE}/lib64/python2.7/site-packages:\
${USD_DEPEND_LIB}/python27_lib:\
/usr/lib/python2.6/site-packages:\
$PYTHONPATH

export LD_LIBRARY_PATH=${USD_DEPEND_LIB}/glew/lib:\
${USD_DEPEND_LIB}/ptex/lib:\
${USD_DEPEND_LIB}/flex/lib:\
${USD_DEPEND_LIB}/tbb44/lib/intel64/gcc4.4:\
${USD_DEPEND_LIB}/oiio/lib:\
/opt/rh/python27/root/usr/lib64:\
${USD_DEPEND_LIB}/python27_lib:\
${USD_DEPEND_LIB}/open_subd/lib:\
${USD_DEPEND_LIB}/open_exr/lib:\
${USD_DEPEND_LIB}/boost_155/lib:\
${USD_DEPEND_LIB}/double_conversion/lib64:\
${USD_INSTALL_ROOT}/plugin:\
${PYSIDE}/lib:\
${LD_LIBRARY_PATH}
USD_INCLUDE=$USD_INSTALL_ROOT/include
USD_LIB=$USD_INSTALL_ROOT/lib

USD_MAYA_ROOT=$USD_INSTALL_ROOT/third_party/maya
USD_MAYA_INCLUDE=$USD_MAYA_ROOT/include
USD_MAYA_LIB=$USD_MAYA_ROOT/lib

export PATH=$USD_MAYA_ROOT/bin:$PATH
export LD_LIBRARY_PATH=$USD_MAYA_LIB:$USD_LIB:$LD_LIBRARY_PATH
export MAYA_SCRIPT_PATH=$USD_MAYA_ROOT/share/usd/plugins/usdMaya/resources:$MAYA_SCRIPT_PATH
export MAYA_PLUG_IN_PATH=$USD_MAYA_ROOT/plugin:$MAYA_PLUG_IN_PATH