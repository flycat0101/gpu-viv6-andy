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

qnx_build_dir:=$(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../../..)

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION="vv_launcher"
endef

NAME=vv_launcher

include $(qnx_build_dir)/common.mk

MY_SRC := $(ES20_TEST_DIR)/vv_launcher

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/sdk/inc
EXTRA_INCVPATH += $(MY_SRC)/lib

SOURCE_OBJECTS += $(MY_SRC)/src/Chiclet.o
SOURCE_OBJECTS += $(MY_SRC)/src/ChicletProg.o
SOURCE_OBJECTS += $(MY_SRC)/src/ContentProg.o
SOURCE_OBJECTS += $(MY_SRC)/src/EnvProg.o
SOURCE_OBJECTS += $(MY_SRC)/src/Geodesic.o
SOURCE_OBJECTS += $(MY_SRC)/src/LauncherApp.o
SOURCE_OBJECTS += $(MY_SRC)/src/resources.o
SOURCE_OBJECTS += $(MY_SRC)/src/vv_main.o
EXTRA_SRCVPATH += $(MY_SRC)/src

SOURCE_OBJECTS += $(MY_SRC)/lib/glutils/check.o
SOURCE_OBJECTS += $(MY_SRC)/lib/glutils/dds.o
SOURCE_OBJECTS += $(MY_SRC)/lib/glutils/log.o
SOURCE_OBJECTS += $(MY_SRC)/lib/glutils/program.o
SOURCE_OBJECTS += $(MY_SRC)/lib/glutils/Texture2D.o
SOURCE_OBJECTS += $(MY_SRC)/lib/glutils/Texture2DRenderSurface.o
SOURCE_OBJECTS += $(MY_SRC)/lib/glutils/TextureCube.o
EXTRA_SRCVPATH += $(MY_SRC)/lib/glutils

SOURCE_OBJECTS += $(MY_SRC)/lib/math/icosa.o
SOURCE_OBJECTS += $(MY_SRC)/lib/math/Mat.o
SOURCE_OBJECTS += $(MY_SRC)/lib/math/misc.o
SOURCE_OBJECTS += $(MY_SRC)/lib/math/Vec.o
EXTRA_SRCVPATH += $(MY_SRC)/lib/math

SOURCE_OBJECTS += $(MY_SRC)/lib/rc/resource.o
EXTRA_SRCVPATH += $(MY_SRC)/lib/rc

EXTRA_LIBVPATH += $(INSTALL_ROOT_$(OS))/$(CPU)$(filter le be, $(VARIANT_LIST))$(CPUVARDIR_SUFFIX)/$(INSTALLDIR)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

LIBS += GAL VDK VSC EGL GLESv2

include $(qnx_build_dir)/math.mk

INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) samples/es20/vv_launcher)

include $(MKFILES_ROOT)/qtargets.mk
