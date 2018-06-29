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

# The prman display driver
OUTFILENAME=d_SocketMultiresDisplayDriver.so
OUTFILEPATH = $(OBJDIR)/$(OUTFILENAME)

# Plugin sources and includes
PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
SOURCES = SocketMultiresDisplayDriver.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/CloseMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/EndOfRenderMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/IsAliveMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/ZmqHelpers.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/ContextSingleton.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/DataMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/KatanaPipe.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/Message.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/NewChannelMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/NewFrameMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/KatanaPipeSingleton.cpp

INCLUDES = -I$(KATANA_HOME)/plugin_apis/include

# Third Party libraries
BOOST_INCLUDE ?= $(BOOST_HOME)/include
INCLUDES += -I$(RMANTREE)/include -I$(BOOST_INCLUDE) -I$(ZMQ_HOME)/include

LIBPATH   =  -L$(RMANTREE)/lib -L$(ZMQ_HOME)/lib -L$(BOOST_HOME)/lib
LIBS      =  -lprman -lzmq_static -lboost_thread

# Object files and flags
OBJS := $(patsubst %.cpp, $(OBJDIR)/%.o, $(SOURCES) )

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
	@echo "  Compiling PRMan display driver..."
	$(CXX) $(CXXFLAGS) $(RPATH_FLAGS) $(OBJS)  $(LIBPATH) $(LIBS) -shared -o $(OUTFILEPATH) -Wl,-soname,$(OUTFILENAME)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@


clean:
	@echo "  Cleaning PRMan display driver"
	@rm -rf $(OBJDIR)
