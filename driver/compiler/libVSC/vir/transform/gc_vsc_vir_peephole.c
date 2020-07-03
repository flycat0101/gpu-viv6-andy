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


#include "vir/transform/gc_vsc_vir_peephole.h"

static void VSC_PH_Peephole_Init(
    IN OUT VSC_PH_Peephole* ph,
    IN VIR_Shader* shader,
    IN VIR_DEF_USAGE_INFO* du_info,
    IN VSC_HW_CONFIG* hwCfg,
    IN VSC_OPTN_PHOptions* options,
    IN VIR_Dumper* dumper,
    IN VSC_MM* pMM
    )
{
    if (VIR_Shader_IsVulkan(shader))
    {
        VSC_OPTN_PHOptions_SetOPTS(options, VSC_OPTN_PHOptions_GetOPTS(options) | VSC_OPTN_PHOptions_OPTS_ADD_MEM_ADDR);
    }

    VSC_PH_Peephole_SetShader(ph, shader);
    VSC_PH_Peephole_SetCurrBB(ph, gcvNULL);
    VSC_PH_Peephole_SetDUInfo(ph, du_info);
    VSC_PH_Peephole_SetHwCfg(ph, hwCfg);
    VSC_PH_Peephole_SetOptions(ph, options);
    VSC_PH_Peephole_SetDumper(ph, dumper);
    ph->pMM = pMM;
    VSC_PH_Peephole_SetCfgChanged(ph, gcvFALSE);
    VSC_PH_Peephole_SetExprChanged(ph, gcvFALSE);
    ph->work_set     = gcvNULL;
    ph->usage_set    = gcvNULL;
    ph->def_set0     = gcvNULL;
    ph->def_set1     = gcvNULL;
}

static void VSC_PH_Peephole_Final(
    IN OUT VSC_PH_Peephole* ph
    )
{
    VSC_PH_Peephole_SetShader(ph, gcvNULL);
    VSC_PH_Peephole_SetOptions(ph, gcvNULL);
    VSC_PH_Peephole_SetDumper(ph, gcvNULL);
    vscHTBL_Destroy (ph->work_set);
    vscHTBL_Destroy (ph->usage_set);
    vscHTBL_Destroy (ph->def_set0);
    vscHTBL_Destroy (ph->def_set1);
}

typedef gctBOOL
(* _VSC_PH_MODIFIERTOGEN_VERIFYDEFINST)(
    IN VIR_Instruction*         pDefInst
    );

typedef gctBOOL
(* _VSC_PH_MODIFIERTOGEN_VERIFYUSAGEINST)(
    IN VIR_Operand*             pDefOpnd,
    IN VIR_Instruction*         pUsageInst
    );

static gctBOOL
_VSC_PH_ModifierSAT_VerifyDefInst(
    IN VIR_Instruction*         pDefInst
    )
{
    gctBOOL     valid = gcvTRUE;
    VIR_OpCode  opCode = VIR_Inst_GetOpcode(pDefInst);

    /* Now only support destModifier for FLOAT32 or LOAD/STORE/IMG_STORE/I2I/CONV */
    if (!VIR_TypeId_isFloat(VIR_Inst_GetInstType(pDefInst)) &&
        !VIR_OPCODE_isMemLd(opCode)                         &&
        !VIR_OPCODE_isMemSt(opCode)                         &&
        !VIR_OPCODE_isImgSt(opCode)                         &&
        (opCode != VIR_OP_CONVERT))
    {
        valid = gcvFALSE;
    }

    return valid;
}

static gctBOOL
_VSC_PH_ModifierNEG_VerifyUsageInst(
    IN VIR_Operand*             pDefOpnd,
    IN VIR_Instruction*         pUsageInst
    )
{
    gctBOOL                     valid = gcvTRUE;
    VIR_TypeId                  opndBaseTypeId = VIR_GetTypeComponentType(VIR_Operand_GetTypeId(pDefOpnd));
    VIR_TypeId                  usageDstTypeId;
    VIR_Operand*                pUsageDstOpnd = VIR_Inst_GetDest(pUsageInst);

    if (VIR_OPCODE_hasDest(VIR_Inst_GetOpcode(pUsageInst)) && pUsageDstOpnd)
    {
        usageDstTypeId = VIR_GetTypeComponentType(VIR_Operand_GetTypeId(pUsageDstOpnd));

        /* If NEG and its usage have different type, we need to skip it. */
        if (opndBaseTypeId != usageDstTypeId)
        {
            valid = gcvFALSE;
        }
    }

    return valid;
}

static gctBOOL
_VSC_PH_ModifierABS_VerifyUsageInst(
    IN VIR_Operand*             pDefOpnd,
    IN VIR_Instruction*         pUsageInst
    )
{
    gctBOOL                     valid = gcvTRUE;
    VIR_TypeId                  opndBaseTypeId = VIR_GetTypeComponentType(VIR_Operand_GetTypeId(pDefOpnd));
    VIR_TypeId                  usageDstTypeId;
    VIR_Operand*                pUsageDstOpnd = VIR_Inst_GetDest(pUsageInst);

    if (VIR_OPCODE_hasDest(VIR_Inst_GetOpcode(pUsageInst)) && pUsageDstOpnd)
    {
        usageDstTypeId = VIR_GetTypeComponentType(VIR_Operand_GetTypeId(pUsageDstOpnd));

        /* If ABS and its usage have different type, we need to skip it. */
        if (opndBaseTypeId != usageDstTypeId)
        {
            valid = gcvFALSE;
        }
    }

    return valid;
}

typedef struct VSC_PH_MODIFIERTOGEN
{
    VIR_OpCode                              opc : 20;
    gctUINT                                 lvalue : 1;
    gctUINT                                 modifier : 4;
    gctSTRING                               name;
    _VSC_PH_MODIFIERTOGEN_VERIFYDEFINST     verifyDefInst;
    _VSC_PH_MODIFIERTOGEN_VERIFYUSAGEINST   verifyUsageInst;
} VSC_PH_ModifierToGen;

#define VSC_PH_ModifierToGen_GetOPC(mtg)            ((mtg)->opc)
#define VSC_PH_ModifierToGen_SetOPC(mtg, o)         ((mtg)->opc = (o))
#define VSC_PH_ModifierToGen_GetLValue(mtg)         ((mtg)->lvalue)
#define VSC_PH_ModifierToGen_SetLValue(mtg, l)      ((mtg)->lvalue = (l))
#define VSC_PH_ModifierToGen_GetModifier(mtg)       ((mtg)->modifier)
#define VSC_PH_ModifierToGen_SetModifier(mtg, m)    ((mtg)->modifier = (m))
#define VSC_PH_ModifierToGen_GetName(mtg)           ((mtg)->name)
#define VSC_PH_ModifierToGen_SetName(mtg, n)        ((mtg)->name = (n))
#define VSC_PH_ModifierToGen_GetDefVerifyFunc(mtg)  ((mtg)->verifyDefInst)
#define VSC_PH_ModifierToGen_GetVerifyUsageFunc(mtg)((mtg)->verifyUsageInst)

#define VSC_PH_ModifierToGen_COUNT                  4
VSC_PH_ModifierToGen VSC_PH_ModifierToGen_SAT = {VIR_OP_SAT, 1, VIR_MOD_SAT_0_TO_1, "SAT_0_TO_1", _VSC_PH_ModifierSAT_VerifyDefInst, gcvNULL};
VSC_PH_ModifierToGen VSC_PH_ModifierToGen_NEG = {VIR_OP_NEG, 0, VIR_MOD_NEG, "NEG", gcvNULL, _VSC_PH_ModifierNEG_VerifyUsageInst};
VSC_PH_ModifierToGen VSC_PH_ModifierToGen_ABS = {VIR_OP_ABS, 0, VIR_MOD_ABS, "ABS", gcvNULL, _VSC_PH_ModifierABS_VerifyUsageInst};
VSC_PH_ModifierToGen VSC_PH_ModifierToGen_CONJ = {VIR_OP_CONJ, 0, VIR_MOD_ABS, "CONJ", gcvNULL, gcvNULL};

static void VSC_PH_ModifierToGen_Dump(VSC_PH_ModifierToGen* mtg, VIR_Dumper* dumper)
{
    VIR_LOG(dumper, "%s modifier %s\n", VSC_PH_ModifierToGen_GetLValue(mtg) ? "Lvalue" : "Rvalue", VSC_PH_ModifierToGen_GetName(mtg));
}

typedef struct VSC_PH_OPNDTARGET
{
    VIR_Instruction* inst;
    VIR_Operand* opnd;
    void* pPrivData;
} VSC_PH_OpndTarget;

static gctUINT _VSC_PH_OpndTarget_HFUNC(const void* ptr)
{
    return (gctUINT)(gctUINTPTR_T)((VSC_PH_OpndTarget*)ptr)->inst >> 2;
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
    ot->pPrivData = gcvNULL;
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

    gctUINT hashVal = (((gctUINT)(gctUINTPTR_T) pMergeKey->defKey->pDefInst) << 4) ^ pMergeKey->src1_imm ;

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

/* new PH code start */
typedef struct VSC_PH_TREE_NODE
{
    gctUINT id;
    VIR_Instruction* inst;
    gctUINT channel;
} VSC_PH_Tree_Node;

#define VSC_PH_Tree_Node_GetID(N)                                           ((N)->id)
#define VSC_PH_Tree_Node_GetInst(N)                                         ((N)->inst)
#define VSC_PH_Tree_Node_SetInst(N, i)                                      ((N)->inst = (i))
#define VSC_PH_Tree_Node_GetChannel(N)                                      ((N)->channel)
#define VSC_PH_Tree_Node_Setchannel(N, c)                                   ((N)->channel = (c))

static void _VSC_PH_Tree_Node_Dump(
    IN VSC_PH_Tree_Node* node,
    IN VIR_Dumper* dumper
    )
{
    if(VSC_PH_Tree_Node_GetInst(node))
    {
        VIR_LOG(dumper, "Node id: %d, channel: %d\n", VSC_PH_Tree_Node_GetID(node), VSC_PH_Tree_Node_GetChannel(node));
        VIR_Inst_Dump(dumper, VSC_PH_Tree_Node_GetInst(node));
    }
}

#define VSC_PH_TREE_N 2
#define VSC_PH_TREE_MAX_LEVEL 3
#define VSC_PH_TREE_NODE_COUNT 15

typedef struct VSC_PH_TREE
{
    VSC_PH_Tree_Node nodes[VSC_PH_TREE_NODE_COUNT];
} VSC_PH_Tree;

#define VSC_PH_Tree_GetRoot(tree)                                           (&(tree)->nodes[0])
#define VSC_PH_Tree_GetNode(tree, i)                                        (&(tree)->nodes[i])
#define VSC_PH_Tree_GetParentIDFromID(i)                                    (((i) - 1) / 2)
#define VSC_PH_Tree_GetFirstChildIDFromID(i)                                ((i) * 2 + 1)
#define VSC_PH_Tree_GetSecondChildIDFromID(i)                               ((i) * 2 + 2)
#define VSC_PH_Tree_GetNthChildIDFromID(i, j)                               ((i) * 2 + j + 1)
#define VSC_PH_Tree_GetParentFromID(tree, i)                                VSC_PH_Tree_GetNode(tree, VSC_PH_Tree_GetParentIDFromID(i))
#define VSC_PH_Tree_GetFirstChildFromID(tree, i)                            VSC_PH_Tree_GetNode(tree, VSC_PH_Tree_GetFirstChildIDFromID(i))
#define VSC_PH_Tree_GetSecondChildFromID(tree, i)                           VSC_PH_Tree_GetNode(tree, VSC_PH_Tree_GetSecondChildIDFromID(i))
#define VSC_PH_Tree_GetNthChildFromID(tree, i, j)                           VSC_PH_Tree_GetNode(tree, VSC_PH_Tree_GetNthChildIDFromID(i, j))
#define VSC_PH_Tree_GetParentFromNode(tree, node)                           VSC_PH_Tree_GetNode(tree, VSC_PH_Tree_GetParentIDFromID(VSC_PH_Tree_Node_GetID(node)))
#define VSC_PH_Tree_GetFirstChildFromNode(tree, node)                       VSC_PH_Tree_GetNode(tree, VSC_PH_Tree_GetFirstChildIDFromID(VSC_PH_Tree_Node_GetID(node)))
#define VSC_PH_Tree_GetSecondChildFromNode(tree, node)                      VSC_PH_Tree_GetNode(tree, VSC_PH_Tree_GetSecondChildIDFromID(VSC_PH_Tree_Node_GetID(node)))
#define VSC_PH_Tree_GetNthChildFromNode(tree, node, i)                      VSC_PH_Tree_GetNode(tree, VSC_PH_Tree_GetNthChildIDFromID(VSC_PH_Tree_Node_GetID(node), i))

static gctUINT _VSC_PH_Tree_GetLevelFromID(
    IN gctUINT ID
    )
{
    gctUINT level = 0;

    while(ID)
    {
        ID >>= 1;
        level++;
    }

    return level;
}

static void _VSC_PH_Tree_Dump(
    IN VSC_PH_Tree* tree,
    IN VIR_Dumper* dumper
    )
{
    gctUINT i;

    VIR_LOG(dumper, "PH tree:\n");
    for(i = 0; i < VSC_PH_TREE_NODE_COUNT; i++)
    {
        _VSC_PH_Tree_Node_Dump(VSC_PH_Tree_GetNode(tree, i), dumper);
    }
}

typedef gctUINT (*VSC_PH_FuncP)(
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VSC_PH_Tree* tree,
    IN OUT void* dynamicInputOutput,
    IN gctUINT argCount,
    IN gctUINT* args
    );

/****************** Hast Table functions **********************************/
VSC_ErrCode
_VSC_PH_InitHashTable(
    IN OUT VSC_PH_Peephole*  ph,
    IN OUT VSC_HASH_TABLE ** ppHT,
    IN PFN_VSC_HASH_FUNC     pfnHashFunc,
    IN PFN_VSC_KEY_CMP       pfnKeyCmp,
    IN gctINT                tableSize
    )
{
    return vscHTBL_CreateOrInitialize(VSC_PH_Peephole_GetMM(ph), ppHT, pfnHashFunc, pfnKeyCmp, tableSize);
}

void
_VSC_PH_ResetHashTable(
    IN OUT VSC_HASH_TABLE * pHT
    )
{
    if (pHT)
    {
        vscHTBL_Reset(pHT);
    }
}

/********************************************** tree building requirment functions starts **********************************************/

gctUINT _VSC_PH_Func_NodeLevelLessThan(
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VSC_PH_Tree* tree,
    IN OUT void* dynamicInputOutput,
    IN gctUINT argCount,
    IN gctUINT* args
    )
{
    gctUINT level = args[0];

    gcmASSERT(argCount == 1);
    return _VSC_PH_Tree_GetLevelFromID(VSC_PH_Tree_Node_GetID((VSC_PH_Tree_Node*)dynamicInputOutput)) < level;
}

/********************************************** tree building requirment functions ends **********************************************/

/********************************************** tree transform requirment functions starts **********************************************/

gctUINT _VSC_PH_Func_GetNodeOpCode(
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VSC_PH_Tree* tree,
    IN OUT void* dynamicInputOutput,
    IN gctUINT argCount,
    IN gctUINT* args
    )
{
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    gctUINT nodeId = args[0];

    gcmASSERT(argCount == 1);

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_REQUIREMENTS))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        gctUINT i;

        VIR_LOG(dumper, "%s got %d parameters:", "_VSC_PH_Func_GetNodeOpCode", argCount);
        for(i = 0; i < argCount; i++)
        {
            VIR_LOG(dumper, " %x", args[i]);
        }
    }

    {
        VSC_PH_Tree_Node* node = VSC_PH_Tree_GetNode(tree, nodeId);
        VIR_Instruction* inst = VSC_PH_Tree_Node_GetInst(node);

        return inst ? VIR_Inst_GetOpcode(inst) : VIR_OP_NOP;
    }
}

gctUINT _VSC_PH_Func_GetNodeSourceBaseTypeId(
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VSC_PH_Tree* tree,
    IN OUT void* dynamicInputOutput,
    IN gctUINT argCount,
    IN gctUINT* args
    )
{
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    gctUINT nodeId = args[0];
    gctUINT operandId = args[1];

    gcmASSERT(argCount == 2);

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_REQUIREMENTS))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        gctUINT i;

        VIR_LOG(dumper, "%s got %d parameters:", "_VSC_PH_Func_GetNodeSourceBaseTypeId", argCount);
        for(i = 0; i < argCount; i++)
        {
            VIR_LOG(dumper, " %x", args[i]);
        }
    }

    {
        VSC_PH_Tree_Node* node = VSC_PH_Tree_GetNode(tree, nodeId);
        VIR_Instruction* inst = VSC_PH_Tree_Node_GetInst(node);
        VIR_Operand* operand = VIR_Inst_GetSource(inst, operandId);
        VIR_TypeId typeId = VIR_Operand_GetTypeId(operand);

        return VIR_GetTypeComponentType(typeId);
    }
}

gctUINT _VSC_PH_Func_TwoSourcesHavingTheSameSym(
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VSC_PH_Tree* tree,
    IN OUT void* dynamicInputOutput,
    IN gctUINT argCount,
    IN gctUINT* args
    )
{
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    gctUINT node0Id = args[0];
    gctUINT inst0SourceId = args[1];
    gctUINT node1Id = args[2];
    gctUINT inst1SourceId = args[3];

    gcmASSERT(argCount == 4);

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_REQUIREMENTS))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        gctUINT i;

        VIR_LOG(dumper, "%s got %d parameters:", "_VSC_PH_Func_TwoSourcesHavingTheSameSym", argCount);
        for(i = 0; i < argCount; i++)
        {
            VIR_LOG(dumper, " %x", args[i]);
        }
    }

    {
        VSC_PH_Tree_Node* node0 = VSC_PH_Tree_GetNode(tree, node0Id);
        VIR_Instruction* inst0 = VSC_PH_Tree_Node_GetInst(node0);
        VIR_Operand* inst0Source = VIR_Inst_GetSource(inst0, inst0SourceId);
        VSC_PH_Tree_Node* node1 = VSC_PH_Tree_GetNode(tree, node1Id);
        VIR_Instruction* inst1 = VSC_PH_Tree_Node_GetInst(node1);
        VIR_Operand* inst1Source = VIR_Inst_GetSource(inst1, inst1SourceId);

        return VIR_Operand_SameIndexedSymbol(inst0Source, inst1Source);
    }
}

gctUINT _VSC_PH_Func_SourceHavingDefBeforeRoot(
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VSC_PH_Tree* tree,
    IN OUT void* dynamicInputOutput,
    IN gctUINT argCount,
    IN gctUINT* args
    )
{
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    gctUINT nodeId = args[0];
    gctUINT instSourceId = args[1];

    gcmASSERT(argCount == 2);

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_REQUIREMENTS))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        gctUINT i;

        VIR_LOG(dumper, "%s got %d parameters:", "_VSC_PH_Func_SourceHavingDefBeforeRoot", argCount);
        for(i = 0; i < argCount; i++)
        {
            VIR_LOG(dumper, " %x", args[i]);
        }
    }

    {
        VSC_PH_Tree_Node* root = VSC_PH_Tree_GetRoot(tree);
        VIR_Instruction* rootInst = VSC_PH_Tree_Node_GetInst(root);
        VSC_PH_Tree_Node* node = VSC_PH_Tree_GetNode(tree, nodeId);
        VIR_Instruction* nodeInst = VSC_PH_Tree_Node_GetInst(node);
        gctUINT nodeChannel = VSC_PH_Tree_Node_GetChannel(node);
        VIR_Operand* nodeInstSrc = VIR_Inst_GetSource(nodeInst, instSourceId);
        VIR_Swizzle nodeInstSrcSwizzle = VIR_Operand_GetSwizzle(nodeInstSrc);
        VIR_Swizzle nodeInstSrcChannelSwizzle = VIR_Swizzle_GetChannel(nodeInstSrcSwizzle, nodeChannel);
        VIR_Enable nodeInstSrcChannelEnable = (VIR_Enable)(1 << (gctUINT)nodeInstSrcChannelSwizzle);

        VIR_Instruction* nextInst = nodeInst;
        while(nextInst != rootInst)
        {
            VIR_OpCode nextOpcode = VIR_Inst_GetOpcode(nextInst);
            if(VIR_OPCODE_hasDest(nextOpcode))
            {
                VIR_Operand* nextInstDest = VIR_Inst_GetDest(nextInst);
                VIR_Enable nextInstDestEnable = VIR_Operand_GetEnable(nextInstDest);

                if(VIR_Operand_SameSymbol(nodeInstSrc, nextInstDest))
                {
                    if(nextInstDestEnable & nodeInstSrcChannelEnable)
                    {
                        return gcvTRUE;
                    }
                }

                if(!VIR_Operand_GetIsConstIndexing(nodeInstSrc) && VIR_Operand_GetRelAddrMode(nodeInstSrc))
                {
                    gctUINT relIndexingSymId = VIR_Operand_GetRelIndexing(nodeInstSrc);
                    VIR_Symbol* nextInstDestSym = VIR_Operand_GetSymbol(nextInstDest);
                    gctUINT relIndexingRegIndexed = VIR_Operand_GetRelAddrMode(nodeInstSrc);

                    if(VIR_Symbol_GetIndex(nextInstDestSym) == relIndexingSymId &&
                       (nextInstDestEnable & (1 << (relIndexingRegIndexed - 1))))
                    {
                        return gcvTRUE;
                    }
                }
            }
            nextInst = VIR_Inst_GetNext(nextInst);
        }

        return gcvFALSE;
    }
}

gctUINT _VSC_PH_Func_SourceIsImm(
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VSC_PH_Tree* tree,
    IN OUT void* dynamicInputOutput,
    IN gctUINT argCount,
    IN gctUINT* args
    )
{
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    gctUINT nodeId = args[0];
    gctUINT instSourceId = args[1];

    gcmASSERT(argCount == 2);

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_REQUIREMENTS))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        gctUINT i;

        VIR_LOG(dumper, "%s got %d parameters:", "_VSC_PH_Func_SourceIsImm", argCount);
        for(i = 0; i < argCount; i++)
        {
            VIR_LOG(dumper, " %x", args[i]);
        }
    }

    {
        VSC_PH_Tree_Node* node = VSC_PH_Tree_GetNode(tree, nodeId);
        VIR_Instruction* nodeInst = VSC_PH_Tree_Node_GetInst(node);
        VIR_Operand* nodeInstSrc = VIR_Inst_GetSource(nodeInst, instSourceId);

        return VIR_Operand_isImm(nodeInstSrc) || VIR_Operand_isConst(nodeInstSrc);
    }
}

/********************************************** tree transform requirment functions ends **********************************************/

