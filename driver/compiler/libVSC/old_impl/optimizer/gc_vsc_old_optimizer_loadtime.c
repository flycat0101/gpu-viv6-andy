/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/



#include "gc_vsc.h"
#include "old_impl/gc_vsc_old_optimizer.h"

#include "old_impl/gc_vsc_old_optimizer_loadtime.h"
#if gcdENABLE_3D


#define _GC_OBJ_ZONE    gcvZONE_COMPILER
/*
**  Load Time Optimizer module.
**   load time optimizer is divided in two parts, compiler-time optimization
**   to find the load-time constants and do symbolic constant propagation and
**   folding, the purpose is to hoist the constant calculation out of the
**   shader code, put it into shader's data structure so it can be folded to
**   real constant at load time with minimum cost. The load time part maps the
**   constants specified by application to symbols in the shader saved symbolic
**   expression, after evaluating the expression, the constant value of the
**   expression is mapped to compiler genereated fake uniform.
**
**   the shader code may have branches usings the load time constant, in that
**   case we can two alternatives:
**     1. if there is only one or two branches using the load time constants,
**        we can multiversion the shader code to upto four versions and load
**        the correct version at load time depends on the load time value
**     2. if the multiversion is infeasible, and the benefit to remove the
**        branch is greater than the cost of load-time optimization, then the
**        shader code is marked as load-time otpimizable, at the load-time,
**        a light weight optimizer is invoked to optimize the code. The light
**        weight optimizer works on the machine code, currently only constant
**        propagation and dead code elimination are pplied to the code
**
**   The shader code may load texture using load-time constant, we don't know
**   the impact if the texture load is optimized as load time constant, since
**   the texure load is very complicated, cannot be easily modelled by CPU.
**   If we really want to optimize it, there two options:
**     1. model it with CPU code, basically using the c-model texture module
**        to get the texture value at load time
**     2. compose a small kernel code to have the GPU load the the texture and
**        get the return value from the kernel and use it for load-time constant
**   Both methods will have big overhead, and the benefit is unclear, because
**   the constant texture load may not have big latency as one expected, due to
**   the fact that the frequently accessed data may stay in the cache.
**
**
*/

/********************************
*  Compiler-time LTC collection
*********************************/

static gceSTATUS
_ltcAllocate(
    IN gctUINT32 Bytes,
    OUT gctPOINTER * Memory
    )
{
    return gcoOS_Allocate(gcvNULL, Bytes, Memory);
};

static gceSTATUS
_ltcFree(
    IN gctPOINTER Memory
    )
{
    return gcoOS_Free(gcvNULL, Memory);
}

gcsAllocator ltcAllocator = {_ltcAllocate, _ltcFree };

gctBOOL
CompareCode (
     IN void *    Data,
     IN void *    Key
     )
{
    gcOPT_CODE code1 = (gcOPT_CODE)Data;
    gcOPT_CODE code2 = (gcOPT_CODE)Key;

    return  code1 ==  code2 ;
} /* CompareIndex */

/* treat the Data and Key as temp register index with components
 * encoded in the value, check if the Index part are equal */
gctBOOL
CompareIndex (
     IN void *    Data,
     IN void *    Key
     )
{
    gctINT regIndex1 = ltcGetRegister((gctUINT)(gctUINTPTR_T)Data);
    gctINT regIndex2 = ltcGetRegister((gctUINT)(gctUINTPTR_T)Key);

    return  regIndex1 ==  regIndex2 ;
} /* CompareIndex */

/* treat the Data and Key as temp register index with components
 * encoded in the value, check if the Index part are equal, and
 * all components in the Key are included in Data's components
 */
gctBOOL
CompareIndexAndComponents (
     IN void *    Data,
     IN void *    Key
     )
{
    gctINT regIndex1      = ltcGetRegister((gctUINT)(gctUINTPTR_T)Data);
    gctINT regIndex2      = ltcGetRegister((gctUINT)(gctUINTPTR_T)Key);

    if (regIndex1 == regIndex2)
    {
        gctINT regComponents1 = ltcGetComponents((gctUINT)(gctUINTPTR_T)Data);
        gctINT regComponents2 = ltcGetComponents((gctUINT)(gctUINTPTR_T)Key);
        /* check if Data contains all components in Key */
        return ((regComponents1 ^ regComponents2) & regComponents2) == 0;
    }
    return gcvFALSE;
} /* CompareIndex */

static
gctBOOL
_ComparePtr (
     IN void *    Data,
     IN void *    Key
     )
{
    return  Data == Key ;
} /* ComparePtr */

static
gceSTATUS
_AddToCodeList(
    IN gctCodeList        List,
    IN gcOPT_CODE         Code
    )
{
    gcsListNode * node = gcList_FindNode(List, Code, CompareCode);

    if (node != gcvNULL)
    {
        return gcvSTATUS_OK;
    }

    return gcList_AddNode(List, Code);
} /* _AddToCodeList */

static
gceSTATUS
_RemoveCodeFromList(
    IN gctCodeList        List,
    IN gcOPT_CODE         Code
    )
{
    gcsListNode * node = gcList_FindNode(List, Code, CompareCode);

    if (node == gcvNULL)
    {
        return gcvSTATUS_OK;
    }

    return gcList_RemoveNode(List, node);
} /* _RemoveCodeFromList */

static
gceSTATUS
_AddToTempRegList(
     IN gctTempRegisterList    List,
     IN gctINT                 Index)
{
    gctCHAR  buffer[512];
    gctUINT  offset = 0;
    /* check if there is a node in the list which has the same temp
     * register Index
     */
    gcsListNode * node = gcList_FindNode(List, (gctPOINTER)(gctUINTPTR_T)Index, CompareIndex);
    if (node != gcvNULL)
    {
        /* the temp register is already in the list, update it with new
         * components be merged with old ones
         */
        gctINT oldValue = (gctINT)(gctUINTPTR_T)node->data;
        gctINT newValue;

        gcmASSERT(ltcGetRegister(Index) == ltcGetRegister(oldValue));

        newValue = ltcRegisterWithComponents(ltcGetRegister(Index),
                      ltcGetComponents(oldValue) | ltcGetComponents(Index));
        node->data = (gctPOINTER)(gctUINTPTR_T)newValue;
        if (gcDumpOption(gceLTC_DUMP_COLLECTING))
        {
            gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                "_AddToTempRegList(Index=%#x [%d.%#x]): update %#x to %#x", Index, ltcGetRegister(Index), ltcGetComponents(Index), oldValue, newValue));
            gcoOS_Print("%s", buffer);
        }
        return gcvSTATUS_OK;
    }
    else
    {
        if (gcDumpOption(gceLTC_DUMP_COLLECTING))
        {
            gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                "_AddToTempRegList(Index=%#x  [%d.%#x]): added new index", Index, ltcGetRegister(Index), ltcGetComponents(Index)));
            gcoOS_Print("%s", buffer);
        }
        /* not in the list, add a new one */
        return gcList_AddNode(List, (gctPOINTER)(gctUINTPTR_T)Index);
    } /* if */
} /* _AddToTempRegList */

