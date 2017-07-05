INCLUDES = -I/mnt/proj/software/k2/Arnold-4.2.10.0-linux/include -I/mnt/public/home/john/arnoldshader_src/alShaders-src-1.0.0rc11/common
LIBS = -L/mnt/proj/software/k2/Arnold-4.2.10.0-linux/bin


interior.so : interior.o
	g++ -o $@ -shared $(LIBS) -lai $<
interior.o : interior.cpp
	g++ -o $@ -fPIC -O2 -c $(INCLUDES) $<


