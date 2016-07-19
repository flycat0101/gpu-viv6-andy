#/*
# *  Copyright (C) 2013-2014 Freescale Semiconductor, Inc.
# *  All Rights Reserved.
# *
# *  The following programs are the sole property of Freescale Semiconductor Inc.,
# *  and contain its proprietary and confidential information.
# *
# */
#
# Linux build file for g2d library
#
.PHONY: clean

AQROOT ?= ../../../driver/
ifeq ($(ARCH), arm64-yocto)
ARCH_TYPE := arm64
CPU_TYPE := cortex-a53
CPU_ARCH := armv8-a
G2D_VG := 0
else
ARCH_TYPE := arm
CPU_TYPE := cortex-a9
CPU_ARCH := armv7-a
G2D_VG := 1
endif

include $(AQROOT)/makefile.linux.def

OBJS := g2d.o

#API version
API := 1.1

#Library name used in application
LIBRARY := libg2d.so

#Make g2d wrapper with soname later
SONAME := $(LIBRARY).$(API)

#Change target as symlink, make target with new rule in wrapper later
#libg2d-viv.so -> libg2d-viv.so.x.y
TARGET := libg2d-viv.so

CFLAGS += -I ../include -fPIC -I $(AQROOT)/sdk/inc -I $(AQROOT)/hal/inc -I $(ROOTFS_USR)/include -DgcdENABLE_VG=1 -Dg2dENABLE_VG=$(G2D_VG)
CFLAGS += -DLINUX

ifeq ($(SDK_BUILD), 1)
LFLAGS += -L $(SDK_DIR)/drivers -L $(ROOTFS_USR)/lib -lGAL
else
LFLAGS += -L $(ROOTFS_USR)/lib -lGAL
endif

CC = $(CROSS_COMPILE)gcc

COMMITNR=$(shell git log -n 1 --format=%h)
DIRTY=$(shell (git diff --quiet HEAD || echo '-dirty'))
CFLAGS += -DGIT_STRING=$(COMMITNR)$(DIRTY)

# build as shared library
$(TARGET): $(OBJS)
	$(CC) -shared -fPIC -Wl,-soname,$(SONAME) -o $@ $(OBJS) $(LFLAGS)
	-@ln -s $@ $(SONAME)
	-@ln -s $(SONAME) $(LIBRARY)
ifeq ($(SDK_BUILD), 1)
	-@cp -af $@ $(SDK_DIR)/drivers/
	-@cp -af $(LIBRARY)* $(SDK_DIR)/drivers/
endif
	-@mkdir -p ../lib/
	-@cp -af $@ ../lib/
	-@cp -af $(LIBRARY)* ../lib/

clean:
	-@rm $(TARGET) $(LIBRARY)* $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
