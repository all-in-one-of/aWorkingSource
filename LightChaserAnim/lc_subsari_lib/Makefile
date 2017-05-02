OUT_PATH = out
ARGS_PATH = args
INCLUDE_PATH=src/*.h
ARNOLD_PATH = /mnt/proj/software/develop/katana/sources/ArnoldSDKs/Arnold-4.2.10.0-linux

all:
	@echo "Start Compiling..."
	g++ -o ${OUT_PATH}/shader1.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/histogram_scan.cpp
	g++ -o ${OUT_PATH}/loader.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/shader_loader.cpp
	g++ -o ${OUT_PATH}/lc_subsari.so -shared ${OUT_PATH}/shader1.so ${OUT_PATH}/loader.so -I{INCLUDE_PATH} -L${ARNOLD_PATH}/bin -lai
	@echo "Done!"

in:
	@echo "Install Shader lc_subsari ... "
	@cp -f ${OUT_PATH}/lc_subsari.so /mnt/proj/software/develop/katana/utility/shaders/devShaders
	@echo "Done!"
.PHONY : lc_subsari_so

arg:
	@echo "Install Args ... "
	@cp -f ${ARGS_PATH}/HistogramScan.args /mnt/proj/software/develop/katana/utility/shaders/devShaders/Args
	@echo "Done!"
.PHONY : lc_subsari_args