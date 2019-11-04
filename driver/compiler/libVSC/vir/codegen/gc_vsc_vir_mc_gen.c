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
#include "vir/codegen/gc_vsc_vir_mc_gen.h"

static VSC_ErrCode
_VSC_MC_GEN_GenInst(
    IN VSC_MCGen       *Gen,
    IN VIR_Function    *Func,
    IN VIR_Instruction *Inst,
    IN gctBOOL         bIsBackFill,
    OUT gctUINT        *GenCount
    );

static void _CheckNeedToBeFixed(IN VIR_Instruction  *Inst, gctUINT* GenCount)
{
    gctSIZE_T  i = 0;
    VIR_OpCode opcode = VIR_Inst_GetOpcode(Inst);

    *GenCount = 1;

    /* !!!NOTICE!!!:
          All following assertions mean lower failed to lower these opcodes to correct
          mc level opcodes, please check lower code and fix it. */

    if (!(VIR_Opcode_GetInfo((opcode))->level & VIR_OPLEVEL_Machine))
    {
        gcmASSERT(gcvFALSE);
        *GenCount = 0;
    }

    for(i = 0; i < VIR_Inst_GetSrcNum(Inst); ++i)
    {
        if (VIR_Operand_GetOpKind(VIR_Inst_GetSource(Inst, i)) == VIR_OPND_TEXLDPARM)
        {
            gcmASSERT(gcvFALSE);
            *GenCount = 0;
        }
    }
}

static VSC_ErrCode
_VSC_MC_GEN_InitializeInstMark(
    IN  VIR_Shader            *pShader,
    IN  VSC_MC_InstMark       *pInstMark,
    IN  gctINT                maxInstId
    )
{
    VIR_FuncIterator    func_iter;
    VIR_FunctionNode   *func_node;
    gctINT              instId   = 0;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));

    for (func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function    *func       = func_node->function;
        VIR_Instruction *inst       = gcvNULL;

        inst = func->instList.pHead;
        while (inst != gcvNULL)
        {
            gcmASSERT(VIR_Inst_GetId(inst) == instId);

            pInstMark[instId].Label = -1;
            pInstMark[instId].Inst  = gcvNULL;
            pInstMark[instId].LabelInst = inst;

            instId++;
            inst = VIR_Inst_GetNext(inst);
        }
    }

    gcmASSERT(instId == maxInstId);

    return VSC_ERR_NONE;
}

static VSC_ErrCode
_VSC_MC_GEN_Initialize(
    IN  VIR_Shader            *Shader,
    IN  VSC_COMPILER_CONFIG   *pComCfg,
    IN  VSC_OPTN_MCGenOptions *Options,
    IN  VIR_Dumper            *Dumper,
    IN  VSC_MM                *pMM,
    OUT VSC_MCGen             *Gen
    )
{
    gctINT              maxInstId   = 0;

    Gen->Shader  = Shader;
    Gen->pComCfg = pComCfg;
    Gen->Options = Options;
    Gen->Dumper  = Dumper;
    Gen->pMM     = pMM;

    vscMC_BeginCodec(&Gen->MCCodec, &Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg, VIR_Shader_isDual16Mode(Shader), gcvTRUE);

    maxInstId = VIR_Shader_RenumberInstId(Gen->Shader);

    if (maxInstId > 0)
    {
        Gen->InstMark = (VSC_MC_InstMark *)vscMM_Alloc(Gen->pMM,
            sizeof(VSC_MC_InstMark) * maxInstId);

        _VSC_MC_GEN_InitializeInstMark(Gen->Shader, Gen->InstMark, maxInstId);
    }

    Gen->InstCount = 0;

    /* dEQP, CTS and WEBGL requires RTNE (for precision purpose) */
    if (pComCfg->cFlags & VSC_COMPILER_FLAG_NEED_RTNE_ROUNDING)
    {
        Gen->RTNERequired   = gcvTRUE;
    }
    else
    {
        Gen->RTNERequired   = gcvFALSE;
    }

    return VSC_ERR_NONE;
}

static VSC_ErrCode
_VSC_MC_GEN_Finalize(
    IN OUT VSC_MCGen    *Gen
    )
{
    Gen->Shader   = gcvNULL;
    Gen->pComCfg  = gcvNULL;
    Gen->InstMark = gcvNULL;
    vscMC_EndCodec(&Gen->MCCodec);

    return VSC_ERR_NONE;
}

static gctBOOL
_VSC_MC_GEN_IsTypeEqualTo(
    IN VIR_Operand *Opnd,
    IN VIR_TyFlag   TyFlag
)
{
    VIR_TypeId   ty   = VIR_Operand_GetTypeId(Opnd);
    gcmASSERT(ty < VIR_TYPE_PRIMITIVETYPE_COUNT);

    return (VIR_GetTypeFlag(ty) & TyFlag);
}

