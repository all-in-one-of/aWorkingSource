
ARNOLD_PATH="/mnt/work/software/arnold/arnold-4.2.10.0"
OUT=out
SRC=src
all: shader0
	g++ -o ${OUT}/lc_nested_dielectric.so -shared ${OUT}/shader0.os -L${ARNOLD_PATH}/bin -lai
shader0:
	g++ -o ${OUT}/shader0.os -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include ${SRC}/jf_nested_dielectric.cpp
install:
	@echo "Install ... "
	cp -f ${OUT}/lc_nested_dielectric.so /mnt/work/software/develop/katana/shader/4.x/devShaders
	@echo "Done!"
clean:
	rm -f ${OUT}/shader0.os
