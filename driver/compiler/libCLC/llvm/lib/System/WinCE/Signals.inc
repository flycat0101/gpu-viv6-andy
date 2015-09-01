//===- WinCE/Signals.cpp - Win32 Signals Implementation ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides the Win32 specific implementation of the Signals class.
//
//===----------------------------------------------------------------------===//

#include "Win32.h"
#include <stdio.h>
#include <vector>
#include <algorithm>


// Forward declare.
static LONG WINAPI LLVMUnhandledExceptionFilter(LPEXCEPTION_POINTERS ep);
static BOOL WINAPI LLVMConsoleCtrlHandler(DWORD dwCtrlType);

// InterruptFunction - The function to call if ctrl-c is pressed.
static void (*InterruptFunction)() = 0;

static std::vector<llvm::sys::Path> *FilesToRemove = NULL;
static std::vector<std::pair<void(*)(void*), void*> > *CallBacksToRun = 0;
static bool RegisteredUnhandledExceptionFilter = false;
static bool CleanupExecuted = false;
#ifdef _MSC_VER
static bool ExitOnUnhandledExceptions = false;
#endif

// Windows creates a new thread to execute the console handler when an event
// (such as CTRL/C) occurs.  This causes concurrency issues with the above
// globals which this critical section addresses.
static CRITICAL_SECTION CriticalSection;

namespace llvm {

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only Win32 specific code
//===          and must not be UNIX code
//===----------------------------------------------------------------------===//

#ifdef _MSC_VER
/// CRTReportHook - Function called on a CRT debugging event.
static int CRTReportHook(int ReportType, char *Message, int *Return) {
  // Don't cause a DebugBreak() on return.
  if (Return)
    *Return = 0;

  // Don't call _CrtDbgReport.
  return TRUE;
}
#endif

static void RegisterHandler() {
  if (RegisteredUnhandledExceptionFilter) {
    EnterCriticalSection(&CriticalSection);
    return;
  }

  // Now's the time to create the critical section.  This is the first time
  // through here, and there's only one thread.
  InitializeCriticalSection(&CriticalSection);

  // Enter it immediately.  Now if someone hits CTRL/C, the console handler
  // can't proceed until the globals are updated.
  EnterCriticalSection(&CriticalSection);

  RegisteredUnhandledExceptionFilter = true;

  // Environment variable to disable any kind of crash dialog.
#ifdef _MSC_VER
  char *envVar = gcvNULL;
  gcoOS_GetEnv(gcvNULL, "LLVM_DISABLE_CRT_DEBUG", &envVar);
  if (envVar) {
    ExitOnUnhandledExceptions = true;
  }
#endif

  // IMPORTANT NOTE: Caller must call LeaveCriticalSection(&CriticalSection) or
  // else multi-threading problems will ensue.
}

// RemoveFileOnSignal - The public API
bool sys::RemoveFileOnSignal(const sys::Path &Filename, std::string* ErrMsg) {
  RegisterHandler();

  if (CleanupExecuted) {
    if (ErrMsg)
      *ErrMsg = "Process terminating -- cannot register for removal";
    return true;
  }

  if (FilesToRemove == NULL)
    FilesToRemove = new std::vector<sys::Path>;

  FilesToRemove->push_back(Filename);

  LeaveCriticalSection(&CriticalSection);
  return false;
}

/// PrintStackTraceOnErrorSignal - When an error signal (such as SIBABRT or
/// SIGSEGV) is delivered to the process, print a stack trace and then exit.
void sys::PrintStackTraceOnErrorSignal() {
  RegisterHandler();
  LeaveCriticalSection(&CriticalSection);
}


void sys::SetInterruptFunction(void (*IF)()) {
  RegisterHandler();
  InterruptFunction = IF;
  LeaveCriticalSection(&CriticalSection);
}


/// AddSignalHandler - Add a function to be called when a signal is delivered
/// to the process.  The handler can have a cookie passed to it to identify
/// what instance of the handler it is.
void sys::AddSignalHandler(void (*FnPtr)(void *), void *Cookie) {
  if (CallBacksToRun == 0)
    CallBacksToRun = new std::vector<std::pair<void(*)(void*), void*> >();
  CallBacksToRun->push_back(std::make_pair(FnPtr, Cookie));
  RegisterHandler();
  LeaveCriticalSection(&CriticalSection);
}
}

static void Cleanup() {
  EnterCriticalSection(&CriticalSection);

  // Prevent other thread from registering new files and directories for
  // removal, should we be executing because of the console handler callback.
  CleanupExecuted = true;

  // FIXME: open files cannot be deleted.

  if (FilesToRemove != NULL)
    while (!FilesToRemove->empty()) {
      FilesToRemove->back().eraseFromDisk();
      FilesToRemove->pop_back();
    }

  if (CallBacksToRun)
    for (unsigned i = 0, e = CallBacksToRun->size(); i != e; ++i)
      (*CallBacksToRun)[i].first((*CallBacksToRun)[i].second);

  LeaveCriticalSection(&CriticalSection);
}

void llvm::sys::RunInterruptHandlers() {
  Cleanup();
}

  // TODO - Need to implement LLVMUnhandledExceptionFilter for WinCE
static LONG WINAPI LLVMUnhandledExceptionFilter(LPEXCEPTION_POINTERS ep) {
  try {
    Cleanup();
  } catch (...) {
      assert(0 && "Crashed in LLVMUnhandledExceptionFilter");
  }
#ifdef _MSC_VER
  if (ExitOnUnhandledExceptions)
    _exit(-3);
#endif
  // Allow dialog box to pop up allowing choice to start debugger.
    return EXCEPTION_CONTINUE_SEARCH;
}


static BOOL WINAPI LLVMConsoleCtrlHandler(DWORD dwCtrlType) {
  // We are running in our very own thread, courtesy of Windows.
  EnterCriticalSection(&CriticalSection);
  Cleanup();

  // If an interrupt function has been set, go and run one it; otherwise,
  // the process dies.
  void (*IF)() = InterruptFunction;
  InterruptFunction = 0;      // Don't run it on another CTRL-C.

  if (IF) {
    // Note: if the interrupt function throws an exception, there is nothing
    // to catch it in this thread so it will kill the process.
    IF();                     // Run it now.
    LeaveCriticalSection(&CriticalSection);
    return TRUE;              // Don't kill the process.
  }

  // Allow normal processing to take place; i.e., the process dies.
  LeaveCriticalSection(&CriticalSection);
  return FALSE;
}

