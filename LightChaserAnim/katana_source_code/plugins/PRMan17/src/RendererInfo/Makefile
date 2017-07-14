# ********************************************************************
# This file contains copyrighted work from The Foundry,
# Sony Pictures Imageworks and Pixar, is intended for
# Katana and PRMan customers, and is not for distribution
# outside the terms of the corresponding EULA(s).
# ********************************************************************
#
# See README for instructions on how to use this Makefile
#

# The output objects dir
OBJDIR = ./out

# The plugin .so file
OUTFILENAME=PRManRendererInfo.so
OUTFILEPATH = $(OBJDIR)/$(OUTFILENAME)

# Plugin sources and includes
SOURCES = 	src/PRManRendererInfo.cpp
SOURCES += 	src/Main.cpp

INCLUDES = -Iinclude


# Third Party libraries
INCLUDES  += -I$(RMANTREE)/include

LIBPATH   = -L$(RMANTREE)/lib

LIBS      = -lprman


# PLUGIN APIs sources and includes
PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
SOURCES += $(PLUGIN_SRC)/FnLogging/client/FnLogging.cpp
SOURCES += $(PLUGIN_SRC)/FnAttribute/client/FnAttribute.cpp
SOURCES += $(PLUGIN_SRC)/FnAttribute/client/FnDataBuilder.cpp
SOURCES += $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnRendererInfo/plugin/RendererInfoBase.cpp
SOURCES += $(PLUGIN_SRC)/FnRendererInfo/plugin/RenderMethod.cpp
SOURCES += $(PLUGIN_SRC)/FnRendererInfo/plugin/LiveRenderFilter.cpp
SOURCES += $(PLUGIN_SRC)/FnRendererInfo/plugin/LiveRenderControlModule.cpp
SOURCES += $(PLUGIN_SRC)/FnRendererInfo/plugin/ShaderInfoCache.cpp
SOURCES += $(PLUGIN_SRC)/FnAsset/client/FnDefaultAssetPlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnAsset/client/FnDefaultFileSequencePlugin.cpp
SOURCES += $(PLUGIN_SRC)/pystring/pystring.cpp

INCLUDES += -I$(KATANA_HOME)/plugin_apis/include


# Object files and flags
OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))

CXXFLAGS = -Wall -pipe -m64 -fPIC -DPIC

# RPATH flags - These allow the plugin to find the right shared libraries
# $ORIGIN/../prman/lib assumes that there is a symbolic link in the plugin
# directory called 'prman' pointing at the PRMan's installation directory.
# This symbolic link is optional as LD_LIBRARY_PATH can be used instead to point
# at $RMANTREE/lib, but that might lead to potentially uncessary polution of the
# environment that can lead to procedurals loading the wrong versions of certain
# libraries.
# Add as many directories more to RPATH as you need.
RPATH_FLAGS  = -Wl,-zorigin
RPATH_FLAGS += -Wl,--rpath=\$$ORIGIN
RPATH_FLAGS += -Wl,--rpath=\$$ORIGIN/../prman/lib


# Targets:
all: $(OUTFILEPATH)

$(OUTFILEPATH): $(OBJS)
	@echo "  Compiling PRMan RendererInfo plugin..."
	$(CXX) $(CXXFLAGS) $(RPATH_FLAGS) $(OBJS)  $(LIBPATH) $(LIBS)  -shared -o $(OUTFILEPATH)


$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@


clean:
	@echo "  Cleaning PRMan RendererInfo plugin"
	@rm -rf $(OBJDIR)
