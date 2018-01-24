# ********************************************************************
# This file contains copyrighted work from The Foundry,
# Sony Pictures Imageworks and Solid Angle, is intended for
# Katana and Solid Angle customers, and is not for distribution
# outside the terms of the corresponding EULA(s).
# ********************************************************************
# See README for instructions on how to use this Makefile
#

# The output objects dir
OBJDIR = ./out

# The plugin .so file
OUTFILENAME=ArnoldRendererInfo.so
OUTFILEPATH = $(OBJDIR)/$(OUTFILENAME)

# Plugin sources and includes
SOURCES = 	src/ArnoldRendererInfo.cpp
SOURCES += 	src/Main.cpp

INCLUDES = -Iinclude


# Third Party libraries
LIBS      += -ldl

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
LIBSRC += $(PLUGIN_SRC)/pystring/pystring.cpp

INCLUDES += -I$(KATANA_HOME)/plugin_apis/include


# Object files and flags
OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))

CXXFLAGS += -Wall -pipe -m64 -fPIC -DPIC 


# RPATH flags - These allow the plugin to find the right shared libraries
# $ORIGIN/../arnold/bin assumes that there is a symbolic link in the plugin
# directory called 'arnold' pointing at your Arnold's installation directory.
# This symbolic link is optional as LD_LIBRARY_PATH can be used instead to point
# at $ARNOLDTREE/bin, but that might lead to potentially uncessary polution of
# the environment that can lead to procedurals loading the wrong versions of
# certain libraries.
# Add as many directories more to RPATH as you need.
RPATH_FLAGS  = -Wl,-zorigin
RPATH_FLAGS += -Wl,--rpath=\$$ORIGIN
RPATH_FLAGS += -Wl,--rpath=\$$ORIGIN/../arnold/bin


# Targets:
all: $(OUTFILEPATH)

$(OUTFILEPATH): $(OBJS)
	@echo "  Compiling Arnold RendererInfo plugin..."
	$(CXX) $(RPATH_FLAGS) $(CXXFLAGS) $(OBJS) $(LIBPATH) $(LIBS) -shared -o $(OUTFILEPATH)


$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "  Cleaning Arnold RendererInfo plugin"
	@rm -rf $(OBJDIR)
