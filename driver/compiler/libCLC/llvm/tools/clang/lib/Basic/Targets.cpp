//===--- Targets.cpp - Implement -arch option and targets -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements construction of a TargetInfo object from a
// target triple.
//
//===----------------------------------------------------------------------===//

#include "llvm/Config/config.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/Builtins.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/MacroBuilder.h"
#include "clang/Basic/TargetBuiltins.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Triple.h"
////KLC COMMENT OUT
#include <algorithm>
using namespace clang;

//===----------------------------------------------------------------------===//
//  Common code shared among targets.
//===----------------------------------------------------------------------===//

/// DefineStd - Define a macro name and standard variants.  For example if
/// MacroName is "unix", then this will define "__unix", "__unix__", and "unix"
/// when in GNU mode.
static void DefineStd(MacroBuilder &Builder, llvm::StringRef MacroName,
                      const LangOptions &Opts) {
  assert(MacroName[0] != '_' && "Identifier should be in the user's namespace");

  // If in GNU mode (e.g. -std=gnu99 but not -std=c99) define the raw identifier
  // in the user's namespace.
  if (Opts.GNUMode)
    Builder.defineMacro(MacroName);

  // Define __unix.
  Builder.defineMacro("__" + MacroName);

  // Define __unix__.
  Builder.defineMacro("__" + MacroName + "__");
}

////KLC COMMENT OUT MOST FOR NOW

namespace {
// Namespace for x86 abstract base class
const Builtin::Info BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS) { #ID, TYPE, ATTRS, 0, false },
#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER) { #ID, TYPE, ATTRS, HEADER, false },
#include "clang/Basic/BuiltinsX86.def"
};

static const char* const GCCRegNames[] = {
  "ax", "dx", "cx", "bx", "si", "di", "bp", "sp",
  "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)",
  "argp", "flags", "fspr", "dirflag", "frame",
  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
  "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7",
  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
  "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
};

const TargetInfo::GCCRegAlias GCCRegAliases[] = {
  { { "al", "ah", "eax", "rax" }, "ax" },
  { { "bl", "bh", "ebx", "rbx" }, "bx" },
  { { "cl", "ch", "ecx", "rcx" }, "cx" },
  { { "dl", "dh", "edx", "rdx" }, "dx" },
  { { "esi", "rsi" }, "si" },
  { { "edi", "rdi" }, "di" },
  { { "esp", "rsp" }, "sp" },
  { { "ebp", "rbp" }, "bp" },
};

// X86 target abstract base class; x86-32 and x86-64 are very close, so
// most of the implementation can be shared.
class X86TargetInfo : public TargetInfo {
  enum X86SSEEnum {
    NoMMXSSE, MMX, SSE1, SSE2, SSE3, SSSE3, SSE41, SSE42
  } SSELevel;
  enum AMD3DNowEnum {
    NoAMD3DNow, AMD3DNow, AMD3DNowAthlon
  } AMD3DNowLevel;

  bool HasAES;
  bool HasAVX;

public:
  X86TargetInfo(const std::string& triple)
    : TargetInfo(triple), SSELevel(NoMMXSSE), AMD3DNowLevel(NoAMD3DNow),
      HasAES(false), HasAVX(false) {
    LongDoubleFormat = &llvm::APFloat::x87DoubleExtended;
  }
  virtual void getTargetBuiltins(const Builtin::Info *&Records,
                                 unsigned &NumRecords) const {
    Records = BuiltinInfo;
    NumRecords = clang::X86::LastTSBuiltin-Builtin::FirstTSBuiltin;
  }
  virtual void getGCCRegNames(const char * const *&Names,
                              unsigned &NumNames) const {
    Names = GCCRegNames;
    NumNames = llvm::array_lengthof(GCCRegNames);
  }
  virtual void getGCCRegAliases(const GCCRegAlias *&Aliases,
                                unsigned &NumAliases) const {
    Aliases = GCCRegAliases;
    NumAliases = llvm::array_lengthof(GCCRegAliases);
  }
  virtual bool validateAsmConstraint(const char *&Name,
                                     TargetInfo::ConstraintInfo &info) const;
  virtual std::string convertConstraint(const char Constraint) const;
  virtual const char *getClobbers() const {
    return "~{dirflag},~{fpsr},~{flags}";
  }
  virtual void getTargetDefines(const LangOptions &Opts,
                                MacroBuilder &Builder) const;
  virtual bool setFeatureEnabled(llvm::StringMap<bool> &Features,
                                 const std::string &Name,
                                 bool Enabled) const;
  virtual void getDefaultFeatures(const std::string &CPU,
                                  llvm::StringMap<bool> &Features) const;
  virtual void HandleTargetFeatures(std::vector<std::string> &Features);
};

