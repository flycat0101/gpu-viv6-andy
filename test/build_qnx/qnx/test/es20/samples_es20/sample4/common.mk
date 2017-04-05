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
PINFO DESCRIPTION="vdksample4_es20"
endef

NAME=vdksample4_es20

MY_SRC := $(VDK_TEST_DIR)/es20/samples

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/sdk/inc
EXTRA_INCVPATH += $(MY_SRC)/Common

SOURCE_OBJECTS += $(wildcard $(MY_SRC)/sample4/*.c)
EXTRA_SRCVPATH += $(MY_SRC)/sample4

EXTRA_LIBVPATH += $(INSTALL_ROOT_$(OS))/$(CPU)$(filter le be, $(VARIANT_LIST))$(CPUVARDIR_SUFFIX)/$(firstword $(INSTALLDIR_$(OS)) samples/vdk/samples_es20)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

STATIC_LIBS += vdk_sample_commonS
$(foreach lib, $(STATIC_LIBS), $(eval LIBPREF_$(lib) = -Bstatic))
$(foreach lib, $(STATIC_LIBS), $(eval LIBPOST_$(lib) = -Bdynamic))

LIBS += $(STATIC_LIBS)
LDOPTS += -lGAL -lVSC -lVDK -lEGL_viv -lGLESv2_viv

include $(qnx_build_dir)/math.mk

INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) samples/vdk/samples_es20)

include $(MKFILES_ROOT)/qtargets.mk
