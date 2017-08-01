//===--- ExecuteCompilerInvocation.cpp ------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file holds ExecuteCompilerInvocation(). It is split into its own file to
// minimize the impact of pulling in essentially everything else in Clang.
//
//===----------------------------------------------------------------------===//
#include "gc_cl_compiler_int.h"
#include "llvm/Config/config.h"
#include "clang/FrontendTool/Utils.h"
#include "clang/Driver/CC1Options.h"
#include "clang/Driver/OptTable.h"
#include "clang/Basic/FileManager.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/DiagnosticOptions.h"
#include "clang/Frontend/LangStandard.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/Utils.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/MacroInfo.h"
#include "llvm/LLVMContext.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/System/Path.h"
#include "llvm/System/DynamicLibrary.h"
#include "llvm/System/Host.h"
#include <algorithm>
using namespace clang;

static FrontendAction *CreateFrontendBaseAction(CompilerInstance &CI) {
  using namespace clang::frontend;
/***KLC Just return preprocessing action ***/
  return new PrintPreprocessedAction();
}

bool clang::ExecuteCompilerInvocation(CompilerInstance *Clang) {
  // Honor -help.
/**** KLC
  if (Clang->getFrontendOpts().ShowHelp) {
    llvm::OwningPtr<driver::OptTable> Opts(driver::createCC1OptTable());
    Opts->PrintHelp(llvm::outs(), "clang -cc1",
                    "LLVM 'Clang' Compiler: http://clang.llvm.org");
    return 0;
  }

  // Honor -version.
  //
  // FIXME: Use a better -version message?
  if (Clang->getFrontendOpts().ShowVersion) {
    llvm::cl::PrintVersionMessage();
    return 0;
  }

  // Honor -mllvm.
  //
  // FIXME: Remove this, one day.
  if (!Clang->getFrontendOpts().LLVMArgs.empty()) {
    unsigned NumArgs = Clang->getFrontendOpts().LLVMArgs.size();
    const char **Args = new const char*[NumArgs + 2];
    Args[0] = "clang (LLVM option parsing)";
    for (unsigned i = 0; i != NumArgs; ++i)
      Args[i + 1] = Clang->getFrontendOpts().LLVMArgs[i].c_str();
    Args[NumArgs + 1] = 0;
    llvm::cl::ParseCommandLineOptions(NumArgs + 1, const_cast<char **>(Args));
  }

  // Load any requested plugins.
  for (unsigned i = 0,
         e = Clang->getFrontendOpts().Plugins.size(); i != e; ++i) {
    const std::string &Path = Clang->getFrontendOpts().Plugins[i];
    std::string Error;
    if (llvm::sys::DynamicLibrary::LoadLibraryPermanently(Path.c_str(), &Error))
      Clang->getDiagnostics().Report(diag::err_fe_unable_to_load_plugin)
        << Path << Error;
  }
KLC  ***/

  // If there were errors in processing arguments, don't do anything else.
  bool Success = false;
  if (!Clang->getDiagnostics().getNumErrors()) {
    // Create and execute the frontend action.
    llvm::OwningPtr<FrontendAction> Act(CreateFrontendBaseAction(*Clang));
////KLC
/** KLC call directly to BaseAction instead
    llvm::OwningPtr<FrontendAction> Act(CreateFrontendAction(*Clang));
KLC    **/
    if (Act) {
      Success = Clang->ExecuteAction(*Act);
      if (Clang->getFrontendOpts().DisableFree)
        Act.take();
    }
  }

  return Success;
}

#define RESOURCE_DIRECTORY "/driver/openGL/libCL/frontend/llvm/bin/lib/clang/2.8"

static char *_IncludePaths[] = {
/* 0 */  (char *) ".",
};

static int _IncludePathSels[] = {
 1};

#define _cldFILENAME_MAX 1024
#define NUM_INCLUDE_PATHS    sizeof(_IncludePaths)/ sizeof(char *)
static int _NumIncludePaths = NUM_INCLUDE_PATHS;