typedef struct VSC_PH_RESULTINSTSYMOPERAND
{
    VIR_Instruction* srcFromInst[VIR_CHANNEL_COUNT];
    gctUINT srcFromOperandNum[VIR_CHANNEL_COUNT];              /* used for dump only */
    VIR_Operand* srcFromOperand[VIR_CHANNEL_COUNT];
    VIR_Swizzle srcSwizzle[VIR_CHANNEL_COUNT];
} VSC_PH_ResultInstSymOperand;

#define VSC_PH_ResultInstSymOperand_GetNthSrcFromInst(riso, i)              ((riso)->srcFromInst[i])
#define VSC_PH_ResultInstSymOperand_SetNthSrcFromInst(riso, i, inst)        ((riso)->srcFromInst[i] = (inst))
#define VSC_PH_ResultInstSymOperand_GetNthSrcFromOperandNum(riso, i)        ((riso)->srcFromOperandNum[i])
#define VSC_PH_ResultInstSymOperand_SetNthSrcFromOperandNum(riso, i, on)    ((riso)->srcFromOperandNum[i] = (on))
#define VSC_PH_ResultInstSymOperand_GetNthSrcFromOperand(riso, i)           ((riso)->srcFromOperand[i])
#define VSC_PH_ResultInstSymOperand_SetNthSrcFromOperand(riso, i, op)       ((riso)->srcFromOperand[i] = (op))
#define VSC_PH_ResultInstSymOperand_GetNthSrcSwizzle(riso, i)               ((riso)->srcSwizzle[i])
#define VSC_PH_ResultInstSymOperand_SetNthSrcSwizzle(riso, i, s)            ((riso)->srcSwizzle[i] = (s))

static void
_VSC_PH_ResultInstSymOperand_Dump(
    IN VSC_PH_ResultInstSymOperand* symOperand,
    IN gctUINT channelCount,
    IN VIR_Dumper* dumper
    )
{
    gctUINT i;

    for(i = 0; i < channelCount; i++)
    {
        VIR_Inst_Dump(dumper, VSC_PH_ResultInstSymOperand_GetNthSrcFromInst(symOperand, i));
        VIR_LOG(dumper, "operand number: %d, swizzle: ", VSC_PH_ResultInstSymOperand_GetNthSrcFromOperandNum(symOperand, i));
        VIR_Swizzle_Dump(dumper, VSC_PH_ResultInstSymOperand_GetNthSrcSwizzle(symOperand, i));
        VIR_LOG(dumper, "\n");
        VIR_LOG_FLUSH(dumper);
    }
}

typedef struct VSC_PH_RESULTINSTCONSTOPERAND
{
    union
    {
        gctINT int32Val[VIR_CHANNEL_COUNT];
        gctUINT uint32Val[VIR_CHANNEL_COUNT];
        gctFLOAT float32Val[VIR_CHANNEL_COUNT];
    } constVal;
} VSC_PH_ResultInstConstOperand;

#define VSC_PH_ResultInstConstOperand_GetNthConvstVal(rico, i)          ((rico)->constVal.uint32Val[i])
#define VSC_PH_ResultInstConstOperand_SetNthConvstVal(rico, i, cv)      ((rico)->constVal.uint32Val[i] = (cv))
#define VSC_PH_ResultInstConstOperand_GetNthInt32Val(rico, i)           ((rico)->constVal.int32Val[i])
#define VSC_PH_ResultInstConstOperand_SetNthInt32Val(rico, i, cv)       ((rico)->constVal.int32Val[i] = (cv))
#define VSC_PH_ResultInstConstOperand_GetNthUint32Val(rico, i)          ((rico)->constVal.uint32Val[i])
#define VSC_PH_ResultInstConstOperand_SetNthUint32Val(rico, i, cv)      ((rico)->constVal.uint32Val[i] = (cv))
#define VSC_PH_ResultInstConstOperand_GetNthFloat32Val(rico, i)         ((rico)->constVal.float32Val[i])
#define VSC_PH_ResultInstConstOperand_SetNthFloat32Val(rico, i, cv)     ((rico)->constVal.float32Val[i] = (cv))

void static
_VSC_PH_ResultInstConstOperand_Dump(
    IN VSC_PH_ResultInstConstOperand* constOperand,
    IN gctUINT channelCount,
    IN VIR_TypeId typeId,
    IN VIR_Dumper* dumper
    )
{
    gctUINT i;

    switch(typeId)
    {
        case VIR_TYPE_INT32:
        {
            VIR_LOG(dumper, "int32 const values: ");
            for(i = 0; i < channelCount; i++)
            {
                VIR_LOG(dumper, "%d ", VSC_PH_ResultInstConstOperand_GetNthInt32Val(constOperand, i));
            }
            break;
        }
        case VIR_TYPE_UINT32:
        {
            VIR_LOG(dumper, "uint32 const values: ");
            for(i = 0; i < channelCount; i++)
            {
                VIR_LOG(dumper, "%d ", VSC_PH_ResultInstConstOperand_GetNthUint32Val(constOperand, i));
            }
            break;
        }
        case VIR_TYPE_FLOAT32:
        {
            VIR_LOG(dumper, "float32 const values: ");
            for(i = 0; i < channelCount; i++)
            {
                VIR_LOG(dumper, "%f ", VSC_PH_ResultInstConstOperand_GetNthFloat32Val(constOperand, i));
            }
            break;
        }
        default:
            gcmASSERT(0);
    }

    VIR_LOG(dumper, "\n");
    VIR_LOG_FLUSH(dumper);
}

typedef struct VSC_PH_RESULTINSTOPERAND
{
    gctBOOL isConst;
    gctUINT channelCount;
    VIR_TypeId baseTypeId;
    union
    {
        VSC_PH_ResultInstSymOperand symOperand;
        VSC_PH_ResultInstConstOperand constVal;
    } u;
} VSC_PH_ResultInstOperand;

#define VSC_PH_ResultInstOperand_GetIsConst(rio)                        ((rio)->isConst)
#define VSC_PH_ResultInstOperand_SetIsConst(rio, ic)                    ((rio)->isConst = (ic))
#define VSC_PH_ResultInstOperand_GetChannelCount(rio)                   ((rio)->channelCount)
#define VSC_PH_ResultInstOperand_SetChannelCount(rio, cc)               ((rio)->channelCount = (cc))
#define VSC_PH_ResultInstOperand_GetBaseTypeId(rio)                     ((rio)->baseTypeId)
#define VSC_PH_ResultInstOperand_SetBaseTypeId(rio, bti)                ((rio)->baseTypeId = (bti))
#define VSC_PH_ResultInstOperand_IncChannelCount(rio)                   ((rio)->channelCount++)
#define VSC_PH_ResultInstOperand_GetSymOperand(rio)                     (&(rio)->u.symOperand)
#define VSC_PH_ResultInstOperand_GetConstOperand(rio)                   (&(rio)->u.constVal)

static void
_VSC_PH_ResultInstOperand_Dump(
    VSC_PH_ResultInstOperand* operand,
    VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "channel count: %d\n", VSC_PH_ResultInstOperand_GetChannelCount(operand));
    if(VSC_PH_ResultInstOperand_GetIsConst(operand))
    {
        _VSC_PH_ResultInstConstOperand_Dump(VSC_PH_ResultInstOperand_GetConstOperand(operand),
                                            VSC_PH_ResultInstOperand_GetChannelCount(operand),
                                            VSC_PH_ResultInstOperand_GetBaseTypeId(operand),
                                            dumper);
    }
    else
    {
        _VSC_PH_ResultInstSymOperand_Dump(VSC_PH_ResultInstOperand_GetSymOperand(operand),
                                          VSC_PH_ResultInstOperand_GetChannelCount(operand),
                                          dumper);
    }
}

static void
_VSC_PH_ResultInstOperand_AppendSymOperand(
    IN OUT VSC_PH_ResultInstOperand* operand,
    IN VIR_Instruction* inst,
    IN gctUINT operandNum,
    IN VIR_Operand* opnd,
    IN VIR_Swizzle swizzle
    )
{
    gctUINT channelCount = VSC_PH_ResultInstOperand_GetChannelCount(operand);
    VSC_PH_ResultInstSymOperand* symOperand = VSC_PH_ResultInstOperand_GetSymOperand(operand);
    VIR_TypeId opndTypeId = VIR_Operand_GetTypeId(opnd);
    VIR_TypeId opndBaseTypeId = VIR_GetTypeComponentType(opndTypeId);

    gcmASSERT(channelCount == 0  || !VSC_PH_ResultInstOperand_GetIsConst(operand));
    gcmASSERT(VSC_PH_ResultInstOperand_GetBaseTypeId(operand) == VIR_TYPE_UNKNOWN  || VSC_PH_ResultInstOperand_GetBaseTypeId(operand) == opndBaseTypeId);
    gcmASSERT(((gctUINT8)swizzle & 0x7c) == 0);         /* must be a single channel swizzle */

    VSC_PH_ResultInstSymOperand_SetNthSrcFromInst(symOperand, channelCount, inst);
    VSC_PH_ResultInstSymOperand_SetNthSrcFromOperandNum(symOperand, channelCount, operandNum);
    VSC_PH_ResultInstSymOperand_SetNthSrcFromOperand(symOperand, channelCount, opnd);
    VSC_PH_ResultInstSymOperand_SetNthSrcSwizzle(symOperand, channelCount, swizzle);
    VSC_PH_ResultInstOperand_IncChannelCount(operand);
    VSC_PH_ResultInstOperand_SetIsConst(operand, gcvFALSE);
    VSC_PH_ResultInstOperand_SetBaseTypeId(operand, opndBaseTypeId);
}

static void
_VSC_PH_ResultInstOperand_AppendImm(
    IN OUT VSC_PH_ResultInstOperand* operand,
    IN VIR_TypeId typeId,
    IN gctUINT imm
    )
{
    gctUINT channelCount = VSC_PH_ResultInstOperand_GetChannelCount(operand);
    VSC_PH_ResultInstConstOperand* constVal = VSC_PH_ResultInstOperand_GetConstOperand(operand);

    gcmASSERT(channelCount == 0  || VSC_PH_ResultInstOperand_GetIsConst(operand));
    gcmASSERT(VSC_PH_ResultInstOperand_GetBaseTypeId(operand) == VIR_TYPE_UNKNOWN  || VSC_PH_ResultInstOperand_GetBaseTypeId(operand) == typeId);

    VSC_PH_ResultInstConstOperand_SetNthConvstVal(constVal, channelCount, imm);
    VSC_PH_ResultInstOperand_IncChannelCount(operand);
    VSC_PH_ResultInstOperand_SetIsConst(operand, gcvTRUE);
    VSC_PH_ResultInstOperand_SetBaseTypeId(operand, typeId);
}

typedef struct VSC_PH_ResultInst
{
    VIR_OpCode opcode;
    VSC_PH_ResultInstOperand operands[VIR_MAX_SRC_NUM];
} VSC_PH_ResultInst;

#define VSC_PH_ResultInst_GetOpcode(ri)                         ((ri)->opcode)
#define VSC_PH_ResultInst_SetOpcode(ri, oc)                     ((ri)->opcode = (oc))
#define VSC_PH_ResultInst_GetOperands(ri, i)                    ((ri)->operands)
#define VSC_PH_ResultInst_GetNthOperand(ri, i)                  (&(ri)->operands[i])

static void
_VSC_PH_ResultInst_Dump(
    VSC_PH_ResultInst* resultInst,
    VIR_Dumper* dumper
    )
{
    gctUINT i;

    VIR_LOG(dumper, "ph result inst:\nopcode: %s\n", VIR_OPCODE_GetName(VSC_PH_ResultInst_GetOpcode(resultInst)));
    for(i = 0; i < VIR_OPCODE_GetSrcOperandNum(VSC_PH_ResultInst_GetOpcode(resultInst)); i++)
    {
        VIR_LOG(dumper, "operand %d:\n");
        _VSC_PH_ResultInstOperand_Dump(VSC_PH_ResultInst_GetNthOperand(resultInst, i), dumper);
    }
}


/********************************************** tree transformation functions starts **********************************************/

gctUINT _VSC_PH_Func_InitResultInstOpcode(
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VSC_PH_Tree* tree,
    IN OUT void* dynamicInputOutput,
    IN gctUINT argCount,
    IN gctUINT* args
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VSC_PH_ResultInst* resultInst = (VSC_PH_ResultInst*)dynamicInputOutput;
    VIR_OpCode opcode = (VIR_OpCode)args[0];
    gctUINT i;

    gcmASSERT(argCount == 1);

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_TRANSFORMS))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        gctUINT i;

        VIR_LOG(dumper, "%s got %d parameters:", "_VSC_PH_Func_InitResultInstOpcode", argCount);
        for(i = 0; i < argCount; i++)
        {
            VIR_LOG(dumper, " %x", args[i]);
        }
    }

    VSC_PH_ResultInst_SetOpcode(resultInst, opcode);
    for(i = 0; i < VIR_OPCODE_GetSrcOperandNum(opcode); i++)
    {
        VSC_PH_ResultInstOperand* operand = VSC_PH_ResultInst_GetNthOperand(resultInst, i);
        VSC_PH_ResultInstOperand_SetChannelCount(operand, 0);
    }

    return errCode;
}

gctUINT _VSC_PH_Func_AppendResultInstOperand(
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VSC_PH_Tree* tree,
    IN OUT void* dynamicInputOutput,
    IN gctUINT argCount,
    IN gctUINT* args
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VSC_PH_ResultInst* resultInst = (VSC_PH_ResultInst*)dynamicInputOutput;
    gctUINT resultSrcNum = args[0];
    gctUINT fromNodeID = args[1];
    gctUINT fromSrcNum = args[2];

    gcmASSERT(argCount == 3);
    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_TRANSFORMS))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        gctUINT i;

        VIR_LOG(dumper, "%s got %d parameters:", "_VSC_PH_Func_AppendResultInstOperand", argCount);
        for(i = 0; i < argCount; i++)
        {
            VIR_LOG(dumper, " %x", args[i]);
        }
    }

    {
        VSC_PH_ResultInstOperand* resultOperand = VSC_PH_ResultInst_GetNthOperand(resultInst, resultSrcNum);
        VSC_PH_Tree_Node* fromNode = VSC_PH_Tree_GetNode(tree, fromNodeID);
        VIR_Instruction* fromNodeInst = VSC_PH_Tree_Node_GetInst(fromNode);
        gctUINT fromChannel = VSC_PH_Tree_Node_GetChannel(fromNode);
        VIR_Operand* fromNodeInstSrc = VIR_Inst_GetSource(fromNodeInst, fromSrcNum);
        VIR_Swizzle fromNodeInstSrcSwizzle = VIR_Operand_GetSwizzle(fromNodeInstSrc);
        VIR_Swizzle fromNodeInstSrcChannelSwizzle = VIR_Swizzle_GetChannel(fromNodeInstSrcSwizzle, fromChannel);

        _VSC_PH_ResultInstOperand_AppendSymOperand(resultOperand, fromNodeInst, fromSrcNum, fromNodeInstSrc, fromNodeInstSrcChannelSwizzle);
    }

    return errCode;
}

gctUINT _VSC_PH_Func_AppendResultInstImm(
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VSC_PH_Tree* tree,
    IN OUT void* dynamicInputOutput,
    IN gctUINT argCount,
    IN gctUINT* args
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VSC_PH_ResultInst* resultInst = (VSC_PH_ResultInst*)dynamicInputOutput;
    gctUINT resultSrcNum = args[0];
    VIR_TypeId baseTypeId = (VIR_TypeId)args[1];
    gctUINT imm = args[2];

    gcmASSERT(argCount == 3);
    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_TRANSFORMS))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        gctUINT i;

        VIR_LOG(dumper, "%s got %d parameters:", "_VSC_PH_Func_AppendResultInstImm", argCount);
        for(i = 0; i < argCount; i++)
        {
            VIR_LOG(dumper, " %x", args[i]);
        }
    }

    {
        VSC_PH_ResultInstOperand* resultOperand = VSC_PH_ResultInst_GetNthOperand(resultInst, resultSrcNum);

        _VSC_PH_ResultInstOperand_AppendImm(resultOperand, baseTypeId, imm);
    }

    return errCode;
}

gctUINT _VSC_PH_Func_AppendResultInstImmAsOperand(
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VSC_PH_Tree* tree,
    IN OUT void* dynamicInputOutput,
    IN gctUINT argCount,
    IN gctUINT* args
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VSC_PH_ResultInst* resultInst = (VSC_PH_ResultInst*)dynamicInputOutput;
    gctUINT resultSrcNum = args[0];
    gctUINT fromNodeID = args[1];
    gctUINT fromSrcNum = args[2];

    gcmASSERT(argCount == 3);
    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_TRANSFORMS))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        gctUINT i;

        VIR_LOG(dumper, "%s got %d parameters:", "_VSC_PH_Func_AppendResultInstImmAsOperand", argCount);
        for(i = 0; i < argCount; i++)
        {
            VIR_LOG(dumper, " %x", args[i]);
        }
    }

    {
        VSC_PH_ResultInstOperand* resultOperand = VSC_PH_ResultInst_GetNthOperand(resultInst, resultSrcNum);
        VSC_PH_Tree_Node* fromNode = VSC_PH_Tree_GetNode(tree, fromNodeID);
        VIR_Instruction* fromNodeInst = VSC_PH_Tree_Node_GetInst(fromNode);
        gctUINT fromChannel = VSC_PH_Tree_Node_GetChannel(fromNode);
        VIR_Operand* fromNodeInstSrc = VIR_Inst_GetSource(fromNodeInst, fromSrcNum);
        VIR_Swizzle fromNodeInstSrcSwizzle = VIR_Operand_GetSwizzle(fromNodeInstSrc);
        VIR_Swizzle fromNodeInstSrcChannelSwizzle = VIR_Swizzle_GetChannel(fromNodeInstSrcSwizzle, fromChannel);

        if(VIR_Operand_isImm(fromNodeInstSrc))
        {
            VIR_TypeId baseTypeId = VIR_Operand_GetTypeId(fromNodeInstSrc);
            switch(baseTypeId)
            {
                case VIR_TYPE_INT32:
                {
                    gctINT32 imm = VIR_Operand_GetImmediateInt(fromNodeInstSrc);
                    _VSC_PH_ResultInstOperand_AppendImm(resultOperand, baseTypeId, (gctUINT32)imm);
                    break;
                }
                case VIR_TYPE_UINT32:
                {
                    gctUINT32 imm = VIR_Operand_GetImmediateUint(fromNodeInstSrc);
                    _VSC_PH_ResultInstOperand_AppendImm(resultOperand, baseTypeId, (gctUINT32)imm);
                    break;
                }
                case VIR_TYPE_FLOAT32:
                {
                    gctFLOAT imm = VIR_Operand_GetImmediateFloat(fromNodeInstSrc);
                    _VSC_PH_ResultInstOperand_AppendImm(resultOperand, baseTypeId, *(gctUINT32*)&imm);
                    break;
                }
                default:
                    gcmASSERT(0);
            }
        }
        else if(VIR_Operand_isConst(fromNodeInstSrc))
        {
            VIR_Shader* shader = VSC_PH_Peephole_GetShader(ph);
            VIR_ConstId constId = VIR_Operand_GetConstId(fromNodeInstSrc);
            VIR_Const* constVal = VIR_Shader_GetConstFromId(shader, constId);
            VIR_TypeId constValTypeId = constVal->type;
            VIR_TypeId constValBaseTypeId = VIR_GetTypeComponentType(constValTypeId);

            switch(constValBaseTypeId)
            {
                case VIR_TYPE_INT32:
                {
                    gctINT32 imm = constVal->value.vecVal.i32Value[fromNodeInstSrcChannelSwizzle];
                    _VSC_PH_ResultInstOperand_AppendImm(resultOperand, constValBaseTypeId, (gctUINT32)imm);
                    break;
                }
                case VIR_TYPE_UINT32:
                {
                    gctUINT32 imm = constVal->value.vecVal.u32Value[fromNodeInstSrcChannelSwizzle];
                    _VSC_PH_ResultInstOperand_AppendImm(resultOperand, constValBaseTypeId, (gctUINT32)imm);
                    break;
                }
                case VIR_TYPE_FLOAT32:
                {
                    gctFLOAT imm = constVal->value.vecVal.f32Value[fromNodeInstSrcChannelSwizzle];
                    _VSC_PH_ResultInstOperand_AppendImm(resultOperand, constValBaseTypeId, *(gctUINT32*)&imm);
                    break;
                }
                default:
                    gcmASSERT(0);
            }
        }
    }

    return errCode;
}

