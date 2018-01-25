
ARNOLD_PATH="/mnt/work/software/arnold/arnold-5.0.2.1"
NAME=lc_houdini_glow
all:
	c++ src/lc_houdini_glow.cpp -o out/lc_houdini_glow.so -Wall -O2 -shared -fPIC -I${ARNOLD_PATH}/include -L${ARNOLD_PATH}/bin -lai
install:
	@echo "Install ... "
	cp -f out/lc_houdini_glow.so /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders
	@echo "Done!"
