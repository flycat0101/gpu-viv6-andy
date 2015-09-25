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


#include "gc_vsc.h"

#if gcdENABLE_3D

#include "old_impl/gc_vsc_old_optimizer.h"

#define _W_FOR_MEMORY_CORRUPTION_  1
#define _DEBUG_FOR_MEMORY_CORRUPTION_       0

#if _DEBUG_FOR_MEMORY_CORRUPTION_
/*#include <stdio.h>*/
#endif

#define _REMOVE_UNUSED_FUNCTION_CODE_   0
#define _RECURSIVE_BUILD_DU_            1

#define _GC_OBJ_ZONE    gcvZONE_COMPILER

gcOPTIMIZER_OPTION theOptimizerOption =
{
    gcvOPTIMIZATION_NONE, /* optFlags */

    /* debug & dump options:

         VC_OPTION=-DUMP:SRC[:OPT|:OPTV|:CG|:CGV:|ALL|UNIFORM]
     */
    gcvFALSE, /* SRC:  dump shader source code */
    gcvFALSE, /* OPT:  dump incoming and final IR */
    gcvFALSE, /* OPTV: dump result IR in each optimization phase */
    gcvFALSE, /* CG:   dump generated machine code */
    gcvFALSE, /* CGV:  dump BE tree and optimizer detail */
    gcvFALSE, /* IR:   dump BE final IR */
    gcvFALSE, /* LOG:  dump FE log file in case of compiler error */
    gcvFALSE, /* UNIFORM: dump uniform value when setting uniform */
    0, /* _dumpStart; */
    0x7fffffff, /* _dumpEnd */

    /* Varying Packing:

          VC_OPTION=-PACKVARYING:[0-2]|T[-]m[,n]

          0: turn off varying packing
          1: pack varyings, donot split any varying
          2: pack varyings, may split to make fully packed output  (not implemented)
     */
    gcvOPTIMIZATION_VARYINGPACKING_NOSPLIT, /* packVarying; */
    0, /* _triageStart; */
    0x7fffffff, /* _triageEnd */
    0, /* _loadBalanceShaderIdx; */
    0, /* _loadBalanceMin; */
    0, /* _loadBalanceMax;*/

    /* Do not generate immdeiate

          VC_OPTION=-NOIMM

       Force generate immediate even the machine model don't support it,
       for testing purpose only

          VC_OPTION=-FORCEIMM
     */
    gcvFALSE, /* NOIMM:    Do not generate immdeiate */
    gcvFALSE, /* FORCEIMM: Force to generate immediate */

    /* If need to do any power optimization, set needPowerOptimization
       to true, then set the needed sub options in below */
    gcvFALSE, /* needPowerOptimization; */

    /* Patch TEXLD instruction by adding dummy texld
       (can be used to tune GPU power usage):
         for every TEXLD we seen, add n dummy TEXLD

        it can be enabled by environment variable:

          VC_OPTION=-PATCH_TEXLD:M:N

        (for each M texld, add N dummy texld)
     */
    0, /* M: patchEveryTEXLDs; */
    0, /* N: patchDummyTEXLDs; */

    /* Insert NOP after high power consumption instructions

         VC_OPTION="-INSERTNOP:MUL:MULLO:DP3:DP4:SEENTEXLD"
     */
    gcvFALSE, /* INSERTNOP: insert NOP; */
    gcvFALSE, /* MUL:       insert NOP After MUL; */
    gcvFALSE, /* MULLO:     insert NOP After MULLO; */
    gcvFALSE, /* DP3:       insert NOP After DP3; */
    gcvFALSE, /* DP4:       insert NOP After DP4; */
    gcvFALSE, /* not used, insertNOPOnlyWhenTexldSeen; */

    /* split MAD to MUL and ADD:

         VC_OPTION=-SPLITMAD
     */
    gcvFALSE, /* splitMAD; */

    /* Convert vect3/vec4 operations to multiple vec2/vec1 operations

         VC_OPTION=-SPLITVEC:MUL:MULLO:DP3:DP4
     */
    gcvFALSE, /* SPLITVEC:  split Vec; */
    gcvFALSE, /* MUL:       split Vec4 MUL; */
    gcvFALSE, /* MULLO:     split Vec4 MULLO; */
    gcvFALSE, /* DP3:       split Vec4 DP3; */
    gcvFALSE, /* DP4:       split Vec4 DP4; */

    /* turn/off features:

          VC_OPTION=-F:n,[0|1]
     */
    FB_LIVERANGE_FIX1 | FB_INLINE_RENAMETEMP, /* featureBits; */

    /* Replace specified shader's source code with the contents in
       specified file:

         VC_OPTION=-SHADER:id1,file1[:id2,file ...]

    */
    gcvNULL, /* shaderSrcList; */

    /* Load-time Constant optimization:

        VC_OPTION=-LTC:0|1

     */
#if !DX_SHADER
    gcvTRUE, /* enableLTC; */
#else
    gcvFALSE,
#endif

    /* VC_OPTION=-Ddef1[=value1] -Ddef2[=value2] -Uundef1 */
    gcvNULL, /* macroDefines; */

    /* inliner kind (default 1 VIR inliner):

          VC_OPTION=-INLINER:[0-1]
             0:  gcsl inliner
             1:  VIR inliner

        when VIRCG is not enabled, gcsl inliner is always used.
     */
    GCSL_INLINER_KIND, /* inlinerKind; */

    /* inline level (default 2 at O1):

          VC_OPTION=-INLINELEVEL:[0-3]
             0:  no inline
             1:  only inline the function only called once or small function
             2:  inline functions be called less than 5 times or medium size function
             3:  inline everything possible
     */
    2, /* inlineLevel; */

    /* inline recompilation functions for depth comparison if inline level is not 0.

          VC_OPTION=-INLINEDEPTHCOMP:[0-3]
             0:  follows inline level
             1:  inline depth comparison functions for halti2
             2:  inline depth comparison functions for halti1
             3:  inline depth comparison functions for halti0
     */
    0, /* inlineDepthComparison; */

    /* inline recompilation functions for format conversion if inline level is not 0.

          VC_OPTION=-INLINEFORMATCONV:[0-3]
             0:  follows inline level
             1:  inline format conversion functions for halti2
             2:  inline format conversion functions for halti1
             3:  inline format conversion functions for halti0
     */
    1, /* inlineFormatConversion; */


    /* dual 16 mode
     *
     *    VC_OPTION=-DUAL16:[0-3]
     *       0:  force dual16 off, no dual16 any more.
     *       1:  auto-on mode for specific benchmarks
     *       2:  auto-on mode for all applications.
     *       3:  force dual16 on for all applications no matter highp is specified or not.
     */
    DUAL16_AUTO_BENCH, /* dual16Mode; */
    0, /* _dual16Start; */
    0x7fffffff, /* _dual16End */

    /* force inline or not inline a function
     *
     *   VC_OPTION=-FORCEINLINE:func[,func]*
     *
     *   VC_OPTION=-NOTINLINE:func[,func]*
     *
     */
    gcvNULL, /* forceInline; */

    /* Upload Uniform Block to state buffer if there are space available
     * Doing this may potentially improve the performance as the load
     * instruction for uniform block member can be removed.
     *
     *   VC_OPTION=-UPLOADUBO:0|1
     *
     */
    gcvFALSE, /* uploadUBO; */

    /* OpenCL floating point capabilities setting
     * FASTRELAXEDMATH => -cl-fast-relaxed-math option
     * FINITEMATHONLY => -cl-finite-math-only option
     * RTNE => Round To Even
     * RTZ => Round to Zero
     *
     * VC_OPTION=-OCLFPCAPS:FASTRELAXEDMATH:FINITEMATHONLY:RTNE:RTZ
     */
    0, /* oclFpCaps; */

    /* use VIR code generator:
     *
     *   VC_OPTION=-VIRCG:[0|1|2]|T[-]m[,n]
     *    Tm:    turn on VIRCG for shader id m
     *    Tm,n:  turn on VIRCG for shader id is in range of [m, n]
     *    T-m:   turn off VIRCG for shader id m
     *    T-m,n: turn off VIRCG for shader id is in range of [m, n]
     *
     */
#if VSC_BUILD
    VIRCG_WITH_TREECG, /* useVIRCodeGen; */
#else
    VIRCG_None, /* useVIRCodeGen; */
#endif
    0, /* _vircgStart; */
    0x7fffffff, /* _vircgEnd */

    /* create default UBO:
     *
     *   VC_OPTION=-CREATEDEAULTUBO:0|1
     *
     */
    gcvFALSE, /* createDefaultUBO; */

    /*  OCL has long:
     *
     *   VC_OPTION=-OCLHASLONG:0|1
     *
     */
    gcvTRUE, /* oclHasLong; */

    /*  USE gcSL_NEG for -a instead of SUB(0, a)
     *
     *   VC_OPTION=-OCLUSENEG:0|1
     *
     */
    gcvFALSE, /* oclUseNeg; */

    /* Specify the log file name
     *
     *   VC_OPTION=-LOG:filename
     */
    gcvNULL, /* logFileName; */
    gcvNULL,

    /* turn on/off shader patch:
     *   VC_OPTION=-PATCH:[0|1]|T[-]m[,n]
     *    Tm:    turn on shader patch for shader id m
     *    Tm,n:  turn on shader patch for shader id is in range of [m, n]
     *    T-m:   turn off shader patch for shader id m
     *    T-m,n: turn off shader patch for shader id is in range of [m, n]
     */
    gcvTRUE, /* patchShader; */
    0, /* _patchShaderStart; */
    0x7fffffff, /* _patchShaderEnd; */

    /* set default fragment shader floating point precision if not specified in shader
     *   VC_OPTION=-FRAGMENT_FP_PRECISION:highp:mediump:lowp
     *    highp: high precision
     *    mediump:  medium precision
     *    lowp:   low precision
     */
    gcSHADER_PRECISION_DEFAULT,

    /* OCL use VIR code generator:
     *
     *   VC_OPTION=-CLVIRCG:[0|1]|T[-]m[,n]
     *    Tm:    turn on VIRCG for OCL shader id m
     *    Tm,n:  turn on VIRCG for OCL shader id is in range of [m, n]
     *    T-m:   turn off VIRCG for OCL shader id m
     *    T-m,n: turn off VIRCG for OCL shader id is in range of [m, n]
     */
    gcvFALSE, /* CLUseVIRCodeGen; */

};

