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


#ifndef __gc_vsc_vir_mc_gen_h_
#define __gc_vsc_vir_mc_gen_h_

BEGIN_EXTERN_C()

#include "chip/gc_vsc_chip_mc_codec.h"

typedef struct _VSC_MC_INST_MASK
{
    gctINT           Label;
    VIR_Instruction *Inst;
} VSC_MC_InstMask;

typedef struct _VSC_MC_GEN
{
    VIR_Shader            *Shader;
    VSC_MC_InstMask       *InstMark;
    gctUINT                InstCount;
    VSC_PRIMARY_MEM_POOL   PMP;
    VSC_HW_CONFIG         *HwCfg;
    VSC_MC_CODEC           MCCodec;
    VIR_Dumper            *Dumper;
    VSC_OPTN_MCGenOptions *Options;

    gctBOOL               RTNERequired;
} VSC_MCGen;

VSC_ErrCode
VSC_MC_GEN_MachineCodeGen(
    IN  VIR_Shader            *Shader,
    IN  VSC_HW_CONFIG         *HwCfg,
    IN  VSC_OPTN_MCGenOptions *Options,
    IN  VIR_Dumper            *Dumper
    );

END_EXTERN_C()

#endif /* __gc_vsc_vir_mc_gen_h_ */


