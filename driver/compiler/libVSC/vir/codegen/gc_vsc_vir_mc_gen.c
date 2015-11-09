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
#include "vir/codegen/gc_vsc_vir_mc_gen.h"

void
gcDumpMCStates(
               IN gctUINT32 Address,
               IN gctUINT32 * States,
               IN gctBOOL OutputFormat,
               IN gctBOOL OutputHexStates,
               IN gctBOOL OutputDual16Modifiers,
               IN gctSIZE_T BufSize,
               OUT gctSTRING Buffer
               );

static VSC_ErrCode
_VSC_MC_GEN_GenInst(
    IN VSC_MCGen       *Gen,
    IN VIR_Function    *Func,
    IN VIR_Instruction *Inst,
    OUT gctUINT        *GenCount
    );

/* To be fixed in lower and MC-gen ASAP */
static void _CheckNeedToBeFixed(IN VIR_Instruction  *Inst, gctUINT* GenCount)
{
    gctSIZE_T  i = 0;
    VIR_OpCode opcode = VIR_Inst_GetOpcode(Inst);

    *GenCount = 1;

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
_VSC_MC_GEN_Initialize(
    IN  VIR_Shader            *Shader,
    IN  VSC_HW_CONFIG         *HwCfg,
    IN  VSC_OPTN_MCGenOptions *Options,
    IN  VIR_Dumper            *Dumper,
    OUT VSC_MCGen             *Gen
    )
{
    gctINT              maxInstId   = 0;
    gcePATCH_ID         patchID = gcvPATCH_INVALID;

    Gen->Shader  = Shader;
    Gen->HwCfg   = HwCfg;
    Gen->Options = Options;
    Gen->Dumper  = Dumper;

    vscMC_BeginCodec(&Gen->MCCodec, Gen->HwCfg, VIR_Shader_isDual16Mode(Shader));

    vscPMP_Intialize(&Gen->PMP, gcvNULL, 1024,
                     sizeof(void*), gcvTRUE);

    maxInstId = VIR_Shader_RenumberInstId(Gen->Shader);

    if (maxInstId > 0)
    {
        gctINT i = 0;
        Gen->InstMark = (VSC_MC_InstMask *)vscMM_Alloc(&Gen->PMP.mmWrapper,
            sizeof(VSC_MC_InstMask) * maxInstId);

        for (i = 0; i < maxInstId; ++i)
        {
            Gen->InstMark[i].Label = -1;
            Gen->InstMark[i].Inst  = gcvNULL;
        }
    }

    Gen->InstCount = 0;

    gcoHAL_GetPatchID(gcvNULL, &patchID);

    /* dEQP, CTS and WEBGL requires RTNE (for precision purpose) */
    if (gcdPROC_IS_WEBGL(patchID) ||
        patchID == gcvPATCH_DEQP  ||
        patchID == gcvPATCH_OESCTS)
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
    Gen->HwCfg    = gcvNULL;
    Gen->InstMark = gcvNULL;
    vscMC_EndCodec(&Gen->MCCodec);
    vscPMP_Finalize(&Gen->PMP);

    return VSC_ERR_NONE;
}

static gctBOOL
_VSC_MC_GEN_IsTypeEqualTo(
    IN VIR_Operand *Opnd,
    IN VIR_TyFlag   TyFlag
)
{
    VIR_TypeId   ty   = VIR_Operand_GetType(Opnd);
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
    case VIR_OP_AQ_IMADHI0:
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
    case VIR_OP_AQ_IMADHI1:
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
    case VIR_OP_AQ_IMADLO0:
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
    case VIR_OP_AQ_IMADLO1:
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
    case VIR_OP_MOD:
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
        *BaseOpcode = (VIR_Inst_Store_Have_Dst(Inst) && Gen->HwCfg->hwFeatureFlags.hasHalti5) ?
                      MC_AUXILIARY_OP_CODE_USC_STORE :
                      0x33;
        break;
    case VIR_OP_LOAD:
    case VIR_OP_LOAD_S:
        *BaseOpcode = 0x32;
        break;
    case VIR_OP_KILL:
        *BaseOpcode = 0x17;
        break;
    case VIR_OP_BITINSERT:
        *BaseOpcode = 0x54;
        break;
    case VIR_OP_BITINSERT1:
        *BaseOpcode = 0x55;
        break;
    case VIR_OP_BARRIER:
        if (VIR_Shader_GetKind(Gen->Shader) == VIR_SHADER_TESSELLATION_CONTROL &&
            Gen->HwCfg->maxCoreCount >= 8)
        {
            *BaseOpcode = 0x00;
        }
        else
        {
            *BaseOpcode = 0x2A;
        }
        break;
    case VIR_OP_MEM_BARRIER:
        *BaseOpcode = 0x00; /* HW has logic to naturally insure memory access is in-order */
        break;
    case VIR_OP_ATOMADD:
    case VIR_OP_ATOMADD_S:
        *BaseOpcode = 0x65;
        break;
    case VIR_OP_ATOMSUB:
        *BaseOpcode = 0x65;    /* NOTE!!! */
        break;
    case VIR_OP_ATOMXCHG:
        *BaseOpcode = 0x66;
        break;
    case VIR_OP_ATOMCMPXCHG:
        *BaseOpcode = 0x67;
        break;
    case VIR_OP_ATOMMIN:
        *BaseOpcode = 0x68;
        break;
    case VIR_OP_ATOMMAX:
        *BaseOpcode = 0x69;
        break;
    case VIR_OP_ATOMOR:
        *BaseOpcode = 0x6A;
        break;
    case VIR_OP_ATOMAND:
        *BaseOpcode = 0x6B;
        break;
    case VIR_OP_ATOMXOR:
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
    case VIR_OP_AQ_CONV:
        *BaseOpcode = 0x72;
        break;
    case VIR_OP_AQ_F2I:
        *BaseOpcode = 0x2E;
        break;
    case VIR_OP_AQ_I2I:
        *BaseOpcode = 0x2C;
        break;
    case VIR_OP_AQ_I2F:
        *BaseOpcode = 0x2D;
        break;
    case VIR_OP_AQ_CMP:
        *BaseOpcode = 0x31;
        break;
    case VIR_OP_AQ_F2IRND:
        *BaseOpcode = 0x2F;
        break;
    case VIR_OP_AQ_SELECT:
        *BaseOpcode = 0x0F;
        break;
    case VIR_OP_AQ_SET:
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
        *BaseOpcode = (VIR_Inst_Store_Have_Dst(Inst) && Gen->HwCfg->hwFeatureFlags.hasHalti5) ?
                      MC_AUXILIARY_OP_CODE_USC_IMG_STORE :
                      0x7A;
        break;
    case VIR_OP_IMG_LOAD_3D:
    case VIR_OP_VX_IMG_LOAD_3D:
        *BaseOpcode = 0x34;
        break;
    case VIR_OP_IMG_STORE_3D:
    case VIR_OP_VX_IMG_STORE_3D:
        *BaseOpcode = (VIR_Inst_Store_Have_Dst(Inst) && Gen->HwCfg->hwFeatureFlags.hasHalti5) ?
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
        *BaseOpcode = 0x78;
        break;
    case VIR_OP_STORE_ATTR:
        *BaseOpcode = (VIR_Inst_Store_Have_Dst(Inst) && Gen->HwCfg->hwFeatureFlags.hasHalti5) ?
                      MC_AUXILIARY_OP_CODE_USC_STORE_ATTR :
                      0x42;
        break;
    case VIR_OP_SELECT_MAP:
        *BaseOpcode = 0x43;
        break;

    /* Below for extended opcodes */
    case VIR_OP_EMIT:
    case VIR_OP_AQ_EMIT:
        *BaseOpcode = 0x7F;
        *ExternOpcode = 0x01;
        break;
    case VIR_OP_RESTART:
    case VIR_OP_AQ_RESTART:
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
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x08;
        break;
    case VIR_OP_VX_DP8X2:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x09;
        break;
    case VIR_OP_VX_DP4X4:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x0A;
        break;
    case VIR_OP_VX_DP2X8:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x0B;
        break;
    case VIR_OP_VX_DP32X1:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x12;
        break;
    case VIR_OP_VX_DP16X2:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x13;
        break;
    case VIR_OP_VX_DP8X4:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x14;
        break;
    case VIR_OP_VX_DP4X8:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x15;
        break;
    case VIR_OP_VX_DP2X16:
        *BaseOpcode   = 0x45;
        *ExternOpcode = 0x16;
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
    gctUINT label = Inst->id_;
    while (label != -1)
    {
        VIR_Instruction *jmpInst   = Gen->InstMark[label].Inst;
        gctUINT          nextLabel = Gen->InstMark[label].Label;
        gctUINT          t = 0;

        Gen->InstMark[label].Inst  = gcvNULL;
        Gen->InstMark[label].Label = Label;

        /* Transform */
        _VSC_MC_GEN_GenInst(Gen, VIR_Inst_GetFunction(jmpInst), jmpInst, &t);

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
_VSC_MC_GEN_GenSat(
    IN VIR_Operand *Dest
    )
{
    VIR_Modifier mod  = VIR_Operand_GetModifier(Dest);

    switch (mod)
    {
    case VIR_MOD_NONE:
        return 0x0;
    case VIR_MOD_SAT_0_TO_1:
        return 0x1;
    case VIR_MOD_SAT_0_TO_INF:
    case VIR_MOD_SAT_NINF_TO_1:
    default:
        gcmASSERT(0);
        return 0x0;
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
            VIR_Inst_GetThreadMode(Inst) == VIR_THREAD_D16_DUAL_16)
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
    IN VIR_LayoutQual  imgQual
    )
{
    if ((imgQual == VIR_LAYQUAL_IMAGE_FORMAT_RGBA32F) ||
        (imgQual == VIR_LAYQUAL_IMAGE_FORMAT_R32F)    ||
        (imgQual == VIR_LAYQUAL_IMAGE_FORMAT_RGBA16F) ||
        (imgQual == VIR_LAYQUAL_IMAGE_FORMAT_RGBA8)   ||
        (imgQual == VIR_LAYQUAL_IMAGE_FORMAT_RGBA8_SNORM))
    {
        return 0x0;
    }
    else if ((imgQual == VIR_LAYQUAL_IMAGE_FORMAT_RGBA32I) ||
             (imgQual == VIR_LAYQUAL_IMAGE_FORMAT_R32I)    ||
             (imgQual == VIR_LAYQUAL_IMAGE_FORMAT_RGBA16I) ||
             (imgQual == VIR_LAYQUAL_IMAGE_FORMAT_RGBA8I))
    {
        return 0x2;
    }
    else if ((imgQual == VIR_LAYQUAL_IMAGE_FORMAT_RGBA32UI) ||
             (imgQual == VIR_LAYQUAL_IMAGE_FORMAT_R32UI)    ||
             (imgQual == VIR_LAYQUAL_IMAGE_FORMAT_RGBA16UI) ||
             (imgQual == VIR_LAYQUAL_IMAGE_FORMAT_RGBA8UI))
    {
        return 0x5;
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
    VIR_TypeId      ty           = VIR_Operand_GetType(Opnd);
    VIR_OperandKind opndKind     = VIR_Operand_GetOpKind(Opnd);
    VIR_TypeId      componentTy;
    VIR_LayoutQual  imgQual;

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
                  isSymUniformTreatSamplerAsConst(sym));
        imgQual = VIR_Symbol_GetLayoutQualifier(sym);

        return _VSC_MC_GEN_GetInstTypeFromImgFmt(imgQual);
    }

    if (VIR_OPCODE_isTexLd(opcode))
    {
        /* Only v60 HW supports inst-type for texld related insts */
        if (!Gen->HwCfg->hwFeatureFlags.hasHalti5)
        {
            return 0x0;
        }
    }

    gcmASSERT(ty < VIR_TYPE_PRIMITIVETYPE_COUNT);

    componentTy  = VIR_GetTypeComponentType(ty);

    /* we change the integer/boolen from highp to mediump for dual-t*/
    if (VIR_Shader_isDual16Mode(Gen->Shader) &&
        VIR_Inst_GetThreadMode(Inst) == VIR_THREAD_D16_DUAL_16)
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
    default:
        gcmASSERT(0);
        return 0x0;
    }
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
    case VIR_OP_AQ_CMP:
    case VIR_OP_AQ_SET:
    case VIR_OP_AQ_I2I:
    case VIR_OP_AQ_I2F:
    case VIR_OP_RCP:
    case VIR_OP_IMG_LOAD:
    case VIR_OP_IMG_LOAD_3D:
    case VIR_OP_BITFIND_LSB:
    case VIR_OP_BITFIND_MSB:
    case VIR_OP_MOV:
        return _VSC_MC_GEN_GetInstType(Gen, Inst, VIR_Inst_GetSource(Inst, 0));
    case VIR_OP_CMOV:
    case VIR_OP_AQ_SELECT:
        if (VIR_Inst_GetConditionOp(Inst) == VIR_COP_ALLMSB ||
            VIR_Inst_GetConditionOp(Inst) == VIR_COP_ANYMSB ||
            VIR_Inst_GetConditionOp(Inst) == VIR_COP_SELMSB)
        {
            /* HW limitation: one instruction type to do two things:
               to control comparison and to control implicit conversion (e.g., f32 -> f16).
               We have issue when src0 is integer type, but src1 is float type for dual16.
               Using src0 integer type will lose the implict conversion for source/destination. */
            return _VSC_MC_GEN_GetInstType(Gen, Inst, VIR_Inst_GetSource(Inst, 1));
        }
        else
        {
            return _VSC_MC_GEN_GetInstType(Gen, Inst, VIR_Inst_GetSource(Inst, 0));
        }
    case VIR_OP_STORE:
    case VIR_OP_STORE_ATTR:
    case VIR_OP_IMG_STORE:
    case VIR_OP_VX_IMG_STORE:
    case VIR_OP_IMG_STORE_3D:
        return _VSC_MC_GEN_GetInstType(Gen, Inst, VIR_Inst_GetSource(Inst, 2));
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

    if ((opcode == VIR_OP_ATOMADD)     ||
        (opcode == VIR_OP_ATOMSUB)     ||
        (opcode == VIR_OP_ATOMXCHG)    ||
        (opcode == VIR_OP_ATOMCMPXCHG) ||
        (opcode == VIR_OP_ATOMMIN)     ||
        (opcode == VIR_OP_ATOMMAX)     ||
        (opcode == VIR_OP_ATOMOR)      ||
        (opcode == VIR_OP_ATOMAND)     ||
        (opcode == VIR_OP_ATOMXOR)     ||
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
        (opcode == VIR_OP_STORE)          ||
        (opcode == VIR_OP_IMG_STORE)      ||
        (opcode == VIR_OP_VX_IMG_STORE)   ||
        (opcode == VIR_OP_IMG_STORE_3D)   ||
        (opcode == VIR_OP_VX_IMG_STORE_3D))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static void
_VSC_MC_GEN_GenInstCtrl(
    IN  VSC_MCGen              *Gen,
    IN  VIR_Instruction        *Inst,
    OUT VSC_MC_CODEC_INST_CTRL *McInstCtrl
    )
{
    VIR_OpCode opCode      = VIR_Inst_GetOpcode(Inst);
    gctBOOL    bVisionInst = VIR_OPCODE_isVXOnly(opCode) &&
                            !(opCode == VIR_OP_VX_IMG_LOAD    ||
                              opCode == VIR_OP_VX_IMG_STORE   ||
                              opCode == VIR_OP_VX_IMG_LOAD_3D ||
                              opCode == VIR_OP_VX_IMG_STORE_3D);

    gcmASSERT(McInstCtrl != gcvNULL);

    McInstCtrl->condOpCode              = _VSC_MC_GEN_GenCondition(Inst);
    McInstCtrl->roundingMode            = _VSC_MC_GEN_GenRound(Gen, Inst);
    McInstCtrl->threadType              = _VSC_MC_GEN_GenThreadType(Gen, Inst);
    McInstCtrl->instType                = _VSC_MC_GEN_GenInstType(Gen, Inst);
    McInstCtrl->bSkipForHelperKickoff   = _VSC_MC_GEN_GenSkipHelperFlag(Gen, Inst);
    McInstCtrl->bPacked                 = gcvFALSE;
    McInstCtrl->u.bInfX0ToZero          = (opCode == VIR_OP_MUL_Z);
    McInstCtrl->u.maCtrl.bUnderEvisMode = (opCode == VIR_OP_VX_IMG_LOAD    ||
                                           opCode == VIR_OP_VX_IMG_STORE   ||
                                           opCode == VIR_OP_VX_IMG_LOAD_3D ||
                                           opCode == VIR_OP_VX_IMG_STORE_3D);

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
    VIR_OpCode  opcode      = VIR_Inst_GetOpcode(Inst);

    switch(VIR_Symbol_GetKind(sym))
    {
    case VIR_SYM_IMAGE:
        {
            VIR_Uniform *image = VIR_Symbol_GetImage(sym);
            uniformSwizzle = image->swizzle;
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

    /* TODO: Need to be refined */
    if (!VIR_OPCODE_isUnComponentWised(opcode))
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
            gcmASSERT(VIR_Inst_GetOpcode(Inst) == VIR_OP_STORE ||
                      VIR_Inst_GetOpcode(Inst) == VIR_OP_STORE_ATTR ||
                      VIR_Inst_GetOpcode(Inst) == VIR_OP_IMG_STORE ||
                      VIR_Inst_GetOpcode(Inst) == VIR_OP_VX_IMG_STORE ||
                      VIR_Inst_GetOpcode(Inst) == VIR_OP_IMG_STORE_3D);
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
                gcmASSERT(!Gen->HwCfg->hwFeatureFlags.vtxInstanceIdAsAttr);

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
            else if (regNo == VIR_SR_A0)
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
_VSC_MC_GEN_GenImmType(
    IN VSC_MCGen        *Gen,
    IN VIR_Instruction  *Inst,
    IN VIR_Operand      *Opnd
    )
{
    VIR_TypeId ty           = VIR_Operand_GetType(Opnd);
    VIR_TypeId componentTy  = VIR_GetTypeComponentType(ty);

    if (VIR_Shader_isDual16Mode(Gen->Shader))
    {
        gcmASSERT(VIR_Opnd_ValueFit16Bits(Opnd));

        /* float is put into uniform already */
        gcmASSERT(componentTy != VIR_TYPE_FLOAT32);

         if ((VIR_Inst_GetOpcode(Inst) == VIR_OP_JMP ||
             VIR_Inst_GetOpcode(Inst) == VIR_OP_JMPC ||
             VIR_Inst_GetOpcode(Inst) == VIR_OP_JMP_ANY) &&
             VIR_Inst_GetSourceIndex(Inst, Opnd) == 2)
        {
            return 0x2;
        }

        if (VIR_Inst_GetThreadMode(Inst) == VIR_THREAD_D16_DUAL_16 &&
           ((VIR_Inst_GetOpcode(Inst) != VIR_OP_JMP &&
             VIR_Inst_GetOpcode(Inst) != VIR_OP_JMPC &&
             VIR_Inst_GetOpcode(Inst) != VIR_OP_JMP_ANY) ||
             VIR_Inst_GetSourceIndex(Inst, Opnd) != 2))
        {
            return 0x3;
        }

        /* other cases fall through */
    }

    switch (componentTy)
    {
    case VIR_TYPE_FLOAT32:
    case VIR_TYPE_FLOAT16:
        return 0x0;
    case VIR_TYPE_INT32:
    case VIR_TYPE_INT16:
    case VIR_TYPE_INT8:
    case VIR_TYPE_BOOLEAN:
        return 0x1;
    case VIR_TYPE_UINT32:
    case VIR_TYPE_UINT16:
    case VIR_TYPE_UINT8:
        return 0x2;
    default:
        gcmASSERT(0);
        return 0x0;
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
    case VIR_SYM_IMAGE:
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
                } else if (!Gen->HwCfg->hwFeatureFlags.vtxInstanceIdAsAttr) {
                    /* TODO: Need check correct chip and revision to dynamically select when
                    it is clear vertexid/instanceid has been moved into extended type */
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
    IN VIR_Operand *Opnd
    )
{
    VIR_Indexed indexed = VIR_Operand_GetRelAddrMode(Opnd);

    switch(indexed)
    {
    case VIR_INDEXED_NONE:
        return 0x0;
    case VIR_INDEXED_X:
        return 0x1;
    case VIR_INDEXED_Y:
        return 0x2;
    case VIR_INDEXED_Z:
        return 0x3;
    case VIR_INDEXED_W:
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
    *bWrite  = gcvTRUE;

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
            Src->u.imm.immData.ui  = Opnd->u1.uConst;
            Src->u.imm.immType     = _VSC_MC_GEN_GenImmType(Gen, Inst, Opnd);

            break;
        }
    case VIR_OPND_SAMPLER_INDEXING:
        {
            VIR_Symbol  *sym        = VIR_Operand_GetSymbol(Opnd);
            gctUINT      index      = VIR_Operand_GetMatrixConstIndex(Opnd) +
                                        _VSC_MC_GEN_GenOpndRelIndexing(Gen, Inst, Opnd);
            gctUINT      swizzle    = _VSC_MC_GEN_GenOpndSwizzle(Gen, Inst, Opnd);
            gctUINT      indexed    = _VSC_MC_GEN_GenIndexed(Opnd);
            gctUINT      srcKind    = _VSC_MC_GEN_GenSrcType(Gen, sym);

            Src->regType            = srcKind;
            Src->u.reg.regNo        = index;
            Src->u.reg.swizzle      = swizzle;
            Src->u.reg.indexingAddr = indexed;
            Src->u.reg.bNegative    = (VIR_Operand_GetModifier(Opnd) & VIR_MOD_NEG) != 0;
            Src->u.reg.bAbs         = (VIR_Operand_GetModifier(Opnd) & VIR_MOD_ABS) != 0;

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
            gctUINT      index      = _VSC_MC_GEN_GenRegisterNo(Gen, Inst, sym, Opnd) +
                (VIR_Operand_GetMatrixConstIndex(Opnd) +
                 _VSC_MC_GEN_GenOpndRelIndexing(Gen, Inst, Opnd)) *
                 VIR_Symbol_GetRegSize(Gen->Shader, Gen->HwCfg, sym);
            gctUINT      swizzle    = _VSC_MC_GEN_GenOpndSwizzle(Gen, Inst, Opnd);
            gctUINT      indexed    = _VSC_MC_GEN_GenIndexed(Opnd);

            Src->regType            = srcKind;
            Src->u.reg.regNo        = index;
            Src->u.reg.swizzle      = swizzle;
            Src->u.reg.indexingAddr = indexed;
            Src->u.reg.bNegative    = (VIR_Operand_GetModifier(Opnd) & VIR_MOD_NEG) != 0;
            Src->u.reg.bAbs         = (VIR_Operand_GetModifier(Opnd) & VIR_MOD_ABS) != 0;

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

    gcmASSERT(Gen->Shader != gcvNULL &&
        Inst != gcvNULL);

    if(Opnd == gcvNULL)
    {
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
            gctBOOL      bSat         = _VSC_MC_GEN_GenSat(Opnd);
            gctUINT      dstKind      = _VSC_MC_GEN_GenDstType(Gen, sym);
            gctUINT      index        = _VSC_MC_GEN_GenRegisterNo(Gen, Inst, sym, Opnd) +
                (VIR_Operand_GetMatrixConstIndex(Opnd) +
                 _VSC_MC_GEN_GenOpndRelIndexing(Gen, Inst, Opnd)) *
                 VIR_Symbol_GetRegSize(Gen->Shader, Gen->HwCfg, sym);
            gctUINT      enable       = _VSC_MC_GEN_GenOpndEnable(Gen, Inst, Opnd);
            gctUINT      indexed      = _VSC_MC_GEN_GenIndexed(Opnd);

            Dst->regType              = dstKind;
            Dst->regNo                = index;
            Dst->bSaturated           = bSat;

            if (VIR_OPCODE_isVXOnly(VIR_Inst_GetOpcode(Inst)))
            {
                VIR_OpCode opCode      = VIR_Inst_GetOpcode(Inst);
                if (opCode == VIR_OP_VX_IMG_LOAD    ||
                    opCode == VIR_OP_VX_IMG_STORE   ||
                    opCode == VIR_OP_VX_IMG_LOAD_3D ||
                    opCode == VIR_OP_VX_IMG_STORE_3D)
                {
                    VIR_Operand * opnd = (opCode == VIR_OP_VX_IMG_LOAD ||
                                          opCode == VIR_OP_VX_IMG_LOAD_3D) ? VIR_Inst_GetDest(Inst)
                                                                           : VIR_Inst_GetSource(Inst, 2);
                    VIR_TypeId srcTyId = VIR_Operand_GetType(opnd);

                    Dst->u.evisDst.startCompIdx  = 0;
                    Dst->u.evisDst.compIdxRange  = VIR_GetTypeComponents(srcTyId);
                }
                else
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
            }
            else
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

        _VSC_MC_GEN_GenSource(
            Gen,
            Inst,
            VIR_Inst_GetSource(Inst, i),
            Src + *srcNum,
            &bWrite);

        if(bWrite)
        {
            ++(*srcNum);
        }
    }
    return VSC_ERR_NONE;
}

static void _NegMcSrc(VSC_MC_CODEC_SRC *McSrc)
{
    if (McSrc->regType == 0x7)
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
                        VSC_MC_CODEC_INST_CTRL instCtrl,
                        VSC_MC_CODEC_DST*      pDst,
                        VSC_MC_CODEC_SRC*      pSrc,
                        gctUINT                srcCount)
{
    gctUINT channel;

    /* FB_INSERT_MOV_INPUT: insert MOV Rn, Rn for input to help HW team to debug */
    if (!gctOPT_hasFeature(FB_INSERT_MOV_INPUT) &&
        baseOpcode == 0x09 &&
        pSrc[srcCount - 1].regType == 0x0 &&
        pSrc[srcCount - 1].u.reg.regNo == pDst->regNo &&
        pDst->u.nmlDst.indexingAddr == pSrc[srcCount - 1].u.reg.indexingAddr &&
        !pSrc[srcCount - 1].u.reg.bAbs &&
        !pSrc[srcCount - 1].u.reg.bNegative &&
        !pDst->bSaturated &&
        instCtrl.condOpCode == 0x00 &&
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

    return gcvTRUE;
}

static VSC_ErrCode
_VSC_MC_GEN_GenInst(
    IN VSC_MCGen       *Gen,
    IN VIR_Function    *Func,
    IN VIR_Instruction *Inst,
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
                                              Gen->Shader->functions.info.count > 1 &&
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
            mcDest.bSaturated = gcvTRUE;

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

            _NegMcSrc(&mcSrc[0]);

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
                    VIR_Inst_GetThreadMode(Inst) == VIR_THREAD_D16_DUAL_16)
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

            _NegMcSrc(&mcSrc[2]);

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

            _NegMcSrc(&mcSrc[1]);

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
    case VIR_OP_AQ_EMIT:
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
            mcInstCtrl.u.bNeedRestartPrim = mcSrc[1].u.imm.immData.ui & 0x01;
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
            mcSrc->u.imm.immType     = _VSC_MC_GEN_GenImmType(Gen, Inst, indexSrc);
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
                Inst,
                indexSrc,
                mcSrc + 1,
                &bDstWrite);

            srcNum = 2;
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

    /* Because VIR's src seq is different with HW MC's src seq, so we need adjust
       it before sending it to mc-codec */
    if (baseOpcode == 0x7A ||
        baseOpcode == 0x35 ||
        baseOpcode == MC_AUXILIARY_OP_CODE_USC_IMG_STORE ||
        baseOpcode == MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D)
    {
        VSC_MC_CODEC_SRC mcSrcTemp;

        mcSrcTemp = mcSrc[0];

        mcSrc[0] = mcSrc[1];
        mcSrc[1] = mcSrc[2];
        mcSrc[2] = mcSrcTemp;
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
        if (!_NeedGen(baseOpcode,
                      externOpcode,
                      mcInstCtrl,
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

    if (Inst->mcInst != gcvNULL)
    {
        vscMM_Free(&Gen->Shader->mempool, Inst->mcInst);
    }

    Inst->mcInstCount = *GenCount;
    Inst->mcInst = (VSC_MC_RAW_INST *)vscMM_Alloc(&Gen->Shader->mempool,
                                                  sizeof(VSC_MC_RAW_INST) * Inst->mcInstCount);
    memset(Inst->mcInst, 0, sizeof(VSC_MC_RAW_INST) * Inst->mcInstCount);

    if (bNeedGen)
    {
        if (baseOpcode == 0x17)
        {
            Gen->Shader->psHasDiscard = gcvTRUE;
        }

        if (IS_NORMAL_LOAD_STORE_MC_OPCODE(baseOpcode))
        {
            Gen->Shader->hasLoadStore = gcvTRUE;
        }

        if (IS_IMG_LOAD_STORE_MC_OPCODE(baseOpcode))
        {
            Gen->Shader->hasImgReadWrite = gcvTRUE;
        }

        if (IS_ATOMIC_MC_OPCODE(baseOpcode))
        {
            Gen->Shader->hasAtomicOp = gcvTRUE;
        }

        Gen->Shader->hasMemoryAccess = (Gen->Shader->hasLoadStore    |
                                        Gen->Shader->hasImgReadWrite |
                                        Gen->Shader->hasAtomicOp);

        if (IS_BARRIER_MC_OPCODE(baseOpcode))
        {
            gcmASSERT(Gen->Shader->shaderKind == VIR_SHADER_CL ||
                      Gen->Shader->shaderKind == VIR_SHADER_COMPUTE);

            Gen->Shader->hasBarrier = gcvTRUE;
        }

        if (baseOpcode == 0x08)
        {
            Gen->Shader->hasDsy = gcvTRUE;
        }

        vscMC_EncodeInstDirect(&Gen->MCCodec,
                               baseOpcode,
                               externOpcode,
                               mcInstCtrl,
                               bDstWrite ? &mcDest : gcvNULL,
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
        _VSC_MC_GEN_GenInst(Gen, Func, inst, &genCount);
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

VSC_ErrCode
VSC_MC_GEN_MachineCodeGen(
    IN  VIR_Shader            *Shader,
    IN  VSC_HW_CONFIG         *HwCfg,
    IN  VSC_OPTN_MCGenOptions *Options,
    IN  VIR_Dumper            *Dumper
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_MCGen   gen;

    _VSC_MC_GEN_Initialize(Shader, HwCfg, Options, Dumper, &gen);
    errCode = _VSC_MC_GEN_PerformOnShader(&gen);
    _VSC_MC_GEN_Finalize(&gen);

    return errCode;
}