gctUINT _VSC_PH_Func_AppendResultInstImmAsTwoOperandsComputation(
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VSC_PH_Tree* tree,
    IN OUT void* dynamicInputOutput,
    IN gctUINT argCount,
    IN gctUINT* args
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VSC_PH_ResultInst* resultInst = (VSC_PH_ResultInst*)dynamicInputOutput;
    gctUINT resultSrcNum = args[0];
    gctUINT fromNodeID0 = args[1];
    gctUINT fromSrcNum0 = args[2];
    gctUINT fromNodeID1= args[3];
    gctUINT fromSrcNum1 = args[4];
    VIR_OpCode opcode = (VIR_OpCode)args[5];

    gcmASSERT(argCount == 6);
    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_TRANSFORMS))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        gctUINT i;

        VIR_LOG(dumper, "%s got %d parameters:", "_VSC_PH_Func_AppendResultInstImmAsTwoOperandsComputation", argCount);
        for(i = 0; i < argCount; i++)
        {
            VIR_LOG(dumper, " %x", args[i]);
        }
    }

    {
        VSC_PH_ResultInstOperand* resultOperand = VSC_PH_ResultInst_GetNthOperand(resultInst, resultSrcNum);
        VSC_PH_Tree_Node* fromNode0 = VSC_PH_Tree_GetNode(tree, fromNodeID0);
        VIR_Instruction* fromNodeInst0 = VSC_PH_Tree_Node_GetInst(fromNode0);
        gctUINT fromChannel0 = VSC_PH_Tree_Node_GetChannel(fromNode0);
        VIR_Operand* fromNodeInstSrc0 = VIR_Inst_GetSource(fromNodeInst0, fromSrcNum0);
        VIR_TypeId fromNodeInstSrcTypeId0 = VIR_Operand_GetTypeId(fromNodeInstSrc0);
        VIR_TypeId fromNodeInstSrcBaseTypeId0 = VIR_GetTypeComponentType(fromNodeInstSrcTypeId0);
        VIR_Swizzle fromNodeInstSrcSwizzle0 = VIR_Operand_GetSwizzle(fromNodeInstSrc0);
        VIR_Swizzle fromNodeInstSrcChannelSwizzle0 = VIR_Swizzle_GetChannel(fromNodeInstSrcSwizzle0, fromChannel0);
        VSC_PH_Tree_Node* fromNode1 = VSC_PH_Tree_GetNode(tree, fromNodeID1);
        VIR_Instruction* fromNodeInst1 = VSC_PH_Tree_Node_GetInst(fromNode1);
        gctUINT fromChannel1 = VSC_PH_Tree_Node_GetChannel(fromNode1);
        VIR_Operand* fromNodeInstSrc1 = VIR_Inst_GetSource(fromNodeInst1, fromSrcNum1);
        VIR_TypeId fromNodeInstSrcTypeId1 = VIR_Operand_GetTypeId(fromNodeInstSrc1);
        VIR_TypeId fromNodeInstSrcBaseTypeId1 = VIR_GetTypeComponentType(fromNodeInstSrcTypeId1);
        VIR_Swizzle fromNodeInstSrcSwizzle1 = VIR_Operand_GetSwizzle(fromNodeInstSrc1);
        VIR_Swizzle fromNodeInstSrcChannelSwizzle1 = VIR_Swizzle_GetChannel(fromNodeInstSrcSwizzle1, fromChannel1);

        gctUINT32 imm0 = 0, imm1 = 0;


        if(VIR_Operand_isImm(fromNodeInstSrc0))
        {
            imm0 = VIR_Operand_GetImmediateInt(fromNodeInstSrc0);
        }
        else if(VIR_Operand_isConst(fromNodeInstSrc0))
        {
            VIR_Shader* shader = VSC_PH_Peephole_GetShader(ph);
            VIR_ConstId constId0 = VIR_Operand_GetConstId(fromNodeInstSrc0);
            VIR_Const* constVal0 = VIR_Shader_GetConstFromId(shader, constId0);

            imm0 = constVal0->value.vecVal.u32Value[fromNodeInstSrcChannelSwizzle0];
        }
        else
        {
            gcmASSERT(0);
        }

        if(VIR_Operand_isImm(fromNodeInstSrc1))
        {
            imm1 = VIR_Operand_GetImmediateInt(fromNodeInstSrc1);
        }
        else if(VIR_Operand_isConst(fromNodeInstSrc1))
        {
            VIR_Shader* shader = VSC_PH_Peephole_GetShader(ph);
            VIR_ConstId constId1 = VIR_Operand_GetConstId(fromNodeInstSrc1);
            VIR_Const* constVal1 = VIR_Shader_GetConstFromId(shader, constId1);

            imm1 = constVal1->value.vecVal.u32Value[fromNodeInstSrcChannelSwizzle1];
        }
        else
        {
            gcmASSERT(0);
        }

        gcmASSERT(fromNodeInstSrcBaseTypeId0 == fromNodeInstSrcBaseTypeId1 ||
                  (fromNodeInstSrcBaseTypeId0 == VIR_TYPE_INT32 && fromNodeInstSrcBaseTypeId1 == VIR_TYPE_UINT32) ||
                  (fromNodeInstSrcBaseTypeId0 == VIR_TYPE_UINT32 && fromNodeInstSrcBaseTypeId1 == VIR_TYPE_INT32));

        switch(fromNodeInstSrcBaseTypeId0)
        {
            case VIR_TYPE_INT32:
            {
                gctINT32 imm0i32 = (gctINT32)imm0;
                gctINT32 imm1i32 = (gctINT32)imm1;
                gctINT32 result = 0;

                switch(opcode)
                {
                    case VIR_OP_ADD:
                        result = imm0i32 + imm1i32;
                        break;
                    case VIR_OP_SUB:
                        result = imm0i32 - imm1i32;
                        break;
                    case VIR_OP_MUL:
                        result = imm0i32 * imm1i32;
                        break;
                    case VIR_OP_DIV:
                        result = imm0i32 / imm1i32;
                        break;
                    default:
                        gcmASSERT(0);
                }
                _VSC_PH_ResultInstOperand_AppendImm(resultOperand, VIR_TYPE_INT32, (gctUINT32)result);
                break;
            }
            case VIR_TYPE_UINT32:
            {
                if(fromNodeInstSrcBaseTypeId1 == VIR_TYPE_INT32)
                {
                    gctINT32 imm0i32 = (gctINT32)imm0;
                    gctINT32 imm1i32 = (gctINT32)imm1;
                    gctINT32 result = 0;

                    switch(opcode)
                    {
                        case VIR_OP_ADD:
                            result = imm0i32 + imm1i32;
                            break;
                        case VIR_OP_SUB:
                            result = imm0i32 - imm1i32;
                            break;
                        case VIR_OP_MUL:
                            result = imm0i32 * imm1i32;
                            break;
                        case VIR_OP_DIV:
                            result = imm0i32 / imm1i32;
                            break;
                        default:
                            gcmASSERT(0);
                    }
                    _VSC_PH_ResultInstOperand_AppendImm(resultOperand, VIR_TYPE_INT32, (gctUINT32)result);
                }
                else
                {
                    gctUINT32 imm0u32 = (gctUINT32)imm0;
                    gctUINT32 imm1u32 = (gctUINT32)imm1;
                    gctUINT32 result = 0;

                    switch(opcode)
                    {
                        case VIR_OP_ADD:
                            result = imm0u32 + imm1u32;
                            break;
                        case VIR_OP_SUB:
                            result = imm0u32 - imm1u32;
                            break;
                        case VIR_OP_MUL:
                            result = imm0u32 * imm1u32;
                            break;
                        case VIR_OP_DIV:
                            result = imm0u32 / imm1u32;
                            break;
                        default:
                            gcmASSERT(0);
                    }
                    _VSC_PH_ResultInstOperand_AppendImm(resultOperand, VIR_TYPE_UINT32, (gctUINT32)result);
                }
                break;
            }
            case VIR_TYPE_FLOAT32:
            {
                gctFLOAT imm0f32 = *(gctFLOAT*)&imm0;
                gctFLOAT imm1f32 = *(gctFLOAT*)&imm1;
                gctFLOAT result = 0.0;

                switch(opcode)
                {
                    case VIR_OP_ADD:
                        result = imm0f32 + imm1f32;
                        break;
                    case VIR_OP_SUB:
                        result = imm0f32 - imm1f32;
                        break;
                    case VIR_OP_MUL:
                        result = imm0f32 * imm1f32;
                        break;
                    case VIR_OP_DIV:
                        result = imm0f32 / imm1f32;
                        break;
                    default:
                        gcmASSERT(0);
                }
                _VSC_PH_ResultInstOperand_AppendImm(resultOperand, VIR_TYPE_FLOAT32, *(gctUINT32*)&result);
                break;
            }
            default:
                gcmASSERT(0);
        }
    }

    return errCode;
}

/********************************************** tree transformation functions end **********************************************/

#define VSC_PH_STEP_OPER_MAX_ARG_COUNT 6
typedef struct VSC_PH_OPER
{
    VSC_PH_FuncP func;
    gctUINT expected;
    gctUINT argCount;
    gctUINT arguments[VSC_PH_STEP_OPER_MAX_ARG_COUNT];
} VSC_PH_Oper;

#define VSC_PH_Oper_GetFunc(o)                                              ((o)->func)
#define VSC_PH_Oper_GetExpected(o)                                          ((o)->expected)
#define VSC_PH_Oper_GetArgCount(o)                                          ((o)->argCount)
#define VSC_PH_Oper_GetArguments(o)                                         ((o)->arguments)

static gctBOOL VSC_PH_Oper_PerformBuildRequirements(
    IN VSC_PH_Oper* oper,
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VSC_PH_Tree* tree,
    IN OUT void* dynamicInputOutput
    )
{
    VSC_PH_FuncP func = VSC_PH_Oper_GetFunc(oper);
    gctUINT expected = VSC_PH_Oper_GetExpected(oper);
    gctUINT argCount = VSC_PH_Oper_GetArgCount(oper);
    gctUINT* arguments = VSC_PH_Oper_GetArguments(oper);
    gctUINT result = (*func)(ph, tree, dynamicInputOutput, argCount, arguments);

    return expected == result;
}

static gctBOOL VSC_PH_Oper_PerformTransRequirements(
    IN VSC_PH_Oper* oper,
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VSC_PH_Tree* tree,
    IN OUT void* dynamicInputOutput
    )
{
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VSC_PH_FuncP func = VSC_PH_Oper_GetFunc(oper);
    gctUINT expected = VSC_PH_Oper_GetExpected(oper);
    gctUINT argCount = VSC_PH_Oper_GetArgCount(oper);
    gctUINT* arguments = VSC_PH_Oper_GetArguments(oper);
    gctUINT result = (*func)(ph, tree, dynamicInputOutput, argCount, arguments);

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_REQUIREMENTS))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);

        VIR_LOG(dumper, ", got output %x. expected result is %x\n", result, expected);
    }
    return expected == result;
}

static gctBOOL VSC_PH_Oper_PerformTransformations(
    IN VSC_PH_Oper* oper,
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VSC_PH_Tree* tree,
    IN OUT void* dynamicInputOutput
    )
{
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VSC_PH_FuncP func = VSC_PH_Oper_GetFunc(oper);
    gctUINT expected = VSC_PH_Oper_GetExpected(oper);
    gctUINT argCount = VSC_PH_Oper_GetArgCount(oper);
    gctUINT* arguments = VSC_PH_Oper_GetArguments(oper);
    gctUINT result = (*func)(ph, tree, dynamicInputOutput, argCount, arguments);

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_TRANSFORMS))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);

        VIR_LOG(dumper, ", got output %x. expected result is %x\n", result, expected);
    }
    return expected == result;
}

typedef struct VSC_PH_STEP
{
    gctUINT endDist;
    VSC_PH_Oper oper;
} VSC_PH_Step;

#define VSC_PH_Step_GetEndDist(s)                                           ((s)->endDist)
#define VSC_PH_Step_GetOper(s)                                              (&(s)->oper)

static VSC_PH_Step _VSC_PH_Add_Steps[] = {
    {1, {_VSC_PH_Func_NodeLevelLessThan, gcvTRUE, 1, {VSC_PH_TREE_MAX_LEVEL}}},
    /*
        ADD t0, s.x, s.y
        ADD t1, t0, s.z

        DP3 t1, s.xyz, 1.0
    */
    {6, {_VSC_PH_Func_GetNodeSourceBaseTypeId, VIR_TYPE_FLOAT32, 2, {0, 1}}},
    {5, {_VSC_PH_Func_GetNodeOpCode, VIR_OP_ADD, 1, {1}}},
    {4, {_VSC_PH_Func_TwoSourcesHavingTheSameSym, gcvTRUE, 4, {0, 1, 1, 0}}},
    {3, {_VSC_PH_Func_TwoSourcesHavingTheSameSym, gcvTRUE, 4, {0, 1, 1, 1}}},
    {2, {_VSC_PH_Func_SourceHavingDefBeforeRoot, gcvFALSE, 2, {1, 0}}},
    {1, {_VSC_PH_Func_SourceHavingDefBeforeRoot, gcvFALSE, 2, {1, 1}}},
    {7, {_VSC_PH_Func_InitResultInstOpcode, VSC_ERR_NONE, 1, {VIR_OP_DP3}}},
    {6, {_VSC_PH_Func_AppendResultInstOperand, VSC_ERR_NONE, 3, {0, 1, 0}}},
    {5, {_VSC_PH_Func_AppendResultInstOperand, VSC_ERR_NONE, 3, {0, 1, 1}}},
    {4, {_VSC_PH_Func_AppendResultInstOperand, VSC_ERR_NONE, 3, {0, 0, 1}}},
    {3, {_VSC_PH_Func_AppendResultInstImm, VSC_ERR_NONE, 3, {1, VIR_TYPE_FLOAT32, 0x3f800000 /* 1.0 */}}},
    {2, {_VSC_PH_Func_AppendResultInstImm, VSC_ERR_NONE, 3, {1, VIR_TYPE_FLOAT32, 0x3f800000 /* 1.0 */}}},
    {1, {_VSC_PH_Func_AppendResultInstImm, VSC_ERR_NONE, 3, {1, VIR_TYPE_FLOAT32, 0x3f800000 /* 1.0 */}}},
    {0, {0}}
};


static VSC_PH_Step _VSC_PH_Mad_Steps[] = {
    {1, {_VSC_PH_Func_NodeLevelLessThan, gcvTRUE, 1, {1}}},
    /*
        MUL t0, s0, c0
        MAD t1, t0, c1, s1

        MAD t1, s0, (c0 * c1), s1
    */
    {4, {_VSC_PH_Func_GetNodeOpCode, VIR_OP_MUL, 1, {1}}},
    {3, {_VSC_PH_Func_SourceIsImm, gcvTRUE, 2, {0, 1}}},
    {2, {_VSC_PH_Func_SourceIsImm, gcvTRUE, 2, {1, 1}}},
    {1, {_VSC_PH_Func_SourceHavingDefBeforeRoot, gcvFALSE, 2, {1, 0}}},
    {4, {_VSC_PH_Func_InitResultInstOpcode, VSC_ERR_NONE, 1, {VIR_OP_MAD}}},
    {3, {_VSC_PH_Func_AppendResultInstOperand, VSC_ERR_NONE, 3, {0, 1, 0}}},
    {2, {_VSC_PH_Func_AppendResultInstImmAsTwoOperandsComputation, VSC_ERR_NONE, 6, {1, 0, 1, 1, 1, VIR_OP_MUL}}},
    {1, {_VSC_PH_Func_AppendResultInstOperand, VSC_ERR_NONE, 3, {2, 0, 2}}},
    {0, {0}}
};

static VSC_PH_Step* _VSC_PH_GetOpSteps(
    VIR_OpCode opcode
    )
{
    switch(opcode)
    {
        case VIR_OP_ADD:
            return _VSC_PH_Add_Steps;
        case VIR_OP_MAD:
            return _VSC_PH_Mad_Steps;
        default:
            return gcvNULL;
    }
}

static void _VSC_PH_BuildSubTree(
    IN OUT VSC_PH_Peephole* ph,
    IN VIR_Instruction* inst,
    IN gctUINT channel,
    IN VSC_PH_Step* steps,
    IN OUT VSC_PH_Tree* tree,
    IN OUT VSC_PH_Tree_Node* subRoot
    )
{
    VIR_BB* bb = VIR_Inst_GetBasicBlock(inst);
    VSC_PH_Step* currentStep = steps;
    gctBOOL buildReqMet = gcvTRUE;

    VSC_PH_Tree_Node_SetInst(subRoot, inst);
    VSC_PH_Tree_Node_Setchannel(subRoot, channel);

    if(inst == BB_GET_START_INST(bb))
    {
        return;
    }

    while(gcvTRUE)
    {
        VSC_PH_Oper* oper = VSC_PH_Step_GetOper(currentStep);

        buildReqMet = VSC_PH_Oper_PerformBuildRequirements(oper, ph, tree, subRoot);
        if(!buildReqMet || VSC_PH_Step_GetEndDist(currentStep) == 1)
        {
            break;
        }
        currentStep++;
    }

    if(buildReqMet)
    {
        gctUINT i;

        for(i = 0; i < VIR_Inst_GetSrcNum(inst) && i < VSC_PH_TREE_N; i++)
        {
            VIR_Operand* src = VIR_Inst_GetSource(inst, i);
            if(VIR_Operand_GetOpKind(src) == VIR_OPND_SYMBOL || VIR_Operand_GetOpKind(src) == VIR_OPND_SAMPLER_INDEXING)
            {
                VIR_Swizzle srcSwizzle = VIR_Operand_GetSwizzle(src);
                VIR_Swizzle srcChannelSwizzle = VIR_Swizzle_GetChannel(srcSwizzle, channel);
                VIR_Enable srcChannelEnable = (VIR_Enable)(1 << (gctUINT)srcChannelSwizzle);
                VIR_Instruction* srcDefInst = gcvNULL;
                VIR_Instruction* prev = VIR_Inst_GetPrev(inst);

                while(gcvTRUE)
                {
                    VIR_OpCode prevOpcode = VIR_Inst_GetOpcode(prev);

                    if(VIR_OPCODE_hasDest(prevOpcode))
                    {
                        VIR_Operand* prevDest = VIR_Inst_GetDest(prev);
                        VIR_Enable prevDestEnable = VIR_Operand_GetEnable(prevDest);

                        if(prevDestEnable & srcChannelEnable &&
                            VIR_Operand_SameSymbol(prevDest, src))
                        {
                            srcDefInst = prev;
                            break;
                        }
                    }
                    if(prev == BB_GET_START_INST(bb))
                    {
                        break;
                    }
                    prev = VIR_Inst_GetPrev(prev);
                }
                if(srcDefInst)
                {
                    _VSC_PH_BuildSubTree(ph, srcDefInst, (gctUINT)srcChannelSwizzle, steps, tree, VSC_PH_Tree_GetNthChildFromNode(tree, subRoot, i));
                }
            }
        }
    }
}

static VSC_ErrCode _VSC_PH_BuildTree(
    IN VSC_PH_Peephole* ph,
    IN VIR_Instruction* inst,
    IN gctUINT channel,
    IN VSC_PH_Step* steps,
    IN OUT VSC_PH_Tree* tree
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VSC_PH_Tree_Node* root;

    root = VSC_PH_Tree_GetRoot(tree);
    _VSC_PH_BuildSubTree(ph, inst, channel, steps, tree, root);

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_BUILT_TREE))
    {
        _VSC_PH_Tree_Dump(tree, VSC_PH_Peephole_GetDumper(ph));
    }

    return errCode;
}

typedef struct VSC_PH_RESULTINSTS
{
    VSC_PH_ResultInst resultInsts[VIR_CHANNEL_COUNT];
} VSC_PH_ResultInsts;

#define VSC_PH_ResultInstsGetNthInst(ris, i)                    (&(ris)->resultInsts[i])
#define VSC_PH_ResultInstsGetMthInstOpcode(ris, i)              (VSC_PH_ResultInst_GetOpcode(&(ris)->resultInsts[i]))
#define VSC_PH_ResultInstsGetMthInstNthOpnd(ris, i, j)          (VSC_PH_ResultInst_GetNthOperand(&(ris)->resultInsts[i], j))

static gctBOOL VSC_PH_ResultInsts_CanMerge(
    IN VSC_PH_Peephole* ph,
    IN VSC_PH_ResultInsts* resultInsts,
    IN VIR_Instruction* inst
    )
{
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
    VIR_OpCode mergedOpcode = VIR_OP_NOP;
    VIR_Operand* instDest = VIR_Inst_GetDest(inst);
    VIR_Enable instDestEnable = VIR_Operand_GetEnable(instDest);
    gctUINT channel;
    gctBOOL canMerge = gcvTRUE;
    gctINT firstChannel = -1;

    for(channel = 0; channel < VIR_CHANNEL_COUNT; channel++)
    {
        if(instDestEnable & (1 << channel))
        {
            VSC_PH_ResultInst* currentResultInst = VSC_PH_ResultInstsGetNthInst(resultInsts, channel);

            if(VSC_PH_ResultInst_GetOpcode(currentResultInst) == VIR_OP_NOP)
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MERGING))
                {
                    VIR_LOG(dumper, "channel %d has not result inst. \n", channel);
                }
                canMerge = gcvFALSE;
                break;
            }

            if(mergedOpcode == VIR_OP_NOP)
            {
                mergedOpcode = VSC_PH_ResultInst_GetOpcode(currentResultInst);
            }
            else if(mergedOpcode != VSC_PH_ResultInst_GetOpcode(currentResultInst))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MERGING))
                {
                    VIR_LOG(dumper, "different opcodes. \n");
                }
                canMerge = gcvFALSE;
                break;
            }

            if(firstChannel == -1)
            {
                firstChannel = channel;
            }
            else if(!VIR_OPCODE_isComponentwise(mergedOpcode))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MERGING))
                {
                    VIR_LOG(dumper, "multiple result insts(%d, %d), not componentwise. \n", firstChannel, channel);
                }
                canMerge = gcvFALSE;
                break;
            }
            else
            {
                VSC_PH_ResultInst* firstChannelInst = VSC_PH_ResultInstsGetNthInst(resultInsts, firstChannel);
                gctUINT srcNum;
                gctBOOL srcMatch = gcvTRUE;

                gcmASSERT(mergedOpcode != VIR_OP_NOP);
                for(srcNum = 0; srcNum < VIR_OPCODE_GetSrcOperandNum(mergedOpcode); srcNum++)
                {
                    VSC_PH_ResultInstOperand* firstChannelInstOperand = VSC_PH_ResultInst_GetNthOperand(firstChannelInst, srcNum);
                    VSC_PH_ResultInstOperand* currentChannelInstOperand = VSC_PH_ResultInst_GetNthOperand(currentResultInst, srcNum);

                    gcmASSERT(VSC_PH_ResultInstOperand_GetChannelCount(firstChannelInstOperand) == 1);
                    gcmASSERT(VSC_PH_ResultInstOperand_GetChannelCount(currentChannelInstOperand) == 1);

                    if(VSC_PH_ResultInstOperand_GetIsConst(firstChannelInstOperand) != VSC_PH_ResultInstOperand_GetIsConst(currentChannelInstOperand))
                    {
                        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MERGING))
                        {
                            VIR_LOG(dumper, "source(%d's %d) dosen't match source(%d's %d). \n", firstChannel, srcNum, channel, srcNum);
                        }
                        srcMatch = gcvFALSE;
                        break;
                    }

                    if(VSC_PH_ResultInstOperand_GetBaseTypeId(firstChannelInstOperand) != VSC_PH_ResultInstOperand_GetBaseTypeId(currentChannelInstOperand))
                    {
                        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MERGING))
                        {
                            VIR_LOG(dumper, "source(%d's %d) dosen't match source(%d's %d). \n", firstChannel, srcNum, channel, srcNum);
                        }
                        srcMatch = gcvFALSE;
                        break;
                    }

                    if(!VSC_PH_ResultInstOperand_GetIsConst(firstChannelInstOperand))
                    {
                        VSC_PH_ResultInstSymOperand* firstChannelInstSymOperand = VSC_PH_ResultInstOperand_GetSymOperand(firstChannelInstOperand);
                        VSC_PH_ResultInstSymOperand* currentChannelInstSymOperand = VSC_PH_ResultInstOperand_GetSymOperand(currentChannelInstOperand);

                        if(!VIR_Operand_SameIndexedSymbol(VSC_PH_ResultInstSymOperand_GetNthSrcFromOperand(firstChannelInstSymOperand, 0),
                                                   VSC_PH_ResultInstSymOperand_GetNthSrcFromOperand(currentChannelInstSymOperand, 0)))
                        {
                            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MERGING))
                            {
                                VIR_LOG(dumper, "source(%d's %d) dosen't match source(%d's %d). \n", firstChannel, srcNum, channel, srcNum);
                            }
                            srcMatch = gcvFALSE;
                            break;
                        }
                    }
                }

                if(!srcMatch)
                {
                    canMerge = gcvFALSE;
                    break;
                }
            }
        }
    }

    return mergedOpcode != VIR_OP_NOP && canMerge;
}

