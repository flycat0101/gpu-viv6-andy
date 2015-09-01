//===--- Utils.h - Misc utilities for the front-end -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This header contains miscellaneous utilities for various front-end actions
//  which were split from Frontend to minimise Frontend's dependencies.
//
//===------------------ukkk:----------------------------------------------------===//

#ifndef LLVM_CLANG_FRONTENDTOOL_UTILS_H
#define LLVM_CLANG_FRONTENDTOOL_UTILS_H

#include "gc_cl_compiler.h"

namespace clang {

class CompilerInstance;

/// ExecuteCompilerInvocation - Execute the given actions described by the
/// compiler invocation object in the given compiler instance.
///
/// \return - True on success.
bool ExecuteCompilerInvocation(CompilerInstance *Clang);
}
gceSTATUS Clang_Preprocess_Hardcoded(cloCOMPILER Compiler, const char** strings, unsigned count, const char* Options,
char ** ppedStrings, unsigned *ppedCount);
#ifdef __cplusplus
extern "C" {
#endif
gceSTATUS Clang_Preprocess(cloCOMPILER Compiler, const char** strings, unsigned count, const char* Options,
                           char ** ppedStrings, unsigned *ppedCount);
#ifdef __cplusplus
}
#endif
#endif