void X86TargetInfo::getDefaultFeatures(const std::string &CPU,
                                       llvm::StringMap<bool> &Features) const {
  // FIXME: This should not be here.
  Features["3dnow"] = false;
  Features["3dnowa"] = false;
  Features["mmx"] = false;
  Features["sse"] = false;
  Features["sse2"] = false;
  Features["sse3"] = false;
  Features["ssse3"] = false;
  Features["sse41"] = false;
  Features["sse42"] = false;
  Features["aes"] = false;
  Features["avx"] = false;

  // LLVM does not currently recognize this.
  // Features["sse4a"] = false;

  // FIXME: This *really* should not be here.

  // X86_64 always has SSE2.
  if (PointerWidth == 64)
    Features["sse2"] = Features["sse"] = Features["mmx"] = true;

  if (CPU == "generic" || CPU == "i386" || CPU == "i486" || CPU == "i586" ||
      CPU == "pentium" || CPU == "i686" || CPU == "pentiumpro")
    ;
  else if (CPU == "pentium-mmx" || CPU == "pentium2")
    setFeatureEnabled(Features, "mmx", true);
  else if (CPU == "pentium3")
    setFeatureEnabled(Features, "sse", true);
  else if (CPU == "pentium-m" || CPU == "pentium4" || CPU == "x86-64")
    setFeatureEnabled(Features, "sse2", true);
  else if (CPU == "yonah" || CPU == "prescott" || CPU == "nocona")
    setFeatureEnabled(Features, "sse3", true);
  else if (CPU == "core2")
    setFeatureEnabled(Features, "ssse3", true);
  else if (CPU == "penryn") {
    setFeatureEnabled(Features, "sse4", true);
    Features["sse42"] = false;
  } else if (CPU == "atom")
    setFeatureEnabled(Features, "sse3", true);
  else if (CPU == "corei7") {
    setFeatureEnabled(Features, "sse4", true);
    setFeatureEnabled(Features, "aes", true);
  }
  else if (CPU == "k6" || CPU == "winchip-c6")
    setFeatureEnabled(Features, "mmx", true);
  else if (CPU == "k6-2" || CPU == "k6-3" || CPU == "athlon" ||
           CPU == "athlon-tbird" || CPU == "winchip2" || CPU == "c3") {
    setFeatureEnabled(Features, "mmx", true);
    setFeatureEnabled(Features, "3dnow", true);
  } else if (CPU == "athlon-4" || CPU == "athlon-xp" || CPU == "athlon-mp") {
    setFeatureEnabled(Features, "sse", true);
    setFeatureEnabled(Features, "3dnowa", true);
  } else if (CPU == "k8" || CPU == "opteron" || CPU == "athlon64" ||
           CPU == "athlon-fx") {
    setFeatureEnabled(Features, "sse2", true);
    setFeatureEnabled(Features, "3dnowa", true);
  } else if (CPU == "c3-2")
    setFeatureEnabled(Features, "sse", true);
}

