# Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.
# See README for instructions on how to use this Makefile
#

# The output objects dir
OBJDIR = ./out

# The plugin .so file
OUTFILENAME=TemplateRenderPlugin.so
OUTFILEPATH = $(OBJDIR)/$(OUTFILENAME)

# Plugin sources and includes
SOURCES +=  src/TemplateRenderPlugin.cpp
INCLUDES = -Iinclude
INCLUDES += -I$(KATANA_HOME)/plugin_apis/include

# Third Party libraries
INCLUDES += -I$(OPENEXR_HOME)/include
INCLUDES += -I$(ZMQ_HOME)/include

# Boost
BOOST_INCLUDE ?= $(BOOST_HOME)/include
INCLUDES += -I$(BOOST_INCLUDE)

LIBPATH = -L$(BOOST_HOME)/lib
LIBPATH += -L$(ZMQ_HOME)/lib

LIBS = -Wl,-Bstatic -lzmq_static -lboost_thread
LIBS += -Wl,-Bdynamic

# PLUGIN APIs sources and includes
PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src

# Base Foundry Plugin API
SOURCES += $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp

# Render Plugin API: Base
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/RenderBase.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/CameraSettings.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/DiskRenderOutputProcess.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/GlobalSettings.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/RenderSettings.cpp

# Render Plugin API: Render Actions
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/RenderAction.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/NoOutputRenderAction.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/TemporaryRenderAction.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/CopyRenderAction.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/CopyAndConvertRenderAction.cpp

# Render Plugin API: Interprocess Communication
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/IdSenderFactory.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/SocketConnection.cpp
SOURCES += $(PLUGIN_SRC)/FnRender/plugin/SocketIdSender.cpp

# Renderer Info API
SOURCES += $(PLUGIN_SRC)/FnRendererInfo/plugin/RenderMethod.cpp

# Render Output Utilities API
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/FnRenderOutputUtils.cpp
SOURCES += $(PLUGIN_SRC)/FnGeolibServices/client/FnArbitraryOutputAttr.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/CameraInfo.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/ShadingNodeConnectionDescription.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/ShadingNodeDescription.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/ShadingNodeDescriptionMap.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputUtils/client/XFormMatrix.cpp

# Display Driver API
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/ContextSingleton.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/CloseMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/DataMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/KatanaPipe.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/KatanaPipeSingleton.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/Message.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/NewChannelMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/NewFrameMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/ZmqHelpers.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/IsAliveMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/EndOfRenderMessage.cpp

# Asset API
SOURCES += $(PLUGIN_SRC)/FnAsset/plugin/FnAsset.cpp
SOURCES += $(PLUGIN_SRC)/FnAsset/plugin/FnFileSequence.cpp
SOURCES += $(PLUGIN_SRC)/FnAsset/client/FnDefaultAssetPlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnAsset/client/FnDefaultFileSequencePlugin.cpp

# Attribute API
SOURCES += $(PLUGIN_SRC)/FnAttribute/client/FnAttribute.cpp
SOURCES += $(PLUGIN_SRC)/FnAttribute/client/FnDataBuilder.cpp

# Logging API
SOURCES += $(PLUGIN_SRC)/FnLogging/client/FnLogging.cpp

# Scenegraph Iterator API
SOURCES += $(PLUGIN_SRC)/FnScenegraphIterator/client/FnScenegraphIterator.cpp

# Plugin API
SOURCES += $(PLUGIN_SRC)/FnPluginManager/client/FnPluginManager.cpp

# Object files and flags
OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))

CXXFLAGS = -Wall -pipe -m64 -fPIC -DPIC

# Targets:
all: $(OUTFILEPATH)

$(OUTFILEPATH): $(OBJS)
	@echo "  Compiling Template Render plugin..."
	$(CXX) $(CXXFLAGS) $(OBJS)  $(LIBPATH) $(LIBS) -shared -o $(OUTFILEPATH) -Wl,-soname,$(OUTFILENAME)


$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@


clean:
	@echo "  Cleaning Template Render plugin"
	@rm -rf $(OBJDIR)
