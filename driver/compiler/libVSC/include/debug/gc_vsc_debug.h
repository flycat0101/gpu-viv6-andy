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


#ifndef __gc_vsc_debug_h_
#define __gc_vsc_debug_h_

#include "utils/gc_vsc_utils_base.h"
#include "utils/gc_vsc_err.h"
#include "utils/gc_vsc_utils_list.h"
#include "utils/gc_vsc_utils_mm.h"
#include "utils/gc_vsc_utils_hash.h"
#include "utils/gc_vsc_utils_table.h"
#include "old_impl/gc_vsc_old_gcsl.h"
#include "vir/passmanager/gc_vsc_options.h"
#include "vir/ir/gc_vsc_vir_ir.h"
#include "gc_vsc_debug_extern.h"

typedef enum {
    VSC_DI_TAG_INVALID       = 0,
    VSC_DI_TAG_COMPILE_UNIT  = 1,
    VSC_DI_TAG_VARIABE       = 2,
    VSC_DI_TAG_SUBPROGRAM    = 3,
    VSC_DI_TAG_LEXICALBLOCK  = 4,
    VSC_DI_TAG_PARAMETER     = 5,
    VSC_DI_TAG_CONSTANT      = 6,
    VSC_DI_TAG_TYPE          = 7,
}VSC_DIE_TAG;

typedef enum{
    VSC_DIE_REG_TYPE_TMP,
    VSC_DIE_REG_TYPE_CONST,
}VSC_DIE_REG_TYPE;

typedef struct _VSC_DI_REG{
    VSC_DIE_REG_TYPE    type;
    gctUINT16 start;
    gctUINT16 end;
    /* we need consider give mask for every reg, for struct, etc */
    gctUINT16 mask;
}VSC_DI_REG;

#define VSC_DI_INVALID_SW_LOC        0xffff

typedef struct _VSC_DI_SW_LOC{
    gctUINT16 id;
    gctUINT16 next;

    gctBOOL reg;
    union
    {
        VSC_DI_REG reg;
        VSC_DI_OFFSET offset;
    }u;

    gctUINT16 hwLoc;
}VSC_DI_SW_LOC;

#define VSC_DI_INVALID_HW_LOC        0xffff

typedef struct _VSC_DI_HW_LOC{
    gctUINT16 id;
    gctUINT16 next;

    gctUINT16 beginPC;
    gctUINT16 endPC;

    gctBOOL reg;
    union
    {
        VSC_DI_HW_REG reg;
        VSC_DI_OFFSET offset;
    }u;
}VSC_DI_HW_LOC;

#define VSC_DI_MAX_ARRAY_DIM   4

typedef struct _VSC_DI_ARRAY_DESC{
    gctINT numDim;
    gctINT length[VSC_DI_MAX_ARRAY_DIM];
} VSC_DI_ARRAY_DESC;

/* if it's primitve type, its VIR_TypeId, or it's DIE_ID */
typedef struct _VSC_DI_TYPE{
    gctINT  type;
    VSC_DI_ARRAY_DESC array;
    gctBOOL primitiveType;
}VSC_DI_TYPE;

struct _VSC_DIE
{
    gctUINT16 id;
    VSC_DIE_TAG tag;

    /* Form a DIE tree to descript source tree */
    gctUINT16 parent;
    gctUINT16 child;
    gctUINT16 sib;

    gctUINT name;
    gctUINT8 fileNo;
    gctUINT8 colNo;
    gctUINT16 lineNo;
    gctUINT16 endLineNo;

    union
    {
        struct
        {
            gctUINT16 swLoc;
            VSC_DI_TYPE type;
        }
        variable;

        struct
        {
            gctUINT16 pcLine[2];
            VSC_DI_TYPE retType;
        }
        func;

        VSC_DI_TYPE type;
    }u;
};

typedef struct _VSC_DIE VSC_DIE;

typedef gceSTATUS (* PFN_Allocate)(
    IN gcoOS Os,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Memory
    );

typedef gceSTATUS (* PFN_Free)(
    IN gcoOS Os,
    IN gctPOINTER Memory
    );

#define VSC_DI_INVALIDE_DIE 0xffff
#define VSC_DI_SPACE_SKIP_DIE 0xfffe

#define VSC_DI_STRTABLE_INIT_SIZE    10240

typedef struct{
    gctSTRING str;
    gctUINT size;
    gctUINT usedSize;
}VSC_DI_STRTABLE;

#define VSC_DI_DIETABLE_INIT_COUNT    1024

typedef struct{
    VSC_DIE * die;
    gctUINT16 count;
    gctUINT16 usedCount;
}VSC_DI_DIETABLE;

#define VSC_DI_TEMP_LOG_SIZE  256

/* used to quickly find a DIE */
struct _VSC_DI_DIE_INDEX{
    VSC_DIE_TAG tag;
    gctUINT16 parent;
    gctUINT name;
    gctUINT16 die;
};

#define VSC_DI_MC_RANGE_MASK    0xffff
#define VSC_DI_MC_RANGE_SHIFT   16
#define VSC_DI_MC_RANGE_START(mc)  ((mc) & VSC_DI_MC_RANGE_MASK)
#define VSC_DI_MC_RANGE_END(mc)   (((mc) >> VSC_DI_MC_RANGE_SHIFT) & VSC_DI_MC_RANGE_MASK)
#define VSC_DI_MC_RANGE(start, end) ((start) | ((end) << VSC_DI_MC_RANGE_SHIFT ))

