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


#ifndef __gc_vsc_vir_scalarization_h_
#define __gc_vsc_vir_scalarization_h_

#include "gc_vsc.h"

typedef struct _VSC_SCL_SCALARIZATION
{
    VIR_Shader* shader;
    VSC_HASH_TABLE array_infos;
    VSC_OPTN_SCLOptions* options;
    VIR_Dumper* dumper;
    VSC_PRIMARY_MEM_POOL pmp;
} VSC_SCL_Scalarization;

#define VSC_SCL_Scalarization_GetShader(scl)           ((scl)->shader)
#define VSC_SCL_Scalarization_SetShader(scl, shader)           ((scl)->shader = shader)
#define VSC_SCL_Scalarization_GetArrayInfos(scl)      (&((scl)->array_infos))
#define VSC_SCL_Scalarization_GetOptions(scl)          ((scl)->options)
#define VSC_SCL_Scalarization_SetOptions(scl, options)          ((scl)->options = options)
#define VSC_SCL_Scalarization_GetDumper(scl)           ((scl)->dumper)
#define VSC_SCL_Scalarization_SetDumper(scl, dumper)           ((scl)->dumper = dumper)
#define VSC_SCL_Scalarization_GetPmp(scl)              (&((scl)->pmp))
#define VSC_SCL_Scalarization_GetMM(scl)               (&((scl)->pmp.mmWrapper))

extern void VSC_SCL_Scalarization_Init(
    IN OUT VSC_SCL_Scalarization* scl,
    IN VIR_Shader* shader,
    IN VSC_OPTN_SCLOptions* options,
    IN VIR_Dumper* dumper
    );

extern void VSC_SCL_Scalarization_Final(
    IN OUT VSC_SCL_Scalarization* scl
    );

/* because arrays could be defined in shader scope, this
 * optimization should be performed on shader scope */
extern VSC_ErrCode VSC_SCL_Scalarization_PerformOnShader(
    IN OUT VSC_SCL_Scalarization* scl
    );

#endif

