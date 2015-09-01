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


#include "vir/transform/gc_vsc_vir_peephole.h"

void VSC_PH_Peephole_Init(
    IN OUT VSC_PH_Peephole* ph,
    IN VIR_Shader* shader,
    IN VIR_DEF_USAGE_INFO* du_info,
    IN VSC_OPTN_PHOptions* options,
    IN VIR_Dumper* dumper
    )
{
    VSC_PH_Peephole_SetShader(ph, shader);
    VSC_PH_Peephole_SetCurrBB(ph, gcvNULL);
    VSC_PH_Peephole_SetDUInfo(ph, du_info);
    VSC_PH_Peephole_SetOptions(ph, options);
    VSC_PH_Peephole_SetDumper(ph, dumper);
    vscPMP_Intialize(VSC_PH_Peephole_GetPmp(ph), gcvNULL, 1024,
                     sizeof(void*), gcvTRUE);
     VSC_PH_Peephole_SetCfgChanged(ph, gcvFALSE);
}

void VSC_PH_Peephole_Final(
    IN OUT VSC_PH_Peephole* ph
    )
{
    VSC_PH_Peephole_SetShader(ph, gcvNULL);
    VSC_PH_Peephole_SetOptions(ph, gcvNULL);
    VSC_PH_Peephole_SetDumper(ph, gcvNULL);
    vscPMP_Finalize(VSC_PH_Peephole_GetPmp(ph));
}

typedef struct VSC_PH_MODIFIERTOGEN
{
    VIR_OpCode opc : 20;
    gctUINT lvalue : 1;
    gctUINT modifier : 3;
    gctSTRING name;
} VSC_PH_ModifierToGen;

#define VSC_PH_ModifierToGen_GetOPC(mtg)            ((mtg)->opc)
#define VSC_PH_ModifierToGen_SetOPC(mtg, o)         ((mtg)->opc = (o))
#define VSC_PH_ModifierToGen_GetLValue(mtg)         ((mtg)->lvalue)
#define VSC_PH_ModifierToGen_SetLValue(mtg, l)      ((mtg)->lvalue = (l))
#define VSC_PH_ModifierToGen_GetModifier(mtg)       ((mtg)->modifier)
#define VSC_PH_ModifierToGen_SetModifier(mtg, m)    ((mtg)->modifier = (m))
#define VSC_PH_ModifierToGen_GetName(mtg)           ((mtg)->name)
#define VSC_PH_ModifierToGen_SetName(mtg, n)        ((mtg)->name = (n))

#define VSC_PH_ModifierToGen_COUNT                  3
VSC_PH_ModifierToGen VSC_PH_ModifierToGen_SAT = {VIR_OP_SAT, 1, VIR_MOD_SAT_0_TO_1, "SAT_0_TO_1"};
VSC_PH_ModifierToGen VSC_PH_ModifierToGen_NEG = {VIR_OP_NEG, 0, VIR_MOD_NEG, "NEG"};
VSC_PH_ModifierToGen VSC_PH_ModifierToGen_ABS = {VIR_OP_ABS, 0, VIR_MOD_ABS, "ABS"};

static void VSC_PH_ModifierToGen_Dump(VSC_PH_ModifierToGen* mtg, VIR_Dumper* dumper)
{
    VIR_LOG(dumper, "%s modifier %s\n", VSC_PH_ModifierToGen_GetLValue(mtg) ? "Lvalue" : "Rvalue", VSC_PH_ModifierToGen_GetName(mtg));
}

typedef struct VSC_PH_OPNDTARGET
{
    VIR_Instruction* inst;
    VIR_Operand* opnd;
} VSC_PH_OpndTarget;

static gctUINT _VSC_PH_OpndTarget_HFUNC(const void* ptr)
{
    return (gctUINT)(gctUINTPTR_T)((VSC_PH_OpndTarget*)ptr)->inst & 0xff;
}

static gctBOOL _VSC_PH_OpndTarget_HKCMP(const void* pHashKey1, const void* pHashKey2)
{
    return (((VSC_PH_OpndTarget*)pHashKey1)->inst == ((VSC_PH_OpndTarget*)pHashKey2)->inst)
        && (((VSC_PH_OpndTarget*)pHashKey1)->opnd == ((VSC_PH_OpndTarget*)pHashKey2)->opnd);
}

static VSC_PH_OpndTarget* _VSC_PH_Peephole_NewOpndTarget(
    IN OUT VSC_PH_Peephole* ph,
    IN VIR_USAGE* usage
    )
{
    VSC_PH_OpndTarget* ot = (VSC_PH_OpndTarget*)vscMM_Alloc(VSC_PH_Peephole_GetMM(ph), sizeof(VSC_PH_OpndTarget));
    ot->inst = usage->usageKey.pUsageInst;
    ot->opnd = usage->usageKey.pOperand;
    return ot;
}

typedef struct VSC_PH_MERGE_KEY
{
    VIR_OpCode      opcode;
    gctUINT         src1_imm;
    VIR_DEF_KEY     *defKey;
} VSC_PH_MergeKey;

static gctUINT _HKCMP_MergeKeyHFUNC(const void* pKey)
{
    VSC_PH_MergeKey*     pMergeKey = (VSC_PH_MergeKey*)pKey;

    gctUINT hashVal = (((((gctUINT)(gctUINTPTR_T) pMergeKey->defKey->pDefInst) & 0xFF) << 4) |
                          (pMergeKey->src1_imm & 0xF)) & 0xFFF;

    return hashVal;
}

static gctBOOL _HKCMP_MergeKeyEqual(const void* pHashKey1, const void* pHashKey2)
{
    VSC_PH_MergeKey*     pMergeKey1 = (VSC_PH_MergeKey*)pHashKey1;
    VSC_PH_MergeKey*     pMergeKey2 = (VSC_PH_MergeKey*)pHashKey2;

    if (pMergeKey1->defKey == gcvNULL || pMergeKey2->defKey == gcvNULL)
    {
        return gcvFALSE;
    }

    if ((pMergeKey1->opcode == pMergeKey2->opcode) &&
        (pMergeKey1->src1_imm == pMergeKey2->src1_imm) &&
        (pMergeKey1->defKey->pDefInst == pMergeKey2->defKey->pDefInst || /* Only compare Inst, not channel*/
         pMergeKey1->defKey->pDefInst == VIR_ANY_DEF_INST  ||
         pMergeKey1->defKey->pDefInst == VIR_ANY_DEF_INST))
    {
        return (pMergeKey1->defKey->regNo == pMergeKey2->defKey->regNo);
    }
    else
    {
        return gcvFALSE;
    }
}

static VSC_PH_MergeKey* _HKCMP_NewMergeKey(
    VSC_PH_Peephole     *ph,
    VIR_DEF_KEY         *defKey,
    VIR_OpCode          opcode,
    gctUINT             src1_imm
    )
{
    VSC_PH_MergeKey* outputKey = (VSC_PH_MergeKey*)vscMM_Alloc(VSC_PH_Peephole_GetMM(ph), sizeof(VSC_PH_MergeKey));
    outputKey->defKey = defKey;
    outputKey->opcode = opcode;
    outputKey->src1_imm = src1_imm;

    return outputKey;
}

typedef struct VSC_PH_MERGED_INST
{
    VIR_Instruction *inst;
    VIR_Enable      enable;
} VSC_PH_MergedInst;

static VSC_PH_MergedInst* _VSC_PH_Peephole_NewMergedInst(
    IN OUT VSC_PH_Peephole  *ph,
    IN VIR_Instruction      *inst,
    IN gctUINT8            channel
    )
{
    VSC_PH_MergedInst *mergedInst = (VSC_PH_MergedInst*)vscMM_Alloc(VSC_PH_Peephole_GetMM(ph), sizeof(VSC_PH_MergedInst));
    mergedInst->inst = inst;
    mergedInst->enable = (VIR_Enable) (1 << channel);
    return mergedInst;
}