static VSC_ErrCode _VSC_PH_MergeAndAddResultInsts(
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VIR_Instruction* inst,
    IN OUT VSC_PH_ResultInsts* resultInsts
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_DEF_USAGE_INFO* duInfo = VSC_PH_Peephole_GetDUInfo(ph);
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
    VIR_OpCode mergedOpcode;
    VIR_Operand* instDest = VIR_Inst_GetDest(inst);
    VIR_Enable instDestEnable = VIR_Operand_GetEnable(instDest);
    VIR_TypeId instDestTypeId = VIR_Operand_GetTypeId(instDest);
    VIR_Shader* shader = VSC_PH_Peephole_GetShader(ph);
    VIR_Function* func = VIR_Shader_GetCurrentFunction(shader);
    VIR_Instruction* newInst = gcvNULL;
    VIR_Operand* newInstDest;
    gctUINT channel;
    gctINT firstHandledChannel = -1;

    gctUINT constMatrix[VIR_MAX_SRC_NUM][VIR_CHANNEL_COUNT];
    gctUINT constLength[VIR_MAX_SRC_NUM] = {0};
    gctUINT constBaseTypeId[VIR_MAX_SRC_NUM] = {0};
    VIR_Swizzle constSwizzle[VIR_MAX_SRC_NUM] = {VIR_SWIZZLE_XXXX};

    for(channel = 0; channel < VIR_CHANNEL_COUNT; channel++)
    {
        if(instDestEnable & (1 << channel))
        {
            break;
        }
    }

    mergedOpcode = VSC_PH_ResultInstsGetMthInstOpcode(resultInsts, channel);
    VIR_Function_AddInstructionAfter(func, mergedOpcode, instDestTypeId, inst, gcvTRUE, &newInst);
    newInstDest = VIR_Inst_GetDest(newInst);
    VIR_Operand_Copy(newInstDest, instDest);
    /* du update for newInstDest */
    {
        VIR_OperandInfo newInstDestInfo;
        VIR_GENERAL_DU_ITERATOR instDuIter;
        VIR_USAGE* instUsage;
        VIR_NATIVE_DEF_FLAGS nativeDefFlags;

        VIR_Operand_GetOperandInfo(newInst, newInstDest, &newInstDestInfo);
        memset(&nativeDefFlags, 0, sizeof(nativeDefFlags));
        nativeDefFlags.bIsOutput = newInstDestInfo.isOutput;
        vscVIR_AddNewDef(duInfo,
                         newInst,
                         newInstDestInfo.u1.virRegInfo.startVirReg,
                         newInstDestInfo.u1.virRegInfo.virRegCount,
                         instDestEnable,
                         VIR_HALF_CHANNEL_MASK_FULL,
                         &nativeDefFlags,
                         gcvNULL);

        for(channel = 0; channel < VIR_CHANNEL_NUM; channel++)
        {
            if(!(instDestEnable & (1 << channel)))
            {
                continue;
            }

            vscVIR_InitGeneralDuIterator(&instDuIter, duInfo, inst, newInstDestInfo.u1.virRegInfo.virReg, (VIR_Enable)channel, gcvFALSE);

            for(instUsage = vscVIR_GeneralDuIterator_First(&instDuIter); instUsage != gcvNULL;
                instUsage = vscVIR_GeneralDuIterator_Next(&instDuIter))
            {
                VIR_Instruction* instUsageInst = instUsage->usageKey.pUsageInst;
                VIR_Operand* instUsageInstOperand = instUsage->usageKey.pOperand;

                vscVIR_AddNewUsageToDef(duInfo,
                                        newInst,
                                        instUsageInst,
                                        instUsageInstOperand,
                                        instUsage->usageKey.bIsIndexingRegUsage,
                                        newInstDestInfo.u1.virRegInfo.startVirReg,
                                        newInstDestInfo.u1.virRegInfo.virRegCount,
                                        (VIR_Enable)(1 << channel),
                                        VIR_HALF_CHANNEL_MASK_FULL,
                                        gcvNULL);
            }
        }
    }

    for(channel = 0; channel < VIR_CHANNEL_COUNT; channel++)
    {
        if(instDestEnable & (1 << channel))
        {
            VSC_PH_ResultInst* resultInst = VSC_PH_ResultInstsGetNthInst(resultInsts, channel);
            gctUINT srcNum;

            for(srcNum = 0; srcNum < VIR_OPCODE_GetSrcOperandNum(mergedOpcode); srcNum++)
            {
                VSC_PH_ResultInstOperand* resultInstOperand = VSC_PH_ResultInst_GetNthOperand(resultInst, srcNum);
                VIR_Operand* newInstOperand = VIR_Inst_GetSource(newInst, srcNum);

                if(VIR_OPCODE_isComponentwise(mergedOpcode))
                {
                    gcmASSERT(VSC_PH_ResultInstOperand_GetChannelCount(resultInstOperand) == 1);
                    if(VSC_PH_ResultInstOperand_GetIsConst(resultInstOperand))
                    {
                        VSC_PH_ResultInstConstOperand* constOperand = VSC_PH_ResultInstOperand_GetConstOperand(resultInstOperand);
                        gctUINT i;

                        constBaseTypeId[srcNum] = VSC_PH_ResultInstOperand_GetBaseTypeId(resultInstOperand);
                        /* skip duplicated const */
                        for(i = 0; i < constLength[srcNum]; i++)
                        {
                            if(constMatrix[srcNum][i] == VSC_PH_ResultInstConstOperand_GetNthUint32Val(constOperand, 0))
                            {
                                VIR_Swizzle_SetChannel(constSwizzle[srcNum], channel, i);
                                break;
                            }
                        }
                        if(i == constLength[srcNum])
                        {
                            constMatrix[srcNum][constLength[srcNum]] = VSC_PH_ResultInstConstOperand_GetNthUint32Val(constOperand, 0);
                            VIR_Swizzle_SetChannel(constSwizzle[srcNum], channel, constLength[srcNum]);
                            constLength[srcNum]++;
                        }

                        if(firstHandledChannel == -1)
                        {
                            VIR_Operand_SetTypeId(newInstOperand,
                                                VIR_TypeId_ComposeNonOpaqueType(VSC_PH_ResultInstOperand_GetBaseTypeId(resultInstOperand),
                                                                                VIR_Enable_Channel_Count(instDestEnable),
                                                                                1));
                        }
                    }
                    else
                    {
                        VSC_PH_ResultInstSymOperand* symOperand = VSC_PH_ResultInstOperand_GetSymOperand(resultInstOperand);
                        VIR_Operand* fromOperand = VSC_PH_ResultInstSymOperand_GetNthSrcFromOperand(symOperand, 0);
                        VIR_Swizzle swizzle = VSC_PH_ResultInstSymOperand_GetNthSrcSwizzle(symOperand, 0);
                        VIR_Swizzle newSwizzle;

                        if(firstHandledChannel == -1)
                        {
                            VIR_Operand_Copy(newInstOperand, fromOperand);
                            VIR_Operand_SetTypeId(newInstOperand,
                                                VIR_TypeId_ComposeNonOpaqueType(VSC_PH_ResultInstOperand_GetBaseTypeId(resultInstOperand),
                                                                                VIR_Enable_Channel_Count(instDestEnable),
                                                                                1));
                        }

                        newSwizzle = VIR_Operand_GetSwizzle(newInstOperand);
                        VIR_Swizzle_SetChannel(newSwizzle, channel, swizzle);
                        VIR_Operand_SetSwizzle(newInstOperand, newSwizzle);
                        /* du update for newInstOperand */
                        {
                            VIR_Instruction* fromInst = VSC_PH_ResultInstSymOperand_GetNthSrcFromInst(symOperand, 0);
                            VIR_OperandInfo newInstOperandInfo;
                            VIR_GENERAL_UD_ITERATOR fromOperandUdIter;
                            VIR_DEF* fromOperandDef;

                            VIR_Operand_GetOperandInfo(fromInst, fromOperand, &newInstOperandInfo);
                            vscVIR_InitGeneralUdIterator(&fromOperandUdIter, duInfo, fromInst, fromOperand, gcvFALSE, gcvFALSE);

                            for(fromOperandDef = vscVIR_GeneralUdIterator_First(&fromOperandUdIter); fromOperandDef != gcvNULL;
                                fromOperandDef = vscVIR_GeneralUdIterator_Next(&fromOperandUdIter))
                            {
                                VIR_Instruction* fromOperandDefInst = fromOperandDef->defKey.pDefInst;

                                vscVIR_AddNewUsageToDef(duInfo,
                                                        fromOperandDefInst,
                                                        newInst,
                                                        newInstOperand,
                                                        gcvFALSE,
                                                        newInstOperandInfo.u1.virRegInfo.startVirReg,
                                                        newInstOperandInfo.u1.virRegInfo.virRegCount,
                                                        (VIR_Enable)(1 << swizzle),
                                                        VIR_HALF_CHANNEL_MASK_FULL,
                                                        gcvNULL);
                            }
                            if(newInstOperandInfo.indexingVirRegNo != VIR_INVALID_REG_NO)
                            {
                                vscVIR_InitGeneralUdIterator(&fromOperandUdIter, duInfo, fromInst, fromOperand, gcvTRUE, gcvFALSE);

                                for(fromOperandDef = vscVIR_GeneralUdIterator_First(&fromOperandUdIter); fromOperandDef != gcvNULL;
                                    fromOperandDef = vscVIR_GeneralUdIterator_Next(&fromOperandUdIter))
                                {
                                    VIR_Instruction* fromOperandDefInst = fromOperandDef->defKey.pDefInst;

                                    vscVIR_AddNewUsageToDef(duInfo,
                                                            fromOperandDefInst,
                                                            newInst,
                                                            newInstOperand,
                                                            gcvTRUE,
                                                            newInstOperandInfo.indexingVirRegNo,
                                                            1,
                                                            (VIR_Enable)(newInstOperandInfo.indexingKind),
                                                            VIR_HALF_CHANNEL_MASK_FULL,
                                                            gcvNULL);
                                }
                            }
                        }
                    }
                }
                else
                {
                    if(VSC_PH_ResultInstOperand_GetIsConst(resultInstOperand))
                    {
                        VSC_PH_ResultInstConstOperand* constOperand = VSC_PH_ResultInstOperand_GetConstOperand(resultInstOperand);
                        gctUINT i;

                        for(i = 0; i < VSC_PH_ResultInstOperand_GetChannelCount(resultInstOperand); i++)
                        {
                            gctUINT j;

                            constBaseTypeId[srcNum] = VSC_PH_ResultInstOperand_GetBaseTypeId(resultInstOperand);
                            /* skip duplicated const */
                            for(j = 0; j < i; j++)
                            {
                                if(constMatrix[srcNum][j] == VSC_PH_ResultInstConstOperand_GetNthUint32Val(constOperand, i))
                                {
                                    VIR_Swizzle_SetChannel(constSwizzle[srcNum], i, j);
                                    break;
                                }
                            }
                            if(j == i)
                            {
                                constMatrix[srcNum][constLength[srcNum]] = VSC_PH_ResultInstConstOperand_GetNthUint32Val(constOperand, i);
                                VIR_Swizzle_SetChannel(constSwizzle[srcNum], i, constLength[srcNum]);
                                constLength[srcNum]++;
                            }
                        }

                    }
                    else
                    {
                        VSC_PH_ResultInstSymOperand* symOperand = VSC_PH_ResultInstOperand_GetSymOperand(resultInstOperand);
                        gctUINT i;

                        for(i = 0; i < VSC_PH_ResultInstOperand_GetChannelCount(resultInstOperand); i++)
                        {
                            VIR_Operand* fromOperand = VSC_PH_ResultInstSymOperand_GetNthSrcFromOperand(symOperand, i);
                            VIR_Swizzle swizzle = VSC_PH_ResultInstSymOperand_GetNthSrcSwizzle(symOperand, i);
                            VIR_Swizzle newSwizzle;

                            if(i == 0)
                            {
                                VIR_Operand_Copy(newInstOperand, fromOperand);
                            }

                            newSwizzle = VIR_Operand_GetSwizzle(newInstOperand);
                            VIR_Swizzle_SetChannel(newSwizzle, i, swizzle);
                            VIR_Operand_SetSwizzle(newInstOperand, newSwizzle);
                            /* du update for newInstOperand */
                            {
                                VIR_Instruction* fromInst = VSC_PH_ResultInstSymOperand_GetNthSrcFromInst(symOperand, i);
                                VIR_OperandInfo newInstOperandInfo;
                                VIR_GENERAL_UD_ITERATOR fromOperandUdIter;
                                VIR_DEF* fromOperandDef;

                                VIR_Operand_GetOperandInfo(fromInst, fromOperand, &newInstOperandInfo);
                                vscVIR_InitGeneralUdIterator(&fromOperandUdIter, duInfo, fromInst, fromOperand, gcvFALSE, gcvFALSE);

                                for(fromOperandDef = vscVIR_GeneralUdIterator_First(&fromOperandUdIter); fromOperandDef != gcvNULL;
                                    fromOperandDef = vscVIR_GeneralUdIterator_Next(&fromOperandUdIter))
                                {
                                    VIR_Instruction* fromOperandDefInst = fromOperandDef->defKey.pDefInst;

                                    vscVIR_AddNewUsageToDef(duInfo,
                                                            fromOperandDefInst,
                                                            newInst,
                                                            newInstOperand,
                                                            gcvFALSE,
                                                            newInstOperandInfo.u1.virRegInfo.startVirReg,
                                                            newInstOperandInfo.u1.virRegInfo.virRegCount,
                                                            (VIR_Enable)(1 << swizzle),
                                                            VIR_HALF_CHANNEL_MASK_FULL,
                                                            gcvNULL);
                                }
                                if(newInstOperandInfo.indexingVirRegNo != VIR_INVALID_REG_NO)
                                {
                                    vscVIR_InitGeneralUdIterator(&fromOperandUdIter, duInfo, fromInst, fromOperand, gcvTRUE, gcvFALSE);

                                    for(fromOperandDef = vscVIR_GeneralUdIterator_First(&fromOperandUdIter); fromOperandDef != gcvNULL;
                                        fromOperandDef = vscVIR_GeneralUdIterator_Next(&fromOperandUdIter))
                                    {
                                        VIR_Instruction* fromOperandDefInst = fromOperandDef->defKey.pDefInst;

                                        vscVIR_AddNewUsageToDef(duInfo,
                                                                fromOperandDefInst,
                                                                newInst,
                                                                newInstOperand,
                                                                gcvTRUE,
                                                                newInstOperandInfo.indexingVirRegNo,
                                                                1,
                                                                (VIR_Enable)(newInstOperandInfo.indexingKind),
                                                                VIR_HALF_CHANNEL_MASK_FULL,
                                                                gcvNULL);
                                    }
                                }
                            }
                        }
                    }
                    VIR_Operand_SetTypeId(newInstOperand,
                                        VIR_TypeId_ComposeNonOpaqueType(VSC_PH_ResultInstOperand_GetBaseTypeId(resultInstOperand),
                                                                        VSC_PH_ResultInstOperand_GetChannelCount(resultInstOperand),
                                                                        1));
                }
            }

            if(firstHandledChannel == -1)
            {
                firstHandledChannel = channel;
            }
        }
    }

    /* set src constant value */
    {
        gctUINT srcNum;
        for(srcNum = 0; srcNum < VIR_OPCODE_GetSrcOperandNum(mergedOpcode); srcNum++)
        {
            VIR_Operand* newInstSrc = VIR_Inst_GetSource(newInst, srcNum);

            if(constLength[srcNum] > 0)
            {
                if(constLength[srcNum] == 1)
                {
                    switch(constBaseTypeId[srcNum])
                    {
                        case VIR_TYPE_INT32:
                            VIR_Operand_SetImmediateInt(newInstSrc, (gctINT)constMatrix[srcNum][0]);
                            break;
                        case VIR_TYPE_UINT32:
                            VIR_Operand_SetImmediateUint(newInstSrc, constMatrix[srcNum][0]);
                            break;
                        case VIR_TYPE_FLOAT32:
                            VIR_Operand_SetImmediateFloat(newInstSrc, *(gctFLOAT*)&constMatrix[srcNum][0]);
                            break;
                        default:
                            gcmASSERT(0);
                    }
                }
                else
                {

                }
            }
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_OUTPUT_INST))
    {
        VIR_LOG(dumper, "new instrucion:\n");
        VIR_Inst_Dump(dumper, newInst);
    }

    return errCode;
}

static VSC_ErrCode _VSC_PH_MoveSingleInstBefore(
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VIR_Function*    func,
    IN OUT VIR_Instruction* beforeMe,
    IN OUT VIR_Instruction* inst
    )
{
    return VIR_Pass_MoveInstructionBefore(func, beforeMe, inst, &VSC_PH_Peephole_GetCfgChanged(ph));
}

static VSC_ErrCode _VSC_PH_RemoveSingleInst(
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VIR_Function*    func,
    IN OUT VIR_Instruction* inst
    )
{
    return VIR_Pass_RemoveInstruction(func, inst, &VSC_PH_Peephole_GetCfgChanged(ph));
}

static VSC_ErrCode _VSC_PH_RemoveInst(
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VIR_Instruction* inst
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_DEF_USAGE_INFO* duInfo = VSC_PH_Peephole_GetDUInfo(ph);
    VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);

    if(VIR_OPCODE_hasDest(opcode))
    {
        VIR_Operand* instDest = VIR_Inst_GetDest(inst);
        VIR_Enable instDestEnable = VIR_Operand_GetEnable(instDest);
        VIR_OperandInfo instDestInfo;
        gctUINT8 channel;

        VIR_Operand_GetOperandInfo(inst, instDest, &instDestInfo);

        for(channel = 0; channel < VIR_CHANNEL_COUNT; channel++)
        {
            if(instDestEnable & (1 << channel))
            {
                vscVIR_DeleteDef(duInfo, inst, instDestInfo.u1.virRegInfo.startVirReg, instDestInfo.u1.virRegInfo.virRegCount, (VIR_Enable)(1 << channel), VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
            }
        }
    }

    {
        gctUINT i;

        for(i = 0; i < VIR_OPCODE_GetSrcOperandNum(opcode); i++)
        {
            VIR_Operand* instSrc = VIR_Inst_GetSource(inst, i);
            VIR_Swizzle instSrcSwizzle = VIR_Operand_GetSwizzle(instSrc);
            VIR_Enable instSrcEnable = VIR_Swizzle_2_Enable(instSrcSwizzle);
            VIR_OperandInfo instSrcInfo;
            VIR_GENERAL_UD_ITERATOR instSrcUdIter;
            VIR_DEF* instSrcDef;

            VIR_Operand_GetOperandInfo(inst, instSrc, &instSrcInfo);
            vscVIR_InitGeneralUdIterator(&instSrcUdIter, duInfo, inst, instSrc, gcvFALSE, gcvFALSE);

            for(instSrcDef = vscVIR_GeneralUdIterator_First(&instSrcUdIter); instSrcDef != gcvNULL; instSrcDef = vscVIR_GeneralUdIterator_Next(&instSrcUdIter))
            {
                VIR_Instruction* instSrcDefInst = instSrcDef->defKey.pDefInst;

                vscVIR_DeleteUsage(duInfo,
                                   instSrcDefInst,
                                   inst,
                                   instSrc,
                                   gcvFALSE,
                                   instSrcInfo.u1.virRegInfo.startVirReg,
                                   instSrcInfo.u1.virRegInfo.virRegCount,
                                   instSrcEnable,
                                   VIR_HALF_CHANNEL_MASK_FULL,
                                   gcvNULL);
            }
            if(instSrcInfo.indexingVirRegNo != VIR_INVALID_REG_NO)
            {
                vscVIR_InitGeneralUdIterator(&instSrcUdIter, duInfo, inst, instSrc, gcvTRUE, gcvFALSE);

                for(instSrcDef = vscVIR_GeneralUdIterator_First(&instSrcUdIter); instSrcDef != gcvNULL; instSrcDef = vscVIR_GeneralUdIterator_Next(&instSrcUdIter))
                {
                    VIR_Instruction* instSrcDefInst = instSrcDef->defKey.pDefInst;

                    vscVIR_DeleteUsage(duInfo,
                                       instSrcDefInst,
                                       inst,
                                       instSrc,
                                       gcvTRUE,
                                       instSrcInfo.indexingVirRegNo,
                                       1,
                                       (VIR_Enable)instSrcInfo.indexingKind,
                                       VIR_HALF_CHANNEL_MASK_FULL,
                                       gcvNULL);
                }
            }
        }
    }

    {
        VIR_Shader* shader = VSC_PH_Peephole_GetShader(ph);
        VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
        VIR_Function* func = VIR_Shader_GetCurrentFunction(shader);

        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_OUTPUT_INST))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);

            VIR_LOG(dumper, "removed instrucion:\n");
            VIR_Inst_Dump(dumper, inst);
        }
        _VSC_PH_RemoveSingleInst(ph, func, inst);
    }
    return errCode;
}

static VSC_ErrCode _VSC_PH_PerformOnInst(
    IN OUT VSC_PH_Peephole* ph,
    IN OUT VIR_Instruction* inst
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);
    VSC_PH_Step* steps = _VSC_PH_GetOpSteps(opcode);
    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);

    if(steps)
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_INPUT_INST))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);

            VIR_LOG(dumper, "start peephole optimization for inst:\n");
            VIR_Inst_Dump(dumper, inst);
        }

        if(VIR_Inst_isComponentwise(inst))
        {
            VIR_Operand* dest = VIR_Inst_GetDest(inst);
            VIR_Enable enable = VIR_Operand_GetEnable(dest);
            gctUINT channel;
            VSC_PH_ResultInsts resultInsts;

            gcoOS_MemFill(&resultInsts, 0, sizeof(VSC_PH_ResultInsts));
            for(channel = 0; channel < VIR_CHANNEL_COUNT; channel++)
            {
                VSC_PH_Step* currentStep = steps;
                VIR_Enable channelEnable = (VIR_Enable)(1 << channel);
                if(enable & channelEnable)
                {
                    VSC_PH_Tree tree = {
                                        {
                                         {0, gcvNULL},
                                         {1, gcvNULL},
                                         {2, gcvNULL},
                                         {3, gcvNULL},
                                         {4, gcvNULL},
                                         {5, gcvNULL},
                                         {6, gcvNULL},
                                         {7, gcvNULL},
                                         {8, gcvNULL},
                                         {9, gcvNULL},
                                         {10, gcvNULL},
                                         {11, gcvNULL},
                                         {12, gcvNULL},
                                         {13, gcvNULL},
                                         {14, gcvNULL}
                                        }
                                       };
                    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_BUILT_TREE) ||
                       VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_REQUIREMENTS) ||
                       VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_TRANSFORMS))
                    {
                        VIR_LOG(dumper, "for channel %d:\n", channel);
                    }

                    _VSC_PH_BuildTree(ph, inst, channel, currentStep, &tree);
                    currentStep += VSC_PH_Step_GetEndDist(currentStep);
                    while(VSC_PH_Step_GetEndDist(currentStep))
                    {
                        gctBOOL reqMet = gcvTRUE;

                        while(gcvTRUE)
                        {
                            VSC_PH_Oper* oper = VSC_PH_Step_GetOper(currentStep);
                            reqMet = VSC_PH_Oper_PerformTransRequirements(oper, ph, &tree, gcvNULL);
                            if(!reqMet || VSC_PH_Step_GetEndDist(currentStep) == 1)
                            {
                                currentStep++;
                                break;
                            }
                            currentStep++;
                        }

                        if(reqMet)
                        {
                            while(gcvTRUE)
                            {
                                VSC_PH_Oper* oper = VSC_PH_Step_GetOper(currentStep);
#if gcmIS_DEBUG(gcdDEBUG_ASSERT)
                                gctBOOL result = VSC_PH_Oper_PerformTransformations(oper, ph, &tree, VSC_PH_ResultInstsGetNthInst(&resultInsts, channel));
                                gcmASSERT(result);
#else
                                VSC_PH_Oper_PerformTransformations(oper, ph, &tree, VSC_PH_ResultInstsGetNthInst(&resultInsts, channel));
#endif
                                if(VSC_PH_Step_GetEndDist(currentStep) == 1)
                                {
                                    break;
                                }
                                currentStep++;
                            }

                            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_TRANSFORMS))
                            {
                                _VSC_PH_ResultInst_Dump(VSC_PH_ResultInstsGetNthInst(&resultInsts, channel), dumper);
                            }

                            /* one transformation is enough */
                            break;
                        }
                        else
                        {
                            currentStep += VSC_PH_Step_GetEndDist(currentStep);
                            currentStep += VSC_PH_Step_GetEndDist(currentStep);
                        }
                    }
                }
            }

            if(VSC_PH_ResultInsts_CanMerge(ph, &resultInsts, inst))
            {
                _VSC_PH_MergeAndAddResultInsts(ph, inst, &resultInsts);
                _VSC_PH_RemoveInst(ph, inst);
                VSC_PH_Peephole_SetExprChanged(ph, gcvTRUE);
            }
        }
    }

    return errCode;
}

