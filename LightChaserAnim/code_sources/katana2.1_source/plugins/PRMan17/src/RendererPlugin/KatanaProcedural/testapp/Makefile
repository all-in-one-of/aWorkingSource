# ********************************************************************
# This file contains copyrighted work from The Foundry,
# Sony Pictures Imageworks and Pixar, is intended for
# Katana and PRMan customers, and is not for distribution
# outside the terms of the corresponding EULA(s).
# ********************************************************************
# 
# See README for instructions on how to use this Makefile
#

PROJECT = testapp
PROJECT_TYPE = bin
OPT_LEVEL = 3
OUT = out

CXXFLAGS=-DSPI_ANSI_STREAM -DSPI_PYDSO="\\\"$(PYDSO)\\\""
CFLAGS_OPT = -Wall

IPATH  = -I$(SRCDIR)/../include 
IPATH += -I${RMANDIR}/include
IPATH += -I${PYINC}
IPATH += -I$(SRCDIR)/../../../../GEOLIB/export
IPATH += -I$(SRCDIR)/../../../../UTIL/export

SPCOMP2_DEPEND = ScenegraphAttr?v=4 pystring?v=1 VnpWrappers?v=2

LPATH  = -L$(SRCDIR)/../../../../DSOS
LPATH += -L${RMANDIR}/lib
LPATH += -Xlinker -export-dynamic $(SRCDIR)/../../../../DSOS/GeoAPI_cmodule.so
LPATH +=  -lrt -ljpeg -ltiff -ljpeg  -Wl,--whole-archive  -Wl,--no-whole-archive -lz -lprman

LDFLAGS += -Wl,-rpath,$(RMANDIR)/lib

SYSLIBS = ${PYDSO} -lssl

CXXFILES = $(SRCDIR)/../src/readScript.cpp $(SRCDIR)/../src/PRManProcedural.cpp \
	$(SRCDIR)/../src/WriteRI.cpp $(SRCDIR)/src/testapp.cpp

# Source the katana then facility makefile standards
KATMAKE = ../../../../katana.mk
ifdef SRCDIR
    KATMAKE := $(SRCDIR)/$(KATMAKE)
endif
include $(KATMAKE)
include $(MAKEFILE_SOURCE)/Makefile.unique

test: debug
	@echo env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(SRCDIR)/../../../../DSOS KATANA_PROCEDURAL_PATH=$(SRCDIR)/../../../../PROCEDURALS RMAN_SHADERPATH=$(SRCDIR)/../../../../TEST_IMAGES/sl:$(RMAN_SHADERPATH):/net/apps/linux/pixar/prman-12.0/lib/shaders $(BUILDDIR)_D/testapp_d
	@echo ------------------------------------------------------------------------
	@env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(SRCDIR)/../../../../DSOS KATANA_PROCEDURAL_PATH=$(SRCDIR)/../../../../PROCEDURALS RMAN_SHADERPATH=$(SRCDIR)/../../../../TEST_IMAGES/sl:$(RMAN_SHADERPATH):/net/apps/linux/pixar/prman-12.0/lib/shaders $(BUILDDIR)_D/testapp_d
	@echo ------------------------------------------------------------------------
