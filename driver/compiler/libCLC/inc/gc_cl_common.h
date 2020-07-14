/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_cl_common_h_
#define __gc_cl_common_h_

#ifndef gcdEMPTY_HEADER_FOOTER
/* use empty header & footer for compiler FE and BE */
#define gcdEMPTY_HEADER_FOOTER 1
#endif

#include "gc_hal.h"
#include "gc_hal_user_debug.h"
#include "gc_hal_user_os_memory.h"
#include "gc_hal_driver.h"
#include "gc_hal_types.h"
#include "gc_hal_priv.h"
#include "drvi/gc_vsc_drvi_interface.h"

typedef char *        gctCHAR_PTR;
typedef const char *    gctCONST_CHAR_PTR;

#include "gc_cl_link_list.h"
#include "gc_cl_hash_table.h"

#define _GC_OBJ_ZONE    gcdZONE_COMPILER

gctINT
clFindLCM(
gctINT A,
gctINT B
);

gctBOOL
clIsPowerOf2(
gctINT IntValue
);

gctINT
clFindNearestPowerOf2(
gctINT IntValue
);

void
clQuickSort(
IN void * base,
IN gctSIZE_T num,
IN gctSIZE_T size,
IN int ( * comparator ) ( const void *, const void * )
);

void *
clBsearch(
IN void *key,
IN void *base,
IN gctSIZE_T num,
IN gctSIZE_T size,
IN int ( * comparator ) ( const void *, const void * )
);

gctUINT
clEvaluateCRC32ForShaderString(
    IN const void *pData,
    IN gctUINT32 dataSizeInByte
    );

#endif /* __gc_cl_common_h_ */