const struct _gcSL_INSTRUCTION gcvSL_NOP_INSTR =
{gcSL_NOP, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/******************************************************************************\
|************************* Memory management Functions ************************|
|******************************************************************************/
gcmMEM_DeclareFSMemPool(struct _gcOPT_CODE, Code, )
gcmMEM_DeclareFSMemPool(struct _gcOPT_LIST, List, )
gcmMEM_DeclareFSMemPool(struct _gcOPT_GLOBAL_USAGE, GlobalUsage, )

gcmMEM_DeclareAFSMemPool(struct _gcOPT_CODE, CodeArray, )
gcmMEM_DeclareAFSMemPool(struct _gcOPT_FUNCTION, FunctionArray, )
gcmMEM_DeclareAFSMemPool(struct _gcOPT_TEMP, TempArray, )
gcmMEM_DeclareAFSMemPool(struct _gcOPT_TEMP_DEFINE, TempDefineArray, )

static gceSTATUS
_MemPoolInit(
    gcOPTIMIZER         Optimizer
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcoOS               os = gcvNULL;

    gcmHEADER_ARG("Optimizer=%p", Optimizer);

    gcmERR_RETURN(gcfMEM_InitFSMemPool(&Optimizer->codeMemPool, os, 100, sizeof(struct _gcOPT_CODE)));
    gcmERR_RETURN(gcfMEM_InitFSMemPool(&Optimizer->listMemPool, os, 500, sizeof(struct _gcOPT_LIST)));
    gcmERR_RETURN(gcfMEM_InitFSMemPool(&Optimizer->usageMemPool, os, 50, sizeof(struct _gcOPT_GLOBAL_USAGE)));

    gcmERR_RETURN(gcfMEM_InitAFSMemPool(&Optimizer->codeArrayMemPool, os, 300, sizeof(struct _gcOPT_CODE)));
    gcmERR_RETURN(gcfMEM_InitAFSMemPool(&Optimizer->functionArrayMemPool, os, 10, sizeof(struct _gcOPT_FUNCTION)));
    gcmERR_RETURN(gcfMEM_InitAFSMemPool(&Optimizer->tempArrayMemPool, os, 100, sizeof(struct _gcOPT_TEMP)));
    gcmERR_RETURN(gcfMEM_InitAFSMemPool(&Optimizer->tempDefineArrayMemPool, os, 100, sizeof(struct _gcOPT_TEMP_DEFINE)));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_MemPoolCleanup(
    gcOPTIMIZER         Optimizer
    )
{
    gcmHEADER_ARG("Optimizer=%p", Optimizer);

    gcfMEM_FreeFSMemPool(&Optimizer->codeMemPool);
    gcfMEM_FreeFSMemPool(&Optimizer->listMemPool);
    gcfMEM_FreeFSMemPool(&Optimizer->usageMemPool);

    gcfMEM_FreeAFSMemPool(&Optimizer->codeArrayMemPool);
    gcfMEM_FreeAFSMemPool(&Optimizer->functionArrayMemPool);
    gcfMEM_FreeAFSMemPool(&Optimizer->tempArrayMemPool);
    gcfMEM_FreeAFSMemPool(&Optimizer->tempDefineArrayMemPool);

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/* return -1 if not found, otherwise return the mapped value */
gctUINT32
gcSimpleMap_Find(
     IN SimpleMap *     Map,
     IN gctUINT32       Key
     )
{
    while (Map)
    {
        if (Map->key == Key)
            return Map->val;
        Map = Map->next;
    }
    return (gctUINT32)(-1);
} /* gcSimpleMap_Find */

/* Add a pair <Key, Val> to the Map head, the user should be aware that the
 * map pointer is always changed when adding a new node :
 *
 *   gcSimpleMap_AddNode(&theMap, key, val, allocator);
 *
 */
gceSTATUS
gcSimpleMap_AddNode(
     IN SimpleMap **    Map,
     IN gctUINT32       Key,
     IN gctUINT32       Val,
     IN gcsAllocator *  Allocator
     )
{
    gceSTATUS           status = gcvSTATUS_OK;
    SimpleMap *         nodePtr;
    gcmHEADER();

    /* allocate memory */
    gcmONERROR(Allocator->allocate(sizeof(SimpleMap), (gctPOINTER *)&nodePtr));

    nodePtr->key  = Key;
    nodePtr->val  = Val;
    nodePtr->next = *Map;  /* always put the new node at the front */

    *Map = nodePtr;

OnError:
    gcmFOOTER();
    return status;
} /* gcSimpleMap_AddNode */


gceSTATUS
gcSimpleMap_Destory(
     IN SimpleMap *     Map,
     IN gcsAllocator *  Allocator
     )
{
    gceSTATUS           status = gcvSTATUS_OK;
    SimpleMap *         nodePtr;

    gcmHEADER();

    while (Map)
    {
        nodePtr = Map->next;
        gcmONERROR(Allocator->deallocate(Map));
        Map = nodePtr;
    }

OnError:
    gcmFOOTER();
    return status;
} /* gcSimpleMap_Destory */

/* gcsList operations */

void
gcList_Init(
    IN gcsList *        list,
    IN gcsAllocator *   allocator
    )
{
    list->head = list->tail = gcvNULL;
    list->count = 0;
    list->allocator = allocator;
}

gceSTATUS
gcList_CreateNode(
    IN void *           Data,
    IN gctAllocatorFunc Allocator,
    OUT gcsListNode **  ListNode
    )
{
    gceSTATUS           status;
    gctPOINTER          pointer = gcvNULL;

    gcmHEADER();

    /* Allocate a new gcsCodeListCode structure. */
    gcmONERROR(
        Allocator(sizeof(gcsListNode), &pointer));
    *ListNode = (gcsListNode*) pointer;
    (*ListNode)->data = Data;
    (*ListNode)->next = gcvNULL;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
} /* gcList_CreateNode */

gceSTATUS
gcList_Clean(
    IN gcsList *        List,
    IN gctBOOL          FreeData
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcsListNode *       curNode;
    gctDeallocatorFunc  Dealloc = List->allocator->deallocate;

    gcmHEADER();

    curNode = List->head;
    while (curNode)
    {
        gcsListNode *nextNode = curNode->next;
        if (Dealloc)
        {
            if (FreeData)
            {
                gcmONERROR(Dealloc(curNode->data));
            }
            gcmONERROR(Dealloc(curNode));
        }
        curNode = nextNode;
    }

    List->head = gcvNULL;
    List->tail = gcvNULL;
    List->count = 0;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
} /* gcList_Clean */

gcsListNode *
gcList_FindNode(
    IN gcsList *        List,
    IN void *           Key,
    IN compareFunc      compare
    )
{
    gcsListNode *       curNode;

    gcmHEADER();

    curNode = List->head;
    while (curNode)
    {
        if (compare(curNode->data, Key))
        {
            gcmFOOTER_NO();
            return curNode;
        }

        curNode = curNode->next;
    }

    gcmFOOTER_NO();
    return gcvNULL;
} /* gcList_FindNode */

gceSTATUS
gcList_AddNode(
    IN gcsList *        List,
    IN void *           Data
    )
{
    gceSTATUS           status;
    gcsListNode *       node;

    gcmHEADER();

    gcmVERIFY_ARGUMENT(List != gcvNULL);

    gcmONERROR(gcList_CreateNode(Data, List->allocator->allocate, &node));
    /* the list is empty, set the head and tail to the new node */
    if (List->head == gcvNULL)
        List->head = List->tail = node;
    else
    {
        List->tail->next = node;
        List->tail = node;
    }

    List->count++;
    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
} /* gcList_AddNode */

/* remove the Node from List, free the Node memory */
gceSTATUS
gcList_RemoveNode(
    IN gcsList *        List,
    IN gcsListNode *    Node
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcmHEADER();

    gcmVERIFY_ARGUMENT(List != gcvNULL && Node != gcvNULL);

    if (List->head == Node)
    {
        List->head = Node->next;

        /* No nodes are in list now, so must set tail as NULL */
        if (List->head == gcvNULL)
        {
            List->tail = gcvNULL;
        }
    }
    else
    {
        gcsListNode *curNode = List->head;
        while (curNode && curNode->next != Node)
            curNode = curNode->next;
        if (curNode)
        {
            /* found the next node is the Node to remove */
            gcmASSERT(curNode->next == Node);
            curNode->next = Node->next;

            /* Tail moves backward by one node */
            if (List->tail == Node)
            {
                List->tail = curNode;
            }
        }
        else {
            /* not found */
            gcmASSERT(gcvFALSE);
        }
    }

    /* free the Node */
    gcmONERROR(List->allocator->deallocate(Node));
    List->count--;
    gcmASSERT(List->count >= 0);

OnError:
    gcmFOOTER();
    return status;
} /* gcList_RemoveNode */

gctBOOL
gcDoTriageForShaderId(
    IN gctINT           shaderId,
    IN gctINT           startId,
    IN gctINT           endId
    )
{
    if ((startId == 0 && endId == 0) || shaderId == 0)
        return gcvTRUE;

    if (startId >= 0)
    {
        gcmASSERT(startId <= endId);  /* [21, 60 ] */
        return shaderId >= startId && shaderId <= endId;
    }
    else
    {
        gcmASSERT(endId < 0 && startId >= endId);  /* [-45, -100] */
        return shaderId < -startId || shaderId > -endId;
    }
}

gctINT gcSHADER_getEffectiveShaderId(
    IN void *         Shader
    )
{
    if (Shader == gcvNULL)
        return 0;

    if (((gcSHADER)Shader)->object.type == gcvOBJ_SHADER)
    {
        return ((gcSHADER)Shader)->_id;
    }
    else
    {
#if VSC_BUILD
        VIR_Shader * vShader;
        vShader = (VIR_Shader *)Shader;
        return vShader->_id;
#else
        return 0;
#endif
    }
}

/* */
gctBOOL
gcOPT_doVaryingPackingForShader(
    IN gcSHADER         Shader
    )
{
    gctINT              startId = gcmOPT_PACKVARYING_triageStart();
    gctINT              endId = gcmOPT_PACKVARYING_triageEnd();

    return gcDoTriageForShaderId(gcSHADER_getEffectiveShaderId(Shader), startId, endId);
}

gctBOOL
gcSHADER_GoVIRPass(gcSHADER Shader)
{
    if ((GetShaderType(Shader) == gcSHADER_TYPE_CL && gcmOPT_CLUseVIRCodeGen()) ||
        (GetShaderType(Shader) != gcSHADER_TYPE_CL && (gcmOPT_UseVIRCodeGen() != VIRCG_None)))
    {
        gctINT   startId = gcmOPT_VIRCGStart();
        gctINT   endId   = gcmOPT_VIRCGEnd();
        return gcDoTriageForShaderId(gcSHADER_getEffectiveShaderId(Shader), startId, endId);
    }
    return gcvFALSE;
}

gctBOOL
gcSHADER_DoPatch(gcSHADER Shader)
{
    if (gcmOPT_PatchShader())
    {
        gctINT   startId = gcmOPT_PatchShaderStart();
        gctINT   endId   = gcmOPT_PatchShaderEnd();
        return gcDoTriageForShaderId(gcSHADER_getEffectiveShaderId(Shader), startId, endId);
    }
    return gcvFALSE;
}

gctBOOL
gcSHADER_DumpSource(gcSHADER Shader)
{
    gcOPTIMIZER_OPTION * option = gcmGetOptimizerOption();

    if (option->dumpShaderSource)
    {
        gctINT   startId = option->_dumpStart;
        gctINT   endId   = option->_dumpEnd;
        return gcDoTriageForShaderId(gcSHADER_getEffectiveShaderId(Shader), startId, endId);
    }
    return gcvFALSE;
}

gctBOOL
gcSHADER_DumpOptimizer(gcSHADER Shader)
{
    gcOPTIMIZER_OPTION * option = gcmGetOptimizerOption();

    if (option->dumpOptimizer || option->dumpOptimizerVerbose)
    {
        gctINT   startId = option->_dumpStart;
        gctINT   endId   = option->_dumpEnd;
        return gcDoTriageForShaderId(gcSHADER_getEffectiveShaderId(Shader), startId, endId);
    }
    return gcvFALSE;
}

gctBOOL
gcSHADER_DumpOptimizerVerbose(gcSHADER Shader)
{
    gcOPTIMIZER_OPTION * option = gcmGetOptimizerOption();

    if (option->dumpOptimizerVerbose)
    {
        gctINT   startId = option->_dumpStart;
        gctINT   endId   = option->_dumpEnd;
        return gcDoTriageForShaderId(gcSHADER_getEffectiveShaderId(Shader), startId, endId);
    }
    return gcvFALSE;
}

gctBOOL
gcSHADER_DumpCodeGen(void * Shader)
{
    gcOPTIMIZER_OPTION * option = gcmGetOptimizerOption();

    if (option->dumpBEGenertedCode || option->dumpBEVerbose)
    {
        gctINT   startId = option->_dumpStart;
        gctINT   endId   = option->_dumpEnd;
        return gcDoTriageForShaderId(gcSHADER_getEffectiveShaderId(Shader), startId, endId);
    }
    return gcvFALSE;
}

gctBOOL
gcSHADER_DumpCodeGenVerbose(void * Shader)
{
    gcOPTIMIZER_OPTION * option = gcmGetOptimizerOption();

    if (option->dumpBEVerbose)
    {
        gctINT   startId = option->_dumpStart;
        gctINT   endId   = option->_dumpEnd;
        return gcDoTriageForShaderId(gcSHADER_getEffectiveShaderId(Shader), startId, endId);
    }
    return gcvFALSE;
}

gctBOOL VirSHADER_DumpCodeGenVerbose(gctINT ShaderId)
{
    gcOPTIMIZER_OPTION * option = gcmGetOptimizerOption();

    if (option->dumpBEVerbose)
    {
        gctINT   startId = option->_dumpStart;
        gctINT   endId   = option->_dumpEnd;
        return gcDoTriageForShaderId(ShaderId, startId, endId);
    }
    return gcvFALSE;
}

gctBOOL
gcSHADER_DumpFinalIR(gcSHADER Shader)
{
    gcOPTIMIZER_OPTION * option = gcmGetOptimizerOption();

    if (option->dumpBEFinalIR)
    {
        gctINT   startId = option->_dumpStart;
        gctINT   endId   = option->_dumpEnd;
        return gcDoTriageForShaderId(gcSHADER_getEffectiveShaderId(Shader), startId, endId);
    }
    return gcvFALSE;
}

gctBOOL VirSHADER_DoDual16(gctINT ShaderId)
{
    gcOPTIMIZER_OPTION * option = gcmGetOptimizerOption();

    gctINT   startId = option->_dual16Start;
    gctINT   endId   = option->_dual16End;
    return gcDoTriageForShaderId(ShaderId, startId, endId);
}

gctBOOL
gcOPT_getLoadBalanceForShader(
    IN gcSHADER         Shader,
    OUT gctINT *        Min,
    OUT gctINT *        Max
    )
{
    if (gcmOPT_LB_ShaderIdx() == -1)
    {
        /* the min and max are factor*100 */
        *Min = (gctINT)((gctFLOAT)*Min * gcmOPT_LB_Min()/100.0);
        *Max = (gctINT)((gctFLOAT)*Max * gcmOPT_LB_Max()/100.0);
        if (*Max < *Min)
           *Max = *Min;
        return gcvTRUE;
    }
    if (gcmOPT_LB_ShaderIdx() == (gctINT)Shader->_id)
    {
        *Min = gcmOPT_LB_Min();
        *Max = gcmOPT_LB_Max();
        return gcvTRUE;
    }
    return gcvFALSE;
}

/*
 * Get used components in Source for the Instruction, the return value
 * is in Enable format:
 *
 *     MOV   temp(1).yz, temp(0).xxw
 *
 * the components used in source 0 are x and w, so the return value
 * is (gcSL_ENABLE_X | gcSL_ENABLE_W)
 *
 */
gcSL_ENABLE
gcGetUsedComponents(
    IN gcSL_INSTRUCTION Instruction,
    IN gctINT           SourceNo
    )
{
    gctUINT16           enable = gcmSL_TARGET_GET(Instruction->temp, Enable);
    gctSOURCE_t         source = SourceNo == 0 ? Instruction->source0 : Instruction->source1;
    gcSL_OPCODE         opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(Instruction->opcode, Opcode);
    gcSL_ENABLE         usedComponents = 0;
    gctINT              i;

    if (opcode == gcSL_DP3)
    {
        return gcSL_ENABLE_XYZ;
    }
    else if (opcode == gcSL_DP4)
    {
        return gcSL_ENABLE_XYZW;
    }
    else if (opcode == gcSL_CROSS)
    {
        return gcSL_ENABLE_XYZ;
    }

    if(enable == gcSL_ENABLE_NONE)
    {
        /* for instructions like JMP, because the enable is NONE, the source should be
           counted directly
        */
        usedComponents = gcSL_ConvertSwizzle2Enable(gcmSL_SOURCE_GET(source, SwizzleX),
                                                    gcmSL_SOURCE_GET(source, SwizzleY),
                                                    gcmSL_SOURCE_GET(source, SwizzleZ),
                                                    gcmSL_SOURCE_GET(source, SwizzleW));
    }
    else
    {
        for (i=0; i < gcSL_COMPONENT_COUNT; i++)
        {
            if (gcmIsComponentEnabled(enable, i))
            {
                gcSL_SWIZZLE swizzle = 0;
                switch (i)
                {
                case 0:
                    swizzle = (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleX);
                    break;
                case 1:
                    swizzle = (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleY);
                    break;
                case 2:
                    swizzle = (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleZ);
                    break;
                case 3:
                    swizzle = (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleW);
                    break;
                default:
                    gcmASSERT(0);
                    break;
                } /* switch */
                /* the component in swizzle is used, convert swizzle to enable */
                usedComponents |= 1 << swizzle;
            } /* if */
        } /* for */
    }
    return usedComponents;
} /* gcGetUsedComponents */

/* return true if the FristCode and SecondCode in the same Basic Block:
 *
 *   1. There is no control flow instruction in between
 *   2. There is no jump or call to the code in between
 *
 */
gctBOOL
gcOpt_isCodeInSameBB(
    IN gcOPT_CODE       FirstCode,
    IN gcOPT_CODE       SecondCode
    )
{
    gcOPT_CODE          curCode, firstCode, secondCode;

    /* Same codes, in the same BB. */
    if (FirstCode == SecondCode)
    {
        return gcvTRUE;
    }

    /* Belong to two functions, not in the same BB. */
    if (FirstCode->function != SecondCode->function)
    {
        return gcvFALSE;
    }

    /* Find the real first code. */
    if (FirstCode->id > SecondCode->id)
    {
        firstCode = SecondCode;
        secondCode = FirstCode;
    }
    else
    {
        firstCode = FirstCode;
        secondCode = SecondCode;
    }

    for (curCode = firstCode; curCode != gcvNULL && curCode != secondCode; curCode = curCode->next)
    {
        gcSL_OPCODE opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(curCode->instruction.opcode, Opcode);
        if (opcode == gcSL_JMP ||
            opcode == gcSL_CALL ||
            opcode == gcSL_RET)
        {
            return gcvFALSE;
        }

        if (curCode->callers != gcvNULL && curCode != firstCode)
        {
            /* has other instruction jump to this code */
            return gcvFALSE;
        }
    } /* for */

    if (curCode == secondCode)
    {
        /* found the second code can be reached without meet any control flow */
        return gcvTRUE;
    }

    return gcvFALSE;
} /* gcOpt_isCodeInSameBB */

/* return true if the FristCode is dominated by the SecondCode */
gctBOOL
gcOpt_dominatedBy(
    IN gcOPT_CODE       FirstCode,
    IN gcOPT_CODE       SecondCode
    )
{
    /* now we only handle if the frist and second code are in
     * the same basic block
     */
    return gcOpt_isCodeInSameBB(SecondCode, FirstCode);

} /* gcOpt_dominatedBy */

gctBOOL
gcOpt_isRedefKillsAllPrevDef(
      IN gcOPT_LIST     Dependencies,
      IN gcSL_ENABLE    EanbledComponents
      )
{
    gcOPT_LIST          curDep = Dependencies;

    /* go through the dependencies list and find the first one with
     * at least one of same enabled component, then find the next one
     * with the enabled component, if the later one has some component
     * overlapped with the first one, check if the later one kills the
     * first one, if the later one doesn't have overlapped component
     * with the first one, set the later one as start of next round of
     * check
     *
     *        14: MOV             temp(14).xyz, uniform(5).xyz
     *        14: Users: 16
     *            N Def: 15
     *        15: ADD             temp(14).z, uniform(5).z, 1.000000
     *        15: Users: 16
     *            P Def: 14
     *        16: NORM            temp(15).xyz, temp(14).xyz
     *        16: Users: 17
     *            Src 0: 15, 14
     */
    while(curDep != gcvNULL )
    {
        gcOPT_CODE   firstCode;
        gctUINT16    curDepIdx;
        gcOPT_LIST   nextDep;
        gcOPT_LIST   dep;
        gcOPT_LIST   nextRoundStart = gcvNULL;
        gcSL_ENABLE  usedComponents;

        nextDep = curDep->next;
        if (nextDep == gcvNULL)
            break;

        if (curDep->index < 0) /* depend on global/parameter/input, skip it */
        {
            curDep = nextDep;
            continue;
        }

        firstCode = curDep->code;
        curDepIdx = firstCode->instruction.tempIndex;
        dep = nextDep;
        usedComponents = gcmSL_TARGET_GET(firstCode->instruction.temp, Enable);

        usedComponents &= EanbledComponents;
        /* the user of the dependency may only use part of components */
        if (usedComponents == 0)
        {
            curDep = nextDep;
            continue;
        }

        /* go through the rest of the list to find if there is
           a dependency has the same index */

        for (; dep != gcvNULL; dep = dep->next)
        {
            /* found the next dependency has the same index
               as current dependency */
            gctUINT16    depIdx;

            if (dep->code == gcvNULL) /* depend on input */
                continue;

            depIdx = dep->code->instruction.tempIndex;
            /*  check if the dependencies are in the same basic block
             *  or defines different components
             */
            if (depIdx == curDepIdx)
            {
                /* check if any component is used in the both dependencies */
                gcSL_ENABLE  depUsedComponents =
                              gcmSL_TARGET_GET(dep->code->instruction.temp, Enable);

                depUsedComponents &= EanbledComponents;

                if ((usedComponents & depUsedComponents) != 0)
                {
                    /* both dependencies define the same component,
                     * check if the later kills the first one */
                    if (!gcOpt_dominatedBy(firstCode, dep->code ))
                        return gcvFALSE;
                    else
                        continue;
                }
                else if (nextRoundStart == gcvNULL)
                {
                    /* the later dependency code should be the next round start */
                    nextRoundStart = dep;
                }
            }

        } /* for */
        /* set the next round start */
        curDep = (nextRoundStart == gcvNULL) ? curDep->next
                                             : nextRoundStart;
    } /* for */

    /* all definitions are killed by later redefinition */
    return gcvTRUE;
} /* gcOpt_isRedefKillsAllPrevDef */

/* check if the code's dependencies have a same temp variable as
 * dependency for more than one time, it can happen in following case:
 *
 *  001  if (cond)
 *  002    temp1 = ...
 *  003  else
 *  004    temp1 = ...
 *  005  temp2 = temp1;
 *
 *  The source0 of stmt 005 will have 002 and 004 as its depenencies,
 *  the target temp registers of both statements are temp1.
 *
 * There are other cases may depend on the same temp register but
 * different components, which we should not count them as multiple
 * dependencies:
 *
 *  001  temp1.xy = u1
 *  002  temp1.z = 0.0
 *  003  temp1.w = 1.0
 *  004  temp2.yzw = temp1.xwyz
 *
 *  In 004, temp2 has 001, 002, 003 as its source0's dependencies, and temp1
 *  are the target temp register for these 3 statements
 *
 *  The EnabledComponents are the components enabled in the instruction,
 *  in the examle above, 004 has yzw enabled
 *
 *  Need to handle this case:
 *
 *  005  temp1.xy = u1
 *  006  temp1.yz = 0.0
 *  007  temp1.w = 1.0
 *  008  temp2 = temp1
 *
 *  008 is dependen on 005, 006, 007, there is a overlap component temp1.y@005
 *  and temp1.y@006, however the temp1.y@005 is killed by temp1.y@006, we should
 *  be able to get the temp1@008 as <u1.x, 0.0, 0.0, 1.0>
 *
 */
gctBOOL
gcOpt_hasMultipleDependencyForSameTemp(
      IN gcOPT_LIST     Dependencies,
      IN gcSL_ENABLE    EanbledComponents
      )
{
    gcOPT_LIST          curDep = Dependencies;

    for (curDep = Dependencies; curDep != gcvNULL; curDep = curDep->next )
    {
        gcOPT_CODE   depCode;
        gctUINT16    curDepIdx;
        gcOPT_LIST   nextDep;
        gcOPT_LIST   dep;
        gcSL_ENABLE  usedComponents;

        if (curDep->index < 0) /* depend on global/parameter/input, skip it */
            continue;

        depCode = curDep->code;
        curDepIdx = depCode->instruction.tempIndex;
        nextDep = curDep->next;
        dep = nextDep;
        usedComponents = gcmSL_TARGET_GET(depCode->instruction.temp, Enable);

        /* the user of the dependency may only use part of components */
        usedComponents &= EanbledComponents ;

        /* go through the rest of the list to find if there is
           a dependency has the same index */

        for (; dep != gcvNULL; dep = dep->next)
        {
            /* found the next dependency has the same index
               as current dependency */
            gctUINT16    depIdx;

            if (dep->code == gcvNULL) /* depend on input */
                continue;

            depIdx = dep->code->instruction.tempIndex;
            /* TODO: check if the dependencies are in the same basic block
             *       or defines different components
             */
            if (depIdx == curDepIdx)
            {
                /* check if any component is used in the both dependencies */
                gcSL_ENABLE  depUsedComponents =
                              gcmSL_TARGET_GET(dep->code->instruction.temp, Enable);

                depUsedComponents &= EanbledComponents;

                if ((usedComponents & depUsedComponents) != 0)
                {
                    /* TODO: check if the conflict component is actually killed by later
                     * assignment */
                    if (!gcOpt_isRedefKillsAllPrevDef(Dependencies,
                                                usedComponents & depUsedComponents))
                        return gcvTRUE;
                }

                usedComponents |= depUsedComponents; /* merge used components */
            } /* if */
        } /* for */
    } /* for */
    return gcvFALSE;
} /* gcOpt_hasMultipleDependencyForSameTemp */

gctBOOL
gcOpt_IsTempFunctionArgument(
    IN  gcOPTIMIZER      Optimizer,
    IN  gcOPT_FUNCTION   Function,
    IN  gctUINT          Index,
    IN  gctUINT          InputOrOutputOrAll,
    OUT gctUINT *        ArgIndex,
    OUT gcOPT_FUNCTION * ArgFunction
    )
{
    gctUINT     i, j, argIndex = 0;
    gctBOOL     specFunc = (Function == gcvNULL) ? gcvFALSE : gcvTRUE, found = gcvFALSE;
    gcOPT_FUNCTION  argFunction = gcvNULL;

    for (i = 0; i < Optimizer->functionCount; i++)
    {
        gcOPT_FUNCTION function;

        if (specFunc)
            function = Function;
        else
            function = Optimizer->functionArray + i;

        for (j = 0; j < function->argumentCount; j++)
        {
            gcsFUNCTION_ARGUMENT_PTR argument = function->arguments + j;

            if (InputOrOutputOrAll == 0)
            {
                if (argument->qualifier == gcvFUNCTION_INPUT ||
                    argument->qualifier == gcvFUNCTION_INOUT)
                {
                    if (argument->index == Index)
                    {
                        argFunction = function;
                        argIndex = j;
                        found = gcvTRUE;
                        break;
                    }
                }
            }
            else if (InputOrOutputOrAll == 1)
            {
                if (argument->qualifier == gcvFUNCTION_OUTPUT ||
                    argument->qualifier == gcvFUNCTION_INOUT)
                {
                    if (argument->index == Index)
                    {
                        argFunction = function;
                        argIndex = j;
                        found = gcvTRUE;
                        break;
                    }
                }
            }
            else
            {
                if (argument->index == Index)
                {
                    argFunction = function;
                    argIndex = j;
                    found = gcvTRUE;
                    break;
                }
            }
        }

        if (specFunc || found)
            break;
    }

    if (ArgIndex)
    {
        *ArgIndex = argIndex;
    }

    if (Function == gcvNULL && ArgFunction)
    {
        *ArgFunction = argFunction;
    }

    return found;
}

#if _DEBUG_FOR_MEMORY_CORRUPTION_
static void
_CheckList(
    IN gcOPT_LIST       InputList,
    IN gcOPT_CODE       Code
    )
{
    gcOPT_LIST          list;

    for (list = InputList; list; list = list->next)
    {
        if (list->index >= 0)
        {
            if (list->code == gcvNULL)
            {
                printf("Invalid list->code (index=%d).\n", list->index);
                printf("InputList=%p list=%p\n", InputList, list);
                printf("Code->id=%d Code->instruction.opcode=%d\n", Code->id, gcmSL_OPCODE_GET(Code->instruction.opcode, Opcode));
                printf("list->code->id=%d\n", list->code->id);
            }
        }
    }
}
#endif

/******************************************************************************\
|************************** List Supporting Functioins ************************|
\******************************************************************************/
gceSTATUS
gcOpt_AddIndexToList(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gctINT           Index
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_LIST          list;

    gcmHEADER_ARG("Optimizer=%p Root=%p Index=%d", Optimizer, Root, Index);

    /* Walk all entries in the list. */
    for (list = *Root; list; list = list->next)
    {
        /* Does the current list entry matches the new one? */
        if (list->index == Index)
        {
            /* Success. */
            gcmFOOTER_ARG("*Root=%p", *Root);
            return gcvSTATUS_OK;
        }
    }

    /* Allocate a new gcOPT_LIST structure. */
    gcmERR_RETURN(_CAllocateList(Optimizer->listMemPool, &list));

    /* Initialize the gcOPT_LIST structure. */
    list->next    = *Root;
    list->index   = Index;
    list->code    = gcvNULL;

    /* Link the new gcOPT_LIST structure into the list. */
    *Root = list;

    /* Success. */
    gcmFOOTER_ARG("*Root=%p", *Root);
    return gcvSTATUS_OK;
}

gceSTATUS
gcOpt_CheckListHasUndefined(
    IN gcOPTIMIZER Optimizer,
    IN gcOPT_LIST  Root
    )
{
    gcOPT_LIST          list;

    gcmHEADER_ARG("Optimizer=%p Root=%p ", Optimizer, Root);

    for (list = Root; list; list = list->next)
    {
        if (list->index == gcvOPT_UNDEFINED_REGISTER ||
            list->index == gcvOPT_JUMPUNDEFINED_REGISTER)
        {
            return gcvSTATUS_TRUE;
        }
    }

    /* Success. */
    gcmFOOTER_ARG("*Root=%p", *Root);
    return gcvSTATUS_OK;
}

gceSTATUS
gcOpt_ReplaceIndexInList(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gctUINT          Index,
    IN gctUINT          NewIndex
    )
{
    gcOPT_LIST          list;

    gcmHEADER_ARG("Optimizer=%p Root=%p Index=%d NewIndex=%d",
                    Optimizer, Root, Index, NewIndex);

    /* Walk all entries in the list. */
    for (list = *Root; list; list = list->next)
    {
        if (list->index == (gctINT) Index)
        {
            /* Found it, and replace it. */
            list->index = NewIndex;
            gcmFOOTER_ARG("*Root=%p", *Root);
            return gcvSTATUS_OK;
        }
    }

    gcmFOOTER_ARG("*Root=%p status=%d", *Root, gcvSTATUS_NO_MORE_DATA);
    return gcvSTATUS_NO_MORE_DATA;
}

gceSTATUS
gcOpt_DeleteIndexFromList(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gctINT           Index
    )
{
    gcOPT_LIST          list;
    gcOPT_LIST          prevList = gcvNULL;

    gcmHEADER_ARG("Optimizer=%p Root=%p Index=%d", Optimizer, Root, Index);

    /* Walk all entries in the list. */
    for (list = *Root; list; list = list->next)
    {
        if (list->index == Index)
        {
            break;
        }
        prevList = list;
    }

    if (list)
    {
        if (prevList)
        {
            prevList->next = list->next;
        }
        else
        {
            *Root = list->next;
        }

        /* Free the gcOPT_LIST structure. */
        gcmVERIFY_OK(_FreeList(Optimizer->listMemPool, list));
    }
    /*else
    {
        gcmFOOTER_ARG("*Root=%p status=%d", *Root, gcvSTATUS_NO_MORE_DATA);
        return gcvSTATUS_NO_MORE_DATA;
    }*/

    /* Success. */
    gcmFOOTER_ARG("*Root=%p", *Root);
    return gcvSTATUS_OK;
}

gceSTATUS
gcOpt_DestroyList(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root
    )
{
    gcOPT_LIST          list, nextList;
    gceSTATUS           status;

    gcmHEADER_ARG("Optimizer=%p Root=%p", Optimizer, Root);

    gcmASSERT(Root);
    if (Root == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    for (list = *Root; list; list = nextList)
    {
        nextList = list->next;

        /* Free the gcOPT_LIST structure. */
        gcmVERIFY_OK(_FreeList(Optimizer->listMemPool, list));
    }

    *Root = gcvNULL;

    gcmFOOTER_ARG("*Root=%p", *Root);
    return gcvSTATUS_OK;
}

gceSTATUS
gcOpt_FreeList(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * List
    )
{
    gceSTATUS           status;

    gcmHEADER_ARG("Optimizer=%p List=%p", Optimizer, List);

    gcmASSERT(List);
    if (List == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    gcmVERIFY_OK(_FreeList(Optimizer->listMemPool, *List));

    *List = gcvNULL;

    gcmFOOTER_ARG("*List=%p", *List);
    return gcvSTATUS_OK;
}

gceSTATUS
gcOpt_FindCodeInList(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_LIST       Root,
    IN gcOPT_CODE       Code
    )
{
    gceSTATUS           status = gcvSTATUS_FALSE;
    gcOPT_LIST          list;

    gcmHEADER_ARG("Optimizer=%p Root=%p Code=%p", Optimizer, Root, Code);

    /* Walk all entries in the list. */
    for (list = Root; list; list = list->next)
    {
        if (list->code == Code)
        {
            /* Success. */
            gcmFOOTER_ARG("*Root=%p", *Root);
            return gcvSTATUS_TRUE;
        }
    }

    /* Success. */
    gcmFOOTER_ARG("Root=0x%x", Root);
    return status;
}

gceSTATUS
gcOpt_AddCodeToList(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gcOPT_CODE       Code
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_LIST          list;
#if _W_FOR_MEMORY_CORRUPTION_
    gcOPT_LIST          list1;
#endif

    gcmHEADER_ARG("Optimizer=%p Root=%p Code=%p", Optimizer, Root, Code);

    /* Walk all entries in the list. */
    for (list = *Root; list; list = list->next)
    {
        /* Does the current list entry matches the new one? */
        if (list->code == Code)
        {
            /* Success. */
            gcmFOOTER_ARG("*Root=%p", *Root);
            return gcvSTATUS_OK;
        }
    }

    /* Allocate a new gcOPT_LIST structure. */
    gcmERR_RETURN(_CAllocateList(Optimizer->listMemPool, &list));

#if _W_FOR_MEMORY_CORRUPTION_
    list1 = list;
#endif

    /* Initialize the gcOPT_LIST structure. */
    list->next  = *Root;
    list->code  = Code;
    list->index = 0;

#if _W_FOR_MEMORY_CORRUPTION_
    if (list1->code != Code)
    {
#if _DEBUG_FOR_MEMORY_CORRUPTION_
        printf("list->code=%p\n", list->code);
        printf("list1->code=%p\n", list1->code);
        printf("list->code->id=%d\n", list->code->id);
        printf("(*Root)->code=%p\n", (*Root)->code);
#else
        list1->next  = *Root;
        list1->code  = Code;
        list1->index = 0;
#endif
    }
#endif

#if _DEBUG_FOR_MEMORY_CORRUPTION_
    _CheckList(list, Code);
#endif

    /* Link the new gcOPT_LIST structure into the list. */
#if _W_FOR_MEMORY_CORRUPTION_
    *Root = list1;
#else
    *Root = list;
#endif

    /* Success. */
    gcmFOOTER_ARG("*Root=%p", *Root);
    return gcvSTATUS_OK;
}

gceSTATUS
gcOpt_DeleteCodeFromList(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gcOPT_CODE       Code
    )
{
    gcOPT_LIST          list;
    gcOPT_LIST          prevList = gcvNULL;

    gcmHEADER_ARG("Optimizer=%p Root=%p Code=%p", Optimizer, Root, Code);

    /* Walk all entries in the list. */
    for (list = *Root; list; list = list->next)
    {
        if (list->code == Code)
        {
            break;
        }
        prevList = list;
    }

    if (list)
    {
        if (prevList)
        {
            prevList->next = list->next;
        }
        else
        {
            *Root = list->next;
        }

        /* Free the gcOPT_LIST structure. */
        gcmVERIFY_OK(_FreeList(Optimizer->listMemPool, list));
    }
    else
    {
        gcmFOOTER_ARG("*Root=%p status=%d", *Root, gcvSTATUS_NO_MORE_DATA);
        return gcvSTATUS_NO_MORE_DATA;
    }

    /* Success. */
    gcmFOOTER_ARG("*Root=%p", *Root);
    return gcvSTATUS_OK;
}

gceSTATUS
gcOpt_ReplaceCodeInList(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gcOPT_CODE       Code,
    IN gcOPT_CODE       NewCode
    )
{
    gcOPT_LIST          list;

    gcmHEADER_ARG("Optimizer=%p Root=%p Code=%d NewCode=%d",
                    Optimizer, Root, Code, NewCode);

    /* Walk all entries in the list. */
    for (list = *Root; list; list = list->next)
    {
        if (list->code == Code)
        {
            /* Found it, and replace it. */
            list->code = NewCode;
            gcmFOOTER_ARG("*Root=%p", *Root);
            return gcvSTATUS_OK;
        }
    }

    gcmFOOTER_ARG("*Root=%p status=%d", *Root, gcvSTATUS_NO_MORE_DATA);
    return gcvSTATUS_NO_MORE_DATA;
}

gceSTATUS
gcOpt_CheckCodeInList(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gcOPT_CODE       Code
    )
{
    gcOPT_LIST          list;

    gcmHEADER_ARG("Optimizer=%p Root=%p Code=%d",
                    Optimizer, Root, Code);

    /* Walk all entries in the list. */
    for (list = *Root; list; list = list->next)
    {
        if (list->code == Code)
        {
            /* Found it */
            gcmFOOTER_ARG("*Root=%p", *Root);
            return gcvSTATUS_TRUE;
        }
    }

    gcmFOOTER_ARG("*Root=%p status=%d", *Root, gcvSTATUS_NO_MORE_DATA);
    return gcvSTATUS_FALSE;
}

gceSTATUS
gcOpt_AddListToList(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_LIST       SrcList,
    IN gctBOOL          IsJump,
    IN OUT gcOPT_LIST * Root
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_LIST          list;

    gcmHEADER_ARG("Optimizer=%p SrcList=%p Root=%p", Optimizer, SrcList, Root);

    for (list = SrcList; list; list = list->next)
    {
        if (list->index < 0)
        {
            if (IsJump && list->index == gcvOPT_UNDEFINED_REGISTER)
            {
                gcmERR_BREAK(gcOpt_AddIndexToList(Optimizer, Root, gcvOPT_JUMPUNDEFINED_REGISTER));
            }
            else
            {
                gcmERR_BREAK(gcOpt_AddIndexToList(Optimizer, Root, list->index));
            }
        }
        else
        {
            gcmERR_BREAK(gcOpt_AddCodeToList(Optimizer, Root, list->code));
        }
    }

    /* Success. */
    gcmFOOTER_ARG("*Root=%p status=%d", *Root, status);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**                               gcOpt_InitializeTempArray
********************************************************************************
**
**    Initialize temp registers' isGlobal, usage, isIndex.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_InitializeTempArray(
    IN gcOPTIMIZER      Optimizer
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_TEMP          tempArray = Optimizer->tempArray;
    gcOPT_TEMP          temp;
    gctSOURCE_t         source;
    gctTARGET_t         target;
    gctUINT16           index;
    gcOPT_CODE          code;
    gctUINT             tempCount = Optimizer->tempCount;
    gctUINT             i;
    gctBOOL             needToCheckGlobal = (Optimizer->functionCount > 0);

    gcmHEADER_ARG("Optimizer=%p", Optimizer);

    /* Reset isGlobal, usage, isIndex, and temp. */
    temp = tempArray;

    if (tempCount > 0 && temp == gcvNULL)
    {
        gcmASSERT(temp);
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    for (i = 0; i < tempCount; i++, temp++)
    {
        temp->isGlobal = gcvFALSE;
        temp->usage = 0;
        temp->isIndex = gcvFALSE;
        temp->temp = (gctPOINTER) -1;
        temp->format = -1;
    }

    if (needToCheckGlobal)
    {
        if (Optimizer->outputCount > 0 && tempArray == gcvNULL)
        {
            gcmASSERT(tempArray);
            status = gcvSTATUS_INVALID_ARGUMENT;
            gcmFOOTER();
            return status;
        }

        /* Set output registers as global. */
        for (i = 0; i < Optimizer->outputCount; i++)
        {
            /* output could be NULL if it is not used */
            if (Optimizer->outputs[i] == gcvNULL)
                continue;

            tempArray[Optimizer->outputs[i]->tempIndex].isGlobal = gcvTRUE;
        }
    }

    /* Set usage and isGlobal. */
    for (code = Optimizer->codeHead; code; code = code->next)
    {
        gcSL_OPCODE opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->instruction.opcode, Opcode);

        switch (opcode)
        {
        case gcSL_STORE:
        case gcSL_IMAGE_WR:
        case gcSL_IMAGE_WR_3D:
        case gcSL_ATTR_ST:
            /* Skip store instructions. */
            temp = gcvNULL;
            break;

        default:
            if (gcSL_isOpcodeHaveNoTarget(opcode))
            {
                temp = gcvNULL;
                break;
            }

            if (opcode == gcSL_SET)
            {
                /* Skip specail SET.Z for select/cmp. */
                if (gcmSL_TARGET_GET(code->instruction.temp, Condition) == gcSL_ZERO)
                {
                    temp = gcvNULL;
                    break;
                }
            }

            /* Get gcSL_TARGET field. */
            target = code->instruction.temp;

            /* Get pointer to temporary register. */
            temp = tempArray + code->instruction.tempIndex;

            /* Update register usage. */
            temp->usage |= gcmSL_TARGET_GET(target, Enable);

            if (needToCheckGlobal && temp->function == gcvNULL && !temp->isGlobal)
            {
                /* Check if the register is global. */
                if (temp->temp != (gctPOINTER) code->function)
                {
                    if (temp->temp == (gctPOINTER) -1)
                    {
                        temp->temp = (gctPOINTER) code->function;
                    }
                    else
                    {
                        temp->isGlobal = gcvTRUE;
                    }
                }
            }
            break;
        }

        /* If isIndex is needed, comment out the next line. */
        if (!needToCheckGlobal) continue;

        /* Determine usage of source0. */
        source = code->instruction.source0;

        if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
        {
            /* Get pointer to temporary register. */
            temp = tempArray + code->instruction.source0Index;

            if (temp->function == gcvNULL && !temp->isGlobal)
            {
                /* Check if the register is global. */
                if (temp->temp != (gctPOINTER) code->function)
                {
                    if (temp->temp == (gctPOINTER) -1)
                    {
                        temp->temp = (gctPOINTER) code->function;
                    }
                    else
                    {
                        temp->isGlobal = gcvTRUE;
                    }
                }
            }
        }

        if (gcmSL_TARGET_GET(code->instruction.temp, Indexed) != gcSL_NOT_INDEXED)
        {
            index = code->instruction.tempIndexed;

            /* Get pointer to temporary register. */
            temp = tempArray + gcmSL_INDEX_GET(index, Index);

            /* Mark the temporary register is used as an index. */
            temp->isIndex = gcvTRUE;

            if (temp->function == gcvNULL && !temp->isGlobal)
            {
                /* Check if the register is global. */
                if (temp->temp != (gctPOINTER) code->function)
                {
                    if (temp->temp == (gctPOINTER) -1)
                    {
                        temp->temp = (gctPOINTER) code->function;
                    }
                    else
                    {
                        temp->isGlobal = gcvTRUE;
                    }
                }
            }
        }

        /* Determine usage of source1. */
        source = code->instruction.source1;

        if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
        {
            /* Get pointer to temporary register. */
            temp = tempArray + code->instruction.source1Index;

            if (temp->function == gcvNULL && !temp->isGlobal)
            {
                /* Check if the register is global. */
                if (temp->temp != (gctPOINTER) code->function)
                {
                    if (temp->temp == (gctPOINTER) -1)
                    {
                        temp->temp = (gctPOINTER) code->function;
                    }
                    else
                    {
                        temp->isGlobal = gcvTRUE;
                    }
                }
            }
        }

        if (gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
        {
            index = code->instruction.source1Indexed;

            /* Get pointer to temporary register. */
            temp = tempArray + gcmSL_INDEX_GET(index, Index);

            /* Mark the temporary register is used as an index. */
            temp->isIndex = gcvTRUE;

            if (temp->function == gcvNULL && !temp->isGlobal)
            {
                /* Check if the register is global. */
                if (temp->temp != (gctPOINTER) code->function)
                {
                    if (temp->temp == (gctPOINTER) -1)
                    {
                        temp->temp = (gctPOINTER) code->function;
                    }
                    else
                    {
                        temp->isGlobal = gcvTRUE;
                    }
                }
            }
        }
        if (opcode == gcSL_STORE || opcode == gcSL_ATTR_ST)
        {
            /* STORE's target is the value to be stored */

            temp = tempArray + code->instruction.tempIndex;

            if (temp->function == gcvNULL && !temp->isGlobal)
            {
                /* Check if the register is global. */
                if (temp->temp != (gctPOINTER) code->function)
                {
                    if (temp->temp == (gctPOINTER) -1)
                    {
                        temp->temp = (gctPOINTER) code->function;
                    }
                    else
                    {
                        temp->isGlobal = gcvTRUE;
                    }
                }
            }
            if (gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
            {
                index = code->instruction.source0Indexed;

                /* Get pointer to temporary register. */
                temp = tempArray + gcmSL_INDEX_GET(index, Index);

                /* Mark the temporary register is used as an index. */
                temp->isIndex = gcvTRUE;

                if (temp->function == gcvNULL && !temp->isGlobal)
                {
                    /* Check if the register is global. */
                    if (temp->temp != (gctPOINTER) code->function)
                    {
                        if (temp->temp == (gctPOINTER) -1)
                        {
                            temp->temp = (gctPOINTER) code->function;
                        }
                        else
                        {
                            temp->isGlobal = gcvTRUE;
                        }
                    }
                }
            }
        }
    }
    if (needToCheckGlobal)
    {
        /* Add global temp registers to global list. */
        temp = tempArray;
        for (i = 0; i < tempCount; i++, temp++)
        {
            if (temp->isGlobal)
            {
                gcOPT_LIST list;
                gcmERR_RETURN(_CAllocateList(Optimizer->listMemPool, &list));

                /* Add to global variable list. */
                list->index = i;
                list->next = Optimizer->global;
                Optimizer->global = list;
            }
        }
    }

    /* Initialize arrayVariable. */
    if (Optimizer->shader->variableCount > 0)
    {
        gctUINT variableCount = Optimizer->shader->variableCount;
        gctUINT v;

        for (v = 0; v < variableCount; v++)
        {
            gcVARIABLE variable = Optimizer->shader->variables[v];

            if (isVariableNormal(variable))
            {
                gctUINT size = 1;
                gctUINT j;
                gctINT i;

                for (i = 0; i < variable->arrayLengthCount; i++)
                {
                    size *= (gctUINT)(variable->arrayLengthList[i] <= 0 ? 1 : variable->arrayLengthList[i]);
                }

                size *= gcmType_Rows(variable->u.type);

                gcmASSERT(variable->tempIndex + size <= tempCount);
                temp = tempArray + variable->tempIndex;
                for (j = 0; j < size; j++, temp++)
                {
                    temp->arrayVariable = variable;
                    temp->precision = variable->precision;
                }
            }
        }
    }

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                               _BuildTempArray
********************************************************************************
**
**    Build optimizer's temp register array.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
*/
static gceSTATUS
_BuildTempArray(
    IN gcOPTIMIZER      Optimizer
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gctUINT             i, tempCount;
    gcOPT_TEMP          tempArray = gcvNULL;

    gcmHEADER_ARG("Optimizer=%p", Optimizer);

    /* Determine the number of temporary registers. */
    tempCount = gcSHADER_GetTempCount(Optimizer->shader);

    if (Optimizer->tempArray == gcvNULL)
    {
        if (tempCount > 0)
        {
            gcmERR_RETURN(_CAllocateTempArray(Optimizer->tempArrayMemPool, &tempArray, tempCount));
            Optimizer->tempArray = tempArray;
        }
        Optimizer->tempCount = tempCount;
    }
    /* Set function for function arguments. */
    for (i = 0; i < Optimizer->functionCount; i++)
    {
        gcOPT_FUNCTION function = Optimizer->functionArray + i;
        gcsFUNCTION_ARGUMENT_PTR argument;
        gctUINT j;

        if (function->argumentCount > 0 && tempArray == gcvNULL)
        {
            gcmASSERT(tempArray);
            status = gcvSTATUS_INVALID_ARGUMENT;
            gcmFOOTER();
            return status;
        }

        argument = function->arguments;
        for (j = 0; j < function->argumentCount; j++, argument++)
        {
            gctUINT index = argument->index;

            gcmASSERT(tempArray[index].function == gcvNULL);
            tempArray[index].function = function;
            tempArray[index].argument = argument;
            tempArray[index].precision = (gcSHADER_PRECISION)argument->precision;

            if (argument->variableIndex != 0xffff)
            {
                gcmASSERT(argument->variableIndex < Optimizer->shader->variableCount);
                tempArray[index].arrayVariable = Optimizer->shader->variables[argument->variableIndex];
                tempArray[index].precision = tempArray[index].arrayVariable->precision;
            }
        }
    }

    /* Initialize temp registers' flags. */
    gcmERR_RETURN(gcOpt_InitializeTempArray(Optimizer));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**                               gcOpt_ClearTempArray
********************************************************************************
**
**    Clear the lists in optimizer's temp register array.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
**
**        gcOPT_TEMP_DEFINE TempDefineArray
**            Pointer to a gcOPT_TEMP_DEFINE structure.
*/
gceSTATUS
gcOpt_ClearTempArray(
    IN gcOPTIMIZER          Optimizer,
    IN gcOPT_TEMP_DEFINE    TempDefineArray
    )
{
    gctUINT                 i;
    gcOPT_TEMP_DEFINE       tempDefine;

    gcmHEADER_ARG("Optimizer=%p TempDefineArray=%p", Optimizer, TempDefineArray);

    tempDefine = TempDefineArray;
    for (i = 0; i < Optimizer->tempCount; i++, tempDefine++)
    {
        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &tempDefine->xDefines));
        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &tempDefine->yDefines));
        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &tempDefine->zDefines));
        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &tempDefine->wDefines));
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**                               _DestroyTempArray
********************************************************************************
**
**    Destroy optimizer's temp register array.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
*/
static gceSTATUS
_DestroyTempArray(
    IN gcOPTIMIZER      Optimizer
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_TEMP          tempArray = Optimizer->tempArray;

    gcmHEADER_ARG("Optimizer=%p", Optimizer);

    if (tempArray == gcvNULL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    gcmVERIFY_OK(_FreeTempArray(Optimizer->tempArrayMemPool, Optimizer->tempArray));
    Optimizer->tempArray = gcvNULL;

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                               _BuildGlobalUsage
********************************************************************************
**
**    Build global usage for functions.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
*/
static gceSTATUS
_BuildGlobalUsage(
    IN gcOPTIMIZER      Optimizer
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_FUNCTION      functionArray = Optimizer->functionArray;
    gcOPT_FUNCTION      function;
    gcOPT_TEMP          tempArray = Optimizer->tempArray;
    gcOPT_TEMP          temp;
    gcOPT_LIST          list;
    gctSOURCE_t         source;
    gctUINT16           index;
    gcOPT_CODE          code;
    gctBOOL             updated;
    gctUINT             i;
    gctUINT8*           enableDefArray = gcvNULL;
    gctUINT8*           enableUseArray = gcvNULL;

    gcmHEADER_ARG("Optimizer=%p", Optimizer);

    if (Optimizer->global == gcvNULL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    if (Optimizer->functionCount > 0 &&
         tempArray == gcvNULL)
    {
        gcmASSERT(tempArray);
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    enableDefArray = (gctUINT8*)malloc(Optimizer->tempCount * sizeof(gctUINT8));
    enableUseArray = (gctUINT8*)malloc(Optimizer->tempCount * sizeof(gctUINT8));
    if(enableDefArray == gcvNULL || enableUseArray == gcvNULL)
    {
        gcmFOOTER();
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    function = functionArray;
    for (i = 0; i < Optimizer->functionCount; i++, function++)
    {
        /* Reset tempInt, used as direction. */
        for (list = Optimizer->global; list; list = list->next)
        {
            tempArray[list->index].tempInt = -1;
        }

        memset(enableDefArray, 0, Optimizer->tempCount * sizeof(gctUINT8));
        memset(enableUseArray, 0, Optimizer->tempCount * sizeof(gctUINT8));
        /* Set direction in tempInt. */
        for (code = function->codeHead; code != gcvNULL && code != function->codeTail->next; code = code->next)
        {
            gctUINT8 enable;
            gctUINT8 swizzlex;
            gctUINT8 swizzley;
            gctUINT8 swizzlez;
            gctUINT8 swizzlew;
            gcSL_OPCODE opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->instruction.opcode, Opcode);

            switch (opcode)
            {
            case gcSL_STORE1:
                /* Skip store1 instructions. */
                temp = gcvNULL;
                break;

            case gcSL_ATTR_ST:
                temp = gcvNULL;
                break;

            default:
                if (gcSL_isOpcodeHaveNoTarget(opcode))
                {
                    temp = gcvNULL;
                    break;
                }
                /* Get pointer to temporary register. */
                temp = tempArray + code->instruction.tempIndex;
                enable = gcmSL_TARGET_GET(code->instruction.temp, Enable);

                if (temp->isGlobal)
                {
                    if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_STORE &&
                        gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_IMAGE_WR &&
                        gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_IMAGE_WR_3D &&
                        gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_ATTR_ST)
                    {
                        if (temp->tempInt == -1)
                        {
                            temp->tempInt = gcvFUNCTION_OUTPUT;
                        }
                        else if (temp->tempInt == gcvFUNCTION_INPUT)
                        {
                            temp->tempInt = gcvFUNCTION_INOUT;
                        }
                        enableDefArray[code->instruction.tempIndex] |= enable;
                    }
                    else
                    {
                        if (temp->tempInt == -1)
                        {
                            temp->tempInt = gcvFUNCTION_INPUT;
                        }
                        else if (temp->tempInt == gcvFUNCTION_OUTPUT)
                        {
                            temp->tempInt = gcvFUNCTION_INOUT;
                        }
                        enableUseArray[code->instruction.tempIndex] |= enable;
                    }
                }
                break;
            }

            /* Determine usage of source0. */
            source = code->instruction.source0;
            swizzlex = gcmSL_SOURCE_GET(source, SwizzleX);
            swizzley = gcmSL_SOURCE_GET(source, SwizzleY);
            swizzlez = gcmSL_SOURCE_GET(source, SwizzleZ);
            swizzlew = gcmSL_SOURCE_GET(source, SwizzleW);
            enable = gcSL_ConvertSwizzle2Enable((gcSL_SWIZZLE)swizzlex,
                                                (gcSL_SWIZZLE)swizzley,
                                                (gcSL_SWIZZLE)swizzlez,
                                                (gcSL_SWIZZLE)swizzlew);

            if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
            {
                /* Get pointer to temporary register. */
                temp = tempArray + code->instruction.source0Index;

                if (temp->isGlobal)
                {
                    if (temp->tempInt == -1)
                    {
                        temp->tempInt = gcvFUNCTION_INPUT;
                    }
                    else if (temp->tempInt == gcvFUNCTION_OUTPUT)
                    {
                        temp->tempInt = gcvFUNCTION_INOUT;
                    }
                    enableUseArray[code->instruction.source0Index] |= enable;
                }
            }

            if (gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
            {
                index = code->instruction.source0Indexed;

                /* Get pointer to temporary register. */
                temp = tempArray + gcmSL_INDEX_GET(index, Index);

                /* Mark the temporary register is used as an index. */
                temp->isIndex = gcvTRUE;

                if (temp->isGlobal)
                {
                    if (temp->tempInt == -1)
                    {
                        temp->tempInt = gcvFUNCTION_INPUT;
                    }
                    else if (temp->tempInt == gcvFUNCTION_OUTPUT)
                    {
                        temp->tempInt = gcvFUNCTION_INOUT;
                    }
                    enableUseArray[gcmSL_INDEX_GET(index, Index)] |= enable;
                }
            }


            /* Determine usage of source1. */
            source = code->instruction.source1;
            swizzlex = gcmSL_SOURCE_GET(source, SwizzleX);
            swizzley = gcmSL_SOURCE_GET(source, SwizzleY);
            swizzlez = gcmSL_SOURCE_GET(source, SwizzleZ);
            swizzlew = gcmSL_SOURCE_GET(source, SwizzleW);
            enable = gcSL_ConvertSwizzle2Enable((gcSL_SWIZZLE)swizzlex,
                                                (gcSL_SWIZZLE)swizzley,
                                                (gcSL_SWIZZLE)swizzlez,
                                                (gcSL_SWIZZLE)swizzlew);

            if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
            {
                /* Get pointer to temporary register. */
                temp = tempArray + code->instruction.source1Index;

                if (temp->isGlobal)
                {
                    if (temp->tempInt == -1)
                    {
                        temp->tempInt = gcvFUNCTION_INPUT;
                    }
                    else if (temp->tempInt == gcvFUNCTION_OUTPUT)
                    {
                        temp->tempInt = gcvFUNCTION_INOUT;
                    }
                    enableUseArray[code->instruction.source1Index] |= enable;
                }

            }

            if (gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
            {
                index = code->instruction.source1Indexed;

                /* Get pointer to temporary register. */
                temp = tempArray + gcmSL_INDEX_GET(index, Index);

                /* Mark the temporary register is used as an index. */
                temp->isIndex = gcvTRUE;

                if (temp->isGlobal)
                {
                    if (temp->tempInt == -1)
                    {
                        temp->tempInt = gcvFUNCTION_INPUT;
                    }
                    else if (temp->tempInt == gcvFUNCTION_OUTPUT)
                    {
                        temp->tempInt = gcvFUNCTION_INOUT;
                    }
                    enableUseArray[gcmSL_INDEX_GET(index, Index)] |= enable;
                }
            }
        }

        /* Add global usage list to the function. */
        for (list = Optimizer->global; list; list = list->next)
        {
            temp = tempArray + list->index;

            if (temp->tempInt != -1)
            {
                gcOPT_GLOBAL_USAGE usage = gcvNULL;
                status = _CAllocateGlobalUsage(Optimizer->usageMemPool, &usage);
                if(status != gcvSTATUS_OK)
                {
                    free(enableDefArray);
                    free(enableUseArray);
                    gcmFOOTER();
                    return status;
                }
                usage->index = list->index;
                usage->direction = (gceINPUT_OUTPUT) temp->tempInt;
                usage->defEnable = enableDefArray[list->index];
                usage->useEnable = enableUseArray[list->index];
                usage->next = function->globalUsage;
                function->globalUsage = usage;
            }
        }
    }

    free(enableDefArray);
    free(enableUseArray);

    /* Propagate global usage for nested calling. */
    function = functionArray;
    for (i = 0; i < Optimizer->functionCount; i++, function++)
    {
        function->updated = gcvTRUE;
    }

    do
    {
        updated = gcvFALSE;
        function = functionArray;
        for (i = 0; i < Optimizer->functionCount; i++, function++)
        {
            gcOPT_LIST caller;
            gcOPT_FUNCTION callerFunction;
            gcOPT_GLOBAL_USAGE usage, callerUsage;

            if (! function->updated)
            {
                continue;
            }

            function->updated = gcvFALSE;
            code = function->codeHead;
            for (caller = code->callers; caller; caller = caller->next)
            {
                callerFunction = caller->code->function;
                if (callerFunction)
                {
                    /* Not main function, propagate and merge global usage. */
                    for (usage = function->globalUsage; usage; usage = usage->next)
                    {
                        for (callerUsage = callerFunction->globalUsage;
                             callerUsage;
                             callerUsage = callerUsage->next)
                        {
                            if (callerUsage->index == usage->index)
                            {
                                if (callerUsage->direction != usage->direction)
                                {
                                    if (callerUsage->direction != gcvFUNCTION_INOUT)
                                    {
                                        callerUsage->direction = gcvFUNCTION_INOUT;

                                        updated = callerFunction->updated = gcvTRUE;
                                    }
                                }
                                /* if not caller's subset, then update */
                                if((callerUsage->defEnable != (callerUsage->defEnable | usage->defEnable)) ||
                                   (callerUsage->useEnable != (callerUsage->useEnable | usage->useEnable)))
                                {
                                    callerUsage->defEnable |= usage->defEnable;
                                    callerUsage->useEnable |= usage->useEnable;
                                    updated = callerFunction->updated = gcvTRUE;
                                }
                                break;
                            }
                        }

                        if (callerUsage == gcvNULL)
                        {
                            /* Add global usage. */
                            gcmERR_RETURN(_CAllocateGlobalUsage(Optimizer->usageMemPool, &callerUsage));

                            callerUsage->index = usage->index;
                            callerUsage->direction = usage->direction;
                            callerUsage->defEnable = usage->defEnable;
                            callerUsage->useEnable = usage->useEnable;
                            callerUsage->next = callerFunction->globalUsage;
                            callerFunction->globalUsage = callerUsage;

                            updated = callerFunction->updated = gcvTRUE;
                        }
                    }
                }
            }
        }
    }
    while (updated);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_BuildCodeList(
    IN gcOPTIMIZER      Optimizer
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcSHADER            shader = Optimizer->shader;
    gcSL_INSTRUCTION    shaderCode = shader->code;
    gctUINT             codeCount = shader->codeCount + 1;
    gcOPT_CODE          code, codePrev, codeNext;
    gctUINT             i;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    /* Allocate an array for code list. */
    /* Allocate one more code for the extra RET for main. */
    gcmERR_RETURN(_CAllocateCodeArray(
        Optimizer->codeArrayMemPool,
        &Optimizer->codeHead, codeCount));

    /* Build code list. */
    codePrev = gcvNULL;
    code = Optimizer->codeHead;
    for (i = 0; i < codeCount; i++)
    {
        /* Initialization. */
        code->id            = i;
        code->function      = gcvNULL;
        code->callers       = gcvNULL;
        code->callee        = gcvNULL;
        code->tempDefine    = gcvNULL;
        code->dependencies0 = gcvNULL;
        code->dependencies1 = gcvNULL;
        code->users         = gcvNULL;
        code->prevDefines   = gcvNULL;
        code->nextDefines   = gcvNULL;
        code->ltcArrayIdx   = -1;

        /* Copy instruction. */
        if (i < codeCount - 1)
        {
            code->instruction   = *(shaderCode + i);
        }

        /* Set links. */
        code->prev          = codePrev;
        if (i == codeCount - 1)
        {
            codeNext            = gcvNULL;
        }
        else
        {
            codeNext            = code + 1;
        }
        code->next          = codeNext;

        /* Move to next instruction. */
        codePrev = code;
        code = codeNext;
    }

    /* Set the extra RET code. */
    gcoOS_ZeroMemory(&codePrev->instruction, gcvINSTR_SIZE);
    codePrev->instruction.opcode = gcSL_RET;
    codePrev->next = gcvNULL;
    Optimizer->codeTail = codePrev;

    /* Initialize caller and callee. */
    Optimizer->jmpCount = 0;
    for (code = Optimizer->codeHead; code; code = code->next)
    {
        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_CALL
        ||  gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_JMP)
        {
            /* Add caller and callee. */
            gctUINT jumpTarget = code->instruction.tempIndex;
            gcOPT_CODE codeTarget = Optimizer->codeHead + jumpTarget;

            gcmASSERT(codeTarget->id == jumpTarget);
            code->callee = codeTarget;
            if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_JMP && jumpTarget < code->id)
            {
                code->backwardJump = gcvTRUE;
            }

            Optimizer->jmpCount ++;

            gcmERR_RETURN(gcOpt_AddCodeToList(Optimizer, &codeTarget->callers, code));
        }
    }

    gcmFOOTER();
    return status;
}

void
gcOpt_UpdateCodeId(
    IN gcOPTIMIZER      Optimizer
    )
{
    gcOPT_CODE          code;
    gctUINT             i;

    for (i = 0, code = Optimizer->codeHead;
         /*i < Optimizer->codeCount &&*/ code;
         i++, code = code->next)
    {
        if (code->callers)
        {
            gcOPT_LIST list;

            for (list = code->callers; list; list = list->next)
            {
                gcOPT_CODE caller = list->code;
                gcmASSERT(gcmSL_OPCODE_GET(caller->instruction.opcode, Opcode) == gcSL_CALL
                       || gcmSL_OPCODE_GET(caller->instruction.opcode, Opcode) == gcSL_JMP);
                /* Cannot check this due to removeNOP. */
                /*gcmASSERT(caller->instruction.tempIndex == code->id);*/
                caller->instruction.tempIndex = (gctUINT16) i;
            }
        }

        code->id = i;
    }
    /*gcmASSERT(i == Optimizer->codeCount && code == gcvNULL);*/
}

gctBOOL
gcOpt_IsCodeBelongToFunc(
    IN gcOPTIMIZER       Optimizer,
    IN gcOPT_CODE        Code,
    OUT gcOPT_FUNCTION * Function
    )
{
    gctUINT              i;
    gcOPT_FUNCTION       function = gcvNULL;
    gcOPT_CODE           code;
    gctBOOL              found = gcvFALSE;

    for (i = 0; i < Optimizer->functionCount && !found; i++)
    {
        function = Optimizer->functionArray + i;

        if (function == gcvNULL) continue;

        for (code = function->codeHead; code != function->codeTail->next; code = code->next)
        {
            if (code == Code)
            {
                found = gcvTRUE;
                break;
            }
        }
    }

    if (Function && found)
    {
        *Function = function;
    }

    return found;
}

void
gcOpt_UpdateCodeFunction(
    IN gcOPTIMIZER       Optimizer
    )
{
    gcOPT_CODE           code;
    gcOPT_FUNCTION       function = gcvNULL;

    for (code = Optimizer->codeHead; code; code = code->next)
    {
        if (gcOpt_IsCodeBelongToFunc(Optimizer, code, &function))
        {
            code->function = function;
        }
        else
        {
            code->function = gcvNULL;
        }
    }

    return;
}

/*******************************************************************************
**                            gcOpt_MoveCodeListBefore
********************************************************************************
**
**    Move a piece of code to another location.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
**
**        gcOPT_CODE SrcCodeHead
**            Start point of instructions to be moved.
**
**        gcOPT_CODE SrcCodeTail
**            End point of instructions to be moved.
**
**        gcOPT_CODE DestCode
**            Destination code.
*/
gctBOOL
gcOpt_MoveCodeListBefore(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_CODE       SrcCodeHead,
    IN gcOPT_CODE       SrcCodeTail,
    IN gcOPT_CODE       DestCode
    )
{
    /* No need to go on if target is already before our ref */
    if (SrcCodeTail == DestCode->prev)
    {
        return gcvFALSE;
    }

    /* Unlink the list from the original place. */
    if (SrcCodeTail->next)
    {
        SrcCodeTail->next->prev = SrcCodeHead->prev;
    }
    else
    {
        gcmASSERT(Optimizer->codeTail == SrcCodeTail);
        Optimizer->codeTail = SrcCodeHead->prev;
    }
    if (SrcCodeHead->prev)
    {
        SrcCodeHead->prev->next = SrcCodeTail->next;
    }
    else
    {
        gcmASSERT(Optimizer->codeHead == SrcCodeHead);
        Optimizer->codeHead = SrcCodeTail->next;
    }

    /* Update function head */
    if (Optimizer->main->codeHead == SrcCodeHead)
    {
        Optimizer->main->codeHead = SrcCodeTail->next;
    }
    else if (SrcCodeHead->function && SrcCodeHead->function->codeHead == SrcCodeHead)
    {
        SrcCodeHead->function->codeHead = SrcCodeTail->next;
    }
    if (Optimizer->main->codeHead == DestCode)
    {
        Optimizer->main->codeHead = SrcCodeHead;
    }
    else if (DestCode->function && DestCode->function->codeHead == DestCode)
    {
        DestCode->function->codeHead = SrcCodeHead;
    }
    /* Update function tail */
    if (Optimizer->main->codeTail == SrcCodeTail)
    {
        Optimizer->main->codeTail = SrcCodeHead->prev;
    }
    else if (SrcCodeTail->function && SrcCodeTail->function->codeTail == SrcCodeTail)
    {
        SrcCodeTail->function->codeTail = SrcCodeHead->prev;
    }

    /* Link the list to destination place. */
    SrcCodeHead->prev = DestCode->prev;
    SrcCodeTail->next = DestCode;

    /* Update destination links. */
    if (DestCode->prev)
    {
        DestCode->prev->next = SrcCodeHead;
    }
    else
    {
        gcmASSERT(Optimizer->codeHead == DestCode);
        Optimizer->codeHead = SrcCodeHead;
    }

    DestCode->prev = SrcCodeTail;

    return gcvTRUE;
}

void
gcOpt_MoveCodeListAfter(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_CODE       SrcCodeHead,
    IN gcOPT_CODE       SrcCodeTail,
    IN gcOPT_CODE       DestCode
    )
{
    /* Unlink the list from the original place. */
    if (SrcCodeTail->next)
    {
        SrcCodeTail->next->prev = SrcCodeHead->prev;
    }
    else
    {
        gcmASSERT(Optimizer->codeTail == SrcCodeTail);
        Optimizer->codeTail = SrcCodeHead->prev;
    }
    if (SrcCodeHead->prev)
    {
        SrcCodeHead->prev->next = SrcCodeTail->next;
    }
    else
    {
        gcmASSERT(Optimizer->codeHead == SrcCodeHead);
        Optimizer->codeHead = SrcCodeTail->next;
    }

    /* Link the list to destination place. */
    SrcCodeHead->prev = DestCode;
    SrcCodeTail->next = DestCode->next;

    /* Update destination links. */
    if (DestCode->next)
    {
        DestCode->next->prev = SrcCodeTail;
    }
    else
    {
        gcmASSERT(Optimizer->codeTail == DestCode);
        Optimizer->codeTail = SrcCodeTail;
    }
    DestCode->next = SrcCodeHead;
}

/*******************************************************************************
**                            gcOpt_AddCodeAfter
********************************************************************************
**
**    Add a piece of new code after the Code location.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
**
**        gcOPT_CODE Code
**            Start point of code to be added after.
**
**        gcOPT_CODE* NewCode
**            Pointer to newly added code.
*/
gceSTATUS
gcOpt_AddCodeAfter(
    IN  gcOPTIMIZER     Optimizer,
    IN  gcOPT_CODE      Code,
    OUT gcOPT_CODE*     NewCodePtr
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_CODE          newCode;
    gcmHEADER_ARG("Optimizer=0x%x Code=0x%x NewCode=0x%x",
                   Optimizer, Code, NewCodePtr);
    gcmERR_RETURN(_CAllocateCode(Optimizer->codeMemPool, &newCode));

    newCode->function      = Code->function;
    newCode->callers       = gcvNULL;
    newCode->callee        = gcvNULL;
    newCode->tempDefine    = gcvNULL;
    newCode->dependencies0 = gcvNULL;
    newCode->dependencies1 = gcvNULL;
    newCode->users         = gcvNULL;
    newCode->prevDefines   = gcvNULL;
    newCode->nextDefines   = gcvNULL;

    if (Code == Optimizer->codeTail)
    {
         Optimizer->codeTail = newCode;
         newCode->next       = gcvNULL;
         newCode->prev       = Code;
         Code->next          = newCode;
    }
    else {
        /* Set links between dummyTexld and code */
        newCode->prev          = Code;
        newCode->next          = Code->next;

        /* Set links. */
        Code->next             = newCode;
        newCode->next->prev    = newCode;
    }

    *NewCodePtr            = newCode;

     Optimizer->codeCount++;

    /* Return the status. */
    gcmFOOTER();
    return status;
}



/*******************************************************************************
**                            gcOpt_AddCodeBefore
********************************************************************************
**
**    Add a piece of new code before the Code location.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
**
**        gcOPT_CODE Code
**            Start point of code to be added after.
**
**        gcOPT_CODE* NewCode
**            Pointer to newly added code.
*/
gceSTATUS
gcOpt_AddCodeBefore(
    IN  gcOPTIMIZER     Optimizer,
    IN  gcOPT_CODE      Code,
    OUT gcOPT_CODE*     NewCodePtr
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_CODE          newCode;
    gcOPT_LIST          callerList;

    gcmHEADER_ARG("Optimizer=0x%x Code=0x%x NewCode=0x%x",
                   Optimizer, Code, NewCodePtr);
    gcmERR_RETURN(_CAllocateCode(Optimizer->codeMemPool, &newCode));

    newCode->function      = Code->function;
    newCode->callers       = gcvNULL;
    newCode->callee        = gcvNULL;
    newCode->tempDefine    = gcvNULL;
    newCode->dependencies0 = gcvNULL;
    newCode->dependencies1 = gcvNULL;
    newCode->users         = gcvNULL;
    newCode->prevDefines   = gcvNULL;
    newCode->nextDefines   = gcvNULL;

    if (Code == Optimizer->codeHead)
    {
         Optimizer->codeHead = newCode;
         newCode->prev = gcvNULL;
         newCode->next = Code;
         Code->prev    = newCode;
    }
    else {
        /* Set links between dummyTexld and code */
        newCode->prev          = Code->prev;
        newCode->next          = Code;

        /* Set links. */
        Code->prev             = newCode;
        newCode->prev->next    = newCode;
    }

    /* If the code is the head of one function, we need to change the callers and callee.*/
    if (Code->function && Code == Code->function->codeHead)
    {
        Code->function->codeHead = newCode;
        callerList = Code->callers;

        while(callerList)
        {
            gcmVERIFY_OK(gcOpt_AddCodeToList(Optimizer, &newCode->callers, callerList->code));
            callerList->code->callee = newCode;
            callerList = callerList->next;
        }

        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &Code->callers));
    }
    else if (Code->function == gcvNULL && Code == Optimizer->main->codeHead)
    {
        Optimizer->main->codeHead = newCode;

        callerList = Code->callers;

        while(callerList)
        {
            gcmVERIFY_OK(gcOpt_AddCodeToList(Optimizer, &newCode->callers, callerList->code));
            callerList->code->callee = newCode;
            callerList = callerList->next;
        }

        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &Code->callers));
    }

    *NewCodePtr            = newCode;



     Optimizer->codeCount++;

    /* Return the status. */
    gcmFOOTER();
    return status;
}



/*******************************************************************************
**                            gcOpt_CopyCodeListAfter
********************************************************************************
**
**    Copy a piece of code to another location.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
**
**        gcOPT_CODE SrcCodeHead
**            Start point of code to be moved.
**
**        gcOPT_CODE SrcCodeTail
**            End point of code to be moved.
**
**        gcOPT_CODE DestCode
**            Destination code.
*/
gceSTATUS
gcOpt_CopyCodeListAfter(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_CODE       SrcCodeHead,
    IN gcOPT_CODE       SrcCodeTail,
    IN gcOPT_CODE       DestCode
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_CODE          srcCode, code = gcvNULL;
    gcOPT_CODE          codeNext = DestCode->next;
    gcOPT_CODE          srcCodePrev = SrcCodeHead->prev;
    gcOPT_CODE          destCodeNext = DestCode->next;
    gcOPT_CODE          codePrev;

    gcmHEADER_ARG("Optimizer=0x%x SrcCodeHead=0x%x SrcCodeTail=0x%x DestCode=0x%x",
                   Optimizer, SrcCodeHead, SrcCodeTail, DestCode);

    /* The following update is done in gcOpt_ExpandFunctions temporarily to avoid assertion. */
    /* Need to update code id for checking. */
    /*gcOpt_UpdateCodeId(Optimizer);*/

    /* Copy code. */
    /* Use prev pointers as temp storage to link srcCode and code. */
    for (srcCode = SrcCodeTail; srcCode != gcvNULL && srcCode != srcCodePrev; srcCode = codePrev)
    {
        codePrev = srcCode->prev;
        if (Optimizer->freeCodeList)
        {
            code = Optimizer->freeCodeList;
            Optimizer->freeCodeList = code->next;
        }
        else
        {
            /* Allocate a new gcOPT_CODE structure. */
            gcmERR_RETURN(_CAllocateCode(Optimizer->codeMemPool, &code));
        }

        code->function      = DestCode->function;
        /*code->callers       = gcvNULL;
        code->callee        = gcvNULL;
        code->tempDefine    = gcvNULL;
        code->dependencies0 = gcvNULL;
        code->dependencies1 = gcvNULL;
        code->users         = gcvNULL;
        code->prevDefines   = gcvNULL;
        code->nextDefines   = gcvNULL;*/
        code->instruction   = srcCode->instruction;

        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_CALL)
        {
            /* Function definition should not be part of copied code. */
            gcmASSERT(srcCode->callee->id < SrcCodeHead->id || srcCode->callee->id > SrcCodeTail->id);
            code->callee = srcCode->callee;
            gcmERR_RETURN(gcOpt_AddCodeToList(Optimizer, &srcCode->callee->callers, code));
        }

        /* Set temp links between srcCode and code using prev. */
        srcCode->prev = code;
        code->prev = srcCode;

        /* Set links. */
        code->next          = codeNext;
        codeNext            = code;
    }
    DestCode->next = code;

    /* Set callers and callee for JUMP instructions. */
    for (code = DestCode->next; code != gcvNULL&&code != destCodeNext; code = code->next)
    {
        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_JMP)
        {
            srcCode = code->prev;
            if (srcCode->callee->id >= SrcCodeHead->id && srcCode->callee->id <= SrcCodeTail->id)
            {
                /* Target is inside the copied code--add code to the real callee's callers list. */
                code->callee = srcCode->callee->prev;
                gcmERR_RETURN(gcOpt_AddCodeToList(Optimizer, &code->callee->callers, code));
            }
            else
            {
                /* Target is outside the copied code--add code to callee's callers list. */
                code->callee = srcCode->callee;
                gcmERR_RETURN(gcOpt_AddCodeToList(Optimizer, &code->callee->callers, code));
            }
        }
    }

    /* Restore prev pointers back. */
    codePrev = srcCodePrev;
    for (code = SrcCodeHead; code != gcvNULL && code != SrcCodeTail->next; code = code->next)
    {
        code->prev = codePrev;
        codePrev = code;
    }

    codePrev = DestCode;
    for (code = DestCode->next; code != gcvNULL && code != destCodeNext; code = code->next)
    {
        code->prev = codePrev;
        codePrev = code;
    }
    if (destCodeNext)
    {
        destCodeNext->prev = codePrev;
    }

    /* Note that no housekeeping work done here. */
    /* The caller is responsible to update dependencies. */

    gcmFOOTER();
    return gcvSTATUS_OK;
}

gceSTATUS
gcOpt_RemoveCodeList(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_CODE       CodeHead,
    IN gcOPT_CODE       CodeTail
    )
{
    gcOPT_CODE          code;

    gcmHEADER_ARG("Optimizer=0x%x CodeHead=0x%x CodeTail=0x%x",
                   Optimizer, CodeHead, CodeTail);

    /* Unlink the list from the original place. */
    if (CodeHead->prev)
    {
        CodeHead->prev->next = CodeTail->next;
    }
    else
    {
        gcmASSERT(Optimizer->codeHead == CodeHead);
        Optimizer->codeHead = CodeTail->next;
    }
    if (CodeTail->next)
    {
        CodeTail->next->prev = CodeHead->prev;
    }
    else
    {
        gcmASSERT(Optimizer->codeTail == CodeTail);
        Optimizer->codeTail = CodeHead->prev;
    }

    /* Remove reference from callers. */
    for (code = CodeHead; code != gcvNULL && code != CodeTail->next; code = code->next)
    {
        if (code->callee &&
            code->callee->callers &&
            (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_CALL ||
             gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_JMP ||
             gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_JMP_ANY))
        {
            gcOPT_LIST list;
            gcOPT_LIST prevList = gcvNULL;
            gceSTATUS  status;

            for (list = code->callee->callers; list; list = list->next)
            {
                if (list->code == code)
                {
                    break;
                }
                prevList = list;
            }

            if (! list)
            {
                gcmASSERT(list);
                status = gcvSTATUS_INVALID_ARGUMENT;
                gcmFOOTER();
                return status;
            }

            if (prevList)
            {
                prevList->next = list->next;
            }
            else
            {
                code->callee->callers = list->next;
            }

            /* Free the gcOPT_LIST structure. */
            gcmVERIFY_OK(_FreeList(Optimizer->listMemPool, list));
        }
    }

    /* Add the list to the freeCodeList. */
    CodeTail->next = Optimizer->freeCodeList;
    Optimizer->freeCodeList = CodeHead;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**                               _BuildFunctionArray
********************************************************************************
**
**    Build optimizer's function array.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
*/
static gceSTATUS
_BuildFunctionArray(
    IN gcOPTIMIZER      Optimizer
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcSHADER            shader = Optimizer->shader;
    gcOPT_FUNCTION      functionArray;
    gcOPT_FUNCTION      function;
    gcFUNCTION          shaderFunction;
    gcKERNEL_FUNCTION   kernelFunction;
    gctUINT             i, j, k;
    gcOPT_CODE          codeHead = Optimizer->codeHead;
    gcOPT_CODE          code;
    gcKERNEL_FUNCTION * kernelFunctions = shader->kernelFunctions;

    gcmHEADER_ARG("Optimizer=%p", Optimizer);

    gcmASSERT(Optimizer->main == gcvNULL);

    /* Allocate main program data structure. */
    if (Optimizer->main == gcvNULL)
    {
        gcmERR_RETURN(_CAllocateFunctionArray(Optimizer->functionArrayMemPool, &Optimizer->main, 1));
    }

    gcmASSERT(Optimizer->functionArray == gcvNULL);

    if (Optimizer->isMainMergeWithKerenel)
    {
        /* Find the kernel merged with main (after inlining ) */
        for (i = 0; i < shader->kernelFunctionCount; i++)
        {
            if (kernelFunctions[i]->isMain)
            {
                Optimizer->main->kernelFunction = kernelFunctions[i];
                break;
            }
        }
    }

    if (Optimizer->functionCount == 0)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    /* Allocate function array data structure. */
    gcmERR_RETURN(_CAllocateFunctionArray(Optimizer->functionArrayMemPool, &functionArray, Optimizer->functionCount));

    Optimizer->functionArray = functionArray;

    /* Initialize functionArray's data struture. */
    function = functionArray;

    /* Merge both function array. */
    i = j = 0;
    shaderFunction = shader->functionCount ? shader->functions[0] : gcvNULL;
    kernelFunction = shader->kernelFunctionCount ? shader->kernelFunctions[0] : gcvNULL;
    for (k = 0; k < Optimizer->functionCount; k++, function++)
    {
        if (j >= shader->kernelFunctionCount
        ||  (i < shader->functionCount  && shaderFunction->codeStart < kernelFunction->codeStart))
        {
            function->shaderFunction = shaderFunction;
            function->argumentCount = shaderFunction->argumentCount;
            function->arguments = shaderFunction->arguments;
            function->globalUsage = gcvNULL;

            function->maxDepthValue = -1;
            function->maxDepthFunc = gcvNULL;
            function->hasEmitCode = gcvFALSE;

            /* At beginning, codeHead is sequential in an array. */
            function->codeHead = codeHead + shaderFunction->codeStart;
            function->codeTail = codeHead + shaderFunction->codeStart + shaderFunction->codeCount - 1;

            /* Set function pointer. */
            for (code = function->codeHead;
                 code != gcvNULL && code != function->codeTail->next;
                 code = code->next)
            {
                code->function = function;
            }

            /* Move to next shader function. */
            i++;
            if (i < shader->functionCount)
            {
                shaderFunction = shader->functions[i];
            }
        }
        else
        {
            gcmASSERT(i >= shader->functionCount ||
                      shaderFunction->codeStart > kernelFunction->codeStart);

            if (kernelFunction->isMain)
            {
                /* Skip the kernel function that is merged into main. */
                if (Optimizer->main->kernelFunction == gcvNULL)
                {
                    Optimizer->main->kernelFunction = kernelFunction;
                }
                else
                {
                    gcmASSERT(Optimizer->main->kernelFunction == kernelFunction);
                }
                k--;
                function--;
            }
            else
            {
                function->kernelFunction = kernelFunction;
                function->argumentCount = kernelFunction->argumentCount;
                function->arguments = kernelFunction->arguments;
                function->globalUsage = gcvNULL;

                function->maxDepthValue = -1;
                function->maxDepthFunc = gcvNULL;
                function->hasEmitCode = gcvFALSE;

                /* At beginning, codeHead is sequential in an array. */
                function->codeHead = codeHead + kernelFunction->codeStart;
                function->codeTail = codeHead + kernelFunction->codeEnd - 1;

                /* Set function pointer. */
                for (code = function->codeHead;
                     code != gcvNULL && code != function->codeTail->next;
                     code = code->next)
                {
                    code->function = function;
                }
            }

            /* Move to next shader function. */
            j++;
            if (j < shader->kernelFunctionCount)
            {
                kernelFunction = shader->kernelFunctions[j];
            }
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**                               _DestroyFunctionArray
********************************************************************************
**
**    Destroy optimizer's function array.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
*/
static gceSTATUS
_DestroyFunctionArray(
    IN gcOPTIMIZER      Optimizer
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_TEMP          tempArray = Optimizer->tempArray;
    gcOPT_FUNCTION      functionArray = Optimizer->functionArray;
    gcOPT_FUNCTION      function;

    gcmHEADER_ARG("Optimizer=%p", Optimizer);

    if (Optimizer->main)
    {
        gcmVERIFY_OK(_FreeFunctionArray(Optimizer->functionArrayMemPool, Optimizer->main));

        Optimizer->main = gcvNULL;
    }

    if (functionArray == gcvNULL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    for (function = functionArray + Optimizer->functionCount - 1;
         function >= functionArray;
         function--)
    {
        /* Free global variable usage list. */
        while (function->globalUsage)
        {
            gcOPT_GLOBAL_USAGE usage = function->globalUsage;
            function->globalUsage = usage->next;
            gcmVERIFY_OK(_FreeGlobalUsage(Optimizer->usageMemPool, usage));
        }

        if (tempArray)
        {
            gcsFUNCTION_ARGUMENT_PTR argument;
            gctUINT i;

            /* Update temps' function for function arguments. */
            argument = function->arguments;
            for (i = 0; i < function->argumentCount; i++, argument++)
            {
                gctUINT index = argument->index;

                gcmASSERT(tempArray[index].function == function);
                tempArray[index].function = gcvNULL;
                tempArray[index].argument = gcvNULL;
            }
        }
    }

    gcmVERIFY_OK(_FreeFunctionArray(Optimizer->functionArrayMemPool, functionArray));

    Optimizer->functionArray = gcvNULL;

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                               gcOpt_DeleteFunction
********************************************************************************
**
**    Delete one function from optimizer's function array.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
**
**        gcOPT_FUNCTION Function
**            Pointer to a gcOPT_FUNCTION structure to be deleted.
**
**        gctBOOL        RebiuldDF
**            rebuild data flow if true, otherwise delay the rebuild
**
*/
gceSTATUS
gcOpt_DeleteFunction(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_FUNCTION   Function,
    IN gctBOOL          RebiuldDF,
    IN gctBOOL          DeleteVariable
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_CODE          code;
    gcOPT_TEMP          tempArray = Optimizer->tempArray;
    gcOPT_FUNCTION      functionArray = Optimizer->functionArray;
    gctUINT             fIndex = (gctINT) (Function - functionArray);
    gctUINT             i, j;
    gcsFUNCTION_ARGUMENT_PTR argument;

    gcmHEADER_ARG("Optimizer=%p Function=%p", Optimizer, Function);

    gcmASSERT(functionArray);
    gcmASSERT(fIndex < Optimizer->functionCount);

    /* Update temps' function for function arguments. */
    argument = Function->arguments;
    for (j = 0; j < Function->argumentCount; j++, argument++)
    {
        gctUINT index = argument->index;
        gcVARIABLE variable = gcvNULL;

        gcmASSERT(tempArray[index].function == Function);
        tempArray[index].function = gcvNULL;
        tempArray[index].argument = gcvNULL;

        if (argument->variableIndex != 0xffff)
        {
            gcSHADER_GetVariable(Optimizer->shader, argument->variableIndex, &variable);
            if (variable != gcvNULL && DeleteVariable)
                SetVariableIsNotUsed(variable);
        }
    }

    /* Free global variable usage list. */
    while (Function->globalUsage)
    {
        gcOPT_GLOBAL_USAGE usage = Function->globalUsage;
        Function->globalUsage = usage->next;
        gcmVERIFY_OK(_FreeGlobalUsage(Optimizer->usageMemPool, usage));
    }

    /* Remove the function. */
    if (Function->codeHead)
    {
        gcOpt_RemoveCodeList(Optimizer, Function->codeHead, Function->codeTail);
    }

    for (j = fIndex; j < Optimizer->functionCount - 1; j++)
    {
        gcsFUNCTION_ARGUMENT_PTR argument;

        functionArray[j] = functionArray[j + 1];

        /* Update temps' function for function arguments. */
        argument = functionArray[j].arguments;
        for (i = 0; i < functionArray[j].argumentCount; i++, argument++)
        {
            gctUINT index = argument->index;

            gcmASSERT(tempArray[index].function == functionArray + j + 1);
            tempArray[index].function = functionArray + j;
            tempArray[index].argument = argument;
            tempArray[index].precision = (gcSHADER_PRECISION)argument->precision;

            if (argument->variableIndex != 0xffff)
            {
                gcmASSERT(argument->variableIndex < Optimizer->shader->variableCount);
                tempArray[index].arrayVariable = Optimizer->shader->variables[argument->variableIndex];
                tempArray[index].precision = tempArray[index].arrayVariable->precision;
            }
        }

        /* Update code's and callee's function */
        for (code = Optimizer->codeHead; code; code = code->next)
        {
            if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_CALL)
            {
                if (code->callee->function == &functionArray[j + 1])
                    code->callee->function = &functionArray[j];
            }

            if (code->function == &functionArray[j + 1])
                code->function = &functionArray[j];
        }
    }
    Optimizer->functionArray[Optimizer->functionCount - 1].shaderFunction = gcvNULL;
    Optimizer->functionCount--;
    if (Optimizer->functionCount == 0)
    {
        /* Free the empty function array. */
        gcmVERIFY_OK(_FreeFunctionArray(Optimizer->functionArrayMemPool, functionArray));
        Optimizer->functionArray = gcvNULL;
    }

    if (RebiuldDF)
    {
        /* Rebuild flow graph. */
        gcmVERIFY_OK(gcOpt_RebuildFlowGraph(Optimizer));
    }
    else
    {
        gcOpt_UpdateCodeId(Optimizer);
    }

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                            gcOpt_CopyInShader
********************************************************************************
**
**    Copy shader data into optimizer, and add an extra instruction, RET, at the
**  end of main program to avoid ambiguity and memory corruption (ABW and ABR).
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
**
**        gcSHADER Shader
**            Pointer to a gcSHADER object holding information about the compiled shader.
*/
gceSTATUS
gcOpt_CopyInShader(
    IN gcOPTIMIZER      Optimizer,
    IN gcSHADER         Shader
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcFUNCTION *        shaderFunctions = Shader->functions;
    gctBOOL             mainLast = gcvTRUE;

    gcmHEADER_ARG("Optimizer=%p Shader=%p", Optimizer, Shader);

    /*  Sort shader's functions and locate the main program. */
    if (Shader->functionCount > 0)
    {
        gctINT i, j;
        gcFUNCTION function;

        /* Check and sort functions to make sure they are in order. */
        for (i = Shader->functionCount - 1; i > 0; i--)
        {
            gctBOOL functionInOrder = gcvTRUE;
            for (j = 0; j < i; j++)
            {
                if (shaderFunctions[j]->codeStart > shaderFunctions[j + 1]->codeStart)
                {
                    /* Out of order.  Swap it. */
                    gcFUNCTION temp = shaderFunctions[j];
                    gctUINT tempLabel = temp->label;
                    gctUINT tempLabel1 = shaderFunctions[j + 1]->label;
                    shaderFunctions[j] = shaderFunctions[j + 1];
                    shaderFunctions[j + 1] = temp;
                    shaderFunctions[j]->label = (gctUINT16) tempLabel;
                    shaderFunctions[j + 1]->label = (gctUINT16) tempLabel1;
                    functionInOrder = gcvFALSE;
                }
            }
            if (functionInOrder) break;
        }

        function = Shader->functions[Shader->functionCount - 1];
        if (function->codeStart + function->codeCount == Shader->codeCount)
        {
            mainLast = gcvFALSE;
        }
    }

    /*  Sort shader's functions and locate the main program. */
    if (Shader->kernelFunctionCount > 0)
    {
        gctINT i, j;
        gcKERNEL_FUNCTION * kernelFunctions = Shader->kernelFunctions;

        /* check if the kernel is merged with main (after inlining ) */
        for (i = 0; i < (gctINT)Shader->kernelFunctionCount; i++)
        {
            if (kernelFunctions[i]->isMain)
            {
                Optimizer->isMainMergeWithKerenel = gcvTRUE;
            }
        }

        /* Check and sort functions to make sure they are in order. */
        for (i = Shader->kernelFunctionCount - 1; i > 0; i--)
        {
            gctBOOL functionInOrder = gcvTRUE;
            for (j = 0; j < i; j++)
            {
                if (kernelFunctions[j]->codeStart > kernelFunctions[j + 1]->codeStart)
                {
                    /* Out of order.  Swap it. */
                    gcKERNEL_FUNCTION temp = kernelFunctions[j];
                    gctUINT tempLabel = temp->label;
                    gctUINT tempLabel1 = kernelFunctions[j + 1]->label;
                    kernelFunctions[j] = kernelFunctions[j + 1];
                    kernelFunctions[j + 1] = temp;
                    kernelFunctions[j]->label = (gctUINT16) tempLabel;
                    kernelFunctions[j + 1]->label = (gctUINT16) tempLabel1;
                    functionInOrder = gcvFALSE;
                }
            }
            if (functionInOrder) break;
        }

        if (mainLast)
        {
            gcKERNEL_FUNCTION kernelFunction = Shader->kernelFunctions[Shader->kernelFunctionCount - 1];
            if (kernelFunction->codeEnd == Shader->codeCount && !kernelFunction->isMain)
            {
                mainLast = gcvFALSE;
            }
        }
    }

    /* Initialize Optimizer. */
    Optimizer->shader        = Shader;
    Optimizer->functionCount = Shader->functionCount + Shader->kernelFunctionCount;
    Optimizer->outputCount   = Shader->outputCount;
    Optimizer->outputs       = Shader->outputs;

    /* adjust function count to exclude the kernel which is merged with main */
    if (Optimizer->isMainMergeWithKerenel)
    {
        Optimizer->functionCount--;
    }

    /* Copy codes/instructions from shader to code list. */
    gcmERR_RETURN(_BuildCodeList(Optimizer));

    /* Allocate main and function array data structure. */
    gcmERR_RETURN(_BuildFunctionArray(Optimizer));

    if (mainLast)
    {
        /* Set main's codeHead and codeTail. */
        if (Optimizer->functionCount == 0 ||
            (Optimizer->functionCount == 1 &&  /* only one kernel which is already merged with main */
             Optimizer->functionArray[0].kernelFunction != gcvNULL &&
             Optimizer->functionArray[0].kernelFunction->isMain))
        {
            Optimizer->main->codeHead = Optimizer->codeHead;
        }
        else
        {
            Optimizer->main->codeHead = Optimizer->functionArray[Optimizer->functionCount - 1].codeTail->next;
        }
        Optimizer->main->codeTail = Optimizer->codeTail;
    }
    else
    {
        gctUINT i;
        gcOPT_FUNCTION functionArray = Optimizer->functionArray;
        gcOPT_CODE codeRET;
        gcOPT_CODE codeNext;
        gcOPT_CODE code;

        /* Locate the last instruction of main program. */
        for (i = Optimizer->functionCount - 1; i > 0; i--)
        {
            if (functionArray[i].codeHead != functionArray[i - 1].codeTail->next)
                break;
        }

        /* Move the RET instruction from last to after main's last instruction. */
        codeRET  = Optimizer->codeTail;
        codeNext = functionArray[i].codeHead;

        /* Unlink codeRET from the end of codeHead. */
        codeRET->prev->next = gcvNULL;
        Optimizer->codeTail = codeRET->prev;

        /* Insert codeRET to before codeNext. */
        codeRET->next = codeNext;
        codeRET->prev = codeNext->prev;
        if (codeNext->prev)
            codeNext->prev->next = codeRET;
        codeNext->prev = codeRET;

        /* Set main's codeHead and codeTail. */
        if (i == 0)
        {
            if(codeRET->prev)
            {
                Optimizer->main->codeHead = Optimizer->codeHead;
            }
            else    /* empty main */
            {
                Optimizer->main->codeHead = codeRET;
                Optimizer->codeHead = codeRET;
            }
        }
        else
        {
            Optimizer->main->codeHead = functionArray[i - 1].codeTail->next;
        }
        Optimizer->main->codeTail = codeRET;

        /* Update code's id from codeRET. */
        codeRET->id = codeNext->id;
        for (code = codeNext; code; code = code->next)
        {
            if (code->callers)
            {
                gcOPT_LIST list = code->callers;
                gcOPT_LIST list_next = gcvNULL;
                while (list)
                {
                    gcOPT_CODE caller = list->code;
                    list_next = list->next;
                    gcmASSERT(gcmSL_OPCODE_GET(caller->instruction.opcode, Opcode) == gcSL_CALL
                           || gcmSL_OPCODE_GET(caller->instruction.opcode, Opcode) == gcSL_JMP);
                    gcmASSERT(caller->instruction.tempIndex == code->id);

                    /* If in main function, there is a jmp out of main function,
                    ** we need to change the callee to codeRET.
                    */
                    if (caller->instruction.opcode == gcSL_JMP &&
                        caller->function == gcvNULL &&
                        code == codeNext &&
                        caller->instruction.tempIndex == codeRET->id)
                    {
                        gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &caller->callee->callers, caller));
                        gcmVERIFY_OK(gcOpt_AddCodeToList(Optimizer, &codeRET->callers, caller));
                        caller->callee = codeRET;
                        list = list_next;
                        continue;
                    }

                    caller->instruction.tempIndex++;

                    list = list_next;
                }
            }
            code->id++;
        }
    }

    /* Shader's labels should be empty. */
    gcmASSERT(Shader->labels == gcvNULL);

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**                            gcOpt_CopyOutShader
********************************************************************************
**
**    Copy shader data from optimizer back to shader, and change the extra RET
**  instruction to NOP instruction.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
**
**        gcSHADER Shader
**            Pointer to a gcSHADER object holding information about the compiled shader.
*/
gceSTATUS
gcOpt_CopyOutShader(
    IN gcOPTIMIZER      Optimizer,
    IN gcSHADER         Shader
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcoOS               os = gcvNULL;
    gctUINT             codeCount = Optimizer->codeTail->id + 1;
    gcOPT_CODE          srcCode;
    gcSL_INSTRUCTION    code;
    gctUINT             pc;
    gctUINT             i;

    gcmHEADER_ARG("Optimizer=%p Shader=%p", Optimizer, Shader);

    if (Shader->codeCount != codeCount)
    {
        gctPOINTER pointer = gcvNULL;
        gctSIZE_T Bytes;

        /* Free the old code array. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(os, Shader->code));

        /* Allocate new code array for the shader. */
        Bytes = (gctSIZE_T)((gctSIZE_T)codeCount * gcvINSTR_SIZE);
        gcmERR_RETURN(gcoOS_Allocate(os,
                            Bytes,
                            &pointer));

        /* Switch over to new code array. */
        Shader->code = pointer;
        Shader->codeCount = codeCount;
        Shader->lastInstruction = codeCount;
    }

    code = Shader->code;
    for (srcCode = Optimizer->codeHead; srcCode; srcCode = srcCode->next, code++)
    {
        *code = srcCode->instruction;
    }

    /* Copy the function data back. */
    if (Optimizer->functionCount == 0)
    {
        if (Shader->functionCount > 0)
        {
            /* All functions are expanded, so delete the functions. */
            for (i = 0; i < Shader->functionCount; i++)
            {
                gcFUNCTION shaderFunction = Shader->functions[i];

                if (shaderFunction->arguments)
                {
                    /* Free argument array. */
                    gcmVERIFY_OK(gcmOS_SAFE_FREE(os, shaderFunction->arguments));
                }
                gcmVERIFY_OK(gcmOS_SAFE_FREE(os, shaderFunction));
            }

            /* Free the function array. */
            gcmVERIFY_OK(gcmOS_SAFE_FREE(os, Shader->functions));

            Shader->functionCount = 0;
        }
        if (Shader->kernelFunctionCount > 0)
        {
            gcOPT_FUNCTION function;
            gcKERNEL_FUNCTION kernelFunction;

            gcmASSERT(Optimizer->isMainMergeWithKerenel);

            /* All functions are expanded except the main kernel function. */
            for (i = 0; i < Shader->kernelFunctionCount; i++)
            {
                kernelFunction = Shader->kernelFunctions[i];

                if (kernelFunction == Optimizer->main->kernelFunction)
                {
                    continue;
                }

                if (kernelFunction->arguments)
                {
                    /* Free argument array. */
                    gcmVERIFY_OK(gcmOS_SAFE_FREE(os, kernelFunction->arguments));
                }
                gcmVERIFY_OK(gcmOS_SAFE_FREE(os, kernelFunction));
            }

            /* Free the function array. */
            function = Optimizer->main;
            kernelFunction = function->kernelFunction;
            Shader->kernelFunctions[0] = kernelFunction;
            Shader->currentKernelFunction = kernelFunction;
            kernelFunction->label = (gctUINT16) (~0);
            kernelFunction->codeStart = function->codeHead->id;
            kernelFunction->codeCount = function->codeTail->id - function->codeHead->id + 1;
            kernelFunction->codeEnd   = function->codeTail->id + 1;
            kernelFunction->isMain    = gcvTRUE;

            Shader->kernelFunctionCount = 1;
        }
    }
    else
    {
        gcOPT_FUNCTION function = Optimizer->functionArray;
        gcFUNCTION shaderFunction;
        gcKERNEL_FUNCTION kernelFunction;
        gctUINT j = 0, k = 0;
        gctUINT j1 = 0, k1 = 0;

        shaderFunction = Shader->functionCount ? Shader->functions[0] : gcvNULL;
        kernelFunction = Shader->kernelFunctionCount ? Shader->kernelFunctions[0] : gcvNULL;
        gcmASSERT(Optimizer->functionCount <= Shader->functionCount + Shader->kernelFunctionCount);

        for (i = 0; i < Optimizer->functionCount; i++, function++)
        {
            gcmASSERT(i <= j + k);
            if (function->shaderFunction)
            {
                while (shaderFunction != gcvNULL && shaderFunction != function->shaderFunction)
                {
                    /* Free the expanded function. */
                    if (shaderFunction->arguments)
                    {
                        /* Free argument array. */
                        gcmVERIFY_OK(gcmOS_SAFE_FREE(os, shaderFunction->arguments));
                    }
                    gcmVERIFY_OK(gcmOS_SAFE_FREE(os, shaderFunction));
                    Shader->functions[j] = gcvNULL;

                    j++;
                    gcmASSERT(j < Shader->functionCount);
                    shaderFunction = Shader->functions[j];
                }
                gcmASSERT(shaderFunction != gcvNULL);
                /* Copy new function data back to Shader. */
                shaderFunction->argumentCount = function->argumentCount;
                gcmASSERT(shaderFunction->arguments == function->arguments);
                if (i == j)
                {
/* KLC - comment out the assert: its assumption maybe incorrect
                    gcmASSERT(shaderFunction->label == (gctUINT16) (~0 - i));
*/
                    ;
                }
                else
                {
                    shaderFunction->label = (gctUINT16) (~0 - i);
                }
                if (j != j1)
                {
                    Shader->functions[j1] = shaderFunction;
                    Shader->functions[j] = gcvNULL;
                }
                shaderFunction->codeStart = function->codeHead->id;
                shaderFunction->codeCount = function->codeTail->id - function->codeHead->id + 1;
                j++;
                j1++;
                shaderFunction = j < Shader->functionCount ? Shader->functions[j] : gcvNULL;
            }
            else
            {
                gcmASSERT(function->kernelFunction);
                while (kernelFunction != gcvNULL && kernelFunction != function->kernelFunction)
                {
                    if (kernelFunction != Optimizer->main->kernelFunction)
                    {
                        /* Free the expanded function. */
                        if (kernelFunction->arguments)
                        {
                            /* Free argument array. */
                            gcmVERIFY_OK(gcmOS_SAFE_FREE(os, kernelFunction->arguments));
                        }
                        gcmVERIFY_OK(gcmOS_SAFE_FREE(os, kernelFunction));
                        Shader->kernelFunctions[k] = gcvNULL;
                    }

                    k++;
                    gcmASSERT(k < Shader->kernelFunctionCount);
                    kernelFunction = Shader->kernelFunctions[k];
                }
                gcmASSERT(kernelFunction != gcvNULL);
                /* Copy new function data back to Shader. */
                kernelFunction->argumentCount = function->argumentCount;
                gcmASSERT(kernelFunction->arguments == function->arguments);
                if (i == k)
                {
                    gcmASSERT(kernelFunction->label == (gctUINT16) (~0 - i));
                }
                else
                {
                    kernelFunction->label = (gctUINT16) (~0 - i);
                }
                if (k != k1)
                {
                    Shader->kernelFunctions[k1] = kernelFunction;
                    Shader->kernelFunctions[k] = gcvNULL;
                }
                kernelFunction->codeStart = function->codeHead->id;
                kernelFunction->codeCount = function->codeTail->id - function->codeHead->id + 1;
                kernelFunction->codeEnd = function->codeTail->id + 1;
                k++;
                k1++;
                kernelFunction = k < Shader->kernelFunctionCount ? Shader->kernelFunctions[k] : gcvNULL;
            }

            gcmASSERT(i == j1 + k1 - 1);
        }

        gcmASSERT(Optimizer->functionCount == j1 + k1);

        /* Free the rest of shader functions. */
        while (j < Shader->functionCount)
        {
            shaderFunction = Shader->functions[j];

            /* Free the expanded function. */
            if (shaderFunction->arguments)
            {
                /* Free argument array. */
                gcmVERIFY_OK(gcmOS_SAFE_FREE(os, shaderFunction->arguments));
            }
            gcmVERIFY_OK(gcmOS_SAFE_FREE(os, shaderFunction));
            Shader->functions[j] = gcvNULL;

            j++;
        }

        if (j1 == 0 && Shader->functionCount > 0)
        {
            /* Free the function array. */
            gcmVERIFY_OK(gcmOS_SAFE_FREE(os, Shader->functions));
        }

        Shader->functionCount = j1;

        /* Free the rest of shader kernel functions. */
        while (k < Shader->kernelFunctionCount)
        {
            kernelFunction = Shader->kernelFunctions[k];

            if (kernelFunction != Optimizer->main->kernelFunction)
            {
                /* Free the expanded function. */
                if (kernelFunction->arguments)
                {
                    /* Free argument array. */
                    gcmVERIFY_OK(gcmOS_SAFE_FREE(os, kernelFunction->arguments));
                }
                gcmVERIFY_OK(gcmOS_SAFE_FREE(os, kernelFunction));
            }
            Shader->kernelFunctions[k] = gcvNULL;

            k++;
        }

        function = Optimizer->main;
        kernelFunction = function->kernelFunction;
        if (kernelFunction)
        {
            Shader->kernelFunctions[k1] = kernelFunction;
            kernelFunction->label = (gctUINT16) (~0 - i);
            kernelFunction->codeStart = function->codeHead->id;
            kernelFunction->codeCount = function->codeTail->id - function->codeHead->id + 1;
            kernelFunction->codeEnd   = function->codeTail->id + 1;
            kernelFunction->isMain    = gcvTRUE;

            Shader->kernelFunctionCount = k1 + 1;
        }
        else
        {
            Shader->kernelFunctionCount = k1;
        }
    }

    /* Locate the last instruction of main program. */
    pc = Optimizer->main->codeTail->id;

    /* Change the last instruction of main program to NOP if it is RET. */
    code = Shader->code + pc;
    if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_RET)
    {
        /* Make the instruction NOP. */
        *code = gcvSL_NOP_INSTR;
    }
    else
    {
        gcmASSERT(gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_RET);
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**                            _PackMainProgram
********************************************************************************
**
**    Set an instruction as removed by changing it to NOP instruction
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
**
**      gcOPT_CODE       code
**          Pointer to a code to be removed
**
**  OUTPUT:
**
**  If successfully changed, return TRUE, else FALSE
*/
static gctBOOL
_RemoveInstByChangeToNOP(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_CODE       code
    )
{
    gctBOOL             changeCode = gcvFALSE;

    if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_NOP)
    {
        /* For jump, we need update its target's info before we change it to
        ** NOP. */
        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_JMP)
        {
            gcOPT_LIST caller;
            gcmASSERT(code->callee);

            /* Callee's caller. */
            for (caller = code->callee->callers; caller != gcvNULL; caller = caller->next)
            {
                if (caller->code == code)
                {
                    gcOpt_DeleteCodeFromList(Optimizer,
                                             &code->callee->callers,
                                             caller->code);
                    break;
                }
            }

            /* Callee. */
            code->callee = gcvNULL;
        }

        code->instruction = gcvSL_NOP_INSTR;
        changeCode = gcvTRUE;
    }

    return changeCode;
}

/*******************************************************************************
**                            _PackMainProgram
********************************************************************************
**
**    Pact main program if main program is not in one chunk, and initialize
**    main program's codeStart, etc.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
*/
static gceSTATUS
_PackMainProgram(
    IN gcOPTIMIZER      Optimizer
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_FUNCTION      functionArray = Optimizer->functionArray;
    gcOPT_FUNCTION      function;
    gcOPT_CODE          code;
    gcOPT_CODE          codeNext;
    gctUINT             i;
    gctINT              k;
    gctBOOL             changeCode = gcvFALSE;
    gctBOOL             functionRemoved;

    gcmHEADER_ARG("Optimizer=%p", Optimizer);

    if (Optimizer->functionCount == 0)
    {
        /* Initialize main program data struture. */
        gcmASSERT(functionArray == gcvNULL);
        gcmASSERT(Optimizer->main->codeHead == Optimizer->codeHead);
        gcmASSERT(Optimizer->main->codeTail  == Optimizer->codeTail);

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    /* Remove unreachable code, especially for jump-out-of-bound JMP in functions. */
    /* And check to make sure no jump-out-of-bound JMP in functions. */
    for (i = 0; i < Optimizer->functionCount; i++)
    {
        function  = functionArray + i;
        code = function->codeHead;
        codeNext = function->codeTail->next;

        do
        {
            if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_JMP)
            {
                gcmASSERT(code->instruction.tempIndex >= function->codeHead->id
                    && code->instruction.tempIndex <= function->codeTail->id);

                if (gcmSL_TARGET_GET(code->instruction.temp, Condition) != gcSL_ALWAYS)
                {
                    code = code->next;
                    continue;
                }

                code = code->next;
                while (code != gcvNULL && code != codeNext && code->callers == gcvNULL)
                {
                    changeCode = _RemoveInstByChangeToNOP(Optimizer, code);
                    code = code->next;
                }
            }
            else if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_RET)
            {
                if (code != function->codeTail)
                {
                    gctUINT jumpTarget = function->codeTail->id;
                    gcOPT_CODE codeTarget = function->codeTail;

                    gcmASSERT(gcmSL_OPCODE_GET(codeTarget->instruction.opcode, Opcode) == gcSL_RET);
                    gcmASSERT(jumpTarget > code->id);
                    code->instruction.opcode = gcmSL_OPCODE_SET(code->instruction.opcode, Opcode, gcSL_JMP);
                    code->instruction.tempIndex = (gctUINT16)jumpTarget;

                    code->callee = codeTarget;
                    gcmERR_RETURN(gcOpt_AddCodeToList(Optimizer, &codeTarget->callers, code));
                }

                code = code->next;
                while (code != gcvNULL && code != codeNext && code->callers == gcvNULL)
                {
                    changeCode = _RemoveInstByChangeToNOP(Optimizer, code);
                    code = code->next;
                }
            }
            else
            {
                code = code->next;
            }
        }
        while (code != gcvNULL && code != codeNext);
    }

    /* Remove unused functions. */
    do
    {
        functionRemoved = gcvFALSE;
        for (k = Optimizer->functionCount - 1; k >= 0; k--)
        {
            function = functionArray + k;

            /* skip main kernel function if it is already
             * merged with main */
            if (function->kernelFunction != gcvNULL &&
                function->kernelFunction->isMain)
            {
                gcmASSERT(Optimizer->isMainMergeWithKerenel);
                continue;
            }
            if (function->codeHead->callers == gcvNULL)
            {
                /* Function is not used at all or is a recursive function. */
                /* Remove the function. */
                gcOPT_TEMP tempArray = Optimizer->tempArray;
                gcsFUNCTION_ARGUMENT_PTR argument;
                gctUINT j;

                /* Free the code list in the function. */
#if _REMOVE_UNUSED_FUNCTION_CODE_
                gcOpt_RemoveCodeList(Optimizer, function->codeHead, function->codeTail);
#else
                for (code = function->codeHead; code != gcvNULL && code != function->codeTail->next; code = code->next)
                {
                    if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_CALL)
                    {
                        /* Remove from callee's caller list */
                        gcOpt_DeleteCodeFromList(Optimizer,
                                    &code->callee->callers,
                                    code);
                        code->callee = gcvNULL;
                    }
                    else
                    {
                        /* All other instruction is inside this function, remove caller and callee */
                        gcOPT_LIST caller;
                        gcOPT_LIST nextCaller;

                        for (caller = code->callers ;caller != gcvNULL;)
                        {
                            nextCaller = caller->next;
                            _FreeList(Optimizer->listMemPool, caller);
                            caller = nextCaller;
                        }

                        code->callee = gcvNULL;
                        code->callers = gcvNULL;
                    }

                    code->instruction = gcvSL_NOP_INSTR;
                    code->function = gcvNULL;
                }

                if (k > 0 &&
                    ((function->codeHead->prev == functionArray[k - 1].codeTail) ||
                     (function->codeTail->next == functionArray[k - 1].codeHead)))
                {
                    if (function->codeHead->prev == functionArray[k - 1].codeTail)
                        functionArray[k - 1].codeTail = function->codeTail;

                    if (function->codeTail->next == functionArray[k - 1].codeHead)
                        functionArray[k - 1].codeHead = function->codeHead;
                }
                else if (function->codeTail->next == Optimizer->main->codeHead)
                {
                    if (k == 0)
                    {
                        Optimizer->main->codeHead = Optimizer->codeHead;
                    }
                    else if (function->codeHead->prev != functionArray[k - 1].codeTail)
                    {
                        Optimizer->main->codeHead = functionArray[k - 1].codeTail->next;
                    }
                    else
                    {
                        Optimizer->main->codeHead = function->codeHead;
                    }
                }
                /* If the implementation of this function is after the main function,
                ** we need to adjust the main function.
                */
                else if (function->codeHead->prev == Optimizer->main->codeTail)
                {
                    Optimizer->main->codeTail = function->codeTail;
                    if (k + 1 < (gctINT)Optimizer->functionCount)
                    {
                        /* adjust the function after deleted function */
                        gcmASSERT(functionArray[k + 1].codeHead->prev == function->codeTail);
                        functionArray[k + 1].codeHead->prev = Optimizer->main->codeTail;
                    }
                }
#endif

                /* Update temps' function for function arguments. */
                argument = function->arguments;
                for (j = 0; j < function->argumentCount; j++, argument++)
                {
                    gctUINT index = argument->index;

                    gcmASSERT(tempArray[index].function == function);
                    tempArray[index].function = gcvNULL;
                    tempArray[index].argument = gcvNULL;
                }

                for (j = k; j < Optimizer->functionCount - 1; j++)
                {
                    functionArray[j] = functionArray[j + 1];

                    /* Update temps' function for function arguments. */
                    argument = functionArray[j].arguments;
                    for (i = 0; i < functionArray[j].argumentCount; i++, argument++)
                    {
                        gctUINT index = argument->index;

                        gcmASSERT(tempArray[index].function == functionArray + j + 1);
                        tempArray[index].function = functionArray + j;
                        tempArray[index].argument = argument;
                        tempArray[index].precision = (gcSHADER_PRECISION)argument->precision;

                        if (argument->variableIndex != 0xffff)
                        {
                            gcmASSERT(argument->variableIndex < Optimizer->shader->variableCount);
                            tempArray[index].arrayVariable = Optimizer->shader->variables[argument->variableIndex];
                            tempArray[index].precision = tempArray[index].arrayVariable->precision;
                        }
                    }
                }
                Optimizer->functionCount--;
                if (Optimizer->functionCount == 0)
                {
                    /* Free the empty function array. */
                    gcmVERIFY_OK(_FreeFunctionArray(Optimizer->functionArrayMemPool, functionArray));
                    Optimizer->functionArray = gcvNULL;
                }

                functionRemoved = gcvTRUE;
                changeCode = gcvTRUE;
            }
        }
    } while(functionRemoved);

    if (changeCode)
    {
        /* Update code id. */
        gcOpt_UpdateCodeId(Optimizer);

        /* Update function pointer. */
        function = functionArray;
        for (i = 0; i < Optimizer->functionCount; i++, function++)
        {
            for (code = function->codeHead; code != gcvNULL && code != function->codeTail->next; code = code->next)
            {
                code->function = function;
            }
        }

        changeCode = gcvFALSE;
    }

    if (Optimizer->functionCount == 0)
    {
        /* Initialize main program data struture. */
        gcmASSERT(Optimizer->functionArray == gcvNULL);
        gcmASSERT(Optimizer->main->codeHead == Optimizer->codeHead);
        gcmASSERT(Optimizer->main->codeTail  == Optimizer->codeTail);
        /*Optimizer->main->codeHead = Optimizer->codeHead;
        Optimizer->main->codeTail  = Optimizer->codeTail;*/

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    /* Skip the functions after main. */
    function = functionArray + Optimizer->functionCount - 1;
    for (k = Optimizer->functionCount - 1; k >= 0; k--, function--)
    {
        if (function->codeHead->id < Optimizer->main->codeHead->id)
        {
            break;
        }
    }

    if (k >= 0)
    {
        gcOPT_FUNCTION functionPrev;

        gcmASSERT(Optimizer->main->codeHead == function->codeTail->next);
        gcmASSERT(Optimizer->main->codeTail  ==
            ((k == (gctINT) Optimizer->functionCount - 1) ?
            Optimizer->codeTail :
                    functionArray[k + 1].codeHead->prev));
        for (; k > 0; k--, function = functionPrev)
        {
            functionPrev = function - 1;
            if (function->codeHead->prev != functionPrev->codeTail)
            {
                /* Move the code in the gap to in front of main. */
                gcOPT_CODE codeHead = functionPrev->codeTail->next;
                gcOpt_MoveCodeListBefore(Optimizer,
                    codeHead,
                    function->codeHead->prev,
                    Optimizer->main->codeHead);
                gcmASSERT(function->codeHead->prev == functionPrev->codeTail
                       && function->codeHead == functionPrev->codeTail->next);
                Optimizer->main->codeHead = codeHead;

                changeCode = gcvTRUE;
            }
        }

        /* Handle the first function. */
        if (function->codeHead->prev)
        {
            /* Move the code before function to in front of main. */
            gcOPT_CODE codeHead = Optimizer->codeHead;
            gcOpt_MoveCodeListBefore(Optimizer,
                codeHead,
                function->codeHead->prev,
                Optimizer->main->codeHead);
            gcmASSERT(function->codeHead->prev == gcvNULL);
            Optimizer->main->codeHead = codeHead;

            changeCode = gcvTRUE;
        }
    }
#if !_REMOVE_UNUSED_FUNCTION_CODE_
    else
    {
        if (Optimizer->main->codeHead != Optimizer->codeHead)
        {
            Optimizer->main->codeHead = Optimizer->codeHead;
        }
        gcmASSERT(Optimizer->main->codeTail == functionArray[0].codeHead->prev ||
                  (functionArray[0].kernelFunction != gcvNULL &&
                   functionArray[0].kernelFunction->isMain) );
    }
#endif

    if (changeCode)
    {
        /* Update code id. */
        gcOpt_UpdateCodeId(Optimizer);
    }

    gcmFOOTER();
    return status;
}

/******************************************************************************\
|********************* Data Flow Graph Supporting Functioins ******************|
\******************************************************************************/

static gceSTATUS
_SetDefineInput(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gctINT           Index
    )
{
    gcmASSERT(Index < 0);
    if (*Root == gcvNULL) {
        return gcOpt_AddIndexToList(Optimizer, Root, Index);
    }
    else
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }
}

static gceSTATUS
_SetTempDefineInput(
    IN gcOPTIMIZER          Optimizer,
    IN gcOPT_TEMP_DEFINE    TempDefine,
    IN gctUINT              Enable,
    IN gctINT               Index
    )
{
    gceSTATUS               status = gcvSTATUS_OK;

    gcmHEADER_ARG("Optimizer=0x%x TempDefine=0x%x Enable=%u Index=%d",
                   Optimizer, TempDefine, Enable, Index);

    /* Set temp defines according to enable. */
    if (Enable & gcSL_ENABLE_X)
    {
        gcmERR_RETURN(_SetDefineInput(Optimizer, &TempDefine->xDefines, Index));
    }
    if (Enable & gcSL_ENABLE_Y)
    {
        gcmERR_RETURN(_SetDefineInput(Optimizer, &TempDefine->yDefines, Index));
    }
    if (Enable & gcSL_ENABLE_Z)
    {
        gcmERR_RETURN(_SetDefineInput(Optimizer, &TempDefine->zDefines, Index));
    }
    if (Enable & gcSL_ENABLE_W)
    {
        gcmERR_RETURN(_SetDefineInput(Optimizer, &TempDefine->wDefines, Index));
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_AddFunctionInputDefine(
    IN gcOPTIMIZER          Optimizer,
    IN gcOPT_FUNCTION       Function,
    IN gcOPT_TEMP_DEFINE    TempDefineArray
    )
{
    gceSTATUS                status = gcvSTATUS_OK;
    gcsFUNCTION_ARGUMENT_PTR argument;
    gcOPT_TEMP               tempArray = Optimizer->tempArray;
    gcOPT_GLOBAL_USAGE       usage;
    gctUINT                  i;

    gcmHEADER_ARG("Optimizer=0x%x Function=0x%x TempDefineArray=0x%x",
                   Optimizer, Function, TempDefineArray);

    /* Add define for all input arguments. */
    argument = Function->arguments;
    for (i = 0; i < Function->argumentCount; i++, argument++)
    {
        /* Set output arguments. */
        if (argument->qualifier != gcvFUNCTION_OUTPUT)
        {
            gcmERR_RETURN(_SetTempDefineInput(Optimizer,
                                &TempDefineArray[argument->index],
                                argument->enable,
                                gcvOPT_INPUT_REGISTER));
        }
    }

    /* Add define for all global variables. */
    for (usage = Function->globalUsage; usage; usage = usage->next)
    {
        gctUINT index = usage->index;

        gcmERR_RETURN(_SetTempDefineInput(Optimizer,
                                &TempDefineArray[index],
                                tempArray[index].usage,
                                gcvOPT_GLOBAL_REGISTER));
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_AddUndefined(
    IN gcOPTIMIZER          Optimizer,
    IN gcOPT_TEMP_DEFINE    TempDefineArray
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gcOPT_TEMP_DEFINE       tempDefine = TempDefineArray;
    gctUINT                 i;

    gcmHEADER_ARG("Optimizer=0x%x TempDefineArray=0x%x",
                   Optimizer, TempDefineArray);

    /* Add define for all input arguments. */
    for (i = 0; i < Optimizer->tempCount; i++, tempDefine++)
    {
        /* Add undfined if not defined. */
        if (tempDefine->xDefines == gcvNULL)
        {
            gcmERR_RETURN(_SetDefineInput(Optimizer, &tempDefine->xDefines, gcvOPT_UNDEFINED_REGISTER));
        }
        if (tempDefine->yDefines == gcvNULL)
        {
            gcmERR_RETURN(_SetDefineInput(Optimizer, &tempDefine->yDefines, gcvOPT_UNDEFINED_REGISTER));
        }
        if (tempDefine->zDefines == gcvNULL)
        {
            gcmERR_RETURN(_SetDefineInput(Optimizer, &tempDefine->zDefines, gcvOPT_UNDEFINED_REGISTER));
        }
        if (tempDefine->wDefines == gcvNULL)
        {
            gcmERR_RETURN(_SetDefineInput(Optimizer, &tempDefine->wDefines, gcvOPT_UNDEFINED_REGISTER));
        }
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_SetDefineList(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gcOPT_CODE       Code
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_LIST          list;

    gcmHEADER_ARG("Optimizer=0x%x Root=0x%x Code=0x%x",
                   Optimizer, Root, Code);

    gcmVERIFY_OK(gcOpt_DeleteIndexFromList(Optimizer, Root, gcvOPT_UNDEFINED_REGISTER));
    gcmVERIFY_OK(gcOpt_DeleteIndexFromList(Optimizer, Root, gcvOPT_JUMPUNDEFINED_REGISTER));

    if (*Root == gcvNULL) {
        status = gcOpt_AddCodeToList(Optimizer, Root, Code);
        gcmFOOTER();
        return status;
    }

    for (list = *Root; list; list = list->next)
    {
        if (list->code == Code)
        {
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }

        if (list->index >= 0)
        {
            gcmERR_RETURN(gcOpt_AddCodeToList(Optimizer, &list->code->nextDefines, Code));
        }
    }

    if (Code->prevDefines == gcvNULL)
    {
        Code->prevDefines = *Root;
        *Root = gcvNULL;
        status = gcOpt_AddCodeToList(Optimizer, Root, Code);
        gcmFOOTER();
        return status;
    }

    gcmERR_RETURN(gcOpt_AddListToList(Optimizer, *Root, gcvFALSE, &Code->prevDefines));

    /* Free all entries in the list except one. */
    while ((list = (*Root)->next) != gcvNULL)
    {
        (*Root)->next = list->next;

        /* Free the gcOPT_LIST structure. */
        gcmVERIFY_OK(_FreeList(Optimizer->listMemPool, list));
    }

    /* Set code. */
    (*Root)->code  = Code;
    (*Root)->index = 0;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_SetTempDefine(
    IN gcOPTIMIZER          Optimizer,
    IN gcOPT_TEMP_DEFINE    TempDefine,
    IN gctUINT              Enable,
    IN gcOPT_CODE           Code
    )
{
    gceSTATUS               status = gcvSTATUS_OK;

    gcmHEADER_ARG("Optimizer=0x%x TempDefine=0x%x Enable=%u Code=0x%x",
                   Optimizer, TempDefine, Enable, Code);

    /* Set temp defines according to enable. */
    if (Enable & gcSL_ENABLE_X)
    {
        gcmERR_RETURN(_SetDefineList(Optimizer, &TempDefine->xDefines, Code));
    }
    if (Enable & gcSL_ENABLE_Y)
    {
        gcmERR_RETURN(_SetDefineList(Optimizer, &TempDefine->yDefines, Code));
    }
    if (Enable & gcSL_ENABLE_Z)
    {
        gcmERR_RETURN(_SetDefineList(Optimizer, &TempDefine->zDefines, Code));
    }
    if (Enable & gcSL_ENABLE_W)
    {
        gcmERR_RETURN(_SetDefineList(Optimizer, &TempDefine->wDefines, Code));
    }

    gcmFOOTER();
    return status;
}

#if _RECURSIVE_BUILD_DU_
static gceSTATUS
_AddUserRecusive(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_CODE       Code,
    IN gcOPT_CODE       defCode,
    IN gcOPT_CODE       endCode
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_CODE          code;
    gcOPT_LIST          preDef;

    gcmHEADER_ARG("Optimizer=0x%x Code=0x%x", Optimizer, Code);

    code = defCode;
    if (code != gcvNULL)
    {
        preDef = code->prevDefines;
        while (preDef)
        {
            code = preDef->code;
            if (code != gcvNULL)
            {
                gcmERR_BREAK(gcOpt_AddCodeToList(Optimizer, &code->users, Code));

                /* No need go on if we have found an exact def. Otherwise, go into further */
                if (gcmSL_TARGET_GET(code->instruction.temp, Indexed) != gcSL_NOT_INDEXED &&
                    code != endCode)
                    gcmERR_BREAK(_AddUserRecusive(Optimizer, Code, code, endCode));
            }

            preDef = preDef->next;
        }
    }

    gcmFOOTER();
    return status;
}
#endif

static gceSTATUS
_AddUser(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_LIST       InputList,
    IN gcOPT_CODE       Code,
    IN gctBOOL          bForSuccessiveReg
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_LIST          list;
    gcOPT_CODE          code;

    gcmHEADER_ARG("Optimizer=0x%x InputList=0x%x Code=0x%x",
                   Optimizer, InputList, Code);

    /* Add the input list to Root. */
    for (list = InputList; list; list = list->next)
    {
        if (list->index >= 0)
        {
            gcmERR_BREAK(gcOpt_AddCodeToList(Optimizer, &list->code->users, Code));

            /* Successive registers, such as for array/matrix, we use very conservative solution that uses */
            /* all defs before */
            if (bForSuccessiveReg && (gcmSL_TARGET_GET(list->code->instruction.temp, Indexed) != gcSL_NOT_INDEXED))
            {
                code = list->code;

#if _RECURSIVE_BUILD_DU_
                gcmERR_BREAK(_AddUserRecusive(Optimizer, Code, code, code));
#else
                while (code != gcvNULL && code->prevDefines)
                {
                    code = code->prevDefines->code;
                    if (code)
                    {
                        gcmERR_BREAK(gcOpt_AddCodeToList(Optimizer, &code->users, Code));

                        /* No need go on since we have found an exact def. */
                        if (gcmSL_TARGET_GET(code->instruction.temp, Indexed) == gcSL_NOT_INDEXED)
                            break;
                    }
                }
#endif
            }
        }
    }

    gcmFOOTER();
    return status;
}

#if _RECURSIVE_BUILD_DU_
static gceSTATUS
_AddTempDependencyRecusive(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gcOPT_CODE       code,
    IN gcOPT_CODE       endCode
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_LIST          preDef;
    gctINT              index;

    gcmHEADER_ARG("Optimizer=0x%x Root=%p Code=0x%x", Optimizer, Root, code);

    if (code != gcvNULL)
    {
        preDef = code->prevDefines;
        while (preDef)
        {
            code = preDef->code;
            index = preDef->index;
            if (code != gcvNULL)
            {
                if (index < 0)
                {
                    gcmERR_BREAK(gcOpt_AddIndexToList(Optimizer, Root, index));
                }
                else
                {
                    gcmERR_BREAK(gcOpt_AddCodeToList(Optimizer, Root, code));
                }

                /* No need go on if we have found an exact def. Otherwise, go into further */
                if (gcmSL_TARGET_GET(code->instruction.temp, Indexed) != gcSL_NOT_INDEXED &&
                    code != endCode)
                    gcmERR_BREAK(_AddTempDependencyRecusive(Optimizer, Root, code, endCode));
            }

            preDef = preDef->next;
        }
    }

    gcmFOOTER();
    return status;
}
#endif

static gceSTATUS
_AddTempDependency(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_LIST       SrcList,
    IN OUT gcOPT_LIST * Root,
    IN gctBOOL          bForSuccessiveReg
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_LIST          list;
    gcOPT_CODE          code;

    gcmHEADER_ARG("Optimizer=%p SrcList=%p Root=%p", Optimizer, SrcList, Root);

    for (list = SrcList; list; list = list->next)
    {
        if (list->index < 0)
        {
            gcmERR_BREAK(gcOpt_AddIndexToList(Optimizer, Root, list->index));
        }
        else
        {
            gcmERR_BREAK(gcOpt_AddCodeToList(Optimizer, Root, list->code));
        }

        /* Successive registers, such as for array/matrix, we use very conservative solution that uses */
        /* all defs before */
        if (bForSuccessiveReg && list->code && (gcmSL_TARGET_GET(list->code->instruction.temp, Indexed) != gcSL_NOT_INDEXED))
        {
            code = list->code;

#if _RECURSIVE_BUILD_DU_
            gcmERR_BREAK(_AddTempDependencyRecusive(Optimizer, Root, code, code));
#else
            while (code != gcvNULL && code->prevDefines)
            {
                if (code->prevDefines->index < 0)
                {
                    gcmERR_BREAK(gcOpt_AddIndexToList(Optimizer, Root, code->prevDefines->index));
                }
                else
                {
                    gcmERR_BREAK(gcOpt_AddCodeToList(Optimizer, Root, code->prevDefines->code));
                }

                /* No need go on since we have found an exact def. */
                if (gcmSL_TARGET_GET(code->prevDefines->code->instruction.temp, Indexed) == gcSL_NOT_INDEXED)
                    break;

                code = code->prevDefines->code;
            }
#endif
        }
    }

    /* Success. */
    gcmFOOTER_ARG("*Root=%p status=%d", *Root, status);
    return gcvSTATUS_OK;
}

static gceSTATUS
_AddTempUsage(
    IN gcOPTIMIZER          Optimizer,
    IN gcOPT_TEMP_DEFINE    TempDefine,
    IN gctUINT              enable,
    IN OUT gcOPT_LIST *     Root,
    IN gcOPT_CODE           Code,
    IN gctBOOL              bForSuccessiveReg
    )
{
    gceSTATUS               status = gcvSTATUS_OK;

    gcmHEADER_ARG("Optimizer=0x%x TempDefine=0x%x enable=0x%x Root=0x%x Code=0x%x",
                   Optimizer, TempDefine, enable, Root, Code);

    /* Set temp defines according to enable. */
    if (enable & gcSL_ENABLE_X)
    {
        /* Add dependencies. */
        if (Root)
        {
            gcmERR_RETURN(_AddTempDependency(Optimizer, TempDefine->xDefines, Root, bForSuccessiveReg));
        }

        /* Add user. */
        gcmERR_RETURN(_AddUser(Optimizer, TempDefine->xDefines, Code, bForSuccessiveReg));
    }
    if (enable & gcSL_ENABLE_Y)
    {
        /* Add dependencies. */
        if (Root)
        {
            gcmERR_RETURN(_AddTempDependency(Optimizer, TempDefine->yDefines, Root, bForSuccessiveReg));
        }

        /* Add user. */
        gcmERR_RETURN(_AddUser(Optimizer, TempDefine->yDefines, Code, bForSuccessiveReg));
    }
    if (enable & gcSL_ENABLE_Z)
    {
        /* Add dependencies. */
        if (Root)
        {
            gcmERR_RETURN(_AddTempDependency(Optimizer, TempDefine->zDefines, Root, bForSuccessiveReg));
        }

        /* Add user. */
        gcmERR_RETURN(_AddUser(Optimizer, TempDefine->zDefines, Code, bForSuccessiveReg));
    }
    if (enable & gcSL_ENABLE_W)
    {
        /* Add dependencies. */
        if (Root)
        {
            gcmERR_RETURN(_AddTempDependency(Optimizer, TempDefine->wDefines, Root, bForSuccessiveReg));
        }

        /* Add user. */
        gcmERR_RETURN(_AddUser(Optimizer, TempDefine->wDefines, Code, bForSuccessiveReg));
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_AddOutputUser(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_LIST       InputList,
    IN gctINT           Index
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_LIST          list;
    gcOPT_CODE          code;
    gcOPT_CODE          endCode = gcvNULL;

    gcmHEADER_ARG("Optimizer=0x%x InputList=0x%x Index=%d",
                   Optimizer, InputList, Index);

    /* Add the input list to Root. */
    for (list = InputList; list; list = list->next)
    {
        if (list->index >= 0)
        {
            gcmERR_BREAK(gcOpt_AddIndexToList(Optimizer, &list->code->users, Index));

            /* No need go on since we have found an exact def. */
            if (gcmSL_TARGET_GET(list->code->instruction.temp, Indexed) == gcSL_NOT_INDEXED)
                continue;

            /* All appeared output defs must be add usage at end of program */
            code = list->code;

            while (code != gcvNULL && code->prevDefines &&
                (endCode == gcvNULL || (endCode != gcvNULL && endCode != code->prevDefines->code)))
            {
                code = code->prevDefines->code;

                if (endCode == gcvNULL)
                {
                    endCode = code;
                }

                if (code)
                {
                    gcmERR_BREAK(gcOpt_AddIndexToList(Optimizer, &code->users, Index));

                    /* No need go on since we have found an exact def. */
                    if (gcmSL_TARGET_GET(code->instruction.temp, Indexed) == gcSL_NOT_INDEXED)
                        break;
                }
            }
        }
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_AddTempOutputUsage(
    IN gcOPTIMIZER          Optimizer,
    IN gcOPT_TEMP_DEFINE    TempDefine,
    IN gctUINT              enable,
    IN gctINT               Index
    )
{
    gceSTATUS               status = gcvSTATUS_OK;

    gcmHEADER_ARG("Optimizer=0x%x TempDefine=0x%x enable=%u Index=%u",
                   Optimizer, TempDefine, enable, Index);

    /* Set temp defines according to enable. */
    if (enable & gcSL_ENABLE_X)
    {
        /* Add user. */
        gcmERR_RETURN(_AddOutputUser(Optimizer, TempDefine->xDefines, Index));
    }
    if (enable & gcSL_ENABLE_Y)
    {
        /* Add user. */
        gcmERR_RETURN(_AddOutputUser(Optimizer, TempDefine->yDefines, Index));
    }
    if (enable & gcSL_ENABLE_Z)
    {
        /* Add user. */
        gcmERR_RETURN(_AddOutputUser(Optimizer, TempDefine->zDefines, Index));
    }
    if (enable & gcSL_ENABLE_W)
    {
        /* Add user. */
        gcmERR_RETURN(_AddOutputUser(Optimizer, TempDefine->wDefines, Index));
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_AddFunctionOutputUsage(
    IN gcOPTIMIZER          Optimizer,
    IN gcOPT_FUNCTION       Function,
    IN gcOPT_TEMP_DEFINE    TempDefineArray
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gctUINT i;

    gcmHEADER_ARG("Optimizer=0x%x Function=0x%x TempDefineArray=0x%x",
                   Optimizer, Function, TempDefineArray);

    /* Add output dependancy. */
    if (Function == gcvNULL)
    {
        /* Main program. */
        for (i = 0; i < Optimizer->outputCount; i++)
        {
            gctUINT regIdx;
            gctUINT size;
            gctUINT    endIndex;

            /* output could be NULL if it is not used */
            if (Optimizer->outputs[i] == gcvNULL)
                continue;

            size = gcmType_Rows(Optimizer->outputs[i]->type);
            endIndex = Optimizer->outputs[i]->tempIndex + Optimizer->outputs[i]->arraySize * size;

            /*
            *  If the output is an array and it is not the first element of the array,
            *  we don't need to add the usage for it.
            */
            if (Optimizer->outputs[i]->arraySize > 1 && Optimizer->outputs[i]->arrayIndex != 0)
            {
                endIndex = Optimizer->outputs[i]->tempIndex;
            }

            for (regIdx = Optimizer->outputs[i]->tempIndex; regIdx < endIndex; regIdx++)
            {
                gcmERR_RETURN(_AddTempOutputUsage(Optimizer,
                                        &TempDefineArray[regIdx],
                                        gcSL_ENABLE_XYZW,
                                        gcvOPT_OUTPUT_REGISTER));
            }
        }
    }
    else
    {
        gcsFUNCTION_ARGUMENT_PTR argument;
        gcOPT_TEMP tempArray = Optimizer->tempArray;
        gcOPT_GLOBAL_USAGE usage;

        /* Function. */
        /* Add users for all output arguments. */
        argument = Function->arguments;
        for (i = 0; i < Function->argumentCount; i++, argument++)
        {
            /* Set output arguments. */
            if (argument->qualifier != gcvFUNCTION_INPUT)
            {
                gcmERR_RETURN(_AddTempOutputUsage(Optimizer,
                                    &TempDefineArray[argument->index],
                                    argument->enable,
                                    gcvOPT_OUTPUT_REGISTER));
            }
        }

        /* Add users for all global variables. */
        for (usage = Function->globalUsage; usage; usage = usage->next)
        {
            gctUINT index = usage->index;

            gcmERR_RETURN(_AddTempOutputUsage(Optimizer,
                                    &TempDefineArray[index],
                                    tempArray[index].usage,
                                    gcvOPT_GLOBAL_REGISTER));
        }
    }

    gcmVERIFY_OK(gcOpt_ClearTempArray(Optimizer, TempDefineArray));

    gcmFOOTER();
    return status;
}

gceSTATUS
gcOpt_DestroyCodeDependency(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gcOPT_CODE       Code
    )
{
    gcOPT_LIST          list, nextList;

    gcmHEADER_ARG("Optimizer=%p Root=%p Code=%p",
                    Optimizer, Root, Code);

    for (list = *Root; list; list = nextList)
    {
        nextList = list->next;

        if (list->index >= 0)
        {
            gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &list->code->users, Code));
        }

        /* Free the gcOPT_LIST structure. */
        gcmVERIFY_OK(_FreeList(Optimizer->listMemPool, list));
    }

    *Root = gcvNULL;

    gcmFOOTER_ARG("*Root=%p", *Root);
    return gcvSTATUS_OK;
}

static gceSTATUS
_MergeTempDefineArray(
    IN gcOPTIMIZER          Optimizer,
    IN gcOPT_TEMP_DEFINE    SrcTempDefineArray,
    IN gctBOOL              isBackJump,
    OUT gcOPT_TEMP_DEFINE * DestTempDefineArray
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gcOPT_TEMP_DEFINE       srcTempDefine;
    gcOPT_TEMP_DEFINE       destTempDefine;
    gctUINT                 i;

    gcmHEADER_ARG("Optimizer=0x%x SrcTempDefineArray=0x%x",
                   Optimizer, SrcTempDefineArray);

    /* Allocate new array if needed. */
    if (*DestTempDefineArray == gcvNULL && Optimizer->tempCount > 0)
    {
        gcmERR_RETURN(_CAllocateTempDefineArray(Optimizer->tempDefineArrayMemPool,
            DestTempDefineArray, Optimizer->tempCount));
    }

    srcTempDefine = SrcTempDefineArray;
    destTempDefine = *DestTempDefineArray;
    for (i = 0; i < Optimizer->tempCount; i++, srcTempDefine++, destTempDefine++)
    {
        gcmERR_BREAK(gcOpt_AddListToList(Optimizer, srcTempDefine->xDefines, isBackJump, &destTempDefine->xDefines));
        gcmERR_BREAK(gcOpt_AddListToList(Optimizer, srcTempDefine->yDefines, isBackJump, &destTempDefine->yDefines));
        gcmERR_BREAK(gcOpt_AddListToList(Optimizer, srcTempDefine->zDefines, isBackJump, &destTempDefine->zDefines));
        gcmERR_BREAK(gcOpt_AddListToList(Optimizer, srcTempDefine->wDefines, isBackJump, &destTempDefine->wDefines));
    }

    gcmFOOTER();
    return status;
}

static gctUINT
_GetEnableFromSwizzles(
    IN gcSL_SWIZZLE     SwizzleX,
    IN gcSL_SWIZZLE     SwizzleY,
    IN gcSL_SWIZZLE     SwizzleZ,
    IN gcSL_SWIZZLE     SwizzleW
    )
{
    gctUINT             enable = 0;

    static const gcSL_ENABLE swizzleToEnable[] =
    {
        gcSL_ENABLE_X, gcSL_ENABLE_Y, gcSL_ENABLE_Z, gcSL_ENABLE_W
    };

    /* Calulate enable from swizzles. */
    if ((SwizzleX == gcSL_SWIZZLE_X) &&
        (SwizzleY == gcSL_SWIZZLE_Y) &&
        (SwizzleZ == gcSL_SWIZZLE_Z) &&
        (SwizzleW == gcSL_SWIZZLE_W))
    {
        enable = gcSL_ENABLE_XYZW;
    }
    else
    {
        /* Enable the x swizzle. */
        enable = swizzleToEnable[SwizzleX];

        /* Only continue of the other swizzles are different. */
        if ((SwizzleY != SwizzleX) ||
            (SwizzleZ != SwizzleX) ||
            (SwizzleW != SwizzleX))
        {
            /* Enable the y swizzle. */
            enable |= swizzleToEnable[SwizzleY];

            /* Only continue of the other swizzles are different. */
            if ((SwizzleZ != SwizzleY) || (SwizzleW != SwizzleY) )
            {
                /* Enable the z swizzle. */
                enable |= swizzleToEnable[SwizzleZ];

                /* Only continue of the other swizzle are different. */
                if (SwizzleW != SwizzleZ)
                {
                    /* Enable the w swizzle. */
                    enable |= swizzleToEnable[SwizzleW];
                }
            }
        }
    }

    return enable;
}

static gctBOOL _needSuccessiveReg(
    IN gcOPTIMIZER      Optimizer,
    IN gctUINT          regIndex
    )
{
    gctBOOL             ret = gcvFALSE;

    if (Optimizer->tempArray[regIndex].arrayVariable)
    {
        ret = (GetVariableArraySize(Optimizer->tempArray[regIndex].arrayVariable) > 1) ||
              gcmType_isMatrix(Optimizer->tempArray[regIndex].arrayVariable->u.type);
    }

    return ret;
}

static gceSTATUS
_InsertInitializerInstForOutput(
    IN gcOPTIMIZER          Optimizer,
    IN gcOPT_CODE           Code,
    IN gcOPT_TEMP_DEFINE    TempDefineArray
    )
{
    gceSTATUS        status = gcvSTATUS_OK;
    gctUINT32        i;

    if (Optimizer->shader->type == gcSHADER_TYPE_FRAGMENT && !gctOPT_hasFeature(FB_ENABLE_FS_OUT_INIT))
    {
        /* 15.2.1 Selecting Buffers for Writing
         * If a fragment shader writes to none of gl_FragColor, gl_FragData,
         * nor any user-defined output variables, the values of the fragment colors following
         * shader execution are undefined, and may differ for each fragment color. If some,
         * but not all user-defined output variables are written, the values of fragment colors
         * corresponding to unwritten variables are similarly undefined.
         */
        return status;
    }
    for (i = 0; i < Optimizer->outputCount;)
    {
        gcOUTPUT      output = Optimizer->outputs[i];
        gctINT        j;
        gctINT        componentCount;
        gcSL_ENABLE   enable[4] = {gcSL_ENABLE_X, gcSL_ENABLE_XY, gcSL_ENABLE_XYZ, gcSL_ENABLE_XYZW};

        if (output == gcvNULL)
        {
            i++;
            continue;
        }

        if (gcmOUTPUT_isPerVertexArray(output) ||
            gcmOUTPUT_isPerPatch(output))
        {
            i++;
            continue;
        }

        componentCount = gcmType_Comonents(output->type);

        for (j = 0; j < output->arraySize; j++)
        {
            /* Insert a MOV. */
            gctBOOL insert;

            if ((gctUINT)(j + output->tempIndex) >= Optimizer->tempCount)
            {
                continue;
            }

            insert = gcOpt_CheckListHasUndefined(Optimizer, TempDefineArray[j + output->tempIndex].xDefines);

            if (componentCount > 1)
                insert |= gcOpt_CheckListHasUndefined(Optimizer, TempDefineArray[j + output->tempIndex].yDefines);
            if (componentCount > 2)
                insert |= gcOpt_CheckListHasUndefined(Optimizer, TempDefineArray[j + output->tempIndex].zDefines);
            if (componentCount > 3)
                insert |= gcOpt_CheckListHasUndefined(Optimizer, TempDefineArray[j + output->tempIndex].wDefines);

            if (insert)
            {
                gcOPT_CODE insertCode = gcvNULL;

                gcmVERIFY_OK(gcOpt_AddCodeBefore(Optimizer, Optimizer->main->codeHead, &insertCode));
                gcoOS_ZeroMemory(&insertCode->instruction, sizeof(struct _gcSL_INSTRUCTION));

                /* Insert a MOV instruction. */
                insertCode->instruction.opcode = gcSL_MOV;
                insertCode->instruction.temp = gcmSL_TARGET_SET(0, Enable, enable[componentCount - 1])
                                                         | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                                                         | gcmSL_TARGET_SET(0, Format, gcGetFormatFromType(output->type))
                                                         | gcmSL_TARGET_SET(0, Precision, output->precision)
                                                         | gcmSL_TARGET_SET(0, Condition, gcSL_ALWAYS);
                insertCode->instruction.tempIndex = (gctUINT16)(j + output->tempIndex);
                insertCode->instruction.source0 = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                                                            | gcmSL_SOURCE_SET(0, Format, gcGetFormatFromType(output->type))
                                                            | gcmSL_SOURCE_SET(0, Precision, output->precision)
                                                            | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYZW);

                gcmVERIFY_OK(gcOpt_AddIndexToList(
                                            Optimizer,
                                            &insertCode->users,
                                            gcvOPT_OUTPUT_REGISTER));

                Optimizer->checkOutputUse = gcvTRUE;
            }
        }

        i += output->arraySize;
    }

    gcOpt_UpdateCodeId(Optimizer);

    return status;
}

static gceSTATUS
_BuildDataFlowForCode(
    IN gcOPTIMIZER          Optimizer,
    IN gcOPT_CODE           Code,
    IN gcOPT_TEMP_DEFINE    TempDefineArray
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gcSL_INSTRUCTION        code = &Code->instruction;
    gcSL_OPCODE             opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);
    gcOPT_TEMP              tempArray = Optimizer->tempArray;
    gctTARGET_t               target;
    gctSOURCE_t             source;
    gctUINT16               index;
    gctUINT8                format;
    gctBOOL                 targetIsTemp = gcvFALSE;

    gcmHEADER_ARG("Optimizer=0x%x Code=0x%x TempDefineArray=0x%x",
                   Optimizer, Code, TempDefineArray);

    /* Dispatch on opcode. */
    switch (opcode)
    {
    case gcSL_CALL:
        {
        /* Add dependencies for input and output arguments. */
        gcOPT_FUNCTION function = Code->callee->function;
        gcsFUNCTION_ARGUMENT_PTR argument;
        gcOPT_GLOBAL_USAGE usage;
        gctUINT i;

        /* Add dependencies for all input arguments, and add users for all output arguments. */
        argument = function->arguments;
        for (i = 0; i < function->argumentCount; i++, argument++)
        {
            gctUINT indexA = argument->index;
            gctUINT enable = argument->enable;

            /* Set input arguments. */
            if (argument->qualifier != gcvFUNCTION_OUTPUT)
            {
                gcmERR_RETURN(_AddTempUsage(Optimizer,
                                    &TempDefineArray[indexA],
                                    enable,
                                    &Code->dependencies0,
                                    Code,
                                    _needSuccessiveReg(Optimizer, indexA)));
            }

            /* Set output arguments. */
            if (argument->qualifier != gcvFUNCTION_INPUT)
            {
                gcmERR_RETURN(_SetTempDefine(Optimizer,
                                    &TempDefineArray[indexA],
                                    enable,
                                    Code));
            }
        }

        /* Add global variable dependencies and users. */
        for (usage = function->globalUsage; usage; usage = usage->next)
        {
            gctUINT indexT = usage->index;

            if (usage->direction != gcvFUNCTION_OUTPUT)
            {
                if (Optimizer->tempArray[indexT].arrayVariable && !GetVariableIsPerVertex(Optimizer->tempArray[indexT].arrayVariable))
                {
                    gcVARIABLE variable = Optimizer->tempArray[indexT].arrayVariable;

                    gctUINT startIndex, endIndex;
                    gcmASSERT(isVariableNormal(variable));
                    gcSHADER_GetVariableIndexingRange(Optimizer->shader, variable, gcvFALSE,
                                                      &startIndex, &endIndex);
                    gcmASSERT(startIndex == variable->tempIndex);

                    for (i = startIndex; i < endIndex; i++)
                    {
                        gcmERR_RETURN(_AddTempUsage(Optimizer,
                                                    &TempDefineArray[i],
                                                    usage->useEnable,
                                                    &Code->dependencies0,
                                                    Code,
                                                    _needSuccessiveReg(Optimizer, i)));
                    }
                }
                else
                {
                    gcmERR_RETURN(_AddTempUsage(Optimizer,
                                                &TempDefineArray[indexT],
                                                usage->useEnable,
                                                &Code->dependencies0,
                                                Code,
                                                _needSuccessiveReg(Optimizer, indexT)));
                }
            }
            if (usage->direction != gcvFUNCTION_INPUT)
            {
                if (Optimizer->tempArray[indexT].arrayVariable && !GetVariableIsPerVertex(Optimizer->tempArray[indexT].arrayVariable))
                {
                    gcVARIABLE variable = Optimizer->tempArray[indexT].arrayVariable;

                    gctUINT startIndex, endIndex;
                    gcmASSERT(isVariableNormal(variable));
                    gcSHADER_GetVariableIndexingRange(Optimizer->shader, variable, gcvFALSE,
                                                      &startIndex, &endIndex);
                    gcmASSERT(startIndex == variable->tempIndex);

                    for (i = startIndex; i < endIndex; i++)
                    {
                        gcmERR_RETURN(_SetTempDefine(Optimizer,
                                                     &TempDefineArray[i],
                                                     usage->defEnable,
                                                     Code));
                    }
                }
                else
                {
                    gcmERR_RETURN(_SetTempDefine(Optimizer,
                                                 &TempDefineArray[indexT],
                                                 usage->defEnable,
                                                 Code));
                }
            }
        }
        }
        break;

    case gcSL_RET:
        /* If we are in the main function, check the undefined output variable. */
        if (!gcdHasOptimization(Optimizer->option, gcvOPTIMIZATION_RECOMPILER) &&
            Code->function == gcvNULL &&
            !Optimizer->checkOutputUse)
        {
            if (Optimizer->shader->type != gcSHADER_TYPE_GEOMETRY)
            {
                gcmVERIFY_OK(_InsertInitializerInstForOutput(Optimizer, Code, TempDefineArray));
            }
        }

        /* Function returns, so add output dependancy and clean up TempDefineArray. */
        gcmVERIFY_OK(_AddFunctionOutputUsage(Optimizer,
                                    Code->function,
                                    TempDefineArray));

        gcmFOOTER();
        return gcvSTATUS_OK;

    case gcSL_STORE:
    case gcSL_IMAGE_WR:
    case gcSL_IMAGE_WR_3D:
        /* Get gcSL_TARGET field. */
        target = code->temp;

        /* Target is actually source, so add usage. */
        gcmERR_RETURN(_AddTempUsage(Optimizer,
                            &TempDefineArray[code->tempIndex],
                            gcmSL_TARGET_GET(target, Enable),
                           &Code->dependencies1,
                            Code,
                            _needSuccessiveReg(Optimizer, code->tempIndex)));

        if (gcmSL_TARGET_GET(target, Indexed) != gcSL_NOT_INDEXED)
        {
            gctUINT enable = 0;

            switch (gcmSL_TARGET_GET(target, Indexed))
            {
            case gcSL_INDEXED_X:
                enable = gcSL_ENABLE_X; break;
            case gcSL_INDEXED_Y:
                enable = gcSL_ENABLE_Y; break;
            case gcSL_INDEXED_Z:
                enable = gcSL_ENABLE_Z; break;
            case gcSL_INDEXED_W:
                enable = gcSL_ENABLE_W; break;
            }

            index = code->tempIndexed;

            gcmERR_RETURN(_AddTempUsage(Optimizer,
                                &TempDefineArray[gcmSL_INDEX_GET(index, Index)],
                                enable,
                                &Code->dependencies1,
                                Code,
                                _needSuccessiveReg(Optimizer, gcmSL_INDEX_GET(index, Index))));

            /* Mark all array temp registers as inputs. */
            {
                gctUINT i;

                if (Optimizer->tempArray[code->tempIndex].arrayVariable)
                {
                    gcVARIABLE variable = Optimizer->tempArray[code->tempIndex].arrayVariable;

                    gctUINT startIndex, endIndex;
                    gcmASSERT(isVariableNormal(variable));
                    gcSHADER_GetVariableIndexingRange(Optimizer->shader, variable, gcvFALSE,
                                                      &startIndex, &endIndex);
                    gcmASSERT(startIndex == variable->tempIndex);

                    for (i = startIndex; i < endIndex; i++)
                    {
                        gcmERR_RETURN(_AddTempUsage(Optimizer,
                                &TempDefineArray[i],
                                gcmSL_TARGET_GET(target, Enable),
                                &Code->dependencies1,
                                Code,
                                _needSuccessiveReg(Optimizer, i)));
                    }
                }
                else
                {
                    gcmASSERT(Optimizer->tempArray[code->tempIndex].arrayVariable);
                }
            }
        }

        /* Add output usage. */
        gcmERR_RETURN(gcOpt_AddIndexToList(Optimizer, &Code->users, gcvOPT_OUTPUT_REGISTER));

        break;

    case gcSL_STORE1:
        /* Add output usage. */
        gcmERR_RETURN(gcOpt_AddIndexToList(Optimizer, &Code->users, gcvOPT_OUTPUT_REGISTER));
        break;

    case gcSL_ATOMADD:
    case gcSL_ATOMSUB:
    case gcSL_ATOMXCHG:
    case gcSL_ATOMCMPXCHG:
    case gcSL_ATOMMIN:
    case gcSL_ATOMMAX:
    case gcSL_ATOMOR:
    case gcSL_ATOMAND:
    case gcSL_ATOMXOR:

    case gcSL_ATTR_ST:
        /* Add output usage. */
        gcmERR_RETURN(gcOpt_AddIndexToList(Optimizer, &Code->users, gcvOPT_OUTPUT_REGISTER));
        /* fall thruogh */

    default:
        if (gcSL_isOpcodeHaveNoTarget(opcode))
        {
            break;
        }
        if (opcode == gcSL_SET)
        {
            /* Skip specail SET.Z for select/cmp. */
            if (gcmSL_TARGET_GET(code->temp, Condition) == gcSL_ZERO)
            {
                break;
            }
        }

        /* Get gcSL_TARGET field. */
        target = code->temp;

        /* Set instruction to definitions. */
        gcmERR_RETURN(_SetTempDefine(Optimizer,
                                    &TempDefineArray[code->tempIndex],
                                    gcmSL_TARGET_GET(target, Enable),
                                    Code));

        /* Set temp format. */
        format = gcmSL_TARGET_GET(target, Format);
        if (tempArray[code->tempIndex].format == -1)
        {
            tempArray[code->tempIndex].format = format;
        }
        else
        {
            if (tempArray[code->tempIndex].format != format)
            {
                if (tempArray[code->tempIndex].format != -2)
                {
                    if (format == gcSL_FLOAT || tempArray[code->tempIndex].format == gcSL_FLOAT)
                    {
                        tempArray[code->tempIndex].format = -2;
                    }
                }
            }
        }

        if (gcmSL_TARGET_GET(target, Indexed) != gcSL_NOT_INDEXED)
        {
            gctUINT i;

            if (Optimizer->tempArray[code->tempIndex].arrayVariable)
            {
                gcVARIABLE variable = Optimizer->tempArray[code->tempIndex].arrayVariable;

                gctUINT startIndex, endIndex;
                gcmASSERT(isVariableNormal(variable));
                gcSHADER_GetVariableIndexingRange(Optimizer->shader, variable, gcvFALSE,
                                                  &startIndex, &endIndex);
                gcmASSERT(startIndex == variable->tempIndex);

                for (i = startIndex; i < endIndex; i++)
                {
                    gcmERR_RETURN(_SetTempDefine(Optimizer,
                                    &TempDefineArray[i],
                                    gcmSL_TARGET_GET(target, Enable),
                                    Code));
                    tempArray[i].format = format;
                }
            }
            else
            {
                /* This temp register should be an array function argument. */
            }
        }

        targetIsTemp = gcvTRUE;

        /* TODO - need to handle indexed mode? */

        break;
    }

    /* Determine usage of source0. */
    source = code->source0;

    if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
    {
        gctUINT enable = _GetEnableFromSwizzles((gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleX),
                            (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleY),
                            (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleZ),
                            (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleW));

        if (targetIsTemp && code->source0Index == code->tempIndex)
        {
            gcmASSERT(code->source0Index != code->tempIndex);
            status = gcvSTATUS_INVALID_ARGUMENT;
            gcmFOOTER();
            return status;
        }

        gcmERR_RETURN(_AddTempUsage(Optimizer,
                            &TempDefineArray[code->source0Index],
                            enable,
                            &Code->dependencies0,
                            Code,
                            _needSuccessiveReg(Optimizer, code->source0Index)));
    }

    if (gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
    {
        gctUINT enable = 0;

        switch (gcmSL_SOURCE_GET(source, Indexed))
        {
        case gcSL_INDEXED_X:
            enable = gcSL_ENABLE_X; break;
        case gcSL_INDEXED_Y:
            enable = gcSL_ENABLE_Y; break;
        case gcSL_INDEXED_Z:
            enable = gcSL_ENABLE_Z; break;
        case gcSL_INDEXED_W:
            enable = gcSL_ENABLE_W; break;
        }

        index = code->source0Indexed;

        gcmERR_RETURN(_AddTempUsage(Optimizer,
                            &TempDefineArray[gcmSL_INDEX_GET(index, Index)],
                            enable,
                            &Code->dependencies0,
                            Code,
                            _needSuccessiveReg(Optimizer, gcmSL_INDEX_GET(index, Index))));

        /* If the source is temp register, mark all array temp registers as inputs. */
        if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
        {
            gctUINT i;
            enable = _GetEnableFromSwizzles((gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleX),
                            (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleY),
                            (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleZ),
                            (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleW));

            if (Optimizer->tempArray[code->source0Index].arrayVariable)
            {
                gcVARIABLE variable = Optimizer->tempArray[code->source0Index].arrayVariable;

                gctUINT startIndex, endIndex;
                gcmASSERT(isVariableNormal(variable));
                gcSHADER_GetVariableIndexingRange(Optimizer->shader, variable, gcvFALSE,
                                                  &startIndex, &endIndex);
                gcmASSERT(startIndex == variable->tempIndex);

                for (i = startIndex; i < endIndex; i++)
                {
                    gcmERR_RETURN(_AddTempUsage(Optimizer,
                            &TempDefineArray[i],
                            enable,
                            &Code->dependencies0,
                            Code,
                            _needSuccessiveReg(Optimizer, i)));
                }
            }
            else
            {
                /* This temp register should be an array function argument. */
            }
        }
    }

    /* TODO - Special case for TEXLD? */

    /* Determine usage of source1. */
    source = code->source1;

    if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
    {
        gctUINT enable = _GetEnableFromSwizzles((gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleX),
                            (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleY),
                            (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleZ),
                            (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleW));

        if (targetIsTemp && code->source1Index == code->tempIndex)
        {
            gcmASSERT(code->source1Index != code->tempIndex);
            status = gcvSTATUS_INVALID_ARGUMENT;
            gcmFOOTER();
            return status;
        }

        gcmERR_RETURN(_AddTempUsage(Optimizer,
                            &TempDefineArray[code->source1Index],
                            enable,
                            &Code->dependencies1,
                            Code,
                            _needSuccessiveReg(Optimizer, code->source1Index)));
    }

    if (gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
    {
        gctUINT enable = 0;

        switch (gcmSL_SOURCE_GET(source, Indexed))
        {
        case gcSL_INDEXED_X:
            enable = gcSL_ENABLE_X; break;
        case gcSL_INDEXED_Y:
            enable = gcSL_ENABLE_Y; break;
        case gcSL_INDEXED_Z:
            enable = gcSL_ENABLE_Z; break;
        case gcSL_INDEXED_W:
            enable = gcSL_ENABLE_W; break;
        }

        index = code->source1Indexed;

        gcmERR_RETURN(_AddTempUsage(Optimizer,
                            &TempDefineArray[gcmSL_INDEX_GET(index, Index)],
                            enable,
                            &Code->dependencies1,
                            Code,
                            _needSuccessiveReg(Optimizer, gcmSL_INDEX_GET(index, Index))));

        /* If the source is temp register, mark all array temp registers as inputs. */
        if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
        {
            gctUINT i;
            enable = _GetEnableFromSwizzles((gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleX),
                            (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleY),
                            (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleZ),
                            (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleW));

            if (Optimizer->tempArray[code->source1Index].arrayVariable)
            {
                gcVARIABLE variable = Optimizer->tempArray[code->source1Index].arrayVariable;

                gctUINT startIndex, endIndex;
                gcmASSERT(isVariableNormal(variable));
                gcSHADER_GetVariableIndexingRange(Optimizer->shader, variable, gcvFALSE,
                                                  &startIndex, &endIndex);
                gcmASSERT(startIndex == variable->tempIndex);

                for (i = startIndex; i < endIndex; i++)
                {
                    gcmERR_RETURN(_AddTempUsage(Optimizer,
                            &TempDefineArray[i],
                            enable,
                            &Code->dependencies1,
                            Code,
                            _needSuccessiveReg(Optimizer, i)));
                }
            }
            else
            {
                gcmASSERT(Optimizer->tempArray[code->source1Index].arrayVariable);
            }
        }
    }

    /* Determine usage of dst index register. */
    target = code->temp;

    if (gcmSL_TARGET_GET(target, Indexed) != gcSL_NOT_INDEXED)
    {
        /* Mark usage for index register */
        gctUINT enable = 0;

        switch (gcmSL_TARGET_GET(target, Indexed))
        {
        case gcSL_INDEXED_X:
            enable = gcSL_ENABLE_X; break;
        case gcSL_INDEXED_Y:
            enable = gcSL_ENABLE_Y; break;
        case gcSL_INDEXED_Z:
            enable = gcSL_ENABLE_Z; break;
        case gcSL_INDEXED_W:
            enable = gcSL_ENABLE_W; break;
        }

        index = code->tempIndexed;

        gcmERR_RETURN(_AddTempUsage(Optimizer,
                            &TempDefineArray[gcmSL_INDEX_GET(index, Index)],
                            enable,
                            &Code->dependencies1,
                            Code,
                            _needSuccessiveReg(Optimizer, gcmSL_INDEX_GET(index, Index))));
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_BuildFunctionFlowGraph(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_FUNCTION   Function
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_TEMP_DEFINE   tempDefineArray = gcvNULL;
    gcOPT_CODE          code;
    gcOPT_CODE          lastBackwardCallee = gcvNULL;

    gcmHEADER_ARG("Optimizer=0x%x Function=0x%x",
                   Optimizer, Function);
    /* Allocate array of lists to save where temp registers are defined. */
    if (Optimizer->tempCount)
    {
        gcmERR_RETURN(_CAllocateTempDefineArray(Optimizer->tempDefineArrayMemPool, &tempDefineArray, Optimizer->tempCount));
    }

    /* Add input and global dependancies. */
    if (Function != Optimizer->main)
    {
        gcmVERIFY_OK(_AddFunctionInputDefine(Optimizer, Function, tempDefineArray));
    }

    gcmVERIFY_OK(_AddUndefined(Optimizer, tempDefineArray));

    /* Build data flow. */
    for (code = Function->codeHead;
         code != Function->codeTail->next;
         code = code->next)
    {
        if(lastBackwardCallee && code->id == lastBackwardCallee->id)
        {
            lastBackwardCallee = gcvNULL;
        }

        if (code->callers)
        {
            gctBOOL isBackJump = gcvFALSE;
            gcOPT_LIST callers = code->callers;

            while(callers)
            {
                if (callers->code && callers->code->backwardJump)
                {
                    if(lastBackwardCallee == gcvNULL && !callers->code->handled)
                    {
                        lastBackwardCallee = code;
                    }
                    isBackJump = gcvTRUE;
                    break;
                }
                callers = callers->next;
            }

            if (code->tempDefine)
            {
                gcmERR_RETURN(_MergeTempDefineArray(Optimizer,
                                        code->tempDefine, isBackJump, &tempDefineArray));
            }

            if(lastBackwardCallee == gcvNULL)
            {
                if(code->tempDefine)
                {
                    gcmVERIFY_OK(gcOpt_ClearTempArray(Optimizer, code->tempDefine));
                    gcmVERIFY_OK(_FreeTempDefineArray(Optimizer->tempDefineArrayMemPool, code->tempDefine));
                    code->tempDefine = gcvNULL;
                }
            }
            else
            {
                gcmERR_RETURN(_MergeTempDefineArray(Optimizer,
                                            tempDefineArray, isBackJump, &code->tempDefine));
            }
        }

        gcmERR_RETURN(_BuildDataFlowForCode(Optimizer, code, tempDefineArray));

        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_JMP)
        {
            gcOPT_CODE codeDest = code->callee;
            gctINT codeID = code->id;
            gcSL_CONDITION condition = gcmSL_TARGET_GET(code->instruction.temp, Condition);

            if (! code->backwardJump)
            {
                gcmERR_RETURN(_MergeTempDefineArray(Optimizer,
                                        tempDefineArray, gcvFALSE, &codeDest->tempDefine));
            }
            else if (! code->handled)
            {
                code->handled = gcvTRUE;
                for (code = code->prev;
                     code != gcvNULL && code != codeDest->prev;
                     code = code->prev)
                {
                    /* Reset handled flag for the codes in between. */
                    if (code->backwardJump)
                    {
                        gcmASSERT(gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_JMP ? code->handled : 1);
                        code->handled = gcvFALSE;
                    }
                }

                /* If there is a for loop, we need to add dependencies for the condition code. */
                if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_JMP
                    && condition != gcSL_ALWAYS
                    && code->instruction.tempIndex == codeID + 1)
                {
                    code = code->prev;
                }

                continue;
            }

            /* Clear tempDefines if the jump is unconditional. */
            if (gcmSL_TARGET_GET(code->instruction.temp, Condition) == gcSL_ALWAYS)
            {
                gcmVERIFY_OK(gcOpt_ClearTempArray(Optimizer, tempDefineArray));
            }
        }
    }

    /* Output dependancies are already added by RET instruction (main program has extra RET instruction). */

    /* tempDefineArray's lists are freed in _AddOutputUsage. */
    /* Free tempDefineArray. */
    if (tempDefineArray)
    {
        gcmVERIFY_OK(_FreeTempDefineArray(Optimizer->tempDefineArrayMemPool, tempDefineArray));
    }

    /* Free tempDefines in codes. */
    for (code = Function->codeHead;
         code != gcvNULL && code != Function->codeTail->next;
         code = code->next)
    {
        if (code->tempDefine)
        {
            gcmVERIFY_OK(gcOpt_ClearTempArray(Optimizer, code->tempDefine));
            gcmVERIFY_OK(_FreeTempDefineArray(Optimizer->tempDefineArrayMemPool, code->tempDefine));
            code->tempDefine = gcvNULL;
        }
    }

    gcmFOOTER();
    return status;
}

static gctBOOL
_CheckFuncCallHasEmit(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_CODE       Code
    )
{
    gctBOOL             hasEmit = gcvFALSE;
    gcOPT_CODE          code;
    gcOPT_FUNCTION      function = Code->callee->function;

    if (function->hasEmitCode)
        return gcvTRUE;

    for (code = function->codeHead; code != function->codeTail->next; code = code->next)
    {
        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_CALL)
            continue;

        hasEmit = _CheckFuncCallHasEmit(Optimizer, code);

        if (hasEmit)
        {
            function->hasEmitCode = hasEmit;
            break;
        }
    }

    return hasEmit;
}

static gceSTATUS
_BuildEmitOutputUsageForCode(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_FUNCTION   Function,
    IN gctBOOL          IsMainFunc
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_CODE          code, prevCode;
    gcSL_OPCODE         opcode, prevOpcode;
    gcOUTPUT            output = gcvNULL;
    gctBOOL             checkOutput, callHasEmit;

    /* Search this function. */
    for (code = Function->codeHead; code != Function->codeTail->next; code = code->next)
    {
        if (!code) continue;
        opcode = gcmSL_OPCODE_GET(code->instruction.opcode, Opcode);
        callHasEmit = gcvFALSE;

        /* Check function call. */
        if (opcode == gcSL_CALL && IsMainFunc)
        {
            callHasEmit = _CheckFuncCallHasEmit(Optimizer, code);
        }

        if (opcode != gcSL_EMIT_VERTEX && opcode != gcSL_END_PRIMITIVE &&
            !callHasEmit)
        {
            continue;
        }

        /* This function has EMIT code. */
        Function->hasEmitCode = gcvTRUE;

        /* Check output assignment. */
        for (prevCode = code->prev; prevCode != Function->codeHead->prev; prevCode = prevCode->prev)
        {
            if (!prevCode) continue;

            prevOpcode = gcmSL_OPCODE_GET(prevCode->instruction.opcode, Opcode);

            if (prevOpcode == gcSL_EMIT_VERTEX || prevOpcode == gcSL_END_PRIMITIVE)
                break;

            switch(prevOpcode)
            {
            case gcSL_STORE:
            case gcSL_STORE1:
            case gcSL_IMAGE_WR:
            case gcSL_IMAGE_WR_3D:
            case gcSL_RET:
            case gcSL_CALL:
            case gcSL_JMP:
            case gcSL_NOP:
            case gcSL_KILL:
            case gcSL_TEXBIAS:
            case gcSL_TEXGRAD:
            case gcSL_TEXGATHER:
            case gcSL_TEXFETCH_MS:
            case gcSL_TEXLOD:
            case gcSL_ATOMADD:
            case gcSL_ATOMSUB:
            case gcSL_ATOMXCHG:
            case gcSL_ATOMCMPXCHG:
            case gcSL_ATOMMIN:
            case gcSL_ATOMMAX:
            case gcSL_ATOMOR:
            case gcSL_ATOMAND:
            case gcSL_ATOMXOR:
                checkOutput = gcvFALSE;
                break;
            default:
                checkOutput = gcvTRUE;
                break;
            }

            if (!checkOutput) continue;
            /* Check if this temp is an output. */
            gcmONERROR(gcSHADER_GetOutputByTempIndex(Optimizer->shader,
                                                     prevCode->instruction.tempIndex,
                                                     &output));
            if (output == gcvNULL) continue;
            /* Add the code to the users list. */
            gcmONERROR(gcOpt_AddCodeToList(Optimizer,
                                           &prevCode->users,
                                           code));
        }
    }

OnError:
    return status;
}

static gceSTATUS
_BuildEmitOutputUsage(
    IN gcOPTIMIZER      Optimizer
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gctUINT             i;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    /* Right now only GS has EMIT code. */
    if (Optimizer->shader->type != gcSHADER_TYPE_GEOMETRY)
    {
        gcmFOOTER();
        return status;
    }

    for (i = 0; i < Optimizer->functionCount; i++)
    {
        gcmONERROR(_BuildEmitOutputUsageForCode(Optimizer,
                                                Optimizer->functionArray + i,
                                                gcvFALSE));
    }

    gcmONERROR(_BuildEmitOutputUsageForCode(Optimizer,
                                            Optimizer->main,
                                            gcvTRUE));

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcOpt_CalculateStackCallDepth(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_FUNCTION   Function,
    OUT gctINT *        Depth
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_CODE          code;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    if (Function == gcvNULL)
    {
        gcmFOOTER();
        return status;
    }

    Function->maxDepthFunc = gcvNULL;
    Function->maxDepthValue = -1;

    /* Calculate the stack call depth for all callees. */
    for (code = Function->codeHead; code != gcvNULL && code != Function->codeTail->next; code = code->next)
    {
        gctINT depth = 0;

        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_CALL)
            continue;

        gcOpt_CalculateStackCallDepth(Optimizer, code->callee->function, &depth);

        /* Update the max call stack depth. */
        if (depth + 1 > Function->maxDepthValue)
        {
            Function->maxDepthValue = depth + 1;
            Function->maxDepthFunc = code->callee->function;
        }
    }

    if (Function->maxDepthValue == -1)
    {
        Function->maxDepthValue = 0;
        Function->maxDepthFunc = gcvNULL;
    }

    if (Depth)
    {
        *Depth = Function->maxDepthValue;
    }

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                            gcOpt_UpdateCallStackDepth
********************************************************************************
**
**    Update the call stack depth for functions.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a Optimizer structure.
**
**        gctBOOL ForceUpdate
**            Force to update call stack depth.
*/
gceSTATUS
gcOpt_UpdateCallStackDepth(
    IN gcOPTIMIZER      Optimizer,
    IN gctBOOL          ForceUpdate
    )
{
    gceSTATUS           status = gcvSTATUS_FALSE;
    gcOPT_FUNCTION      functionArray = Optimizer->functionArray;
    gctINT              i;
    gcOPT_FUNCTION      function;

    gcmHEADER_ARG("Optimizer=%p", Optimizer);

    /* GLSLES don't support recursion functions,
    ** so if the function count is less than _MAX_CALL_STACK_DEPTH_,
    ** then the call stack depth would never exceed _MAX_CALL_STACK_DEPTH_.
    */
    if (Optimizer->functionCount < _MAX_CALL_STACK_DEPTH_ && !ForceUpdate)
    {
        gcmFOOTER();
        return status;
    }

    /* Id's will be used to calculate functions' codeCount. */
    gcOpt_UpdateCodeId(Optimizer);

    /* Reset max depth value for all functions. */
    for (i = Optimizer->functionCount - 1; i >= 0; i--)
    {
        function = functionArray + i;
        function->maxDepthValue = -1;
        function->maxDepthFunc = gcvNULL;
    }

    for (i = Optimizer->functionCount - 1; i >= 0; i--)
    {
        gctINT                depth = 0;
        function = functionArray + i;

        gcOpt_CalculateStackCallDepth(Optimizer, function, &depth);

        if (function->maxDepthValue + 1 >= _MAX_CALL_STACK_DEPTH_)
            status = gcvSTATUS_TRUE;
    }

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                            gcOpt_BuildFlowGraph
********************************************************************************
**
**    Build control flow and data flow.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a Optimizer structure.
*/
gceSTATUS
gcOpt_BuildFlowGraph(
    IN gcOPTIMIZER      Optimizer
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_FUNCTION      functionArray = Optimizer->functionArray;
    gctUINT             i;

    gcmHEADER_ARG("Optimizer=%p", Optimizer);

#if gcmIS_DEBUG(gcdDEBUG_ASSERT)
    /* Sanity check. */
    if (Optimizer->functionCount == 0)
    {
        gcmASSERT(Optimizer->functionArray == gcvNULL);
        gcmASSERT(Optimizer->main->codeHead == Optimizer->codeHead);
        gcmASSERT(Optimizer->main->codeTail  == Optimizer->codeTail);
        gcmASSERT(Optimizer->codeHead->id == 0);
        /*gcmASSERT(Optimizer->codeTail->id == Optimizer->codeCount - 1);*/
    }
    else
    {
        gcOPT_CODE prevCode = gcvNULL;
        gctBOOL findMain = gcvFALSE;
        gctBOOL mainFirst = gcvFALSE;

        gcmASSERT(functionArray);
        if (functionArray == gcvNULL)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            gcmFOOTER();
            return status;
        }

        for (i = 0; i < Optimizer->functionCount; i++)
        {
            if (functionArray[i].codeHead->prev == prevCode)
            {
                gcmASSERT(! prevCode || prevCode->next == functionArray[i].codeHead);
                prevCode = functionArray[i].codeTail;
            }
            else
            {
                gcmASSERT(! findMain);
                findMain = gcvTRUE;
                gcmASSERT(! prevCode || prevCode->next == Optimizer->main->codeHead);
                if (! prevCode)
                {
                    mainFirst = gcvTRUE;
                }
                gcmASSERT(Optimizer->main->codeHead->prev == prevCode);
                prevCode = Optimizer->main->codeTail;
                i--;  /* Recheck the function again. */
            }
        }

        if (!prevCode)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            gcmFOOTER();
            return status;
        }

        if (findMain)
        {
            gcmASSERT(prevCode == Optimizer->codeTail);
            gcmASSERT(prevCode->next == gcvNULL);
            if (mainFirst)
            {
                gcmASSERT(Optimizer->main->codeHead == Optimizer->codeHead);
                gcmASSERT(prevCode->next == gcvNULL);
            }
        }
        else
        {
            gcmASSERT(Optimizer->main->codeHead->prev == prevCode);
            gcmASSERT(prevCode->next == Optimizer->main->codeHead);
            gcmASSERT(Optimizer->main->codeTail == Optimizer->codeTail);
        }
    }
#endif

    if (Optimizer->functionCount > 0)
    {
        gcmERR_RETURN(_BuildGlobalUsage(Optimizer));
    }

    /* Build data flow for main. */
    gcmERR_RETURN(_BuildFunctionFlowGraph(Optimizer, Optimizer->main));

    /* Build data flow for each function. */
    for (i = 0; i < Optimizer->functionCount; i++)
    {
        gcmERR_RETURN(_BuildFunctionFlowGraph(Optimizer, functionArray + i));
    }

    /* Build output usage for EMIT code. */
    gcmERR_RETURN(_BuildEmitOutputUsage(Optimizer));

    /*DUMP_OPTIMIZER("Build data flow for the shader", Optimizer);*/
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                            gcOpt_DestroyCodeFlowData
********************************************************************************
**
**    Free code's data flow data.
**
**    INPUT:
**
**        gcOPT_CODE            code
**            Pointer to a gcOPTIMIZER structure.
*/
void
gcOpt_DestroyCodeFlowData(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_CODE   code
    )
{
    code->handled = gcvFALSE;
    if (code->dependencies0)
    {
        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &code->dependencies0));
    }

    if (code->dependencies1)
    {
        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &code->dependencies1));
    }

    if (code->users)
    {
        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &code->users));
    }

    if (code->prevDefines)
    {
        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &code->prevDefines));
    }

    if (code->nextDefines)
    {
        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &code->nextDefines));
    }
}

/*******************************************************************************
**                            gcOpt_DestroyFlowGraph
********************************************************************************
**
**    Free control flow and data flow data.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_DestroyFlowGraph(
    IN gcOPTIMIZER      Optimizer
    )
{
    gcOPT_CODE          code;

    gcmHEADER_ARG("Optimizer=%p", Optimizer);

    if (Optimizer->functionArray != gcvNULL)
    {
        gcOPT_FUNCTION functionArray = Optimizer->functionArray;
        gcOPT_FUNCTION function;

        function = functionArray + Optimizer->functionCount - 1;
        for (; function >= functionArray; function--)
        {
            /* Free global variable usage list. */
            while (function->globalUsage)
            {
                gcOPT_GLOBAL_USAGE usage = function->globalUsage;
                function->globalUsage = usage->next;
                gcmVERIFY_OK(_FreeGlobalUsage(Optimizer->usageMemPool, usage));
            }
        }
    }

    /* Free data flow. */
    for (code = Optimizer->codeHead; code; code = code->next)
    {
        gcOpt_DestroyCodeFlowData(Optimizer, code);
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**                            gcOpt_RebuildFlowGraph
********************************************************************************
**
**    Rebuild flow graph.
**        This is used after functionCount change or code change that does not
**            update flow graph properly.
**        Assume tempCount is not changed.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_RebuildFlowGraph(
    IN gcOPTIMIZER      Optimizer
    )
{
    gceSTATUS           status = gcvSTATUS_OK;

    gcmHEADER_ARG("Optimizer=%p", Optimizer);

    /* Free flow graph. */
    gcmVERIFY_OK(gcOpt_DestroyFlowGraph(Optimizer));

    /* Update code id. */
    gcOpt_UpdateCodeId(Optimizer);

    /* Rebuild flow graph. */
    gcmONERROR(gcOpt_BuildFlowGraph(Optimizer));

    /* DUMP_OPTIMIZER("Update the flow graph of the shader", Optimizer); */
    gcmFOOTER();
    return status;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                            gcOpt_ConstructOptimizer
********************************************************************************
**
**    Construct a new gcOPTIMIZER structure.
**
**    INPUT:
**
**        gcSHADER Shader
**            Pointer to a gcSHADER object holding information about the compiled shader.
**
**    OUT:
**
**        gcOPTIMIZER * Optimizer
**            Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_ConstructOptimizer(
    IN gcSHADER         Shader,
    OUT gcOPTIMIZER *   Optimizer
    )
{
    gceSTATUS           status;
    gcOPTIMIZER         optimizer = gcvNULL;
    gctPOINTER          pointer = gcvNULL;

    gcmHEADER_ARG("Shader=0x%x", Shader);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Shader, gcvOBJ_SHADER);
    gcmVERIFY_ARGUMENT(Optimizer != gcvNULL);

    /* Allocate optimizer data structure. */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              gcmSIZEOF(struct _gcOPTIMIZER),
                              &pointer));
    optimizer = pointer;

    gcoOS_ZeroMemory(optimizer, gcmSIZEOF(struct _gcOPTIMIZER));

    optimizer->globalOptions = &theOptimizerOption;
    optimizer->option = Shader->optimizationOption;

    gcmONERROR(_MemPoolInit(optimizer));

    gcmONERROR(gcOpt_CopyInShader(optimizer, Shader));

    /* Query the hardware. */
    gcmONERROR(gcQueryShaderCompilerHwCfg(gcvNULL, &optimizer->hwCfg));

    optimizer->isHalti2 = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI2);
    optimizer->supportImmediate = (optimizer->hwCfg.chipRevision >= 0x5310);

    gcmVERIFY_OK(gcoHAL_QueryShaderCaps(
                   gcvNULL,
                   gcvNULL,
                   &optimizer->maxVertexUniforms,
                   &optimizer->maxFragmentUniforms,
                   &optimizer->maxVaryings,
                   &optimizer->maxShaderCoreCount,
                   &optimizer->maxThreadCount,
                   &optimizer->maxVertexInstructionCount,
                   &optimizer->maxFragmentInstructionCount));

    optimizer->isCTSInline = gcvFALSE;

    /* Build temp register array. */
    gcmONERROR(_BuildTempArray(optimizer));

    /* Pack main program if necessary. */
    gcmONERROR(_PackMainProgram(optimizer));

    /* Build flow graph. */
    gcmONERROR(gcOpt_BuildFlowGraph(optimizer));

    *Optimizer = optimizer;
    gcmFOOTER_ARG("*Optimizer=0x%x", *Optimizer);
    return gcvSTATUS_OK;

OnError:
    if (optimizer != gcvNULL)
    {
        /* Destroy the optimizer. */
        gcOpt_DestroyOptimizer(optimizer);
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                            gcOpt_ReconstructOptimizer
********************************************************************************
**
**    Reconstruct a gcOPTIMIZER structure.
**
**    INPUT:
**
**        gcSHADER Shader
**            Pointer to a gcSHADER object holding information about the compiled shader.
**
**    OUT:
**
**        gcOPTIMIZER * Optimizer
**            Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_ReconstructOptimizer(
    IN gcSHADER         Shader,
    OUT gcOPTIMIZER *   OptimizerPtr
    )
{
    gceSTATUS           status;
    gcOPTIMIZER         optimizer = *OptimizerPtr;

    gcmHEADER_ARG("Shader=0x%x", Shader);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Shader, gcvOBJ_SHADER);
    gcmVERIFY_ARGUMENT(optimizer != gcvNULL);

    gcmONERROR(gcOpt_CopyOutShader(optimizer, Shader));
    /* Free optimizer. */
    gcmVERIFY_OK(gcOpt_DestroyOptimizer(optimizer));
    /* reconstruct the optimizer. */
    gcmONERROR(gcOpt_ConstructOptimizer(Shader, OptimizerPtr));

OnError:

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                               gcOpt_DestroyOptimizer
********************************************************************************
**
**    Destroy a gcOPTIMIZER structure.
**
**    IN OUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_DestroyOptimizer(
    IN OUT gcOPTIMIZER  Optimizer
    )
{
    gcmHEADER_ARG("Optimizer=%p", Optimizer);

    if (Optimizer == gcvNULL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    /* Free main and function array. */
    gcmVERIFY_OK(_DestroyFunctionArray(Optimizer));

    /* Free temp reg array. */
    gcmVERIFY_OK(_DestroyTempArray(Optimizer));

    /* Free global variable list. */
    if (Optimizer->global)
    {
        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &Optimizer->global));
    }

    /* Free code flow graph. */
    gcmVERIFY_OK(gcOpt_DestroyFlowGraph(Optimizer));

    /* Free code list. */
    gcOpt_RemoveCodeList(Optimizer, Optimizer->codeHead, Optimizer->codeTail);

    /* Free memory pools. */
    _MemPoolCleanup(Optimizer);

    /* Free optimizer data structure. */
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Optimizer));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/******************************************************************************\
|******************** Supporting Functions for Optimization *******************|
\******************************************************************************/

