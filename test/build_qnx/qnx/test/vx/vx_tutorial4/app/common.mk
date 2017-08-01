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

qnx_build_dir:=$(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../../../..)

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION="vx tutorial 4"
endef

NAME=vx_tutorial4

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/sdk/inc

SOURCE_OBJECTS += $(test_root)/test/ovx/vx_tutorial4/app/main.o

EXTRA_SRCVPATH += $(test_root)/test/ovx/vx_tutorial4/app

EXTRA_LIBVPATH += $(INSTALL_ROOT_$(OS))/$(CPU)$(filter le be, $(VARIANT_LIST))$(CPUVARDIR_SUFFIX)/$(INSTALLDIR)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

#LIBS += GAL VDK EGL OpenVG

LDOPTS += -lGAL -lLLVM_viv -lOpenVX -lOpenVXU -lCLC -lVSC

include $(qnx_build_dir)/math.mk

INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) samples/ovx/vx_tutorial4/)

target_dir=$(INSTALL_ROOT_$(OS))/$(CPU)$(filter le be, $(VARIANT_LIST))$(CPUVARDIR_SUFFIX)/$(INSTALLDIR)

POST_BUILD+= && $(CP_HOST) $(test_root)/test/ovx/vx_tutorial4/app/res/bmp_list.txt $(target_dir)/ -f
POST_BUILD+= && $(CP_HOST) $(test_root)/test/ovx/vx_tutorial4/app/res/Scene_500Lux__2xInt_NoSh_1920x1080_1frame_Yonly.bayer $(target_dir)/ -f
POST_BUILD+= && $(CP_HOST) $(driver_root)/sdk/inc/CL/cl_viv_vx_ext.h $(target_dir)/ -f

include $(MKFILES_ROOT)/qtargets.mk
