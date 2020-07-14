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


#ifndef __gc_vsc_dump_h_
#define __gc_vsc_dump_h_

BEGIN_EXTERN_C()

#define INDENT_GAP      4
#define VSC_GET_DUMPER_FILE()   gcvNULL
typedef struct _VSC_DUMPER
{
    gcoOS           pOs;
    gctFILE         pFile;
    gctSTRING       pBuffer;
    gctSIZE_T       bufferSize;
    gctSIZE_T       curOffset;
    gctSIZE_T*      pOffset;
    gctBOOL         verbose;
}VSC_DUMPER;

void vscDumper_Initialize(
    VSC_DUMPER* pDumper,
    gcoOS pOs,
    gctFILE pFile,
    gctSTRING pBuffer,
    gctSIZE_T bufferSize);

VSC_ErrCode
vscDumper_PrintStrSafe(
    IN OUT VSC_DUMPER*  pDumper,
    IN gctCONST_STRING  pFormat,
    ...
    );

void
vscDumper_DumpBuffer(
    IN OUT VSC_DUMPER *pDumper
    );

void
vscDumpMessage(
    IN gcoOS pOs,
    IN gctFILE pFile,
    IN gctSTRING pMessage
    );

extern gctSTRING VSC_TRACE_BAR_LINE;
extern gctSTRING VSC_TRACE_STAR_LINE;
extern gctSTRING VSC_TRACE_SHARP_LINE;

END_EXTERN_C()

#endif  /* __gc_vsc_dump_h_ */

