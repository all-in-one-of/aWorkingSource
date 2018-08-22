
INSTALL_DIR = ../Release

OUT_PATH = out
SRC_PATH = source
ARNOLD_PATH = /mnt/work/software/arnold/arnold-5.0.1.0
ST_LIB_PATH = ../Release
SPEEDTREE_SDK_PATH = /mnt/work/software/lib/Speedtree-SDK7.0
MAYA_PATH= /mnt/usr/autodesk/maya2017
XGEN_PATH = ${MAYA_PATH}/plug-ins/xgen

LOGFILE=$(LOGPATH)`date +'%H:%M:%S'`
CXX = c++
CXXFLAGS = -Wall -O2 -shared -fPIC

INCLUDES = -I${ARNOLD_PATH}/include
INCLUDES += -I${MAYA_PATH}/include
INCLUDES += -I${XGEN_PATH}/include
INCLUDES += -I${XGEN_PATH}/include/XGen
INCLUDES += -I${XGEN_PATH}/include/SeExpr
INCLUDES += -I${XGEN_PATH}/include/XgPorting
# INCLUDES += -I${ST_LIB_PATH}/include
# INCLUDES += -I${SPEEDTREE_SDK_PATH}/Include
LIBS = -L${ARNOLD_PATH}/bin -lai
LIBS += -L${MAYA_PATH}/lib -lMaya
LIBS += -L${XGEN_PATH}/lib -lAdskSeExpr -lAdskXGen
# LIBS += -L${ST_LIB_PATH}/lib -lSpeedtreeReader
# LIBS += -L${SPEEDTREE_SDK_PATH}/lib -lSpeedTreeCore_fPIC

.PHONY: clean default install start

all:start procedural

procedural:
	@echo [${LOGFILE}] "--Start Compilie Procedural..."
	@${CXX} ${CXXFLAGS} ${SRC_PATH}/XgArnoldProcedural.cpp -o ${OUT_PATH}/xgenk_procedural.so ${INCLUDES} ${LIBS}
	@echo [${LOGFILE}] "--Done!"
install:
	@echo [${LOGFILE}] "--Done!"
start:
	@echo [${LOGFILE}] "--XgArnoldProcedural..."
	@mkdir -p ${OUT_PATH}