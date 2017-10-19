# See README for instructions on how to use this Makefile

# The output objects dir
OBJDIR = out
INSTALLDIR = out
KATANA_HOME = /mnt/work/software/katana/katana2.1v5
OPENEXR_HOME = /mnt/work/software/develop/dependence/openexr-2.2.0
OUTFILENAME = ArnoldXGenOp.so
OUTFILEPATH = $(OBJDIR)/$(OUTFILENAME)

# Plugin sources and includes
PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
SOURCES = src/ArnoldXGenOp.cpp

# Directly suck in the FnAttribute/FnGeolibOp/FnGeolibUtil/pystring cpp files
SOURCES += $(shell find $(PLUGIN_SRC)/FnAttribute -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnGeolib/op -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnGeolib/util -name \*.cpp)
SOURCES += $(PLUGIN_SRC)/FnGeolibServices/client/FnXFormUtil.cpp
SOURCES += $(shell find $(PLUGIN_SRC)/FnPluginManager -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/pystring -name \*.cpp)
SOURCES += $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp

INCLUDES  = -I$(KATANA_HOME)/plugin_apis/include
INCLUDES += -I$(OPENEXR_HOME)/include -I$(OPENEXR_HOME)/include/OpenEXR

CFLAGS=-ffloat-store
CXXFLAGS=-Wall -Wextra -Wshadow -Wconversion -Wcast-qual -fPIC -DPIC

# Object files and flags
OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))

LIBPATH  = -L$(KATANA_HOME)/bin/Geolib3/internal/CEL
LIBPATH += -L$(OPENEXR_HOME)/lib

LIBS = -lCEL -Wl,-Bstatic -lImath -lHalf -lIex -Wl,-Bdynamic

# VERSION_SCRIPT - This allows the DSO to strictly limit which symbols are made
# visible to the host, to avoid symbol conflicts
VERSION_SCRIPT = -Wl,--version-script=../common/plugin.map

# Targets:
all: $(OUTFILEPATH)

$(OUTFILEPATH): $(OBJS)
	@echo "  Compiling ArnoldXgenOp "
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBPATH) $(LIBS) -shared -o $(OUTFILEPATH) -Wl,-soname,$(OUTFILENAME)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "  Cleaning ArnoldXGenOp"
	@rm -rf $(OBJDIR)

install: all ArnoldXGenOp.py
	@echo "  Installing to $(INSTALLDIR)"
	@mkdir -p $(INSTALLDIR)/Libs
	@mkdir -p $(INSTALLDIR)/Plugins
	@cp $(OUTFILEPATH) $(INSTALLDIR)/Libs/
	@cp ArnoldXGenOp.py $(INSTALLDIR)/Plugins/

