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

/* If you want to get old-style dump that all operands are printed startting from dst
   location whatever dst is enabled or not, and all operands are not aligned, you can
   enable this macro */
#define SQUEEZE_OPERANDS_DUMP    0

#define INST_ORDINAL_COLUMN_SIZE 6
#define OPCODE_SEC_COLUMN_SIZE   28
#define OPERAND_SEC_COLUMN_SIZE  9
#define DST_START_COLUMN         (INST_ORDINAL_COLUMN_SIZE + OPCODE_SEC_COLUMN_SIZE)
#define SRC_START_COLUMN(srcIdx) (INST_ORDINAL_COLUMN_SIZE + OPCODE_SEC_COLUMN_SIZE + (1 + (srcIdx)) * OPERAND_SEC_COLUMN_SIZE)
#define HEX_MC_START_COLUMN      (INST_ORDINAL_COLUMN_SIZE + OPCODE_SEC_COLUMN_SIZE + 5 * OPERAND_SEC_COLUMN_SIZE)

#define APPEND_PADDING_TO_ALIGN(alignColumnSize)              \
    while (pDumper->curOffset < alignColumnSize)              \
    {                                                         \
        vscDumper_PrintStrSafe(pDumper, " ");                 \
    }

typedef enum _DST_ADDR_REG_TYPE
{
    DST_ADDR_REG_TYPE_NONE,
    DST_ADDR_REG_TYPE_A0,
    DST_ADDR_REG_TYPE_B0,
}DST_ADDR_REG_TYPE;

