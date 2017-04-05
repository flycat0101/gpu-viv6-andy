/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vsc_vir_dump_h_
#define __gc_vsc_vir_dump_h_

#include "gc_vsc.h"

#define VIR_DUMP_OPNDIDX       0

struct _VIR_DUMPER
{
    VSC_DUMPER      baseDumper;
    VIR_Shader *    Shader;
    gctBOOL         dumpOperandId;
    gctBOOL         invalidCFG;
};

#define VIR_LOG(pVirDumper, ...)    vscDumper_PrintStrSafe(&(pVirDumper)->baseDumper, __VA_ARGS__)
#define VIR_LOG_FLUSH(pVirDumper)   vscDumper_DumpBuffer(&(pVirDumper)->baseDumper)

VSC_ErrCode
VIR_Symbol_Dump(
    IN OUT VIR_Dumper *  Dumper,
    IN  VIR_Symbol      *Sym,
    IN  gctBOOL          FullType
    );

VSC_ErrCode
VIR_Inst_Dump(
    IN OUT VIR_Dumper   *Dumper,
    IN VIR_Instruction  *Inst
    );

VSC_ErrCode
VIR_BasicBlock_Dump(
    IN OUT VIR_Dumper  *Dumper,
    IN VIR_BASIC_BLOCK *Bb,
    IN gctBOOL          Indent
    );

VSC_ErrCode
VIR_BasicBlock_DumpRange(
    IN OUT VIR_Dumper  *Dumper,
    IN VIR_BASIC_BLOCK *BbStart,
    IN VIR_BASIC_BLOCK *BbEnd,
    IN gctBOOL          Indent
    );

VSC_ErrCode
VIR_CFG_Dump(
    IN OUT VIR_Dumper *Dumper,
    IN VIR_CFG        *Cfg,
    IN gctBOOL        Indent
    );

VSC_ErrCode
VIR_Enable_Dump(
    IN OUT VIR_Dumper *  Dumper,
    IN     VIR_Enable    Enable
    );

VSC_ErrCode
VIR_Swizzle_Dump(
    IN OUT VIR_Dumper *  Dumper,
    IN     VIR_Swizzle   Swizzle
    );

VSC_ErrCode
VIR_Function_Dump(
    IN OUT VIR_Dumper   *Dumper,
    IN VIR_Function     *Func
    );

VSC_ErrCode
VIR_Uniform_Dump(
    IN OUT VIR_Dumper  *Dumper,
    IN     VIR_Uniform *Uniform
    );

VSC_ErrCode
VIR_UniformBlock_Dump(
    IN OUT VIR_Dumper       *Dumper,
    IN     VIR_UniformBlock *UniformBlock
    );

VSC_ErrCode
VIR_StorageBlock_Dump(
    IN OUT VIR_Dumper       *Dumper,
    IN     VIR_StorageBlock *StorageBlock
    );

VSC_ErrCode
VIR_DU_Info_Dump(
    IN VIR_DEF_USAGE_INFO* pDuInfo
);

VSC_ErrCode
VIR_CFG_LIVENESS_Dump(
    IN OUT VIR_Dumper       *Dumper,
    IN VIR_LIVENESS_INFO    *pLvInfo,
    IN VIR_CFG              *Cfg
    );

VSC_ErrCode
VIR_BasicBlock_Name_Dump(
    IN OUT VIR_Dumper *Dumper,
    IN VIR_BB         *Bb
    );

#endif


