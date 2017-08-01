/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_vsc.h"

void vscDumper_Initialize(
    VSC_DUMPER* pDumper,
    gcoOS pOs,
    gctFILE pFile,
    gctSTRING pBuffer,
    gctSIZE_T bufferSize)
{
    pDumper->pOs         = pOs;
    pDumper->pOffset     = &pDumper->curOffset;
    pDumper->pFile       = pFile;
    pDumper->pBuffer     = pBuffer;
    pDumper->bufferSize  = bufferSize;
    pDumper->curOffset   = 0;
    pDumper->verbose     = gcmOPT_DUMP_OPTIMIZER_VERBOSE() ? gcvTRUE : gcvFALSE;
}

VSC_ErrCode
vscDumper_PrintStrSafe(
    IN OUT VSC_DUMPER*  pDumper,
    IN gctCONST_STRING  pFormat,
    ...
    )
{
    va_list arguments;

    /* Verify the arguments. */
    gcmASSERT(pDumper->pBuffer != gcvNULL);
    gcmASSERT(pDumper->pOffset != gcvNULL);
    gcmASSERT(pFormat != gcvNULL);

    /* Get the pointer to the variable arguments. */
    va_start(arguments, pFormat);

#if defined(_WINDOWS) || (defined(_WIN32) || defined(WIN32))
    /* Format the string. */
    *pDumper->pOffset += vsprintf_s(pDumper->pBuffer + *pDumper->pOffset,
                                    pDumper->bufferSize - *pDumper->pOffset,
                                    pFormat,
                                    arguments);
#else
#if defined(__i386__) || defined(__x86_64__) || !defined(__STRICT_ANSI__)
    if (*pDumper->pOffset < pDumper->bufferSize)
    {
        /* Format the string. */
        gctINT n = vsnprintf(pDumper->pBuffer + *pDumper->pOffset,
                             pDumper->bufferSize - *pDumper->pOffset,
                             pFormat,
                             arguments);

        if (n > 0)
        {
            *pDumper->pOffset += n;
        }
    }
#endif
#endif
    /* Delete the pointer to the variable arguments. */
    va_end(arguments);

    /* Success. */
    return VSC_ERR_NONE;
}

void
vscDumper_DumpBuffer(
    IN OUT VSC_DUMPER *pDumper
    )
{
    gcmASSERT(pDumper != gcvNULL);

    if (pDumper->pFile)
    {
        if (gcmIS_ERROR(gcoOS_Write(pDumper->pOs, pDumper->pFile,
                                    *pDumper->pOffset, pDumper->pBuffer)))
        {
            gcmTRACE(gcvLEVEL_ERROR, "gcoOS_Write fail.");
        }
    }
    else if(*pDumper->pOffset != 0)
    {
        /* Print string to debug terminal. */
        gcoOS_Print("%s", pDumper->pBuffer);
    }

    if (pDumper->pOffset != gcvNULL)
    {
        *pDumper->pOffset = 0;
    }

    if (pDumper->pFile != gcvNULL)
    {
        gcoOS_Flush(pDumper->pOs, pDumper->pFile);
    }
}

void
vscDumpMessage(
    IN gcoOS pOs,
    IN gctFILE pFile,
    IN gctSTRING pMessage
    )
{
    VSC_DUMPER dumper = {0};

    vscDumper_Initialize(&dumper, pOs, pFile, pMessage, gcoOS_StrLen(pMessage, gcvNULL));

    *dumper.pOffset = dumper.bufferSize;

    vscDumper_DumpBuffer(&dumper);
}

gctSTRING VSC_TRACE_BAR_LINE   = "========================================================";
gctSTRING VSC_TRACE_STAR_LINE  = "********************************************************";
gctSTRING VSC_TRACE_SHARP_LINE = "########################################################";

