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


/*
**  Common gcSL optimizer module.
*/

#include "gc_vsc.h"

#if gcdENABLE_3D

#include "old_impl/gc_vsc_old_optimizer.h"

#if (defined(_WIN32) && defined (_MSC_VER))
// need for _controlfp_s and rouinding modes in RoundingMode
#include <float.h>
#elif defined(ANDROID) && defined(__arm__)

#else
#include <fenv.h>
#endif

#define _GC_OBJ_ZONE    gcdZONE_COMPILER

#define SHADER_TOO_MANY_CODE        5000
#define SHADER_TOO_MANY_JMP         600

/******************************************************************************\
|*************************** Optimization Functioins **************************|
|******************************************************************************|
|* All the functions in this file optimize the input shader without changing  *|
|* the functionality of the shader.  All the underlying utilities, including  *|
|* housekeeping functions are in gcOptUtil.c.                                 *|
\******************************************************************************/
extern gctSOURCE_t
_SetSwizzle(
    IN gctUINT16 Swizzle,
    IN gcSL_SWIZZLE Value,
    IN OUT gctSOURCE_t *Source
    );

extern gctUINT16
_SelectSwizzle(
    IN gctUINT16 Swizzle,
    IN gctSOURCE_t Source
    );

extern gctBOOL
isConditionReversible(
    IN  gcSL_CONDITION   Condition,
    OUT gcSL_CONDITION * ReversedCondition
    );

static gceSTATUS
_RemoveFunctionUnreachableCode(
    IN gcOPTIMIZER Optimizer,
    IN gcOPT_FUNCTION Function
    )
{
    gctUINT codeRemoved;
    gcOPT_CODE code;
    gcOPT_CODE codeNext;
    gceSTATUS status = gcvSTATUS_OK;

    do
    {
        code = Function->codeHead;
        codeNext = Function->codeTail->next;
        codeRemoved = 0;

        do
        {
            if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_JMP)
            {
                gcOPT_CODE codeJump;
                gcOPT_CODE codeTarget;

                if (gcmSL_TARGET_GET(code->instruction.temp, Condition) != gcSL_ALWAYS)
                {
                    code  = code->next;
                    continue;
                }

                codeJump = code;
                codeTarget = code->callee;

                code = code->next;
                while (code != codeNext &&code !=gcvNULL && code->callers == gcvNULL)
                {
                    if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_NOP)
                    {
                        gcmVERIFY_OK(gcOpt_ChangeCodeToNOP(Optimizer, code));
                        codeRemoved++;
                    }
                    code = code->next;
                }

                /* Check if the jump is redundant. */
                if (codeTarget == code)
                {
                    gcmVERIFY_OK(gcOpt_ChangeCodeToNOP(Optimizer, codeJump));
                    codeRemoved++;
                }
            }
            else if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_RET)
            {
                code = code->next;
                while (code != codeNext && code !=gcvNULL && code->callers == gcvNULL)
                 {
                    if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_NOP)
                    {
                        gcmVERIFY_OK(gcOpt_ChangeCodeToNOP(Optimizer, code));
                        codeRemoved++;
                    }
                    code = code->next;
                }
            }
            else
            {
                code = code->next;
            }
        }
        while (code != codeNext && code !=gcvNULL );

        if (codeRemoved > 0)
        {
            status = gcvSTATUS_CHANGED;
        }
    }
    while (codeRemoved > 0);

    return status;
}

extern gctBOOL
isConditionReversible(
    IN  gcSL_CONDITION   Condition,
    OUT gcSL_CONDITION * ReversedCondition
    );

/*******************************************************************************
**                          gcOpt_RemoveDeadCode
********************************************************************************
**
**  Remove unused code.
**
**  INPUT:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_RemoveDeadCode(
    IN gcOPTIMIZER Optimizer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT codeRemoved = 0;
    gctUINT i;
    gcOPT_CODE code;
    gctBOOL removeCombinedInstruction = gcvFALSE;
    gctBOOL keepCombinedInstruction = gcvFALSE;
    gcSL_CONDITION firstCondition = gcSL_ALWAYS, secondCondition = gcSL_ALWAYS;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    {
        gctBOOL     useFullNewLinker = gcvFALSE;
        gctBOOL     hasHalti2 = gcHWCaps.hwFeatureFlags.hasHalti2;

        useFullNewLinker = gcUseFullNewLinker(hasHalti2);

        /* onlu disable old DCE for vx shader for now. */
        if (useFullNewLinker && gcSHADER_GoVIRPass(Optimizer->shader) && gcShaderHasVivVxExtension(Optimizer->shader))
        {
            gcmFOOTER();
            return status;
        }
    }

    gcmONERROR(gcOpt_RemoveNOP(Optimizer));

    /* Pass 1: remove un-reachable instructions. */
    status = _RemoveFunctionUnreachableCode(Optimizer, Optimizer->main);

    for (i = 0; i < Optimizer->functionCount; i++)
    {
        gceSTATUS oStatus =
            _RemoveFunctionUnreachableCode(Optimizer,
                                           Optimizer->functionArray + i);
        if (oStatus == gcvSTATUS_CHANGED)
        {
            status = gcvSTATUS_CHANGED;
        }
    }

    if (status == gcvSTATUS_CHANGED)
        codeRemoved++;

    /* Pass 2: remove unused instructions. */
    for (code = Optimizer->codeTail; code; code = code->prev)
    {
        gcSL_OPCODE opcode = gcmSL_OPCODE_GET(code->instruction.opcode, Opcode);

        switch (opcode)
        {
        case gcSL_CALL:
        case gcSL_RET:
        case gcSL_NOP:
        case gcSL_KILL:
            /* Skip control flow instructions. */
            break;

        case gcSL_BITRANGE:
        case gcSL_BITRANGE1:
        case gcSL_TEXBIAS:
        case gcSL_TEXGRAD:
        case gcSL_TEXGATHER:
        case gcSL_TEXFETCH_MS:
        case gcSL_TEXLOD:
        case gcSL_IMAGE_SAMPLER:
        case gcSL_TEXU:
        case gcSL_TEXU_LOD:
        case gcSL_CMAD:
        case gcSL_CMADCJ:
        case gcSL_CMUL:
            if (removeCombinedInstruction)
            {
                removeCombinedInstruction = gcvFALSE;

                gcmVERIFY_OK(gcOpt_ChangeCodeToNOP(Optimizer, code));
                codeRemoved++;
            }
            break;

        case gcSL_BARRIER:
        case gcSL_MEM_BARRIER:
            /* Skip barrier instruction. */
            break;

        case gcSL_EMIT_VERTEX:
        case gcSL_END_PRIMITIVE:
        case gcSL_EMIT_STREAM_VERTEX:
        case gcSL_END_STREAM_PRIMITIVE:
            /* Skip GS control instructions. */
            break;

        case gcSL_INTRINSIC:
        case gcSL_INTRINSIC_ST:
            /* Skip intrinsic instructions. */
            {
                gcOPT_CODE definingCode;
                VIR_IntrinsicsKind intrinsicKind;
                intrinsicKind = (VIR_IntrinsicsKind)(code->instruction.source0Index |
                                                     code->instruction.source0Indexed << 16);
                if(intrinsicKind != VIR_IK_swizzle) break;
                /* Skip if no prevDefines or more than one preDefines. */
                if (code->prevDefines == gcvNULL ||
                    code->prevDefines->next) break;
                definingCode = code->prevDefines->code;
                if (definingCode->users == gcvNULL)
                {
                    gcOpt_AddCodeToList(Optimizer, &definingCode->users, code);
                }
            }
            break;

        case gcSL_JMP:
            if (! code->backwardJump)
            {
                gcOPT_CODE codeBtwn = code->callee->prev;
                gctBOOL redundant = gcvTRUE;

                /* Check if the jump jumps to next non-NOP instruction. */
                for (; codeBtwn !=gcvNULL && codeBtwn != code; codeBtwn = codeBtwn->prev)
                {
                    if (gcmSL_OPCODE_GET(codeBtwn->instruction.opcode, Opcode) != gcSL_NOP)
                    {
                        redundant = gcvFALSE;
                        break;
                    }
                }
                if (redundant)
                {
                    /* The jump is redundant. */
                    gcmVERIFY_OK(gcOpt_ChangeCodeToNOP(Optimizer, code));
                    codeRemoved++;
                }
            }
            break;

        case gcSL_SET:
        case gcSL_CMP:
            if (keepCombinedInstruction)
            {
                keepCombinedInstruction = gcvFALSE;
                break;
            }
            /* else fall through. */

        default:
            if (code->users == gcvNULL)
            {
                if (gcSL_isOpcodeTexld(opcode))
                {
                    if (code->prev)
                    {
                        gcSL_OPCODE opcodePrev = gcmSL_OPCODE_GET(code->prev->instruction.opcode, Opcode);

                        if (gcSL_isOpcodeTexldModifier(opcodePrev))
                        {
                            removeCombinedInstruction = gcvTRUE;
                        }
                    }
                }
                else if (opcode == gcSL_IMAGE_RD || opcode == gcSL_IMAGE_RD_3D)
                {
                    if (code->prev)
                    {
                        gcSL_OPCODE opcodePrev = gcmSL_OPCODE_GET(code->prev->instruction.opcode, Opcode);

                        if (opcodePrev == gcSL_IMAGE_SAMPLER)
                        {
                            removeCombinedInstruction = gcvTRUE;
                        }
                    }
                }

                /* The target is not used. */
                /* Change the instruction to NOP for non-VX shader.
                   VX shader is tricky, since it has instructions (e.g., select_add)
                   whose def are partially defined. In old optimizer there is no such partial define concept and
                   mistakenly remove "dead" instructions. Thus disable it for non-VX shader. */
                if (!gcShaderHasVivVxExtension(Optimizer->shader))
                {
                    gcmVERIFY_OK(gcOpt_ChangeCodeToNOP(Optimizer, code));
                    codeRemoved++;
                }
            }
            else if (opcode == gcSL_SET || opcode == gcSL_CMP)
            {
                if (code->prev)
                {
                    gcSL_OPCODE prevOpcode = gcmSL_OPCODE_GET(code->prev->instruction.opcode, Opcode);
                    if (prevOpcode == opcode)
                    {
                        firstCondition = gcmSL_TARGET_GET(code->prev->instruction.temp, Condition);
                        secondCondition = gcmSL_TARGET_GET(code->instruction.temp, Condition);
                        if ((firstCondition == gcSL_ALLMSB && secondCondition == gcSL_ALLMSB) ||
                            (firstCondition == gcSL_SELMSB && secondCondition == gcSL_SELMSB))
                        {
                            keepCombinedInstruction = gcvTRUE;
                        }
                        else if(secondCondition == gcSL_ZERO || secondCondition == gcSL_NOT_ZERO ||
                            secondCondition == gcSL_LESS_ZERO || secondCondition == gcSL_LESS_OREQUAL_ZERO ||
                            secondCondition == gcSL_GREATER_ZERO || secondCondition == gcSL_GREATER_OR_EQUAL_ZERO)
                        {
                            gcSL_CONDITION reversed = gcSL_ALWAYS;
                            isConditionReversible(secondCondition, &reversed);
                            if(firstCondition == reversed)
                            {
                                keepCombinedInstruction = gcvTRUE;
                            }
                        }
                    }
                }
            }
            break;
        }
    }

    if (codeRemoved != 0)
    {
        gcmONERROR(gcOpt_RemoveNOP(Optimizer));
        DUMP_OPTIMIZER("Removed dead code from the shader", Optimizer);
        gcmFOOTER();
        return status;
    }

OnError:
    gcmFOOTER();
    return status;
}

gctUINT16
_GetSwizzle(
    IN gctUINT16 Swizzle,
    IN gctSOURCE_t Source
    )
{
    /* Select on swizzle. */
    switch ((gcSL_SWIZZLE) Swizzle)
    {
    case gcSL_SWIZZLE_X:
        /* Select swizzle x. */
        return gcmSL_SOURCE_GET(Source, SwizzleX);

    case gcSL_SWIZZLE_Y:
        /* Select swizzle y. */
        return gcmSL_SOURCE_GET(Source, SwizzleY);

    case gcSL_SWIZZLE_Z:
        /* Select swizzle z. */
        return gcmSL_SOURCE_GET(Source, SwizzleZ);

    case gcSL_SWIZZLE_W:
        /* Select swizzle w. */
        return gcmSL_SOURCE_GET(Source, SwizzleW);

    default:
        return (gctUINT16) 0xFFFF;
    }
}

static gctUINT8
_ConvertEnable2Swizzle(
    IN gctUINT32 Enable
    )
{
    switch (Enable)
    {
    case gcSL_ENABLE_X:
        return gcSL_SWIZZLE_XXXX;

    case gcSL_ENABLE_Y:
        return gcSL_SWIZZLE_YYYY;

    case gcSL_ENABLE_Z:
        return gcSL_SWIZZLE_ZZZZ;

    case gcSL_ENABLE_W:
        return gcSL_SWIZZLE_WWWW;

    case gcSL_ENABLE_XY:
        return gcSL_SWIZZLE_XYYY;

    case gcSL_ENABLE_XZ:
        return gcSL_SWIZZLE_XZZZ;

    case gcSL_ENABLE_XW:
        return gcSL_SWIZZLE_XWWW;

    case gcSL_ENABLE_YZ:
        return gcSL_SWIZZLE_YZZZ;

    case gcSL_ENABLE_YW:
        return gcSL_SWIZZLE_YWWW;

    case gcSL_ENABLE_ZW:
        return gcSL_SWIZZLE_ZWWW;

    case gcSL_ENABLE_XYZ:
        return gcSL_SWIZZLE_XYZZ;

    case gcSL_ENABLE_XYW:
        return gcSL_SWIZZLE_XYWW;

    case gcSL_ENABLE_XZW:
        return gcSL_SWIZZLE_XZWW;

    case gcSL_ENABLE_YZW:
        return gcSL_SWIZZLE_YZWW;

    case gcSL_ENABLE_XYZW:
        return gcSL_SWIZZLE_XYZW;

    default:
        gcmFATAL("ERROR: Invalid enable 0x%04X", Enable);
        return gcSL_SWIZZLE_XYZW;
    }
}

static gctBOOL
_IsSingleChannelEnabled(IN gctUINT32 Enable)
{
    return ((Enable == gcSL_ENABLE_X) ||
            (Enable == gcSL_ENABLE_Y) ||
            (Enable == gcSL_ENABLE_Z) ||
            (Enable == gcSL_ENABLE_W));
}

/* check if the Code source is simple move without
   any cross swizzle like: MOV  temp(2).xy, temp(1).yz
 */
static gctBOOL
_noPrevDefineForTemp(
    IN gcOPT_CODE Code,
    IN gcOPT_CODE DepCode
    )
{
    gcOPT_CODE iterCode;
    const gctUINT32 tempIndex = Code->instruction.tempIndex;

    if (Code->prevDefines)
        return gcvFALSE;

    /* check following case:

        mov temp(2).xyz, temp(1).xyz
        MOV temp(2).w, temp(0).w    // cannot change to MOV temp(2), temp(0)
     */
    iterCode = Code->prev;
    while (iterCode)
    {
        if (iterCode->instruction.tempIndex == tempIndex)
            return gcvFALSE;

        iterCode = iterCode->prev;
    }

    return gcvTRUE;
}

/* check if the Code source is simple move without
   any cross swizzle like: MOV  temp(2).xy, temp(1).yz
 */
static gctBOOL
_isSimpleMOV(
    IN gcOPT_CODE Code
    )
{
    gctUINT16 codeEnable;
    gctSOURCE_t source;

    if (gcmSL_OPCODE_GET(Code->instruction.opcode, Opcode) != gcSL_MOV)
        return gcvFALSE;

    codeEnable = gcmSL_TARGET_GET(Code->instruction.temp, Enable);
    source     = Code->instruction.source0;

    if ((codeEnable & gcSL_ENABLE_X) &&
        (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleX) != gcSL_SWIZZLE_X)
        return gcvFALSE;

    if ((codeEnable & gcSL_ENABLE_Y) &&
        (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleY) != gcSL_SWIZZLE_Y)
        return gcvFALSE;

    if ((codeEnable & gcSL_ENABLE_Z) &&
        (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleZ) != gcSL_SWIZZLE_Z)
        return gcvFALSE;

    if ((codeEnable & gcSL_ENABLE_W) &&
        (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleW) != gcSL_SWIZZLE_W)
        return gcvFALSE;

    return gcvTRUE;
}

/*******************************************************************************
**                          gcOpt_OptimizeMOVInstructions
********************************************************************************
**
**  Optimize MOV Instruction.
**      Remove unnecessary MOV instructions while honoring gcSL's restriction--
**      target cannot be the same as either of the source operands.
**
**  INPUT:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_OptimizeMOVInstructions(
    IN gcOPTIMIZER Optimizer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcOPT_TEMP tempArray = Optimizer->tempArray;
    gcVARIABLE variable;
    gctUINT16 i;
    gctUINT codeRemoved, workingId;
    gctBOOL isArgument;
    gcOPT_CODE code, depCode, iterCode;
    gcSL_OPCODE opcode, depOpcode;
    gcOPT_LIST list, caller, userList, defList;
    gctUINT16 codeEnable;
    gctUINT16 depCodeEnable;
    gctBOOL bSelfMoveGenerated;
    gctBOOL imagePatch;
    gctBOOL isdepOpcodeSourceComponentWised;
    gctBOOL isdepOpcodeTargetComponentWised;
    gctBOOL needToChangeSwizzle;
    gctSOURCE_t orgDepSource0, orgDepSource1;
    gcSL_SWIZZLE lastSwizzle0, lastSwizzle1;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    if (Optimizer->shader->type == gcSHADER_TYPE_GEOMETRY)
    {
        gcmFOOTER();
        return status;
    }

    if (Optimizer->shader->codeCount > SHADER_TOO_MANY_CODE &&
        Optimizer->jmpCount > SHADER_TOO_MANY_JMP)
    {
        gcmFOOTER();
        return status;
    }

    /* For CTS, don't do MOV optimizer before LTC so we can move more instructions into LTC. */
    if (Optimizer->isCTSInline && Optimizer->shader->ltcUniformCount == 0)
    {
        gcmFOOTER();
        return status;
    }

    imagePatch = gcdHasOptimization(Optimizer->option, gcvOPTIMIZATION_IMAGE_PATCHING);
    /* Pass 1: Check if a MOV instruction can be removed by replacing the dependant instructions' target
               with the target of the MOV instruction. */
    codeRemoved = 0;
    for (code = Optimizer->codeTail; code; code = code->prev)
    {
        /* Test for MOV instruction. */
        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_MOV) continue;

        if (gcmSL_OPCODE_GET(code->instruction.opcode, Sat) != gcSL_NO_SATURATE) continue;

        /* Skip MOV without dependancy. */
        if (code->dependencies0 == gcvNULL) continue;

        /* Skip MOV with multiple dependancies. */
        if (code->dependencies0->next != gcvNULL) continue;

        /* Skip if src is volatile. */
        if (gcmSL_SOURCE_GET(code->instruction.source0, Type) == gcSL_TEMP)
        {
            variable = tempArray[gcmSL_INDEX_GET(code->instruction.source0Index, Index)].arrayVariable;
            if (variable && (variable->qualifier & gcvTYPE_QUALIFIER_VOLATILE)) continue;
        }

        /* Skip function arguments in callers. */
        if (tempArray[code->instruction.tempIndex].argument != gcvNULL)
        {
            if (tempArray[code->instruction.tempIndex].function != code->function)
            {
                gctBOOL skip = gcvFALSE;

                if (tempArray[code->instruction.tempIndex].argument->qualifier == gcvFUNCTION_OUTPUT)
                {
                    continue;
                }

                /* When a temp register is used as a function input,
                ** if this function is called inside the other function, we can't optimize it.
                */
                for (caller = tempArray[code->instruction.tempIndex].function->codeHead->callers;
                      caller != gcvNULL;
                      caller = caller->next)
                {
                    if (caller->code->function != code->function)
                    {
                        skip = gcvTRUE;
                        break;
                    }
                }
                if (skip)
                    continue;
            }
        }

        /* Skip union. */
        if (tempArray[code->instruction.tempIndex].format == -2) continue;
        if (gcmSL_SOURCE_GET(code->instruction.source0, Type) == gcSL_TEMP &&
            tempArray[code->instruction.source0Index].format == -2) continue;

        /* Make sure the source is temp register and not indexed. */
        if ((gcmSL_SOURCE_GET(code->instruction.source0, Type) != gcSL_TEMP)
        ||  (gcmSL_SOURCE_GET(code->instruction.source0, Indexed) != gcSL_NOT_INDEXED))
        {
            continue;
        }

        /* Skip global register. */
        if (code->dependencies0->index < 0) continue;

        depCode = code->dependencies0->code;
        depOpcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode);
        isdepOpcodeSourceComponentWised =
            !(depOpcode == gcSL_DP3 || depOpcode == gcSL_DP4 || depOpcode == gcSL_DP2 ||
              depOpcode == gcSL_CROSS || depOpcode == gcSL_NORM || depOpcode == gcSL_LOAD ||
              depOpcode == gcSL_ATTR_ST || depOpcode == gcSL_ATTR_LD ||
              gcSL_isOpcodeTexld(depOpcode) ||
              depOpcode == gcSL_ARCTRIG0 || depOpcode == gcSL_ARCTRIG1 ||
              depOpcode == gcSL_IMAGE_WR || depOpcode == gcSL_IMAGE_RD ||
              depOpcode == gcSL_IMAGE_ADDR || depOpcode == gcSL_IMAGE_ADDR_3D);

        isdepOpcodeTargetComponentWised =
            !(depOpcode == gcSL_CROSS || depOpcode == gcSL_NORM || depOpcode == gcSL_LOAD ||
              depOpcode == gcSL_ATTR_ST || depOpcode == gcSL_ATTR_LD ||
              gcSL_isOpcodeTexld(depOpcode) ||
              depOpcode == gcSL_ARCTRIG0 || depOpcode == gcSL_ARCTRIG1 ||
              depOpcode == gcSL_IMAGE_WR || depOpcode == gcSL_IMAGE_RD ||
              depOpcode == gcSL_IMAGE_ADDR || depOpcode == gcSL_IMAGE_ADDR_3D);

        needToChangeSwizzle = (depOpcode != gcSL_GET_SAMPLER_IDX);

        /* Skip if dependant instruction is a CALL instruction. */
        if (gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode) == gcSL_CALL) continue;

        /* Skip if dependant instruction is a CMP instruction, CMP.Z and CMP.nz
           are special pair to mean select so they cannot be altered */
        if (gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode) == gcSL_CMP) continue;

        /*For PS shader, we don't optimize this case:
          OR_BITWISE      temp.uint(60).hp, temp.uint(55).hp, temp.uint(59).hp
          MOV             temp(38), temp(60).hp,
          where temp(38) is mediump and temp(60) is highp and
          temp(60)'s format is different in MOV and OR_BITWISE
          Since in dual16, integer and float have different behavior in highp ->mediump
        */
        if (Optimizer->shader->type == gcSHADER_TYPE_FRAGMENT &&
            gcmSL_SOURCE_GET(code->instruction.source0, Precision) != gcmSL_TARGET_GET(code->instruction.temp, Precision) &&
            gcmSL_SOURCE_GET(code->instruction.source0, Format) != gcmSL_TARGET_GET(depCode->instruction.temp, Format))
        {
            continue;
        }

        /* Skip if dependant instruction is a BITRANGE/BITEXTRACT/BITINSERT instruction,
           since they are special pair to mean select so they cannot be altered */
        if (gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode) == gcSL_BITEXTRACT ||
            gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode) == gcSL_BITRANGE ||
            gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode) == gcSL_BITRANGE1 ||
            gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode) == gcSL_BITINSERT ||
            gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode) == gcSL_TEXU ||
            gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode) == gcSL_TEXU_LOD ||
            gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode) == gcSL_STORE ||
            gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode) == gcSL_CMAD ||
            gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode) == gcSL_CMADCJ ||
            gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode) == gcSL_CMUL)
        {
                continue;
        }

        /* Skip if dependant instruction has different target format. */
        if (tempArray[depCode->instruction.tempIndex].format !=
            tempArray[code->instruction.tempIndex].format) continue;

        /* Skip MOV with dependant that has multiple users. */
        if (depCode->users->next != gcvNULL) continue;

        /* skip MOV which depends on CALL:
         *    CALL  120
         *    MOV   temp(20), temp(64)      // temp(64) is return value
         *    CALL  120
         *    MOV   temp(21), temp(64)
         *    ..
         *    MOV   temp(120), temp(20)
         *    MOV   temp(121), temp(21)
         */
        if (depCode->dependencies0 && depCode->dependencies0->code &&
            gcmSL_OPCODE_GET(depCode->dependencies0->code->instruction.opcode, Opcode) == gcSL_CALL)
        {
            continue;
        }
        /* Make sure the source is the same as the target of the dependant instruction. */
        /* Make sure the target of the current and the dependant instructions are not indexed. */
        if ((gcmSL_INDEX_GET(code->instruction.source0Index, Index) != depCode->instruction.tempIndex)
        ||  (gcmSL_TARGET_GET(code->instruction.temp, Indexed) != gcSL_NOT_INDEXED)
        ||  (gcmSL_TARGET_GET(depCode->instruction.temp, Indexed) != gcSL_NOT_INDEXED))
        {
            continue;
        }

        /* gcSL does not allow target temp register the same as any of the source temp register. */
        bSelfMoveGenerated = gcvFALSE;
        if ((code->instruction.tempIndex == gcmSL_INDEX_GET(depCode->instruction.source0Index, Index)) &&
            (gcmSL_SOURCE_GET(depCode->instruction.source0, Type) == gcSL_TEMP))
        {
            if (gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode) == gcSL_MOV)
            {
                /* If the src0 of the dep code is indexed, just skip. */
                if (gcmSL_SOURCE_GET(depCode->instruction.source0, Indexed) != gcSL_NOT_INDEXED)
                {
                    continue;
                }

                if (_ConvertEnable2Swizzle(gcmSL_TARGET_GET(code->instruction.temp, Enable)) !=
                    gcmSL_SOURCE_GET(depCode->instruction.source0, Swizzle))
                {
                    continue;
                }

                /*
                    This case need skip too.
                       2: MOV             temp(4), temp(0)
                       3: MOV             temp(0), temp(4).hp.zyxw
                */
                if (_ConvertEnable2Swizzle(gcmSL_TARGET_GET(depCode->instruction.temp, Enable)) !=
                    gcmSL_SOURCE_GET(code->instruction.source0, Swizzle))
                {
                    continue;
                }

                bSelfMoveGenerated = gcvTRUE;
            }
            else
                continue;
        }

        /* skip target temp register the same as source1 temp register:
             add temp(2), temp(0), temp(1)
             mov temp(1), temp(2)
         */
        if ((code->instruction.tempIndex == gcmSL_INDEX_GET(depCode->instruction.source1Index, Index)) &&
            (gcmSL_SOURCE_GET(depCode->instruction.source1, Type) == gcSL_TEMP))
        {
            continue;
        }

        /* Skip MOV with prev user after dependant instruction. */
        if (code->prevDefines)
        {
            for (list = code->prevDefines; list; list = list->next)
            {
                gcOPT_CODE pdCode;
                gcOPT_LIST ulist;

                if (list->index < 0) continue;

                pdCode = list->code;
                if (pdCode->id > depCode->id && pdCode->id < code->id) break;

                for (ulist = pdCode->users; ulist; ulist = ulist->next)
                {
                    if (ulist->index < 0) continue;
                    if (ulist->code->id > depCode->id && ulist->code->id < code->id) break;
                }

                if (ulist) break;
            }

            if (list) continue;
        }

        /* Make sure there is no jump in or out between code and depCode. */
        workingId = 0;
        for (iterCode = code; iterCode != depCode; iterCode = iterCode->prev)
        {
            if (iterCode->callers || gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_JMP)
            {
                break;
            }

            iterCode->workingId = workingId ++;
        }
        if (iterCode != depCode)
        {
            continue;
        }

        depCode->workingId = workingId ++;

        /* Srcs of dep-code can not be re-written */
        if (depCode->dependencies0)
        {
            for (list = depCode->dependencies0; list; list = list->next)
            {
                if (list->index < 0) continue;

                if (list->code->nextDefines)
                {
                    for (defList = list->code->nextDefines; defList; defList = defList->next)
                    {
                        gcOPT_CODE ndCode;

                        if (defList->index < 0) continue;

                        ndCode = defList->code;
                        if (ndCode->workingId < depCode->workingId && ndCode->workingId > code->workingId)
                        {
                            if (gcmSL_SOURCE_GET(depCode->instruction.source0, Indexed) != gcSL_NOT_INDEXED ||
                                gcmSL_INDEX_GET(depCode->instruction.source0Index, Index) == ndCode->instruction.tempIndex)
                            {
                                break;
                            }
                        }
                    }

                    if (defList) break;
                }
            }

            if (list)
            {
                continue;
            }
        }

        if (depCode->dependencies1)
        {
            for (list = depCode->dependencies1; list; list = list->next)
            {
                if (list->index < 0) continue;

                if (list->code->nextDefines)
                {
                    for (defList = list->code->nextDefines; defList; defList = defList->next)
                    {
                        gcOPT_CODE ndCode;

                        if (defList->index < 0) continue;

                        ndCode = defList->code;
                        if (ndCode->workingId < depCode->workingId && ndCode->workingId > code->workingId)
                        {
                            if (gcmSL_SOURCE_GET(depCode->instruction.source1, Indexed) != gcSL_NOT_INDEXED ||
                                gcmSL_INDEX_GET(depCode->instruction.source1Index, Index) == ndCode->instruction.tempIndex)
                            {
                                break;
                            }
                        }
                    }

                    if (defList) break;
                }
            }

            if (list)
            {
                continue;
            }
        }

        codeEnable    = gcmSL_TARGET_GET(code->instruction.temp, Enable);
        depCodeEnable = gcmSL_TARGET_GET(depCode->instruction.temp, Enable);

        /* Special handling for LOAD and IMAGE_RD. */
        /* The swizzle of input data for LOAD is always xyzw, */
        /* so not all enables in MOV instruction can be optimized. */
        opcode = gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode);

        if (opcode == gcSL_IMAGE_RD || opcode == gcSL_IMAGE_RD_3D)
        {
            continue;
        }

        if (gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode) == gcSL_LOAD)
        {
            if ((codeEnable & (~depCodeEnable)) == 0 &&
                _noPrevDefineForTemp(code, depCode)  &&
                _isSimpleMOV(code) &&
                depCode->users->next == gcvNULL &&
                _ConvertEnable2Swizzle(codeEnable) == gcmSL_SOURCE_GET(code->instruction.source0, Swizzle))
            {
                gctUINT firstChannel0 = 0, firstChannel1 = 0, index;

                for (index = 0; index < 4; index++)
                {
                    if (depCodeEnable & (1 << index))
                    {
                        firstChannel0 = index;
                        break;
                    }
                }

                for (index = 0; index < 4; index++)
                {
                    if (codeEnable & (1 << index))
                    {
                        firstChannel1 = index;
                        break;
                    }
                }

                if (firstChannel1 != firstChannel0)
                {
                    continue;
                }

                /* copy code's enable to depCode */
                depCode->instruction.temp = gcmSL_TARGET_SET(depCode->instruction.temp, Enable, codeEnable);

                depCodeEnable = gcmSL_TARGET_GET(depCode->instruction.temp, Enable);
            }
            else
                continue;
        }

        if (opcode == gcSL_LOAD && codeEnable != depCodeEnable)
        {
            /* This is not true after MOV optimization. */
            /* gcmASSERT(depCodeEnable & gcSL_ENABLE_X); */

            if (depCodeEnable << 1 != codeEnable
            &&  depCodeEnable << 2 != codeEnable
            &&  depCodeEnable << 3 != codeEnable)
            {
                continue;
            }
        }

        if ((!isdepOpcodeTargetComponentWised || depOpcode == gcSL_SET)
            && !_isSimpleMOV(code))
        {
            continue;
        }

        /* skip if the src is a variable for VX shader */
        if (gcShaderHasVivVxExtension(Optimizer->shader) &&
            Optimizer->tempArray[code->instruction.source0Index].arrayVariable)
        {
            continue;
        }

        if (isdepOpcodeSourceComponentWised && needToChangeSwizzle)
        {
            orgDepSource0 = depCode->instruction.source0;
            orgDepSource1 = depCode->instruction.source1;
            lastSwizzle0 = gcSL_SWIZZLE_INVALID;
            lastSwizzle1 = gcSL_SWIZZLE_INVALID;
            /* Get the swizzle from target enable. */
            for (i = 0; i < 4; i++)
            {
                gcSL_SWIZZLE tempSwizzle, depSourceSwizzle;

                if (codeEnable & (gcSL_ENABLE_X << i))
                {
                    tempSwizzle = _SelectSwizzle(i, code->instruction.source0);
                    if (gcmSL_SOURCE_GET(orgDepSource0, Type) != gcSL_NONE)
                    {
                        /* Adjust source0 of depCode. */
                        depSourceSwizzle = _SelectSwizzle((gctUINT16)tempSwizzle, orgDepSource0);
                        depCode->instruction.source0 =
                            _SetSwizzle(i, depSourceSwizzle, &depCode->instruction.source0);
                        if (lastSwizzle0 == gcSL_SWIZZLE_INVALID)
                        {
                            gctUINT16 j;
                            for (j = 0; j < i; j++)
                            {
                                depCode->instruction.source0 =
                                    _SetSwizzle(j, depSourceSwizzle, &depCode->instruction.source0);
                            }
                        }
                        lastSwizzle0 = depSourceSwizzle;
                    }
                    if (gcmSL_SOURCE_GET(orgDepSource1, Type) != gcSL_NONE)
                    {
                        /* Adjust source1 of depCode. */
                        depSourceSwizzle = _SelectSwizzle((gctUINT16)tempSwizzle, orgDepSource1);
                        depCode->instruction.source1 =
                            _SetSwizzle(i, depSourceSwizzle, &depCode->instruction.source1);
                        if (lastSwizzle1 == gcSL_SWIZZLE_INVALID)
                        {
                            gctUINT16 j;
                            for (j = 0; j < i; j++)
                            {
                                depCode->instruction.source1 =
                                    _SetSwizzle(j, depSourceSwizzle, &depCode->instruction.source1);
                            }
                        }
                        lastSwizzle1 = depSourceSwizzle;
                    }
                }
                else
                {
                    if (gcmSL_SOURCE_GET(orgDepSource0, Type) != gcSL_NONE &&
                        lastSwizzle0 != gcSL_SWIZZLE_INVALID)
                    {
                        depCode->instruction.source0 =
                            _SetSwizzle(i, lastSwizzle0, &depCode->instruction.source0);
                    }
                    if (gcmSL_SOURCE_GET(orgDepSource1, Type) != gcSL_NONE &&
                        lastSwizzle1 != gcSL_SWIZZLE_INVALID)
                    {
                        depCode->instruction.source1 =
                            _SetSwizzle(i, lastSwizzle1, &depCode->instruction.source1);
                    }
                }
            }

            /* If failed, reset value. */
            if (i != 4)
            {
                depCode->instruction.source0 = orgDepSource0;
                depCode->instruction.source1 = orgDepSource1;
                continue;
            }
        }

        /* Special handling for SET.Z and SET.NZ pair. */
        if ((depCode->instruction.opcode == gcSL_SET || depCode->instruction.opcode == gcSL_CMP)
        &&  gcmSL_TARGET_GET(depCode->instruction.temp, Condition) == gcSL_NOT_ZERO)
        {
            if (depCode->prev)
            {
                if ((depCode->prev->instruction.opcode == gcSL_SET || depCode->prev->instruction.opcode == gcSL_CMP)
                &&  gcmSL_TARGET_GET(depCode->prev->instruction.temp, Condition) == gcSL_ZERO)
                {
                    orgDepSource0 = depCode->prev->instruction.source0;
                    orgDepSource1 = depCode->prev->instruction.source1;
                    lastSwizzle0 = gcSL_SWIZZLE_INVALID;
                    lastSwizzle1 = gcSL_SWIZZLE_INVALID;
                    /* Get the swizzle from target enable. */
                    for (i = 0; i < 4; i++)
                    {
                        gcSL_SWIZZLE tempSwizzle, depSourceSwizzle;

                        if (codeEnable & (gcSL_ENABLE_X << i))
                        {
                            tempSwizzle = _SelectSwizzle(i, code->instruction.source0);
                            /* Adjust source0 of depCode. */
                            depSourceSwizzle = _SelectSwizzle((gctUINT16)tempSwizzle, orgDepSource0);
                            depCode->prev->instruction.source0 =
                                _SetSwizzle(i, depSourceSwizzle, &depCode->prev->instruction.source0);
                            if (lastSwizzle0 == gcSL_SWIZZLE_INVALID)
                            {
                                gctUINT16 j;
                                for (j = 0; j < i; j++)
                                {
                                    depCode->prev->instruction.source0 =
                                        _SetSwizzle(j, depSourceSwizzle, &depCode->prev->instruction.source0);
                                }
                            }
                            lastSwizzle0 = tempSwizzle;
                            /* Adjust source1 of depCode. */
                            depSourceSwizzle = _SelectSwizzle((gctUINT16)tempSwizzle, orgDepSource1);
                            depCode->prev->instruction.source1 =
                                _SetSwizzle(i, depSourceSwizzle, &depCode->prev->instruction.source1);
                            if (lastSwizzle1 == gcSL_SWIZZLE_INVALID)
                            {
                                gctUINT16 j;
                                for (j = 0; j < i; j++)
                                {
                                    depCode->prev->instruction.source1 =
                                        _SetSwizzle(j, depSourceSwizzle, &depCode->prev->instruction.source1);
                                }
                            }
                            lastSwizzle1 = tempSwizzle;
                        }
                        else
                        {
                            if (gcmSL_SOURCE_GET(orgDepSource0, Type) != gcSL_NONE &&
                                lastSwizzle0 != gcSL_SWIZZLE_INVALID)
                            {
                                depCode->prev->instruction.source0 =
                                    _SetSwizzle(i, lastSwizzle0, &depCode->prev->instruction.source0);
                            }
                            if (gcmSL_SOURCE_GET(orgDepSource1, Type) != gcSL_NONE &&
                                lastSwizzle1 != gcSL_SWIZZLE_INVALID)
                            {
                                depCode->prev->instruction.source1 =
                                    _SetSwizzle(i, lastSwizzle1, &depCode->prev->instruction.source1);
                            }
                        }
                    }
                }
            }
        }

        /* Update data flow. */
        gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &code->dependencies0, depCode));
        gcmASSERT(code->dependencies0 == gcvNULL);
        gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &depCode->users, code));
        gcmASSERT(depCode->users == gcvNULL);
        depCode->users = code->users;
        code->users = gcvNULL;
        for (list = depCode->users; list; list = list->next)
        {
            gcOPT_CODE userCode;

            if (list->index < 0) continue;   /* Skip output and global dependency. */

            userCode = list->code;
            gcOpt_ReplaceCodeInList(Optimizer, &userCode->dependencies0, code, depCode);
            gcOpt_ReplaceCodeInList(Optimizer, &userCode->dependencies1, code, depCode);
        }

        /* Merge code into depCode. */
        /*depCode->instruction.temp        = code->instruction.temp;*/

#if gcdUSE_WCLIP_PATCH
        if (gcSHADER_FindList(Optimizer->shader,
                              Optimizer->shader->wClipTempIndexList,
                              (gctINT)depCode->instruction.tempIndex,
                              gcvNULL))
        {
            gcSHADER_UpdateList(Optimizer->shader,
                                Optimizer->shader->wClipTempIndexList,
                                (gctINT)depCode->instruction.tempIndex,
                                (gctINT)code->instruction.tempIndex);
        }

        if (gcSHADER_FindList(Optimizer->shader,
                              Optimizer->shader->wClipUniformIndexList,
                              (gctINT)depCode->instruction.tempIndex,
                              gcvNULL))
        {
            gcSHADER_UpdateList(Optimizer->shader,
                                Optimizer->shader->wClipUniformIndexList,
                                (gctINT)depCode->instruction.tempIndex,
                                (gctINT)code->instruction.tempIndex);
        }
#endif

        depOpcode = gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode);

        if (depCode->prev != gcvNULL && gcSL_isOpcodeTexld(depOpcode))
        {
            if (gcmSL_OPCODE_GET(depCode->prev->instruction.opcode, Opcode) == gcSL_TEXGRAD)
            {
                gcmASSERT(depCode->instruction.tempIndex == depCode->prev->instruction.tempIndex);
                depCode->prev->instruction.temp = gcmSL_TARGET_SET(depCode->instruction.temp, Enable, codeEnable);
                depCode->prev->instruction.tempIndex   = code->instruction.tempIndex;
                depCode->prev->instruction.tempIndexed = code->instruction.tempIndexed;
            }
        }

        depCode->instruction.temp = gcmSL_TARGET_SET(depCode->instruction.temp, Enable, codeEnable);
        depCode->instruction.temp = gcmSL_TARGET_SET(depCode->instruction.temp, Precision,
                                                     gcmSL_TARGET_GET(code->instruction.temp, Precision));
        depCode->instruction.tempIndex   = code->instruction.tempIndex;
        depCode->instruction.tempIndexed = code->instruction.tempIndexed;

        /* Special handling for SET.Z and SET.NZ pair. */
        if ((depCode->instruction.opcode == gcSL_SET || depCode->instruction.opcode == gcSL_CMP)
        &&  gcmSL_TARGET_GET(depCode->instruction.temp, Condition) == gcSL_NOT_ZERO)
        {
            if (depCode->prev)
            {
                if ((depCode->prev->instruction.opcode == gcSL_SET || depCode->prev->instruction.opcode == gcSL_CMP)
                &&  gcmSL_TARGET_GET(depCode->prev->instruction.temp, Condition) == gcSL_ZERO)
                {
                    depCode->prev->instruction.temp = gcmSL_TARGET_SET(depCode->prev->instruction.temp, Enable, codeEnable);
                    depCode->prev->instruction.temp = gcmSL_TARGET_SET(depCode->prev->instruction.temp, Precision,
                                                                       gcmSL_TARGET_GET(code->instruction.temp, Precision));
                    depCode->prev->instruction.tempIndex   = code->instruction.tempIndex;
                    depCode->prev->instruction.tempIndexed = code->instruction.tempIndexed;
                }
            }
        }

        /* Update nextDefines and prevDefines. */
        if (depCode->nextDefines)
        {
            for (list = depCode->nextDefines; list; list = list->next)
            {
                gcOpt_DeleteCodeFromList(Optimizer, &list->code->prevDefines, depCode);
            }
            if (depCode->prevDefines)
            {
                /* Add nextDefines to all prevDefines. */
                for (list = depCode->prevDefines; list; list = list->next)
                {
                    if (list->index == 0)
                    {
                        gcmVERIFY_OK(gcOpt_AddListToList(Optimizer, depCode->nextDefines, gcvFALSE, &list->code->nextDefines));
                    }
                }
            }
            gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &depCode->nextDefines));
        }
        if (code->nextDefines)
        {
            gcmASSERT(depCode->nextDefines == gcvNULL);
            depCode->nextDefines = code->nextDefines;
            code->nextDefines = gcvNULL;
            for (list = depCode->nextDefines; list; list = list->next)
            {
                gcOpt_ReplaceCodeInList(Optimizer, &list->code->prevDefines, code, depCode);
            }
        }
        if (depCode->prevDefines)
        {
            for (list = depCode->prevDefines; list; list = list->next)
            {
                if (list->index == 0)
                {
                    gcOpt_DeleteCodeFromList(Optimizer, &list->code->nextDefines, depCode);
                }
            }
            if (depCode->nextDefines)
            {
                /* Add prevDefines to all nextDefines. */
                for (list = depCode->nextDefines; list; list = list->next)
                {
                    gcmVERIFY_OK(gcOpt_AddListToList(Optimizer, depCode->prevDefines, gcvFALSE, &list->code->prevDefines));
                }
            }
            gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &depCode->prevDefines));
        }
        if (code->prevDefines)
        {
            gcmASSERT(depCode->prevDefines == gcvNULL);
            depCode->prevDefines = code->prevDefines;
            code->prevDefines = gcvNULL;
            for (list = depCode->prevDefines; list; list = list->next)
            {
                if (list->index == 0)
                {
                    gcOpt_ReplaceCodeInList(Optimizer, &list->code->nextDefines, code, depCode);
                }
            }
        }

        /* We need to assume that the assignment for the function arguments appear before function call. */
        isArgument = gcvFALSE;
        userList = depCode->users;
        while (userList)
        {
            if (userList->code && userList->code->instruction.opcode == gcSL_CALL)
            {
                isArgument = gcvTRUE;
                break;
            }
            userList = userList->next;
        }

        if (isArgument && depCode->function == code->function)
        {
            gcOPT_CODE nextCodeOfMovedCode = depCode->next;
            gcOPT_CODE prevCode = depCode->prev;
            gcOPT_CODE movedCode;
            gctBOOL    bMoved;

            if (prevCode != gcvNULL &&
                gcSL_isOpcodeTexldModifier(gcmSL_OPCODE_GET(prevCode->instruction.opcode, Opcode)))
            {
                movedCode = prevCode;
            }
            else
            {
                movedCode = depCode;
            }

            bMoved = gcOpt_MoveCodeListBefore(Optimizer, movedCode, depCode, code);

            if (depCode->callers && bMoved)
            {
                gcOPT_LIST  callerOfMoved;
                for (callerOfMoved = depCode->callers;
                     callerOfMoved; callerOfMoved = callerOfMoved->next)
                {
                    /* Change caller's coresponding callee node to the new code */
                    gcOPT_CODE  theCallerCode = callerOfMoved->code;
                    gcmASSERT(theCallerCode->callee == depCode);

                    /* Update caller-callee relation */
                    theCallerCode->callee = nextCodeOfMovedCode;
                }

                gcmASSERT(nextCodeOfMovedCode->callers == gcvNULL);
                nextCodeOfMovedCode->callers = depCode->callers;
                depCode->callers = gcvNULL;
            }

            if (movedCode != depCode)
            {
                if (movedCode->callers && bMoved)
                {
                    gcOPT_LIST  callerOfMoved;
                    for (callerOfMoved = movedCode->callers;
                         callerOfMoved; callerOfMoved = callerOfMoved->next)
                    {
                        /* Change caller's coresponding callee node to the new code */
                        gcOPT_CODE  theCallerCode = callerOfMoved->code;
                        gcmASSERT(theCallerCode->callee == movedCode);

                        /* Update caller-callee relation */
                        theCallerCode->callee = nextCodeOfMovedCode;
                    }

                    gcmASSERT(nextCodeOfMovedCode->callers == gcvNULL);
                    nextCodeOfMovedCode->callers = movedCode->callers;
                    movedCode->callers = gcvNULL;
                }
            }
        }

        gcmVERIFY_OK(gcOpt_ChangeCodeToNOP(Optimizer, code));
        /*code->instruction = gcvSL_NOP_INSTR;*/
        codeRemoved++;

        if (bSelfMoveGenerated)
        {
            /* Update DU info */
            for (list = depCode->dependencies0; list; list = list->next)
            {
                gcOPT_LIST ulist;

                if (list->index < 0)
                    continue;

                for (ulist = depCode->users; ulist; ulist = ulist->next)
                {
                    if (ulist->index < 0)
                        continue;

                    if (gcOpt_CheckCodeInList(Optimizer, &ulist->code->dependencies0, depCode) == gcvSTATUS_TRUE)
                        gcOpt_AddCodeToList(Optimizer, &ulist->code->dependencies0, list->code);
                    if (gcOpt_CheckCodeInList(Optimizer, &ulist->code->dependencies1, depCode) == gcvSTATUS_TRUE)
                        gcOpt_AddCodeToList(Optimizer, &ulist->code->dependencies1, list->code);

                    gcOpt_AddCodeToList(Optimizer, &list->code->users, ulist->code);
                }
            }

            for (list = depCode->users; list; list = list->next)
            {
                if (list->index < 0)
                {
                    /* the user of the self assignment statement is output
                       variable, we need to propagate the info to all its
                       prevDefines so they will have the output variable
                       as their user, then they wouldn't be optimized away
                       due to lack of this info
                     */
                    gcOPT_LIST prev_def;
                    for (prev_def = depCode->prevDefines; prev_def;
                                            prev_def = prev_def->next)
                    {
                        /* add the output dependency to the prevDefines' users */
                        if (prev_def->index < 0) continue;
                        gcOpt_AddIndexToList(Optimizer,
                                             &prev_def->code->users,
                                             list->index);
                    }
                }
            }

            gcmVERIFY_OK(gcOpt_ChangeCodeToNOP(Optimizer, depCode));
            codeRemoved++;
        }
    }

    if (codeRemoved > 0)
    {
        status = gcvSTATUS_CHANGED;
    }

    /* Pass 2: Check if a MOV instruction can be removed by replacing the users' source
               with the source of the MOV instruction. */
    codeRemoved = 0;
    for (code = Optimizer->codeHead; code; code = code->next)
    {
        gctSOURCE_t source;
        gcOPT_LIST nlist;
        gcOPT_CODE reDefinedCode;
        gctBOOL usageRemoved = gcvFALSE;
        gctBOOL propagateUniform = gcvFALSE;
        gctBOOL samplerIndexed = gcvFALSE;
        gctBOOL callDependent = gcvFALSE;

        /* Test for MOV instruction. */
        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_MOV) continue;

        /* Skip if the target is a variable. */
        if (Optimizer->tempArray[code->instruction.tempIndex].arrayVariable)
        {
            gcVARIABLE variable = Optimizer->tempArray[code->instruction.tempIndex].arrayVariable;

            if (variable &&
                (!(variable->_varCategory == gcSHADER_VAR_CATEGORY_FUNCTION_INPUT_ARGUMENT &&
                   (isSamplerType(variable->u.type) ||
                    isOCLImageType(variable->u.type) || variable->u.type == gcSHADER_SAMPLER_T))
                )
               )
            {
                if (GetVariableArraySize(variable) > 1) continue;
                if (variable->u.type != gcSHADER_FLOAT_X1 &&
                    variable->u.type != gcSHADER_INTEGER_X1 &&
                    variable->u.type != gcSHADER_UINT_X1) continue;

                /* Skip if dest is volatile. */
                if (variable->qualifier & gcvTYPE_QUALIFIER_VOLATILE) continue;
            }
        }

        /* Skip if the source is a constant (already done in constant propagation. */
        if (gcmSL_SOURCE_GET(code->instruction.source0, Type) == gcSL_CONSTANT) continue;

        /* Skip function arguments in callers. */
        if (gcmSL_SOURCE_GET(code->instruction.source0, Type) == gcSL_TEMP &&
            tempArray[gcmSL_INDEX_GET(code->instruction.source0Index, Index)].argument != gcvNULL)
        {
            if (tempArray[gcmSL_INDEX_GET(code->instruction.source0Index, Index)].function != code->function)
            {
                if (tempArray[gcmSL_INDEX_GET(code->instruction.source0Index, Index)].argument->qualifier == gcvFUNCTION_INPUT)
                {
                    continue;
                }
            }
        }

        if (code->users == gcvNULL) continue;

        /* Skip union. */
        if (tempArray[code->instruction.tempIndex].format == -2) continue;
        if (gcmSL_SOURCE_GET(code->instruction.source0, Type) == gcSL_TEMP &&
            tempArray[gcmSL_INDEX_GET(code->instruction.source0Index, Index)].format == -2) continue;

        /* Find the closest dependency temp's redefined pc. */
        reDefinedCode = Optimizer->codeTail;
        for (list = code->dependencies0; list; list = list->next)
        {
            /* Skip input arguments. */
            if (list->index < 0) break;

            /* Skip if depends on future change in a loop. */
            depCode = list->code;
            if (depCode->id >= code->id) break;

            for (nlist = depCode->nextDefines; nlist; nlist = nlist->next)
            {
                gcOPT_CODE nDefCode;

                gcmASSERT(nlist->index == 0);
                nDefCode = nlist->code;
                if (nDefCode->id < reDefinedCode->id && nDefCode->id > code->id)
                {
                    reDefinedCode = nDefCode;
                }
            }
        }

        if (list) continue;     /* Skip when future dependency exists. */
        if (gcmSL_SOURCE_GET(code->instruction.source0, Type) == gcSL_UNIFORM)
        {
            propagateUniform = gcvTRUE;
            if (imagePatch)
            {
                /* Check if uniform is image or sampler. */
                if (gcmSL_SOURCE_GET(code->instruction.source0, Indexed) == gcSL_NOT_INDEXED)
                {
                    gcUNIFORM uniform = Optimizer->shader->uniforms[gcmSL_INDEX_GET(code->instruction.source0Index, Index)];

                    if (gcmType_Kind(uniform->u.type) == gceTK_IMAGE_T ||
                        (uniform->u.type == gcSHADER_SAMPLER_T))
                    {
                        propagateUniform = gcvFALSE;
                    }
                }
            }
        }

        /* skip MOV which depends on CALL:
             *    CALL  120
             *    MOV   temp(20), temp(64)      // temp(64) is return value
             *    CALL  120
             *    MOV   temp(21), temp(64)
             *    ..
             *    MOV   temp(120), temp(20)    // should not be copied, since call 120 in between
             *    MOV   temp(121), temp(21)
             */
        if (code->dependencies0 &&
            code->dependencies0->code &&
            gcmSL_OPCODE_GET(code->dependencies0->code->instruction.opcode, Opcode) == gcSL_CALL)
        {
            callDependent = gcvTRUE;
        }

        for (list = code->users; list; list = list->next)
        {
            gcOPT_CODE userCode;
            gctBOOL canReplace = gcvFALSE;
            gctBOOL cannotReplace = gcvFALSE;

            /* Skip output or global dependency. */
            if (list->index < 0) break;

            /* Skip if any user is over the source's next define. */
            userCode = list->code;
            if (userCode->id >= reDefinedCode->id) break;

            switch (gcmSL_OPCODE_GET(userCode->instruction.opcode, Opcode))
            {
            case gcSL_CALL:
                /* No inter-procedural propagation. */
                cannotReplace = gcvTRUE;
                break;

            case gcSL_TEXBIAS:
            case gcSL_TEXGRAD:
            case gcSL_TEXGATHER:
            case gcSL_TEXFETCH_MS:
            case gcSL_TEXLOD:
            case gcSL_TEXU:
            case gcSL_TEXU_LOD:
                /* Skip texture state instructions. */
            case gcSL_TEXLD:
            case gcSL_TEXLD_U:
            case gcSL_TEXLDPROJ:
            case gcSL_TEXLDPCF:
            case gcSL_TEXLDPCFPROJ:
                /* Skip texture sample instructions. */
                {
                    /* It is a sampler indexed. */
                    if (gcmSL_SOURCE_GET(userCode->instruction.source0, Indexed) != gcSL_NOT_INDEXED &&
                        gcmSL_SOURCE_GET(userCode->instruction.source0, Type) == gcSL_SAMPLER &&
                        userCode->instruction.source0Indexed == code->instruction.tempIndex &&
                        gcmSL_SOURCE_GET(code->instruction.source0, Type) == gcSL_TEMP)
                    {
                        samplerIndexed = gcvTRUE;
                    }
                    else
                    {
                        cannotReplace = gcvTRUE;
                    }
                }
                break;

            default:
                break;
            }

            if (cannotReplace) break;

            if (callDependent)
            {
                for (iterCode = userCode; (iterCode != code) && (iterCode != gcvNULL) ; iterCode = iterCode->prev)
                {
                    if (gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_JMP ||
                        gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_CALL)
                    {
                        break;
                    }
                }
                if (iterCode != code)
                {
                    break;
                }
            }

            if (userCode->dependencies0 &&
                userCode->dependencies0->code == code &&
                userCode->dependencies0->next == gcvNULL &&
                gcmSL_SOURCE_GET(userCode->instruction.source0, Type) == gcSL_TEMP)
            {
                /* uniform and immediates are not conflict if immediate is supported, codegen can take care of uniform/uniform conflict */
                /* If source is a uniform, check if it causes two constants in an instruction. */
                if (propagateUniform &&
                    gcmSL_SOURCE_GET(userCode->instruction.source1, Type) == gcSL_CONSTANT &&
                    !Optimizer->supportImmediate)
                {
                    break;
                }
                canReplace = gcvTRUE;
            }
            if (userCode->dependencies1 &&
                userCode->dependencies1->code == code &&
                userCode->dependencies1->next == gcvNULL &&
                gcmSL_SOURCE_GET(userCode->instruction.source1, Type) == gcSL_TEMP)
            {
                /* If source is a uniform, check if it causes two constants in an instruction. */
                if (propagateUniform &&
                    gcmSL_SOURCE_GET(userCode->instruction.source0, Type) == gcSL_CONSTANT &&
                    !Optimizer->supportImmediate)
                {
                    break;
                }
                canReplace = gcvTRUE;
            }

            /* If it is a sampler indexed, we can still optimize this move. */
            if (samplerIndexed &&
                userCode->dependencies0 &&
                userCode->dependencies0->code == code &&
                userCode->dependencies0->next == gcvNULL &&
                gcmSL_SOURCE_GET(userCode->instruction.source0, Type) == gcSL_SAMPLER)
            {
                canReplace = gcvTRUE;
            }

            if (!canReplace) break;
        }

        if (list) continue;     /* Skip when not all users can be replaced. */

        source = code->instruction.source0;

        for (list = code->users; list; list = nlist)
        {
            gcOPT_CODE userCode;
            gctSOURCE_t source1;
            gcOPT_LIST ulist, depList;
            gctBOOL canRemoveUsage = gcvTRUE;

            nlist = list->next;

            gcmASSERT(list->index == 0);

            /* Get the user code. */
            userCode = list->code;

            /* gcSL does not allow target temp register the same as any of the source temp register. */
            if ((gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP) &&
                userCode->instruction.tempIndex == code->instruction.source0Index &&
                !samplerIndexed)
            {
                continue;
            }

            if (userCode->dependencies0 &&
                userCode->dependencies0->code == code &&
                userCode->dependencies0->next == gcvNULL &&
                gcmSL_TARGET_GET(code->instruction.temp, Indexed) == gcSL_NOT_INDEXED &&
                gcmSL_SOURCE_GET(userCode->instruction.source0, Indexed) == gcSL_NOT_INDEXED)
            {
                /* Get source operands. */
                gctSOURCE_t source0 = userCode->instruction.source0;

                /* Test if source0 is the target of the MOV. */
                gcmASSERT(gcmSL_SOURCE_GET(source0, Type) == gcSL_TEMP &&
                          userCode->instruction.source0Index == code->instruction.tempIndex);

                /* Change userCode's source0 to code's source0. */
                source0 = gcmSL_SOURCE_SET(source0, Type, gcmSL_SOURCE_GET(source, Type));
                source0 = gcmSL_SOURCE_SET(source0, Indexed, gcmSL_SOURCE_GET(source, Indexed));
                source0 = gcmSL_SOURCE_SET(source0, Neg, gcmSL_SOURCE_GET(source, Neg) ^ gcmSL_SOURCE_GET(source0, Neg));
                source0 = gcmSL_SOURCE_SET(source0, Abs, gcmSL_SOURCE_GET(source, Abs) | gcmSL_SOURCE_GET(source0, Abs));
                source0 = gcmSL_SOURCE_SET(source0, Precision, gcmSL_SOURCE_GET(source, Precision));
                source0 = gcmSL_SOURCE_SET(source0, SwizzleX, _GetSwizzle(gcmSL_SOURCE_GET(source0, SwizzleX), source));
                source0 = gcmSL_SOURCE_SET(source0, SwizzleY, _GetSwizzle(gcmSL_SOURCE_GET(source0, SwizzleY), source));
                source0 = gcmSL_SOURCE_SET(source0, SwizzleZ, _GetSwizzle(gcmSL_SOURCE_GET(source0, SwizzleZ), source));
                source0 = gcmSL_SOURCE_SET(source0, SwizzleW, _GetSwizzle(gcmSL_SOURCE_GET(source0, SwizzleW), source));

                userCode->instruction.source0        = source0;
                userCode->instruction.source0Index   = code->instruction.source0Index;
                userCode->instruction.source0Indexed = code->instruction.source0Indexed;

                /* Update data flow. */
                gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &userCode->dependencies0, code));

                if (code->dependencies0)
                {
                    gcmVERIFY_OK(gcOpt_AddListToList(Optimizer, code->dependencies0, gcvFALSE, &userCode->dependencies0));
                    for (ulist = code->dependencies0; ulist; ulist = ulist->next)
                    {
                        if (ulist->index >= 0)
                        {
                            gcmVERIFY_OK(gcOpt_AddCodeToList(Optimizer, &ulist->code->users, userCode));
                        }
                    }
                }
            }

            if (userCode->dependencies1 &&
                userCode->dependencies1->code == code &&
                userCode->dependencies1->next == gcvNULL &&
                gcmSL_TARGET_GET(code->instruction.temp, Indexed) == gcSL_NOT_INDEXED &&
                gcmSL_SOURCE_GET(userCode->instruction.source1, Indexed) == gcSL_NOT_INDEXED)
            {
                /* Get source operands. */
                source1 = userCode->instruction.source1;

                /* Test if source1 is the target of the MOV. */
                gcmASSERT(gcmSL_SOURCE_GET(source1, Type) == gcSL_TEMP &&
                          gcmSL_INDEX_GET(userCode->instruction.source1Index, Index) == gcmSL_INDEX_GET(code->instruction.tempIndex, Index) &&
                          gcmSL_INDEX_GET(userCode->instruction.source1Index, ConstValue) == gcmSL_INDEX_GET(code->instruction.tempIndex, ConstValue) );

                /* Change userCode's source0 to code's source0. */
                source1 = gcmSL_SOURCE_SET(source1, Type, gcmSL_SOURCE_GET(source, Type));
                source1 = gcmSL_SOURCE_SET(source1, Indexed, gcmSL_SOURCE_GET(source, Indexed));
                source1 = gcmSL_SOURCE_SET(source1, Neg, gcmSL_SOURCE_GET(source, Neg) ^ gcmSL_SOURCE_GET(source1, Neg));
                source1 = gcmSL_SOURCE_SET(source1, Abs, gcmSL_SOURCE_GET(source, Abs) | gcmSL_SOURCE_GET(source1, Abs));
                source1 = gcmSL_SOURCE_SET(source1, Precision, gcmSL_SOURCE_GET(source, Precision));
                source1 = gcmSL_SOURCE_SET(source1, SwizzleX, _GetSwizzle(gcmSL_SOURCE_GET(source1, SwizzleX), source));
                source1 = gcmSL_SOURCE_SET(source1, SwizzleY, _GetSwizzle(gcmSL_SOURCE_GET(source1, SwizzleY), source));
                source1 = gcmSL_SOURCE_SET(source1, SwizzleZ, _GetSwizzle(gcmSL_SOURCE_GET(source1, SwizzleZ), source));
                source1 = gcmSL_SOURCE_SET(source1, SwizzleW, _GetSwizzle(gcmSL_SOURCE_GET(source1, SwizzleW), source));

                userCode->instruction.source1        = source1;
                userCode->instruction.source1Index   = code->instruction.source0Index;
                userCode->instruction.source1Indexed = code->instruction.source0Indexed;

                /* Update data flow. */
                gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &userCode->dependencies1, code));

                if (code->dependencies0)
                {
                    gcmVERIFY_OK(gcOpt_AddListToList(Optimizer, code->dependencies0, gcvFALSE, &userCode->dependencies1));
                    for (ulist = code->dependencies0; ulist; ulist = ulist->next)
                    {
                        if (ulist->index >= 0)
                        {
                            gcmVERIFY_OK(gcOpt_AddCodeToList(Optimizer, &ulist->code->users, userCode));
                        }
                    }
                }
            }

            if (samplerIndexed)
            {
                /* Get new indexed. */
                gcSL_INDEXED indexed = _GetSwizzle(gcmSL_SOURCE_GET(userCode->instruction.source0, Indexed) - 1, source) + 1;

                /* Test if source0 is the target of the MOV. */
                gcmASSERT(gcmSL_SOURCE_GET(userCode->instruction.source0, Type) == gcSL_SAMPLER &&
                          userCode->instruction.source0Indexed == code->instruction.tempIndex);

                userCode->instruction.source0 = gcmSL_SOURCE_SET(userCode->instruction.source0, Indexed, indexed);
                userCode->instruction.source0Indexed = (gctUINT16)code->instruction.source0Index;
                /* Update data flow. */
                gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &userCode->dependencies0, code));

                if (code->dependencies0)
                {
                    gcmVERIFY_OK(gcOpt_AddListToList(Optimizer, code->dependencies0, gcvFALSE, &userCode->dependencies0));
                    for (ulist = code->dependencies0; ulist; ulist = ulist->next)
                    {
                        if (ulist->index >= 0)
                        {
                            gcmVERIFY_OK(gcOpt_AddCodeToList(Optimizer, &ulist->code->users, userCode));
                        }
                    }
                }
            }

            /* It is possible that both sources use the same temp register, so before we can */
            /* delete usage for this instruction against MOV instruction, we need assure that */
            /* all dependencies on two sources has been cleaned up with this MOV instruction */
            for (depList = userCode->dependencies0; depList; depList = depList->next)
            {
                if (depList->code == code)
                {
                    canRemoveUsage = gcvFALSE;
                    break;
                }
            }

            if (canRemoveUsage)
            {
                for (depList = userCode->dependencies1; depList; depList = depList->next)
                {
                    if (depList->code == code)
                    {
                        canRemoveUsage = gcvFALSE;
                        break;
                    }
                }
            }

            /* Yes, we can remove safely now */
            if (canRemoveUsage)
            {
                gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &code->users, userCode));
                usageRemoved = gcvTRUE;
            }
        }

        if (code->users == gcvNULL)
        {
            /* No more user for the MOV instruction, change it to NOP. */
            gcmVERIFY_OK(gcOpt_ChangeCodeToNOP(Optimizer, code));

        }
        else if (usageRemoved)
        {
            /* Try to diminish channel */

            gctUINT16 remainderEnable = 0;
            gctSOURCE_t source;
            gctBOOL canDiminishChannel = gcvTRUE;

            for (list = code->users; list; list = list->next)
            {
                gcOPT_CODE userCode;
                gcOPT_LIST depList;
                gctUINT16 thisEnable;

                /* Get the user code. */
                userCode = list->code;
                for (depList = userCode->dependencies0; depList; depList = depList->next)
                {
                    /* source0 has a usage from MOV instruction */
                    if (depList->code == code)
                    {
                        source = userCode->instruction.source0;

                        /* Since currently our DFA is not very accurate on channels, we only */
                        /* consider single-channel case conservationally */
                        thisEnable = gcSL_ConvertSwizzle2Enable((gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleX),
                                                                (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleY),
                                                                (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleZ),
                                                                (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleW));

                        if (!_IsSingleChannelEnabled(thisEnable))
                        {
                            canDiminishChannel = gcvFALSE;
                            break;
                        }
                        else
                        {
                            remainderEnable |= thisEnable;
                        }
                    }
                }

                for (depList = userCode->dependencies1; depList; depList = depList->next)
                {
                    source = userCode->instruction.source1;

                    /* source1 has a usage from MOV instruction */
                    if (depList->code == code)
                    {
                        source = userCode->instruction.source1;

                        /* Since currently our DFA is not very accurate on channels, we only */
                        /* consider single-channel case conservationally */
                        thisEnable = gcSL_ConvertSwizzle2Enable((gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleX),
                                                                (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleY),
                                                                (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleZ),
                                                                (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleW));

                        if (!_IsSingleChannelEnabled(thisEnable))
                        {
                            canDiminishChannel = gcvFALSE;
                            break;
                        }
                        else
                        {
                            remainderEnable |= thisEnable;
                        }
                    }
                }
            }

            remainderEnable &= gcmSL_TARGET_GET(code->instruction.temp, Enable);
            if (remainderEnable && canDiminishChannel)
            {
                /* Ok, we can remove other channels rather than channels enabled by remainderEnable */

                gctUINT16 disable = ~remainderEnable;
                gctUINT16 swizzle;

                code->instruction.temp = gcmSL_TARGET_SET(code->instruction.temp, Enable, remainderEnable);

                source = code->instruction.source0;

                if (remainderEnable & gcSL_ENABLE_X)
                    swizzle = gcmSL_SOURCE_GET(source, SwizzleX);
                else if (remainderEnable & gcSL_ENABLE_Y)
                    swizzle = gcmSL_SOURCE_GET(source, SwizzleY);
                else if (remainderEnable & gcSL_ENABLE_Z)
                    swizzle = gcmSL_SOURCE_GET(source, SwizzleZ);
                else
                    swizzle = gcmSL_SOURCE_GET(source, SwizzleW);

                source = (disable & gcSL_ENABLE_X) ? gcmSL_SOURCE_SET(source, SwizzleX, swizzle) : source;
                source = (disable & gcSL_ENABLE_Y) ? gcmSL_SOURCE_SET(source, SwizzleY, swizzle) : source;
                source = (disable & gcSL_ENABLE_Z) ? gcmSL_SOURCE_SET(source, SwizzleZ, swizzle) : source;
                source = (disable & gcSL_ENABLE_W) ? gcmSL_SOURCE_SET(source, SwizzleW, swizzle) : source;

                code->instruction.source0 = source;
            }
        }

        codeRemoved++;
    }

    if (codeRemoved > 0)
    {
        status = gcvSTATUS_CHANGED;
    }

    /* Pass 3: Check if a MOV instruction can be removed because it is the same
               as the previous MOV instruction. */
    codeRemoved = 0;
    for (code = Optimizer->codeTail; code; code = code->prev)
    {
        gcOPT_LIST nlist;

        /* Test for MOV instruction. */
        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_MOV) continue;

        /* Skip if there are callers. */
        if (code->callers) break;

        /* Skip if no prevDefines or more than one preDefines. */
        if (code->prevDefines == gcvNULL) continue;
        if (code->prevDefines->next) continue;

        /* Skip if prevDefine is not exact the same instruction. */
        depCode = code->prevDefines->code;
        if (depCode == gcvNULL) continue;
        if (gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode) != gcmSL_OPCODE_GET(code->instruction.opcode, Opcode)) continue;
        if (depCode->instruction.temp != code->instruction.temp) continue;
        if (depCode->instruction.tempIndex != code->instruction.tempIndex) continue;
        if (depCode->instruction.tempIndexed != code->instruction.tempIndexed) continue;
        if (depCode->instruction.source0 != code->instruction.source0) continue;
        if (depCode->instruction.source0Index != code->instruction.source0Index) continue;
        if (depCode->instruction.source0Indexed != code->instruction.source0Indexed) continue;

        /* Skip if there is any jump out/in in between. */
        for (iterCode = depCode->next; iterCode != code; iterCode = iterCode->next)
        {
            if (iterCode->callers) break;
            if (gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_JMP ||
                gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_CALL) break;
        }
        if (iterCode != code) continue;

        /* Skip if the dependencies are not the same. */
        if (depCode->dependencies0)
        {
            if (code->dependencies0 == gcvNULL) continue;

            for (list = depCode->dependencies0; list; list = list->next)
            {
                for (nlist = code->dependencies0; nlist; nlist = nlist->next)
                {
                    if (nlist->index != list->index) continue;
                    if (nlist->index >= 0 && nlist->code == list->code) break;
                }

                if (nlist == gcvNULL) break;
            }

            if (list) continue;

            for (list = code->dependencies0; list; list = list->next)
            {
                for (nlist = depCode->dependencies0; nlist; nlist = nlist->next)
                {
                    if (nlist->index != list->index) continue;
                    if (nlist->index >= 0 && nlist->code == list->code) break;
                }

                if (nlist == gcvNULL) break;
            }

            if (list) continue;
        }
        else if (code->dependencies0) continue;

        /* Remove code since it is redundant. */
        gcmVERIFY_OK(gcOpt_ChangeCodeToNOP(Optimizer, code));
        /*code->instruction = gcvSL_NOP_INSTR;*/
        codeRemoved++;
    }

    if (codeRemoved > 0)
    {
        status = gcvSTATUS_CHANGED;
    }

    if (!gcdHasOptimization(Optimizer->option, gcvOPTIMIZATION_MIN_COMP_TIME))
    {
        if (status == gcvSTATUS_CHANGED)
        {
            /* Rebuild flow graph. */
            gcmONERROR(gcOpt_RebuildFlowGraph(Optimizer));

            DUMP_OPTIMIZER("Optimize MOV instructions in the shader", Optimizer);
        }
    }

    gcmFOOTER();
    return status;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gctBOOL
_IsCodeUseThisAssignment(
    gctSOURCE_t Source,
    gctTARGET_t Target
    )
{
    gcSL_SWIZZLE swizzle[4];
    gcSL_ENABLE enable[4], targetEnable;
    static const gctUINT8 _enable[] =
    {
        gcSL_ENABLE_X,
        gcSL_ENABLE_Y,
        gcSL_ENABLE_Z,
        gcSL_ENABLE_W
    };
    gctINT i;

    swizzle[0] = gcmSL_SOURCE_GET(Source, SwizzleX);
    swizzle[1] = gcmSL_SOURCE_GET(Source, SwizzleY);
    swizzle[2] = gcmSL_SOURCE_GET(Source, SwizzleZ);
    swizzle[3] = gcmSL_SOURCE_GET(Source, SwizzleW);

    for (i = 0; i < 4; i++)
    {
        enable[i] = _enable[swizzle[i]];
    }

    targetEnable = gcmSL_TARGET_GET(Target, Enable);

    for (i = 0; i < 4; i++)
    {
        if (targetEnable & enable[i])
            return gcvTRUE;
    }

    return gcvFALSE;
}

gctBOOL isChannelOfEnableEqualSwizzle(
    IN gcOPT_CODE Code
    )
{
    gctUINT16 codeEnable;
    gctSOURCE_t source;
    gctUINT8  channels[4] = {0};

    if (gcmSL_OPCODE_GET(Code->instruction.opcode, Opcode) != gcSL_MOV)
        return gcvFALSE;

    codeEnable = gcmSL_TARGET_GET(Code->instruction.temp, Enable);
    source     = Code->instruction.source0;

    if (codeEnable & gcSL_ENABLE_X)
        channels[0]++;
    if (codeEnable & gcSL_ENABLE_Y)
        channels[1]++;
    if (codeEnable & gcSL_ENABLE_Z)
        channels[2]++;
    if (codeEnable & gcSL_ENABLE_W)
        channels[3]++;

    if (channels[gcmSL_SOURCE_GET(source, SwizzleX)] == 0)
        return gcvFALSE;
    if (channels[gcmSL_SOURCE_GET(source, SwizzleY)] == 0)
        return gcvFALSE;
    if (channels[gcmSL_SOURCE_GET(source, SwizzleZ)] == 0)
        return gcvFALSE;
    if (channels[gcmSL_SOURCE_GET(source, SwizzleW)] == 0)
        return gcvFALSE;

    return gcvTRUE;
}

static gctBOOL
_NoAssignmentBetweenDepAndUser(
    IN gcOPT_CODE   DepCode,
    IN gcOPT_CODE   UserCode
    )
{
    gcOPT_CODE      codeIter;

    /*
    ** This is just a rough check, no need to consider the back jump or cross-function usage.
    ** The only thing we need to make sure that the depCode and the userCode are in the same BB.
    */
    if (DepCode->function != UserCode->function || DepCode->id > UserCode->id)
    {
        return gcvFALSE;
    }

    for (codeIter = DepCode->next; codeIter && codeIter != UserCode; codeIter = codeIter->next)
    {
        gcSL_OPCODE opCode = (gcSL_OPCODE)gcmSL_OPCODE_GET(codeIter->instruction.opcode, Opcode);

        if (opCode == gcSL_JMP || opCode == gcSL_JMP_ANY || opCode == gcSL_CALL)
        {
            return gcvFALSE;
        }

        if (codeIter->callers != gcvNULL)
        {
            return gcvFALSE;
        }

        if (gcSL_isOpcodeHaveNoTarget(opCode))
        {
            continue;
        }

        if (codeIter->instruction.tempIndex == UserCode->instruction.tempIndex)
        {
            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

gceSTATUS
gcOpt_OptimizeConstantAssignment(
    IN gcOPTIMIZER Optimizer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcOPT_CODE code, code1, code2, codeIter, userCode, endCode;
    gcOPT_LIST depList, list;
    gctINT i, vectorCount, depCount;
    gctBOOL changed = gcvFALSE;

    gcmHEADER_ARG("Optimizer=%p", Optimizer);

    if (Optimizer->tempCount == 0)
    {
        gcmFOOTER();
        return status;
    }

    {
        gctBOOL     useFullNewLinker = gcvFALSE;
        gctBOOL     hasHalti2 = gcHWCaps.hwFeatureFlags.hasHalti2;

        useFullNewLinker = gcUseFullNewLinker(hasHalti2);

        /* only disable it for vx shader for now. */
        if (useFullNewLinker && gcSHADER_GoVIRPass(Optimizer->shader) && gcShaderHasVivVxExtension(Optimizer->shader))
        {
            gcmFOOTER();
            return status;
        }
    }

    if (Optimizer->shader->codeCount > SHADER_TOO_MANY_CODE &&
        Optimizer->jmpCount > SHADER_TOO_MANY_JMP)
    {
        gcmFOOTER();
        return status;
    }

    for (code = Optimizer->codeHead; code; code = code->next)
    {
        gctBOOL hasJmp = gcvFALSE;

        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_MOV)
            continue;

        if (gcmSL_SOURCE_GET(code->instruction.source0, Type) != gcSL_CONSTANT)
            continue;

        if (gcmSL_TARGET_GET(code->instruction.temp, Indexed) != gcSL_NOT_INDEXED)
            continue;

        if ((code->users == gcvNULL) || (code->users && code->users->next != gcvNULL))
            continue;

        if (code->users->code == gcvNULL)
            continue;

        userCode = code->users->code;

        if (gcmSL_OPCODE_GET(userCode->instruction.opcode, Opcode) != gcSL_MOV ||
            gcmSL_TARGET_GET(userCode->instruction.temp, Indexed) != gcSL_NOT_INDEXED)
        {
            continue;
        }

        for (i = 1, codeIter = code->next; i < 4 && codeIter; i++, codeIter = codeIter->next)
        {
            if (gcmSL_OPCODE_GET(codeIter->instruction.opcode, Opcode) != gcSL_MOV)
                break;

            if (gcmSL_SOURCE_GET(codeIter->instruction.source0, Type) != gcSL_CONSTANT)
                break;

            if (gcmSL_TARGET_GET(codeIter->instruction.temp, Indexed) != gcSL_NOT_INDEXED)
                break;

            if (codeIter->instruction.tempIndex != code->instruction.tempIndex)
                break;

            if (codeIter->users == gcvNULL || (codeIter->users && codeIter->users->next))
                break;

            if (codeIter->users->code != userCode)
                break;
        }

        /* Not matched. */
        if (codeIter->prev == code)
            continue;

        endCode = codeIter->prev;
        vectorCount = i;

        depCount = 0;
        depList = userCode->dependencies0;
        while (depList)
        {
            depCount++;
            depList = depList->next;
        }

        /* User has multiple dep, skip it now. */
        if (depCount != vectorCount || !_NoAssignmentBetweenDepAndUser(endCode, userCode))
        {
            code = endCode;
            continue;
        }

        if (userCode->id > endCode->id)
        {
            code1 = endCode;
            code2 = userCode;
        }
        else
        {
            code1 = userCode;
            code2 = endCode;
        }

        for (codeIter = code1; codeIter != code2->next; codeIter = codeIter->next)
        {
            if (codeIter->callee != gcvNULL)
            {
                hasJmp = gcvTRUE;
                break;
            }

            if (codeIter->callers != gcvNULL)
            {
                hasJmp = gcvTRUE;
                break;
            }
        }

        if (hasJmp)
            continue;

        /* It is a simple MOV, just replace it. */
        if (_isSimpleMOV(userCode) &&
            isChannelOfEnableEqualSwizzle(userCode))
        {
            for (i = 0, code1 = code; i < vectorCount; i++, code1 = code1->next)
            {
                /* Update data flow. */
                gcOpt_DeleteCodeFromList(Optimizer, &code1->users, userCode);
                gcOpt_DeleteCodeFromList(Optimizer, &userCode->dependencies0, code1);

                for (list = userCode->prevDefines; list; list = list->next)
                {
                    if (list->index < 0)
                    {
                        continue;
                    }
                    code2 = list->code;
                    if (gcmSL_TARGET_GET(code1->instruction.temp, Enable) &
                        gcmSL_TARGET_GET(code2->instruction.temp, Enable))
                    {
                        gcOpt_AddCodeToList(Optimizer, &code1->prevDefines, code2);
                        gcOpt_AddCodeToList(Optimizer, &code2->nextDefines, code1);
                    }
                }

                for (list = userCode->nextDefines; list; list = list->next)
                {
                    if (list->index < 0)
                    {
                        continue;
                    }
                    code2 = list->code;
                    if (gcmSL_TARGET_GET(code1->instruction.temp, Enable) &
                        gcmSL_TARGET_GET(code2->instruction.temp, Enable))
                    {
                        gcOpt_AddCodeToList(Optimizer, &code1->nextDefines, code2);
                        gcOpt_AddCodeToList(Optimizer, &code2->prevDefines, code1);
                    }
                }

                for (list = userCode->users; list; list = list->next)
                {
                    gctINT source0 = 0, source1 = 0;

                    if (list->index < 0)
                    {
                        if (_IsCodeUseThisAssignment(userCode->instruction.source0, code1->instruction.temp))
                        {
                            gcOpt_AddIndexToList(Optimizer, &code1->users, list->index);
                        }

                        continue;
                    }

                    code2 = list->code;

                    if (gcOpt_FindCodeInList(Optimizer, code2->dependencies0, userCode) == gcvSTATUS_TRUE)
                    {
                        source0 = 1;
                        if (i == vectorCount - 1)
                        {
                            gcOpt_DeleteCodeFromList(Optimizer, &code2->dependencies0, userCode);
                        }
                    }

                    if (gcOpt_FindCodeInList(Optimizer, code2->dependencies1, userCode) == gcvSTATUS_TRUE)
                    {
                        source1 = 1;
                        if (i == vectorCount - 1)
                        {
                            gcOpt_DeleteCodeFromList(Optimizer, &code2->dependencies1, userCode);
                        }
                    }

                    /* Add the matched assignment to the user list. */
                    if (source0 &&
                        (gcmSL_OPCODE_GET(code2->instruction.opcode, Opcode) == gcSL_IMAGE_WR ||
                         gcmSL_OPCODE_GET(code2->instruction.opcode, Opcode) == gcSL_IMAGE_WR_3D ||
                         gcmSL_OPCODE_GET(code2->instruction.opcode, Opcode) == gcSL_CALL ||
                         _IsCodeUseThisAssignment(code2->instruction.source0, code1->instruction.temp)))
                    {
                        gcOpt_AddCodeToList(Optimizer, &code2->dependencies0, code1);
                        gcOpt_AddCodeToList(Optimizer, &code1->users, code2);
                    }

                    if (source1 &&
                        (gcmSL_OPCODE_GET(code2->instruction.opcode, Opcode) == gcSL_IMAGE_WR ||
                         gcmSL_OPCODE_GET(code2->instruction.opcode, Opcode) == gcSL_IMAGE_WR_3D ||
                         gcmSL_OPCODE_GET(code2->instruction.opcode, Opcode) == gcSL_CALL ||
                         _IsCodeUseThisAssignment(code2->instruction.source1, code1->instruction.temp)))
                    {
                        gcOpt_AddCodeToList(Optimizer, &code2->dependencies1, code1);
                        gcOpt_AddCodeToList(Optimizer, &code1->users, code2);
                    }

                    if (!source0 && !source1)   /* code2 may have no source operand */
                    {
                        gcOpt_AddCodeToList(Optimizer, &code1->users, code2);
                    }
                }

                /* Update code. */
                code1->instruction.tempIndex = userCode->instruction.tempIndex;
                code1->instruction.temp = gcmSL_TARGET_SET(code1->instruction.temp, Precision, gcmSL_TARGET_GET(userCode->instruction.temp, Precision));
            }

            gcmVERIFY_OK(gcOpt_ChangeCodeToNOP(Optimizer, userCode));
            changed = gcvTRUE;
        }

        code = endCode;
    }

    if (changed)
    {
        /* Code optimzed. */
        DUMP_OPTIMIZER("optimize constant assignment", Optimizer);
        gcmFOOTER_ARG("status=%d", gcvSTATUS_CHANGED);
        return gcvSTATUS_CHANGED;
    }
    gcmFOOTER();
    return status;
}

static gctBOOL
_IsTempModified(
    IN gcOPTIMIZER Optimizer,
    IN gctUINT Index,
    IN gcOPT_CODE Code1,
    IN gcOPT_CODE Code2
    )
{
    gcOPT_CODE code;

    for (code = Code1->next; code != gcvNULL && code != Code2; code = code->next)
    {
        gcSL_OPCODE opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->instruction.opcode, Opcode);

        switch (opcode)
        {
        case gcSL_CALL:
            /* Check global and output. */
            {
            /* Add dependencies for input and output arguments. */
            gcOPT_FUNCTION function = code->callee->function;
            gcsFUNCTION_ARGUMENT_PTR argument;
            gcOPT_GLOBAL_USAGE usage;
            gctUINT a;

            /* Add dependencies for all input arguments, and add users for all output arguments. */
            argument = function->arguments;
            for (a = 0; a < function->argumentCount; a++, argument++)
            {
                /* Check output arguments. */
                if (argument->qualifier != gcvFUNCTION_INPUT)
                {
                    if (function->arguments[a].index == Index) return gcvTRUE;
                }
            }

            /* Add global variable dependencies and users. */
            for (usage = function->globalUsage; usage; usage = usage->next)
            {
                if (usage->direction != gcvFUNCTION_INPUT)
                {
                    if (usage->index == Index) return gcvTRUE;
                }
            }
            }
            break;

        case gcSL_RET:
        case gcSL_JMP:
            /* Should not happen because it is checked before. */
            gcmASSERT(0);
            break;

        default:
            if (gcSL_isOpcodeHaveNoTarget(opcode))
            {
                break;
            }
            if (code->instruction.tempIndex == Index) return gcvTRUE;

            /* Since the value of indexed reg is unknown, just be cautious. */
            if (gcmSL_TARGET_GET(code->instruction.temp, Indexed) != gcSL_NOT_INDEXED) return gcvTRUE;

            break;
        }
    }

    return gcvFALSE;
}

static gctBOOL
_IsSourceModified(
    IN gcOPTIMIZER Optimizer,
    IN gctSOURCE_t Source,
    IN gctUINT SourceIndex,
    IN gctUINT SourceIndexed,
    IN gcOPT_LIST Dependencies,
    IN gcOPT_CODE Code1,
    IN gcOPT_CODE Code2
    )
{
    gcOPT_LIST list, dlist;
    gctBOOL needDetailedCheck = gcvFALSE;

    for (list = Dependencies; list; list = list->next)
    {
        if (list->index < 0)
        {
            needDetailedCheck = gcvTRUE;
            continue;
        }
        for (dlist = list->code->nextDefines; dlist; dlist = dlist->next)
        {
            if (dlist->index == 0)
            {
                gctUINT id = dlist->code->id;
                if (id > Code1->id && id < Code2->id) return gcvTRUE;
            }
        }
    }

    if (! needDetailedCheck) return gcvFALSE;

    if (gcmSL_SOURCE_GET(Source, Type) == gcSL_TEMP)
    {
        if (_IsTempModified(Optimizer, SourceIndex, Code1, Code2)) return gcvTRUE;
    }

    if (gcmSL_SOURCE_GET(Source, Indexed) == gcSL_NOT_INDEXED) return gcvFALSE;

    if (_IsTempModified(Optimizer, SourceIndexed, Code1, Code2)) return gcvTRUE;

    return gcvFALSE;
}

static gctBOOL
_IsADDForMADD(
    IN gcOPTIMIZER Optimizer,
    IN gcOPT_CODE Code,
    IN gctSOURCE_t Source,
    IN gcOPT_LIST Dependencies
    )
{
    gcOPT_CODE code, depCode;
    gctUINT16 enable;

    /* Check if the source is direct temp register. */
    if (gcmSL_SOURCE_GET(Source, Type) != gcSL_TEMP) return gcvFALSE;
    if (gcmSL_SOURCE_GET(Source, Indexed) != gcSL_NOT_INDEXED) return gcvFALSE;

    /* Check if only one dependency. */
    if (Dependencies == gcvNULL) return gcvFALSE;
    if (Dependencies->next != gcvNULL) return gcvFALSE;

    /* Check if the dependent code is MUL. */
    if (Dependencies->index < 0) return gcvFALSE;
    depCode = Dependencies->code;
    if (gcmSL_OPCODE_GET(depCode->instruction.opcode, Opcode) != gcSL_MUL) return gcvFALSE;
    if (gcmSL_TARGET_GET(depCode->instruction.temp, Indexed) != gcSL_NOT_INDEXED) return gcvFALSE;

    /* Check if the enables are the same. */
    enable = gcmSL_TARGET_GET(Code->instruction.temp, Enable);
    if (gcmSL_TARGET_GET(depCode->instruction.temp, Enable) != enable) return gcvFALSE;

    /* Check if depCode has only one user, code. */
    gcmASSERT(depCode->users != gcvNULL);
    if (depCode->users == gcvNULL)  return gcvFALSE;
    if (depCode->users->next != gcvNULL)  return gcvFALSE;
    gcmASSERT(depCode->users->code == Code);

    /* Check if the swizzle matches the enable. */
    if ((enable & gcSL_ENABLE_X) && gcmSL_SOURCE_GET(Source, SwizzleX) != gcSL_SWIZZLE_X) return gcvFALSE;
    if ((enable & gcSL_ENABLE_Y) && gcmSL_SOURCE_GET(Source, SwizzleY) != gcSL_SWIZZLE_Y) return gcvFALSE;
    if ((enable & gcSL_ENABLE_Z) && gcmSL_SOURCE_GET(Source, SwizzleZ) != gcSL_SWIZZLE_Z) return gcvFALSE;
    if ((enable & gcSL_ENABLE_W) && gcmSL_SOURCE_GET(Source, SwizzleW) != gcSL_SWIZZLE_W) return gcvFALSE;

    /* Check if any branch-out instruction or jump-in point between depPc and Pc. */
    /* This should not happen, but be cautious. */
    gcmASSERT(depCode->id < Code->id);
    if (depCode->callers != gcvNULL) return gcvFALSE;
    for (code = depCode->next;code != gcvNULL && code != Code; code = code->next)
    {
        if (code->callers != gcvNULL) return gcvFALSE;
        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_JMP &&
            (code->callee->id < depCode->id || code->callee->id > Code->id)) return gcvFALSE;
        /* Not likely, but be cautious. */
        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_RET) return gcvFALSE;
    }

    /* Check if MUL instruction can be moved to right before the ADD instruction. */
    /* Check if source0 may be modified between depPc and Pc. */
    if (depCode->dependencies0 != gcvNULL &&
        _IsSourceModified(Optimizer, depCode->instruction.source0, depCode->instruction.source0Index,
                          depCode->instruction.source0Indexed, depCode->dependencies0, depCode, Code)) return gcvFALSE;

    /* Check if source1 may be modified between depPc and Pc. */
    if (depCode->dependencies1 != gcvNULL &&
        _IsSourceModified(Optimizer, depCode->instruction.source1, depCode->instruction.source1Index,
                          depCode->instruction.source1Indexed, depCode->dependencies1, depCode, Code)) return gcvFALSE;

    /* A special ugly WAR for dual16 to make dual16 pattern unbroken, it should be removed after
       dual16 enhancement later. Now put here for GFX30 perf tuning. */
#if (GC_ENABLE_DUAL_FP16 > 0)
    if (gcmSL_TARGET_GET(depCode->instruction.temp, Precision) == gcSL_PRECISION_MEDIUM &&
        gcmSL_SOURCE_GET(depCode->instruction.source0, Type) == gcSL_UNIFORM &&
        gcmSL_SOURCE_GET(depCode->instruction.source0, Precision) == gcSL_PRECISION_HIGH &&
        gcmSL_SOURCE_GET(depCode->instruction.source1, Type) == gcSL_TEMP &&
        gcmSL_SOURCE_GET(depCode->instruction.source1, Precision) == gcSL_PRECISION_HIGH)
    {
        gcOPT_CODE rcpDepCode;

        gcmASSERT(depCode->dependencies1);
        rcpDepCode = depCode->dependencies1->code;

        /* High precision for RCP need more care */
        if (gcmSL_OPCODE_GET(rcpDepCode->instruction.opcode, Opcode) == gcSL_RCP &&
            gcmSL_TARGET_GET(rcpDepCode->instruction.temp, Precision) == gcSL_PRECISION_HIGH &&
            gcmSL_SOURCE_GET(rcpDepCode->instruction.source0, Precision) == gcSL_PRECISION_HIGH)
        {
            return gcvFALSE;
        }
    }
#endif

    return gcvTRUE;
}

/*******************************************************************************
**                          gcOpt_OptimizeMULADDInstructions
********************************************************************************
**
**  Optimize MUL and ADD Instructions for MADD instructions.
**      Move MUL and ADD instructions with MADD instruction pattern to be together
**      for backend linker to convert them into MADD instructions.
**
**  INPUT:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_OptimizeMULADDInstructions(
    IN gcOPTIMIZER Optimizer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcOPT_CODE code, codeMUL, iterCode;
    gctUINT codeMoved;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    /* Check if one of a ADD instruction's dependency is only one MUL instruction,
       and the MUL's target's enable is the same as the ADD's dependency's enable.
       Then, move the MUL instruction to right before ADD or move ADD instruction
       to right after MUL instruction. */
    do
    {
        codeMoved = 0;
        for (code = Optimizer->codeTail; code; code = code->prev)
        {
            /* Check for ADD/SUB instruction. */
            if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_ADD &&
                gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_SUB) continue;

            /* Skip if ADD/SUB instruction is a target of jump. */
            if (code->callers)
                continue;

            /* Check if the ADD instruction can be combined with MUL to MADD instruction. */
            if (_IsADDForMADD(Optimizer, code, code->instruction.source0, code->dependencies0))
            {
                /* Check if MUL is right before ADD. */
                if (code->dependencies0->code == code->prev)
                    continue;

                /* Check if dependencies1 is also MUL and right before ADD. */
                if (_IsADDForMADD(Optimizer, code, code->instruction.source1, code->dependencies1))
                {
                    /* Check if MUL is right before ADD. */
                    if (code->dependencies1->code == code->prev)
                        continue;
                }

                /* Check if any jump in/out. */
                for (iterCode = code->dependencies0->code; iterCode != code; iterCode = iterCode->next)
                {
                    /* Stop if any jump in/out. */
                    if (iterCode->callers) break;
                    if (gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_JMP ||
                        gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_CALL) break;
                }

                if (iterCode != code)
                    continue;

                /* Find one candidate. */
                codeMUL = code->dependencies0->code;
                gcOpt_MoveCodeListBefore(Optimizer, codeMUL, codeMUL, code);
                codeMoved++;
            }
            else if (_IsADDForMADD(Optimizer, code, code->instruction.source1, code->dependencies1))
            {
                /* Check if MUL is right before ADD. */
                if (code->dependencies1->code == code->prev)
                    continue;

                /* Check if any jump in/out. */
                for (iterCode = code->dependencies1->code; iterCode != code; iterCode = iterCode->next)
                {
                    /* Stop if any jump in/out. */
                    if (iterCode->callers) break;
                    if (gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_JMP ||
                        gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_CALL) break;
                }

                if (iterCode != code)
                    continue;

                /* Find one candidate. */
                codeMUL = code->dependencies1->code;
                gcOpt_MoveCodeListBefore(Optimizer, codeMUL, codeMUL, code);
                codeMoved++;
            }
        }

        if (codeMoved > 0)
        {
            status = gcvSTATUS_CHANGED;
        }
    }
    while (codeMoved > 0);

    if (status == gcvSTATUS_CHANGED)
    {
        DUMP_OPTIMIZER("Optimize MUL and ADD for MADD instructions in the shader", Optimizer);
        gcmFOOTER();
        return status;
    }

    gcmFOOTER();
    return status;
}

typedef union _IndexValue
{
    gctINT       ival;
    gctUINT      uval;
    gctFLOAT     fval;
} _IndexValue;

typedef enum RerollExpKind
{
    REK_None,
    REK_SelfReduction, /* temp = temp op var[index] */
    REK_DescendingReduction, /* temp_n = temp_n-1 op var[index] */
    REK_NoDependcyAssignment   /* temp1[index] = var1[index] op var2[index] */
} RerollExpKind;


typedef struct RerollLocInfo
{
    gctBOOL           useTemp      : 2;  /* src/result in temp register */
    gctBOOL           useLoadStore : 2;  /* src/result need to use load/store */

    gctUINT           constValue : 2;
    gctUINT           index      : 14;
    gctUINT           indexed    : 16;
} RerollSourceInfo;

#define REROLL_MAX_PATTERN_LENGTH 10
typedef struct LoopRerollInfo
{
    /* loop info */
    gcSL_FORMAT        indexFormat;
    _IndexValue        startIndex;
    _IndexValue        endIndex;
    _IndexValue        stride;

    /* loop body info:
     *    1.  temp = temp op  var[index];
     *    2.  temp_1 = temp_0 op var[index_1];
     *        temp_2 = temp_1 op var[index_2];
     *         ...
     *        temp_n = temp_n-1 op var[index_n];
     *    3.  temp1[index] = var1[index] op var2[index];
     */
    RerollExpKind     expKind;
    gcSL_OPCODE       opCode;
    /* pattern code info */
    gctINT            patternLength;
    gcOPT_CODE        patterns[REROLL_MAX_PATTERN_LENGTH];
    struct _gcSL_INSTRUCTION    patternsInst[REROLL_MAX_PATTERN_LENGTH];
    gcOPT_CODE        lastLoadCode;
    gctUINT           temp0Index;
    gcOPT_CODE        temp0Code;
    gctUINT           lastReductionTempIndex;
    gcOPT_CODE        assignLastTempCode;
    gctINT            loopIndex;
    gctINT            iterations;
} LoopRerollInfo;

#define REROLL_CODE_COUNT            256
#define REROLL_ITERATION_THRESHOLD   20

static void
_resetRerollInfo(
    IN LoopRerollInfo * Info
    )
{
    gcoOS_ZeroMemory((gctPOINTER)Info, sizeof(LoopRerollInfo));
}

static gctBOOL
_getLoadStrideInfo(
    IN  gcOPT_CODE       Load,
    IN  gcOPT_CODE       NextLoad,
    OUT gcSL_FORMAT *    Format,
    OUT _IndexValue *    Stride
    )
{
    _IndexValue   offset1, offset2;
    gcmASSERT(gcmSL_OPCODE_GET(Load->instruction.opcode, Opcode) == gcSL_LOAD &&
              gcmSL_OPCODE_GET(NextLoad->instruction.opcode, Opcode) == gcSL_LOAD);

    /* get offset from first Load */
    if (gcmSL_SOURCE_GET(Load->instruction.source1, Type) != gcSL_CONSTANT)
        return gcvFALSE;
    offset1.uval  =
           (Load->instruction.source1Index & 0xFFFF) |
           (Load->instruction.source1Indexed << 16);

    /* get offset from second Load */
    if (gcmSL_SOURCE_GET(NextLoad->instruction.source1, Type) != gcSL_CONSTANT)
        return gcvFALSE;
    offset2.uval  =
           (NextLoad->instruction.source1Index & 0xFFFF) |
           (NextLoad->instruction.source1Indexed << 16);

    /* compute stride */
    *Format = gcmSL_SOURCE_GET(NextLoad->instruction.source1, Format);
    switch (*Format)
    {
    case gcSL_FLOAT:
        Stride->fval = offset2.fval - offset1.fval;
        break;
    case gcSL_INTEGER:
        Stride->ival = offset2.ival - offset1.ival;
        break;
    case gcSL_UINT32:
        Stride->uval = offset2.uval - offset1.uval;
        break;
    default:
        return gcvFALSE;
    }
    return gcvTRUE;
}

static gctBOOL
_findRerollPattern(
    IN      gcOPTIMIZER      Optimizer,
    IN      gcOPT_CODE       Code,
    IN OUT  LoopRerollInfo * Info,
    OUT     gcOPT_CODE     * NextPatternCode
    )
{
    gcOPT_CODE curCode;

    if (gcmSL_OPCODE_GET(Code->instruction.opcode, Opcode) == gcSL_LOAD)
    {
        gctUINT32    loadResultTemp;
        gctINT       patternLength = 0;
        /* 1. legality check for load instruction */
        /*    now only handle case that offset is constant */
        if (gcmSL_SOURCE_GET(Code->instruction.source1, Type) != gcSL_CONSTANT)
            return gcvFALSE;

        /*    target should not be indexed */
        if (gcmSL_TARGET_GET(Code->instruction.temp, Indexed) != gcSL_NOT_INDEXED)
            return gcvFALSE;

        /*    load result should has only one user */
        if (Code->users == gcvNULL || Code->users->next != gcvNULL)
            return gcvFALSE;

        /*    no redef for the result, and no dependency on source,
              no jump to the code */
        if (Code->prevDefines != gcvNULL ||
            Code->nextDefines != gcvNULL ||
            Code->dependencies0 != gcvNULL ||
            Code->dependencies1 != gcvNULL ||
            Code->callers       != gcvNULL ||
            gcmSL_INDEX_GET(Code->instruction.tempIndex, ConstValue) != 0)
            return gcvFALSE;

        /* record pattern */
        Info->patterns[patternLength++] = Code;

        loadResultTemp = gcmSL_INDEX_GET(Code->instruction.tempIndex, Index);
        curCode = Code->next;
        if (curCode == gcvNULL) return gcvFALSE;

        /* handle if the load result is moved to another temp */
        if (gcmSL_OPCODE_GET(curCode->instruction.opcode, Opcode) == gcSL_MOV &&
            Code->users->index == (gctINT)curCode->id &&
            curCode->users != gcvNULL &&
            curCode->users->next == gcvNULL &&
            curCode->callers == gcvNULL         )
        {
            gcmASSERT(gcmSL_INDEX_GET(Code->instruction.tempIndex, Index) == gcmSL_INDEX_GET(curCode->instruction.source0Index, Index) &&
                      gcmSL_INDEX_GET(Code->instruction.tempIndex, ConstValue) == gcmSL_INDEX_GET(curCode->instruction.source0Index, ConstValue) );

            loadResultTemp = gcmSL_INDEX_GET(curCode->instruction.tempIndex, Index);
            /* record pattern */
            Info->patterns[patternLength++] = curCode;
            curCode = curCode->next;
            if (curCode == gcvNULL) return gcvFALSE;
        }

        /* handle the op part */
        if (curCode->callers != gcvNULL || /* no jump to it */
            (gcmSL_SOURCE_GET(curCode->instruction.source0, Type) == gcSL_TEMP &&
             gcmSL_INDEX_GET(curCode->instruction.source0Index, Index) == loadResultTemp) ||
            (gcmSL_SOURCE_GET(curCode->instruction.source1, Type) == gcSL_TEMP &&
             gcmSL_INDEX_GET(curCode->instruction.source1Index, Index) == loadResultTemp                  )
           )
        {
            gctINT        srcNo;         /* source using load result */
            gctSOURCE_t   otherSrc;      /* the other source */

            /* record pattern */
            Info->patterns[patternLength++] = curCode;

            /* fill Loop reroll info */
            Info->opCode           = curCode->instruction.opcode;
            Info->indexFormat      = gcmSL_SOURCE_GET(Code->instruction.source1, Format);
            Info->startIndex.uval  = Info->endIndex.uval =
                   (Code->instruction.source1Index & 0xFFFF) |
                   (Code->instruction.source1Indexed << 16);
            /* source has load target */
            srcNo = (gcmSL_SOURCE_GET(curCode->instruction.source0, Type) == gcSL_TEMP &&
                     gcmSL_INDEX_GET(curCode->instruction.source0Index, Index) == loadResultTemp) ? 0 : 1;

            otherSrc =  (srcNo == 0) ? curCode->instruction.source1
                                     : curCode->instruction.source0;

            /* the other source */
            if (gcmSL_SOURCE_GET(otherSrc, Type) != gcSL_NONE)
            {
                Info->temp0Index =
                    (srcNo == 0) ? gcmSL_INDEX_GET(curCode->instruction.source1Index, Index)
                                 : gcmSL_INDEX_GET(curCode->instruction.source0Index, Index);
                Info->temp0Code  = curCode;
            }
            curCode = curCode->next;
            if (curCode == gcvNULL) return gcvFALSE;

            if (gcmSL_OPCODE_GET(curCode->instruction.opcode, Opcode) == gcSL_LOAD)
            {
                Info->assignLastTempCode     = curCode->prev;
                Info->lastReductionTempIndex = curCode->prev->instruction.tempIndex;
                if ((patternLength == 2 &&
                     curCode->next != gcvNULL &&
                     gcmSL_OPCODE_GET(curCode->next->instruction.opcode, Opcode) == Info->opCode) ||
                    (patternLength == 3 &&
                     curCode->next != gcvNULL && curCode->next->next != gcvNULL &&
                     gcmSL_OPCODE_GET(curCode->next->next->instruction.opcode, Opcode) == Info->opCode))
                {
                    gcSL_FORMAT  format;
                    _IndexValue  stride;
                    gctINT       i;

                    /* we found LOAD-OP or LOAD-MOV-LOAD pattern */

                    /* find the stride info */
                    if (_getLoadStrideInfo(Code, curCode, &format, &stride) == gcvFALSE)
                        return gcvFALSE;

                    gcmASSERT(format == Info->indexFormat);
                    Info->stride = stride;
                    if (curCode->next->instruction.tempIndex != Info->lastReductionTempIndex)
                    {
                        Info->expKind = REK_DescendingReduction;
                    }
                    else
                    {
                        Info->expKind = REK_SelfReduction;
                    }

                    Info->patternLength = patternLength;
                    Info->lastLoadCode = Code;
                    Info->iterations   = 1;     /* first iteration */

                    /* copy the pattern instructions */
                    for (i = 0; i < patternLength; i++)
                        Info->patternsInst[i] = Info->patterns[i]->instruction;

                    /* set the next pattern start code */
                    *NextPatternCode = curCode;
                    return gcvTRUE;
                }
            }
        }
    }

    return gcvFALSE;
}

static gctBOOL
_findNextRerollPattern(
    IN      gcOPTIMIZER      Optimizer,
    IN      gcOPT_CODE       Code,
    IN OUT  LoopRerollInfo * Info,
    OUT     gcOPT_CODE     * NextPatternCode
    )
{
    gctINT     i;
    gcOPT_CODE curCode = Code;
    /* check if the next few code match the existing pattern */
    for (i=0; i < Info->patternLength; i++)
    {
        if (gcmSL_OPCODE_GET(curCode->instruction.opcode, Opcode) !=
            gcmSL_OPCODE_GET(Info->patterns[i]->instruction.opcode, Opcode) )
        {
            /* not matching the existing pattern */
            return gcvFALSE;
        }
        curCode = curCode->next;
    }

    if (gcmSL_OPCODE_GET(Code->instruction.opcode, Opcode) == gcSL_LOAD)
    {
        gctUINT      loadResultTemp;
        gctINT       patternLength = 0;

        /* 1. legality check for load instruction */
        /*    now only handle case that offset is constant */
        if (gcmSL_SOURCE_GET(Code->instruction.source1, Type) != gcSL_CONSTANT)
            return gcvFALSE;

        /*    target should not be indexed */
        if (gcmSL_TARGET_GET(Code->instruction.temp, Indexed) != gcSL_NOT_INDEXED)
            return gcvFALSE;

        /*    load result should has only one user */
        if (Code->users == gcvNULL || Code->users->next != gcvNULL)
            return gcvFALSE;

        /*    no redef for the result, and no dependency on source,
              no jump to the code */
        if (Code->prevDefines != gcvNULL ||
            Code->nextDefines != gcvNULL ||
            Code->dependencies0 != gcvNULL ||
            Code->dependencies1 != gcvNULL ||
            Code->callers       != gcvNULL||
            gcmSL_INDEX_GET(Code->instruction.tempIndex, ConstValue) != 0)
            return gcvFALSE;

        /* goto next code*/
        patternLength++;

        loadResultTemp = Code->instruction.tempIndex;
        curCode = Code->next;
        if (curCode == gcvNULL) return gcvFALSE;

        /* handle if the load result is moved to another temp */
        if (gcmSL_OPCODE_GET(curCode->instruction.opcode, Opcode) == gcSL_MOV &&
            Code->users->index == (gctINT)curCode->id &&
            curCode->users != gcvNULL &&
            curCode->users->next == gcvNULL &&
            curCode->callers == gcvNULL         )
        {
            gcmASSERT(Code->instruction.tempIndex == curCode->instruction.source0Index);
            loadResultTemp = curCode->instruction.tempIndex;
            /* goto next code*/
            patternLength++;
            curCode = curCode->next;
            if (curCode == gcvNULL) return gcvFALSE;
        }

        /* handle the op part */
        if (curCode->callers != gcvNULL || /* no jump to it */
            (gcmSL_SOURCE_GET(curCode->instruction.source0, Type) == gcSL_TEMP &&
             gcmSL_INDEX_GET(curCode->instruction.source0Index, Index) == loadResultTemp ) ||
            (gcmSL_SOURCE_GET(curCode->instruction.source1, Type) == gcSL_TEMP &&
             gcmSL_INDEX_GET(curCode->instruction.source1Index, Index) == loadResultTemp )
           )
        {
            gcSL_FORMAT  format;
            _IndexValue  stride;

            /* increase pattern length */
            patternLength++;

            /* check  Loop reroll info */
            gcmASSERT(Info->opCode  == curCode->instruction.opcode);
            gcmASSERT(Info->indexFormat == (gcSL_FORMAT)gcmSL_SOURCE_GET(Code->instruction.source1, Format));

            /* check the stride info */
            if (_getLoadStrideInfo(Info->lastLoadCode, Code, &format, &stride) == gcvFALSE)
                return gcvFALSE;

            if (format != Info->indexFormat || Info->stride.ival != stride.ival)
                return gcvFALSE;

            /* stride type and value are equal, now check reduction kind */
            if (curCode->next->instruction.tempIndex == Info->lastReductionTempIndex &&
                Info->expKind != REK_SelfReduction)
            {
                /* use same temp index, it should be self reduction */
                return gcvFALSE;
            }
            else if (curCode->next->instruction.tempIndex != Info->lastReductionTempIndex &&
                     Info->expKind != REK_DescendingReduction)
            {
                /* use differenct temp index, it should be desending reduction */
                return gcvFALSE;
            }

            /* now we are sure the last few code matchses existing pattern,
             * need to update pattern info */
            Info->endIndex.uval =
                   (Code->instruction.source1Index & 0xFFFF) |
                   (Code->instruction.source1Indexed << 16);
            Info->assignLastTempCode     = curCode;
            Info->lastReductionTempIndex = curCode->instruction.tempIndex;
            Info->iterations++;     /* increase iteration */

            Info->lastLoadCode = Code;

            gcmASSERT(Info->patternLength == patternLength);
            /* set the next pattern start code */
            *NextPatternCode = curCode->next;
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static gcOPT_CODE
_findLoopStart(
    IN    gcOPTIMIZER      Optimizer,
    IN    LoopRerollInfo * Info
    )
{
    gcOPT_CODE loopStart;
    if (Info->patternLength == 1)
    {
        loopStart = Info->patterns[0]->next->next;
        gcmASSERT(loopStart != gcvNULL);
    }
    else
    {
        gcmASSERT(Info->patternLength > 1);
        /* start with second pattern */
        loopStart = Info->patterns[Info->patternLength - 1]->next;
    }

    gcmASSERT(gcmSL_OPCODE_GET(loopStart->instruction.opcode, Opcode) ==
              gcmSL_OPCODE_GET(Info->patternsInst->opcode, Opcode));

    return loopStart;
}

static gcOPT_CODE
_findLoopGetResult(
    IN    gcOPTIMIZER      Optimizer,
    IN    LoopRerollInfo * Info,
    IN    gcOPT_CODE       LoopStartCode
    )
{
    gcOPT_CODE code = LoopStartCode;
    gctINT     i;

    /* the last instruction of the loop must generate the result */
    for (i=0; i < Info->patternLength-1; i++)
    {
        code = code->next;
        gcmASSERT(code != gcvNULL);
    }

    return code;
}

static gcOPT_CODE
_createInitTemp(
    IN OUT  gcOPTIMIZER      Optimizer,
    IN      LoopRerollInfo * Info
    )
{
    gcOPT_CODE        code      = Info->patterns[0];  /* use code as init temp */
    gcSL_INSTRUCTION  inst      = &code->instruction;
    gcSL_INSTRUCTION  temp0Inst;  /* instruction constains the first temp */
    gctINT            temp0SrcNo; /* the source no for the temp0 */
    gcmASSERT(Info->assignLastTempCode != gcvNULL);  /* only handle this case now .*/

    if (Info->temp0Code == gcvNULL)
        return gcvNULL;

    /* copy target info */
    inst->opcode      = gcSL_MOV;
    inst->temp        = Info->assignLastTempCode->instruction.temp;
    inst->tempIndex   = Info->assignLastTempCode->instruction.tempIndex;
    inst->tempIndexed = Info->assignLastTempCode->instruction.tempIndexed;

    temp0Inst   = &Info->temp0Code->instruction;
    temp0SrcNo  = (gcmSL_SOURCE_GET(temp0Inst->source0, Type) == gcSL_TEMP &&
                   temp0Inst->source0Index == Info->temp0Index) ? 0 : 1;

    /* source 0: */
    inst->source0        = (temp0SrcNo == 0) ? temp0Inst->source0
                                             : temp0Inst->source1;
    inst->source0Index   = (temp0SrcNo == 0) ? temp0Inst->source0Index
                                             : temp0Inst->source1Index;
    inst->source0Indexed = (temp0SrcNo == 0) ? temp0Inst->source0Indexed
                                             : temp0Inst->source1Indexed;

    /* clean up source 1 */
    inst->source1        = gcmSL_SOURCE_SET(0, Type, gcSL_NONE);
    inst->source1Index   = 0;
    inst->source1Indexed = 0;

    /* clean up data flow */
    gcOpt_DestroyCodeFlowData(Optimizer, code);

    return code;
}

static gcOPT_CODE
_createInitLoopIndex(
    IN OUT  gcOPTIMIZER      Optimizer,
    IN      LoopRerollInfo * Info
    )
{
    gcOPT_CODE        code      = Info->patterns[0]->next;  /* use code as init loop index */
    gcSL_INSTRUCTION  inst      = &code->instruction;
    gcSL_INSTRUCTION  loadInst  = &Info->patternsInst[0];    /* load instruction */
    gcmASSERT(Info->assignLastTempCode != gcvNULL);  /* only handle this case now .*/

    if (Info->temp0Code == gcvNULL)
        return gcvNULL;

    /* copy target info */
    inst->opcode      = gcSL_MOV;
    inst->temp        = gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                      | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                      | gcmSL_TARGET_SET(0, Condition, gcSL_ALWAYS)
                      | gcmSL_TARGET_SET(0, Format, Info->indexFormat);

    Info->loopIndex   = inst->tempIndex   = loadInst->tempIndex;
    inst->tempIndexed = 0;

    /* source 0: */
    inst->source0        = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                         | gcmSL_SOURCE_SET(0, Indexed,gcSL_NOT_INDEXED)
                         | gcmSL_SOURCE_SET(0, Format, Info->indexFormat)
                         | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX);

    inst->source0Index   =  (Info->startIndex.uval & 0xFFFF);
    inst->source0Indexed =  (Info->startIndex.uval >> 16) & 0xFFFF;

    /* clean up source 1 */
    inst->source1        = gcmSL_SOURCE_SET(0, Type, gcSL_NONE);
    inst->source1Index   = 0;
    inst->source1Indexed = 0;

    /* clean up data flow */
    gcOpt_DestroyCodeFlowData(Optimizer, code);

    return code;
}

/* move the result generated by GenResultCode to lastReductionTempIndex */
static gcOPT_CODE
_createMovResultToTemp(
    IN OUT  gcOPTIMIZER      Optimizer,
    IN      LoopRerollInfo * Info,
    IN      gcOPT_CODE       GenResultCode
    )
{

    gcOPT_CODE        code      = GenResultCode->next;  /* use the code as move temp */
    gcSL_INSTRUCTION  inst      = &code->instruction;
    /* instruction constains the intermediate temp */
    gcSL_INSTRUCTION  intmTempInst = &GenResultCode->instruction;
    gcSL_SWIZZLE      swizzle;

    gcmASSERT(Info->assignLastTempCode != gcvNULL);  /* only handle this case now .*/

    /* copy target info */
    inst->opcode      = gcSL_MOV;
    inst->temp        = Info->assignLastTempCode->instruction.temp;
    inst->tempIndex   = Info->assignLastTempCode->instruction.tempIndex;
    inst->tempIndexed = Info->assignLastTempCode->instruction.tempIndexed;

    gcmASSERT(gcmSL_TARGET_GET(inst->temp, Format) ==
                  gcmSL_TARGET_GET(intmTempInst->temp, Format));

    swizzle = _ConvertEnable2Swizzle(gcmSL_TARGET_GET(inst->temp, Enable));

    /* source 0: */
    inst->source0        = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
                         | gcmSL_SOURCE_SET(0, Indexed,gcSL_NOT_INDEXED)
                         | gcmSL_SOURCE_SET(0, Format,
                                 gcmSL_TARGET_GET(inst->temp, Format) )
                         | gcmSL_SOURCE_SET(0, Swizzle, swizzle);

    inst->source0Index   = intmTempInst->tempIndex;
    inst->source0Indexed = 0;

    /* clean up source 1 */
    inst->source1        = gcmSL_SOURCE_SET(0, Type, gcSL_NONE);
    inst->source1Index   = 0;
    inst->source1Indexed = 0;

    /* clean up data flow */
    gcOpt_DestroyCodeFlowData(Optimizer, code);

    return code;
}

/* ADD temp1, index, stride */
static gcOPT_CODE
_createIncreaseLoopIndex(
    IN OUT  gcOPTIMIZER      Optimizer,
    IN      LoopRerollInfo * Info,
    IN      gcOPT_CODE       PrevCode
    )
{
    gcOPT_CODE        code      = PrevCode->next;  /* use code as init loop index */
    gcSL_INSTRUCTION  inst      = &code->instruction;
    gcmASSERT(Info->assignLastTempCode != gcvNULL);  /* only handle this case now .*/

    if (Info->temp0Code == gcvNULL)
        return gcvNULL;

    /* copy target info */
    inst->opcode      = gcSL_ADD;
    inst->temp        = gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                      | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                      | gcmSL_TARGET_SET(0, Condition, gcSL_ALWAYS)
                      | gcmSL_TARGET_SET(0, Format, Info->indexFormat);

    inst->tempIndex   = (Optimizer->tempCount+1);  /* use the next temp register */
    inst->tempIndexed = 0;

    /* source 0: */
    inst->source0        = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
                         | gcmSL_SOURCE_SET(0, Indexed,gcSL_NOT_INDEXED)
                         | gcmSL_SOURCE_SET(0, Format, Info->indexFormat)
                         | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX);
    inst->source0Index   = Info->loopIndex;
    inst->source0Indexed = 0;

    /* source 1: stride */
    inst->source1        = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                         | gcmSL_SOURCE_SET(0, Indexed,gcSL_NOT_INDEXED)
                         | gcmSL_SOURCE_SET(0, Format, Info->indexFormat)
                         | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX);
    inst->source1Index   =  (Info->stride.uval & 0xFFFF);
    inst->source1Indexed =  (Info->stride.uval >> 16) & 0xFFFF;

    /* clean up data flow */
    gcOpt_DestroyCodeFlowData(Optimizer, code);

    return code;
}

static gcOPT_CODE
_createMovToLoopIndex(
    IN OUT  gcOPTIMIZER      Optimizer,
    IN      LoopRerollInfo * Info,
    IN      gcOPT_CODE       PrevCode
    )
{

    gcOPT_CODE        code      = PrevCode->next;  /* use the code as move temp */
    gcSL_INSTRUCTION  inst      = &code->instruction;
    /* instruction constains the intermediate temp */
    gcSL_INSTRUCTION  intmTempInst = &PrevCode->instruction;

    gcmASSERT(Info->assignLastTempCode != gcvNULL);  /* only handle this case now .*/

    /* copy target info */
    inst->opcode      = gcSL_MOV;
    inst->temp        = gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                      | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                      | gcmSL_TARGET_SET(0, Condition, gcSL_ALWAYS)
                      | gcmSL_TARGET_SET(0, Format, Info->indexFormat);

    inst->tempIndex   = Info->loopIndex;
    inst->tempIndexed = 0;

    gcmASSERT((gctUINT32) Info->indexFormat == gcmSL_TARGET_GET(intmTempInst->temp, Format));

    /* source 0: */
    inst->source0        = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
                         | gcmSL_SOURCE_SET(0, Indexed,gcSL_NOT_INDEXED)
                         | gcmSL_SOURCE_SET(0, Format,
                                 gcmSL_TARGET_GET(inst->temp, Format) )
                         | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX);

    inst->source0Index   = intmTempInst->tempIndex;
    inst->source0Indexed = 0;

    /* clean up source 1 */
    inst->source1        = gcmSL_SOURCE_SET(0, Type, gcSL_NONE);
    inst->source1Index   = 0;
    inst->source1Indexed = 0;

    /* clean up data flow */
    gcOpt_DestroyCodeFlowData(Optimizer, code);

    return code;
}

static gcOPT_CODE
_createConditionJump(
    IN OUT  gcOPTIMIZER      Optimizer,
    IN      LoopRerollInfo * Info,
    IN      gcOPT_CODE       PrevCode,
    IN      gcOPT_CODE       LoopStartCode
    )
{

    gcOPT_CODE        code      = PrevCode->next;  /* use the code as move temp */
    gcSL_INSTRUCTION  inst      = &code->instruction;

    gcmASSERT(Info->assignLastTempCode != gcvNULL);  /* only handle this case now .*/

    /* copy target info */
    inst->opcode      = gcSL_JMP;
    inst->temp        = gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                      | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                      | gcmSL_TARGET_SET(0, Condition, gcSL_LESS_OR_EQUAL)
                      | gcmSL_TARGET_SET(0, Format, Info->indexFormat);

    inst->tempIndex   = LoopStartCode->id;  /* jump to loop start */
    inst->tempIndexed = 0;

    /* source 0: */
    inst->source0        = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
                         | gcmSL_SOURCE_SET(0, Indexed,gcSL_NOT_INDEXED)
                         | gcmSL_SOURCE_SET(0, Format, Info->indexFormat)
                         | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX);

    inst->source0Index   = Info->loopIndex;
    inst->source0Indexed = 0;

    /* source 1: end index */
    inst->source1        = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                         | gcmSL_SOURCE_SET(0, Indexed,gcSL_NOT_INDEXED)
                         | gcmSL_SOURCE_SET(0, Format, Info->indexFormat)
                         | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX);
    inst->source1Index   =  (Info->endIndex.uval & 0xFFFF);
    inst->source1Indexed =  (Info->endIndex.uval >> 16) & 0xFFFF;

    /* clean up data flow */
    gcOpt_DestroyCodeFlowData(Optimizer, code);

    return code;
}

static gcOPT_CODE
_fixMovResultToTemp(
    IN OUT  gcOPTIMIZER      Optimizer,
    IN      LoopRerollInfo * Info
    )
{
    gcOPT_CODE code= gcvNULL;

    gcmASSERT(gcvFALSE);  /* not implemented yet! */
    return code;
}

/* change the intermediate temp in OP code to use lastReductionTemp */
static void
_fixLoopBody(
    IN OUT  gcOPTIMIZER      Optimizer,
    IN      LoopRerollInfo * Info,
    IN      gcOPT_CODE       LoopStart,
    IN      gcOPT_CODE       LoopGenResultCode
    )
{
    gctUINT32 loadResultTemp;
    /* only handle loop start with gcSL_LOAD case now */
    gcmASSERT(gcmSL_OPCODE_GET(LoopStart->instruction.opcode, Opcode) == gcSL_LOAD);

    loadResultTemp = LoopStart->instruction.tempIndex;
    if (gcmSL_INDEX_GET(LoopGenResultCode->instruction.source0Index, Index) == loadResultTemp)
    {
        LoopGenResultCode->instruction.source1Index =
                                     Info->lastReductionTempIndex;
    }
    else
    {
        gcmASSERT(LoopGenResultCode->instruction.source1Index == loadResultTemp);
        LoopGenResultCode->instruction.source0Index =
                                      Info->lastReductionTempIndex;
    }
    return ;
}

/* cleanup code from StartCode to EndCode (not including EndCode) */
static void
_cleanupCode(
    IN OUT  gcOPTIMIZER      Optimizer,
    IN OUT  gcOPT_CODE       StartCode,
    IN OUT  gcOPT_CODE       EndCode
    )
{
    gcOPT_CODE  code;

    for (code = StartCode; code && code != EndCode; code = code->next)
    {
        /* set the instruction to nop */
        code->instruction = gcvSL_NOP_INSTR;
        /* delete data flow data */
        gcOpt_DestroyCodeFlowData(Optimizer, code);
    }

    return ;
}

static void
_fixDataFlow(
    IN OUT  gcOPT_CODE       StartCode
    )
{
}

/*   reroll the unrolled statements
 *
 *        temp(1) = temp(0) OP var[1]
 *        temp(2) = temp(1) OP var[2]
 *        ...
 *        temp(n) = temp(n-1) OP var[n]
 *
 *   to
 *
 *        temp(n) = temp(0);
 *        for(i=1; i <= n; i++)
 *           temp(n) = temp(n) + var[i]l;
 *
 */
static void
_rerollLoop(
    IN OUT  gcOPTIMIZER      Optimizer,
    IN      LoopRerollInfo * Info,
    IN      gcOPT_CODE       LastCode
    )
{
    /*
         0: MOV     temp(0), 0.000000  // init temp value
         1: MOV     temp.int(1).x, 0   // init loop index
         2: ADD     temp(2), temp(0), uniform(0+temp.int(1).x)
         3: MOV     temp(0), temp(2)
         4: ADD     temp.int(3).x, temp.int(1).x, 1    // increase index
         5: MOV     temp.int(1).x, temp.int(3).x
         6: JMP.L   2, temp.int(1).x, 18          // conditional jump to loop start


        ***********************************************************************

        [DATA FLOW]

          main() : [0 - 8]
             0: Users: output, 2
                N Def: 3
             1: Users: 6, 4, 2
                N Def: 5
             2: Users: 3
                Src 0: 3, 0
                Src 1: 5, 1
             3: Users: output, 2
                Src 0: 2
                P Def: 0
             4: Users: 5
                Src 0: 5, 1
             5: Users: 4, 2, 6
                Src 0: 4
                P Def: 1
             6: Src 0: 1, 5
             7: Users: output
                P Def:
    */
    gcOPT_CODE    initTemp;
    gcOPT_CODE    initLoopIndex;
    gcOPT_CODE    movResultToTemp = gcvNULL;
    gcOPT_CODE    increaseLoopIndex;
    gcOPT_CODE    movToLoopIndex;
    gcOPT_CODE    conditionJump;
    gcOPT_CODE    loopStart;         /* the real loop start code, it is
                                        2nd or 3rd pattern start code */
    gcOPT_CODE    loopGenResultCode; /* the code produce the result
                                        in the real loop */

    /* make sure we don't need to do head test and have enough code
     * to change to other instructions
     */
    gcmASSERT(Info->iterations >= REROLL_ITERATION_THRESHOLD);

    /* we need to change two Code to initTemp and initLoopIndex
     * lets start the loop in the second or third iteration depends on
     * the pattern length
     */
    initTemp      = _createInitTemp(Optimizer, Info);
    initLoopIndex = _createInitLoopIndex(Optimizer, Info);

    loopStart          = _findLoopStart(Optimizer, Info);
    loopGenResultCode  = _findLoopGetResult(Optimizer, Info, loopStart);

    _fixDataFlow(initTemp);
    _fixLoopBody(Optimizer, Info, loopStart, loopGenResultCode);

    if (Info->expKind == REK_DescendingReduction)
    {
        movResultToTemp =
            _createMovResultToTemp(Optimizer, Info, loopGenResultCode);
    }
    else
    {
        movResultToTemp = _fixMovResultToTemp(Optimizer, Info);
    }

    increaseLoopIndex = _createIncreaseLoopIndex(Optimizer, Info, movResultToTemp);
    movToLoopIndex    = _createMovToLoopIndex(Optimizer, Info, increaseLoopIndex);
    conditionJump     = _createConditionJump(Optimizer, Info, movToLoopIndex, loopStart);

    if (initLoopIndex && initLoopIndex->next != loopStart)
    {
        /* cleanup the unsed code between initLoopIndex and loopStart */
        _cleanupCode(Optimizer, initLoopIndex->next, loopStart);
    }
    if (conditionJump)
        _cleanupCode(Optimizer, conditionJump->next, LastCode->next);

    return;
}


/*******************************************************************************
**                          gcOpt_OptimizeLoopRerolling
********************************************************************************
**
**  Rerolling loop, a reverse process of loop rolling
**      if
**      for backend linker to convert them into MADD instructions.
**
**  INPUT:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_OptimizeLoopRerolling(
    IN gcOPTIMIZER Optimizer
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    LoopRerollInfo  rerollInfo;
    gcOPT_CODE      code;
    gctUINT         codeRerolled = 0;
    gcOPT_CODE      nextPatternCode = gcvNULL;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    _resetRerollInfo(&rerollInfo);

    for (code = Optimizer->codeHead; code; )
    {
        if (!_findRerollPattern(Optimizer, code, &rerollInfo, &nextPatternCode))
        {
            /* not found a pattern, go to next code */
            code = code->next;
            continue;
        }

        /* found a pattern, continue to search until pattern terminated */
        do
        {
            code = nextPatternCode;
        }
        while (code &&
               _findNextRerollPattern(Optimizer, code,
                                      &rerollInfo, &nextPatternCode));

        /* reroll loop */
        if (rerollInfo.iterations > REROLL_ITERATION_THRESHOLD)
        {
            codeRerolled++;
            _rerollLoop(Optimizer, &rerollInfo, code->prev);
        }

        /* reset reroll info */
        _resetRerollInfo(&rerollInfo);
    }

    if (codeRerolled > 0)
    {
        status = gcvSTATUS_CHANGED;
    }

    if (status == gcvSTATUS_CHANGED)
    {
        DUMP_OPTIMIZER("Optimize reroll loops in the shader", Optimizer);
        gcmFOOTER();
        return status;
    }

    gcmFOOTER();
    return status;
}

static gctBOOL
_isTempRegLocal(
    IN gcOPTIMIZER    Optimizer,
    IN gcOPT_FUNCTION Function,
    IN gctINT         TempIndex
    )
{
    gcOPT_TEMP  temp = &Optimizer->tempArray[TempIndex];

    gcmASSERT(TempIndex >= 0 && TempIndex < (gctINT)Optimizer->tempCount);

    if (temp && !temp->isGlobal && temp->function == Function)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_updateIndex(
    IN gcOPTIMIZER    Optimizer,
    IN gcOPT_FUNCTION Function,
    IN gctUINT32      OldTempIndexStart,
    IN gctUINT32      TempIndexCount,
    IN gctUINT32      NewTempIndexStart,
    OUT gctUINT32 *   IndexPtr
    )
{
    if (*IndexPtr >= OldTempIndexStart &&
        *IndexPtr < OldTempIndexStart + TempIndexCount &&
        _isTempRegLocal(Optimizer, Function, *IndexPtr))
    {
        gctINT  tempOffset = NewTempIndexStart - OldTempIndexStart;
        /* Adjust temporary register count. */
        *IndexPtr +=  tempOffset;
        return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL
_updateIndexed(
    IN gcOPTIMIZER    Optimizer,
    IN gcOPT_FUNCTION Function,
    IN gctUINT        OldTempIndexStart,
    IN gctUINT        TempIndexCount,
    IN gctUINT        NewTempIndexStart,
    OUT gctUINT16 *   IndexPtr
    )
{
    if (*IndexPtr >= OldTempIndexStart &&
        *IndexPtr < OldTempIndexStart + TempIndexCount &&
        _isTempRegLocal(Optimizer, Function, *IndexPtr))
    {
        gctINT  tempOffset = NewTempIndexStart - OldTempIndexStart;
        /* Adjust temporary register count. */
        *IndexPtr +=  (gctUINT16)tempOffset;
        return gcvTRUE;
    }
    return gcvFALSE;
}
/* return TRUE if the Code has tempIndex renamed */
static gctBOOL
_renameTempIndex(
    IN gcOPTIMIZER    Optimizer,
    IN gcOPT_CODE     Code,
    IN gcOPT_FUNCTION Function,
    IN gctINT         OldTempIndexStart,
    IN gctINT         TempIndexCount,
    IN gctINT         NewTempIndexStart,
    IN gctBOOL        RenameInput,
    IN gctBOOL        RenameTarget,
    IN gctBOOL        RenameSource
    )
{
    gcSL_INSTRUCTION inst   = &Code->instruction;
    gcSL_OPCODE      opcode = gcmSL_OPCODE_GET(inst->opcode, Opcode);
    gctINT           i;
    gctBOOL          renamed = gcvFALSE;

    /* check if there is temp register in the function */
    if (TempIndexCount <= 0)
        return gcvFALSE;

    /* Adjust label temporary register number. */
    switch (opcode)
    {
    case gcSL_NOP:
        /* fall through */
    case gcSL_RET:
        /* fall through */
    case gcSL_CALL:
        /* Skip instructions without destination. */
        break;

    default:
        /* check target */
        if (RenameTarget &&
            gcmSL_TARGET_GET(inst->temp, Enable) != gcSL_ENABLE_NONE)
        {
            if (_updateIndex(Optimizer, Function, OldTempIndexStart,
                    TempIndexCount, NewTempIndexStart,&inst->tempIndex))
            {
                renamed = gcvTRUE;
            }


            if (gcmSL_TARGET_GET(inst->temp, Indexed) != gcSL_NOT_INDEXED)
            {
                /* fix indexed register */
                if (_updateIndexed(Optimizer, Function, OldTempIndexStart,
                        TempIndexCount, NewTempIndexStart,&inst->tempIndexed))
                {
                    renamed = gcvTRUE;
                }
            }
        }

        /* fix sources */
        if (RenameSource)
        {
            for (i = 0; i < 2; i++)
            {
                gctSOURCE_t source = (i==0) ? inst->source0 : inst->source1;
                gctUINT32 * index  = (i==0) ? &inst->source0Index
                                            : &inst->source1Index;

                if (RenameInput)
                {
                    gctINT j;
                    gcsFUNCTION_ARGUMENT_PTR argument;
                    gctBOOL isOutput = gcvFALSE;

                    for (j=0; j < (gctINT)Function->argumentCount; j++)
                    {
                        argument = Function->arguments + j;
                        if (argument->qualifier != gcvFUNCTION_INPUT &&
                            *index == argument->index)
                        {
                            isOutput = gcvTRUE;
                            break;
                        }
                    }

                    if (isOutput)
                        continue;
                }

                if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
                {
                    /* Adjust temporary register count. */
                    if (_updateIndex(Optimizer, Function, OldTempIndexStart,
                            TempIndexCount, NewTempIndexStart,index))
                    {
                        renamed = gcvTRUE;
                    }
                }
                /* fix indexed value */
                if (gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
                {
                    gctUINT16 * indexed = (i==0) ? &inst->source0Indexed
                                                 : &inst->source1Indexed;

                    /* fix indexed register */
                    if (_updateIndexed(Optimizer, Function, OldTempIndexStart,
                            TempIndexCount, NewTempIndexStart,indexed))
                    {
                        renamed = gcvTRUE;
                    }
                }
            }
        }
    }
    return renamed;
}

static gceSTATUS
_RemapTempIndexForExpandFunction(
    IN gcOPTIMIZER Optimizer,
    IN gcOPT_FUNCTION Function,
    IN VSC_HASH_TABLE* pCallerInstTable,
    IN gcOPT_CODE Caller,
    IN gcOPT_CODE CodeNext,
    IN gctINT OldTempIndexStart,
    IN gctINT TempIndexCount,
    IN gctINT NewTempIndexStart,
    IN gctUINT RealCallerCount
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcOPT_TEMP tempArray = Optimizer->tempArray + OldTempIndexStart;
    gcOPT_CODE codeNext = CodeNext;
    gcOPT_CODE code;
    gctINT i;

    /* duplicate arrayVariable if existing */
    for (i = 0; i < TempIndexCount && (i + OldTempIndexStart) < (gctINT)Optimizer->tempCount; i++)
    {
        if (tempArray[i].function != Function)
            continue;
        /*
        ** We need to make sure that we only remap the registers belong to this function.
        */
        if (tempArray[i].arrayVariable && tempArray[i].arrayVariable->nameLength >= 0)
        {
            gctCHAR variableName[256];
            gctUINT offset = 0;
            gcVARIABLE variable = tempArray[i].arrayVariable;
            gctINT16   varIndex = -1;
            gctCONST_STRING name = (variable->nameLength < 0)
                                    ? gcSHADER_GetBuiltinNameString(variable->nameLength)
                                    : variable->name;

            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(variableName, sizeof(variableName), &offset,
                                    "%s_%d", name, RealCallerCount));

            gcmONERROR(gcSHADER_AddVariableEx(Optimizer->shader,
                                              variableName,
                                              variable->u.type,
                                              variable->arrayLengthCount,
                                              variable->arrayLengthList,
                                              (gctUINT16)(i + NewTempIndexStart),
                                              gcSHADER_VAR_CATEGORY_NORMAL,
                                              variable->precision,
                                              0,
                                              -1,
                                              -1,
                                              &varIndex));
            i += GetVariableKnownArraySize(variable) * gcmType_Rows(variable->u.type) - 1;
        }
    }

    /* first handle inlined function body */
    for (code = Caller; code != gcvNULL && code != codeNext; code = code->next)
    {
        _renameTempIndex(Optimizer,
                         code,
                         Function,
                         OldTempIndexStart,
                         TempIndexCount,
                         NewTempIndexStart,
                         gcvFALSE,
                         gcvTRUE,
                         gcvTRUE);
    }

    /* then handle argument passing in caller */
    if (Function->argumentCount > 0)
    {
        gctINT inputArgs  = 0;
        gctINT outputArgs = 0;
        gctINT j;
        gcsFUNCTION_ARGUMENT_PTR argument;

        /* count intput output arg numbers */
        for (j=0; j < (gctINT)Function->argumentCount; j++)
        {
            argument = Function->arguments + j;
            if (argument->qualifier == gcvFUNCTION_INPUT)
            {
                inputArgs++;
            }
            else if (argument->qualifier == gcvFUNCTION_INOUT)
            {
                inputArgs++;
                outputArgs++;
            }
            else
            {
                gcmASSERT((argument->qualifier == gcvFUNCTION_OUTPUT));
                outputArgs++;
            }
        }

        /* check input arguments */
        if (inputArgs > 0)
        {
            for (code = Caller->prev; code != gcvNULL; code = code->prev)
            {
                gcSL_OPCODE opcode =
                    gcmSL_OPCODE_GET(code->instruction.opcode, Opcode);

                if (opcode == gcSL_CALL ||
                    opcode == gcSL_RET )
                {
                    /* bail out if encountered control follow code */
                    break;
                }

                /* The CALL may be changed to a NOP, so we need to check if it is in the hash table. */
                if (opcode == gcSL_NOP
                    &&
                    vscHTBL_DirectTestAndGet(pCallerInstTable, (void *)code, gcvNULL))
                {
                    break;
                }

                if (gcmSL_TARGET_GET(code->instruction.temp, Enable) != gcSL_ENABLE_NONE)
                {
                    gctUINT32   tempIndex = code->instruction.tempIndex;

                    /* check against each argument */
                    for (j = 0; j < (gctINT)Function->argumentCount; j++)
                    {
                        argument = Function->arguments + j;
                        if (argument->qualifier != gcvFUNCTION_OUTPUT &&
                            tempIndex == argument->index)
                        {
                            /* found */
                            _renameTempIndex(Optimizer,
                                             code,
                                             Function,
                                             OldTempIndexStart,
                                             TempIndexCount,
                                             NewTempIndexStart,
                                             gcvTRUE,
                                             gcvTRUE,
                                             gcvFALSE);
                            break;
                        }
                    }
                }
            }
        }

        /* check output arguments */
        if (outputArgs > 0)
        {
            for (code = codeNext; code != gcvNULL; code = code->next)
            {
                gctINT i;
                gcSL_INSTRUCTION inst   = &code->instruction;
                gcSL_OPCODE opcode      = gcmSL_OPCODE_GET(inst->opcode, Opcode);

                if (opcode == gcSL_RET)
                {
                    /* bail out if encountered control follow code */
                    break;
                }

                /* bail out if encountered control follow code */
                if (opcode == gcSL_CALL &&
                    code->callee->function == Function)
                {
                    break;
                }

                /* The CALL may be changed to a NOP, so we need to check if it is in the hash table. */
                if (opcode == gcSL_NOP
                    &&
                    vscHTBL_DirectTestAndGet(pCallerInstTable, (void *)code, gcvNULL))
                {
                    break;
                }

                if (gcmSL_TARGET_GET(inst->temp, Enable) != gcSL_ENABLE_NONE ||
                    (opcode == gcSL_JMP && gcmSL_TARGET_GET(inst->temp, Condition) != gcSL_ALWAYS))
                {
                    for (i = 0; i < 2; i++)
                    {
                        gctSOURCE_t source = (i==0) ? inst->source0 : inst->source1;

                        if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
                        {
                            gctUINT32 * index  = (i==0) ? &inst->source0Index
                                                        : &inst->source1Index;
                            /* check against each argument */
                            for (j = 0; j < (gctINT)Function->argumentCount; j++)
                            {
                                argument = Function->arguments + j;
                                if (argument->qualifier != gcvFUNCTION_INPUT &&
                                    *index == argument->index)
                                {
                                    /* Adjust temporary register count. */
                                    gctBOOL renamed =
                                        _updateIndex(Optimizer, Function, OldTempIndexStart,
                                            TempIndexCount, NewTempIndexStart,index);
                                    if (renamed != gcvTRUE)
                                    {
                                        gcmASSERT(gcvFALSE);
                                    }

                                    break;
                                }
                                /* no need to check indexed value */
                            }
                        }
                    }
                }
            }
        }
    }

OnError:
    return status;
}

/*******************************************************************************
**                          _ExpandOneFunctionCall
********************************************************************************
**
**  Expand one functon into the caller, and remove the function if specified.
**
**  INPUT:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
**
**      gcOPT_FUNCTION function
**          Pointer to a gcOPT_FUNCTION structure.
**
**      gctUINT caller
**          Caller location.
**
**      gctUINT realCallerCount
**          Whether to remove the function.
*/
static gceSTATUS
_ExpandOneFunctionCall(
    IN gcOPTIMIZER Optimizer,
    IN gcOPT_FUNCTION Function,
    IN VSC_HASH_TABLE* pCallerInstTable,
    IN gcOPT_CODE Caller,
    IN gctUINT RealCallerCount,
    IN gctBOOL *RenameArgument
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcOPT_CODE codeNext = Caller->next;
    gcOPT_CODE code;
    gcePATCH_ID patchID = Optimizer->patchID;
    gctBOOL inlineAllFunctionForCTS = gcvFALSE;
    gctBOOL WillRemoveFunction = (RealCallerCount == 0);
    gctBOOL FunctionwithCall = gcvFALSE;
    gcOPT_FUNCTION callerFunction = Caller->function;
    gctUINT32 newTempStart = 0, newTempEnd = 0;
    gctBOOL renamed = gcvFALSE;

    gcmHEADER_ARG("Optimizer=0x%x Function=0x%x Caller=0x%x WillRemoveFunction=%d",
                   Optimizer, Function, Caller, WillRemoveFunction);

    /* After expansion, the CALL instruction will be after the function block. */
    if (WillRemoveFunction)
    {
        /* Expand the function by moving the code to after caller. */
        gcOpt_MoveCodeListAfter(Optimizer,
                                Function->codeHead,
                                Function->codeTail,
                                Caller,
                                gcvTRUE);
        Function->codeHead = gcvNULL;
        Function->codeTail = gcvNULL;
    }
    else
    {
        /* Expand the function by copying the code to after caller. */
        gcmONERROR(gcOpt_CopyCodeListAfter(Optimizer,
                                           Function->codeHead,
                                           Function->codeTail,
                                           Caller,
                                           gcvFALSE));
    }

    /* Change the caller to NOP. */
    Caller->instruction = gcvSL_NOP_INSTR;

    /* If the last instruction of the function is RET, change it to NOP. */
    code = codeNext->prev;
    if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_RET)
    {
        code->instruction = gcvSL_NOP_INSTR;
    }

    /* Change the rest RET instructions to JMP. */
    for (code = code->prev; code != gcvNULL && code != Caller; code = code->prev)
    {
        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_RET)
        {
            gcmSL_OPCODE_UPDATE(code->instruction.opcode, Opcode, gcSL_JMP);
            code->instruction.tempIndex = codeNext->id;

            /* Set caller and callee. */
            code->callee = codeNext;
            gcmONERROR(gcOpt_AddCodeToList(Optimizer, &codeNext->callers, code));
        }
    }

    if (patchID == gcvPATCH_DEQP)
    {
        for (code = Function->codeHead; code != Function->codeTail; code = code->next)
        {
            if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_CALL)
            {
                FunctionwithCall = gcvTRUE;
                break;
            }
        }
    }

    /* rename local temp registers used in the inlined function, including
       all parameters
     */
    /* Don't do this optimization for CTS30, it would use a lot of memory. */
    if (Function->shaderFunction &&
        gcoOS_StrNCmp(Function->shaderFunction->name, "compare_", 8) == 0 &&
        Optimizer->shader->storageBlockCount == 0 &&
        (patchID == gcvPATCH_GTFES30 || patchID == gcvPATCH_DEQP))
    {
        inlineAllFunctionForCTS = gcvTRUE;
    }

    if (callerFunction)
    {
        newTempStart = Function->tempIndexStart;
        newTempEnd = Function->tempIndexEnd;
    }

    if (!FunctionwithCall &&
        !WillRemoveFunction &&
        gcmOPT_hasFeature(FB_INLINE_RENAMETEMP)
        && !inlineAllFunctionForCTS
        /* if a function has its callee be inlined, its tempRegCount may be bloated to a very big number
         * which can cause rename to even bigger number, it may exceed the gcSL UR limits (65536/4),
         * to avoid this, just disable rename for the inlined body */
        && (!Function->shaderFunction ||
            !(IsFunctionHasBigGapInTempReg(Function->shaderFunction) && RealCallerCount > 2 && gcSHADER_GetTempCount(Optimizer->shader) > 10000) )
        )
    {
        gctINT oldTempIndexStart = Function->tempIndexStart;
        gctINT tempIndexCount    = Function->tempIndexCount;
        gctINT newTempIndexStart = gcSHADER_NewTempRegs(Optimizer->shader,
                                                        tempIndexCount,
                                                        gcSHADER_FLOAT_X1);

        newTempStart = newTempIndexStart;
        newTempEnd = gcSHADER_GetTempCount(Optimizer->shader);

        gcmONERROR(_RemapTempIndexForExpandFunction(Optimizer,
                                                    Function,
                                                    pCallerInstTable,
                                                    Caller,
                                                    codeNext,
                                                    oldTempIndexStart,
                                                    tempIndexCount,
                                                    newTempIndexStart,
                                                    RealCallerCount));
        if (RenameArgument)
        {
            *RenameArgument = gcvTRUE;
        }

        renamed = gcvTRUE;
    }

    if (callerFunction != gcvNULL)
    {
        gctUINT32 tempStart = 0, tempEnd = 0, tempCount = 0;

        tempStart = gcmMIN(newTempStart, callerFunction->tempIndexStart);
        tempEnd = gcmMAX(newTempEnd, callerFunction->tempIndexEnd);
        if (renamed)
        {
            tempCount = callerFunction->tempIndexEnd - callerFunction->tempIndexStart + 1;
        }
        else
        {
            tempCount = tempEnd - tempStart + 1;
        }

        callerFunction->tempIndexStart = tempStart;
        callerFunction->tempIndexEnd = tempEnd;
        callerFunction->tempIndexCount = tempCount;

        if (callerFunction->shaderFunction)
        {
            if (tempCount > callerFunction->shaderFunction->tempIndexCount + 400)
            {
                SetFunctionFlags(callerFunction->shaderFunction, gcvFUNC_HAS_TEMPREG_BIGGAP);
            }
            callerFunction->shaderFunction->tempIndexStart = tempStart;
            callerFunction->shaderFunction->tempIndexEnd = tempEnd;
            callerFunction->shaderFunction->tempIndexCount = tempCount;
        }
        else
        {
            gcmASSERT(callerFunction->kernelFunction);

            callerFunction->kernelFunction->tempIndexStart = tempStart;
            callerFunction->kernelFunction->tempIndexEnd = tempEnd;
            callerFunction->kernelFunction->tempIndexCount = tempCount;
        }
    }

    gcmFOOTER();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gctINT
_GetInlineBudget(
    IN gcOPTIMIZER Optimizer
    )
{
    gctUINT instMax = 0;
    gctINT  budget=0;
    gcSHADER shader = Optimizer->shader;
    gctSIZE_T codeCount = Optimizer->codeTail->id + 1;
    gcePATCH_ID patchID = Optimizer->patchID;

    if (gcHWCaps.hwFeatureFlags.hasInstCache &&
        /* WAR for some APPs. */
        (patchID != gcvPATCH_YOUILABS_SHADERTEST && !gcdPROC_IS_WEBGL(patchID)))
    {
        if (patchID == gcvPATCH_GTFES30 || patchID == gcvPATCH_DEQP)
        {
            return 0x7FFFFFFF;
        }
        else
        {
            instMax = 1024 * gcmOPT_INLINELEVEL();
        }
    }
    else
    {
        /* Determine the maximum number of instructions. */
        if (shader->type == gcSHADER_TYPE_VERTEX)
        {
            instMax = gcHWCaps.maxVSInstCount;
        }
        else
        {
            instMax = gcHWCaps.maxPSInstCount;
        }
    }
    /* get budget based on current shader code count */
    if (codeCount * 1.2 < (gctFLOAT)instMax)
    {
        budget = (gctINT)(instMax - (gctUINT)(codeCount * 1.2));
    }

    return budget;
}

static gctBOOL
_isFunctionInlinable(
    IN OUT gctINT *     CurrentBudegt,
    IN gcOPT_FUNCTION   Function,
    IN gctUINT          CallerCount,
    IN gctUINT          InlineLevel)
{
    gctUINT  funcCodeCount;
    gctUINT  inlineFuncCountThreshold = (InlineLevel > 2) ? 0xFFFF :
                              (InlineLevel > 1) ? 4 : 1;
    gctINT   packedAwayArgNo = Function->shaderFunction ? Function->shaderFunction->packedAwayArgNo : 0;
    if (InlineLevel == 0)
        return gcvFALSE;

    /* do not inline intrinsic functions,
     * it will be lowered later */
    if (Function->shaderFunction &&
        (gcoOS_StrNCmp(Function->shaderFunction->name, "viv_intrinsic_", 14) == 0 ))
    {
        return gcvFALSE;
    }

    funcCodeCount = Function->codeTail->id - Function->codeHead->id + 1;
    if (CallerCount == 1 ||
        funcCodeCount < (Function->argumentCount + packedAwayArgNo + 2 + InlineLevel))
    {
        /* always inline if it will not increase code size */
        return gcvTRUE;
    }

    if (CallerCount <= inlineFuncCountThreshold &&
        (gctINT)(funcCodeCount * CallerCount) < *CurrentBudegt)
    {
        /* update inline budget */
        *CurrentBudegt -= (funcCodeCount * CallerCount);
        return gcvTRUE;
    }

    /* check for small function */
    if ((funcCodeCount < Function->argumentCount + 3*InlineLevel) &&
         *CurrentBudegt > (gctINT)(3*InlineLevel*CallerCount) )
    {
        /* update inline budget */
        *CurrentBudegt -=  3*InlineLevel*CallerCount;
        return gcvTRUE;
    }

    /* inline if there are enough budget left: inline use less than 2/3 of budget */
   if ((gctINT)(funcCodeCount * CallerCount) < (*CurrentBudegt * 2)/3 )
    {
        /* update inline budget */
        *CurrentBudegt -= (funcCodeCount * CallerCount);
        return gcvTRUE;
    }
    return gcvFALSE;
}

static enum ForceInlineKind
_getForceInlineKind(
    IN gcOPT_FUNCTION Function
    )
{
    enum ForceInlineKind kind      = FIK_None;
    gctSTRING            funcName  = gcvNULL;
    InlineStringList *   forceInline = gcmOPT_ForceInline();

    if (Function->kernelFunction != gcvNULL)
    {
        funcName = Function->kernelFunction->name;
    }
    else if (Function->shaderFunction != gcvNULL)
    {
        funcName = Function->shaderFunction->name;
    }

    if (funcName != gcvNULL)
    {
        /* walk through the inline string list */
        while (forceInline != gcvNULL)
        {
            if (gcoOS_StrCmp(forceInline->func, funcName) == 0)
            {
                /* found the function name */
                kind = forceInline->kind;
                break;
            }
            forceInline = forceInline->next;
        }
    }

    return kind;
}

static gctBOOL
_IsFunctionNeedToBeExpand(
    IN OUT gctINT *     CurrentBudegt,
    IN gcOPT_FUNCTION   Function,
    IN gctUINT          CallerCount,
    IN gctUINT          inlineDepthComparison,
    IN gctUINT          inlineFormatConversion,
    IN gctUINT          InlineLevel)
{
    gcFUNCTION function;
    gctUINT funcCodeCount = 0;
    gctBOOL bNeedToBeExpand = gcvFALSE;
    gctUINT  inlineFuncCountThreshold = (InlineLevel > 2) ? 0xFFFF : (InlineLevel > 1) ? 4 : 1;

    if (Function->shaderFunction == gcvNULL)
    {
        return gcvFALSE;
    }

    function = Function->shaderFunction;
    funcCodeCount = Function->codeTail->id - Function->codeHead->id + 1;

    if (IsFunctionParamAsImgSource0(function))
    {
        bNeedToBeExpand = gcvTRUE;
    }
    else if (IsFunctionUsingSamplerVirtual(function))
    {
        bNeedToBeExpand = gcvTRUE;
    }
    else if ((gcoOS_StrNCmp(function->name, "_viv_image_store", 16) == 0)   ||
             (gcoOS_StrNCmp(function->name, "_viv_image_load", 15) == 0)    ||
             (gcoOS_StrNCmp(function->name, "_write_image", 12) == 0)       ||
             (gcoOS_StrNCmp(function->name, "_read_image_nearest", 19) == 0))
    {
        bNeedToBeExpand = gcvTRUE;
    }
    else if (inlineDepthComparison > 0 &&
             gcoOS_StrNCmp(function->name, "_txpcfcvt", 9) == 0)
    {
        bNeedToBeExpand = gcvTRUE;
    }
    else if (inlineFormatConversion > 0)
    {
        if (gcoOS_StrNCmp(function->name, "_txcvt_swizzle", 14) == 0)
        {
            bNeedToBeExpand = gcvTRUE;
        }
#if DX_SHADER
        else if (gcoOS_StrNCmp(function->name, "_itxcvt", 7) == 0)
        {
            if (function->name[8] >= 'A' && function->name[8] <= 'Z')
            {
                bNeedToBeExpand = gcvTRUE;
            }
        }
#else
        else if (gcoOS_StrNCmp(function->name, "_txcvt", 6) == 0)
        {
            if (function->name[7] >= 'A' && function->name[7] <= 'Z')
            {
                bNeedToBeExpand = gcvTRUE;
            }

            /* If there is no budegt left and this function is used many times, don't inline it. */
            if (bNeedToBeExpand &&
                (gcGetHWCaps()->chipModel == gcv880 && gcGetHWCaps()->chipRevision == 0x5124) &&
                CallerCount >= inlineFuncCountThreshold &&
                *CurrentBudegt <= 0)
            {
                bNeedToBeExpand = gcvFALSE;
            }
        }
#endif
    }

    /* Calculate the budegt. */
    if (bNeedToBeExpand)
    {
        *CurrentBudegt -= (funcCodeCount * CallerCount);
    }

    return bNeedToBeExpand;
}

static gceSTATUS
_InlineSinglelFunction(
    IN gcOPTIMIZER Optimizer,
    IN gcOPT_FUNCTION function,
    IN gctUINT inlineDepthComparison,
    IN gctUINT inlineFormatConversion,
    IN gctUINT inlineLevel,
    IN gctBOOL imagePatch,
    IN gctBOOL alwaysInline,
    IN OUT gctBOOL * imageFunctionInlined,
    IN OUT gctINT *  currentBudget,
    IN OUT gctUINT * functionRemoved
    )
{
    gceSTATUS status = gcvSTATUS_TRUE;
    gctUINT realCallerCount = 0;
    gcOPT_LIST caller;
    gcOPT_CODE codeCaller;
    gctBOOL isMainKernelFunction = gcvFALSE;
    gctBOOL renameArgument = gcvFALSE;
    enum ForceInlineKind forceInline;
    gctBOOL inlineAllFunctionForCTS = gcvFALSE;
    VSC_HASH_TABLE* pCallerInstTable = gcvNULL;
    VSC_PRIMARY_MEM_POOL mp;

    if (Optimizer->isCTSInline)
    {
        inlineAllFunctionForCTS = gcvTRUE;
    }

    /* Remove all the recursive function */
    if (function->shaderFunction != gcvNULL && function->shaderFunction->isRecursion)
    {
        gcmONERROR(gcOpt_DeleteFunction(Optimizer, function, gcvFALSE, gcvTRUE));
        (*functionRemoved)++;
        return status;
    }

    /* Calculate the real caller count. */
    for (caller = function->codeHead->callers;
         caller != gcvNULL;
         caller = caller->next)
    {
        if (gcmSL_OPCODE_GET(caller->code->instruction.opcode, Opcode) == gcSL_CALL)
        {
            if (caller->code->function != gcvNULL && caller->code->function->shaderFunction != gcvNULL &&
                caller->code->function->shaderFunction->isRecursion)
                continue;
            realCallerCount++;
        }
    }

    /* Some unused functions are not removed on _PackMainProgram, */
    /* we just remove them without budget check. */
    if (realCallerCount == 0)
    {
        gcmONERROR(gcOpt_DeleteFunction(Optimizer, function, gcvFALSE, gcvTRUE));
        (*functionRemoved)++;
        return status;
    }

    forceInline = _getForceInlineKind(function);
    if (forceInline == FIK_NotInline)
    {
        return gcvSTATUS_FALSE;
    }

    /* For image patching, force inline functions containing IMAGE_RD or IMAGE_WR. */
    if (imagePatch)
    {
        gcOPT_CODE code;

        for (code = function->codeHead; code != function->codeTail; code = code->next)
        {
            gcSL_OPCODE opcode = gcmSL_OPCODE_GET(code->instruction.opcode, Opcode);

            if (opcode == gcSL_IMAGE_SAMPLER || opcode == gcSL_IMAGE_WR || opcode == gcSL_IMAGE_WR_3D)
            {
                forceInline = FIK_Inline;
                if (imageFunctionInlined)
                {
                    *imageFunctionInlined = gcvTRUE;
                }
            }
        }
    }

    /* Check if function is unnecessary. */
    /* Function is called only once or is short but with many arguments. */
    if (!alwaysInline
    &&  function->kernelFunction == gcvNULL
    &&  forceInline != FIK_Inline
    &&  !(_IsFunctionNeedToBeExpand(currentBudget, function, realCallerCount, inlineDepthComparison, inlineFormatConversion, inlineLevel))
    &&  !inlineAllFunctionForCTS
    &&  !_isFunctionInlinable(currentBudget, function, realCallerCount, inlineLevel))
    {
        return gcvSTATUS_FALSE;
    }

    /* Initialize the caller hash table. */
    vscPMP_Intialize(&mp, gcvNULL, 1024, sizeof(void *), gcvTRUE);
    pCallerInstTable = vscHTBL_Create(&mp.mmWrapper, vscHFUNC_Default, vscHKCMP_Default, 32);

    do
    {
        /* The following call should be in gcOpt_copyCodeListAfter. */
        /* Move here to avoid an assertion, which should be removed later after testing. */
        /* Need to update code id for checking. */
        gcOpt_UpdateCodeId(Optimizer);

        realCallerCount--;

        /* Expand the function. */
        /* Note that hintArray is rebuilt after each function expansion. */
        for (caller = function->codeHead->callers;
             caller != gcvNULL;
             caller = caller->next)
        {
            if (gcmSL_OPCODE_GET(caller->code->instruction.opcode, Opcode) == gcSL_CALL)
            {
                if (caller->code->function != gcvNULL && caller->code->function->shaderFunction != gcvNULL &&
                    caller->code->function->shaderFunction->isRecursion)
                    continue;
                break;
            }
        }

        if (caller == gcvNULL)
        {
            gcmASSERT(caller);
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        codeCaller = caller->code;
        codeCaller->callee = gcvNULL;

        if (function->kernelFunction && codeCaller->function == gcvNULL)
        {
            isMainKernelFunction = gcvTRUE;
        }

        /* If the caller is inside the other function, we need to remap temp index. */
        gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer,
                        &function->codeHead->callers,
                        codeCaller));

        /* Insert the caller instruction to the hash table. */
        vscHTBL_DirectSet(pCallerInstTable, (void*)codeCaller, gcvNULL);

        gcmONERROR(_ExpandOneFunctionCall(Optimizer,
                                          function,
                                          pCallerInstTable,
                                          codeCaller,
                                          realCallerCount,
                                          (realCallerCount == 0) ? &renameArgument : gcvNULL));
    }
    while (realCallerCount > 0);

    if (isMainKernelFunction)
    {
        Optimizer->main->kernelFunction = function->kernelFunction;
        Optimizer->isMainMergeWithKerenel = gcvTRUE;
    }

    gcmONERROR(gcOpt_DeleteFunction(Optimizer, function, gcvFALSE, renameArgument));

    (*functionRemoved)++;

    status = gcvSTATUS_TRUE;

OnError:
    if (pCallerInstTable)
    {
        vscHTBL_Destroy(pCallerInstTable);
        vscPMP_Finalize(&mp);
    }

    /* Return the status. */
    return status;
}

/*******************************************************************************
**                          gcOpt_ExpandFunctions
********************************************************************************
**
**  Expand functons that are called once or short but with many arguments.
**
**  INPUT:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_InlineFunctions(
    IN gcOPTIMIZER * OptimizerPtr,
    IN gctBOOL       AlwaysInline
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT functionRemoved = 0;
    gctINT i;
    gctUINT inlineLevel;
    gcOPTIMIZER Optimizer = *OptimizerPtr;
    gctUINT inlineDepthComparison  = gcmOPT_INLINEDEPTHCOMP();
    gctUINT inlineFormatConversion = gcmOPT_INLINEFORMATCONV();
    gctINT  currentBudget;
    gctUINT savedShaderTempCount = Optimizer->shader->_tempRegCount;
    gctBOOL imagePatch;
    gctBOOL imageFunctionInlined;
    gctBOOL isCTSInline = gcvFALSE;
    gctUINT option = Optimizer->shader->optimizationOption;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    imagePatch = gcdHasOptimization(Optimizer->option, gcvOPTIMIZATION_IMAGE_PATCHING);

    if (gcdHasOptimization(option, gcvOPTIMIZATION_INLINE_LEVEL_0))
    {
        inlineLevel = 0;
    }
    else if (gcdHasOptimization(option, gcvOPTIMIZATION_INLINE_LEVEL_1))
    {
        inlineLevel = 1;
    }
    else if (gcdHasOptimization(option, gcvOPTIMIZATION_INLINE_LEVEL_2))
    {
        inlineLevel = 2;
    }
    else if (gcdHasOptimization(option, gcvOPTIMIZATION_INLINE_LEVEL_3))
    {
        inlineLevel = 3;
    }
    else if (gcdHasOptimization(option, gcvOPTIMIZATION_INLINE_LEVEL_4))
    {
        inlineLevel = 4;
    }
    else
    {
        inlineLevel = gcmOPT_INLINELEVEL();
    }

    currentBudget = (inlineLevel == 4) ? 0x7fffffff : _GetInlineBudget(Optimizer);

    if (Optimizer->functionCount == 0 || inlineLevel == 0)
    {
        gcmFOOTER();
        return status;
    }

    /* Update code id. */
    gcOpt_UpdateCodeId(Optimizer);

    /* Inline all recompiler stub first. */
    if (Optimizer->shader->type != gcSHADER_TYPE_CL)
    {
        for (i = Optimizer->functionCount - 1; i >= 0; i--)
        {
            gcOPT_FUNCTION function = Optimizer->functionArray + i;

            if (function == gcvNULL ||
                !(function->shaderFunction != gcvNULL &&
                  IsFunctionRecompilerStub(function->shaderFunction)))
            {
                continue;
            }

            _InlineSinglelFunction(Optimizer,
                                   function,
                                   inlineDepthComparison,
                                   inlineFormatConversion,
                                   inlineLevel,
                                   imagePatch,
                                   gcvTRUE,
                                   &imageFunctionInlined,
                                   &currentBudget,
                                   &functionRemoved);
        }
    }

    /* Update code id. */
    gcOpt_UpdateCodeId(Optimizer);

    /* Inline all the other functions. */
    do
    {
        imageFunctionInlined = gcvFALSE;

        for (i = Optimizer->functionCount - 1; i >= 0; i--)
        {
            gcOPT_FUNCTION function = Optimizer->functionArray + i;

            _InlineSinglelFunction(Optimizer,
                                   function,
                                   inlineDepthComparison,
                                   inlineFormatConversion,
                                   inlineLevel,
                                   imagePatch,
                                   AlwaysInline,
                                   &imageFunctionInlined,
                                   &currentBudget,
                                   &functionRemoved);
        }
    }
    while (imageFunctionInlined);

    isCTSInline = Optimizer->isCTSInline;

    /* Rebuild flow graph or reconstruct optimizer. */
    if (functionRemoved)
    {
        if (savedShaderTempCount != Optimizer->shader->_tempRegCount)
        {
            gcmVERIFY_OK(gcOpt_ReconstructOptimizer(
                                Optimizer->shader,
                                OptimizerPtr
                                ));
            Optimizer = *OptimizerPtr;
        }
        else
        {
            /* Rebuild flow graph. */
            gcmVERIFY_OK(gcOpt_RebuildFlowGraph(Optimizer));
        }

        Optimizer->isCTSInline = isCTSInline;

        /* Code optimzed. */
        DUMP_OPTIMIZER("Inline functions in the shader", Optimizer);
        gcmFOOTER_ARG("status=%d", gcvSTATUS_CHANGED);
        return gcvSTATUS_CHANGED;
    }
    else
    {
        /* Code unchanged. */
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
}

#define TREAT_DP_AS_ANYP 1
void
gcOpt_UpdatePrecision(
    IN gcOPTIMIZER  Optimizer
    )
{
#if DX_SHADER
    return;
#else
    gctUINT i;
    gcOPT_TEMP tempArray = Optimizer->tempArray;
    gcOPT_CODE code;

    gctUINT         dual16PrecisionRule = gcmOPT_DualFP16PrecisionRule();

    if (Optimizer->shader->type != gcSHADER_TYPE_FRAGMENT ||
        gcShaderIsDesktopGL(Optimizer->shader) /* OpenGL doesn't care precision */)
    {
        return;
    }

    for (i = 0; i < Optimizer->functionCount; i++)
    {
        gcOPT_FUNCTION function = Optimizer->functionArray + i;
        gctUINT j;

        for(j = 0; j < function->argumentCount; j++)
        {
            if(function->arguments[j].precision == gcSHADER_PRECISION_ANY
#if TREAT_DP_AS_ANYP
               || function->arguments[j].precision == gcSHADER_PRECISION_DEFAULT
#endif
               )
            {
                function->arguments[j].precision = gcSHADER_PRECISION_HIGH;
                tempArray[function->arguments[j].index].precision = gcSHADER_PRECISION_HIGH;
            }
            else
            {
                tempArray[function->arguments[j].index].precision = function->arguments[j].precision;
            }
        }
    }

    code = Optimizer->codeHead;
    while(code != gcvNULL)
    {
        gcSL_INSTRUCTION inst = &code->instruction;
        gcSL_TYPE type0 = gcmSL_SOURCE_GET(inst->source0, Type);
        gcSL_TYPE type1 = gcmSL_SOURCE_GET(inst->source1, Type);

#if !TREAT_DP_AS_ANYP
        gcmASSERT(gcmSL_SOURCE_GET(inst->source0, Precision) != gcSHADER_PRECISION_DEFAULT);
#endif
        /* Only process valid source0. */
        if (type0 != gcSL_NONE)
        {
            if(type0 == gcSL_CONSTANT)
            {
                if (dual16PrecisionRule & Dual16_PrecisionRule_IMMED_HP)
                {
                    inst->source0 = gcmSL_SOURCE_SET(inst->source0, Precision, gcSHADER_PRECISION_HIGH);
                }
                else if (dual16PrecisionRule & Dual16_PrecisionRule_IMMED_MP)
                {
                    inst->source0 = gcmSL_SOURCE_SET(inst->source0, Precision, gcSHADER_PRECISION_MEDIUM);
                }
                else
                {
                    /* Assemble the 32-bit value. */
                    gctUINT32 value = inst->source0Index | (inst->source0Indexed << 16);

                    switch (gcmSL_SOURCE_GET(inst->source0, Format))
                    {
                        case gcSL_FLOAT:
                            if(CAN_EXACTLY_CVT_S23E8_2_S10E5(value))
                            {
                                inst->source0 = gcmSL_SOURCE_SET(inst->source0, Precision, gcSHADER_PRECISION_MEDIUM);
                            }
                            else
                            {
                                inst->source0 = gcmSL_SOURCE_SET(inst->source0, Precision, gcSHADER_PRECISION_HIGH);
                            }
                            break;

                        case gcSL_INTEGER:
                            if(CAN_EXACTLY_CVT_S32_2_S16((gctINT)value))
                            {
                                inst->source0 = gcmSL_SOURCE_SET(inst->source0, Precision, gcSHADER_PRECISION_MEDIUM);
                            }
                            else
                            {
                                inst->source0 = gcmSL_SOURCE_SET(inst->source0, Precision, gcSHADER_PRECISION_HIGH);
                            }
                            break;

                        case gcSL_UINT32:
                            if(CAN_EXACTLY_CVT_U32_2_U16(value))
                            {
                                inst->source0 = gcmSL_SOURCE_SET(inst->source0, Precision, gcSHADER_PRECISION_MEDIUM);
                            }
                            else
                            {
                                inst->source0 = gcmSL_SOURCE_SET(inst->source0, Precision, gcSHADER_PRECISION_HIGH);
                            }
                            break;

                        case gcSL_BOOLEAN:
                            inst->source0 = gcmSL_SOURCE_SET(inst->source0, Precision, gcSHADER_PRECISION_MEDIUM);
                            break;
                        default:
                            gcmASSERT(0);
                    }
                }
            }
            else if(type0 == gcSL_SAMPLER && gcmSL_SOURCE_GET(inst->source0, Indexed))
            {
                gcmASSERT(tempArray[inst->source0Indexed].precision != gcSHADER_PRECISION_ANY &&
                          tempArray[inst->source0Indexed].precision != gcSHADER_PRECISION_DEFAULT);

                inst->source0 = gcmSL_SOURCE_SET(inst->source0, Precision, tempArray[inst->source0Indexed].precision);
            }
            else if(type0 == gcSL_TEMP &&
                    (gcmSL_SOURCE_GET(inst->source0, Precision) == gcSHADER_PRECISION_ANY
#if TREAT_DP_AS_ANYP
                     || gcmSL_SOURCE_GET(inst->source0, Precision) == gcSHADER_PRECISION_DEFAULT
#endif
                     || (gctUINT)tempArray[inst->source0Index].precision > (gctUINT)gcmSL_SOURCE_GET(inst->source0, Precision)
                    ))
            {
                gcmASSERT(tempArray[inst->source0Index].precision != gcSHADER_PRECISION_ANY &&
                          tempArray[inst->source0Index].precision != gcSHADER_PRECISION_DEFAULT);

                inst->source0 = gcmSL_SOURCE_SET(inst->source0, Precision, tempArray[inst->source0Index].precision);
            }
        }

        /* Only process valid source1. */
        if (type1 != gcSL_NONE)
        {
#if !TREAT_DP_AS_ANYP
            gcmASSERT(gcmSL_SOURCE_GET(inst->source1, Precision) != gcSHADER_PRECISION_DEFAULT);
#endif
            if(type1 == gcSL_CONSTANT)
            {
                if (dual16PrecisionRule & Dual16_PrecisionRule_IMMED_HP)
                {
                    inst->source1 = gcmSL_SOURCE_SET(inst->source1, Precision, gcSHADER_PRECISION_HIGH);
                }
                else if (dual16PrecisionRule & Dual16_PrecisionRule_IMMED_MP)
                {
                    inst->source1 = gcmSL_SOURCE_SET(inst->source1, Precision, gcSHADER_PRECISION_MEDIUM);
                }
                else
                {
                    /* Assemble the 32-bit value. */
                    gctUINT32 value = inst->source1Index | (inst->source1Indexed << 16);

                    switch (gcmSL_SOURCE_GET(inst->source1, Format))
                    {
                        case gcSL_FLOAT:
                            if(CAN_EXACTLY_CVT_S23E8_2_S10E5(value))
                            {
                                inst->source1 = gcmSL_SOURCE_SET(inst->source1, Precision, gcSHADER_PRECISION_MEDIUM);
                            }
                            else
                            {
                                inst->source1 = gcmSL_SOURCE_SET(inst->source1, Precision, gcSHADER_PRECISION_HIGH);
                            }
                            break;

                        case gcSL_INTEGER:
                            if(CAN_EXACTLY_CVT_S32_2_S16((gctINT)value))
                            {
                                inst->source1 = gcmSL_SOURCE_SET(inst->source1, Precision, gcSHADER_PRECISION_MEDIUM);
                            }
                            else
                            {
                                inst->source1 = gcmSL_SOURCE_SET(inst->source1, Precision, gcSHADER_PRECISION_HIGH);
                            }
                            break;

                        case gcSL_UINT32:
                            if(CAN_EXACTLY_CVT_U32_2_U16(value))
                            {
                                inst->source1 = gcmSL_SOURCE_SET(inst->source1, Precision, gcSHADER_PRECISION_MEDIUM);
                            }
                            else
                            {
                                inst->source1 = gcmSL_SOURCE_SET(inst->source1, Precision, gcSHADER_PRECISION_HIGH);
                            }
                            break;

                        case gcSL_BOOLEAN:
                            inst->source1 = gcmSL_SOURCE_SET(inst->source1, Precision, gcSHADER_PRECISION_MEDIUM);
                            break;
                        default:
                            gcmASSERT(0);
                    }
                }
            }
            else if(type1 == gcSL_TEMP &&
                    (gcmSL_SOURCE_GET(inst->source1, Precision) == gcSHADER_PRECISION_ANY
#if TREAT_DP_AS_ANYP
                     || gcmSL_SOURCE_GET(inst->source1, Precision) == gcSHADER_PRECISION_DEFAULT
#endif
                     || (gctUINT)tempArray[inst->source1Index].precision > (gctUINT)gcmSL_SOURCE_GET(inst->source1, Precision)
                    ))
            {
                gcmASSERT(tempArray[inst->source1Index].precision != gcSHADER_PRECISION_ANY &&
                          tempArray[inst->source1Index].precision != gcSHADER_PRECISION_DEFAULT);

                inst->source1 = gcmSL_SOURCE_SET(inst->source1, Precision, tempArray[inst->source1Index].precision);
            }
        }

        if(gcSL_isOpcodeUseTargetAsSource(gcmSL_OPCODE_GET(inst->opcode, Opcode)))
        {
            if(gcmSL_TARGET_GET(inst->temp, Precision) == gcSHADER_PRECISION_ANY
#if TREAT_DP_AS_ANYP
               || gcmSL_TARGET_GET(inst->temp, Precision) == gcSHADER_PRECISION_DEFAULT
#endif
              )
            {
                gcmASSERT(tempArray[inst->tempIndex].precision != gcSHADER_PRECISION_ANY &&
                          tempArray[inst->tempIndex].precision != gcSHADER_PRECISION_DEFAULT);

                inst->temp = gcmSL_TARGET_SET(inst->temp, Precision, tempArray[inst->tempIndex].precision);
            }
        }
        else if(gcvOpcodeAttr[gcmSL_OPCODE_GET(inst->opcode, Opcode)].resultPrecision != _gcSL_OPCODE_ATTR_RESULT_PRECISION_INVALID) /* dest should be taken care of */
        {
            if(gcmSL_TARGET_GET(inst->temp, Precision) == gcSHADER_PRECISION_ANY
#if TREAT_DP_AS_ANYP
               || gcmSL_TARGET_GET(inst->temp, Precision) == gcSHADER_PRECISION_DEFAULT
#endif
              )
            {
                gcSHADER_PRECISION expPrecision = gcSHADER_PRECISION_DEFAULT;

#if !TREAT_DP_AS_ANYP
                gcmSL_TARGET_GET(inst->temp, Precision) != gcSHADER_PRECISION_DEFAULT);
#endif
                switch(gcvOpcodeAttr[gcmSL_OPCODE_GET(inst->opcode, Opcode)].resultPrecision)
                {
                    case _gcSL_OPCODE_ATTR_RESULT_PRECISION_INVALID:
                        gcmASSERT(0);
                        break;
                    case _gcSL_OPCODE_ATTR_RESULT_PRECISION_HIA:
                    {
                        gcSHADER_PRECISION precision0 = gcmSL_SOURCE_GET(inst->source0, Precision);
                        gcSHADER_PRECISION precision1 = gcmSL_SOURCE_GET(inst->source1, Precision);

                        /* Set precision to highp for a default precision attribute. */
                        if (type0 == gcSL_ATTRIBUTE && precision0 == gcSHADER_PRECISION_DEFAULT)
                        {
                            precision0 = gcSHADER_PRECISION_HIGH;
                        }

                        if (type1 == gcSL_ATTRIBUTE && precision1 == gcSHADER_PRECISION_DEFAULT)
                        {
                            precision1 = gcSHADER_PRECISION_HIGH;
                        }

                        gcmASSERT(type0 != gcSL_NONE && type1 != gcSL_NONE);
                        gcmASSERT(precision0 != gcSHADER_PRECISION_ANY && precision1 != gcSHADER_PRECISION_ANY);
#if !TREAT_DP_AS_ANYP
                        gcmASSERT(precision0 != gcSHADER_PRECISION_DEFAULT && precision1 != gcSHADER_PRECISION_DEFAULT);
#endif
                        expPrecision = precision0 > precision1 ? precision0 : precision1;
                        break;
                    }
                    case _gcSL_OPCODE_ATTR_RESULT_PRECISION_AF:
                    {
                        gcSHADER_PRECISION precision0 = gcmSL_SOURCE_GET(inst->source0, Precision);

                        /* Set precision to highp for a default precision attribute. */
                        if (type0 == gcSL_ATTRIBUTE && precision0 == gcSHADER_PRECISION_DEFAULT)
                        {
                            precision0 = gcSHADER_PRECISION_HIGH;
                        }

                        gcmASSERT(type0 != gcSL_NONE);
                        gcmASSERT(precision0 != gcSHADER_PRECISION_ANY);
#if !TREAT_DP_AS_ANYP
                        gcmASSERT(precision0 != gcSHADER_PRECISION_DEFAULT);
#endif
                        expPrecision = precision0;
                        break;
                    }
                    case _gcSL_OPCODE_ATTR_RESULT_PRECISION_AS:
                    {
                        gcSHADER_PRECISION precision1 = gcmSL_SOURCE_GET(inst->source1, Precision);

                        /* Set precision to highp for a default precision attribute. */
                        if (type1 == gcSL_ATTRIBUTE && precision1 == gcSHADER_PRECISION_DEFAULT)
                        {
                            precision1 = gcSHADER_PRECISION_HIGH;
                        }

                        gcmASSERT(type1 != gcSL_NONE);
                        gcmASSERT(precision1 != gcSHADER_PRECISION_ANY);
#if !TREAT_DP_AS_ANYP
                        gcmASSERT(precision1 != gcSHADER_PRECISION_DEFAULT);
#endif
                        expPrecision = precision1;
                        break;
                    }
                    case _gcSL_OPCODE_ATTR_RESULT_PRECISION_HP:
                    {
                        expPrecision = gcSHADER_PRECISION_HIGH;
                        break;
                    }
                    case _gcSL_OPCODE_ATTR_RESULT_PRECISION_MP:
                    {
                        expPrecision = gcSHADER_PRECISION_MEDIUM;
                        break;
                    }
                    case _gcSL_OPCODE_ATTR_RESULT_PRECISION_LP:
                    {
                        expPrecision = gcSHADER_PRECISION_LOW;
                        break;
                    }
                    case _gcSL_OPCODE_ATTR_RESULT_PRECISION_IGNORE:
                        expPrecision = gcSHADER_PRECISION_LOW;
                        break;
                    default:
                        gcmASSERT(0);
                }
                gcmASSERT(expPrecision != gcSHADER_PRECISION_DEFAULT && expPrecision != gcSHADER_PRECISION_ANY);
                if(tempArray[inst->tempIndex].precision && tempArray[inst->tempIndex].precision != gcSHADER_PRECISION_ANY)
                {
                    if((gctUINT)tempArray[inst->tempIndex].precision >= (gctUINT)expPrecision)
                    {
                        /* precision need to be promoted */
                        inst->temp = gcmSL_TARGET_SET(inst->temp, Precision, tempArray[inst->tempIndex].precision);
                    }
                    else
                    {
                        /* precision conflict, need to redo the precision update */
                        tempArray[inst->tempIndex].precision = expPrecision;
                        code = Optimizer->codeHead;
                        continue;
                    }
                }
                else
                {
                    inst->temp = gcmSL_TARGET_SET(inst->temp, Precision, expPrecision);
                    tempArray[inst->tempIndex].precision = expPrecision;
                }
            }
            else if((gctUINT)tempArray[inst->tempIndex].precision > (gctUINT)gcmSL_TARGET_GET(inst->temp, Precision))
            {
                inst->temp = gcmSL_TARGET_SET(inst->temp, Precision, tempArray[inst->tempIndex].precision);
            }
            tempArray[inst->tempIndex].precision = gcmSL_TARGET_GET(inst->temp, Precision);     /* indexed sampler may need this */
        }
        code = code->next;
    }

    DUMP_OPTIMIZER("Update precision in the shader", Optimizer);
#endif
}
#undef TREAT_DP_AS_ANYP

static gcOPT_FUNCTION
_GetMinimumCodeFunction(
    IN gcOPTIMIZER Optimizer,
    IN gcOPT_FUNCTION Function
    )
{
    gctINT funcCodeCount= 0, minFuncCodeCount = 0;
    gcOPT_FUNCTION function, minFunction;

    minFuncCodeCount = Function->codeTail->id - Function->codeHead->id + 1;
    minFunction = Function;
    function = Function->maxDepthFunc;

    while(function)
    {
        funcCodeCount = function->codeTail->id - function->codeHead->id + 1;
        if (funcCodeCount < minFuncCodeCount)
        {
            minFuncCodeCount = funcCodeCount;
            minFunction = function;
        }
        function = function->maxDepthFunc;
    }
    return minFunction;
}

/*******************************************************************************
**                          gcOpt_OptimizeCallStackDepth
********************************************************************************
**
**  Expand functons that are exceed the max call stack depth.
**
**  INPUT:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_OptimizeCallStackDepth(
    IN gcOPTIMIZER * OptimizerPtr
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT functionRemoved = 0;
    gcOPTIMIZER Optimizer = *OptimizerPtr;
    gctINT                i;
    gcOPT_FUNCTION function, minFunction;
    gcFUNCTION  prevShaderFunc;
    gctUINT inlineLevel = gcmOPT_INLINELEVEL();
    gctUINT inlineDepthComparison  = gcmOPT_INLINEDEPTHCOMP();
    gctUINT inlineFormatConversion = gcmOPT_INLINEFORMATCONV();
    gctINT  currentBudget = (inlineLevel == 4) ? 0x7fffffff
                                               : _GetInlineBudget(Optimizer);
    gctUINT savedShaderTempCount = Optimizer->shader->_tempRegCount;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    /* Update the call stack information. */
    status = gcOpt_UpdateCallStackDepth(Optimizer, gcvFALSE);
    prevShaderFunc = gcvNULL;

    if (!status)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    for (i = Optimizer->functionCount - 1; i >= 0; i--)
    {
        function = Optimizer->functionArray + i;

        /* Make sure we get the right function. */
        if (prevShaderFunc && function->shaderFunction == prevShaderFunc)
            continue;
        prevShaderFunc = function->shaderFunction;

        /* If the depth of call stack on a function is larget the limitation,
        ** expand the minimum function first so we can expand as many functions as possible.
        */
        while(function->maxDepthValue + 1 >= _MAX_CALL_STACK_DEPTH_)
        {
            minFunction = _GetMinimumCodeFunction(Optimizer, function);
            status = _InlineSinglelFunction(Optimizer, minFunction, inlineDepthComparison, inlineFormatConversion,
                                        inlineLevel, gcvFALSE, gcvTRUE, gcvNULL, &currentBudget, &functionRemoved);
            if (status)
            {
                gcOpt_UpdateCallStackDepth(Optimizer, gcvTRUE);
                i = Optimizer->functionCount;
            }
            break;
        }
    }

    if (savedShaderTempCount != Optimizer->shader->_tempRegCount)
    {
        gcmVERIFY_OK(gcOpt_ReconstructOptimizer(
                            Optimizer->shader,
                            OptimizerPtr
                            ));
        Optimizer = *OptimizerPtr;
    }
    else
    {
        /* Rebuild flow graph. */
        gcmVERIFY_OK(gcOpt_RebuildFlowGraph(Optimizer));
    }

    DUMP_OPTIMIZER("Inline functions whose call stack depth is larget than the max value.", Optimizer);
    gcmFOOTER_ARG("status=%d", gcvSTATUS_CHANGED);
    return gcvSTATUS_CHANGED;
}


/*******************************************************************************
**                          gcOpt_OptimizeIntrinsics
********************************************************************************
**
**  Optimize intrinsics functon calls with specialized instance:
**    shuffle/shuffle:  if the mask is constant, replace it with move
**
**  INPUT:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_OptimizeIntrinsics(
    IN gcOPTIMIZER Optimizer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    return status;
}

static gceSTATUS _EvaluateConstantInstruction(
    IN gcOPTIMIZER Optimizer,
    IN gcOPT_CODE Code,
    OUT gctSOURCE_t *SourceUint,
    OUT gctUINT32 * SourceIndex,
    OUT gctUINT16 * SourceIndexed,
    OUT gctBOOL * NeedToPropagate
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcOPT_TEMP tempArray = Optimizer->tempArray;
    gcVARIABLE variable;
    gcOPT_CODE code = Code;
    gctSOURCE_t source, source1;
    gctSOURCE_t sourceUint = 0;
    gctUINT32 sourceIndex = 0;
    gctUINT16 sourceIndexed = 0;
    gctUINT32 value = 0, value0, value1;
    gcSL_FORMAT format0, format1, targetFormat;
    gctBOOL needToPropagate = gcvFALSE;
    gcSL_OPCODE opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->instruction.opcode, Opcode);
    gcSL_FORMAT format = (gcSL_FORMAT)gcmSL_TARGET_GET(code->instruction.temp, Format);

    switch (opcode)
    {
        case gcSL_MOV:
            /* Skip if dest is volatile. */
            variable = tempArray[code->instruction.tempIndex].arrayVariable;
            if (variable && (variable->qualifier & gcvTYPE_QUALIFIER_VOLATILE)) break;

            source = code->instruction.source0;
            if (gcmSL_SOURCE_GET(source, Type) == gcSL_CONSTANT &&
                (gcSL_FORMAT) gcmSL_SOURCE_GET(source, Format) == format)
            {
                gcmASSERT(code->dependencies0 == gcvNULL);
                needToPropagate = gcvTRUE;
                sourceUint      = code->instruction.source0;
                sourceIndex     = code->instruction.source0Index;
                sourceIndexed   = code->instruction.source0Indexed;
            }
            break;

        case gcSL_SAT:
        case gcSL_RCP:
        case gcSL_RSQ:
        case gcSL_LOG:
        case gcSL_FRAC:
        case gcSL_FLOOR:
        case gcSL_CEIL:
        case gcSL_I2F:
        case gcSL_F2I:
            /* Skip if dest is volatile. */
            variable = tempArray[code->instruction.tempIndex].arrayVariable;
            if (variable && (variable->qualifier & gcvTYPE_QUALIFIER_VOLATILE)) break;

            /* One-operand float computational instruction. */
            source = code->instruction.source0;
            if (gcmSL_SOURCE_GET(source, Type) != gcSL_CONSTANT) break;
            gcmASSERT(code->dependencies0 == gcvNULL);
            format0 = (gcSL_FORMAT) gcmSL_SOURCE_GET(source, Format);

            /* Assemble the 32-bit value. */
            value0 = (code->instruction.source0Index & 0xFFFF) | (code->instruction.source0Indexed << 16);
            sourceUint = code->instruction.source0;
            sourceUint = gcmSL_SOURCE_SET(code->instruction.source0, Format, format);

            if (opcode != gcSL_I2F && opcode != gcSL_F2I)
            {
                union
                {
                    gctFLOAT f;
                    gctINT32 hex32;
                }
                uValue;
                gctFLOAT f, f0;

                uValue.hex32 = value0;

                if (format == gcSL_FLOAT)
                {
                    f = f0 = gcoMATH_UIntAsFloat(uValue.hex32);

                    switch (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode))
                    {
                    case gcSL_SAT:
                        f = f0 > 1.0f ? 1.0f : f < 0.0f ? 0.0f : f0;
                        break;
                    case gcSL_RCP:
                        f = gcoMATH_Divide(1.0f, f0);
                        break;
                    case gcSL_RSQ:
                        f = gcoMATH_ReciprocalSquareRoot(f0);
                        break;
                    case gcSL_LOG:
                        f = gcoMATH_Log2(f0);
                        break;
                    case gcSL_FRAC:
                        f = gcoMATH_Add(f0, -gcoMATH_Floor(f0));
                        break;
                    case gcSL_FLOOR:
                        f = gcoMATH_Floor(f0);
                        break;
                    case gcSL_CEIL:
                        f = gcoMATH_Ceiling(f0);
                        break;
                    default:
                        gcmASSERT(gcvFALSE);
                        break;
                    }
                    value = gcoMATH_FloatAsUInt(f);
                }
                else
                {

                }
                sourceIndex = value & 0xFFFF;
                sourceIndexed = (value >> 16) & 0xFFFF;

                needToPropagate = gcvTRUE;
            }
            else
            {
                union
                {
                    gctFLOAT f;
                    gctINT32 hex32;
                }
                value;

                if (opcode != gcSL_I2F && opcode != gcSL_F2I)
                {
                    gcmASSERT(format0 == gcSL_FLOAT);
                }

                if (opcode == gcSL_I2F)
                {
                    if(format0 == gcSL_INT8 ||
                        format0 == gcSL_INT16 ||
                        format0 == gcSL_INT32 ||
                        format0 == gcSL_BOOLEAN)
                    {
                        value.f = (gctFLOAT)(gctINT)(value0);
                    }
                    else
                    {
                        gcmASSERT(format0 == gcSL_UINT8 ||
                            format0 == gcSL_UINT16 ||
                            format0 == gcSL_UINT32);
                        value.f = (gctFLOAT)(gctUINT)(value0);
                    }
                }
                else
                {
                    gcmASSERT(opcode == gcSL_F2I);
                    value.hex32 = (gctINT)(code->instruction.source0Index |
                                           (code->instruction.source0Indexed << 16));
                    value.hex32 = (gctINT)(value.f);
                }

                sourceIndex = (gctUINT16)(value.hex32 & 0xFFFF);
                sourceIndexed = (gctUINT16)(value.hex32 >> 16);
                needToPropagate = gcvTRUE;
            }
            break;

        case gcSL_ABS:
            /* Skip if dest is volatile. */
            variable = tempArray[code->instruction.tempIndex].arrayVariable;
            if (variable && (variable->qualifier & gcvTYPE_QUALIFIER_VOLATILE)) break;

            /* One-operand computational instruction. */
            source = code->instruction.source0;
            if (gcmSL_SOURCE_GET(source, Type) != gcSL_CONSTANT) break;
            gcmASSERT(code->dependencies0 == gcvNULL);
            format0 = (gcSL_FORMAT) gcmSL_SOURCE_GET(source, Format);

            /* Assemble the 32-bit value. */
            value0 = (code->instruction.source0Index & 0xFFFF) | (code->instruction.source0Indexed << 16);

            if (format0 == gcSL_FLOAT)
            {
                gctFLOAT f, f0;

                f = f0 = gcoMATH_UIntAsFloat(value0);
                sourceUint = code->instruction.source0;

                f = f0 > 0.0f ? f0 : -f0;
                value = gcoMATH_FloatAsUInt(f);
                sourceIndex = value & 0xFFFF;
                sourceIndexed = (value >> 16) & 0xFFFF;

                needToPropagate = gcvTRUE;
            }
            else if (format0 == gcSL_INTEGER ||
                     format0 == gcSL_INT8 || format0 == gcSL_INT16)
            {
                gctINT i, i0;

                i = i0 = *(gctINT *) &value0;
                sourceUint = code->instruction.source0;

                i = gcmABS(i0);
                value = *(gctUINT *) &i;
                sourceIndex = value & 0xFFFF;
                sourceIndexed = (value >> 16) & 0xFFFF;

                needToPropagate = gcvTRUE;
            }
            else if (format0 == gcSL_UINT32 ||
                     format0 == gcSL_UINT8 || format0 == gcSL_UINT16)
            {
                value = (gctINT)value0 >= 0 ? value0 : (gctUINT)(-(gctINT)value0);
                sourceUint = code->instruction.source0;
                sourceIndex = value & 0xFFFF;
                sourceIndexed = (value >> 16) & 0xFFFF;

                needToPropagate = gcvTRUE;
            }
            else
            {
                /* Error. */
                return gcvSTATUS_INVALID_DATA;
            }
            break;

        /* Two-operand integer computational instruction. */
        case gcSL_AND_BITWISE:
        case gcSL_OR_BITWISE:
        case gcSL_XOR_BITWISE:
        case gcSL_LSHIFT:
        case gcSL_RSHIFT:
            /* Skip if dest is volatile. */
            variable = tempArray[code->instruction.tempIndex].arrayVariable;
            if (variable && (variable->qualifier & gcvTYPE_QUALIFIER_VOLATILE)) break;

            source  = code->instruction.source0;
            format0 = (gcSL_FORMAT) gcmSL_SOURCE_GET(source, Format);
            source1 = code->instruction.source1;
            format1 = (gcSL_FORMAT) gcmSL_SOURCE_GET(source1, Format);

            if ((gcmSL_SOURCE_GET(source, Type) != (gctUINT16) gcSL_CONSTANT)
            ||  (gcmSL_SOURCE_GET(source1, Type) != (gctUINT16) gcSL_CONSTANT)
            )
            {
                break;
            }

            /* Assemble the 32-bit value. */
            value0 = (code->instruction.source0Index & 0xFFFF) | (code->instruction.source0Indexed << 16);
            value1 = (code->instruction.source1Index & 0xFFFF) | (code->instruction.source1Indexed << 16);

            switch (opcode)
            {
            case gcSL_AND_BITWISE:
                value = value0 & value1;
                break;

            case gcSL_OR_BITWISE:
                value = value0 | value1;
                break;

            case gcSL_XOR_BITWISE:
                value = value0 ^ value1;
                break;

            case gcSL_LSHIFT:
                value = value0 << value1;
                break;

            case gcSL_RSHIFT:
                value = value0 >> value1;
                break;

            default:
                gcmASSERT(gcvFALSE);
                break;
            }

            sourceUint = code->instruction.source0;
            sourceIndex = value & 0xFFFF;
            sourceIndexed = (value >> 16) & 0xFFFF;
            needToPropagate = gcvTRUE;
            break;

        /* One-operand integer computational instruction. */
        case gcSL_NOT_BITWISE:
            /* Skip if dest is volatile. */
            variable = tempArray[code->instruction.tempIndex].arrayVariable;
            if (variable && (variable->qualifier & gcvTYPE_QUALIFIER_VOLATILE)) break;

            source  = code->instruction.source0;
            format0 = (gcSL_FORMAT) gcmSL_SOURCE_GET(source, Format);

            if (gcmSL_SOURCE_GET(source, Type) != (gctUINT16) gcSL_CONSTANT)
            {
                break;
            }

            /* Assemble the 32-bit value. */
            value0 = (code->instruction.source0Index & 0xFFFF) | (code->instruction.source0Indexed << 16);

            switch (opcode)
            {
            case gcSL_NOT_BITWISE:
                value = ~value0;
                break;

            default:
                gcmASSERT(gcvFALSE);
                break;
            }

            sourceUint = code->instruction.source0;
            sourceIndex = value & 0xFFFF;
            sourceIndexed = (value >> 16) & 0xFFFF;
            needToPropagate = gcvTRUE;
            break;

        case gcSL_ROTATE:
            /* Two-operand integer computational instruction. */
            break;

        case gcSL_POW:
            /* Two-operand float computational instruction. */

        case gcSL_ADD:
        case gcSL_SUB:
        case gcSL_MUL:
        case gcSL_MAX:
        case gcSL_MIN:
        case gcSL_MOD:
        case gcSL_DIV:
            /* Skip if dest is volatile. */
            variable = tempArray[code->instruction.tempIndex].arrayVariable;
            if (variable && (variable->qualifier & gcvTYPE_QUALIFIER_VOLATILE)) break;

            /* Two-operand computational instruction. */
            source  = code->instruction.source0;
            format0 = (gcSL_FORMAT) gcmSL_SOURCE_GET(source, Format);
            source1 = code->instruction.source1;
            format1 = (gcSL_FORMAT) gcmSL_SOURCE_GET(source1, Format);

            if ((gcmSL_SOURCE_GET(source, Type) != (gctUINT16) gcSL_CONSTANT)
            &&  (gcmSL_SOURCE_GET(source1, Type) != (gctUINT16) gcSL_CONSTANT)
            )
            {
                break;
            }

            if ((gcmSL_SOURCE_GET(source, Type) == gcSL_CONSTANT) &&
                (gcmSL_SOURCE_GET(source1, Type) == gcSL_CONSTANT))
            {
                needToPropagate = gcvTRUE;
            }

            /* Assemble the 32-bit value. */
            value0 = (code->instruction.source0Index & 0xFFFF) | (code->instruction.source0Indexed << 16);
            value1 = (code->instruction.source1Index & 0xFFFF) | (code->instruction.source1Indexed << 16);

            if (gcmSL_SOURCE_GET(source, Type) != gcSL_CONSTANT || gcmSL_SOURCE_GET(source1, Type) != gcSL_CONSTANT)
            {
                if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_ADD)
                {
                    if (gcmSL_SOURCE_GET(source, Type) == gcSL_CONSTANT)
                    {
                        if (format0 == gcSL_FLOAT)
                        {
                            float f = gcoMATH_UIntAsFloat(value0);
                            if (f != 0.0f) break;

                            /* It is equivalent to MOV instruction. */
                            gcmSL_OPCODE_UPDATE(code->instruction.opcode, Opcode, gcSL_MOV);
                            code->instruction.source0        = code->instruction.source1;
                            code->instruction.source0Index   = code->instruction.source1Index;
                            code->instruction.source0Indexed = code->instruction.source1Indexed;
                            code->instruction.source1        = 0;
                            code->instruction.source1Index   = 0;
                            code->instruction.source1Indexed = 0;
                        }
                        else /*if (format0 == gcSL_INTEGER)*/
                        {
                            gctINT i = *(gctINT *) &value0;
                            if (i != 0) break;

                            /* It is equivalent to MOV instruction. */
                            gcmSL_OPCODE_UPDATE(code->instruction.opcode, Opcode, gcSL_MOV);
                            code->instruction.source0        = code->instruction.source1;
                            code->instruction.source0Index   = code->instruction.source1Index;
                            code->instruction.source0Indexed = code->instruction.source1Indexed;
                            code->instruction.source1        = 0;
                            code->instruction.source1Index   = 0;
                            code->instruction.source1Indexed = 0;
                        }

                        /* Update dataFlow. */
                        gcmASSERT(code->dependencies0 == gcvNULL);
                        code->dependencies0 = code->dependencies1;
                        code->dependencies1 = gcvNULL;
                    }
                    else
                    {
                        if (format1 == gcSL_FLOAT)
                        {
                            float f = gcoMATH_UIntAsFloat(value1);
                            if (f != 0.0f) break;

                            /* It is equivalent to MOV instruction. */
                            gcmSL_OPCODE_UPDATE(code->instruction.opcode, Opcode, gcSL_MOV);
                            code->instruction.source1        = 0;
                            code->instruction.source1Index   = 0;
                            code->instruction.source1Indexed = 0;
                        }
                        else /*if (format1 == gcSL_INTEGER)*/
                        {
                            gctINT i = *(gctINT *) &value1;
                            if (i != 0) break;

                            /* It is equivalent to MOV instruction. */
                            gcmSL_OPCODE_UPDATE(code->instruction.opcode, Opcode, gcSL_MOV);
                            code->instruction.source1        = 0;
                            code->instruction.source1Index   = 0;
                            code->instruction.source1Indexed = 0;
                        }
                    }

                    /* The result is not constant. */
                    break;
                }
                else if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_MUL)
                {
                    if (gcmSL_SOURCE_GET(source, Type) == gcSL_CONSTANT)
                    {
                        if (format0 == gcSL_FLOAT)
                        {
                            float f = gcoMATH_UIntAsFloat(value0);
                            if (f == 0.0f)
                            {
                                /* The result is 0. */
                                sourceUint = code->instruction.source0;
                                value = gcoMATH_FloatAsUInt(f);
                                sourceIndex = value & 0xFFFF;
                                sourceIndexed = (value >> 16) & 0xFFFF;

                                needToPropagate = gcvTRUE;

                                /* Remove dependencies1. */
                                gcmASSERT(code->dependencies0 == gcvNULL);
                                if (code->dependencies1)
                                {
                                    gcmVERIFY_OK(gcOpt_DestroyCodeDependency(Optimizer,
                                                        &code->dependencies1,
                                                        code));
                                }
                            }
                            else if (f == 1.0f)
                            {
                                /* It is equivalent to MOV instruction. */
                                gcmSL_OPCODE_UPDATE(code->instruction.opcode, Opcode, gcSL_MOV);
                                code->instruction.source0        = code->instruction.source1;
                                code->instruction.source0Index   = code->instruction.source1Index;
                                code->instruction.source0Indexed = code->instruction.source1Indexed;
                                code->instruction.source1        = 0;
                                code->instruction.source1Index   = 0;
                                code->instruction.source1Indexed = 0;

                                /* Update dataFlow. */
                                gcmASSERT(code->dependencies0 == gcvNULL);
                                code->dependencies0 = code->dependencies1;
                                code->dependencies1 = gcvNULL;
                            }
                        }
                        else /*if (format0 == gcSL_INTEGER)*/
                        {
                            gctINT i = *(gctINT *) &value0;
                            if (i == 0)
                            {
                                /* The result is 0. */
                                sourceUint = code->instruction.source0;
                                sourceIndex = value0 & 0xFFFF;
                                sourceIndexed = (value0 >> 16) & 0xFFFF;

                                needToPropagate = gcvTRUE;

                                /* Remove dependencies1. */
                                gcmASSERT(code->dependencies0 == gcvNULL);
                                if (code->dependencies1)
                                {
                                    gcmVERIFY_OK(gcOpt_DestroyCodeDependency(Optimizer,
                                                        &code->dependencies1,
                                                        code));
                                }
                            }
                            else if (i == 1)
                            {
                                /* It is equivalent to MOV instruction. */
                                gcmSL_OPCODE_UPDATE(code->instruction.opcode, Opcode, gcSL_MOV);
                                code->instruction.source0        = code->instruction.source1;
                                code->instruction.source0Index   = code->instruction.source1Index;
                                code->instruction.source0Indexed = code->instruction.source1Indexed;
                                code->instruction.source1        = 0;
                                code->instruction.source1Index   = 0;
                                code->instruction.source1Indexed = 0;

                                /* Update dataFlow. */
                                gcmASSERT(code->dependencies0 == gcvNULL);
                                code->dependencies0 = code->dependencies1;
                                code->dependencies1 = gcvNULL;
                            }
                        }
                    }
                    else
                    {
                        if (format1 == gcSL_FLOAT)
                        {
                            float f = gcoMATH_UIntAsFloat(value1);
                            if (f == 0.0f)
                            {
                                /* The result is 0. */
                                sourceUint = code->instruction.source1;
                                value = gcoMATH_FloatAsUInt(f);
                                sourceIndex = value & 0xFFFF;
                                sourceIndexed = (value >> 16) & 0xFFFF;

                                needToPropagate = gcvTRUE;

                                /* Remove dependencies0. */
                                gcmASSERT(code->dependencies1 == gcvNULL);
                                if (code->dependencies0)
                                {
                                    gcmVERIFY_OK(gcOpt_DestroyCodeDependency(Optimizer,
                                                        &code->dependencies0,
                                                        code));
                                }
                            }
                            else if (f == 1.0f)
                            {
                                /* It is equivalent to MOV instruction. */
                                gcmSL_OPCODE_UPDATE(code->instruction.opcode, Opcode, gcSL_MOV);
                                code->instruction.source1        = 0;
                                code->instruction.source1Index   = 0;
                                code->instruction.source1Indexed = 0;

                                gcmASSERT(code->dependencies1 == gcvNULL);
                            }
                        }
                        else /*if (format1 == gcSL_INTEGER)*/
                        {
                            gctINT i = *(gctINT *) &value1;
                            if (i == 0)
                            {
                                /* The result is 0. */
                                sourceUint = code->instruction.source1;
                                sourceIndex = value1 & 0xFFFF;
                                sourceIndexed = (value1 >> 16) & 0xFFFF;

                                needToPropagate = gcvTRUE;

                                /* Remove dependencies0. */
                                gcmASSERT(code->dependencies1 == gcvNULL);
                                if (code->dependencies0)
                                {
                                    gcmVERIFY_OK(gcOpt_DestroyCodeDependency(Optimizer,
                                                        &code->dependencies0,
                                                        code));
                                }
                            }
                            else if (i == 1)
                            {
                                /* It is equivalent to MOV instruction. */
                                gcmSL_OPCODE_UPDATE(code->instruction.opcode, Opcode, gcSL_MOV);
                                code->instruction.source1        = 0;
                                code->instruction.source1Index   = 0;
                                code->instruction.source1Indexed = 0;

                                gcmASSERT(code->dependencies1 == gcvNULL);
                            }
                        }
                    }
                    break;
                }
                else if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_DIV)
                {
                    if (gcmSL_SOURCE_GET(source, Type) == gcSL_CONSTANT)
                    {
                        if (format0 == gcSL_FLOAT)
                        {
                            float f = gcoMATH_UIntAsFloat(value0);
                            if (f == 0.0f)
                            {
                                /* The result is 0. */
                                sourceUint = code->instruction.source0;
                                value = gcoMATH_FloatAsUInt(f);
                                sourceIndex = value & 0xFFFF;
                                sourceIndexed = (value >> 16) & 0xFFFF;

                                needToPropagate = gcvTRUE;

                                /* Remove dependencies1. */
                                gcmASSERT(code->dependencies0 == gcvNULL);
                                if (code->dependencies1)
                                {
                                    gcmVERIFY_OK(gcOpt_DestroyCodeDependency(Optimizer,
                                                        &code->dependencies1,
                                                        code));
                                }
                            }
                        }
                        else /*if (format0 == gcSL_INTEGER)*/
                        {
                            gctINT i = *(gctINT *) &value0;
                            if (i == 0)
                            {
                                /* The result is 0. */
                                sourceUint = code->instruction.source0;
                                sourceIndex = value0 & 0xFFFF;
                                sourceIndexed = (value0 >> 16) & 0xFFFF;

                                needToPropagate = gcvTRUE;

                                /* Remove dependencies1. */
                                gcmASSERT(code->dependencies0 == gcvNULL);
                                if (code->dependencies1)
                                {
                                    gcmVERIFY_OK(gcOpt_DestroyCodeDependency(Optimizer,
                                                        &code->dependencies1,
                                                        code));
                                }
                            }
                        }
                    }
                    else
                    {
                        if (format1 == gcSL_FLOAT)
                        {
                            float f = gcoMATH_UIntAsFloat(value1);
                            if (f == 1.0f)
                            {
                                /* It is equivalent to MOV instruction. */
                                gcmSL_OPCODE_UPDATE(code->instruction.opcode, Opcode, gcSL_MOV);
                                code->instruction.source1        = 0;
                                code->instruction.source1Index   = 0;
                                code->instruction.source1Indexed = 0;

                                gcmASSERT(code->dependencies1 == gcvNULL);
                            }
                        }
                        else /*if (format1 == gcSL_INTEGER)*/
                        {
                            gctINT i = *(gctINT *) &value1;
                            if (i == 1)
                            {
                                /* It is equivalent to MOV instruction. */
                                gcmSL_OPCODE_UPDATE(code->instruction.opcode, Opcode, gcSL_MOV);
                                code->instruction.source1        = 0;
                                code->instruction.source1Index   = 0;
                                code->instruction.source1Indexed = 0;

                                gcmASSERT(code->dependencies1 == gcvNULL);
                            }
                        }
                    }
                    break;
                }
                else if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_SUB)
                {
                    if (gcmSL_SOURCE_GET(source1, Type) != gcSL_CONSTANT) break;

                    if (format1 == gcSL_FLOAT)
                    {
                        float f = gcoMATH_UIntAsFloat(value1);
                        if (f != 0.0f) break;

                        /* It is equivalent to MOV instruction. */
                        gcmSL_OPCODE_UPDATE(code->instruction.opcode, Opcode, gcSL_MOV);
                        code->instruction.source1 = 0;
                        code->instruction.source1Index = 0;
                        code->instruction.source1Indexed = 0;
                    }
                    else /*if (format1 == gcSL_INTEGER)*/
                    {
                        gctINT i = *(gctINT *) &value1;
                        if (i != 0) break;

                        /* It is equivalent to MOV instruction. */
                        gcmSL_OPCODE_UPDATE(code->instruction.opcode, Opcode, gcSL_MOV);
                        code->instruction.source1 = 0;
                        code->instruction.source1Index = 0;
                        code->instruction.source1Indexed = 0;
                    }

                    /* The result is not constant. */
                    break;
                }
                else if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_POW)
                {
                    if (gcmSL_SOURCE_GET(source, Type) == gcSL_CONSTANT)
                    {
                        float f;
                        gcmASSERT(format0 == gcSL_FLOAT);
                        f = gcoMATH_UIntAsFloat(value0);
                        if (f != 0.0f && f != 1.0f) break;

                        /* The result is f. */
                        sourceUint = code->instruction.source0;
                        value = gcoMATH_FloatAsUInt(f);
                        sourceIndex = value & 0xFFFF;
                        sourceIndexed = (value >> 16) & 0xFFFF;

                        needToPropagate = gcvTRUE;

                        /* Remove dependencies1. */
                        if (code->dependencies1)
                        {
                            gcmVERIFY_OK(gcOpt_DestroyCodeDependency(Optimizer,
                                                &code->dependencies1,
                                                code));
                        }
                    }
                    else
                    {
                        float f;
                        gcmASSERT(format1 == gcSL_FLOAT);
                        f = gcoMATH_UIntAsFloat(value1);
                        if (f == 0.0f)
                        {
                            /* The result is 1. */
                            sourceUint = code->instruction.source1;
                            f = 1.0f;
                            value = gcoMATH_FloatAsUInt(f);
                            sourceIndex = value & 0xFFFF;
                            sourceIndexed = (value >> 16) & 0xFFFF;

                            needToPropagate = gcvTRUE;

                            /* Remove dependencies1. */
                            if (code->dependencies1)
                            {
                                gcmVERIFY_OK(gcOpt_DestroyCodeDependency(Optimizer,
                                                    &code->dependencies1,
                                                    code));
                            }
                            if (code->dependencies0)
                            {
                                gcmVERIFY_OK(gcOpt_DestroyCodeDependency(Optimizer,
                                                    &code->dependencies0,
                                                    code));
                            }
                        }
                        else if (f == 1.0f)
                        {
                            /* It is equivalent to MOV instruction. */
                            gcmSL_OPCODE_UPDATE(code->instruction.opcode, Opcode, gcSL_MOV);
                            code->instruction.source1        = 0;
                            code->instruction.source1Index   = 0;
                            code->instruction.source1Indexed = 0;
                        }
                    }
                    break;
                }
                else break;
            }

            if (!needToPropagate)
                break;

            gcmASSERT(code->dependencies0 == gcvNULL &&
                      code->dependencies1 == gcvNULL);

            targetFormat = (gcSL_FORMAT)gcmSL_TARGET_GET(code->instruction.temp, Format);

            if (targetFormat == gcSL_FLOAT)
            {
                float f = 0.0f, f0 = 0.0f, f1;

                if (format0 == gcSL_FLOAT)
                {
                    f0 = gcoMATH_UIntAsFloat(value0);
                    sourceUint = code->instruction.source0;
                }
                else if (format0 == gcSL_INTEGER || format0 == gcSL_UINT32 ||
                         format0 == gcSL_INT16 || format0 == gcSL_UINT16 ||
                         format0 == gcSL_INT8 || format0 == gcSL_UINT8)
                {
                    f0 = (float) value0;
                }
                else
                {
                }

                if (format1 == gcSL_FLOAT)
                {
                    f1 = gcoMATH_UIntAsFloat(value1);
                    sourceUint = code->instruction.source1;
                }
                else if (format1 == gcSL_INTEGER || format1 == gcSL_UINT32 ||
                         format1 == gcSL_INT16 || format1 == gcSL_UINT16 ||
                         format1 == gcSL_INT8 || format1 == gcSL_UINT8)
                {
                    f1 = (float) value1;
                }
                else
                {
                    /* Error. */
                    return gcvSTATUS_INVALID_DATA;
                }

                switch (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode))
                {
                case gcSL_ADD:
                    f = gcoMATH_Add(f0, f1); break;
                case gcSL_SUB:
                    f = gcoMATH_Add(f0, -f1); break;
                case gcSL_MUL:
                    f = gcoMATH_Multiply(f0, f1); break;
                case gcSL_MAX:
                    f = f0 > f1 ? f0 : f1; break;
                case gcSL_MIN:
                    f = f0 < f1 ? f0 : f1; break;
                case gcSL_POW:
                    f = gcoMATH_Power(f0, f1); break;
                case gcSL_DIV:
                    f = gcoMATH_Divide(f0, f1); break;
                default:
                    gcmASSERT(gcvFALSE);
                    break;
                }
                value = gcoMATH_FloatAsUInt(f);
                sourceIndex = value & 0xFFFF;
                sourceIndexed = (value >> 16) & 0xFFFF;

                needToPropagate = gcvTRUE;
            }
            else if ((targetFormat == gcSL_INTEGER || targetFormat == gcSL_UINT32) ||
                      (targetFormat == gcSL_INT16 || targetFormat == gcSL_UINT16) ||
                      (targetFormat == gcSL_INT8 || targetFormat == gcSL_UINT8))
            {
                union {
                    gctUINT32 u32;
                    gctINT32  i32;
                } v0, v1;
                v0.u32 = value0;
                v1.u32 = value1;

                if (format0 == gcSL_INTEGER || format0 == gcSL_INT16 || format0 == gcSL_INT8 ||
                    format0 == gcSL_UINT32 || format0 == gcSL_UINT16 || format0 == gcSL_UINT8)
                {
                    sourceUint = code->instruction.source0;
                }
                else if (format1 == gcSL_INTEGER || format1 == gcSL_INT16 || format1 == gcSL_INT8 ||
                    format1 == gcSL_UINT32 || format1 == gcSL_UINT16 || format1 == gcSL_UINT8)
                {
                    sourceUint = code->instruction.source1;
                }

                if (targetFormat == gcSL_INTEGER || targetFormat == gcSL_INT16 || targetFormat == gcSL_INT8)
                {
                    switch (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode))
                    {
                    case gcSL_ADD:
                        value = (gctUINT32)(v0.i32 + v1.i32); break;
                    case gcSL_SUB:
                        value = (gctUINT32)(v0.i32 - v1.i32); break;
                    case gcSL_MUL:
                        value = (gctUINT32)(v0.i32 * v1.i32); break;
                    case gcSL_MAX:
                        value = v0.i32 > v1.i32 ? (gctUINT32)v0.i32 : (gctUINT32)v1.i32; break;
                    case gcSL_MIN:
                        value = v0.i32 < v1.i32 ? (gctUINT32)v0.i32 : (gctUINT32)v1.i32; break;
                    case gcSL_POW:
                        value = gcoMATH_Float2Int(
                                    gcoMATH_Power(gcoMATH_Int2Float(v0.i32),
                                                  gcoMATH_Int2Float(v1.i32)));
                        break;
                    case gcSL_MOD:
                        value = (v1.i32 == 0) ? (gctUINT32)v0.i32 : (gctUINT32)(v0.i32 % v1.i32); break;
                    case gcSL_DIV:
                        value = (gctUINT32)(v0.i32 / v1.i32); break;
                    default:
                        gcmASSERT(gcvFALSE);
                        break;
                    }
                }
                else
                {
                    switch (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode))
                    {
                    case gcSL_ADD:
                        value = v0.u32 + v1.u32; break;
                    case gcSL_SUB:
                        value = v0.u32 - v1.u32; break;
                    case gcSL_MUL:
                        value = v0.u32 * v1.u32; break;
                    case gcSL_MAX:
                        value = v0.u32 > v1.u32 ? v0.u32 : v1.u32; break;
                    case gcSL_MIN:
                        value = v0.u32 < v1.u32 ? v0.u32 : v1.u32; break;
                    case gcSL_POW:
                        value = gcoMATH_Float2Int(
                                    gcoMATH_Power(gcoMATH_Int2Float(v0.u32),
                                                  gcoMATH_Int2Float(v1.u32)));
                        break;
                    case gcSL_MOD:
                        value = (v1.u32 == 0) ? v0.u32 : (v0.u32 % v1.u32); break;
                    case gcSL_DIV:
                        value = v0.u32 / v1.u32; break;
                    default:
                        gcmASSERT(gcvFALSE);
                        break;
                    }
                }
                sourceIndex = value & 0xFFFF;
                sourceIndexed = (value >> 16) & 0xFFFF;

                needToPropagate = gcvTRUE;
            }
            else
            {
                {
                }
            }

            break;

        default:
            break;
    }

    *SourceUint = sourceUint;
    *SourceIndex = sourceIndex;
    *SourceIndexed = sourceIndexed;
    *NeedToPropagate = needToPropagate;
    return status;
}

static gctBOOL
_IsCodeIndexedDependency(
    IN gcOPTIMIZER Optimizer,
    IN gcOPT_CODE Code,
    IN gcOPT_CODE UserCode,
    IN gcOPT_LIST DepList,
    IN gctINT SourceNo,
    IN gctINT DepCount
    )
{
    gctBOOL matched = gcvFALSE;

    /* User code has multi-dependencies*/
    if (DepList && DepList->next != gcvNULL && DepCount == 1)
    {
        gctSOURCE_t source = (SourceNo == 0) ? UserCode->instruction.source0 :
                                               UserCode->instruction.source1;
        gctUINT16 sourceIndexed = (SourceNo == 0) ? UserCode->instruction.source0Indexed :
                                                    UserCode->instruction.source1Indexed;

        if (gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED &&
            Code->instruction.tempIndex == sourceIndexed)
        {
            matched = gcvTRUE;
        }
    }

    return matched;
}


static gctBOOL _IsCodeMultiDependencies(
    IN gcOPTIMIZER Optimizer,
    IN gcOPT_LIST DepList,
    IN gcOPT_CODE Code,
    IN gctSOURCE_t SourceUint,
    IN gctUINT32 SourceIndex,
    IN gctUINT16 SourceIndexed,
    OUT gctBOOL *RemoveAllDep
    )
{
    gctBOOL isMulti = gcvTRUE;
    gctSOURCE_t sourceUint = 0;
    gctUINT32 sourceIndex = 0;
    gctUINT16 sourceIndexed = 0;
    gctBOOL needToPropagate = gcvFALSE;
    gctBOOL allDepSameValue = gcvTRUE;
    gcSL_ENABLE enable = gcmSL_TARGET_GET(Code->instruction.temp, Enable);

    *RemoveAllDep = gcvFALSE;
    /* 1: If all dependencies are constant and have the same value, we can propagate it. */
    while(DepList && allDepSameValue)
    {
        if (DepList->code == gcvNULL)
            break;

        /* Skip current code. */
        if (DepList->code == Code)
        {
            DepList = DepList->next;
            continue;
        }

        _EvaluateConstantInstruction(Optimizer,
                    DepList->code, &sourceUint, &sourceIndex, &sourceIndexed, &needToPropagate);

        if (needToPropagate
            && sourceUint == SourceUint && sourceIndex == SourceIndex && sourceIndexed == SourceIndexed
            && Code->instruction.tempIndex == DepList->code->instruction.tempIndex
            && (gctUINT32) enable == gcmSL_TARGET_GET(DepList->code->instruction.temp, Enable))
        {
            DepList = DepList->next;
        }
        else
        {
            allDepSameValue = gcvFALSE;
        }
    }

    if (DepList == gcvNULL && allDepSameValue)
    {
        *RemoveAllDep = gcvTRUE;
        return gcvFALSE;
    }


    /* Defalut. */
    if (DepList && DepList->code == Code && DepList->next == gcvNULL)
        isMulti = gcvFALSE;

    return isMulti;
}

#if defined(_WIN32) && defined(_MSC_VER) && !defined(UNDER_CE)
#define __SET_ROUNDING_MODE_TO_ZERO__ \
    do \
    { \
        _controlfp_s(&oldRound, 0, 0); \
        oldRound &= _MCW_RC; \
        _controlfp_s(&value0, _RC_CHOP, _MCW_RC); \
    } \
    while (gcvFALSE)

#define __RESET_ROUNDING_MODE__ \
    do \
    { \
        _controlfp_s(&value0, oldRound, _MCW_RC); \
    } \
    while (gcvFALSE)
#elif defined(ANDROID) && defined(__arm__) || defined(UNDER_CE)
#define __SET_ROUNDING_MODE_TO_ZERO__
#define __RESET_ROUNDING_MODE__
#else
#define __SET_ROUNDING_MODE_TO_ZERO__ \
    do \
    { \
        oldRound = fegetround(); \
        fesetround(FE_TOWARDZERO); \
    } \
    while (gcvFALSE)

#define __RESET_ROUNDING_MODE__ \
    do \
    { \
        fesetround(oldRound); \
    } \
    while (gcvFALSE)
#endif

/*******************************************************************************
**                          gcOpt_PropagateArgumentConstants
********************************************************************************
**
**  Propagate argument constants.
**
**  INPUT:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_PropagateArgumentConstants(
    IN gcOPTIMIZER Optimizer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcOPT_CODE code;
    gctUINT constPropagated = 0;
#define INDEX(reg, channel) ((reg) * 4 + (channel))
    typedef struct ConstValue
    {
        gcSL_FORMAT Format;
        gctUINT32   v;
    } ConstValue;

    typedef struct FunctionInfo
    {
        ConstValue Value;
        gctBOOL    Live;
        gctBOOL    Init;
    } FunctionInfo;

    FunctionInfo  *funcInfo = gcvNULL;
    gctUINT       funcIdx =   0;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    if (Optimizer->functionCount == 0)
    {
        gcmFOOTER_ARG("status=%d", status);
        return status;
    }

    if (Optimizer->shader->codeCount > SHADER_TOO_MANY_CODE &&
        Optimizer->jmpCount > SHADER_TOO_MANY_JMP)
    {
        gcmFOOTER();
        return status;
    }
    gcmONERROR(gcoOS_Allocate(gcvNULL,
        sizeof(FunctionInfo) * Optimizer->tempCount * 4, (gctPOINTER *)&funcInfo));
    gcoOS_ZeroMemory((gctPOINTER)funcInfo, sizeof(FunctionInfo) * Optimizer->tempCount * 4);

    for (funcIdx = 0; funcIdx < Optimizer->functionCount; ++funcIdx)
    {
        gctUINT        arg      = 0;
        gcOPT_FUNCTION function = &Optimizer->functionArray[funcIdx];

        for (arg = 0; arg < function->argumentCount; ++arg)
        {
            gctUINT8 channel = 0;
            gctUINT  tempIndex  = function->arguments[arg].index;

            if (function->arguments[arg].qualifier !=
                gcvFUNCTION_INPUT)
            {
                continue;
            }

            for (channel = 0; channel < 4; ++channel)
            {
                if ((1 << channel) & function->arguments[arg].enable)
                {
                    funcInfo[INDEX(tempIndex, channel)].Live = gcvTRUE;
                    funcInfo[INDEX(tempIndex, channel)].Init = gcvFALSE;
                }
            }
        }
    }

    /* for each constant, propagate it. */
    for (code = Optimizer->codeHead; code; code = code->next)
    {
        gcOPT_LIST  user =       gcvNULL;
        gctUINT32   value =      0;
        gctUINT32   destIdx =    0;
        gctUINT8    destEnable = gcSL_ENABLE_NONE;
        gcSL_OPCODE opcode =     gcSL_NOP;
        gcSL_INSTRUCTION instruction = &code->instruction;

        opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(instruction->opcode, Opcode);

        if (gcSL_isOpcodeHaveNoTarget(opcode))
        {
            continue;
        }

        destIdx    = instruction->tempIndex;
        destEnable = gcmSL_TARGET_GET(instruction->temp, Enable);

        for (user = code->users; user; user = user->next)
        {
            gcSL_INSTRUCTION userInst = gcvNULL;

            if (user->index < 0)
            {
                continue;
            }

            userInst = &user->code->instruction;

            if (gcmSL_OPCODE_GET(userInst->opcode, Opcode) == gcSL_CALL)
            {
                gctINT channel = 0;
                gctINT kill    = 0;

                if (gcmSL_TARGET_GET(instruction->temp, Indexed) != gcSL_NOT_INDEXED)
                {
                    gctUINT        j        = 0;
                    gcOPT_FUNCTION function = user->code->callee->function;

                    for (j = 0; j < function->argumentCount; ++j)
                    {
                        gctUINT argNo = function->arguments[j].index;
                        for (channel = 0; channel < 4; ++channel)
                        {
                            if ((destEnable & (1 << channel)) &&
                                (function->arguments[j].enable & (1 << channel)))
                            {
                                funcInfo[INDEX(argNo, channel)].Live = gcvFALSE;
                            }
                        }
                    }
                    continue;
                }

                if (gcmSL_OPCODE_GET(instruction->opcode, Opcode) != gcSL_MOV)
                {
                    kill = destEnable;
                }

                if (gcmSL_OPCODE_GET(instruction->opcode, Sat) != gcSL_NO_SATURATE)
                {
                    kill = destEnable;
                }

                if (gcmSL_SOURCE_GET(instruction->source0, Type) != gcSL_CONSTANT)
                {
                    kill = destEnable;
                }

                for (channel = 0; channel < 4; ++channel)
                {
                    if (kill & (1 << channel))
                    {
                        continue;
                    }

                    if (!(destEnable & (1 << channel)))
                    {
                        continue;
                    }

                    if (funcInfo[INDEX(destIdx, channel)].Live
                        && funcInfo[INDEX(destIdx, channel)].Init)
                    {
                        value  = (instruction->source0Index & 0xFFFF) |
                            (instruction->source0Indexed << 16);

                        if ((funcInfo[INDEX(destIdx, channel)].Value.Format !=
                            (gcSL_FORMAT) gcmSL_SOURCE_GET(instruction->source0, Format)) ||
                            value != (funcInfo[INDEX(destIdx, channel)].Value.v))
                        {
                            kill |= (1 << channel);
                        }
                    }
                }

                for (channel = 0; channel < 4; ++channel)
                {
                    if (destEnable & (1 << channel))
                    {
                        if (kill & (1 << channel))
                        {
                            funcInfo[INDEX(destIdx, channel)].Live = gcvFALSE;
                        }
                        else
                        {
                            funcInfo[INDEX(destIdx, channel)].Value.Format =
                                (gcSL_FORMAT) gcmSL_SOURCE_GET(instruction->source0, Format);
                            funcInfo[INDEX(destIdx, channel)].Value.v =
                                (instruction->source0Index & 0xFFFF) | (instruction->source0Indexed << 16);
                            funcInfo[INDEX(destIdx, channel)].Init = gcvTRUE;
                        }
                    }
                }
            }
        }
    }

    /* Do copy propagation */
    for (funcIdx = 0; funcIdx < Optimizer->functionCount; ++funcIdx)
    {
        gctUINT        arg         = 0;
        gcOPT_FUNCTION optFunction = &Optimizer->functionArray[funcIdx];

        for (arg = 0; arg < optFunction->argumentCount; ++arg)
        {
            gctUINT8 channel = 0;
            gctUINT  tempIndex  = optFunction->arguments[arg].index;

            for (channel = 0; channel < 4; ++channel)
            {
                if (((1 << channel) & optFunction->arguments[arg].enable) &&
                    funcInfo[INDEX(tempIndex, channel)].Live)
                {
                    gcOPT_CODE movCode;

                    gcOpt_AddCodeBefore(Optimizer, optFunction->codeHead, &movCode);

                    /* mov.enable dest, const_value */
                    movCode->instruction.opcode = gcSL_MOV;
                    movCode->instruction.temp   = gcmSL_TARGET_SET(0, Enable, (1 << channel))
                                                | gcmSL_TARGET_SET(0, Precision, optFunction->arguments[arg].precision)
                                                | gcmSL_TARGET_SET(0, Format, (funcInfo[INDEX(tempIndex, channel)].Value.Format)) ;
                    movCode->instruction.tempIndexed = 0;
                    movCode->instruction.tempIndex = tempIndex;

                    movCode->instruction.source0        = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                        | gcmSL_SOURCE_SET(0, Format, funcInfo[INDEX(tempIndex, channel)].Value.Format);
                    movCode->instruction.source0Index   = (funcInfo[INDEX(tempIndex, channel)].Value.v & 0xFFFF);
                    movCode->instruction.source0Indexed = (gctUINT16) (funcInfo[INDEX(tempIndex, channel)].Value.v >> 16);

                    ++constPropagated;
                    optFunction->arguments[arg].enable &= ~(1 << channel);
                }
            }
        }
    }

OnError:

    if (funcInfo)
    {
        gcoOS_Free(gcvNULL, (gctPOINTER)funcInfo);
    }
    if (constPropagated > 0)
    {
        gcOpt_RebuildFlowGraph(Optimizer);
        status = gcvSTATUS_CHANGED;
    }

    if (status == gcvSTATUS_CHANGED)
    {
        DUMP_OPTIMIZER("Propagated argument constants in the shader", Optimizer);
        gcmFOOTER();
        return status;
    }

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                          gcOpt_PropagateConstants
********************************************************************************
**
**  Propagate constants.
**
**  INPUT:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_PropagateConstants(
    IN gcOPTIMIZER Optimizer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcOPT_CODE code;
    gctUINT constPropagated = 0;
    gctSOURCE_t sourceUint = 0;
    gctUINT32 sourceIndex = 0;
    gctUINT16 sourceIndexed = 0;
    gctBOOL needToPropagate;
    gctBOOL needRebuildFlow = gcvFALSE;
    gcOPT_LIST list, nextList;
    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);
    /* for each constant, propagate it. */
    for (code = Optimizer->codeHead; code; code = code->next)
    {
        needToPropagate = gcvFALSE;

        gcmVERIFY_OK(
            _EvaluateConstantInstruction(Optimizer, code, &sourceUint, &sourceIndex, &sourceIndexed, &needToPropagate));

        if (!needToPropagate) continue;

        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_MOV)
        {
            /* Convert the instruction to MOV instruction. */
            gcmSL_OPCODE_UPDATE(code->instruction.opcode, Opcode, gcSL_MOV);
            code->instruction.source0 = sourceUint;
            code->instruction.source0Index = sourceIndex;
            code->instruction.source0Indexed = sourceIndexed;
            code->instruction.source1 = 0;
            code->instruction.source1Index = 0;
            code->instruction.source1Indexed = 0;
        }

        for (list = code->users; list; list = nextList)
        {
            gcOPT_CODE codeUser;
            gcOPT_LIST depList;
            gctBOOL cannotReplace = gcvFALSE;
            gctBOOL handleSampler = gcvFALSE;
            gctBOOL useOnSource0 = gcvFALSE;
            gctBOOL useOnSource1 = gcvFALSE;
            gctBOOL propagateSource0 = gcvFALSE;
            gctBOOL propagateSource1 = gcvFALSE;
            gctBOOL useOnBothSource0AndSource1 = gcvFALSE;
            gctINT depCount0 = 0, depCount1 = 0;
            gctBOOL removeAllDep = gcvFALSE;

            nextList = list->next;

            /* Skip output. */
            if (list->index < 0) continue;

            codeUser = list->code;
            switch (gcmSL_OPCODE_GET(codeUser->instruction.opcode,Opcode))
            {
            case gcSL_TEXBIAS:
            case gcSL_TEXGRAD:
            case gcSL_TEXGATHER:
            case gcSL_TEXFETCH_MS:
            case gcSL_TEXLOD:
            case gcSL_TEXU:
            case gcSL_TEXU_LOD:
                /* Skip texture state instructions. */
            case gcSL_TEXLD:
            case gcSL_TEXLD_U:
            case gcSL_TEXLDPROJ:
            case gcSL_TEXLDPCF:
            case gcSL_TEXLDPCFPROJ:
                /* Skip texture sample instructions. */
                handleSampler = gcvTRUE;
                break;

            case gcSL_CALL:
                cannotReplace = gcvTRUE;
                break;
            default:
                if (gcSL_isOpcodeTargetInvalid(gcmSL_OPCODE_GET(codeUser->instruction.opcode,Opcode)))
                {
                    cannotReplace = gcvTRUE;
                }
                break;
            }

            if (cannotReplace) continue;

            depList = codeUser->dependencies0;
            while (depList)
            {
                if (depList->code && depList->code->instruction.tempIndex == code->instruction.tempIndex)
                    depCount0++;

                if (depList->code == code)
                {
                    useOnSource0 = gcvTRUE;
                }
                depList = depList->next;
            }

            depList = codeUser->dependencies1;
            while (depList)
            {
                if (depList->code && depList->code->instruction.tempIndex == code->instruction.tempIndex)
                    depCount1++;

                if (depList->code == code)
                {
                    useOnSource1 = gcvTRUE;
                }
                depList = depList->next;
            }

            /* If the code is used by both source0 and source1,
            ** we woulde delete the user of the code when both source0 and source1 are propagated
            */
            if (useOnSource0 && useOnSource1)
            {
                useOnBothSource0AndSource1 = gcvTRUE;
            }

            depList = codeUser->dependencies0;
            removeAllDep = gcvFALSE;
            if (useOnSource0 &&
                !_IsCodeMultiDependencies(Optimizer, depList, code, sourceUint, sourceIndex, sourceIndexed, &removeAllDep))
            {
                gctSOURCE_t source = codeUser->instruction.source0;
                gcSL_TYPE type   = gcmSL_SOURCE_GET(source, Type);
                if (handleSampler )
                {
                    if (type == gcSL_SAMPLER &&
                        gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
                    {
                        gctINT       indexedValue = 0, arrayIndex = 0;
                        gctUINT      val;
                        gcSL_FORMAT  valueFormat;
                        gcUNIFORM    uniform = gcvNULL;

                        /* get the indexed value */
                        val = (code->instruction.source0Index & 0xFFFF) |
                                 (code->instruction.source0Indexed << 16);
                        valueFormat =
                            (gcSL_FORMAT) gcmSL_SOURCE_GET(code->instruction.source0,
                                                           Format);
                        if (valueFormat == gcSL_FLOAT)
                        {
                            gctFLOAT f0  = gcoMATH_UIntAsFloat(val);
                            indexedValue = (gctINT)f0 ;
                        }
                        else if (valueFormat == gcSL_INTEGER)
                        {
                            indexedValue = *(gctINT *) &val;
                        }
                        else if (valueFormat == gcSL_UINT32)
                        {
                            indexedValue = (gctINT)(*(gctUINT *) &val);
                        }
                        else
                        {
                            /* Error. */
                            gcmFOOTER();
                            return gcvSTATUS_INVALID_DATA;
                        }

                        gcmASSERT(codeUser->instruction.source0Indexed ==
                                                 code->instruction.tempIndex);

                        uniform = gcSHADER_GetUniformBySamplerIndex(Optimizer->shader,
                                                                    codeUser->instruction.source0Index + indexedValue,
                                                                    &arrayIndex);

                        gcmASSERT(uniform);

                        codeUser->instruction.source0 = gcmSL_SOURCE_SET(0, Type, gcSL_UNIFORM)
                                                      | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                                      | gcmSL_SOURCE_SET(0, Precision, GetUniformPrecision(uniform))
                                                      | gcmSL_SOURCE_SET(0, Format, GetUniformFormat(uniform))
                                                      | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYZW);
                        codeUser->instruction.source0Index = gcmSL_INDEX_SET(0, Index, uniform->index)
                                                           | gcmSL_INDEX_SET(0, ConstValue, arrayIndex);
                        codeUser->instruction.source0Indexed = 0;

                        /* Remove dependency and user. */
                        if (!useOnBothSource0AndSource1)
                        {
                            gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer,
                                                       &code->users, codeUser));
                        }

                        if (removeAllDep)
                        {
                            gcmVERIFY_OK(gcOpt_FreeList(Optimizer, &codeUser->dependencies0));
                        }
                        else
                        {
                            /* make sure all dependents are removed from list once constant propogated:
                                *  it happens when multiple dependents having same value or only one
                                *  is the true dependent
                                */
                            gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &codeUser->dependencies0));
                        }

                       propagateSource0 = gcvTRUE;
                       constPropagated++;
                    }
                }
                else if (type == gcSL_TEMP)
                {
                    gcSL_FORMAT src0format = (gcSL_FORMAT) gcmSL_SOURCE_GET(codeUser->instruction.source0, Format);
                    /* Change temp usage to constant. */
                    codeUser->instruction.source0 = sourceUint;
                    /* keep the src0format */
                    codeUser->instruction.source0 = gcmSL_SOURCE_SET(codeUser->instruction.source0, Format, src0format);
                    codeUser->instruction.source0Index = sourceIndex;
                    codeUser->instruction.source0Indexed = sourceIndexed;

                    /* Remove dependency and user. */
                    if (!useOnBothSource0AndSource1)
                    {
                        gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &code->users, codeUser));
                    }

                    if (removeAllDep)
                    {
                        gcmVERIFY_OK(gcOpt_FreeList(Optimizer, &codeUser->dependencies0));
                    }
                    else
                    {
                        /* make sure all dependents are removed from list once constant propogated:
                            *  it happens when multiple dependents having same value or only one
                            *  is the true dependent
                            */
                        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &codeUser->dependencies0));
                    }
                    propagateSource0 = gcvTRUE;
                    constPropagated++;
                }
                else if (type == gcSL_CONSTANT)
                {
                    /* The source of user may had been replaced by a constant
                    ** because all of its dependencies have the same value.
                    */
                    if (!useOnBothSource0AndSource1)
                    {
                        gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &code->users, codeUser));
                    }

                    if (removeAllDep)
                    {
                        gcmVERIFY_OK(gcOpt_FreeList(Optimizer, &codeUser->dependencies0));
                    }
                    else
                    {
                        /* make sure all dependents are removed from list once constant propogated:
                            *  it happens when multiple dependents having same value or only one
                            *  is the true dependent
                            */
                        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &codeUser->dependencies0));
                    }
                }
                else if (type == gcSL_UNIFORM &&
                    gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
                {
                    gctINT       indexedValue = 0;
                    gctUINT      val;
                    gcSL_FORMAT  valueFormat;
                    /* the indexed value is constant, change the sampler
                    * index to orig_sampler_index + constant */
                    gcmASSERT(codeUser->instruction.source0Indexed ==
                        code->instruction.tempIndex );
                    /* set source0 as not indexed */
                    codeUser->instruction.source0 =
                        gcmSL_SOURCE_SET(source, Indexed, gcSL_NOT_INDEXED);
                    codeUser->instruction.source0Indexed = 0;

                    /* get the indexed value */
                    val = (code->instruction.source0Index & 0xFFFF) |
                        (code->instruction.source0Indexed << 16);
                    valueFormat =
                        (gcSL_FORMAT) gcmSL_SOURCE_GET(code->instruction.source0,
                        Format);
                    if (valueFormat == gcSL_FLOAT)
                    {
                        gctFLOAT f0  = gcoMATH_UIntAsFloat(val);
                        indexedValue = (gctINT)f0 ;
                    }
                    else if (valueFormat == gcSL_INTEGER)
                    {
                        indexedValue = *(gctINT *) &val;
                    }
                    else if (valueFormat == gcSL_UINT32)
                    {
                        indexedValue = (gctINT)(*(gctUINT *) &val);
                    }
                    else
                    {
                        /* Error. */
                        gcmFOOTER();
                        return gcvSTATUS_INVALID_DATA;
                    }

                    gcmASSERT(indexedValue >= 0); /* we should not have
                                                  negative indexing for sampler */

                    codeUser->instruction.source0Indexed =
                        (gctUINT16)gcmSL_INDEX_SET(codeUser->instruction.source0Indexed, Index, indexedValue);

                    /* Remove dependency and user. */
                    if (!useOnBothSource0AndSource1)
                    {
                        gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer,
                            &code->users, codeUser));
                    }

                    if (removeAllDep)
                    {
                        gcmVERIFY_OK(gcOpt_FreeList(Optimizer, &codeUser->dependencies0));
                    }
                    else
                    {
                        gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &codeUser->dependencies0, code));
                    }

                    propagateSource0 = gcvTRUE;
                    constPropagated++;
                }
            }
            else if (_IsCodeIndexedDependency(Optimizer, code, codeUser, depList, 0, depCount0))
            {
                gctINT       indexedValue = 0;
                gctUINT      val;
                gcSL_FORMAT  format;
                gctSOURCE_t  source = codeUser->instruction.source0;
                gcVARIABLE   variable = Optimizer->tempArray[codeUser->instruction.source0Index].arrayVariable;
                gctBOOL      validIndex = gcvTRUE;

                /* get the indexed value */
                val = (code->instruction.source0Index & 0xFFFF) |
                            (code->instruction.source0Indexed << 16);
                format = (gcSL_FORMAT) gcmSL_SOURCE_GET(code->instruction.source0,
                                                    Format);
                if (format == gcSL_FLOAT)
                {
                    gctFLOAT f0  = gcoMATH_UIntAsFloat(val);
                    indexedValue = (gctINT)f0 ;
                }
                else if (format == gcSL_INT32 || format == gcSL_INT16 || format == gcSL_INT8)
                {
                    indexedValue = *(gctINT *) &val;
                }
                else if (format == gcSL_UINT32 || format == gcSL_UINT16 || format == gcSL_UINT8)
                {
                    indexedValue = (gctINT)(*(gctUINT *) &val);
                }
                else
                {
                    /* Error. */
                    gcmFOOTER();
                    return gcvSTATUS_INVALID_DATA;
                }

                if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
                {
                    /* This is a invalid index, it may be skip by any previous JMPs. */
                    if (variable && variable->arraySize > 1)
                    {
                        if ((codeUser->instruction.source0Index + indexedValue) >=
                            (variable->tempIndex + variable->arraySize))
                        {
                            validIndex = gcvFALSE;
                        }
                    }

                    if ((codeUser->instruction.source0Index + indexedValue) >= Optimizer->tempCount)
                    {
                        validIndex = gcvFALSE;
                    }
                }

                if (validIndex)
                {
                    /* set source0 as not indexed */
                    codeUser->instruction.source0 =
                        gcmSL_SOURCE_SET(source, Indexed, gcSL_NOT_INDEXED);
                    codeUser->instruction.source0Indexed = 0;

                    codeUser->instruction.source0Index += indexedValue;

                    /* Remove dependency and user. */
                    if (!useOnBothSource0AndSource1)
                    {
                        gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer,
                                                    &code->users, codeUser));
                    }
                    /* make sure all dependents are removed from list once constant propogated:
                        *  it happens when multiple dependents having same value or only one
                        *  is the true dependent
                        */
                    gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &codeUser->dependencies0));

                    propagateSource0 = gcvTRUE;
                    constPropagated++;

                    needRebuildFlow = gcvTRUE;
                }
            }

            removeAllDep = gcvFALSE;
            depList = codeUser->dependencies1;
            if (useOnSource1 && !_IsCodeMultiDependencies(Optimizer, depList, code, sourceUint, sourceIndex, sourceIndexed, &removeAllDep))
            {
                gcSL_TYPE type = gcmSL_SOURCE_GET(codeUser->instruction.source1, Type);
                gctSOURCE_t source = codeUser->instruction.source1;
                if (type == gcSL_TEMP )
                {
                    /* Change temp usage to constant. */
                    gcSL_FORMAT src1format = (gcSL_FORMAT) gcmSL_SOURCE_GET(codeUser->instruction.source1, Format);
                    codeUser->instruction.source1 = sourceUint;
                    codeUser->instruction.source1 = gcmSL_SOURCE_SET(codeUser->instruction.source1, Format, src1format);
                    codeUser->instruction.source1Index = sourceIndex;
                    codeUser->instruction.source1Indexed = sourceIndexed;

                    /* Remove dependency and user. */
                    /* source1 may be the same as source0, so do not verify it. */
                    if (!useOnBothSource0AndSource1)
                    {
                        gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &code->users, codeUser));
                    }

                    if (removeAllDep)
                    {
                        gcmVERIFY_OK(gcOpt_FreeList(Optimizer, &codeUser->dependencies1));
                    }
                    else
                    {
                        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &codeUser->dependencies1));
                    }

                    propagateSource1 = gcvTRUE;
                    constPropagated++;
                }
                else if (type == gcSL_CONSTANT)
                {
                    /* The source of user may had been replaced by a constant
                    ** because all of its dependencies have the same value.
                    */
                    if (!useOnBothSource0AndSource1)
                    {
                        gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &code->users, codeUser));
                    }

                    if (removeAllDep)
                    {
                        gcmVERIFY_OK(gcOpt_FreeList(Optimizer, &codeUser->dependencies1));
                    }
                    else
                    {
                        gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &codeUser->dependencies1));
                    }
                }
                else if (type == gcSL_UNIFORM &&
                    gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED)
                {
                    gctINT       indexedValue = 0;
                    gctUINT      val;
                    gcSL_FORMAT  valueFormat;
                    /* the indexed value is constant, change the sampler
                    * index to orig_sampler_index + constant */
                    gcmASSERT((gctUINT)codeUser->instruction.source1Indexed ==
                        code->instruction.tempIndex );
                    /* set source1 as not indexed */
                    codeUser->instruction.source1 =
                        gcmSL_SOURCE_SET(source, Indexed, gcSL_NOT_INDEXED);
                    codeUser->instruction.source1Indexed = 0;

                    /* get the indexed value */
                    val = (code->instruction.source0Index & 0xFFFF) |
                        (code->instruction.source0Indexed << 16);
                    valueFormat =
                        (gcSL_FORMAT) gcmSL_SOURCE_GET(code->instruction.source0,
                        Format);
                    if (valueFormat == gcSL_FLOAT)
                    {
                        gctFLOAT f0  = gcoMATH_UIntAsFloat(val);
                        indexedValue = (gctINT)f0 ;
                    }
                    else if (valueFormat == gcSL_INTEGER)
                    {
                        indexedValue = *(gctINT *) &val;
                    }
                    else if (valueFormat == gcSL_UINT32)
                    {
                        indexedValue = (gctINT)(*(gctUINT *) &val);
                    }
                    else
                    {
                        /* Error. */
                        gcmFOOTER();
                        return gcvSTATUS_INVALID_DATA;
                    }

                    gcmASSERT(indexedValue >= 0); /* we should not have
                                                  negative indexing for sampler */

                    codeUser->instruction.source1Indexed =
                        (gctUINT16)gcmSL_INDEX_SET(codeUser->instruction.source1Indexed, Index, indexedValue);

                    /* Remove dependency and user. */
                    if (!useOnBothSource0AndSource1)
                    {
                        gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer,
                            &code->users, codeUser));
                    }

                    if (removeAllDep)
                    {
                        gcmVERIFY_OK(gcOpt_FreeList(Optimizer, &codeUser->dependencies1));
                    }
                    else
                    {
                        gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &codeUser->dependencies1, code));
                    }

                    propagateSource1 = gcvTRUE;
                    constPropagated++;
                }
            }
            else if (_IsCodeIndexedDependency(Optimizer, code, codeUser, depList, 1, depCount1))
            {
                gctINT       indexedValue = 0;
                gctUINT      val;
                gcSL_FORMAT  format;
                gctSOURCE_t  source = codeUser->instruction.source1;
                gcVARIABLE   variable = Optimizer->tempArray[codeUser->instruction.source1Index].arrayVariable;
                gctBOOL      validIndex = gcvTRUE;

                /* get the indexed value */
                val = (code->instruction.source0Index & 0xFFFF) |
                            (code->instruction.source0Indexed << 16);
                format = (gcSL_FORMAT) gcmSL_SOURCE_GET(code->instruction.source0,
                                                    Format);
                if (format == gcSL_FLOAT)
                {
                    gctFLOAT f0  = gcoMATH_UIntAsFloat(val);
                    indexedValue = (gctINT)f0 ;
                }
                else if (format == gcSL_INT32 || format == gcSL_INT16 || format == gcSL_INT8)
                {
                    indexedValue = *(gctINT *) &val;
                }
                else if (format == gcSL_UINT32 || format == gcSL_UINT16 || format == gcSL_UINT8)
                {
                    indexedValue = (gctINT)(*(gctUINT *) &val);
                }
                else
                {
                    /* Error. */
                    gcmFOOTER();
                    return gcvSTATUS_INVALID_DATA;
                }

                if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
                {
                    /* This is a invalid index, it may be skip by any previous JMPs. */
                    if (variable && variable->arraySize > 1)
                    {
                        if ((codeUser->instruction.source1Index + indexedValue) >=
                            (variable->tempIndex + variable->arraySize))
                        {
                            validIndex = gcvFALSE;
                        }
                    }

                    if ((codeUser->instruction.source1Index + indexedValue) >= Optimizer->tempCount)
                    {
                        validIndex = gcvFALSE;
                    }
                }

                if (validIndex)
                {
                    /* set source1 as not indexed */
                    codeUser->instruction.source1 =
                        gcmSL_SOURCE_SET(source, Indexed, gcSL_NOT_INDEXED);
                    codeUser->instruction.source1Indexed = 0;

                    /* set the sampler index */
                    codeUser->instruction.source1Index += indexedValue;

                    /* Remove dependency and user. */
                    if (!useOnBothSource0AndSource1)
                    {
                        gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer,
                                                    &code->users, codeUser));
                    }
                    gcmVERIFY_OK(gcOpt_DestroyList(Optimizer, &codeUser->dependencies1));

                    propagateSource1 = gcvTRUE;
                    constPropagated++;

                    needRebuildFlow = gcvTRUE;
                }
            }

            if (useOnBothSource0AndSource1 && propagateSource0 && propagateSource1)
            {
                gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &code->users, codeUser));
            }
        }
    }
    if (constPropagated > 0)
    {
        status = gcvSTATUS_CHANGED;
    }

    if (needRebuildFlow)
    {
        /* Rebuild flow graph. */
        gcmVERIFY_OK(gcOpt_RebuildFlowGraph(Optimizer));
    }

    if (status == gcvSTATUS_CHANGED)
    {
        DUMP_OPTIMIZER("Propagated constants in the shader", Optimizer);
        gcmFOOTER();
        return status;
    }

    gcmFOOTER();
    return status;
}

static gctBOOL
_GetLTCValue(
    IN gcOPTIMIZER Optimizer,
    IN gcOPT_CODE Code,
    IN gctINT Source,
    OUT PLTCValue Value
    )
{
    gctBOOL match = gcvTRUE;
    gcSHADER shader = Optimizer->shader;
    gctSOURCE_t source = (Source == 0) ? Code->instruction.source0 : Code->instruction.source1;
    gctUINT32 sourceIndex = (Source == 0) ? Code->instruction.source0Index : Code->instruction.source1Index;
    gcOPT_LIST  depList = (Source == 0) ? Code->dependencies0 : Code->dependencies1;
    gcSL_TYPE type = gcmSL_SOURCE_GET(source, Type);
    gcSL_SWIZZLE swizzle[4];
    LTCValue tempValue;
    gctINT i;

    if (shader->ltcUniformValues == gcvNULL)
    {
        gcSHADER_EvaluateLTCValueWithinLinkTime(shader);

        if (shader->ltcUniformValues == gcvNULL)
            return gcvFALSE;
    }

    /* Init tempValue. */
    tempValue.elementType = 0;
    tempValue.enable = 0;
    tempValue.instructionIndex = 0;
    tempValue.sourceInfo = 0;
    for (i = 0; i < MAX_LTC_COMPONENTS; i++)
    {
        tempValue.v[i].u64 = 0;
    }

    /* Get the uniform data. */
    if (type == gcSL_UNIFORM)
    {
        gctINT uniformIndex = gcmSL_INDEX_GET(sourceIndex, Index);
        gcUNIFORM uniform = shader->uniforms[uniformIndex];

        if (isUniformLoadtimeConstant(uniform) &&
            GetUniformDummyUniformIndex(uniform) != -1)
        {
            tempValue = shader->ltcUniformValues[GetUniformDummyUniformIndex(uniform)];
            if (tempValue.enable == gcSL_ENABLE_NONE)
            {
                match = gcvFALSE;
            }
        }
        else
        {
            match = gcvFALSE;
        }
    }
    else
    {
        if (depList != gcvNULL &&
            depList->index >= 0 &&
            depList->next == gcvNULL &&
            depList->code != gcvNULL &&
            gcmSL_OPCODE_GET(depList->code->instruction.opcode, Opcode) == gcSL_MOV)
        {
            match = _GetLTCValue(Optimizer, depList->code, 0, &tempValue);
        }
        else
        {
            match = gcvFALSE;
        }
    }

    if (!match)
        return match;

    /* Get the data. */
    swizzle[0] = gcmSL_SOURCE_GET(source, SwizzleX);
    swizzle[1] = gcmSL_SOURCE_GET(source, SwizzleY);
    swizzle[2] = gcmSL_SOURCE_GET(source, SwizzleZ);
    swizzle[3] = gcmSL_SOURCE_GET(source, SwizzleW);

    Value->elementType = tempValue.elementType;
    Value->instructionIndex = tempValue.instructionIndex;
    Value->sourceInfo = tempValue.sourceInfo;

    for (i = 0; i < 4; i++)
    {
        Value->v[i] = tempValue.v[swizzle[i]];
    }

    if (gcmSL_OPCODE_GET(Code->instruction.opcode, Opcode) == gcSL_JMP)
    {
        Value->enable = gcSL_ENABLE_X;
    }
    else
    {
        Value->enable = gcmSL_TARGET_GET(Code->instruction.temp, Enable);
    }

    return match;
}

static gctBOOL
_EvaluateSingleChannelChecking(
    IN gctUINT32 Value0,
    IN gctUINT32 Value1,
    IN gcSL_FORMAT Format0,
    IN gcSL_FORMAT Format1,
    IN gcSL_CONDITION Condition,
    OUT gctBOOL * CheckingResult
    )
{
    if (Format0 == gcSL_FLOAT || Format1 == gcSL_FLOAT)
    {
        float f0, f1;

        if (Format0 == gcSL_FLOAT)
        {
            f0 = gcoMATH_UIntAsFloat(Value0);
        }
        else if (Format0 == gcSL_INTEGER)
        {
            f0 = gcoMATH_UInt2Float(Value0);
        }
        else
        {
            /* Error. */
            return gcvFALSE;
        }

        if (Format1 == gcSL_FLOAT)
        {
            f1 = gcoMATH_UIntAsFloat(Value1);
        }
        else if (Format1 == gcSL_INTEGER)
        {
            f1 = gcoMATH_UInt2Float(Value1);
        }
        else
        {
            /* Error. */
            return gcvFALSE;
        }

        switch (Condition)
        {
        case gcSL_ALWAYS:
            /* Error. */ return gcvFALSE;
        case gcSL_NOT_EQUAL:
            *CheckingResult = (f0 != f1); break;
        case gcSL_LESS_OR_EQUAL:
            *CheckingResult = (f0 <= f1); break;
        case gcSL_LESS:
            *CheckingResult = (f0 < f1); break;
        case gcSL_EQUAL:
            *CheckingResult = (f0 == f1); break;
        case gcSL_GREATER:
            *CheckingResult = (f0 > f1); break;
        case gcSL_GREATER_OR_EQUAL:
            *CheckingResult = (f0 >= f1); break;
        case gcSL_NOT_ZERO:
            *CheckingResult = (f0 != 0.0f); break;
        case gcSL_AND:
        case gcSL_OR:
        case gcSL_XOR:
        case gcSL_ALLMSB:
        case gcSL_ANYMSB:
        case gcSL_SELMSB:
        default:
            return gcvFALSE;
        }
    }
    else
    {
        /* could be int32 or uint32 */
        int i0 = Value0;
        int i1 = Value1;

        switch (Condition)
        {
        case gcSL_NOT_EQUAL:
            *CheckingResult = (Value0 != Value1);
            break;
        case gcSL_LESS_OR_EQUAL:
            if (Format0 == gcSL_INT32 &&
                Format1 == gcSL_INT32)
            {
                *CheckingResult = (i0 <= i1);
            }
            else
            {
                *CheckingResult = (Value0 <= Value1);
            }
             break;
        case gcSL_LESS:
            if (Format0 == gcSL_INT32 &&
                Format1 == gcSL_INT32)
            {
                *CheckingResult = (i0 < i1);
            }
            else
            {
                *CheckingResult = (Value0 < Value1);
            }
             break;
        case gcSL_EQUAL:
            *CheckingResult = (Value0 == Value1); break;
        case gcSL_GREATER:
            if (Format0 == gcSL_INT32 &&
                Format1 == gcSL_INT32)
            {
                *CheckingResult = (i0 > i1);
            }
            else
            {
                *CheckingResult = (Value0 > Value1);
            }
            break;
        case gcSL_GREATER_OR_EQUAL:
            if (Format0 == gcSL_INT32 &&
                Format1 == gcSL_INT32)
            {
                *CheckingResult = (i0 >= i1);
            }
            else
            {
                *CheckingResult = (Value0 >= Value1);
            }
             break;
        case gcSL_AND:
            *CheckingResult = (Value0 & Value1); break;
        case gcSL_OR:
            *CheckingResult = (Value0 | Value1); break;
        case gcSL_XOR:
            *CheckingResult = (Value0 ^ Value1); break;
        case gcSL_NOT_ZERO:
            *CheckingResult = (Value0 != 0); break;
        case gcSL_ALWAYS:
        default:
            /* Error. */
            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

static gctBOOL
_EvaluateChecking(
    IN gcOPTIMIZER Optimizer,
    IN gcOPT_CODE Code,
    OUT gctBOOL * CheckingResult
    )
{
    gctBOOL validCase = gcvTRUE;
    gcSL_INSTRUCTION inst = &Code->instruction;
    gcSL_FORMAT format0, format1;
    gcSL_TYPE type0, type1;
    LTCValue value0, value1;
    gctBOOL results[4] = {gcvTRUE, gcvTRUE, gcvTRUE, gcvTRUE};
    gcSL_CONDITION condition = gcmSL_TARGET_GET(inst->temp, Condition);
    gctINT i;

    format0 = (gcSL_FORMAT) gcmSL_SOURCE_GET(inst->source0, Format);
    type0 = gcmSL_SOURCE_GET(inst->source0, Type);
    if (gcmEnableChannelCount(gcSL_ConvertSwizzle2Enable(gcmSL_SOURCE_GET(inst->source0, SwizzleX),
                                                         gcmSL_SOURCE_GET(inst->source0, SwizzleY),
                                                         gcmSL_SOURCE_GET(inst->source0, SwizzleZ),
                                                         gcmSL_SOURCE_GET(inst->source0, SwizzleW))) != 1)
    {
        return gcvFALSE;
    }
    if (type0 == gcSL_UNIFORM || type0 == gcSL_TEMP)
    {
        if (!_GetLTCValue(Optimizer, Code, 0, &value0))
        {
            return gcvFALSE;
        }
    }
    else if (type0 == gcSL_CONSTANT)
    {
        value0.elementType = format0;
        value0.enable = gcSL_ENABLE_X;
        value0.v[0].u32 = (inst->source0Index & 0xFFFF) | (inst->source0Indexed << 16);
    }
    else
    {
        return gcvFALSE;
    }

    format1 = (gcSL_FORMAT) gcmSL_SOURCE_GET(inst->source1, Format);
    type1 = gcmSL_SOURCE_GET(inst->source1, Type);
    if (gcmEnableChannelCount(gcSL_ConvertSwizzle2Enable(gcmSL_SOURCE_GET(inst->source1, SwizzleX),
                                                         gcmSL_SOURCE_GET(inst->source1, SwizzleY),
                                                         gcmSL_SOURCE_GET(inst->source1, SwizzleZ),
                                                         gcmSL_SOURCE_GET(inst->source1, SwizzleW))) != 1)
    {
        return gcvFALSE;
    }
    if (type1 == gcSL_UNIFORM || type1 == gcSL_TEMP)
    {
        if (!_GetLTCValue(Optimizer, Code, 1, &value1))
        {
            return gcvFALSE;
        }
    }
    else if (type1 == gcSL_CONSTANT)
    {
        value1.elementType = format0;
        value1.enable = gcSL_ENABLE_X;
        value1.v[0].u32 = (inst->source1Index & 0xFFFF) | (inst->source1Indexed << 16);
    }
    else
    {
        return gcvFALSE;
    }

    gcmASSERT(gcmEnableChannelCount(value0.enable) == gcmEnableChannelCount(value1.enable));

    for (i = 0; i < 4; i++)
    {
        if ((value0.enable & (1 << i)) && (value1.enable & (1 << i)))
        {
            validCase = _EvaluateSingleChannelChecking(value0.v[i].u32,
                                                       value1.v[i].u32,
                                                       format0,
                                                       format1,
                                                       condition,
                                                       &results[i]);

            if (!validCase)
            {
                break;
            }

            if (i == 0)
                *CheckingResult = results[i];
            else
                *CheckingResult &= results[i];
        }
    }

    return validCase;
}

/*******************************************************************************
**                          gcOpt_RemoveRedundantCheckings
********************************************************************************
**
**  Remove redundant checkings.
**
**  INPUT:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_RemoveRedundantCheckings(
    IN gcOPTIMIZER Optimizer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcOPT_CODE code;
    gctUINT changeCount = 0;
    gctUINT i;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    if (Optimizer->shader->codeCount > SHADER_TOO_MANY_CODE &&
        Optimizer->jmpCount > SHADER_TOO_MANY_JMP)
    {
        gcmFOOTER();
        return status;
    }

    for (code = Optimizer->codeHead; code; code = code->next)
    {
        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) == gcSL_JMP)
        {
            if (gcmSL_TARGET_GET(code->instruction.temp, Condition) != gcSL_ALWAYS)
            {
                gctBOOL checkingResult = gcvFALSE;

                if (_EvaluateChecking(Optimizer, code, &checkingResult))
                {
                    changeCount++;
                    if (checkingResult)
                    {
                        /* Change the conditional jump to unconditional jump. */
                        code->instruction.temp = gcmSL_TARGET_SET(code->instruction.temp, Condition, gcSL_ALWAYS);

                        /* Remove sources. */
                        code->instruction.source0 = code->instruction.source0Index = code->instruction.source0Indexed = 0;
                        code->instruction.source1 = code->instruction.source1Index = code->instruction.source1Indexed = 0;
                    }
                    else
                    {
                        /* Change the checking/jump instruction to NOP. */
                        gcmVERIFY_OK(gcOpt_ChangeCodeToNOP(Optimizer, code));
                    }
                }
            }
        }
    }

    if (changeCount)
    {
        _RemoveFunctionUnreachableCode(Optimizer, Optimizer->main);
        for (i = 0; i < Optimizer->functionCount; i++)
        {
            _RemoveFunctionUnreachableCode(Optimizer, Optimizer->functionArray + i);
        }
    }

    if (changeCount)
    {
        /* Rebuild flow graph. */
        gcmONERROR(gcOpt_RebuildFlowGraph(Optimizer));

        DUMP_OPTIMIZER("Removed redundant checkings from the shader", Optimizer);
        gcmFOOTER_ARG("status=%d", gcvSTATUS_CHANGED);
        return gcvSTATUS_CHANGED;
    }
    else
    {
        /* Code unchanged. */
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                          gcOpt_MergeVectorInstructions
********************************************************************************
**
**  Merge separate vector instructions into one vector instruction.
**
**  INPUT:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_MergeVectorInstructions(
    IN gcOPTIMIZER Optimizer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT codeRemoved = 0;
    gcOPT_TEMP tempArray = Optimizer->tempArray;
    gcOPT_TEMP temp;
    gcOPT_CODE code, endCode, iterCode;
    gcOPT_LIST list, list1;
    gctUINT16 enable, enable1;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    if (Optimizer->shader->codeCount > SHADER_TOO_MANY_CODE &&
        Optimizer->jmpCount > SHADER_TOO_MANY_JMP)
    {
        gcmFOOTER();
        return status;
    }

    gcOpt_UpdateCodeId(Optimizer);

    /* Phase I: merge MOV instructions for the same vector. */
    for (code = Optimizer->codeTail; code; code = code->prev)
    {
        /* Test for MOV instruction. */
        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_MOV) continue;

        if (! code->users) continue;

        /* Get pointer to temporary register. */
        temp = tempArray + code->instruction.tempIndex;

        enable = gcmSL_TARGET_GET(code->instruction.temp, Enable);

        /* Skip if all used components are enabled. */
        if (enable == temp->usage) continue;

        /* Check if users have the multiple dependencies that set different components. */
        endCode = Optimizer->codeTail;

        for (list = code->users; list; list = list->next)
        {
            if (list->code && list->code->id < endCode->id && list->code->id > code->id)
            {
                endCode = list->code;
            }
        }

        for (list = code->nextDefines; list; list = list->next)
        {
            if (list->code && list->code->id < endCode->id && list->code->id > code->id)
            {
                endCode = list->code;
            }
        }

        if (endCode->id < code->id)
            continue;

        /* Find candidates that can be merged. */
        for (iterCode = code->next; iterCode != endCode; iterCode = iterCode->next)
        {
            /* Stop if any jump in/out. */
            if (iterCode->callers) break;
            if (gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_JMP ||
                gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_CALL) break;

            /* Check if iterCode is a candidate. */
            if (gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) != gcSL_MOV) continue;
            if (iterCode->instruction.tempIndex != code->instruction.tempIndex) continue;
            if (iterCode->instruction.source0Index != code->instruction.source0Index) continue;
            if (gcmSL_SOURCE_GET(code->instruction.source0, Type) !=
                gcmSL_SOURCE_GET(iterCode->instruction.source0, Type)) continue;
            if (gcmSL_TARGET_GET(code->instruction.temp, Indexed) !=
                gcmSL_TARGET_GET(iterCode->instruction.temp, Indexed)) continue;
            if (iterCode->instruction.tempIndexed != code->instruction.tempIndexed) continue;
            if (gcmSL_SOURCE_GET(code->instruction.source0, Indexed) !=
                gcmSL_SOURCE_GET(iterCode->instruction.source0, Indexed)) continue;
            if (iterCode->instruction.source0Indexed != code->instruction.source0Indexed) continue;

            /* Check if the dest has been changed after code. */
            for (list = iterCode->prevDefines; list; list = list->next)
            {
                if (list->code && list->code->id >= code->id) break;

                if (list->index < 0) continue;

                /* Check if the dest is used after code. */
                for (list1 = list->code->users; list1; list1 = list1->next)
                {
                    if (list1->code && list1->code->id > code->id) break;
                }
            }
            if (list) continue;

            /* Check if the source is redefined after code */
            for (list = iterCode->dependencies0; list; list = list->next)
            {
                if (list->code && list->code->id > code->id) break;
            }
            if (list) continue;

            enable1 = gcmSL_TARGET_GET(iterCode->instruction.temp, Enable);
            if ((enable & enable1) != 0) continue;

            /* Merge iterCode into code. */
            enable |= enable1;
            code->instruction.temp = gcmSL_TARGET_SET(code->instruction.temp, Enable, enable);

            if (enable1 & gcSL_ENABLE_X)
            {
                code->instruction.source0 = gcmSL_SOURCE_SET(code->instruction.source0,
                                                             SwizzleX,
                                                             gcmSL_SOURCE_GET(iterCode->instruction.source0, SwizzleX));
            }
            if (enable1 & gcSL_ENABLE_Y)
            {
                code->instruction.source0 = gcmSL_SOURCE_SET(code->instruction.source0,
                                                             SwizzleY,
                                                             gcmSL_SOURCE_GET(iterCode->instruction.source0, SwizzleY));
            }
            if (enable1 & gcSL_ENABLE_Z)
            {
                code->instruction.source0 = gcmSL_SOURCE_SET(code->instruction.source0,
                                                             SwizzleZ,
                                                             gcmSL_SOURCE_GET(iterCode->instruction.source0, SwizzleZ));
            }
            if (enable1 & gcSL_ENABLE_W)
            {
                code->instruction.source0 = gcmSL_SOURCE_SET(code->instruction.source0,
                                                             SwizzleW,
                                                             gcmSL_SOURCE_GET(iterCode->instruction.source0, SwizzleW));
            }

            /* Update data flow. */
            if (iterCode->users)
            {
                for (list = iterCode->users; list; list = list->next)
                {
                    gcOPT_CODE userCode;

                    if (list->index < 0) continue;   /* Skip output and global dependency. */

                    userCode = list->code;
                    gcOpt_ReplaceCodeInList(Optimizer, &userCode->dependencies0, iterCode, code);
                    gcOpt_ReplaceCodeInList(Optimizer, &userCode->dependencies1, iterCode, code);
                }
                gcmVERIFY_OK(gcOpt_AddListToList(Optimizer, iterCode->users, gcvFALSE, &code->users));
                gcmVERIFY_OK(gcOpt_FreeList(Optimizer, &iterCode->users));
            }

            if (iterCode->dependencies0)
            {
                for (list = iterCode->dependencies0; list; list = list->next)
                {
                    if (list->index < 0) continue;   /* Skip output and global dependency. */

                    gcOpt_ReplaceCodeInList(Optimizer, &list->code->users, iterCode, code);
                }

                gcmVERIFY_OK(gcOpt_AddListToList(Optimizer, iterCode->dependencies0, gcvFALSE, &code->dependencies0));
                gcmVERIFY_OK(gcOpt_FreeList(Optimizer, &iterCode->dependencies0));
            }

            if (iterCode->nextDefines)
            {
                gcmVERIFY_OK(gcOpt_AddListToList(Optimizer, iterCode->nextDefines, gcvFALSE, &code->nextDefines));
                gcmVERIFY_OK(gcOpt_FreeList(Optimizer, &iterCode->nextDefines));
            }

            if (iterCode->prevDefines)
            {
                gcmVERIFY_OK(gcOpt_AddListToList(Optimizer, iterCode->prevDefines, gcvFALSE, &code->prevDefines));
                gcmVERIFY_OK(gcOpt_FreeList(Optimizer, &iterCode->prevDefines));
            }

            gcmVERIFY_OK(gcOpt_ChangeCodeToNOP(Optimizer, iterCode));
            /*iterCode->instruction = gcvSL_NOP_INSTR;*/
            codeRemoved++;
        }
    }

    if (codeRemoved != 0)
    {
        /* Rebuild flow graph. */
        gcmONERROR(gcOpt_RebuildFlowGraph(Optimizer));

        DUMP_OPTIMIZER("Merged vector instructions in the shader", Optimizer);
        gcmFOOTER_ARG("status=%d", gcvSTATUS_CHANGED);
        return gcvSTATUS_CHANGED;
    }
    else
    {
        /* Code unchanged. */
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/* return 1 if the User depends only on definition Def for the temp register,
 *        0 otherwise
 */
static gctINT isUserOnlyDependsOn(gcOPT_CODE User, gcOPT_CODE Def)
{
    /* check source 0 */
    if (User->dependencies0 &&
        User->dependencies0->code == Def &&
        User->dependencies0->next == gcvNULL)
        return gcvTRUE;

    /* check source1 */
    if (User->dependencies1 &&
        User->dependencies1->code == Def &&
        User->dependencies1->next == gcvNULL)
        return gcvTRUE;

    return gcvFALSE;
}

/*******************************************************************************
**                          gcOpt_ConditionalizeCode
********************************************************************************
**
**  If the code is only used inside a conditional block, try to move the code
**  into the conditional block. There are two benefits for this code move:
**    o  reduced the code execution times, it will only be executed when
**       the condition is true for the block
**    o  reduce the register pressure, the live range for the definition
**       is shortened
**
**  Legality Check:
**
**
**  INPUT:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_ConditionalizeCode(
    IN gcOPTIMIZER Optimizer
    )
{
    gctUINT codeMoved = 0;
    gcOPT_CODE code, theUserCode, iterCode, theNextCode;
    gcOPT_LIST caller;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    gcOpt_UpdateCodeId(Optimizer);

    /* Move definition of the code to close to user if possible. */
    for (code = Optimizer->codeHead; code; code = theNextCode)
    {
        gctINT seenJmp = 0, inLoop = 0;

        theNextCode = code->next;

        /* Skip if no user or more than one user, or user is output or global register. */
        if (! code->users || code->users->next || code->users->index < 0 ) continue;

        /* Skip if the code is a branch target. */
        if (code->callers ) continue;

        /* Skip if indexed. */
        if (gcmSL_TARGET_GET(code->instruction.temp, Indexed) != gcSL_NOT_INDEXED)
            continue;

        /* Get pointer to temporary register. */
        /*temp = tempArray + code->instruction.tempIndex;*/

        theUserCode = code->users->code;
        /* Check if the user is inside a branch block, and if it is not
           in a loop, by backward iterating from the user code */
        for (iterCode = theUserCode->prev; iterCode && iterCode != code;
                                           iterCode = iterCode->prev)
        {
            if (gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_JMP &&
                iterCode->callee->id > theUserCode->id)
            {
                seenJmp++;
            }
            /* check if the code is a backward jmp target */
            for (caller = iterCode->callers; caller ;
                                             caller = caller->next) {
                if (caller->index >= 0 && caller->code->backwardJump) {
                    inLoop = 1;
                    break;
                }
            }
            if (inLoop) break;  /* bail out if the user is in a loop */
        }

        if (iterCode == gcvNULL || !seenJmp || inLoop)
            continue;

        /* Check if the definition can be moved before user instruction. */

        if (code->dependencies0 || code->dependencies1
            || code->nextDefines || code->prevDefines
            || !isUserOnlyDependsOn(theUserCode, code)
            || gcmSL_OPCODE_GET(theUserCode->instruction.opcode, Opcode) == gcSL_TEXLD)
        {
            continue;
        }

        /* now we know the definition is safe to move to the conditional block*/

        /* Move definition to before the user. */
        if (code->next != theUserCode) {
            if (code == Optimizer->main->codeHead)
            {
                Optimizer->main->codeHead = code->next;
            }

            gcOpt_MoveCodeListBefore(Optimizer, code, code, theUserCode);

            /* Update code id because the checkings depend on it. */
            gcOpt_UpdateCodeId(Optimizer);
            /* if the user code is branch/call target, we need to
               change the targetto the new code we just moved before it. */
            if (theUserCode->callers) {
                gcOPT_LIST  caller;
                for (caller = theUserCode->callers; caller;
                     caller = caller->next) {
                    /* change caller's coresponding callee node to the new code */
                    gcOPT_CODE  callerCode = caller->code;
                    gcmASSERT(callerCode->callee == theUserCode);
                    /* update caller-callee relation */
                    callerCode->callee = code;
                    /* update label in instruction */
                    gcmSL_JMP_TARGET(&(callerCode->instruction)) = code->id;
                }
                /* move theUserCode's caller list to new code */
                gcmASSERT(code->callers == gcvNULL);
                code->callers = theUserCode->callers;
                theUserCode->callers = gcvNULL;
            }

            ++codeMoved;
        }
    }

    if (codeMoved != 0)
    {
        DUMP_OPTIMIZER("Moved definion instruction before its user", Optimizer);
        gcmFOOTER_ARG("status=%d", gcvSTATUS_CHANGED);
        return gcvSTATUS_CHANGED;
    }
    else
    {
        /* Code unchanged. */
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
}

#if gcdUSE_WCLIP_PATCH
/* find the temp register assigned to gl_position output */
static gctINT
_findPosTemp(
    IN gcSHADER Shader
    )
{
    gctUINT  outputCount = Shader->outputCount;
    gctINT   posTemp = -1;
    gctUINT  i;

    gcmASSERT(Shader->type == gcSHADER_TYPE_VERTEX);
    for (i = 0; i < outputCount; i++)
    {
        gcOUTPUT output = Shader->outputs[i];

        /* output could be null if it is not used */
        if (output == gcvNULL)
            continue;
        if (output->nameLength == gcSL_POSITION)
        {
            posTemp = output->tempIndex;
        }
    }
    return posTemp;
}

static void
_analyzeGlPositionDependency(
    IN gcOPTIMIZER Optimizer
    )
{
    gctINT32    posTemp;
    gcOPT_CODE  code, nextCode;


    gcmASSERT(Optimizer->shader->type == gcSHADER_TYPE_VERTEX);

    /* vertex shader specific analysis:
         1. find if the gl_position.z is depended on gl_position.w
             1.1  gl_Position.z = gl_Position.w
             1.2  gl_Position   = gl_Position.xyww
             1.3  tpos.z      = tpos.w;
                  gl_Position = tpos;
             1.4  gl_Position = tpos.xyww;
     */

    /* find the temp register which holds the gl_position */
    posTemp = _findPosTemp(Optimizer->shader);

    if (posTemp >= 0) {
        /* found the pos temp register in use */
        for (code = Optimizer->codeHead; code; code = nextCode)
        {
            gcSL_ENABLE enable = gcmSL_TARGET_GET(code->instruction.temp, Enable);
            nextCode = code->next;
            /* Skip if indexed. */
            if (gcmSL_TARGET_GET(code->instruction.temp, Indexed) !=
                                                        gcSL_NOT_INDEXED)
                continue;
            if (code->instruction.tempIndex == (gctUINT32)posTemp &&
                (enable & gcSL_ENABLE_Z)) {
                /* the gl_Position.z is enabled, check if is depended on
                   gl_Position.w directly or indirectly:
                          gl_Position.z = gl_Position.w;
                      or
                          temp.x = gl_Position.w;
                          gl_Position.z = temp.x;
                      or
                          gl_Position.z = gl_Position.w - 0.005;
                 */
                gctBOOL dependOnW = gcvFALSE;
                gcOPT_CODE  code1 = code;
                /* the enable in the code1 coresponding to gl_position.z */
                gcSL_ENABLE code1Enable1 = gcSL_ENABLE_Z;
                /* the enable in the code1 coresponding to gl_position.w */
                gcSL_ENABLE code1Enable2 = gcSL_ENABLE_W; /* start with gl_position.w */
                gctBOOL posWEnabled = (enable & gcSL_ENABLE_W) != 0;
                if (posWEnabled) {
                    /* both z and w components are enabled in the code
                       need to track the both components
                     */
                    if (code1->instruction.opcode == gcSL_MOV &&
                           gcmSL_SOURCE_GET(code1->instruction.source0, Type) == gcSL_TEMP)
                    {
                        /* get the source register*/
                        gctUINT32 srcTemp = code1->instruction.source0Index;
                        /* get the swizzle of the enabled component */
                        gctUINT16 srcSwizzle1 =
                            _GetSwizzle(code1Enable1 == gcSL_ENABLE_X ? gcSL_SWIZZLE_X :
                                        code1Enable1 == gcSL_ENABLE_Y ? gcSL_SWIZZLE_Y :
                                        code1Enable1 == gcSL_ENABLE_Z ? gcSL_SWIZZLE_Z :
                                                                       gcSL_SWIZZLE_W,
                                        code1->instruction.source0);
                        gctUINT16 srcSwizzle2 =
                            _GetSwizzle(code1Enable2 == gcSL_ENABLE_X ? gcSL_SWIZZLE_X :
                                        code1Enable2 == gcSL_ENABLE_Y ? gcSL_SWIZZLE_Y :
                                        code1Enable2 == gcSL_ENABLE_Z ? gcSL_SWIZZLE_Z :
                                                                       gcSL_SWIZZLE_W,
                                        code1->instruction.source0);
                        if ((srcTemp == (gctUINT32)posTemp && srcSwizzle1 == gcSL_SWIZZLE_W) ||
                            (srcSwizzle1 == srcSwizzle2))
                        {
                            /* found the gl_position.z depends on gl_position.w */
                            dependOnW = gcvTRUE;
                        }
                    }
                }
                else {
                    while (code1->instruction.opcode == gcSL_MOV &&
                           gcmSL_SOURCE_GET(code1->instruction.source0, Type) == gcSL_TEMP)
                    {
                        /* get the source register*/
                        gctUINT32 srcTemp = code1->instruction.source0Index;
                        /* get the swizzle of the enabled component */
                        gctUINT16 srcSwizzle =
                            _GetSwizzle(code1Enable1 == gcSL_ENABLE_X ? gcSL_SWIZZLE_X :
                                        code1Enable1 == gcSL_ENABLE_Y ? gcSL_SWIZZLE_Y :
                                        code1Enable1 == gcSL_ENABLE_Z ? gcSL_SWIZZLE_Z :
                                                                        gcSL_SWIZZLE_W,
                                        code1->instruction.source0);
                        if (srcTemp == (gctUINT32)posTemp && srcSwizzle == gcSL_SWIZZLE_W )
                        {
                            /* found the gl_position.z depends on gl_position.w */
                            dependOnW = gcvTRUE;
                            break;
                        }
                        /* tracking the assignee: which component is be used:
                              pl_position.z = temp1.x;

                           now we need to tracking the x component instead of
                           originally z component
                         */
                        code1Enable1 = srcSwizzle == gcSL_SWIZZLE_X ? gcSL_ENABLE_X :
                                       srcSwizzle == gcSL_SWIZZLE_Y ? gcSL_ENABLE_Y :
                                       srcSwizzle == gcSL_SWIZZLE_Z ? gcSL_ENABLE_Z :
                                                                      gcSL_ENABLE_W ;
                        gcmASSERT(code1->dependencies0 != gcvNULL);
                        code1 = code1->dependencies0->code;
                    }
                    if (!dependOnW)
                    {
                        /* after handle all MOV operation, didn't find the depends */
                        gctSOURCE_t src0 = code1->instruction.source0;
                        gctSOURCE_t src1 = code1->instruction.source1;

                        /* ckeck src0 first */
                        if (gcmSL_SOURCE_GET(src0, Type) == gcSL_TEMP &&
                            gcmSL_SOURCE_GET(src0, Indexed) == gcSL_NOT_INDEXED)
                        {
                            /* get the source register*/
                            gctUINT32 srcTemp = code1->instruction.source0Index;
                            /* get the swizzle of the enabled component */
                            gctUINT16 srcSwizzle =
                                _GetSwizzle(code1Enable1 == gcSL_ENABLE_X ? gcSL_SWIZZLE_X :
                                            code1Enable1 == gcSL_ENABLE_Y ? gcSL_SWIZZLE_Y :
                                            code1Enable1 == gcSL_ENABLE_Z ? gcSL_SWIZZLE_Z :
                                                                            gcSL_SWIZZLE_W,
                                            code1->instruction.source0);
                            if (srcTemp == (gctUINT32)posTemp && srcSwizzle == gcSL_SWIZZLE_W )
                            {
                                /* found the gl_position.z depends on gl_position.w */
                                dependOnW = gcvTRUE;
                            }
                        }

                        /* check src1 if it exists */
                        if (!dependOnW &&
                            gcmSL_SOURCE_GET(src1, Type) == gcSL_TEMP &&
                            gcmSL_SOURCE_GET(src1, Indexed) == gcSL_NOT_INDEXED)
                        {
                            /* get the source register*/
                            gctUINT32 srcTemp = code1->instruction.source1Index;
                            /* get the swizzle of the enabled component */
                            gctUINT16 srcSwizzle =
                                _GetSwizzle(code1Enable1 == gcSL_ENABLE_X ? gcSL_SWIZZLE_X :
                                            code1Enable1 == gcSL_ENABLE_Y ? gcSL_SWIZZLE_Y :
                                            code1Enable1 == gcSL_ENABLE_Z ? gcSL_SWIZZLE_Z :
                                                                            gcSL_SWIZZLE_W,
                                            code1->instruction.source1);
                            if (srcTemp == (gctUINT32)posTemp && srcSwizzle == gcSL_SWIZZLE_W )
                            {
                                /* found the gl_position.z depends on gl_position.w */
                                dependOnW = gcvTRUE;
                            }
                        }
                    }
                }
                Optimizer->shader->vsPositionZDependsOnW = dependOnW;
            }
        }
    }
    return;
}
#endif

#define UNDEFINED_REG_FULLY_FIX  1

static gctBOOL
_InsertInitializerInstAtEntry(
    IN gcOPTIMIZER Optimizer,
    IN gcOPT_CODE Code,
    IN gctINT Source,
    IN OUT gcOPT_CODE *TempInitialized
    )
{
    gctBOOL instAdded = gcvFALSE;
    gctUINT32 tempIndex = (Source == 0)
                                ? Code->instruction.source0Index : Code->instruction.source1Index;
    gcOPT_CODE beginCode;
    gctSOURCE_t source = (Source == 0)
                                ? Code->instruction.source0 : Code->instruction.source1;
    gcSL_FORMAT format = gcmSL_SOURCE_GET(source, Format);
    gcSL_PRECISION precision = gcmSL_SOURCE_GET(source, Precision);

    /* Get the head code. */
    if (Code->function)
    {
        beginCode = Code->function->codeHead;
    }
    else
    {
        beginCode = Optimizer->main->codeHead;
    }

    /* If this temp is not initialized before, insert a initilizer. */
    if (TempInitialized[tempIndex] == gcvNULL)
    {
        /* Because we only check local variables, we can ignore other dependencies. */
        gcOPT_CODE insertCode = gcvNULL;

        gcmVERIFY_OK(gcOpt_AddCodeBefore(Optimizer, beginCode, &insertCode));

        gcoOS_ZeroMemory(&insertCode->instruction, sizeof(struct _gcSL_INSTRUCTION));

        /* Insert a MOV instruction. */
        insertCode->instruction.opcode = gcSL_MOV;
        insertCode->instruction.temp = gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_XYZW)
                                                 | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                                                 | gcmSL_TARGET_SET(0, Format, format)
                                                 | gcmSL_TARGET_SET(0, Precision, precision)
                                                 | gcmSL_TARGET_SET(0, Condition, gcSL_ALWAYS);
        insertCode->instruction.tempIndex = tempIndex;
        insertCode->instruction.source0 = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                                                    | gcmSL_SOURCE_SET(0, Format, format)
                                                    | gcmSL_SOURCE_SET(0, Precision, precision)
                                                    | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYZW);

        /* Set flag. */
        TempInitialized[tempIndex] = insertCode;
        instAdded = gcvTRUE;
    }

    /* Delete the undefined index. */
    gcmVERIFY_OK(gcOpt_DeleteIndexFromList(
                                            Optimizer,
                                            (Source == 0) ? &Code->dependencies0 : &Code->dependencies1,
                                            gcvOPT_UNDEFINED_REGISTER));
    gcmVERIFY_OK(gcOpt_DeleteIndexFromList(
                                            Optimizer,
                                            (Source == 0) ? &Code->dependencies0 : &Code->dependencies1,
                                            gcvOPT_JUMPUNDEFINED_REGISTER));

    /* Insert the new dependency. */
    gcmVERIFY_OK(gcOpt_AddCodeToList(
                                            Optimizer,
                                            (Source == 0) ? &Code->dependencies0 : &Code->dependencies1,
                                            TempInitialized[tempIndex]));

    /* Update the users. */
    gcmVERIFY_OK(gcOpt_AddCodeToList(
                                            Optimizer,
                                            &TempInitialized[tempIndex]->users,
                                            Code));
    return instAdded;
}

static void
_ChangeSrcToImmZero(
    gcSL_INSTRUCTION    Inst,
    gctINT              SrcNo
    )
{
    if (SrcNo == 0)
    {
        Inst->source0        = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                             | gcmSL_SOURCE_SET(0, Format, gcmSL_SOURCE_GET(Inst->source0, Format));
        Inst->source0Index   = 0;
        Inst->source0Indexed = 0;
    }
    else
    {
        Inst->source1        = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                             | gcmSL_SOURCE_SET(0, Format, gcmSL_SOURCE_GET(Inst->source1, Format));
        Inst->source1Index   = 0;
        Inst->source1Indexed = 0;
    }
}

static gctUINT
_CheckSrcUndefinedPerInst(
    IN gcOPTIMIZER  Optimizer,
    IN gcOPT_CODE   Code,
    IN OUT gcOPT_CODE* TempInitialized,
    OUT gctUINT* InstAdded
    )
{
    gctUINT         modifiedCode = 0;
    gcOPT_LIST      list;
    gctINT          thisIndex;
    gcOPT_TEMP      temp;
    gctBOOL         skip;

    gcmASSERT(InstAdded);

    /* Src 0 */
    skip = gcvFALSE;

    /* We only check when the source is a temp register */
    if (gcmSL_SOURCE_GET(Code->instruction.source0, Type) != gcSL_TEMP)
    {
        skip = gcvTRUE;
    }

    if (!skip)
    {
        if (Code->dependencies0)
        {
            temp = Optimizer->tempArray + Code->instruction.source0Index;
            if (temp->arrayVariable)
            {
                /* They should be as SGV attributes.
                   We should not initialize the input arguemnts. */
                skip = (temp->arrayVariable->nameLength == gcSL_VERTEX_ID ||
                        temp->arrayVariable->nameLength == gcSL_INSTANCE_ID ||
                        temp->arrayVariable->_varCategory == gcSHADER_VAR_CATEGORY_FUNCTION_INPUT_ARGUMENT ||
                        temp->arrayVariable->_varCategory == gcSHADER_VAR_CATEGORY_FUNCTION_INOUT_ARGUMENT);
            }
        }
        list = Code->dependencies0;
        while (list != gcvNULL && !skip &&
               (gcmSL_SOURCE_GET(Code->instruction.source0, Indexed) == gcSL_NOT_INDEXED))
        {
            thisIndex= list->index;

            /* Right now we can only check undefine local variables, not global variables.*/
            if (thisIndex == gcvOPT_UNDEFINED_REGISTER ||
                thisIndex == gcvOPT_JUMPUNDEFINED_REGISTER)
            {
                /* If the whole register is undefined, we change the source to immediate zero,
                ** otherwise, we insert a initialization inst at the beginning of this function.
                */

                /* The whole register is undefined. */
                if (list == Code->dependencies0 && list->next == gcvNULL)
                {
                    _ChangeSrcToImmZero(&Code->instruction, 0);
                    modifiedCode++;

                    list = list->next;
                    gcOpt_DeleteIndexFromList(Optimizer, &Code->dependencies0, thisIndex);
                    continue;
                }
#if UNDEFINED_REG_FULLY_FIX
                else
                {
                    /* Partial register is undefined. */
                    if(_InsertInitializerInstAtEntry(Optimizer, Code, 0, TempInitialized))
                    {
                        ++(*InstAdded);
                    }
                    modifiedCode++;
                    break;
                }
#endif
            }

            list = list->next;
        }
    }

    /* Src 1 */
    skip = gcvFALSE;

    /* We only check when the source is a temp register */
    if (gcmSL_SOURCE_GET(Code->instruction.source1, Type) != gcSL_TEMP)
    {
        skip = gcvTRUE;
    }

    if (!skip)
    {
        if (Code->dependencies1)
        {
            temp = Optimizer->tempArray + Code->instruction.source1Index;
            if (temp->arrayVariable)
            {
                /* They should be as SGV attributes */
                skip = (temp->arrayVariable->nameLength == gcSL_VERTEX_ID ||
                        temp->arrayVariable->nameLength == gcSL_INSTANCE_ID);
            }
        }
        list = Code->dependencies1;
        while (list != gcvNULL && !skip &&
               (gcmSL_SOURCE_GET(Code->instruction.source1, Indexed) == gcSL_NOT_INDEXED))
        {
            thisIndex= list->index;

            if (thisIndex == gcvOPT_UNDEFINED_REGISTER ||
                thisIndex == gcvOPT_JUMPUNDEFINED_REGISTER)
            {
                if (list == Code->dependencies1 && list->next == gcvNULL)
                {
                    _ChangeSrcToImmZero(&Code->instruction, 1);
                    modifiedCode++;

                    list = list->next;
                    gcOpt_DeleteIndexFromList(Optimizer, &Code->dependencies1, thisIndex);
                    continue;
                }
#if UNDEFINED_REG_FULLY_FIX
                else
                {
                    /* Partial register is undefined. */
                    if(_InsertInitializerInstAtEntry(Optimizer, Code, 1, TempInitialized))
                    {
                        ++(*InstAdded);
                    }
                    modifiedCode++;
                    break;
                }
#endif
            }

            list = list->next;
        }
    }

    return modifiedCode;
}

static gceSTATUS
_analyzeUndefinedRegisters(
    IN gcOPTIMIZER Optimizer,
    OUT gctUINT* ModifiedCode
    )
{
    gceSTATUS         status = gcvSTATUS_OK;
    gcOPT_FUNCTION    function;
    gcOPT_CODE        code;
    gctSIZE_T         i;

#if UNDEFINED_REG_FULLY_FIX
    gctPOINTER        pointer = gcvNULL;
    gcOPT_CODE*       TempInitialized;
#endif
    gctUINT inst_added;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

#if UNDEFINED_REG_FULLY_FIX
    if(Optimizer->tempCount > 0)
    {
        gcmERR_RETURN(gcoOS_Allocate(gcvNULL, Optimizer->tempCount * sizeof(gcOPT_CODE), &pointer));
        TempInitialized = pointer;
        gcoOS_ZeroMemory(TempInitialized, Optimizer->tempCount * sizeof(gcOPT_CODE));
    }
    else
    {
        return status;
    }
#else
    gctBOOL*          TempInitialized = gcvNULL;
#endif

    *ModifiedCode = 0;

    /* Subroutines first */
    for (i = 0; i < Optimizer->functionCount; i++)
    {
        function = Optimizer->functionArray + i;

        /*
        ** If this function is a recompile stub function,
        ** its inputs may be another function's outputs and there is no assignments for those inputs.
        */
        if (function->shaderFunction &&
            IsFunctionRecompilerStub(function->shaderFunction))
        {
            continue;
        }

        inst_added = 0;
        for (code = function->codeHead; code != gcvNULL && code != function->codeTail->next; code = code->next)
        {
            if (code->instruction.opcode != gcSL_CALL)
            {
                *ModifiedCode += _CheckSrcUndefinedPerInst(Optimizer, code, TempInitialized, &inst_added);
            }
        }
        if(inst_added)
        {
            gcOpt_UpdateCodeId(Optimizer);
        }
    }

    /* Now for main routine */
    function = Optimizer->main;
    inst_added = 0;
    for (code = function->codeHead; code != gcvNULL && code != function->codeTail->next; code = code->next)
    {
        if (code->instruction.opcode != gcSL_CALL)
        {
            *ModifiedCode += _CheckSrcUndefinedPerInst(Optimizer, code, TempInitialized, &inst_added);
        }

    }
    if(inst_added)
    {
        gcOpt_UpdateCodeId(Optimizer);
    }

#if UNDEFINED_REG_FULLY_FIX
    gcoOS_Free(gcvNULL, TempInitialized);

    /* Need fully reconstruct optimizer */
#endif

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
gcOpt_AnalysisCode(
    IN gcOPTIMIZER Optimizer
    )
{
    gctUINT           modifiedCode = 0;
#if !DX_SHADER
    gceSTATUS         status = gcvSTATUS_OK;
#endif
    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

#if gcdUSE_WCLIP_PATCH
    if (Optimizer->shader->type == gcSHADER_TYPE_VERTEX) {
        _analyzeGlPositionDependency(Optimizer);
    }
#endif

#if !DX_SHADER
    /* Sometimes, bad applications may introduce undefined temp register, if so we
       need firstly change it to zero before doing any opts */
    if (!gcUseFullNewLinker(Optimizer->hwCfg.hwFeatureFlags.hasHalti2))
    {
        gcmERR_RETURN(_analyzeUndefinedRegisters(Optimizer, &modifiedCode));
    }
#endif

    if (modifiedCode)
    {
        /* Code changed. */
        DUMP_OPTIMIZER("Analyze code to zero uninitialized variables", Optimizer);
        gcmFOOTER_ARG("status=%d", gcvSTATUS_CHANGED);
        return gcvSTATUS_CHANGED;
    }
    else
    {
        /* Code unchanged. */
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
}

/* return true if the Opcode is one of the instruction type need to
   split vec/3/vec4 operation to vec2/vec1 operation
 */
static gctBOOL
_codeNeedToSplit(
    gcSL_OPCODE Opcode
    )
{
    gcOPTIMIZER_OPTION *opt = gcmGetOptimizerOption();
    if (gcmOPT_SPLITVEC() &&
        ((Opcode == gcSL_MUL && opt->splitVec4MUL) ||
         (Opcode == gcSL_MULLO && opt->splitVec4MULLO) ||
         (Opcode == gcSL_DP3 && opt->splitVec4DP3) ||
         (Opcode == gcSL_DP4 && opt->splitVec4DP4)))
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL
_has3Or4Components(
    gcSL_INSTRUCTION Instruction
    )
{
    gcSL_ENABLE enable = gcmSL_TARGET_GET(Instruction->temp, Enable);
    gctINT count = 0;

    if (Instruction->opcode == gcSL_DP3 || Instruction->opcode == gcSL_DP4 ||
        Instruction->opcode == gcSL_CROSS)
        return gcvTRUE;

    if ((enable & gcSL_ENABLE_X) != 0) ++count;
    if ((enable & gcSL_ENABLE_Y) != 0) ++count;
    if ((enable & gcSL_ENABLE_Z) != 0) ++count;
    if ((enable & gcSL_ENABLE_W) != 0) ++count;

    return count > 2;
}

/*******************************************************************************
**                          gcOpt_PowerOptimization
********************************************************************************
**
**  Special power usage optimization:
**        1. add dummy TEXLD
**        2. insert NOPs
**        3. split vec3/vec4 to vec2/vec1 operations
**
**  INPUT:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_PowerOptimization(
    IN gcOPTIMIZER Optimizer
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    gctUINT     dummyTexldAdded = 0;
    gcOPT_CODE  code, nextCode;
    gctINT      codeChanged = 0, texldSeen = 0;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    for (code = Optimizer->codeHead; code; code = nextCode)
    {
        gcSL_OPCODE  opcode = gcmSL_OPCODE_GET(code->instruction.opcode, Opcode);
        nextCode = code->next;

        /* Test for texure load instruction. */
        if (gcmOPT_PATCH_TEXLD() &&
            gcSL_isOpcodeTexld(opcode) )
        {
            ++texldSeen;

            if (texldSeen >= Optimizer->globalOptions->patchEveryTEXLDs) {
                int i;
                gcOPT_CODE  insertBeforeCode = code;
                /* reset the texld seen */
                texldSeen = 0;
                ++codeChanged;
                if (code->prev)
                {
                    gcSL_OPCODE  prevOpcode =
                        gcmSL_OPCODE_GET(code->prev->instruction.opcode, Opcode);
                    /* the newly added texld shouldn't break the
                       TEXBIAS/TEXLOD/TEXGRAD/TEXGATHER/TEXFETCH_MS and TEDLD/TEXLDP pair */
                    if (gcSL_isOpcodeTexldModifier(prevOpcode))
                    {
                        insertBeforeCode = code->prev;
                        /* check if the insertBeforeCode->prev is a MOV,
                           don't break its tie with the gcSL_TEXBIAS too */
                        if (insertBeforeCode->prev &&
                            insertBeforeCode->prev->instruction.opcode == gcSL_MOV)
                        {
                            insertBeforeCode = insertBeforeCode->prev;
                        }
                    }
                }
                /* add the dummy TEXLDs */
                for (i=0; i < Optimizer->globalOptions->patchDummyTEXLDs; i++) {
                    gcOPT_CODE dummyTexld;
                    gctUINT samplerId = gcmSL_INDEX_GET(code->instruction.source0Index,
                                                        Index);
                    gcmASSERT(!gcmIsDummySamplerId(samplerId));

                    /* Add a new code before the texture load. */
                    gcmERR_RETURN(gcOpt_AddCodeBefore(Optimizer,
                                           insertBeforeCode, &dummyTexld));

                    dummyTexld->instruction   = code->instruction;
                    /* change TEXLDPROJ/TEXLDPCF/TEXLDPCFPROJ to TEXLD for dummy texld for simplicity */
                    if (code->instruction.opcode == gcSL_TEXLDPROJ ||
                        code->instruction.opcode == gcSL_TEXLDPCF ||
                        code->instruction.opcode == gcSL_TEXLDPCFPROJ)
                    {
                        dummyTexld->instruction.opcode = gcSL_TEXLD;
                    }

                    dummyTexld->id            = 0;  /* patch it later */

                    /* change the sampler id to dummy sampler id */
                    dummyTexld->instruction.source0Index =
                        gcmSL_INDEX_SET(dummyTexld->instruction.source0Index,
                                        Index, gcmMakeDummySamplerId(samplerId)) ;

                    dummyTexldAdded++;
               }
            }
        }
        if (gcmOPT_SPLITVEC() && _codeNeedToSplit(code->instruction.opcode) &&
            _has3Or4Components(&code->instruction))
        {
            gcSL_ENABLE enable = gcmSL_TARGET_GET(code->instruction.temp, Enable);
            gcSL_ENABLE enable1;
            gctBOOL s0Indexed =
                    gcmSL_SOURCE_GET(code->instruction.source0, Indexed) != gcSL_NOT_INDEXED;
            gctBOOL s1Indexed =
                    gcmSL_SOURCE_GET(code->instruction.source1, Indexed) != gcSL_NOT_INDEXED;
            gctBOOL targetIndexed =
                    gcmSL_TARGET_GET(code->instruction.temp, Indexed) != gcSL_NOT_INDEXED;
            if ((code->instruction.opcode == gcSL_MUL   ||
                 code->instruction.opcode == gcSL_MULLO ))

            {
                /* VEC4 MUL:
                       MUL r2, r1.wzyx, r0
                       ==>
                         MUL r2.xy, r1.wz, r0.xy
                         MUL r2.zw, r1.yz, r0.zw

                   VEC3 MUL:
                       MUL r2.xyz, r1.wzy, r0.xyz
                       ==>
                         MUL r2.xy, r1.wz, r0.xy
                         MUL r2.z, r1.y, r0.z
                */
                gcOPT_CODE splitCode;
                /* gcSL_SWIZZLE swizzleX, swizzleY, swizzleZ, swizzleW; */
                /* Add a new code after the texture load. */
                gcmERR_RETURN(gcOpt_AddCodeAfter(Optimizer, code, &splitCode));

                splitCode->instruction   = code->instruction;
                splitCode->id            = 0;  /* patch it later */

                /* change the original target enable to remove zw components*/
                enable1 = enable & ~(gcSL_ENABLE_Z | gcSL_ENABLE_W);
                code->instruction.temp =
                    gcmSL_TARGET_SET(code->instruction.temp, Enable, enable1 );

                /* change the new code target enable to remove xy components */
                enable1 = enable & ~(gcSL_ENABLE_X | gcSL_ENABLE_Y);
                splitCode->instruction.temp =
                    gcmSL_TARGET_SET(splitCode->instruction.temp, Enable, enable1 );

                /* handle data flow */
                ++codeChanged;
            }
            else if ((code->instruction.opcode == gcSL_DP3 ||
                      code->instruction.opcode == gcSL_DP4)   &&
                     !(s0Indexed || s1Indexed || targetIndexed)  )
            {
                /*
                    For DP4 R2.x, R0, R1
                      R2.xy = R0.xy * R1.xy
                      R2.zw = R0.zw * R1.zw
                      R2.xy = R2.xy + R2.zw
                      R2.xyzw = R2.xxxxx + R2.yyyy

                    For DP3 R2.x, R0.xyz, R1.xyz
                      R2.xy = R0.xy * R1.xy
                      R2.z = R0.z * R1.z
                      R2.x = R2.x + R2.y
                      R2.xyzw = R2.xxxxx + R2.zzzzz

                    s0 and s1 could be constant.
                 */
                gcOPT_CODE  mul1Code = code, mul2Code, add1Code, add2Code;
                gctBOOL     dp4      = (code->instruction.opcode == gcSL_DP4);
                gcSL_FORMAT format   = gcmSL_TARGET_GET(code->instruction.temp,
                                                        Format);

                gcmERR_RETURN(gcOpt_AddCodeAfter(Optimizer, code, &mul2Code));

                mul2Code->instruction   = code->instruction;
                mul2Code->id            = 0;  /* patch it later */

                gcmERR_RETURN(gcOpt_AddCodeAfter(Optimizer, mul2Code, &add1Code));

                add1Code->instruction   = code->instruction;
                add1Code->id            = 0;  /* patch it later */

                gcmERR_RETURN(gcOpt_AddCodeAfter(Optimizer, add1Code, &add2Code));

                add2Code->instruction   = code->instruction;
                add2Code->id            = 0;  /* patch it later */

                /* change opcodes */
                mul1Code->instruction.opcode = gcSL_MUL;
                mul2Code->instruction.opcode = gcSL_MUL;
                add1Code->instruction.opcode = gcSL_ADD;
                add2Code->instruction.opcode = gcSL_ADD;

                /* change target enables */
                mul1Code->instruction.temp =
                    gcmSL_TARGET_SET(mul1Code->instruction.temp, Enable,
                                     gcSL_ENABLE_XY );

                mul2Code->instruction.temp =
                    gcmSL_TARGET_SET(mul2Code->instruction.temp, Enable,
                                     dp4 ? gcSL_ENABLE_ZW : gcSL_ENABLE_Z );

                add1Code->instruction.temp =
                    gcmSL_TARGET_SET(add1Code->instruction.temp, Enable,
                                     dp4 ? gcSL_ENABLE_XY : gcSL_ENABLE_X );

                add2Code->instruction.temp =
                    gcmSL_TARGET_SET(add2Code->instruction.temp, Enable,
                                     gcSL_ENABLE_XYZW);

                /* change add1Code, add2Code source type, no need to change
                   mul1Code and mul2Code type */

                add1Code->instruction.source0 =
                    gcmSL_SOURCE_SET(add1Code->instruction.source0, Type, gcSL_TEMP );
                add1Code->instruction.source1 =
                    gcmSL_SOURCE_SET(add1Code->instruction.source1, Type, gcSL_TEMP );
                add2Code->instruction.source0 =
                    gcmSL_SOURCE_SET(add2Code->instruction.source0, Type, gcSL_TEMP );
                add2Code->instruction.source1 =
                    gcmSL_SOURCE_SET(add2Code->instruction.source1, Type, gcSL_TEMP );

                /* change add1Code, add2Code source format, no need to change
                   mul1Code and mul2Code format */

                add1Code->instruction.source0 =
                    gcmSL_SOURCE_SET(add1Code->instruction.source0, Format, format );
                add1Code->instruction.source1 =
                    gcmSL_SOURCE_SET(add1Code->instruction.source1, Format, format );
                add2Code->instruction.source0 =
                    gcmSL_SOURCE_SET(add2Code->instruction.source0, Format, format );
                add2Code->instruction.source1 =
                    gcmSL_SOURCE_SET(add2Code->instruction.source1, Format, format );

                /* change source swizzles, no need to change mul1Code and
                   mul2Code swizzle */

                add1Code->instruction.source0 =
                    gcmSL_SOURCE_SET(add1Code->instruction.source0, Swizzle,
                                     dp4 ? gcSL_SWIZZLE_XYZW : gcSL_SWIZZLE_XXXX );
                add1Code->instruction.source1 =
                    gcmSL_SOURCE_SET(add1Code->instruction.source1, Swizzle,
                                     dp4 ? gcSL_SWIZZLE_ZWWW : gcSL_SWIZZLE_YYYY );
                add2Code->instruction.source0 =
                    gcmSL_SOURCE_SET(add2Code->instruction.source0, Swizzle,
                                     gcSL_SWIZZLE_XXXX );
                add2Code->instruction.source1 =
                    gcmSL_SOURCE_SET(add2Code->instruction.source1, Swizzle,
                                     dp4 ? gcSL_SWIZZLE_YYYY : gcSL_SWIZZLE_ZZZZ );

                /* change source indexed, no need to change mul1Code and
                   mul2Code indexed */

                add1Code->instruction.source0 =
                    gcmSL_SOURCE_SET(add1Code->instruction.source0, Indexed,
                                     gcSL_NOT_INDEXED );
                add1Code->instruction.source1 =
                    gcmSL_SOURCE_SET(add1Code->instruction.source1, Indexed,
                                     gcSL_NOT_INDEXED );
                add2Code->instruction.source0 =
                    gcmSL_SOURCE_SET(add2Code->instruction.source0, Indexed,
                                     gcSL_NOT_INDEXED );
                add2Code->instruction.source1 =
                    gcmSL_SOURCE_SET(add2Code->instruction.source1, Indexed,
                                     gcSL_NOT_INDEXED );

                /* change add1Code and add2Code source temp index */
                add1Code->instruction.source0Index =
                        gcmSL_INDEX_SET(add1Code->instruction.source0Index,
                                        Index, code->instruction.tempIndex);
                add1Code->instruction.source1Index =
                        gcmSL_INDEX_SET(add1Code->instruction.source1Index,
                                        Index, code->instruction.tempIndex);
                add2Code->instruction.source0Index =
                        gcmSL_INDEX_SET(add2Code->instruction.source0Index,
                                        Index, code->instruction.tempIndex);
                add2Code->instruction.source1Index =
                        gcmSL_INDEX_SET(add2Code->instruction.source1Index,
                                        Index, code->instruction.tempIndex);

                /* change add1Code and add2Code source index ConstValue */
                add1Code->instruction.source0Index =
                        gcmSL_INDEX_SET(add1Code->instruction.source0Index,
                                        ConstValue, 0);
                add1Code->instruction.source1Index =
                        gcmSL_INDEX_SET(add1Code->instruction.source1Index,
                                        ConstValue, 0);
                add2Code->instruction.source0Index =
                        gcmSL_INDEX_SET(add2Code->instruction.source0Index,
                                        ConstValue, 0);
                add2Code->instruction.source1Index =
                        gcmSL_INDEX_SET(add2Code->instruction.source1Index,
                                        ConstValue, 0);
            }
        }
    }

    if (codeChanged != 0)
    {
        /* Update code id because the checkings depend on it. */
        gcOpt_UpdateCodeId(Optimizer);
        DUMP_OPTIMIZER("Patch texld", Optimizer);
        gcmFOOTER_ARG("status=%d", gcvSTATUS_CHANGED);
        return gcvSTATUS_CHANGED;
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcOpt_LoadSWW(
    IN gcOPTIMIZER Optimizer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT codeMoved = 0;
    gcOPT_TEMP tempArray = Optimizer->tempArray;
    gcOPT_CODE code, nextLoadCode, endCode, lastUserCode, iterCode;
    gcOPT_LIST list, list1;
    gctUINT nextLoadCodeId, id;
    gctINT * loadUsers = gcvNULL;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    if (Optimizer->shader->loadUsers)
    {
        /* Free the old load users. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Optimizer->shader->loadUsers));
    }

    gcOpt_UpdateCodeId(Optimizer);

    /* Move users before next LOAD instruction if possible. */
    for (code = Optimizer->codeHead; code; code = nextLoadCode)
    {
        gcOPT_TEMP temp;
        nextLoadCode = code->next;

        /* Test for MOV instruction. */
        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_LOAD) continue;

        /* Skip if no users. */
        if (! code->users) continue;

        /* Skip if indexed. */
        if (gcmSL_TARGET_GET(code->instruction.temp, Indexed) != gcSL_NOT_INDEXED) continue;


        /* Get pointer to temporary register. */
        temp = tempArray + code->instruction.tempIndex;

        /* Skip if target is part of an array. */
        if (temp->arrayVariable && GetVariableKnownArraySize(temp->arrayVariable) > 1) continue;

        /* Skip if not all used components are enabled. */
        if (gcmSL_TARGET_GET(code->instruction.temp, Enable) != temp->usage) continue;

        /* Find next LOAD instruction. */
        for (nextLoadCode = code->next; nextLoadCode; nextLoadCode = nextLoadCode->next)
        {
            if (gcmSL_OPCODE_GET(nextLoadCode->instruction.opcode, Opcode) == gcSL_LOAD) break;
        }
        if (nextLoadCode == gcvNULL) break;

        /* Check if all users can be moved before next LOAD instruction. */
        nextLoadCodeId = nextLoadCode->id;
        lastUserCode = code;
        for (list = code->users; list; list = list->next)
        {
            /* Skip output and global registers. */
            if (list->index < 0) break;

            /* Stop if any jump in/out. */
            iterCode = list->code;
            if (iterCode->callers) break;
            if (gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_JMP ||
                gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_CALL) break;

            /* Skip MULSAT and ADDSAT if they are together. */
            if (gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_MULSAT)
            {
                if (iterCode->next &&
                    (gcmSL_OPCODE_GET(iterCode->next->instruction.opcode, Opcode) == gcSL_ADDSAT ||
                     gcmSL_OPCODE_GET(iterCode->next->instruction.opcode, Opcode) == gcSL_SUBSAT)) break;
            }
            else if (gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_ADDSAT ||
                     gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_SUBSAT)
            {
                if (iterCode->prev &&
                    gcmSL_OPCODE_GET(iterCode->prev->instruction.opcode, Opcode) == gcSL_MULSAT) break;
            }

            /* Skip IMAGE_SAMPLER and IMAGE_RD  */
            if (gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_IMAGE_SAMPLER ||
                gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_IMAGE_RD ||
                gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_IMAGE_RD_3D)
            {
                break;
            }

            /* If the user is gcSL_LOAD, it can be only the nextLoadCode. */
            /* Stop if gcSL_LOAD for now. */
            if (gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_LOAD /*&&
                iterCode != nextLoadCode*/) break;

            if (iterCode->id > lastUserCode->id)
            {
                lastUserCode = list->code;
            }

            /* Skip if users have prevDefines after nextLoadCode. */
            for (list1 = iterCode->prevDefines; list1; list1 = list1->next)
            {
                if (list1->code && list1->code->id >= nextLoadCodeId) break;
            }
            if (list1) break;

            /* Skip if users have other dependencies after nextLoadCode. */
            for (list1 = iterCode->dependencies0; list1; list1 = list1->next)
            {
                if (list1->code && list1->code->id >= nextLoadCodeId) break;
            }
            if (list1) break;
            for (list1 = iterCode->dependencies1; list1; list1 = list1->next)
            {
                if (list1->code && list1->code->id >= nextLoadCodeId) break;
            }
            if (list1) break;
        }
        if (list) continue;

        /* Check if there is any jump in/out between code and lastUserCode. */
        endCode = lastUserCode->next;
        for (iterCode = code->next; iterCode != endCode; iterCode = iterCode->next)
        {
            /* Stop if any jump in/out. */
            if (iterCode->callers) break;
            if (gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_JMP ||
                gcmSL_OPCODE_GET(iterCode->instruction.opcode, Opcode) == gcSL_CALL) break;
        }
        if (iterCode != endCode) continue;

        /* Move users before nextLoadCode. */
        for (list = code->users; list; list = list->next)
        {
            iterCode = list->code;
            if (iterCode->id > nextLoadCodeId)
            {
                gcOpt_MoveCodeListBefore(Optimizer, iterCode, iterCode, nextLoadCode);
                lastUserCode = iterCode;
            }
        }

        /* Update code id because the checkings depend on it. */
        /*gcOpt_UpdateCodeId(Optimizer);*/
        for (id = code->id + 1, iterCode = code->next;
             iterCode != endCode;
             id++, iterCode = iterCode->next)
        {
            if (iterCode->id != id)
            {
                iterCode->id = id;
            }
        }
        codeMoved++;

        if (Optimizer->shader->loadUsers == gcvNULL)
        {
            gctSIZE_T codeCount = Optimizer->codeTail->id + 1;
            gctPOINTER pointer = gcvNULL;
            gctSIZE_T bytes;
            gctUINT i;

            /* Allocate new loadUsers. */
            bytes = (gctSIZE_T)(codeCount * gcmSIZEOF(gctINT));
            gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));

            loadUsers = Optimizer->shader->loadUsers = pointer;

            /* Initialize all entries to -1. */
            for (i = 0; i < codeCount; i++)
            {
                loadUsers[i] = -1;
            }
        }

        loadUsers[code->id] = lastUserCode->id;
    }

    if (codeMoved != 0)
    {
        DUMP_OPTIMIZER("Moved user instructions for LOAD the shader", Optimizer);
        gcmFOOTER_ARG("status=%d", gcvSTATUS_CHANGED);
        return gcvSTATUS_CHANGED;
    }
    else
    {
        /* Code unchanged. */
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                          gcOpt_OptimizeBranches
********************************************************************************
**
**  Optimize branches.
**
**  IN:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_OptimizeBranches(
    IN gcOPTIMIZER Optimizer
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    DUMP_OPTIMIZER("Optimized branches in the shader", Optimizer);
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                          gcOpt_RemoveCommonExpression
********************************************************************************
**
**  Remove common sub-expressions.
**
**  INPUT:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_RemoveCommonExpression(
    IN gcOPTIMIZER Optimizer
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    DUMP_OPTIMIZER("Removed common sub-expressions from the shader", Optimizer);

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                          gcOpt_MoveLoopInvariants
********************************************************************************
**
**  Move loop invariants out of the loops.
**
**  INPUT:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_MoveLoopInvariants(
    IN gcOPTIMIZER Optimizer
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    DUMP_OPTIMIZER("Moved loop invariants out of loops in the shader", Optimizer);

    gcmFOOTER();
    return status;
}

#define SPLIT_TEMP_LIVERANGE 0
#if SPLIT_TEMP_LIVERANGE
static gceSTATUS
_listAllocate(
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Memory
    )
{
    return gcoOS_Allocate(gcvNULL, Bytes, Memory);
};

static gceSTATUS
_listFree(
    IN gctPOINTER Memory
    )
{
    return gcoOS_Free(gcvNULL, Memory);
}

static const gcsAllocator listAllocator = {_listAllocate, _listFree };

typedef struct SplitTempInfo
{
    gcLINKTREE_TEMP         originTemp;
    gcsLINKTREE_LIST_PTR    defined;
    gcsLINKTREE_LIST_PTR    dependencies;
    gcsLINKTREE_LIST_PTR    users;

    gctUINT8                usage;     /* Usage flags for the split temp */
    gctINT                  lastUse;   /* last use for the split temp */
} SplitTempInfo;

typedef gcsList  gctSplitTempList;
typedef gctSplitTempList * gctSplitTempListPtr;


static gctBOOL
_isDominatedBy(
    IN gcLINKTREE         Tree,
    IN gcLINKTREE_TEMP    Temp,
    IN gctINT             Def1,
    IN gctINT             Def2
    )
{
    gctINT        crossLoopIdx;

    if (gcOpt_isCodeInSameBB(Tree, Def1, Def2, Temp, &crossLoopIdx))
    {
        return gcvTRUE;
    }

    /* handle in same Extended basic blok case:
     *
     *    def1:  t1 = ...;
     *           if (cond) goto L1
     *           stmt1
     *    L1:    stmt2
     *           t1 = ...
     */
    return gcvFALSE;
}

static gctBOOL
_isLiveRangeDisjoint(
    IN gcLINKTREE              Tree,
    IN gcLINKTREE_TEMP         Temp,
    IN  gcsLINKTREE_LIST_PTR   Def,
    IN  gcsLINKTREE_LIST_PTR   DefList,
    OUT gctINT *               LastUse
    )
{
    gctBOOL retval = gcvFALSE;

    if (DefList == gcvNULL)
    {
        return gcvTRUE;
    }
    else if (Def->index > DefList->index)
    {
        gcsLINKTREE_LIST_PTR def1, def2, prevDef = gcvNULL;
        def1 = DefList;
        def2 = Def;
        /* find the prevDef */
        if (Def != Temp->defined)
        {
            prevDef = Temp->defined;
            while (prevDef && prevDef->next != Def)
                prevDef = prevDef->next;
        }
        if (_isDominatedBy(Tree, Temp, def1->index, def2->index))
        {
            gcSL_INSTRUCTION inst2    = Tree->shader->code + def2->index;
            gcSL_ENABLE      enable2  = gcmSL_TARGET_GET(inst2->temp, Enable);
            gcsLINKTREE_LIST_PTR user = Temp->users;

            /* find last use, the user is sorted from last to first */
            while (user && prevDef && user->index > prevDef->index)
            {
                user = user->next;
            }
            *LastUse = user ? user->index : def1->index;
            /* check if all components defined by def1 are killed */
            if (enable2 == Temp->usage)
            {
                return gcvTRUE;
            }
            else
            {
                gcSL_INSTRUCTION useInst;
                gcSL_TYPE        type;

                user = Temp->users;
                /* check if all the usage after def2 only use what
                 *  it's defined */
                while (user && user->index > def2->index)
                {
                    gcSL_ENABLE sourceUsage;
                    gctSOURCE_t source, index;
                    gctINT      i;

                    useInst = Tree->shader->code + user->index;
                    /* check source 0 and 1 */
                    for (i=0; i<2; i++)
                    {
                        source = (i==0) ? useInst->source0
                                        : useInst->source1;
                        index  = (i==0) ? useInst->source0Index
                                        : useInst->source1Index;
                        type = gcmSL_SOURCE_GET(source, Type);
                        if (type == user->type &&
                            index == user->index )
                        {
                            sourceUsage =
                                gcSL_ConvertSwizzle2Enable((gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleX),
                                                           (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleY),
                                                           (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleZ),
                                                           (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleW));
                            if ((sourceUsage & ~enable2) != gcSL_ENABLE_NONE)
                            {
                                /* use component not defined by def2 */
                                return gcvFALSE;
                            }
                        }
                    }
                }
                /* no one uses components not defined by def2, which
                 * means def2 killed all live components defined by def1
                 */
                return gcvTRUE;
            }
        }
    }
    return retval;
}

/* split disjoint temp definitions to SplitTempList,
 * handle the definition's users and dependencies properly
 */
static gceSTATUS
_splitTemp(
    IN gcOPTIMIZER             Optimizer,
    IN gcOPT_CODE              Code,
    IN OUT gctSplitTempListPtr SplitTempList
    )
{
    gceSTATUS            status  = gcvSTATUS_OK;
    gctPOINTER           pointer = gcvNULL;
    gcOPT_TEMP_DEFINE    temp    = Code->tempDefine;
    gcsLINKTREE_LIST_PTR defined, nextDefined;
    gctINT               lastUse;

    defined = Temp->defined;
    while (defined)
    {
        nextDefined = defined->next;
        if (nextDefined == gcvNULL)
            break;
        /* check if defined and next defines are disjoint */
        if (_isLiveRangeDisjoint(Tree, Temp, defined, nextDefined, &lastUse))
        {
            gcsLINKTREE_LIST_PTR user, nextUser;
            gcsLINKTREE_LIST_PTR dependency, nextDependency;
            gcSL_INSTRUCTION     inst    = Tree->shader->code + defined->index;
            SplitTempInfo *      tempInfo;
            /* create split temp info */
            gcmONERROR(
                SplitTempList->allocator->allocate(sizeof(SplitTempInfo), &pointer));

            tempInfo = (SplitTempInfo *)pointer;
            tempInfo->originTemp   = Temp;
            tempInfo->defined      = defined;
            tempInfo->lastUse      = lastUse;
            tempInfo->usage        = gcmSL_TARGET_GET(inst->temp, Enable);
            tempInfo->users        = gcvNULL;
            tempInfo->dependencies = gcvNULL;

            /* transfer users */
            user = Temp->users;
            while (user)
            {
                nextUser = user->next;
                if (user->index > defined->index && user->index <= lastUse)
                {
                    _UnlinkNode(&Temp->users, user);
                    user->next = tempInfo->users;
                    tempInfo->users = user;
                }
                user = nextUser;
            }

            /* process dependencies:
             *   the dependcies could be shared, e.g.
             *    MOV  temp(12), temp(3).x
             *        use of temp(12)
             *    ADD  temp(12), temp(10), temp(3).y
             *
             *  for this case, we need to split the dependency node
             *  and adjust the reference count
             */
            dependency = Temp->dependencies;
            while (dependency)
            {
                gcSL_TYPE type;
                gctSOURCE_t source;
                gctINT    index;
                gctINT    i;
                nextDependency = dependency->next;

                for (i=0; i<2; i++)
                {
                    /* check source0 and source1 */
                    source = (i == 0) ? inst->source0
                                      : inst->source1;
                    index  = (i == 0) ? inst->source0Index
                                      : inst->source1Index;
                    type = gcmSL_SOURCE_GET(source, Type);
                    if (type  == dependency->type &&
                        index == dependency->index  )
                    {
                        if (dependency->counter > 1)
                        {
                            /* unlink decrease reference count */
                            _UnlinkNode(&Temp->dependencies, dependency);
                            gcmONERROR(gcLINKTREE_AddList(Tree,
                                                &tempInfo->dependencies,
                                                type,
                                                dependency->index));

                        }
                        else
                        {
                            /* unlink detatch dependency node from list */
                            _UnlinkNode(&Temp->dependencies, dependency);
                            /* add it to tempInfo */
                            dependency->next = tempInfo->dependencies;
                            tempInfo->dependencies = dependency;
                        }
                    }
                }
                dependency = nextDependency;
            }
            gcList_AddNode(SplitTempList, pointer);
            _UnlinkNode(&Temp->defined, defined);
        }
        /* go to next definition */
        defined = nextDefined;
    }

OnError:
    /* Return the status. */
    return status;
}

static void
_InitOptTemp(
    IN OUT gcOPT_TEMP          Temp,
    IN     gcSL_FORMAT         Format
    )
{
    Temp->inUse         = gcvFALSE;
    Temp->isGlobal      = gcvFALSE;
    Temp->isIndex       = gcvFALSE;
    Temp->usage         = 0;
    Temp->format        = Format;
    Temp->arrayVariable = gcvNULL;
    Temp->function      = gcvNULL;
    Temp->argument      = gcvNULL;
    Temp->tempInt       = -1;
    Temp->temp          = gcvNULL;
}

static gceSTATUS
_resizeOptimizerTempArray(
    IN gcOPTIMIZER   Optimizer,
    IN gctINT        NewTempCount
    )
{
    gceSTATUS       status        = gcvSTATUS_OK;
    gcSL_FORMAT     format        = gcSL_FLOAT;
    const gctSIZE_T tempNodeSize  = gcmSIZEOF(struct _gcOPT_TEMP);
    gctPOINTER      pointer       = gcvNULL;
    gcOPT_TEMP      newTempArray;
    gctINT          i;

    gcmASSERT(NewTempCount > (gctINT)Optimizer->tempCount);
    if (Optimizer->shader->type == gcSHADER_TYPE_CL)
    {
        format = gcSL_UINT32;
    }

    /* Allocate temporary register array. */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                               tempNodeSize * NewTempCount,
                               &pointer));

    newTempArray = (gcOPT_TEMP)pointer;
    gcoOS_MemCopy(newTempArray, Optimizer->tempArray,
                  tempNodeSize * Optimizer->tempCount);

    /* Initialize all temporary registers as not defined. */
    for (i = (gctINT)Optimizer->tempCount; i < NewTempCount; i++)
    {
        _InitOptTemp(&Optimizer->tempArray[i], format);
    }

    /* deallocate old tempArray */
    gcmONERROR(gcoOS_Free(gcvNULL, Optimizer->tempArray));

    /* Set temporary register count. */
    Optimizer->tempCount = NewTempCount;
    Optimizer->tempArray = newTempArray;

OnError:
    return status;
}

static gctBOOL
_hasFunctionCall(
     gcOPT_LIST List
     )
{
    while (List)
    {
        gcSL_OPCODE  opcode =
            gcmSL_OPCODE_GET(List->code->instruction.opcode, Opcode);
        if (opcode == gcSL_CALL)
            return gcvTRUE;
        List = List->next;
    }
    return gcvFALSE;
}

static gctBOOL
_hasMultiDependcies(
     gcOPT_LIST Users,
     gctINT     TempIndex
     )
{
    while (Users)
    {
        gcOPT_CODE userCode = Users->code;
        gcOPT_LIST curDep = userCode->dependencies0;
        gcSL_OPCODE  opcode =
            gcmSL_OPCODE_GET(Users->code->instruction.opcode, Opcode);
        if (opcode == gcSL_CALL)
            return gcvTRUE;
        Users = Users->next;
    }
    return gcvFALSE;
}

gceSTATUS
gcOpt_SplitRedefinedTemps(
    IN gcOPTIMIZER    Optimizer
    )
{
    gceSTATUS           status        = gcvSTATUS_OK;
    gctUINT             i;
    gctSplitTempList    splitTempList;
    gcsListNode *       splitTemp;
    gcOPT_CODE          code, nextCode;
    gctINT              codeChanged = 0;

    gcList_Init(&splitTempList, &listAllocator);

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    for (code = Optimizer->codeHead; code; code = nextCode)
    {
        gcSL_INSTRUCTION inst      = &code->instruction;
        gcOPT_TEMP       temp;
        gctINT           tempIndex = code->instruction.tempIndex;

        nextCode = code->next;

        if (code->nextDefines == gcvNULL || /* skip if no next definition */
            code->users       == gcvNULL || /* skip if no user */
            gcmSL_TARGET_GET(inst->temp, Indexed) != gcSL_NOT_INDEXED ||
            gcmSL_TARGET_GET(inst->temp, Enable) == gcSL_ENABLE_NONE  ||
            _hasFunctionCall(code->users) ||
            _hasMultiDependcies(code->users, tempIndex)
            )
        {
            continue;
        }

        /* Get pointer to temporary register. */
        temp = Optimizer->tempArray + tempIndex;
        /* Skip if target is part of an array. */
        if (temp->arrayVariable && GetVariableKnownArraySize(temp->arrayVariable) > 1)
            continue;

        _splitTemp(Optimizer, code, &splitTempList);
    }

    if (splitTempList.count > 0)
    {
        gctINT    oldTempCount  = Optimizer->tempCount;
        gctINT    newTempCount  = oldTempCount + splitTempList.count;

        /* resize temp array */
        gcmONERROR(_resizeOptimizerTempArray(Optimizer, newTempCount));

        /* iterate through the list to rename temp and change its usage */
        splitTemp = splitTempList.head;
        i         = oldTempCount;
        while (splitTemp)
        {
            gcLINKTREE_TEMP  temp     = &Optimizer->tempArray[i];
            SplitTempInfo *  tempInfo = (SplitTempInfo *)splitTemp->data;

            *temp              = *tempInfo->originTemp;  /* copy most of data */
            temp->defined      = tempInfo->defined;
            temp->users        = tempInfo->users;
            temp->dependencies = tempInfo->dependencies;
            temp->lastUse      = tempInfo->lastUse;

            splitTemp  = splitTemp->next;
            i++;
        }
        /* clean up list */
        gcList_Clean(&splitTempList, gcvTRUE);

        /* dump the tree */
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}
#endif

/* Right now, we only check four sequential jump instructions. */
#define __MAX_MERGE_BRANCH_NUMBER__ 4

static gctINT
_GetComponentSwizzleAndCount(
    IN gctINT MergedCount,
    IN gcOPT_CODE *MergedCode,
    IN gcSL_SWIZZLE ComponentSwizzle[][8],
    gctINT *ComponentCount
    )
{
    gctINT i, j;
    gcSL_SWIZZLE swizzle;
    gctINT count[2];
    gctINT componentSum = 0;

    for (i = 0; i < MergedCount; i++)
    {
        swizzle = gcmSL_SOURCE_GET(MergedCode[i]->instruction.source0, Swizzle);
        for (j = 0; j < 4; j++)
        {
            ComponentSwizzle[i][j] = gcmExtractSwizzle(swizzle, j);
        }
        count[0] = gcmEnableChannelCount(gcSL_ConvertSwizzle2Enable(ComponentSwizzle[i][0], ComponentSwizzle[i][1],
                                                          ComponentSwizzle[i][2], ComponentSwizzle[i][3]));

        swizzle = gcmSL_SOURCE_GET(MergedCode[i]->instruction.source1, Swizzle);
        for (j = 4; j < 8; j++)
        {
            ComponentSwizzle[i][j] = gcmExtractSwizzle(swizzle, (j - 4));
        }
        count[1] = gcmEnableChannelCount(gcSL_ConvertSwizzle2Enable(ComponentSwizzle[i][4], ComponentSwizzle[i][5],
                                                          ComponentSwizzle[i][6], ComponentSwizzle[i][7]));
        ComponentCount[i] = count[0] > count[1] ? count[0] : count[1];
        componentSum += ComponentCount[i];
    }
    return componentSum;
}

static gctBOOL
_IsTwoJumpsCanMerge(
    IN gcSL_INSTRUCTION FirstInst,
    IN gcSL_INSTRUCTION SecondInst
    )
{
    if (gcmSL_TARGET_GET(FirstInst->temp, Condition) != gcmSL_TARGET_GET(SecondInst->temp, Condition) ||
        FirstInst->tempIndex != SecondInst->tempIndex)
        return gcvFALSE;

    if (!((gcmSL_SOURCE_GET(FirstInst->source0, Type) == gcmSL_SOURCE_GET(SecondInst->source0, Type) &&
                gcmSL_SOURCE_GET(FirstInst->source0, Format) == gcmSL_SOURCE_GET(SecondInst->source0, Format) &&
                (FirstInst->source0Index == SecondInst->source0Index && FirstInst->source0Indexed == SecondInst->source0Indexed))
            &&
          (gcmSL_SOURCE_GET(FirstInst->source1, Type) == gcmSL_SOURCE_GET(SecondInst->source1, Type) &&
                gcmSL_SOURCE_GET(FirstInst->source1, Format) == gcmSL_SOURCE_GET(SecondInst->source1, Format) &&
                (FirstInst->source1Index == SecondInst->source1Index && FirstInst->source1Indexed == SecondInst->source1Indexed))))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctSOURCE_t
_SetSingleComponentSwizzleByIndex(
    IN gctSOURCE_t Source,
    IN gctINT Index,
    IN gcSL_SWIZZLE Swizzle
    )
{
    switch(Index)
    {
    case 0:
        return gcmSL_SOURCE_SET(Source, SwizzleX, Swizzle);
    case 1:
        return gcmSL_SOURCE_SET(Source, SwizzleY, Swizzle);
    case 2:
        return gcmSL_SOURCE_SET(Source, SwizzleZ, Swizzle);
    case 3:
        return gcmSL_SOURCE_SET(Source, SwizzleW, Swizzle);
    default:
        gcmASSERT(0);
        return gcSL_SWIZZLE_XYZW;
    }
}

/*******************************************************************************
**                          gcOpt_MergeBranchCode
********************************************************************************
**
**  Merge several jump codes to one jump code. e.g:
**     3: JMP.LE          8, temp(1).x, attribute(0).hp.y
**     4: JMP.LE          8, temp(1).y, attribute(0).hp.x
**     5: JMP.LE          8, temp(1).w, attribute(0).hp.w
**     6: ADD             temp(2).x, temp(1).x, 0.100000
**                -------------------->
**     3: JMP.G           6, temp(1).xywx, attribute(0).hp.yxwy
**     4: NOP
**     5: JMP             8
**     6: ADD             temp(2).x, temp(1).x, 0.100000
**     or:
**     3: JMP.LE          7, temp(1).x, attribute(0).hp.y
**     7: JMP              10
**                -------------------->
**     3: JMP.LE          10, temp(1).x, attribute(0).hp.y
**     7: NOP
**
**  INPUT:
**
**      gcOPTIMIZER Optimizer
**          Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_MergeBranchCode(
    IN gcOPTIMIZER Optimizer
    )
{
    gctUINT codeMoved = 0;
    gcOPT_CODE code;

    gcmHEADER_ARG("Optimizer=0x%x", Optimizer);

    for (code = Optimizer->codeHead; code; code = code->next)
    {
        gcSL_CONDITION reversedCond;
        gcOPT_CODE mergedCode[__MAX_MERGE_BRANCH_NUMBER__], nextCode = code, lastCode;
        gctINT i, j, mergedNum = 1, componentSum = 0;
        gctINT componentCount[__MAX_MERGE_BRANCH_NUMBER__];
        gcSL_SWIZZLE componentSwizzle[__MAX_MERGE_BRANCH_NUMBER__][8];

        /* Skip non-jmp code. */
        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_JMP) continue;

        /* Skip non-reversible jmp. */
        if (!isConditionReversible((gcSL_CONDITION)gcmSL_TARGET_GET(code->instruction.temp, Condition),
            &reversedCond))
            continue;

        /* Initialize merged code array. */
        mergedCode[0] = code;
        for (i = 1; i < __MAX_MERGE_BRANCH_NUMBER__; i++)
        {
            nextCode = nextCode->next;
            /*
            ** These jmps code must have same target, condition and sources, only have difference on swizzle.
            ** Tips: gcSL_ALWAYS should be removed by gcOpt_RemoveRedundantCheckings.
            */
            if (gcmSL_OPCODE_GET(nextCode->instruction.opcode, Opcode) != gcSL_JMP ||
                !_IsTwoJumpsCanMerge(&code->instruction, &nextCode->instruction))
            {
                break;
            }
            mergedNum++;
            mergedCode[i] = nextCode;
        }

        if (mergedNum == 1)
            continue;

        /* Now try to merge these branches. */
        lastCode = mergedCode[mergedNum - 1];
        componentSum = _GetComponentSwizzleAndCount(mergedNum, mergedCode, componentSwizzle, componentCount);

        if (componentSum <= 4)
        {
            gctINT componentStart = 0;
            for (i = 0; i < mergedNum; i++)
            {
                for (j = 0; j < componentCount[i]; j++)
                {
                    code->instruction.source0 = _SetSingleComponentSwizzleByIndex(code->instruction.source0,
                                                                componentStart, componentSwizzle[i][j]);
                    code->instruction.source1 = _SetSingleComponentSwizzleByIndex(code->instruction.source1,
                                                                componentStart, componentSwizzle[i][j + 4]);
                    componentStart++;
                }
            }
        }
        else
        {
            continue;
        }

        /* Update data flow. */
        for (i = 0; i < mergedNum - 1; i++)
        {
            gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &mergedCode[i]->callee->callers, mergedCode[i]));

            if (i != 0)
            {
                gcmVERIFY_OK(gcOpt_ChangeCodeToNOP(Optimizer, mergedCode[i]));
            }
            else
            {
                mergedCode[i]->instruction.temp = gcmSL_TARGET_SET(mergedCode[i]->instruction.temp, Condition, reversedCond);
                mergedCode[i]->instruction.tempIndex = (lastCode->id + 1);
                mergedCode[i]->callee = lastCode->next;
                gcmVERIFY_OK(gcOpt_AddCodeToList(Optimizer, &mergedCode[i]->callee->callers, mergedCode[i]));
            }
        }

        lastCode->instruction.temp = gcmSL_TARGET_SET(lastCode->instruction.temp, Condition, gcSL_ALWAYS);
        lastCode->instruction.source0 = lastCode->instruction.source0Index = lastCode->instruction.source0Indexed = 0;
        lastCode->instruction.source1 = lastCode->instruction.source1Index = lastCode->instruction.source1Indexed = 0;
        codeMoved = 1;
    }

    if (codeMoved)
    {
        gcOpt_UpdateCodeId(Optimizer);
    }

    for (code = Optimizer->codeTail; code; code = code->prev)
    {
        gcOPT_CODE targetCode, startCode;

        if (gcmSL_OPCODE_GET(code->instruction.opcode, Opcode) != gcSL_JMP)
            continue;

        if (code->backwardJump)
            continue;

        targetCode = code->callee;

        if ((gcmSL_OPCODE_GET(targetCode->instruction.opcode, Opcode) == gcSL_JMP) &&
            (gcmSL_TARGET_GET(targetCode->instruction.temp, Condition) == gcSL_ALWAYS) &&
            (targetCode->callers->next == gcvNULL))
        {
            gctBOOL existNonNop = gcvFALSE;
            for (startCode = code->next; startCode != targetCode; startCode = startCode->next)
            {
                if (gcmSL_OPCODE_GET(startCode->instruction.opcode, Opcode) != gcSL_NOP)
                {
                    existNonNop = gcvTRUE;
                    break;
                }
            }

            if (existNonNop)
                continue;

            code->instruction.tempIndex = targetCode->instruction.tempIndex;
            gcOpt_DeleteCodeFromList(Optimizer, &code->callee->callers, code);
            code->callee = targetCode->callee;
            gcmVERIFY_OK(gcOpt_AddCodeToList(Optimizer, &code->callee->callers, code));

            gcmVERIFY_OK(gcOpt_DeleteCodeFromList(Optimizer, &targetCode->callee->callers, targetCode));
            gcmVERIFY_OK(gcOpt_ChangeCodeToNOP(Optimizer, targetCode));
            codeMoved = 1;
            continue;
        }
    }

    if (codeMoved != 0)
    {
        gcOpt_UpdateCodeId(Optimizer);
        DUMP_OPTIMIZER("After merge branch code", Optimizer);
        gcmFOOTER_ARG("status=%d", gcvSTATUS_CHANGED);
        return gcvSTATUS_CHANGED;
    }
    else
    {
        /* Code unchanged. */
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
}

static gceSTATUS
_removeUnusedArguments(
    IN OUT gcSHADER Shader
)
{
    gctUINT   funcIdx = 0;
    gceSTATUS status  = gcvSTATUS_OK;

    for (funcIdx = 0; funcIdx < Shader->functionCount; ++funcIdx)
    {
        gctUINT        arg           = 0;
        gctINT         leftArg       = 0;
        gctINT         rightArg      = 0;
        gctINT         argumentCount = 0;
        gcFUNCTION     function      = Shader->functions[funcIdx];

        argumentCount = function->argumentCount;
        for (arg = 0; arg < function->argumentCount; ++arg)
        {
            if (!function->arguments[arg].enable)
            {
                --argumentCount;
            }
        }

        leftArg  = 0;
        rightArg = function->argumentCount - 1;
        while(leftArg < rightArg)
        {
            while ((leftArg < rightArg) &&
                (function->arguments[leftArg].enable))
            {
                ++leftArg;
            }

            while ((leftArg < rightArg) &&
                (!function->arguments[rightArg].enable))
            {
                --rightArg;
            }

            if (leftArg < rightArg)
            {
                function->arguments[leftArg++] = function->arguments[rightArg--];
            }
        }

        function->argumentCount = argumentCount;
    }

    for (funcIdx = 0; funcIdx < Shader->kernelFunctionCount; ++funcIdx)
    {
        gctUINT        arg           = 0;
        gctINT         leftArg       = 0;
        gctINT         rightArg      = 0;
        gctINT         argumentCount = 0;
        gcKERNEL_FUNCTION function   = Shader->kernelFunctions[funcIdx];

        argumentCount = function->argumentCount;
        for (arg = 0; arg < function->argumentCount; ++arg)
        {
            if (!function->arguments[arg].enable)
            {
                --argumentCount;
            }
        }

        leftArg  = 0;
        rightArg = function->argumentCount - 1;
        while(leftArg < rightArg)
        {
            while ((leftArg < rightArg) &&
                (function->arguments[leftArg].enable))
            {
                ++leftArg;
            }

            while ((leftArg < rightArg) &&
                (!function->arguments[rightArg].enable))
            {
                --rightArg;
            }

            if (leftArg < rightArg)
            {
                function->arguments[leftArg++] = function->arguments[rightArg--];
            }
        }

        function->argumentCount = argumentCount;
    }

    return status;
}

/*******************************************************************************
                            _findFuncByStartIndex
********************************************************************************

    Mark used codes in code usage list.

    INPUT:
        gcSHADER Shader
            Pointer to a gcSHADER structure.
        gctUINT funStartIndex
            Function start Index.

    OUTPUT:
        gctBOOL * isKernelFunc
           return true if it finds a kernel function.
        gctUINT * funcIndex
            Function index in function array.

    ALGORITHM:
        First search kernel function array and find the kernel function start with funcStartIndex.
        If it is not found, search function array and find the function start with funcStartIndex.
*********************************************************************************/
gceSTATUS
_findFuncByStartIndex(
    IN gcSHADER Shader,
    IN gctUINT funcStartIndex,
    OUT gctBOOL * isKernelFunc,
    OUT gctUINT * funcIndex
    )
{
    gctSIZE_T i;
    gcFUNCTION * functions = Shader->functions;
    gcKERNEL_FUNCTION * kernelFunctions = Shader->kernelFunctions;

    /* check kernel functions */
    for (i = 0; i < Shader->kernelFunctionCount; i++)
    {
        if (kernelFunctions[i]->codeStart == funcStartIndex)
        {
            *funcIndex = i;
            *isKernelFunc = gcvTRUE;
            return gcvSTATUS_OK;
        }
    }
    /* check normal functions */
    for (i = 0; i < Shader->functionCount; i++)
    {
        if (functions[i]->codeStart == funcStartIndex)
        {
            *funcIndex = i;
            *isKernelFunc = gcvFALSE;
            return gcvSTATUS_OK;
        }
    }

    return gcvSTATUS_TERMINATE;
}

/*******************************************************************************
                            _markUsedFunHelper
********************************************************************************

    Mark used codes in code usage list.

    INPUT:
        gcSHADER Shader
            Pointer to a gcSHADER structure.
        gctUINT funStartIndex
            Function start Index.

    INPUT & OUTPUT:
        gctINT8 * funUsageList
            Pointer to function usage list.

    ALGORITHM:
    Traverse through function list and kernel function list, look for function that
    start index correspond to funStartIndex, mark it as current function or current
    kernel function.
    Traverse through all instructions in current function of kernel function, recurse
    when finding CALL instruction.
*******************************************************************************/
gceSTATUS
_markUsedFuncHelper(
    IN gcSHADER Shader,
    IN gctUINT funcStartIndex,
    INOUT gctINT8 * funcUsageList
    )
{
    gctSIZE_T i;

    gctUINT currentFuncIndex = 0;

    gctUINT funcEndIndex = 0;

    gcFUNCTION * functions = Shader->functions;
    gcKERNEL_FUNCTION * kernelFunctions = Shader->kernelFunctions;

    gctBOOL isKernalFunction = gcvFALSE;

    gcFUNCTION currentFunction = gcvNULL;
    gcKERNEL_FUNCTION currentKernelFunction = gcvNULL;

    gcSL_INSTRUCTION code = Shader->code;

    gcmVERIFY_OK(_findFuncByStartIndex(Shader, funcStartIndex, &isKernalFunction, &currentFuncIndex));
    if (isKernalFunction)
    {
        currentKernelFunction = kernelFunctions[currentFuncIndex];
        isKernalFunction = gcvTRUE;
    }
    else
    {
        currentFunction = functions[currentFuncIndex];
        funcUsageList[currentFuncIndex] = gcvTRUE;
    }

    if (!isKernalFunction)
    {
        funcEndIndex = funcStartIndex + currentFunction->codeCount;
    }
    else
    {
        funcEndIndex = currentKernelFunction->codeEnd;
    }

    for (i = funcStartIndex; i < funcEndIndex; i++)
    {
        gcSL_OPCODE opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code[i].opcode, Opcode);
        if (opcode == gcSL_CALL && ((gctINT)code[i].tempIndex) >= 0 )
        {
            /*Make sure there is no self calling.*/
            gcmASSERT(code[i].tempIndex != funcStartIndex);
            gcmVERIFY_OK(_markUsedFuncHelper(Shader, code[i].tempIndex, funcUsageList));
        }
    }

    return gcvSTATUS_OK;

}

/*******************************************************************************
                            _markUsedFun
********************************************************************************

    Mark used codes in code usage list.

    INPUT:
        gcSHADER Shader
            Pointer to a gcSHADER structure.

    INPUT & OUTPUT:
        gctINT8 * funUsageList
            Pointer to function usage list.

    ALGORITHM:
    Find the beginning of main function, then call _markUsedFunHelper to recursively
    mark all used function.
*******************************************************************************/
gceSTATUS
_markUsedFunc(
    IN gcSHADER Shader,
    INOUT gctINT8 * funcUsageList
    )
{
    gctSIZE_T i;
    gctUINT mainStartIndex = 0;
    gctUINT mainEndIndex = 0;
    gcSL_INSTRUCTION code = Shader->code;

    gcmVERIFY_OK(gcSHADER_FindMainFunction(Shader, (gctINT *)&mainStartIndex, (gctINT *)&mainEndIndex));

    for (i = mainStartIndex; i < mainEndIndex; i++)
    {
        gcSL_OPCODE opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code[i].opcode, Opcode);
        if (opcode == gcSL_CALL && ((gctINT)code[i].tempIndex) >= 0 )
        {
            gcmVERIFY_OK(_markUsedFuncHelper(Shader, code[i].tempIndex, funcUsageList));
        }
    }

    if (Shader->currentKernelFunction)
    {
        for (i = Shader->currentKernelFunction->codeStart; i < Shader->currentKernelFunction->codeEnd; i++)
        {
            gcSL_OPCODE opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code[i].opcode, Opcode);
            if (opcode == gcSL_CALL && ((gctINT)code[i].tempIndex) >= 0 )
            {
                gcmVERIFY_OK(_markUsedFuncHelper(Shader, code[i].tempIndex, funcUsageList));
            }
        }
    }
    return gcvSTATUS_OK;
}



/*******************************************************************************
                            _markUsedCode
********************************************************************************

    Mark used codes in code usage list.

    INPUT:
        gcSHADER Shader
            Pointer to a gcSHADER structure.
        gctINT8 * funUsageList
            Pointer to function usage list.

    INPUT & OUTPUT:
        gctINT8 * codeUsageList
            Pointer to code usage list.

    ALGORITHM:
    Find the begin and end of main function instructions, and mark all of them
    as used.
    For all instructions in kernel function, mark it as used.
    For all functions that are marked as used in funcUsageList, mark its instructions
    as used.
*******************************************************************************/
gceSTATUS
_markUsedCode(
    IN gcSHADER Shader,
    IN gctINT8 * funcUsageList,
    INOUT gctINT8 * codeUsageList
    )
{
    gctSIZE_T i;
    gctSIZE_T j;

    gctUINT mainStartIndex = 0;
    gctUINT mainEndIndex = 0;
    gcFUNCTION * functions = gcvNULL;
    gcKERNEL_FUNCTION * kernelFunctions = gcvNULL;

    if (Shader != gcvNULL)
    {
        functions = Shader->functions;
        kernelFunctions = Shader->kernelFunctions;

    }
    else
    {
        return gcvSTATUS_TERMINATE;
    }


    gcmVERIFY_OK(gcSHADER_FindMainFunction(Shader, (gctINT *)&mainStartIndex, (gctINT *)&mainEndIndex));

    /*mark used code in main function.*/
    for (i = mainStartIndex; i < mainEndIndex; i++)
    {
        codeUsageList[i] = gcvTRUE;
    }

    /*mark used code in kernel function.*/
    for (i = 0; i < Shader->kernelFunctionCount; i++)
    {
        gcKERNEL_FUNCTION kernelFunction = kernelFunctions[i];
        if (kernelFunction != Shader->currentKernelFunction &&
            !kernelFunction->isCalledByEntryKernel)
        {
            /* skip unused kernel functions */
            continue;
        }
        for (j = kernelFunction->codeStart; j < kernelFunction->codeEnd; j++)
        {
            codeUsageList[j] = gcvTRUE;
        }
        if (kernelFunction == Shader->currentKernelFunction && mainStartIndex!= kernelFunction->codeStart)
        {
            /* mark all the parameter passing and CALL code after kernel end */
            j = kernelFunction->codeEnd;
            while (j < Shader->codeCount)
            {
                gcSL_INSTRUCTION code = &Shader->code[j];
                gcSL_OPCODE opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->opcode, Opcode);
                codeUsageList[j] = gcvTRUE;
                if (opcode == gcSL_CALL && (gctINT)code->tempIndex == kernelFunction->codeStart )
                {
                    /* found the call to the kernel function, which should
                     * be the real end of main function */
                    break;
                }
                j++;
            }
        }
    }

    /*mark used code in called functions.*/
    for (i = 0; i < Shader->functionCount; i++)
    {
        gctUINT funcStartIndex = 0;
        gctUINT funcEndIndex = 0;
        if (funcUsageList[i] != gcvTRUE)
        {
            continue;
        }
        funcStartIndex = functions[i]->codeStart;
        funcEndIndex = funcStartIndex + functions[i]->codeCount - 1;
        for (j = funcStartIndex; j <= funcEndIndex; j++)
        {
            codeUsageList[j] = gcvTRUE;
        }
    }

    return gcvSTATUS_OK;
}


typedef struct regMap
{
    gctUINT  isUsed : 2;
    gctUINT packedIndex : 30;
}regMap;

/*******************************************************************************
                            _packRegester
********************************************************************************

    Remove unused register.

    INPUT:
        gcSHADER Shader
            Pointer to a gcSHADER structure.

    ALGORITHM:
    Initialize and allocate memory to local variables that record variable, code,
    and function usage.

    Mark used functions
    Mark used instructions;

    Add used register index in instructions to list.
    Add used register index in variables to list.
    Add used register index in outputs to list.
    Add used register index in function local variables and arguments to list.
    Add used register index in kernel function local variables and arguments to list.

    Generate compacted register index according to index mapping list.

    Assign new packed index to instructions.
    Assign new packed index to variables.
    Assign new packed index to outputs.
    Assign new packed index to function's local variables and arguments.
    Assign new packed index to kernel function's local variables and arguments.

    free pointers.

*******************************************************************************/
gceSTATUS
gcSHADER_PackRegister(
    IN gcSHADER Shader
    )

{
    gctSIZE_T i;
    gctSIZE_T j;
    gctSIZE_T k;

    gctUINT32 regIndex = 0;
    gctUINT regCount = 0;   /*Register count that actually used.*/
    gctUINT maxCodeCount = 0;
    gctUINT maxFuncCount = 0;
    gctUINT maxRegCount = 0;
    gctUINT maxLocalRegCount = 0;
    gctUINT maxVarCount = 0;
    gctUINT maxOutputCount = 0;
    gcSL_INSTRUCTION code;
    gcVARIABLE * variables = gcvNULL;
    gcOUTPUT * outputs = gcvNULL;
    gctINT8 * codeUsageList = gcvNULL;
    gctINT8 * funcUsageList = gcvNULL;
    gctINT8 * varUsageList = gcvNULL;
    struct regMap * regMapList = gcvNULL;

    if (gcSHADER_DumpOptimizerVerbose(Shader))
    {
        gcDump_Shader(gcvNULL, "Before pack resgister shader IR.", gcvNULL, Shader, gcvTRUE);
    }

    if (Shader != gcvNULL)
    {
        code = Shader->code;
        variables = Shader->variables;
        outputs = Shader->outputs;
        maxVarCount = Shader->variableCount;
        maxFuncCount = Shader->functionCount;
        maxCodeCount = Shader->codeCount;
        maxRegCount = Shader->_tempRegCount;
        maxLocalRegCount = GetShaderMaxLocalTempRegCount(Shader);

        /*Do not pack reserved local register if shader type is gcSHADER_TYPE_CL.*/
        regIndex = GetShaderType(Shader) == gcSHADER_TYPE_CL ? maxLocalRegCount : 0;
        maxOutputCount = Shader->outputCount;

        /*Allocate memory to array and set value to zero.*/
        if (maxCodeCount > 0)
        {
            gcoOS_Allocate(gcvNULL, sizeof(gctINT8) * maxCodeCount, (gctPOINTER *)&codeUsageList);
            gcoOS_ZeroMemory((gctPOINTER)codeUsageList, sizeof(gctINT8) * maxCodeCount);
        }

        if (maxFuncCount > 0)
        {
            gcoOS_Allocate(gcvNULL, sizeof(gctINT8) * maxFuncCount, (gctPOINTER *)&funcUsageList);
            gcoOS_ZeroMemory((gctPOINTER)funcUsageList, sizeof(gctINT8) * maxFuncCount);
        }
        if (maxVarCount > 0)
        {
            gcoOS_Allocate(gcvNULL, sizeof(gctINT8) * maxVarCount, (gctPOINTER *)&varUsageList);
            gcoOS_ZeroMemory((gctPOINTER)varUsageList, sizeof(gctINT8) * maxVarCount);
        }
        if (maxRegCount > 0)
        {
            gcoOS_Allocate(gcvNULL, sizeof(struct regMap) * (maxRegCount + 1), (gctPOINTER *)&regMapList);
            gcoOS_ZeroMemory((gctPOINTER)regMapList, sizeof(struct regMap) * (maxRegCount + 1));
        }
    }
    else
    {
        return gcvSTATUS_TERMINATE;
    }


    /*mark used function.*/
    gcmVERIFY_OK(_markUsedFunc(Shader, funcUsageList));


    /*mark used code.*/
    gcmVERIFY_OK(_markUsedCode(Shader, funcUsageList, codeUsageList));

    /***********************************************************************************
        Add used register index in codes to regMapList.
        First check if opcode have target, if so, add tempIndex and tempIndexed to list.
        Notice that we should use two registers if type is long.
        For the first and second source, if source type is gcSL_TEMP, Add both index and
        indexed register to list, else if type is gcSL_SAMPLER, gcSL_UNIFORM, or
        gcSL_ATTRIBUTE and this source is gcSL_NOT_INDEXED, then only add indexed register.
    ***********************************************************************************/
    for (i = 0; i < maxCodeCount; i++)
    {
        if (codeUsageList[i] != gcvTRUE)
        {
            /* change unused code to nop */
            Shader->code[i] = gcvSL_NOP_INSTR;
            continue;
        }
        if (!gcSL_isOpcodeHaveNoTarget(code[i].opcode) || gcSL_isOpcodeTexldModifier(code[i].opcode))
        {
            gcSL_FORMAT targetType = (gcSL_FORMAT) gcmSL_TARGET_GET(code[i].temp, Format);

            if (regMapList[code[i].tempIndex].isUsed == gcvFALSE)
            {
                regMapList[code[i].tempIndex].isUsed = gcvTRUE;
                regCount++;
            }
            if (regMapList[code[i].tempIndexed].isUsed == gcvFALSE)
            {
                regMapList[code[i].tempIndexed].isUsed = gcvTRUE;
                regCount++;
            }
            /*Mark two registers if type is long*/
            if (targetType == gcSL_INT64 || targetType == gcSL_UINT64)
            {
                if (regMapList[code[i].tempIndex + 1].isUsed == gcvFALSE)
                {
                    regMapList[code[i].tempIndex + 1].isUsed = gcvTRUE;
                    regCount++;
                }
            }
        }
        if (gcmSL_SOURCE_GET(code[i].source0, Type) == gcSL_TEMP)
        {
            gcSL_FORMAT sourceType = (gcSL_FORMAT) gcmSL_SOURCE_GET(code[i].source0, Format);

            if (regMapList[code[i].source0Index].isUsed == gcvFALSE)
            {
                regMapList[code[i].source0Index].isUsed = gcvTRUE;
                regCount++;
            }
            if (regMapList[code[i].source0Indexed].isUsed == gcvFALSE
                && (gcSL_INDEXED) gcmSL_SOURCE_GET(code[i].source0, Indexed) != gcSL_NOT_INDEXED)
            {
                regMapList[code[i].source0Indexed].isUsed = gcvTRUE;
                regCount++;
            }
            /*Mark two registers if type is long*/
            if (sourceType == gcSL_INT64 || sourceType == gcSL_UINT64)
            {
                if (regMapList[code[i].source0Index + 1].isUsed == gcvFALSE)
                {
                    regMapList[code[i].source0Index + 1].isUsed = gcvTRUE;
                    regCount++;
                }
            }
        }
        else if (gcmSL_SOURCE_GET(code[i].source0, Type) == gcSL_SAMPLER
            ||gcmSL_SOURCE_GET(code[i].source0, Type) == gcSL_UNIFORM
            ||gcmSL_SOURCE_GET(code[i].source0, Type) == gcSL_ATTRIBUTE
            ||gcmSL_SOURCE_GET(code[i].source0, Type) == gcSL_OUTPUT)
        {
            if (regMapList[code[i].source0Indexed].isUsed == gcvFALSE
                && (gcSL_INDEXED) gcmSL_SOURCE_GET(code[i].source0, Indexed) != gcSL_NOT_INDEXED)
            {
                regMapList[code[i].source0Indexed].isUsed = gcvTRUE;
                regCount++;
            }
        }
        if (gcmSL_SOURCE_GET(code[i].source1, Type) == gcSL_TEMP)
        {
            gcSL_FORMAT sourceType = (gcSL_FORMAT) gcmSL_SOURCE_GET(code[i].source1, Format);

            if (regMapList[code[i].source1Index].isUsed == gcvFALSE)
            {
                regMapList[code[i].source1Index].isUsed = gcvTRUE;
                regCount++;
            }
            if (regMapList[code[i].source1Indexed].isUsed == gcvFALSE
                && (gcSL_INDEXED) gcmSL_SOURCE_GET(code[i].source1, Indexed) != gcSL_NOT_INDEXED)
            {
                regMapList[code[i].source1Indexed].isUsed = gcvTRUE;
                regCount++;
            }
            /*Mark two registers if type is long*/
            if (sourceType == gcSL_INT64 || sourceType == gcSL_UINT64)
            {
                if (regMapList[code[i].source1Index + 1].isUsed == gcvFALSE)
                {
                    regMapList[code[i].source1Index + 1].isUsed = gcvTRUE;
                    regCount++;
                }
            }
        }
        else if (gcmSL_SOURCE_GET(code[i].source1, Type) == gcSL_SAMPLER
            ||gcmSL_SOURCE_GET(code[i].source1, Type) == gcSL_UNIFORM
            ||gcmSL_SOURCE_GET(code[i].source1, Type) == gcSL_ATTRIBUTE
            ||gcmSL_SOURCE_GET(code[i].source1, Type) == gcSL_OUTPUT)
        {
            if (regMapList[code[i].source1Indexed].isUsed == gcvFALSE
                && (gcSL_INDEXED) gcmSL_SOURCE_GET(code[i].source1, Indexed) != gcSL_NOT_INDEXED)
            {
                regMapList[code[i].source1Indexed].isUsed = gcvTRUE;
                regCount++;
            }
        }

    }

    /*********************************************************************************************
        Add used register index in variables to list.
        For each variables, search from start index to end index, if any one register is marked as
        used in regMapList, mark this variable as used.
        For each used variable, add all its registers to regMapList.
    *********************************************************************************************/
    for (i = 0; i < maxVarCount; i++)
    {
        gcVARIABLE variable = variables[i];
        gctUINT arraySize = GetVariableKnownArraySize(variable);
        gctUINT32 startIndex = variable->tempIndex;
        gctUINT endIndex = startIndex + arraySize * gcmType_Rows(variable->u.type);

        for (j = startIndex; j <= endIndex && j < maxRegCount; j++)
        {
            if (regMapList[j].isUsed == gcvTRUE)
            {
                varUsageList[i] = gcvTRUE;
                break;
            }
        }
        if (varUsageList[i])
        {
            for (j = startIndex; j <= endIndex && j < maxRegCount; j++)
            {
                if (regMapList[j].isUsed == gcvFALSE)
                {
                    regMapList[j].isUsed = gcvTRUE;
                    regCount++;
                }
            }
        }
    }

    /********************************************************************************************
        Add used register index in outputs to list.
    *********************************************************************************************/
    for (i = 0; i < maxOutputCount; i++)
    {
        gcOUTPUT output = outputs[i];
        if (output != gcvNULL && regMapList[output->tempIndex].isUsed == gcvFALSE)
        {
            regMapList[output->tempIndex].isUsed = gcvTRUE;
            regCount++;
        }
    }

    /********************************************************************************************
        Add used register index in function local variables and arguments to list.
    *********************************************************************************************/
    for (i = 0; i < Shader->functionCount; i++)
    {
        gcFUNCTION function = gcvNULL;
        gctUINT32 argumentRegStartIndex = 0;
        gctUINT32 argumentRegEndIndex = 0;
        if (funcUsageList[i] != gcvTRUE)
        {
            continue;
        }
        function = Shader->functions[i];
        /*Mark function's start index as used*/
        if (!regMapList[function->tempIndexStart].isUsed && function->tempIndexCount != 0)
        {
            regMapList[function->tempIndexStart].isUsed = gcvTRUE;
            regCount++;
        }
        for (j = 0; j < function->localVariableCount; j++)
        {
            gcVARIABLE variable = function->localVariables[j];
            gctUINT32 startIndex = variable->tempIndex;
            gctUINT arraySize = GetVariableKnownArraySize(variable);
            gctUINT endIndex = startIndex + arraySize * gcmType_Rows(variable->u.type);

            for (k = startIndex; k <= endIndex && k < maxRegCount; k++)
            {
                if (regMapList[k].isUsed == gcvFALSE)
                {
                    regMapList[k].isUsed = gcvTRUE;
                    regCount++;
                }
            }
        }
        if (function->argumentCount != 0)
        {
            argumentRegStartIndex = function->arguments[0].index;
            /*Mark additional one more register in case the last argument is 64bit long.
              Since we do not have method to determine whether one argument is 64 bit. This
              might result to marking one variable that is actually not used. Though this will
              not result to bugs.*/
            argumentRegEndIndex = function->arguments[function->argumentCount - 1].index + 1;
            for (j = argumentRegStartIndex; j <= argumentRegEndIndex; j++)
            {
                if (regMapList[j].isUsed == gcvFALSE)
                {
                    regMapList[j].isUsed = gcvTRUE;
                    regCount++;
                }
            }
        }
    }

    /********************************************************************************************
        Add used register index in kernel function local variables and arguments to list.
    *********************************************************************************************/
    for (i = 0; i < Shader->kernelFunctionCount; i++)
    {
        gcKERNEL_FUNCTION function = Shader->kernelFunctions[i];
        gctUINT32 argumentRegStartIndex = 0;
        gctUINT32 argumentRegEndIndex = 0;

        /* skip unused kernel functions */
        if (function == gcvNULL ||
            (function != Shader->currentKernelFunction && !function->isCalledByEntryKernel))
        {
            continue;
        }

        /*Mark function's start index as used*/
        if (!regMapList[function->tempIndexStart].isUsed && function->tempIndexCount != 0)
        {
            regMapList[function->tempIndexStart].isUsed = gcvTRUE;
            regCount++;
        }
        for (j = 0; j < function->localVariableCount; j++)
        {
            gcVARIABLE variable = function->localVariables[j];
            gctUINT32 startIndex = variable->tempIndex;
            gctUINT arraySize = GetVariableKnownArraySize(variable);
            gctUINT endIndex = startIndex + arraySize * gcmType_Rows(variable->u.type);

            for (k = startIndex; k <= endIndex && k < maxRegCount; k++)
            {
                if (regMapList[k].isUsed == gcvFALSE)
                {
                    regMapList[k].isUsed = gcvTRUE;
                    regCount++;
                }
            }
        }
        if (function->argumentCount != 0)
        {
            argumentRegStartIndex = function->arguments[0].index;
            /*Mark additional one more register in case the last argument is 64bit long.
              Since we do not have method to determine whether one argument is 64 bit. This
              might result to marking one variable that is actually not used. Though this will
              not result to bugs.*/
            argumentRegEndIndex = function->arguments[function->argumentCount - 1].index + 1;
            for (j = argumentRegStartIndex; j <= argumentRegEndIndex; j++)
            {
                if (regMapList[j].isUsed == gcvFALSE)
                {
                    regMapList[j].isUsed = gcvTRUE;
                    regCount++;
                }
            }
        }
    }

    /************************************************************************************************
        Assign packed index to register list.
        For registers which index smaller than maxLocalRegCount, do not change its index.
        For all registers after, pack the index: Used two pointers, each time move the first to next
        used index, and move the second one step ahead. Assign the value of second pointer to first.
     ***********************************************************************************************/
    if (GetShaderType(Shader) == gcSHADER_TYPE_CL)
    {
        for(i = 0; i < maxLocalRegCount; i++)
        {
            while(regMapList[i].isUsed != gcvTRUE && (i + 1) < maxRegCount)
            {
                i++;
            }
            if (i < maxLocalRegCount && regMapList[i].isUsed)
            {
                regMapList[i].packedIndex = i;
            }
        }
        for(i = maxLocalRegCount; i < maxRegCount; i++)
        {
            while(regMapList[i].isUsed != gcvTRUE && (i + 1) < maxRegCount)
            {
                i++;
            }
            if (regMapList[i].isUsed)
            {
                regMapList[i].packedIndex = regIndex;
                regIndex++;
            }
        }
    }
    else
    {
        for(i = 0; i < maxRegCount; i++)
        {
            while(regMapList[i].isUsed != gcvTRUE && (i + 1) < maxRegCount)
            {
                i++;
            }
            if (regMapList[i].isUsed)
            {
                regMapList[i].packedIndex = regIndex;
                regIndex++;
            }
        }
    }


    /********************************************************************************************
        Assign new packed index to instructions.
    *********************************************************************************************/
    for(i = 0; i < maxCodeCount; i++)
    {
        if (codeUsageList[i] != gcvTRUE)
        {
            continue;
        }
        if (!gcSL_isOpcodeHaveNoTarget(code[i].opcode) || gcSL_isOpcodeTexldModifier(code[i].opcode))
        {
            code[i].tempIndex = gcmSL_INDEX_SET(code[i].tempIndex, Index, regMapList[code[i].tempIndex].packedIndex);
            code[i].tempIndexed = (gctUINT16)gcmSL_INDEX_SET(code[i].tempIndexed, Index, regMapList[code[i].tempIndexed].packedIndex);
        }

        if (gcmSL_SOURCE_GET(code[i].source0, Type) == gcSL_TEMP)
        {
            code[i].source0Index = gcmSL_INDEX_SET(code[i].source0Index, Index, regMapList[code[i].source0Index].packedIndex);
            if ((gcSL_INDEXED) gcmSL_SOURCE_GET(code[i].source0, Indexed) != gcSL_NOT_INDEXED)
            {
                code[i].source0Indexed = (gctUINT16)gcmSL_INDEX_SET(code[i].source0Indexed, Index, regMapList[code[i].source0Indexed].packedIndex);
            }
        }
        else if (gcmSL_SOURCE_GET(code[i].source0, Type) == gcSL_SAMPLER
            ||gcmSL_SOURCE_GET(code[i].source0, Type) == gcSL_UNIFORM
            ||gcmSL_SOURCE_GET(code[i].source0, Type) == gcSL_ATTRIBUTE
            ||gcmSL_SOURCE_GET(code[i].source0, Type) == gcSL_OUTPUT)
        {
            if ((gcSL_INDEXED) gcmSL_SOURCE_GET(code[i].source0, Indexed) != gcSL_NOT_INDEXED)
            {
                code[i].source0Indexed = (gctUINT16)gcmSL_INDEX_SET(code[i].source0Indexed, Index, regMapList[code[i].source0Indexed].packedIndex);
            }
        }

        if (gcmSL_SOURCE_GET(code[i].source1, Type) == gcSL_TEMP)
        {
            code[i].source1Index = gcmSL_INDEX_SET(code[i].source1Index, Index, regMapList[code[i].source1Index].packedIndex);
            if ((gcSL_INDEXED) gcmSL_SOURCE_GET(code[i].source1, Indexed) != gcSL_NOT_INDEXED)
            {
                code[i].source1Indexed = (gctUINT16)gcmSL_INDEX_SET(code[i].source1Indexed, Index, regMapList[code[i].source1Indexed].packedIndex);
            }
        }
        else if (gcmSL_SOURCE_GET(code[i].source1, Type) == gcSL_SAMPLER
            ||gcmSL_SOURCE_GET(code[i].source1, Type) == gcSL_UNIFORM
            ||gcmSL_SOURCE_GET(code[i].source1, Type) == gcSL_ATTRIBUTE
            ||gcmSL_SOURCE_GET(code[i].source1, Type) == gcSL_OUTPUT)
        {
            if ((gcSL_INDEXED) gcmSL_SOURCE_GET(code[i].source1, Indexed) != gcSL_NOT_INDEXED)
            {
                code[i].source1Indexed = (gctUINT16)gcmSL_INDEX_SET(code[i].source1Indexed, Index, regMapList[code[i].source1Indexed].packedIndex);
            }
        }

    }

    /********************************************************************************************
        Renew function's temp register start and end index.
    *********************************************************************************************/
    for (i = 0; i < Shader->functionCount; i++)
    {
        gctUINT32 originalFuncStartIndex = 0;
        gctUINT32 originalFuncEndIndex = 0;
        gctUINT32 originalFuncRegCount = 0;
        gctUINT32 packedFuncStartIndex = 0;
        gctUINT32 packedFuncEndIndex = 0;
        gctUINT32 packedFuncRegCount = 0;
        gctUINT32 currentStartIndex = 0;
        gctUINT32 currentEndIndex = 0;
        if (funcUsageList[i] == gcvTRUE)
        {
            originalFuncStartIndex = Shader->functions[i]->tempIndexStart;
            originalFuncEndIndex = Shader->functions[i]->tempIndexEnd;
            originalFuncRegCount = Shader->functions[i]->tempIndexCount;
            gcmASSERT(originalFuncStartIndex <= maxRegCount && originalFuncEndIndex <= maxRegCount);
            if (originalFuncRegCount == 0)
            {
                currentStartIndex = originalFuncStartIndex;
                /*Move currentIndex to the last used reg index or 0, which means there
                 is no register used before this function.*/
                while (currentStartIndex > 0 && !regMapList[currentEndIndex].isUsed)
                {
                    currentEndIndex--;
                }
                /*Set function start index to the first available register or 0.*/
                packedFuncStartIndex = regMapList[currentStartIndex].isUsed?
                    regMapList[currentStartIndex].packedIndex + 1: regMapList[currentStartIndex].packedIndex;
            }
            else
            {
                currentStartIndex = originalFuncStartIndex;
                currentEndIndex = originalFuncEndIndex;
                /*Move currentEndIndex to the last used reg index after currentStartIndex or currentStartIndex itself,
                 which means there is no register used other than currentStartIndex.*/
                while (currentStartIndex < currentEndIndex && !regMapList[currentEndIndex].isUsed)
                {
                    currentEndIndex--;
                }

                packedFuncStartIndex = regMapList[currentStartIndex].packedIndex;
                packedFuncEndIndex = regMapList[currentEndIndex].packedIndex;
                packedFuncRegCount = packedFuncEndIndex - packedFuncStartIndex + 1;
            }
            Shader->functions[i]->tempIndexStart = packedFuncStartIndex;
            Shader->functions[i]->tempIndexEnd = packedFuncEndIndex;
            Shader->functions[i]->tempIndexCount = packedFuncRegCount;
            gcmASSERT(maxRegCount != (gctUINT32)-1);
        }
        else
        {
            /* mark the function not used */
            SetFunctionNotUsed(Shader->functions[i]);
        }
    }

    /********************************************************************************************
        Renew kernel function's temp register start and end index.
    *********************************************************************************************/
    for (i = 0; i < Shader->kernelFunctionCount; i++)
    {
        gctUINT32 originalFuncStartIndex = 0;
        gctUINT32 originalFuncEndIndex = 0;
        gctUINT32 originalFuncRegCount = 0;
        gctUINT32 packedFuncStartIndex = 0;
        gctUINT32 packedFuncEndIndex = 0;
        gctUINT32 packedFuncRegCount = 0;
        gctUINT32 currentStartIndex = 0;
        gctUINT32 currentEndIndex = 0;
        gcKERNEL_FUNCTION function = Shader->kernelFunctions[i];

        /* skip unused kernel functions */
        if (function == gcvNULL ||
            (function != Shader->currentKernelFunction && !function->isCalledByEntryKernel))
        {
            /* mark the kernel function not used */
            SetFunctionNotUsed(Shader->kernelFunctions[i]);
            continue;
        }
        originalFuncStartIndex = Shader->kernelFunctions[i]->tempIndexStart;
        originalFuncEndIndex = Shader->kernelFunctions[i]->tempIndexEnd;
        originalFuncRegCount = Shader->kernelFunctions[i]->tempIndexCount;
        gcmASSERT(originalFuncStartIndex <= maxRegCount && originalFuncEndIndex <= maxRegCount);
        if (originalFuncRegCount == 0)
        {
            currentStartIndex = originalFuncStartIndex;
            /*Move currentIndex to the last used reg index or 0, which means there
                 is no register used before this function.*/
                while (currentStartIndex > 0 && !regMapList[currentEndIndex].isUsed)
                {
                    currentEndIndex--;
                }
                /*Set function start index to the first available register or 0.*/
                packedFuncStartIndex = regMapList[currentStartIndex].isUsed?
                    regMapList[currentStartIndex].packedIndex + 1: regMapList[currentStartIndex].packedIndex;
        }
        else
        {
            currentStartIndex = originalFuncStartIndex;
            currentEndIndex = originalFuncEndIndex;
            /*Move currentEndIndex to the last used reg index after currentStartIndex or currentStartIndex itself,
              which means there is no register used other than currentStartIndex.*/
            while (currentStartIndex <= currentEndIndex && !regMapList[currentEndIndex].isUsed)
            {
                currentEndIndex--;
            }

            packedFuncStartIndex = regMapList[currentStartIndex].packedIndex;
            packedFuncEndIndex = regMapList[currentEndIndex].packedIndex;
            packedFuncRegCount = packedFuncEndIndex - packedFuncStartIndex + 1;
        }
        Shader->kernelFunctions[i]->tempIndexStart = packedFuncStartIndex;
        Shader->kernelFunctions[i]->tempIndexEnd = packedFuncEndIndex;
        Shader->kernelFunctions[i]->tempIndexCount = packedFuncRegCount;
        gcmASSERT(maxRegCount != (gctUINT32)-1);
    }


    /********************************************************************************************
        Assign new packed index to variables.
    *********************************************************************************************/
    for (i = 0; i < maxVarCount; i++)
    {
        if (varUsageList[i])
        {
            variables[i]->tempIndex = regMapList[variables[i]->tempIndex].packedIndex;
        }
        else
        {
            SetVariableIsNotUsed(variables[i]);
            /*Set unused variable index to one more than last used one.*/
            variables[i]->tempIndex = regIndex;
        }
    }

    /********************************************************************************************
        Assign new packed index to outputs.
    *********************************************************************************************/
    for (i = 0; i < maxOutputCount; i++)
    {
        if (outputs[i] != gcvNULL)
        {
            outputs[i]->tempIndex = regMapList[outputs[i]->tempIndex].packedIndex;
        }
    }

    /********************************************************************************************
        Assign new packed index to function's local variables and arguments.
    *********************************************************************************************/
    for (i = 0; i < Shader->functionCount; i++)
    {
        gcFUNCTION function = gcvNULL;
        if (funcUsageList[i] != gcvTRUE)
        {
            continue;
        }
        function = Shader->functions[i];
        for (j = 0; j < function->localVariableCount; j++)
        {
            gcVARIABLE variable = function->localVariables[j];
            variable->tempIndex = regMapList[variable->tempIndex].packedIndex;
        }
        for (j = 0; j < function->argumentCount; j++)
        {
            gcsFUNCTION_ARGUMENT_PTR argument = &function->arguments[j];
            argument->index = regMapList[argument->index].packedIndex;
        }

    }

    /********************************************************************************************
        Assign new packed index to kernel function's local variables and arguments.
    *********************************************************************************************/
    for (i = 0; i < Shader->kernelFunctionCount; i++)
    {
        gcKERNEL_FUNCTION function = Shader->kernelFunctions[i];
        /* skip unused kernel functions */
        if (function == gcvNULL ||
            (function != Shader->currentKernelFunction && !function->isCalledByEntryKernel))
        {
            continue;
        }
        for (j = 0; j < function->localVariableCount; j++)
        {
            gcVARIABLE variable = function->localVariables[j];
            variable->tempIndex = regMapList[variable->tempIndex].packedIndex;
        }
        for (j = 0; j < function->argumentCount; j++)
        {
            gcsFUNCTION_ARGUMENT_PTR argument = &function->arguments[j];
            argument->index = regMapList[argument->index].packedIndex;
        }
    }

    /* udpate the temp reg id of sw locations in debug info */
    if (Shader->debugInfo)
    {
        VSC_DIContext * context = (VSC_DIContext *)(Shader->debugInfo);
        gctUINT     i;
        VSC_DI_SW_LOC * sl = gcvNULL;
        for (i = 0 ; i < context->swLocTable.usedCount; i ++)
        {
            sl = &context->swLocTable.loc[i];
            if (sl->reg && sl->u.reg.type == VSC_DIE_REG_TYPE_TMP)
            {
                sl->u.reg.start = (gctUINT16) regMapList[sl->u.reg.start].packedIndex;
                sl->u.reg.end   = (gctUINT16) regMapList[sl->u.reg.end].packedIndex;
            }
        }
    }

    /*Increase tempRegCount by 2 to hold unused variable.*/
    Shader->_tempRegCount =  regIndex + 2;

    /********************************************************************************************
        Free pointers
    *********************************************************************************************/
    if (regMapList != gcvNULL)
    {
        gcoOS_Free(gcvNULL, (gctPOINTER)regMapList);
    }
    if (funcUsageList != gcvNULL)
    {
        gcoOS_Free(gcvNULL, (gctPOINTER)funcUsageList);
    }
    if (varUsageList != gcvNULL)
    {
        gcoOS_Free(gcvNULL, (gctPOINTER)varUsageList);
    }
    if (codeUsageList != gcvNULL)
    {
        gcoOS_Free(gcvNULL, (gctPOINTER)codeUsageList);
    }

    if (gcSHADER_DumpOptimizerVerbose(Shader))
    {
        gcDump_Shader(gcvNULL, "After pack resgister shader IR.", gcvNULL, Shader, gcvTRUE);
    }
    return gcvSTATUS_OK;

}

/*******************************************************************************
**                              gcOptimizeShader
********************************************************************************
**
**  Optimize a shader.
**
**  INPUT:
**
**      gcSHADER Shader
**          Pointer to a gcSHADER object holding information about the compiled
**          shader.
**
**      gctFILE LogFile
**          Pointer to an open FILE object.
*/
gceSTATUS
gcOptimizeShader(
    IN gcSHADER Shader,
    IN gctFILE LogFile
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT option;
    gcOPTIMIZER optimizer = gcvNULL;
    gctBOOL   dumpOptimizer = gcSHADER_DumpOptimizerVerbose(Shader);
    gctBOOL   useFullNewLinker = gcvFALSE;

    gcmHEADER_ARG("Shader=0x%x LogFile=0x%x", Shader, LogFile);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Shader, gcvOBJ_SHADER);

    /* Sanity check. */
    if (Shader->codeCount == 0)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    if (Shader->optimizationOption == gcvOPTIMIZATION_NONE)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    if (dumpOptimizer) {
       gcOpt_DumpMessage(gcvNULL, LogFile, "gcOptimizeShader: start\n");
    }

    do
    {
        /* Construct the optimizer. */
        gcmERR_BREAK(gcOpt_ConstructOptimizer(Shader, &optimizer));
        useFullNewLinker = gcUseFullNewLinker(optimizer->hwCfg.hwFeatureFlags.hasHalti2);

        if (useFullNewLinker)
        {
            optimizer->option &= ~gcvOPTIMIZATION_VECTOR_INSTRUCTION_MERGE;
            optimizer->option &= ~gcvOPTIMIZATION_MAD_INSTRUCTION;
            optimizer->option &= ~gcvOPTIMIZATION_LOAD_SW_W;
            optimizer->option &= ~gcvOPTIMIZATION_CONDITIONALIZE;
            optimizer->option &= ~gcvOPTIMIZATION_POWER_OPTIMIZATION;
            optimizer->option &= ~gcvOPTIMIZATION_CONSTANT_ARGUMENT_PROPAGATION;
        }
        if (gcdHasOptimization(optimizer->option, gcvOPTIMIZATION_MIN_COMP_TIME))
        {
            optimizer->option &= ~gcvOPTIMIZATION_CONSTANT_PROPAGATION;
            optimizer->option &= ~gcvOPTIMIZATION_LOADTIME_CONSTANT;
            optimizer->option &= ~gcvOPTIMIZATION_VECTOR_INSTRUCTION_MERGE;
            optimizer->option &= ~gcvOPTIMIZATION_MAD_INSTRUCTION;
            optimizer->option &= ~gcvOPTIMIZATION_LOAD_SW_W;
            optimizer->option &= ~gcvOPTIMIZATION_CONDITIONALIZE;
            optimizer->option &= ~gcvOPTIMIZATION_POWER_OPTIMIZATION;
            optimizer->option &= ~gcvOPTIMIZATION_CONSTANT_ARGUMENT_PROPAGATION;
        }
        if (gcdHasOptimization(optimizer->option, gcvOPTIMIZATION_INLINE_LEVEL_0))
        {
            optimizer->option &= ~gcvOPTIMIZATION_INLINE_EXPANSION;
        }
        option = optimizer->option;

        gcmERR_BREAK(gcOpt_RemoveDeadCode(optimizer));

#if _REMAP_TEMP_INDEX_FOR_FUNCTION_
        gcmERR_BREAK(gcOpt_RemapTempIndex(&optimizer));
#endif

        optimizer->logFile = LogFile;

        DUMP_OPTIMIZER("After preprocessing", optimizer);

        /* Analysis the code. */
        gcmERR_BREAK(gcOpt_AnalysisCode(optimizer));

        if (option != gcvOPTIMIZATION_NONE &&
            !gcdHasOptimization(option, gcvOPTIMIZATION_UNIT_TEST))
        {
            /* The normal full optimization mode. */
            status = gcvSTATUS_OK;

            if (optimizer->functionCount > 0)
            {
                gctBOOL changed = gcvFALSE;

                if (gcdHasOptimization(option, gcvOPTIMIZATION_DEAD_CODE))
                {
                    /* Remove dead code. */
                    gcmERR_BREAK(gcOpt_RemoveDeadCode(optimizer));
                    if (status == gcvSTATUS_CHANGED) changed = gcvTRUE;
                }

                if (status == gcvSTATUS_CHANGED)
                {
                    if (gcdHasOptimization(option, gcvOPTIMIZATION_VECTOR_INSTRUCTION_MERGE))
                    {
                        /* Merge separate vector instructions into one vector instruction. */
                        gcmERR_BREAK(gcOpt_MergeVectorInstructions(optimizer));
                    }
                }

                if (gcdHasOptimization(option, gcvOPTIMIZATION_CONSTANT_PROPAGATION))
                {
                    /* Propagate constants. */
                    gcmERR_BREAK(gcOpt_PropagateConstants(optimizer));
                }

                if (gcdHasOptimization(option, gcvOPTIMIZATION_CONSTANT_ARGUMENT_PROPAGATION))
                {
                    /* Propagate argument constants. */
                    gcmERR_BREAK(gcOpt_PropagateArgumentConstants(optimizer));
                }

                /* When VIRCG is enabled, we will disable gcsl inliner unless
                   the inlineKind is GCSL_INLINER_KIND, which is set through
                   VC_OPTION=-INLINER:0 */
                if (gcdHasOptimization(option, gcvOPTIMIZATION_INLINE_EXPANSION) &&
                    (gcGetVIRCGKind(optimizer->hwCfg.hwFeatureFlags.hasHalti2) == VIRCG_None || gcmOPT_INLINERKIND() == GCSL_INLINER_KIND) &&
                    !gcdHasOptimization(option, gcvOPTIMIZATION_MIN_COMP_TIME))
                {
                    /* Remove nops before inline function optimization, so there can be more code budget for expand functions. */
                    gcmERR_BREAK(gcOpt_RemoveNOP(optimizer));

                    /* Expand functions used only once or short but with many
                    ** arguments. */
                    gcmERR_BREAK(gcOpt_InlineFunctions(&optimizer, gcvFALSE));

                    gcOpt_UpdateCodeFunction(optimizer);
                }

                if (gcdHasOptimization(option, gcvOPTIMIZATION_UPDATE_PRECISION))
                {
                    gcOpt_UpdatePrecision(optimizer);
                }

                if (gcdHasOptimization(option, gcvOPTIMIZATION_INLINE_EXPANSION) &&
                    (gcGetVIRCGKind(optimizer->hwCfg.hwFeatureFlags.hasHalti2) == VIRCG_None || gcmOPT_INLINERKIND() == GCSL_INLINER_KIND))
                {
                    if (status == gcvSTATUS_CHANGED)
                    {
                        /* Remove dead code. */
                        gcmERR_BREAK(gcOpt_RemoveDeadCode(optimizer));

                        /* Optimize MOV instructions for arguments and outputs. */
                        gcOpt_OptimizeMOVInstructions(optimizer);
                    }
                }

                if (gcdHasOptimization(option, gcvOPTIMIZATION_REDUNDANT_MOVE))
                {
                    /* Optimize MOV instructions for arguments and outputs. */
                    gcmERR_BREAK(gcOpt_OptimizeMOVInstructions(optimizer));
                    if (status == gcvSTATUS_CHANGED) changed = gcvTRUE;
                }
                if (gcdHasOptimization(option, gcvOPTIMIZATION_CONSTANT_PROPAGATION))
                {
                    /* Propagate constants. */
                    gcmERR_BREAK(gcOpt_PropagateConstants(optimizer));

                    if (status == gcvSTATUS_CHANGED)
                    {
                        changed = gcvTRUE;

                        /* Remove dead code. */
                        gcmERR_BREAK(gcOpt_RemoveDeadCode(optimizer));

                        if (gcdHasOptimization(option, gcvOPTIMIZATION_REDUNDANT_CHECKING))
                        {
                            /* Remove redundant checking. */
                            gcmERR_BREAK(gcOpt_RemoveRedundantCheckings(optimizer));
                        }
                    }
                }

                if (gcdHasOptimization(option, gcvOPTIMIZATION_CONSTANT_ARGUMENT_PROPAGATION))
                {
                    /* Propagate argument constants. */
                    gcmERR_BREAK(gcOpt_PropagateArgumentConstants(optimizer));
                }

                if (gcdHasOptimization(option, gcvOPTIMIZATION_DEAD_CODE))
                {
                    /* Remove dead code. */
                    gcmERR_BREAK(gcOpt_RemoveDeadCode(optimizer));
                    if (status == gcvSTATUS_CHANGED) changed = gcvTRUE;
                }

                if (changed)
                {
                    /* Get accurate function codeCount after optimizaton. */
                    gcmERR_BREAK(gcOpt_RemoveNOP(optimizer));
                }
            }
            else if (gcdHasOptimization(option, gcvOPTIMIZATION_UPDATE_PRECISION))
            {
                gcOpt_UpdatePrecision(optimizer);
            }

            if (gcdHasOptimization(option, gcvOPTIMIZATION_LOOP_REROLLING) &&
                (optimizer->codeTail->id + 1) > REROLL_CODE_COUNT &&
                !gcdHasOptimization(option, gcvOPTIMIZATION_MIN_COMP_TIME))
            {
                /* Optimize MOV instructions. */
                gcmERR_BREAK(gcOpt_OptimizeMOVInstructions(optimizer));

                /* Remove NOP instructions from the shader. */
                gcmERR_BREAK(gcOpt_RemoveNOP(optimizer));

                gcmERR_BREAK(gcOpt_OptimizeLoopRerolling(optimizer));
                if (status == gcvSTATUS_CHANGED)
                {
                    /* reconstruct the optimizer. */
                    gcmERR_BREAK(gcOpt_ReconstructOptimizer(Shader, &optimizer));
                    /* Remove NOP instructions from the shader. */
                    gcmERR_BREAK(gcOpt_RemoveNOP(optimizer));
                }
            }

            do
            {

                if (gcdHasOptimization(option, gcvOPTIMIZATION_DEAD_CODE))
                {
                    /* Remove dead code. */
                    gcmERR_BREAK(gcOpt_RemoveDeadCode(optimizer));
                }

                if (gcdHasOptimization(option, gcvOPTIMIZATION_REDUNDANT_MOVE) &&
                    !gcdHasOptimization(option, gcvOPTIMIZATION_MIN_COMP_TIME))
                {
                    /* Optimize MOV instructions. */
                    gcmERR_BREAK(gcOpt_OptimizeMOVInstructions(optimizer));
                }

                gcmERR_BREAK(gcOpt_RemoveNOP(optimizer));
                gcmERR_BREAK(gcOpt_OptimizeConstantAssignment(optimizer));

                if (gcdHasOptimization(option, gcvOPTIMIZATION_CONSTANT_PROPAGATION))
                {
                    /* Propagate constants. */
                    gcmERR_BREAK(gcOpt_PropagateConstants(optimizer));
                }

                if (gcdHasOptimization(option, gcvOPTIMIZATION_CONSTANT_ARGUMENT_PROPAGATION))
                {
                    /* Propagate argument constants. */
                    gcmERR_BREAK(gcOpt_PropagateArgumentConstants(optimizer));
                }

                if (status == gcvSTATUS_CHANGED)
                {
                    /* Remove dead code. */
                    gcmERR_BREAK(gcOpt_RemoveDeadCode(optimizer));

                    if (gcdHasOptimization(option, gcvOPTIMIZATION_REDUNDANT_CHECKING))
                    {
                        /* Remove redundant checking. */
                        gcmERR_BREAK(gcOpt_RemoveRedundantCheckings(optimizer));
                    }
                }
            }
            while (status == gcvSTATUS_CHANGED);

            gcmERR_BREAK(status);

            if (gcdHasOptimization(option, gcvOPTIMIZATION_REDUNDANT_MOVE) &&
                !gcdHasOptimization(option, gcvOPTIMIZATION_MIN_COMP_TIME))
            {
                /* Optimize MOV instructions. */
                gcmERR_BREAK(gcOpt_OptimizeMOVInstructions(optimizer));
            }

            if (gcdHasOptimization(option, gcvOPTIMIZATION_DEAD_CODE))
            {
                /* Remove dead code. */
                gcmERR_BREAK(gcOpt_RemoveDeadCode(optimizer));
            }

            /* Remove NOP instructions from the shader. */
            gcmERR_BREAK(gcOpt_RemoveNOP(optimizer));

            if (gcdHasOptimization(option, gcvOPTIMIZATION_VECTOR_INSTRUCTION_MERGE))
            {
                /* Merge separate vector instructions into one vector instruction. */
                gcmERR_BREAK(gcOpt_MergeVectorInstructions(optimizer));
            }

            /* After merge separate vector instructions, maybe some constants are merge together,
            ** we should do propagate constants again.
            */
            if (gcdHasOptimization(option, gcvOPTIMIZATION_CONSTANT_PROPAGATION) &&
                !gcdHasOptimization(option, gcvOPTIMIZATION_MIN_COMP_TIME))
            {
                /* Propagate constants. */
                gcmERR_BREAK(gcOpt_PropagateConstants(optimizer));

                if (status == gcvSTATUS_CHANGED)
                {
                    /* Optimize MOV instructions. */
                    gcmERR_BREAK(gcOpt_OptimizeMOVInstructions(optimizer));
                }
            }

            if (gcdHasOptimization(option, gcvOPTIMIZATION_CONSTANT_ARGUMENT_PROPAGATION))
            {
                /* Propagate argument constants. */
                gcmERR_BREAK(gcOpt_PropagateArgumentConstants(optimizer));
            }

            if (gcdHasOptimization(option, gcvOPTIMIZATION_MAD_INSTRUCTION))
            {
                /* Optimize MUL and ADD for MADD instructions. */
                gcmERR_BREAK(gcOpt_OptimizeMULADDInstructions(optimizer));
            }

            if (gcdHasOptimization(option, gcvOPTIMIZATION_CONDITIONALIZE))
            {
                /* move code to conditional block if possible. */
                gcmERR_BREAK(gcOpt_ConditionalizeCode(optimizer));
            }

            if (gcdHasOptimization(option, gcvOPTIMIZATION_LOADTIME_CONSTANT) &&
                !Shader->ltcInstructionCount && !Shader->ltcUniformCount)
            {
                gcmERR_BREAK(gcOpt_OptimizeMOVInstructions(optimizer));
                gcmERR_BREAK(gcOpt_RemoveDeadCode(optimizer));
                gcmERR_BREAK(gcOpt_RemoveNOP(optimizer));

                gcmERR_BREAK(gcOPT_OptimizeLoadtimeConstant(optimizer));
                if (status == gcvSTATUS_CHANGED)
                {
                    /* Evaluate LTC values that don't related to user define uniforms. */
                    gcmERR_BREAK(gcSHADER_EvaluateLTCValueWithinLinkTime(optimizer->shader));

                    /* Optimize MOV instructions. */
                    gcmERR_BREAK(gcOpt_OptimizeMOVInstructions(optimizer));

                    /* Remove dead code. */
                    gcmERR_BREAK(gcOpt_RemoveDeadCode(optimizer));
                }
            }

            /* adding some dummy code, cannot do DCE after it */
            if (gcdHasOptimization(option, gcvOPTIMIZATION_POWER_OPTIMIZATION))
            {
                /* patch texld  */
                gcmERR_BREAK(gcOpt_PowerOptimization(optimizer));
            }

            gcmERR_BREAK(gcOpt_RemoveNOP(optimizer));

            /* When VIRCG is enabled, we will disable gcsl inliner unless
               the inlineKind is GCSL_INLINER_KIND, which is set through
               VC_OPTION=-INLINER:0 */
            if (gcdHasOptimization(option, gcvOPTIMIZATION_INLINE_EXPANSION) &&
                (gcGetVIRCGKind(optimizer->hwCfg.hwFeatureFlags.hasHalti2) == VIRCG_None || gcmOPT_INLINERKIND() == GCSL_INLINER_KIND))
            {
                gcmERR_BREAK(gcOpt_OptimizeCallStackDepth(&optimizer));
                gcOpt_UpdateCodeFunction(optimizer);
            }

            if (status == gcvSTATUS_CHANGED)
            {
                /* Optimize MOV instructions. */
                gcmERR_BREAK(gcOpt_OptimizeMOVInstructions(optimizer));

                /* Remove dead code. */
                gcmERR_BREAK(gcOpt_RemoveDeadCode(optimizer));
            }

            if (gcdHasOptimization(option, gcvOPTIMIZATION_REDUNDANT_CHECKING))
            {
                gcmERR_BREAK(gcOpt_RemoveRedundantCheckings(optimizer));
            }

            gcmERR_BREAK(gcOpt_RemoveNOP(optimizer));

            gcmERR_BREAK(gcOpt_MergeBranchCode(optimizer));

            if (gcdHasOptimization(option, gcvOPTIMIZATION_LOAD_SW_W))
            {
                /* Remove NOP instructions from the shader. */
                gcmERR_BREAK(gcOpt_RemoveNOP(optimizer));

                gcmERR_BREAK(gcOpt_LoadSWW(optimizer));
            }

            /* Call inline again. After above optimizations,
               maybe it'll have enough budget to inline. */
            if (optimizer->functionCount > 0)
            {
                if (gcdHasOptimization(option, gcvOPTIMIZATION_INLINE_EXPANSION) &&
                    (gcGetVIRCGKind(optimizer->hwCfg.hwFeatureFlags.hasHalti2) == VIRCG_None || gcmOPT_INLINERKIND() == GCSL_INLINER_KIND))
                {
                    /* Remove nops before inline function optimization, so there can be more code budget for expand functions. */
                    gcmERR_BREAK(gcOpt_RemoveNOP(optimizer));

                    /* Expand functions used only once or short but with many
                    ** arguments. */
                    gcmERR_BREAK(gcOpt_InlineFunctions(&optimizer, gcvFALSE));

                    gcOpt_UpdateCodeFunction(optimizer);

                    if (status == gcvSTATUS_CHANGED)
                    {
                        /* Remove dead code. */
                        gcmERR_BREAK(gcOpt_RemoveDeadCode(optimizer));

                        /* Optimize MOV instructions for arguments and outputs. */
                        gcOpt_OptimizeMOVInstructions(optimizer);
                    }
                }
            }
        }
        else if (gcdHasOptimization(option, gcvOPTIMIZATION_UNIT_TEST))
        {
            status = gcvSTATUS_OK;

            if (gcdHasOptimization(option, gcvOPTIMIZATION_DEAD_CODE))
            {
                status = gcOpt_RemoveDeadCode(optimizer);
            }

            if (gcdHasOptimization(option, gcvOPTIMIZATION_REDUNDANT_MOVE))
            {
                status = gcOpt_OptimizeMOVInstructions(optimizer);
            }

            /* When VIRCG is enabled, we will disable gcsl inliner unless
               the inlineKind is GCSL_INLINER_KIND, which is set through
               VC_OPTION=-INLINER:0 */
            if (gcdHasOptimization(option, gcvOPTIMIZATION_INLINE_EXPANSION) &&
                (gcGetVIRCGKind(optimizer->hwCfg.hwFeatureFlags.hasHalti2) == VIRCG_None || gcmOPT_INLINERKIND() == GCSL_INLINER_KIND))
            {
                status = gcOpt_InlineFunctions(&optimizer, gcvFALSE);
                gcOpt_UpdateCodeFunction(optimizer);
            }

            if (gcdHasOptimization(option, gcvOPTIMIZATION_UPDATE_PRECISION))
            {
                gcOpt_UpdatePrecision(optimizer);
            }

            if (gcdHasOptimization(option, gcvOPTIMIZATION_CONSTANT_PROPAGATION))
            {
                status = gcOpt_PropagateConstants(optimizer);
            }

            if (gcdHasOptimization(option, gcvOPTIMIZATION_REDUNDANT_CHECKING))
            {
                status = gcOpt_PropagateConstants(optimizer);
                status = gcOpt_RemoveRedundantCheckings(optimizer);
            }

            if (gcdHasOptimization(option, gcvOPTIMIZATION_MAD_INSTRUCTION))
            {
                /* Optimize MUL and ADD for MADD instructions. */
                gcmERR_BREAK(gcOpt_OptimizeMULADDInstructions(optimizer));
            }

            if (gcdHasOptimization(option, gcvOPTIMIZATION_LOAD_SW_W))
            {
                gcmERR_BREAK(gcOpt_LoadSWW(optimizer));
            }
        }

        /* Remove NOP instructions from the shader. */
        gcmERR_BREAK(gcOpt_RemoveNOP(optimizer));

        gcmERR_BREAK(gcOpt_CopyOutShader(optimizer, Shader));
    }
    while (gcvFALSE);

    if (optimizer != gcvNULL)
    {
        /* Free optimizer. */
        gcmVERIFY_OK(gcOpt_DestroyOptimizer(optimizer));
        optimizer = gcvNULL;
    }

    if (!useFullNewLinker)
    {
        gcmVERIFY_OK(_removeUnusedArguments(Shader));
    }


    if (gcmOPT_PackRegister())
    {
        if (dumpOptimizer)
        {
            gcOpt_Dump(LogFile, "Shader before packing register", gcvNULL, Shader);
            gcOpt_DumpMessage(gcvNULL, LogFile, "gcOptimizeShader: end\n");
        }

        gcmVERIFY_OK(gcSHADER_PackRegister(Shader));

        if (dumpOptimizer)
        {
            gcOpt_Dump(LogFile, "Final shader after gcOptimizeShader()", gcvNULL, Shader);
            gcOpt_DumpMessage(gcvNULL, LogFile, "gcOptimizeShader: end\n");
        }
    }

    /*gcOpt_GenShader(Shader, LogFile);*/

    /* Return the status. */
    gcmFOOTER();
    return status;
}
#endif