static VSC_ErrCode _VSC_PH_PerformOnBB(
    IN OUT VSC_PH_Peephole* ph
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_BB* bb = VSC_PH_Peephole_GetCurrBB(ph);
    VIR_Instruction* inst = BB_GET_START_INST(bb);

    if(inst)
    {
        while(gcvTRUE)
        {
            _VSC_PH_PerformOnInst(ph, inst);
            if(inst == BB_GET_END_INST(bb))
            {
                break;
            }
            inst = VIR_Inst_GetNext(inst);
        }
    }

    return errCode;
}

/* new PH code end */

static gctBOOL _VSC_PH_DoesOpcodeSupportLValueModifier(
    IN VIR_OpCode opcode
    )
{
    if (VIR_OPCODE_isTexLd(opcode) ||
        VIR_OPCODE_isMemLd(opcode)  ||
        VIR_OPCODE_isAttrLd(opcode) ||
        VIR_OPCODE_isVX(opcode))
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
    VIR_NATIVE_DEF_FLAGS nativeDefFlags;

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
    ON_ERROR(_VSC_PH_InitHashTable(ph, &VSC_PH_Peephole_WorkSet(ph), vscHFUNC_Default, vscHKCMP_Default, 512),
             "Failed to initialize Hashtable");
    work_set = VSC_PH_Peephole_WorkSet(ph);
    vscVIR_InitGeneralUdIterator(&ud_iter, VSC_PH_Peephole_GetDUInfo(ph), inst, inst_src0, gcvFALSE, gcvFALSE);
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
        if(!_VSC_PH_DoesOpcodeSupportLValueModifier(VIR_Inst_GetOpcode(def_inst))
           ||
           (VSC_PH_ModifierToGen_GetDefVerifyFunc(mtg) && !VSC_PH_ModifierToGen_GetDefVerifyFunc(mtg)(def_inst)))
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
            if(!vscVIR_IsUniqueUsageInstOfDefInst(VSC_PH_Peephole_GetDUInfo(ph), def_inst, inst, gcvNULL,
                                                  gcvFALSE, &breaking_use, gcvNULL, gcvNULL))
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

            ON_ERROR(_VSC_PH_InitHashTable(ph, &VSC_PH_Peephole_UsageSet(ph), _VSC_PH_OpndTarget_HFUNC, _VSC_PH_OpndTarget_HKCMP, 512),
                     "Failed to initialize Hashtable");
            inst_usage_set = VSC_PH_Peephole_UsageSet(ph);
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
                VIR_Operand_ReplaceDefOperandWithDef(work_inst_dest, inst_dest, new_work_inst_enable, gcvFALSE);

                memset(&nativeDefFlags, 0, sizeof(nativeDefFlags));
                nativeDefFlags.bIsInput = inst_dest_info.isInput;
                nativeDefFlags.bIsOutput = inst_dest_info.isOutput;
                vscVIR_AddNewDef(VSC_PH_Peephole_GetDUInfo(ph), work_inst, inst_dest_info.u1.virRegInfo.virReg, 1,
                                 new_work_inst_enable, VIR_HALF_CHANNEL_MASK_FULL, &nativeDefFlags, gcvNULL);

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

                        vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph), inst, inst_usage_inst, inst_usage_opnd, gcvFALSE,
                                           inst_dest_info.u1.virRegInfo.virReg, 1,
                                           (VIR_Enable)(enable & inst_dest_enable), VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
                        if(new_work_inst_enable & enable)
                            vscVIR_AddNewUsageToDef(VSC_PH_Peephole_GetDUInfo(ph), work_inst, inst_usage_inst, inst_usage_opnd, gcvFALSE,
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
                           inst_src0, gcvFALSE, inst_src0_info.u1.virRegInfo.virReg, 1,
                           inst_src0_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        /* delete the def info of the dest of input inst */
        vscVIR_DeleteDef(VSC_PH_Peephole_GetDUInfo(ph), inst, inst_dest_info.u1.virRegInfo.virReg,
                         1, inst_dest_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        /* remove the input inst */
        _VSC_PH_RemoveSingleInst(ph, func, inst);
        _VSC_PH_ResetHashTable(inst_usage_set);
    }

OnError:
    _VSC_PH_ResetHashTable(work_set);

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

    ON_ERROR(_VSC_PH_InitHashTable(ph, &VSC_PH_Peephole_WorkSet(ph), _VSC_PH_OpndTarget_HFUNC, _VSC_PH_OpndTarget_HKCMP, 512),
             "Failed to initialize Hashtable");
    work_set = VSC_PH_Peephole_WorkSet(ph);
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

            if (VSC_PH_ModifierToGen_GetVerifyUsageFunc(mtg) &&
                !VSC_PH_ModifierToGen_GetVerifyUsageFunc(mtg)(inst_src0, usage_inst))
            {
                invalid_case = gcvTRUE;
                break;
            }

            /* 1) no EVIS instruction support neg, abs, sat
             * 2) except iADD which support neg
             * 3) and except BiLinear which support neg and abs for Src2
             */
            if (VIR_OPCODE_isVX(opcode) &&
                (VIR_Inst_GetOpcode(inst) == VIR_OP_ABS || VIR_Inst_GetOpcode(inst) == VIR_OP_NEG) &&
                !(opcode == VIR_OP_VX_IADD && VIR_Inst_GetOpcode(inst) == VIR_OP_NEG) &&
                !(opcode == VIR_OP_VX_BILINEAR && usage_opnd == VIR_Inst_GetSource(usage_inst, 2)))
            {
                invalid_case = gcvTRUE;
                break;
            }
            /* conjugate should only be propagated to complex instructions */
            if(VIR_Inst_GetOpcode(inst) == VIR_OP_CONJ &&
               !VIR_OPCODE_isCmplx(VIR_Inst_GetOpcode(usage_inst)))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MODIFIER))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "not processed because use instr is not a complex instruction:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, usage_inst);
                    VIR_LOG_FLUSH(dumper);
                }
                invalid_case = gcvTRUE;
                break;
            }

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
                if(!vscVIR_IsUniqueDefInstOfUsageInst(VSC_PH_Peephole_GetDUInfo(ph), usage_inst, usage_opnd,
                                                      usage->usageKey.bIsIndexingRegUsage, inst, &breaking_def))
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
                VIR_Instruction* next = inst;
                while(next && next != usage_inst)
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

            ON_ERROR(_VSC_PH_InitHashTable(ph, &VSC_PH_Peephole_DefSet(ph), vscHFUNC_Default, vscHKCMP_Default, 512),
                     "Failed to initialize Hashtable");
            def_inst_set = VSC_PH_Peephole_DefSet(ph);

            vscVIR_InitGeneralUdIterator(&inst_ud_iter, VSC_PH_Peephole_GetDUInfo(ph), inst, inst_src0, gcvFALSE, gcvFALSE);
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
                if (VSC_PH_ModifierToGen_GetModifier(mtg) == VIR_MOD_NEG)
                {
                    VIR_Operand_NegateOperand(shader, work_opnd);
                }
                else
                {
                    VIR_Operand_SetModifier(work_opnd, VSC_PH_ModifierToGen_GetModifier(mtg));
                }
                vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph), VIR_ANY_DEF_INST, work_inst,
                                   work_opnd, gcvFALSE, inst_dest_info.u1.virRegInfo.virReg, 1,
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
                            vscVIR_AddNewUsageToDef(VSC_PH_Peephole_GetDUInfo(ph), def_inst, work_inst, work_opnd, gcvFALSE,
                                                    inst_src0_info.u1.virRegInfo.virReg, 1,
                                                    (VIR_Enable)(enable & work_inst_enable), VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
                        }
                    }
                }
            }
        }

        vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph), VIR_ANY_DEF_INST,
                           inst, inst_src0, gcvFALSE, inst_src0_info.u1.virRegInfo.virReg,
                           1, inst_src0_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        vscVIR_DeleteDef(VSC_PH_Peephole_GetDUInfo(ph), inst, inst_dest_info.u1.virRegInfo.virReg,
                         1, inst_dest_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        _VSC_PH_RemoveSingleInst(ph, func, inst);
        _VSC_PH_ResetHashTable(def_inst_set);
    }

OnError:
    _VSC_PH_ResetHashTable(work_set);
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
        VIR_TypeId src0_typeid = VIR_Operand_GetTypeId(src0);
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
    VSC_HASH_TABLE* add_sub_set   = gcvNULL;
    VSC_HASH_TABLE* def_inst_set0 = gcvNULL;
    VSC_HASH_TABLE* def_inst_set1 = gcvNULL;
    gctBOOL invalid_case = gcvFALSE;
    gctBOOL is_mulsat = VIR_Inst_GetOpcode(mul) == VIR_OP_MULSAT;
    gctBOOL bIsFloatInst = gcvFALSE;

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

    /* Skip if this chip can't support the floating MAD. */
    bIsFloatInst = VIR_TypeId_isFloat(VIR_Operand_GetTypeId(mul_dest));
    if (!VSC_PH_Peephole_GetHwCfg(ph)->hwFeatureFlags.hasFloatingMadFix && bIsFloatInst)
    {
        if (generated != gcvNULL)
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

    ON_ERROR(_VSC_PH_InitHashTable(ph, &VSC_PH_Peephole_WorkSet(ph), _VSC_PH_OpndTarget_HFUNC, _VSC_PH_OpndTarget_HKCMP, 512),
             "Failed to initialize Hashtable");
    add_sub_set = VSC_PH_Peephole_WorkSet(ph);
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

            /* the result of mul must be of the same basic type of its usage in add/sub */
            if(VIR_GetTypeComponentType(VIR_Operand_GetTypeId(mul_dest)) != VIR_GetTypeComponentType(VIR_Operand_GetTypeId(use_inst_opnd)))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MAD))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "not processed because of its usage in:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, use_inst);
                    VIR_LOG(dumper, "\nis not of the same basic type of the def\n");
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
                if(!vscVIR_IsUniqueDefInstOfUsageInst(VSC_PH_Peephole_GetDUInfo(ph), use_inst, use_inst_opnd,
                                                      usage->usageKey.bIsIndexingRegUsage, mul, &breaking_def))
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
                VIR_Instruction* next = mul;
                while(next && next != use_inst)
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
        VSC_HASH_ITERATOR iter;
        VSC_DIRECT_HNODE_PAIR pair;
        VIR_Swizzle mapping_swizzle0 = VIR_Enable_GetMappingSwizzle(mul_enable, mul_src0_swizzle);
        VIR_Swizzle mapping_swizzle1 = VIR_Enable_GetMappingSwizzle(mul_enable, mul_src1_swizzle);

        ON_ERROR(_VSC_PH_InitHashTable(ph, &VSC_PH_Peephole_DefSet0(ph), vscHFUNC_Default, vscHKCMP_Default, 512),
                 "Failed to initialize Hashtable");
        def_inst_set0 = VSC_PH_Peephole_DefSet0(ph);
        ON_ERROR(_VSC_PH_InitHashTable(ph, &VSC_PH_Peephole_DefSet1(ph), vscHFUNC_Default, vscHKCMP_Default, 512),
                 "Failed to initialize Hashtable");
        def_inst_set1 = VSC_PH_Peephole_DefSet1(ph);
        /* collect the def of the src0 and src1 of the mul inst, and delete the usage between mul's src0/src1's def and mul's src0/src1 */
        {
            VIR_GENERAL_UD_ITERATOR inst_ud_iter;
            VIR_DEF* def;

            vscVIR_InitGeneralUdIterator(&inst_ud_iter, VSC_PH_Peephole_GetDUInfo(ph), mul, mul_src0, gcvFALSE, gcvFALSE);
            for(def = vscVIR_GeneralUdIterator_First(&inst_ud_iter); def != gcvNULL;
                def = vscVIR_GeneralUdIterator_Next(&inst_ud_iter))
            {
                VIR_Instruction* def_inst = def->defKey.pDefInst;
                vscHTBL_DirectSet(def_inst_set0, (void*)def_inst, gcvNULL);
            }

            vscVIR_InitGeneralUdIterator(&inst_ud_iter, VSC_PH_Peephole_GetDUInfo(ph), mul, mul_src1, gcvFALSE, gcvFALSE);
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
                               mul_dest_usage, gcvFALSE, mul_dest_info.u1.virRegInfo.virReg, 1,
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
            VIR_Function_FreeOperand(func, mul_dest_usage);

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
                        vscVIR_AddNewUsageToDef(VSC_PH_Peephole_GetDUInfo(ph), def_inst, add_sub, mad_src0, gcvFALSE,
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
                        vscVIR_AddNewUsageToDef(VSC_PH_Peephole_GetDUInfo(ph), def_inst, add_sub, mad_src1, gcvFALSE,
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
                               mul_src0, gcvFALSE, mul_src0_info.u1.virRegInfo.virReg, 1,
                               mul_src0_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
            vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph), VIR_ANY_DEF_INST, mul,
                               mul_src1, gcvFALSE, mul_src1_info.u1.virRegInfo.virReg, 1,
                               mul_src1_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        }
        /* remove the def of mul */
        {
            vscVIR_DeleteDef(VSC_PH_Peephole_GetDUInfo(ph), mul, mul_dest_info.u1.virRegInfo.virReg,
                             1, mul_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        }
        _VSC_PH_RemoveSingleInst(ph, func, mul);
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

OnError:
    _VSC_PH_ResetHashTable(add_sub_set);
    _VSC_PH_ResetHashTable(def_inst_set0);
    _VSC_PH_ResetHashTable(def_inst_set1);
   return errCode;
}

/*
** a = b + c
** d = c - a
**  -->
** a = b + c
** d = -b
*/
typedef struct _VSC_PH_ADD_SRC_INFO
{
    VIR_Operand*    pSrcOpnd;
    gctBOOL         bIsNegative;
} VSC_PH_AddSrcInfo;

static VSC_ErrCode _VSC_PH_MergeAddSubSameValue(
    VSC_PH_Peephole*        pPh,
    VIR_Instruction*        pFirstAddInst,
    gctBOOL*                pChanged
    )
{
    VSC_ErrCode             errCode  = VSC_ERR_NONE;
    VIR_Shader*             pShader = VSC_PH_Peephole_GetShader(pPh);
    VIR_DEF_USAGE_INFO*     pDuInfo = VSC_PH_Peephole_GetDUInfo(pPh);
    gctBOOL                 bChanged = gcvFALSE;
    VSC_PH_AddSrcInfo       firstInstSrcInfo[3], secondInstSrcInfo[2];
    VIR_OpCode              firstOpcode = VIR_Inst_GetOpcode(pFirstAddInst);
    VIR_OperandInfo         opndInfo;
    VIR_Operand*            pFirstInstDestOpnd = VIR_Inst_GetDest(pFirstAddInst);
    VIR_Enable              firstInstEnable = VIR_Operand_GetEnable(pFirstInstDestOpnd);
    gctUINT                 i;
    VSC_HASH_TABLE*         pAddSubSet = gcvNULL;
    gctUINT8                channel;
    VIR_GENERAL_DU_ITERATOR du_iter;
    VSC_HASH_ITERATOR       hashIter;
    VSC_DIRECT_HNODE_PAIR   nodePair;
    VIR_SrcOperand_Iterator opndIter;
    VIR_Operand*            pNextOpnd;

    gcmASSERT(firstOpcode == VIR_OP_ADD || firstOpcode == VIR_OP_SUB || firstOpcode == VIR_OP_MAD);

    /* Get the operand information of the first DEST. */
    VIR_Operand_GetOperandInfo(pFirstAddInst, pFirstInstDestOpnd, &opndInfo);

    /* Get the source information of the first ADD instruction. */
    for (i = 0; i < VIR_Inst_GetSrcNum(pFirstAddInst); i++)
    {
        firstInstSrcInfo[i].pSrcOpnd = VIR_Inst_GetSource(pFirstAddInst, i);

        if (firstOpcode != VIR_OP_SUB || i == 0)
        {
            firstInstSrcInfo[i].bIsNegative = (VIR_Operand_GetModifier(firstInstSrcInfo[i].pSrcOpnd) & VIR_MOD_NEG) != 0;
        }
        else
        {
            firstInstSrcInfo[i].bIsNegative = (VIR_Operand_GetModifier(firstInstSrcInfo[i].pSrcOpnd) & VIR_MOD_NEG) == 0;
        }
    }

    errCode = _VSC_PH_InitHashTable(pPh,
                                    &VSC_PH_Peephole_WorkSet(pPh),
                                    _VSC_PH_OpndTarget_HFUNC,
                                    _VSC_PH_OpndTarget_HKCMP, 16);
    ON_ERROR(errCode, "Failed to initialize the hash table.");
    pAddSubSet = VSC_PH_Peephole_WorkSet(pPh);

    /* get usage of MUL's dest in per channel way*/
    for(channel = 0; channel < VIR_CHANNEL_NUM; channel++)
    {
        VIR_USAGE*              pUsage;

        if(!(firstInstEnable & (1 << channel)))
        {
            continue;
        }

        /* Get all the usage, now only check the usages within the same BB. */
        vscVIR_InitGeneralDuIterator(&du_iter,
                                     pDuInfo,
                                     pFirstAddInst,
                                     opndInfo.u1.virRegInfo.virReg,
                                     channel,
                                     gcvTRUE);
        for (pUsage = vscVIR_GeneralDuIterator_First(&du_iter);
             pUsage != gcvNULL;
             pUsage = vscVIR_GeneralDuIterator_Next(&du_iter))
        {
            VIR_Instruction*    pUsageInst;
            VIR_Instruction*    pTempInst;
            VIR_Operand*        pUsageOpnd;
            VIR_OpCode          usageOpcode;
            gctBOOL             bSrcNeg[2], bRedefined = gcvFALSE;
            gctUINT             usageSrcIndex = 0xFFFFFFFF, oppositeSrcIndex = 0, matchSrcIndex = 0;
            VSC_PH_OpndTarget*  pOpndTarget = gcvNULL;

            if (vscHTBL_DirectTestAndGet(pAddSubSet, (void*)&(pUsage->usageKey), gcvNULL))
            {
                continue;
            }

            /* Skip no matched instruction. */
            if (pUsage->usageKey.bIsIndexingRegUsage)
            {
                continue;
            }
            pUsageInst = pUsage->usageKey.pUsageInst;
            if (VIR_IS_OUTPUT_USAGE_INST(pUsageInst))
            {
                continue;
            }
            usageOpcode = VIR_Inst_GetOpcode(pUsageInst);
            if (usageOpcode != VIR_OP_ADD && usageOpcode != VIR_OP_SUB)
            {
                continue;
            }
            pUsageOpnd = pUsage->usageKey.pOperand;

            /* Get the source information of the second ADD instruction. */
            for (i = 0; i < 2; i++)
            {
                secondInstSrcInfo[i].pSrcOpnd = VIR_Inst_GetSource(pUsageInst, i);

                if (secondInstSrcInfo[i].pSrcOpnd == pUsageOpnd)
                {
                    usageSrcIndex = i;

                    oppositeSrcIndex = ((i == 0) ? 1 : 0);
                }

                if (usageOpcode == VIR_OP_ADD || i == 0)
                {
                    secondInstSrcInfo[i].bIsNegative = (VIR_Operand_GetModifier(secondInstSrcInfo[i].pSrcOpnd) & VIR_MOD_NEG) != 0;
                }
                else
                {
                    secondInstSrcInfo[i].bIsNegative = (VIR_Operand_GetModifier(secondInstSrcInfo[i].pSrcOpnd) & VIR_MOD_NEG) == 0;
                }
            }
            if (usageSrcIndex == 0xFFFFFFFF)
            {
                continue;
            }

            /* Don't check NEG modifier here. */
            if (!VIR_Operand_Identical(pFirstInstDestOpnd, pUsageOpnd, pShader, gcvTRUE))
            {
                continue;
            }

            /*
            ** The other source of the second ADD must be the source of the first ADD,
            ** and these two sources must have the different sign bit.
            */
            for (i = 0; i < VIR_Inst_GetSrcNum(pFirstAddInst); i++)
            {
                if (firstOpcode == VIR_OP_MAD && i != 2)
                {
                    continue;
                }

                if (!VIR_Operand_Identical(firstInstSrcInfo[i].pSrcOpnd, secondInstSrcInfo[oppositeSrcIndex].pSrcOpnd, pShader, gcvTRUE))
                {
                    continue;
                }

                bSrcNeg[0] = firstInstSrcInfo[i].bIsNegative;
                bSrcNeg[1] = secondInstSrcInfo[oppositeSrcIndex].bIsNegative;

                if (secondInstSrcInfo[usageSrcIndex].bIsNegative)
                {
                    bSrcNeg[0] = !bSrcNeg[0];
                }

                if (bSrcNeg[0] == bSrcNeg[1])
                {
                    continue;
                }
                else
                {
                    matchSrcIndex = i;
                    break;
                }
            }
            if (i == VIR_Inst_GetSrcNum(pFirstAddInst))
            {
                continue;
            }

            /* in case of loop, the first ADD should be the only def of the second ADD usage. */
            if (!vscVIR_IsUniqueDefInstOfUsageInst(pDuInfo,
                                                   pUsageInst,
                                                   pUsageOpnd,
                                                   pUsage->usageKey.bIsIndexingRegUsage,
                                                   pFirstAddInst,
                                                   gcvNULL))
            {
                continue;
            }

            /* there should be no def of first ADD's SRC between ADD and usage_inst */
            pTempInst = pFirstAddInst;
            while(pTempInst && pTempInst != pUsageInst)
            {
                VIR_SrcOperand_Iterator_Init(pFirstAddInst, &opndIter);

                for (pNextOpnd = VIR_SrcOperand_Iterator_First(&opndIter);
                     pNextOpnd != gcvNULL;
                     pNextOpnd = VIR_SrcOperand_Iterator_Next(&opndIter))
                {
                    if (VIR_Operand_SameLocation(pFirstAddInst,
                                                 pNextOpnd,
                                                 pTempInst,
                                                 VIR_Inst_GetDest(pTempInst)))
                    {
                        bRedefined = gcvTRUE;
                        break;
                    }
                }
                if (bRedefined)
                {
                    break;
                }
                pTempInst = VIR_Inst_GetNext(pTempInst);
            }
            if (bRedefined)
            {
                continue;
            }

            pOpndTarget = _VSC_PH_Peephole_NewOpndTarget(pPh, pUsage);
            pOpndTarget->pPrivData = (void*)(gctUINTPTR_T)matchSrcIndex;
            vscHTBL_DirectSet(pAddSubSet, (void*)pOpndTarget, gcvNULL);
        }
    }

    /* No match case, just return. */
    if (HTBL_GET_ITEM_COUNT(pAddSubSet) == 0)
    {
        _VSC_PH_ResetHashTable(pAddSubSet);
        return errCode;
    }

    /* Start to merge the code. */
    vscHTBLIterator_Init(&hashIter, pAddSubSet);
    for (nodePair = vscHTBLIterator_DirectFirst(&hashIter);
         IS_VALID_DIRECT_HNODE_PAIR(&nodePair);
         nodePair = vscHTBLIterator_DirectNext(&hashIter))
    {
        VSC_PH_OpndTarget*      pOpndTarget = (VSC_PH_OpndTarget*)VSC_DIRECT_HNODE_PAIR_FIRST(&nodePair);
        VIR_Instruction*        pUsageInst = pOpndTarget->inst;
        VIR_Operand*            pUsageOpnd = pOpndTarget->opnd;
        VIR_OpCode              usageOpcode = VIR_Inst_GetOpcode(pUsageInst);
        gctUINT                 matchSrcIndex = (gctUINT)(gctUINTPTR_T)pOpndTarget->pPrivData;
        gctBOOL                 bInverse = gcvFALSE;
        VIR_GENERAL_UD_ITERATOR inst_ud_iter;
        VIR_DEF*                pDef;

        if (usageOpcode == VIR_OP_ADD || (pUsageOpnd == VIR_Inst_GetSource(pUsageInst, 0)))
        {
            bInverse = (VIR_Operand_GetModifier(pUsageOpnd) & VIR_MOD_NEG) != 0;
        }
        else
        {
            bInverse = (VIR_Operand_GetModifier(pUsageOpnd) & VIR_MOD_NEG) == 0;
        }

        /*
        ** a = b + c
        ** d = c - a
        **  -->
        ** a = b + c
        ** d = -b
        **  Or
        ** a = b * d + c
        ** d = c - a
        **  -->
        ** a = b * d + c
        ** d = -b * d
        */

        /* Delete the usage. */
        for (i = 0; i < VIR_Inst_GetSrcNum(pUsageInst); i++)
        {
            /* Get the operand information. */
            VIR_Operand_GetOperandInfo(pUsageInst, VIR_Inst_GetSource(pUsageInst, i), &opndInfo);

            if (opndInfo.isVreg)
            {
                vscVIR_DeleteUsage(pDuInfo,
                                   VIR_ANY_DEF_INST,
                                   pUsageInst,
                                   VIR_Inst_GetSource(pUsageInst, i),
                                   gcvFALSE,
                                   opndInfo.u1.virRegInfo.virReg,
                                   1,
                                   VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(VIR_Inst_GetSource(pUsageInst, i))),
                                   VIR_HALF_CHANNEL_MASK_FULL,
                                   gcvNULL);
            }
        }

        if (firstOpcode == VIR_OP_MAD)
        {
            /* Change the opcode. */
            VIR_Inst_SetOpcode(pUsageInst, VIR_OP_MUL);
            VIR_Inst_SetSrcNum(pUsageInst, 2);

            /* Copy the source. */
            for (i = 0; i < 2; i++)
            {
                VIR_Inst_CopySource(pUsageInst, i, VIR_Inst_GetSource(pFirstAddInst, i), gcvFALSE);

                /* Get the operand information. */
                VIR_Operand_GetOperandInfo(pUsageInst, VIR_Inst_GetSource(pUsageInst, i), &opndInfo);

                /* Update the usage. */
                if (opndInfo.isVreg)
                {
                    vscVIR_InitGeneralUdIterator(&inst_ud_iter,
                                                 pDuInfo,
                                                 pFirstAddInst,
                                                 VIR_Inst_GetSource(pFirstAddInst, i),
                                                 gcvFALSE,
                                                 gcvFALSE);
                    for (pDef = vscVIR_GeneralUdIterator_First(&inst_ud_iter);
                         pDef != gcvNULL;
                         pDef = vscVIR_GeneralUdIterator_Next(&inst_ud_iter))
                    {
                        vscVIR_AddNewUsageToDef(pDuInfo,
                                                pDef->defKey.pDefInst,
                                                pUsageInst,
                                                VIR_Inst_GetSource(pUsageInst, i),
                                                gcvFALSE,
                                                opndInfo.u1.virRegInfo.virReg,
                                                1,
                                                VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(VIR_Inst_GetSource(pUsageInst, i))),
                                                VIR_HALF_CHANNEL_MASK_FULL,
                                                gcvNULL);
                    }
                }
            }

            /* Inverse the source if needed. */
            if (bInverse)
            {
                if ((VIR_Operand_GetModifier(VIR_Inst_GetSource(pUsageInst, 0)) & VIR_MOD_NEG) != 0)
                {
                    VIR_Operand_SetModifier(VIR_Inst_GetSource(pUsageInst, 0), VIR_MOD_NEG ^ VIR_Operand_GetModifier(VIR_Inst_GetSource(pUsageInst, 0)));
                }
                else
                {
                    VIR_Operand_SetModifier(VIR_Inst_GetSource(pUsageInst, 0), VIR_MOD_NEG | VIR_Operand_GetModifier(VIR_Inst_GetSource(pUsageInst, 0)));
                }
            }
        }
        else
        {
            /* Change the opcode. */
            VIR_Inst_SetOpcode(pUsageInst, VIR_OP_MOV);
            VIR_Inst_SetSrcNum(pUsageInst, 1);

            /* Copy the source. */
            VIR_Inst_CopySource(pUsageInst, 0, VIR_Inst_GetSource(pFirstAddInst, (matchSrcIndex == 0 ? 1 : 0)), gcvFALSE);

            /* Get the operand information. */
            VIR_Operand_GetOperandInfo(pUsageInst, VIR_Inst_GetSource(pUsageInst, 0), &opndInfo);

            /* Update the usage. */
            if (opndInfo.isVreg)
            {
                vscVIR_InitGeneralUdIterator(&inst_ud_iter,
                                             pDuInfo,
                                             pFirstAddInst,
                                             VIR_Inst_GetSource(pFirstAddInst, (matchSrcIndex == 0 ? 1 : 0)),
                                             gcvFALSE,
                                             gcvFALSE);
                for (pDef = vscVIR_GeneralUdIterator_First(&inst_ud_iter);
                     pDef != gcvNULL;
                     pDef = vscVIR_GeneralUdIterator_Next(&inst_ud_iter))
                {
                    vscVIR_AddNewUsageToDef(pDuInfo,
                                            pDef->defKey.pDefInst,
                                            pUsageInst,
                                            VIR_Inst_GetSource(pUsageInst, 0),
                                            gcvFALSE,
                                            opndInfo.u1.virRegInfo.virReg,
                                            1,
                                            VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(VIR_Inst_GetSource(pUsageInst, 0))),
                                            VIR_HALF_CHANNEL_MASK_FULL,
                                            gcvNULL);
                }
            }

            /* Inverse the source if needed. */
            if (bInverse)
            {
                if ((VIR_Operand_GetModifier(VIR_Inst_GetSource(pUsageInst, 0)) & VIR_MOD_NEG) != 0)
                {
                    VIR_Operand_SetModifier(VIR_Inst_GetSource(pUsageInst, 0), VIR_MOD_NEG ^ VIR_Operand_GetModifier(VIR_Inst_GetSource(pUsageInst, 0)));
                }
                else
                {
                    VIR_Operand_SetModifier(VIR_Inst_GetSource(pUsageInst, 0), VIR_MOD_NEG | VIR_Operand_GetModifier(VIR_Inst_GetSource(pUsageInst, 0)));
                }
            }
        }

        bChanged = gcvTRUE;
    }

    if (pChanged)
    {
        *pChanged = bChanged;
    }

