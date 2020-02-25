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

# find the driver's root directory, which is 3 levels below the current make file
driver_root:=$(abspath ../../$(dir $(lastword $(MAKEFILE_LIST))))
qnx_build_dir:=$(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION="NN arch model"
endef


include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/sdk/inc
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libOpenVX/driver/include
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libOpenVX/libarchmodelInterface/include
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libOpenVX/kernels
EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/hal/os/qnx/user
EXTRA_INCVPATH += $(driver_root)/compiler/libVSC/include
EXTRA_INCVPATH += $(driver_root)/arch/vipArchPerfMdl_dev/vipArchPerf
EXTRA_INCVPATH += $(driver_root)/arch/vipArchPerfMdl_dev/libarchmodelSw/include

SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/libarchmodel/archModelInterface.o
EXTRA_SRCVPATH += $(driver_root)/driver/khronos/libOpenVX/libarchmodelInterface

EXTRA_LIBVPATH += $(LOCAL_INSTALL)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

include $(MKFILES_ROOT)/qmacros.mk

include $(qnx_build_dir)/math.mk

ifneq ($(filter so, $(VARIANT_LIST)), so)
INSTALLDIR=/dev/null
endif

include $(MKFILES_ROOT)/qtargets.mk
