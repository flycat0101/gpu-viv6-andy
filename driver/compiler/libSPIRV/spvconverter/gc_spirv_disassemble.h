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


#ifndef __gc_spirv_disassemble_h_
#define __gc_spirv_disassemble_h_

#include "gc_spirv_types.h"
#include "gc_hal_user_precomp.h"
#include "vulkan/spirv.h"
#include "vulkan/vulkan_core.h"

gceSTATUS __SpvDumpValidator(
    gctUINT * stream,
    gctUINT sizeInByte);

gceSTATUS __SpvDumpLine(
    SpvId resultId,
    SpvId typeId,
    SpvOp opCode,
    gctUINT * stream,
    gctUINT numOperands);

gceSTATUS __SpvDumpSpecConstant(
    VkSpecializationInfo        *specInfo);

gceSTATUS __SpvDumpSpriv(
    gctUINT * stream,
    gctUINT sizeInByte);

#endif