bool X86TargetInfo::setFeatureEnabled(llvm::StringMap<bool> &Features,
                                      const std::string &Name,
                                      bool Enabled) const {
  // FIXME: This *really* should not be here.  We need some way of translating
  // options into llvm subtarget features.
  if (!Features.count(Name) &&
      (Name != "sse4" && Name != "sse4.2" && Name != "sse4.1"))
    return false;

  if (Enabled) {
    if (Name == "mmx")
      Features["mmx"] = true;
    else if (Name == "sse")
      Features["mmx"] = Features["sse"] = true;
    else if (Name == "sse2")
      Features["mmx"] = Features["sse"] = Features["sse2"] = true;
    else if (Name == "sse3")
      Features["mmx"] = Features["sse"] = Features["sse2"] =
        Features["sse3"] = true;
    else if (Name == "ssse3")
      Features["mmx"] = Features["sse"] = Features["sse2"] = Features["sse3"] =
        Features["ssse3"] = true;
    else if (Name == "sse4" || Name == "sse4.2")
      Features["mmx"] = Features["sse"] = Features["sse2"] = Features["sse3"] =
        Features["ssse3"] = Features["sse41"] = Features["sse42"] = true;
    else if (Name == "sse4.1")
      Features["mmx"] = Features["sse"] = Features["sse2"] = Features["sse3"] =
        Features["ssse3"] = Features["sse41"] = true;
    else if (Name == "3dnow")
      Features["3dnowa"] = true;
    else if (Name == "3dnowa")
      Features["3dnow"] = Features["3dnowa"] = true;
    else if (Name == "aes")
      Features["aes"] = true;
    else if (Name == "avx")
      Features["avx"] = true;
  } else {
    if (Name == "mmx")
      Features["mmx"] = Features["sse"] = Features["sse2"] = Features["sse3"] =
        Features["ssse3"] = Features["sse41"] = Features["sse42"] = false;
    else if (Name == "sse")
      Features["sse"] = Features["sse2"] = Features["sse3"] =
        Features["ssse3"] = Features["sse41"] = Features["sse42"] = false;
    else if (Name == "sse2")
      Features["sse2"] = Features["sse3"] = Features["ssse3"] =
        Features["sse41"] = Features["sse42"] = false;
    else if (Name == "sse3")
      Features["sse3"] = Features["ssse3"] = Features["sse41"] =
        Features["sse42"] = false;
    else if (Name == "ssse3")
      Features["ssse3"] = Features["sse41"] = Features["sse42"] = false;
    else if (Name == "sse4")
      Features["sse41"] = Features["sse42"] = false;
    else if (Name == "sse4.2")
      Features["sse42"] = false;
    else if (Name == "sse4.1")
      Features["sse41"] = Features["sse42"] = false;
    else if (Name == "3dnow")
      Features["3dnow"] = Features["3dnowa"] = false;
    else if (Name == "3dnowa")
      Features["3dnowa"] = false;
    else if (Name == "aes")
      Features["aes"] = false;
    else if (Name == "avx")
      Features["avx"] = false;
  }

  return true;
}

/// HandleTargetOptions - Perform initialization based on the user
/// configured set of features.
void X86TargetInfo::HandleTargetFeatures(std::vector<std::string> &Features) {
  // Remember the maximum enabled sselevel.
  for (unsigned i = 0, e = Features.size(); i !=e; ++i) {
    // Ignore disabled features.
    if (Features[i][0] == '-')
      continue;

    if (Features[i].substr(1) == "aes") {
      HasAES = true;
      continue;
    }

    // FIXME: Not sure yet how to treat AVX in regard to SSE levels.
    // For now let it be enabled together with other SSE levels.
    if (Features[i].substr(1) == "avx") {
      HasAVX = true;
      continue;
    }

    assert(Features[i][0] == '+' && "Invalid target feature!");
    X86SSEEnum Level = llvm::StringSwitch<X86SSEEnum>(Features[i].substr(1))
      .Case("sse42", SSE42)
      .Case("sse41", SSE41)
      .Case("ssse3", SSSE3)
      .Case("sse3", SSE3)
      .Case("sse2", SSE2)
      .Case("sse", SSE1)
      .Case("mmx", MMX)
      .Default(NoMMXSSE);
    SSELevel = std::max(SSELevel, Level);

    AMD3DNowEnum ThreeDNowLevel =
      llvm::StringSwitch<AMD3DNowEnum>(Features[i].substr(1))
        .Case("3dnowa", AMD3DNowAthlon)
        .Case("3dnow", AMD3DNow)
        .Default(NoAMD3DNow);

    AMD3DNowLevel = std::max(AMD3DNowLevel, ThreeDNowLevel);
  }
}

