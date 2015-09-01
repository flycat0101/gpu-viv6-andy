/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __chip_state_h_
#define __chip_state_h_
#include "gc_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define __GL_ATTR1_MASK               (~(__GL_CLEARCOLOR_BIT |\
                                       __GL_DITHER_ENDISABLE_BIT |\
                                       __GL_CLEARDEPTH_BIT |\
                                       __GL_CLEARSTENCIL_BIT |\
                                       __GL_CLEARACCUM_BIT) )

#define __GL_ATTR2_MASK              (~(__GL_FOGINDEX_BIT |\
                                       __GL_FOGDENSITY_BIT|\
                                       __GL_FOGSTART_BIT|\
                                       __GL_FOGEND_BIT|\
                                       __GL_FOGCOORDSRC_BIT|\
                                       __GL_POLYGONSMOOTH_ENDISABLE_BIT |\
                                       __GL_POINT_FADE_THRESHOLD_SIZE_BIT |\
                                       __GL_POINT_DISTANCE_ATTENUATION_BIT))

#define __GL_ATTR3_MASK             (~(__GL_SAMPLECOVERAGE_BIT |\
                                     __GL_SAMPLE_ALPHA_TO_COVERAGE_ENDISABLE_BIT |\
                                     __GL_SAMPLE_ALPHA_TO_ONE_ENDISABLE_BIT |\
                                     __GL_SAMPLE_COVERAGE_ENDISABLE_BIT | \
                                     __GL_MAP1_ENDISABLE_BIT | \
                                     __GL_MAP1_ENDISABLE_BIT | \
                                     __GL_AUTONORMAL_ENDISABLE_BIT | \
                                     __GL_HINT_BIT))

#define __GL_LIGHTING_MASK      (~( __GL_COLORMATERIAL_BIT))

#define __GL_SAMPLER_BITS       ( __GL_TEXPARAM_WRAP_S_BIT | \
                                __GL_TEXPARAM_WRAP_T_BIT | \
                                __GL_TEXPARAM_WRAP_R_BIT | \
                                __GL_TEXPARAM_MIN_FILTER_BIT | \
                                __GL_TEXPARAM_MAG_FILTER_BIT | \
                                __GL_TEXPARAM_MIN_LOD_BIT | \
                                __GL_TEXPARAM_MAX_LOD_BIT | \
                                __GL_TEXPARAM_BASE_LEVEL_BIT | \
                                __GL_TEXPARAM_MAX_LEVEL_BIT | \
                                __GL_TEXPARAM_LOD_BIAS_BIT | \
                                __GL_TEX_UNIT_LODBIAS_BIT |\
                                __GL_TEXPARAM_COMPARE_MODE_BIT | \
                                __GL_TEXPARAM_COMPARE_FUNC_BIT | \
                                __GL_TEXPARAM_GENERATE_MIPMAP_BIT | \
                                __GL_TEXPARAM_COMPARE_FAIL_VALUE_BIT | \
                                __GL_TEXPARAM_MAX_ANISOTROPY_BIT)


#define __GL_STENCILOP_BITS     ( __GL_STENCILTEST_ENDISABLE_BIT |    \
                                  __GL_STENCILOP_FRONT_BIT       |    \
                                  __GL_STENCILFUNC_FRONT_BIT     |    \
                                  __GL_STENCILMASK_FRONT_BIT     |    \
                                  __GL_STENCILOP_BACK_BIT        |    \
                                  __GL_STENCILFUNC_BACK_BIT      |    \
                                  __GL_STENCILMASK_BACK_BIT      |    \
                                  __GL_STENCIL_ATTR_BITS         |    \
                                  __GL_CLEARSTENCIL_BIT)

#define __GL_DEPTH_BITS          (__GL_DEPTHTEST_ENDISABLE_BIT   |    \
                                  __GL_DEPTHFUNC_BIT             |    \
                                  __GL_DEPTHMASK_BIT             |    \
                                  __GL_DEPTHRANGE_BIT            |    \
                                  __GL_CLEARDEPTH_BIT)

#define __GL_ALPHABLEND_BITS    (__GL_ALPHAFUNC_BIT              |    \
                                 __GL_ALPHATEST_ENDISABLE_BIT    |    \
                                 __GL_BLENDCOLOR_BIT             |    \
                                 __GL_BLENDFUNC_BIT              |    \
                                 __GL_BLENDEQUATION_BIT          |    \
                                 __GL_BLEND_ENDISABLE_BIT)


GLvoid __glChipAttributeChange(__GLcontext *gc);
GLvoid __glChipClear(__GLcontext * gc, GLuint mask);
GLvoid __glChipUpdateShadingMode(__GLcontext * gc, GLenum value);
#ifdef __cplusplus
}
#endif
#endif /* __chip_state_h_ */
