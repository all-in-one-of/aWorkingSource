OUT_PATH = out
SRC_PATH = src
ARNOLD_PATH = /mnt/work/software/arnold/arnold-5.0.1.0

LOGFILE=$(LOGPATH)`date +'%H:%M:%S'`

all:procedural shader

procedural:
	@echo [${LOGFILE}] "Start Compilie Procedural..."
	@c++ ${SRC_PATH}/speedtree_procedural.cpp -o ${OUT_PATH}/speedtree_procedural.so -Wall -O2 -shared -fPIC -I${ARNOLD_PATH}/include -L${ARNOLD_PATH}/bin -lai
	@echo [${LOGFILE}] "Done!"
shader:
	@echo [${LOGFILE}] "Start Compilie Shader..."
	@c++ ${SRC_PATH}/speedtree_shader.cpp -o ${OUT_PATH}/speedtree_shader.so -Wall -O2 -shared -fPIC -I${ARNOLD_PATH}/include -L${ARNOLD_PATH}/bin -lai
	@echo [${LOGFILE}] "Done!"
op:
	@echo [${LOGFILE}] "Start Compilie Katana Op..."
	@echo [${LOGFILE}] "Done!"	
install:
	@echo [${LOGFILE}] "Install..."
	@echo [${LOGFILE}] "Done!"