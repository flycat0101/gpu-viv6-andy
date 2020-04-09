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
    SPV_SPECFLAG_DISABLE_IR_DUMP        = 0x00000004,
    SPV_SPECFLAG_LOWER_LEFT_DOMAIN      = 0x00000008,
    SPV_SPECFLAG_INTERNAL_LIBRARY       = 0x00000010,
} Spv_SpecFlag;

typedef enum SPV_ATTACHMENTFLAG
{
    SPV_ATTACHMENTFLAG_NONE             = 0x00000000,
    SPV_ATTACHMENTFLAG_TREAT_AS_SAMPLER = 0x00000001,
    SPV_ATTACHMENTFLAG_MULTI_SAMPLE     = 0x00000002,
} Spv_AttachmentFlag;

typedef struct
{
    gctUINT32 format;
    Spv_AttachmentFlag attachmentFlag;
}SpvAttachmentDesc;

#define __SPV_MAX_RENDER_TARGETS 4
typedef struct SpvRenderSubPassInfoRec
{
    gctUINT32 input_attachment_index[__SPV_MAX_RENDER_TARGETS];

}SpvRenderSubPassInfo;

typedef struct
{
    gctUINT attachmentCount;
    SpvAttachmentDesc * attachments;

    gctUINT subPassInfoCount;
    SpvRenderSubPassInfo *subPassInfo;
}SpvRenderPassInfo;

typedef struct SpvDecodeInfo{
    gctUINT * binary;
    gctUINT sizeInByte;
    gctPOINTER stageInfo;
    Spv_SpecFlag specFlag;
    gctUINT localSize[3];
    gctUINT tcsInputVertices;
    gctPOINTER funcCtx;
    SpvRenderPassInfo* renderpassInfo;
    gctUINT subPass;
    gctBOOL isLibraryShader;
    /* The default image format for those image types with the unknown image format.*/
    VSC_IMAGE_FORMAT defaultImageFormat;
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

