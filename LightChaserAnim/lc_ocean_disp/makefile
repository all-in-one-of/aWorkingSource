# Makefile for Arnold aaOcean shader

ARNOLD_PATH=/mnt/work/software/arnold/arnold-4.2.16.3
SOURCE=src
OUT=out
INCLUDE=/mnt/work/software/develop/dependences/open_exr/include
LIB=/mnt/work/software/develop/dependences/open_exr/lib
all:obj
	c++ ${SOURCE}/lc_ocean_disp.cpp ${OUT}/aaOceanClass.o -o ${OUT}/lc_ocean_disp.so -Wall -O2 -shared -fPIC -I${ARNOLD_PATH}/include -I${INCLUDE} -L${ARNOLD_PATH}/bin -L${LIB} -lai
obj:
	c++ -c -Wall -g ${SOURCE}/aaOceanClass.cpp -o ${OUT}/aaOceanClass.o