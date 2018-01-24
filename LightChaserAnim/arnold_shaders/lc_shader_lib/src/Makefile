
ARNOLD_TREE ?= /home/mike/Development/arnold-current
ARNOLD_INCLUDE = -I$(ARNOLD_TREE)/include
ARNOLD_LIBDIR = -L$(ARNOLD_TREE)/bin
ARNOLD_LIB = -lai

INCLUDES = $(ARNOLD_INCLUDE) -I.
LIBDIRS = $(ARNOLD_LIBDIR)
LIBS = $(ARNOLD_LIB)

SHADERLIB_PREFIX ?= sa_

#CXXFLAGS ?= -g3 -fPIC -DSHADERLIB_PREFIX=\"$(SHADERLIB_PREFIX)\"
CXXFLAGS ?= -O3 -fPIC -DSHADERLIB_PREFIX=\"$(SHADERLIB_PREFIX)\"
LINKFLAGS ?= -shared -fPIC
CXX ?= g++

HEADERS = color_utils.h memory.h
SOURCES = abs.cpp add.cpp blackbody.cpp cache.cpp clamp.cpp color_convert.cpp \
          color_correct.cpp color_utils.cpp complement.cpp cross.cpp \
          divide.cpp dot.cpp exp.cpp length.cpp max.cpp min.cpp mix.cpp \
          multiply.cpp normalize.cpp pow.cpp random.cpp range.cpp \
          reciprocal.cpp rgb_to_float.cpp  rgb_to_vector.cpp shaderlib.cpp \
          subtract.cpp user_data_float.cpp user_data_int.cpp user_data_rgb.cpp \
          user_data_rgba.cpp user_data_string.cpp vector_to_rgb.cpp \
          volume_collector.cpp
OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all shaders clean

all: shaders

shaders: $(SHADERLIB_PREFIX)shaderlib.so

.cpp.o: $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDES)

$(SHADERLIB_PREFIX)shaderlib.so: $(OBJECTS)
	$(CXX) $(OBJECTS) $(LIBDIRS) $(LIBS) $(LINKFLAGS) -o $@

clean:
	rm -f *.so *.o