static gceSTATUS
GetTemporaryFilename(
cloCOMPILER Compiler,
gctUINT StringNo,
const char *tmpdir,
char *Suffix,
char *namebuffer,
gctSIZE_T bufsize
)
{
  llvm::sys::Path P(tmpdir);
  std::string Error;
  P.appendComponent("cl");
  if (P.makeUnique(false, &Error)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    0,
                                    StringNo,
                                    clvREPORT_ERROR,
                                    "Unable to make temporary file"));
    return gcvSTATUS_INVALID_ARGUMENT;
  }

  // FIXME: Grumble, makeUnique sometimes leaves the file around!?  PR3837.
  P.eraseFromDisk(false, 0);

  P.appendSuffix(Suffix);
  gcmVERIFY_OK(gcoOS_StrCopySafe(namebuffer, bufsize, P.str().c_str()));
  return gcvSTATUS_OK;
}

static void
GetTemporaryDir(char *dirnamebuf, gctSIZE_T bufsize) {
  gctSTRING TmpDir = gcvNULL;

#if defined(UNDER_CE)
  char cwdpath[MAX_PATH];
#elif defined(ANDROID)
  char path[_cldFILENAME_MAX];
#endif

  gcoOS_GetEnv(gcvNULL,
               "TMPDIR",
               &TmpDir);
  if (!TmpDir) {
    gcoOS_GetEnv(gcvNULL,
                 "TEMP",
                 &TmpDir);
  }
  if (!TmpDir) {
    gcoOS_GetEnv(gcvNULL,
                 "TMP",
                 &TmpDir);
  }
  if (!TmpDir) {
    gcoOS_GetEnv(gcvNULL,
                 "/tmp",
                 &TmpDir);
  }
  if (!TmpDir)
  {
#if defined(UNDER_CE)
    /* Wince has no relative path */
    gcoOS_GetCwd(gcvNULL, MAX_PATH, cwdpath);
    TmpDir = cwdpath;
#elif defined(ANDROID)
    gctSIZE_T len;
    gctFILE filp;
    static const char prefix[] = "/data/data/";

    /* Could these fail? */
    gcoOS_Open(gcvNULL, "/proc/self/cmdline", gcvFILE_READ, &filp);
    gcoOS_Read(gcvNULL, filp, _cldFILENAME_MAX - 1, path, &len);
    gcoOS_Close(gcvNULL, filp);

    /* Add terminator. */
    path[len] = '\0';

    if (strchr(path, '/')) {
      /* Like a relative path or abs path. */
      TmpDir = ".";
    }
    else if (strchr(path, '.') && len < _cldFILENAME_MAX - sizeof (prefix)) {
      /* Like an android apk. */
      for (int i = len; i >= 0; i--) {
        path[i + sizeof (prefix) - 1] = path[i];
      }

      gcoOS_MemCopy(path, prefix, sizeof (prefix) - 1);
      gcoOS_StrCatSafe(path, _cldFILENAME_MAX, "/cache/");

      TmpDir = path;
    }
    else {
      TmpDir = ".";
    }
#else
    TmpDir = (gctSTRING) ".";
#endif
  }
  llvm::sys::Path P(TmpDir);
  gcmASSERT(bufsize > gcoOS_StrLen(P.str().c_str(), gcvNULL));
  gcmVERIFY_OK(gcoOS_StrCopySafe(dirnamebuf, bufsize, P.str().c_str()));
  return;
}

/***************************************************************************************
***************************************************************************************/
// Computes file length.
static gctUINT32
_Flength(gctFILE Fptr)
{
    gctUINT32 length;
        gcoOS_Seek(gcvNULL, Fptr, 0, gcvFILE_SEEK_END);
        gcoOS_GetPos(gcvNULL, Fptr, &length);
        gcoOS_Seek(gcvNULL, Fptr, 0, gcvFILE_SEEK_SET);
    return length;
}

#ifdef _WIN32
    #if !defined(UNDER_CE)
        #include <direct.h>
        #define GetCurrentDir _getcwd
        #define IS_DIR_SEPARATOR_CHAR(x) ((x) == '/' || (x) == '\\')
    #else
        #define GetCurrentDir(buf, size) gcoOS_GetCwd(gcvNULL, size, buf)
    #endif
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
    #define IS_DIR_SEPARATOR_CHAR(x) ((x) == '/')
#endif

