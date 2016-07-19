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
PINFO DESCRIPTION="Vivante HAL arch user"
endef

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/hal/os/qnx/user
EXTRA_INCVPATH += $(driver_root)/arch/XAQ2/cmodel/inc
ifeq ($(VIVANTE_ENABLE_3D), 1)
EXTRA_INCVPATH += $(driver_root)/compiler/libVSC/include
endif
ifeq ($(VIVANTE_ENABLE_2D), 1)
EXTRA_INCVPATH += $(driver_root)/hal/user/arch/thirdparty_special
endif
ifeq ($(VIVANTE_ENABLE_VG), 1)
EXTRA_INCVPATH += $(driver_root)/hal/user/archvg
endif

# from libhalarchuser (trunk/hal/user/arch/Makefile.linux)
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_blt.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_filter_blt_vr.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_filter_blt_blocksize.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_pattern.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_pipe.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_primitive.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_query.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_source.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_target.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_dec.o

ifeq ($(VIVANTE_ENABLE_3D), 1)
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_composition.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_clear.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_filter_blt_de.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_engine.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_frag_proc.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_texture.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_texture_upload.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_stream.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/gc_hal_user_hardware_shader.o
endif

ifeq ($(VIVANTE_ENABLE_2D), 1)
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/thirdparty_special/gc_hal_user_hardware_thirdparty.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/thirdparty_special/gc_hal_user_hardware_thirdparty_v10.o
SOURCE_OBJECTS += $(driver_root)/hal/user/arch/thirdparty_special/gc_hal_user_hardware_thirdparty_v11.o
endif

EXTRA_SRCVPATH += $(driver_root)/hal/user/arch
ifeq ($(VIVANTE_ENABLE_2D), 1)
EXTRA_SRCVPATH += $(driver_root)/hal/user/arch/thirdparty_special
endif

EXTRA_LIBVPATH += $(LOCAL_INSTALL)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

ifneq ($(filter v7, $(VARIANT_LIST)), v7)
	CCFLAGS += -mfpu=vfp -mfloat-abi=softfp
	LIBS += m-vfp
else
	LIBS += m
endif

ifneq ($(filter so, $(VARIANT_LIST)), so)
INSTALLDIR=/dev/null
endif

include $(MKFILES_ROOT)/qtargets.mk
