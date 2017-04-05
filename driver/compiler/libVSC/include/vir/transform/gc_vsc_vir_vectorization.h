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


#ifndef __gc_vsc_vir_vectorization_h_
#define __gc_vsc_vir_vectorization_h_

#include "gc_vsc.h"

BEGIN_EXTERN_C()

#define MAX_VECTORIZABLE_IO_NUM_IN_PACKET  CHANNEL_NUM

typedef struct _VIR_IO_VECTORIZABLE_PACKET
{
    /* These io-syms must have same vir-reg-count and they are all packed
       from first one to last one as sequence that they are appeared in
       this array with limitation that total channels count is not greater
       than 4 (MAX_VECTORIZABLE_IO_NUM_IN_PACKET). */
    VIR_Symbol*      pSymIo[MAX_VECTORIZABLE_IO_NUM_IN_PACKET];

    gctUINT          vectorizedLocation;
    gctUINT          realCount;
    gctBOOL          bOutput;
    gctBOOL          bTFB;
}VIR_IO_VECTORIZABLE_PACKET;

typedef struct _VIR_IO_VECTORIZE_PARAM
{
    VIR_Shader*                 pShader;
    VIR_IO_VECTORIZABLE_PACKET* pIoVectorizablePackets;
    gctUINT                     numOfPackets;
    VSC_MM*                     pMM;
}VIR_IO_VECTORIZE_PARAM;

gctBOOL vscVIR_CheckTwoSymsVectorizability(VIR_Shader* pShader,
                                           VIR_Symbol* pSym1,
                                           VIR_Symbol* pSym2);

/* Io vectorization */
VSC_ErrCode vscVIR_VectorizeIoPackets(VIR_IO_VECTORIZE_PARAM* pIoVecParam);

/* General local vectorization with instructions merge. To get global-like vectorization
   result, other opts should be done before it, like CSE */
VSC_ErrCode vscVIR_DoLocalVectorization(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_DoLocalVectorization);

END_EXTERN_C()

#endif /* __gc_vsc_vir_vectorization_h_ */


