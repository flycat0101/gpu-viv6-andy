//===-- LLVMContextImpl.h - The LLVMContextImpl opaque class ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file declares LLVMContextImpl, the opaque implementation
//  of LLVMContext.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LLVMCONTEXT_IMPL_H
#define LLVM_LLVMCONTEXT_IMPL_H


#include "llvm/LLVMContext.h"
///KLC COMMENT OUT
#include "llvm/ADT/StringMap.h"
#include <vector>

namespace llvm {

///KLC COMMENT OUT
class LLVMContext;

///KLC COMMENT OUT

class LLVMContextImpl {
public:
///KLC COMMENT OUT


  /// CustomMDKindNames - Map to hold the metadata string to ID mapping.
  StringMap<unsigned> CustomMDKindNames;

///KLC COMMENT OUT

  LLVMContextImpl(/*LLVMContext &C*/);
  ~LLVMContextImpl();
};

}

#endif