static gctBOOL _VSC_PH_DoesOpcodeSupportLValueModifier(
    IN VIR_OpCode opcode
    )
{
    if (VIR_OPCODE_isTexLd(opcode))
    {
        return gcvFALSE;
    }

    if (opcode == VIR_OP_LOAD      ||
        opcode == VIR_OP_LOAD_ATTR ||
        opcode == VIR_OP_ATTR_LD)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static VSC_ErrCode _VSC_PH_GenerateLValueModifier(
    IN OUT VSC_PH_Peephole* ph,
    IN VIR_Instruction* inst,
    IN VSC_PH_ModifierToGen* mtg,
    OUT gctBOOL* generated
    )
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_Shader* shader = VSC_PH_Peephole_GetShader(ph);
    VIR_Function* func = VIR_Shader_GetCurrentFunction(shader);
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);

    VIR_Operand* inst_dest;
    VIR_Enable inst_dest_enable;
    VIR_Operand* inst_src0;
    VIR_Swizzle inst_src0_swizzle, swizzling_swizzle, mapping_swizzle;
    VIR_Enable inst_src0_enable;
    VIR_OperandInfo inst_dest_info, inst_src0_info;

    gctBOOL invalid_case = gcvFALSE;
    VSC_HASH_TABLE* work_set = gcvNULL;
    VSC_HASH_TABLE* inst_usage_set = gcvNULL;
    gctUINT8 channel;
    VIR_GENERAL_UD_ITERATOR ud_iter;
    VIR_DEF* def;

    inst_src0 = VIR_Inst_GetSource(inst, 0);
    inst_src0_swizzle = VIR_Operand_GetSwizzle(inst_src0);
    inst_src0_enable = VIR_Swizzle_2_Enable(inst_src0_swizzle);
    VIR_Operand_GetOperandInfo(inst, inst_src0, &inst_src0_info);

    inst_dest = VIR_Inst_GetDest(inst);
    inst_dest_enable = VIR_Operand_GetEnable(inst_dest);
    VIR_Operand_GetOperandInfo(inst, inst_dest, &inst_dest_info);

    swizzling_swizzle = VIR_Swizzle_GetSwizzlingSwizzle(inst_src0_swizzle, VIR_Enable_2_Swizzle(inst_src0_enable));

    /* inst_src0_swizzle should be one-to-one mapping to inst_dest_enable */
    if(!VIR_Swizzle_GetMappingSwizzle2Enable(inst_src0_swizzle, inst_dest_enable, &mapping_swizzle))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "not processed because inst_src_swizzle is not one-to-oen mapping to inst_dest_enable.\n");
            VIR_LOG_FLUSH(dumper);
        }
        if(generated != gcvNULL)
        {
            *generated = gcvFALSE;
        }
        return errCode;
    }

    /* inst_dest should not have modifier */
    if(VIR_Operand_GetModifier(inst_dest))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "not processed because its dest has modifier.\n");
            VIR_LOG_FLUSH(dumper);
        }
        if(generated != gcvNULL)
        {
            *generated = gcvFALSE;
        }
        return errCode;
    }

    /* inst_src0 should not have modifier */
    if(VIR_Operand_GetModifier(inst_src0))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "not processed because its src0 has modifier.\n");
            VIR_LOG_FLUSH(dumper);
        }
        if(generated != gcvNULL)
        {
            *generated = gcvFALSE;
        }
        return errCode;
    }

    /* inst_src0 should not be an input or output. */
    if(inst_src0_info.isInput || inst_src0_info.isOutput)
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "not processed because its src0 is an input or output.\n");
            VIR_LOG_FLUSH(dumper);
        }
        if(generated != gcvNULL)
        {
            *generated = gcvFALSE;
        }
        return errCode;
    }

    /* check prerequisite and collect work set at the same time */
    work_set = vscHTBL_Create(VSC_PH_Peephole_GetMM(ph), vscHFUNC_Default, vscHKCMP_Default, 512);
    vscVIR_InitGeneralUdIterator(&ud_iter, VSC_PH_Peephole_GetDUInfo(ph), inst, inst_src0, gcvFALSE);
    def = vscVIR_GeneralUdIterator_First(&ud_iter);

    /* inst_src0 should have def. */
    if(def == gcvNULL)
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "not processed because its src0 does not have a def.\n");
            VIR_LOG_FLUSH(dumper);
        }
        if(generated != gcvNULL)
        {
            *generated = gcvFALSE;
        }
        return errCode;
    }

    for(; def != gcvNULL; def = vscVIR_GeneralUdIterator_Next(&ud_iter))
    {
        VIR_Instruction* def_inst = def->defKey.pDefInst;
        VIR_Operand* def_inst_dest;
        VIR_Enable def_inst_enable;
        if(vscHTBL_DirectTestAndGet(work_set, (void*)def_inst, gcvNULL))
        {
            continue;
        }

        /* op should support lvalue modifier */
        if(!_VSC_PH_DoesOpcodeSupportLValueModifier(VIR_Inst_GetOpcode(def_inst)))
        {
            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
            {
                VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                VIR_LOG(dumper, "prevented by its def instruction:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, def_inst);
                VIR_LOG(dumper, "because the op dose not support Lvalue modifier\n");
                VIR_LOG_FLUSH(dumper);
            }
            invalid_case = gcvTRUE;
            break;
        }

        def_inst_dest = VIR_Inst_GetDest(def_inst);
        def_inst_enable = VIR_Operand_GetEnable(def_inst_dest);

        /* inst_dest_enable should cover def_inst_enable */
        if(!VIR_Enable_Covers(inst_src0_enable, def_inst_enable))
        {
            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
            {
                VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                VIR_LOG(dumper, "prevented by its def instruction:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, def_inst);
                VIR_LOG(dumper, "because the enable is not covered\n");
                VIR_LOG_FLUSH(dumper);
            }
            invalid_case = gcvTRUE;
            break;
        }
        /* may be improved later */
        else if(VIR_Operand_GetModifier(def_inst_dest))
        {
            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
            {
                VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                VIR_LOG(dumper, "not processed because its def in:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, def_inst);
                VIR_LOG(dumper, "has modifier:\n");
                VIR_LOG_FLUSH(dumper);
            }
            invalid_case = gcvTRUE;
            break;
        }
        /* inst and its def should be in the same bb */
        if(VIR_Inst_GetBasicBlock(inst) != VIR_Inst_GetBasicBlock(def_inst))
        {
            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
            {
                VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                VIR_LOG(dumper, "not processed because of the following def and inst are not in the same bb:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, def_inst);
                VIR_LOG_FLUSH(dumper);
            }
            invalid_case = gcvTRUE;
            break;
        }
        /* inst should be the only use of def_inst */
        {
            VIR_Instruction* breaking_use;
            if(!vscVIR_IsUniqueUsageInstOfDefInst(VSC_PH_Peephole_GetDUInfo(ph), def_inst, inst, &breaking_use))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "prevented by another use instruction:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, breaking_use);
                    VIR_LOG_FLUSH(dumper);
                }
                invalid_case = gcvTRUE;
                break;
            }
        }
        /* there should be no use of inst_dest between def_inst and inst */
        {
            VIR_Instruction* next = VIR_Inst_GetNext(def_inst);
            while(next != inst)
            {
                gctUINT i;
                for(i = 0; i < VIR_Inst_GetSrcNum(next); i++)
                {
                    if(VIR_Operand_SameLocation(inst, inst_dest, next, VIR_Inst_GetSource(next, i)))
                    {
                        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
                        {
                            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                            VIR_LOG(dumper, "not processed because between def_inst and inst:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, def_inst);
                            VIR_LOG(dumper, "\nthis intruction uses inst_dest:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, next);
                            VIR_LOG_FLUSH(dumper);
                        }
                        invalid_case = gcvTRUE;
                        break;
                    }
                }
                if(invalid_case)
                {
                    break;
                }
                next = VIR_Inst_GetNext(next);
            }
        }

        if(invalid_case)
        {
            break;
        }

        /* all prerequisites are met. add this def_inst to work set */
        vscHTBL_DirectSet(work_set, (void*)def_inst, gcvNULL);
    }

    gcmASSERT(invalid_case || HTBL_GET_ITEM_COUNT(work_set));

    /* do transformation */
    if(!invalid_case && HTBL_GET_ITEM_COUNT(work_set))
    {
        /* collect the usage of the dest of the input inst, change the sym and map the usage channel */
        {
            VIR_GENERAL_DU_ITERATOR inst_du_iter;
            VIR_USAGE* inst_usage;

            inst_usage_set = vscHTBL_Create(VSC_PH_Peephole_GetMM(ph), _VSC_PH_OpndTarget_HFUNC, _VSC_PH_OpndTarget_HKCMP, 512);
            for(channel = 0; channel < VIR_CHANNEL_NUM; channel++)
            {
                if(!(inst_dest_enable & (1 << channel)))
                {
                    continue;
                }

                vscVIR_InitGeneralDuIterator(&inst_du_iter, VSC_PH_Peephole_GetDUInfo(ph), inst, inst_dest_info.u1.virRegInfo.virReg, channel, gcvFALSE);

                for(inst_usage = vscVIR_GeneralDuIterator_First(&inst_du_iter); inst_usage != gcvNULL;
                    inst_usage = vscVIR_GeneralDuIterator_Next(&inst_du_iter))
                {
                    if(vscHTBL_DirectTestAndGet(inst_usage_set, (void*)&(inst_usage->usageKey), gcvNULL))
                    {
                        continue;
                    }
                    vscHTBL_DirectSet(inst_usage_set, (void*)_VSC_PH_Peephole_NewOpndTarget(ph, inst_usage), gcvNULL);
                }
            }
        }

        /* do modification on work set */
        {
            VSC_HASH_ITERATOR work_set_iter;
            VSC_DIRECT_HNODE_PAIR work_set_pair;
            vscHTBLIterator_Init(&work_set_iter, work_set);
            for(work_set_pair = vscHTBLIterator_DirectFirst(&work_set_iter);
                IS_VALID_DIRECT_HNODE_PAIR(&work_set_pair); work_set_pair = vscHTBLIterator_DirectNext(&work_set_iter))
            {
                VIR_Instruction* work_inst = (VIR_Instruction*)VSC_DIRECT_HNODE_PAIR_FIRST(&work_set_pair);
                VIR_Operand* work_inst_dest = VIR_Inst_GetDest(work_inst);
                VIR_Enable old_work_inst_enable = VIR_Operand_GetEnable(work_inst_dest);
                VIR_Enable new_work_inst_enable = VIR_Swizzle_2_Enable(VIR_Swizzle_ApplyMappingSwizzle(VIR_Enable_2_Swizzle(old_work_inst_enable), mapping_swizzle));
                gctUINT i;

                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "changed def instrucion from:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, work_inst);
                    VIR_LOG_FLUSH(dumper);
                }

                /* this statement is the modification */
                vscVIR_DeleteDef(VSC_PH_Peephole_GetDUInfo(ph), work_inst, inst_src0_info.u1.virRegInfo.virReg,
                                 1, old_work_inst_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
                VIR_Operand_ReplaceDefOperandWithDef(work_inst_dest, inst_dest, new_work_inst_enable);
                vscVIR_AddNewDef(VSC_PH_Peephole_GetDUInfo(ph), work_inst, inst_dest_info.u1.virRegInfo.virReg, 1,
                                 new_work_inst_enable, VIR_HALF_CHANNEL_MASK_FULL,
                                 inst_dest_info.isInput, inst_dest_info.isOutput, gcvNULL);

                if (!(VIR_Inst_GetOpcode(work_inst) == VIR_OP_DP2
                    || VIR_Inst_GetOpcode(work_inst) == VIR_OP_DP3
                    || VIR_Inst_GetOpcode(work_inst) == VIR_OP_DP4))
                {
                    for(i = 0; i < VIR_Inst_GetSrcNum(work_inst); i++)
                    {
                        VIR_Operand* src = VIR_Inst_GetSource(work_inst, i);
                        VIR_Operand_SetSwizzle(src, VIR_Swizzle_ApplySwizzlingSwizzle(VIR_Operand_GetSwizzle(src), swizzling_swizzle));
                    }
                }

                if(VIR_Inst_GetOpcode(work_inst) == VIR_OP_MOV)
                {
                    VIR_Inst_SetOpcode(work_inst, VIR_OP_SAT);
                }
                else
                {
                    VIR_Operand_SetModifier(work_inst_dest, VSC_PH_ModifierToGen_GetModifier(mtg));
                }

                /* add the usage info between the def of work_inst and the src of inst_usage_inst */
                {
                    VSC_HASH_ITERATOR inst_usage_set_iter;
                    VSC_DIRECT_HNODE_PAIR inst_usage_set_pair;
                    vscHTBLIterator_Init(&inst_usage_set_iter, inst_usage_set);
                    for(inst_usage_set_pair = vscHTBLIterator_DirectFirst(&inst_usage_set_iter);
                        IS_VALID_DIRECT_HNODE_PAIR(&inst_usage_set_pair); inst_usage_set_pair = vscHTBLIterator_DirectNext(&inst_usage_set_iter))
                    {
                        VSC_PH_OpndTarget* inst_usage = (VSC_PH_OpndTarget*)VSC_DIRECT_HNODE_PAIR_FIRST(&inst_usage_set_pair);
                        VIR_Instruction* inst_usage_inst = inst_usage->inst;
                        VIR_Operand* inst_usage_opnd;
                        VIR_Swizzle swizzle;
                        VIR_Enable enable;

                        if (VIR_IS_OUTPUT_USAGE_INST(inst_usage_inst))
                        {
                            /* Operand of output usage must be reg number of def */
                            inst_usage_opnd = (VIR_Operand*)(gctUINTPTR_T)(inst_dest_info.u1.virRegInfo.virReg);
                            swizzle = VIR_SWIZZLE_XYZW;
                            enable = VIR_ENABLE_XYZW;
                        }
                        else
                        {
                            inst_usage_opnd = inst_usage->opnd;
                            swizzle = VIR_Operand_GetSwizzle(inst_usage_opnd);
                            enable = VIR_Swizzle_2_Enable(swizzle);
                        }

                        vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph), inst, inst_usage_inst, inst_usage_opnd,
                                           inst_dest_info.u1.virRegInfo.virReg, 1,
                                           (VIR_Enable)(enable & inst_dest_enable), VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
                        if(new_work_inst_enable & enable)
                            vscVIR_AddNewUsageToDef(VSC_PH_Peephole_GetDUInfo(ph), work_inst, inst_usage_inst, inst_usage_opnd,
                                                    inst_dest_info.u1.virRegInfo.virReg, 1, (VIR_Enable)(new_work_inst_enable & enable),
                                                    VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
                    }
                }

                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "to:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, work_inst);
                    VIR_LOG_FLUSH(dumper);
                }

                /* recursively deal with newly generated SAT instruction */
                /*if(VIR_Inst_GetOpcode(work_inst) == VIR_OP_SAT)
                {
                    _VSC_PH_GenerateLValueModifier(ph, work_inst, mtg, generated);
                }*/
            }
        }

        /* delete the usage info of the src of input inst */
        vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph), VIR_ANY_DEF_INST, inst,
                           inst_src0, inst_src0_info.u1.virRegInfo.virReg, 1,
                           inst_src0_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        /* delete the def info of the dest of input inst */
        vscVIR_DeleteDef(VSC_PH_Peephole_GetDUInfo(ph), inst, inst_dest_info.u1.virRegInfo.virReg,
                         1, inst_dest_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        /* remove the input inst */
        VIR_Function_RemoveInstruction(func, inst);
        vscHTBL_Destroy(inst_usage_set);
    }

    vscHTBL_Destroy(work_set);

    return errCode;
}

static gctBOOL _VSC_PH_DoesOpcodeSupportRValueModifier(
    IN VIR_OpCode opcode
    )
{
    if(opcode == VIR_OP_AND_BITWISE ||
       opcode == VIR_OP_OR_BITWISE ||
       opcode == VIR_OP_XOR_BITWISE)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static VSC_ErrCode _VSC_PH_GenerateRValueModifier(
    IN OUT VSC_PH_Peephole* ph,
    IN VIR_Instruction* inst,
    IN VSC_PH_ModifierToGen* mtg,
    OUT gctBOOL* generated
    )
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_Shader* shader = VSC_PH_Peephole_GetShader(ph);
    VIR_Function* func = VIR_Shader_GetCurrentFunction(shader);
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VIR_Operand* inst_dest;
    VIR_Operand* inst_src0;
    VIR_Swizzle inst_src0_swizzle, mapping_swizzle;
    VIR_Enable inst_src0_enable, inst_dest_enable;
    VIR_OperandInfo inst_dest_info, inst_src0_info;
    gctBOOL invalid_case = gcvFALSE, skipSrcModCheck = gcvFALSE;
    VSC_HASH_TABLE* work_set = gcvNULL;
    VSC_HASH_TABLE* def_inst_set = gcvNULL;
    gctUINT8 channel;
    VIR_GENERAL_DU_ITERATOR du_iter;

    inst_src0 = VIR_Inst_GetSource(inst, 0);
    inst_src0_swizzle = VIR_Operand_GetSwizzle(inst_src0);
    inst_src0_enable = VIR_Swizzle_2_Enable(inst_src0_swizzle);
    VIR_Operand_GetOperandInfo(inst, inst_src0, &inst_src0_info);

    inst_dest = VIR_Inst_GetDest(inst);
    inst_dest_enable = VIR_Operand_GetEnable(inst_dest);
    VIR_Operand_GetOperandInfo(inst, inst_dest, &inst_dest_info);

    mapping_swizzle = VIR_Enable_GetMappingSwizzle(inst_dest_enable, inst_src0_swizzle);

    /* inst_dest should not have modifier */
    if(VIR_Operand_GetModifier(inst_dest))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "not processed because its dest has modifier.\n");
            VIR_LOG_FLUSH(dumper);
        }
        if(generated != gcvNULL)
        {
            *generated = gcvFALSE;
        }
        return errCode;
    }

    /* ||variable|| = |variable| */
    skipSrcModCheck = (mtg->modifier == VIR_MOD_ABS && VIR_Operand_GetModifier(inst_src0) == VIR_MOD_ABS);

    /* inst_src0 should not have modifier */
    if(VIR_Operand_GetModifier(inst_src0) && !skipSrcModCheck)
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "not processed because its src0 has modifier.\n");
            VIR_LOG_FLUSH(dumper);
        }
        if(generated != gcvNULL)
        {
            *generated = gcvFALSE;
        }
        return errCode;
    }

    /* inst_dest should not be an output */
    if(inst_dest_info.isOutput)
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "not processed because its dest is an output.\n");
            VIR_LOG_FLUSH(dumper);
        }
        if(generated != gcvNULL)
        {
            *generated = gcvFALSE;
        }
        return errCode;
    }

    /* check prerequisite and collect work set at the same time */
    work_set = vscHTBL_Create(VSC_PH_Peephole_GetMM(ph), _VSC_PH_OpndTarget_HFUNC, _VSC_PH_OpndTarget_HKCMP, 512);
    for(channel = 0; channel < VIR_CHANNEL_NUM; channel++)
    {
        VIR_USAGE* usage;
        if(!(inst_dest_enable & (1 << channel)))
        {
            continue;
        }
        vscVIR_InitGeneralDuIterator(&du_iter, VSC_PH_Peephole_GetDUInfo(ph), inst, inst_dest_info.u1.virRegInfo.virReg, channel, gcvFALSE);
        for(usage = vscVIR_GeneralDuIterator_First(&du_iter); usage != gcvNULL;
            usage = vscVIR_GeneralDuIterator_Next(&du_iter))
        {
            VIR_Instruction* usage_inst = usage->usageKey.pUsageInst;
            VIR_OpCode opcode = VIR_Inst_GetOpcode(usage_inst);
            VIR_Operand* usage_opnd;
            VIR_Swizzle usage_opnd_swizzle;
            VIR_Enable usage_opnd_enable;

            if(vscHTBL_DirectTestAndGet(work_set, (void*)&(usage->usageKey), gcvNULL))
            {
                continue;
            }

            usage_opnd = usage->usageKey.pOperand;
            usage_opnd_swizzle = VIR_Operand_GetSwizzle(usage_opnd);
            usage_opnd_enable = VIR_Swizzle_2_Enable(usage_opnd_swizzle);

            /* inst and its usage should be in the same bb */
            if(VIR_Inst_GetBasicBlock(inst) != VIR_Inst_GetBasicBlock(usage_inst))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "not processed because of the following usage and inst are not in the same bb:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, usage_inst);
                    VIR_LOG_FLUSH(dumper);
                }
                invalid_case = gcvTRUE;
                break;
            }
            /* usage inst must not be an integer instruction, like AND_BITWISSE */
            if(!_VSC_PH_DoesOpcodeSupportRValueModifier(opcode))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "not processed because of the following usage's op does not support Rvalue modifier:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, usage_inst);
                    VIR_LOG_FLUSH(dumper);
                }
                invalid_case = gcvTRUE;
                break;
            }
            /* inst_enalbe should cover the enable of inst's def's usage's enable */
            if(!VIR_Enable_Covers(inst_dest_enable, usage_opnd_enable))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "prevented by its use instruction:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, usage->usageKey.pUsageInst);
                    VIR_LOG(dumper, "because the enable is not covered\n");
                    VIR_LOG_FLUSH(dumper);
                }
                invalid_case = gcvTRUE;
                break;
            }
            /* could be improved later */
            else if(VIR_Operand_GetModifier(usage_opnd))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "not processed because its usage in:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, usage_inst);
                    VIR_LOG(dumper, "has modifier:\n");
                    VIR_LOG_FLUSH(dumper);
                }
                invalid_case = gcvTRUE;
                break;
            }
            /* in case of loop, inst should be the only def of usage_inst */
            {
                VIR_Instruction* breaking_def;
                if(!vscVIR_IsUniqueDefInstOfUsageInst(VSC_PH_Peephole_GetDUInfo(ph), usage_inst, usage_opnd, inst, &breaking_def))
                {
                    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
                    {
                        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                        VIR_LOG(dumper, "prevented by another def instruction:\n");
                        VIR_LOG_FLUSH(dumper);
                        VIR_Inst_Dump(dumper, breaking_def);
                        VIR_LOG(dumper, "\nto usage instruction:\n");
                        VIR_LOG_FLUSH(dumper);
                        VIR_Inst_Dump(dumper, usage_inst);
                        VIR_LOG_FLUSH(dumper);
                    }
                    invalid_case = gcvTRUE;
                    break;
                }
            }
            /* there should be no def of inst_src0 between inst and use_inst */
            {
                VIR_Instruction* next = VIR_Inst_GetNext(inst);
                while(next != usage_inst)
                {
                    if(VIR_Operand_SameLocation(inst, inst_src0, next, VIR_Inst_GetDest(next)))
                    {
                        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options),
                                          VSC_OPTN_PHOptions_TRACE_MODIFIER))
                        {
                            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                            VIR_LOG(dumper, "not processed because between inst and use_inst:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, usage_inst);
                            VIR_LOG(dumper, "\nthis intruction redefs inst_src0:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, next);
                            VIR_LOG_FLUSH(dumper);
                        }
                        invalid_case = gcvTRUE;
                        break;
                    }
                    next = VIR_Inst_GetNext(next);
                }
            }

            vscHTBL_DirectSet(work_set, (void*)_VSC_PH_Peephole_NewOpndTarget(ph, usage), gcvNULL);
        }

        if(invalid_case)
        {
            break;
        }
    }

    /* do transformation */
    if(!invalid_case && HTBL_GET_ITEM_COUNT(work_set))
    {
        /* collect the def of the src0 of the input inst, and delete the usage between input inst's src0's def and input inst's src0 */
        {
            VIR_GENERAL_UD_ITERATOR inst_ud_iter;
            VIR_DEF* def;

            def_inst_set = vscHTBL_Create(VSC_PH_Peephole_GetMM(ph), vscHFUNC_Default, vscHKCMP_Default, 512);

            vscVIR_InitGeneralUdIterator(&inst_ud_iter, VSC_PH_Peephole_GetDUInfo(ph), inst, inst_src0, gcvFALSE);
            for(def = vscVIR_GeneralUdIterator_First(&inst_ud_iter); def != gcvNULL;
                def = vscVIR_GeneralUdIterator_Next(&inst_ud_iter))
            {
                VIR_Instruction* def_inst = def->defKey.pDefInst;
                vscHTBL_DirectSet(def_inst_set, (void*)def_inst, gcvNULL);
            }
        }

        {
            VSC_HASH_ITERATOR work_set_iter;
            VSC_DIRECT_HNODE_PAIR work_set_pair;
            vscHTBLIterator_Init(&work_set_iter, work_set);
            for(work_set_pair = vscHTBLIterator_DirectFirst(&work_set_iter);
                IS_VALID_DIRECT_HNODE_PAIR(&work_set_pair); work_set_pair = vscHTBLIterator_DirectNext(&work_set_iter))
            {
                VSC_PH_OpndTarget* usage_tgt = (VSC_PH_OpndTarget*)VSC_DIRECT_HNODE_PAIR_FIRST(&work_set_pair);
                VIR_Instruction* work_inst = usage_tgt->inst;
                VIR_Operand* work_opnd = usage_tgt->opnd;
                VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(work_opnd);
                VIR_Enable old_work_inst_enable, work_inst_enable;

                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "changed instrucion from:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, work_inst);
                    VIR_LOG_FLUSH(dumper);
                }

                /* these two statements are the modification */
                swizzle = VIR_Operand_GetSwizzle(work_opnd);
                old_work_inst_enable = VIR_Swizzle_2_Enable(swizzle);
                swizzle = VIR_Swizzle_ApplyMappingSwizzle(swizzle, mapping_swizzle);
                work_inst_enable = VIR_Swizzle_2_Enable(swizzle);
                VIR_Operand_ReplaceUseOperandWithUse(work_opnd, inst_src0, swizzle);
                VIR_Operand_SetModifier(work_opnd, VSC_PH_ModifierToGen_GetModifier(mtg));
                vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph), VIR_ANY_DEF_INST, work_inst,
                                   work_opnd, inst_dest_info.u1.virRegInfo.virReg, 1,
                                   old_work_inst_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);

                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "to:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, work_inst);
                    VIR_LOG_FLUSH(dumper);
                }
                /* add the DU info between def_inst and work_inst */
                {
                    VSC_HASH_ITERATOR def_inst_set_iter;
                    VSC_DIRECT_HNODE_PAIR def_inst_set_pair;
                    vscHTBLIterator_Init(&def_inst_set_iter, def_inst_set);
                    for(def_inst_set_pair = vscHTBLIterator_DirectFirst(&def_inst_set_iter);
                        IS_VALID_DIRECT_HNODE_PAIR(&def_inst_set_pair); def_inst_set_pair = vscHTBLIterator_DirectNext(&def_inst_set_iter))
                    {
                        VIR_Instruction* def_inst = (VIR_Instruction*)VSC_DIRECT_HNODE_PAIR_FIRST(&def_inst_set_pair);
                        VIR_Operand* def_inst_dest;
                        VIR_Enable enable;

                        if (VIR_IS_IMPLICIT_DEF_INST(def_inst))
                        {
                            enable = VIR_ENABLE_XYZW;
                        }
                        else
                        {
                            def_inst_dest = VIR_Inst_GetDest(def_inst);
                            enable = VIR_Operand_GetEnable(def_inst_dest);
                        }

                        /* not all def of inst could be connected to work_inst, because
                           the channels used in work_inst is a subset of the enable of inst*/
                        if(enable & work_inst_enable)
                        {
                            vscVIR_AddNewUsageToDef(VSC_PH_Peephole_GetDUInfo(ph), def_inst, work_inst, work_opnd,
                                                    inst_src0_info.u1.virRegInfo.virReg, 1,
                                                    (VIR_Enable)(enable & work_inst_enable), VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
                        }
                    }
                }
            }
        }

        vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph), VIR_ANY_DEF_INST,
                           inst, inst_src0, inst_src0_info.u1.virRegInfo.virReg,
                           1, inst_src0_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        vscVIR_DeleteDef(VSC_PH_Peephole_GetDUInfo(ph), inst, inst_dest_info.u1.virRegInfo.virReg,
                         1, inst_dest_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        VIR_Function_RemoveInstruction(func, inst);
        vscHTBL_Destroy(def_inst_set);
    }

    vscHTBL_Destroy(work_set);
    return errCode;
}

