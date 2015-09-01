//===--- FrontendAction.cpp -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Config/config.h"
#include "clang/Frontend/FrontendAction.h"
///KLC COMMENT OUT
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace clang;

FrontendAction::FrontendAction() : Instance(0) {}

FrontendAction::~FrontendAction() {}

///KLC COMMENT OUT NO AST
////KLC removed AST argument from above
void FrontendAction::setCurrentFile(llvm::StringRef Value, InputKind Kind){
  CurrentFile = Value;
  CurrentFileKind = Kind;
}

bool FrontendAction::BeginSourceFile(CompilerInstance &CI,
                                     llvm::StringRef Filename,
                                     InputKind InputKind) {
  assert(!Instance && "Already processing a source file!");
  assert(!Filename.empty() && "Unexpected empty filename!");
  setCurrentFile(Filename, InputKind);
  setCompilerInstance(&CI);

////KLC COMMENT OUT

  // Set up the file and source managers, if needed.
  if (!CI.hasFileManager())
    CI.createFileManager();
  if (!CI.hasSourceManager())
    CI.createSourceManager();


///KLC COMMENT OUT

  // Set up the preprocessor.
  CI.createPreprocessor();

  // Inform the diagnostic client we are processing a source file.
  CI.getDiagnosticClient().BeginSourceFile(CI.getLangOpts(),
                                           &CI.getPreprocessor());

  // Initialize the action.
  if (!BeginSourceFileAction(CI, Filename))
    goto failure;


////KLC comment out
////KLC THE FOLLOWING IS FOR PREPROCESSING ONLY COPIED FROM ABOVE
  {
    Preprocessor &PP = CI.getPreprocessor();
    PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
                                           PP.getLangOptions().NoBuiltin);
  }

  return true;


  // If we failed, reset state since the client will not end up calling the
  // matching EndSourceFile().
  failure:
////KLC COMMENT OUT

  CI.getDiagnosticClient().EndSourceFile();
  setCurrentFile("", IK_None);
  setCompilerInstance(0);
  return false;
}

void FrontendAction::Execute() {
  CompilerInstance &CI = getCompilerInstance();

///KLC COMMENT OUT

///KLC JUST COPY FROM ABOVE WHAT IS NEEDED
    if (!CI.InitializeSourceManager(getCurrentFile()))
      return;
    ExecuteAction();
}

void FrontendAction::EndSourceFile() {
  CompilerInstance &CI = getCompilerInstance();

  // Finalize the action.
  EndSourceFileAction();

//KLC COMMENT OUT AST

  // Inform the preprocessor we are done.
  if (CI.hasPreprocessor())
    CI.getPreprocessor().EndSourceFile();

  if (CI.getFrontendOpts().ShowStats) {
    llvm::errs() << "\nSTATISTICS FOR '" << getCurrentFile() << "':\n";
    CI.getPreprocessor().PrintStats();
    CI.getPreprocessor().getIdentifierTable().PrintStats();
    CI.getPreprocessor().getHeaderSearchInfo().PrintStats();
    CI.getSourceManager().PrintStats();
    llvm::errs() << "\n";
  }

  // Cleanup the output streams, and erase the output files if we encountered
  // an error.
  CI.clearOutputFiles(/*EraseFiles=*/(CI.getDiagnostics().getNumErrors() ? true : false));

  // Inform the diagnostic client we are done with this source file.
  CI.getDiagnosticClient().EndSourceFile();

////KLC COMMENT OUT AST

  setCompilerInstance(0);
  setCurrentFile("", IK_None);
}

///KLC COMMENT OUT
