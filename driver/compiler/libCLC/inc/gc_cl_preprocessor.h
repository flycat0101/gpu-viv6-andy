/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_cl_preprocessor_h_
#define __gc_cl_preprocessor_h_

#include "gc_cl_compiler.h"

typedef struct    _ppoPREPROCESSOR*    cloPREPROCESSOR;

gceSTATUS
cloPREPROCESSOR_Construct(
    IN cloCOMPILER Compiler,
    OUT cloPREPROCESSOR * PP
    );

gceSTATUS
cloPREPROCESSOR_Destroy(
    IN cloCOMPILER Compiler,
    IN cloPREPROCESSOR PP
    );

gceSTATUS
cloPREPROCESSOR_SetSourceStrings(
    IN cloPREPROCESSOR PP,
    IN gctUINT StringCount,
    IN gctCONST_STRING Strings[]
);

gceSTATUS
cloPREPROCESSOR_Parse(
    IN cloPREPROCESSOR PP,
    IN gctUINT StringCount,
    IN gctCONST_STRING Strings[],
    IN gctCONST_STRING Options
    );

gceSTATUS
cloPREPROCESSOR_GetPPedInfo(IN cloPREPROCESSOR PP,
 OUT gctCONST_STRING **Strings,
 OUT gctUINT *StringCount);

#define ppvMAX_MACRO_ARGS_NUMBER            64
#define    ppvMAX_PPTOKEN_CHAR_NUMBER            1000
#define ppvCHAR_EOF                            (char) (unsigned char) 0xFF

#if ppvMAX_PPTOKEN_CHAR_NUMBER < 8
#error    Please set ppvMAX_PPTOKEN_CHAR_NUMBER >= 8.
#endif

#endif /* __gc_cl_preprocessor_h_ */
