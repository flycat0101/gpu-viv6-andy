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

int vscDIGetCallStackDepth(
    void * ptr
    );

void vscDIGetStackFrameInfo(
    void * ptr,
    int frameId,
    unsigned int * functionId,
    unsigned int * callerPc,
    char * functionName,
    unsigned int nameLength,
    char * fileName, /* not used currently */
    char * fullName, /* not used currently */
    unsigned int fileNameLength /* not used currently */
    );

void vscDIGetFunctionInfo(
    void * ptr,
    int functionId,
    char * functionName,
    unsigned int nameLength,
    unsigned int * lowPC,
    unsigned int * highPC
    );

int vscDIGetVariableCount(
    void * ptr,
    int functionId,
    gctBOOL bArgument
    );

void vscDIGetVariableInfo(
    void * ptr,
    const char * parentIdStr,
    int idx,
    gctBOOL bArgument,
    char * varName,
    char * typeStr,
    unsigned int nameLength,
    char * varIdStr,
    unsigned int * lowPC,
    unsigned int * highPC,
    unsigned int * hwLocCount,
    unsigned int * childrenCount
    );

void vscDIGetVariableHWLoc(
    void * ptr,
    const char * varIdStr,
    int idx,
    gctBOOL * bIsReg,
    gctBOOL * bIsConst,
    unsigned int * lowPC,
    unsigned int * highPC,
    unsigned int * data0, /* regStart or baseAdd */
    unsigned int * data1, /* regEnd or offset */
    unsigned int * data2, /*swizzle, HwShift or endOffset */
    unsigned int * data3 /*indicate chanel when store in memory*/
    );

#endif

