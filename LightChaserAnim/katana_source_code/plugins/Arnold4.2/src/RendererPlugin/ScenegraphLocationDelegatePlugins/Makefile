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

# The Arnold procedural
OUTFILENAME=arnold_ScenegraphLocationDelegatePlugins.so
OUTFILEPATH = $(OBJDIR)/lib$(OUTFILENAME)
PLUGIN_RELATIVE_PATH=../ArnoldPlugin
PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src

SOURCES = src/ArnoldCurvesScenegraphLocationDelegate.cpp
SOURCES += src/ArnoldGenericScenegraphLocationDelegate.cpp
SOURCES += src/ArnoldNurbspatchScenegraphLocationDelegate.cpp
SOURCES += src/ArnoldPointcloudScenegraphLocationDelegate.cpp
SOURCES += src/ArnoldPolymeshScenegraphLocationDelegate.cpp
SOURCES += src/ArnoldProceduralScenegraphLocationDelegate.cpp
SOURCES += src/ArnoldSphereScenegraphLocationDelegate.cpp
SOURCES += src/ArnoldVolumeScenegraphLocationDelegate.cpp
SOURCES += src/registry.cpp

SOURCES += $(PLUGIN_RELATIVE_PATH)/src/buildCommons.cpp
SOURCES += $(PLUGIN_RELATIVE_PATH)/src/objectSettings.cpp
SOURCES += $(PLUGIN_RELATIVE_PATH)/src/buildLights.cpp
SOURCES += $(PLUGIN_RELATIVE_PATH)/src/buildCamera.cpp
SOURCES += $(PLUGIN_RELATIVE_PATH)/src/buildMaterial.cpp
SOURCES += $(PLUGIN_RELATIVE_PATH)/src/objectId.cpp

SOURCES += $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnScenegraphIterator/client/FnScenegraphIterator.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/ScenegraphLocationDelegate.cpp
SOURCES += $(PLUGIN_SRC)/FnAttribute/client/FnAttribute.cpp
SOURCES += $(PLUGIN_SRC)/FnAttribute/client/FnAttributeUtils.cpp
SOURCES += $(PLUGIN_SRC)/FnAttribute/client/FnDataBuilder.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/FnRenderOutputUtils.cpp
SOURCES += $(PLUGIN_SRC)/FnGeolibServices/client/FnArbitraryOutputAttr.cpp
SOURCES += $(PLUGIN_SRC)/FnGeolibServices/client/FnXFormUtil.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/CameraInfo.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/ShadingNodeConnectionDescription.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/ShadingNodeDescription.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/ShadingNodeDescriptionMap.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/XFormMatrix.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/SocketConnection.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/SocketIdSender.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/IdSenderFactory.cpp
SOURCES += $(PLUGIN_SRC)/FnLogging/client/FnLogging.cpp
SOURCES += $(PLUGIN_SRC)/FnAsset/client/FnDefaultAssetPlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnAsset/client/FnDefaultFileSequencePlugin.cpp
SOURCES += $(PLUGIN_SRC)/pystring/pystring.cpp
SOURCES += $(PLUGIN_SRC)/FnPluginManager/client/FnPluginManager.cpp

INCLUDES = -Iinclude
INCLUDES += -I$(KATANA_HOME)/plugin_apis/include

# Third Party libraries
INCLUDES += -I$(ARNOLDTREE_4_2)/include
INCLUDES  += -I$(OPENEXR_HOME)/include
INCLUDES  += -I../../thirdparty/ptr/include

# Boost
BOOST_INCLUDE ?= $(BOOST_HOME)/include
INCLUDES += -I$(BOOST_INCLUDE)

LIBPATH   = -L$(BOOST_HOME)/lib
LIBPATH   +=  -L$(ARNOLDTREE_4_2)/bin
LIBPATH   += -L$(OPENEXR_HOME)/lib

LIBS      =  -lai -lboost_thread
LIBS      += -lHalf -lIex -lIlmImf -lIlmThread -lImath

INCLUDES += -I$(KATANA_HOME)/plugin_apis/include
INCLUDES += -I$(PLUGIN_RELATIVE_PATH)/include

# Object files and flags
OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))

CXXFLAGS += -Wall -pipe -m64 -fPIC -DPIC


# RPATH flags - These allow the plugin to find the right shared libraries
# $ORIGIN/../arnold/bin assumes that there is a symbolic link in the plugin
# directory called 'arnold' pointing at your Arnold's installation directory.
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
	@echo "  Compiling Arnold Scenegraph Location Delegate Plugins..."
	$(CXX) $(CXXFLAGS) $(RPATH_FLAGS)  $(OBJS)  $(LIBPATH) $(LIBS) -shared -o $(OUTFILEPATH) -Wl,-soname,$(OUTFILENAME)


$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@


clean:
	@echo "  Cleaning Arnold Scenegraph Location Delegate Plugins"
	@rm -rf $(OBJDIR)