static VSC_ErrCode _VSC_PH_GenerateModifier(
    IN OUT VSC_PH_Peephole* ph,
    IN VIR_Instruction* inst,
    IN VSC_PH_ModifierToGen* mtg,
    OUT gctBOOL* generated
    )
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_OpCode opc = VIR_Inst_GetOpcode(inst);
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VIR_Operand* inst_dest = VIR_Inst_GetDest(inst);
    VIR_Operand* inst_src0 = VIR_Inst_GetSource(inst, 0);

    if(opc != VSC_PH_ModifierToGen_GetOPC(mtg))
    {
        if(generated)
        {
            *generated = gcvFALSE;
        }
        return errCode;
    }

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        VIR_LOG(dumper, "\nInstrunction:\n");
        VIR_LOG_FLUSH(dumper);
        VIR_Inst_Dump(dumper, inst);
        VIR_LOG(dumper, "\nis up for generating ");
        VSC_PH_ModifierToGen_Dump(mtg, dumper);
        VIR_LOG_FLUSH(dumper);
    }

    /* if inst's src0 is const/imm, we do not process it. it should be handled by other optimization */
    if(VIR_Operand_GetOpKind(inst_src0) == VIR_OPND_CONST || VIR_Operand_GetOpKind(inst_src0) == VIR_OPND_IMMEDIATE)
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "not processed because its src0 is const/imm.\n");
            VIR_LOG_FLUSH(dumper);
        }
        if(generated != gcvNULL)
        {
            *generated = gcvFALSE;
        }
        return errCode;
    }

    if(VSC_PH_ModifierToGen_GetLValue(mtg))
    {
        /* if inst's src0 has fewer chanels used than dest, we do not process it. Would be enhanced later */
        if(VIR_Swizzle_Channel_Count(VIR_Operand_GetSwizzle(inst_src0)) < VIR_Enable_Channel_Count(VIR_Operand_GetEnable(inst_dest)))
        {
            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
            {
                VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                VIR_LOG(dumper, "not processed because its src0 has fewer chanels used than dest.\n");
                VIR_LOG_FLUSH(dumper);
            }
            if(generated != gcvNULL)
            {
                *generated = gcvFALSE;
            }
            return errCode;
        }
        errCode  = _VSC_PH_GenerateLValueModifier(ph, inst, mtg, generated);
    }
    else
    {
        /* if inst's src0 is an integer, skip it */
        VIR_Operand* src0 = VIR_Inst_GetSource(inst, 0);
        VIR_TypeId src0_typeid = VIR_Operand_GetType(src0);
        VIR_Type* src0_type = VIR_Shader_GetTypeFromId(VSC_PH_Peephole_GetShader(ph), src0_typeid);
        if(VIR_Type_GetFlags(src0_type) & VIR_TYFLAG_ISINTEGER)
        {
            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
            {
                VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                VIR_LOG(dumper, "not processed because its operating on interger.\n");
                VIR_LOG_FLUSH(dumper);
            }
            if(generated)
            {
                *generated = gcvFALSE;
            }
            return errCode;
        }

        errCode  = _VSC_PH_GenerateRValueModifier(ph, inst, mtg, generated);
    }

    return errCode;
}

static VSC_ErrCode _VSC_PH_GenerateModifiers(
    IN OUT VSC_PH_Peephole* ph,
    IN VIR_Instruction* inst,
    IN VSC_PH_ModifierToGen** mtgs,
    IN gctUINT mtg_count,
    OUT gctBOOL* generated
    )
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    gctUINT i;
    gctBOOL sub_gen = gcvFALSE;

    if(generated)
    {
        *generated = gcvFALSE;
    }
    for(i = 0; i < mtg_count; i++)
    {
        errCode = _VSC_PH_GenerateModifier(ph, inst, mtgs[i], &sub_gen);
        if(sub_gen && generated)
        {
            *generated = gcvTRUE;
        }
    }

    return errCode;
}

