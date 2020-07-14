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


#ifndef __gc_vsc_vir_mc_gen_h_
#define __gc_vsc_vir_mc_gen_h_

BEGIN_EXTERN_C()

typedef struct _VSC_MC_INST_MARK
{
    /*
    ** Label and Inst that use this InstMark.
    ** LabelInst point to this InstMark.
    ** For example, for InstMark[18], the LabelInst is 018, and the Inst is 013.
    **  013: JMPC.lt            26, uint temp(46).x{r7.<2}, uint 32
    **  ....
    **  018: LABEL              26:
    **
    */

    gctINT           Label;
    VIR_Instruction *Inst;
    VIR_Instruction *LabelInst;
} VSC_MC_InstMark;

typedef struct _VSC_MC_GEN
{
    VIR_Shader            *Shader;
    VSC_MC_InstMark       *InstMark;
    gctUINT                InstCount;
    VSC_MM                *pMM;
    VSC_COMPILER_CONFIG   *pComCfg;
    VSC_MC_CODEC           MCCodec;
    VIR_Dumper            *Dumper;
    VSC_OPTN_MCGenOptions *Options;

    gctBOOL               RTNERequired;
} VSC_MCGen;

VSC_ErrCode
VSC_MC_GEN_MachineCodeGen(
    VSC_SH_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(VSC_MC_GEN_MachineCodeGen);
DECLARE_SH_NECESSITY_CHECK(VSC_MC_GEN_MachineCodeGen);

END_EXTERN_C()

#endif /* __gc_vsc_vir_mc_gen_h_ */