OnError:
    _VSC_PH_ResetHashTable(pAddSubSet);
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
    VSC_HASH_TABLE* rcp_set       = gcvNULL;
    VSC_HASH_TABLE* def_inst_set0 = gcvNULL;
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

    ON_ERROR(_VSC_PH_InitHashTable(ph, &VSC_PH_Peephole_WorkSet(ph), _VSC_PH_OpndTarget_HFUNC, _VSC_PH_OpndTarget_HKCMP, 512),
             "Failed to initialize Hashtable");
    rcp_set = VSC_PH_Peephole_WorkSet(ph);
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
                if(!vscVIR_IsUniqueDefInstOfUsageInst(VSC_PH_Peephole_GetDUInfo(ph), use_inst, use_inst_opnd,
                                                      usage->usageKey.bIsIndexingRegUsage, sqrt, &breaking_def))
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
                VIR_Instruction* next = sqrt;
                while(next && next != use_inst)
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
        VSC_HASH_ITERATOR iter;
        VSC_DIRECT_HNODE_PAIR pair;
        VIR_Swizzle mapping_swizzle0 = VIR_Enable_GetMappingSwizzle(sqrt_enable, sqrt_src0_swizzle);

        ON_ERROR(_VSC_PH_InitHashTable(ph, &VSC_PH_Peephole_DefSet0(ph), _VSC_PH_OpndTarget_HFUNC, _VSC_PH_OpndTarget_HKCMP, 512),
                 "Failed to initialize Hashtable");
        def_inst_set0 = VSC_PH_Peephole_DefSet0(ph);
        /* collect the def of the src0 of the sqrt inst, and delete the usage between sqrt's src0's def and sqrt's src0 */
        {
            VIR_GENERAL_UD_ITERATOR inst_ud_iter;
            VIR_DEF* def;

            vscVIR_InitGeneralUdIterator(&inst_ud_iter, VSC_PH_Peephole_GetDUInfo(ph), sqrt, sqrt_src0, gcvFALSE, gcvFALSE);
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

            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_RSQ))
            {
                VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                VIR_LOG(dumper, "merges with instruction:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, rcp);
                VIR_LOG_FLUSH(dumper);
            }

            vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph), VIR_ANY_DEF_INST, rcp,
                               sqrt_dest_usage, gcvFALSE, sqrt_dest_info.u1.virRegInfo.virReg, 1,
                               sqrt_dest_usage_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
            VIR_Operand_Copy(VIR_Inst_GetSource(rcp, 0), sqrt_src0);
            VIR_Operand_SetSwizzle(VIR_Inst_GetSource(rcp, 0), rsq_src0_swizzle);

            VIR_Inst_SetOpcode(rcp, VIR_OP_RSQ);
            VIR_Inst_SetSource(rcp, 0, VIR_Inst_GetSource(rcp, 0));
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
                        vscVIR_AddNewUsageToDef(VSC_PH_Peephole_GetDUInfo(ph), def_inst, rcp, VIR_Inst_GetSource(rcp, 0), gcvFALSE,
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
                               sqrt_src0, gcvFALSE, sqrt_src0_info.u1.virRegInfo.virReg, 1,
                               sqrt_src0_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        }
        /* remove the def of sqrt */
        {
            vscVIR_DeleteDef(VSC_PH_Peephole_GetDUInfo(ph), sqrt, sqrt_dest_info.u1.virRegInfo.virReg,
                             1, sqrt_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        }
        _VSC_PH_RemoveSingleInst(ph, func, sqrt);
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

OnError:
    _VSC_PH_ResetHashTable(rcp_set);
    _VSC_PH_ResetHashTable(def_inst_set0);
    return errCode;
}

static VSC_ErrCode _VSC_PH_GenerateLShiftedLS(
    IN OUT VSC_PH_Peephole* ph,
    IN VIR_Instruction* lshift,
    OUT gctBOOL* generated
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_Shader* shader = VSC_PH_Peephole_GetShader(ph);
    VIR_Function* func = VIR_Shader_GetCurrentFunction(shader);
    VSC_OPTN_PHOptions* options = VSC_PH_Peephole_GetOptions(ph);
    VIR_Swizzle lshift_src0_swizzle;
    VIR_Enable lshift_enable, lshift_src0_enable;
    VIR_GENERAL_DU_ITERATOR du_iter;
    VIR_Operand *lshift_dest, *lshift_src0, *lshift_src1;
    VIR_OperandInfo lshift_dest_info, lshift_src0_info;
    gctUINT8 channel;
    VSC_HASH_TABLE* ls_set        = gcvNULL;
    VSC_HASH_TABLE* def_inst_set0 = gcvNULL;
    gctBOOL invalid_case = gcvFALSE;
    gctBOOL bNeedDelLShift = gcvTRUE;

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_LSHIFT_LS))
    {
        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
        VIR_LOG(dumper, "\nlshift instruction:\n");
        VIR_LOG_FLUSH(dumper);
        VIR_Inst_Dump(dumper, lshift);
        VIR_LOG_FLUSH(dumper);
    }

    lshift_dest = VIR_Inst_GetDest(lshift);
    if(VIR_Operand_GetModifier(lshift_dest))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_LSHIFT_LS))
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

    lshift_enable = VIR_Operand_GetEnable(lshift_dest);
    if (lshift_enable != VIR_ENABLE_X &&
        lshift_enable != VIR_ENABLE_Y &&
        lshift_enable != VIR_ENABLE_Z &&
        lshift_enable != VIR_ENABLE_W)
    {
        return errCode;
    }

    VIR_Operand_GetOperandInfo(lshift, lshift_dest, &lshift_dest_info);
    gcmASSERT(~lshift_dest_info.isTempVar && ~lshift_dest_info.isVecConst);

    /* the result of lshift can not be an output */
    if (lshift_dest_info.isOutput)
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_LSHIFT_LS))
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

    lshift_src1 = VIR_Inst_GetSource(lshift, 1);
    if (VIR_Operand_GetOpKind(lshift_src1) != VIR_OPND_IMMEDIATE ||
        VIR_Operand_GetImmediateUint(lshift_src1) >= 8)
    {
        return errCode;
    }

    lshift_src0 = VIR_Inst_GetSource(lshift, 0);
    VIR_Operand_GetOperandInfo(lshift, lshift_src0, &lshift_src0_info);
    lshift_src0_swizzle = VIR_Operand_GetSwizzle(lshift_src0);
    lshift_src0_enable = VIR_Swizzle_2_Enable(lshift_src0_swizzle);

    ON_ERROR(_VSC_PH_InitHashTable(ph, &VSC_PH_Peephole_WorkSet(ph), _VSC_PH_OpndTarget_HFUNC, _VSC_PH_OpndTarget_HKCMP, 512),
             "Failed to initialize Hashtable");
    ls_set = VSC_PH_Peephole_WorkSet(ph);
    /* get usage of lshift's dest in per channel way*/
    for(channel = 0; channel < VIR_CHANNEL_NUM; channel++)
    {
        VIR_USAGE* usage;
        if(!(lshift_enable & (1 << channel)))
        {
            continue;
        }

        /*  */
        vscVIR_InitGeneralDuIterator(&du_iter, VSC_PH_Peephole_GetDUInfo(ph), lshift, lshift_dest_info.u1.virRegInfo.virReg, channel, gcvFALSE);
        for(usage = vscVIR_GeneralDuIterator_First(&du_iter); usage != gcvNULL;
            usage = vscVIR_GeneralDuIterator_Next(&du_iter))
        {
            VIR_Instruction* use_inst;
            VIR_Operand* use_inst_dest;
            VIR_Operand* use_inst_opnd;

            if(vscHTBL_DirectTestAndGet(ls_set, (void*)&(usage->usageKey), gcvNULL))
            {
                continue;
            }

            use_inst = usage->usageKey.pUsageInst;
            use_inst_opnd = usage->usageKey.pOperand;

            /* the result of lshift must be used in L/S */
            if(!VIR_OPCODE_isMemLd(VIR_Inst_GetOpcode(use_inst)) &&
               !VIR_OPCODE_isMemSt(VIR_Inst_GetOpcode(use_inst)))
            {
                bNeedDelLShift = gcvFALSE;
                continue;
            }

            if (use_inst_opnd != VIR_Inst_GetSource(use_inst, 1))
            {
                invalid_case = gcvTRUE;
                break;
            }

            /* use_inst should not have different rounding mode from the lshift instruction */
            use_inst_dest = VIR_Inst_GetDest(use_inst);
            if(VIR_Operand_GetRoundMode(use_inst_dest) != VIR_Operand_GetRoundMode(lshift_dest))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_LSHIFT_LS))
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
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_LSHIFT_LS))
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

            /* lshift and L/S should be in the same bb */
            if(VIR_Inst_GetBasicBlock(lshift) != VIR_Inst_GetBasicBlock(use_inst))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_LSHIFT_LS))
                {
                    VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                    VIR_LOG(dumper, "not processed because of the following usage and lshift are not in the same bb:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, use_inst);
                    VIR_LOG_FLUSH(dumper);
                }
                invalid_case = gcvTRUE;
                break;
            }

            /* dest of lshift should cover its usage in L/S */
            {
                VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(use_inst_opnd);
                VIR_Enable ls_enable = VIR_Swizzle_2_Enable(swizzle);
                if(!VIR_Enable_Covers(lshift_enable, ls_enable))
                {
                    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_LSHIFT_LS))
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

            /* in case of loop, lshift should be the only def of ls usage. */
            {
                VIR_Instruction* breaking_def;
                if(!vscVIR_IsUniqueDefInstOfUsageInst(VSC_PH_Peephole_GetDUInfo(ph), use_inst, use_inst_opnd,
                                                      usage->usageKey.bIsIndexingRegUsage, lshift, &breaking_def))
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
            /* there should be no def of lshift_src0 between lshift and use_inst */
            {
                VIR_Instruction* next = lshift;
                while(next && next != use_inst)
                {
                    if(VIR_Operand_SameLocation(lshift, lshift_src0, next, VIR_Inst_GetDest(next)))
                    {
                        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_LSHIFT_LS))
                        {
                            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                            VIR_LOG(dumper, "not processed because between lshift and use_inst:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, use_inst);
                            VIR_LOG(dumper, "\nthis intruction redefs lshift_src0:\n");
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

            vscHTBL_DirectSet(ls_set, (void*)_VSC_PH_Peephole_NewOpndTarget(ph, usage), gcvNULL);
        }

        if(invalid_case)
        {
            break;
        }
    }

    if(!invalid_case && HTBL_GET_ITEM_COUNT(ls_set))
    {
        VSC_HASH_ITERATOR iter;
        VSC_DIRECT_HNODE_PAIR pair;
        VIR_Swizzle mapping_swizzle0 = VIR_Enable_GetMappingSwizzle(lshift_enable, lshift_src0_swizzle);

        ON_ERROR(_VSC_PH_InitHashTable(ph, &VSC_PH_Peephole_DefSet0(ph), vscHFUNC_Default, vscHKCMP_Default, 512),
                 "Failed to initialize Hashtable");
        def_inst_set0 = VSC_PH_Peephole_DefSet0(ph);
        /* collect the def of the src0 of the lshift inst, and delete the usage between lshift's src0's def and lshift's src0 */
        {
            VIR_GENERAL_UD_ITERATOR inst_ud_iter;
            VIR_DEF* def;

            vscVIR_InitGeneralUdIterator(&inst_ud_iter, VSC_PH_Peephole_GetDUInfo(ph), lshift, lshift_src0, gcvFALSE, gcvFALSE);
            for(def = vscVIR_GeneralUdIterator_First(&inst_ud_iter); def != gcvNULL;
                def = vscVIR_GeneralUdIterator_Next(&inst_ud_iter))
            {
                VIR_Instruction* def_inst = def->defKey.pDefInst;
                vscHTBL_DirectSet(def_inst_set0, (void*)def_inst, gcvNULL);
            }
        }

        /* do the transformation */
        vscHTBLIterator_Init(&iter, ls_set);
        for(pair = vscHTBLIterator_DirectFirst(&iter);
            IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
        {
            VIR_USAGE* usage = (VIR_USAGE*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
            VIR_Instruction* ls = usage->usageKey.pUsageInst;
            VIR_Operand* lshift_dest_usage = usage->usageKey.pOperand;
            VIR_Swizzle lshift_dest_usage_swizzle = VIR_Operand_GetSwizzle(lshift_dest_usage);
            VIR_Enable lshift_dest_usage_enable = VIR_Swizzle_2_Enable(lshift_dest_usage_swizzle);

            VIR_Swizzle lshifted_ls_src1_swizzle = VIR_Swizzle_ApplyMappingSwizzle(lshift_dest_usage_swizzle, mapping_swizzle0);
            VIR_Enable lshifted_ls_src1_enable = VIR_Swizzle_2_Enable(lshifted_ls_src1_swizzle);

            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_LSHIFT_LS))
            {
                VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                VIR_LOG(dumper, "merges with instruction:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, ls);
                VIR_LOG_FLUSH(dumper);
            }

            vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph), VIR_ANY_DEF_INST, ls,
                               lshift_dest_usage, gcvFALSE, lshift_dest_info.u1.virRegInfo.virReg, 1,
                               lshift_dest_usage_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);

            VIR_Operand_Copy(VIR_Inst_GetSource(ls, 1), lshift_src0);
            VIR_Operand_SetSwizzle(VIR_Inst_GetSource(ls, 1), lshifted_ls_src1_swizzle);
            VIR_Operand_SetLShift(VIR_Inst_GetSource(ls, 1), VIR_Operand_GetImmediateUint(lshift_src1));

            /* add the use of ls_src1 to the def of lshift_src0 */
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

                    if (def_dest_enable & lshifted_ls_src1_enable)
                    {
                        vscVIR_AddNewUsageToDef(VSC_PH_Peephole_GetDUInfo(ph), def_inst, ls, VIR_Inst_GetSource(ls, 1), gcvFALSE,
                                                lshift_src0_info.u1.virRegInfo.virReg, 1, (VIR_Enable)(def_dest_enable & lshifted_ls_src1_enable),
                                                VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
                    }
                }
            }
            if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_LSHIFT_LS))
            {
                VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                VIR_LOG(dumper, "into:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, ls);
                VIR_LOG_FLUSH(dumper);
            }
        }
        if(generated != gcvNULL)
        {
            *generated = gcvTRUE;
        }

        if (bNeedDelLShift)
        {
            /* remove the use of lshift_src0 */
            vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph), VIR_ANY_DEF_INST, lshift,
                                lshift_src0, gcvFALSE, lshift_src0_info.u1.virRegInfo.virReg, 1,
                                lshift_src0_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);

            /* remove the def of lshift */
            vscVIR_DeleteDef(VSC_PH_Peephole_GetDUInfo(ph), lshift, lshift_dest_info.u1.virRegInfo.virReg,
                             1, lshift_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);

            _VSC_PH_RemoveSingleInst(ph, func, lshift);
        }
    }
    else
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_LSHIFT_LS)
            && !invalid_case && HTBL_GET_ITEM_COUNT(ls_set) == 0)
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "is prevented from being merging because there is no following L/S instruction.\n");
            VIR_LOG_FLUSH(dumper);
        }
        if(generated != gcvNULL)
        {
            *generated = gcvFALSE;
        }
    }

