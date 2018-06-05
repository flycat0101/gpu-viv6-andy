/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vsc_vir_cpf_h_
#define __gc_vsc_vir_cpf_h_

/******************************************************************************
            constant propagation and folding
    conditional constant propagtion worklist algorithm is uesd
******************************************************************************/

#include "gc_vsc.h"

#define     VSC_CPF_MAX_TEMP        1024
#define     VSC_CPF_MAX_INST_COUNT  1024

BEGIN_EXTERN_C()

/* lattice state */
typedef enum _VSC_CPF_LATTICE
{
    VSC_CPF_UNDEFINE = 0, /* undefined */
    VSC_CPF_CONSTANT = 1, /* all pathes are same constant */
    VSC_CPF_NOT_CONSTANT = 3, /* not constant */
} VSC_CPF_LATTICE;

/* const value for each channel */
/* to-do: four channel has the same type, is it better to have const value for each vec4 */
typedef struct _VSC_CPF_CONST
{
    gctUINT                 value;      /* use unsigned int to store the bit value */
    VIR_PrimitiveTypeId     type;       /* the type of the const */
} VSC_CPF_Const;

/* data flow for each BB */
typedef struct _VSC_CPF_BLOCK_FLOW
{
    VSC_STATE_VECTOR  inFlow;
    VSC_STATE_VECTOR  outFlow;
} VSC_CPF_BLOCK_FLOW;

typedef struct _VSC_CPF
{
    VIR_Shader                  *pShader;
    VSC_HW_CONFIG               *pHwCfg;
    VSC_OPTN_CPFOptions         *pOptions;
    VIR_Dumper                  *pDumper;
    VSC_MM                      *pMM;
    gctUINT                     flowSize;

    VSC_SIMPLE_QUEUE            workList;       /* worklist to save working bb */
    VSC_SIMPLE_RESIZABLE_ARRAY  blkFlowArray;   /* array to save constant data flow,
                                                  it is indexed by BB id */
    VSC_HASH_TABLE              constTable;    /* hash table to save the const information
                                                  key: (tempIdx*4 + channel) and bbIdx
                                                  value: VSC_CPF_Const */
} VSC_CPF;

#define VSC_CPF_GetShader(cpf)                  ((cpf)->pShader)
#define VSC_CPF_SetShader(cpf, s)               ((cpf)->pShader = (s))
#define VSC_CPF_GetHwCfg(cpf)                   ((cpf)->pHwCfg)
#define VSC_CPF_SetHwCfg(cpf, s)                ((cpf)->pHwCfg = (s))
#define VSC_CPF_GetOptions(cpf)                 ((cpf)->pOptions)
#define VSC_CPF_SetOptions(cpf, o)              ((cpf)->pOptions = (o))
#define VSC_CPF_GetDumper(cpf)                  ((cpf)->pDumper)
#define VSC_CPF_SetDumper(cpf, d)               ((cpf)->pDumper = (d))
#define VSC_CPF_GetMM(cpf)                      ((cpf)->pMM)

#define VSC_CPF_GetWorkList(cpf)                (&(cpf)->workList)
#define VSC_CPF_GetBlkFlowArray(cpf)            (&(cpf)->blkFlowArray)
#define VSC_CPF_GetConstTable(cpf)              (&(cpf)->constTable)

extern VSC_ErrCode VSC_CPF_PerformOnShader(
    IN VSC_SH_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(VSC_CPF_PerformOnShader);

END_EXTERN_C()

#endif /* __gc_vsc_vir_cpf_h_ */

