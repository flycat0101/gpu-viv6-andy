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
PINFO DESCRIPTION="Vivante OpenGLES 2.0 SC(front)"
endef

NAME=libGLSLC

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/hal/os/qnx/user
EXTRA_INCVPATH += $(driver_root)/compiler/libGLSLC/inc
EXTRA_INCVPATH += $(driver_root)/compiler/libVSC/include

# from libGLESv2 (trunk/compiler/libGLSLC/common/Makefile.linux)
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/common/gc_glsl_common.o

# from libGLESv2 (trunk/compiler/libGLSLC/compiler/Makefile.linux)
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/compiler/gc_glsl_ast_walk.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/compiler/gc_glsl_built_ins.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/compiler/gc_glsl_compiler.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/compiler/gc_glsl_emit_code.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/compiler/gc_glsl_gen_code.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/compiler/gc_glsl_ir.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/compiler/gc_glsl_parser.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/compiler/gc_glsl_parser_misc.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/compiler/gc_glsl_scanner.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/compiler/gc_glsl_scanner_misc.o

# from libGLESv2 (trunk/compiler/libGLSLC/preprocessor/Makefile.linux)
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/preprocessor/gc_glsl_api.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/preprocessor/gc_glsl_base.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/preprocessor/gc_glsl_expression.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/preprocessor/gc_glsl_hide_set.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/preprocessor/gc_glsl_input_stream.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/preprocessor/gc_glsl_macro_expand.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/preprocessor/gc_glsl_macro_manager.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/preprocessor/gc_glsl_preprocessor.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/preprocessor/gc_glsl_syntax.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/preprocessor/gc_glsl_syntax_util.o
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/preprocessor/gc_glsl_token.o

# from libGLESv2 (trunk/compiler/libGLSLC/entry/Makefile.linux)
SOURCE_OBJECTS += $(driver_root)/compiler/libGLSLC/entry/gc_glsl_entry.o

EXTRA_SRCVPATH += $(driver_root)/compiler/libGLSLC/common
EXTRA_SRCVPATH += $(driver_root)/compiler/libGLSLC/compiler
EXTRA_SRCVPATH += $(driver_root)/compiler/libGLSLC/preprocessor
EXTRA_SRCVPATH += $(driver_root)/compiler/libGLSLC/entry

EXTRA_LIBVPATH += $(LOCAL_INSTALL)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

STATIC_LIBS += khronosS
$(foreach lib, $(STATIC_LIBS), $(eval LIBPREF_$(lib) = -Bstatic))
$(foreach lib, $(STATIC_LIBS), $(eval LIBPOST_$(lib) = -Bdynamic))

LIBS += VSC GAL $(STATIC_LIBS)

ifneq ($(filter v7, $(VARIANT_LIST)), v7)
	CCFLAGS += -mfpu=vfp -mfloat-abi=softfp
	LIBS += m-vfp
else
	LIBS += m
endif

LDFLAGS += -Wl,--version-script=$(driver_root)/compiler/libGLSLC/entry/libGLSLC.map

ifeq ($(filter so dll, $(VARIANT_LIST)),)
INSTALLDIR=/dev/null
endif

include $(MKFILES_ROOT)/qtargets.mk