/* check if the temp register 'index' is load time constant at Code */
gctBOOL
_isTempRegisterALoadtimeConstant(
       IN gcOPTIMIZER          Optimizer,
       IN gcOPT_CODE           Code,
       IN gctUINT              SourceNo,
       IN gctUINT              Index,
       IN gctTempRegisterList  LTCTempRegList)
{
    gcSL_INSTRUCTION    inst = &Code->instruction;
    gctUINT16           target = inst->tempIndex;
    gcOPT_LIST          dependencies;
    gcSL_ENABLE         enable = gcGetUsedComponents(inst, SourceNo);
    gcOPT_LIST          dep, depIter;
    gctINT              depCount = 0;
    gcOPT_CODE          depCode, codeIter;
    gctBOOL             isAllDepLTC = gcvTRUE, isAllDepSameBB = gcvTRUE;
    gctINT              indexWithComponents =
                           (gctINT)ltcRegisterWithComponents(Index, enable);

    /* if the temp register is not in the known loadtime constant temp
       register list, then it is not a loadtime constant */
    if (gcList_FindNode(LTCTempRegList,
                      (gctPOINTER)(gctUINTPTR_T)indexWithComponents,
                      CompareIndexAndComponents) == gcvNULL)
    {
        return gcvFALSE;
    }

    /* if the Index is merged before or at the Code with different values,
       then it is not a loadtime constant */
    dependencies = (SourceNo == 0) ? Code->dependencies0
                                   : Code->dependencies1;
    /* TODO: check if the target temp register depends on itself indirectly:
     *
     *     28: ADD             temp(20).x, temp(0).x, 2.000000
     *     29: MOV             temp(0).x, temp(20).x
     *     30: ADD             temp(21).x, temp(19).x, 1.000000
     *     31: MOV             temp(19).x, temp(21).x
     *     32: JMP.L           28, temp(19).x, uniform(2).x
     */
    if (target == Index && gcmSL_OPCODE_GET(inst->opcode, Opcode) != gcSL_JMP)
        return gcvFALSE;  /* the code is depend on itself */

    /*
    ** If all dependencies are within LTC:
    ** 1) If all dependencies are in the same BB, then return TRUE;
    ** 2) Otherwise, check CTS/dEQP.
    */
    for (dep = dependencies; dep != gcvNULL; dep = dep->next)
    {
        if (dep->index < 0)
        {
            continue;
        }

        depCode = dep->code;

        if (gcList_FindNode(&Optimizer->theLTCCodeList, depCode, _ComparePtr) == gcvNULL)
        {
            isAllDepLTC = gcvFALSE;
            break;
        }

        for (depIter = dep->next; depIter != gcvNULL; depIter = depIter->next)
        {
            if (depIter->index < 0)
            {
                continue;
            }

            codeIter = depIter->code;

            if (!gcOpt_isCodeInSameBB(depCode, codeIter))
            {
                isAllDepSameBB = gcvFALSE;
                break;
            }
        }
    }

    if (!isAllDepLTC)
    {
        return gcvFALSE;
    }

    if (isAllDepSameBB)
    {
        return gcvTRUE;
    }

    /* When turn on JMP and LOAD support for LTC,
    ** if a code has multiple dependency, and all these dependency are within the branches.
    ** we can add it to LTC list.
    */
    if (Optimizer->isCTSInline)
    {
        for (dep = dependencies; dep != gcvNULL; dep = dep->next)
        {
            if (dep->index < 0)
            {
                continue;
            }

            depCode = dep->code;

            for (codeIter = depCode->prev; codeIter; codeIter = codeIter->next)
            {
                if (codeIter->instruction.opcode == gcSL_JMP)
                {
                    if (gcList_FindNode(&Optimizer->theLTCCodeList, codeIter, _ComparePtr) == gcvNULL)
                    {
                        return gcvFALSE;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            if (gcList_FindNode(&Optimizer->theLTCCodeList, depCode, _ComparePtr) == gcvNULL)
            {
                return gcvFALSE;
            }
            depCount++;
        }

        if (depCount != 2)
        {
            return gcvFALSE;
        }

        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
} /* _isTempRegisterALoadtimeConstant */

static gctBOOL
_isSourceHasIndexedAssigment(
       IN gcOPTIMIZER          Optimizer,
       IN gcOPT_CODE           Code,
       IN gctINT               SourceNo)
{
    gcSL_INSTRUCTION        inst = &Code->instruction;
    gctSOURCE_t             source;
    gctUINT16               index;
    gctBOOL                 sourceIsLTC = gcvTRUE;

    gcmASSERT(SourceNo == 0 || SourceNo == 1);
    source = SourceNo == 0 ? inst->source0 : inst->source1;
    index = (SourceNo == 0) ? inst->source0Index : inst->source1Index;

    if (Optimizer->indexedVariableListForLTC.count != 0 &&
        gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
    {
        gcOPT_TEMP temp = &Optimizer->tempArray[index];

        if (temp->arrayVariable != gcvNULL)
        {
            gcsListNode *node = gcList_FindNode(&Optimizer->indexedVariableListForLTC,
                                                (gctPOINTER)(gctUINTPTR_T)temp->arrayVariable->tempIndex,
                                                CompareIndex);
            if (node != gcvNULL)
            {
                sourceIsLTC = gcvFALSE;
            }
        }
    }

    return sourceIsLTC;
}

/* for source in Code's SourceNo (0 or 1), check if it is a loadtime constant */
static gctBOOL
_isLoadtimeConastant(
       IN gcOPTIMIZER          Optimizer,
       IN gcOPT_CODE           Code,
       IN gctINT               SourceNo,
       IN gctTempRegisterList  LTCTempRegList)
{
    /* a source is a loadtime constant if it is one of following:
         1. Compile-time constant value (gcSL_CONSTANT)
         2. Loadtime constant value (gcSL_UNIFORM), if indexed,
            the index must LTC itself
         3. temp register (gcSL_TEMP) which operands are LTC, if indexed,
            the index must LTC itself, and temp register is not depend on
            itself directly or indirectly
    */
    gcSL_INSTRUCTION        inst = &Code->instruction;
    gctSOURCE_t             source;
    gctUINT16               index;
    gctBOOL                 sourceIsLTC = gcvFALSE;
    gcSL_FORMAT             format;

    gcmASSERT(SourceNo == 0 || SourceNo == 1);
    source = SourceNo == 0 ? inst->source0 : inst->source1;
    format = (gcSL_FORMAT) gcmSL_SOURCE_GET(source, Format);
    index = (SourceNo == 0) ? inst->source0Index : inst->source1Index;

    if (!(format == gcSL_FLOAT || format == gcSL_INTEGER || format == gcSL_UINT32 || format == gcSL_BOOLEAN))
    {
        return gcvFALSE;
    }

    if (gcmSL_SOURCE_GET(source, Type) == gcSL_CONSTANT)
    {
        sourceIsLTC = gcvTRUE;
    }
    else if (gcmSL_SOURCE_GET(source, Type) == gcSL_UNIFORM)
    {
        gctINT uniformIndex = gcmSL_INDEX_GET(index, Index);
        gcUNIFORM           uniform = Optimizer->shader->uniforms[uniformIndex];

        /* exclude Uniform Block for Halti2, as it uses Load to get the data */
        if (isUniformNormal(uniform)
        ||  isUniformLodMinMax(uniform)
        ||  isUniformLevelBaseSize(uniform)
        ||  isUniformBlockMember(uniform)
        ||  isUniformSampleLocation(uniform)
        ||  isUniformMultiSampleBuffers(uniform)
        ||  isUniformUBOAddress(uniform)
           )
        {
            /* check if it is indexed and index is LTC */
            if (gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
            {
                index = SourceNo == 0 ? inst->source0Indexed : inst->source1Indexed;
                if (_isTempRegisterALoadtimeConstant(Optimizer, Code, SourceNo, index, LTCTempRegList))
                    sourceIsLTC = gcvTRUE;
            }
            else sourceIsLTC = gcvTRUE;
        }
    }
    else if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
    {
        /* the temp register should be LTC */
        if (gcmSL_INDEX_GET(index, ConstValue) == 0
             && _isTempRegisterALoadtimeConstant(Optimizer, Code, SourceNo, index, LTCTempRegList) )
        {
            if (gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
            {
                /* cannot handle indexed temp variable yet */
                sourceIsLTC = gcvFALSE;
            }
            else sourceIsLTC = gcvTRUE;
        }
    }

    if (sourceIsLTC)
    {
        sourceIsLTC = _isSourceHasIndexedAssigment(Optimizer, Code, SourceNo);
    }

    return sourceIsLTC;
} /* _isLoadtimeConastant */

static gceSTATUS
_RemoveTempComponentsFromLTCTempRegList(
       IN gctTempRegisterList  LTCTempRegList,
       IN gctINT               TempIndex,
       IN gcSL_ENABLE          Components)
{
    gceSTATUS           status = gcvSTATUS_OK;
    gctINT              indexWithComponents =
                           (gctINT)ltcRegisterWithComponents(TempIndex, Components);
    gcsListNode *       node;

    gcmHEADER();

    /* check if the components is empty */
    if (Components == gcSL_ENABLE_NONE)
    {
        node = gcvNULL;
    }
    else
    {
        node = gcList_FindNode(LTCTempRegList, (gctPOINTER)(gctUINTPTR_T)indexWithComponents, CompareIndex);
    }

    if (node != gcvNULL)
    {
        /* the temp register is already in the list, update it with new
         * components be removing the components in the node
         */
        gctCHAR  buffer[512];
        gctUINT  offset = 0;
        gctINT       oldValue = (gctINT)(gctUINTPTR_T)node->data;
        gctINT       newValue;
        gcSL_ENABLE  updatedComponets;

        gcmASSERT(ltcGetRegister(indexWithComponents) == ltcGetRegister(oldValue));

        updatedComponets = ltcGetComponents(oldValue) & ~Components;
        if (updatedComponets == gcSL_ENABLE_NONE)
        {
            /* now after remove the killed components, there is no
             * loadtime constant component left in the temp register,
             * remove the temp register from the list
             */
            gcList_RemoveNode(LTCTempRegList, node);
            if (gcDumpOption(gceLTC_DUMP_COLLECTING))
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                    "%s(TempIndex=%d)", __FUNCTION__, TempIndex));
                gcoOS_Print("%s", buffer);
            }
        }
        else
        {
            /* update the node with new value */
            newValue = ltcRegisterWithComponents(ltcGetRegister(indexWithComponents),
                                                 updatedComponets);
            node->data = (gctPOINTER)(gctUINTPTR_T)newValue;
            if (gcDumpOption(gceLTC_DUMP_COLLECTING))
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                    "%s(TempIndex=%d): update %#x to %#x", __FUNCTION__, TempIndex,
                    oldValue, newValue));
                gcoOS_Print("%s", buffer);
            }
        }
    }

    gcmFOOTER();
    return status;
} /* _RemoveTempComponentsFromLTCTempRegList */

/* kill all output variables and defined global variable from
   Optimizer's LTCTempRegList
 */
static void
_RemoveOutputFromLTCTempRegList(
    IN gcOPTIMIZER          Optimizer,
    IN gcOPT_CODE           Code
    )
{
    gcOPT_FUNCTION       function;
    gcsFUNCTION_ARGUMENT_PTR argument;
    gcOPT_GLOBAL_USAGE usage;
    gctUINT a;

    gcmASSERT(Code->instruction.opcode == gcSL_CALL);
    /* find the callee function from Code */
    function = Code->callee->function;
    /* go through all its arugments to find the output variable
       and remove it from LTCTempRegList */
    argument = function->arguments;
    for (a = 0; a < function->argumentCount; a++, argument++)
    {
        /* Check output arguments. */
        if (argument->qualifier != gcvFUNCTION_INPUT)
        {
            _RemoveTempComponentsFromLTCTempRegList(&Optimizer->theLTCTempRegList,
                                                    function->arguments[a].index,
                                                    gcSL_ENABLE_XYZW);
        }
    }

    /* find all defined global variable and remove it from LTCTempRegList */
    for (usage = function->globalUsage; usage; usage = usage->next)
    {
        if (usage->direction != gcvFUNCTION_INPUT)
        {
            _RemoveTempComponentsFromLTCTempRegList(&Optimizer->theLTCTempRegList,
                                                    usage->index,
                                                    gcSL_ENABLE_XYZW);
        }
    }
}

static gceSTATUS
_RemoveTargetFromLTCTempRegList(
       IN gcOPTIMIZER          Optimizer,
       IN gcOPT_CODE           Code,
       IN gctBOOL              ScanAllUsers)
{
    gcSL_INSTRUCTION    inst = &Code->instruction;
    gctUINT16           target = inst->tempIndex;
    gcSL_ENABLE         enable = (gcSL_ENABLE) gcmSL_TARGET_GET(inst->temp,
                                                                Enable);
    gcOPT_FUNCTION      function = gcvNULL;
    gcSL_OPCODE         opcode = gcmSL_OPCODE_GET(Code->instruction.opcode, Opcode);
    if (ScanAllUsers)
    {
        gcOPT_LIST          list;

        _AddToCodeList(&Optimizer->theLTCRemoveCodeList, Code);
        _RemoveCodeFromList(&Optimizer->theLTCCodeList, Code);

        for (list = Code->users; list != gcvNULL; list = list->next)
        {
            if (list->code == gcvNULL) continue;

            if (list->code->function != Code->function) continue;

            if (list->code->instruction.opcode == gcSL_CALL) continue;

            if (gcList_FindNode(&Optimizer->theLTCRemoveCodeList, list->code, CompareCode)) continue;

            _AddToCodeList(&Optimizer->theLTCRemoveCodeList, list->code);
            _RemoveTargetFromLTCTempRegList(Optimizer, list->code, gcvTRUE);
            _RemoveCodeFromList(&Optimizer->theLTCCodeList, list->code);
        }
    }

    if (opcode != gcSL_CALL &&
        opcode != gcSL_JMP &&
        Optimizer->tempArray[inst->tempIndex].argument != gcvNULL)
    {
        function = Optimizer->tempArray[inst->tempIndex].function;

        /* Remove all codes in LTC list that depends on this code. */
        if (Code->function != function && function->shaderFunction != gcvNULL)
        {
            gcOPT_CODE code;

            for (code = function->codeHead; code != function->codeTail->next; code = code->next)
            {
                gcSL_INSTRUCTION userInst = &code->instruction;

                if (gcmSL_SOURCE_GET(userInst->source0, Type) == gcSL_TEMP &&
                    gcmSL_INDEX_GET(userInst->source0Index, Index) == inst->tempIndex)
                {
                    _RemoveTargetFromLTCTempRegList(Optimizer, code, gcvTRUE);
                }

                if (gcmSL_SOURCE_GET(userInst->source0, Indexed) != gcSL_NOT_INDEXED &&
                    userInst->source0Indexed == inst->tempIndex)
                {
                    _RemoveTargetFromLTCTempRegList(Optimizer, code, gcvTRUE);
                }

                if (gcmSL_SOURCE_GET(userInst->source1, Type) == gcSL_TEMP &&
                    gcmSL_INDEX_GET(userInst->source1Index, Index) == inst->tempIndex)
                {
                    _RemoveTargetFromLTCTempRegList(Optimizer, code, gcvTRUE);
                }

                if (gcmSL_SOURCE_GET(userInst->source1, Indexed) != gcSL_NOT_INDEXED &&
                    userInst->source1Indexed == inst->tempIndex)
                {
                    _RemoveTargetFromLTCTempRegList(Optimizer, code, gcvTRUE);
                }
            }
        }
    }

    return _RemoveTempComponentsFromLTCTempRegList(&Optimizer->theLTCTempRegList, target, enable);
} /* _RemoveTargetFromLTCTempRegList */

static gceSTATUS
_addInstructionToLTCList(
    IN gcOPTIMIZER          Optimizer,
    IN gcOPT_CODE           Code,
    IN gctBOOL              TrueAdd
    )
{
    gceSTATUS status = gcvSTATUS_TRUE;
    gcOPT_CODE           code = Code;
    gcSL_INSTRUCTION     inst;
    gctUINT16            target;
    gctUINT16            enabled;
    gcSL_OPCODE          opcode;
    do
    {
        if (gcDumpOption(gceLTC_DUMP_COLLECTING))
            dbg_dumpCode(code);

        /* Get instruction. */
        inst = &code->instruction;
        enabled = gcmSL_TARGET_GET(inst->temp, Enable);
        /* Get target. */
        target = inst->tempIndex;
        opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(inst->opcode, Opcode);

        if (gcmSL_TARGET_GET(inst->temp, Indexed) != gcSL_NOT_INDEXED)
        {
            /* the target is indexed temp register, kill the temp register
             * in theLTCTempRegList
             */
            if (TrueAdd)
            {
                _RemoveTargetFromLTCTempRegList(Optimizer, code, gcvFALSE);
            }

            /* If this temp register is an array variable, then push it into indexed variable list. */
            if (Optimizer->tempArray[target].arrayVariable != gcvNULL)
            {
                _AddToTempRegList(&Optimizer->indexedVariableListForLTC, target);
            }
            status = gcvSTATUS_FALSE;
            break;
        }

        if ((gcmSL_SOURCE_GET(inst->source0, Type) != gcSL_NONE
             && !_isLoadtimeConastant(Optimizer, code, 0, &Optimizer->theLTCTempRegList)) ||
            (gcmSL_SOURCE_GET(inst->source1, Type) != gcSL_NONE
             && !_isLoadtimeConastant(Optimizer, code, 1, &Optimizer->theLTCTempRegList)) ||
             (opcode != inst->opcode && opcode != gcSL_CONV))
        {
            /* the source0/source1 is not loadtime constant, or inst->opcode has rounding mode
             * or saturate bits set (LTC not supporting them yet), kill the temp register in
             * theLTCTempRegList
             */
            if (TrueAdd)
            {
                _RemoveTargetFromLTCTempRegList(Optimizer, code, gcvFALSE);
            }
            status = gcvSTATUS_FALSE;
            break;
        }

        /* all sources are loadtime constant, now find if it is an instructions
           we cannot fold it at loadtime to constant: texture load, kill, barrier,
           load, store, etc.
         */
        switch (opcode)
        {
        case gcSL_MOV:
            /* mov instruction */
        case gcSL_RCP:
        case gcSL_ABS:
        case gcSL_FLOOR:
        case gcSL_CEIL:
        case gcSL_FRAC:
        case gcSL_LOG:
        case gcSL_RSQ:
        case gcSL_SAT:
        case gcSL_NORM:

        case gcSL_SIN:
        case gcSL_COS:
        case gcSL_TAN:
        case gcSL_EXP:
        case gcSL_SIGN:
        case gcSL_SQRT:
        case gcSL_ACOS:
        case gcSL_ASIN:
        case gcSL_ATAN:
        case gcSL_I2F:
        case gcSL_F2I:
            /* One-operand computational instruction. */
        case gcSL_ADD:
        case gcSL_SUB:
        case gcSL_MUL:
        case gcSL_MAX:
        case gcSL_MIN:
        case gcSL_POW:
        case gcSL_STEP:

        case gcSL_LSHIFT:
        case gcSL_RSHIFT:

        case gcSL_DP2:
        case gcSL_DP3:
        case gcSL_DP4:
        case gcSL_CROSS:
        case gcSL_SET:

        case gcSL_LOAD:
            /* Two-operand computational instruction. */

            /* all these operators are supported in
             * current constant folding routine
             */
            if (TrueAdd)
            {
                _AddToCodeList(&Optimizer->theLTCCodeList, code);

                /* add to LTC temp register list if is not already in it,
                 * update it otherwise
                 */
                _AddToTempRegList(&Optimizer->theLTCTempRegList,
                                  ltcRegisterWithComponents(target, enabled));
            }
            break;

        case gcSL_JMP:
            if (TrueAdd)
            {
                _AddToCodeList(&Optimizer->theLTCCodeList, code);
            }
            break;

        case gcSL_CALL:
            if (TrueAdd)
            {
                _RemoveOutputFromLTCTempRegList(Optimizer, code);
            }
            status = gcvSTATUS_FALSE;
            break;

        /* Only add "CONV.RTZ, float, float, float"*/
        case gcSL_CONV:
            {
                gcSL_ROUND          rounding;
                gcSL_FORMAT         targetFormat, src0Format;
                gcSL_TYPE           src1Type;
                gctBOOL             added = gcvTRUE;
                union
                {
                    gctUINT16 hex[2];
                    gcSL_FORMAT hex32;
                } src1Format;

                rounding = (gcSL_ROUND)gcmSL_OPCODE_GET(inst->opcode, Round);
                targetFormat = (gcSL_FORMAT)gcmSL_TARGET_GET(inst->temp, Format);
                src0Format = (gcSL_FORMAT)gcmSL_SOURCE_GET(inst->source0, Format);
                src1Type = (gcSL_TYPE)gcmSL_SOURCE_GET(inst->source1, Type);

                if (rounding != gcSL_ROUND_RTZ ||
                    targetFormat != gcSL_FLOAT || src0Format != gcSL_FLOAT ||
                    src1Type != gcSL_CONSTANT)
                {
                    added = gcvFALSE;
                }
                else
                {
                    /* Get the convet format. */
                    src1Format.hex[0] = inst->source1Index;
                    src1Format.hex[1] = inst->source1Indexed;

                    if (src1Format.hex32 != gcSL_FLOAT)
                    {
                        added = gcvFALSE;
                    }
                }

                if (added)
                {
                    if (TrueAdd)
                    {
                        code->instruction.opcode = gcmSL_OPCODE_SET(code->instruction.opcode, Round, gcSL_ROUND_DEFAULT);
                        _AddToCodeList(&Optimizer->theLTCCodeList, code);

                        /* add to LTC temp register list if is not already in it,
                         * update it otherwise
                         */
                        _AddToTempRegList(&Optimizer->theLTCTempRegList,
                                          ltcRegisterWithComponents(target, enabled));
                    }
                    break;
                }
                else
                {
                    if (TrueAdd)
                    {
                        _RemoveTargetFromLTCTempRegList(Optimizer, code, gcvFALSE);
                    }
                    status = gcvSTATUS_FALSE;
                    break;
                }
            }

        case gcSL_DSX:
        case gcSL_DSY:
        case gcSL_FWIDTH:
        case gcSL_DIV:
        case gcSL_MOD:
        case gcSL_AND_BITWISE:
        case gcSL_OR_BITWISE:
        case gcSL_XOR_BITWISE:
        case gcSL_NOT_BITWISE:
        case gcSL_ROTATE:
        case gcSL_BITSEL:
        case gcSL_LEADZERO:
        case gcSL_ADDLO:
        case gcSL_MULLO:
        case gcSL_MULHI:
        case gcSL_CMP:
        case gcSL_ADDSAT:
        case gcSL_SUBSAT:
        case gcSL_MULSAT:
        case gcSL_MADSAT:
            /* these operators are not supported by constant folding
               need to add handling for them */
        case gcSL_TEXBIAS:
        case gcSL_TEXGRAD:
        case gcSL_TEXGATHER:
        case gcSL_TEXFETCH_MS:
        case gcSL_TEXLOD:
        case gcSL_TEXU:
        case gcSL_TEXU_LOD:
                /* Skip texture state instructions. */
        case gcSL_TEXLD:
        case gcSL_TEXLDPROJ:
        case gcSL_TEXLDPCF:
        case gcSL_TEXLDPCFPROJ:
                /* Skip texture sample instructions. */
        case gcSL_STORE:
        case gcSL_STORE1:
        case gcSL_STORE_L:
        case gcSL_IMAGE_SAMPLER:
        case gcSL_IMAGE_RD:
        case gcSL_IMAGE_WR:
        case gcSL_IMAGE_RD_3D:
        case gcSL_IMAGE_WR_3D:
        case gcSL_IMAGE_ADDR:
        case gcSL_IMAGE_ADDR_3D:
        case gcSL_BARRIER:
        case gcSL_MEM_BARRIER:
        case gcSL_GETEXP:
        case gcSL_GETMANT:
            /* although the sources are loadtime constant, but since we
             * don't process the result, the target is no longer a loadtime
             * constant, so we need to remove it from Loadtime temp register
             * list
             */
            if (TrueAdd)
            {
                _RemoveTargetFromLTCTempRegList(Optimizer, code, gcvFALSE);
            }
            status = gcvSTATUS_FALSE;
            break;

        case gcSL_RET:
        case gcSL_NOP:
        case gcSL_KILL:
            /* skip control flow. */
            status = gcvSTATUS_FALSE;
            break;
        default:
            break;
        }
    } while(gcvFALSE);

    return status;
}

/* If all instructions in the branches can be folded by LTC,
** add all of them into LTC list and update the code cursor.
** This is only enabled for 2 patterns:
    1:
         1: JMP.CON          5, temp(1).hp.x, 0
         2: MOV                temp(2).x, 1.000000
         3: MOV                temp(3).x, 2.000000
         4: JMP                 7
         5: MOV                temp(2).x, 0.000000
         6: MOV                temp(3).x, 5.000000
    2:
         1: JMP.EQ            3, temp.int(1).hp.xyz, temp.int(148).hp.xyz
         2: JMP                 5
         3: MOV                temp(2).x, 1.000000
         4: JMP                 6
         5: MOV                temp(2).x, 0.000000
*/
static gctBOOL
_checkJmpControlFlowAllCanBeLTC(
    IN gcOPTIMIZER          Optimizer,
    IN gcOPT_CODE           Code,
    IN gcOPT_CODE           *EndCode
    )
{
    gcOPT_CODE callee = Code->callee, elseCallee;
    gcOPT_CODE code, ifCode, elseCode;
    gcOPT_CODE codeIter1, codeIter2;
    gctINT offset;
    gctUINT instCount = 0;
    gcSL_CONDITION condition = gcmSL_TARGET_GET(Code->instruction.temp, Condition);
    gctBOOL vectorEqual = gcvFALSE;

    if (Code->callers != gcvNULL)
    {
        if (!(Code->callers->next == gcvNULL &&
            gcList_FindNode(&Optimizer->theLTCCodeList, Code->callers->code, _ComparePtr)))
        {
            return gcvFALSE;
        }
    }

    /* Only implement below conditions. */
    if (condition != gcSL_NOT_EQUAL && condition != gcSL_LESS_OR_EQUAL &&
        condition != gcSL_LESS && condition != gcSL_EQUAL && condition != gcSL_GREATER &&
        condition != gcSL_GREATER_OR_EQUAL)
    {
        return gcvFALSE;
    }

    /* Skip multiple callers. */
    if (callee->callers->next != gcvNULL)
        return gcvFALSE;

    ifCode = Code;

    /* Skip back jump. */
    if (ifCode->backwardJump)
        return gcvFALSE;

    if (_addInstructionToLTCList(Optimizer, ifCode, gcvFALSE) == gcvSTATUS_FALSE)
        return gcvFALSE;

    /* Find the else condition code. */
    elseCode = gcvNULL;
    for (code = callee->prev; code && code != ifCode; code = code->prev)
    {
        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_JMP)
            continue;

        if (elseCode == gcvNULL)
        {
            elseCode = code;
        }
        else
        {
            return gcvFALSE;
        }
    }

    /* Not match. */
    if (elseCode == gcvNULL || gcmSL_TARGET_GET(elseCode->instruction.temp, Condition) != gcSL_ALWAYS ||
        (elseCode == ifCode->next && condition != gcSL_EQUAL))
    {
        return gcvFALSE;
    }

    if (condition == gcSL_EQUAL && elseCode == ifCode->next)
        vectorEqual = gcvTRUE;

    /* There is not the other jmp between else and end. */
    if (!vectorEqual)
    {
        elseCallee = elseCode->callee;

        if (gcmSL_OPCODE_GET(elseCallee->instruction.opcode, Opcode) == gcSL_JMP)
            return gcvFALSE;

        for (code = elseCallee->prev; code && code != callee; code = code->prev)
        {
            if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_JMP)
            {
                return gcvFALSE;
            }
        }
    }
    else
    {
        ifCode = gcvNULL;
        elseCallee = elseCode->callee;

        if (gcmSL_OPCODE_GET(elseCallee->instruction.opcode, Opcode) == gcSL_JMP)
            return gcvFALSE;

        for (code = elseCallee->prev; code && code != elseCode; code = code->prev)
        {
            if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_JMP)
                continue;

            if (ifCode == gcvNULL)
            {
                ifCode = code;
            }
            else
            {
                return gcvFALSE;
            }
        }

        if (ifCode == gcvNULL || gcmSL_TARGET_GET(ifCode->instruction.temp, Condition) != gcSL_ALWAYS)
            return gcvFALSE;

        if (gcmSL_OPCODE_GET(ifCode->callee->instruction.opcode, Opcode) == gcSL_JMP)
        {
            if (ifCode->callee->callers->next != gcvNULL)
                return gcvFALSE;
        }

        /* Switch if and else. */
        code = ifCode;
        ifCode = elseCode;
        elseCode = code;
    }

    if (ifCode->callee->callers->next != gcvNULL ||
        elseCode->callee->callers->next != gcvNULL)
    {
        return gcvFALSE;
    }

    /* Now, we get a match pattern, we need to make sure all branches are constants. */
    codeIter1 = ifCode->next;
    codeIter2 = elseCode->next;

    while(codeIter1 && codeIter2 && codeIter1 != elseCode)
    {
        if (codeIter1->callers && codeIter1->callers->code != Code)
            return gcvFALSE;

        if (codeIter1->instruction.temp != codeIter2->instruction.temp ||
            codeIter1->instruction.tempIndex != codeIter2->instruction.tempIndex ||
            codeIter1->instruction.tempIndexed != codeIter2->instruction.tempIndexed)
        {
            return gcvFALSE;
        }

        if (_addInstructionToLTCList(Optimizer, codeIter1, gcvFALSE) == gcvSTATUS_FALSE ||
            _addInstructionToLTCList(Optimizer, codeIter2, gcvFALSE) == gcvSTATUS_FALSE)
        {
            return gcvFALSE;
        }

        codeIter1 = codeIter1->next;
        codeIter2 = codeIter2->next;
        instCount++;
    }

    if (elseCode->id - ifCode->id != instCount + 1)
        return gcvFALSE;

    if (codeIter2 != elseCode->callee)
        return gcvFALSE;

    /* We can add all branches to LTC now. */
    offset = (gctINT)Code->id - Optimizer->theLTCCodeList.count;
    for (code = Code; code != elseCode->callee; code = code->next)
    {
        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_JMP)
        {
            /* Update the jump target on LTC. */
            gctINT newTarget = (gctINT)code->instruction.tempIndex;
            newTarget -= offset;
            code->instruction.tempIndex = (gctUINT16)newTarget;
        }
        _addInstructionToLTCList(Optimizer, code, gcvTRUE);
    }

    *EndCode = elseCode->callee->prev;
    return gcvTRUE;
}

