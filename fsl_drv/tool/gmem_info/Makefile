#/*
# *  Copyright (C) 2013 Freescale Semiconductor, Inc.
# *  All Rights Reserved.
# *
# *  The following programs are the sole property of Freescale Semiconductor Inc.,
# *  and contain its proprietary and confidential information.
# *
# */
#
# Linux build file for gmem info code
#
#
.PHONY: clean

AQROOT ?= ../../../driver/

include $(AQROOT)/makefile.linux.def

OBJS := gmem_info.o

CFLAGS += -I ../include -fPIC -I $(AQROOT)/sdk/inc -I $(AQROOT)/hal/inc -I$(ROOTFS_USR)/include
ifeq ($(SDK_BUILD), 1)
LFLAGS = -L $(SDK_DIR)/drivers -L $(ROOTFS_USR)/lib -lGAL
else
LFLAGS = -L $(ROOTFS_USR)/lib -lGAL
endif

CC = $(CROSS_COMPILE)gcc

# build as executable binary
TARGET := gmem_info

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LFLAGS)

clean:
	-@rm $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
