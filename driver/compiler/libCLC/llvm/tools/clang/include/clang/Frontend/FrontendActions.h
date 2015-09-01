//===-- FrontendActions.h - Useful Frontend Actions -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_FRONTEND_FRONTENDACTIONS_H
#define LLVM_CLANG_FRONTEND_FRONTENDACTIONS_H

#include "clang/Frontend/FrontendAction.h"
#include <string>
#include <vector>

namespace clang {

//===----------------------------------------------------------------------===//
// Custom Consumer Actions
//===----------------------------------------------------------------------===//

class InitOnlyAction : public FrontendAction {
  virtual void ExecuteAction();

////KLC COMMENT OUT

public:
  // Don't claim to only use the preprocessor, we want to follow the AST path,
  // but do nothing.
  virtual bool usesPreprocessorOnly() const { return false; }
};


////KLC COMMENT OUT

//===----------------------------------------------------------------------===//
// Preprocessor Actions
//===----------------------------------------------------------------------===//

class DumpRawTokensAction : public PreprocessorFrontendAction {
protected:
  void ExecuteAction();
};

class DumpTokensAction : public PreprocessorFrontendAction {
protected:
  void ExecuteAction();
};

class GeneratePTHAction : public PreprocessorFrontendAction {
protected:
  void ExecuteAction();
};

class PreprocessOnlyAction : public PreprocessorFrontendAction {
protected:
  void ExecuteAction();
};

class PrintPreprocessedAction : public PreprocessorFrontendAction {
protected:
  void ExecuteAction();

  virtual bool hasPCHSupport() const { return true; }
};

}  // end namespace clang

#endif
