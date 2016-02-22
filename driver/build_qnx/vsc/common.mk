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
PINFO DESCRIPTION="Vivante OpenGLES 2.0 SC(end)"
endef

NAME=VSC

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/hal/os/qnx/user
EXTRA_INCVPATH += $(driver_root)/hal/user/arch
EXTRA_INCVPATH += $(driver_root)/hal/user/archvg
EXTRA_INCVPATH += $(driver_root)/arch/XAQ2/cmodel/inc
EXTRA_INCVPATH += $(driver_root)/compiler/libVSC/include

SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/old_impl/optimizer/gc_vsc_old_optimizer_dump.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/old_impl/optimizer/gc_vsc_old_optimizer.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/old_impl/optimizer/gc_vsc_old_optimizer_loadtime.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/old_impl/optimizer/gc_vsc_old_optimizer_util.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/old_impl/gc_vsc_old_compiler.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/old_impl/gc_vsc_old_recompile.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/old_impl/gc_vsc_old_linker.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/old_impl/gc_vsc_old_hw_code_gen.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/old_impl/gc_vsc_old_hw_linker.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/old_impl/gc_vsc_vir_gcsl_converter.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/old_impl/gc_vsc_gcsl_vir_converter.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/drvi/gc_vsc_drvi_compile.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/drvi/gc_vsc_drvi_ep_dump.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/drvi/gc_vsc_drvi_link.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/drvi/gc_vsc_drvi_recompile.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/chip/gc_vsc_chip_state_programming.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/chip/gc_vsc_chip_uarch_caps.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/chip/gc_vsc_chip_mc_codec.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/chip/gc_vsc_chip_mc_dump.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/asm/gc_vsc_asm_al_codec.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/array/gc_vsc_utils_array.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/base/gc_vsc_utils_base_node.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/base/gc_vsc_utils_bit_op.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/base/gc_vsc_utils_data_digest.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/base/gc_vsc_utils_dump.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/base/gc_vsc_utils_err.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/base/gc_vsc_utils_math.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/bitvector/gc_vsc_utils_bm.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/bitvector/gc_vsc_utils_bv.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/bitvector/gc_vsc_utils_sv.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/graph/gc_vsc_utils_dg.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/graph/gc_vsc_utils_udg.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/hash/gc_vsc_utils_hash.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/list/gc_vsc_utils_bi_list.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/list/gc_vsc_utils_uni_list.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/mm/gc_vsc_utils_mm.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/mm/gc_vsc_utils_mm_arena.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/mm/gc_vsc_utils_mm_buddy.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/mm/gc_vsc_utils_mm_primary_pool.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/string/gc_vsc_utils_string.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/table/gc_vsc_utils_block_table.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/utils/tree/gc_vsc_utils_tree.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/analysis/gc_vsc_vir_cfa.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/analysis/gc_vsc_vir_du.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/analysis/gc_vsc_vir_liveness.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/analysis/gc_vsc_vir_ms_dfa_iterator.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/analysis/gc_vsc_vir_ssa.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/analysis/gc_vsc_vir_ts_dfa_iterator.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/codegen/gc_vsc_vir_inst_scheduler.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/codegen/gc_vsc_vir_reg_alloc.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/codegen/gc_vsc_vir_mc_gen.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/codegen/gc_vsc_vir_ep_gen.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/codegen/gc_vsc_vir_ep_back_patch.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/ir/gc_vsc_vir_dump.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/ir/gc_vsc_vir_ir.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/ir/gc_vsc_vir_symbol_table.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/lower/gc_vsc_vir_hl_2_ml.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/lower/gc_vsc_vir_hl_2_ml_expand.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/lower/gc_vsc_vir_ll_2_ll_expand.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/lower/gc_vsc_vir_ll_2_ll_scalar.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/lower/gc_vsc_vir_ll_2_ll_machine.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/lower/gc_vsc_vir_lower_common_func.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/lower/gc_vsc_vir_pattern.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/lower/gc_vsc_vir_ml_2_ll.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/lower/gc_vsc_vir_ll_2_mc.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/passmanager/gc_vsc_options.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/passmanager/gc_vsc_vir_pass_mnger.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/transform/gc_vsc_vir_misc_opts.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/transform/gc_vsc_vir_peephole.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/transform/gc_vsc_vir_scalarization.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/transform/gc_vsc_vir_simplification.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/transform/gc_vsc_vir_cfo.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/transform/gc_vsc_vir_cpp.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/transform/gc_vsc_vir_dce.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/transform/gc_vsc_vir_inline.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/transform/gc_vsc_vir_fcp.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/transform/gc_vsc_vir_cpf.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/transform/gc_vsc_vir_cse.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/transform/gc_vsc_vir_uniform.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/transform/gc_vsc_vir_static_patch.o
SOURCE_OBJECTS += $(driver_root)/compiler/libVSC/vir/transform/gc_vsc_vir_vectorization.o

EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/old_impl
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/old_impl/optimizer
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/asm
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/chip
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/drvi
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/utils
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/utils/array
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/utils/base
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/utils/bitvector
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/utils/graph
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/utils/hash
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/utils/list
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/utils/mm
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/utils/string
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/utils/table
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/utils/tree
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/vir/analysis
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/vir/codegen
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/vir/ir
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/vir/lower
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/vir/passmanager
EXTRA_SRCVPATH += $(driver_root)/compiler/libVSC/vir/transform

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

LIBS += GAL-$(HARDWARENAME) $(STATIC_LIBS)

ifneq ($(filter v7, $(VARIANT_LIST)), v7)
	CCFLAGS += -mfpu=vfp -mfloat-abi=softfp
	LIBS += m-vfp
else
	LIBS += m
endif

LDFLAGS += -Wl,--version-script=$(driver_root)/compiler/libVSC/libVSC.map

ifeq ($(filter so dll, $(VARIANT_LIST)),)
INSTALLDIR=/dev/null
endif

include $(MKFILES_ROOT)/qtargets.mk