/// X86TargetInfo::getTargetDefines - Return a set of the X86-specific #defines
/// that are not tied to a specific subtarget.
void X86TargetInfo::getTargetDefines(const LangOptions &Opts,
                                     MacroBuilder &Builder) const {
  // Target identification.
  if (PointerWidth == 64) {
    Builder.defineMacro("_LP64");
    Builder.defineMacro("__LP64__");
    Builder.defineMacro("__amd64__");
    Builder.defineMacro("__amd64");
    Builder.defineMacro("__x86_64");
    Builder.defineMacro("__x86_64__");
  } else {
    DefineStd(Builder, "i386", Opts);
  }

  if (HasAES)
    Builder.defineMacro("__AES__");

  if (HasAVX)
    Builder.defineMacro("__AVX__");

  // Target properties.
  Builder.defineMacro("__LITTLE_ENDIAN__");

  // Subtarget options.
  Builder.defineMacro("__nocona");
  Builder.defineMacro("__nocona__");
  Builder.defineMacro("__tune_nocona__");
  Builder.defineMacro("__REGISTER_PREFIX__", "");

  // Define __NO_MATH_INLINES on linux/x86 so that we don't get inline
  // functions in glibc header files that use FP Stack inline asm which the
  // backend can't deal with (PR879).
  Builder.defineMacro("__NO_MATH_INLINES");

  // Each case falls through to the previous one here.
  switch (SSELevel) {
  case SSE42:
    Builder.defineMacro("__SSE4_2__");
  case SSE41:
    Builder.defineMacro("__SSE4_1__");
  case SSSE3:
    Builder.defineMacro("__SSSE3__");
  case SSE3:
    Builder.defineMacro("__SSE3__");
  case SSE2:
    Builder.defineMacro("__SSE2__");
    Builder.defineMacro("__SSE2_MATH__");  // -mfp-math=sse always implied.
  case SSE1:
    Builder.defineMacro("__SSE__");
    Builder.defineMacro("__SSE_MATH__");   // -mfp-math=sse always implied.
  case MMX:
    Builder.defineMacro("__MMX__");
  case NoMMXSSE:
    break;
  }

  // Each case falls through to the previous one here.
  switch (AMD3DNowLevel) {
  case AMD3DNowAthlon:
    Builder.defineMacro("__3dNOW_A__");
  case AMD3DNow:
    Builder.defineMacro("__3dNOW__");
  case NoAMD3DNow:
    break;
  }
}

bool
X86TargetInfo::validateAsmConstraint(const char *&Name,
                                     TargetInfo::ConstraintInfo &Info) const {
  switch (*Name) {
  default: return false;
  case 'Y': // first letter of a pair:
    switch (*(Name+1)) {
    default: return false;
    case '0':  // First SSE register.
    case 't':  // Any SSE register, when SSE2 is enabled.
    case 'i':  // Any SSE register, when SSE2 and inter-unit moves enabled.
    case 'm':  // any MMX register, when inter-unit moves enabled.
      break;   // falls through to setAllowsRegister.
  }
  case 'a': // eax.
  case 'b': // ebx.
  case 'c': // ecx.
  case 'd': // edx.
  case 'S': // esi.
  case 'D': // edi.
  case 'A': // edx:eax.
  case 'f': // any x87 floating point stack register.
  case 't': // top of floating point stack.
  case 'u': // second from top of floating point stack.
  case 'q': // Any register accessible as [r]l: a, b, c, and d.
  case 'y': // Any MMX register.
  case 'x': // Any SSE register.
  case 'Q': // Any register accessible as [r]h: a, b, c, and d.
  case 'R': // "Legacy" registers: ax, bx, cx, dx, di, si, sp, bp.
  case 'l': // "Index" registers: any general register that can be used as an
            // index in a base+index memory access.
    Info.setAllowsRegister();
    return true;
  case 'C': // SSE floating point constant.
  case 'G': // x87 floating point constant.
  case 'e': // 32-bit signed integer constant for use with zero-extending
            // x86_64 instructions.
  case 'Z': // 32-bit unsigned integer constant for use with zero-extending
            // x86_64 instructions.
    return true;
  }
  return false;
}

