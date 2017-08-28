
ARNOLD_PATH=/mnt/work/software/arnold/arnold-4.2.10.0
SRC=src
OUT=out

ALEMBIC_HOME=/mnt/proj/software/develop/usd/dependences/alembic-1.5.5
HDF5_HOME=/mnt/proj/software/develop/usd/dependences/HDF5-1.8.9
ILMBASE_HOME=/mnt/proj/software/develop/usd/dependences/IlmBase-2.2.0
BOOST_HOME=/mnt/proj/software/develop/usd/dependences/boost_155
# Third Party libraries
INCLUDES=-I$(ALEMBIC_HOME)/include
INCLUDES+=-I$(HDF5_HOME)/include
INCLUDES+=-I$(ILMBASE_HOME)/include/OpenEXR
INCLUDES+=-I$(BOOST_HOME)/include
all: tempProc
	c++ -o ${OUT}/gpuCacheProcural.so -shared ${OUT}/tempProc.os -L${ARNOLD_PATH}/bin -lai
tempProc:
	c++ -o ${OUT}/tempProc.os -c -fPIC -D_LINUX -fPIC -I. ${INCLUDES} -I${ARNOLD_PATH}/include ${SRC}/ProcMain.cpp
test:
	/mnt/work/software/arnold/arnold-4.2.16.3/bin/kick ./exp/example.ass -o ohyeah_output.jpg -l /home/xukai/Git/build_repo/alembic_build/install/arnold/procedurals