static void
_VSC_MC_GEN_GenOpcode(
    IN VSC_MCGen        *Gen,
    IN  VIR_Instruction *Inst,
    OUT gctUINT         *BaseOpcode,
    OUT gctUINT         *ExternOpcode
    )
{
    VIR_OpCode opcode = VIR_Inst_GetOpcode(Inst);

    *ExternOpcode = 0;

    switch(opcode)
    {
    case VIR_OP_NOP:
        *BaseOpcode = 0x00;
        break;
    case VIR_OP_MOV:
        *BaseOpcode = 0x09;
        break;
    case VIR_OP_CMOV:
        *BaseOpcode = 0x09;
        break;
    case VIR_OP_SAT:
        *BaseOpcode = 0x09; /* NOTE */
        break;
    case VIR_OP_ABS:
        if (_VSC_MC_GEN_IsTypeEqualTo(VIR_Inst_GetSource(Inst, 0), VIR_TYFLAG_ISINTEGER))
        {
            *BaseOpcode = 0x57;
        }
        else
        {
            *BaseOpcode = 0x09; /* NOTE */
        }

        break;
    case VIR_OP_NEG:
        if (_VSC_MC_GEN_IsTypeEqualTo(VIR_Inst_GetSource(Inst, 0), VIR_TYFLAG_ISINTEGER))
        {
            gcmASSERT(0);
            *BaseOpcode = 0x01;
        }
        else
        {
            *BaseOpcode = 0x09; /* NOTE */
        }

        break;
    case VIR_OP_FLOOR:
        *BaseOpcode = 0x25;
        break;
    case VIR_OP_CEIL:
        *BaseOpcode = 0x26;
        break;
    case VIR_OP_LOG2:
        *BaseOpcode = 0x12;
        break;
    case VIR_OP_EXP2:
        *BaseOpcode = 0x11;
        break;
    case VIR_OP_SIGN:
        *BaseOpcode = 0x27;
        break;
    case VIR_OP_FRAC:
        *BaseOpcode = 0x13;
        break;
    case VIR_OP_RCP:
        *BaseOpcode = 0x0C;
        break;
    case VIR_OP_RSQ:
        *BaseOpcode = 0x0D;
        break;
    case VIR_OP_SQRT:
        *BaseOpcode = 0x21;
        break;
    case VIR_OP_SINPI:
        *BaseOpcode = 0x22;
        break;
    case VIR_OP_COSPI:
        *BaseOpcode = 0x23;
        break;
    case VIR_OP_ARCTRIG:
        *BaseOpcode = 0x63;
        break;
    case VIR_OP_ADD:
        if (_VSC_MC_GEN_IsTypeEqualTo(Inst->dest, VIR_TYFLAG_ISINTEGER) &&
            VIR_Operand_GetModifier(Inst->dest) == VIR_MOD_SAT_TO_MAX_UINT)
        {
            *BaseOpcode = 0x3B;
        }
        else
        {
            *BaseOpcode = 0x01;
        }
        break;
    case VIR_OP_ADDSAT:
        gcmASSERT(_VSC_MC_GEN_IsTypeEqualTo(Inst->dest, VIR_TYFLAG_ISINTEGER));
        *BaseOpcode = 0x3B;
        break;
    case VIR_OP_IMADHI0:
        if (_VSC_MC_GEN_IsTypeEqualTo(Inst->dest, VIR_TYFLAG_ISINTEGER) &&
            VIR_Operand_GetModifier(Inst->dest) == VIR_MOD_SAT_TO_MAX_UINT)
        {
            *BaseOpcode = 0x52;
        }
        else
        {
            *BaseOpcode = 0x50;
        }
        break;
    case VIR_OP_IMADHI1:
        if (_VSC_MC_GEN_IsTypeEqualTo(Inst->dest, VIR_TYFLAG_ISINTEGER) &&
            VIR_Operand_GetModifier(Inst->dest) == VIR_MOD_SAT_TO_MAX_UINT)
        {
            *BaseOpcode = 0x53;
        }
        else
        {
            *BaseOpcode = 0x51;
        }
        break;
    case VIR_OP_IMADLO0:
        if (_VSC_MC_GEN_IsTypeEqualTo(Inst->dest, VIR_TYFLAG_ISINTEGER) &&
            VIR_Operand_GetModifier(Inst->dest) == VIR_MOD_SAT_TO_MAX_UINT)
        {
            *BaseOpcode = 0x4E;
        }
        else
        {
            *BaseOpcode = 0x4C;
        }
        break;
    case VIR_OP_IMADLO1:
        if (_VSC_MC_GEN_IsTypeEqualTo(Inst->dest, VIR_TYFLAG_ISINTEGER) &&
            VIR_Operand_GetModifier(Inst->dest) == VIR_MOD_SAT_TO_MAX_UINT)
        {
            *BaseOpcode = 0x4F;
        }
        else
        {
            *BaseOpcode = 0x4D;
        }
        break;
    case VIR_OP_SUB:
        if (_VSC_MC_GEN_IsTypeEqualTo(Inst->dest, VIR_TYFLAG_ISINTEGER) &&
            VIR_Operand_GetModifier(Inst->dest) == VIR_MOD_SAT_TO_MAX_UINT)
        {
            *BaseOpcode = 0x3B;
        }
        else
        {
            *BaseOpcode = 0x01; /* NOTE!! */
        }
        break;
    case VIR_OP_SUBSAT:
        gcmASSERT(_VSC_MC_GEN_IsTypeEqualTo(Inst->dest, VIR_TYFLAG_ISINTEGER));
        *BaseOpcode = 0x3B;
        break;
    case VIR_OP_MUL:
        if (_VSC_MC_GEN_IsTypeEqualTo(Inst->dest, VIR_TYFLAG_ISINTEGER))
        {
            if (VIR_Operand_GetModifier(Inst->dest) == VIR_MOD_SAT_TO_MAX_UINT)
            {
                *BaseOpcode = 0x3E;
            }
            else
            {
                *BaseOpcode = 0x3C;
            }
        }
        else
        {
            *BaseOpcode = 0x03;
        }
        break;
    case VIR_OP_MULSAT:
        gcmASSERT(_VSC_MC_GEN_IsTypeEqualTo(Inst->dest, VIR_TYFLAG_ISINTEGER));
        *BaseOpcode = 0x3E;
        break;
    case VIR_OP_MUL_Z:
        gcmASSERT(_VSC_MC_GEN_IsTypeEqualTo(Inst->dest, VIR_TYFLAG_ISFLOAT));
        *BaseOpcode = 0x03;
        break;
    case VIR_OP_DIV:
        if (_VSC_MC_GEN_IsTypeEqualTo(Inst->dest, VIR_TYFLAG_ISINTEGER))
        {
            *BaseOpcode = 0x44;
        }
        else
        {
            *BaseOpcode = 0x64;
        }
        break;
    case VIR_OP_IMOD:
        *BaseOpcode = 0x48;
        break;
    case VIR_OP_ADDLO:
        *BaseOpcode = 0x28;
        break;
    case VIR_OP_MULLO:
        *BaseOpcode = 0x29;
        break;
    case VIR_OP_MULHI:
        *BaseOpcode = 0x40;
        break;
    case VIR_OP_DP2:
        *BaseOpcode = 0x73;
        break;
    case VIR_OP_DP3:
        *BaseOpcode = 0x05;
        break;
    case VIR_OP_DP4:
        *BaseOpcode = 0x06;
        break;
    case VIR_OP_NORM_MUL:
        *BaseOpcode = 0x77;
        break;
    case VIR_OP_NORM_DP2:
        *BaseOpcode = 0x74;
        break;
    case VIR_OP_NORM_DP3:
        *BaseOpcode = 0x75;
        break;
    case VIR_OP_NORM_DP4:
        *BaseOpcode = 0x76;
        break;
    case VIR_OP_STEP:
        *BaseOpcode = 0x10;
        break;
    case VIR_OP_DSX:
        *BaseOpcode = 0x07;
        break;
    case VIR_OP_DSY:
        *BaseOpcode = 0x08;
        break;
    case VIR_OP_AND_BITWISE:
        *BaseOpcode = 0x5D;
        break;
    case VIR_OP_OR_BITWISE:
        *BaseOpcode = 0x5C;
        break;
    case VIR_OP_NOT_BITWISE:
        *BaseOpcode = 0x5F;
        break;
    case VIR_OP_XOR_BITWISE:
        *BaseOpcode = 0x5E;
        break;
    case VIR_OP_LSHIFT:
        *BaseOpcode = 0x59;
        break;
    case VIR_OP_RSHIFT:
        *BaseOpcode = 0x5A;
        break;
    case VIR_OP_ROTATE:
        *BaseOpcode = 0x5B;
        break;
    case VIR_OP_STORE:
    case VIR_OP_STORE_S:
    case VIR_OP_STORE_L:
        if (VIR_TypeId_isPacked(VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 2))))
        {
            *BaseOpcode = (VIR_Inst_Store_Have_Dst(Inst) && Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti5) ?
                          MC_AUXILIARY_OP_CODE_USC_STOREP :
                          0x3A;
        }
        else
        {
            *BaseOpcode = (VIR_Inst_Store_Have_Dst(Inst) && Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti5) ?
                          MC_AUXILIARY_OP_CODE_USC_STORE :
                          0x33;
        }
        break;
    case VIR_OP_LOAD:
    case VIR_OP_LOAD_L:
    case VIR_OP_LOAD_S:
        if (VIR_TypeId_isPacked(VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst))))
        {
            *BaseOpcode = 0x39;
        }
        else
        {
            *BaseOpcode = 0x32;
        }
        break;
    case VIR_OP_KILL:
        *BaseOpcode = 0x17;
        break;
    case VIR_OP_BITINSERT1:
        *BaseOpcode = 0x54;
        break;
    case VIR_OP_BITINSERT2:
        *BaseOpcode = 0x55;
        break;
    case VIR_OP_SWIZZLE:
        *BaseOpcode = 0x2B;
        break;
    case VIR_OP_BARRIER:
        if (VIR_Shader_GetKind(Gen->Shader) == VIR_SHADER_TESSELLATION_CONTROL)
        {
            *BaseOpcode = (Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.maxCoreCount >= 8) ? 0x00 :
                                                                      0x2A;
        }
        else if (VIR_Shader_GetKind(Gen->Shader) == VIR_SHADER_COMPUTE)
        {
            gctUINT workgrpSize = VIR_Shader_GetWorkGroupSize(Gen->Shader);

            if ((workgrpSize > 0) &&
                (workgrpSize <= (Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.maxCoreCount * 4)))
            {
                *BaseOpcode = 0x00;
            }
            else
            {
                *BaseOpcode = 0x2A;
            }
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
        break;
    case VIR_OP_MEM_BARRIER:
        *BaseOpcode = 0x00; /* HW has logic to naturally insure memory access is in-order */
        break;
    case VIR_OP_ATOMADD:
    case VIR_OP_ATOMADD_S:
    case VIR_OP_ATOMADD_L:
        *BaseOpcode = 0x65;
        break;
    case VIR_OP_ATOMSUB:
    case VIR_OP_ATOMSUB_L:
        *BaseOpcode = 0x65;    /* NOTE!!! */
        break;
    case VIR_OP_ATOMXCHG:
    case VIR_OP_ATOMXCHG_L:
        *BaseOpcode = 0x66;
        break;
    case VIR_OP_ATOMCMPXCHG:
    case VIR_OP_ATOMCMPXCHG_L:
        *BaseOpcode = 0x67;
        break;
    case VIR_OP_ATOMMIN:
    case VIR_OP_ATOMMIN_L:
        *BaseOpcode = 0x68;
        break;
    case VIR_OP_ATOMMAX:
    case VIR_OP_ATOMMAX_L:
        *BaseOpcode = 0x69;
        break;
    case VIR_OP_ATOMOR:
    case VIR_OP_ATOMOR_L:
        *BaseOpcode = 0x6A;
        break;
    case VIR_OP_ATOMAND:
    case VIR_OP_ATOMAND_L:
        *BaseOpcode = 0x6B;
        break;
    case VIR_OP_ATOMXOR:
    case VIR_OP_ATOMXOR_L:
        *BaseOpcode = 0x6C;
        break;
    case VIR_OP_LEADZERO:
        *BaseOpcode = 0x58;
        break;
    case VIR_OP_JMP:
        *BaseOpcode = 0x16;
        break;
    case VIR_OP_JMPC:
        *BaseOpcode = 0x16;
        break;
    case VIR_OP_JMP_ANY:
        *BaseOpcode = 0x24;
        break;
    case VIR_OP_CALL:
        *BaseOpcode = 0x14;
        break;
    case VIR_OP_RET:
        *BaseOpcode = 0x15;
        break;
    case VIR_OP_CONV:
        *BaseOpcode = 0x72;
        break;
    case VIR_OP_F2I:
        *BaseOpcode = 0x2E;
        break;
    case VIR_OP_I2I:
        *BaseOpcode = 0x2C;
        break;
    case VIR_OP_I2F:
        *BaseOpcode = 0x2D;
        break;
    case VIR_OP_CMP:
        *BaseOpcode = 0x31;
        break;
    case VIR_OP_F2IRND:
        *BaseOpcode = 0x2F;
        break;
    case VIR_OP_SELECT:
        *BaseOpcode = 0x0F;
        break;
    case VIR_OP_SET:
        *BaseOpcode = 0x10;
        break;
    case VIR_OP_MOVA:
        {
            VIR_Operand *dest = VIR_Inst_GetDest(Inst);

            *BaseOpcode = _VSC_MC_GEN_IsTypeEqualTo(dest, VIR_TYFLAG_ISFLOAT) ?
                            0x0B :
                            0x56;
            break;
        }
    case VIR_OP_PRE_DIV:
        *BaseOpcode = 0x64;
        break;
    case VIR_OP_PRE_LOG2:
        *BaseOpcode = 0x12;
        break;
    case VIR_OP_MAD:
        if (_VSC_MC_GEN_IsTypeEqualTo(Inst->dest, VIR_TYFLAG_ISINTEGER))
        {
            if (VIR_Operand_GetModifier(Inst->dest) == VIR_MOD_SAT_TO_MAX_UINT)
            {
                *BaseOpcode = 0x4E;
            }
            else
            {
                *BaseOpcode = 0x4C;
            }
        }
        else
        {
            *BaseOpcode = 0x02;
        }
        break;
    case VIR_OP_MADSAT:
        gcmASSERT(_VSC_MC_GEN_IsTypeEqualTo(Inst->dest, VIR_TYFLAG_ISINTEGER));
        *BaseOpcode = 0x4E;
        break;
    case VIR_OP_FMA:
        *BaseOpcode = 0x30;
        break;
    case VIR_OP_BITEXTRACT:
        *BaseOpcode = 0x60;
        break;
    case VIR_OP_BITREV:
        *BaseOpcode = 0x6D;
        break;
    case VIR_OP_BYTEREV:
        *BaseOpcode = 0x6E;
        break;
    case VIR_OP_POPCOUNT:
        *BaseOpcode = 0x61;
        break;
    case VIR_OP_TEXLD_BIAS:
        *BaseOpcode = MC_AUXILIARY_OP_CODE_TEXLD_BIAS;
        break;
    case VIR_OP_TEXLD_BIAS_PCF:
        *BaseOpcode = MC_AUXILIARY_OP_CODE_TEXLD_BIAS_PCF;
        break;
    case VIR_OP_TEXLD_PLAIN:
    case VIR_OP_TEXLD:
        *BaseOpcode = MC_AUXILIARY_OP_CODE_TEXLD_PLAIN;
        break;
    case VIR_OP_TEXLD_PCF:
        *BaseOpcode = MC_AUXILIARY_OP_CODE_TEXLD_PCF;
        break;
    case VIR_OP_TEXLDB:
        *BaseOpcode = 0x19;
        break;
    case VIR_OP_TEXLDD:
        *BaseOpcode = 0x1A;
        break;
    case VIR_OP_TEXLD_G:
        *BaseOpcode = 0x1A;
        break;
    case VIR_OP_TEXLDL:
        *BaseOpcode = 0x1B;
        break;
    case VIR_OP_TEXLDP:
    case VIR_OP_TEXLDPROJ:
        *BaseOpcode = 0x1C;
        break;
    case VIR_OP_TEXLD_LOD:
        *BaseOpcode = MC_AUXILIARY_OP_CODE_TEXLD_LOD;
        break;
    case VIR_OP_TEXLD_LOD_PCF:
        *BaseOpcode = MC_AUXILIARY_OP_CODE_TEXLD_LOD_PCF;
        break;
    case VIR_OP_TEXLD_G_PCF:
        *BaseOpcode = 0x70;
        break;
    case VIR_OP_TEXLD_U_PLAIN:
        *BaseOpcode = MC_AUXILIARY_OP_CODE_TEXLD_U_PLAIN;
        break;
    case VIR_OP_TEXLD_U_LOD:
        *BaseOpcode = MC_AUXILIARY_OP_CODE_TEXLD_U_LOD;
        break;
    case VIR_OP_TEXLD_U_BIAS:
        *BaseOpcode = MC_AUXILIARY_OP_CODE_TEXLD_U_BIAS;
        break;
    case VIR_OP_TEXLD_GATHER:
        *BaseOpcode = MC_AUXILIARY_OP_CODE_TEXLD_GATHER;
        break;
    case VIR_OP_TEXLD_GATHER_PCF:
        *BaseOpcode = MC_AUXILIARY_OP_CODE_TEXLD_GATHER_PCF;
        break;
    case VIR_OP_TEXLD_U_F_L:
        *BaseOpcode = 0x4B;
        break;
    case VIR_OP_TEXLD_U_F_B:
        *BaseOpcode = 0x7B;
        break;
    case VIR_OP_TEXLD_U_S_L:
        *BaseOpcode = 0x49;
        break;
    case VIR_OP_TEXLD_U_U_L:
        *BaseOpcode = 0x4A;
        break;
    case VIR_OP_IMG_LOAD:
    case VIR_OP_VX_IMG_LOAD:
        *BaseOpcode = 0x79;
        break;
    case VIR_OP_IMG_STORE:
    case VIR_OP_VX_IMG_STORE:
        *BaseOpcode = (VIR_Inst_Store_Have_Dst(Inst) && Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti5) ?
                      MC_AUXILIARY_OP_CODE_USC_IMG_STORE :
                      0x7A;
        break;
    case VIR_OP_IMG_LOAD_3D:
    case VIR_OP_VX_IMG_LOAD_3D:
        *BaseOpcode = 0x34;
        break;
    case VIR_OP_IMG_STORE_3D:
    case VIR_OP_VX_IMG_STORE_3D:
        *BaseOpcode = (VIR_Inst_Store_Have_Dst(Inst) && Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti5) ?
                      MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D :
                      0x35;
        break;
    case VIR_OP_CLAMP0MAX:
        *BaseOpcode = 0x36;
        break;
    case VIR_OP_IMG_ADDR:
        *BaseOpcode = 0x37;
        break;
    case VIR_OP_IMG_ADDR_3D:
        *BaseOpcode = 0x38;
        break;
    case VIR_OP_LOAD_ATTR:
    case VIR_OP_LOAD_ATTR_O:
        *BaseOpcode = 0x78;
        break;
    case VIR_OP_STORE_ATTR:
        *BaseOpcode = (VIR_Inst_Store_Have_Dst(Inst) && Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti5) ?
                      MC_AUXILIARY_OP_CODE_USC_STORE_ATTR :
                      0x42;
        break;
    case VIR_OP_SELECT_MAP:
        *BaseOpcode = 0x43;
        break;

    /* Below for extended opcodes */
    case VIR_OP_EMIT0:
    case VIR_OP_EMIT:
        *BaseOpcode = 0x7F;
        *ExternOpcode = 0x01;
        break;
    case VIR_OP_RESTART0:
    case VIR_OP_RESTART:
        *BaseOpcode = 0x7F;
        *ExternOpcode = 0x02;
        break;
    case VIR_OP_FLUSH:
        *BaseOpcode = 0x7F;
        *ExternOpcode = 0x03;
        break;
    case VIR_OP_LODQ:
        *BaseOpcode = 0x7F;
        *ExternOpcode = 0x04;
        break;
    case VIR_OP_BITFIND_LSB:
        *BaseOpcode = 0x7F;
        *ExternOpcode = 0x0B;
        break;
    case VIR_OP_BITFIND_MSB:
        *BaseOpcode = 0x7F;
        *ExternOpcode = 0x0C;
        break;
    case VIR_OP_TEXLD_FETCH_MS:
        *BaseOpcode = 0x7F;
        *ExternOpcode = 0x0D;
        break;
    case VIR_OP_GET_SAMPLER_IDX:
        if (_VSC_MC_GEN_IsTypeEqualTo(Inst->dest, VIR_TYFLAG_ISINTEGER) &&
            VIR_Operand_GetModifier(Inst->dest) == VIR_MOD_SAT_TO_MAX_UINT)
        {
            *BaseOpcode = 0x3B;
        }
        else
        {
            *BaseOpcode = 0x01;
        }
        break;
    case VIR_OP_VX_ABSDIFF:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x01;
        break;
    case VIR_OP_VX_IADD:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x02;
        break;
    case VIR_OP_VX_IACCSQ:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x03;
        break;
    case VIR_OP_VX_LERP:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x04;
        break;
    case VIR_OP_VX_FILTER:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x05;
        break;
    case VIR_OP_VX_MAGPHASE:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x06;
        break;
    case VIR_OP_VX_MULSHIFT:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x07;
        break;
    case VIR_OP_VX_DP16X1:
    case VIR_OP_VX_DP16X1_B:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x08;
        break;
    case VIR_OP_VX_DP8X2:
    case VIR_OP_VX_DP8X2_B:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x09;
        break;
    case VIR_OP_VX_DP4X4:
    case VIR_OP_VX_DP4X4_B:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x0A;
        break;
    case VIR_OP_VX_DP2X8:
    case VIR_OP_VX_DP2X8_B:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x0B;
        break;
    case VIR_OP_VX_DP32X1:
    case VIR_OP_VX_DP32X1_B:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x12;
        break;
    case VIR_OP_VX_DP16X2:
    case VIR_OP_VX_DP16X2_B:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x13;
        break;
    case VIR_OP_VX_DP8X4:
    case VIR_OP_VX_DP8X4_B:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x14;
        break;
    case VIR_OP_VX_DP4X8:
    case VIR_OP_VX_DP4X8_B:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x15;
        break;
    case VIR_OP_VX_DP2X16:
    case VIR_OP_VX_DP2X16_B:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x16;
        break;
    case VIR_OP_VX_GATHER:
    case VIR_OP_VX_GATHER_B:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x1E;
        break;
    case VIR_OP_VX_SCATTER:
    case VIR_OP_VX_SCATTER_B:
        *BaseOpcode   = 0x45;
        *ExternOpcode = (VIR_Inst_Store_Have_Dst(Inst) && Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti5) ?
                        MC_AUXILIARY_OP_CODE_USC_SCATTER :
                        0x1F;
        break;
    case VIR_OP_VX_ATOMIC_S:
    case VIR_OP_VX_ATOMIC_S_B:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x20;
        break;
    case VIR_OP_VX_CLAMP:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x0C;
        break;
    case VIR_OP_VX_BILINEAR:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x0D;
        break;
    case VIR_OP_VX_SELECTADD:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x0E;
        break;
    case VIR_OP_VX_ATOMICADD:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x0F;
        break;
    case VIR_OP_VX_BITEXTRACT:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x10;
        break;
    case VIR_OP_VX_BITREPLACE:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x11;
        break;
    case VIR_OP_VX_INDEXADD:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x17;
        break;
    case VIR_OP_VX_VERTMIN3:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x18;
        break;
    case VIR_OP_VX_VERTMAX3:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x19;
        break;
    case VIR_OP_VX_VERTMED3:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x1A;
        break;
    case VIR_OP_VX_HORZMIN3:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x1B;
        break;
    case VIR_OP_VX_HORZMAX3:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x1C;
        break;
    case VIR_OP_VX_HORZMED3:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x1D;
        break;
    case VIR_OP_CMAD:
        *BaseOpcode   = 0x62;
        *ExternOpcode = 0x1;
        break;
    case VIR_OP_CMUL:
        *BaseOpcode   = 0x62;
        *ExternOpcode = 0x0;
        break;
    case VIR_OP_CADD:
        *BaseOpcode   = 0x62;
        *ExternOpcode = 0x2;
        break;
    case VIR_OP_PACK:
        *BaseOpcode = 0x71;
        break;
    default:
        gcmASSERT(0);
        *BaseOpcode   = 0xffffffff;
        *ExternOpcode = 0xffffffff;
        break;
    }
}

static VSC_ErrCode
_VSC_MC_GEN_BackFill(
    IN VSC_MCGen       *Gen,
    IN VIR_Instruction *Inst,
    IN gctINT           Label
    )
{
    gctUINT origLabel = VIR_Inst_GetId(Inst);
    gctUINT label = origLabel;

    while (label != -1)
    {
        VIR_Instruction *jmpInst   = Gen->InstMark[label].Inst;
        gctUINT          nextLabel = Gen->InstMark[label].Label;
        gctUINT          t = 0;

        Gen->InstMark[label].Inst  = gcvNULL;

        /*
        ** Update all the users of this LABEL,
        ** and for those users that have been generated,
        ** use PC to replace the InstMark label because they may be the target of other instructions.
        */
        if (label == origLabel)
        {
            Gen->InstMark[label].Label = Label;
        }
        else
        {
            VIR_Instruction *labelInst = Gen->InstMark[label].LabelInst;

            if (labelInst && VIR_Inst_GetMCInstPC(labelInst))
            {
                Gen->InstMark[label].Label = VIR_Inst_GetMCInstPC(labelInst);
            }
            else
            {
                Gen->InstMark[label].Label = Label;
            }
        }

        /* Transform */
        _VSC_MC_GEN_GenInst(Gen, VIR_Inst_GetFunction(jmpInst), jmpInst, gcvTRUE, &t);

        label = nextLabel;
    }

    return VSC_ERR_NONE;
}