static gceSTATUS
_findLoadtimeConstantInFunction(
    IN gcOPTIMIZER          Optimizer,
    IN gcOPT_FUNCTION       Function
    )
{
    gceSTATUS            status = gcvSTATUS_OK;
    gcOPT_CODE           code;
    gctBOOL              maxUniform = gcvFALSE;

    /* reset global and argument temp register. */
    {
        gcsFUNCTION_ARGUMENT_PTR argument;
        gcOPT_GLOBAL_USAGE usage;
        gctUINT a;

        /* remove constantness for all input arguments. */
        argument = Function->arguments;
        for (a = 0; a < Function->argumentCount; a++, argument++)
        {
            /* Check input arguments. */
            if (argument->qualifier != gcvFUNCTION_OUTPUT)
            {
                _RemoveTempComponentsFromLTCTempRegList(&Optimizer->theLTCTempRegList,
                                                        Function->arguments[a].index,
                                                        gcSL_ENABLE_XYZW);
            }
        }

        /* remove constantness for all global variables. */
        for (usage = Function->globalUsage; usage; usage = usage->next)
        {
            _RemoveTempComponentsFromLTCTempRegList(&Optimizer->theLTCTempRegList,
                                                    usage->index,
                                                    gcSL_ENABLE_XYZW);
        }
    }

    /* Walk through all the instructions, finding instructions uses LTC as sources. */
    for (code = Function->codeHead;
         code != Function->codeTail->next && !maxUniform;
         code = code->next)
    {
        gcSL_OPCODE opcode = gcmSL_OPCODE_GET(code->instruction.opcode, Opcode);

        if (gcList_FindNode(&Optimizer->theLTCRemoveCodeList, code, CompareCode) != gcvNULL)
            continue;

        if (opcode == gcSL_JMP || opcode == gcSL_LOAD)
        {
            if (Optimizer->isCTSInline)
            {
                if (opcode == gcSL_LOAD)
                {
                    gcOPT_CODE nextCode = code->next;
                    gcSL_OPCODE opcode = gcmSL_OPCODE_GET(nextCode->instruction.opcode, Opcode);

                    if (gcSL_isOpcodeAtom(opcode))
                    {
                        continue;
                    }
                    else
                    {
                        _addInstructionToLTCList(Optimizer, code, gcvTRUE);
                    }
                }
                else
                {
                    gcOPT_CODE endCode = code;

                    _checkJmpControlFlowAllCanBeLTC(Optimizer, code, &endCode);
                    code = endCode;
                }
            }
            else
            {
                continue;
            }
            continue;
        }
        /* Right now, we only add set.z/set.nz that would generate select instruction to LTC. */
        else if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_SET)
        {
            if (gcmSL_TARGET_GET(code->instruction.temp, Condition) == gcSL_ZERO &&
                _addInstructionToLTCList(Optimizer, code, gcvFALSE) == gcvSTATUS_TRUE &&
                code != Function->codeTail)
            {
                gcOPT_CODE nextCode = code->next;

                if (gcmSL_OPCODE_GET(nextCode->instruction.opcode, Opcode) == gcSL_SET &&
                    gcmSL_TARGET_GET(nextCode->instruction.temp, Condition) == gcSL_NOT_ZERO &&
                    (_addInstructionToLTCList(Optimizer, nextCode, gcvFALSE) == gcvSTATUS_TRUE) &&
                    (code->instruction.temp == gcmSL_TARGET_SET(nextCode->instruction.temp, Condition, gcSL_ZERO)) &&
                    nextCode->instruction.tempIndex == code->instruction.tempIndex &&
                    nextCode->instruction.tempIndexed == code->instruction.tempIndexed &&
                    nextCode->instruction.source0 == code->instruction.source0 &&
                    nextCode->instruction.source0Index == code->instruction.source0Index &&
                    nextCode->instruction.source0Indexed == code->instruction.source0Indexed)
                {
                    _addInstructionToLTCList(Optimizer, code, gcvTRUE);
                    _addInstructionToLTCList(Optimizer, nextCode, gcvTRUE);
                    code = nextCode;
                }
            }
        }
        else
        {
            _addInstructionToLTCList(Optimizer, code, gcvTRUE);
        }
    }
    return status;
} /* _findLoadtimeConstantInFunction */

/***************************************************************
**
**  find LTC in a function
**  for each code in function
**    if (all sources are ((in set_of_LTC_registers
**                          or  is_constant_or_uniform )
**                  && is not depend on mutiple value))
**    {
**      set_of_LTC_registers += code's target;
**
**      list_of_LTC_code += code;
**    }
**
**  // find LDC in program
**  list_of_LTC_code = {};
**  set_of_LTC_registers = {};
**  find LTC in main
**  for each used function
**      find LTC in the function
*/
static gceSTATUS
_FindLoadtimeConstant(
    IN gcOPTIMIZER Optimizer
    )
{
    gctSIZE_T            i;
    gceSTATUS            status;

    gcmHEADER();

    /* initialize the code list and temp register list */
    gcList_Init(&Optimizer->theLTCTempRegList, &ltcAllocator);
    gcList_Init(&Optimizer->theLTCCodeList, &ltcAllocator);
    gcList_Init(&Optimizer->theLTCRemoveCodeList, &ltcAllocator);
    gcList_Init(&Optimizer->indexedVariableListForLTC, &ltcAllocator);

    /* Find loadtime constant for each function. */
    for (i = 0; i < Optimizer->functionCount; i++)
    {
        gcmERR_RETURN(_findLoadtimeConstantInFunction(Optimizer, Optimizer->functionArray + i));
    }

    /* Find loadtime constant for main. */
    gcmERR_RETURN(_findLoadtimeConstantInFunction(Optimizer, Optimizer->main));

    gcmFOOTER();
    return status;
} /* _FindLoadtimeConstant */

/* static */ gctBOOL
_isSourceSwizzled(gctSOURCE_t source)
{
    return (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, Swizzle) != gcSL_SWIZZLE_XYZW;
}

/* return true if the code in the Loadtime expression only consists of MOV instructions */
static gctBOOL
_isLTCExpressionOnlySimpleMoves(
    IN gcOPT_CODE      Code
    )
{
    gcSL_INSTRUCTION        inst = &Code->instruction;
    gctSOURCE_t             source;

    if (gcmSL_OPCODE_GET(inst->opcode, Opcode) == gcSL_MOV)
    {
        /* go through its source to check if it is also MOV or constant/uniform */
        source = inst->source0;
        if (gcmSL_SOURCE_GET(source, Type) == gcSL_CONSTANT)
        {
            return gcvTRUE;
        }
        else if (gcmSL_SOURCE_GET(source, Type) == gcSL_UNIFORM)
        {
            /* since the code is already a LTC expression, we don't need
               to check its index */
            return gcvTRUE;
        }
    } /* if */

    return gcvFALSE;
} /* _isLTCExpressionOnlySimpleMoves */

static gcSHADER_TYPE
_MapTargetFormatToShaderType(
    IN  gcOPT_CODE                Code,
    OUT gctINT                    ComponentMap[4]
    )
{
    gctUINT16      enabled = gcmSL_TARGET_GET(Code->instruction.temp, Enable);
    gctINT         usedComponents = 0;
    gcSHADER_TYPE  type = gcSHADER_FLOAT_X1;
    gcSL_FORMAT    format;

    /* if the result is temp.yw, we should only create vector of two
     * of component type, e.g. float2 or int2, and map the y => x,
     * w => y
     */
    if (enabled & gcSL_ENABLE_X)
        ComponentMap[0] = usedComponents++;
    if (enabled & gcSL_ENABLE_Y)
        ComponentMap[1] = usedComponents++;
    if (enabled & gcSL_ENABLE_Z)
        ComponentMap[2] = usedComponents++;
    if (enabled & gcSL_ENABLE_W)
        ComponentMap[3] = usedComponents++;

    /* get uniform type from target format field */
    format = (gcSL_FORMAT)gcmSL_TARGET_GET(Code->instruction.temp, Format);
    switch (format)
    {
    case gcSL_FLOAT:
    case gcSL_FLOAT16:
        type = (usedComponents == 4) ? gcSHADER_FLOAT_X4 :
               (usedComponents == 3) ? gcSHADER_FLOAT_X3 :
               (usedComponents == 2) ? gcSHADER_FLOAT_X2 :
                                       gcSHADER_FLOAT_X1;
        break;
    case gcSL_INTEGER:
    case gcSL_INT8:
    case gcSL_INT16:
        type = (usedComponents == 4) ? gcSHADER_INTEGER_X4 :
               (usedComponents == 3) ? gcSHADER_INTEGER_X3 :
               (usedComponents == 2) ? gcSHADER_INTEGER_X2 :
                                       gcSHADER_INTEGER_X1;
        break;
    case gcSL_UINT32:
    case gcSL_UINT8:
    case gcSL_UINT16:
        type = (usedComponents == 4) ? gcSHADER_UINT_X4 :
               (usedComponents == 3) ? gcSHADER_UINT_X3 :
               (usedComponents == 2) ? gcSHADER_UINT_X2 :
                                       gcSHADER_UINT_X1;
        break;
    case gcSL_BOOLEAN:
        type = (usedComponents == 4) ? gcSHADER_BOOLEAN_X4 :
               (usedComponents == 3) ? gcSHADER_BOOLEAN_X3 :
               (usedComponents == 2) ? gcSHADER_BOOLEAN_X2 :
                                       gcSHADER_BOOLEAN_X1;
        break;
    default:
        gcmASSERT(gcvFALSE);
    } /* switch */

    return type;
} /* _MapTargetFormatToShaderType */

/*******************************************************************************
**                            _CreateDummyUniformInfo
********************************************************************************
**
**  Create and add a dummy uniform with the Code's target type to shader
**
**  INPUT:
**
**      gcOPTIMIZER    Optimizer
**         Pointer to a gcOPTIMIZER structure
**
**      gcOPT_CODE     Code
**         the Code is final node in LTC expression, the result is
**         used by normal shader instruction
**
**  OUTPUT:
**
**      gcUNIFORM **     DummyUniformPtr
**          Pointer to created dummy uniform.
*/
static gceSTATUS
_CreateDummyUniformInfo(
    IN gcOPTIMIZER               Optimizer,
    IN gctCodeList               LTCCodeList,
    IN gcOPT_CODE                Code,
    OUT gcUNIFORM *              DummyUniformPtr,
    OUT gctINT                   ComponentMap[4],
    IN gcSHADER_PRECISION        Precision
    )
{
    gceSTATUS                 status;
    gcUNIFORM                 uniform;
    gctCHAR                   name[64];
    gcSHADER_TYPE             type;
    gctUINT                   offset;

    gcmHEADER();

    /* construct dummy uniform name */
    offset = 0;
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(name,
                           gcmSIZEOF(name),
                           &offset,
                           "#sh%d__ltc_uniform_%d", /* private (internal)
                                                       * uniform starts with '#' */
                           Optimizer->shader->_id,
                           Optimizer->shader->_dummyUniformCount++));
    /* add uniform */
    type = _MapTargetFormatToShaderType(Code, ComponentMap);
    gcmONERROR(gcSHADER_AddUniformEx(Optimizer->shader, name,
                                   type, Precision, 1 /* Length */, &uniform));
    SetUniformFlag(uniform, gcvUNIFORM_FLAG_LOADTIME_CONSTANT);
    SetUniformFlag(uniform, gcvUNIFORM_FLAG_COMPILER_GEN);
    *DummyUniformPtr = uniform;
    gcmFOOTER();
    return status;

