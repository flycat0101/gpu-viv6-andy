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


#include "gc_cl_preprocessor_int.h"

/******************************************************************************\
|***** Version Signature ******************************************************|
\******************************************************************************/

#define _gcmTXT2STR(t) #t
#define gcmTXT2STR(t) _gcmTXT2STR(t)
const char * clcVersion = "\n\0$VERSION$"
                           gcmTXT2STR(gcvVERSION_MAJOR) "."
                           gcmTXT2STR(gcvVERSION_MINOR) "."
                           gcmTXT2STR(gcvVERSION_PATCH) ":"
                           gcmTXT2STR(gcvVERSION_BUILD)
                           "$\n";

gceSTATUS
cloPREPROCESSOR_Construct(
    IN    cloCOMPILER        Compiler,
    OUT cloPREPROCESSOR *Preprocessor
    )
{
    gceSTATUS status = gcvSTATUS_INVALID_DATA;

    /* Verify the arguments. */

    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    gcmASSERT(Preprocessor);

    status = ppoPREPROCESSOR_Construct(Compiler, (ppoPREPROCESSOR*)Preprocessor);

    return status;

}

gceSTATUS
cloPREPROCESSOR_Destroy(
    IN cloCOMPILER Compiler,
    IN cloPREPROCESSOR Preprocessor
    )
{
    gceSTATUS status = gcvSTATUS_INVALID_DATA;
    ppoPREPROCESSOR    PP = Preprocessor;

    /*gcmHEADER_ARG("Compiler=0x%x Preprocessor=0x%x", Compiler, Preprocessor);*/

    /* Verify the arguments. */

    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);

    status = ppoPREPROCESSOR_Destroy(PP);

    return status;
}

extern gceSTATUS Clang_Preprocess(cloCOMPILER Compiler, gctCONST_STRING* strings, gctUINT count, gctCONST_STRING Options,
                                  char ** ppedStrings, gctUINT* ppedCount);

gceSTATUS
cloPREPROCESSOR_Parse(
    IN cloPREPROCESSOR Preprocessor,
    IN gctUINT StringCount,
    IN gctCONST_STRING Strings[],
    IN gctCONST_STRING Options
    )
{
    gceSTATUS        status    = gcvSTATUS_INVALID_ARGUMENT;
    ppoPREPROCESSOR        PP= (ppoPREPROCESSOR)Preprocessor;

    /*gcmHEADER_ARG("Preprocessor=0x%x StringCount=%u Strings[]=0x%x",
                  Preprocessor, StringCount, Strings);*/

    /* Verify the arguments. */
    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);
    gcmASSERT(StringCount > 0);
    gcmASSERT(Strings);

    status = ppoPREPROCESSOR_SetSourceStrings(PP, Strings, StringCount);
        if (status == gcvSTATUS_OK) {
        status = Clang_Preprocess(PP->compiler, PP->strings, PP->count, Options, PP->ppedStrings, &PP->ppedCount);
        }
    return status;
}

/******************************************************************************* */

gceSTATUS
cloCOMPILER_SetDebug(
    cloCOMPILER Compiler,
    gctBOOL        Debug
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_SetOptimize(
    cloCOMPILER Compiler,
    gctBOOL        Optimize
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_SetVersion(
    cloCOMPILER Compiler,
    gctINT Line
    )
{
    return gcvSTATUS_OK;
}


gceSTATUS
cloPREPROCESSOR_GetPPedInfo(
    cloPREPROCESSOR Preprocessor,
    gctCONST_STRING **Strings,
    gctUINT *StringCount
    )
{
   ppoPREPROCESSOR    PP= (ppoPREPROCESSOR)Preprocessor;
   *Strings = (gctCONST_STRING *) PP->ppedStrings;
   *StringCount = PP->ppedCount;
   return gcvSTATUS_OK;
}