std::string
X86TargetInfo::convertConstraint(const char Constraint) const {
  switch (Constraint) {
  case 'a': return std::string("{ax}");
  case 'b': return std::string("{bx}");
  case 'c': return std::string("{cx}");
  case 'd': return std::string("{dx}");
  case 'S': return std::string("{si}");
  case 'D': return std::string("{di}");
  case 't': // top of floating point stack.
    return std::string("{st}");
  case 'u': // second from top of floating point stack.
    return std::string("{st(1)}"); // second from top of floating point stack.
  default:
    return std::string(1, Constraint);
  }
}
} // end anonymous namespace


namespace {
// X86-32 generic target
class X86_32TargetInfo : public X86TargetInfo { public: X86_32TargetInfo(const std::string& triple) : X86TargetInfo(triple) {
    DoubleAlign = LongLongAlign = 32;
    LongDoubleWidth = 96;
    LongDoubleAlign = 32;
    DescriptionString = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
                        "i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-"
                        "a0:0:64-f80:32:32-n8:16:32";
    SizeType = UnsignedInt;
    PtrDiffType = SignedInt;
    IntPtrType = SignedInt;
    RegParmMax = 3;

    // Use fpret for all types.
    RealTypeUsesObjCFPRet = ((1 << TargetInfo::Float) |
                             (1 << TargetInfo::Double) |
                             (1 << TargetInfo::LongDouble));
  }
  virtual const char *getVAListDeclaration() const {
    return "typedef char* __builtin_va_list;";
  }

  int getEHDataRegisterNumber(unsigned RegNo) const {
    if (RegNo == 0) return 0;
    if (RegNo == 1) return 2;
    return -1;
  }
};
} // end anonymous namespace

///KLC COMMENT OUT

////KLC HARDCODING THE TARGET FOR NOW
static TargetInfo *AllocateTarget(const std::string &T) {
      return new X86_32TargetInfo(T);
}

/// CreateTargetInfo - Return the target info object for the specified target
/// triple.
TargetInfo *TargetInfo::CreateTargetInfo(Diagnostic &Diags,
                                         TargetOptions &Opts) {
  llvm::Triple Triple(Opts.Triple);

  // Construct the target
  llvm::OwningPtr<TargetInfo> Target(AllocateTarget(Triple.str()));
  if (!Target) {
    Diags.Report(diag::err_target_unknown_triple) << Triple.str();
    return 0;
  }

  // Set the target CPU if specified.
  if (!Opts.CPU.empty() && !Target->setCPU(Opts.CPU)) {
    Diags.Report(diag::err_target_unknown_cpu) << Opts.CPU;
    return 0;
  }

  // Set the target ABI if specified.
  if (!Opts.ABI.empty() && !Target->setABI(Opts.ABI)) {
    Diags.Report(diag::err_target_unknown_abi) << Opts.ABI;
    return 0;
  }

  // Set the target C++ ABI.
  if (!Opts.CXXABI.empty() && !Target->setCXXABI(Opts.CXXABI)) {
    Diags.Report(diag::err_target_unknown_cxxabi) << Opts.CXXABI;
    return 0;
  }

  // Compute the default target features, we need the target to handle this
  // because features may have dependencies on one another.
  llvm::StringMap<bool> Features;
  Target->getDefaultFeatures(Opts.CPU, Features);

  // Apply the user specified deltas.
  for (std::vector<std::string>::const_iterator it = Opts.Features.begin(),
         ie = Opts.Features.end(); it != ie; ++it) {
    const char *Name = it->c_str();

    // Apply the feature via the target.
    if ((Name[0] != '-' && Name[0] != '+') ||
        !Target->setFeatureEnabled(Features, Name + 1, (Name[0] == '+'))) {
      Diags.Report(diag::err_target_invalid_feature) << Name;
      return 0;
    }
  }

  // Add the features to the compile options.
  //
  // FIXME: If we are completely confident that we have the right set, we only
  // need to pass the minuses.
  Opts.Features.clear();
  for (llvm::StringMap<bool>::const_iterator it = Features.begin(),
         ie = Features.end(); it != ie; ++it)
    Opts.Features.push_back(std::string(it->second ? "+" : "-") + it->first());
  Target->HandleTargetFeatures(Opts.Features);

  return Target.take();
}
