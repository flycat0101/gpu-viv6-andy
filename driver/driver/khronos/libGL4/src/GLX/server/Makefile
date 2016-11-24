##############################################################################
#
#    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
#
#    The material in this file is confidential and contains trade secrets
#    of Vivante Corporation. This is proprietary information owned by
#    Vivante Corporation. No part of this work may be disclosed,
#    reproduced, copied, transmitted, or used in any way for any purpose,
#    without the express written permission of Vivante Corporation.
#
##############################################################################


# Makefile for GLX server

################################################################################

#Include common definitions.
include $(AQROOT)/makefile.linux.def


TOPDIR = ../../..
SERVER_GLX_DIR = $(TOPDIR)/src/GLX/server


SERVER_GLX_SOURCE_FILES = \
	g_disptab.c \
	g_disptab_EXT.c \
	g_render.c \
	g_renderswap.c \
	g_single.c \
	g_singleswap.c \
	glcontextmodes.c \
	global.c \
	glxbuf.c \
	glxcmds.c \
	glxcmdsswap.c \
	glxext.c \
	glxfb.c \
	glximports.c \
	glxmem.c \
	glxpix.c \
	glxscreens.c \
	glxutil.c \
	indirect_size.c \
	render2.c \
	render2swap.c \
	renderpix.c \
	renderpixswap.c \
	rensize.c \
	rensizetab.c \
	single2.c \
	single2swap.c \
	singlepix.c \
	singlepixswap.c \
	singlesize.c \
	xfont.c \
	glxmodule.c \
	xf86glx.c


# compiler define
GCC=gcc

ifeq ($(DEBUG),1)
CFLAGS=-g -DDEBUG -fPIC -march=armv7-a
BUILDDIR=obj_linux_debug
else
BUILDDIR=obj_linux_release
CFLAGS=-O3 -fPIC -march=armv7-a
endif

DEFINES=-DGLXEXT -D_LINUX_ -DPTHREADS -D_POSIX_SOURCE -D_POSIX_C_SOURCE=199309L -D_GNU_SOURCE

INCLUDES=-I ./ \
	-I$(TOPDIR)/include \
	-I$(TOPDIR)/include/gl \
	-I$(TOPDIR)/include/glcore \
	-I$(TOPDIR)/include/GLX \
	-I$(TOPDIR)/include/GLX/server \
	-I$(TOPDIR)/include/GLX/server/GL \
	-I$(TOPDIR)/include/GLX/server/mi \
	-I$(TOPDIR)/include/GLX/server/fonts \
	-I/usr/X11R6/lib/Server/include

OBJECTS=$(SERVER_GLX_SOURCE_FILES:%.c=%.o)

TARGET=libglx.so.viv
TARGETDIR=lib

LINKOBJS = $(BUILDDIR)/g_disptab.o \
	$(BUILDDIR)/g_disptab_EXT.o \
	$(BUILDDIR)/g_render.o \
	$(BUILDDIR)/g_renderswap.o \
	$(BUILDDIR)/g_single.o \
	$(BUILDDIR)/g_singleswap.o \
	$(BUILDDIR)/glcontextmodes.o \
	$(BUILDDIR)/global.o \
	$(BUILDDIR)/glxbuf.o \
	$(BUILDDIR)/glxcmds.o \
	$(BUILDDIR)/glxcmdsswap.o \
	$(BUILDDIR)/glxext.o \
	$(BUILDDIR)/glxfb.o \
	$(BUILDDIR)/glximports.o \
	$(BUILDDIR)/glxmem.o \
	$(BUILDDIR)/glxpix.o \
	$(BUILDDIR)/glxscreens.o \
	$(BUILDDIR)/glxutil.o \
	$(BUILDDIR)/indirect_size.o \
	$(BUILDDIR)/render2.o \
	$(BUILDDIR)/render2swap.o \
	$(BUILDDIR)/renderpix.o \
	$(BUILDDIR)/renderpixswap.o \
	$(BUILDDIR)/rensize.o \
	$(BUILDDIR)/rensizetab.o \
	$(BUILDDIR)/single2.o \
	$(BUILDDIR)/single2swap.o \
	$(BUILDDIR)/singlepix.o \
	$(BUILDDIR)/singlepixswap.o \
	$(BUILDDIR)/singlesize.o \
	$(BUILDDIR)/xfont.o \
	$(BUILDDIR)/glxmodule.o \
	$(BUILDDIR)/xf86glx.o


# rules
$(BUILDDIR)/%.o: %.c
	$(GCC) -c  $(INCLUDES) $(CFLAGS) $(DEFINES) -o $@ $<

default: .depend $(TOPDIR)/$(TARGETDIR)/$(TARGET)

$(TOPDIR)/$(TARGETDIR)/$(TARGET): MKDIR $(LINKOBJS)
	$(GCC) $(LINKOBJS) -shared -Wl,--version-script=GLSymExport -o $@ -ldl -lpthread


.PHONY: .depend
.depend:
	rm -rf .depend
	touch .depend
	makedepend $(SERVER_GLX_SOURCE_FILES) $(INCLUDES) $(CFLAGS) $(DEFINES)  -f .depend -p$(BUILDDIR)/ > /dev/null 2>&1

MKDIR:
	mkdir -p $(BUILDDIR)
	mkdir -p $(TOPDIR)/$(TARGETDIR)

clean:
	rm -f $(BUILDDIR)/*.o .depend $(TOPDIR)/$(TARGETDIR)/$(TARGET)

include .depend
