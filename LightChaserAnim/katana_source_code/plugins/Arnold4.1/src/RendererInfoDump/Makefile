# ********************************************************************
# This file contains copyrighted work from The Foundry,
# Sony Pictures Imageworks and Solid Angle, is intended for
# Katana and Solid Angle customers, and is not for distribution
# outside the terms of the corresponding EULA(s).
# ********************************************************************

# See README for how to build this plugin
#

# The output objects dir
OBJDIR = ./out

# The arnold executable file
OUTFILENAME=ArnoldRendererInfoDump
OUTFILEPATH = $(OBJDIR)/$(OUTFILENAME)

# Plugin sources and includes
SOURCES =  src/ArnoldRendererInfoDump.cpp


INCLUDES += -Iinclude


# Third Party libraries
INCLUDES += -I$(ARNOLDTREE_4_1)/include


LIBPATH   =  -L$(ARNOLDTREE_4_1)/bin


LIBS      =  -lai



# PLUGIN APIs sources and includes 
PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
SOURCES += $(PLUGIN_SRC)/FnAttribute/client/FnAttribute.cpp 
SOURCES += $(PLUGIN_SRC)/FnAttribute/client/FnDataBuilder.cpp
SOURCES += $(PLUGIN_SRC)/pystring/pystring.cpp

INCLUDES += -I$(KATANA_HOME)/plugin_apis/include



# Object files and flags
OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))

CXXFLAGS += -Wall -pipe -m64 -fPIC -DPIC -Werror -Wfatal-errors


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
	$(CXX) $(CXXFLAGS) $(RPATH_FLAGS) $(OBJS)  $(LIBPATH) $(LIBS) -o $(OUTFILEPATH)


$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@


clean:
	@echo "  Cleaning Arnold Info Dump executable"
	@rm -rf $(OBJDIR)
