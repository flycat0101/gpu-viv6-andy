##############################################################################
#
#    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
#
#    The material in this file is confidential and contains trade secrets
#    of Vivante Corporation. This is proprietary information owned by
#    Vivante Corporation. No part of this work may be disclosed,
#    reproduced, copied, transmitted, or used in any way for any purpose,
#    without the express written permission of Vivante Corporation.
#
##############################################################################


lib_LTLIBRARIES = libgc_wayland_protocol.la libwayland-viv.la

AM_CFLAGS =			\
	-DLINUX=1		\
	-DEGL_API_FB=1	\
	-DEGL_API_WL=1	\
	$(GCC_CFLAGS)

libgc_wayland_protocol_la_CPPFLAGS = \
   $(WAYLAND_CLIENT_CFLAGS)

libwayland_viv_la_CPPFLAGS = \
   $(WAYLAND_SERVER_CFLAGS)

libwayland_viv_la_LIBADD = $(WAYLAND_SERVER_LIBS)

libgc_wayland_protocol_la_LDFLAGS = -version-info 0:0:0
libgc_wayland_protocol_la_LIBADD = $(WAYLAND_CLIENT_LIBS) -lm -ldl -lwayland-client

libgc_wayland_protocol_la_SOURCES = \
    wayland-viv-protocol.c \
    wayland-viv-client-protocol.h \
    gc_wayland_protocol.h \
    gc_wayland_protocol.c

libwayland_viv_la_SOURCES =  \
   wayland-viv-protocol.c \
   wayland-viv-server-protocol.h

viv_includedir = $(includedir)/wayland-viv

viv_include_HEADERS = \
   wayland-viv-server-protocol.h       \
   wayland-viv-client-protocol.h       \
   gc_wayland_protocol.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gc_wayland_protocol.pc wayland-viv.pc wayland-egl.pc

BUILT_SOURCES = \
   wayland-viv-protocol.c \
   wayland-viv-server-protocol.h \
   wayland-viv-client-protocol.h

CLEANFILES = $(BUILT_SOURCES)

@wayland_scanner_rules@

