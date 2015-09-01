//===- WinCE/Process.cpp - WinCE Process Implementation ------- -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides the WinCE specific implementation of the Process class.
//
//===----------------------------------------------------------------------===//

#include "Win32.h"
#include <psapi.h>
#include <malloc.h>
//#include <io.h>

#pragma comment(lib, "psapi.lib")

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only Win32 specific code
//===          and must not be UNIX code
//===----------------------------------------------------------------------===//


namespace llvm {
using namespace sys;

// This function retrieves the page size using GetSystemInfo and is present
// solely so it can be called once in Process::GetPageSize to initialize the
// static variable PageSize.
inline unsigned GetPageSizeOnce() {
  // NOTE: A 32-bit application running under WOW64 is supposed to use
  // GetNativeSystemInfo.  However, this interface is not present prior
  // to Windows XP so to use it requires dynamic linking.  It is not clear
  // how this affects the reported page size, if at all.  One could argue
  // that LLVM ought to run as 64-bits on a 64-bit system, anyway.
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  return static_cast<unsigned>(info.dwPageSize);
}

unsigned
Process::GetPageSize() {
  static const unsigned PageSize = GetPageSizeOnce();
  return PageSize;
}

size_t
Process::GetMallocUsage()
{
  // TODO - Need to implement for WinCE
  return 0;
}

size_t
Process::GetTotalMemoryUsage()
{
  // TODO - Need to implement for WinCE
  return 0;
}

void
Process::GetTimeUsage(
  TimeValue& elapsed, TimeValue& user_time, TimeValue& sys_time)
{
  // TODO - Need to implement for WinCE
#if 0
  elapsed = TimeValue::now();

  uint64_t ProcCreate, ProcExit, KernelTime, UserTime;
  GetProcessTimes(GetCurrentProcess(), (FILETIME*)&ProcCreate,
                  (FILETIME*)&ProcExit, (FILETIME*)&KernelTime,
                  (FILETIME*)&UserTime);

  // FILETIME's are # of 100 nanosecond ticks (1/10th of a microsecond)
  user_time.seconds( UserTime / 10000000 );
  user_time.nanoseconds( unsigned(UserTime % 10000000) * 100 );
  sys_time.seconds( KernelTime / 10000000 );
  sys_time.nanoseconds( unsigned(KernelTime % 10000000) * 100 );
#endif
}

int Process::GetCurrentUserId()
{
  return 65536;
}

int Process::GetCurrentGroupId()
{
  return 65536;
}

// Some LLVM programs such as bugpoint produce core files as a normal part of
// their operation. To prevent the disk from filling up, this configuration item
// does what's necessary to prevent their generation.
void Process::PreventCoreFiles() {
  // Windows doesn't do core files, but it does do modal pop-up message
  // boxes.  As this method is used by bugpoint, preventing these pop-ups
  // is the moral equivalent of suppressing core files.

  // TODO - SEM_FAILCRITICALERRORS not defined in WinCE
  //SetErrorMode(SEM_FAILCRITICALERRORS |
  //             SEM_NOGPFAULTERRORBOX |
  //             SEM_NOOPENFILEERRORBOX);
}

bool Process::StandardInIsUserInput() {
  return FileDescriptorIsDisplayed(0);
}

bool Process::StandardOutIsDisplayed() {
  return FileDescriptorIsDisplayed(1);
}

bool Process::StandardErrIsDisplayed() {
  return FileDescriptorIsDisplayed(2);
}

bool Process::FileDescriptorIsDisplayed(int fd) {
  // TODO - Need to implement GetFileType for WinCE
  return false;
}

unsigned Process::StandardOutColumns() {
  unsigned Columns = 0;
  // TODO - Need to implement GetConsoleScreenBufferInfo for WinCE
  return Columns;
}

unsigned Process::StandardErrColumns() {
  unsigned Columns = 0;
 // TODO - Need to implement GetConsoleScreenBufferInfo for WinCE
  return Columns;
}

// It always has colors.
bool Process::StandardErrHasColors() {
  return StandardErrIsDisplayed();
}

bool Process::StandardOutHasColors() {
  return StandardOutIsDisplayed();
}


bool Process::ColorNeedsFlush() {
  return true;
}

const char *Process::OutputBold(bool bg) {
  return 0;
}

const char *Process::OutputColor(char code, bool bold, bool bg) {
  return 0;
}

const char *Process::ResetColor() {
  return 0;
}

}
