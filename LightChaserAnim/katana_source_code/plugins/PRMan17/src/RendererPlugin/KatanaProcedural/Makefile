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

# The prman procedural
OUTFILENAME=KatanaProcedural.so
OUTFILEPATH = $(OBJDIR)/$(OUTFILENAME)

# Plugin sources and includes
SOURCES = 	src/ErrorReporting.cpp
SOURCES += 	src/main.cpp
SOURCES += 	src/ProducerCache.cpp
SOURCES += 	src/PRManProcedural.cpp
SOURCES += 	src/WriteRI.cpp
SOURCES += 	src/WriteRI_ObjectID.cpp
SOURCES += 	src/WriteRI_Shapes.cpp
SOURCES += 	src/WriteRI_Util.cpp
SOURCES += 	src/WriteRI_Statements.cpp
SOURCES += 	src/WriteRI_Shaders.cpp
SOURCES += 	src/WriteRI_Instances.cpp
SOURCES += 	src/PRManRender.cpp
SOURCES += 	src/PRManLiveRender.cpp
SOURCES += 	src/PRManRenderSettings.cpp
SOURCES += 	src/PRManGlobalSettings.cpp
SOURCES += 	src/WriteRI_Attributes.cpp

INCLUDES = -Iinclude


# Third Party libraries
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
LIBPATH   += -L$(BOOST_HOME)/lib

LIBS      =  -lprman
LIBS      += -lHalf -lIex -lIlmImf -lIlmThread -lImath
LIBS      += -lboost_regex -lboost_filesystem -lboost_system


# PLUGIN APIs sources and includes
PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
SOURCES += $(PLUGIN_SRC)/FnAttribute/client/FnAttribute.cpp
SOURCES += $(PLUGIN_SRC)/FnAttribute/client/FnDataBuilder.cpp
SOURCES += $(PLUGIN_SRC)/FnScenegraphIterator/client/FnScenegraphIterator.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/FnRenderOutputUtils.cpp
SOURCES += $(PLUGIN_SRC)/FnGeolibServices/client/FnArbitraryOutputAttr.cpp
SOURCES += $(PLUGIN_SRC)/FnGeolibServices/client/FnXFormUtil.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/CameraInfo.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/ShadingNodeConnectionDescription.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/ShadingNodeDescription.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/ShadingNodeDescriptionMap.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/XFormMatrix.cpp
SOURCES += $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/RenderSettings.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/RenderBase.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/CameraSettings.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/GlobalSettings.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/CommandLineTools.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/SocketConnection.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/SocketIdSender.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/IdSenderFactory.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/DiskRenderOutputProcess.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/RenderAction.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/NoOutputRenderAction.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/TemporaryRenderAction.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/CopyRenderAction.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/CopyAndConvertRenderAction.cpp
SOURCES += $(PLUGIN_SRC)/FnRendererInfo/plugin/RenderMethod.cpp
SOURCES += $(PLUGIN_SRC)/FnAsset/client/FnDefaultAssetPlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnAsset/client/FnDefaultFileSequencePlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnLogging/client/FnLogging.cpp
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
# at $ARNOLDTREE/bin, but that might lead to potentially unecessary polution of
# the environment that can lead to procedurals loading the wrong versions of
# certain libraries.
# Add as many directories more to RPATH as you need.
RPATH_FLAGS  = -Wl,-zorigin
RPATH_FLAGS += -Wl,--rpath=\$$ORIGIN
RPATH_FLAGS += -Wl,--rpath=\$$ORIGIN/../prman/lib

# Targets:
all: $(OUTFILEPATH)

$(OUTFILEPATH): $(OBJS)
	@echo "  Compiling PRMan plugin..."
	$(CXX) $(CXXFLAGS) $(RPATH_FLAGS) $(OBJS)  $(LIBPATH) $(LIBS) -shared -o $(OUTFILEPATH) -Wl,-soname,$(OUTFILENAME)


$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@


clean:
	@echo "  Cleaning PRMan plugin"
	@rm -rf $(OBJDIR)