OnError:
    _VSC_PH_ResetHashTable(ls_set);
    _VSC_PH_ResetHashTable(def_inst_set0);
    return errCode;
}

/*
** Merge ADD/LOAD instructions and generate LOAD/STORE instruction:
**  ADD   r1, r2, r3
**  LOAD  r4, r1, 0
** -->
**  LOAD  r4, r2, r3
*/
static VSC_ErrCode _VSC_PH_GenerateLoadStore(
    IN OUT VSC_PH_Peephole*     ph,
    IN VIR_Instruction*         pAddInst,
    OUT gctBOOL*                bGenerated
    )
{
    VSC_ErrCode                 errCode  = VSC_ERR_NONE;
    VIR_DEF_USAGE_INFO*         pDuInfo = VSC_PH_Peephole_GetDUInfo(ph);
    VIR_Operand*                pAddDestOpnd = VIR_Inst_GetDest(pAddInst);
    VIR_Enable                  addEnable = VIR_Operand_GetEnable(pAddDestOpnd);
    VIR_OperandInfo             addDestInfo, addSrc0Info, addSrc1Info;
    VIR_Operand*                pAddSrc0Opnd = VIR_Inst_GetSource(pAddInst, 0);
    VIR_Operand*                pAddSrc1Opnd = VIR_Inst_GetSource(pAddInst, 1);
    gctBOOL                     bAddSrc1Imm = gcvFALSE;
    gctUINT                     src1Imm = 0;
    gctUINT8                    channel;
    VIR_GENERAL_DU_ITERATOR     du_iter;
    gctBOOL                     bChanged = gcvFALSE, bNeedToMatchAllUsageInst = gcvFALSE;
    VSC_HASH_TABLE*             pWorkingSet = gcvNULL;

    /* Skip the non-uint ADD. */
    if (!VIR_TypeId_isUnSignedInteger(VIR_Operand_GetTypeId(pAddDestOpnd)))
    {
        return errCode;
    }

    /* Get the operand info. */
    VIR_Operand_GetOperandInfo(pAddInst, pAddDestOpnd, &addDestInfo);
    VIR_Operand_GetOperandInfo(pAddInst, pAddSrc0Opnd, &addSrc0Info);
    VIR_Operand_GetOperandInfo(pAddInst, pAddSrc1Opnd, &addSrc1Info);

    /*
    ** If the dest operand is also one source operand, we can do this optimization only when the LOADs are all the
    ** usage instructions, and we also need to remove this ADD instruction after the optimization.
    */
    if ((addSrc0Info.isVreg && addSrc0Info.u1.virRegInfo.virReg == addDestInfo.u1.virRegInfo.virReg)
        ||
        (addSrc1Info.isVreg && addSrc1Info.u1.virRegInfo.virReg == addDestInfo.u1.virRegInfo.virReg))
    {
        /* We can't remove this ADD if the dest is an output, so just skip it. */
        if (addDestInfo.isOutput)
        {
            return errCode;
        }
        bNeedToMatchAllUsageInst = gcvTRUE;
    }

    /* Check if the source1 of ADD instruction is a immediate. */
    if (VIR_Operand_isImm(pAddSrc1Opnd))
    {
        src1Imm = VIR_Operand_GetImmediateUint(pAddSrc1Opnd);
        bAddSrc1Imm = gcvTRUE;
    }

    /* Initialize the working hash table. */
    ON_ERROR(_VSC_PH_InitHashTable(ph, &VSC_PH_Peephole_WorkSet(ph), _VSC_PH_OpndTarget_HFUNC, _VSC_PH_OpndTarget_HKCMP, 512),
             "Failed to initialize Hashtable");
    pWorkingSet = VSC_PH_Peephole_WorkSet(ph);

    for (channel = 0; channel < VIR_CHANNEL_NUM; channel++)
    {
        VIR_USAGE*              pUsage;

        if (!(addEnable & (1 << channel)))
        {
            continue;
        }

        /* Check the usage and find the LOAD/STORE instruction. */
        vscVIR_InitGeneralDuIterator(&du_iter, VSC_PH_Peephole_GetDUInfo(ph), pAddInst, addDestInfo.u1.virRegInfo.virReg, channel, gcvFALSE);
        for (pUsage = vscVIR_GeneralDuIterator_First(&du_iter);
             pUsage != gcvNULL;
             pUsage = vscVIR_GeneralDuIterator_Next(&du_iter))
        {
            VIR_Instruction*    pUsageInst = pUsage->usageKey.pUsageInst;
            VIR_Operand*        pUsageOpnd = pUsage->usageKey.pOperand;
            VIR_Operand*        pBaseOpnd = gcvNULL;
            VIR_Operand*        pOffsetOpnd = gcvNULL;
            gctUINT             newOffset = 0;

            /* Skip the exist one. */
            if (vscHTBL_DirectTestAndGet(pWorkingSet, (void*)&(pUsage->usageKey), gcvNULL))
            {
                continue;
            }

            /* Skip the implicit usage. */
            if (VIR_IS_IMPLICIT_USAGE_INST(pUsageInst))
            {
                if (bNeedToMatchAllUsageInst)
                {
                    goto OnError;
                }
                else
                {
                    continue;
                }
            }

            pBaseOpnd = VIR_Inst_GetSource(pUsageInst, 0);
            pOffsetOpnd = VIR_Inst_GetSource(pUsageInst, 1);

            /* The usage operand must be the SRC0 of a LOAD/STORE instruction. */
            if (!(VIR_OPCODE_isMemLd(VIR_Inst_GetOpcode(pUsageInst)) || VIR_OPCODE_isMemSt(VIR_Inst_GetOpcode(pUsageInst)))
                ||
                pUsageOpnd != pBaseOpnd)
            {
                if (bNeedToMatchAllUsageInst)
                {
                    goto OnError;
                }
                else
                {
                    continue;
                }
            }

            /* The offset must be a immediate operand. */
            if (!VIR_Operand_isImm(pOffsetOpnd))
            {
                if (bNeedToMatchAllUsageInst)
                {
                    goto OnError;
                }
                else
                {
                    continue;
                }
            }

            /* The offset can be a non-zero immediate only when the src1 of ADD instruction is also an imediate. */
            newOffset = VIR_Operand_GetImmediateUint(pOffsetOpnd);
            if (newOffset != 0 && !bAddSrc1Imm)
            {
                if (bNeedToMatchAllUsageInst)
                {
                    goto OnError;
                }
                else
                {
                    continue;
                }
            }

            /* Save the valid case. */
            vscHTBL_DirectSet(pWorkingSet, (void*)_VSC_PH_Peephole_NewOpndTarget(ph, pUsage), gcvNULL);
        }
    }

    /* do the transformation */
    if (HTBL_GET_ITEM_COUNT(pWorkingSet))
    {
        VSC_HASH_ITERATOR       iter;
        VSC_DIRECT_HNODE_PAIR   pair;

        vscHTBLIterator_Init(&iter, pWorkingSet);
        for (pair = vscHTBLIterator_DirectFirst(&iter);
             IS_VALID_DIRECT_HNODE_PAIR(&pair);
             pair = vscHTBLIterator_DirectNext(&iter))
        {
            VIR_USAGE*          pUsage = (VIR_USAGE*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
            VIR_Instruction*    pUsageInst = pUsage->usageKey.pUsageInst;
            VIR_Operand*        pBaseOpnd = VIR_Inst_GetSource(pUsageInst, 0);
            VIR_Operand*        pOffsetOpnd = VIR_Inst_GetSource(pUsageInst, 1);
            VIR_Swizzle         newSwizzle;
            gctBOOL             pUpdateOffset = gcvFALSE;
            gctUINT             newOffset = VIR_Operand_GetImmediateUint(pOffsetOpnd);

            gcmASSERT(VIR_Operand_isImm(pOffsetOpnd));

            if (bAddSrc1Imm)
            {
                newOffset += src1Imm;
                pUpdateOffset = gcvTRUE;
            }
            else
            {
                gcmASSERT(newOffset == 0);
            }

            /* Delete the usage. */
            vscVIR_DeleteUsage(pDuInfo,
                               pAddInst,
                               pUsageInst,
                               pBaseOpnd,
                               gcvFALSE,
                               addDestInfo.u1.virRegInfo.virReg,
                               1,
                               VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pBaseOpnd)),
                               VIR_HALF_CHANNEL_MASK_FULL,
                               gcvNULL);

            /* Copy the base from src0 of ADD instruction and update the usage if needed. */
            VIR_Operand_Copy(pBaseOpnd, pAddSrc0Opnd);
            if (addSrc0Info.isVreg)
            {
                VIR_GENERAL_UD_ITERATOR udIter;
                VIR_DEF*                pDef = gcvNULL;

                newSwizzle = VIR_Enable_GetMappingSwizzle(addEnable, VIR_Operand_GetSwizzle(pAddSrc0Opnd));
                VIR_Operand_SetSwizzle(pBaseOpnd, newSwizzle);

                vscVIR_InitGeneralUdIterator(&udIter, pDuInfo, pAddInst, pAddSrc0Opnd, gcvFALSE, gcvFALSE);

                for (pDef = vscVIR_GeneralUdIterator_First(&udIter);
                     pDef != gcvNULL;
                     pDef = vscVIR_GeneralUdIterator_Next(&udIter))
                {
                    vscVIR_AddNewUsageToDef(pDuInfo,
                                            pDef->defKey.pDefInst,
                                            pUsageInst,
                                            pBaseOpnd,
                                            gcvFALSE,
                                            addSrc0Info.u1.virRegInfo.virReg,
                                            1,
                                            VIR_Swizzle_2_Enable(newSwizzle),
                                            VIR_HALF_CHANNEL_MASK_FULL,
                                            gcvNULL);
                }
            }

            /* Update the offset if it is an immeidate. */
            if (pUpdateOffset)
            {
                VIR_Operand_SetImmediateUint(pOffsetOpnd, newOffset);
            }
            else
            {
                /* Copy the offset from src1 of ADD instruction and update the usage if needed. */
                VIR_Operand_Copy(pOffsetOpnd, pAddSrc1Opnd);
                if (addSrc1Info.isVreg)
                {
                    VIR_GENERAL_UD_ITERATOR udIter;
                    VIR_DEF*                pDef = gcvNULL;

                    newSwizzle = VIR_Enable_GetMappingSwizzle(addEnable, VIR_Operand_GetSwizzle(pAddSrc1Opnd));
                    VIR_Operand_SetSwizzle(pBaseOpnd, newSwizzle);

                    vscVIR_InitGeneralUdIterator(&udIter, pDuInfo, pAddInst, pAddSrc1Opnd, gcvFALSE, gcvFALSE);

                    for (pDef = vscVIR_GeneralUdIterator_First(&udIter);
                         pDef != gcvNULL;
                         pDef = vscVIR_GeneralUdIterator_Next(&udIter))
                    {
                        vscVIR_AddNewUsageToDef(pDuInfo,
                                                pDef->defKey.pDefInst,
                                                pUsageInst,
                                                pOffsetOpnd,
                                                gcvFALSE,
                                                addSrc1Info.u1.virRegInfo.virReg,
                                                1,
                                                VIR_Swizzle_2_Enable(newSwizzle),
                                                VIR_HALF_CHANNEL_MASK_FULL,
                                                gcvNULL);
                    }
                }
            }
        }

        /* We need to delete this ADD instruction when all usage instructions are matched. */
        if (bNeedToMatchAllUsageInst)
        {
            errCode = VIR_Pass_DeleteInstruction(VIR_Inst_GetFunction(pAddInst), pAddInst, &VSC_PH_Peephole_GetCfgChanged(ph));
            ON_ERROR(errCode, "Delete instruction error.");
        }

        bChanged = gcvTRUE;
    }

    /* Save the result. */
    if (bGenerated)
    {
        *bGenerated = bChanged;
    }

OnError:
    _VSC_PH_ResetHashTable(pWorkingSet);
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
                gcvFALSE,
                use_opnd_info.u1.virRegInfo.virReg,
                1,
                use_enable,
                VIR_HALF_CHANNEL_MASK_FULL,
                gcvNULL);

        /* duplicate merged_inst_dst */
        VIR_Operand_Copy(VIR_Inst_GetSource(use_inst, srcIndex), merged_inst_dst);
        VIR_Operand_SetLvalue(VIR_Inst_GetSource(use_inst, srcIndex), 0);
        VIR_Operand_SetSwizzle(VIR_Inst_GetSource(use_inst, srcIndex),
            VIR_Swizzle_ApplyMappingSwizzle(use_swizzle, mapping_swizzle));

        vscVIR_AddNewUsageToDef(VSC_PH_Peephole_GetDUInfo(ph),
            merged_inst,
            use_inst,
            VIR_Inst_GetSource(use_inst, srcIndex),
            gcvFALSE,
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
            VIR_Operand_GetTypeId(inst_src1) == VIR_TYPE_INT32)
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
    vscVIR_InitGeneralUdIterator(&ud_iter, VSC_PH_Peephole_GetDUInfo(ph), inst, inst_src0, gcvFALSE, gcvFALSE);
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
                ON_ERROR(_VSC_PH_InitHashTable(ph, &VSC_PH_Peephole_UsageSet(ph), _VSC_PH_OpndTarget_HFUNC, _VSC_PH_OpndTarget_HKCMP, 512),
                         "Failed to initialize Hashtable");
                usages_set = VSC_PH_Peephole_UsageSet(ph);

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
                    merged_inst->enable,
                    gcvFALSE);
                VIR_Function_FreeOperand(func, new_dst);

                vscVIR_AddNewDef(VSC_PH_Peephole_GetDUInfo(ph),
                    merged_inst->inst,
                    merged_inst_dst_info.u1.virRegInfo.virReg,
                    1,
                    merged_inst->enable,
                    VIR_HALF_CHANNEL_MASK_FULL,
                    gcvNULL,
                    gcvNULL);

                VIR_Operand_SetSwizzle(merged_inst_src,
                    VIR_Enable_2_Swizzle_WShift(merged_inst->enable));

                /* update the du info */
                vscVIR_AddNewUsageToDef(VSC_PH_Peephole_GetDUInfo(ph),
                            mergeKey->defKey->pDefInst,
                            merged_inst->inst,
                            merged_inst_src,
                            gcvFALSE,
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

                _VSC_PH_ResetHashTable(usages_set);
            }

            /* delete the usage of inst_src */
            vscVIR_DeleteUsage(VSC_PH_Peephole_GetDUInfo(ph),
                VIR_ANY_DEF_INST,
                inst,
                inst_src0,
                gcvFALSE,
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
            ON_ERROR(_VSC_PH_InitHashTable(ph, &VSC_PH_Peephole_UsageSet(ph), _VSC_PH_OpndTarget_HFUNC, _VSC_PH_OpndTarget_HKCMP, 512),
                        "Failed to initialize Hashtable");
            usages_set = VSC_PH_Peephole_UsageSet(ph);
            _VSC_PH_RecordUsages(ph, inst, usages_set);
            _VSC_PH_ReplaceUsages(ph, merged_inst->inst, channelMapping, usages_set);
            _VSC_PH_ResetHashTable(usages_set);

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
            _VSC_PH_RemoveSingleInst(ph, func, inst);
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

OnError:
    if (mergeKey)
    {
        vscMM_Free(VSC_PH_Peephole_GetMM(ph), mergeKey);
    }
    return errCode;
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

        if (vscVIR_DoesUsageInstHaveUniqueDefInst(VSC_PH_Peephole_GetDUInfo(ph), inst, currSrc, gcvFALSE, def_inst) &&
            vscVIR_IsUniqueUsageInstOfDefInst(VSC_PH_Peephole_GetDUInfo(ph), *def_inst, inst, gcvNULL, gcvFALSE, gcvNULL, gcvNULL, gcvNULL) &&
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
    IN VIR_Instruction      **defInst
    )
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;
    VIR_Dumper          *dumper = VSC_PH_Peephole_GetDumper(ph);
    VSC_OPTN_PHOptions  *options = VSC_PH_Peephole_GetOptions(ph);
    gctBOOL             invalidCase = gcvFALSE;
    VIR_Instruction     *nextDefInst = VIR_Inst_GetNext(*defInst);

    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_MOV_DEF))
    {
        VIR_LOG(dumper, "\nInstruction:");
        VIR_LOG_FLUSH(dumper);
        VIR_Inst_Dump(dumper, inst);
        VIR_LOG_FLUSH(dumper);
    }

    gcmASSERT(inst_src != gcvNULL);
    gcmASSERT(nextDefInst);
    gcmASSERT(VIR_Inst_GetPrev(inst));

    /* if the defInst and inst is not next to each other, move defInst down */
    if (nextDefInst != inst)
    {
        gcmASSERT(VIR_Inst_GetPrev(inst) != *defInst);

        /* defInst and inst is at the same BB */
        if(VIR_Inst_GetBasicBlock(*defInst) == VIR_Inst_GetBasicBlock(inst))
        {
            /* there is no redefine of defInst'src in between */
            VIR_Instruction *next = *defInst;
            VIR_SrcOperand_Iterator opndIter;
            VIR_Operand     *nextOpnd;

            while(next && next != inst)
            {
                VIR_SrcOperand_Iterator_Init(*defInst, &opndIter);
                nextOpnd = VIR_SrcOperand_Iterator_First(&opndIter);

                for (; nextOpnd != gcvNULL; nextOpnd = VIR_SrcOperand_Iterator_Next(&opndIter))
                {
                    if(VIR_Operand_SameLocation(*defInst, nextOpnd, next, VIR_Inst_GetDest(next)))
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
                VIR_Inst_Dump(dumper, *defInst);
                VIR_LOG_FLUSH(dumper);
                VIR_LOG(dumper, "close to \n");
                VIR_Inst_Dump(dumper, inst);
                VIR_LOG_FLUSH(dumper);
            }

            /* If those two instructions are from the same function, we just move the instruction.
            ** Otherwise we need to add a new instruction and copy the old one
            */
            if (VIR_Inst_GetFunction(*defInst) == VIR_Inst_GetFunction(inst))
            {
                _VSC_PH_MoveSingleInstBefore(ph, VIR_Inst_GetFunction(*defInst), inst, *defInst);
            }
            else
            {
                VIR_Instruction *newInst = gcvNULL;

                /* Add a new instruction. */
                errCode = VIR_Function_AddInstructionBefore(VIR_Inst_GetFunction(inst),
                                                            VIR_Inst_GetOpcode(*defInst),
                                                            VIR_Inst_GetInstType(*defInst),
                                                            inst,
                                                            gcvTRUE,
                                                            &newInst);
                ON_ERROR(errCode, "Add instruction");

                /* Copy the instruction. */
                errCode = VIR_Inst_Copy(newInst, *defInst, gcvFALSE);
                ON_ERROR(errCode, "Copy instruction");

                /* Now we can remove this instruction. */
                errCode = _VSC_PH_RemoveSingleInst(ph, VIR_Inst_GetFunction(*defInst), *defInst);
                ON_ERROR(errCode, "Remove instruction");

                *defInst = newInst;
            }
        }
    }

OnError:
    return errCode;
}

static gctBOOL _VSC_PH_LocalVariable(
    IN OUT VSC_PH_Peephole  *ph,
    VIR_Instruction         *pInst,
    VIR_Operand             *pOpnd,
    IN OUT VSC_HASH_TABLE   *visitSet)
{
    VIR_Shader  *pShader = ph->shader;
    VIR_DEF_USAGE_INFO* duInfo = VSC_PH_Peephole_GetDUInfo(ph);
    VIR_Symbol  *sym = VIR_Operand_GetSymbol(pOpnd);

    if (!VIR_Operand_isSymbol(pOpnd))
    {
        return gcvFALSE;
    }

    if (vscHTBL_DirectTestAndGet(visitSet, (void*) pOpnd, gcvNULL))
    {
        return gcvFALSE;
    }

    vscHTBL_DirectSet(visitSet, (void*) pOpnd, gcvNULL);

    if (VIR_Symbol_isUniform(sym) &&
        (strcmp(VIR_Shader_GetSymNameString(pShader, sym), _sldLocalStorageAddressName) == 0 ||
         strcmp(VIR_Shader_GetSymNameString(pShader, sym), _sldSharedVariableStorageBlockName) == 0))
    {
        return gcvTRUE;
    }
    else
    {
        VIR_GENERAL_UD_ITERATOR fromOperandUdIter;
        VIR_DEF* defKey;
        VIR_OperandInfo operandInfo;

        VIR_Operand_GetOperandInfo(pInst, pOpnd, &operandInfo);
        vscVIR_InitGeneralUdIterator(&fromOperandUdIter, duInfo, pInst, pOpnd, gcvFALSE, gcvFALSE);

        for(defKey = vscVIR_GeneralUdIterator_First(&fromOperandUdIter); defKey != gcvNULL;
            defKey = vscVIR_GeneralUdIterator_Next(&fromOperandUdIter))
        {
            VIR_Instruction* defInst = defKey->defKey.pDefInst;
            gctUINT i;
            if (defInst != VIR_INPUT_DEF_INST && defInst && (!VIR_OPCODE_Loads(VIR_Inst_GetOpcode(defInst))))
            {
                for (i = 0; i < VIR_Inst_GetSrcNum(defInst);i++)
                {
                    if (_VSC_PH_LocalVariable(ph, defInst, VIR_Inst_GetSource(defInst, i), visitSet))
                    {
                        return gcvTRUE;
                    }
                }
            }
        }
    }

    return gcvFALSE;
}