static gctUINT
_VSC_MC_GEN_GenCondition(
    IN VIR_Instruction *Inst
    )
{
    VIR_ConditionOp condtion = VIR_Inst_GetConditionOp(Inst);

    switch(condtion)
    {
    case VIR_COP_ALWAYS:
        if (VIR_Inst_GetOpcode(Inst) == VIR_OP_STEP)
        {
            return 0x04;
        }
        else
        {
            return 0x00;
        }
    case VIR_COP_GREATER:
        return 0x01;
    case VIR_COP_LESS:
        return 0x02;
    case VIR_COP_GREATER_OR_EQUAL:
        return 0x03;
    case VIR_COP_LESS_OR_EQUAL:
        return 0x04;
    case VIR_COP_EQUAL:
        return 0x05;
    case VIR_COP_NOT_EQUAL:
        return 0x06;
    case VIR_COP_AND:
        return 0x07;
    case VIR_COP_OR:
        return 0x08;
    case VIR_COP_XOR:
        return 0x09;
    case VIR_COP_NOT:
        return 0x0A;
    case VIR_COP_NOT_ZERO:
        return 0x0B;
    case VIR_COP_GREATER_OR_EQUAL_ZERO:
        return 0x0C;
    case VIR_COP_GREATER_ZERO:
        return 0x0D;
    case VIR_COP_LESS_OREQUAL_ZERO:
        return 0x0E;
    case VIR_COP_LESS_ZERO:
        return 0x0F;
    case VIR_COP_FINITE:
        return 0x10;
    case VIR_COP_INFINITE:
        return 0x11;
    case VIR_COP_NAN:
        return 0x12;
    case VIR_COP_NORMAL:
        return 0x13;
    case VIR_COP_ANYMSB:
        return 0x14;
    case VIR_COP_ALLMSB:
        return 0x15;
    case VIR_COP_SELMSB:
        return 0x16;
    case VIR_COP_UCARRY:
        return 0x17;
    case VIR_COP_HELPER:
        return 0x18;
    case VIR_COP_NOTHELPER:
        return 0x19;
    default:
        gcmASSERT(0);
        return 0x00;
    }
}

static gctUINT
_VSC_MC_GEN_GenRound(
    IN  VSC_MCGen       *Gen,
    IN  VIR_Instruction *Inst
    )
{
    VIR_Operand     *dest  = VIR_Inst_GetDest(Inst);
    VIR_RoundMode    round = VIR_ROUND_DEFAULT;

    if(dest == gcvNULL)
    {
        return 0x0;
    }

    round = VIR_Operand_GetRoundMode(dest);

    switch (round)
    {
    case VIR_ROUND_DEFAULT:
        if (!Gen->RTNERequired &&
            VIR_Shader_isDual16Mode(Gen->Shader) &&
            (VIR_Inst_GetThreadMode(Inst) == VIR_THREAD_D16_DUAL_16 ||
             VIR_Inst_GetThreadMode(Inst) == VIR_THREAD_D16_DUAL_HIGHPVEC2))
        {
            return 0x1;
        }
        else
        {
            return 0x0;
        }
    case VIR_ROUND_RTZ:
        return 0x1;
    case VIR_ROUND_RTE:
        return 0x2;
    case VIR_ROUND_RTP:
    case VIR_ROUND_RTN:
    default:
        /* OCL can have RTP/RTN rounding mode, use default mode
         * until we have real RTP/RTN mode
         */
        return 0x0;
    }
}

static gctUINT
_VSC_MC_GEN_GetInstTypeFromImgFmt(
    IN VIR_ImageFormat  imgQual
    )
{
    switch (imgQual)
    {
    case VIR_IMAGE_FORMAT_RGBA32F:
    case VIR_IMAGE_FORMAT_RG32F:
    case VIR_IMAGE_FORMAT_R32F:
    case VIR_IMAGE_FORMAT_RGBA16F:
    case VIR_IMAGE_FORMAT_RG16F:
    case VIR_IMAGE_FORMAT_R16F:
        return 0x0;

    case VIR_IMAGE_FORMAT_BGRA8_UNORM:
    case VIR_IMAGE_FORMAT_RGBA8:
    case VIR_IMAGE_FORMAT_RG8:
    case VIR_IMAGE_FORMAT_R8:
        return 0x0;

    case VIR_IMAGE_FORMAT_RGBA8_SNORM:
    case VIR_IMAGE_FORMAT_RG8_SNORM:
    case VIR_IMAGE_FORMAT_R8_SNORM:
        return 0x0;

    case VIR_IMAGE_FORMAT_RGBA16:
    case VIR_IMAGE_FORMAT_RG16:
    case VIR_IMAGE_FORMAT_R16:
        return 0x0;

    case VIR_IMAGE_FORMAT_RGBA16_SNORM:
    case VIR_IMAGE_FORMAT_RG16_SNORM:
    case VIR_IMAGE_FORMAT_R16_SNORM:
        return 0x0;

    case VIR_IMAGE_FORMAT_RGBA32I:
    case VIR_IMAGE_FORMAT_RG32I:
    case VIR_IMAGE_FORMAT_R32I:
    case VIR_IMAGE_FORMAT_RGBA16I:
    case VIR_IMAGE_FORMAT_RG16I:
    case VIR_IMAGE_FORMAT_R16I:
    case VIR_IMAGE_FORMAT_RGBA8I:
    case VIR_IMAGE_FORMAT_RG8I:
    case VIR_IMAGE_FORMAT_R8I:
    case VIR_IMAGE_FORMAT_ABGR8I_PACK32:
        return 0x2;

    case VIR_IMAGE_FORMAT_RGBA32UI:
    case VIR_IMAGE_FORMAT_RG32UI:
    case VIR_IMAGE_FORMAT_R32UI:
    case VIR_IMAGE_FORMAT_RGBA16UI:
    case VIR_IMAGE_FORMAT_RG16UI:
    case VIR_IMAGE_FORMAT_R16UI:
    case VIR_IMAGE_FORMAT_RGBA8UI:
    case VIR_IMAGE_FORMAT_RG8UI:
    case VIR_IMAGE_FORMAT_R8UI:
    case VIR_IMAGE_FORMAT_ABGR8UI_PACK32:
    case VIR_IMAGE_FORMAT_A2B10G10R10UI_PACK32:
        return 0x5;

    case VIR_IMAGE_FORMAT_R5G6B5_UNORM_PACK16:
    case VIR_IMAGE_FORMAT_ABGR8_UNORM_PACK32:
    case VIR_IMAGE_FORMAT_A2R10G10B10_UNORM_PACK32:
    case VIR_IMAGE_FORMAT_A2B10G10R10_UNORM_PACK32:
        return 0x0;

    default:
        gcmASSERT(gcvFALSE);
        break;;
    }

    return 0x2;
}

static gctUINT
_VSC_MC_GEN_GetInstType(
    IN  VSC_MCGen      *Gen,
    IN VIR_Instruction *Inst,
    IN VIR_Operand     *Opnd
    )
{
    VIR_OpCode      opcode = VIR_Inst_GetOpcode(Inst);
    VIR_Symbol     *sym         = VIR_Operand_GetSymbol(Opnd);
    VIR_TypeId      ty           = VIR_Operand_GetTypeId(Opnd);
    VIR_OperandKind opndKind     = VIR_Operand_GetOpKind(Opnd);
    VIR_TypeId      componentTy;
    VIR_ImageFormat imgQual;

    if(opndKind == VIR_OPND_NONE || opndKind == VIR_OPND_UNDEF)
    {
        return 0x0;
    }

    /* Inst type of img_load is regareded as result type which is converted from src img fmt.
       So far, we dont support arbitary conversion, so just get corresponding inst type from
       image fmt */
    if (opcode == VIR_OP_IMG_LOAD || opcode == VIR_OP_IMG_LOAD_3D)
    {
        gcmASSERT(VIR_Symbol_GetKind(sym) == VIR_SYM_IMAGE ||
                  VIR_Symbol_GetKind(sym) == VIR_SYM_IMAGE_T ||         /* IMAGE_T will be invalid here */
                  isSymUniformTreatSamplerAsConst(sym));

        imgQual = VIR_Symbol_GetImageFormat(sym);

        /* OCL image type has no layout info */
        if (imgQual != VIR_IMAGE_FORMAT_NONE)
        {
            return _VSC_MC_GEN_GetInstTypeFromImgFmt(imgQual);
        }
        else
        {
            /* need to get the image type from dest for load */
            Opnd       = VIR_Inst_GetDest(Inst);
            opndKind   = VIR_Operand_GetOpKind(Opnd);
            ty         = VIR_Operand_GetTypeId(Opnd);
        }
    }

    if (VIR_OPCODE_isTexLd(opcode))
    {
        /* Only v60 HW supports inst-type for texld related insts */
        if (!Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti5)
        {
            return 0x0;
        }
    }

    gcmASSERT(ty < VIR_TYPE_PRIMITIVETYPE_COUNT);

    if (VIR_TypeId_isSampler(ty))
    {
        componentTy = VIR_TYPE_UINT32;
    }
    else
    {
        componentTy  = VIR_GetTypeComponentType(ty);
    }

    /* we change the integer/boolen from highp to mediump for dual-t*/
    if (VIR_Shader_isDual16Mode(Gen->Shader) &&
        (VIR_Inst_GetThreadMode(Inst) == VIR_THREAD_D16_DUAL_16 ||
         VIR_Inst_GetThreadMode(Inst) == VIR_THREAD_D16_DUAL_HIGHPVEC2))
    {
        if (componentTy == VIR_TYPE_INT32)
        {
            componentTy = VIR_TYPE_INT16;
        }
        else if (componentTy == VIR_TYPE_UINT32)
        {
            componentTy = VIR_TYPE_UINT16;
        }
        else if (componentTy == VIR_TYPE_BOOLEAN)
        {
            componentTy = VIR_TYPE_INT16;
        }
    }

    switch (componentTy)
    {
    case VIR_TYPE_FLOAT32:
        return 0x0;
    case VIR_TYPE_FLOAT16:
        return 0x1;
    case VIR_TYPE_INT32:
    case VIR_TYPE_BOOLEAN:
        return 0x2;
    case VIR_TYPE_INT16:
        return 0x3;
    case VIR_TYPE_INT8:
        return 0x4;
    case VIR_TYPE_UINT32:
        return 0x5;
    case VIR_TYPE_UINT16:
        return 0x6;
    case VIR_TYPE_UINT8:
        return 0x7;
    case VIR_TYPE_INT64:
        return 0xA;
    case VIR_TYPE_UINT64:
        return 0xD;
    default:
        gcmASSERT(0);
        return 0x0;
    }
}

static gctBOOL
_IsDestTypeFP16(
    IN VIR_Instruction *Inst
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId typeId;
    gcmASSERT(dest);

    typeId = VIR_Operand_GetTypeId(dest);
    return VIR_Shader_GetBuiltInTypes(typeId)->componentType == VIR_TYPE_FLOAT16;
}

static gctUINT
_VSC_MC_GEN_GenInstType(
    IN VSC_MCGen       *Gen,
    IN VIR_Instruction *Inst
    )
{
    VIR_OpCode opcode = VIR_Inst_GetOpcode(Inst);

    switch(opcode)
    {
    case VIR_OP_LOOP:
    case VIR_OP_KILL:
    case VIR_OP_FLUSH:
    case VIR_OP_JMPC:
    case VIR_OP_JMP_ANY:
    /*case VIR_OP_MOVA:, let mova be determined by dst because old gcsl has no type info for indexing reg */
    case VIR_OP_ABS:
    case VIR_OP_NEG:
    case VIR_OP_CMP:
    case VIR_OP_SET:
    case VIR_OP_I2I:
    case VIR_OP_I2F:
    case VIR_OP_RCP:
    case VIR_OP_IMG_LOAD:
    case VIR_OP_IMG_LOAD_3D:
    case VIR_OP_BITFIND_LSB:
    case VIR_OP_BITFIND_MSB:
    case VIR_OP_MOV:
        gcmASSERT(VIR_OPCODE_useSrc0AsInstType(opcode));
        return _VSC_MC_GEN_GetInstType(Gen, Inst, VIR_Inst_GetSource(Inst, 0));
    case VIR_OP_CMOV:
    case VIR_OP_SELECT:
        if (VIR_Inst_GetConditionOp(Inst) == VIR_COP_ALLMSB ||
            VIR_Inst_GetConditionOp(Inst) == VIR_COP_ANYMSB ||
            VIR_Inst_GetConditionOp(Inst) == VIR_COP_SELMSB)
        {
            /* HW limitation: one instruction type to do two things:
               to control comparison and to control implicit conversion (e.g., f32 -> f16).
               We have issue when src0 is integer type, but src1 is float type for dual16.
               Using src0 integer type will lose the implict conversion for source/destination. */
            if (VIR_Inst_GetFlags(Inst) & VIR_INSTFLAG_PACKEDMODE)
            {
                return _VSC_MC_GEN_GetInstType(Gen, Inst, VIR_Inst_GetSource(Inst, 0));
            }
            else
            {
                return _VSC_MC_GEN_GetInstType(Gen, Inst, VIR_Inst_GetSource(Inst, 1));
            }
        }
        else
        {
            VIR_TypeId src0TypeId = VIR_GetTypeComponentType(VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0)));
            VIR_TypeId src1TypeId = VIR_GetTypeComponentType(VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 1)));

            if ((VIR_TypeId_isFloat(src0TypeId) && VIR_TypeId_isFloat(src1TypeId))
                ||
                (VIR_TypeId_isInteger(src0TypeId) && VIR_TypeId_isInteger(src1TypeId)))
            {
                /* Try to use the dataType of src0 first. */
                if (VIR_GetTypeSize(src0TypeId) >= VIR_GetTypeSize(src1TypeId))
                {
                    return _VSC_MC_GEN_GetInstType(Gen, Inst, VIR_Inst_GetSource(Inst, 0));
                }
                else
                {
                    return _VSC_MC_GEN_GetInstType(Gen, Inst, VIR_Inst_GetSource(Inst, 1));
                }
            }
            else
            {
                return _VSC_MC_GEN_GetInstType(Gen, Inst, VIR_Inst_GetSource(Inst, 0));
            }
        }
    case VIR_OP_STORE:
    case VIR_OP_STORE_S:
    case VIR_OP_STORE_L:
        gcmASSERT(VIR_OPCODE_useSrc2AsInstType(opcode));
        if(_IsDestTypeFP16(Inst))
        {
            return _VSC_MC_GEN_GetInstType(Gen, Inst, VIR_Inst_GetDest(Inst));
        }
        else
        {
            return _VSC_MC_GEN_GetInstType(Gen, Inst, VIR_Inst_GetSource(Inst, 2));
        }

    case VIR_OP_STORE_ATTR:
    case VIR_OP_IMG_STORE:
    case VIR_OP_VX_IMG_STORE:
    case VIR_OP_IMG_STORE_3D:
    case VIR_OP_VX_IMG_STORE_3D:
        gcmASSERT(VIR_OPCODE_useSrc2AsInstType(opcode));
        return _VSC_MC_GEN_GetInstType(Gen, Inst, VIR_Inst_GetSource(Inst, 2));

    case VIR_OP_VX_SCATTER:
    case VIR_OP_VX_ATOMIC_S:
        gcmASSERT(VIR_OPCODE_useSrc2AsInstType(opcode));
        return _VSC_MC_GEN_GetInstType(Gen, Inst, VIR_Inst_GetSource(Inst, 2));
    case VIR_OP_VX_SCATTER_B:
    case VIR_OP_VX_ATOMIC_S_B:
        gcmASSERT(VIR_OPCODE_useSrc3AsInstType(opcode));
        return _VSC_MC_GEN_GetInstType(Gen, Inst, VIR_Inst_GetSource(Inst, 3));

        /* No dest. */
    case VIR_OP_NOP:
    case VIR_OP_CALL:
    case VIR_OP_JMP:
    case VIR_OP_BARRIER:
    case VIR_OP_MEM_BARRIER:
    case VIR_OP_FENCE:
    case VIR_OP_RET:
    case VIR_OP_EXIT:
    case VIR_OP_THREADEXIT:
    case VIR_OP_ENDLOOP:
    case VIR_OP_REP:
    case VIR_OP_ENDREP:
    case VIR_OP_PREFETCH:
        return 0x0;
    default:
        return _VSC_MC_GEN_GetInstType(Gen, Inst, VIR_Inst_GetDest(Inst));
    }
}

