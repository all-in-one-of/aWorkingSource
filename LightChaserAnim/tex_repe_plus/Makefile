OUT_PATH = out/
INCLUDE_PATH=src/*.h
ARNOLD_PATH = /mnt/proj/software/develop/katana/sources/ArnoldSDKs/Arnold-4.2.10.0-linux

all:
	@echo "Start Compiling..."
	g++ -o ${OUT_PATH}tempShader1.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/texture_repetition.cpp
	g++ -o ${OUT_PATH}tempShader2.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/texture_blend.cpp
	g++ -o ${OUT_PATH}loader.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/shader_loader.cpp
	g++ -o ${OUT_PATH}LcaTextureRepetition.so -shared ${OUT_PATH}tempShader1.so ${OUT_PATH}tempShader2.so ${OUT_PATH}loader.so -I{INCLUDE_PATH} -L${ARNOLD_PATH}/bin -lai
	@echo "Done!"
.PHONY : TextureRepetition

in:
	@echo "Install Shader TextureRepetition ... "
	@cp -f ${OUT_PATH}LcaTextureRepetition.so /mnt/proj/software/develop/katana/utility/shaders/devShaders
	@echo "Done!"
.PHONY : TextureRepetition

args:
	@echo "Install Args ... "
	@cp -f TextureRepetition.args /mnt/proj/software/develop/katana/utility/shaders/devShaders/Args
	@cp -f TextureBlend.args /mnt/proj/software/develop/katana/utility/shaders/devShaders/Args
	@echo "Done!"
.PHONY : TextureRepetition