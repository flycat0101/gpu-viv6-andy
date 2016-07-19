##############################################################################
#
# Copyright 2010, QNX Software Systems Ltd.  All Rights Reserved
#
# This source code has been published by QNX Software Systems Ltd.
# (QSSL).  However, any use, reproduction, modification, distribution
# or transfer of this software, or any software which includes or is
# based upon any of this code, is only permitted under the terms of
# the QNX Open Community License version 1.0 (see licensing.qnx.com for
# details) or as otherwise expressly authorized by a written license
# agreement from QSSL.  For more information, please email licensing@qnx.com.
#
##############################################################################

#
# Common inlude file for QNX build.
#

CUSTOM_PIXMAP?=0
USE_NEW_LINUX_SIGNAL?=1
USE_FB_DOUBLE_BUFFER?=0
BUILD_OPENCL_FP?=1
ENABLE_CL_GL ?=0

# This prevents the platform/board name from getting appended to every build target name.
# This happens automatically as the build directory structure now includes the board above the
# common.mk for each component.
EXTRA_SILENT_VARIANTS+=$(PLATFORM)

INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/lib/graphics/$(PLATFORM))

# Copy all build targets to a local folder for incremental building
LOCAL_INSTALL=$(qnx_build_dir)/platform_binaries/$(CPU)$(filter le be, $(VARIANT_LIST))$(CPUVARDIR_SUFFIX)/$(PLATFORM)

# Remove *S.a files that get built along with any so build.
# POST_INSTALL=$(RM_HOST) $(INSTALL_ROOT_$(OS))/$(CPU)$(filter le be, $(VARIANT_LIST))$(CPUVARDIR_SUFFIX)/$(INSTALLDIR)/*$(NAME)*.a

# Perform local install
POST_BUILD=$(CP_HOST) $@ $(LOCAL_INSTALL)/$(@F)

# Perform clean of local install directory
POST_CLEAN=$(RM_HOST) $(LOCAL_INSTALL)/*$(NAME)*

include $(qnx_build_dir)/platform_config/$(PLATFORM)/platform_config

###############################################################
# Common CCFLAGS.
ifneq ($(ABI), 0)
	CCFLAGS += -mabi=$(ABI)
endif

CCFLAGS += -Werror
CCFLAGS += -ansi
#CCFLAGS += -pedantic
#CCFLAGS += -fmudflap
#LDFLAGS += -lmudflap
CCFLAGS += -Wno-error=unused-but-set-variable -Wno-strict-aliasing

ifeq ($(EGL_API_FB), 1)
	CCFLAGS += -DEGL_API_FB
endif

ifeq ($(STATIC_LINK), 1)
	CCFLAGS += -DSTATIC_LINK
endif

ifeq ($(PLATFORM), iMX8DV)
        CCFLAGS += -DIMX8X
endif

ifeq ($(USE_VDK), 1)
	CCFLAGS += -DUSE_VDK=1 -DUSE_SW_FB=$(USE_SW_FB)
else
	CCFLAGS += -DUSE_VDK=0
endif

ifeq ($(USE_FB_DOUBLE_BUFFER), 1)
	CCFLAGS += -DUSE_FB_DOUBLE_BUFFER=1
else
	CCFLAGS += -DUSE_FB_DOUBLE_BUFFER=0
endif

ifeq ($(USE_NEW_LINUX_SIGNAL), 1)
	CCFLAGS += -DUSE_NEW_LINUX_SIGNAL=1
else
	CCFLAGS += -DUSE_NEW_LINUX_SIGNAL=0
endif

ifeq ($(BUILD_OPENCL_FP), 1)
	CCFLAGS += -DBUILD_OPENCL_FP=1
else
	CCFLAGS += -DBUILD_OPENCL_FP=0
endif

ifeq ($(ENABLE_CL_GL), 1)
	CCFLAGS += -DgcdENABLE_CL_GL=1
else
	CCFLAGS += -DgcdENABLE_CL_GL=0
endif

ifeq ($(VIVANTE_ENABLE_VG), 1)
	CCFLAGS += -DgcdENABLE_VG=1
else
	CCFLAGS += -DgcdENABLE_VG=0
endif

ifeq ($(VIVANTE_ENABLE_3D), 1)
	CCFLAGS += -DgcdENABLE_3D=1
else
	CCFLAGS += -DgcdENABLE_3D=0
endif

ifeq ($(VIVANTE_ENABLE_2D), 1)
	CCFLAGS += -DgcdENABLE_2D=1
else
	CCFLAGS += -DgcdENABLE_2D=0
endif

ifeq ($(USE_FAST_MEM_COPY), 1)
	CCFLAGS += -DgcdUSE_FAST_MEM_COPY=1
else
	CCFLAGS += -DgcdUSE_FAST_MEM_COPY=0
endif

CCFLAGS += -DgcdDISPLAY_BACK_BUFFERS=3

# GN TODO Check to see if this is required
#CXXFLAGS += -fno-short-enums

################################################################################
# Build with profiler
CCFLAGS += -DVIVANTE_PROFILER=1
CCFLAGS += -DVIVANTE_PROFILER_CONTEXT=1

################################################################################
# Build with HAL tracer
ifeq ($(USE_HAL_TRACER),1)
	CCFLAGS += -DgcdDUMP=1
else
	CCFLAGS += -DgcdDUMP=0
endif

################################################################################
# Build with HAL tracer
ifeq ($(USE_CMDBUF_TRACER),1)
	CCFLAGS += -DgcdDUMP_COMMAND=1
else
	CCFLAGS += -DgcdDUMP_COMMAND=0
endif

################################################################################
# Build with GL tracer
ifeq ($(USE_GL_TRACER),1)
	CCFLAGS += -DgcdDEBUG=1 -DgcdDUMP_API=1
else
	CCFLAGS += -DgcdDUMP_API=0
endif

################################################################################
# Build with VG tracer
ifeq ($(USE_VG_TRACER),1)
	CCFLAGS += -DOVG_DBGOUT=1
endif

################################################################################
# Build for debugging
ifeq ($(ENABLE_DEBUG),1)
	CCFLAGS += -g -O0 -DgcdDEBUG=1
#else
#	CCFLAGS += -O2
#	CCFLAGS += -Wno-strict-aliasing
endif

################################################################################
# Build for profiling
ifeq ($(ENABLE_PROFILING),1)
	CCFLAGS += -p
	CXXFLAGS += -p
	LDFLAGS += -p
	CCFLAGS += -g -finstrument-functions
	LIBS += profilingS
	LDFLAGS += -Wl,-E
endif

################################################################################
# Report unresolved symbols at build time
ENABLE_BUILD_TIME_SYMBOL_CHECK ?= 1
ifeq ($(ENABLE_BUILD_TIME_SYMBOL_CHECK),1)
   SCREEN_LIBS = screen
   LDFLAGS += -Wl,--unresolved-symbols=report-all
else
   SCREEN_LIBS =
endif

################################################################################
# Build for bank alignment setting
ifeq ($(USE_BANK_ALIGNMENT),1)
    CCFLAGS += -DgcdENABLE_BANK_ALIGNMENT=1
    ifneq ($(BANK_BIT_START), 0)
	        ifneq ($(BANK_BIT_END), 0)
	            CCFLAGS += -DgcdBANK_BIT_START=$(BANK_BIT_START)
	            CCFLAGS += -DgcdBANK_BIT_END=$(BANK_BIT_END)
	        endif
    endif

    ifneq ($(BANK_CHANNEL_BIT), 0)
        CCFLAGS += -DgcdBANK_CHANNEL_BIT=$(BANK_CHANNEL_BIT)
    endif
endif