static gctUINT
_VSC_MC_GEN_GenThreadType(
    IN  VSC_MCGen      *Gen,
    IN VIR_Instruction *Inst
    )
{
    VIR_ThreadMode threadMode = VIR_Inst_GetThreadMode(Inst);

    switch(threadMode)
    {
    case VIR_THREAD_D16_DUAL_16:
    case VIR_THREAD_D16_DUAL_HIGHPVEC2:
        return 0x0;
    case VIR_THREAD_D16_SINGLE_T0:
        return 0x1;
    case VIR_THREAD_D16_SINGLE_T1:
        return 0x2;
    case VIR_THREAD_D16_DUAL_32:  /* DUAL32 must be expanded to SINGLE_T0 & SINGLE_T1 in dual16 mode */
        if (!VIR_Shader_isDual16Mode(Gen->Shader))
        {
            return 0x0;
        }
        /* else fall through */
    default:
        gcmASSERT(0);
        return 0;
    }
}

static gctUINT
_VSC_MC_GEN_GenSkipHelperFlag(
    IN  VSC_MCGen      *Gen,
    IN VIR_Instruction *Inst
    )
{
    VIR_OpCode  opcode      = VIR_Inst_GetOpcode(Inst);

    if (VIR_OPCODE_isAtom(opcode)      ||
        /*
        ** LOAD instruction may be used for coordinate computation,
        ** which should not be skipped for helper.
        */
        (opcode == VIR_OP_IMG_LOAD)       ||
        (opcode == VIR_OP_VX_IMG_LOAD)    ||
        (opcode == VIR_OP_IMG_LOAD_3D)    ||
        (opcode == VIR_OP_VX_IMG_LOAD_3D) ||
        (opcode == VIR_OP_IMG_ADDR)       ||
        (opcode == VIR_OP_IMG_ADDR_3D)    ||
        /* store_s should not skip helper */
        VIR_OPCODE_isLocalMemSt(opcode)   ||
        VIR_OPCODE_isGlobalMemSt(opcode)  ||
        (opcode == VIR_OP_IMG_STORE)      ||
        (opcode == VIR_OP_VX_IMG_STORE)   ||
        (opcode == VIR_OP_IMG_STORE_3D)   ||
        (opcode == VIR_OP_VX_IMG_STORE_3D))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_VSC_MC_GEN_GenDenormFlag(
    IN  VSC_MCGen      *Gen,
    IN VIR_Instruction *Inst
    )
{
    if (!(Gen->pComCfg->cFlags & VSC_COMPILER_FLAG_FLUSH_DENORM_TO_ZERO))
    {
        if (VIR_OPCODE_isMemLd(VIR_Inst_GetOpcode(Inst))    ||
            VIR_OPCODE_isMemSt(VIR_Inst_GetOpcode(Inst))    ||
            VIR_OPCODE_isImgLd(VIR_Inst_GetOpcode(Inst))    ||
            VIR_OPCODE_isImgSt(VIR_Inst_GetOpcode(Inst))    ||
            VIR_OPCODE_isAttrLd(VIR_Inst_GetOpcode(Inst))   ||
            VIR_Inst_GetOpcode(Inst) == VIR_OP_CMP ||
            VIR_Inst_GetOpcode(Inst) == VIR_OP_SELECT)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static gctBOOL
_VSC_MC_GEN_GenResultSat(
    IN  VSC_MCGen      *Gen,
    IN VIR_Instruction *Inst
    )
{
    VIR_Operand*    dstOpnd = VIR_Inst_GetDest(Inst);
    VIR_Operand*    opnd;
    VIR_OpCode      opcode = VIR_Inst_GetOpcode(Inst);
    VIR_TypeId      ty;
    VIR_TypeId      componentTy;

    /* Currently, we have different pathes to get saturate, however, IR should make
       sure to put it into same location, so we can uniformly get it */

    if (dstOpnd)
    {
        if (VIR_Operand_GetOpKind(dstOpnd) == VIR_OPND_SYMBOL)
        {
            if (VIR_Operand_GetModifier(dstOpnd) == VIR_MOD_SAT_0_TO_1)
            {
                return gcvTRUE;
            }

            if (VIR_OPCODE_isVX(opcode))
            {
                VIR_Operand * opnd = VIR_Inst_GetEvisModiferOpnd(Inst);

                if (opnd)
                {
                    VIR_EVIS_Modifier evisModifier;
                    evisModifier.u1 = VIR_Operand_GetEvisModifier(opnd);

                    if (evisModifier.u0.clamp)
                    {
                        return gcvTRUE;
                    }
                }
            }
        }
    }

    if (opcode == VIR_OP_IMG_STORE || opcode == VIR_OP_IMG_STORE_3D ||
        opcode == VIR_OP_VX_IMG_STORE || opcode == VIR_OP_VX_IMG_STORE_3D)
    {
        opnd = VIR_Inst_GetSource(Inst, 2);
        ty  = VIR_Operand_GetTypeId(opnd);

        gcmASSERT(ty < VIR_TYPE_PRIMITIVETYPE_COUNT);
        componentTy  = VIR_GetTypeComponentType(ty);

        if (componentTy != VIR_TYPE_FLOAT32 && componentTy != VIR_TYPE_FLOAT16)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static gctUINT _VSC_MC_GEN_GetLsAttrClient(VIR_ShaderKind shKind)
{
    gctUINT client = 0x0;

    if (shKind == VIR_SHADER_VERTEX)
    {
        client = 0x0;
    }
    else if (shKind == VIR_SHADER_TESSELLATION_CONTROL)
    {
        client = 0x1;
    }
    else if (shKind == VIR_SHADER_TESSELLATION_EVALUATION)
    {
        client = 0x2;
    }
    else if (shKind == VIR_SHADER_GEOMETRY)
    {
        client = 0x3;
    }

    return client;
}

static void
_VSC_MC_GEN_GenInstCtrl(
    IN  VSC_MCGen              *Gen,
    IN  VIR_Instruction        *Inst,
    OUT VSC_MC_CODEC_INST_CTRL *McInstCtrl
    )
{
    VIR_OpCode opCode      = VIR_Inst_GetOpcode(Inst);
    gctBOOL    bVisionInst = VIR_OPCODE_isVX(opCode) &&
                            !(opCode == VIR_OP_VX_IMG_LOAD    ||
                              opCode == VIR_OP_VX_IMG_STORE   ||
                              opCode == VIR_OP_VX_IMG_LOAD_3D ||
                              opCode == VIR_OP_VX_IMG_STORE_3D);

    gcmASSERT(McInstCtrl != gcvNULL);

    McInstCtrl->condOpCode                      = _VSC_MC_GEN_GenCondition(Inst);
    McInstCtrl->roundingMode                    = _VSC_MC_GEN_GenRound(Gen, Inst);
    McInstCtrl->threadType                      = _VSC_MC_GEN_GenThreadType(Gen, Inst);
    McInstCtrl->instType                        = _VSC_MC_GEN_GenInstType(Gen, Inst);
    McInstCtrl->bSkipForHelperKickoff           = (Gen->Shader->shaderKind == VIR_SHADER_FRAGMENT) ? _VSC_MC_GEN_GenSkipHelperFlag(Gen, Inst) : gcvFALSE;
    McInstCtrl->bDenorm                         = _VSC_MC_GEN_GenDenormFlag(Gen, Inst);
    McInstCtrl->bResultSat                      = _VSC_MC_GEN_GenResultSat(Gen, Inst);
    McInstCtrl->packMode                        = VIR_Inst_GetFlags(Inst) & VIR_INSTFLAG_PACKEDMODE ?
                                                      0x0 : 0x1;
    McInstCtrl->bForceGen                       = (VIR_Inst_GetFlags(Inst) & VIR_INSTFLAG_FORCE_GEN) ? gcvTRUE : gcvFALSE;

    /* For a union variable, set value only if the opcode match the requirement. */
    if (opCode == VIR_OP_MUL_Z || opCode == VIR_OP_NORM_MUL)
    {
        McInstCtrl->u.bInfX0ToZero              = gcvTRUE;
    }

    if ((VIR_OPCODE_isLocalMemLd(opCode) ||
         VIR_OPCODE_isLocalMemSt(opCode) ||
         VIR_OPCODE_isLocalAtom(opCode)) ||
        ((VIR_OPCODE_isMemLd(opCode) || VIR_OPCODE_isMemSt(opCode) || VIR_OPCODE_isAttrLd(opCode)) &&
          gcmOPT_hasFeature(FB_FORCE_LS_ACCESS) /* triage option */    )        )
    {
        McInstCtrl->u.maCtrl.bAccessLocalStorage= gcvTRUE;
    }

    if ((VIR_OPCODE_isGlobalMemLd(opCode) ||
         VIR_OPCODE_isGlobalMemSt(opCode) ||
         VIR_OPCODE_isAtom(opCode)    ) &&
        (VIR_Operand_isBigEndian(VIR_Inst_GetSource(Inst, 0))) &&
        Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.supportBigEndianLdSt)
    {
        McInstCtrl->u.maCtrl.bBigEndian= gcvTRUE;
    }

    if (VIR_Inst_IsUSCUnallocate(Inst))
    {
        gcmASSERT(VIR_OPCODE_isMemLd(opCode) ||
                   VIR_OPCODE_isMemSt(opCode) ||
                   VIR_OPCODE_isImgLd(opCode) ||
                   VIR_OPCODE_isImgSt(opCode) );
        McInstCtrl->u.maCtrl.bUnallocate= gcvTRUE;
    }

    if (opCode == VIR_OP_VX_IMG_LOAD    ||
        opCode == VIR_OP_VX_IMG_STORE   ||
        opCode == VIR_OP_VX_IMG_LOAD_3D ||
        opCode == VIR_OP_VX_IMG_STORE_3D)
    {
        McInstCtrl->u.maCtrl.bUnderEvisMode     = gcvTRUE;
    }

    if (VIR_Inst_IsEndOfBB(Inst))
    {
        McInstCtrl->bEndOfBB     = gcvTRUE;
    }

    if (opCode == VIR_OP_LOAD_ATTR ||
        opCode == VIR_OP_LOAD_ATTR_O ||
        opCode == VIR_OP_STORE_ATTR)
    {
        McInstCtrl->u.lsAttrCtrl.shStageClient = (opCode == VIR_OP_LOAD_ATTR) ?
                                                 _VSC_MC_GEN_GetLsAttrClient(Gen->Shader->inLinkedShaderStage) :
                                                 _VSC_MC_GEN_GetLsAttrClient(Gen->Shader->shaderKind);

        if ((Gen->Shader->shaderKind == VIR_SHADER_GEOMETRY && opCode == VIR_OP_STORE_ATTR) ||
            (Gen->Shader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL && opCode == VIR_OP_STORE_ATTR) ||
            (opCode == VIR_OP_LOAD_ATTR_O) ||
            (Gen->Shader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION && opCode == VIR_OP_LOAD_ATTR))
        {
            McInstCtrl->u.lsAttrCtrl.attrLayout = 0x1;
        }
        else
        {
            McInstCtrl->u.lsAttrCtrl.attrLayout = 0x0;
        }
    }

    if (VIR_OPCODE_isMemLd(opCode) || VIR_OPCODE_isMemSt(opCode))
    {
        VIR_Operand* pOffsetOpnd = VIR_Inst_GetSource(Inst, 1);

        McInstCtrl->u.maCtrl.u.lsCtrl.bOffsetX3 = ((VIR_Operand_GetModifier(pOffsetOpnd) & VIR_MOD_X3) != 0);
        McInstCtrl->u.maCtrl.u.lsCtrl.offsetLeftShift = VIR_Operand_GetLShift(pOffsetOpnd);
    }

    if (opCode == VIR_OP_CONV)
    {
        McInstCtrl->u.convCtrl.bEvisMode = gcvFALSE;
        McInstCtrl->u.convCtrl.bDstPack = VIR_TypeId_isPacked(VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst)));
        McInstCtrl->u.convCtrl.bSrcPack = VIR_TypeId_isPacked(VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0)));
    }

    if (bVisionInst)
    {
        VIR_EVIS_Modifier evisModifier;
        gctUINT i;

        /* find EvisModifier operand  */
        for (i = 0; i < VIR_Inst_GetSrcNum(Inst); i ++)
        {
            VIR_Operand * opnd = VIR_Inst_GetSource(Inst, i);
            if (opnd && VIR_Operand_GetOpKind(opnd) == VIR_OPND_EVIS_MODIFIER)
            {
                evisModifier.u1 = VIR_Operand_GetEvisModifier(opnd);

                /* setting startBin etc. */
                McInstCtrl->u.visionCtrl.evisState       = VIR_Inst_GetEvisState(Inst, opnd);
                McInstCtrl->u.visionCtrl.startSrcCompIdx = evisModifier.u0.sourceBin;
                break;
            }
        }
    }
}

static gctUINT
_VSC_MC_GEN_GenOpndSwizzle(
    IN VSC_MCGen        *Gen,
    IN VIR_Instruction  *Inst,
    IN VIR_Operand      *Opnd
    )
{
    VIR_Symbol *sym         = VIR_Operand_GetSymbol(Opnd);
    gctUINT     swizzle     = VIR_Operand_GetSwizzle(Opnd);
    gctUINT     retSwizzle  = 0, uniformSwizzle = NOT_ASSIGNED;
    gctUINT8    shift       = (gctUINT8) VIR_Operand_GetHwShift(Opnd);
    gctUINT     srcIndex    = VIR_Inst_GetSourceIndex(Inst, Opnd);

    gcmASSERT(srcIndex < VIR_MAX_SRC_NUM);

    switch(VIR_Symbol_GetKind(sym))
    {
    case VIR_SYM_IMAGE:
        {
            VIR_Uniform *image = VIR_Symbol_GetImage(sym);
            uniformSwizzle = image->swizzle;
            break;
        }
    case VIR_SYM_IMAGE_T:               /* IMAGE_T will be invalid here */
        {
            VIR_Uniform *imageT = VIR_Symbol_GetImageT(sym);
            uniformSwizzle = imageT->swizzle;
            break;
        }
    case VIR_SYM_UNIFORM:
        {
            VIR_Uniform *uniform = VIR_Symbol_GetUniform(sym);
            uniformSwizzle = uniform->swizzle;
            break;
        }
    /*case VIR_SYM_SAMPLER:
        {
            VIR_Uniform *sampler = VIR_Symbol_GetSampler(sym);
            uniformSwizzle = sampler->swizzle;
            break;
        }*/
        default:
            break;
    }

    /* compnent shift */
    if (uniformSwizzle != NOT_ASSIGNED)
    {
        gcmASSERT(shift == 0);

        retSwizzle   = ((uniformSwizzle >> (((swizzle >> 6) & 0x3) * 2)) & 0x3) << 6;
        retSwizzle  |= ((uniformSwizzle >> (((swizzle >> 4) & 0x3) * 2)) & 0x3) << 4;
        retSwizzle  |= ((uniformSwizzle >> (((swizzle >> 2) & 0x3) * 2)) & 0x3) << 2;
        retSwizzle  |= ((uniformSwizzle >> (((swizzle >> 0) & 0x3) * 2)) & 0x3) << 0;
    }
    else
    {
        retSwizzle  = ((((swizzle >> 6) & 0x3) + shift) & 0x3) << 6;
        retSwizzle |= ((((swizzle >> 4) & 0x3) + shift) & 0x3) << 4;
        retSwizzle |= ((((swizzle >> 2) & 0x3) + shift) & 0x3) << 2;
        retSwizzle |= ((((swizzle >> 0) & 0x3) + shift) & 0x3) << 0;
    }

    if (VIR_Inst_isComponentwise(Inst) ||
        VIR_OPCODE_isSourceComponentwise(VIR_Inst_GetOpcode(Inst), srcIndex))
    {
        gctUINT8 enableShift = 0;

        if (Inst->dest)
        {
            enableShift = (gctUINT8) VIR_Operand_GetHwShift(Inst->dest);
        }

        while (enableShift-- >0)
        {
            retSwizzle = (retSwizzle << 2 | (retSwizzle & 0x3));
        }

        /* Make swizzle be kept in LSB8, and not dirty other bits because enable is shifted */
        retSwizzle &= 0xFF;
    }

    return retSwizzle;
}

static gctUINT
_VSC_MC_GEN_GenOpndEnable(
    IN VSC_MCGen       *Gen,
    IN VIR_Instruction *Inst,
    IN VIR_Operand     *Opnd
    )
{
    gctUINT enable = VIR_ENABLE_NONE;

    gcmASSERT(VIR_Operand_isLvalue(Opnd));

    if (VIR_OPCODE_mustFullDestEnable(VIR_Inst_GetOpcode(Inst)))
    {
        enable = VIR_ENABLE_XYZW;
    }
    else
    {
        enable = VIR_Operand_GetEnable(Opnd);

        if ((VIR_Operand_GetOpKind(Opnd) == VIR_OPND_SYMBOL) ||
            (VIR_Operand_GetOpKind(Opnd) == VIR_OPND_VIRREG))
        {
            if (VIR_Operand_isRegAllocated(Opnd))
            {
                enable = enable << VIR_Operand_GetHwShift(Opnd);
            }
            else
            {
                /* store instruction could have no dest (only the enable used), thus
                   its def could be not register allocated */
                gcmASSERT(VIR_OPCODE_hasStoreOperation(VIR_Inst_GetOpcode(Inst)));
            }
        }
    }
    return enable;
}

static gctUINT
_VSC_MC_GEN_GenRegisterNo(
    IN VSC_MCGen       *Gen,
    IN VIR_Instruction *Inst,
    IN VIR_Symbol      *Sym,
    IN VIR_Operand     *Opnd
    )
{
    switch(VIR_Symbol_GetKind(Sym))
    {
    case VIR_SYM_IMAGE:
        {
            VIR_Uniform *image = VIR_Symbol_GetImage(Sym);
            return image->physical;
        }
    case VIR_SYM_IMAGE_T:       /* will be removed after write_image is implemented with source library */
        {
            VIR_Uniform *imageT = VIR_Symbol_GetImageT(Sym);
            return imageT->physical;
        }
    case VIR_SYM_UNIFORM:
        {
            VIR_Uniform *uniform = VIR_Symbol_GetUniform(Sym);
            return uniform->physical;
        }
    case VIR_SYM_SAMPLER:
        {
            VIR_Uniform *sampler = VIR_Symbol_GetSampler(Sym);
            return sampler->physical;
        }
    case VIR_SYM_SAMPLER_T:
        {
            VIR_Uniform *sampler_t = VIR_Symbol_GetSamplerT(Sym);
            return sampler_t->physical;
        }
    case VIR_SYM_TEXTURE:
    case VIR_SYM_VARIABLE:
    case VIR_SYM_VIRREG:
        {
            gctUINT regNo;
            if(Opnd != gcvNULL)
            {
                regNo = VIR_Operand_GetHwRegId(Opnd);
            }
            else
            {
                regNo = VIR_Symbol_GetHwRegId(Sym);
            }

            if (regNo == VIR_SR_INSTATNCEID)
            {
                gcmASSERT(!Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.vtxInstanceIdAsAttr);

                regNo = 0x00;
            }
            else if (regNo == VIR_SR_VERTEXID)
            {
                regNo = 0x0F;
            }
            else if (regNo == VIR_REG_MULTISAMPLEDEPTH)
            {
                /* It indicates last register number */
                if (VIR_Inst_GetThreadMode(Inst) == VIR_THREAD_D16_SINGLE_T1 ||
                    VIR_Inst_GetThreadMode(Inst) == VIR_THREAD_D16_DUAL_32 ||
                    VIR_Inst_GetThreadMode(Inst) == VIR_THREAD_SINGLE32)
                {
                    regNo = VIR_Shader_GetRegWatermark(Gen->Shader) - 1;
                }
                else if (VIR_Inst_GetThreadMode(Inst) == VIR_THREAD_D16_SINGLE_T0)
                {
                    regNo = VIR_Shader_GetRegWatermark(Gen->Shader) - 2;
                }
                else
                {
                    /* Can not be VIR_THREAD_D16_DUAL_16 as msaadepth must be on HP */
                    gcmASSERT(gcvFALSE);
                }
            }
            else if (regNo == VIR_REG_SAMPLE_MASK_IN)
            {
                regNo = 0x12;
            }
            else if (regNo == VIR_REG_SAMPLE_ID)
            {
                regNo = 0x10;
            }
            else if (regNo == VIR_REG_SAMPLE_POS)
            {
                regNo = 0x11;
            }
            else if (regNo == VIR_SR_A0 ||
                     regNo == VIR_SR_B0)
            {
                regNo = 0;
            }

            return regNo;
        }
    case VIR_SYM_CONST:
    case VIR_SYM_UBO:
    default:
        gcmASSERT(0);
        break;
    }
    return 0xffff;
}

static gctINT
_VSC_MC_GEN_GenOpndRelIndexing(
    IN VSC_MCGen        *Gen,
    IN VIR_Instruction  *Inst,
    IN VIR_Operand      *Opnd
    )
{
    if (VIR_Operand_GetRelAddrMode(Opnd) == VIR_INDEXED_NONE)
    {
        return VIR_Operand_GetRelIndexing(Opnd);
    }

    return 0;
}

static gctUINT16
_VSC_MC_GEN_GenImmTypeAndValue(
    IN VSC_MCGen                *Gen,
    IN VSC_MC_CODEC_INST_CTRL   *McInstCtrl,
    IN VIR_Instruction          *Inst,
    IN VIR_Operand              *Opnd,
    OUT VSC_MC_CODEC_IMM_VALUE  *ImmVal
    )
{
    VIR_TypeId ty           = VIR_Operand_GetTypeId(Opnd);
    VIR_TypeId componentTy;

    if (VIR_OPCODE_isComponentwise(VIR_Inst_GetOpcode(Inst)) &&
        VIR_TypeId_isPacked(VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst))) &&
        !VIR_TypeId_isPacked(ty))
    {
        /* change the immediate number type to packed type for instruction like:
         *   CMP.ge   short8P temp(42), short8P temp(36), short8P 0, int -1
         */
        /* use the src0 type if it is packed type otherwise use dest type, it may need
         * some special care for opCode which get different type than the source operands */
        if (VIR_TypeId_isPacked(VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0))))
        {
            ty = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
        }
        else
        {
            ty = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
        }
    }
    componentTy  = VIR_GetTypeComponentType(ty);
    if (VIR_Shader_isDual16Mode(Gen->Shader))
    {
        gcmASSERT(VIR_Opnd_ValueFit16Bits(Opnd));

        /* float is put into uniform already */
        gcmASSERT(componentTy != VIR_TYPE_FLOAT32);

        if (ImmVal)
        {
            ImmVal->ui = VIR_Operand_GetImmediateUint(Opnd);
        }
        if ((VIR_Inst_GetOpcode(Inst) == VIR_OP_JMP ||
             VIR_Inst_GetOpcode(Inst) == VIR_OP_JMPC ||
             VIR_Inst_GetOpcode(Inst) == VIR_OP_JMP_ANY) &&
             VIR_Inst_GetSourceIndex(Inst, Opnd) == 2)
        {
            return 0x2;
        }

        if ((VIR_Inst_GetThreadMode(Inst) == VIR_THREAD_D16_DUAL_16 || VIR_Inst_GetThreadMode(Inst) == VIR_THREAD_D16_DUAL_HIGHPVEC2) &&
           ((VIR_Inst_GetOpcode(Inst) != VIR_OP_JMP &&
             VIR_Inst_GetOpcode(Inst) != VIR_OP_JMPC &&
             VIR_Inst_GetOpcode(Inst) != VIR_OP_JMP_ANY) ||
             VIR_Inst_GetSourceIndex(Inst, Opnd) != 2))
        {
            return 0x3;
        }

        /* other cases fall through */
    }

    if (VIR_TypeId_isPacked(ty))
    {
        switch (ty)
        {
        case VIR_TYPE_INT8_P2:
        case VIR_TYPE_UINT8_P2:
        case VIR_TYPE_BOOLEAN_P2:
            if (ImmVal)
            {
                ImmVal->ui = (VIR_Operand_GetImmediateUint(Opnd) & 0xFF) |
                               ((VIR_Operand_GetImmediateUint(Opnd) & 0xFF) << 8);
            }
            return 0x2;
        case VIR_TYPE_INT8_P3:
        case VIR_TYPE_INT8_P4:
        case VIR_TYPE_INT8_P8:
        case VIR_TYPE_INT8_P16:
        case VIR_TYPE_UINT8_P3:
        case VIR_TYPE_UINT8_P4:
        case VIR_TYPE_UINT8_P8:
        case VIR_TYPE_UINT8_P16:
        case VIR_TYPE_BOOLEAN_P3:
        case VIR_TYPE_BOOLEAN_P4:
        case VIR_TYPE_BOOLEAN_P8:
        case VIR_TYPE_BOOLEAN_P16:
            if (ImmVal)
            {
                ImmVal->ui = (VIR_Operand_GetImmediateUint(Opnd) & 0xFF) |
                               ((VIR_Operand_GetImmediateUint(Opnd) & 0xFF) << 8);
                ImmVal->ui |= ((ImmVal->ui & 0xFFFF) << 16);
            }
            return 0x3;
        case VIR_TYPE_INT16_P2:
        case VIR_TYPE_UINT16_P2:
        case VIR_TYPE_INT16_P3:
        case VIR_TYPE_INT16_P4:
        case VIR_TYPE_INT16_P8:
        case VIR_TYPE_UINT16_P3:
        case VIR_TYPE_UINT16_P4:
        case VIR_TYPE_UINT16_P8:
            if (ImmVal)
            {
                ImmVal->ui = (VIR_Operand_GetImmediateUint(Opnd) & 0xFFFF);
            }
            return 0x3;
        case VIR_TYPE_FLOAT16_P2:
        case VIR_TYPE_FLOAT16_P3:
        case VIR_TYPE_FLOAT16_P4:
        case VIR_TYPE_FLOAT16_P8:
            if (ImmVal)
            {
                ImmVal->ui = VIR_ConvertF32ToFP16(VIR_Operand_GetImmediateFloat(Opnd));
            }
            return 0x3;
        default:
            gcmASSERT(gcvFALSE);
            if (ImmVal)
            {
                ImmVal->ui = VIR_Operand_GetImmediateUint(Opnd);
            }
            return 0x2;
        }
    }
    else
    {
        if (ImmVal)
        {
            ImmVal->ui = VIR_Operand_GetImmediateUint(Opnd);
        }
        switch (componentTy)
        {
        case VIR_TYPE_FLOAT32:
            return 0x0;
        case VIR_TYPE_FLOAT16:
            if (McInstCtrl->instType == 0x1)
            {
                return 0x2;
            }
            else
            {
                return 0x0;
            }

        case VIR_TYPE_INT64:
        case VIR_TYPE_INT32:
        case VIR_TYPE_INT16:
        case VIR_TYPE_INT8:
        case VIR_TYPE_BOOLEAN:
            return 0x1;
        case VIR_TYPE_UINT64:
        case VIR_TYPE_UINT32:
        case VIR_TYPE_UINT16:
        case VIR_TYPE_UINT8:
            return 0x2;
        default:
            gcmASSERT(0);
            return 0x0;
        }
    }
}

