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


lib_LTLIBRARIES = libvulkan.la

libvulkan_la_SOURCES = \
    src/chip/gc_halti2_chip.c \
    src/chip/gc_halti3_chip.c \
    src/chip/gc_halti5_chip.c \
    src/chip/gc_halti5_cmdbuf.c \
    src/chip/gc_halti5_computeblit.c \
    src/chip/gc_halti5_pipeline.c \
    src/chip/gc_halti5_resource.c \
    src/chip/gc_halti5_tweak.c \
    src/gc_vk_api.c \
    src/gc_vk_chip.c \
    src/gc_vk_cmdbuf.c \
    src/gc_vk_context.c \
    src/gc_vk_descriptor.c \
    src/gc_vk_devqueue.c \
    src/gc_vk_dispatch.c \
    src/gc_vk_framebuffer.c \
    src/gc_vk_instance.c \
    src/gc_vk_noop.c \
    src/gc_vk_object.c \
    src/gc_vk_phydevice.c \
    src/gc_vk_pipeline.c \
    src/gc_vk_query.c \
    src/gc_vk_resource.c \
    src/gc_vk_shader.c \
    src/gc_vk_sync.c \
    src/gc_vk_utils.c \
    src/gc_vk_validation.c \
    src/wsi/gc_wsi_surface.c \
    src/wsi/gc_wsi_display.c \
    src/wsi/gc_wsi_swapchain.c

libvulkan_la_SOURCES += \
    src/wsi/gc_wsi_display_fbdev.c

#libvulkan_la_SOURCES += \
#    src/wsi/gc_wsi_surface_mir.c \
#    src/wsi/gc_wsi_surface_xcb.c \
#    src/wsi/gc_wsi_surface_xlib.c

vulkan_generated = \
    src/chip/gc_halti5_patchlib.frag.h \
    src/chip/gc_halti5_blit.comp.h

src/chip/gc_halti5_chip.c: src/chip/gc_halti5_patchlib.frag.h
src/chip/gc_halti5_computeblit.c: src/chip/gc_halti5_blit.comp.h

src/chip/gc_halti5_patchlib.frag.h: src/chip/gc_halti5_patchlib.frag
src/chip/gc_halti5_blit.comp.h: src/chip/gc_halti5_blit.comp

$(vulkan_generated):
	@python $(top_srcdir)/tools/bin/ConvertFragToSpvToC.py -t $(top_srcdir)/tools/bin/glslangValidator/Linux/glslangValidator -c $(srcdir)/src/chip -f $<

AM_CPPFLAGS = \
    $(DEFINES) \
    -I$(top_srcdir)/sdk/inc \
    -I$(top_srcdir)/hal/inc \
    -I$(top_srcdir)/hal/os/linux/user \
    -I$(top_srcdir)/compiler/libVSC/include \
    -I$(top_srcdir)/compiler/libSPIRV/spvconverter \
    -I$(top_srcdir)/driver/khronos/libEGL/inc \
    -I$(srcdir)/src \
    -I$(srcdir)/include

AM_CPPFLAGS += \
    -I$(top_srcdir)/hal/user \
    -I$(top_srcdir)/hal/user/arch \
    -I$(top_srcdir)/arch/XAQ2/cmodel/inc

AM_CFLAGS = \
    $(GCC_CFLAGS) \
    -Wno-unused-function

libvulkan_la_LIBADD = \
    -ldl -lm \
    $(top_builddir)/hal/user/libGAL.la \
    $(top_builddir)/compiler/libVSC/libVSC.la \
    $(top_builddir)/compiler/libGLSLC/libGLSLC.la \
    $(top_builddir)/compiler/libSPIRV/libSPIRV.la

if EGL_PLATFORM_X11
AM_CPPFLAGS +=

AM_CFLAGS += $(X_CFLAGS)
endif

if EGL_PLATFORM_DRI
AM_CFLAGS += $(X_DRI_CFLAGS)
endif

if EGL_PLATFORM_GBM
AM_CPPFLAGS += -D__GBM__

AM_CFLAGS += $(GBM_CFLAGS)
endif

if EGL_PLATFORM_WAYLAND
libvulkan_la_SOURCES += \
    src/wsi/gc_wsi_surface_wayland.c \
    ../../wayland/wayland-viv-protocol.c

AM_CPPFLAGS += \
    -I$(top_srcdir)/driver/wayland

AM_CFLAGS += \
    $(WAYLAND_CFLAGS) \
    -DVK_USE_PLATFORM_WAYLAND_KHR=1

libvulkan_la_LIBADD += \
     $(WAYLAND_LIBS)
endif

libvulkan_la_LDFLAGS = \
    -pthread \
    -version-number 1:0 \
    -Wl,-z,defs \
    -Wl,--version-script,$(srcdir)/libVulkan.map

vulkanincdir = $(includedir)/vulkan

vulkaninc_HEADERS = \
    $(top_srcdir)/sdk/inc/vulkan/vk_icd.h \
    $(top_srcdir)/sdk/inc/vulkan/vk_platform.h \
    $(top_srcdir)/sdk/inc/vulkan/vulkan.h

