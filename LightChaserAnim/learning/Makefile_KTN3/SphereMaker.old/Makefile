# See README for instructions on how to use this Makefile

KATANA_HOME=/mnt/work/software/katana/katana3.0v1b4

# The output objects dir
OBJDIR = ./out
INSTALLDIR = $(KATANA_HOME)/plugins/Resources/Examples/Ops

OUTFILENAME = SphereMaker.so
OUTFILEPATH = $(OBJDIR)/$(OUTFILENAME)

# Plug-in sources and includes
PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
SOURCES = src/op.cpp

# Directly suck in the FnAttribute/FnGeolibOp/FnGeolibUtil/pystring cpp files
SOURCES += $(shell find $(PLUGIN_SRC)/FnAttribute -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnGeolib/op -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnGeolib/util -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnGeolibServices/client -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnPluginManager -name \*.cpp)
# SOURCES += $(PLUGIN_SRC)/FnGeolibServices/client/FnGeolibCookInterfaceUtilsService.cpp
SOURCES += $(shell find $(PLUGIN_SRC)/pystring -name \*.cpp)
SOURCES += $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp

INCLUDES = -I$(KATANA_HOME)/plugin_apis/include

CFLAGS=-ffloat-store
CXXFLAGS=-Wall -Wextra -Wshadow -Wconversion -Wcast-qual -fPIC -DPIC

# Object files and flags
OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))

LIBPATH = -L$(KATANA_HOME)/bin
LIBS = -lCEL

# Targets:
# all: $(OUTFILEPATH)
all:
	$(CXX) $(CXXFLAGS) src/op.cpp -L/mnt/work/software/katana/katana3.0v1b4/bin ${INCLUDES} -shared -o $(OUTFILEPATH)

$(OUTFILEPATH): $(OBJS)
	@echo "  Compiling SphereMaker"
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBPATH) $(LIBS) -shared -o $(OUTFILEPATH) -Wl,--no-undefined,$(OUTFILENAME)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "  Cleaning SphereMaker"
	@rm -rf $(OBJDIR)

install:
	@echo "  Installing to $(INSTALLDIR)"
	@mkdir -p $(INSTALLDIR)
	@cp $(OUTFILEPATH) $(INSTALLDIR)
