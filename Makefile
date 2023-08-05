
SOURCES += Main.cpp Krkr2DrawDeviceWrapper.cpp DDrawAPI.cpp PassThroughDrawDevice.cpp

PROJECT_BASENAME = PassThroughDrawDevice

LDLIBS += -lwinmm -lgdi32 -ldxguid

ifeq (x,x$(findstring arm,$(TARGET_ARCH)))
CFLAGS += -DUSE_VFW
LDLIBS += -lvfw32
endif

RC_LEGALCOPYRIGHT ?= Copyright (C) 2023-2023 Julian Uy; See details of license at license.txt, or the source code location.

include external/tp_stubz/Rules.lib.make
