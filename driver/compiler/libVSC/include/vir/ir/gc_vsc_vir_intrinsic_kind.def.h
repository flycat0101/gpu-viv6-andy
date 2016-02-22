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


    VIR_INTRINSIC_INFO(NONE), /* not an intrinsics */
    VIR_INTRINSIC_INFO(UNKNOWN), /* is an intrinsics, but unknown kind */

    /* common functions */
    VIR_INTRINSIC_INFO(clamp),
    VIR_INTRINSIC_INFO(min),
    VIR_INTRINSIC_INFO(max),
    VIR_INTRINSIC_INFO(sign),
    VIR_INTRINSIC_INFO(fmix),
    VIR_INTRINSIC_INFO(mix),
    /* Angle and Trigonometry Functions */
    VIR_INTRINSIC_INFO(radians),
    VIR_INTRINSIC_INFO(degrees),
    VIR_INTRINSIC_INFO(step),
    VIR_INTRINSIC_INFO(smoothstep),
    /* Geometric Functions */
    VIR_INTRINSIC_INFO(cross),
    VIR_INTRINSIC_INFO(fast_length),
    VIR_INTRINSIC_INFO(length),
    VIR_INTRINSIC_INFO(distance),
    VIR_INTRINSIC_INFO(dot),
    VIR_INTRINSIC_INFO(normalize),
    VIR_INTRINSIC_INFO(fast_normalize),
    VIR_INTRINSIC_INFO(faceforward),
    VIR_INTRINSIC_INFO(reflect),
    VIR_INTRINSIC_INFO(refract),
    /* Vector Relational Functions */
    VIR_INTRINSIC_INFO(isequal),
    VIR_INTRINSIC_INFO(isnotequal),
    VIR_INTRINSIC_INFO(isgreater),
    VIR_INTRINSIC_INFO(isgreaterequal),
    VIR_INTRINSIC_INFO(isless),
    VIR_INTRINSIC_INFO(islessequal),
    VIR_INTRINSIC_INFO(islessgreater),
    VIR_INTRINSIC_INFO(isordered),
    VIR_INTRINSIC_INFO(isunordered),
    VIR_INTRINSIC_INFO(isfinite),
    VIR_INTRINSIC_INFO(isnan),
    VIR_INTRINSIC_INFO(isinf),
    VIR_INTRINSIC_INFO(isnormal),
    VIR_INTRINSIC_INFO(signbit),
    VIR_INTRINSIC_INFO(lgamma),
    VIR_INTRINSIC_INFO(lgamma_r),
    VIR_INTRINSIC_INFO(shuffle),
    VIR_INTRINSIC_INFO(shuffle2),
    VIR_INTRINSIC_INFO(select),
    VIR_INTRINSIC_INFO(bitselect),
    VIR_INTRINSIC_INFO(any),
    VIR_INTRINSIC_INFO(all),
    /* Matrix Functions */
    VIR_INTRINSIC_INFO(matrixCompMult),
    /* Async copy and prefetch */
    VIR_INTRINSIC_INFO(async_work_group_copy),
    VIR_INTRINSIC_INFO(async_work_group_strided_copy),
    VIR_INTRINSIC_INFO(wait_group_events),
    VIR_INTRINSIC_INFO(prefetch),
    /* Atomic Functions */
    VIR_INTRINSIC_INFO(atomic_add),
    VIR_INTRINSIC_INFO(atomic_sub),
    VIR_INTRINSIC_INFO(atomic_inc),
    VIR_INTRINSIC_INFO(atomic_dec),
    VIR_INTRINSIC_INFO(atomic_xchg),
    VIR_INTRINSIC_INFO(atomic_cmpxchg),
    VIR_INTRINSIC_INFO(atomic_min),
    VIR_INTRINSIC_INFO(atomic_max),
    VIR_INTRINSIC_INFO(atomic_or),
    VIR_INTRINSIC_INFO(atomic_and),
    VIR_INTRINSIC_INFO(atomic_xor),

    /* work-item functions */
    VIR_INTRINSIC_INFO(get_global_id),
    VIR_INTRINSIC_INFO(get_local_id),
    VIR_INTRINSIC_INFO(get_group_id),
    VIR_INTRINSIC_INFO(get_work_dim),
    VIR_INTRINSIC_INFO(get_global_size),
    VIR_INTRINSIC_INFO(get_local_size),
    VIR_INTRINSIC_INFO(get_global_offset),
    VIR_INTRINSIC_INFO(get_num_groups),
    /* synchronization functions */
    VIR_INTRINSIC_INFO(barrier),
    VIR_INTRINSIC_INFO(LAST),