static gctUINT
_VSC_MC_GEN_GenSrcType(
    IN VSC_MCGen  *Gen,
    IN VIR_Symbol *Symbol
    )
{
    VIR_SymbolKind kind      = VIR_Symbol_GetKind(Symbol);
    VIR_Precision  precision = VIR_Symbol_GetPrecision(Symbol);

    switch(kind)
    {
    case VIR_SYM_UNIFORM:
        return 0x2;
    case VIR_SYM_SAMPLER:
        if (isSymUniformTreatSamplerAsConst(Symbol))
        {
            return 0x2;
        }
        else
        {
            return MC_AUXILIARY_SRC_TYPE_SAMPLER;
        }
    case VIR_SYM_SAMPLER_T:
        return 0x2;
    case VIR_SYM_IMAGE:
        return 0x2;
    case VIR_SYM_IMAGE_T:       /* will be removed after write_image is implemented with source library */
        return 0x2;
    case VIR_SYM_VIRREG:
    case VIR_SYM_VARIABLE:
        {
            VIR_NameId  VirNameId  = 0;
            VIR_Symbol *symbol     = Symbol;
            gctBOOL     isVariable = gcvFALSE;

            if (VIR_Symbol_isVreg(symbol))
            {
                symbol = VIR_Symbol_GetVregVariable(symbol);
            }

            if (symbol &&
                VIR_Symbol_isVariable(symbol))
            {
                VirNameId = VIR_Symbol_GetName(symbol);
                isVariable = gcvTRUE;
            }

            if (isVariable &&
                VIR_Symbol_GetStorageClass(symbol) == VIR_STORAGE_INPUT)
            {
                if (VirNameId == VIR_NAME_FRONT_FACING) {
                    return 0x1;
                } else if (VirNameId == VIR_NAME_SAMPLE_ID ||
                    VirNameId == VIR_NAME_SAMPLE_POSITION ||
                    VirNameId == VIR_NAME_SAMPLE_MASK_IN) {
                        return 0x5;
                } else if (!Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.vtxInstanceIdAsAttr) {
                    if (gcvTRUE)
                    {
                        if (VirNameId == VIR_NAME_VERTEX_ID) {
                            return 0x4;
                        } else if (VirNameId == VIR_NAME_INSTANCE_ID) {
                            return 0x5;
                        }
                    }
                    else
                    {
                        return 0x5;
                    }
                }
            }

            switch (VIR_Symbol_GetAddrSpace(Symbol))
            {
            case VIR_AS_PRIVATE:
            case VIR_AS_GLOBAL:
                if(VIR_Shader_isDual16Mode(Gen->Shader))
                {
                    if(precision == VIR_PRECISION_DEFAULT) {
                        return 0x0;
                    } else if(precision == VIR_PRECISION_HIGH) {
                        return 0x4;
                    } else if(precision == VIR_PRECISION_MEDIUM || precision == VIR_PRECISION_LOW) {
                        return 0x0;
                    } else {
                        gcmASSERT(0);
                        return 0x0;
                    }
                }
                else
                {
                    return 0x0;
                }
            case VIR_AS_LOCAL:
                return 0x6;
            default:
                break;
            }
        }

        gcmASSERT(0);
        return 0x0;

    default:
        gcmASSERT(0);
        return 0x0;
    }
}

static gctUINT
_VSC_MC_GEN_GenDstType(
    IN VSC_MCGen  *Gen,
    IN VIR_Symbol *Symbol
    )
{
    VIR_SymbolKind kind = VIR_Symbol_GetKind(Symbol);
    VIR_Precision  precision = VIR_Symbol_GetPrecision(Symbol);

    if(kind == VIR_SYM_VIRREG || kind == VIR_SYM_VARIABLE)
    {
        switch (VIR_Symbol_GetAddrSpace(Symbol))
        {
        case VIR_AS_PRIVATE:
        case VIR_AS_GLOBAL:
            if(VIR_Shader_isDual16Mode(Gen->Shader))
            {
                if(precision == VIR_PRECISION_DEFAULT) {
                    return 0x0;
                } else if(precision == VIR_PRECISION_HIGH) {
                    return 0x1;
                } else if(precision == VIR_PRECISION_MEDIUM || precision == VIR_PRECISION_LOW) {
                    return 0x0;
                } else {
                    gcmASSERT(0);
                    return 0x0;
                }
            }
            else
            {
                return 0x0;
            }
        case VIR_AS_LOCAL:
            return 0x1;
        default:
            break;
        }
    }

    gcmASSERT(0);
    return 0x0;
}

