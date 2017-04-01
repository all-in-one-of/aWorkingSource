OUT_PATH = out/
SOURCE_FILE = src/texture_repetition.cpp
ARNOLD_PATH = /mnt/proj/software/develop/katana/sources/ArnoldSDKs/Arnold-4.2.11.3-linux

all:
	@echo "Start Compiling..."
	g++ -o ${OUT_PATH}shaderTexRep.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include ${SOURCE_FILE}
	g++ -o ${OUT_PATH}TextureRepetition.so -shared ${OUT_PATH}shaderTexRep.so -L${ARNOLD_PATH}/bin -lai
	@echo "Done!"

in:
	@echo "Install Shader TextureRepetition ... "
	@cp -f ${OUT_PATH}TextureRepetition.so /mnt/proj/software/develop/katana/utility/shaders/devShaders
	@echo "Done!"
.PHONY : TextureRepetition