static void _DumpOpcode(gctUINT baseOpcode, gctUINT extOpcode, VSC_DUMPER* pDumper)
{
    static const char * _strBaseOpcode[] =
    {
        "nop", /* 0x00 */
        "add", /* 0x01 */
        "mad", /* 0x02 */
        "mul", /* 0x03 */
        "dst", /* 0x04 */
        "dp3", /* 0x05 */
        "dp4", /* 0x06 */
        "dsx", /* 0x07 */
        "dsy", /* 0x08 */
        "mov", /* 0x09 */
        "movar", /* 0x0a */
        "movaf", /* 0x0b */
        "rcp", /* 0x0c */
        "rsq", /* 0x0d */
        "litp", /* 0x0e */
        "select", /* 0x0f */
        "set", /* 0x10 */
        "exp", /* 0x11 */
        "log", /* 0x12 */
        "frc", /* 0x13 */
        "call", /* 0x14 */
        "ret", /* 0x15 */
        "branch", /* 0x16 */
        "texkill", /* 0x17 */
        "texld", /* 0x18 */
        "texldb", /* 0x19 */
        "texldd", /* 0x1a */
        "texldl", /* 0x1b */
        "texldpcf", /* 0x1c */
        "rep", /* 0x1d */
        "endrep", /* 0x1e */
        "loop", /* 0x1f */
        "endloop", /* 0x20 */
        "sqrt", /* 0x21 */
        "sin", /* 0x22 */
        "cos", /* 0x23 */
        "branch_any", /* 0x24 */
        "floor", /* 0x25 */
        "ceil", /* 0x26 */
        "sign", /* 0x27 */
        "addlo", /* 0x28 */
        "mullo", /* 0x29 */
        "barrier", /* 0x2a */
        "swizzle", /* 0x2b */
        "i2i", /* 0x2c */
        "i2f", /* 0x2d */
        "f2i", /* 0x2e */
        "f2irnd", /* 0x2f */
        "fma", /* 0x30 */
        "cmp", /* 0x31 */
        "load", /* 0x32 */
        "store", /* 0x33 */
        "img_load_3d", /* 0x34 */
        "img_store_3d", /* 0x35 */
        "clamp0_max", /* 0x36 */
        "img_addr", /* 0x37 */
        "img_addr_3d", /* 0x38 */
        "loadp", /* 0x39 */
        "storep", /* 0x3a */
        "iaddsat", /* 0x3b */
        "imullo0", /* 0x3c */
        "imullo1", /* 0x3d */
        "imullosat0", /* 0x3e */
        "imullosat1", /* 0x3f */
        "imulhi0", /* 0x40 */
        "imulhi1", /* 0x41 */
        "store_attr", /* 0x42 */
        "select_map", /* 0x43 */
        "idiv0", /* 0x44 */
        "evis", /* 0x45 */
        "img_atom", /* 0x46 */
        "_reserved", /* 0x47 */
        "imod0", /* 0x48 */
        "texld_u_s_l", /* 0x49 */
        "texld_u_u_l", /* 0x4a */
        "texld_u_f_l", /* 0x4b */
        "imadlo0", /* 0x4c */
        "imadlo1", /* 0x4d */
        "imadlosat0", /* 0x4e */
        "imadlosat1", /* 0x4f */
        "imadhi0", /* 0x50 */
        "imadhi1", /* 0x51 */
        "imadhisat0", /* 0x52 */
        "imadhisat1", /* 0x53 */
        "bit_insert1", /* 0x54 */
        "bit_insert2", /* 0x55 */
        "movai", /* 0x56 */
        "iabs", /* 0x57 */
        "leadzero", /* 0x58 */
        "lshift", /* 0x59 */
        "rshift", /* 0x5a */
        "rotate", /* 0x5b */
        "or", /* 0x5c */
        "and", /* 0x5d */
        "xor", /* 0x5e */
        "not", /* 0x5f */
        "bit_extract", /* 0x60 */
        "popcount", /* 0x61 */
        "_cmplx", /* 0x62 */
        "arc_trig", /* 0x63 */
        "div", /* 0x64 */
        "atom_add", /* 0x65 */
        "atom_xchg", /* 0x66 */
        "atom_cmp_xchg", /* 0x67 */
        "atom_min", /* 0x68 */
        "atom_max", /* 0x69 */
        "atom_or", /* 0x6a */
        "atom_and", /* 0x6b */
        "atom_xor", /* 0x6c */
        "bit_rev", /* 0x6d */
        "byte_rev", /* 0x6e */
        "texldl", /* 0x6f */
        "texldgpcf", /* 0x70 */
        "pack", /* 0x71 */
        "conv", /* 0x72 */
        "dp2", /* 0x73 */
        "norm_dp2", /* 0x74 */
        "norm_dp3", /* 0x75 */
        "norm_dp4", /* 0x76 */
        "norm_mul", /* 0x77 */
        "load_attr", /* 0x78 */
        "img_load", /* 0x79 */
        "img_store", /* 0x7a */
        "texld_u", /* 0x7b */
        "lodqg", /* 0x7c */
        "texld_gather", /* 0x7d */
        "_reserved", /* 0x7e */
        "_extended", /* 0x7f */
    };
    static gctUINT baseOpCodeCount = sizeof(_strBaseOpcode) / sizeof(char *);

    static const char * _strNonVisionExtOpcode[] =
    {
        "&%^$#@(*",
        "emit", /* 0x01 */
        "restart", /* 0x02 */
        "flush", /* 0x03 */
        "lodq", /* 0x04 */
        "_reserved", /* 0x05 */
        "_reserved", /* 0x06 */
        "_reserved", /* 0x07 */
        "_reserved", /* 0x08 */
        "_reserved", /* 0x09 */
        "_reserved", /* 0x0a */
        "bit_findlsb", /* 0x0b */
        "bit_findmsb", /* 0x0c */
        "texld_fetchMS", /* 0x0d */
        "texld_fetchMS_u", /* 0x0e */
        "halfmix", /* 0x0f */
        "ldexp", /* 0x10 */
        "_reserved", /* 0x11 */
        "_reserved", /* 0x12 */
        "movbix", /* 0x13 */
    };

    static const char * _strVisionExtOpcode[] =
    {
        "&%^$#@(*",
        "vx_abs_diff",
        "vx_iadd",
        "vx_iacc_sq",
        "vx_lerp",
        "vx_filter",
        "vx_mag_phase",
        "vx_mul_shift",
        "vx_dp16x1",
        "vx_dp8x2",
        "vx_dp4x4",
        "vx_dp2x8",
        "vx_clamp",
        "vx_bi_linear",
        "vx_select_add",
        "vx_atomic_add",
        "vx_bit_extract",
        "vx_bit_replace",
        "vx_dp32x1",
        "vx_dp16x2",
        "vx_dp8x4",
        "vx_dp4x8",
        "vx_dp2x16",
        "vx_index_add",
        "vx_vert_min3",
        "vx_vert_max3",
        "vx_vert_med3",
        "vx_horz_min3",
        "vx_horz_max3",
        "vx_horz_med3",
        "vx_gather",
        "vx_scatter",
        "vx_atomic_s",
    };
    static gctUINT visionExtOpcodeCount = sizeof(_strVisionExtOpcode)/sizeof(char *);

    static const char * _strAuxOpcode[] =
    {
        "texldl",
        "texldl_pcf",
        "texldb",
        "texldb_pcf",
        "texld",
        "texld_pcf",
        "texld_u",
        "texld_u_l",
        "texld_u_b",
        "texld_gather",
        "texld_gather_pcf",
        "texld_u_f_b",
        "texld_u_f_b_bias",
        "_reserved",
        "_reserved",
        "_reserved",
        "_reserved",
        "_reserved",
        "_reserved",
        "_reserved",
        "_reserved",
        "_reserved",
        "_reserved",
        "_reserved",
        "_reserved",
        "_reserved",
        "_reserved",
        "_reserved",
        "_reserved",
        "_reserved",
        "_reserved",
        "_reserved",
        "store",
        "img_store",
        "img_store_3d",
        "store_attr",
        "storep",
        "vx_scatter",
    };

    static const char * _strCmplxSubOpcode[] =
    {
        "cmul", /* 0x00 */
        "cmad", /* 0x01 */
        "cadd"               /* 0x02 */
    };

    if (baseOpcode == 0x7F)
    {
        vscDumper_PrintStrSafe(pDumper, "%s", _strNonVisionExtOpcode[extOpcode]);
    }
    else if (baseOpcode == 0x45)
    {
        if (extOpcode >= MC_AUXILIARY_OP_CODE_TEXLD_LOD)
        {
            vscDumper_PrintStrSafe(pDumper, "%s", _strAuxOpcode[extOpcode - MC_AUXILIARY_OP_CODE_OFFSET]);
        }
        else if (extOpcode < visionExtOpcodeCount)
        {
            vscDumper_PrintStrSafe(pDumper, "%s", _strVisionExtOpcode[extOpcode]);
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
    }
    else if (baseOpcode >= MC_AUXILIARY_OP_CODE_OFFSET)
    {
        vscDumper_PrintStrSafe(pDumper, "%s", _strAuxOpcode[baseOpcode - MC_AUXILIARY_OP_CODE_OFFSET]);
    }
    else if (baseOpcode == 0x62)
    {
        vscDumper_PrintStrSafe(pDumper, "%s", _strCmplxSubOpcode[extOpcode]);
    }
    else
    {
        if (baseOpcode < baseOpCodeCount)
        {
            vscDumper_PrintStrSafe(pDumper, "%s", _strBaseOpcode[baseOpcode]);
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
    }
}

static void _DumpInstCtrl(VSC_MC_CODEC_INST_CTRL* pInstCtrl,
                          gctUINT baseOpcode,
                          gctUINT extOpcode,
                          gctBOOL bDual16ModeEnabled,
                          VSC_DUMPER* pDumper)
{
    static const char * _strThreadType[] =
    {
        "", /* mode TOT1 */
        ".t0", /* mode T0 */
        ".t1"      /* mode T1 */
    };

    static const char * _strCondOp[] =
    {
        "",
        ".gt",
        ".lt",
        ".ge",
        ".le",
        ".eq",
        ".ne",
        ".and",
        ".or",
        ".xor",
        ".not",
        ".nz",
        ".gez",
        ".gz",
        ".lez",
        ".lz",
        ".fin",
        ".inf",
        ".nan",
        ".nml",
        ".anymsb",
        ".allmsb",
        ".selmsb",
        ".uc",
        ".hlp",
        ".nothlp",
    };

    static const char * _strRoundingMode[] =
    {
        "", /* default */
        ".rtz", /* round to zero */
        ".rtne"    /* round to nearest even */
    };

    static const char * _strPackMode[] =
    {
        ".pack",
        ""
    };

    static const char * _strInstType[] =
    {
        ".f32", /* 0x0    0x0 */
        ".f16", /* 0x1    0x1 */
        ".s32", /* 0x2   0x2 */
        ".s16", /* 0x3   0x3 */
        ".s8", /* 0x4    0x4 */
        ".u32", /* 0x5 0x5 */
        ".u16", /* 0x6 0x6 */
        ".u8", /* 0x7  0x7 */
        "", /* 0x8 */
        "", /* 0x9 */
        ".s64", /* 0xA   0xA */
        ".snorm16", /* 0xB    0xB */
        ".snorm8", /* 0xC     0xC */
        ".u64", /* 0xD 0xD */
        ".unorm16", /* 0xE    0xE */
        ".unorm8", /* 0xF     0xF */
    };

    static const char * _strLsAttrClient[] =
    {
        ".VS",
        ".TCS",
        ".TES",
        ".GS"
    };

    static const char * _strLsAttrLayout[] =
    {
        ".ild",
        ".lnr"
    };

    if (bDual16ModeEnabled)
    {
        vscDumper_PrintStrSafe(pDumper, "%s", _strThreadType[pInstCtrl->threadType]);
    }

    vscDumper_PrintStrSafe(pDumper, "%s", _strCondOp[pInstCtrl->condOpCode]);
    vscDumper_PrintStrSafe(pDumper, "%s", _strRoundingMode[pInstCtrl->roundingMode]);
    vscDumper_PrintStrSafe(pDumper, "%s", _strPackMode[pInstCtrl->packMode]);

    if (pInstCtrl->bResultSat)
    {
        vscDumper_PrintStrSafe(pDumper, ".sat");
    }

    if (pInstCtrl->bSkipForHelperKickoff)
    {
        vscDumper_PrintStrSafe(pDumper, ".skpHp");
    }

    if (pInstCtrl->bDenorm)
    {
        vscDumper_PrintStrSafe(pDumper, ".denorm");
    }

    if (pInstCtrl->bEndOfBB)
    {
        vscDumper_PrintStrSafe(pDumper, ".ebb");
    }

    /* set USC Unallocate Bit For Global Memory Load/Store Instructions */
    if (IS_MEM_ACCESS_MC_OPCODE(baseOpcode) &&
        pInstCtrl->u.maCtrl.bUnallocate  )
    {
        vscDumper_PrintStrSafe(pDumper, ".UA");
    }

    /* set Endian Control Bit For Global Memory Access Instructions */
    if ((IS_MEM_ACCESS_MC_OPCODE(baseOpcode) ||IS_ATOMIC_MC_OPCODE(baseOpcode)) &&
        pInstCtrl->u.maCtrl.bBigEndian )
    {
        vscDumper_PrintStrSafe(pDumper, ".BE");
    }
    if (IS_MEM_ACCESS_MC_OPCODE(baseOpcode) ||
        (baseOpcode == 0x7F && extOpcode == 0x13))
    {
        if (pInstCtrl->u.maCtrl.bAccessLocalStorage)
        {
            vscDumper_PrintStrSafe(pDumper, ".local");
        }

        if (pInstCtrl->u.maCtrl.bUnderEvisMode)
        {
            vscDumper_PrintStrSafe(pDumper, ".evis");
        }

        if (IS_NORMAL_LOAD_STORE_MC_OPCODE(baseOpcode) ||
            (baseOpcode == 0x7F && extOpcode == 0x13))
        {
            if (pInstCtrl->u.maCtrl.u.lsCtrl.offsetLeftShift)
            {
                vscDumper_PrintStrSafe(pDumper, ".ls%d", pInstCtrl->u.maCtrl.u.lsCtrl.offsetLeftShift);
            }

            if (pInstCtrl->u.maCtrl.u.lsCtrl.bOffsetX3)
            {
                vscDumper_PrintStrSafe(pDumper, ".x3");
            }
        }

        if (IS_IMG_ATOMIC_MC_OPCODE(baseOpcode))
        {
        }
    }
    else if (baseOpcode == 0x43)
    {
        vscDumper_PrintStrSafe(pDumper, ".rg%d", pInstCtrl->u.smCtrl.rangeToMatch);

        if (pInstCtrl->u.smCtrl.bCompSel)
        {
            vscDumper_PrintStrSafe(pDumper, ".compSel");
        }
    }
    else if (baseOpcode == 0x45)
    {
        vscDumper_PrintStrSafe(pDumper, ".es%3X", pInstCtrl->u.visionCtrl.evisState);
        vscDumper_PrintStrSafe(pDumper, ".srcbin%d", pInstCtrl->u.visionCtrl.startSrcCompIdx);
    }
    else if (baseOpcode == 0x78 ||
             baseOpcode == 0x42)
    {
        if (pInstCtrl->u.lsAttrCtrl.shStageClient)
        {
            vscDumper_PrintStrSafe(pDumper, "%s", _strLsAttrClient[pInstCtrl->u.lsAttrCtrl.shStageClient]);
        }

        if (pInstCtrl->u.lsAttrCtrl.attrLayout)
        {
            vscDumper_PrintStrSafe(pDumper, "%s", _strLsAttrLayout[pInstCtrl->u.lsAttrCtrl.attrLayout]);
        }
    }
    else if (baseOpcode == 0x71)
    {
        if (pInstCtrl->u.srcSelect)
        {
            vscDumper_PrintStrSafe(pDumper, ".ss%d", pInstCtrl->u.srcSelect);
        }
    }
    else if (baseOpcode == 0x7F && extOpcode == 0x01)
    {
        if (pInstCtrl->u.emitCtrl.bNeedRestartPrim)
        {
            vscDumper_PrintStrSafe(pDumper, ".restart");
        }

        if (!pInstCtrl->u.emitCtrl.bNoJmpToEndOnMaxVtxCnt)
        {
            vscDumper_PrintStrSafe(pDumper, ".JmpToEnd");
        }
    }
    else if (baseOpcode == 0x03 ||
             baseOpcode == 0x77 ||
             baseOpcode == 0x29 ||
             baseOpcode == 0x04 ||
             baseOpcode == 0x73 ||
             baseOpcode == 0x05 ||
             baseOpcode == 0x06 ||
             baseOpcode == 0x02 ||
             baseOpcode == 0x74 ||
             baseOpcode == 0x75 ||
             baseOpcode == 0x76)
    {
        if (pInstCtrl->u.bInfX0ToZero)
        {
            vscDumper_PrintStrSafe(pDumper, ".infX0To0");
        }
    }

    if (pInstCtrl->instType == 0x0)
    {
        if (baseOpcode == 0x2C ||
            baseOpcode == 0x72 ||
            baseOpcode == 0x79 ||
            baseOpcode == 0x34 ||
            baseOpcode == 0x0A ||
            baseOpcode == 0x0B ||
            baseOpcode == 0x56 ||
            baseOpcode == 0x33 ||
            baseOpcode == MC_AUXILIARY_OP_CODE_USC_STORE)
        {
            vscDumper_PrintStrSafe(pDumper, "%s", _strInstType[pInstCtrl->instType]);
        }
    }
    else
    {
        vscDumper_PrintStrSafe(pDumper, "%s", _strInstType[pInstCtrl->instType]);
    }
}

static void _DumpInstDst(VSC_MC_CODEC_DST* pDst,
                         gctBOOL bDstValid,
                         DST_ADDR_REG_TYPE addrRegType,
                         gctBOOL bEvisMode,
                         gctBOOL bDual16ModeEnabled,
                         gctBOOL bNeedComma,
                         VSC_DUMPER* pDumper)
{
    char dstRegLetter;

    static const char * _strWriteMask[] =
    {
        ".", ".x", ".y", ".xy",
        ".z", ".xz", ".yz", ".xyz",
        ".w", ".xw", ".yw", ".xyw",
        ".zw", ".xzw", ".yzw", ""
    };

    static const char * _strDynamicIndexing[] =
    {
        "",
        "[a0.x]",
        "[a0.y]",
        "[a0.z]",
        "[a0.w]",
        "[aL]",
        "[InstanceId]",
        "[b0]"
    };

    if (!bDstValid)
    {
        if (bEvisMode)
        {
            vscDumper_PrintStrSafe(pDumper, ".{%d, %d} ",
                                   pDst->u.evisDst.startCompIdx,
                                   pDst->u.evisDst.startCompIdx + pDst->u.evisDst.compIdxRange - 1);
        }
        else if (pDst->u.nmlDst.writeMask)
        {
            vscDumper_PrintStrSafe(pDumper, "%s ", _strWriteMask[pDst->u.nmlDst.writeMask]);
        }

#if SQUEEZE_OPERANDS_DUMP
    /* Append padding. '-1' is because when dumping src, a prefix " " will be added */
    APPEND_PADDING_TO_ALIGN(DST_START_COLUMN - 1);
#endif

        return;
    }

    /* need at least one space between opcode and dst */
    vscDumper_PrintStrSafe(pDumper, " ");
    /* Append padding */
    APPEND_PADDING_TO_ALIGN(DST_START_COLUMN);

    if (addrRegType == DST_ADDR_REG_TYPE_A0)
    {
        dstRegLetter = 'a';
    }
    else if (addrRegType == DST_ADDR_REG_TYPE_B0)
    {
        dstRegLetter = 'b';
    }
    else
    {
        dstRegLetter = 'r';
    }

    vscDumper_PrintStrSafe(pDumper, "%c%u", dstRegLetter, pDst->regNo);

    if (!bEvisMode)
    {
        vscDumper_PrintStrSafe(pDumper, "%s", _strDynamicIndexing[pDst->u.nmlDst.indexingAddr]);
    }

    if (bDual16ModeEnabled && pDst->regType != 0x0)
    {
        vscDumper_PrintStrSafe(pDumper, ".hp");
    }

    if (bEvisMode)
    {
        vscDumper_PrintStrSafe(pDumper, ".{%d, %d}",
                               pDst->u.evisDst.startCompIdx,
                               pDst->u.evisDst.startCompIdx + pDst->u.evisDst.compIdxRange - 1);
    }
    else
    {
        vscDumper_PrintStrSafe(pDumper, "%s", _strWriteMask[pDst->u.nmlDst.writeMask]);
    }

    if (bNeedComma)
    {
        vscDumper_PrintStrSafe(pDumper, ", ");
    }
}

static void _DumpInstSrc(VSC_MC_CODEC_SRC* pSrc,
                         gctUINT srcIdx,
                         gctBOOL bEvisMode,
                         gctBOOL bDual16ModeEnabled,
                         gctBOOL bNeedComma,
                         VSC_DUMPER* pDumper)
{
    static const char * _strSwizzle[] =
    {
        ".x", ".yx", ".zx", ".wx", ".xyx", ".yyx", ".zyx", ".wyx", ".xzx", ".yzx", ".zzx", ".wzx", ".xwx", ".ywx", ".zwx", ".wwx",
        ".xxyx", ".yxyx", ".zxyx", ".wxyx", ".xyyx", ".yyyx", ".zyyx", ".wyyx", ".xzyx", ".yzyx", ".zzyx", ".wzyx", ".xwyx", ".ywyx", ".zwyx", ".wwyx",
        ".xxzx", ".yxzx", ".zxzx", ".wxzx", ".xyzx", ".yyzx", ".zyzx", ".wyzx", ".xzzx", ".yzzx", ".zzzx", ".wzzx", ".xwzx", ".ywzx", ".zwzx", ".wwzx",
        ".xxwx", ".yxwx", ".zxwx", ".wxwx", ".xywx", ".yywx", ".zywx", ".wywx", ".xzwx", ".yzwx", ".zzwx", ".wzwx", ".xwwx", ".ywwx", ".zwwx", ".wwwx",
        ".xxxy", ".yxxy", ".zxxy", ".wxxy", ".xyxy", ".yyxy", ".zyxy", ".wyxy", ".xzxy", ".yzxy", ".zzxy", ".wzxy", ".xwxy", ".ywxy", ".zwxy", ".wwxy",
        ".xxy", ".yxy", ".zxy", ".wxy", ".xy", ".y", ".zy", ".wy", ".xzy", ".yzy", ".zzy", ".wzy", ".xwy", ".ywy", ".zwy", ".wwy",
        ".xxzy", ".yxzy", ".zxzy", ".wxzy", ".xyzy", ".yyzy", ".zyzy", ".wyzy", ".xzzy", ".yzzy", ".zzzy", ".wzzy", ".xwzy", ".ywzy", ".zwzy", ".wwzy",
        ".xxwy", ".yxwy", ".zxwy", ".wxwy", ".xywy", ".yywy", ".zywy", ".wywy", ".xzwy", ".yzwy", ".zzwy", ".wzwy", ".xwwy", ".ywwy", ".zwwy", ".wwwy",
        ".xxxz", ".yxxz", ".zxxz", ".wxxz", ".xyxz", ".yyxz", ".zyxz", ".wyxz", ".xzxz", ".yzxz", ".zzxz", ".wzxz", ".xwxz", ".ywxz", ".zwxz", ".wwxz",
        ".xxyz", ".yxyz", ".zxyz", ".wxyz", ".xyyz", ".yyyz", ".zyyz", ".wyyz", ".xzyz", ".yzyz", ".zzyz", ".wzyz", ".xwyz", ".ywyz", ".zwyz", ".wwyz",
        ".xxz", ".yxz", ".zxz", ".wxz", ".xyz", ".yyz", ".zyz", ".wyz", ".xz", ".yz", ".z", ".wz", ".xwz", ".ywz", ".zwz", ".wwz",
        ".xxwz", ".yxwz", ".zxwz", ".wxwz", ".xywz", ".yywz", ".zywz", ".wywz", ".xzwz", ".yzwz", ".zzwz", ".wzwz", ".xwwz", ".ywwz", ".zwwz", ".wwwz",
        ".xxxw", ".yxxw", ".zxxw", ".wxxw", ".xyxw", ".yyxw", ".zyxw", ".wyxw", ".xzxw", ".yzxw", ".zzxw", ".wzxw", ".xwxw", ".ywxw", ".zwxw", ".wwxw",
        ".xxyw", ".yxyw", ".zxyw", ".wxyw", ".xyyw", ".yyyw", ".zyyw", ".wyyw", ".xzyw", ".yzyw", ".zzyw", ".wzyw", ".xwyw", ".ywyw", ".zwyw", ".wwyw",
        ".xxzw", ".yxzw", ".zxzw", ".wxzw", "", ".yyzw", ".zyzw", ".wyzw", ".xzzw", ".yzzw", ".zzzw", ".wzzw", ".xwzw", ".ywzw", ".zwzw", ".wwzw",
        ".xxw", ".yxw", ".zxw", ".wxw", ".xyw", ".yyw", ".zyw", ".wyw", ".xzw", ".yzw", ".zzw", ".wzw", ".xw", ".yw", ".zw", ".w",
    };

    static const char* _strSrcType[] =
    {
        "r", /* 0x0 or 0x0 */
        "vFace", /* 0x1 or 0x1 (evis) */
        "c", /* 0x2  */
        "c", /* 0x3    */
        "r", /* 0x4  or 0x4 (evis) */
        "Ext", /* 0x5         */
        "L", /* 0x6            */
        "I"           /* 0x7        */
    };

    static const char* _strInstIndex[] =
    {
        "instanceId", /* 0x00         */
        "in_primitiveId", /* 0x01  */
        "out_primitiveId", /* 0x02 */
        "iu", /* 0x03         */
        "is", /* 0x04         */
        "cash_id", /* 0x05            */
        "remap", /* 0x06               */
        "%#@!$", /* Reserved                                       */
        "remap0", /* 0x08              */
        "remap1", /* 0x09              */
        "remap2", /* 0x0A              */
        "remap3", /* 0x0B              */
        "remap4", /* 0x0C              */
        "remap5", /* 0x0D              */
        "next_pc", /* 0x0E         */
        "vertexid", /* 0x0F           */
        "sampleid", /* 0x10           */
        "samplepos", /* 0x11     */
        "in_samplemask", /* 0x12      */
    };

    static const char * _strDynamicIndexing[] =
    {
        "",
        "[a0.x]",
        "[a0.y]",
        "[a0.z]",
        "[a0.w]",
        "[aL]",
        "[InstanceId]",
        "[b0]"
    };

#if SQUEEZE_OPERANDS_DUMP
    vscDumper_PrintStrSafe(pDumper, " ");
#else
    /* Append padding */
    APPEND_PADDING_TO_ALIGN(SRC_START_COLUMN(srcIdx));
#endif

    if (pSrc->regType == 0x7)
    {
        switch (pSrc->u.imm.immType)
        {
        case 0x0:
            vscDumper_PrintStrSafe(pDumper, "%f", pSrc->u.imm.immData.f);
            break;

        case 0x1:
            vscDumper_PrintStrSafe(pDumper, "%d", pSrc->u.imm.immData.si);;
            break;

        case 0x2:
            vscDumper_PrintStrSafe(pDumper, "%u", pSrc->u.imm.immData.ui);
            break;

        case 0x3:
            vscDumper_PrintStrSafe(pDumper, "0x%X", pSrc->u.imm.immData.ui);
            break;
        }

        goto _COMMA;
    }

    if (pSrc->u.reg.bNegative)
    {
        vscDumper_PrintStrSafe(pDumper, "-");
    }

    if (pSrc->u.reg.bAbs)
    {
        vscDumper_PrintStrSafe(pDumper, "|");
    }

    /* A temp solution because it is not clear vertexid/instanceid has been moved into extended type */
    if ((pSrc->u.reg.regNo == 0x01 ||
         pSrc->u.reg.regNo == 0x02) &&
        pSrc->regType == 0x5)
    {
        vscDumper_PrintStrSafe(pDumper, "%s", _strInstIndex[pSrc->u.reg.regNo]);
    }
    else if ((pSrc->u.reg.regNo >= 0x10 &&
              pSrc->u.reg.regNo <= 0x12) &&
             pSrc->regType == 0x5)
    {
        vscDumper_PrintStrSafe(pDumper, "%s", _strInstIndex[pSrc->u.reg.regNo]);
        vscDumper_PrintStrSafe(pDumper, "%s", _strSwizzle[pSrc->u.reg.swizzle]);
    }
    else
    {
        if ((pSrc->regType == 0x1 ||
             pSrc->regType == 0x4) &&
            bEvisMode)
        {
            if (pSrc->regType == 0x1)
            {
                vscDumper_PrintStrSafe(pDumper, "r%u-%u", pSrc->u.reg.regNo, pSrc->u.reg.regNo + 1);
            }
            else if (pSrc->regType == 0x4)
            {
                vscDumper_PrintStrSafe(pDumper, "c%u-%u", pSrc->u.reg.regNo, pSrc->u.reg.regNo + 3);
            }
            else
            {
                gcmASSERT(0);
            }
        }
        else
        {
            if (pSrc->regType == MC_AUXILIARY_SRC_TYPE_SAMPLER)
            {
                vscDumper_PrintStrSafe(pDumper, "s%u", pSrc->u.reg.regNo);
            }
            else
            {
                vscDumper_PrintStrSafe(pDumper, "%s%u", _strSrcType[pSrc->regType], pSrc->u.reg.regNo);
            }
        }

        vscDumper_PrintStrSafe(pDumper, "%s", _strDynamicIndexing[pSrc->u.reg.indexingAddr]);

        if (bDual16ModeEnabled && pSrc->regType != 0x0)
        {
            vscDumper_PrintStrSafe(pDumper, ".hp");
        }

        vscDumper_PrintStrSafe(pDumper, "%s", _strSwizzle[pSrc->u.reg.swizzle]);
    }

    if (pSrc->u.reg.bAbs)
    {
        vscDumper_PrintStrSafe(pDumper, "|");
    }

_COMMA:
    if (bNeedComma)
    {
        vscDumper_PrintStrSafe(pDumper, ", ");
    }
}

static void _DumpCodecHelperInst(VSC_MC_CODEC_INST* pCodecHelperInst,
                                 VSC_MC_RAW_INST* pMcInst,
                                 gctBOOL bDual16ModeEnabled,
                                 gctUINT instIdx,
                                 VSC_DUMPER* pDumper)
{
    DST_ADDR_REG_TYPE addrRegType = DST_ADDR_REG_TYPE_NONE;
    gctUINT           srcIdx;
    gctBOOL           bEvisMode = ((pCodecHelperInst->baseOpcode == 0x45) ||
                                  (IS_MEM_ACCESS_MC_OPCODE(pCodecHelperInst->baseOpcode) && pCodecHelperInst->instCtrl.u.maCtrl.bUnderEvisMode));

    if (pCodecHelperInst->baseOpcode == 0x0A ||
        pCodecHelperInst->baseOpcode == 0x0B ||
        pCodecHelperInst->baseOpcode == 0x56)
    {
        addrRegType = DST_ADDR_REG_TYPE_A0;
    }
    else if (pCodecHelperInst->baseOpcode == 0x7F &&
        pCodecHelperInst->extOpcode == 0x13)
    {
        addrRegType = DST_ADDR_REG_TYPE_B0;
    }

    /* Print inst ordinal */
    vscDumper_PrintStrSafe(pDumper, "%04u: ", instIdx);

    /* print opcode */
    _DumpOpcode(pCodecHelperInst->baseOpcode, pCodecHelperInst->extOpcode, pDumper);

    /* Print inst control */
    _DumpInstCtrl(&pCodecHelperInst->instCtrl, pCodecHelperInst->baseOpcode,
                  pCodecHelperInst->extOpcode, bDual16ModeEnabled, pDumper);

    /* Print dst */
    _DumpInstDst(&pCodecHelperInst->dst,
                 pCodecHelperInst->bDstValid,
                 addrRegType,
                 bEvisMode,
                 bDual16ModeEnabled,
                 (pCodecHelperInst->srcCount != 0),
                 pDumper);

    /* Print srcs */
    for (srcIdx = 0; srcIdx < pCodecHelperInst->srcCount; srcIdx ++)
    {
        _DumpInstSrc(&pCodecHelperInst->src[srcIdx],
                     srcIdx,
                     bEvisMode,
                     bDual16ModeEnabled,
                     (srcIdx < pCodecHelperInst->srcCount -1),
                     pDumper);
    }

    /* Print raw hex mc inst */
    APPEND_PADDING_TO_ALIGN(HEX_MC_START_COLUMN);
    vscDumper_PrintStrSafe(pDumper,
                           "# 0x%08x 0x%08x 0x%08x 0x%08x",
                           pMcInst->word[0], pMcInst->word[1], pMcInst->word[2], pMcInst->word[3]);

    /* Flush dump buffer to output window */
    vscDumper_DumpBuffer(pDumper);
}

void vscMC_DumpInst(VSC_MC_CODEC* pMcCodec,
                    VSC_MC_RAW_INST* pMcInst,
                    gctUINT instIdx,
                    VSC_DUMPER* pDumper)
{
    VSC_MC_CODEC_INST    codecHelperInst;

    /* Decode firstly */
    if (!vscMC_DecodeInst(pMcCodec, pMcInst, &codecHelperInst))
    {
        gcmASSERT(gcvFALSE);
        return;
    }

    /* Dump decoded inst now */
    _DumpCodecHelperInst(&codecHelperInst, pMcInst, pMcCodec->bDual16ModeEnabled, instIdx, pDumper);
}

void vscMC_DumpInsts(VSC_MC_RAW_INST* pMcInsts,
                     gctUINT countOfMCInst,
                     VSC_HW_CONFIG* pHwCfg,
                     gctBOOL bUnderDual16Mode,
                     VSC_DUMPER* pDumper)

{
    gctUINT        instIdx;
    VSC_MC_CODEC   mcCodec;

    if (!pMcInsts || countOfMCInst == 0)
    {
        return;
    }

    vscMC_BeginCodec(&mcCodec, pHwCfg, bUnderDual16Mode, gcvTRUE);

    for (instIdx = 0; instIdx < countOfMCInst; instIdx ++)
    {
        vscMC_DumpInst(&mcCodec, pMcInsts + instIdx, instIdx, pDumper);
    }

    vscMC_EndCodec(&mcCodec);
}

void vscMC_DisassembleInst(VSC_MC_CODEC* pMcCodec,
                           VSC_MC_RAW_INST* pMcInst,
                           gctUINT instIdx,
                           VSC_DUMPER* pDumper)
{
    VSC_MC_CODEC_INST    codecHelperInst;
    DST_ADDR_REG_TYPE addrRegType = DST_ADDR_REG_TYPE_NONE;
    gctUINT           srcIdx;
    gctBOOL           bEvisMode;

    /* Decode firstly */
    if (!vscMC_DecodeInst(pMcCodec, pMcInst, &codecHelperInst))
    {
        gcmASSERT(gcvFALSE);
        return;
    }

    bEvisMode = ((codecHelperInst.baseOpcode == 0x45) ||
                                  (IS_MEM_ACCESS_MC_OPCODE(codecHelperInst.baseOpcode) && codecHelperInst.instCtrl.u.maCtrl.bUnderEvisMode));

    if (codecHelperInst.baseOpcode == 0x0A ||
        codecHelperInst.baseOpcode == 0x0B ||
        codecHelperInst.baseOpcode == 0x56)
    {
        addrRegType = DST_ADDR_REG_TYPE_A0;
    }
    else if (codecHelperInst.baseOpcode == 0x7F &&
        codecHelperInst.extOpcode == 0x13)
    {
        addrRegType = DST_ADDR_REG_TYPE_B0;
    }

    /* Print inst ordinal */
    vscDumper_PrintStrSafe(pDumper, "%04u: ", instIdx);

    /* print opcode */
    _DumpOpcode(codecHelperInst.baseOpcode, codecHelperInst.extOpcode, pDumper);

    /* Print inst control */
    _DumpInstCtrl(&codecHelperInst.instCtrl, codecHelperInst.baseOpcode,
                  codecHelperInst.extOpcode, pMcCodec->bDual16ModeEnabled, pDumper);

    /* Print dst */
    _DumpInstDst(&codecHelperInst.dst,
                 codecHelperInst.bDstValid,
                 addrRegType,
                 bEvisMode,
                 pMcCodec->bDual16ModeEnabled,
                 (codecHelperInst.srcCount != 0),
                 pDumper);

    /* Print srcs */
    for (srcIdx = 0; srcIdx < codecHelperInst.srcCount; srcIdx ++)
    {
        _DumpInstSrc(&codecHelperInst.src[srcIdx],
                     srcIdx,
                     bEvisMode,
                     pMcCodec->bDual16ModeEnabled,
                     (srcIdx < codecHelperInst.srcCount -1),
                     pDumper);
    }

    /* Print raw hex mc inst */
    APPEND_PADDING_TO_ALIGN(HEX_MC_START_COLUMN);
    vscDumper_PrintStrSafe(pDumper,
                           "# 0x%08x 0x%08x 0x%08x 0x%08x",
                           pMcInst->word[0], pMcInst->word[1], pMcInst->word[2], pMcInst->word[3]);
}

