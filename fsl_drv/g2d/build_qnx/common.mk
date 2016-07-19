#
# Common inlude file for QNX build.
#
VIVANTE_SDK_DIR:=$(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../../../driver)

include $(MKFILES_ROOT)/qmacros.mk
#
# Target platform name. Change to your board according $QNX_TARGET/<arch name>/usr/lib/graphics/<platform>
#
ifeq ($(QNX_TARGET_PLATFORM),)
  QNX_TARGET_PLATFORM=iMX8DV
endif

#
# Target architecture name. Change to architecture according to $QNX_TARGET/<arch name>.
#
ifeq ($(QNX_TARGET_ARCH),)
  ifeq ($(CPU),aarch64)
    QNX_TARGET_ARCH=aarch64le
  else
    QNX_TARGET_ARCH=armle-v7
  endif
endif

#
# QNX_GRAPHICS_ROOT variable points to platform-dependent GPU libraries. If not defined,
# default path is constructed from QNX_TARGET_ARCH and QNX_TARGET_PLATFORM variables.
# No change needed in most cases. 
#
ifeq ($(QNX_GRAPHICS_ROOT),)
  QNX_GRAPHICS_ROOT=$(QNX_TARGET)/$(QNX_TARGET_ARCH)/usr/lib/graphics/$(QNX_TARGET_PLATFORM)
  $(warning Environment variable QNX_GRAPHICS_ROOT is not set. Used default path $(QNX_GRAPHICS_ROOT))
endif

EXTRA_LIBVPATH += $(QNX_GRAPHICS_ROOT)
VIVANTE_SDK_INC=$(VIVANTE_SDK_DIR)/sdk/inc

install_dir=$(qnx_build_dir)/platform_binaries/$(QNX_TARGET_PLATFORM)/$(QNX_TARGET_ARCH)
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/lib/graphics/$(QNX_TARGET_PLATFORM))

USEFILE=

# This prevents the platform/board name from getting appended to every build target name.
# This happens automatically as the build directory structure now includes the board above the
# common.mk for each component.
EXTRA_SILENT_VARIANTS+=$(QNX_TARGET_PLATFORM)

# Perform clean of local install directory
POST_CLEAN=$(RM_HOST) $(install_dir)/*$(NAME)*