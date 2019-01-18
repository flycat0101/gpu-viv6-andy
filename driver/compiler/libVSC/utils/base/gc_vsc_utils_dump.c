/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
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
    gctARGUMENTS arguments;
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT   offset;

    /* Verify the arguments. */
    gcmASSERT(pDumper->pBuffer != gcvNULL);
    gcmASSERT(pDumper->pOffset != gcvNULL);
    gcmASSERT(pFormat != gcvNULL);

    /* Get the pointer to the variable arguments. */
    /* Route through gcoOS_PrintStrVSafe. */
    gcmARGUMENTS_START(arguments, pFormat);
    offset = (gctUINT)*pDumper->pOffset;

    status = gcoOS_PrintStrVSafe(pDumper->pBuffer, pDumper->bufferSize,
                                   &offset,
                                   pFormat, arguments);
    if (status < 0)
    {
        gcoOS_Print("Warning: Print status is not OK !!\n");

        gcoOS_ZeroMemory(pDumper->pBuffer, sizeof(pDumper->pBuffer));
        *pDumper->pOffset = 0;
        goto OnError;
    }
    else
        *pDumper->pOffset = (gctSIZE_T)offset;

OnError:
    /* Delete the pointer to the variable arguments. */
    gcmARGUMENTS_END(arguments);

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

