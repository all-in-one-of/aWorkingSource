# ********************************************************************
# This file contains copyrighted work from The Foundry,
# Sony Pictures Imageworks and Solid Angle, is intended for
# Katana and Solid Angle customers, and is not for distribution
# outside the terms of the corresponding EULA(s).
# ********************************************************************
#
# See README for instructions on how to use this Makefile
#

# The output objects dir
OBJDIR = ./out

# The Arnold procedural
OUTFILENAME=ArnoldPlugin.so
OUTFILEPATH = $(OBJDIR)/lib$(OUTFILENAME)

# Plugin sources and includes
SOURCES +=  src/ArnoldRender.cpp
SOURCES +=  src/ArnoldRenderSettings.cpp

SOURCES +=  src/arnold_capabilities.cpp
SOURCES +=  src/buildCamera.cpp
SOURCES +=  src/buildGeometry.cpp
SOURCES +=  src/buildGroup.cpp
SOURCES +=  src/buildInstance.cpp
SOURCES +=  src/buildLights.cpp
SOURCES +=  src/buildMaterial.cpp
SOURCES +=  src/objectId.cpp
SOURCES +=  src/change_q.cpp
SOURCES +=  src/driver_katana.cpp
SOURCES +=  src/procedural.cpp
SOURCES +=  src/buildCommons.cpp
SOURCES +=  src/objectSettings.cpp

INCLUDES = -Iinclude
INCLUDES += -I$(KATANA_HOME)/plugin_apis/include

# Third Party libraries
INCLUDES += -I$(ARNOLDTREE_4_1)/include
INCLUDES  += -I$(OPENEXR_HOME)/include
INCLUDES  += -I$(ZMQ_HOME)/include

# Boost
BOOST_INCLUDE ?= $(BOOST_HOME)/include
INCLUDES += -I$(BOOST_INCLUDE)

LIBPATH   = -L$(BOOST_HOME)/lib
LIBPATH   +=  -L$(ARNOLDTREE_4_1)/bin
LIBPATH   += -L$(OPENEXR_HOME)/lib
LIBPATH   += -L$(ZMQ_HOME)/lib

LIBS      = -Wl,-Bstatic -lzmq_static -lboost_thread
LIBS      += -Wl,-Bdynamic -lai -lHalf -lIex -lIlmImf -lIlmThread -lImath

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
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/RenderBase.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/RenderSettings.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/CameraSettings.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/GlobalSettings.cpp
SOURCES += $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/SocketConnection.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/DiskRenderOutputProcess.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/RenderAction.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/NoOutputRenderAction.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/TemporaryRenderAction.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/CopyRenderAction.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/CopyAndConvertRenderAction.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/SocketIdSender.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/IdSenderFactory.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/CloseMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/EndOfRenderMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/IsAliveMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/ContextSingleton.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/DataMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/KatanaPipe.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/Message.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/NewChannelMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/NewFrameMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/KatanaPipeSingleton.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/ZmqHelpers.cpp
SOURCES += $(PLUGIN_SRC)/FnRendererInfo/plugin/RenderMethod.cpp
SOURCES += $(PLUGIN_SRC)/FnAsset/client/FnDefaultAssetPlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnAsset/client/FnDefaultFileSequencePlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnLogging/client/FnLogging.cpp
SOURCES += $(PLUGIN_SRC)/pystring/pystring.cpp
SOURCES += $(PLUGIN_SRC)/FnPluginManager/client/FnPluginManager.cpp

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
	@echo "  Compiling Arnold plugin..."
	$(CXX) $(CXXFLAGS) $(RPATH_FLAGS)  $(OBJS)  $(LIBPATH) $(LIBS) -shared -o $(OUTFILEPATH) -Wl,-soname,$(OUTFILENAME)


$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@


clean:
	@echo "  Cleaning Arnold plugin"
	@rm -rf $(OBJDIR)
