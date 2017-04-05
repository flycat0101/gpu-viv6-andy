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
PINFO DESCRIPTION="Vivante CLC library"
endef

NAME=CLC

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/sdk/inc
EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/hal/os/qnx/user
EXTRA_INCVPATH += $(driver_root)/compiler/libVSC/include
EXTRA_INCVPATH += $(driver_root)/compiler/libCLC/inc
EXTRA_INCVPATH += $(driver_root)/compiler/libCLC/compiler
EXTRA_INCVPATH += $(driver_root)/compiler/libCLC/llvm/include
EXTRA_INCVPATH += $(driver_root)/compiler/libCLC/llvm/tools/clang/include

# from trunk/compiler/libCLC/common/makefile.linux
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/common/gc_cl_common.o
EXTRA_SRCVPATH += $(driver_root)/compiler/libCLC/common

# from trunk/compiler/libCLC/entry/makefile.linux
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/entry/gc_cl_entry.o
EXTRA_SRCVPATH += $(driver_root)/compiler/libCLC/entry

# from trunk/compiler/libCLC/compiler/makefile.linux
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/compiler/gc_cl_built_ins.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/compiler/gc_cl_compiler.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/compiler/gc_cl_emit_code.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/compiler/gc_cl_gen_code.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/compiler/gc_cl_ir.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/compiler/gc_cl_parser.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/compiler/gc_cl_parser_misc.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/compiler/gc_cl_scanner.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/compiler/gc_cl_scanner_misc.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/compiler/gc_cl_tune.o
EXTRA_SRCVPATH += $(driver_root)/compiler/libCLC/compiler

# from trunk/compiler/libCLC/preprocessor/makefile.linux
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/preprocessor/gc_cl_base.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/preprocessor/gc_cl_expression.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/preprocessor/gc_cl_hide_set.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/preprocessor/gc_cl_input_stream.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/preprocessor/gc_cl_macro_manager.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/preprocessor/gc_cl_syntax.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/preprocessor/gc_cl_syntax_util.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/preprocessor/gc_cl_token.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/preprocessor/gc_cl_api.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/preprocessor/gc_cl_macro_expand.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/preprocessor/gc_cl_preprocessor.o
EXTRA_SRCVPATH += $(driver_root)/compiler/libCLC/preprocessor

EXTRA_LIBVPATH += $(LOCAL_INSTALL)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

LIBS += VSC GAL LLVM_viv

CCFLAGS += -Wno-error=unused-value

ifeq ($(QNX_SDP700), 1)
# LIBS += cpp
# TODO FIXME HACK
LIBS += c++
else
LIBS += cpp
endif

EXTRA_LIBVPATH += $(QNX_TARGET)/$(CPUVARDIR)/lib/gcc/4.7.3

CCFLAGS += -D_LIB -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS

CCFLAGS += -DBUILD_OPENCL_12=1

include $(qnx_build_dir)/math.mk

LDFLAGS += -Wl,--version-script=$(driver_root)/compiler/libCLC/entry/libCLC.map

ifeq ($(filter so dll, $(VARIANT_LIST)),)
INSTALLDIR=/dev/null
endif

include $(MKFILES_ROOT)/qtargets.mk
