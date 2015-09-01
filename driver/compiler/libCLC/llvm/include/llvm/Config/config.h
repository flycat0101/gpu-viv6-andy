#if defined(_MSC_VER)
#if !defined(UNDER_CE)
#   include "config_Win32.h"
#else
#   include "config_WinCE.h"
#endif
#elif defined(__GNUC__)
#   include "config_Unix.h"
#else
# error "Unsupported OS type."
#endif

#include "gc_hal_base.h"
