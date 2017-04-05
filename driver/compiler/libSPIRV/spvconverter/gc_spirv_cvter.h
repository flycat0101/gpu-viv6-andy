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


#ifndef __gc_spirv_cvter_h_
#define __gc_spirv_cvter_h_

#include "gc_vsc_precomp.h"
#include "gc_hal.h"
#include "gc_hal_types.h"
#include "gc_hal_priv.h"
#include "drvi/gc_vsc_drvi_interface.h"

#define gcmDUMP_SPIRV_ASIIC           0

/* for internal usage */
typedef enum SPV_SPECFLAG
{
    SPV_SPECFLAG_NONE                   = 0x00000000,
    SPV_SPECFLAG_ENTRYPOINT             = 0x00000001,
    SPV_SPECFLAG_SPECIFIED_LOCAL_SIZE   = 0x00000002,
} Spv_SpecFlag;

typedef struct SpvDecodeInfo{
    gctUINT * binary;
    gctUINT sizeInByte;
    gctPOINTER stageInfo;
    Spv_SpecFlag specFlag;
    gctUINT localSize[3];
    gctUINT tcsInputVertices;
    gctPOINTER funcCtx;
    gctPOINTER renderpassInfo;
    gctUINT subPass;
}SpvDecodeInfo;

gceSTATUS
gcSPV_PreDecode(
    IN SpvDecodeInfo * info,
    INOUT gctPOINTER * funcTable
    );

gceSTATUS
gcSPV_PostDecode(
    IN gctPOINTER FuncTable
    );

gceSTATUS
gcSPV_Decode(
    IN SpvDecodeInfo * info,
    IN OUT SHADER_HANDLE* hVirShader
    );

#endif

