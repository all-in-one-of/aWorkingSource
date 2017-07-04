OUT_PATH = out/
INCLUDE_PATH=src/*.h
ARNOLD_PATH = /mnt/proj/software/develop/katana/sources/ArnoldSDKs/Arnold-4.2.10.0-linux

all:
	@echo "Start Compiling..."
	g++ -o ${OUT_PATH}shaderTexRep.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/texture_repetition.cpp
	g++ -o ${OUT_PATH}shaderTexBld.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/texture_blend.cpp
	g++ -o ${OUT_PATH}shaderTexBldX.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/texture_blendX.cpp
	g++ -o ${OUT_PATH}shaderTexBldY.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/texture_blendY.cpp
	g++ -o ${OUT_PATH}loader.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/loader.cpp
	g++ -o ${OUT_PATH}TextureRepetition.so -shared ${OUT_PATH}shaderTexRep.so ${OUT_PATH}shaderTexBld.so ${OUT_PATH}shaderTexBldX.so ${OUT_PATH}shaderTexBldY.so ${OUT_PATH}loader.so -I{INCLUDE_PATH} -L${ARNOLD_PATH}/bin -lai
	@echo "Done!"

in:
	@echo "Install Shader TextureRepetition ... "
	@cp -f ${OUT_PATH}TextureRepetition.so /mnt/proj/software/develop/katana/utility/shaders/devShaders
	@echo "Done!"
.PHONY : TextureRepetition

macro:
	@echo "Install Macro TextureRepetition ... "
	@cp -f TextureRepetition.macro /mnt/proj/software/develop/katana/utility/resources/Macros
	@echo "Done!"
.PHONY : Macros
