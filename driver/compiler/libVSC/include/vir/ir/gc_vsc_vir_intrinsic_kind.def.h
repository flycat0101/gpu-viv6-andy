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


    VIR_INTRINSIC_INFO(NONE), /* 0: not an intrinsics */
    VIR_INTRINSIC_INFO(UNKNOWN), /* 1: is an intrinsics, but unknown kind */

    /* EVIS instructions */
    VIR_INTRINSIC_INFO(evis_begin), /* 2: begin of evis instrinsic */
    VIR_INTRINSIC_INFO(evis_abs_diff), /* 3: also used in cl_viv_vx_ext.h, keep all evis values!!! */
    VIR_INTRINSIC_INFO(evis_iadd),
    VIR_INTRINSIC_INFO(evis_iacc_sq),
    VIR_INTRINSIC_INFO(evis_lerp),
    VIR_INTRINSIC_INFO(evis_filter),
    VIR_INTRINSIC_INFO(evis_mag_phase),
    VIR_INTRINSIC_INFO(evis_mul_shift),
    VIR_INTRINSIC_INFO(evis_dp16x1),
    VIR_INTRINSIC_INFO(evis_dp8x2),
    VIR_INTRINSIC_INFO(evis_dp4x4),
    VIR_INTRINSIC_INFO(evis_dp2x8),
    VIR_INTRINSIC_INFO(evis_clamp),
    VIR_INTRINSIC_INFO(evis_bi_linear),
    VIR_INTRINSIC_INFO(evis_select_add),
    VIR_INTRINSIC_INFO(evis_atomic_add),
    VIR_INTRINSIC_INFO(evis_bit_extract),
    VIR_INTRINSIC_INFO(evis_bit_replace),
    VIR_INTRINSIC_INFO(evis_dp32x1),
    VIR_INTRINSIC_INFO(evis_dp16x2),
    VIR_INTRINSIC_INFO(evis_dp8x4),
    VIR_INTRINSIC_INFO(evis_dp4x8),
    VIR_INTRINSIC_INFO(evis_dp2x16),
    VIR_INTRINSIC_INFO(evis_dp32x1_b),
    VIR_INTRINSIC_INFO(evis_dp16x2_b),
    VIR_INTRINSIC_INFO(evis_dp8x4_b),
    VIR_INTRINSIC_INFO(evis_dp4x8_b),
    VIR_INTRINSIC_INFO(evis_dp2x16_b),
    VIR_INTRINSIC_INFO(evis_img_load),
    VIR_INTRINSIC_INFO(evis_img_load_3d),
    VIR_INTRINSIC_INFO(evis_img_store),
    VIR_INTRINSIC_INFO(evis_img_store_3d),
    VIR_INTRINSIC_INFO(evis_vload2),
    VIR_INTRINSIC_INFO(evis_vload3),
    VIR_INTRINSIC_INFO(evis_vload4),
    VIR_INTRINSIC_INFO(evis_vload8),
    VIR_INTRINSIC_INFO(evis_vload16),
    VIR_INTRINSIC_INFO(evis_vstore2),
    VIR_INTRINSIC_INFO(evis_vstore3),
    VIR_INTRINSIC_INFO(evis_vstore4),
    VIR_INTRINSIC_INFO(evis_vstore8),
    VIR_INTRINSIC_INFO(evis_vstore16),
    VIR_INTRINSIC_INFO(evis_index_add),
    VIR_INTRINSIC_INFO(evis_vert_min3),
    VIR_INTRINSIC_INFO(evis_vert_max3),
    VIR_INTRINSIC_INFO(evis_vert_med3),
    VIR_INTRINSIC_INFO(evis_horz_min3),
    VIR_INTRINSIC_INFO(evis_horz_max3),
    VIR_INTRINSIC_INFO(evis_horz_med3),
    VIR_INTRINSIC_INFO(evis_error),
    VIR_INTRINSIC_INFO(evis_end), /* end of evis instrinsic */
    /* DO NOT add or change order of any Intrinsic before */

    /* common functions */
    VIR_INTRINSIC_INFO(clamp),
    VIR_INTRINSIC_INFO(min),
    VIR_INTRINSIC_INFO(max),
    VIR_INTRINSIC_INFO(abs),
    VIR_INTRINSIC_INFO(sign),
    VIR_INTRINSIC_INFO(floor),
    VIR_INTRINSIC_INFO(ceil),
    VIR_INTRINSIC_INFO(fract),
    VIR_INTRINSIC_INFO(mix),
    VIR_INTRINSIC_INFO(round),
    VIR_INTRINSIC_INFO(roundEven),
    VIR_INTRINSIC_INFO(trunc),
    VIR_INTRINSIC_INFO(pow),
    VIR_INTRINSIC_INFO(exp),
    VIR_INTRINSIC_INFO(exp2),
    VIR_INTRINSIC_INFO(log),
    VIR_INTRINSIC_INFO(log2),
    VIR_INTRINSIC_INFO(sqrt),
    VIR_INTRINSIC_INFO(inversesqrt),
    VIR_INTRINSIC_INFO(modf),
    VIR_INTRINSIC_INFO(modfstruct),
    VIR_INTRINSIC_INFO(step),
    VIR_INTRINSIC_INFO(smoothstep),
    VIR_INTRINSIC_INFO(fma),
    VIR_INTRINSIC_INFO(frexp),
    VIR_INTRINSIC_INFO(frexpstruct),
    VIR_INTRINSIC_INFO(ldexp),

    /* Matrix-related functions */
    VIR_INTRINSIC_INFO(determinant),
    VIR_INTRINSIC_INFO(matrixinverse),
    VIR_INTRINSIC_INFO(matrixCompMult),

    /* Integer Functions */
    VIR_INTRINSIC_INFO(findlsb),
    VIR_INTRINSIC_INFO(findmsb),

    /* Angle and Trigonometry Functions */
    VIR_INTRINSIC_INFO(radians),
    VIR_INTRINSIC_INFO(degrees),

    VIR_INTRINSIC_INFO(sin),
    VIR_INTRINSIC_INFO(cos),
    VIR_INTRINSIC_INFO(tan),
    VIR_INTRINSIC_INFO(asin),
    VIR_INTRINSIC_INFO(acos),
    VIR_INTRINSIC_INFO(atan),

    VIR_INTRINSIC_INFO(sinh),
    VIR_INTRINSIC_INFO(cosh),
    VIR_INTRINSIC_INFO(tanh),
    VIR_INTRINSIC_INFO(asinh),
    VIR_INTRINSIC_INFO(acosh),
    VIR_INTRINSIC_INFO(atanh),
    VIR_INTRINSIC_INFO(atan2),

    /* Floating-Point Pack and Unpack Functions */
    VIR_INTRINSIC_INFO(packSnorm4x8),
    VIR_INTRINSIC_INFO(packUnorm4x8),
    VIR_INTRINSIC_INFO(packSnorm2x16),
    VIR_INTRINSIC_INFO(packUnorm2x16),
    VIR_INTRINSIC_INFO(packHalf2x16),
    VIR_INTRINSIC_INFO(packDouble2x32),
    VIR_INTRINSIC_INFO(unpackSnorm2x16),
    VIR_INTRINSIC_INFO(unpackUnorm2x16),
    VIR_INTRINSIC_INFO(unpackHalf2x16),
    VIR_INTRINSIC_INFO(unpackSnorm4x8),
    VIR_INTRINSIC_INFO(unpackUnorm4x8),
    VIR_INTRINSIC_INFO(unpackDouble2x32),

    /* Geometric Functions */
    VIR_INTRINSIC_INFO(cross),
    VIR_INTRINSIC_INFO(length),
    VIR_INTRINSIC_INFO(distance),
    VIR_INTRINSIC_INFO(normalize),
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

    /* Interpolation Functions. */
    VIR_INTRINSIC_INFO(interpolateatcenroid),
    VIR_INTRINSIC_INFO(interpolateatsample),
    VIR_INTRINSIC_INFO(interpolateatoffset),

    /* synchronization functions */
    VIR_INTRINSIC_INFO(barrier),

    /* matrix operations */
    VIR_INTRINSIC_INFO(transpose), /* matrix transpose */
    VIR_INTRINSIC_INFO(matrix_times_scalar), /* matrix * scalar */
    VIR_INTRINSIC_INFO(mattrix_times_vector), /* matrix * vector */
    VIR_INTRINSIC_INFO(matrix_times_matrix), /* matrix * matrix */
    VIR_INTRINSIC_INFO(vector_times_matrix), /* vector * matrix */
    VIR_INTRINSIC_INFO(outer_product), /* matrix outer product */

    /* Image-related functions. */
    VIR_INTRINSIC_INFO(image_store), /* image store */
    VIR_INTRINSIC_INFO(image_load), /* image load */

    /* vector dynamic indexing */
    VIR_INTRINSIC_INFO(vecGet), /* get one element from vec */
    VIR_INTRINSIC_INFO(vecSet), /* Set one element from vec */

    VIR_INTRINSIC_INFO(uaddCarry),
    VIR_INTRINSIC_INFO(usubBorrow),
    VIR_INTRINSIC_INFO(umulExtended),
    VIR_INTRINSIC_INFO(imulExtended),

    VIR_INTRINSIC_INFO(quantizeToF16),

    VIR_INTRINSIC_INFO(image_fetch),
    VIR_INTRINSIC_INFO(image_addr),
    VIR_INTRINSIC_INFO(image_query_format),
    VIR_INTRINSIC_INFO(image_query_order),
    VIR_INTRINSIC_INFO(image_query_size_lod),
    VIR_INTRINSIC_INFO(image_query_size),
    VIR_INTRINSIC_INFO(image_query_lod),
    VIR_INTRINSIC_INFO(image_query_levels),
    VIR_INTRINSIC_INFO(image_query_samples),

    /* texture load.*/
    VIR_INTRINSIC_INFO(texld),
    VIR_INTRINSIC_INFO(texldpcf),
    VIR_INTRINSIC_INFO(texld_proj),
    VIR_INTRINSIC_INFO(texld_gather),
    VIR_INTRINSIC_INFO(texld_fetch_ms),

    /* three operand instructions */
    VIR_INTRINSIC_INFO(swizzle),
    VIR_INTRINSIC_INFO(madsat),
    VIR_INTRINSIC_INFO(swizzle_full_def), /* fully defined swizzle */

    VIR_INTRINSIC_INFO(imadhi0),
    VIR_INTRINSIC_INFO(imadlo0),

    VIR_INTRINSIC_INFO(LAST),

