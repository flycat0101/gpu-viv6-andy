//===----------------------------------------------------------------------===//
//
//  This file implements LLVM initialization and cleanup functions.
//
//===----------------------------------------------------------------------===//

// Declare initernal initialization and clean up functuons.
void InitModule_Debug();
void ExitModule_Debug();
void InitModule_CommandLine();
void ExitModule_CommandLine();
void InitModule_Statistic();
void ExitModule_Statistic();
void InitModule_Timer();
void ExitModule_Timer();

extern "C"
void clInitLLVM()
{
    InitModule_Debug();
    InitModule_CommandLine();
    InitModule_Statistic();
    InitModule_Timer();
}

extern "C"
void clDestroyLLVM()
{
    ExitModule_Timer();
    ExitModule_Statistic();
    ExitModule_CommandLine();
    ExitModule_Debug();
}
