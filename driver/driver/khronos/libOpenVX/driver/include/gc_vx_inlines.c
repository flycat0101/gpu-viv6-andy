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


#pragma once

#ifndef __GC_VX_INLINES_C__
#define __GC_VX_INLINES_C__

#include <gc_vx_common.h>

VX_INLINE_API vx_ptr vxFormatMemoryPtr(
        vx_memory_s *memory, vx_uint32 c, vx_uint32 x, vx_uint32 y, vx_uint32 p)
{
    vx_int64 offset = (memory->strides[p][VX_DIM_Y] * y) +
                      (memory->strides[p][VX_DIM_X] * x) +
                      (memory->strides[p][VX_DIM_CHANNEL] * c);

    return (vx_ptr)&memory->logicals[p][offset];
}

VX_INLINE_API vx_bool vxIsOdd(size_t value)
{
    return (vx_bool)(value & 1);
}

VX_INLINE_API vx_bool vxIsPowerOfTwo(vx_uint32 value)
{
    if (value == 0) return vx_false_e;

    return (value & (value - 1)) == 0;
}

#endif /* __OPENVX_INLINES_C__ */