#define VSC_DI_CALL_DEPTH       4

typedef enum {
    VSC_STEP_STATE_NONE = 0,
    VSC_STEP_STATE_OVER,
    VSC_STEP_STATE_INTO,
    VSC_STEP_STATE_OUT,
} VSC_STEP_STATE;

typedef struct _VSC_DI_CALL_STACK
{
    VIR_SourceFileLoc sourceLoc; /* current location */
    VIR_SourceFileLoc nextSourceLoc;

    gctUINT nextPC;

    VSC_DIE * die; /* DIE of this subprogram */

} VSC_DI_CALL_STACK;

/* use to record source line to mc line mapping */
typedef struct _VSC_DI_LINE_TABLE_MAP
{
    VIR_SourceFileLoc sourcLoc;
    gctUINT32 mcRange;
}VSC_DI_LINE_TABLE_MAP;

typedef struct _VSC_DI_LINE_TABLE
{
    VSC_DI_LINE_TABLE_MAP * map;
    gctUINT count;
}VSC_DI_LINE_TABLE;

typedef struct _VSC_DI_SW_LOC_LIST{
    gctUINT16 swLoc;
    struct _VSC_DI_SW_LOC_LIST * next;
}VSC_DI_SW_LOC_LIST;

#define VSC_DI_LOCTABLE_INIT_COUNT    128

typedef struct _VSC_DI_HW_LOC_TABLE{
    VSC_DI_HW_LOC * loc;
    gctUINT16 count;
    gctUINT16 usedCount;
}VSC_DI_HW_LOC_TABLE;

typedef struct _VSC_DI_SW_LOC_TABLE{
    VSC_DI_SW_LOC * loc;
    gctUINT16 count;
    gctUINT16 usedCount;
}VSC_DI_SW_LOC_TABLE;

typedef struct{
    gctBOOL collect;
    PFN_Allocate pfnAllocate;
    PFN_Free pfnFree;

    VSC_DI_STRTABLE strTable;
    VSC_DI_DIETABLE dieTable;
    VSC_DI_LINE_TABLE lineTable;
    VSC_DI_HW_LOC_TABLE locTable;
    VSC_DI_SW_LOC_TABLE swLocTable;

    gctUINT16 cu;

    gctCHAR * tmpLog;

    VSC_DI_CALL_STACK callStack[VSC_DI_CALL_DEPTH]; /* always be current call frame */
    gctINT32 callDepth;
    VSC_STEP_STATE stepState;
}VSC_DIContext;


gceSTATUS
vscDIConstructContext(
    PFN_Allocate allocPfn,
    PFN_Free freePfn,
    VSC_DIContext ** context
    );

void
vscDIDestroyContext(
    VSC_DIContext * context
    );

gctUINT16
vscDIAddDIE(
    VSC_DIContext * context,
    VSC_DIE_TAG tag,
    gctUINT16  parentID,
    gctCONST_STRING name,
    gctUINT fileNo,
    gctUINT lineNo,
    gctUINT endLineNo,
    gctUINT colNo
    );

void
vscDIDumpDIE(
    VSC_DIContext * context,
    gctUINT16 id,
    gctUINT shift,
    gctUINT tag
    );

gctUINT16
vscDIGetDIEType(
    VSC_DIContext * context
    );

VSC_DIE *
vscDIGetDIE(
    VSC_DIContext * context,
    gctUINT16 id
    );

void
vscDIDumpDIETree(
    VSC_DIContext * context,
    gctUINT16 id,
    gctUINT tag
    );

gceSTATUS
vscDISaveDebugInfo(
    VSC_DIContext * context,
    gctPOINTER * buffer,
    gctUINT32 * bufferSize
    );

gceSTATUS
vscDILoadDebugInfo(
    VSC_DIContext ** context,
    gctPOINTER * buffer,
    gctUINT32 * bufferSize
    );

void
vscDIDumpLineTable(
    VSC_DIContext * context
    );

gceSTATUS
vscDIAddLineTable(
    VSC_DIContext * context,
    gctUINT count
    );

gceSTATUS
vscDIAddLineMap(
    VSC_DIContext * context,
    gctUINT id,
    VIR_SourceFileLoc sourceLoc,
    gctUINT start,
    gctUINT end
    );

gctUINT16
vscDIAddHWLoc(
    VSC_DIContext * context
    );

VSC_DI_HW_LOC *
vscDIGetHWLoc(
    VSC_DIContext * context,
    gctUINT16 loc
    );

gctUINT16
vscDIAddSWLoc(
    VSC_DIContext * context
    );

VSC_DI_SW_LOC *
vscDIGetSWLoc(
    VSC_DIContext * context,
    gctUINT16 loc
    );

void
vscDISetHwLocToSWLoc(
    VSC_DIContext * context,
    VSC_DI_SW_LOC * swLoc,
    VSC_DI_HW_LOC * hwLoc
    );

void
vscDIChangeUniformSWLoc(
    VSC_DIContext * context,
    gctUINT tmpStart,
    gctUINT tmpEnd,
    gctUINT uniformIdx);

#endif


