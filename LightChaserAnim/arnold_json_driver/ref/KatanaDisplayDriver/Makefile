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
OUTFILENAME=driver_katana.so
OUTFILEPATH = $(OBJDIR)/$(OUTFILENAME)

# Plugin sources and includes
SOURCES += src/driver_katana.cpp

INCLUDES =  -Iinclude
INCLUDES += -I$(KATANA_HOME)/plugin_apis/include

# Third Party libraries
INCLUDES += -I$(ARNOLDTREE_4_2)/include
INCLUDES += -I$(ZMQ_HOME)/include

# Boost
BOOST_INCLUDE ?= $(BOOST_HOME)/include
INCLUDES += -I$(BOOST_INCLUDE)

LIBPATH =  -L$(BOOST_HOME)/lib
LIBPATH += -L$(ARNOLDTREE_4_2)/bin
LIBPATH += -L$(ZMQ_HOME)/lib

ifeq ($(KATANA_API_VERSION),2.5)
    LIBS = -lai -Wl,-Bstatic -lboost_thread -lboost_system -Wl,-Bdynamic
else
    LIBS = -lai -Wl,-Bstatic -lboost_thread -lboost_system -lzmq_static -Wl,-Bdynamic
endif

# PLUGIN APIs sources and includes
PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/CloseMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/EndOfRenderMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/DataMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/KatanaPipe.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/Message.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/NewChannelMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/NewFrameMessage.cpp
SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/KatanaPipeSingleton.cpp
ifeq ($(KATANA_API_VERSION),2.5)
    SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/client/FnDisplayDriver.cpp
else
    SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/IsAliveMessage.cpp
    SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/ContextSingleton.cpp
    SOURCES += $(PLUGIN_SRC)/FnDisplayDriver/ZmqHelpers.cpp
endif

# Object files and flags
OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))

CXXFLAGS += -Wall -pipe -m64 -fPIC -DPIC -g3 -O3 -std=c++11

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

# VERSION_SCRIPT - This allows the DSO to strictly limit which symbols are made
# visible to the host, to avoid symbol conflicts
VERSION_SCRIPT = -Wl,--version-script=../common/plugin.map


# Targets:
all: $(OUTFILEPATH)

$(OUTFILEPATH): $(OBJS)
	@echo "  Compiling Katana display driver plugin..."
	$(CXX) $(CXXFLAGS) $(RPATH_FLAGS)  $(OBJS)  $(LIBPATH) $(LIBS) -shared -o $(OUTFILEPATH) $(VERSION_SCRIPT) -Wl,-soname,$(OUTFILENAME)


$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@


clean:
	@echo "  Cleaning Katana display driver plugin"
	@rm -rf $(OBJDIR)