static VSC_ErrCode _VSC_PH_GenerateMAD(
    IN OUT VSC_PH_Peephole* ph,
    IN VIR_Instruction* mul,
    OUT gctBOOL* generated
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_Shader* shader = VSC_PH_Peephole_GetShader(ph);
    VIR_Function* func = VIR_Shader_GetCurrentFunction(shader);
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VIR_Swizzle mul_src0_swizzle, mul_src1_swizzle;
    VIR_Enable mul_enable, mul_src0_enable, mul_src1_enable;
    VIR_GENERAL_DU_ITERATOR du_iter;
    VIR_Operand *mul_dest, *mul_src0, *mul_src1;
    VIR_OperandInfo mul_dest_info, mul_src0_info, mul_src1_info;
    gctUINT8 channel;
    VSC_HASH_TABLE* add_sub_set;
    gctBOOL invalid_case = gcvFALSE;
    gctBOOL is_mulsat = VIR_Inst_GetOpcode(mul) == VIR_OP_MULSAT;

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        VIR_LOG(dumper, "\nMUL instruction:\n");
        VIR_LOG_FLUSH(dumper);
        VIR_Inst_Dump(dumper, mul);
        VIR_LOG_FLUSH(dumper);
    }

    mul_dest = VIR_Inst_GetDest(mul);
    if(VIR_Operand_GetModifier(mul_dest))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "cannot be processed because its dest has modifier.\n");
            VIR_LOG_FLUSH(dumper);
        }
        if(generated != gcvNULL)
        {
            *generated = gcvFALSE;
        }
        return errCode;
    }

    VIR_Operand_GetOperandInfo(mul, mul_dest, &mul_dest_info);
    gcmASSERT(~mul_dest_info.isTempVar && ~mul_dest_info.isVecConst);

    /* the result of mul can not be an output */
    if (mul_dest_info.isOutput)
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "not processed because its result is an output.\n");
            VIR_LOG_FLUSH(dumper);
        }
        if(generated != gcvNULL)
        {
            *generated = gcvFALSE;
        }
        return errCode;
    }

    mul_enable = VIR_Operand_GetEnable(mul_dest);
    mul_src0 = VIR_Inst_GetSource(mul, 0);
    VIR_Operand_GetOperandInfo(mul, mul_src0, &mul_src0_info);
    mul_src0_swizzle = VIR_Operand_GetSwizzle(mul_src0);
    mul_src0_enable = VIR_Swizzle_2_Enable(mul_src0_swizzle);
    mul_src1 = VIR_Inst_GetSource(mul, 1);
    VIR_Operand_GetOperandInfo(mul, mul_src1, &mul_src1_info);
    mul_src1_swizzle = VIR_Operand_GetSwizzle(mul_src1);
    mul_src1_enable = VIR_Swizzle_2_Enable(mul_src1_swizzle);

    add_sub_set = vscHTBL_Create(VSC_PH_Peephole_GetMM(ph), _VSC_PH_OpndTarget_HFUNC, _VSC_PH_OpndTarget_HKCMP, 512);
    /* get usage of MUL's dest in per channel way*/
    for(channel = 0; channel < VIR_CHANNEL_NUM; channel++)
    {
        VIR_USAGE* usage;
        if(!(mul_enable & (1 << channel)))
        {
            continue;
        }

        /*  */
        vscVIR_InitGeneralDuIterator(&du_iter, VSC_PH_Peephole_GetDUInfo(ph), mul, mul_dest_info.u1.virRegInfo.virReg, channel, gcvFALSE);
        for(usage = vscVIR_GeneralDuIterator_First(&du_iter); usage != gcvNULL;
            usage = vscVIR_GeneralDuIterator_Next(&du_iter))
        {
            VIR_Instruction* use_inst;
            VIR_Operand* use_inst_dest;
            VIR_Operand* use_inst_opnd;

            if(vscHTBL_DirectTestAndGet(add_sub_set, (void*)&(usage->usageKey), gcvNULL))
            {
                continue;
            }

            use_inst = usage->usageKey.pUsageInst;
            use_inst_opnd = usage->usageKey.pOperand;
            /* the result of mul must be used in either an add or a sub */
            if((!is_mulsat && VIR_Inst_GetOpcode(use_inst) != VIR_OP_ADD
                && VIR_Inst_GetOpcode(use_inst) != VIR_OP_SUB) ||
               (is_mulsat && VIR_Inst_GetOpcode(use_inst) != VIR_OP_ADDSAT
                && VIR_Inst_GetOpcode(use_inst) != VIR_OP_SUBSAT))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "not processed because of its usage in:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, use_inst);
                    VIR_LOG(dumper, "\nis neither an add nor a sub\n");
                    VIR_LOG_FLUSH(dumper);
                }
                invalid_case = gcvTRUE;
                break;
            }

            /* use_inst should not have different rounding mode from the mul instruction */
            use_inst_dest = VIR_Inst_GetDest(use_inst);
            if(VIR_Operand_GetRoundMode(use_inst_dest) != VIR_Operand_GetRoundMode(mul_dest))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "not processed because of its usage in:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, use_inst);
                    VIR_LOG(dumper, "\nhas different rounding mode\n");
                    VIR_LOG_FLUSH(dumper);
                }
                invalid_case = gcvTRUE;
                break;
            }

            /* use_inst_opnd should not have modifiers. could be improved here */
            if(VIR_Operand_GetModifier(use_inst_opnd))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "not processed because of its usage in:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, use_inst);
                    VIR_LOG(dumper, "\nhas modifier\n");
                    VIR_LOG_FLUSH(dumper);
                }
                invalid_case = gcvTRUE;
                break;
            }
            /* mul and add/sub should be in the same bb */
            if(VIR_Inst_GetBasicBlock(mul) != VIR_Inst_GetBasicBlock(use_inst))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "not processed because of the following usage and mul are not in the same bb:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, use_inst);
                    VIR_LOG_FLUSH(dumper);
                }
                invalid_case = gcvTRUE;
                break;
            }
            /* if mul_dest is used as both operands of an add/sub, skip it*/
            {
                VSC_HASH_ITERATOR iter;
                VSC_DIRECT_HNODE_PAIR pair;
                vscHTBLIterator_Init(&iter, add_sub_set);
                for(pair = vscHTBLIterator_DirectFirst(&iter);
                    IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
                {
                    VIR_USAGE* recorded_usage = (VIR_USAGE*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
                    if(recorded_usage->usageKey.pUsageInst == use_inst)
                    {
                        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD))
                        {
                            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                            VIR_LOG(dumper, "not processed because of its usage in:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, use_inst);
                            VIR_LOG(dumper, "\nhas identical source operands\n");
                            VIR_LOG_FLUSH(dumper);
                        }
                        invalid_case = gcvTRUE;
                        break;
                    }
                }
                if(invalid_case == gcvTRUE)
                {
                    break;
                }
            }
            /* since we can not apply neg modifier to integer operand,
               if the integer result of a mul is the 2nd source of sub,
               or the 2nd source of sub is neither const nor imm, we have to skip */
            if(VIR_Inst_GetOpcode(use_inst) == VIR_OP_SUB)
            {
                if((use_inst_opnd == VIR_Inst_GetSource(use_inst, 0) && !VIR_Operand_IsNegatable(shader, VIR_Inst_GetSource(use_inst, 1)))
                    || (use_inst_opnd == VIR_Inst_GetSource(use_inst, 1) && !VIR_Operand_IsNegatable(shader, mul_src0) && !VIR_Operand_IsNegatable(shader, mul_src1)))
                {
                    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD))
                    {
                        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                        VIR_LOG(dumper, "not processed because of its usage in sub:\n");
                        VIR_LOG_FLUSH(dumper);
                        VIR_Inst_Dump(dumper, use_inst);
                        VIR_LOG(dumper, "and this sub could not be converted to an add\n");
                        VIR_LOG_FLUSH(dumper);
                    }
                    invalid_case = gcvTRUE;
                    break;
                }
            }
            /* dest of mul should cover its usage in add/sub */
            {
                VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(use_inst_opnd);
                VIR_Enable add_sub_enable = VIR_Swizzle_2_Enable(swizzle);
                if(!VIR_Enable_Covers(mul_enable, add_sub_enable))
                {
                    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD))
                    {
                        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                        VIR_LOG(dumper, "not processed because of the following usage is not covered:\n");
                        VIR_LOG_FLUSH(dumper);
                        VIR_Inst_Dump(dumper, use_inst);
                        VIR_LOG_FLUSH(dumper);
                    }
                    invalid_case = gcvTRUE;
                    break;
                }
            }
            /* in case of loop, mul should be the only def of add/sub usage. */
            {
                VIR_Instruction* breaking_def;
                if(!vscVIR_IsUniqueDefInstOfUsageInst(VSC_PH_Peephole_GetDUInfo(ph), use_inst, use_inst_opnd, mul, &breaking_def))
                {
                    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
                    {
                        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                        VIR_LOG(dumper, "prevented by another def instruction:\n");
                        VIR_LOG_FLUSH(dumper);
                        VIR_Inst_Dump(dumper, breaking_def);
                        VIR_LOG(dumper, "\nto usage instruction:\n");
                        VIR_LOG_FLUSH(dumper);
                        VIR_Inst_Dump(dumper, use_inst);
                        VIR_LOG_FLUSH(dumper);
                    }
                    invalid_case = gcvTRUE;
                    break;
                }
            }
            /* there should be no def of mul_src0 and mul_src1 between mul and use_inst */
            {
                VIR_Instruction* next = VIR_Inst_GetNext(mul);
                while(next != use_inst)
                {
                    if(VIR_Operand_SameLocation(mul, mul_src0, next, VIR_Inst_GetDest(next))
                        || VIR_Operand_SameLocation(mul, mul_src1, next, VIR_Inst_GetDest(next)))
                    {
                        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD))
                        {
                            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                            VIR_LOG(dumper, "not processed because between mul and use_inst:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, use_inst);
                            VIR_LOG(dumper, "\nthis intruction redefs mul_src0/mul_src1:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, next);
                            VIR_LOG_FLUSH(dumper);
                        }
                        invalid_case = gcvTRUE;
                        break;
                    }
                    next = VIR_Inst_GetNext(next);
                }
            }

            vscHTBL_DirectSet(add_sub_set, (void*)_VSC_PH_Peephole_NewOpndTarget(ph, usage), gcvNULL);
        }

        if(invalid_case)
        {
            break;
        }
    }

    if(!invalid_case && HTBL_GET_ITEM_COUNT(add_sub_set))
    {
        VSC_HASH_TABLE* def_inst_set0 = vscHTBL_Create(VSC_PH_Peephole_GetMM(ph), vscHFUNC_Default, vscHKCMP_Default, 512);
        VSC_HASH_TABLE* def_inst_set1 = vscHTBL_Create(VSC_PH_Peephole_GetMM(ph), vscHFUNC_Default, vscHKCMP_Default, 512);
        VSC_HASH_ITERATOR iter;
        VSC_DIRECT_HNODE_PAIR pair;
        VIR_Swizzle mapping_swizzle0 = VIR_Enable_GetMappingSwizzle(mul_enable, mul_src0_swizzle);
        VIR_Swizzle mapping_swizzle1 = VIR_Enable_GetMappingSwizzle(mul_enable, mul_src1_swizzle);

        /* collect the def of the src0 and src1 of the mul inst, and delete the usage between mul's src0/src1's def and mul's src0/src1 */
        {
            VIR_GENERAL_UD_ITERATOR inst_ud_iter;
            VIR_DEF* def;

            vscVIR_InitGeneralUdIterator(&inst_ud_iter, VSC_PH_Peephole_GetDUInfo(ph), mul, mul_src0, gcvFALSE);
            for(def = vscVIR_GeneralUdIterator_First(&inst_ud_iter); def != gcvNULL;
                def = vscVIR_GeneralUdIterator_Next(&inst_ud_iter))
            {
                VIR_Instruction* def_inst = def->defKey.pDefInst;
                vscHTBL_DirectSet(def_inst_set0, (void*)def_inst, gcvNULL);
            }

            vscVIR_InitGeneralUdIterator(&inst_ud_iter, VSC_PH_Peephole_GetDUInfo(ph), mul, mul_src1, gcvFALSE);
            for(def = vscVIR_GeneralUdIterator_First(&inst_ud_iter); def != gcvNULL;
                def = vscVIR_GeneralUdIterator_Next(&inst_ud_iter))
            {
                VIR_Instruction* def_inst = def->defKey.pDefInst;
                vscHTBL_DirectSet(def_inst_set1, (void*)def_inst, gcvNULL);
            }
        }

        /* do the transformation */
        vscHTBLIterator_Init(&iter, add_sub_set);
        for(pair = vscHTBLIterator_DirectFirst(&iter);
        IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
        {
            VIR_USAGE* usage = (VIR_USAGE*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
            VIR_Instruction* add_sub = usage->usageKey.pUsageInst;
            VIR_Operand* other_src = gcvNULL;
            VIR_Operand* mul_dest_usage = usage->usageKey.pOperand;
            VIR_Swizzle mul_dest_usage_swizzle = VIR_Operand_GetSwizzle(mul_dest_usage);
            VIR_Swizzle mad_src0_swizzle = VIR_Swizzle_ApplyMappingSwizzle(mul_dest_usage_swizzle, mapping_swizzle0);
            VIR_Swizzle mad_src1_swizzle = VIR_Swizzle_ApplyMappingSwizzle(mul_dest_usage_swizzle, mapping_swizzle1);
            VIR_Enable mad_src0_enable = VIR_Swizzle_2_Enable(mad_src0_swizzle);
            VIR_Enable mad_src1_enable = VIR_Swizzle_2_Enable(mad_src1_swizzle);
            VIR_Enable mul_dest_usage_enable = VIR_Swizzle_2_Enable(mul_dest_usage_swizzle);
            VIR_Operand* mad_src0;
            VIR_Operand* mad_src1;

            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD))
            {
                VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                VIR_LOG(dumper, "merges with instruction:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, add_sub);
                VIR_LOG_FLUSH(dumper);
            }

            vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph), VIR_ANY_DEF_INST, add_sub,
                               mul_dest_usage, mul_dest_info.u1.virRegInfo.virReg, 1,
                               mul_dest_usage_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
            VIR_Function_DupOperand(func, mul_src0, &mad_src0);
            VIR_Function_DupOperand(func, mul_src1, &mad_src1);

            if(VIR_Inst_GetOpcode(add_sub) == VIR_OP_ADD ||
               VIR_Inst_GetOpcode(add_sub) == VIR_OP_ADDSAT)
            {
                if(mul_dest_usage == VIR_Inst_GetSource(add_sub, 0))
                {
                    other_src = VIR_Inst_GetSource(add_sub, 1);
                }
                else
                {
                    other_src = VIR_Inst_GetSource(add_sub, 0);
                }
            }
            if(VIR_Inst_GetOpcode(add_sub) == VIR_OP_SUB ||
               VIR_Inst_GetOpcode(add_sub) == VIR_OP_SUBSAT)
            {
                VIR_Operand* to_be_neg;

                if(mul_dest_usage == VIR_Inst_GetSource(add_sub, 0))
                {
                    other_src = VIR_Inst_GetSource(add_sub, 1);
                    to_be_neg = other_src;
                }
                else
                {
                    other_src = VIR_Inst_GetSource(add_sub, 0);

                    to_be_neg = VIR_Operand_GetOpKind(mad_src0) == VIR_OPND_IMMEDIATE ?
                        mad_src0 : VIR_Operand_GetOpKind(mad_src1) == VIR_OPND_IMMEDIATE ?
                        mad_src1 : gcvNULL;
                    if(!to_be_neg)
                    {
                        to_be_neg = VIR_Operand_GetOpKind(mad_src0) == VIR_OPND_CONST ?
                            mad_src0 : VIR_Operand_GetOpKind(mad_src1) == VIR_OPND_CONST ?
                            mad_src1 : gcvNULL;
                    }
                    if(!to_be_neg)
                    {
                        to_be_neg = VIR_Operand_GetOpKind(mad_src0) == VIR_OPND_SYMBOL ?
                            mad_src0 : VIR_Operand_GetOpKind(mad_src1) == VIR_OPND_SYMBOL ?
                            mad_src1 : gcvNULL;
                    }
                }
                gcmASSERT(VIR_Operand_IsNegatable(shader, to_be_neg));
                VIR_Operand_NegateOperand(shader, to_be_neg);
            }

            VIR_Operand_SetSwizzle(mad_src0, mad_src0_swizzle);
            VIR_Operand_SetSwizzle(mad_src1, mad_src1_swizzle);

            VIR_Inst_SetOpcode(add_sub, is_mulsat ? VIR_OP_MADSAT : VIR_OP_MAD);
            VIR_Inst_SetSource(add_sub, 0, mad_src0);
            VIR_Inst_SetSource(add_sub, 1, mad_src1);
            VIR_Inst_SetSource(add_sub, 2, other_src);
            VIR_Inst_SetSrcNum(add_sub, 3);

            /* add the use of mad_src0 and mad_src1 to the def of mul_src0 and mul_src1 */
            {
                VSC_HASH_ITERATOR def_inst_set_iter;
                VSC_DIRECT_HNODE_PAIR def_inst_set_pair;
                vscHTBLIterator_Init(&def_inst_set_iter, def_inst_set0);
                for(def_inst_set_pair = vscHTBLIterator_DirectFirst(&def_inst_set_iter);
                    IS_VALID_DIRECT_HNODE_PAIR(&def_inst_set_pair); def_inst_set_pair = vscHTBLIterator_DirectNext(&def_inst_set_iter))
                {
                    VIR_Instruction* def_inst = (VIR_Instruction*)VSC_DIRECT_HNODE_PAIR_FIRST(&def_inst_set_pair);
                    VIR_Enable def_dest_enable;
                    if(!VIR_IS_IMPLICIT_DEF_INST(def_inst))
                    {
                        VIR_Operand* def_dest = VIR_Inst_GetDest(def_inst);
                        def_dest_enable = VIR_Operand_GetEnable(def_dest);
                    }
                    else
                    {
                        def_dest_enable = VIR_ENABLE_XYZW;
                    }

                    if (def_dest_enable & mad_src0_enable)
                    {
                        vscVIR_AddNewUsageToDef(VSC_PH_Peephole_GetDUInfo(ph), def_inst, add_sub, mad_src0,
                                                mul_src0_info.u1.virRegInfo.virReg, 1, (VIR_Enable)(def_dest_enable & mad_src0_enable),
                                                VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
                    }
                }
                vscHTBLIterator_Init(&def_inst_set_iter, def_inst_set1);
                for(def_inst_set_pair = vscHTBLIterator_DirectFirst(&def_inst_set_iter);
                    IS_VALID_DIRECT_HNODE_PAIR(&def_inst_set_pair); def_inst_set_pair = vscHTBLIterator_DirectNext(&def_inst_set_iter))
                {
                    VIR_Instruction* def_inst = (VIR_Instruction*)VSC_DIRECT_HNODE_PAIR_FIRST(&def_inst_set_pair);
                    VIR_Enable def_dest_enable;
                    if(!VIR_IS_IMPLICIT_DEF_INST(def_inst))
                    {
                        VIR_Operand* def_dest = VIR_Inst_GetDest(def_inst);
                        def_dest_enable = VIR_Operand_GetEnable(def_dest);
                    }
                    else
                    {
                        def_dest_enable = VIR_ENABLE_XYZW;
                    }

                    if (def_dest_enable & mad_src1_enable)
                    {
                        vscVIR_AddNewUsageToDef(VSC_PH_Peephole_GetDUInfo(ph), def_inst, add_sub, mad_src1,
                                                mul_src1_info.u1.virRegInfo.virReg, 1,
                                                (VIR_Enable)(def_dest_enable & mad_src1_enable),
                                                VIR_HALF_CHANNEL_MASK_FULL,
                                                gcvNULL);
                    }
                }
            }
            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD))
            {
                VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                VIR_LOG(dumper, "into:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, add_sub);
                VIR_LOG_FLUSH(dumper);
            }
        }
        if(generated != gcvNULL)
        {
            *generated = gcvTRUE;
        }

        /* remove the use of mul_src0 and mul_src1 */
        {
            vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph), VIR_ANY_DEF_INST, mul,
                               mul_src0, mul_src0_info.u1.virRegInfo.virReg, 1,
                               mul_src0_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
            vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph), VIR_ANY_DEF_INST, mul,
                               mul_src1, mul_src1_info.u1.virRegInfo.virReg, 1,
                               mul_src1_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        }
        /* remove the def of mul */
        {
            vscVIR_DeleteDef(VSC_PH_Peephole_GetDUInfo(ph), mul, mul_dest_info.u1.virRegInfo.virReg,
                             1, mul_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        }
        VIR_Function_RemoveInstruction(func, mul);
        vscHTBL_Destroy(def_inst_set0);
        vscHTBL_Destroy(def_inst_set1);
    }
    else
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD)
            && !invalid_case && HTBL_GET_ITEM_COUNT(add_sub_set) == 0)
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "is prevented from being merging because there is no following add/sub instruction.\n");
            VIR_LOG_FLUSH(dumper);
        }
        if(generated != gcvNULL)
        {
            *generated = gcvFALSE;
        }
    }

    vscHTBL_Destroy(add_sub_set);
    return errCode;
}

