/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vsc_h_
#define __gc_vsc_h_

/**********************************
         VSC common header
 *********************************/

/* Use empty header & footer for compiler BE */
#ifndef gcdEMPTY_HEADER_FOOTER
#define gcdEMPTY_HEADER_FOOTER 1
#endif

#if (_WINDOWS) || (_WIN32) || (WIN32)
/* C4389 */
#pragma warning(error : 4389 4189)
/* warning C4820: 'NNN' : '4' bytes padding added after data member 'xxx' */
/* warning C4668: 'LINUX' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif' */
/* warning C4255: 'GetTexLoadCycles' : no function prototype given: converting '()' to '(void)' */
#pragma warning(disable : 4820 4668 4255)
#endif

/* A temp control to make vir path implementation not break build for wince since we
   will develop vir path on windows PC. It will be removed when vir path is ready */
#define VSC_BUILD  ((LINUX) || (ANDROID) || (_WINDOWS) || (_WIN32) || (WIN32) || (__QNXNTO__) || defined(__APPLE__))

/* HAL headers */
#include "gc_hal.h"
#include "gc_hal_user.h"
#include "gc_hal_mem.h"
#include "gc_hal_user_hardware.h"

/* HW ISA arch definition */

/* Driver interface */
#include "gc_vsc_drvi_interface.h"

/* Old-implementation (gcSL) headers */
#include "old_impl/gc_vsc_old_compiler_internal.h"
#include "old_impl/gc_vsc_old_optimizer.h"

/*
 *  All new VIR related headers are put below
 */

#if VSC_BUILD

/* C-lib headers */
#include <stdio.h>
#include <stdlib.h>

/* Error check */
#include "utils/gc_vsc_err.h"

/* General dumper */
#include "utils/gc_vsc_dump.h"

/* Global options, not pass controls */
#include "vir/passmanager/gc_vsc_options.h"

/* Utils */
#include "utils/gc_vsc_utils_base.h"
#include "utils/gc_vsc_utils_math.h"
#include "utils/gc_vsc_utils_string.h"
#include "utils/gc_vsc_utils_list.h"
#include "utils/gc_vsc_utils_queuestack.h"
#include "utils/gc_vsc_utils_mm.h"
#include "utils/gc_vsc_utils_bv.h"
#include "utils/gc_vsc_utils_array.h"
#include "utils/gc_vsc_utils_hash.h"
#include "utils/gc_vsc_utils_table.h"
#include "utils/gc_vsc_utils_tree.h"
#include "utils/gc_vsc_utils_graph.h"

/* VIR language */
#include "vir/ir/gc_vsc_vir_symbol_table.h"
#include "vir/ir/gc_vsc_vir_ir.h"

/* VIR analysis */
#include "vir/analysis/gc_vsc_vir_cfa.h"
#include "vir/analysis/gc_vsc_vir_dfa.h"

/* VIR dump */
#include "vir/ir/gc_vsc_vir_dump.h"

/* VIR pass manager */
#include "vir/passmanager/gc_vsc_vir_pass_mnger.h"

#endif

#endif /* __gc_vsc_h_ */