static VSC_ErrCode _VSC_PH_LocalInst(
    IN OUT VSC_PH_Peephole  *ph,
    IN VIR_Instruction      *lsInst,
    IN OUT gctBOOL          *useLocalMem
    )
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;
    VIR_Operand         *baseOpnd = VIR_Inst_GetSource(lsInst, 0);
    VIR_OpCode          opc = VIR_Inst_GetOpcode(lsInst);
    VIR_Symbol          *baseSym = VIR_Operand_GetUnderlyingSymbol(baseOpnd);

    VSC_HASH_TABLE      *visitSet = gcvNULL;

    /* if address space of baseSym is global, do not convert to local Inst */
    if (baseSym && VIR_Symbol_GetAddrSpace(baseSym) == VIR_AS_GLOBAL)
    {
        return errCode;
    }

    ON_ERROR(_VSC_PH_InitHashTable(ph, &VSC_PH_Peephole_WorkSet(ph), vscHFUNC_Default, vscHKCMP_Default, 512),
                "Failed to initialize Hashtable");
    visitSet = VSC_PH_Peephole_WorkSet(ph);

    if (_VSC_PH_LocalVariable(ph, lsInst, baseOpnd, visitSet))
    {
        *useLocalMem = gcvTRUE;
        switch (opc)
        {
        case VIR_OP_LOAD:
            VIR_Inst_SetOpcode(lsInst, VIR_OP_LOAD_L);
            break;
        case VIR_OP_STORE:
            VIR_Inst_SetOpcode(lsInst, VIR_OP_STORE_L);
            break;
        case VIR_OP_ATOMADD:
            VIR_Shader_SetFlag(ph->shader, VIR_SHFLAG_USE_LOCAL_MEM_ATOM);
            VIR_Inst_SetOpcode(lsInst, VIR_OP_ATOMADD_L);
            break;
        case VIR_OP_ATOMSUB:
            VIR_Shader_SetFlag(ph->shader, VIR_SHFLAG_USE_LOCAL_MEM_ATOM);
            VIR_Inst_SetOpcode(lsInst, VIR_OP_ATOMSUB_L);
            break;
        case VIR_OP_ATOMCMPXCHG:
            VIR_Shader_SetFlag(ph->shader, VIR_SHFLAG_USE_LOCAL_MEM_ATOM);
            VIR_Inst_SetOpcode(lsInst, VIR_OP_ATOMCMPXCHG_L);
            break;
        case VIR_OP_ATOMMAX:
            VIR_Shader_SetFlag(ph->shader, VIR_SHFLAG_USE_LOCAL_MEM_ATOM);
            VIR_Inst_SetOpcode(lsInst, VIR_OP_ATOMMAX_L);
            break;
        case VIR_OP_ATOMMIN:
            VIR_Shader_SetFlag(ph->shader, VIR_SHFLAG_USE_LOCAL_MEM_ATOM);
            VIR_Inst_SetOpcode(lsInst, VIR_OP_ATOMMIN_L);
            break;
        case VIR_OP_ATOMOR:
            VIR_Shader_SetFlag(ph->shader, VIR_SHFLAG_USE_LOCAL_MEM_ATOM);
            VIR_Inst_SetOpcode(lsInst, VIR_OP_ATOMOR_L);
            break;
        case VIR_OP_ATOMAND:
            VIR_Shader_SetFlag(ph->shader, VIR_SHFLAG_USE_LOCAL_MEM_ATOM);
            VIR_Inst_SetOpcode(lsInst, VIR_OP_ATOMAND_L);
            break;
        case VIR_OP_ATOMXOR:
            VIR_Shader_SetFlag(ph->shader, VIR_SHFLAG_USE_LOCAL_MEM_ATOM);
            VIR_Inst_SetOpcode(lsInst, VIR_OP_ATOMXOR_L);
            break;
        case VIR_OP_ATOMXCHG:
            VIR_Shader_SetFlag(ph->shader, VIR_SHFLAG_USE_LOCAL_MEM_ATOM);
            VIR_Inst_SetOpcode(lsInst, VIR_OP_ATOMXCHG_L);
            break;
        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }

OnError:
    _VSC_PH_ResetHashTable(visitSet);

    return errCode;
}

void _VSC_PH_Inst_DeleteUses(
    IN OUT VSC_PH_Peephole  *ph,
    IN VIR_Instruction      *pInst,
    IN gctUINT              delSrcNum
    )
{
    VIR_DEF_USAGE_INFO  *pDuInfo = ph->du_info;
    gctUINT         i;
    VIR_Operand     *srcOpnd = gcvNULL;
    VIR_OperandInfo         operandInfo;
    gctUINT     swizzle;

    gcmASSERT(delSrcNum <= VIR_Inst_GetSrcNum(pInst));

    for (i = 0; i < delSrcNum; i++)
    {
        srcOpnd = VIR_Inst_GetSource(pInst, i);
        if (srcOpnd == gcvNULL || VIR_Operand_isUndef(srcOpnd))
        {
            continue;
        }

        swizzle = VIR_Operand_GetSwizzle(srcOpnd);

        VIR_Operand_GetOperandInfo(pInst,
            srcOpnd,
            &operandInfo);

        vscVIR_DeleteUsage(pDuInfo,
            VIR_ANY_DEF_INST,
            pInst,
            srcOpnd,
            gcvFALSE,
            operandInfo.u1.virRegInfo.virReg,
            1,
            VIR_Swizzle_2_Enable(swizzle),
            VIR_HALF_CHANNEL_MASK_FULL,
            gcvNULL);
    }
}

void _VSC_PH_Inst_DeleteDef(
    IN OUT VSC_PH_Peephole  *ph,
    IN VIR_Instruction      *pInst
    )
{
    VIR_DEF_USAGE_INFO  *pDuInfo = ph->du_info;
    VIR_Operand     *dstOpnd = gcvNULL;
    VIR_OperandInfo  operandInfo;
    VIR_Enable      instEnable;

    dstOpnd = VIR_Inst_GetDest(pInst);
    gcmASSERT(dstOpnd);
    instEnable = VIR_Operand_GetEnable(dstOpnd);

    VIR_Operand_GetOperandInfo(pInst,
        dstOpnd,
        &operandInfo);

    vscVIR_DeleteDef(
        pDuInfo,
        pInst,
        operandInfo.u1.virRegInfo.virReg,
        1,
        instEnable,
        VIR_HALF_CHANNEL_MASK_FULL,
        gcvNULL);
}

static void _VSC_PH_DeleteRedundantMOV(
    IN OUT VSC_PH_Peephole* ph,
    IN VIR_Instruction *pMovInst,
    OUT gctBOOL    *changed)
{
    VIR_BB* bb = VSC_PH_Peephole_GetCurrBB(ph);
    VIR_Instruction* nextInst = VIR_Inst_GetNext(pMovInst);
    VIR_Operand *destOpnd = VIR_Inst_GetDest(pMovInst);
    VIR_Operand *src0Opnd = VIR_Inst_GetSource(pMovInst, 0);
    VIR_Enable destEnable = VIR_Operand_GetEnable(destOpnd);
    VIR_Swizzle src0Swizzle = VIR_Operand_GetSwizzle(src0Opnd);
    VIR_Symbol *destSym = VIR_Operand_GetSymbol(destOpnd);
    VIR_Symbol *src0Sym = (VIR_Operand_isImm(src0Opnd) || VIR_Operand_isConst(src0Opnd)) ? gcvNULL: VIR_Operand_GetSymbol(src0Opnd);

    while (nextInst && nextInst != BB_GET_END_INST(bb))
    {
        /* check if any defination to src0Sym or redefine dest by instruction not MOV */
        VIR_Operand *nextDest = VIR_Inst_GetDest(nextInst);
        if ((nextDest &&
             ((VIR_Operand_GetSymbol(nextDest) == src0Sym) ||
              (VIR_Inst_GetOpcode(nextInst) != VIR_OP_MOV && VIR_Operand_GetSymbol(nextDest) == destSym))) ||
            VIR_Inst_GetOpcode(nextInst) == VIR_OP_EMIT ||
            VIR_Inst_GetOpcode(nextInst) == VIR_OP_EMIT0)
        {
            break;
        }
        if (VIR_Inst_GetOpcode(nextInst) == VIR_OP_MOV &&
            VIR_Operand_GetSymbol(VIR_Inst_GetDest(nextInst)) == destSym)
        {
            VIR_Operand *nextInstsrc0 = VIR_Inst_GetSource(nextInst, 0);
            /* if src0opnd is imm, const, sym */
            if ((VIR_Operand_GetEnable(VIR_Inst_GetDest(nextInst)) == destEnable) &&
                (VIR_Operand_GetTypeId(VIR_Inst_GetDest(nextInst)) == VIR_Operand_GetTypeId(destOpnd)) &&
                (VIR_Operand_GetSwizzle(nextInstsrc0) == src0Swizzle) &&
                ((VIR_Operand_isImm(src0Opnd) &&
                  VIR_Operand_isImm(nextInstsrc0) &&
                  VIR_Operand_GetImmediateUint(src0Opnd) == VIR_Operand_GetImmediateUint(nextInstsrc0)) ||
                 (VIR_Operand_isConst(src0Opnd) &&
                  VIR_Operand_isConst(nextInstsrc0) &&
                  VIR_Operand_GetConstId(src0Opnd) == VIR_Operand_GetConstId(nextInstsrc0)) ||
                 (src0Sym && VIR_Operand_GetSymbol(nextInstsrc0) == src0Sym)))
            {
                /*change instruction to nop*/
                _VSC_PH_Inst_DeleteUses(ph, nextInst, VIR_Inst_GetSrcNum(nextInst));
                _VSC_PH_Inst_DeleteDef(ph, nextInst);
                VIR_Function_ChangeInstToNop(VSC_PH_Peephole_GetCurrFunc(ph), nextInst);
                VSC_PH_Peephole_SetCfgChanged(ph, gcvTRUE); /* invalid du infomation */
                if (changed != gcvNULL)
                {
                    *changed = gcvTRUE;
                }
            }
            else
            {
                /* dest is redefined by other value */
                break;
            }
        }
        nextInst = VIR_Inst_GetNext(nextInst);
    }
    return;
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
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetModifiers(options), VSC_OPTN_PHOptions_MODIFIERS_CONJ))
        {
            mtgs[count++] = &VSC_PH_ModifierToGen_CONJ;
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

    /* Merge two ADD/SUB instruction to one. */
    if (VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetOPTS(options), VSC_OPTN_PHOptions_OPTS_MERGE_ADD))
    {
        inst = BB_GET_START_INST(bb);
        while (inst != VIR_Inst_GetNext(BB_GET_END_INST(bb)))
        {
            VIR_OpCode  opCode = VIR_Inst_GetOpcode(inst);
            gctBOOL     bChanged = gcvFALSE;

            if (opCode == VIR_OP_ADD || opCode == VIR_OP_SUB || opCode == VIR_OP_MAD)
            {
                _VSC_PH_MergeAddSubSameValue(ph, inst, &bChanged);
            }
            inst = VIR_Inst_GetNext(inst);
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

    /* merge lshift+L/S instructions to left-shifted L/S instruction */
    if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetOPTS(options), VSC_OPTN_PHOptions_OPTS_LSHIFT_LS) &&
       VSC_PH_Peephole_GetHwCfg(ph)->hwFeatureFlags.hasHalti4)
    {
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_LSHIFT_LS))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "%s\nLSHIFT_LS started\n%s\n", VSC_TRACE_SHARP_LINE, VSC_TRACE_SHARP_LINE);
            VIR_LOG_FLUSH(dumper);
        }
        inst = BB_GET_START_INST(bb);
        while(inst != VIR_Inst_GetNext(BB_GET_END_INST(bb)))
        {
            VIR_OpCode opc;

            opc = VIR_Inst_GetOpcode(inst);
            if(opc == VIR_OP_LSHIFT)
            {
                _VSC_PH_GenerateLShiftedLS(ph, inst, gcvNULL);
            }
            inst = VIR_Inst_GetNext(inst);
        }
        if(VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options), VSC_OPTN_PHOptions_TRACE_LSHIFT_LS))
        {
            VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
            VIR_LOG(dumper, "%s\nLSHIFT_LS ended\n%s\n", VSC_TRACE_SHARP_LINE, VSC_TRACE_SHARP_LINE);
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
        ON_ERROR(_VSC_PH_InitHashTable(ph, &VSC_PH_Peephole_DefSet(ph), _HKCMP_MergeKeyHFUNC, _HKCMP_MergeKeyEqual, 2048),
                 "Failed to initialize Hashtable");
        vec_def_set = VSC_PH_Peephole_DefSet(ph);
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

        _VSC_PH_ResetHashTable(vec_def_set);
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
            gctBOOL checkingResult;
            gctUINT i;

            opc = VIR_Inst_GetOpcode(inst);
            if (opc == VIR_OP_JMPC || opc == VIR_OP_JMP_ANY || opc == VIR_OP_CMOV)
            {
                if (VIR_Inst_CanGetConditionResult(inst))
                {
                    checkingResult = VIR_Inst_EvaluateConditionResult(inst, gcvNULL);

                    if (VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetTrace(options),
                        VSC_OPTN_PHOptions_TRACE_RUC))
                    {
                        VIR_Dumper* dumper = VSC_PH_Peephole_GetDumper(ph);
                        VIR_LOG(dumper, "\nInstruction\n");
                        VIR_Inst_Dump(dumper, inst);
                        VIR_LOG_FLUSH(dumper);
                    }

                    if (opc == VIR_OP_JMPC || opc == VIR_OP_JMP_ANY)
                    {
                        if (checkingResult)
                        {
                            /* change instruction to jmp*/
                            VIR_Inst_SetOpcode(inst, VIR_OP_JMP);
                            VIR_Inst_SetConditionOp(inst, VIR_COP_ALWAYS);
                            /* update du information */
                            _VSC_PH_Inst_DeleteUses(ph, inst, VIR_Inst_GetSrcNum(inst));
                            for (i = 0; i < VIR_Inst_GetSrcNum(inst); i++)
                            {
                                VIR_Inst_FreeSource(inst, i);
                            }
                            VIR_Inst_SetSrcNum(inst, 0);

                        }
                        else
                        {
                            /* update du information */
                            _VSC_PH_Inst_DeleteUses(ph, inst, VIR_Inst_GetSrcNum(inst));
                            VIR_Function_ChangeInstToNop(VSC_PH_Peephole_GetCurrFunc(ph), inst);
                        }
                    }
                    else
                    {
                        if (checkingResult)
                        {
                                /* change instruction to MOV*/
                            VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                            VIR_Inst_SetConditionOp(inst, VIR_COP_ALWAYS);
                            /* update du information - only delete src0, src1 */
                            _VSC_PH_Inst_DeleteUses(ph, inst, 2);
                            VIR_Inst_SetSource(inst, 0, VIR_Inst_GetSource(inst, 2));
                            VIR_Inst_SetSrcNum(inst, 1);
                        }
                        else
                        {
                            /*change instruction to nop*/
                            _VSC_PH_Inst_DeleteUses(ph, inst, VIR_Inst_GetSrcNum(inst));
                            _VSC_PH_Inst_DeleteDef(ph, inst);
                            VIR_Function_ChangeInstToNop(VSC_PH_Peephole_GetCurrFunc(ph), inst);
                        }
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
            VIR_Instruction *defInst = gcvNULL, *origDefInst = gcvNULL, *defDefInst = gcvNULL;

            opc = VIR_Inst_GetOpcode(inst);
            if (VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetOPTS(options), VSC_OPTN_PHOptions_OPTS_MOV_ATOM) &&
                VIR_OPCODE_isAtom(opc))
            {
                srcOpnd = VIR_Inst_GetSource(inst, 0);

                if (vscVIR_DoesUsageInstHaveUniqueDefInst(VSC_PH_Peephole_GetDUInfo(ph), inst, srcOpnd, gcvFALSE, &defInst) &&
                    vscVIR_IsUniqueUsageInstOfDefInst(VSC_PH_Peephole_GetDUInfo(ph), defInst, inst, gcvNULL, gcvFALSE, gcvNULL, gcvNULL, gcvNULL))
                {
                    _VSC_PH_MoveDefCode(ph, inst, srcOpnd, &defInst);
                }
            }

            if (VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetOPTS(options), VSC_OPTN_PHOptions_OPTS_MOV_LDARR) &&
                _VSC_PH_SrcFromUniqueLDARR(ph, inst, &srcOpnd, &defInst))
            {
                origDefInst = defInst;
                _VSC_PH_MoveDefCode(ph, inst, srcOpnd, &defInst);

                srcOpnd = VIR_Inst_GetSource(origDefInst, 1);
                if (vscVIR_DoesUsageInstHaveUniqueDefInst(VSC_PH_Peephole_GetDUInfo(ph), origDefInst, srcOpnd, gcvFALSE, &defDefInst) &&
                    vscVIR_IsUniqueUsageInstOfDefInst(VSC_PH_Peephole_GetDUInfo(ph), defDefInst, origDefInst, gcvNULL, gcvFALSE, gcvNULL, gcvNULL, gcvNULL))
                {
                    _VSC_PH_MoveDefCode(ph, defInst, srcOpnd, &defDefInst);
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

    /* change the load into load_L, if the number of threads can still make the
       max local group size requirement
    */
    if (VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetOPTS(options), VSC_OPTN_PHOptions_OPTS_LOC_MEM) &&
        ph->hwCfg->maxLocalMemSizeInByte > 0)
    {
        VIR_Shader      *pShader = ph->shader;
        gctUINT         localMemorySize = VIR_Shader_GetShareMemorySize(pShader);
        gctBOOL         useLocalMem = VIR_Shader_UseLocalMem(pShader);

        if ((localMemorySize <= ph->hwCfg->maxLocalMemSizeInByte) &&
            (localMemorySize > 0))
        {
            gcmASSERT(VIR_Shader_IsCLFromLanguage(pShader) || VIR_Shader_IsGlCompute(pShader));

            inst = BB_GET_START_INST(bb);
            while (inst != VIR_Inst_GetNext(BB_GET_END_INST(bb)))
            {
                VIR_OpCode opc;

                opc = VIR_Inst_GetOpcode(inst);
                if (opc == VIR_OP_LOAD  ||
                    opc == VIR_OP_STORE ||
                    VIR_OPCODE_isAtom(opc))
                {
                    _VSC_PH_LocalInst(ph, inst, &useLocalMem);
                }

                inst = VIR_Inst_GetNext(inst);
            }
            if (useLocalMem)
                VIR_Shader_SetFlag(pShader, VIR_SHFLAG_USE_LOCAL_MEM);
        }
    }

    /*
    ** Merge ADD/LOAD instructions and generate LOAD/STORE instruction:
    **  ADD   r1, r2, r3
    **  LOAD  r4, r1, 0
    ** -->
    **  LOAD  r4, r2, r3
    */
    if (VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetOPTS(options), VSC_OPTN_PHOptions_OPTS_ADD_MEM_ADDR))
    {
        inst = BB_GET_START_INST(bb);
        while (inst != VIR_Inst_GetNext(BB_GET_END_INST(bb)))
        {
            VIR_OpCode opc;
            /*
            ** We may delete this ADD instruction within this optimization,
            ** so we need to get the next instruction first.
            */
            VIR_Instruction* pNextInst = VIR_Inst_GetNext(inst);

            opc = VIR_Inst_GetOpcode(inst);
            if (opc == VIR_OP_ADD)
            {
                _VSC_PH_GenerateLoadStore(ph, inst, gcvNULL);
            }
            inst = pNextInst;
        }
    }

    /* Delete redundant mov defination
     * MOV  dest, t1
     * MOV  dest, t1   <- delete this mov if no t1 def between two MOVs
     */
    if (VSC_UTILS_MASK(VSC_OPTN_PHOptions_GetOPTS(options), VSC_OPTN_PHOptions_OPTS_REDUNDANT_MOV_DEF))
    {
        inst = BB_GET_START_INST(bb);
        while (inst != BB_GET_END_INST(bb))
        {
            if (VIR_Inst_GetOpcode(inst) == VIR_OP_MOV)
            {
                _VSC_PH_DeleteRedundantMOV(ph, inst, gcvNULL);
            }
            inst = VIR_Inst_GetNext(inst);
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

OnError:
    return errCode;
}

static VSC_ErrCode VSC_PH_Peephole_PerformOnFunction(
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
                errCode = _VSC_PH_PerformOnBB(ph);
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

DEF_QUERY_PASS_PROP(VSC_PH_Peephole_PerformOnShader)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_LL;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_PH;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(VSC_PH_Peephole_PerformOnShader)
{
    return gcvTRUE;
}

VSC_ErrCode VSC_PH_Peephole_PerformOnShader(
    VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode errcode  = VSC_ERR_NONE;
    VIR_Shader* shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_FuncIterator func_iter;
    VIR_FunctionNode* func_node;
    VSC_OPTN_PHOptions tempOptions = *(VSC_OPTN_PHOptions*)pPassWorker->basePassWorker.pBaseOption;
    VSC_OPTN_PHOptions* options = &tempOptions;
    VSC_PH_Peephole ph;

    if(!VSC_OPTN_InRange(VIR_Shader_GetId(shader), VSC_OPTN_PHOptions_GetBeforeShader(options), VSC_OPTN_PHOptions_GetAfterShader(options)))
    {
        if(VSC_OPTN_PHOptions_GetTrace(options))
        {
            VIR_Dumper* dumper = pPassWorker->basePassWorker.pDumper;
            VIR_LOG(dumper, "Peephole skips shader(%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
        }
        return errcode;
    }
    else
    {
        if(VSC_OPTN_PHOptions_GetTrace(options))
        {
            VIR_Dumper* dumper = pPassWorker->basePassWorker.pDumper;
            VIR_LOG(dumper, "Peephole starts for shader(%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
        }
    }

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "Before Peephole.", shader, gcvTRUE);
    }

    /* clean up this flag, and it will be set during peephole on bb */
    VIR_Shader_ClrFlag(shader, VIR_SHFLAG_USE_LOCAL_MEM);

    VSC_PH_Peephole_Init(&ph, shader, pPassWorker->pDuInfo, &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg,
                         options, pPassWorker->basePassWorker.pDumper, pPassWorker->basePassWorker.pMM);

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function* func = func_node->function;

        VIR_Shader_SetCurrentFunction(shader, func);
        errcode = VSC_PH_Peephole_PerformOnFunction(&ph);
        if(errcode)
        {
            break;
        }
    }

    pPassWorker->pResDestroyReq->s.bInvalidateCfg = VSC_PH_Peephole_GetCfgChanged(&ph);

    VSC_PH_Peephole_Final(&ph);

    if(VSC_OPTN_PHOptions_GetTrace(options))
    {
        VIR_Dumper* dumper = pPassWorker->basePassWorker.pDumper;
        VIR_LOG(dumper, "Peephole ends for shader(%d)\n", VIR_Shader_GetId(shader));
        VIR_LOG_FLUSH(dumper);
    }
    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Peephole.", shader, gcvTRUE);
    }

    return errcode;
}