OnError:
    *DummyUniformPtr = gcvNULL;
    /* Return the status. */
    gcmFOOTER();
    return status;
} /* _CreateDummyUniformInfo */


static
gceSTATUS
_DestroyTempRegisterMap(
    IN gcOPTIMIZER             Optimizer
    )
{
   gceSTATUS  status =  gcSimpleMap_Destory(Optimizer->tempRegisterMap,
                                            &ltcAllocator);
   Optimizer->tempRegisterMap = gcvNULL;
   return status;
} /* _DestroytempRegisterMap */

/*****************************************************************************
**                            _CreateMoveAndChangeDependencies
******************************************************************************
**
**  The Code's result is a loadtime constant which is used by instruction which
**  is not LTC expression itself, a dummy uniform is created for the Code, now
**  we need to transform the Code to use the dummy uniform:
**
**     target.yw = expr
**
**  transform to:
**
**     MOV target.yw, dummy_uniform._x_y
**
**  and change Code's dependency list
**
**  INPUT:
**
**        gcOPTIMIZER             Optimizer
**            Pointer to a gcOPTIMIZER structure.
**
**      gcUniform *             DummyUniform
**         Pointer to a dummy uniform created for the Code
**
**      gcOPT_CODE              Code
**         Pointer to Loadtime Constant Code
**
**  OUTPUT:
**
**      NONE
**
**  Note:
**
**    an alternative or "better" way to do it may be to change the user of the
**    Code directly if the Code result is not an output, but it is more
**    complicated, we need to deal with indexed part also
**
*/
static gceSTATUS
_CreateMoveAndChangeDependencies(
    IN gcOPTIMIZER             Optimizer,
    IN gctUINT                 DummyUniformIndex,
    IN gcOPT_CODE              Code,
    IN gctINT                  ComponentMap[4]
    )
{
    gceSTATUS            status = gcvSTATUS_OK;
    gcSL_INSTRUCTION     inst = &Code->instruction;
    gctSOURCE_t          source;
    gctUINT16            index;
    gctUINT16            indexRegister;
    gcOPT_LIST           list;
    gcSL_SWIZZLE         swizzle;
    gcmHEADER();

    /* update Code's dependencies list */
    while ((list = Code->dependencies0) != gcvNULL)
    {
        Code->dependencies0 = list->next;

        if (list->index >= 0)
        {
            gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer,
                                                  &list->code->users, Code));
        }

        /* Free the gcOPT_LIST structure. */
        gcmVERIFY_OK(gcOpt_FreeList(Optimizer, &list));
    } /* while */

    while ((list = Code->dependencies1) != gcvNULL)
    {
        Code->dependencies1 = list->next;

        /* It is possible that both sources are the same. */
        if (list->index >= 0)
        {
            gcOpt_DeleteCodeFromList(Optimizer, &list->code->users, Code);
        }

        /* Free the gcOPT_LIST structure. */
        gcmVERIFY_OK(gcOpt_FreeList(Optimizer, &list));
    } /* while */

    /* change the Code to MOV target, dummy_uniform */
    gcmSL_OPCODE_UPDATE(inst->opcode, Opcode, gcSL_MOV);

    /* Clean the condition. */
    inst->temp = gcmSL_TARGET_SET(inst->temp, Condition, gcSL_ALWAYS);

    swizzle = (ComponentMap[0] << 0 | ComponentMap[1] << 2
               | ComponentMap[2] << 4 | ComponentMap[3] << 6);
    /* Create the source. */
    source = gcmSL_SOURCE_SET(0, Type, gcSL_UNIFORM)
           | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
           | gcmSL_SOURCE_SET(0, Swizzle, swizzle)
           | gcmSL_SOURCE_SET(0, Format, gcmSL_TARGET_GET(inst->temp, Format))
           | gcmSL_SOURCE_SET(0, Precision, Optimizer->shader->uniforms[DummyUniformIndex]->precision);

    /* Create the index. */
    index = gcmSL_INDEX_SET(0, Index, DummyUniformIndex);

    indexRegister = 0;

    /* Update source0 operand. */
    inst->source0        = source;
    inst->source0Index   = index;
    inst->source0Indexed = indexRegister;

    /* set source 1 to proper value */
    source = gcmSL_SOURCE_SET(0, Type, gcSL_NONE)
           | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED);
    inst->source1        = source;

    /* Return the status. */
    gcmFOOTER();
    return status;
} /* _CreateMoveAndChangeDependencies */

/* bit 0: source 0
 * bit 1: source 1
 * bit 2: source 0 Indexed
 * bit 3: source 1 Indexed
 */
#define isSourceProcessed(Map, Index, SourceId)       \
    ((Map)[Index] & (1 << SourceId))
#define setSourceProcessed(Map, Index, SourceId)      \
    ((Map)[Index] |= (1 << SourceId))
#define isSourceIndexProcessed(Map, Index, SourceId)  \
    ((Map)[Index] & (1 << ((SourceId)+2)))
#define setSourceIndexProcessed(Map, Index, SourceId) \
    ((Map)[Index] |=  (1 << ((SourceId)+2)))