static gctUINT
_VSC_MC_GEN_GenIndexed(
    IN VSC_MCGen   *Gen,
    IN VIR_Operand *Opnd
    )
{
    VIR_Indexed indexed = VIR_Operand_GetRelAddrMode(Opnd);

    switch(indexed)
    {
    case VIR_INDEXED_NONE:
        return 0x0;
    case VIR_INDEXED_X:
        if (Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasUniformB0 &&
            VIR_Symbol_isUniform(VIR_Operand_GetSymbol(Opnd)))
        {
            return 0x7;
        }
        else
        {
            return 0x1;
        }
    case VIR_INDEXED_Y:
        gcmASSERT(!Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasUniformB0 ||
                  !VIR_Symbol_isUniform(VIR_Operand_GetSymbol(Opnd)));
        return 0x2;
    case VIR_INDEXED_Z:
        gcmASSERT(!Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasUniformB0 ||
                  !VIR_Symbol_isUniform(VIR_Operand_GetSymbol(Opnd)));
        return 0x3;
    case VIR_INDEXED_W:
        gcmASSERT(!Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasUniformB0 ||
                  !VIR_Symbol_isUniform(VIR_Operand_GetSymbol(Opnd)));
        return 0x4;
    case VIR_INDEXED_AL:
        return 0x5;
    case VIR_INDEXED_VERTEX_ID:
        return 0x6;
    default:
        gcmASSERT(0);
        return 0x0;
    }
}

static void
_VSC_MC_GEN_GenTarget(
    IN VSC_MCGen         *Gen,
    IN VIR_Instruction   *Inst,
    IN VIR_Operand       *Opnd,
    OUT VSC_MC_CODEC_SRC *Src,
    OUT gctBOOL          *bWrite
    )
{
    VIR_OperandKind opndKind = VIR_OPND_NONE;

    gcmASSERT(Gen->Shader != gcvNULL &&
        Opnd != gcvNULL &&
        Inst != gcvNULL);

    opndKind = VIR_Operand_GetOpKind(Opnd);
    *bWrite = gcvTRUE;

    switch(opndKind)
    {
    case VIR_OPND_NONE:
    case VIR_OPND_UNDEF:
        *bWrite = gcvFALSE;
        break;
    case VIR_OPND_LABEL:
        {
            VIR_Label  *label       = VIR_Operand_GetLabel(Opnd);

            gctUINT     labelID     = VIR_Inst_GetId(label->defined);

            Src->regType           = 0x7;

            if (Gen->InstMark[labelID].Inst == gcvNULL
                && Gen->InstMark[labelID].Label != -1)
            {
                Src->u.imm.immData.ui  = Gen->InstMark[labelID].Label;
            }
            else
            {
                gctUINT label = labelID;
                while (gcvTRUE)
                {
                    gctUINT nextLabel = Gen->InstMark[label].Label;

                    if (nextLabel == -1)
                    {
                        break;
                    }

                    label = nextLabel;
                }

                if (Gen->InstMark[label].Inst == gcvNULL)
                {
                    Gen->InstMark[label].Inst = Inst;
                }

                Gen->InstMark[label].Label = Inst->id_;

                Gen->InstMark[Inst->id_].Label = -1;
                Gen->InstMark[Inst->id_].Inst = Inst;
            }
            Src->u.imm.immType     = 0x2;

            break;
        }
    case VIR_OPND_FUNCTION:
        {
            VIR_Function    *func  = VIR_Operand_GetFunction(Opnd);
            VIR_Instruction *fInst = func->instList.pHead;
            gctUINT          labelID = VIR_Inst_GetId(fInst);
            gcmASSERT(fInst != gcvNULL);

            Src->regType           = 0x7;

            if (Gen->InstMark[labelID].Inst == gcvNULL
                && Gen->InstMark[labelID].Label != -1)
            {
                Src->u.imm.immData.ui  = Gen->InstMark[labelID].Label;
            }
            else
            {
                gctUINT label = labelID;
                while (gcvTRUE)
                {
                    gctUINT          nextLabel = Gen->InstMark[label].Label;

                    if (nextLabel == -1)
                    {
                        break;
                    }

                    label = nextLabel;
                }

                if (Gen->InstMark[label].Inst == gcvNULL)
                {
                    Gen->InstMark[label].Inst = Inst;
                }

                Gen->InstMark[label].Label = Inst->id_;

                Gen->InstMark[Inst->id_].Label = -1;
                Gen->InstMark[Inst->id_].Inst = Inst;
            }

            Src->u.imm.immType     = 0x2;
            break;
        }
    default:
        *bWrite = gcvFALSE;
        gcmASSERT(0);
        break;
    }
    return;
}

static void
_VSC_MC_GEN_GenSource(
    IN VSC_MCGen         *Gen,
    IN VSC_MC_CODEC_INST_CTRL *McInstCtrl,
    IN VIR_Instruction   *Inst,
    IN VIR_Operand       *Opnd,
    OUT VSC_MC_CODEC_SRC *Src,
    OUT gctBOOL          *bWrite
    )
{
    VIR_OperandKind opndKind = VIR_OPND_NONE;
    VIR_OpCode      opCode = VIR_Inst_GetOpcode(Inst);

    gcmASSERT(Gen->Shader != gcvNULL &&
        Opnd != gcvNULL &&
        Inst != gcvNULL);

    opndKind = VIR_Operand_GetOpKind(Opnd);
    *bWrite  = gcvTRUE;

     /* dp2 now is 2*dp2, need to set the swizzle to be XYXY */
    if (opCode == VIR_OP_DP2)
    {
        VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(Opnd);

        VIR_Swizzle_SetChannel(swizzle, VIR_CHANNEL_Z, VIR_Swizzle_GetChannel(swizzle, VIR_CHANNEL_X));
        VIR_Swizzle_SetChannel(swizzle, VIR_CHANNEL_W, VIR_Swizzle_GetChannel(swizzle, VIR_CHANNEL_Y));

        VIR_Operand_SetSwizzle(Opnd, swizzle);
    }

    switch(opndKind)
    {
    case VIR_OPND_NONE:
    case VIR_OPND_UNDEF:
    case VIR_OPND_EVIS_MODIFIER:
        *bWrite = gcvFALSE;
        break;
    case VIR_OPND_IMMEDIATE:
        {
            Src->regType           = 0x7;
            Src->u.imm.immData.ui  = VIR_Operand_GetImmediateUint(Opnd);
            Src->u.imm.immType     = _VSC_MC_GEN_GenImmTypeAndValue(Gen, McInstCtrl, Inst, Opnd, &Src->u.imm.immData);

            break;
        }
    case VIR_OPND_SAMPLER_INDEXING:
        {
            VIR_Symbol  *sym        = VIR_Operand_GetSymbol(Opnd);
            gctUINT      index      = VIR_Operand_GetMatrixConstIndex(Opnd) +
                                        _VSC_MC_GEN_GenOpndRelIndexing(Gen, Inst, Opnd);
            gctUINT      swizzle    = VIR_Symbol_isSampler(sym) && VIR_OPCODE_isTexLd(opCode)
                                       ? VIR_SWIZZLE_XYZW : _VSC_MC_GEN_GenOpndSwizzle(Gen, Inst, Opnd);
            gctUINT      indexed    = _VSC_MC_GEN_GenIndexed(Gen, Opnd);
            gctUINT      srcKind    = _VSC_MC_GEN_GenSrcType(Gen, sym);

            Src->regType            = srcKind;
            Src->u.reg.regNo        = index;
            Src->u.reg.swizzle      = swizzle;
            Src->u.reg.indexingAddr = indexed;
            Src->u.reg.bNegative    = (VIR_Operand_GetModifier(Opnd) & VIR_MOD_NEG) != 0;
            Src->u.reg.bAbs         = (VIR_Operand_GetModifier(Opnd) & VIR_MOD_ABS) != 0;
            Src->u.reg.bConstReg    = (srcKind == 0x2);

            /* A WAR to let sampler reg no be zero because all patched code has an assumption that
               it must start with 0. We need remove that assumption and let RA to real assignment.
            */
            if (Src->regType == MC_AUXILIARY_SRC_TYPE_SAMPLER &&
                Src->u.reg.indexingAddr != 0x0)
            {
                Src->u.reg.regNo = 0;
            }

            break;
        }
    case VIR_OPND_SYMBOL:
        {
            VIR_Symbol  *sym        = VIR_Operand_GetSymbol(Opnd);
            gctUINT      srcKind    = _VSC_MC_GEN_GenSrcType(Gen, sym);
            gctUINT      newSrcKind = srcKind;
            gctUINT      index      = _VSC_MC_GEN_GenRegisterNo(Gen, Inst, sym, Opnd) +
                (VIR_Operand_GetMatrixConstIndex(Opnd) +
                 _VSC_MC_GEN_GenOpndRelIndexing(Gen, Inst, Opnd)) *
                 VIR_Symbol_GetRegSize(Gen->Shader, &Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg, sym);
            gctUINT      swizzle    = _VSC_MC_GEN_GenOpndSwizzle(Gen, Inst, Opnd);
            gctUINT      indexed    = _VSC_MC_GEN_GenIndexed(Gen, Opnd);

            /* check if we need 0x4 source type for evis instructions */
            if (srcKind == 0x2 &&
                VIR_OPCODE_U512_SrcNo(opCode) > 0 &&
                VIR_Inst_GetSource(Inst, VIR_OPCODE_U512_SrcNo(opCode)) == Opnd )
            {
                gcmASSERT(VIR_Type_GetSize(VIR_Symbol_GetType(sym)) == 64);
                newSrcKind = 0x4;
            }
            else if (VIR_OPCODE_isVX(opCode) &&
                     srcKind == 0x0 &&
                     (VIR_OPCODE_Src0Src1Temp256(opCode))  &&
                      (VIR_Inst_GetSource(Inst, 0) == Opnd ||
                       VIR_Inst_GetSource(Inst, 1) == Opnd)   )
            {
                /* source is 256 bits test register */
                newSrcKind = 0x1;
            }
            else if (VIR_OPCODE_isVX(opCode) &&
                     srcKind == 0x0 &&
                     (VIR_OPCODE_Src1Src2Temp256(opCode))  &&
                      (VIR_Inst_GetSource(Inst, 1) == Opnd ||
                       VIR_Inst_GetSource(Inst, 2) == Opnd)   )
            {
                /* source is 256 bits test register */
                newSrcKind = 0x1;
            }
            Src->regType            = newSrcKind;
            Src->u.reg.regNo        = index;
            Src->u.reg.swizzle      = swizzle;
            Src->u.reg.indexingAddr = indexed;
            Src->u.reg.bNegative    = (VIR_Operand_GetModifier(Opnd) & VIR_MOD_NEG) != 0;
            Src->u.reg.bAbs         = (VIR_Operand_GetModifier(Opnd) & VIR_MOD_ABS) != 0;
            Src->u.reg.bConstReg    = (srcKind == 0x2);

            break;
        }
    default:
        gcmASSERT(0);
        *bWrite = gcvFALSE;
        break;
    }

    return;
}

static VSC_ErrCode
_VSC_MC_GEN_GenDest(
    IN VSC_MCGen         *Gen,
    IN VIR_Instruction   *Inst,
    IN VIR_Operand       *Opnd,
    OUT VSC_MC_CODEC_DST *Dst,
    OUT gctBOOL          *bWrite
    )
{
    VSC_ErrCode  status = VSC_ERR_NONE;
    VIR_OpCode opCode = VIR_Inst_GetOpcode(Inst);

    gcmASSERT(Gen->Shader != gcvNULL &&
        Inst != gcvNULL);

    /* Now we save the evis startBin/endBin in the DST, so get this information first. */
    if (VIR_OPCODE_isVX(opCode))
    {
        VIR_EVIS_Modifier evisModifier;
        gctUINT i;

        /* find EvisModifier operand  */
        for (i = 0; i < VIR_Inst_GetSrcNum(Inst); i ++)
        {
            VIR_Operand * opnd = VIR_Inst_GetSource(Inst, i);
            if (opnd && VIR_Operand_GetOpKind(opnd) == VIR_OPND_EVIS_MODIFIER)
            {
                evisModifier.u1 = VIR_Operand_GetEvisModifier(opnd);

                Dst->u.evisDst.startCompIdx  = evisModifier.u0.startBin;
                Dst->u.evisDst.compIdxRange  = evisModifier.u0.endBin - evisModifier.u0.startBin + 1;
                break;
            }
        }
    }

    if (Opnd == gcvNULL)
    {
        /* For some evis instructions, there is no DST, we only need to get the evis information. */
        if (VIR_OPCODE_isVX(opCode))
        {
            Dst->bEvisInfoOnly = gcvTRUE;
        }
        *bWrite = gcvFALSE;
        return status;
    }

    *bWrite = gcvTRUE;

    switch(VIR_Operand_GetOpKind(Opnd))
    {
    case VIR_OPND_NONE:
    case VIR_OPND_UNDEF:
        *bWrite = gcvFALSE;
        break;
    case VIR_OPND_SYMBOL:
        {
            VIR_Symbol  *sym          = VIR_Operand_GetSymbol(Opnd);
            gctUINT      dstKind      = _VSC_MC_GEN_GenDstType(Gen, sym);
            gctUINT      index        = _VSC_MC_GEN_GenRegisterNo(Gen, Inst, sym, Opnd) +
                (VIR_Operand_GetMatrixConstIndex(Opnd) +
                 _VSC_MC_GEN_GenOpndRelIndexing(Gen, Inst, Opnd)) *
                 VIR_Symbol_GetRegSize(Gen->Shader, &Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg, sym);
            gctUINT      enable       = _VSC_MC_GEN_GenOpndEnable(Gen, Inst, Opnd);
            gctUINT      indexed      = _VSC_MC_GEN_GenIndexed(Gen, Opnd);

            Dst->regType              = dstKind;
            Dst->regNo                = index;

            if (!VIR_OPCODE_isVX(VIR_Inst_GetOpcode(Inst)))
            {
                Dst->u.nmlDst.writeMask            = enable;
                Dst->u.nmlDst.indexingAddr         = indexed;
            }

            break;
        }
    default:
        *bWrite = gcvFALSE;
        gcmASSERT(0);
        break;
    }

    return status;
}

