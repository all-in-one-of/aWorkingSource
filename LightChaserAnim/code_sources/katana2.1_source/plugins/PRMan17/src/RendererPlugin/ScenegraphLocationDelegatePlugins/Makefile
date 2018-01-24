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
OUTFILENAME=ScenegraphLocationDelegatePlugins.so
OUTFILEPATH = $(OBJDIR)/$(OUTFILENAME)


SOURCES = src/PRManBrickMapScenegraphLocationDelegate.cpp
SOURCES += src/PRManCurvesScenegraphLocationDelegate.cpp
SOURCES += src/PRManNurbspatchScenegraphLocationDelegate.cpp
SOURCES += src/PRManPointCloudScenegraphLocationDelegate.cpp
SOURCES += src/PRManPolymeshScenegraphLocationDelegate.cpp
SOURCES += src/PRManProceduralScenegraphLocationDelegate.cpp
SOURCES += src/PRManRibArchiveScenegraphLocationDelegate.cpp
SOURCES += src/PRManSphereScenegraphLocationDelegate.cpp
SOURCES += src/PRManSpheresScenegraphLocationDelegate.cpp
SOURCES += src/PRManSubdmeshScenegraphLocationDelegate.cpp
SOURCES += src/PRManVolumeScenegraphLocationDelegate.cpp
SOURCES += src/registry.cpp

INCLUDES = -Iinclude


# Third Party libraries

KATANAPROCEDURAL_RELATIVE_PATH=../KatanaProcedural
SOURCES += $(KATANAPROCEDURAL_RELATIVE_PATH)/src/ErrorReporting.cpp
SOURCES += $(KATANAPROCEDURAL_RELATIVE_PATH)/src/WriteRI_Util.cpp
SOURCES += $(KATANAPROCEDURAL_RELATIVE_PATH)/src/WriteRI_Shaders.cpp
SOURCES += $(KATANAPROCEDURAL_RELATIVE_PATH)/src/WriteRI_Statements.cpp
SOURCES += $(KATANAPROCEDURAL_RELATIVE_PATH)/src/WriteRI_Attributes.cpp
SOURCES += $(KATANAPROCEDURAL_RELATIVE_PATH)/src/WriteRI_ObjectID.cpp
INCLUDES += -I$(KATANAPROCEDURAL_RELATIVE_PATH)/include

INCLUDES += -I$(RMANTREE)/include
INCLUDES  += -I$(OPENEXR_HOME)/include/OpenEXR
INCLUDES  += -I$(OPENEXR_HOME)/include
INCLUDES  += -I../../thirdparty/ptr/include
INCLUDES  += -I../thirdparty/openssl/include

# Boost
BOOST_INCLUDE ?= $(BOOST_HOME)/include
INCLUDES += -I$(BOOST_INCLUDE)

LIBPATH   =  -L$(RMANTREE)/lib
LIBPATH   += -L$(OPENEXR_HOME)/lib


LIBS      =  -lprman
LIBS      += -lHalf -lIex -lIlmImf -lIlmThread -lImath


# PLUGIN APIs sources and includes
PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
SOURCES += $(PLUGIN_SRC)/FnScenegraphIterator/client/FnScenegraphIterator.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/ScenegraphLocationDelegate.cpp
SOURCES += $(PLUGIN_SRC)/FnAttribute/client/FnAttribute.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/FnRenderOutputUtils.cpp
SOURCES += $(PLUGIN_SRC)/FnGeolibServices/client/FnArbitraryOutputAttr.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/CameraInfo.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/ShadingNodeConnectionDescription.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/ShadingNodeDescription.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/ShadingNodeDescriptionMap.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/XFormMatrix.cpp
SOURCES += $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnAttribute/client/FnDataBuilder.cpp
SOURCES += $(PLUGIN_SRC)/FnLogging/client/FnLogging.cpp
SOURCES += $(PLUGIN_SRC)/FnAsset/client/FnDefaultAssetPlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnAsset/client/FnDefaultFileSequencePlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/SocketConnection.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/SocketIdSender.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/IdSenderFactory.cpp
SOURCES += $(PLUGIN_SRC)/pystring/pystring.cpp
SOURCES += $(PLUGIN_SRC)/FnPluginManager/client/FnPluginManager.cpp

INCLUDES += -I$(KATANA_HOME)/plugin_apis/include


# Object files and flags
OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))          

CXXFLAGS = -Wall -pipe -m64 -fPIC -DPIC

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
RPATH_FLAGS += -Wl,--rpath=\$$ORIGIN/../prman/lib

# Targets:
all: $(OUTFILEPATH)

$(OUTFILEPATH): $(OBJS)
	@echo "  Compiling PRMan Scenegraph Location Delegate plugin..."
	$(CXX) $(CXXFLAGS) $(RPATH_FLAGS) $(OBJS)  $(LIBPATH) $(LIBS) -shared -o $(OUTFILEPATH) -Wl,-soname,$(OUTFILENAME)


$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@


clean:
	@echo "  Cleaning PRMan Scenegraph Location Delegate plugin"
	@rm -rf $(OBJDIR)
