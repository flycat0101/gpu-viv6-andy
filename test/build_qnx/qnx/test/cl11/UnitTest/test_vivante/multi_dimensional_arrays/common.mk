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

qnx_build_dir:=$(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../../../../..)

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION="multi_dimensional_arrays"
endef

NAME=multi_dimensional_arrays

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/sdk/inc
EXTRA_INCVPATH += $(driver_root)/build_qnx/clc_llvm/inc

SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/main.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/kernelgenerator.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/kernelloader.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/roundingmode.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/multidimensioanlarraystest.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/multidimensionalarraystest2D.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/multidimensionalarraystest3D.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/multidimensionalarraystestrunner.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/randnumgenerator.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/resultchecker.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dchar16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dchar2.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dchar3.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dchar4.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dchar8.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dchar.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dfloat16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dfloat2.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dfloat3.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dfloat4.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dfloat8.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dfloat.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dint16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dint2.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dint3.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dint4.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dint8.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dint.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dshort16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dshort2.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dshort3.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dshort4.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dshort8.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dshort.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2duchar16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2duchar2.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2duchar3.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2duchar4.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2duchar8.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2duchar.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2duint16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2duint2.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2duint3.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2duint4.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2duint8.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2duint.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dushort16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dushort2.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dushort3.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dushort4.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dushort8.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test2dushort.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dchar16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dchar2.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dchar3.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dchar4.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dchar8.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dchar.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dfloat16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dfloat2.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dfloat3.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dfloat4.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dfloat8.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dfloat.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dint16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dint2.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dint3.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dint4.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dint8.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dint.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dshort16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dshort2.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dshort3.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dshort4.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dshort8.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dshort.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3duchar16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3duchar2.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3duchar3.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3duchar4.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3duchar8.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3duchar.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3duint16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3duint2.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3duint3.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3duint4.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3duint8.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3duint.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dushort16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dushort2.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dushort3.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dushort4.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dushort8.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays/test3dushort.o
EXTRA_SRCVPATH += $(CL11_TEST_DIR)/UnitTest/test_vivante/multi_dimensional_arrays

EXTRA_LIBVPATH += $(INSTALL_ROOT_$(OS))/$(CPU)$(filter le be, $(VARIANT_LIST))$(CPUVARDIR_SUFFIX)/$(INSTALLDIR)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

LDOPTS += -lGAL -lVSC -lCLC -lOpenCL

include $(qnx_build_dir)/math.mk

INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) samples/cl11/UnitTest/test_vivante)

include $(MKFILES_ROOT)/qtargets.mk