/*******************************************************************************
**                            _CloneLTCExpressionToShader
********************************************************************************
**
**  Clone the instructions in LTCCodeList to shader's ltcExpressions
**  changes ltcExpressions' register number to the same as the instruction
**  index in the array if the register number is never seen before, otherwise
**  reuse the previous mapped index
**
**    001  mov  temp(10), c10
**    002  add  temp(3), temp(10), c1
**    003  mul  temp(3), temp(3), c2
**
**  ==>
**
**    001  mov  temp(1), c10
**    002  add  temp(2), temp(1), c1
**    003  mul  temp(2), temp(2), c2   // the result is assigned to both
**                                     // location: 002 and 003
**
**  INPUT:
**
**      gcOPTIMIZER    Optimizer
**         Pointer to a gcOPTIMIZER structure
**
**      gctCodeList    LTCCodeList
**         Pointer to Loadtime Constant Code List
**
**  OUTPUT:
**
**      NONE
**
*/
static gceSTATUS
_CloneLTCExpressionToShader(
    IN gcOPTIMIZER               Optimizer,
    IN gctCodeList               LTCCodeList
    )
{
    gceSTATUS            status = gcvSTATUS_OK;
    gcSL_INSTRUCTION     inst_list;
    gctINT               i;
    gcsCodeListNode *    codeNode;
    gcSL_INSTRUCTION     inst;
    gctUINT32            mappedIndex;
    gcOPT_LIST           users;
    gcOPT_CODE           code;
    gctUINT16            temp;
    gctCHAR *            processedSourceMap;  /* used to record which source in the
                                               * instruction is already processed */
    gcmHEADER();

    gcmASSERT(LTCCodeList->count != 0);

    gcmONERROR(ltcAllocator.allocate(
                              sizeof(struct _gcSL_INSTRUCTION)*LTCCodeList->count,
                              (gctPOINTER *)&inst_list));

    gcmONERROR(ltcAllocator.allocate(
                              sizeof(gctCHAR)*LTCCodeList->count,
                              (gctPOINTER *)&processedSourceMap));
    gcoOS_ZeroMemory(processedSourceMap,
                     sizeof(gctCHAR)*LTCCodeList->count);

    /* set shader's ltcExpressions */
    Optimizer->shader->ltcExpressions = inst_list;

    codeNode = LTCCodeList->head;
    for (i=0; i<LTCCodeList->count; i++)
    {
        gcmASSERT(codeNode != gcvNULL);
        code = (gcOPT_CODE)codeNode->data;

        /* copy the instruction */
        inst_list[i] = code->instruction;
        /* record the index to the ltc array to code */
        code->ltcArrayIdx = i;   /* the ltcArrayIdx starts from 1 */

        codeNode = codeNode->next;
    }

    /* fixup the temp registers in the inst_list so it will be easier to
     * calculate later:
     *   we map the 1st instruction's result to temp[0], change all its
     *   users to use temp[0], map 2nd instruction result to temp[1] if
     *   the temp register is a new one, and change all its user to use
     *   temp[1], and so on, ...
    */

    codeNode = LTCCodeList->head;
    for (i=0; i<LTCCodeList->count; i++)
    {
        inst = &inst_list[i];
        code = (gcOPT_CODE)codeNode->data;

        /* Skip jump. */
        if (gcmSL_OPCODE_GET(inst->opcode, Opcode) == gcSL_JMP)
        {
            codeNode = codeNode->next;
            continue;
        }

        /* check the temp register */
        temp = inst->tempIndex;
        /* we don't handle indexed temp register for now */
        gcmASSERT(inst->tempIndexed == gcSL_NOT_INDEXED);

        /* map the temp register to an index in range [0, i) */
        mappedIndex = gcSimpleMap_Find(Optimizer->tempRegisterMap, temp);
        if (mappedIndex == (gctUINT32)-1) /* not in the map, add a new one */
        {
            mappedIndex = i;
            gcmONERROR(gcSimpleMap_AddNode(&Optimizer->tempRegisterMap,
                                           temp, mappedIndex,
                                           &ltcAllocator));
        }

        /* change the tempIndex to i */
        inst->tempIndex = (gctUINT16)mappedIndex;
        /* find all the uses of the temp by going through its users */
        users = code->users;
        for (users = code->users; users; users = users->next)
        {
            gctINT ltcArrayIdx;
            /* skip output/global/parameter variable usage */
            if (users->index < 0) continue;
            gcmASSERT(users->code != gcvNULL);
            ltcArrayIdx = users->code->ltcArrayIdx;
            if (ltcArrayIdx != -1)
            {
                gcSL_INSTRUCTION  use_inst;
                gctINT            j;
                /* the user's code is also in LTCCodeList,
                 * change its use to mappedIndex
                 */
                gcmASSERT(ltcArrayIdx >= 0
                          && ltcArrayIdx < LTCCodeList->count);

                use_inst = &inst_list[ltcArrayIdx];

                for (j = 0; j < 2; j++)   /* iterate through the sources */
                {
                    gctSOURCE_t source = (j == 0) ? use_inst->source0
                                                : use_inst->source1;
                    if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP )
                    {
                        /* check the source index register */
                        gctUINT16 * index = (j == 0) ? &use_inst->source0Index
                                                     : &use_inst->source1Index;
                        if (*index == temp &&
                            !isSourceProcessed(processedSourceMap,
                                           ltcArrayIdx, j) )
                        {
                            /* the user uses the temp, change it to mappedIndex */
                            *index = (gctUINT16)mappedIndex;
                            setSourceProcessed(processedSourceMap,
                                               ltcArrayIdx, j);
                        }
                        /* check the source indexed register */
                        if (gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
                        {
                            index = (j == 0) ? &use_inst->source0Indexed
                                             : &use_inst->source1Indexed;
                            if (*index == temp &&
                                !isSourceIndexProcessed(processedSourceMap,
                                                          ltcArrayIdx, j))
                            {
                                /* the user uses the temp for its indexed register,
                                 * change it to mappedIndex */
                                *index = (gctUINT16)mappedIndex;
                                setSourceIndexProcessed(processedSourceMap,
                                                          ltcArrayIdx, j);
                            }
                        } /* if */
                    }
                    else if (gcmSL_SOURCE_GET(source, Type) == gcSL_UNIFORM )
                    {
                        /* check the source indexed register */
                        if (gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
                        {
                            gctUINT16 * index = (j == 0) ? &use_inst->source0Indexed
                                                         : &use_inst->source1Indexed;
                            if (*index == temp &&
                                !isSourceIndexProcessed(processedSourceMap,
                                                          ltcArrayIdx, j))
                            {
                                /* the user uses the temp for its indexed register,
                                 * change it to mappedIndex */
                                *index = (gctUINT16)mappedIndex;
                                setSourceIndexProcessed(processedSourceMap,
                                                          ltcArrayIdx, j);
                            }
                        } /* if */
                    } /* if */
                } /* for */
            } /* if */
        } /* for */
        codeNode = codeNode->next;
    } /* for */

    /* check sanity of the fixup: every temp register source and its index should be fixed */
    for (i=0; i<LTCCodeList->count; i++)
    {
        gctINT            j;
        inst = &inst_list[i];
        for (j = 0; j < 2; j++)   /* iterate through the sources */
        {
            gctSOURCE_t source = (j == 0) ? inst->source0
                                        : inst->source1;
            if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP )
            {
                /* check the source index register */
                if (!isSourceProcessed(processedSourceMap, i, j) )
                {
                        gcmASSERT(gcvFALSE);
                        status = gcvSTATUS_INVALID_DATA;
                        break;
                }
                /* check the source indexed register */
                if (gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
                {
                    if (!isSourceIndexProcessed(processedSourceMap, i, j))
                    {
                        gcmASSERT(gcvFALSE);
                        status = gcvSTATUS_INVALID_DATA;
                        break;
                    }
                } /* if */
            }
            else if (gcmSL_SOURCE_GET(source, Type) == gcSL_UNIFORM )
            {
                /* check the source indexed register */
                if (gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
                {
                    if (!isSourceIndexProcessed(processedSourceMap, i, j))
                    {
                        gcmASSERT(gcvFALSE);
                        status = gcvSTATUS_INVALID_DATA;
                        break;
                    }
                } /* if */
            } /* if */
        } /* for */
        if (status != gcvSTATUS_OK)
            break;
    } /* for */

    /* clean up the processed source map */
    ltcAllocator.deallocate(processedSourceMap);
    /* cleanup the temp register map */
    _DestroyTempRegisterMap(Optimizer);

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
} /* _CloneLTCExpressionToShader */

static gceSTATUS
_replaceShaderCodeByDummyUniform(
    IN gcOPTIMIZER Optimizer,
    IN gcOPT_CODE Code,
    IN gctINT * LTCCodeUniformIndex,
    IN gctINT * ActiveLTCInstCount,
    IN gctUINT32 * CurUsedUniform,
    IN gctUINT * DummyUniformCount,
    IN gctINT CodeIndex,
    IN gctBOOL IsWithinJmp
    )
{
    gcOPT_CODE code = Code;
    gcOPT_LIST user;
    gceSTATUS status = gcvSTATUS_OK;

    for (user = code->users; user; user = user->next)
    {
        /* check if the user is output/global variable, otherwise
           check if the user is not in theLTCCodeList, if not, it means
           the result of the code is used by operation which is not LTC,
           generate a dummy uniform for the result. Replace the user's code
           to use the dummy unform
        */
        if (user->index < 0
            || gcList_FindNode(&Optimizer->theLTCCodeList, user->code, _ComparePtr) == gcvNULL)
        {
            /* check if the LTC expression is as simple as a string of MOV instruction,
               no need to creat another uniform if the value is moved from uniform or
               constant
            */
            if (!_isLTCExpressionOnlySimpleMoves(code) || IsWithinJmp)
            {
                gctINT    componentMap[4] = {0, 0, 0, 0};
                /* check if the dummy uniform is already created */
                if (LTCCodeUniformIndex[CodeIndex] == -1)
                {
                    /* not created yet, we need to create a dummy uniform
                     * for the expression */
                    gcUNIFORM  aDummyUniformPtr;
                    gcSHADER_PRECISION precision =
                        gcmSL_TARGET_GET(code->instruction.temp, Precision) == gcSL_PRECISION_HIGH ?
                               gcSHADER_PRECISION_HIGH : gcSHADER_PRECISION_MEDIUM;
                    gcmONERROR(_CreateDummyUniformInfo(Optimizer,
                                                       &Optimizer->theLTCCodeList,
                                                       code,
                                                       &aDummyUniformPtr,
                                                       componentMap,
                                                       precision));

                    /* map the codeIndex to uniform index */
                    LTCCodeUniformIndex[CodeIndex] =
                              aDummyUniformPtr->index;

                    *DummyUniformCount = *DummyUniformCount + 1;
                    *CurUsedUniform = *CurUsedUniform + 1;
                }

                /* convert target = expr to MOV target, dummy_uniform */
                _CreateMoveAndChangeDependencies(
                              Optimizer,
                              LTCCodeUniformIndex[CodeIndex],
                              code,
                              componentMap);
                *ActiveLTCInstCount = CodeIndex + 1;
            }
            else
            {
                /* TODO: propagate the value to its users */

            }/* if */
        } /* if */
    } /* for */

    return status;
OnError:
    return status;
}

/***************************************************************
** convert
**
**   target = expr
**
** to
**
**   dummy_uniform = expr;
**   mov target, dummy_uniform
**
** create a dummy uniform with code's result type
** change the instruction to move instruction:
**   move target, dummy_uniform
**
** remove code form code dependencies' user list
** change code's dependencies to dummy_uniform
**
*********************************************************************/
gceSTATUS
_FoldLoadtimeConstant(
    IN gcOPTIMIZER Optimizer
    )
{
    gceSTATUS            status = gcvSTATUS_OK;
    gctINT *             ltcCodeUniformIndex = 0;
    gctINT               i;

    gcOPT_CODE           code;
    gctUINT              dummyUniformCount = 0;
    /* the loadtime constant dummy uniforms always start after the user specified ones */
    gctUINT              ltcUniformBegin = Optimizer->shader->uniformCount;

    gctINT               codeIndex;
    gcsCodeListNode *    codeNode;
    gctINT               activeLTCInstCount = 0;
    gctUINT32            curUsedUniform     = 0;
    gctUINT              maxShaderUniforms;

    gcmHEADER();

    if (Optimizer->theLTCCodeList.count == 0)
    {
        gcmFOOTER();
        return status;
    }

    gcSHADER_GetUniformVectorCount(Optimizer->shader, &curUsedUniform);

    maxShaderUniforms = (Optimizer->shader->type == gcSHADER_TYPE_VERTEX)
                        ? Optimizer->maxVertexUniforms
                        : Optimizer->maxFragmentUniforms;

    /* clone the ltc expressions to shader */
    if (_CloneLTCExpressionToShader(Optimizer, &Optimizer->theLTCCodeList) == gcvSTATUS_OK)
    {
        /* create ltcCodeUniformIndex array */
        gcmONERROR(ltcAllocator.allocate(
                       sizeof(gctINT) * Optimizer->theLTCCodeList.count,
                       (gctPOINTER *)&ltcCodeUniformIndex));

        if (gcDumpOption(gceLTC_DUMP_COLLECTING))
        {
            codeNode = Optimizer->theLTCCodeList.head;
            gcoOS_Print("Collected LTC expressions, count=%d", Optimizer->theLTCCodeList.count);
            for (i=0; i<Optimizer->theLTCCodeList.count; i++)
            {
                dbg_dumpIR(&Optimizer->shader->ltcExpressions[i], ((gcOPT_CODE)(codeNode->data))->id);
                codeNode = codeNode->next;
            }
        }

        /* initialize the array */
        for (i=0; i<Optimizer->theLTCCodeList.count; i++)
            ltcCodeUniformIndex[i] = -1;

        codeNode = Optimizer->theLTCCodeList.head;
        codeIndex = 0;    /* the index number for codeNode in theTLCCodeList */
        /* go through each code in the theLTCCodeList to decide if we need
           to create a uniform for it */
        while ((curUsedUniform < maxShaderUniforms) && codeNode)
        {
            code = (gcOPT_CODE)(codeNode->data);

           _replaceShaderCodeByDummyUniform(Optimizer,
                                                                code,
                                                                ltcCodeUniformIndex,
                                                                &activeLTCInstCount,
                                                                &curUsedUniform,
                                                                &dummyUniformCount,
                                                                codeIndex,
                                                                gcvFALSE);
            codeIndex++;
            codeNode = codeNode->next;
        } /* while */
    }
    else
    {
        /* reset status to gcvSTATUS_OK, to continue optimization without LTC */
        status = gcvSTATUS_OK;
    }

    /* fill the shader with loadtime constant evaluation info */
    Optimizer->shader->ltcUniformCount = dummyUniformCount;
    Optimizer->shader->ltcUniformBegin = ltcUniformBegin;

    Optimizer->shader->ltcCodeUniformIndex = ltcCodeUniformIndex;
    Optimizer->shader->ltcInstructionCount = activeLTCInstCount;
    gcmASSERT(activeLTCInstCount <= Optimizer->theLTCCodeList.count);

    if (Optimizer->shader->ltcUniformCount != 0)
    {
        /* notify the caller status changed */
        status = gcvSTATUS_CHANGED;
        DUMP_OPTIMIZER("After Loadtime Constant Folding", Optimizer);
    }
    else
    {
        /* remove allocated storage if there is no dummy uniform created */
        if (Optimizer->shader->ltcCodeUniformIndex != gcvNULL)
        {
            gcmONERROR(gcoOS_Free(gcvNULL,
                                  Optimizer->shader->ltcCodeUniformIndex));
            Optimizer->shader->ltcCodeUniformIndex = gcvNULL;
        }
        if (Optimizer->shader->ltcExpressions != gcvNULL)
        {
            gcmONERROR(gcoOS_Free(gcvNULL,
                                  Optimizer->shader->ltcExpressions));
            Optimizer->shader->ltcExpressions      = gcvNULL;
            Optimizer->shader->ltcInstructionCount = 0;
        }
    } /* if */

OnError:
    gcmFOOTER();
    return status;
}  /* _FoldLoadtimeConstant */


gceSTATUS
gcOPT_OptimizeLoadtimeConstant(
    IN gcOPTIMIZER Optimizer
    )
{
    gceSTATUS            status = gcvSTATUS_OK;
    gcmHEADER();

    Optimizer->tempRegisterMap  = gcvNULL;
    gcmONERROR(_FindLoadtimeConstant(Optimizer));
    gcmONERROR(_FoldLoadtimeConstant(Optimizer));
    /* cleanup the LTC expressions, remove unneeded stuff */
    /* TODO: _CleanupLoadtimeConstantData() */
    gcList_Clean(&Optimizer->theLTCTempRegList, gcvFALSE);
    gcList_Clean(&Optimizer->theLTCCodeList, gcvFALSE);
    gcList_Clean(&Optimizer->theLTCRemoveCodeList, gcvFALSE);
    gcList_Clean(&Optimizer->indexedVariableListForLTC, gcvFALSE);
    gcmASSERT(Optimizer->tempRegisterMap == gcvNULL);

#if gcdUSE_WCLIP_PATCH
    if (Optimizer->shader->wClipUniformIndexList)
    {
        gcSHADER_LIST list, prevList;

        list = Optimizer->shader->wClipUniformIndexList;
        prevList = gcvNULL;

        while (list)
        {
            gcOPT_CODE code;
            gctBOOL matrixExisted[4] = {gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE};
            gctINT uniformIndex = -1;

            for (code = Optimizer->codeHead; code; code = code->next)
            {
                if (code->instruction.tempIndex < (gctUINT16)list->index ||
                    code->instruction.tempIndex > (gctUINT16)(list->index + 3))
                {
                    continue;
                }
                if (code->instruction.tempIndex > (gctUINT16)list->index &&
                    !matrixExisted[0])
                {
                    break;
                }

                matrixExisted[code->instruction.tempIndex - (gctUINT16)list->index] = gcvTRUE;

                if (code->instruction.tempIndex == (gctUINT16)list->index)
                {
                    if (gcmSL_SOURCE_GET(code->instruction.source0, Type) == gcSL_UNIFORM &&
                        gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_MOV)
                    {
                        uniformIndex = (gctUINT16)code->instruction.source0Index;
                    }
                }
            }

            /* If this shader only use part of this matrix, skip it. */
            if (!matrixExisted[0] || uniformIndex == -1)
            {
                gcSHADER_DeleteList(Optimizer->shader, &Optimizer->shader->wClipUniformIndexList, list->index);
                list = prevList;
            }
            /* Change the index from temp register index to uniform index. */
            else
            {
                gcSHADER_UpdateList(Optimizer->shader, Optimizer->shader->wClipUniformIndexList, list->index, uniformIndex);
            }

            prevList = list;

            if (list)
            {
                list = list->next;
            }
            else
            {
                list = Optimizer->shader->wClipUniformIndexList;
            }
        }
    }
#endif

OnError:
    gcmFOOTER();
    return status;
} /* gcOPT_OptimizeLoadtimeConstant */

/**********************************
 *    Run-time LTC folding
 **********************************/

extern gctUINT16 _GetSwizzle(IN gctUINT16 Swizzle, IN gctSOURCE_t Source);

static gctBOOL
_LTCIsElementEnabled(
    IN gctINT         Enable,
    IN gctUINT        element
    )
{
    gcmASSERT(element < MAX_LTC_COMPONENTS);
    return Enable & (1 << element);
}

static gceSTATUS
_LTCEvaluateUnioperator(
    IN gcSL_OPCODE    Opcode,
    IN LTCValue *     Source0Value,
    IN OUT LTCValue * ResultValue)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT    i;
    gcmHEADER_ARG("Opcode=%d Source0Value=0x%x ResultValue=0x%x", Opcode, Source0Value, ResultValue);


    /* handle operations which need multiple components together first */
    if (Opcode == gcSL_NORM)
    {
        /* gcSL_NORM:

        sqrt_s0 = sqrt(source0_x*source0_x + source0_y*source0_y + source0_z*source0_z + source0_w*source0_w)

        target_x = source0_x / sqrt_s0;
        target_y = source0_y / sqrt_s0;
        target_z = source0_z / sqrt_s0;
        target_w = source0_w / sqrt_s0;
        */
        const gctUINT sourceComponent_x = 0;
        const gctUINT sourceComponent_y = 1;
        const gctUINT sourceComponent_z = 2;
        const gctUINT sourceComponent_w = 3;

        gcmASSERT(((Source0Value->enable == gcSL_ENABLE_XYZW) &&
            (ResultValue->enable == gcSL_ENABLE_XYZW)) ||
            ((Source0Value->enable == gcSL_ENABLE_XYZ) &&
            (ResultValue->enable == gcSL_ENABLE_XYZ)) ||
            ((Source0Value->enable == gcSL_ENABLE_XY) &&
            (ResultValue->enable == gcSL_ENABLE_XY)) );

        if (Source0Value->enable == gcSL_ENABLE_XYZW)
        {
            if (ResultValue->elementType == gcSL_FLOAT)
            {
                /* get 4 components value */
                gctFLOAT source_x = Source0Value->v[sourceComponent_x].f32;
                gctFLOAT source_y = Source0Value->v[sourceComponent_y].f32;
                gctFLOAT source_z = Source0Value->v[sourceComponent_z].f32;
                gctFLOAT source_w = Source0Value->v[sourceComponent_w].f32;
                gctFLOAT sqrt_s0 = gcoMATH_SquareRoot(source_x * source_x +
                    source_y * source_y +
                    source_z * source_z +
                    source_w * source_w);

                if (sqrt_s0 == 0.0f)
                {
                    /* x,y,z,w are all zero, the normal of the vector is NaN  */
                    ResultValue->v[0].f32 = FLOAT_NaN;
                    ResultValue->v[1].f32 = FLOAT_NaN;
                    ResultValue->v[2].f32 = FLOAT_NaN;
                    ResultValue->v[3].f32 = FLOAT_NaN;
                }
                else
                {
                    /* evaluate 4 components value */
                    ResultValue->v[0].f32 = source_x / sqrt_s0;
                    ResultValue->v[1].f32 = source_y / sqrt_s0;
                    ResultValue->v[2].f32 = source_z / sqrt_s0;
                    ResultValue->v[3].f32 = source_w / sqrt_s0;
                } /* if */
            }
            else if (ResultValue->elementType == gcSL_INTEGER)
            {
                /* get 4 components value */
                gctINT source_x = Source0Value->v[sourceComponent_x].i32;
                gctINT source_y = Source0Value->v[sourceComponent_y].i32;
                gctINT source_z = Source0Value->v[sourceComponent_z].i32;
                gctINT source_w = Source0Value->v[sourceComponent_w].i32;
                gctINT sqrt_s0 = (gctINT) gcoMATH_SquareRoot((gctFLOAT)(source_x * source_x +
                    source_y * source_y +
                    source_z * source_z +
                    source_w * source_w));

                if (sqrt_s0 == 0)
                {
                    /* x,y,z,w are all zero, the normal of the vector is NaN  */
                    ResultValue->v[0].i32 = INT32_MAX;
                    ResultValue->v[1].i32 = INT32_MAX;
                    ResultValue->v[2].i32 = INT32_MAX;
                    ResultValue->v[3].i32 = INT32_MAX;
                }
                else
                {
                    /* evaluate 4 components value */
                    ResultValue->v[0].i32 = source_x / sqrt_s0;
                    ResultValue->v[1].i32 = source_y / sqrt_s0;
                    ResultValue->v[2].i32 = source_z / sqrt_s0;
                    ResultValue->v[3].i32 = source_w / sqrt_s0;
                } /* if */
            }
            else
            {
                /* Error. */
                status = gcvSTATUS_INVALID_DATA;
                gcmFOOTER();
                return status;
            } /* if */
        }
        else if (Source0Value->enable == gcSL_ENABLE_XYZ)
        {
            if (ResultValue->elementType == gcSL_FLOAT)
            {
                /* get 3 components value */
                gctFLOAT source_x = Source0Value->v[sourceComponent_x].f32;
                gctFLOAT source_y = Source0Value->v[sourceComponent_y].f32;
                gctFLOAT source_z = Source0Value->v[sourceComponent_z].f32;
                gctFLOAT sqrt_s0 = gcoMATH_SquareRoot(source_x * source_x +
                    source_y * source_y +
                    source_z * source_z );

                if (sqrt_s0 == 0.0f)
                {
                    /* x,y,z are all zero, the normal of the vector is NaN  */
                    ResultValue->v[0].f32 = FLOAT_NaN;
                    ResultValue->v[1].f32 = FLOAT_NaN;
                    ResultValue->v[2].f32 = FLOAT_NaN;
                }
                else
                {
                    /* evaluate 3 components value */
                    ResultValue->v[0].f32 = source_x / sqrt_s0;
                    ResultValue->v[1].f32 = source_y / sqrt_s0;
                    ResultValue->v[2].f32 = source_z / sqrt_s0;
                } /* if */
            }
            else if (ResultValue->elementType == gcSL_INTEGER)
            {
                /* get 3 components value */
                gctINT source_x = Source0Value->v[sourceComponent_x].i32;
                gctINT source_y = Source0Value->v[sourceComponent_y].i32;
                gctINT source_z = Source0Value->v[sourceComponent_z].i32;
                gctINT sqrt_s0 = (gctINT) gcoMATH_SquareRoot((gctFLOAT)(source_x * source_x +
                    source_y * source_y +
                    source_z * source_z));

                if (sqrt_s0 == 0)
                {
                    /* x,y,z are all zero, the normal of the vector is NaN  */
                    ResultValue->v[0].i32 = INT32_MAX;
                    ResultValue->v[1].i32 = INT32_MAX;
                    ResultValue->v[2].i32 = INT32_MAX;
                }
                else
                {
                    /* evaluate 3 components value */
                    ResultValue->v[0].i32 = source_x / sqrt_s0;
                    ResultValue->v[1].i32 = source_y / sqrt_s0;
                    ResultValue->v[2].i32 = source_z / sqrt_s0;
                } /* if */
            }
            else
            {
                /* Error. */
                status = gcvSTATUS_INVALID_DATA;
                gcmFOOTER();
                return status;
            } /* if */
        }
        else
        {
            if (ResultValue->elementType == gcSL_FLOAT)
            {
                /* get 2 components value */
                gctFLOAT source_x = Source0Value->v[sourceComponent_x].f32;
                gctFLOAT source_y = Source0Value->v[sourceComponent_y].f32;
                gctFLOAT sqrt_s0 = gcoMATH_SquareRoot(source_x * source_x +
                    source_y * source_y);

                if (sqrt_s0 == 0.0f)
                {
                    /* x,y are all zero, the normal of the vector is NaN  */
                    ResultValue->v[0].f32 = FLOAT_NaN;
                    ResultValue->v[1].f32 = FLOAT_NaN;
                }
                else
                {
                    /* evaluate 2 components value */
                    ResultValue->v[0].f32 = source_x / sqrt_s0;
                    ResultValue->v[1].f32 = source_y / sqrt_s0;
                } /* if */
            }
            else if (ResultValue->elementType == gcSL_INTEGER)
            {
                /* get 2 components value */
                gctINT source_x = Source0Value->v[sourceComponent_x].i32;
                gctINT source_y = Source0Value->v[sourceComponent_y].i32;
                gctINT sqrt_s0 = (gctINT) gcoMATH_SquareRoot((gctFLOAT)(source_x * source_x +
                    source_y * source_y));

                if (sqrt_s0 == 0)
                {
                    /* x,y are all zero, the normal of the vector is NaN  */
                    ResultValue->v[0].i32 = INT32_MAX;
                    ResultValue->v[1].i32 = INT32_MAX;
                }
                else
                {
                    /* evaluate 2 components value */
                    ResultValue->v[0].i32 = source_x / sqrt_s0;
                    ResultValue->v[1].i32 = source_y / sqrt_s0;
                } /* if */
            }
            else
            {
                /* Error. */
                status = gcvSTATUS_INVALID_DATA;
                gcmFOOTER();
                return status;
            } /* if */
        }
        gcmFOOTER();
        return status;
    } /* if */

    for (i = 0; i < MAX_LTC_COMPONENTS; i++)
    {
        ConstantValueUnion v;
        if (!_LTCIsElementEnabled(Source0Value->enable, i))
        {
            continue;
        }

        v = Source0Value->v[i];

        if (ResultValue->elementType == gcSL_FLOAT)
        {
            gctFLOAT fDst, f0;
            gctINT   intTemp;

            fDst = f0 = v.f32;

            switch (Opcode)
            {
            case gcSL_ABS:
                fDst = f0 >= 0.0f ? f0 : -f0;
                break;
            case gcSL_RCP:
                if (f0 == 0.0f)
                {
                    fDst = FLOAT_NaN;
                }
                else
                {
                    fDst = gcoMATH_Divide(1.0f, f0);
                }
                break;
            case gcSL_FLOOR:
                fDst = gcoMATH_Floor(f0);
                break;
            case gcSL_CEIL:
                fDst = gcoMATH_Ceiling(f0);
                break;
            case gcSL_FRAC:
                fDst = gcoMATH_Add(f0, -gcoMATH_Floor(f0));
                break;
            case gcSL_LOG:
                fDst = gcoMATH_Log2(f0);
                break;
            case gcSL_RSQ:
                if (f0 == 0.0f || isF32NaN(f0))
                    fDst = FLOAT_NaN;
                else
                    fDst = gcoMATH_ReciprocalSquareRoot(f0);
                break;
            case gcSL_SAT:
                fDst = f0 < 0.0f ? 0.0f : (fDst > 1.0f ? 1.0f : f0);
                break;
            case gcSL_SIN:
                fDst = gcoMATH_Sine(f0);
                break;
            case gcSL_COS:
                fDst = gcoMATH_Cosine(f0);
                break;
            case gcSL_TAN:
                fDst = gcoMATH_Tangent(f0);
                break;
            case gcSL_EXP:
                /* gcSL_EXP means exp2f, windows doesn't have this function
                yet (VS11 has it) */
                fDst = gcoMATH_Power(2.0f, f0);
                break;
            case gcSL_SIGN:
                fDst = (f0 > 0.0f) ? 1.0f : ((f0 < 0.0f) ? -1.0f : 0.0f);
                break;
            case gcSL_SQRT:
                fDst = gcoMATH_SquareRoot(f0);
                break;
            case gcSL_ACOS:
                fDst = gcoMATH_ArcCosine(f0);
                break;
            case gcSL_ASIN:
                fDst = gcoMATH_ArcSine(f0);
                break;
            case gcSL_ATAN:
                fDst = gcoMATH_ArcTangent(f0);
                break;
            case gcSL_I2F:
                intTemp = v.i32;
                fDst = gcoMATH_Int2Float(intTemp);
                break;
            default:
                gcmASSERT(gcvFALSE);
                break;
            } /* switch */
            ResultValue->v[i].f32 = fDst;
        }
        else if (ResultValue->elementType == gcSL_INTEGER)
        {
            gctINT32 iDst, i0;
            gctFLOAT ftemp;

            iDst = i0 = v.i32;

            switch (Opcode)
            {
            case gcSL_SIGN:
                iDst = (i0 > 0) ? 1 : ((i0 < 0) ? -1 : 0);
                break;
            case gcSL_ABS:
                iDst = gcmABS(i0);
                break;
            case gcSL_RCP:
                /* for openGL, we should not generate NaN */
                iDst = (i0 == 0) ? INT32_MAX : gcoMATH_Float2Int(gcoMATH_Divide(1.0f, gcoMATH_Int2Float(i0)));
                break;
            case gcSL_FLOOR:
                iDst = i0;
                break;
            case gcSL_CEIL:
                iDst = i0;
                break;
            case gcSL_FRAC:
                iDst = 0;
                break;
            case gcSL_LOG:
                ftemp = gcoMATH_Log2(gcoMATH_Int2Float(i0));
                iDst = gcoMATH_Float2Int(ftemp);
                break;
            case gcSL_RSQ:
                ftemp = gcoMATH_ReciprocalSquareRoot(gcoMATH_Int2Float(i0));
                iDst = gcoMATH_Float2Int(ftemp);
                break;
            case gcSL_SAT:
                iDst = gcmCLAMP(i0, 0, 1);
                break;
            case gcSL_F2I:
                {
                    const gctINT64 one  = 1L;
                    const gctINT64 iMin = -(one << 31);
                    const gctINT64 iMax = (one << 31) - one;
                    gctINT64 i64temp = (gctINT64)(v.f32);
                    iDst = (gctINT32)gcmCLAMP(i64temp, iMin, iMax);
                }
                break;
            default:
                gcmASSERT(gcvFALSE);
                break;
            } /* switch */

            ResultValue->v[i].i32 = iDst;
        }
        else if (ResultValue->elementType == gcSL_UINT32)
        {
            gctUINT32 uDst, u0;
            gctFLOAT ftemp;

            uDst = u0 = v.u32;

            switch (Opcode)
            {
            case gcSL_ABS:
                uDst = u0;
                break;
            case gcSL_RCP:
                /* for openGL, we should not generate NaN */
                uDst = (u0 == 0) ? INT32_MAX : gcoMATH_Float2UInt(gcoMATH_Divide(1.0f, gcoMATH_UInt2Float(u0)));
                break;
            case gcSL_FLOOR:
                uDst = u0;
                break;
            case gcSL_CEIL:
                uDst = u0;
                break;
            case gcSL_FRAC:
                uDst = 0;
                break;
            case gcSL_LOG:
                ftemp = gcoMATH_Log2(gcoMATH_UInt2Float(u0));
                uDst = gcoMATH_Float2UInt(ftemp);
                break;
            case gcSL_RSQ:
                ftemp = gcoMATH_ReciprocalSquareRoot(gcoMATH_UInt2Float(u0));
                uDst = gcoMATH_Float2UInt(ftemp);
                break;
            case gcSL_SAT:
                uDst = (u0 > 1) ? 1 : u0;
                break;
            case gcSL_F2I:
                {
                    const gctUINT64 one  = 1UL;
                    const gctUINT64 uMin = 0UL;
                    const gctUINT64 uMax = (one << 32) - one;
                    gctUINT64 u64temp = (gctUINT64)(v.f32);
                    uDst = (gctUINT32)gcmCLAMP(u64temp, uMin, uMax);
                }
                break;
            default:
                gcmASSERT(gcvFALSE);
                break;
            } /* switch */

            ResultValue->v[i].u32 = uDst;
        }
        else
        {
            status = gcvSTATUS_INVALID_DATA;
            gcmFOOTER();
            /* Error. */
            return status;
        } /* if */
    } /* for */

    /* Return the status. */
    gcmFOOTER();
    return status;
} /* _EvaluateUnioperator */

static gceSTATUS
_LTCEvaluateSetOperator(
    IN gcSL_INSTRUCTION    Inst,
    IN LTCValue *     Source0Value,
    IN LTCValue *     Source1Value,
    IN LTCValue *     Source2Value,
    OUT LTCValue *    ResultValue)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSL_FORMAT format;
    gctINT i;

    format = gcmSL_TARGET_GET(Inst->temp, Format);

    ResultValue->enable = gcSL_ENABLE_NONE;

    for (i = 0; i < MAX_LTC_COMPONENTS; i++)
    {
        ConstantValueUnion v0, v1, v2;

        if (!_LTCIsElementEnabled(Source0Value->enable, i))
        {
            continue;
        }
        v0 = Source0Value->v[i];
        v1 = Source1Value->v[i];
        v2 = Source2Value->v[i];

        switch (format)
        {
        case gcSL_FLOAT:
        case gcSL_FLOAT16:
        case gcSL_FLOAT64:
            if (v0.f32 == 0)
            {
                ResultValue->v[i] = v1;
            }
            else
            {
                ResultValue->v[i] = v2;
            }
            break;

        default:
            if (v0.u32 == 0)
            {
                ResultValue->v[i] = v1;
            }
            else
            {
                ResultValue->v[i] = v2;
            }
            break;
        }

        ResultValue->enable |= (gcSL_ENABLE_X << i);
    }

    ResultValue->elementType = format;
    return status;
}