// Load a source file into a buffer
// returns ptr to buffer: success
//         NULL: failure
// The buffer should be freed after use
char *
_LoadFileToBuffer(cloCOMPILER Compiler, gctUINT StringNo, const char *FName)
{
    gceSTATUS status;
    gctFILE fptr = gcvNULL;
    const char *directoryname;
    char fullpathname[_cldFILENAME_MAX + 1];
    gctSIZE_T len;
    FileManager FileMgr;

    const FileEntry *File = FileMgr.getFile(FName);
    if(File == NULL) {
       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                       0,
                                       StringNo,
                                       clvREPORT_ERROR,
                                       "File %s does not exist",
                                       FName));
       return gcvNULL;
    }
    else {
       const DirectoryEntry *DirInfo = File->getDir();

       if(DirInfo == NULL) {
          gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                          0,
                                          StringNo,
                                          clvREPORT_ERROR,
                                          "Cannot get directory name of file %s",
                                          FName));
         return gcvNULL;
       }
       directoryname = DirInfo->getName();
    }
#ifdef _DEBUG
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    0,
                                    StringNo,
                                    clvREPORT_INFO,
                                    "The directory name of file %s is %s\n",
                                    FName,
                                    directoryname));
#endif
    len = gcoOS_StrLen(directoryname, gcvNULL);
    if(len == 1 && *directoryname == '.') { /*relative to working directory*/
      if (!GetCurrentDir(fullpathname, sizeof(fullpathname))) {
          gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                          0,
                                          StringNo,
                                          clvREPORT_ERROR,
                                          "Can not get the working directory name file: %s",
                                          FName));
          return gcvNULL;
      }
      else {
         len = gcoOS_StrLen(fullpathname, gcvNULL);
         fullpathname[len++] = '/';
         while (*FName != '\0') fullpathname[len++] = *FName++;
             fullpathname[len] = '\0';
#ifdef _DEBUG
             gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                             0,
                                             StringNo,
                                             clvREPORT_INFO,
                                             "The full path name is %s",
                                             fullpathname));
#endif
         }
      }
      else {
         gcmVERIFY_OK(gcoOS_StrCopySafe(fullpathname, _cldFILENAME_MAX + 1, FName));
      }

      status = gcoOS_Open(gcvNULL,
                          fullpathname,
                          gcvFILE_READ,
                          &fptr);
      if(gcmIS_ERROR(status)) {
          gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                          0,
                                          StringNo,
                                          clvREPORT_ERROR,
                                         "Can not open file: %s",
                                          fullpathname));
          return gcvNULL;
      }

      gctUINT32 length = _Flength(fptr);
      char *source = gcvNULL;
      gctPOINTER pointer;

      status = gcoOS_Allocate(gcvNULL,
                              length + 1,
                              (gctPOINTER *)&pointer);

      if(gcmIS_ERROR(status)) {
          gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                          0,
                                          StringNo,
                                          clvREPORT_ERROR,
                                         "Out of memory"));
      }
      else {
          gctSIZE_T byteRead;
          source = (char *) pointer;
          status = gcoOS_Read(gcvNULL,
                              fptr,
                              length,
                              source,
                              &byteRead);
          if(gcmIS_ERROR(status)) {
              gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                              0,
                                              StringNo,
                                              clvREPORT_ERROR,
                                             "Read file \"%s\" error",
                                              fullpathname));
          }
          source[byteRead] = '\0';
    }
    gcoOS_Close(gcvNULL, fptr);
    return source;
}

