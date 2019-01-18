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


#ifndef __gc_vsc_debug_extern_h_
#define __gc_vsc_debug_extern_h_

int vscDIGetSrcLineByPC(
    void * ptr,
    unsigned int pc,
    unsigned int * line
    );

#define VSC_DI_INVALID_PC    0xffff

#define VSC_DI_MAX_VARIABLE_COUNT 16

/*
    If the refPC is 0, we just show the code first we met,
    otherwise, we need show the code after the refPC.
*/
void vscDIGetPCBySrcLine(
    void * ptr,
    unsigned int src,
    unsigned int refPC,
    unsigned int * start,
    unsigned int * end
    );

void vscDIGetNearPCBySrcLine(
    void * ptr,
    unsigned int src,
    unsigned int * newSrc,
    unsigned int * start,
    unsigned int * end
    );

unsigned int
vscDIGetPCByFuncName(
    void * ptr,
    char * name
    );

typedef enum{
    VSC_DIE_HW_REG_TMP,
    VSC_DIE_HW_REG_CONST,
}VSC_DIE_HW_REG_TYPE;

typedef struct _VSC_DI_HW_REG{
    VSC_DIE_HW_REG_TYPE  type;
    unsigned short start;
    unsigned short end;
    unsigned short mask;
}VSC_DI_HW_REG;

typedef struct _VSC_DI_OFFSET{
    VSC_DI_HW_REG baseAddr;
    unsigned short offset;
    unsigned short endOffset;
}VSC_DI_OFFSET;

typedef struct {
    unsigned int reg;
    union
    {
        VSC_DI_HW_REG reg;
        VSC_DI_OFFSET offset;
    }u;
}VSC_DI_EXTERN_LOC;

unsigned int
vscDIGetVaribleLocByNameAndPC(
    void * ptr,
    unsigned int pc,
    char * name,
    void * loc,
    unsigned int * locCount
    );

void vscDIStep(
    void * ptr,
    unsigned int pc,
    unsigned int stepFlag
    );

void vscDIPushCallStack(
    void * ptr,
    unsigned int currentPC,
    unsigned int intoPC
    );

void vscDIPopCallStack(
    void * ptr,
    unsigned int currentPC
    );

#endif

