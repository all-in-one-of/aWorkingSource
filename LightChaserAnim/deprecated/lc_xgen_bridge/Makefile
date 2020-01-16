OUT_PATH = out
INCLUDE_PATH=src/*.h
ARNOLD_PATH = /mnt/work/software/arnold/arnold-5.0.1.1

all:
	@echo "Start Compiling..."
	g++ -o ${OUT_PATH}/tempShader1.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/lc_xgen_rgb.cpp
	g++ -o ${OUT_PATH}/tempShader2.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/lc_xgen_rgba.cpp
	g++ -o ${OUT_PATH}/loader.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/shader_loader.cpp
	g++ -o ${OUT_PATH}/lc_xgen_bridge.so -shared ${OUT_PATH}/tempShader1.so ${OUT_PATH}/tempShader2.so ${OUT_PATH}/loader.so -I{INCLUDE_PATH} -L${ARNOLD_PATH}/bin -lai
	@echo "Done!"

install:
	@echo "Install Shader lc_xgen_bridge ... "
	@cp -f ${OUT_PATH}/lc_xgen_bridge.so /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders
	@echo "Done!"

test:
	#python ./ref/gParseJson.py
	python ./ref/gParseXml.py