static VSC_ErrCode _VSC_PH_GenerateRSQ(
    IN OUT VSC_PH_Peephole* ph,
    IN VIR_Instruction* sqrt,
    OUT gctBOOL* generated
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_Shader* shader = VSC_PH_Peephole_GetShader(ph);
    VIR_Function* func = VIR_Shader_GetCurrentFunction(shader);
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VIR_Swizzle sqrt_src0_swizzle;
    VIR_Enable sqrt_enable, sqrt_src0_enable;
    VIR_GENERAL_DU_ITERATOR du_iter;
    VIR_Operand *sqrt_dest, *sqrt_src0;
    VIR_OperandInfo sqrt_dest_info, sqrt_src0_info;
    gctUINT8 channel;
    VSC_HASH_TABLE* rcp_set;
    gctBOOL invalid_case = gcvFALSE;

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_RSQ))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        VIR_LOG(dumper, "\nSQRT instruction:\n");
        VIR_LOG_FLUSH(dumper);
        VIR_Inst_Dump(dumper, sqrt);
        VIR_LOG_FLUSH(dumper);
    }

    sqrt_dest = VIR_Inst_GetDest(sqrt);
    if(VIR_Operand_GetModifier(sqrt_dest))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_RSQ))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "cannot be processed because its dest has modifier.\n");
            VIR_LOG_FLUSH(dumper);
        }
        if(generated != gcvNULL)
        {
            *generated = gcvFALSE;
        }
        return errCode;
    }

    VIR_Operand_GetOperandInfo(sqrt, sqrt_dest, &sqrt_dest_info);
    gcmASSERT(~sqrt_dest_info.isTempVar && ~sqrt_dest_info.isVecConst);

    /* the result of sqrt can not be an output */
    if (sqrt_dest_info.isOutput)
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_RSQ))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "not processed because its result is an output.\n");
            VIR_LOG_FLUSH(dumper);
        }
        if(generated != gcvNULL)
        {
            *generated = gcvFALSE;
        }
        return errCode;
    }

    sqrt_enable = VIR_Operand_GetEnable(sqrt_dest);
    sqrt_src0 = VIR_Inst_GetSource(sqrt, 0);
    VIR_Operand_GetOperandInfo(sqrt, sqrt_src0, &sqrt_src0_info);
    sqrt_src0_swizzle = VIR_Operand_GetSwizzle(sqrt_src0);
    sqrt_src0_enable = VIR_Swizzle_2_Enable(sqrt_src0_swizzle);

    rcp_set = vscHTBL_Create(VSC_PH_Peephole_GetMM(ph), _VSC_PH_OpndTarget_HFUNC, _VSC_PH_OpndTarget_HKCMP, 512);
    /* get usage of sqrt's dest in per channel way*/
    for(channel = 0; channel < VIR_CHANNEL_NUM; channel++)
    {
        VIR_USAGE* usage;
        if(!(sqrt_enable & (1 << channel)))
        {
            continue;
        }

        /*  */
        vscVIR_InitGeneralDuIterator(&du_iter, VSC_PH_Peephole_GetDUInfo(ph), sqrt, sqrt_dest_info.u1.virRegInfo.virReg, channel, gcvFALSE);
        for(usage = vscVIR_GeneralDuIterator_First(&du_iter); usage != gcvNULL;
            usage = vscVIR_GeneralDuIterator_Next(&du_iter))
        {
            VIR_Instruction* use_inst;
            VIR_Operand* use_inst_dest;
            VIR_Operand* use_inst_opnd;

            if(vscHTBL_DirectTestAndGet(rcp_set, (void*)&(usage->usageKey), gcvNULL))
            {
                continue;
            }

            use_inst = usage->usageKey.pUsageInst;
            use_inst_opnd = usage->usageKey.pOperand;
            /* the result of sqrt must be used in rcp */
            if(VIR_Inst_GetOpcode(use_inst) != VIR_OP_RCP)
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_RSQ))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "not processed because of its usage in:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, use_inst);
                    VIR_LOG(dumper, "\nis not rcp\n");
                    VIR_LOG_FLUSH(dumper);
                }
                invalid_case = gcvTRUE;
                break;
            }

            /* use_inst should not have different rounding mode from the sqrt instruction */
            use_inst_dest = VIR_Inst_GetDest(use_inst);
            if(VIR_Operand_GetRoundMode(use_inst_dest) != VIR_Operand_GetRoundMode(sqrt_dest))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_RSQ))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "not processed because of its usage in:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, use_inst);
                    VIR_LOG(dumper, "\nhas different rounding mode\n");
                    VIR_LOG_FLUSH(dumper);
                }
                invalid_case = gcvTRUE;
                break;
            }

            /* use_inst_opnd should not have modifiers. could be improved here */
            if(VIR_Operand_GetModifier(use_inst_opnd))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_RSQ))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "not processed because of its usage in:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, use_inst);
                    VIR_LOG(dumper, "\nhas modifier\n");
                    VIR_LOG_FLUSH(dumper);
                }
                invalid_case = gcvTRUE;
                break;
            }

            /* sqrt and rcp should be in the same bb */
            if(VIR_Inst_GetBasicBlock(sqrt) != VIR_Inst_GetBasicBlock(use_inst))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_RSQ))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "not processed because of the following usage and sqrt are not in the same bb:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, use_inst);
                    VIR_LOG_FLUSH(dumper);
                }
                invalid_case = gcvTRUE;
                break;
            }

            /* dest of sqrt should cover its usage in rcp */
            {
                VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(use_inst_opnd);
                VIR_Enable rcp_enable = VIR_Swizzle_2_Enable(swizzle);
                if(!VIR_Enable_Covers(sqrt_enable, rcp_enable))
                {
                    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_RSQ))
                    {
                        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                        VIR_LOG(dumper, "not processed because of the following usage is not covered:\n");
                        VIR_LOG_FLUSH(dumper);
                        VIR_Inst_Dump(dumper, use_inst);
                        VIR_LOG_FLUSH(dumper);
                    }
                    invalid_case = gcvTRUE;
                    break;
                }
            }

            /* in case of loop, sqrt should be the only def of rcp usage. */
            {
                VIR_Instruction* breaking_def;
                if(!vscVIR_IsUniqueDefInstOfUsageInst(VSC_PH_Peephole_GetDUInfo(ph), use_inst, use_inst_opnd, sqrt, &breaking_def))
                {
                    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
                    {
                        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                        VIR_LOG(dumper, "prevented by another def instruction:\n");
                        VIR_LOG_FLUSH(dumper);
                        VIR_Inst_Dump(dumper, breaking_def);
                        VIR_LOG(dumper, "\nto usage instruction:\n");
                        VIR_LOG_FLUSH(dumper);
                        VIR_Inst_Dump(dumper, use_inst);
                        VIR_LOG_FLUSH(dumper);
                    }
                    invalid_case = gcvTRUE;
                    break;
                }
            }
            /* there should be no def of sqrt_src0 between sqrt and use_inst */
            {
                VIR_Instruction* next = VIR_Inst_GetNext(sqrt);
                while(next != use_inst)
                {
                    if(VIR_Operand_SameLocation(sqrt, sqrt_src0, next, VIR_Inst_GetDest(next)))
                    {
                        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_RSQ))
                        {
                            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                            VIR_LOG(dumper, "not processed because between sqrt and use_inst:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, use_inst);
                            VIR_LOG(dumper, "\nthis intruction redefs sqrt_src0:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, next);
                            VIR_LOG_FLUSH(dumper);
                        }
                        invalid_case = gcvTRUE;
                        break;
                    }
                    next = VIR_Inst_GetNext(next);
                }
            }

            vscHTBL_DirectSet(rcp_set, (void*)_VSC_PH_Peephole_NewOpndTarget(ph, usage), gcvNULL);
        }

        if(invalid_case)
        {
            break;
        }
    }

    if(!invalid_case && HTBL_GET_ITEM_COUNT(rcp_set))
    {
        VSC_HASH_TABLE* def_inst_set0 = vscHTBL_Create(VSC_PH_Peephole_GetMM(ph), vscHFUNC_Default, vscHKCMP_Default, 512);
        VSC_HASH_ITERATOR iter;
        VSC_DIRECT_HNODE_PAIR pair;
        VIR_Swizzle mapping_swizzle0 = VIR_Enable_GetMappingSwizzle(sqrt_enable, sqrt_src0_swizzle);

        /* collect the def of the src0 of the sqrt inst, and delete the usage between sqrt's src0's def and sqrt's src0 */
        {
            VIR_GENERAL_UD_ITERATOR inst_ud_iter;
            VIR_DEF* def;

            vscVIR_InitGeneralUdIterator(&inst_ud_iter, VSC_PH_Peephole_GetDUInfo(ph), sqrt, sqrt_src0, gcvFALSE);
            for(def = vscVIR_GeneralUdIterator_First(&inst_ud_iter); def != gcvNULL;
                def = vscVIR_GeneralUdIterator_Next(&inst_ud_iter))
            {
                VIR_Instruction* def_inst = def->defKey.pDefInst;
                vscHTBL_DirectSet(def_inst_set0, (void*)def_inst, gcvNULL);
            }
        }

        /* do the transformation */
        vscHTBLIterator_Init(&iter, rcp_set);
        for(pair = vscHTBLIterator_DirectFirst(&iter);
            IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
        {
            VIR_USAGE* usage = (VIR_USAGE*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
            VIR_Instruction* rcp = usage->usageKey.pUsageInst;
            VIR_Operand* sqrt_dest_usage = usage->usageKey.pOperand;
            VIR_Swizzle sqrt_dest_usage_swizzle = VIR_Operand_GetSwizzle(sqrt_dest_usage);
            VIR_Swizzle rsq_src0_swizzle = VIR_Swizzle_ApplyMappingSwizzle(sqrt_dest_usage_swizzle, mapping_swizzle0);
            VIR_Enable rsq_src0_enable = VIR_Swizzle_2_Enable(rsq_src0_swizzle);
            VIR_Enable sqrt_dest_usage_enable = VIR_Swizzle_2_Enable(sqrt_dest_usage_swizzle);
            VIR_Operand* rsq_src0;

            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_RSQ))
            {
                VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                VIR_LOG(dumper, "merges with instruction:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, rcp);
                VIR_LOG_FLUSH(dumper);
            }

            vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph), VIR_ANY_DEF_INST, rcp,
                               sqrt_dest_usage, sqrt_dest_info.u1.virRegInfo.virReg, 1,
                               sqrt_dest_usage_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
            VIR_Function_DupOperand(func, sqrt_src0, &rsq_src0);
            VIR_Operand_SetSwizzle(rsq_src0, rsq_src0_swizzle);

            VIR_Inst_SetOpcode(rcp, VIR_OP_RSQ);
            VIR_Inst_SetSource(rcp, 0, rsq_src0);
            VIR_Inst_SetSrcNum(rcp, 1);

            /* add the use of rsq_src0 to the def of sqrt_src0 */
            {
                VSC_HASH_ITERATOR def_inst_set_iter;
                VSC_DIRECT_HNODE_PAIR def_inst_set_pair;
                vscHTBLIterator_Init(&def_inst_set_iter, def_inst_set0);
                for(def_inst_set_pair = vscHTBLIterator_DirectFirst(&def_inst_set_iter);
                    IS_VALID_DIRECT_HNODE_PAIR(&def_inst_set_pair); def_inst_set_pair = vscHTBLIterator_DirectNext(&def_inst_set_iter))
                {
                    VIR_Instruction* def_inst = (VIR_Instruction*)VSC_DIRECT_HNODE_PAIR_FIRST(&def_inst_set_pair);
                    VIR_Enable def_dest_enable;
                    if(!VIR_IS_IMPLICIT_DEF_INST(def_inst))
                    {
                        VIR_Operand* def_dest = VIR_Inst_GetDest(def_inst);
                        def_dest_enable = VIR_Operand_GetEnable(def_dest);
                    }
                    else
                    {
                        def_dest_enable = VIR_ENABLE_XYZW;
                    }

                    if (def_dest_enable & rsq_src0_enable)
                    {
                        vscVIR_AddNewUsageToDef(VSC_PH_Peephole_GetDUInfo(ph), def_inst, rcp, rsq_src0,
                                                sqrt_src0_info.u1.virRegInfo.virReg, 1, (VIR_Enable)(def_dest_enable & rsq_src0_enable),
                                                VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
                    }
                }
            }
            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_RSQ))
            {
                VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                VIR_LOG(dumper, "into:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, rcp);
                VIR_LOG_FLUSH(dumper);
            }
        }
        if(generated != gcvNULL)
        {
            *generated = gcvTRUE;
        }

        /* remove the use of sqrt_src0 */
        {
            vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph), VIR_ANY_DEF_INST, sqrt,
                               sqrt_src0, sqrt_src0_info.u1.virRegInfo.virReg, 1,
                               sqrt_src0_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        }
        /* remove the def of sqrt */
        {
            vscVIR_DeleteDef(VSC_PH_Peephole_GetDUInfo(ph), sqrt, sqrt_dest_info.u1.virRegInfo.virReg,
                             1, sqrt_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        }
        VIR_Function_RemoveInstruction(func, sqrt);
        vscHTBL_Destroy(def_inst_set0);
    }
    else
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_RSQ)
            && !invalid_case && HTBL_GET_ITEM_COUNT(rcp_set) == 0)
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "is prevented from being merging because there is no following rcp instruction.\n");
            VIR_LOG_FLUSH(dumper);
        }
        if(generated != gcvNULL)
        {
            *generated = gcvFALSE;
        }
    }

    vscHTBL_Destroy(rcp_set);
    return errCode;
}

static VSC_ErrCode _VSC_PH_RecordUsages(
    IN VSC_PH_Peephole      *ph,
    IN VIR_Instruction      *inst,
    IN OUT VSC_HASH_TABLE   *usages_set
    )
{
    VSC_ErrCode     errCode  = VSC_ERR_NONE;
    gctUINT8        channel;
    VIR_Operand     *inst_dst = VIR_Inst_GetDest(inst);
    VIR_Enable      inst_enable =  VIR_Operand_GetEnable(inst_dst);
    VIR_OperandInfo inst_dst_info;

    VIR_GENERAL_DU_ITERATOR du_iter;
    VIR_USAGE       *pUsage;

    VIR_Operand_GetOperandInfo(inst, inst_dst, &inst_dst_info);

    /* get usage of MUL's dest in per channel way*/
    for(channel = 0; channel < VIR_CHANNEL_NUM; channel++)
    {
        if(!(inst_enable & (1 << channel)))
        {
            continue;
        }

        vscVIR_InitGeneralDuIterator(&du_iter, VSC_PH_Peephole_GetDUInfo(ph),
            inst,
            inst_dst_info.u1.virRegInfo.virReg,
            channel,
            gcvFALSE);
        for(pUsage = vscVIR_GeneralDuIterator_First(&du_iter); pUsage != gcvNULL;
            pUsage = vscVIR_GeneralDuIterator_Next(&du_iter))
        {
            if(vscHTBL_DirectTestAndGet(usages_set, (void*)&(pUsage->usageKey), gcvNULL))
            {
                continue;
            }

            vscHTBL_DirectSet(usages_set,
                (void*)_VSC_PH_Peephole_NewOpndTarget(ph, pUsage),
                gcvNULL);
        }
    }

    return errCode;
}

