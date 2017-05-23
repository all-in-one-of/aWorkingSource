
ARNOLD_PATH=/mnt/work/software/arnold/arnold-4.2.10.0
SRC=src
OUT=out
MAYA_HOME=/mnt/usr/autodesk/maya2015
XGEN_HOME=/mnt/usr/autodesk/maya2015/plug-ins/xgen
INCLUDES += -I$(XGEN_HOME)/include/xgen
INCLUDES += -I$(MAYA_HOME)/include

all: tempProc
	c++ -o ${OUT}/xgArnoldProcural.so -shared ${OUT}/tempProc.os -L${ARNOLD_PATH}/bin -lai
tempProc:
	c++ -o ${OUT}/tempProc.os -c -fPIC -D_LINUX -fPIC -I. ${INCLUDES} -I${ARNOLD_PATH}/include ${SRC}/XgArnoldProcedural.cpp