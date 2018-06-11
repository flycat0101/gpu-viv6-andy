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
PINFO DESCRIPTION="LLVM for CLC library"
endef

NAME=libLLVM_viv

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

#
# from trunk/compiler/libCLC/llvm/makefile.linux
#

# Support
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/Allocator.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/APFloat.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/APInt.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/APSInt.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/circular_raw_ostream.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/CommandLine.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/ConstantRange.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/CrashRecoveryContext.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/DAGDeltaAlgorithm.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/Debug.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/DeltaAlgorithm.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/Dwarf.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/ErrorHandling.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/FileUtilities.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/FoldingSet.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/FormattedStream.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/GraphWriter.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/IsInf.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/IsNAN.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/ManagedStatic.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/MemoryBuffer.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/MemoryObject.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/PluginLoader.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/PrettyStackTrace.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/raw_os_ostream.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/raw_ostream.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/regcomp.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/regerror.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/regexec.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/regfree.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/regstrlcpy.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/Regex.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/SmallPtrSet.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/SmallVector.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/SourceMgr.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/Statistic.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/StringExtras.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/StringMap.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/StringPool.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/StringRef.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/SystemUtils.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/TargetRegistry.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/Timer.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/Triple.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/Support/Twine.o
EXTRA_SRCVPATH += $(driver_root)/compiler/libCLC/llvm/lib/Support

# System
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/Alarm.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/Atomic.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/Disassembler.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/DynamicLibrary.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/Errno.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/Host.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/IncludeFile.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/Memory.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/Mutex.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/Path.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/Process.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/Program.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/RWMutex.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/SearchForAddressOfSpecialSymbol.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/Signals.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/Threading.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/ThreadLocal.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/TimeValue.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/System/Valgrind.o
EXTRA_SRCVPATH += $(driver_root)/compiler/libCLC/llvm/lib/System

# VMCore
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/VMCore/LLVMContext.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/lib/VMCore/LLVMContextImpl.o
EXTRA_SRCVPATH += $(driver_root)/compiler/libCLC/llvm/lib/VMCore

# Basic
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Basic/Builtins.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Basic/Diagnostic.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Basic/FileManager.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Basic/IdentifierTable.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Basic/SourceLocation.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Basic/SourceManager.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Basic/TargetInfo.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Basic/Targets.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Basic/TokenKinds.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Basic/Version.o
EXTRA_SRCVPATH += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Basic

# Driver
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/Action.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/Arg.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/ArgList.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/CC1AsOptions.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/CC1Options.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/Compilation.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/Driver.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/DriverOptions.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/HostInfo.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/Job.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/Option.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/OptTable.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/Phases.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/ToolChain.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/ToolChains.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/Tool.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/Tools.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver/Types.o
EXTRA_SRCVPATH += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Driver

# Frontend
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Frontend/CacheTokens.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Frontend/CompilerInstance.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Frontend/CompilerInvocation.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Frontend/FrontendAction.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Frontend/FrontendActions.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Frontend/FrontendOptions.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Frontend/InitHeaderSearch.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Frontend/InitPreprocessor.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Frontend/LangStandards.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Frontend/PrintPreprocessedOutput.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Frontend/TextDiagnosticBuffer.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Frontend/TextDiagnosticPrinter.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Frontend/VerifyDiagnosticsClient.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Frontend/Warnings.o
EXTRA_SRCVPATH += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Frontend

# FrontendTool
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/FrontendTool/ExecuteCompilerInvocation.o
EXTRA_SRCVPATH += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/FrontendTool

# Lex
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/HeaderMap.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/HeaderSearch.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/Lexer.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/LiteralSupport.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/MacroArgs.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/MacroInfo.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/PPCaching.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/PPDirectives.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/PPExpressions.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/PPLexerChange.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/PPMacroExpansion.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/Pragma.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/PreprocessingRecord.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/Preprocessor.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/PreprocessorLexer.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/PTHLexer.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/ScratchBuffer.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/TokenConcatenation.o
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex/TokenLexer.o
EXTRA_SRCVPATH += $(driver_root)/compiler/libCLC/llvm/tools/clang/lib/Lex

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

# from trunk/compiler/libCLC/common/makefile.linux
SOURCE_OBJECTS += $(driver_root)/compiler/libCLC/common/gc_cl_common.o
EXTRA_SRCVPATH += $(driver_root)/compiler/libCLC/common

EXTRA_LIBVPATH += $(LOCAL_INSTALL)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

CCFLAGS := $(filter-out -ansi,$(CCFLAGS))
CCFLAGS += -Wno-error=narrowing
CCFLAGS += -Wno-error=enum-compare
CCFLAGS += -Wno-error=maybe-uninitialized
CCFLAGS += -D_LIB -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS
CCFLAGS += -DBUILD_OPENCL_12=1

LDOPTS += -lVSC -lGAL

CCFLAGS += -Wno-error=unused-value -Wno-attributes

include $(qnx_build_dir)/math.mk

LDFLAGS += -Wl,--version-script=$(driver_root)/compiler/libCLC/llvm/libLLVM.map

ifeq ($(filter so dll, $(VARIANT_LIST)),)
INSTALLDIR=/dev/null
endif

include $(MKFILES_ROOT)/qtargets.mk