static VSC_ErrCode _VSC_PH_ReplaceUsages(
    IN OUT VSC_PH_Peephole  *ph,
    IN VIR_Instruction      *merged_inst,
    IN VIR_Swizzle          mapping_swizzle,
    IN VSC_HASH_TABLE       *usages_set
    )
{
    /* change the usage of mova_dst to use first_def_mova_dst */
    VSC_ErrCode     errCode  = VSC_ERR_NONE;

    VIR_Shader      *shader = VSC_PH_Peephole_GetShader(ph);
    VIR_Function    *func = VIR_Shader_GetCurrentFunction(shader);
    VIR_Dumper      *dumper = VSC_PH_Peephole_GetDumper(ph);
    VSC_OPTN_PHOptions *options = VSC_PH_Peephole_GetOptions(ph);

    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;

    VIR_Operand     *merged_inst_dst = VIR_Inst_GetDest(merged_inst);
    VIR_OperandInfo merged_inst_dst_info;
    VIR_Operand_GetOperandInfo(merged_inst, merged_inst_dst, &merged_inst_dst_info);

    vscHTBLIterator_Init(&iter, usages_set);
    for(pair = vscHTBLIterator_DirectFirst(&iter);
        IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VIR_USAGE* pUsage = (VIR_USAGE*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        VIR_Instruction *use_inst = pUsage->usageKey.pUsageInst;
        VIR_Operand     *use_opnd = pUsage->usageKey.pOperand;
        VIR_Swizzle     use_swizzle = VIR_Operand_GetSwizzle(use_opnd);
        VIR_Enable      use_enable = VIR_Swizzle_2_Enable(use_swizzle);
        VIR_OperandInfo use_opnd_info;
        VIR_Operand     *new_src;
        gctUINT         srcIndex = VIR_Inst_GetSourceIndex(use_inst, use_opnd);
        if (srcIndex >= VIR_MAX_SRC_NUM)
        {
            continue;
        }

        VIR_Operand_GetOperandInfo(use_inst, use_opnd, &use_opnd_info);

        vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph),
                VIR_ANY_DEF_INST,
                use_inst,
                use_opnd,
                use_opnd_info.u1.virRegInfo.virReg,
                1,
                use_enable,
                VIR_HALF_CHANNEL_MASK_FULL,
                gcvNULL);

        /* duplicate merged_inst_dst */
        VIR_Function_DupOperand(func, merged_inst_dst, &new_src);
        VIR_Operand_SetLvalue(new_src, 0);
        VIR_Operand_SetSwizzle(new_src,
            VIR_Swizzle_ApplyMappingSwizzle(use_swizzle, mapping_swizzle));
        VIR_Inst_SetSource(use_inst, srcIndex, new_src);

        vscVIR_AddNewUsageToDef(VSC_PH_Peephole_GetDUInfo(ph),
            merged_inst,
            use_inst,
            new_src,
            merged_inst_dst_info.u1.virRegInfo.virReg,
            1,
            VIR_Swizzle_2_Enable(VIR_Swizzle_ApplyMappingSwizzle(use_swizzle, mapping_swizzle)),
            VIR_HALF_CHANNEL_MASK_FULL,
            gcvNULL);

        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_VEC))
        {
            VIR_LOG(dumper, "==> change its usage to :\n");
            VIR_Inst_Dump(dumper, use_inst);
            VIR_LOG_FLUSH(dumper);
        }
    }

    return errCode;
}

static gctBOOL  _VSC_PH_VEC_mergable(
    VIR_OpCode opcode
    )
{
    if ((opcode == VIR_OP_MOVA) ||
        (opcode == VIR_OP_MUL))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static VSC_ErrCode _VSC_PH_VEC_MergeInst(
    IN OUT VSC_PH_Peephole  *ph,
    IN VIR_Instruction      *inst,
    VSC_HASH_TABLE          *vec_def_set,
    VSC_PH_MergeKey         *curr_mova
    )
{
    VSC_ErrCode     errCode  = VSC_ERR_NONE;
    VIR_Shader      *shader = VSC_PH_Peephole_GetShader(ph);
    VIR_Function    *func = VIR_Shader_GetCurrentFunction(shader);
    VIR_Dumper      *dumper = VSC_PH_Peephole_GetDumper(ph);
    VSC_OPTN_PHOptions *options = VSC_PH_Peephole_GetOptions(ph);

    VIR_Operand     *inst_dst, *inst_src0;
    VIR_OperandInfo inst_dst_info, inst_src0_info;
    VIR_Enable      inst_enable, inst_src0_enable;
    VIR_Swizzle     inst_src0_swizzle;
    VIR_OpCode      inst_opcode = VIR_Inst_GetOpcode(inst);
    gctUINT         inst_src1_immediate = 0;

    VIR_GENERAL_UD_ITERATOR ud_iter;
    VIR_DEF         *pDef;
    gctUINT         defCount = 0;

    VSC_PH_MergedInst   *merged_inst = gcvNULL;
    gctBOOL             found_diff = gcvFALSE;
    VSC_PH_MergeKey     *mergeKey = gcvNULL;

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_VEC))
    {
        VIR_LOG(dumper, "\nInstruction:");
        VIR_LOG_FLUSH(dumper);
        VIR_Inst_Dump(dumper, inst);
        VIR_LOG_FLUSH(dumper);
    }

    inst_dst = VIR_Inst_GetDest(inst);
    gcmASSERT(inst_dst);
    inst_enable = VIR_Operand_GetEnable(inst_dst);
    VIR_Operand_GetOperandInfo(inst, inst_dst, &inst_dst_info);

    inst_src0 = VIR_Inst_GetSource(inst, 0);
    gcmASSERT(inst_src0);
    VIR_Operand_GetOperandInfo(inst, inst_src0, &inst_src0_info);
    inst_src0_swizzle = VIR_Operand_GetSwizzle(inst_src0);
    inst_src0_enable = VIR_Swizzle_2_Enable(inst_src0_swizzle);

    if (VIR_Inst_GetSrcNum(inst) == 2)
    {
        /* src1 must be an immediate */
        VIR_Operand     *inst_src1;
        VIR_OperandInfo inst_src1_info;

        inst_src1 = VIR_Inst_GetSource(inst, 1);
        gcmASSERT(inst_src1);
        VIR_Operand_GetOperandInfo(inst, inst_src1, &inst_src1_info);
        if (inst_src1_info.isImmVal &&
            VIR_Operand_GetType(inst_src1) == VIR_TYPE_INT32)
        {
            inst_src1_immediate = inst_src1_info.u1.immValue.uValue;
            /* hash table only use 4 bits of the inst_src1_immediate as hash value,
               can be adjust later */
            if (inst_src1_immediate >= 16)
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_VEC))
                {
                    VIR_LOG(dumper, "==> bail out, because src1 is a too large const.\n");
                    VIR_LOG_FLUSH(dumper);
                }
                return errCode;
            }
        }
        else
        {
            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_VEC))
            {
                VIR_LOG(dumper, "==> bail out, because src1 is not int const.\n");
                VIR_LOG_FLUSH(dumper);
            }
            return errCode;
        }
    }
    else if (VIR_Inst_GetSrcNum(inst) == 3)
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_VEC))
        {
            VIR_LOG(dumper, "==> bail out, because of src2.\n");
            VIR_LOG_FLUSH(dumper);
        }
        return errCode;
    }

    /* get def of inst's src */
    vscVIR_InitGeneralUdIterator(&ud_iter, VSC_PH_Peephole_GetDUInfo(ph), inst, inst_src0, gcvFALSE);
    for (pDef = vscVIR_GeneralUdIterator_First(&ud_iter); pDef != gcvNULL;
         pDef = vscVIR_GeneralUdIterator_Next(&ud_iter))
    {
        VSC_PH_MergedInst   *currInst = gcvNULL;
        mergeKey = _HKCMP_NewMergeKey(ph, &pDef->defKey, inst_opcode, inst_src1_immediate);

        /* only consider one def for now */
        if (++defCount > 1)
        {
            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_VEC))
            {
                VIR_LOG(dumper, "==> bail out, because multiple def.\n");
                VIR_LOG_FLUSH(dumper);
            }
            return errCode;
        }

        if (inst_opcode != VIR_OP_MOVA)
        {
            if(!vscHTBL_DirectTestAndGet(vec_def_set, (void*)mergeKey, (void **)&currInst))
            {
                currInst = _VSC_PH_Peephole_NewMergedInst(ph, inst, pDef->defKey.channel);
                vscHTBL_DirectSet(vec_def_set, (void*)mergeKey, (void*) currInst);
                if (merged_inst == gcvNULL)
                {
                    merged_inst = currInst;
                }
            }
            else
            {
                if (merged_inst == gcvNULL)
                {
                    merged_inst = currInst;
                }
                else
                {
                    if (merged_inst != currInst)
                    {
                        found_diff = gcvTRUE;
                        break;
                    }
                }
                currInst->enable = currInst->enable | (1 << pDef->defKey.channel);
            }
        }
        else
        {
            /* mova's dst is a0, thus every mova is a redefine,
               only need to keep the current */
            if (!_HKCMP_MergeKeyEqual((void*)mergeKey, (void*) curr_mova))
            {
                curr_mova->defKey = mergeKey->defKey;
                if (merged_inst == gcvNULL)
                {
                    currInst = _VSC_PH_Peephole_NewMergedInst(ph, inst, pDef->defKey.channel);
                    vscHTBL_DirectSet(vec_def_set, (void*)(curr_mova), (void*) currInst);
                    merged_inst = currInst;
                }
                found_diff = gcvFALSE;
            }
            else
            {
                vscHTBL_DirectTestAndGet(vec_def_set, (void*)(curr_mova), (void **)&currInst);
                if (merged_inst == gcvNULL)
                {
                    merged_inst = currInst;
                }
                else
                {
                    if (merged_inst != currInst)
                    {
                        found_diff = gcvFALSE;
                        break;
                    }
                }
                currInst->enable = currInst->enable | (1 << pDef->defKey.channel);
            }
        }
    }

    if (!found_diff)
    {
        if (merged_inst && merged_inst->inst != inst)
        {
            /* do the transformation */
            VIR_Operand *merged_inst_dst = VIR_Inst_GetDest(merged_inst->inst);
            VIR_Operand *merged_inst_src = VIR_Inst_GetSource(merged_inst->inst, 0);
            VIR_Swizzle merged_inst_src_swizzle = VIR_Operand_GetSwizzle(merged_inst_src);
            VIR_Enable  merged_inst_src_enable = VIR_Swizzle_2_Enable(merged_inst_src_swizzle);
            VIR_Enable merged_inst_dst_enable = VIR_Operand_GetEnable(merged_inst_dst);

            VIR_Swizzle channelMapping = VIR_SWIZZLE_XYZW,
                        instDst2SrcMapping = VIR_SWIZZLE_XYZW,
                        mergedSrc2DstMapping = VIR_SWIZZLE_XYZW;

            VIR_OperandInfo merged_inst_dst_info, merged_inst_src_info;
            VIR_Operand *new_dst = gcvNULL;
            VSC_HASH_TABLE* usages_set;

            VIR_Operand_GetOperandInfo(merged_inst->inst, merged_inst_dst, &merged_inst_dst_info);
            VIR_Operand_GetOperandInfo(merged_inst->inst, merged_inst_src, &merged_inst_src_info);

            /*  merge instruction, for example
                mova t1.x, t2.x (merged_inst->inst)
                ...             (no redefine of t2.y in between, since
                                 t2.x and t2.y come from the same mergeKey.defKey.pDefInst)
                mova t1.y, t2.y (inst)

                ==>
                mova t1.xy t2.xy (merged_inst->inst)
            */
            if (merged_inst_src_enable != merged_inst->enable)
            {
                usages_set = vscHTBL_Create(VSC_PH_Peephole_GetMM(ph),
                    _VSC_PH_OpndTarget_HFUNC,
                    _VSC_PH_OpndTarget_HKCMP, 512);

                _VSC_PH_RecordUsages(ph, merged_inst->inst, usages_set);

                channelMapping = VIR_Enable_GetMappingSwizzle(VIR_Operand_GetEnable(VIR_Inst_GetDest(merged_inst->inst)),
                                                              VIR_Operand_GetSwizzle(VIR_Inst_GetSource(merged_inst->inst, 0)));

                VIR_Function_DupOperand(func, merged_inst_dst, &new_dst);
                VIR_Operand_SetEnable(merged_inst_dst, merged_inst->enable);

                /* generates the new dest with the new enable */
                vscVIR_DeleteDef(VSC_PH_Peephole_GetDUInfo(ph),
                    merged_inst->inst,
                    merged_inst_dst_info.u1.virRegInfo.virReg,
                    1,
                    merged_inst_src_enable,
                    VIR_HALF_CHANNEL_MASK_FULL,
                    gcvNULL);
                VIR_Operand_ReplaceDefOperandWithDef(
                    merged_inst_dst,
                    new_dst,
                    merged_inst->enable);
                vscVIR_AddNewDef(VSC_PH_Peephole_GetDUInfo(ph),
                    merged_inst->inst,
                    merged_inst_dst_info.u1.virRegInfo.virReg,
                    1,
                    merged_inst->enable,
                    VIR_HALF_CHANNEL_MASK_FULL,
                    gcvFALSE, /* isInput ? */
                    gcvFALSE, /* isOutput ? */
                    gcvNULL);

                VIR_Operand_SetSwizzle(merged_inst_src,
                    VIR_Enable_2_Swizzle_WShift(merged_inst->enable));

                /* update the du info */
                vscVIR_AddNewUsageToDef(VSC_PH_Peephole_GetDUInfo(ph),
                            mergeKey->defKey->pDefInst,
                            merged_inst->inst,
                            merged_inst_src,
                            merged_inst_src_info.u1.virRegInfo.virReg,
                            1,
                            merged_inst->enable,
                            VIR_HALF_CHANNEL_MASK_FULL,
                            gcvNULL);

                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_VEC))
                {
                    VIR_LOG(dumper, "==> merged instruction:");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, merged_inst->inst);
                    VIR_LOG_FLUSH(dumper);
                }

                _VSC_PH_ReplaceUsages(ph, merged_inst->inst, channelMapping, usages_set);

                vscHTBL_Destroy(usages_set);
            }

            /* delete the usage of inst_src */
            vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph),
                VIR_ANY_DEF_INST,
                inst,
                inst_src0,
                inst_src0_info.u1.virRegInfo.virReg,
                1,
                inst_src0_enable,
                VIR_HALF_CHANNEL_MASK_FULL,
                gcvNULL);

            /* get the swizzle from the merged_inst */
            instDst2SrcMapping = VIR_Enable_GetMappingSwizzle(VIR_Operand_GetEnable(VIR_Inst_GetDest(inst)), VIR_Operand_GetSwizzle(VIR_Inst_GetSource(inst, 0)));
            VIR_Swizzle_GetMappingSwizzle2Enable(merged_inst_src_swizzle, merged_inst_dst_enable, &mergedSrc2DstMapping);
            channelMapping = VIR_Swizzle_MergeMappingSwizzles(instDst2SrcMapping, mergedSrc2DstMapping);

            /* change the usage of mova_dst to use first_def_mova_dst */
            usages_set = vscHTBL_Create(VSC_PH_Peephole_GetMM(ph),
                    _VSC_PH_OpndTarget_HFUNC,
                    _VSC_PH_OpndTarget_HKCMP, 512);

            _VSC_PH_RecordUsages(ph, inst, usages_set);
            _VSC_PH_ReplaceUsages(ph, merged_inst->inst, channelMapping, usages_set);
            vscHTBL_Destroy(usages_set);

            /* delete the def of mova_dst */
            vscVIR_DeleteDef(VSC_PH_Peephole_GetDUInfo(ph),
                inst,
                inst_dst_info.u1.virRegInfo.virReg,
                1,
                inst_enable,
                VIR_HALF_CHANNEL_MASK_FULL,
                gcvNULL);

            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_VEC))
            {
                VIR_LOG(dumper, "==> redundant instruction removed:");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, inst);
                VIR_LOG(dumper, "\n==> use the previous instruction:");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, merged_inst->inst);
                VIR_LOG_FLUSH(dumper);
            }

            /* delete the inst instruction */
            VIR_Function_RemoveInstruction(func, inst);
        }
        else
        {
            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_VEC))
            {
                VIR_LOG(dumper, "first inst: not redundant\n");
                VIR_LOG_FLUSH(dumper);
            }
        }
    }
    else
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_VEC))
        {
            VIR_LOG(dumper, "not the same def: not redundant\n");
            VIR_LOG_FLUSH(dumper);
        }
    }

    if (mergeKey)
    {
        vscMM_Free(VSC_PH_Peephole_GetMM(ph), mergeKey);
    }
    return errCode;
}

