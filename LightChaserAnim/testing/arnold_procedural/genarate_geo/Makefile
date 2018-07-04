OUT_PATH = out
SRC_PATH = src
ARNOLD_PATH = /mnt/work/software/arnold/arnold-5.1.0.0

LOGFILE=$(LOGPATH)`date +'%H:%M:%S'`

all:
	@echo [${LOGFILE}] "Start Compiling..."
	@c++ ${SRC_PATH}/genarate_geo.cpp -o ${OUT_PATH}/genarate_geo.so -Wall -O2 -shared -fPIC -I${ARNOLD_PATH}/include -L${ARNOLD_PATH}/bin -lai
	@echo [${LOGFILE}] "Done!"
install:
	@echo [${LOGFILE}] "Install Done!"