gceSTATUS
Clang_Preprocess_Hardcoded(cloCOMPILER Compiler,
const char **source,
unsigned count,
const char *Options,
char **source_pped,
unsigned *pped_count)
{
  gceSTATUS status = gcvSTATUS_OK;
  gctSIZE_T fileNameLength;
  gctCHAR *fileNameBuffer = gcvNULL;
  gctPOINTER pointer;
  gctUINT offset = 0;
  llvm::OwningPtr<CompilerInstance> Clang(new CompilerInstance());
  Clang->setLLVMContext(new llvm::LLVMContext());
  CompilerInvocation& Invocation = Clang->getInvocation();
  llvm::OwningPtr<driver::OptTable> Opts(driver::createCC1OptTable());
  *pped_count = 0;

  FrontendOptions& Frontendopts = Invocation.getFrontendOpts();
  Frontendopts.ProgramAction = frontend::PrintPreprocessedInput;

  LangOptions& Langopts = Invocation.getLangOpts();
  const LangStandard &Std = LangStandard::getLangStandardForKind(LangStandard::lang_opencl);
  Langopts.BCPLComment = Std.hasBCPLComments();
  Langopts.C99 = Std.isC99();
  Langopts.CPlusPlus = Std.isCPlusPlus();
  Langopts.CPlusPlus0x = Std.isCPlusPlus0x();
  Langopts.Digraphs = Std.hasDigraphs();
  Langopts.GNUMode = Std.isGNUMode();
  Langopts.GNUInline = !Std.isC99();
  Langopts.HexFloats = Std.hasHexFloats();
  Langopts.ImplicitInt = Std.hasImplicitInt();
  // OpenCL has some additional defaults.
  Langopts.OpenCL = 1;
  Langopts.AltiVec = 1;
  Langopts.CXXOperatorNames = 1;
  Langopts.LaxVectorConversions = 1;
  char tmpdir[_cldFILENAME_MAX];

  if(clmHasRightLanguageVersion(Compiler, _cldCL1Dot2)) {
     Langopts.OpenCLVersion = (char *) "120";
  }
  else {
     Langopts.OpenCLVersion = (char *) "110";
  }
  HeaderSearchOptions& Headersearchopts = Invocation.getHeaderSearchOpts();
  Headersearchopts.Sysroot = "/";
  Headersearchopts.Verbose = 0;
  Headersearchopts.UseBuiltinIncludes = 1;
  Headersearchopts.UseStandardIncludes = 1;
  Headersearchopts.  UseStandardCXXIncludes = 1;
  char *rootDir = gcvNULL;
  gcoOS_GetEnv(gcvNULL, "AQROOT", &rootDir);

  if(rootDir) {
    fileNameLength = gcoOS_StrLen(rootDir, gcvNULL);
  }
  else {
    rootDir = (char *) ".";
    fileNameLength = 1;
  }
  fileNameLength += gcoOS_StrLen(RESOURCE_DIRECTORY, gcvNULL) + 1;
  status = cloCOMPILER_Allocate(Compiler, fileNameLength, (gctPOINTER *)&pointer);
  if (gcmIS_ERROR(status)) return status;

  fileNameBuffer = (gctCHAR *)pointer;
  gcmVERIFY_OK(gcoOS_StrCopySafe(fileNameBuffer, fileNameLength, rootDir));
  gcmVERIFY_OK(gcoOS_StrCatSafe(fileNameBuffer, fileNameLength, RESOURCE_DIRECTORY));
  Headersearchopts.ResourceDir = fileNameBuffer;

  PreprocessorOptions& Preprocessoropts = Invocation.getPreprocessorOpts();
  Preprocessoropts.TokenCache = Preprocessoropts.ImplicitPTHInclude;
  Preprocessoropts.UsePredefines = 1;

  PreprocessorOutputOptions& Preprocessoroutputopts = Invocation.getPreprocessorOutputOpts();
  Preprocessoroutputopts.ShowCPP = 1;
  Preprocessoroutputopts.ShowComments = 0;
  Preprocessoroutputopts.ShowHeaderIncludes = 0;
  Preprocessoroutputopts.ShowLineMarkers = 1;
  Preprocessoroutputopts.ShowMacroComments = 0;
  Preprocessoroutputopts.ShowMacros = 0;

  GetTemporaryDir(tmpdir, _cldFILENAME_MAX);

  for(int i=0; i < _NumIncludePaths; i++) {
     if(_IncludePathSels[i]) {
        Headersearchopts.AddPath(_IncludePaths[i], frontend::Angled, true, false, true);
     }
  }

  if(Options != NULL) {
     const std::string options(Options);

     std::string::size_type lastPos = options.find_first_not_of(" ", 0);

     while (std::string::npos != lastPos) {
        if (options.compare(lastPos, 1, "-") == 0) {
            std::string::size_type pos;
            lastPos++;
            if (options.compare(lastPos, 1, "I") == 0) {
                lastPos++;
                if(options.compare(lastPos, 1, " ") == 0) { /* there are spaces in between */
                   lastPos = options.find_first_not_of(" ", lastPos + 1);
                }
                pos = options.find_first_of(" ", lastPos);
                std::string normalizedPath(options.substr(lastPos, pos - lastPos).c_str());
                std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
                Headersearchopts.AddPath(normalizedPath, frontend::Angled, true, false, true);
            }
            else if (options.compare(lastPos, 1, "D") == 0) {
                lastPos++;
                if(options.compare(lastPos, 1, " ") == 0) { /* there are spaces in between */
                   lastPos = options.find_first_not_of(" ", lastPos + 1);
                }
                pos = options.find_first_of(" ", lastPos);
                Preprocessoropts.addMacroDef(options.substr(lastPos, pos - lastPos));
            }
            else if (options.compare(lastPos, 20, "cl-fast-relaxed-math") == 0) {
                gceSTATUS status;
                pos = lastPos + 20;

                status = cloCOMPILER_SetFpConfig(Compiler,
                                                 cldFpFAST_RELAXED_MATH);
                if(gcmIS_ERROR(status)) return status;
                Preprocessoropts.addMacroDef("__FAST_RELAXED_MATH__");
            }
            else if (options.compare(lastPos, 19, "cl-viv-vx-extension") == 0) {
                gceSTATUS status;
                char* env = gcvNULL;

                pos = lastPos + 19;

                status = cloCOMPILER_EnableExtension(Compiler,
                                                     clvEXTENSION_VIV_VX,
                                                     gcvTRUE);
                if(gcmIS_ERROR(status)) return status;
                Preprocessoropts.addMacroDef("_VIV_VX_EXTENSION");
                gcoOS_GetEnv(gcvNULL, "VIVANTE_SDK_DIR", &env);
                if(env) {
                    gctUINT len;
                    char path[_cldFILENAME_MAX];

                    static const char subfix[] = "/include/CL/";
                    len = gcoOS_StrLen(env, gcvNULL);

                    gcoOS_StrCopySafe(path, len + 1, env);
                    gcoOS_StrCatSafe(path, _cldFILENAME_MAX, subfix);

                    std::string normalizedPath(path);
                    std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
                    Headersearchopts.AddPath(normalizedPath, frontend::Angled, true, false, true);

                    static const char subfix1[] = "/inc/CL/";
                    len = gcoOS_StrLen(env, gcvNULL);

                    gcoOS_StrCopySafe(path, len + 1, env);
                    gcoOS_StrCatSafe(path, _cldFILENAME_MAX, subfix1);

                    std::string normalizedPath1(path);
                    std::replace(normalizedPath1.begin(), normalizedPath1.end(), '\\', '/');
                    Headersearchopts.AddPath(normalizedPath1, frontend::Angled, true, false, true);
                }
            }
            else if (options.compare(lastPos, 24, "cl-viv-packed-basic-type") == 0) {
                gceSTATUS status;
                pos = lastPos + 24;

                status = cloCOMPILER_SetBasicTypePacked(Compiler);
                if(gcmIS_ERROR(status)) return status;
            }
            else if (options.compare(lastPos, 31, "cl-viv-vx-image-array-maxlevel=") == 0) {
                lastPos += 31;

                if(options.compare(lastPos, 1, " ") == 0) { /* there are spaces in between */
                   lastPos = options.find_first_not_of(" ", lastPos + 1);
                }

                pos = options.find_first_of(" ", lastPos);
                status = cloCOMPILER_SetImageArrayMaxLevel(Compiler,
                                                           (gctSTRING)options.substr(lastPos, pos - lastPos).c_str());
                if(gcmIS_ERROR(status)) {
                    cloCOMPILER_Report(Compiler,
                                       0,
                                       0,
                                       clvREPORT_ERROR,
                                       "unrecognized image array max level \"%s\" specified in option cl-viv-vx-image-array-maxlevel",
                                       options.substr(lastPos, pos - lastPos).c_str());
                    return gcvSTATUS_INTERFACE_ERROR;
                }
            }
            else if (options.compare(lastPos, 19, "cl-finite-math-only") == 0) {
                gceSTATUS status;
                pos = lastPos + 19;

                status = cloCOMPILER_SetFpConfig(Compiler,
                                                 cldFpFINITE_MATH_ONLY);
                if(gcmIS_ERROR(status)) return status;
            }
            else if (options.compare(lastPos, 7, "cl-std=") == 0) {
                lastPos += 7;
                if(options.compare(lastPos, 1, " ") == 0) { /* there are spaces in between */
                   lastPos = options.find_first_not_of(" ", lastPos + 1);
                }
                pos = options.find_first_of(" ", lastPos);
                status = cloCOMPILER_SetLanguageVersion(Compiler,
                                                        (gctSTRING)options.substr(lastPos, pos - lastPos).c_str());
                if(gcmIS_ERROR(status)) {
                    cloCOMPILER_Report(Compiler,
                                       0,
                                       0,
                                       clvREPORT_ERROR,
                                       "unrecognized language version \"%s\" specified in option cl-std",
                                       options.substr(lastPos, pos - lastPos).c_str());
                    return gcvSTATUS_INTERFACE_ERROR;
                }
            }
            else {
                /* skip all other options */
                pos = options.find_first_of(" ", lastPos + 1);
            }
            lastPos = options.find_first_not_of(" ", pos);
        }
        else {
            cloCOMPILER_Report(Compiler,
                               0,
                               0,
                               clvREPORT_ERROR,
                               "syntax error in compiler option string \"%s\"",
                               options.c_str());
            return gcvSTATUS_INTERFACE_ERROR;
       }
    }
  }

  TargetOptions& Targetopts = Invocation.getTargetOpts();
  // Use the host triple as default for now
  Targetopts.Triple = llvm::sys::getHostTriple();

  // Create the actual diagnostics engine.
  DiagnosticOptions& Diagnosticopts = Invocation.getDiagnosticOpts();
  llvm::IntrusiveRefCntPtr<Diagnostic> Diags(new Diagnostic());
  // Create the diagnostic client for reporting errors or for
  // implementing -verify.
  llvm::OwningPtr<DiagnosticClient> DiagClient;
  Diags->setClient(new TextDiagnosticPrinter(llvm::errs(), Diagnosticopts));
  // Configure our handling of diagnostics.
  ProcessWarningOptions(*Diags, Diagnosticopts);
  Clang->setDiagnostics(Diags);
  if (!Clang->hasDiagnostics()) return gcvSTATUS_INTERFACE_ERROR;

  TextDiagnosticBuffer *DiagsBuffer = new TextDiagnosticBuffer;
  DiagsBuffer->FlushDiagnostics(Clang->getDiagnostics());
  delete DiagsBuffer;

  // If there were errors in processing arguments, don't do anything else.
  bool Success = false;
  if (!Clang->getDiagnostics().getNumErrors()) {
    // Create the preprocessor frontend action.
    bool Temp_success;
    unsigned pped = 0;
    char *NoPreprocess = gcvNULL;
    gcoOS_GetEnv(gcvNULL, "CL_NOPREPROCESS", &NoPreprocess);

    llvm::OwningPtr<FrontendAction> Act(new PrintPreprocessedAction());
    Success = true;
    gctFILE fptr = NULL;

    for(unsigned int i=0; i < count; i++) {
      if(!NoPreprocess) {
        char tmpfile_name[_cldFILENAME_MAX];
        char name_suffix[16];
        char tmpoutputfile_name[_cldFILENAME_MAX + 16];
        status = GetTemporaryFilename(Compiler, count, tmpdir, (char *)"", tmpfile_name, _cldFILENAME_MAX);
        if (gcmIS_ERROR(status)) {
           cloCOMPILER_Report(Compiler,
                              0,
                              i,
                              clvREPORT_ERROR,
                              "Failed to create a temporary file name %s",
                              tmpfile_name);
           break;
        }
#ifdef _DEBUG
        cloCOMPILER_Report(Compiler,
                           0,
                           i,
                           clvREPORT_INFO,
                           "GENERATED TMP FILE NAME %s",
                           tmpfile_name);
#endif
        status = gcoOS_Open(gcvNULL,
                            tmpfile_name,
                            gcvFILE_CREATE,
                            &fptr);
        if(gcmIS_ERROR(status)) {
           cloCOMPILER_Report(Compiler,
                              0,
                              i,
                              clvREPORT_ERROR,
                              "Failed to open the temporary file %s for writing",
                              tmpfile_name);
           break;
        }

        status = gcoOS_Write(gcvNULL,
                             fptr,
                             gcoOS_StrLen(source[i], gcvNULL),
                             source[i]);
        gcoOS_Close(gcvNULL, fptr);
        if(gcmIS_ERROR(status)) {
           cloCOMPILER_Report(Compiler,
                              0,
                              i,
                              clvREPORT_ERROR,
                              "Failed to write to temporary file %s",
                              tmpfile_name);
           break;
        }

        Frontendopts.Inputs.clear();
        Frontendopts.Inputs.push_back(std::make_pair(IK_OpenCL, &tmpfile_name[0]));
        gcmVERIFY_OK(gcoOS_PrintStrSafe(name_suffix, gcmSIZEOF(name_suffix), &offset, "_%d_PPED", i));
        status = GetTemporaryFilename(Compiler, count, tmpdir, name_suffix, tmpoutputfile_name, _cldFILENAME_MAX + 16);
        if (gcmIS_ERROR(status)) {
           cloCOMPILER_Report(Compiler,
                              0,
                              i,
                              clvREPORT_ERROR,
                              "Failed to create a temporary file name %s for the preprocessed input",
                              tmpoutputfile_name);
           break;
        }
        Frontendopts.OutputFile = tmpoutputfile_name;

        // Execute the frontend actions.
        Temp_success = Clang->ExecuteAction(*Act);
        if(Temp_success == false) Success = false;
        if (Clang->getFrontendOpts().DisableFree)
           Act.take();

        {
            IdentifierInfo *id;
            MacroInfo *macroInfo;

            id = Clang->getPreprocessor().getIdentifierInfo("CL_VIV_asm");
            macroInfo = Clang->getPreprocessor().getMacroInfo(id);
            if(macroInfo != gcvNULL && macroInfo->isEnabled())
            {
                cloCOMPILER_EnableExtension(Compiler,
                                            clvEXTENSION_VASM,
                                            gcvTRUE);
            }

            id = Clang->getPreprocessor().getIdentifierInfo("cl_viv_bitfield_extension");
            macroInfo = Clang->getPreprocessor().getMacroInfo(id);
            if(macroInfo != gcvNULL && macroInfo->isEnabled())
            {
                cloCOMPILER_EnableExtension(Compiler,
                                            clvEXTENSION_VIV_BITFIELD,
                                            gcvTRUE);
            }

            id = Clang->getPreprocessor().getIdentifierInfo("cl_viv_cmplx_extension");
            macroInfo = Clang->getPreprocessor().getMacroInfo(id);
            if(macroInfo != gcvNULL && macroInfo->isEnabled())
            {
                cloCOMPILER_EnableExtension(Compiler,
                                            clvEXTENSION_VIV_CMPLX,
                                            gcvTRUE);
            }
        }

        llvm::sys::Path(tmpfile_name).eraseFromDisk();
        source_pped[pped] = _LoadFileToBuffer(Compiler, count, tmpoutputfile_name);
//keep the temporary preprocessed file for debugging
#ifndef _DEBUG
        llvm::sys::Path(tmpoutputfile_name).eraseFromDisk();
#endif
        if(Temp_success == false || source_pped[pped] == NULL) {
          if(source_pped[pped] == NULL) {
#ifndef _DEBUG
            gcmONERROR(cloCOMPILER_Report(Compiler,
                                          0,
                                          i,
                                          clvREPORT_ERROR,
                                           "Failed to load the temporary preprocessed file %s to source buffer",
                                          tmpoutputfile_name));

#endif
          }
          gcmONERROR(cloCOMPILER_Report(Compiler,
                                          0,
                                          i,
                                          clvREPORT_ERROR,
                                          "Failed to preprocess the source string #%d",
                                          i));
          Success = false;
          continue;
        }
      }
      else {
        source_pped[pped] = (char *)source[i];
      }
      pped++;
    }
OnError:
    *pped_count = pped;
  }

  if(Success == false) {
    status = gcvSTATUS_INVALID_DATA;
  }

  // Our error handler depends on the Diagnostics object, which we're
  // potentially about to delete. Uninstall the handler now so that any
  // later errors use the default handling behavior instead.
  llvm::remove_fatal_error_handler();

  if(fileNameBuffer) {
     gcmVERIFY_OK(cloCOMPILER_Free(Compiler, fileNameBuffer));
  }
  // When running with -disable-free, don't do any destruction or shutdown.
  if (Clang->getFrontendOpts().DisableFree) {
    if (Clang->getFrontendOpts().ShowStats)
      llvm::PrintStatistics();
    Clang.take();
    return status;
  }
  // Managed static deconstruction. Useful for making things like
  // -time-passes usable.
  llvm::llvm_shutdown();
  return status;
}

gceSTATUS
Clang_Preprocess(cloCOMPILER Compiler,
const char **source,
unsigned count,
const char *Options,
char **source_pped,
unsigned *pped_count)
{
  return Clang_Preprocess_Hardcoded(Compiler,
                                    source,
                                    count,
                                    Options,
                                    source_pped,
                                    pped_count);
}
