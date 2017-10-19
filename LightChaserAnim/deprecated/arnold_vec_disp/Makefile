OUT_PATH = out/
INCLUDE_PATH=src/*.h
ARNOLD_PATH = /mnt/proj/software/develop/katana/sources/ArnoldSDKs/Arnold-4.2.10.0-linux

all:
	@echo "Start Compiling..."
	g++ -o ${OUT_PATH}tempShader1.os -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/vec_displace.cpp
	#g++ -o ${OUT_PATH}tempShader2.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/texture_blend.cpp
	#g++ -o ${OUT_PATH}loader.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/shader_loader.cpp
	g++ -o ${OUT_PATH}lc_vecDisplace.so -shared ${OUT_PATH}tempShader1.os -I{INCLUDE_PATH} -L${ARNOLD_PATH}/bin -lai
	@echo "Done!"

install:
	@echo "Install Shader TextureRepetition ... "
	cp -f ${OUT_PATH}lc_vecDisplace.so /home/xukai/Git/git_repo/katana/shader/4.x/devShaders
	@echo "Done!"

# args:
# 	@echo "Install Args ... "
# 	@cp -f TextureRepetition.args /mnt/work/software/develop/katana/shader/4.x/devShaders/Args
# 	@cp -f TextureBlend.args /mnt/work/software/develop/katana/shader/4.x/devShaders/Args
# 	@echo "Done!"
