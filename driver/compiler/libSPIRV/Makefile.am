##############################################################################
#
#    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
#
#    The material in this file is confidential and contains trade secrets
#    of Vivante Corporation. This is proprietary information owned by
#    Vivante Corporation. No part of this work may be disclosed,
#    reproduced, copied, transmitted, or used in any way for any purpose,
#    without the express written permission of Vivante Corporation.
#
##############################################################################


lib_LTLIBRARIES = libSPIRV_viv.la

libSPIRV_viv_la_SOURCES = \
    spvconverter/gc_spirv_mempool.c \
    spvconverter/gc_spirv_to_vir.c \
    spvconverter/gc_spriv_disassmble.c

AM_CPPFLAGS = \
    $(DEFINES) \
    -I$(top_srcdir)/sdk/inc \
    -I$(top_srcdir)/hal/inc \
    -I$(top_srcdir)/hal/user \
    -I$(top_srcdir)/hal/os/linux/user \
    -I$(top_srcdir)/arch/XAQ2/cmodel/inc \
    -I$(top_srcdir)/compiler/libVSC/include \
    -I$(srcdir)/spvconverter

AM_CFLAGS = \
    $(GCC_CFLAGS)

libSPIRV_viv_la_LIBADD = \
    -ldl -lm \
    $(top_builddir)/hal/user/libGAL.la \
    $(top_builddir)/compiler/libVSC/libVSC.la

libSPIRV_viv_la_LDFLAGS = \
    -version-number 1:0 \
    -Wl,-z,defs \
    -Wl,--version-script,$(srcdir)/spvconverter/spvconverter.map

# headers
spirvincdir = $(includedir)/SPIRV

spirvinc_HEADERS = \
    $(top_srcdir)/sdk/inc/SPIRV/spirv.h