/*******************************************************************************
**                            gcOpt_RemoveNOP
********************************************************************************
**
**    Remove NOP instructions from the shader.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_RemoveNOP(
    IN gcOPTIMIZER      Optimizer
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcOPT_CODE          code, codePrev;
    gcOPT_CODE          nextNonNOPCode = gcvNULL;
    gctUINT             i;

    gcmHEADER_ARG("Optimizer=%p", Optimizer);

    /* Update codeHead and codeTail of main and fucntions. */
    if (Optimizer->main->codeHead->instruction.opcode == gcSL_NOP)
    {
        code = Optimizer->main->codeHead;
        do
        {
            code = code->next;
        }
        while (code != gcvNULL && gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_NOP);
        gcmASSERT(code != gcvNULL);
        gcmASSERT(code->id <= Optimizer->main->codeTail->id);
        Optimizer->main->codeHead = code;
    }

    if (gcmSL_OPCODE_GET(Optimizer->main->codeTail->instruction.opcode, Opcode) == gcSL_NOP)
    {
        code = Optimizer->main->codeTail;
        do
        {
            code = code->prev;
        }
        while (code != gcvNULL && gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_NOP);
        gcmASSERT(code != gcvNULL);
        gcmASSERT(code->id >= Optimizer->main->codeHead->id);
        Optimizer->main->codeTail = code;
    }

    if (Optimizer->functionCount > 0)
    {
        gcOPT_FUNCTION function;

        function = Optimizer->functionArray;
        for (i = 0; i < Optimizer->functionCount; i++, function++)
        {
            if (gcmSL_OPCODE_GET(function->codeHead->instruction.opcode, Opcode) == gcSL_NOP)
            {
                code = function->codeHead;
                do
                {
                    code = code->next;
                }
                while (code != gcvNULL && gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_NOP);
                gcmASSERT(code != gcvNULL);
                gcmASSERT(code->id <= function->codeTail->id);
                function->codeHead = code;
            }

            if (gcmSL_OPCODE_GET(function->codeTail->instruction.opcode, Opcode) == gcSL_NOP)
            {
                code = function->codeTail;
                do
                {
                    code = code->prev;
                }
                while (code != gcvNULL && gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_NOP);
                gcmASSERT(code != gcvNULL);
                gcmASSERT(code->id >= function->codeHead->id);
                function->codeTail = code;
            }
        }
    }

    /* Remove NOP codes and update callers. */
    for (code = Optimizer->codeTail; code; code = codePrev)
    {
        codePrev = code->prev;
        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_NOP)
        {
            nextNonNOPCode = code;
        }
        else
        {
            if (code->callers)
            {
                gcOPT_LIST caller;
                gcOPT_LIST lastCaller = gcvNULL;

                /* Update NOP instruction's callers. */
                if (! nextNonNOPCode)
                {
                    gcmASSERT(nextNonNOPCode);
                    status = gcvSTATUS_INVALID_ARGUMENT;
                    gcmFOOTER();
                    return status;
                }

                for (caller = code->callers;
                     caller != gcvNULL;
                     caller = caller->next)
                {
                    gcmASSERT(caller->code->callee == code);
                    caller->code->callee = nextNonNOPCode;
                    lastCaller = caller;
                }

                if (! lastCaller)
                {
                    gcmASSERT(lastCaller);
                    status = gcvSTATUS_INVALID_ARGUMENT;
                    gcmFOOTER();
                    return status;
                }

                /* Move caller list from code to nextNonNOPCode. */
                lastCaller->next = nextNonNOPCode->callers;
                nextNonNOPCode->callers = code->callers;
                code->callers = gcvNULL;
            }

            gcOpt_RemoveCodeList(Optimizer, code, code);
        }
    }

    /* Update code id. */
    gcOpt_UpdateCodeId(Optimizer);

    DUMP_OPTIMIZER("Removed NOP instructions from the shader", Optimizer);
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                            gcOpt_ChangeCodeToNOP
********************************************************************************
**
**    Change one code to NOP.
**
**    INPUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
**
**        gcSL_INSTRUCTION Code
**            Pointer to code array.
*/
gceSTATUS
gcOpt_ChangeCodeToNOP(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_CODE       Code
    )
{
    gcSL_INSTRUCTION    instr = &Code->instruction;
    gcOPT_LIST          list;

    gcmHEADER_ARG("Optimizer=%p Code=%p", Optimizer, Code);

    /* Update jumpTarget's caller list if needed. */
    if (gcmSL_OPCODE_GET(instr->opcode, Opcode) == gcSL_JMP || gcmSL_OPCODE_GET(instr->opcode, Opcode) == gcSL_CALL)
    {
        gcOPT_LIST caller, prevCaller = gcvNULL;

        /* Remove Index from caller list. */
        for (caller = Code->callee->callers;
             caller != gcvNULL;
             caller = caller->next)
        {
            if (caller->code == Code)
            {
                if (prevCaller)
                {
                    prevCaller->next = caller->next;
                }
                else
                {
                    Code->callee->callers = caller->next;
                }

                gcmVERIFY_OK(_FreeList(Optimizer->listMemPool, caller));
                break;
            }
            prevCaller = caller;
        }
        Code->callee = gcvNULL;
    }

    /* Update previous and next define lists. */
    for (list = Code->prevDefines; list; list = list->next)
    {
        if (list->index >= 0)
        {
            gcOpt_DeleteCodeFromList(Optimizer, &list->code->nextDefines, Code);
        }
    }

    for (list = Code->nextDefines; list; list = list->next)
    {
        gcmASSERT(list->index >= 0);
        gcOpt_DeleteCodeFromList(Optimizer, &list->code->prevDefines, Code);
    }

    if (Code->prevDefines && Code->nextDefines)
    {
        for (list = Code->prevDefines; list; list = list->next)
        {
            if (list->index >= 0)
            {
                gcmVERIFY_OK(gcOpt_AddListToList(Optimizer, Code->nextDefines, gcvFALSE, &list->code->nextDefines));
            }
        }

        for (list = Code->nextDefines; list; list = list->next)
        {
            gcmASSERT(list->index >= 0);
            gcmVERIFY_OK(gcOpt_AddListToList(Optimizer, Code->prevDefines, gcvFALSE, &list->code->prevDefines));
        }
    }

    /* Clean up the data flow of the instruction. */
    while ((list = Code->users) != gcvNULL)
    {
        Code->users = list->next;

        if (list->index >= 0)
        {
            /* Cannot add gcmVERIFY_OK here. */
            gcOpt_DeleteCodeFromList(Optimizer, &list->code->dependencies0, Code);
            gcOpt_DeleteCodeFromList(Optimizer, &list->code->dependencies1, Code);
        }

        /* Free the gcOPT_LIST structure. */
        gcmVERIFY_OK(_FreeList(Optimizer->listMemPool, list));
    }

    while ((list = Code->dependencies0) != gcvNULL)
    {
        Code->dependencies0 = list->next;

        if (list->index >= 0)
        {
            gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &list->code->users, Code));
        }

        /* Free the gcOPT_LIST structure. */
        gcmVERIFY_OK(_FreeList(Optimizer->listMemPool, list));
    }

    while ((list = Code->dependencies1) != gcvNULL)
    {
        Code->dependencies1 = list->next;

        /* It is possible that both sources are the same. */
        if (list->index >= 0)
        {
            gcOpt_DeleteCodeFromList(Optimizer, &list->code->users, Code);
        }

        /* Free the gcOPT_LIST structure. */
        gcmVERIFY_OK(_FreeList(Optimizer->listMemPool, list));
    }

    if (Code->nextDefines)
    {
        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &Code->nextDefines));
    }

    if (Code->prevDefines)
    {
        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &Code->prevDefines));
    }

    /* Make the instruction NOP. */
    Code->instruction = gcvSL_NOP_INSTR;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

#endif