static VSC_ErrCode
_VSC_MC_GEN_GenGeneralInst(
    IN VSC_MCGen         *Gen,
    IN VIR_Instruction   *Inst,
    OUT gctUINT          *BaseOpcode,
    OUT gctUINT          *ExternOpcode,
    OUT VSC_MC_CODEC_INST_CTRL *McInstCtrl,
    OUT VSC_MC_CODEC_DST *Dst,
    OUT gctBOOL          *bWrite,
    OUT VSC_MC_CODEC_SRC *Src,
    OUT gctUINT          *srcNum
    )
{
    gctSIZE_T  i = 0;
    VIR_OpCode opCode = VIR_Inst_GetOpcode(Inst);

    _VSC_MC_GEN_GenOpcode(Gen, Inst, BaseOpcode, ExternOpcode);
    _VSC_MC_GEN_GenInstCtrl(Gen, Inst, McInstCtrl);
    _VSC_MC_GEN_GenDest(Gen, Inst, VIR_Inst_GetDest(Inst), Dst, bWrite);

    /* A WAR to handle helper pixel case. It will be better that remove corresponding
       srcs used as pixel-helper in ll2mc lower to assure there are no garbage srcs!! */
    if (McInstCtrl->condOpCode == 0x18 ||
        McInstCtrl->condOpCode == 0x19 )
    {
        if (0x0F == *BaseOpcode)
        {
            i = 1;
        }
        else if (*BaseOpcode == 0x10 ||
                 *BaseOpcode == 0x17 ||
                 *BaseOpcode == 0x16)
        {
            i = VIR_Inst_GetSrcNum(Inst);
        }
        else
        {
            i = VIR_Inst_GetSrcNum(Inst) - 1;
        }
    }

    for(; i < VIR_Inst_GetSrcNum(Inst); ++i)
    {
        gctBOOL bWrite = gcvFALSE;
        VIR_Operand * opnd = VIR_Inst_GetSource(Inst, i);
        if (VIR_OPCODE_isVX(opCode) && i == 1)
        {
            /* skip the hidden temp 256 register lower part */
            if (VIR_OPCODE_Src0Src1Temp256(opCode))
            {
                /* must be consecutive and start with even temp register */
                gcmASSERT((VIR_Operand_GetHwRegId(VIR_Inst_GetSource(Inst, 0)) ==
                           VIR_Operand_GetHwRegId(opnd) - 1) &&
                          ((VIR_Operand_GetHwRegId(opnd) & 0x01) == 1) );
                continue;
            }
        }
        else if (VIR_OPCODE_isVX(opCode) && i == 2)
        {
            /* skip the hidden temp 256 register lower part */
            if (VIR_OPCODE_Src1Src2Temp256(opCode))
            {
                /* must be consecutive and start with even temp register */
                gcmASSERT((VIR_Operand_GetHwRegId(VIR_Inst_GetSource(Inst, 1)) ==
                           VIR_Operand_GetHwRegId(opnd) - 1) &&
                          ((VIR_Operand_GetHwRegId(opnd) & 0x01) == 1) );
                continue;
            }
        }
        if ((opCode == VIR_OP_IMG_LOAD || opCode == VIR_OP_IMG_LOAD_3D) &&
            i == (VIR_Inst_GetSrcNum(Inst) -1) )
        {
            /* the last operand of img_load/load_3d is sampler info,
             * it is not in the instruction now. */
            continue;
        }
        _VSC_MC_GEN_GenSource(
            Gen,
            McInstCtrl,
            Inst,
            opnd,
            Src + *srcNum,
            &bWrite);

        if ((Src + *srcNum)->u.reg.bConstReg &&
            (Src + *srcNum)->regType == 0x4)
        {
            McInstCtrl->u.visionCtrl.bUseUniform512 = gcvTRUE;
        }

        if(bWrite)
        {
            ++(*srcNum);
        }
    }
    return VSC_ERR_NONE;
}

static void _NegMcSrc(VSC_MC_CODEC_SRC *McSrc, VIR_Operand * Opnd)
{
    VIR_TypeId ty           = VIR_Operand_GetTypeId(Opnd);

    if (McSrc->regType == 0x7)
    {
        if (VIR_TypeId_isPacked(ty))
        {
            gctUINT     ui;
            gctINT      si;
            switch (McSrc->u.imm.immType)
            {
            case 0x2:
                switch (ty)
                {
                case VIR_TYPE_INT8_P2:
                case VIR_TYPE_UINT8_P2:
                case VIR_TYPE_BOOLEAN_P2:
                    si = -(gctCHAR)(McSrc->u.imm.immData.si) & 0xFF;
                    McSrc->u.imm.immData.ui = (gctUINT)((si & 0xFF) | ((si & 0xFF)<< 8) );
                    break;
                default:
                    gcmASSERT(gcvFALSE);
                    break;
                }
                break;
            case 0x3:
                switch (ty)
                {
                case VIR_TYPE_INT8_P3:
                case VIR_TYPE_INT8_P4:
                case VIR_TYPE_INT8_P8:
                case VIR_TYPE_INT8_P16:
                case VIR_TYPE_UINT8_P3:
                case VIR_TYPE_UINT8_P4:
                case VIR_TYPE_UINT8_P8:
                case VIR_TYPE_UINT8_P16:
                case VIR_TYPE_BOOLEAN_P3:
                case VIR_TYPE_BOOLEAN_P4:
                case VIR_TYPE_BOOLEAN_P8:
                case VIR_TYPE_BOOLEAN_P16:
                    si = -(gctCHAR)((McSrc->u.imm.immData.si) & 0xFF);
                    ui = (gctUINT)((si & 0xFF) | ((si & 0xFF)<< 8) );
                    McSrc->u.imm.immData.ui = ui;
                    break;
                case VIR_TYPE_INT16_P2:
                case VIR_TYPE_UINT16_P2:
                case VIR_TYPE_INT16_P3:
                case VIR_TYPE_INT16_P4:
                case VIR_TYPE_INT16_P8:
                case VIR_TYPE_UINT16_P3:
                case VIR_TYPE_UINT16_P4:
                case VIR_TYPE_UINT16_P8:
                    si = -(gctINT16)((McSrc->u.imm.immData.si) & 0xFFFF);
                    McSrc->u.imm.immData.ui =  (si & 0xFFFF);
                    break;
                default:
                    gcmASSERT(gcvFALSE);
                    break;
                }
                break;
            default:
                gcmASSERT(gcvFALSE);
                break;
            }
        }
        else
        {
            switch (McSrc->u.imm.immType)
            {
            case 0x0:
                McSrc->u.imm.immData.f  = -McSrc->u.imm.immData.f;
                break;
            case 0x1:
                McSrc->u.imm.immData.si = -McSrc->u.imm.immData.si;
                break;
            case 0x2:
                McSrc->u.imm.immData.si = -(gctINT)(McSrc->u.imm.immData.ui & 0x7ffff);
                McSrc->u.imm.immType    = 0x1;
                break;
            case 0x3:
                McSrc->u.imm.immData.si = -McSrc->u.imm.immData.si;
                break;
            default:
                gcmASSERT(0);
                break;
            }
        }
    }
    else
    {
        McSrc->u.reg.bNegative = !McSrc->u.reg.bNegative;
    }
}

static void _AbsMcSrc(VSC_MC_CODEC_SRC *McSrc)
{
    if (McSrc->regType == 0x7)
    {
        switch (McSrc->u.imm.immType)
        {
        case 0x0:
            McSrc->u.imm.immData.f  = McSrc->u.imm.immData.f < 0.0 ?
                -McSrc->u.imm.immData.f : McSrc->u.imm.immData.f;
            break;
        case 0x1:
            McSrc->u.imm.immData.si = McSrc->u.imm.immData.si < 0 ?
                -McSrc->u.imm.immData.si : McSrc->u.imm.immData.si;
            break;
        case 0x2:
            break;
        default:
            gcmASSERT(0);
            break;
        }
    }
    else
    {
        McSrc->u.reg.bAbs      = gcvTRUE;
        McSrc->u.reg.bNegative = gcvFALSE;
    }
}

