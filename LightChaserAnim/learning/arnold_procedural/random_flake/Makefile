OUT_PATH = out
SRC_PATH = src
ARNOLD_PATH = /mnt/work/software/arnold/arnold-5.0.1.0

LOGFILE=$(LOGPATH)`date +'%H:%M:%S'`

all:
	@echo [${LOGFILE}] "Start Compiling..."
	@c++ ${SRC_PATH}/random_flake.cpp -o ${OUT_PATH}/random_flake.so -Wall -O2 -shared -fPIC -I${ARNOLD_PATH}/include -L${ARNOLD_PATH}/bin -lai
	@echo [${LOGFILE}] "Done!"
install:
	@echo [${LOGFILE}] "Install Done!"