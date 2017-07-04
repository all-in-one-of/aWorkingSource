
ARNOLD_PATH="/mnt/work/software/arnold/arnold-4.2.10.0"
OUT=out
SRC=src
all: shader1 shader2 loader
	g++ -o ${OUT}/lca_diffuseo.so -shared ${OUT}/shader1.os ${OUT}/shader2.os ${OUT}/loader.so -L${ARNOLD_PATH}/bin -lai
shader1:
	g++ -o ${OUT}/shader1.os -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include ${SRC}/cryptomatte.cpp
shader2:
	g++ -o ${OUT}/shader2.os -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include ${SRC}/alSurface.cpp
loader:
	g++ -o ${OUT}/loader.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include ${SRC}/loader.cpp
install:
	@echo "Install ... "
	@cp -f ${OUT}/lca_diffuseo.so /mnt/work/software/develop/katana/shader/4.x/previewShaders
	@echo "Done!"
clean:
	rm -f ${OUT}/shader1.os ${OUT}/shader2.os ${OUT}/loader.so