static gctBOOL _VSC_PH_EvaluateChecking(
    VIR_Shader      *pShader,
    VIR_Instruction *inst,
    gctBOOL         *checkingResult)
{
    VIR_Operand     *src0, *src1;
    VIR_TypeId      src0Type, src1Type;
    VIR_OperandInfo  src0Info, src1Info;

    src0 = VIR_Inst_GetSource(inst, 0);
    src1 = VIR_Inst_GetSource(inst, 1);

    src0Type = VIR_Operand_GetType(src0);
    src1Type = VIR_Operand_GetType(src1);

    VIR_Operand_GetOperandInfo(inst, src0, &src0Info);
    VIR_Operand_GetOperandInfo(inst, src1, &src1Info);

    gcmASSERT(src0Info.isImmVal && src1Info.isImmVal);

    if (src0Type == VIR_TYPE_FLOAT32 ||
        src1Type == VIR_TYPE_FLOAT32)
    {
        float f0, f1;

        if (src0Type == VIR_TYPE_FLOAT32)
        {
            f0 = src0Info.u1.immValue.fValue;
        }
        else if (src0Type == VIR_TYPE_INT32)
        {
            f0 = (gctFLOAT)(src0Info.u1.immValue.iValue);
        }
        else
        {
            return gcvFALSE;
        }

        if (src1Type == VIR_TYPE_FLOAT32)
        {
            f1 = src1Info.u1.immValue.fValue;
        }
        else if (src1Type == VIR_TYPE_INT32)
        {
            f1 = (gctFLOAT) src1Info.u1.immValue.iValue;
        }
        else
        {
            /* Error. */
            return gcvFALSE;
        }

        switch (VIR_Inst_GetConditionOp(inst))
        {
        case VIR_COP_ALWAYS:
            /* Error. */ return gcvFALSE;
        case VIR_COP_NOT_EQUAL:
            *checkingResult = (f0 != f1); break;
        case VIR_COP_LESS_OR_EQUAL:
            *checkingResult = (f0 <= f1); break;
        case VIR_COP_LESS:
            *checkingResult = (f0 < f1); break;
        case VIR_COP_EQUAL:
            *checkingResult = (f0 == f1); break;
        case VIR_COP_GREATER:
            *checkingResult = (f0 > f1); break;
        case VIR_COP_GREATER_OR_EQUAL:
            *checkingResult = (f0 >= f1); break;
        case VIR_COP_AND:
        case VIR_COP_OR:
        case VIR_COP_XOR:
            /* TODO - Error. */
            return gcvFALSE;
        case VIR_COP_NOT_ZERO:
            *checkingResult = (f0 != 0.0f); break;
        default:
            return gcvFALSE;
        }
    }
    else
    {
        gctUINT32 value0 = src0Info.u1.immValue.uValue;
        gctUINT32 value1 = src1Info.u1.immValue.uValue;

        gctINT32 i0 = src0Info.u1.immValue.iValue;
        gctINT32 i1 = src1Info.u1.immValue.iValue;

        switch (VIR_Inst_GetConditionOp(inst))
        {
        case VIR_COP_ALWAYS:
            /* Error. */ return gcvFALSE;
        case VIR_COP_NOT_EQUAL:
            *checkingResult = (value0 != value1); break;
        case VIR_COP_LESS_OR_EQUAL:
            if ((src0Type == VIR_TYPE_INT32 ||
                 src0Type == VIR_TYPE_INT16) &&
                (src1Type == VIR_TYPE_INT32 ||
                 src1Type == VIR_TYPE_INT16))
            {
                *checkingResult = (i0 <= i1);
            }
            else
            {
                *checkingResult = (value0 <= value1);
            }
            break;
        case VIR_COP_LESS:
            if ((src0Type == VIR_TYPE_INT32 ||
                 src0Type == VIR_TYPE_INT16) &&
                (src1Type == VIR_TYPE_INT32 ||
                 src1Type == VIR_TYPE_INT16))
            {
                *checkingResult = (i0 < i1);
            }
            else
            {
                *checkingResult = (value0 < value1);
            }
            break;
        case VIR_COP_EQUAL:
            *checkingResult = (value0 == value1); break;
        case VIR_COP_GREATER:
            if ((src0Type == VIR_TYPE_INT32 ||
                 src0Type == VIR_TYPE_INT16) &&
                (src1Type == VIR_TYPE_INT32 ||
                 src1Type == VIR_TYPE_INT16))
            {
                *checkingResult = (i0 > i1);
            }
            else
            {
                *checkingResult = (value0 > value1);
            }
            break;
        case VIR_COP_GREATER_OR_EQUAL:
            if ((src0Type == VIR_TYPE_INT32 ||
                 src0Type == VIR_TYPE_INT16) &&
                (src1Type == VIR_TYPE_INT32 ||
                 src1Type == VIR_TYPE_INT16))
            {
                *checkingResult = (i0 >= i1);
            }
            else
            {
                *checkingResult = (value0 >= value1);
            }
            break;
        case VIR_COP_AND:
            *checkingResult = (value0 & value1); break;
        case VIR_COP_OR:
            *checkingResult = (value0 | value1); break;
        case VIR_COP_XOR:
            *checkingResult = (value0 ^ value1); break;
        case VIR_COP_NOT_ZERO:
            *checkingResult = (value0 != 0); break;
        default:
            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

/* instruction src is uniquely defined and used */
static gctBOOL
_VSC_PH_SrcUniqueDef(
    IN OUT VSC_PH_Peephole  *ph,
    IN VIR_Instruction      *inst,
    IN VIR_Operand          *inst_src,
    OUT VIR_Instruction     **def_inst)
{
    VIR_GENERAL_UD_ITERATOR ud_iter;
    VIR_DEF                 *pDef;

    gcmASSERT(VIR_Operand_IsOwnerInst(inst_src, inst));

    vscVIR_InitGeneralUdIterator(&ud_iter, VSC_PH_Peephole_GetDUInfo(ph), inst, inst_src, gcvFALSE);
    for (pDef = vscVIR_GeneralUdIterator_First(&ud_iter); pDef != gcvNULL;
         pDef = vscVIR_GeneralUdIterator_Next(&ud_iter))
    {
        /* the def has to be the only def */
        VIR_Instruction *defInst = pDef->defKey.pDefInst;
        if (defInst == VIR_INPUT_DEF_INST)
        {
            break;
        }
        if (vscVIR_IsUniqueDefInstOfUsageInst(VSC_PH_Peephole_GetDUInfo(ph), inst, inst_src, defInst, gcvNULL) &&
            vscVIR_IsUniqueUsageInstOfDefInst(VSC_PH_Peephole_GetDUInfo(ph), defInst, inst, gcvNULL))
        {
            *def_inst = defInst;
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static gctBOOL
_VSC_PH_SrcFromUniqueLDARR(
    IN OUT VSC_PH_Peephole  *ph,
    IN VIR_Instruction      *inst,
    OUT VIR_Operand         **inst_src,
    OUT VIR_Instruction     **def_inst)
{
    gctUINT                 i = 0;
    VIR_Operand             *currSrc;

    for (i = 0; i < VIR_Inst_GetSrcNum(inst); ++i)
    {
        currSrc = VIR_Inst_GetSource(inst, i);

        if (_VSC_PH_SrcUniqueDef(ph, inst, currSrc, def_inst) &&
            VIR_Inst_GetOpcode(*def_inst) == VIR_OP_LDARR)
        {
            *inst_src = currSrc;
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static VSC_ErrCode _VSC_PH_MoveDefCode(
    IN OUT VSC_PH_Peephole  *ph,
    IN VIR_Instruction      *inst,
    IN VIR_Operand          *inst_src,
    IN VIR_Instruction      *defInst
    )
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;
    VIR_Dumper          *dumper = VSC_PH_Peephole_GetDumper(ph);
    VSC_OPTN_PHOptions  *options = VSC_PH_Peephole_GetOptions(ph);
    VIR_Shader          *shader = VSC_PH_Peephole_GetShader(ph);
    VIR_Function        *func = VIR_Shader_GetCurrentFunction(shader);

    gctBOOL             invalidCase = gcvFALSE;

    VIR_Instruction *preDefInst = VIR_Inst_GetPrev(defInst);
    VIR_Instruction *nextDefInst = VIR_Inst_GetNext(defInst);
    VIR_Instruction *prevInst = VIR_Inst_GetPrev(inst);

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MOV_DEF))
    {
        VIR_LOG(dumper, "\nInstruction:");
        VIR_LOG_FLUSH(dumper);
        VIR_Inst_Dump(dumper, inst);
        VIR_LOG_FLUSH(dumper);
    }

    gcmASSERT(inst_src != gcvNULL);
    gcmASSERT(nextDefInst);
    gcmASSERT(prevInst);

    /* if the defInst and inst is not next to each other, move defInst down */
    if (nextDefInst != inst)
    {
        gcmASSERT(prevInst != defInst);

        /* defInst and inst is at the same BB */
        if(VIR_Inst_GetBasicBlock(defInst) == VIR_Inst_GetBasicBlock(inst))
        {
            /* there is no redefine of defInst'src in between */
            VIR_Instruction *next = VIR_Inst_GetNext(defInst);
            VIR_SrcOperand_Iterator opndIter;
            VIR_Operand     *nextOpnd;

            while(next != inst)
            {
                VIR_SrcOperand_Iterator_Init(defInst, &opndIter);
                nextOpnd = VIR_SrcOperand_Iterator_First(&opndIter);

                for (; nextOpnd != gcvNULL; nextOpnd = VIR_SrcOperand_Iterator_Next(&opndIter))
                {
                    if(VIR_Operand_SameLocation(defInst, nextOpnd, next, VIR_Inst_GetDest(next)))
                    {
                        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MOV_DEF))
                        {
                            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                            VIR_LOG(dumper, "prevented by another def instruction:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, next);
                            VIR_LOG_FLUSH(dumper);
                        }
                        invalidCase = gcvTRUE;
                        break;
                    }
                }

                if(invalidCase)
                {
                    break;
                }
                next = VIR_Inst_GetNext(next);
            }
        }

        if (!invalidCase)
        {
            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MOV_DEF))
            {
                VIR_LOG(dumper, "Move \n");
                VIR_Inst_Dump(dumper, defInst);
                VIR_LOG_FLUSH(dumper);
                VIR_LOG(dumper, "close to \n");
                VIR_Inst_Dump(dumper, inst);
                VIR_LOG_FLUSH(dumper);
            }

            /* change the instruction prev/next pointer, BB firstInst pointer,
                shader's firstInst pointer */
            if (preDefInst)
            {
                VIR_Inst_SetPrev(nextDefInst, preDefInst);
                VIR_Inst_SetNext(preDefInst, nextDefInst);
                if(VIR_Inst_GetBasicBlock(defInst) != VIR_Inst_GetBasicBlock(preDefInst))
                {
                    BB_SET_START_INST(VIR_Inst_GetBasicBlock(defInst), nextDefInst);
                }
            }
            else
            {
                gcmASSERT(BB_GET_START_INST(VIR_Inst_GetBasicBlock(defInst)) == defInst);
                BB_SET_START_INST(VIR_Inst_GetBasicBlock(defInst), nextDefInst);
                VIR_Inst_SetPrev(nextDefInst, gcvNULL);
                if(func->instList.pHead == defInst)
                {
                    func->instList.pHead = nextDefInst;
                }
            }

            VIR_Inst_SetNext(prevInst, defInst);
            VIR_Inst_SetPrev(defInst, prevInst);

            VIR_Inst_SetPrev(inst, defInst);
            VIR_Inst_SetNext(defInst, inst);
        }
    }

    return errCode;
}
static VSC_ErrCode _VSC_PH_DoPeepholeForBB(
    IN OUT VSC_PH_Peephole* ph
    )
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_BB* bb = VSC_PH_Peephole_GetCurrBB(ph);
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VIR_Instruction* inst;

    /* dump input bb */
    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_INPUT_BB))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        VIR_LOG(dumper, "%s\nPeephole Start for BB %d\n%s\n",
            VSC_TRACE_STAR_LINE, VSC_PH_Peephole_GetCurrBB(ph)->dgNode.id, VSC_TRACE_STAR_LINE);
        VIR_BasicBlock_Dump(dumper, VSC_PH_Peephole_GetCurrBB(ph), gcvFALSE);
    }

    /* generate operand modifiers */
    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetOPTS(options), VSC_OPTN_PHOptions_OPTS_MODIFIER))
    {
        VSC_PH_ModifierToGen* mtgs[VSC_PH_ModifierToGen_COUNT];
        gctINT count = 0;

        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetModifiers(options), VSC_OPTN_PHOptions_MODIFIERS_SAT))
        {
            mtgs[count++] = &VSC_PH_ModifierToGen_SAT;
        }
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetModifiers(options), VSC_OPTN_PHOptions_MODIFIERS_NEG))
        {
            mtgs[count++] = &VSC_PH_ModifierToGen_NEG;
        }
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetModifiers(options), VSC_OPTN_PHOptions_MODIFIERS_ABS))
        {
            mtgs[count++] = &VSC_PH_ModifierToGen_ABS;
        }

        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "%s\nModifier generation started\n%s\n", VSC_TRACE_SHARP_LINE, VSC_TRACE_SHARP_LINE);
            VIR_LOG_FLUSH(dumper);
        }
        inst = BB_GET_START_INST(bb);
        while(inst != VIR_Inst_GetNext(BB_GET_END_INST(bb)))
        {
            _VSC_PH_GenerateModifiers(ph, inst, mtgs, count, gcvNULL);
            inst = VIR_Inst_GetNext(inst);
        }
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "%s\nModifier generation ended\n%s\n", VSC_TRACE_SHARP_LINE, VSC_TRACE_SHARP_LINE);
            VIR_LOG_FLUSH(dumper);
        }
    }

    /* merge MUL/ADD/SUB instructions and generate MAD instruction */
    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetOPTS(options), VSC_OPTN_PHOptions_OPTS_MAD))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "%s\nMAD started\n%s\n", VSC_TRACE_SHARP_LINE, VSC_TRACE_SHARP_LINE);
            VIR_LOG_FLUSH(dumper);
        }
        inst = BB_GET_START_INST(bb);
        while(inst != VIR_Inst_GetNext(BB_GET_END_INST(bb)))
        {
            VIR_OpCode opc;

            opc = VIR_Inst_GetOpcode(inst);
            if(opc == VIR_OP_MUL || opc == VIR_OP_MULSAT)
            {
                _VSC_PH_GenerateMAD(ph, inst, gcvNULL);
            }
            inst = VIR_Inst_GetNext(inst);
        }
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "%s\nMAD ended\n%s\n", VSC_TRACE_SHARP_LINE, VSC_TRACE_SHARP_LINE);
            VIR_LOG_FLUSH(dumper);
        }
    }

    /* merge SQRT/RCP instructions to RSQ instruction */
    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetOPTS(options), VSC_OPTN_PHOptions_OPTS_RSQ))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_RSQ))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "%s\nRSQ started\n%s\n", VSC_TRACE_SHARP_LINE, VSC_TRACE_SHARP_LINE);
            VIR_LOG_FLUSH(dumper);
        }
        inst = BB_GET_START_INST(bb);
        while(inst != VIR_Inst_GetNext(BB_GET_END_INST(bb)))
        {
            VIR_OpCode opc;

            opc = VIR_Inst_GetOpcode(inst);
            if(opc == VIR_OP_SQRT)
            {
                _VSC_PH_GenerateRSQ(ph, inst, gcvNULL);
            }
            inst = VIR_Inst_GetNext(inst);
        }
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_RSQ))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "%s\nRSQ ended\n%s\n", VSC_TRACE_SHARP_LINE, VSC_TRACE_SHARP_LINE);
            VIR_LOG_FLUSH(dumper);
        }
    }

    /* vectorize instructions*/
    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetOPTS(options), VSC_OPTN_PHOptions_OPTS_VEC))
    {
        VSC_HASH_TABLE  *vec_def_set;
        VSC_PH_MergeKey *curr_mova;

        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_VEC))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "%s\npeephole vectorization started\n%s\n", VSC_TRACE_SHARP_LINE, VSC_TRACE_SHARP_LINE);
            VIR_LOG_FLUSH(dumper);
        }
        vec_def_set = vscHTBL_Create(VSC_PH_Peephole_GetMM(ph), _HKCMP_MergeKeyHFUNC, _HKCMP_MergeKeyEqual, 2048);
        curr_mova = (VSC_PH_MergeKey*)vscMM_Alloc(VSC_PH_Peephole_GetMM(ph), sizeof(VSC_PH_MergeKey));
        curr_mova->opcode = VIR_OP_MOVA;
        curr_mova->src1_imm = 0;
        curr_mova->defKey = gcvNULL;
        inst = BB_GET_START_INST(bb);
        while(inst != VIR_Inst_GetNext(BB_GET_END_INST(bb)))
        {
            VIR_OpCode opc;

            opc = VIR_Inst_GetOpcode(inst);
            if (_VSC_PH_VEC_mergable(opc))
            {
                _VSC_PH_VEC_MergeInst(ph, inst, vec_def_set, curr_mova);
            }
            inst = VIR_Inst_GetNext(inst);
        }
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_VEC))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "%s\npeephole vectorization ended\n%s\n", VSC_TRACE_SHARP_LINE, VSC_TRACE_SHARP_LINE);
            VIR_LOG_FLUSH(dumper);
        }

        vscHTBL_Destroy(vec_def_set);
        vscMM_Free(VSC_PH_Peephole_GetMM(ph), curr_mova);
    }

    /* remove unused checking instruction */
    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetOPTS(options), VSC_OPTN_PHOptions_OPTS_RUC))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_RUC))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "%s\npeephole remove unused checking started\n%s\n", VSC_TRACE_SHARP_LINE, VSC_TRACE_SHARP_LINE);
            VIR_LOG_FLUSH(dumper);
        }
        inst = BB_GET_START_INST(bb);
        while(inst && BB_GET_END_INST(bb) && inst != VIR_Inst_GetNext(BB_GET_END_INST(bb)))
        {
            VIR_OpCode opc;
            VIR_OperandInfo  src0Info, src1Info;
            gctBOOL     checkingResult;

            opc = VIR_Inst_GetOpcode(inst);
            if(opc == VIR_OP_JMPC)
            {
                VIR_Operand_GetOperandInfo(inst,
                    VIR_Inst_GetSource(inst, 0),
                    &src0Info);
                VIR_Operand_GetOperandInfo(inst,
                    VIR_Inst_GetSource(inst, 1),
                    &src1Info);

                if (src0Info.isImmVal && src1Info.isImmVal)
                {
                    if (_VSC_PH_EvaluateChecking(VSC_PH_Peephole_GetShader(ph), inst, &checkingResult))
                    {
                        if (VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options),
                            VSC_OPTN_PHOptions_TRACE_RUC))
                        {
                            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                            VIR_LOG(dumper, "\nInstruction\n");
                            VIR_Inst_Dump(dumper, inst);
                            VIR_LOG_FLUSH(dumper);
                        }

                        if (checkingResult)
                        {
                             /* change instruction to jmp*/
                            VIR_Inst_SetOpcode(inst, VIR_OP_JMP);
                            VIR_Inst_SetConditionOp(inst, VIR_COP_ALWAYS);
                            VIR_Inst_SetSrcNum(inst, 0);
                        }
                        else
                        {
                            /*change instruction to nop*/
                            VIR_Inst_SetOpcode(inst, VIR_OP_NOP);
                            VIR_Inst_SetSrcNum(inst, 0);
                            VIR_Inst_SetDest(inst, gcvNULL);
                        }

                        if (VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options),
                            VSC_OPTN_PHOptions_TRACE_RUC))
                        {
                            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                            VIR_LOG(dumper, "==>change to\n");
                            VIR_Inst_Dump(dumper, inst);
                            VIR_LOG_FLUSH(dumper);
                        }

                         VSC_PH_Peephole_SetCfgChanged(ph, gcvTRUE);
                    }
                }
            }
            inst = VIR_Inst_GetNext(inst);
        }
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_RUC))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "%s\npeephole remove unused checking end\n%s\n", VSC_TRACE_SHARP_LINE, VSC_TRACE_SHARP_LINE);
            VIR_LOG_FLUSH(dumper);
        }
    }


    /* move instruction close to its only use
     1) ATOM instruction related optimization - to generate the right code sequence in code gen,
       for example:
        LOAD 1, 2, 3
        ATOMADD 4, 1, 5
            atom_add 4, 2, 3, 5, 0
       We need to move LOAD and AUTMADD next to each other.
     2) mov MOVA/LDARR just before its use to reduce register usage */

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetOPTS(options), VSC_OPTN_PHOptions_OPTS_MOV_DEF))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MOV_DEF))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "%s\npeephole MOV DEF OPT started\n%s\n", VSC_TRACE_SHARP_LINE, VSC_TRACE_SHARP_LINE);
            VIR_LOG_FLUSH(dumper);
        }

        inst = BB_GET_START_INST(bb);
        while(inst != VIR_Inst_GetNext(BB_GET_END_INST(bb)))
        {
            VIR_OpCode opc;
            VIR_Operand *srcOpnd = gcvNULL;
            VIR_Instruction *defInst = gcvNULL, *defDefInst = gcvNULL;

            opc = VIR_Inst_GetOpcode(inst);
            if (VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetOPTS(options), VSC_OPTN_PHOptions_OPTS_MOV_ATOM) &&
                VIR_OPCODE_isAtom(opc))
            {
                srcOpnd = VIR_Inst_GetSource(inst, 0);
                if (_VSC_PH_SrcUniqueDef(ph, inst, srcOpnd, &defInst))
                {
                    _VSC_PH_MoveDefCode(ph, inst, srcOpnd, defInst);
                }
            }

            if (VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetOPTS(options), VSC_OPTN_PHOptions_OPTS_MOV_LDARR) &&
                _VSC_PH_SrcFromUniqueLDARR(ph, inst, &srcOpnd, &defInst))
            {
                _VSC_PH_MoveDefCode(ph, inst, srcOpnd, defInst);

                srcOpnd = VIR_Inst_GetSource(defInst, 1);
                if (_VSC_PH_SrcUniqueDef(ph, defInst, srcOpnd, &defDefInst))
                {
                    _VSC_PH_MoveDefCode(ph, defInst, srcOpnd, defDefInst);
                }
            }

            inst = VIR_Inst_GetNext(inst);
        }

        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MOV_DEF))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "%s\npeephole MOV DEF OPT ended\n%s\n", VSC_TRACE_SHARP_LINE, VSC_TRACE_SHARP_LINE);
            VIR_LOG_FLUSH(dumper);
        }
    }

    /* dump output bb */
    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_OUTPUT_BB))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        VIR_LOG(dumper, "%s\nPeephole End for BB %d\n%s\n",
            VSC_TRACE_STAR_LINE, VSC_PH_Peephole_GetCurrBB(ph)->dgNode.id, VSC_TRACE_STAR_LINE);
        VIR_BasicBlock_Dump(dumper, VSC_PH_Peephole_GetCurrBB(ph), gcvFALSE);
    }

    return errCode;
}

