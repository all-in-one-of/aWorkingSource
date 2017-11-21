ARNOLD_PATH="/mnt/work/software/arnold/arnold-5.0.1.0"

all:
	@echo -e '\033[0;33m'"Compile..."'\033[0m'
	c++ src/lc_uv_emitter.cpp -o out/lc_uv_emitter.so -Wall -O2 -shared -fPIC -I${ARNOLD_PATH}/include -L${ARNOLD_PATH}/bin -lai
	@echo -e '\033[0;33m'"Finish!"'\033[0m'
	@echo ""
install:
	@echo -e '\033[0;33m'"Install..."'\033[0m'
	cp -f out/lc_uv_emitter.so /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders
	cp -f src/lc_uv_emitter.mtd /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders
	cp -f src/lc_uv_emitter.args /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders/Args
	@echo -e '\033[0;33m'"--- Job Done! ---!"'\033[0m'
