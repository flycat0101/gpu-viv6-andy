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


#ifndef __gc_vsc_vir_dce_h_
#define __gc_vsc_vir_dce_h_

/************************ Dead Code Elimination ****************************/

#include "gc_vsc.h"

BEGIN_EXTERN_C()

typedef struct _VSC_DCE_MARK
{
    gctINT8 isAlive;
} VSC_DCE_Mark;

typedef struct _VSC_DCE_BB_INFO
{
    VIR_Instruction         *jmpInst;
    gctBOOL                  calculated;
} VSC_DCE_BBInfo;

typedef struct _VSC_DCE
{
    VIR_Shader          *shader;
    VIR_DEF_USAGE_INFO  *du_info;
    VSC_OPTN_DCEOptions *options;
    VIR_Dumper          *dumper;
    VSC_MM              *pMM;
    VSC_DCE_Mark        *mark;
    VSC_DCE_BBInfo      *BBInfo;
    VSC_BI_LIST          jmpList;
    VSC_SIMPLE_QUEUE     workList;
    gctINT               maxBBCount;
    gctINT               maxInstCount;
    gctINT               optCount;
    gctBOOL              rebuildCFG;
} VSC_DCE;

#define VSC_DCE_GetShader(dce)                  ((dce)->shader)
#define VSC_DCE_SetShader(dce, s)               ((dce)->shader = (s))
#define VSC_DCE_GetDUInfo(dce)                  ((dce)->du_info)
#define VSC_DCE_SetDUInfo(dce, d)               ((dce)->du_info = (d))
#define VSC_DCE_GetOptions(dce)                 ((dce)->options)
#define VSC_DCE_SetOptions(dce, o)              ((dce)->options = (o))
#define VSC_DCE_GetDumper(dce)                  ((dce)->dumper)
#define VSC_DCE_SetDumper(dce, d)               ((dce)->dumper = (d))
#define VSC_DCE_GetMM(dce)                      ((dce)->pMM)
#define VSC_DCE_GetOptCount(dce)                ((dce)->optCount)
#define VSC_DCE_SetOptCount(dce, s)             ((dce)->optCount = (s))
#define VSC_DCE_GetMarkByInst(dce, inst)        (VIR_Inst_GetId(inst) < (dce)->maxInstCount ? VSC_DCE_GetMark(dce)[VIR_Inst_GetId(inst)] : emptyMark)
#define VSC_DCE_SetAliveByInst(dce, inst, alive) (VSC_DCE_GetMark(dce)[VIR_Inst_GetId(inst)].isAlive = (alive))
#define VSC_DCE_GetBBInfoByBB(dce, bb)          ((dce)->BBInfo[(bb)->dgNode.id + (BB_GET_FUNC(bb)->pFuncBlock->dgNode.id) * (dce)->maxBBCount])
#define VSC_DCE_GetMark(dce)                    ((dce)->mark)
#define VSC_DCE_SetMark(dce, i)                 ((dce)->mark = (i))
#define VSC_DCE_GetBBInfo(dce)                  ((dce)->BBInfo)
#define VSC_DCE_SetBBInfo(dce, bb)              ((dce)->BBInfo = (bb))
#define VSC_DCE_GetWorkList(dce)                ((dce)->workList)
#define VSC_DCE_SetWorkList(dce, w)             ((dce)->workList = (w))
#define VSC_DCE_GetJmpList(dce)                 ((dce)->jmpList)
#define VSC_DCE_SetJmpList(dce, j)              ((dce)->jmpList = (j))

extern VSC_ErrCode VSC_DCE_Perform(
    IN VSC_SH_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(VSC_DCE_Perform);

END_EXTERN_C()

#endif /* __gc_vsc_vir_dce_h_ */

