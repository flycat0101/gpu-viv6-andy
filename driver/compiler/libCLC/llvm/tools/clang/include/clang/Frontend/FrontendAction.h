//===-- FrontendAction.h - Generic Frontend Action Interface ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_FRONTEND_FRONTENDACTION_H
#define LLVM_CLANG_FRONTEND_FRONTENDACTION_H

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/OwningPtr.h"
#include <string>
#include <vector>

namespace llvm {
  class raw_ostream;
}

namespace clang {
///KLC COMMENT OUT
class CompilerInstance;

enum InputKind {
  IK_None,
  IK_Asm,
  IK_C,
  IK_CXX,
  IK_ObjC,
  IK_ObjCXX,
  IK_PreprocessedC,
  IK_PreprocessedCXX,
  IK_PreprocessedObjC,
  IK_PreprocessedObjCXX,
  IK_OpenCL,
  IK_AST,
  IK_LLVM_IR
};


/// FrontendAction - Abstract base class for actions which can be performed by
/// the frontend.
class FrontendAction {
  std::string CurrentFile;
  InputKind CurrentFileKind;
  CompilerInstance *Instance;
////KLC comment out

protected:
  /// @name Implementation Action Interface
  /// @{

////KLC COMMENT OUT

  /// BeginSourceFileAction - Callback at the start of processing a single
  /// input.
  ///
  /// \return True on success; on failure \see ExecutionAction() and
  /// EndSourceFileAction() will not be called.
  virtual bool BeginSourceFileAction(CompilerInstance &CI,
                                     llvm::StringRef Filename) {
    return true;
  }

  /// ExecuteAction - Callback to run the program action, using the initialized
  /// compiler instance.
  ///
  /// This routine is guaranteed to only be called between \see
  /// BeginSourceFileAction() and \see EndSourceFileAction().
  virtual void ExecuteAction() = 0;

  /// EndSourceFileAction - Callback at the end of processing a single input;
  /// this is guaranteed to only be called following a successful call to
  /// BeginSourceFileAction (and BeingSourceFile).
  virtual void EndSourceFileAction() {}

  /// @}

public:
  FrontendAction();
  virtual ~FrontendAction();

  /// @name Compiler Instance Access
  /// @{

  CompilerInstance &getCompilerInstance() const {
    assert(Instance && "Compiler instance not registered!");
    return *Instance;
  }

  void setCompilerInstance(CompilerInstance *Value) { Instance = Value; }


  /// @}
  /// @name Current File Information
  /// @{

////KLC COMMENT OUT

  const std::string &getCurrentFile() const {
    assert(!CurrentFile.empty() && "No current file!");
    return CurrentFile;
  }

  InputKind getCurrentFileKind() const {
    assert(!CurrentFile.empty() && "No current file!");
    return CurrentFileKind;
  }

////KLC COMMENT OUT
////KLC REMOVED ASTUNIT from above
  void setCurrentFile(llvm::StringRef Value, InputKind Kind);

  /// @}
  /// @name Supported Modes
  /// @{

  /// usesPreprocessorOnly - Does this action only use the preprocessor? If so
  /// no AST context will be created and this action will be invalid with AST
  /// file inputs.
  virtual bool usesPreprocessorOnly() const = 0;

////KLC COMMENT OUT

  /// hasPCHSupport - Does this action support use with PCH?
  virtual bool hasPCHSupport() const { return !usesPreprocessorOnly(); }

////KLC COMMENT OUT
  /// @}
  /// @name Public Action Interface
  /// @{

  /// BeginSourceFile - Prepare the action for processing the input file \arg
  /// Filename; this is run after the options and frontend have been
  /// initialized, but prior to executing any per-file processing.
  ///
  /// \param CI - The compiler instance this action is being run from. The
  /// action may store and use this object up until the matching EndSourceFile
  /// action.
  ///
  /// \param Filename - The input filename, which will be made available to
  /// clients via \see getCurrentFile().
  ///
  /// \param InputKind - The type of input. Some input kinds are handled
  /// specially, for example AST inputs, since the AST file itself contains
  /// several objects which would normally be owned by the
  /// CompilerInstance. When processing AST input files, these objects should
  /// generally not be initialized in the CompilerInstance -- they will
  /// automatically be shared with the AST file in between \see
  /// BeginSourceFile() and \see EndSourceFile().
  ///
  /// \return True on success; the compilation of this file should be aborted
  /// and neither Execute nor EndSourceFile should be called.
  bool BeginSourceFile(CompilerInstance &CI, llvm::StringRef Filename,
                       InputKind Kind);

  /// Execute - Set the source managers main input file, and run the action.
  void Execute();

  /// EndSourceFile - Perform any per-file post processing, deallocate per-file
  /// objects, and run statistics and output file cleanup code.
  void EndSourceFile();

  /// @}
};

////KLC COMMENT OUT

/// PreprocessorFrontendAction - Abstract base class to use for preprocessor
/// based frontend actions.
class PreprocessorFrontendAction : public FrontendAction {
////KLC COMMENT OUT

public:
  virtual bool usesPreprocessorOnly() const { return true; }
};

}  // end namespace clang

#endif