static gceSTATUS
_LTCEvaluateJumpOperator(
    IN gcSL_INSTRUCTION    Inst,
    IN LTCValue *     Source0Value,
    IN LTCValue *     Source1Value,
    OUT LTCValue *    ResultValue)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSL_CONDITION condition;
    gcSL_FORMAT format;
    gctINT i;
    gctBOOL results[4] = {gcvTRUE, gcvTRUE, gcvTRUE, gcvTRUE};

    condition = gcmSL_TARGET_GET(Inst->temp, Condition);
    format = gcmSL_SOURCE_GET(Inst->source0, Format);

    for (i = 0; i < MAX_LTC_COMPONENTS; i++)
    {
        ConstantValueUnion v0, v1;

        if (!_LTCIsElementEnabled(Source0Value->enable, i))
        {
            continue;
        }
        v0 = Source0Value->v[i];
        v1 = Source1Value->v[i];

        switch (format)
        {
            case gcSL_FLOAT:
                {
                    gctFLOAT value0, value1;

                    value0 = v0.f32;
                    value1 = v1.f32;
                    switch(condition)
                    {
                    case gcSL_ALWAYS:
                        i = MAX_LTC_COMPONENTS;
                        break;
                    case gcSL_NOT_EQUAL:
                        results[i] = (value0 != value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_LESS_OR_EQUAL:
                        results[i] = (value0 <= value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_LESS:
                        results[i] = (value0 < value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_EQUAL:
                        results[i] = (value0 == value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_GREATER:
                        results[i] = (value0 > value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_GREATER_OR_EQUAL:
                        results[i] = (value0 >= value1) ? gcvTRUE : gcvFALSE;
                        break;
                    default:
                        gcmASSERT(0);
                        return gcvSTATUS_INVALID_DATA;
                    }
                    break;
                }

            case gcSL_INTEGER:
                {
                    gctINT value0, value1;

                    value0 = v0.i32;
                    value1 = v1.i32;
                    switch(condition)
                    {
                    case gcSL_ALWAYS:
                        i = MAX_LTC_COMPONENTS;
                        break;
                    case gcSL_NOT_EQUAL:
                        results[i] = (value0 != value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_LESS_OR_EQUAL:
                        results[i] = (value0 <= value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_LESS:
                        results[i] = (value0 < value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_EQUAL:
                        results[i] = (value0 == value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_GREATER:
                        results[i] = (value0 > value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_GREATER_OR_EQUAL:
                        results[i] = (value0 >= value1) ? gcvTRUE : gcvFALSE;
                        break;
                    default:
                        gcmASSERT(0);
                        return gcvSTATUS_INVALID_DATA;
                    }
                    break;
                }
            case gcSL_UINT32:
                {
                    gctUINT32 value0, value1;

                    value0 = v0.u32;
                    value1 = v1.u32;
                    switch(condition)
                    {
                    case gcSL_ALWAYS:
                        i = MAX_LTC_COMPONENTS;
                        break;
                    case gcSL_NOT_EQUAL:
                        results[i] = (value0 != value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_LESS_OR_EQUAL:
                        results[i] = (value0 <= value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_LESS:
                        results[i] = (value0 < value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_EQUAL:
                        results[i] = (value0 == value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_GREATER:
                        results[i] = (value0 > value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_GREATER_OR_EQUAL:
                        results[i] = (value0 >= value1) ? gcvTRUE : gcvFALSE;
                        break;
                    default:
                        gcmASSERT(0);
                        return gcvSTATUS_INVALID_DATA;
                    }
                    break;
                }
            case gcSL_BOOLEAN:
                {
                    gctBOOL value0, value1;

                    value0 = v0.b;
                    value1 = v1.b;
                    switch(condition)
                    {
                    case gcSL_ALWAYS:
                        i = MAX_LTC_COMPONENTS;
                        break;
                    case gcSL_NOT_EQUAL:
                        results[i] = (value0 != value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_LESS_OR_EQUAL:
                        results[i] = (value0 <= value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_LESS:
                        results[i] = (value0 < value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_EQUAL:
                        results[i] = (value0 == value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_GREATER:
                        results[i] = (value0 > value1) ? gcvTRUE : gcvFALSE;
                        break;
                    case gcSL_GREATER_OR_EQUAL:
                        results[i] = (value0 >= value1) ? gcvTRUE : gcvFALSE;
                        break;
                    default:
                        gcmASSERT(0);
                        return gcvSTATUS_INVALID_DATA;
                    }
                    break;
                }
            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_DATA;
        }
    }

    ResultValue->elementType = gcSL_BOOLEAN;
    ResultValue->enable = gcSL_ENABLE_X;
    ResultValue->v[0].b = results[0] & results[1] & results[2] & results[3];
    return status;
}

static gceSTATUS
_LTCEvaluateBioperator(
    IN gcSL_INSTRUCTION    Inst,
    IN LTCValue *     Source0Value,
    IN LTCValue *     Source1Value,
    OUT LTCValue *    ResultValue)
{
    gceSTATUS status = gcvSTATUS_OK;
    const gctUINT source0Component_x = 0;
    const gctUINT source0Component_y = 1;
    const gctUINT source0Component_z = 2;
    const gctUINT source0Component_w = 3;
    const gctUINT source1Component_x = 0;
    const gctUINT source1Component_y = 1;
    const gctUINT source1Component_z = 2;
    const gctUINT source1Component_w = 3;

    gctINT  i;
    gcSL_OPCODE opcode = gcmSL_OPCODE_GET(Inst->opcode, Opcode);

    gcmHEADER_ARG("Inst=0x%x Source0Value=0x%x Source1Value ResultValue=0x%x", Inst, Source0Value, Source1Value, ResultValue);

    /* handle operations which need multiple components together first */
    switch (opcode)
    {
    case gcSL_DP2:
        /* gcSL_DP2:

           target_x = target_y =
               source0_x*source1_x + source0_y*source1_y;

        */

        gcmASSERT(Source0Value->enable == gcSL_ENABLE_XY &&
                  Source1Value->enable == gcSL_ENABLE_XY);

        if (ResultValue->elementType == gcSL_FLOAT)
        {
            /* get 2 components value */
            gctFLOAT source0_x = Source0Value->v[source0Component_x].f32;
            gctFLOAT source0_y = Source0Value->v[source0Component_y].f32;

            gctFLOAT source1_x = Source1Value->v[source1Component_x].f32;
            gctFLOAT source1_y = Source1Value->v[source1Component_y].f32;

            gctFLOAT result = (source0_x * source1_x +
                                source0_y * source1_y );

            for (i = 0; i < MAX_LTC_COMPONENTS; i++)
            {
                if (!_LTCIsElementEnabled(ResultValue->enable, i)) {
                    continue;
                }
                ResultValue->v[i].f32 = result;
            }
        }
        else if (ResultValue->elementType  == gcSL_INTEGER)
        {
            /* get 2 components value */
            gctINT source0_x = Source0Value->v[source0Component_x].i32;
            gctINT source0_y = Source0Value->v[source0Component_y].i32;

            gctINT source1_x = Source1Value->v[source1Component_x].i32;
            gctINT source1_y = Source1Value->v[source1Component_y].i32;

            gctINT result = (source0_x * source1_x +
                              source0_y * source1_y );

            for (i = 0; i < MAX_LTC_COMPONENTS; i++)
            {
                if (!_LTCIsElementEnabled(ResultValue->enable, i)) {
                    continue;
                }
                ResultValue->v[i].i32 = result;
            }
        }
        else
        {
            /* Error. */
            gcmASSERT(gcvFALSE);
            status = gcvSTATUS_INVALID_DATA;
        } /* if */
        gcmFOOTER();
        return status;
    case gcSL_DP3:
        /* gcSL_DP3:

        target_x = target_y = target_z =
        source0_x*source1_x + source0_y*source1_y + source0_z*source1_z;

        */

        gcmASSERT(Source0Value->enable == gcSL_ENABLE_XYZ &&
            Source1Value->enable == gcSL_ENABLE_XYZ);

        if (ResultValue->elementType == gcSL_FLOAT)
        {
            /* get 3 components value */
            gctFLOAT source0_x = Source0Value->v[source0Component_x].f32;
            gctFLOAT source0_y = Source0Value->v[source0Component_y].f32;
            gctFLOAT source0_z = Source0Value->v[source0Component_z].f32;

            gctFLOAT source1_x = Source1Value->v[source1Component_x].f32;
            gctFLOAT source1_y = Source1Value->v[source1Component_y].f32;
            gctFLOAT source1_z = Source1Value->v[source1Component_z].f32;

            gctFLOAT result = (source0_x * source1_x +
                source0_y * source1_y +
                source0_z * source1_z);

            for (i = 0; i < MAX_LTC_COMPONENTS; i++)
            {
                if (!_LTCIsElementEnabled(ResultValue->enable, i)) {
                    continue;
                }
                ResultValue->v[i].f32 = result;
            }
        }
        else if (ResultValue->elementType  == gcSL_INTEGER)
        {
            /* get 3 components value */
            gctINT source0_x = Source0Value->v[source0Component_x].i32;
            gctINT source0_y = Source0Value->v[source0Component_y].i32;
            gctINT source0_z = Source0Value->v[source0Component_z].i32;

            gctINT source1_x = Source1Value->v[source1Component_x].i32;
            gctINT source1_y = Source1Value->v[source1Component_y].i32;
            gctINT source1_z = Source1Value->v[source1Component_z].i32;

            gctINT result = (source0_x * source1_x +
                source0_y * source1_y +
                source0_z * source1_z);

            for (i = 0; i < MAX_LTC_COMPONENTS; i++)
            {
                if (!_LTCIsElementEnabled(ResultValue->enable, i)) {
                    continue;
                }
                ResultValue->v[i].i32 = result;
            }
        }
        else
        {
            /* Error. */
            gcmASSERT(gcvFALSE);
            status = gcvSTATUS_INVALID_DATA;
        } /* if */
        gcmFOOTER();
        return status;
    case gcSL_DP4:
        /* gcSL_DP4:

        target_x = target_y = target_z = target_w
        source0_x*source1_x + source0_y*source1_y +
        source0_z*source1_z + source0_w*source1_w ;

        */

        gcmASSERT(Source0Value->enable == gcSL_ENABLE_XYZW &&
            Source1Value->enable == gcSL_ENABLE_XYZW);

        if (ResultValue->elementType == gcSL_FLOAT)
        {
            /* get 4 components value */
            gctFLOAT source0_x = Source0Value->v[source0Component_x].f32;
            gctFLOAT source0_y = Source0Value->v[source0Component_y].f32;
            gctFLOAT source0_z = Source0Value->v[source0Component_z].f32;
            gctFLOAT source0_w = Source0Value->v[source0Component_w].f32;

            gctFLOAT source1_x = Source1Value->v[source1Component_x].f32;
            gctFLOAT source1_y = Source1Value->v[source1Component_y].f32;
            gctFLOAT source1_z = Source1Value->v[source1Component_z].f32;
            gctFLOAT source1_w = Source1Value->v[source1Component_w].f32;

            gctFLOAT result = (source0_x * source1_x +
                source0_y * source1_y +
                source0_z * source1_z +
                source0_w * source1_w);

            for (i = 0; i < MAX_LTC_COMPONENTS; i++)
            {
                if (!_LTCIsElementEnabled(ResultValue->enable, i)) {
                    continue;
                }
                ResultValue->v[i].f32 = result;
            }
        }
        else if (ResultValue->elementType == gcSL_INTEGER)
        {
            /* get 4 components value */
            gctINT source0_x = Source0Value->v[source0Component_x].i32;
            gctINT source0_y = Source0Value->v[source0Component_y].i32;
            gctINT source0_z = Source0Value->v[source0Component_z].i32;
            gctINT source0_w = Source0Value->v[source0Component_w].i32;

            gctINT source1_x = Source1Value->v[source1Component_x].i32;
            gctINT source1_y = Source1Value->v[source1Component_y].i32;
            gctINT source1_z = Source1Value->v[source1Component_z].i32;
            gctINT source1_w = Source1Value->v[source1Component_w].i32;

            gctINT result = (source0_x * source1_x +
                source0_y * source1_y +
                source0_z * source1_z +
                source0_w * source1_w);
            for (i = 0; i < MAX_LTC_COMPONENTS; i++)
            {
                if (!_LTCIsElementEnabled(ResultValue->enable, i)) {
                    continue;
                }
                ResultValue->v[i].i32 = result;
            }
        }
        else
        {
            /* Error. */
            gcmASSERT(gcvFALSE);
            status = gcvSTATUS_INVALID_DATA;
        } /* if */
        gcmFOOTER();
        return status;
    case gcSL_CROSS:
        /* gcSL_CROSS:

        target_x = source0_y*source1_z - source0_z*source1_y;
        target_y = source0_z*source1_x - source0_x*source1_z;
        target_z = source0_x*source1_y - source0_y*source1_x;
        */

        gcmASSERT(Source0Value->enable == gcSL_ENABLE_XYZ &&
            Source1Value->enable == gcSL_ENABLE_XYZ &&
            ResultValue->enable == gcSL_ENABLE_XYZ);

        if (ResultValue->elementType == gcSL_FLOAT)
        {
            /* get 4 components value */
            gctFLOAT source0_x = Source0Value->v[source0Component_x].f32;
            gctFLOAT source0_y = Source0Value->v[source0Component_y].f32;
            gctFLOAT source0_z = Source0Value->v[source0Component_z].f32;

            gctFLOAT source1_x = Source1Value->v[source1Component_x].f32;
            gctFLOAT source1_y = Source1Value->v[source1Component_y].f32;
            gctFLOAT source1_z = Source1Value->v[source1Component_z].f32;

            ResultValue->v[0].f32 = source0_y*source1_z - source0_z*source1_y;
            ResultValue->v[1].f32 = source0_z*source1_x - source0_x*source1_z;
            ResultValue->v[2].f32 = source0_x*source1_y - source0_y*source1_x;
        }
        else if (ResultValue->elementType == gcSL_INTEGER)
        {
            /* get 3 components value */
            gctINT source0_x = Source0Value->v[source0Component_x].i32;
            gctINT source0_y = Source0Value->v[source0Component_y].i32;
            gctINT source0_z = Source0Value->v[source0Component_z].i32;

            gctINT source1_x = Source1Value->v[source1Component_x].i32;
            gctINT source1_y = Source1Value->v[source1Component_y].i32;
            gctINT source1_z = Source1Value->v[source1Component_z].i32;

            ResultValue->v[0].i32 = source0_y*source1_z - source0_z*source1_y;
            ResultValue->v[1].i32 = source0_z*source1_x - source0_x*source1_z;
            ResultValue->v[2].i32 = source0_x*source1_y - source0_y*source1_x;
        }
        else
        {
            /* Error. */
            gcmASSERT(gcvFALSE);
            status = gcvSTATUS_INVALID_DATA;
        } /* if */

        gcmFOOTER();
        return status;
    default:
        break;
    }
    for (i = 0; i < MAX_LTC_COMPONENTS; i++)
    {
        ConstantValueUnion v0, v1;

        if (!_LTCIsElementEnabled(Source0Value->enable, i)) {
            continue;
        }

        v0 = Source0Value->v[i];

        v1 = Source1Value->v[i];

        if (ResultValue->elementType == gcSL_FLOAT)
        {
            gctFLOAT fDst, f0, f1;

            fDst = f0 = v0.f32;
            f1 = v1.f32;

            switch (opcode)
            {
            case gcSL_CONV:
                fDst = (gctFLOAT)((gctINT)f0);
                break;
            case gcSL_ADD:
                fDst = gcoMATH_Add(f0, f1);
                break;
            case gcSL_SUB:
                fDst = gcoMATH_Add(f0, -f1);
                break;
            case gcSL_MUL:
                fDst = gcoMATH_Multiply(f0, f1);
                break;
            case gcSL_MAX:
                fDst = f0 > f1 ? f0 : f1;
                break;
            case gcSL_MIN:
                fDst = f0 < f1 ? f0 : f1;
                break;
            case gcSL_POW:
                fDst = gcoMATH_Power(f0, f1);
                break;
            case gcSL_STEP:
                fDst = f0 > f1 ? 0.0f : 1.0f;
                break;

            default:
                gcmASSERT(gcvFALSE);
                break;
            } /* switch */
            ResultValue->v[i].f32 = fDst;
        }
        else if (ResultValue->elementType == gcSL_INTEGER ||
                 ResultValue->elementType == gcSL_INT16 ||
                 ResultValue->elementType == gcSL_INT8)
        {
            gctINT iDst, i0, i1;
            gctFLOAT ftemp;

            iDst = i0 = v0.i32;
            i1 = v1.i32;

            switch (opcode)
            {
            case gcSL_ADD:
                iDst = i0 + i1; break;
            case gcSL_SUB:
                iDst = i0 - i1; break;
            case gcSL_MUL:
                iDst = i0 * i1; break;
            case gcSL_MAX:
                iDst = i0 > i1 ? i0 : i1; break;
            case gcSL_MIN:
                iDst = i0 < i1 ? i0 : i1; break;
            case gcSL_MOD:
                iDst = (i1 == 0) ? i0 : (i0 % i1); break;
            case gcSL_POW:
                ftemp = gcoMATH_Power(gcoMATH_Int2Float(i0), gcoMATH_Int2Float(i1));
                iDst = gcoMATH_Float2Int(ftemp);
                break;
            case gcSL_LSHIFT:
                gcmASSERT(i1 >= 0);
                iDst = i0 << i1;
                break;
            case gcSL_RSHIFT:
                gcmASSERT(i1 >= 0);
                iDst = i0 >> i1;
                break;
            default:
                gcmASSERT(gcvFALSE);
                break;
            } /* switch */

            ResultValue->v[i].i32 = iDst;
        }
        else if (ResultValue->elementType == gcSL_UINT32 ||
                 ResultValue->elementType == gcSL_UINT16 ||
                 ResultValue->elementType == gcSL_UINT8)
        {
            gctUINT iDst, i0, i1;
            gctFLOAT ftemp;

            iDst = i0 = v0.u32;
            i1 = v1.u32;

            switch (opcode)
            {
            case gcSL_ADD:
                iDst = i0 + i1; break;
            case gcSL_SUB:
                iDst = i0 - i1; break;
            case gcSL_MUL:
                iDst = i0 * i1; break;
            case gcSL_MAX:
                iDst = i0 > i1 ? i0 : i1; break;
            case gcSL_MIN:
                iDst = i0 < i1 ? i0 : i1; break;
            case gcSL_MOD:
                iDst = (i1 == 0) ? i0 : (i0 % i1); break;
            case gcSL_POW:
                ftemp = gcoMATH_Power(gcoMATH_Int2Float(i0), gcoMATH_Int2Float(i1));
                iDst = gcoMATH_Float2Int(ftemp);
                break;
            case gcSL_LSHIFT:
                iDst = i0 << i1;
                break;
            case gcSL_RSHIFT:
                iDst = i0 >> i1;
                break;
            default:
                gcmASSERT(gcvFALSE);
                break;
            } /* switch */

            ResultValue->v[i].u32 = iDst;
        }
        else
        {
            /* Error. */
            status = gcvSTATUS_INVALID_DATA;
            gcmFOOTER();
            return status;
        } /* if */
    } /* for */

    /* Return the status. */
    gcmFOOTER();
    return status;
} /* _EvaluateBioperator */

/* copy enabled components in ResultValue to the InstructionIndex
* and ResultLocation in the ResultArray
*/
static gceSTATUS
_LTCSetValues(
    IN OUT LTCValue * ResultsArray,
    IN LTCValue *     ResultValue,
    IN gctUINT        InstructionIndex,
    IN gctUINT        ResultLocation
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT    i;

    gcmHEADER_ARG("ResultsArray=0x%x ResultValue=0x%x InstructionIndex=%d ResultLocation=%d",
        ResultsArray, ResultValue, InstructionIndex, ResultLocation);

    ResultsArray[InstructionIndex].elementType = ResultValue->elementType;
    ResultsArray[InstructionIndex].enable |= ResultValue->enable;
    ResultsArray[InstructionIndex].sourceInfo = ResultValue->sourceInfo;
    ResultsArray[InstructionIndex].instructionIndex = InstructionIndex;
    if (ResultLocation != InstructionIndex)
    {
        ResultsArray[ResultLocation].elementType = ResultValue->elementType;
        ResultsArray[ResultLocation].enable |= ResultValue->enable;
        ResultsArray[ResultLocation].sourceInfo = ResultValue->sourceInfo;
        ResultsArray[ResultLocation].instructionIndex = InstructionIndex;
    }

    for (i = 0; i < MAX_LTC_COMPONENTS; i++)
    {
        if (!_LTCIsElementEnabled(ResultValue->enable, i))
        {
            continue;
        }
        if (ResultValue->elementType == gcSL_FLOAT)
        {
            ResultsArray[InstructionIndex].v[i].f32 = ResultValue->v[i].f32;
            if (ResultLocation != InstructionIndex)
            {
                ResultsArray[ResultLocation].v[i].f32 = ResultValue->v[i].f32;
            }
        }
        else if (ResultValue->elementType == gcSL_INTEGER)
        {
            ResultsArray[InstructionIndex].v[i].i32 = ResultValue->v[i].i32;
            if (ResultLocation != InstructionIndex)
            {
                ResultsArray[ResultLocation].v[i].i32 = ResultValue->v[i].i32;
            }
        }
        else if (ResultValue->elementType == gcSL_UINT32)
        {
            ResultsArray[InstructionIndex].v[i].u32 = ResultValue->v[i].u32;
            if (ResultLocation != InstructionIndex)
            {
                ResultsArray[ResultLocation].v[i].u32 = ResultValue->v[i].u32;
            }
        }
        else if (ResultValue->elementType == gcSL_BOOLEAN)
        {
            ResultsArray[InstructionIndex].v[i].b = ResultValue->v[i].b;
            if (ResultLocation != InstructionIndex)
            {
                ResultsArray[ResultLocation].v[i].b = ResultValue->v[i].b;
            }
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
} /* _SetValues */

static void
_LTCDumpValue(
    IN LTCValue              *Value,
    IN OUT gctSTRING         Buffer,
    IN gctUINT               BufferLen,
    IN OUT gctUINT           *Offset
    )
{
    gctINT i = 0;
    gcmHEADER_ARG("Value=0x%x Buffer=0x%x BufferLen=%d Offset=%u", Value, Buffer, BufferLen, Offset);

    gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferLen, Offset, "[ "));

    for (; i <MAX_LTC_COMPONENTS; i++)
    {
        gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferLen, Offset,
            "%10.6f", Value->v[i].f32));
        if (i != MAX_LTC_COMPONENTS -1 )
        {
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferLen, Offset, ", "));
        }
    }
    gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferLen, Offset, " ]"));
    gcmFOOTER_NO();
}

/* get value from Instructions' SourceId, the value can be from
1. Compile-time constant
2. Load-time evaluated value, it should be in some temp register
which is stored in Results
*/
static gceSTATUS
_LTCGetSourceValue(
    IN gcSHADER              Shader,
    IN gcSL_INSTRUCTION      Instruction,
    IN gctINT                SourceId,
    IN LTCValue              *Results,
    IN OUT LTCValue          *SourceValue)
{
    gceSTATUS                  status = gcvSTATUS_OK;
    gctSOURCE_t                source;
    gctUINT16                  index;
    gcSL_FORMAT                format;
    gctUINT32                  value;
    gctINT                     i;
    gcSL_OPCODE                opcode = gcmSL_OPCODE_GET(Instruction->opcode, Opcode);

    gcmHEADER_ARG("Shader=0x%x Instruction=0x%x SourceId=%d Results=0x%x SourceValue=0x%x",
        Shader, Instruction, SourceId, Results, SourceValue);

    gcmASSERT(SourceId == 0 || SourceId == 1);

    /* get source and its format */
    SourceValue->sourceInfo = source =
        (SourceId == 0) ? Instruction->source0 : Instruction->source1;
    format = (gcSL_FORMAT) gcmSL_SOURCE_GET(source, Format);
    SourceValue->elementType = format;

    /* get enabled value channel from Instruction's enable field */
    if (opcode == gcSL_DP2)
        SourceValue->enable = gcSL_ENABLE_XY;
    else if (opcode == gcSL_DP3)
        SourceValue->enable = gcSL_ENABLE_XYZ;
    else if (opcode == gcSL_DP4)
        SourceValue->enable = gcSL_ENABLE_XYZW;
    else if (opcode == gcSL_JMP)
    {
        gctSOURCE_t source = Instruction->source0;
        SourceValue->enable =  gcSL_ConvertSwizzle2Enable((gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleX),
                                                          (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleY),
                                                          (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleZ),
                                                          (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleW));
    }
    else
        SourceValue->enable = gcmSL_TARGET_GET(Instruction->temp, Enable);

    /* set value type */

    switch (gcmSL_SOURCE_GET(source, Type))
    {
    case gcSL_CONSTANT:
        /* get values one component at a time*/
        for (i = 0; i < MAX_LTC_COMPONENTS; i++)
        {
            /* value is constant, it is stored in SourceIndex and
            * SourceINdexed fields */
            value = (SourceId == 0) ? (Instruction->source0Index & 0xFFFF)
                | (Instruction->source0Indexed << 16)
                : (Instruction->source1Index & 0xFFFF)
                | (Instruction->source1Indexed << 16);
            if (format == gcSL_FLOAT)
            {
                /* float value */
                float f = gcoMATH_UIntAsFloat(value);
                SourceValue->v[i].f32 = f;
            }
            else if (format == gcSL_INTEGER)
            {
                /* integer value */
                SourceValue->v[i].i32 = value;
            }
            else if (format == gcSL_UINT32)
            {
                SourceValue->v[i].u32 = value;
            }
            else if (format == gcSL_BOOLEAN)
            {
                SourceValue->v[i].b = (value == 0 ? gcvFALSE : gcvTRUE);
            }
            else
            {
                /* Error. */
                status = gcvSTATUS_INVALID_DATA;
                break;
            }
        }
        break;
    case gcSL_TEMP:
        {
            LTCValue *            tempValue;
            gcmASSERT(gcmSL_SOURCE_GET(source, Indexed) == gcSL_NOT_INDEXED);

            /* the temp register and its indexed value should both be LTC */
            index = (SourceId == 0) ? Instruction->source0Index
                : Instruction->source1Index;
            tempValue = Results + index;

            /* get the data from tempVale one component at a time */
            for (i = 0; i < MAX_LTC_COMPONENTS; i++)
            {
                gctUINT sourceComponent;
                /* get the swizzled source component for channel i
                * (corresponding to target ith component) */
                sourceComponent = _GetSwizzle((gctUINT16)i, source);

                if (format == gcSL_FLOAT)
                {
                    /* float value */
                    SourceValue->v[i].f32 = tempValue->v[sourceComponent].f32;
                }
                else if (format == gcSL_INTEGER)
                {
                    /* integer value */
                    SourceValue->v[i].i32 = tempValue->v[sourceComponent].i32;
                }
                else if (format == gcSL_UINT32)
                {
                    SourceValue->v[i].u32 = tempValue->v[sourceComponent].u32;
                }
                else if (format == gcSL_BOOLEAN)
                {
                    SourceValue->v[i].b = tempValue->v[sourceComponent].b;
                }
                else
                {
                    /* Error. */
                    status = gcvSTATUS_INVALID_DATA;
                    break;
                }
            } /* for */
        }
        break;
    default:
        break;
    } /* switch */

    /* error handling */
    gcmASSERT(gcvSTATUS_INVALID_DATA != status);
    /* Return the status. */
    gcmFOOTER();
    return status;
} /* _GetSourceValue */

gceSTATUS gcOPT_GetUniformSrcLTC(
    IN gcSHADER              Shader,
    IN gctUINT               ltcInstIdx,
    IN gctINT                SourceId,
    IN PLTCValue             Results,
    OUT gcUNIFORM*           RetUniform,
    OUT gctINT*              RetCombinedOffset,
    OUT gctINT*              RetConstOffset,
    OUT gctINT*              RetIndexedOffset,
    OUT PLTCValue            SourceValue
    )
{
    gceSTATUS                  status = gcvSTATUS_OK;
    gctSOURCE_t                source;
    gcSL_FORMAT                format;
    gcSL_INSTRUCTION           Instruction = &Shader->ltcExpressions[ltcInstIdx];
    gcSL_OPCODE                opcode = gcmSL_OPCODE_GET(Instruction->opcode, Opcode);

    gcmHEADER_ARG("Shader=0x%x ltcInstIdx=0x%x SourceId=%d Results=0x%x RetUniform=0x%x RetCombinedOffset=0x%x SourceValue=0x%x",
                   Shader, ltcInstIdx, SourceId, Results, RetUniform, RetCombinedOffset, SourceValue);

    gcmASSERT(SourceId == 0 || SourceId == 1);

    *RetUniform = gcvNULL;
    *RetCombinedOffset = 0;

    /* get source and its format */
    SourceValue->sourceInfo = source =
        (SourceId == 0) ? Instruction->source0 : Instruction->source1;
    format = (gcSL_FORMAT) gcmSL_SOURCE_GET(source, Format);
    SourceValue->elementType = format;

    /* get enabled value channel from Instruction's enable field */
    if (opcode == gcSL_DP2)
        SourceValue->enable = gcSL_ENABLE_XY;
    else if (opcode == gcSL_DP3)
        SourceValue->enable = gcSL_ENABLE_XYZ;
    else if (opcode == gcSL_DP4)
        SourceValue->enable = gcSL_ENABLE_XYZW;
    else if (opcode == gcSL_JMP)
    {
        gctSOURCE_t source = Instruction->source0;
        SourceValue->enable =  gcSL_ConvertSwizzle2Enable((gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleX),
                                                          (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleY),
                                                          (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleZ),
                                                          (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleW));
    }
    else
        SourceValue->enable = gcmSL_TARGET_GET(Instruction->temp, Enable);

    if (gcmSL_SOURCE_GET(source, Type) == gcSL_UNIFORM)
    {
        /* the uniform can be in the form of float4x4 b[10], we may access b[7] row 3,
        which is represented as

        sourceIndex: (3<<14 | (b's uniform index)
        sourceIndexed: (4*7) = 28  (gcSL_NOT_INDEXED)

        if it is accessed by variable i:  b[i] row 3, then the index i is assigned to
        some temp register, e.g. temp(12).x, the representation looks like this:

        MOV temp(12).x, i

        sourceIndex: (3<<14 | (b's uniform index)
        sourceIndexed: 12  (gcSL_INDEXED_X)

        Note: sometime the frontend doesn't generate the constValue and constant index
        in the way described above, but the sum of constValue and constant index is
        the offset of accessed element

        */

        /* get the uniform index */
        gctINT uniformIndex = gcmSL_INDEX_GET((SourceId == 0) ? Instruction->source0Index
            : Instruction->source1Index, Index);
        gctINT constOffset = gcmSL_INDEX_GET((SourceId == 0) ? Instruction->source0Index
            : Instruction->source1Index, ConstValue);
        gctINT indexedOffset = (SourceId == 0) ? Instruction->source0Indexed
            : Instruction->source1Indexed;
        gctINT    combinedOffset = 0;
        gcUNIFORM uniform = Shader->uniforms[uniformIndex];

        gcmASSERT(uniformIndex < (gctINT)Shader->uniformCount);

        if (gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
        {
            LTCValue *            indexedValue;
            gcSL_FORMAT           indexedTempFormat;

            /* check if the index is out of bounds */
            indexedValue = Results + indexedOffset;

            /* Can not use indexedValue->sourceInfo since it (result) was not inilialized */
            indexedTempFormat = indexedValue->elementType;

            switch (gcmSL_SOURCE_GET(source, Indexed))
            {
            case gcSL_INDEXED_X:
                indexedOffset = (indexedTempFormat == gcSL_FLOAT) ? (gctINT)indexedValue->v[0].f32
                    : indexedValue->v[0].i16;
                break;
            case gcSL_INDEXED_Y:
                indexedOffset = (indexedTempFormat == gcSL_FLOAT) ? (gctINT)indexedValue->v[1].f32
                    : indexedValue->v[1].i16;
                break;
            case gcSL_INDEXED_Z:
                indexedOffset = (indexedTempFormat == gcSL_FLOAT) ? (gctINT)indexedValue->v[2].f32
                    : indexedValue->v[2].i16;
                break;
            case gcSL_INDEXED_W:
                indexedOffset = (indexedTempFormat == gcSL_FLOAT) ? (gctINT)indexedValue->v[3].f32
                    : indexedValue->v[3].i16;
                break;
            default:
                break;
            }
        }
        combinedOffset = constOffset + indexedOffset;

        /* For struct case, cast it to indexing destination uniform */
        if (uniform->parent != -1 && gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
        {
            gctINT offsetUniformIndex, deviationInOffsetUniform;

            if(gcmIS_SUCCESS(gcSHADER_GetUniformIndexingRange(Shader,
                uniformIndex,
                combinedOffset,
                gcvNULL,
                &offsetUniformIndex,
                &deviationInOffsetUniform)))
            {
                uniform = Shader->uniforms[offsetUniformIndex];
                combinedOffset = deviationInOffsetUniform;
            }
        }

        *RetUniform = uniform;
        *RetCombinedOffset = combinedOffset;
        *RetConstOffset = constOffset;
        *RetIndexedOffset = indexedOffset;
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS gcOPT_DoConstantFoldingLTC(
    IN gcSHADER              Shader,
    IN gctUINT               ltcInstIdx,
    IN PLTCValue             source0Value, /* set by driver if src0 is app's uniform */
    IN PLTCValue             source1Value, /* set by driver if src1 is app's uniform */
    IN PLTCValue             source2Value, /* set by driver if src2 is app's uniform */
    IN gctBOOL               hasSource2,
    OUT PLTCValue            resultValue, /* regarded as register file */
    IN OUT PLTCValue         Results
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gctUINT                 resultLocation;
    gctINT                  sources = 0;
    LTCValue                tmpSrc0Value, tmpSrc1Value, tmpSrc2Value;
    gcSL_INSTRUCTION        instruction = &Shader->ltcExpressions[ltcInstIdx];
    gcSL_OPCODE             opcode = gcmSL_OPCODE_GET(instruction->opcode, Opcode);

    gcmHEADER_ARG("Shader=0x%x ltcInstIdx=%d source0Value=0x%x source1Value=0x%x source2Value=0x%x resultValue=0x%x Results=0x%x",
                   Shader, ltcInstIdx, source0Value, source1Value, source2Value, resultValue, Results);

    gcmASSERT(ltcInstIdx <= Shader->ltcInstructionCount);
    gcmASSERT(resultValue && Results);

    if (gcDumpOption(gceLTC_DUMP_EVALUATION))
        dbg_dumpIR(instruction, ltcInstIdx);

    if (source0Value == gcvNULL)
    {
        _LTCGetSourceValue(Shader, instruction, 0, Results, &tmpSrc0Value);
        source0Value = &tmpSrc0Value;
    }

    if (source1Value == gcvNULL)
    {
        _LTCGetSourceValue(Shader, instruction, 1, Results, &tmpSrc1Value);
        source1Value = &tmpSrc1Value;
    }

    if (hasSource2 && source2Value == gcvNULL)
    {
        _LTCGetSourceValue(Shader, instruction + 1, 1, Results, &tmpSrc2Value);
        source2Value = &tmpSrc2Value;
    }

    /* setup the enable for the result value */
    resultValue->enable = gcmSL_TARGET_GET(instruction->temp, Enable);
    resultValue->elementType = gcmSL_TARGET_GET(instruction->temp, Format);

    /* get result location from the instruction temp index */
    resultLocation = instruction->tempIndex;

    /* if the result is indexed, we should put the result value in the
    instruction's value location:

    for example,

    005  ADD temp(0), U1, U2

    the result will be put in location {0} and {5} both places

    and

    006  MOV temp(0).1, temp(1)

    the result will only be put at {6}, we do not handle indexed
    register
    */
    if (gcmSL_TARGET_GET(instruction->temp, Indexed) != gcSL_NOT_INDEXED)
    {
        resultLocation = ltcInstIdx;
    }

    switch (opcode)
    {
    case gcSL_MOV:
    case gcSL_LOAD:
        /* mov instruction */
        *resultValue = *source0Value;
        _LTCSetValues(Results, resultValue,
            ltcInstIdx, resultLocation);
        sources = 1;
        break;
    case gcSL_RCP:
    case gcSL_ABS:
    case gcSL_FLOOR:
    case gcSL_CEIL:
    case gcSL_FRAC:
    case gcSL_LOG:
    case gcSL_RSQ:
    case gcSL_SAT:
    case gcSL_NORM:
    case gcSL_SIN:
    case gcSL_COS:
    case gcSL_TAN:
    case gcSL_EXP:
    case gcSL_SIGN:
    case gcSL_SQRT:
    case gcSL_ACOS:
    case gcSL_ASIN:
    case gcSL_ATAN:
    case gcSL_I2F:
    case gcSL_F2I:
        /* One-operand computational instruction. */
        gcmONERROR(_LTCEvaluateUnioperator(opcode,
            source0Value,
            resultValue));
        _LTCSetValues(Results, resultValue,
            ltcInstIdx, resultLocation);
        sources = 1;
        break;

    case gcSL_ADD:
    case gcSL_SUB:
    case gcSL_MUL:
    case gcSL_MAX:
    case gcSL_MIN:
    case gcSL_POW:
    case gcSL_DP2:
    case gcSL_DP3:
    case gcSL_DP4:
    case gcSL_CROSS:
    case gcSL_STEP:
    case gcSL_LSHIFT:
    case gcSL_RSHIFT:
    case gcSL_MOD:
    case gcSL_CONV:
        /* Two-operand computational instruction. */
        gcmONERROR(_LTCEvaluateBioperator(instruction,
            source0Value,
            source1Value,
            resultValue));
        _LTCSetValues(Results, resultValue,
            ltcInstIdx, resultLocation);
        sources = 2;
        break;

    case gcSL_JMP:
        gcmONERROR(_LTCEvaluateJumpOperator(instruction,
            source0Value,
            source1Value,
            resultValue));
        _LTCSetValues(Results, resultValue,
            ltcInstIdx, ltcInstIdx);
        sources = 0;
        break;

    case gcSL_SET:
        gcmONERROR(_LTCEvaluateSetOperator(instruction,
            source0Value,
            source1Value,
            source2Value,
            resultValue));
        _LTCSetValues(Results, resultValue,
            ltcInstIdx, resultLocation);
        sources = 0;
        break;

    /* one operator */
    case gcSL_DSX:
    case gcSL_DSY:
    case gcSL_FWIDTH:
    case gcSL_DIV:
    case gcSL_AND_BITWISE:
    case gcSL_OR_BITWISE:
    case gcSL_XOR_BITWISE:
    case gcSL_NOT_BITWISE:
    case gcSL_ROTATE:
    case gcSL_BITSEL:
    case gcSL_LEADZERO:
    case gcSL_ADDLO:
    case gcSL_MULLO:
    case gcSL_MULHI:
    case gcSL_CMP:
    case gcSL_ADDSAT:
    case gcSL_SUBSAT:
    case gcSL_MULSAT:
    case gcSL_MADSAT:
        /* these operators are not supported by constant folding
        need to add handling for them */

    case gcSL_CALL:
    case gcSL_RET:
    case gcSL_NOP:
    case gcSL_KILL:

    case gcSL_TEXBIAS:
    case gcSL_TEXGRAD:
    case gcSL_TEXGATHER:
    case gcSL_TEXFETCH_MS:
    case gcSL_TEXLOD:
    case gcSL_TEXU:
    case gcSL_TEXU_LOD:

    case gcSL_TEXLD:
    case gcSL_TEXLDPROJ:
    case gcSL_TEXLDPCF:
    case gcSL_TEXLDPCFPROJ:

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    if (sources != 0 && gcDumpOption(gceLTC_DUMP_EVALUATION))
    {
        gctCHAR  buffer[512];
        gctUINT  offset = 0;

        /* dump source 0 */
        gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
            "    source0 = "));
        _LTCDumpValue(source0Value, buffer, gcmSIZEOF(buffer), &offset);
        gcoOS_Print("%s", buffer);
        offset = 0;

        /* dump source 1 */
        if (sources > 1)
        {
            gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                "    source1 = "));
            _LTCDumpValue(source1Value, buffer, gcmSIZEOF(buffer), &offset);
            gcoOS_Print("%s", buffer);
            offset = 0;
        }

        /* dump result */
        gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
            "    result  = "));
        _LTCDumpValue(resultValue, buffer, gcmSIZEOF(buffer), &offset);
        gcoOS_Print("%s", buffer);
        offset = 0;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

#endif


