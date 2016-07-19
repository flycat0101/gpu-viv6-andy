#
# QNX makefile for gmem_info tool
#
# Please define QNX_GRAPHICS_ROOT environment variable that points to platform specific GPU libraries (GAL needed).
# This is equvivalent to GRAPHICS_ROOT in QNX.
#
# Example:
#   export QNX_GRAPHICS_ROOT=$QNX_TARGET/armle-v7/usr/lib/graphics/iMX6X
#

build_dir:=$(abspath $(dir $(lastword $(MAKEFILE_LIST))))
src_dir := $(abspath $(build_dir)/..)
install_dir := $(build_dir)/binaries
driver_dir := $(abspath $(build_dir)/../../../../driver)

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=gmem_info

USEFILE=$(build_dir)/$(NAME).use

ifeq ($(QNX_GRAPHICS_ROOT),)
  $(error ERROR: Please define QNX_GRAPHICS_ROOT environment variable that points to platform specific GPU libraries.)
endif

EXTRA_SRCVPATH += $(src_dir)
EXTRA_INCVPATH += $(driver_dir)/hal/inc
EXTRA_LIBVPATH += $(QNX_GRAPHICS_ROOT)

LIBS+=GAL

# Perform local install
POST_BUILD=$(CP_HOST) $(NAME) $(install_dir)/$(NAME)

# Perform clean of local install directory
POST_CLEAN=$(RM_HOST) $(install_dir)/*$(NAME)*

# QNX includes
include $(MKFILES_ROOT)/qmacros.mk
ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/.qnx_internal.mk
endif
include $(QNX_INTERNAL)
include $(MKFILES_ROOT)/qtargets.mk