static gctBOOL _NeedGen(gctUINT                baseOpcode,
                        gctUINT                extOpcode,
                        VSC_MC_CODEC_INST_CTRL*pInstCtrl,
                        VSC_MC_CODEC_DST*      pDst,
                        VSC_MC_CODEC_SRC*      pSrc,
                        gctUINT                srcCount)
{
    gctUINT channel;
    gctBOOL debugEnabled = gcmOPT_DisableOPTforDebugger();
    if (!debugEnabled)
    {
        /* FB_INSERT_MOV_INPUT: insert MOV Rn, Rn for input to help HW team to debug */
        if (!gcmOPT_hasFeature(FB_INSERT_MOV_INPUT) &&
            !pInstCtrl->bForceGen &&
            baseOpcode == 0x09 &&
            pSrc[srcCount - 1].regType == 0x0 &&
            pSrc[srcCount - 1].u.reg.regNo == pDst->regNo &&
            pDst->u.nmlDst.indexingAddr == pSrc[srcCount - 1].u.reg.indexingAddr &&
            !pSrc[srcCount - 1].u.reg.bAbs &&
            !pSrc[srcCount - 1].u.reg.bNegative &&
            !pInstCtrl->bResultSat &&
            pInstCtrl->condOpCode == 0x00 &&
            pSrc[srcCount - 1].regType == pDst->regType)
        {
            for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
            {
                if (pDst->u.nmlDst.writeMask & (1 << channel))
                {
                    if (((pSrc[0].u.reg.swizzle >> channel * 2) & 0x3) != channel)
                    {
                        return gcvTRUE;
                    }
                }
            }

            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

static VSC_ErrCode
_VSC_MC_GEN_GenInst(
    IN VSC_MCGen       *Gen,
    IN VIR_Function    *Func,
    IN VIR_Instruction *Inst,
    IN gctBOOL         bIsBackFill,
    OUT gctUINT        *GenCount
    )
{
    VSC_ErrCode             status       = VSC_ERR_NONE;
    VIR_OpCode              virOpcode    = VIR_Inst_GetOpcode(Inst);

    gctSIZE_T               i            = 0;
    gctUINT                 baseOpcode   = 0;
    gctUINT                 externOpcode = 0;
    gctUINT                 srcNum       = 0;
    gctBOOL                 bDstWrite    = gcvFALSE;
    gctBOOL                 bNeedGen     = gcvTRUE;

    VSC_MC_CODEC_INST_CTRL  mcInstCtrl;
    VSC_MC_CODEC_DST        mcDest;
    VSC_MC_CODEC_SRC        mcSrc[MAX_MC_SRC_COUNT];
    gctBOOL                 bNeedAppendNop = (Inst == Func->instList.pTail &&
                                              (Gen->Shader->functions.info.count > 1 || virOpcode == VIR_OP_BARRIER) &&
                                              Func == Gen->Shader->mainFunction);

    memset(&mcInstCtrl, 0, sizeof(mcInstCtrl));
    memset(&mcDest, 0, sizeof(mcDest));
    memset(mcSrc, 0, sizeof(mcSrc));

    _CheckNeedToBeFixed(Inst, GenCount);

    switch(virOpcode)
    {
    case VIR_OP_LABEL:
        if (Gen->InstMark[Inst->id_].Inst != gcvNULL)
        {
            _VSC_MC_GEN_BackFill(Gen, Inst, Gen->InstCount);
        }
        else
        {
            Gen->InstMark[Inst->id_].Inst  = gcvNULL;
            Gen->InstMark[Inst->id_].Label = Gen->InstCount;
        }
        *GenCount = 0;
        break;
    case VIR_OP_NOP:
        *GenCount = 0;
        break;
    case VIR_OP_JMP:
    case VIR_OP_JMPC:
    case VIR_OP_JMP_ANY:
    case VIR_OP_CALL:
        {
            gctBOOL bWrite = gcvFALSE;

            _VSC_MC_GEN_GenOpcode(Gen, Inst, &baseOpcode, &externOpcode);
            _VSC_MC_GEN_GenInstCtrl(Gen, Inst, &mcInstCtrl);

            for(i = 0; i < VIR_Inst_GetSrcNum(Inst); ++i)
            {
                _VSC_MC_GEN_GenSource(
                    Gen,
                    &mcInstCtrl,
                    Inst,
                    VIR_Inst_GetSource(Inst, i),
                    mcSrc + srcNum,
                    &bWrite);

                if(bWrite)
                {
                    ++srcNum;
                }
            }

            _VSC_MC_GEN_GenTarget(Gen, Inst, VIR_Inst_GetDest(Inst), mcSrc + srcNum, &bWrite);

            if(bWrite)
            {
                ++srcNum;
            }

            break;
        }
    case VIR_OP_SAT:
        {
            VIR_OperandInfo dst;

            VIR_Operand_GetOperandInfo(Inst, Inst->dest, &dst);

            _VSC_MC_GEN_GenGeneralInst(Gen,
                Inst,
                &baseOpcode,
                &externOpcode,
                &mcInstCtrl,
                &mcDest,
                &bDstWrite,
                mcSrc,
                &srcNum);

            gcmASSERT(VIR_OpndInfo_Is_Virtual_Reg(&dst));
            mcInstCtrl.bResultSat = gcvTRUE;

            break;
        }
    case VIR_OP_ABS:
        {
            _VSC_MC_GEN_GenGeneralInst(Gen,
                Inst,
                &baseOpcode,
                &externOpcode,
                &mcInstCtrl,
                &mcDest,
                &bDstWrite,
                mcSrc,
                &srcNum);

            if (mcInstCtrl.instType ==  0x0 ||
                mcInstCtrl.instType ==  0x1)
            {
                _AbsMcSrc(&mcSrc[0]);
            }

            break;
        }
    case VIR_OP_NEG:
        {
            _VSC_MC_GEN_GenGeneralInst(Gen,
                Inst,
                &baseOpcode,
                &externOpcode,
                &mcInstCtrl,
                &mcDest,
                &bDstWrite,
                mcSrc,
                &srcNum);

            _NegMcSrc(&mcSrc[0], VIR_Inst_GetSource(Inst, 0));

            if (mcInstCtrl.instType !=  0x0 &&
                mcInstCtrl.instType !=  0x1)
            {
                gcmASSERT(baseOpcode == 0x01);

                srcNum = 2;

                mcSrc[1] = mcSrc[0];

                memset(&mcSrc[0], 0, sizeof(mcSrc[0]));

                mcSrc[0].regType           = 0x7;
                mcSrc[0].u.imm.immData.ui  = 0;
                if (VIR_Shader_isDual16Mode(Gen->Shader) &&
                    (VIR_Inst_GetThreadMode(Inst) == VIR_THREAD_D16_DUAL_16 ||
                     VIR_Inst_GetThreadMode(Inst) == VIR_THREAD_D16_DUAL_HIGHPVEC2))
                {
                    mcSrc[0].u.imm.immType     = 0x3;
                }
                else
                {
                    mcSrc[0].u.imm.immType     = 0x2;
                }

            }

            break;
        }
    case VIR_OP_ATOMSUB:
    case VIR_OP_ATOMSUB_L:
        {
            _VSC_MC_GEN_GenGeneralInst(Gen,
                Inst,
                &baseOpcode,
                &externOpcode,
                &mcInstCtrl,
                &mcDest,
                &bDstWrite,
                mcSrc,
                &srcNum);

            _NegMcSrc(&mcSrc[2], VIR_Inst_GetSource(Inst, 2));

            break;
        }
    case VIR_OP_SUB:
    case VIR_OP_SUBSAT:
        {
            _VSC_MC_GEN_GenGeneralInst(Gen,
                Inst,
                &baseOpcode,
                &externOpcode,
                &mcInstCtrl,
                &mcDest,
                &bDstWrite,
                mcSrc,
                &srcNum);

            _NegMcSrc(&mcSrc[1], VIR_Inst_GetSource(Inst, 1));

            break;
        }
    case VIR_OP_DSY:
    case VIR_OP_DSX:
        {
            _VSC_MC_GEN_GenGeneralInst(Gen,
                Inst,
                &baseOpcode,
                &externOpcode,
                &mcInstCtrl,
                &mcDest,
                &bDstWrite,
                mcSrc,
                &srcNum);

            if (srcNum == 1)
            {
                mcSrc[1] = mcSrc[0];
                srcNum++;
            }

            break;
        }

    case VIR_OP_SELECT_MAP:
        {
            _VSC_MC_GEN_GenGeneralInst(Gen,
                Inst,
                &baseOpcode,
                &externOpcode,
                &mcInstCtrl,
                &mcDest,
                &bDstWrite,
                mcSrc,
                &srcNum);

            gcmASSERT(srcNum == 4);
            gcmASSERT(mcSrc[3].regType == 0x7);

            mcInstCtrl.u.smCtrl.bCompSel = (mcSrc[3].u.imm.immData.ui & 0x80) >> 7;
            mcInstCtrl.u.smCtrl.rangeToMatch = mcSrc[3].u.imm.immData.ui & 0xf;
            srcNum --;

            break;
        }
    case VIR_OP_EMIT:
        {
            _VSC_MC_GEN_GenGeneralInst(Gen,
                Inst,
                &baseOpcode,
                &externOpcode,
                &mcInstCtrl,
                &mcDest,
                &bDstWrite,
                mcSrc,
                &srcNum);

            gcmASSERT(srcNum == 2);
            gcmASSERT(mcSrc[1].regType == 0x7);
            mcInstCtrl.u.emitCtrl.bNeedRestartPrim = mcSrc[1].u.imm.immData.ui & 0x01;
            mcInstCtrl.u.emitCtrl.bNoJmpToEndOnMaxVtxCnt = gcvTRUE;
            srcNum --;

            break;
        }
    case VIR_OP_GET_SAMPLER_IDX:
        {
            VIR_Operand *uniformSrc = VIR_Inst_GetSource(Inst, 0);
            VIR_Operand *indexSrc = VIR_Inst_GetSource(Inst, 1);
            VIR_Symbol  *uniform = VIR_Operand_GetSymbol(uniformSrc);

            _VSC_MC_GEN_GenOpcode(Gen, Inst, &baseOpcode, &externOpcode);
            _VSC_MC_GEN_GenInstCtrl(Gen, Inst, &mcInstCtrl);
            _VSC_MC_GEN_GenDest(Gen, Inst, VIR_Inst_GetDest(Inst), &mcDest, &bDstWrite);

            /* Generate the sampler physical address. */
            mcSrc->regType           = 0x7;
            mcSrc->u.imm.immType     = _VSC_MC_GEN_GenImmTypeAndValue(Gen, &mcInstCtrl, Inst, indexSrc, gcvNULL);
            if (mcSrc->u.imm.immType == 0x0)
            {
                mcSrc->u.imm.immData.f = (gctFLOAT)_VSC_MC_GEN_GenRegisterNo(Gen, Inst, uniform, uniformSrc);
            }
            else
            {
                mcSrc->u.imm.immData.ui = _VSC_MC_GEN_GenRegisterNo(Gen, Inst, uniform, uniformSrc);
            }

            /* Generate the array index. */
            _VSC_MC_GEN_GenSource(
                Gen,
                &mcInstCtrl,
                Inst,
                indexSrc,
                mcSrc + 1,
                &bDstWrite);

            srcNum = 2;
            break;
        }
    case VIR_OP_MOVA:
        {
            _VSC_MC_GEN_GenGeneralInst(Gen,
                                       Inst,
                                       &baseOpcode,
                                       &externOpcode,
                                       &mcInstCtrl,
                                       &mcDest,
                                       &bDstWrite,
                                       mcSrc,
                                       &srcNum);

            if (Gen->pComCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasUniformB0 &&
                VIR_Operand_isUniformIndex(VIR_Inst_GetDest(Inst)))
            {
                baseOpcode = 0x7F;
                externOpcode = 0x13;

                mcSrc[1].regType = 0x7;
                mcSrc[1].u.imm.immData.ui = 0;
                mcSrc[1].u.imm.immType = 0x2;
                srcNum = 2;
            }

            break;
        }

    default:
        _VSC_MC_GEN_GenGeneralInst(Gen,
            Inst,
            &baseOpcode,
            &externOpcode,
            &mcInstCtrl,
            &mcDest,
            &bDstWrite,
            mcSrc,
            &srcNum);
        break;
    }

    /* A quick WAR for FPGA verification to convert texld_plain to texld_lod for all
       non-ps shaders because texld_plain will compute LOD but non-ps shader has no
       ability to compute, otherwise, HW will hange.

       The correct fix should change all the original sources (most are in our src lib
       for recompile/builtins) to proper texld*, but it needs us to change lots of code.
       We will do this correct thing when we have time later.

       Because this conversion will assume lod is zero, so if failures are occured by
       doing this conversion, just check the original source and give out correct lod
       number.*/
    if (Gen->Shader->shaderKind != VIR_SHADER_FRAGMENT)
    {
        if (baseOpcode == MC_AUXILIARY_OP_CODE_TEXLD_PLAIN)
        {
            baseOpcode = MC_AUXILIARY_OP_CODE_TEXLD_LOD;
            mcSrc[2].regType = 0x7;
            mcSrc[2].u.imm.immData.f = 0.0;
            mcSrc[2].u.imm.immType = 0x0;
            srcNum = 3;
        }
    }

    if (*GenCount == 1)
    {
        /* we need always to gen endOfBB instruction, otherwise it may cause HW hang */
        if (!Inst->_endOfBB && !_NeedGen(baseOpcode,
                      externOpcode,
                      &mcInstCtrl,
                      bDstWrite ? &mcDest : gcvNULL,
                      mcSrc,
                      srcNum))
        {
            *GenCount = 0;
        }
    }

    if (*GenCount == 0)
    {
        bNeedGen = gcvFALSE;
    }

    /* Append a NOP at end of main function if there are other subroutines */
    if (bNeedAppendNop)
    {
        (*GenCount) ++;
    }

    if (*GenCount == 0)
    {
        return VSC_ERR_NONE;
    }

    VIR_Inst_InitMcInsts(Inst, Gen->Shader, *GenCount, (gctINT32)Gen->InstCount, !bIsBackFill);

    if (bNeedGen)
    {
        if (baseOpcode == 0x17)
        {
            Gen->Shader->psHasDiscard = gcvTRUE;
        }

        if (virOpcode == VIR_OP_BARRIER && Gen->Shader->shaderKind == VIR_SHADER_COMPUTE)
        {
            Gen->Shader->hasThreadGroupSync = gcvTRUE;
        }

        if (baseOpcode == 0x07)
        {
            Gen->Shader->hasDsx = gcvTRUE;
        }
        if (baseOpcode == 0x08)
        {
            Gen->Shader->hasDsy = gcvTRUE;
        }

        vscMC_EncodeInstDirect(&Gen->MCCodec,
                               baseOpcode,
                               externOpcode,
                               mcInstCtrl,
                               (bDstWrite || mcDest.bEvisInfoOnly) ? &mcDest : gcvNULL,
                               mcSrc,
                               srcNum,
                               Inst->mcInst);
    }

    return status;
}

static VSC_ErrCode
_VSC_MC_GEN_GenInstOnFunc(
    IN VSC_MCGen      *Gen,
    IN VIR_Function   *Func
    )
{
    VSC_ErrCode       status = VSC_ERR_NONE;
    VIR_Instruction  *inst;
    VIR_InstIterator  instIter;

    VIR_InstIterator_Init(&instIter, &Func->instList);
    inst = VIR_InstIterator_First(&instIter);

    if (inst != gcvNULL)
    {
        if (Gen->InstMark[inst->id_].Inst != gcvNULL)
        {
            _VSC_MC_GEN_BackFill(Gen, inst, Gen->InstCount);
        }
        else
        {
            Gen->InstMark[inst->id_].Inst  = gcvNULL;
            Gen->InstMark[inst->id_].Label = Gen->InstCount;
        }
    }

    for (; inst != gcvNULL; inst = VIR_InstIterator_Next(&instIter))
    {
        gctUINT genCount = 0;
        _VSC_MC_GEN_GenInst(Gen, Func, inst, gcvFALSE, &genCount);
        Gen->InstCount += genCount;
    }
    return status;
}

static VSC_ErrCode
_VSC_MC_GEN_GenInstOnShader(
    IN VSC_MCGen      *Gen
    )
{
    VSC_ErrCode       status    = VSC_ERR_NONE;
    VIR_Shader       *shader    = Gen->Shader;
    VIR_FunctionNode *funcNode  = gcvNULL;
    VIR_FuncIterator  funcIter;

    if(Gen->Shader->functions.info.count <= 0)
        return status;

    if(shader->mainFunction == gcvNULL)
        return status;

    _VSC_MC_GEN_GenInstOnFunc(Gen, shader->mainFunction);

    VIR_FuncIterator_Init(&funcIter, &shader->functions);
    funcNode = VIR_FuncIterator_First(&funcIter);

    for(;funcNode != gcvNULL; funcNode = VIR_FuncIterator_Next(&funcIter))
    {
        VIR_Function *func = funcNode->function;
        if(!(func->flags & VIR_FUNCFLAG_MAIN))
        {
            _VSC_MC_GEN_GenInstOnFunc(Gen, func);
        }
    }

    return status;
}

static VSC_ErrCode
_VSC_MC_GEN_PerformOnShader(
    IN VSC_MCGen      *Gen
    )
{
    VSC_ErrCode status = VSC_ERR_NONE;
    if(VIR_Shader_isRAEnabled(Gen->Shader) == gcvFALSE)
    {
        return VSC_ERR_NOT_SUPPORTED;
    }

    if(VSC_UTILS_MASK(VSC_OPTN_MCGenOptions_GetTrace(Gen->Options),
        VSC_OPTN_MCGenOptions_TRACE_OUTPUT))
    {
        VIR_LOG(Gen->Dumper, "==============================");
        VIR_LOG_FLUSH(Gen->Dumper);
    }

    status = _VSC_MC_GEN_GenInstOnShader(Gen);
    if(status != VSC_ERR_NONE) return status;

    if(VSC_UTILS_MASK(VSC_OPTN_MCGenOptions_GetTrace(Gen->Options),
        VSC_OPTN_MCGenOptions_TRACE_OUTPUT))
    {
        VIR_LOG(Gen->Dumper, "*******************************");
        VIR_LOG_FLUSH(Gen->Dumper);
    }

    return status;
}

static void
_dumpHashPerfData(
    IN OUT VIR_Dumper*  Dumper,
    IN gctSTRING        pTableName,
    IN VSC_HASH_TABLE*  pHashTable
    )
{
    gctINT              i;
    gctUINT             count = 0;
    gctUINT             longestCount= 0;
    gctINT              averageSearchTimes = 0;
    VSC_HASH_NODE_LIST* pList = gcvNULL;

    gcmASSERT(Dumper != gcvNULL);
    /* Dump hash table name */
    gcoOS_Print("Name : %s\n", pTableName);

    if (pHashTable->itemCount == 0)
    {
        VERIFY_OK(
            VIR_LOG(Dumper, "This hash table no node.\n"));
    }
    else
    {
        /* Dump the hash table size and total number of nodes */
        VERIFY_OK(
            VIR_LOG(Dumper, "Table size is : %d\nTotal number of nodes is : %d\n", pHashTable->tableSize, pHashTable->itemCount));

        /* Dump the empty list number of the hash table */
        for (i = 0; i < pHashTable->tableSize; i++)
        {
            pList = &(pHashTable->pTable[i]);
            if(pList->info.count == 0)
                count++;
        }
        VERIFY_OK(
            VIR_LOG(Dumper, "Number of empty list is : %d\n", count));

        /* Dump hash table longest list and it`s number */
        for (i = 0; i < pHashTable->tableSize; i++)
        {
            pList = &(pHashTable->pTable[i]);
            if(longestCount < pList->info.count)
            {
                longestCount = pList->info.count;
            }
        }

        count = 0;
        for (i = 0; i < pHashTable->tableSize; i++)
        {
            pList = &(pHashTable->pTable[i]);
            if (pList->info.count == longestCount)
                count++;
        }
        VERIFY_OK(
            VIR_LOG(Dumper, "Number of longest list is : %d\nNumber of nodes in longest list is : %d\n", count, longestCount));

        /* Dump hash table search message */
        if (pHashTable->searchTime->searchTotal == 0)
        {
            VERIFY_OK(
                VIR_LOG(Dumper, "This hash table NO search !\n"));
        }
        else
        {
            averageSearchTimes = pHashTable->searchTime->searchTotal /
                (pHashTable->searchTime->searchSucceed + pHashTable->searchTime->searchFailed);
            VERIFY_OK(
                VIR_LOG(Dumper, "Total search time is : %d\nAverage search time is : %d\n",
                pHashTable->searchTime->searchTotal, averageSearchTimes));

            VERIFY_OK(
                VIR_LOG(Dumper, "Succeed search time is : %d\nFailed search time is : %d\n",
                pHashTable->searchTime->searchSucceed, pHashTable->searchTime->searchFailed));

            VERIFY_OK(
                VIR_LOG(Dumper, "Most search time is : %d\n", pHashTable->searchTime->searchMostTimes));

            VERIFY_OK(
                VIR_LOG(Dumper, "Number of most search time is : %d\n\n", pHashTable->searchTime->searchMostCount));

            VERIFY_OK(
                VIR_LOG(Dumper, "Search array times as follows:\n"));
            for (i = 1; i < HTBL_MAX_SEARCH_TIMES(pHashTable); i++)
            {
                if (pHashTable->searchTime->searchTimesArray[i] != 0)
                {
                    VERIFY_OK(
                        VIR_LOG(Dumper, "Number of search %d times is %d\n", i, pHashTable->searchTime->searchTimesArray[i]));
                    VIR_LOG_FLUSH(Dumper);
                }
            }
            VERIFY_OK(
                VIR_LOG(Dumper, "Over max search times(%d) has %d search times\n",
                HTBL_MAX_SEARCH_TIMES(pHashTable),pHashTable->searchTime->searchTimesArray[HTBL_MAX_SEARCH_TIMES(pHashTable)]));
            VERIFY_OK(
                VIR_LOG(Dumper, "Other search array times is 0\n"));
        }
    }
    VERIFY_OK(
        VIR_LOG(Dumper, "\n********************************************************\n"));
    VIR_LOG_FLUSH(Dumper);
}

static void
_dumpShaderHashTable(
    IN OUT VIR_Dumper*  Dumper,
    VIR_Shader*         Shader
    )
{
    if (Shader->stringTable.flag & VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES)
    {
        _dumpHashPerfData(Dumper, "stringTable", Shader->stringTable.pHashTable);
    }
    if (Shader->symTable.flag & VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES)
    {
        _dumpHashPerfData(Dumper, "symbalTable", Shader->symTable.pHashTable);
    }

    if (Shader->typeTable.flag & VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES)
    {
        _dumpHashPerfData(Dumper, "typeTable", Shader->typeTable.pHashTable);
    }

    if (Shader->constTable.flag & VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES)
    {
        _dumpHashPerfData(Dumper, "constTable", Shader->constTable.pHashTable);
    }

    if (Shader->instTable.flag & VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES)
    {
        _dumpHashPerfData(Dumper, "instTable", Shader->instTable.pHashTable);
    }

    if (Shader->symAliasTable.pHashTable != gcvNULL)
    {
        _dumpHashPerfData(Dumper, "symAliasTable", Shader->symAliasTable.pHashTable);
    }

    if (Shader->funcTable != gcvNULL)
    {
        _dumpHashPerfData(Dumper, "funcTable", Shader->funcTable);
    }

    if (VIR_Shader_GetVirRegTable(Shader) != gcvNULL)
    {
        _dumpHashPerfData(Dumper, "virRegTable", VIR_Shader_GetVirRegTable(Shader));
    }
}

DEF_QUERY_PASS_PROP(VSC_MC_GEN_MachineCodeGen)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_CG;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_MC_GEN;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;
}

DEF_SH_NECESSITY_CHECK(VSC_MC_GEN_MachineCodeGen)
{
    return gcvTRUE;
}

VSC_ErrCode
VSC_MC_GEN_MachineCodeGen(
    VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode           errCode = VSC_ERR_NONE;
    VSC_MCGen             gen;
    VIR_Shader            *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_COMPILER_CONFIG   *pComCfg = &pPassWorker->pCompilerParam->cfg;
    VSC_OPTN_MCGenOptions *Options = (VSC_OPTN_MCGenOptions*)pPassWorker->basePassWorker.pBaseOption;
    VIR_Dumper            *pDumper = pPassWorker->basePassWorker.pDumper;

    _VSC_MC_GEN_Initialize(pShader, pComCfg, Options, pDumper, pPassWorker->basePassWorker.pMM, &gen);

     /* Dump hash table performance before code gen */
    if (gcmGetOptimizerOption()->dumpHashPerf)
    {
        VERIFY_OK(
            VIR_LOG(pDumper, "************* Dump hash table performance **************\n"));
        VIR_LOG_FLUSH(pDumper);
        _dumpShaderHashTable(pDumper, pShader);
    }

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "Before Machine code gen", pShader, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }

    errCode = _VSC_MC_GEN_PerformOnShader(&gen);
    _VSC_MC_GEN_Finalize(&gen);

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Machine code gen", pShader, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }

    return errCode;
}