VSC_ErrCode VSC_PH_Peephole_PerformOnFunction(
    IN OUT VSC_PH_Peephole* ph
    )
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_Shader* shader = VSC_PH_Peephole_GetShader(ph);
    VIR_Function* func = VIR_Shader_GetCurrentFunction(shader);
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VIR_CONTROL_FLOW_GRAPH* cfg;
    CFG_ITERATOR cfg_iter;
    VIR_BASIC_BLOCK* bb;
    gctBOOL scheduled = gcvFALSE;
    static gctUINT32 counter = 0;

    if(!VSC_OPTN_InRange(counter, VSC_OPTN_PHOptions_GetBeforeFunc(options), VSC_OPTN_PHOptions_GetAfterFunc(options)))
    {
        if(VSC_OPTN_PHOptions_GetTrace(options))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "Peephole skips function(%d)\n", counter);
            VIR_LOG_FLUSH(dumper);
        }
        counter++;
        return errCode;
    }

    if(VSC_OPTN_PHOptions_GetTrace(options))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        VIR_LOG(dumper, "%s\nPeephole starts for function %s(%d)\n%s\n",
            VSC_TRACE_STAR_LINE, VIR_Shader_GetSymNameString(shader, VIR_Function_GetSymbol(func)), counter, VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
    }

    cfg = VIR_Function_GetCFG(func);

    /* dump input cfg */
    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_INPUT_CFG))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        VIR_LOG(dumper, "%s\nPeephole: input cfg of function %s\n%s\n",
            VSC_TRACE_STAR_LINE, VIR_Shader_GetSymNameString(shader, VIR_Function_GetSymbol(func)), VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
        VIR_CFG_Dump(dumper, cfg, gcvTRUE);
    }
    /*VIR_DU_Info_Dump(ph->du_info);*/
    /* Only inst count GT 1 ph meaningful for inst sched */
    if (VIR_Inst_Count(&func->instList) > 1)
    {
        CFG_ITERATOR_INIT(&cfg_iter, cfg);
        for(bb = CFG_ITERATOR_FIRST(&cfg_iter); bb != gcvNULL; bb = CFG_ITERATOR_NEXT(&cfg_iter))
        {
            if(BB_GET_LENGTH(bb) != 0)
            {
                if(!VSC_OPTN_InRange(BB_GET_ID(bb), VSC_OPTN_PHOptions_GetBeforeBB(options), VSC_OPTN_PHOptions_GetAfterBB(options)))
                {
                    if(VSC_OPTN_PHOptions_GetTrace(options))
                    {
                        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                        VIR_LOG(dumper, "Peephole skips basci block(%d)\n", BB_GET_ID(bb));
                        VIR_LOG_FLUSH(dumper);
                    }
                    continue;
                }

                VSC_PH_Peephole_SetCurrBB(ph, bb);
                errCode = _VSC_PH_DoPeepholeForBB(ph);
                scheduled = gcvTRUE;
            }

            if(errCode)
            {
                return errCode;
            }
        }

        func->instList.pHead = func->instList.pHead->parent.BB->pStartInst;
        func->instList.pTail = func->instList.pTail->parent.BB->pEndInst;
    }

    /* dump output cfg */
    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_OUTPUT_CFG))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        VIR_LOG(dumper, "%s\nPeephole: output cfg of function %s: %s\n%s\n",
            VSC_TRACE_STAR_LINE, VIR_Shader_GetSymNameString(shader, VIR_Function_GetSymbol(func)),
            scheduled ? "scheduled" : "not scheduled", VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
        VIR_CFG_Dump(dumper, cfg, gcvTRUE);
    }
    /*VIR_DU_Info_Dump(ph->du_info);*/
    if(VSC_OPTN_PHOptions_GetTrace(options))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        VIR_LOG(dumper, "%s\nPeephole ends for function %s(%d)\n%s\n",
            VSC_TRACE_BAR_LINE, VIR_Shader_GetSymNameString(shader, VIR_Function_GetSymbol(func)), counter, VSC_TRACE_BAR_LINE);
        VIR_LOG_FLUSH(dumper);
    }
    counter++;

    return errCode;
}

VSC_ErrCode VSC_PH_Peephole_PerformOnShader(
    IN OUT VSC_PH_Peephole* ph
    )
{
    VSC_ErrCode errcode  = VSC_ERR_NONE;
    VIR_Shader* shader = VSC_PH_Peephole_GetShader(ph);
    VIR_FuncIterator func_iter;
    VIR_FunctionNode* func_node;
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);

    if(!VSC_OPTN_InRange(VIR_Shader_GetId(shader), VSC_OPTN_PHOptions_GetBeforeShader(options), VSC_OPTN_PHOptions_GetAfterShader(options)))
    {
        if(VSC_OPTN_PHOptions_GetTrace(options))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "Peephole skips shader(%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
        }
        return errcode;
    }
    else
    {
        if(VSC_OPTN_PHOptions_GetTrace(options))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "Peephole starts for shader(%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
        }
    }

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function* func = func_node->function;

        VIR_Shader_SetCurrentFunction(shader, func);
        errcode = VSC_PH_Peephole_PerformOnFunction(ph);
        if(errcode)
        {
            break;
        }
    }

    if(VSC_OPTN_PHOptions_GetTrace(options))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        VIR_LOG(dumper, "Peephole ends for shader(%d)\n", VIR_Shader_GetId(shader));
        VIR_LOG_FLUSH(dumper);
    }
    if (gcSHADER_DumpCodeGenVerbose(shader))
    {
        VIR_Shader_Dump(gcvNULL, "After Peephole.", shader, gcvTRUE);
    }

    return errcode;
}

