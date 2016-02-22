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


#ifndef __gc_gl_context_h_
#define __gc_gl_context_h_

#include "gltypes.h"
#include "gc_gl_consts.h"
#include "gc_gl_select.h"
#include "gc_gl_xform.h"
#include "gc_gl_ctable.h"
#include "gc_gl_pixel.h"
#include "gc_gl_lighting.h"
#include "gc_gl_eval.h"
#include "gc_gl_enable.h"
#include "gc_gl_sharedobj.h"
#include "gc_gl_dlist.h"
#include "gc_gl_program.h"
#include "gc_gl_bufferobject.h"
#include "gc_gl_texture.h"
#include "gc_gl_query.h"
#include "gc_gl_constantbuffer.h"
#include "gc_gl_shader.h"
#include "gc_gl_extensions.h"
#include "gc_gl_framebufferobject.h"
#include "gc_gl_devicepipe.h"
#include "gc_gl_varray.h"
#include "gc_gl_attrib.h"
#include "gc_gl_vertex.h"
#include "gc_gl_feedback.h"
#include "gc_gl_utils.h"
#include "gc_gl_image.h"
#include "g_imfncs.h"
#include "gc_gl_vsoutput.h"
#include "gc_gl_buffers.h"

#ifdef _LINUX_
//__GL_INLINE GLvoid dummyfunc(long * x, long y)
//{
//}

__GL_INLINE GLvoid _bittestandreset(long *a, long b)
{
    long value = *a;

    if (value & (1 << b)) {
        value &= ~(1 << b);
    }

    *a = value;
}
__GL_INLINE GLboolean _BitScanForward(unsigned long * Index, long data)
{
    unsigned int bitsetindex = 0;

    if (data == 0) {
        *Index = (unsigned long)-1;
        return GL_FALSE;
    }

    while (!(data & (1 << bitsetindex))) {
        bitsetindex++;
    }

    *Index = bitsetindex;
    return  GL_TRUE;
}
#endif


/* Referenced by "gc->globalDirtyState[]".
 */
enum {
    __GL_ALL_ATTRS        = 0,      /* Non-zero following word => bit position */
    __GL_DIRTY_ATTRS_1    = 1,
    __GL_DIRTY_ATTRS_2    = 2,
    __GL_DIRTY_ATTRS_3    = 3,
    __GL_LIGHTING_ATTRS   = 4,
    __GL_LIGHT_SRC_ATTRS  = 5,      /* Light# attribute changed => bit position */
    __GL_CLIP_ATTRS       = 6,      /* bit 0~15: plane param; bit 16~31: endisable */
    __GL_PIPELINE_ATTRS   = 7,      /* Pipeline private dirty bits */
    __GL_PROGRAM_ATTRS    = 8,      /* program dirty bits  */
    __GL_TEX_UNIT_ATTRS   = 9,      /* __GL_TEX_UNIT_ATTRS does not occupy a UINT in globalDirtyState[] */
    __GL_DIRTY_ATTRS_END  = __GL_TEX_UNIT_ATTRS
};

/*
 * Macros that set global dirty attribute bits.
 */
#define __GL_SET_ATTR_DIRTY_BIT(gc, index, bit)                                 \
    (gc)->globalDirtyState[(index)] |= (bit);                                   \
    (gc)->globalDirtyState[__GL_ALL_ATTRS] |= (1 << (index))

#define __GL_SET_LIGHT_SRC_BIT(gc, lighti, bit)                                 \
    (gc)->lightAttrState[(lighti)] |= (bit);                                    \
    (gc)->globalDirtyState[__GL_LIGHT_SRC_ATTRS] |= (1 << (lighti));            \
    (gc)->globalDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_LIGHT_SRC_ATTRS)

#define __GL_SET_TEX_UNIT_BIT(gc, unit, bit)                                    \
    (gc)->texUnitAttrState[(unit)] |= (bit);                                    \
    (gc)->texUnitAttrDirtyMask |= (__GL_ONE_64 << (unit));                      \
    (gc)->globalDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_TEX_UNIT_ATTRS)

#define __GL_SET_SWP_DIRTY_ATTR(gc, bit)                                        \
    (gc)->swpDirtyState[__GL_PIPELINE_ATTRS] |= (bit);                          \
    (gc)->swpDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_PIPELINE_ATTRS)

#define __GL_SET_GLSL_SAMPLER_BIT(gc, samplerRegIdx)             \
    (gc)->shaderProgram.samplerDirtyState |= (__GL_ONE_64 << (samplerRegIdx));  \
    (gc)->globalDirtyState[__GL_PROGRAM_ATTRS] |= (__GL_DIRTY_GLSL_SAMPLER);    \
    (gc)->globalDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_PROGRAM_ATTRS)

/* Referenced by "gc->globalDirtyState[__GL_DIRTY_ATTRS_1]".
 */
enum {
    __GL_VIEWPORT_BIT                           = (1 << 0),
    __GL_DEPTHRANGE_BIT                         = (1 << 1),
    __GL_ALPHAFUNC_BIT                          = (1 << 2),
    __GL_ALPHATEST_ENDISABLE_BIT                = (1 << 3),
    __GL_BLENDCOLOR_BIT                         = (1 << 4),
    __GL_BLENDFUNC_BIT                          = (1 << 5),
    __GL_BLENDEQUATION_BIT                      = (1 << 6),
    __GL_BLEND_ENDISABLE_BIT                    = (1 << 7),
    __GL_LOGICOP_BIT                            = (1 << 8),
    __GL_LOGICOP_ENDISABLE_BIT                  = (1 << 9),
    __GL_CLEARCOLOR_BIT                         = (1 << 10),
    __GL_COLORMASK_BIT                          = (1 << 11),
    __GL_DITHER_ENDISABLE_BIT                   = (1 << 12),
    __GL_DEPTHFUNC_BIT                          = (1 << 13),
    __GL_DEPTHMASK_BIT                          = (1 << 14),
    __GL_DEPTHTEST_ENDISABLE_BIT                = (1 << 15),
    __GL_CLEARDEPTH_BIT                         = (1 << 16),
    __GL_STENCILFUNC_FRONT_BIT                  = (1 << 17),
    __GL_STENCILOP_FRONT_BIT                    = (1 << 18),
    __GL_STENCILFUNC_BACK_BIT                   = (1 << 19),
    __GL_STENCILOP_BACK_BIT                     = (1 << 20),
    __GL_STENCILMASK_FRONT_BIT                  = (1 << 21),
    __GL_STENCILMASK_BACK_BIT                   = (1 << 22),
    __GL_STENCILTEST_ENDISABLE_BIT              = (1 << 23),
    __GL_CLEARSTENCIL_BIT                       = (1 << 24),
    __GL_SCISSOR_BIT                            = (1 << 25),
    __GL_SCISSORTEST_ENDISABLE_BIT              = (1 << 26),
    __GL_CLEARACCUM_BIT                         = (1 << 27),
    __GL_DEPTHBOUNDTEST_BIT                     = (1 << 28),
    __GL_DEPTHBOUNDTESTENABLE_BIT               = (1 << 29),
    __GL_CLAMP_VERTEX_COLOR_BIT                 = (1 << 30),
    __GL_CLAMP_FRAG_COLOR_BIT                   = (1 << 31)
};

#define __GL_CLAMP_COLOR_BITS (__GL_CLAMP_VERTEX_COLOR_BIT | __GL_CLAMP_FRAG_COLOR_BIT)

#define __GL_COLORBUF_ATTR_BITS ( \
                __GL_ALPHAFUNC_BIT | \
                __GL_ALPHATEST_ENDISABLE_BIT | \
                __GL_BLENDCOLOR_BIT | \
                __GL_BLENDFUNC_BIT | \
                __GL_BLENDEQUATION_BIT | \
                __GL_BLEND_ENDISABLE_BIT | \
                __GL_LOGICOP_BIT | \
                __GL_LOGICOP_ENDISABLE_BIT | \
                __GL_CLEARCOLOR_BIT | \
                __GL_COLORMASK_BIT | \
                __GL_DITHER_ENDISABLE_BIT | \
                __GL_CLAMP_FRAG_COLOR_BIT)

#define __GL_DEPTHBUF_ATTR_BITS ( \
                __GL_DEPTHFUNC_BIT | \
                __GL_DEPTHMASK_BIT | \
                __GL_DEPTHTEST_ENDISABLE_BIT | \
                __GL_CLEARDEPTH_BIT | \
                __GL_DEPTHBOUNDTEST_BIT | \
                __GL_DEPTHBOUNDTESTENABLE_BIT)

#define __GL_STENCIL_ATTR_BITS ( \
                __GL_STENCILFUNC_FRONT_BIT | \
                __GL_STENCILOP_FRONT_BIT | \
                __GL_STENCILFUNC_BACK_BIT | \
                __GL_STENCILOP_BACK_BIT | \
                __GL_STENCILTEST_ENDISABLE_BIT | \
                __GL_STENCILMASK_FRONT_BIT | \
                __GL_STENCILMASK_BACK_BIT | \
                __GL_CLEARSTENCIL_BIT)

#define __GL_ATTR1_ENABLE_BITS    ( \
                __GL_ALPHATEST_ENDISABLE_BIT | \
                __GL_BLEND_ENDISABLE_BIT | \
                __GL_LOGICOP_ENDISABLE_BIT | \
                __GL_DITHER_ENDISABLE_BIT | \
                __GL_DEPTHTEST_ENDISABLE_BIT | \
                __GL_STENCILTEST_ENDISABLE_BIT | \
                __GL_SCISSORTEST_ENDISABLE_BIT | \
                __GL_DEPTHBOUNDTESTENABLE_BIT)

#define __GL_ATTR1_VERTEXPROGRAM_BITS 0
#define __GL_ATTR1_FRAGMENTPROGRAM_BITS  0

/* Referenced by "gc->globalDirtyState[__GL_DIRTY_ATTRS_2]".
 */
enum {
    __GL_FOGCOLOR_BIT                           = (1 << 0),
    __GL_FOGINDEX_BIT                           = (1 << 1),
    __GL_FOGDENSITY_BIT                         = (1 << 2),
    __GL_FOGSTART_BIT                           = (1 << 3),
    __GL_FOGEND_BIT                             = (1 << 4),
    __GL_FOGMODE_BIT                            = (1 << 5),
    __GL_FOGCOORDSRC_BIT                        = (1 << 6),
    __GL_FOG_ENDISABLE_BIT                      = (1 << 7),
    __GL_FRONTFACE_BIT                          = (1 << 8),
    __GL_CULLFACE_BIT                           = (1 << 9),
    __GL_CULLFACE_ENDISABLE_BIT                 = (1 << 10),
    __GL_POLYGONMODE_BIT                        = (1 << 11),
    __GL_POLYGONOFFSET_BIT                      = (1 << 12),
    __GL_POLYGONOFFSET_POINT_ENDISABLE_BIT      = (1 << 13),
    __GL_POLYGONOFFSET_LINE_ENDISABLE_BIT       = (1 << 14),
    __GL_POLYGONOFFSET_FILL_ENDISABLE_BIT       = (1 << 15),
    __GL_POLYGONSMOOTH_ENDISABLE_BIT            = (1 << 16),
    __GL_POLYGONSTIPPLE_BIT                     = (1 << 17),
    __GL_POLYGONSTIPPLE_ENDISABLE_BIT           = (1 << 18),
    __GL_LINEWIDTH_BIT                          = (1 << 19),
    __GL_LINESMOOTH_ENDISABLE_BIT               = (1 << 20),
    __GL_LINESTIPPLE_BIT                        = (1 << 21),
    __GL_LINESTIPPLE_ENDISABLE_BIT              = (1 << 22),
    __GL_POINTSIZE_BIT                          = (1 << 23),
    __GL_POINTSMOOTH_ENDISABLE_BIT              = (1 << 24),
    __GL_POINT_SIZE_MIN_BIT                     = (1 << 25),
    __GL_POINT_SIZE_MAX_BIT                     = (1 << 26),
    __GL_POINT_FADE_THRESHOLD_SIZE_BIT          = (1 << 27),
    __GL_POINT_DISTANCE_ATTENUATION_BIT         = (1 << 28),
    __GL_POINTSPRITE_ENDISABLE_BIT              = (1 << 29),
    __GL_POINTSPRITE_COORD_ORIGIN_BIT           = (1 << 30),
    __GL_PRIMMODE_BIT                           = (1 << 31)
};

#define __GL_FOG_ATTR_BITS    ( \
                __GL_FOGCOLOR_BIT | \
                __GL_FOGINDEX_BIT | \
                __GL_FOGDENSITY_BIT | \
                __GL_FOGSTART_BIT | \
                __GL_FOGEND_BIT | \
                __GL_FOGMODE_BIT | \
                __GL_FOGCOORDSRC_BIT | \
                __GL_FOG_ENDISABLE_BIT)

#define __GL_POLYGON_ATTR_BITS    ( \
                __GL_FRONTFACE_BIT | \
                __GL_CULLFACE_BIT | \
                __GL_CULLFACE_ENDISABLE_BIT | \
                __GL_POLYGONMODE_BIT | \
                __GL_POLYGONOFFSET_BIT | \
                __GL_POLYGONOFFSET_POINT_ENDISABLE_BIT | \
                __GL_POLYGONOFFSET_LINE_ENDISABLE_BIT | \
                __GL_POLYGONOFFSET_FILL_ENDISABLE_BIT | \
                __GL_POLYGONSMOOTH_ENDISABLE_BIT | \
                __GL_POLYGONSTIPPLE_BIT | \
                __GL_POLYGONSTIPPLE_ENDISABLE_BIT)

#define __GL_LINE_ATTR_BITS    ( \
                __GL_LINEWIDTH_BIT | \
                __GL_LINESMOOTH_ENDISABLE_BIT | \
                __GL_LINESTIPPLE_BIT | \
                __GL_LINESTIPPLE_ENDISABLE_BIT)

#define __GL_POINT_ATTR_BITS ( \
                __GL_POINTSIZE_BIT | \
                __GL_POINTSMOOTH_ENDISABLE_BIT | \
                __GL_POINT_SIZE_MIN_BIT | \
                __GL_POINT_SIZE_MAX_BIT | \
                __GL_POINT_FADE_THRESHOLD_SIZE_BIT | \
                __GL_POINT_DISTANCE_ATTENUATION_BIT | \
                __GL_POINTSPRITE_ENDISABLE_BIT | \
                __GL_POINTSPRITE_COORD_ORIGIN_BIT)

#define __GL_ATTR2_ENABLE_BITS ( \
                __GL_FOG_ENDISABLE_BIT | \
                __GL_CULLFACE_ENDISABLE_BIT | \
                __GL_POLYGONOFFSET_POINT_ENDISABLE_BIT | \
                __GL_POLYGONOFFSET_LINE_ENDISABLE_BIT | \
                __GL_POLYGONOFFSET_FILL_ENDISABLE_BIT | \
                __GL_POLYGONSMOOTH_ENDISABLE_BIT | \
                __GL_POLYGONSTIPPLE_ENDISABLE_BIT | \
                __GL_LINESMOOTH_ENDISABLE_BIT | \
                __GL_LINESTIPPLE_ENDISABLE_BIT | \
                __GL_POINTSMOOTH_ENDISABLE_BIT)

#define __GL_POINT_SIZE_MIN_MAX_THRESHOLD ( \
                __GL_POINTSIZE_BIT | \
                __GL_POINT_SIZE_MIN_BIT | \
                __GL_POINT_SIZE_MAX_BIT | \
                __GL_POINT_FADE_THRESHOLD_SIZE_BIT)
#if __GL_PS_FOG
/* Exclude the __GL_FOGCOORDSRC_BIT which affects ffvs whenever FP enabled or not */
#define __GL_ATTR2_FOG_FP_BITS ( __GL_FOG_ATTR_BITS & (~__GL_FOGCOORDSRC_BIT))
#else
#define __GL_ATTR2_FOG_FP_BITS ( __GL_FOG_ATTR_BITS)
#endif

#define __GL_ATTR2_VERTEXPROGRAM_BITS 0

#define __GL_ATTR2_FRAGMENTPROGRAM_BITS  (__GL_ATTR2_FOG_FP_BITS)

/* Referenced by "gc->globalDirtyState[__GL_DIRTY_ATTRS_3]".
 */
enum {
    __GL_RENDERMODE_BIT                             = (1 << 0),
    __GL_MODELVIEW_TRANSFORM_BIT                    = (1 << 1),
    __GL_PROJECTION_TRANSFORM_BIT                   = (1 << 2),
    __GL_NORMALIZE_ENDISABLE_BIT                    = (1 << 3),
    __GL_RESCALENORMAL_ENDISABLE_BIT                = (1 << 4),
    __GL_SAMPLECOVERAGE_BIT                         = (1 << 5),
    __GL_MULTISAMPLE_ENDISABLE_BIT                  = (1 << 6),
    __GL_SAMPLE_ALPHA_TO_COVERAGE_ENDISABLE_BIT     = (1 << 7),
    __GL_SAMPLE_ALPHA_TO_ONE_ENDISABLE_BIT          = (1 << 8),
    __GL_SAMPLE_COVERAGE_ENDISABLE_BIT              = (1 << 9),
    __GL_MAP1_ENDISABLE_BIT                         = (1 << 10),
    __GL_MAP2_ENDISABLE_BIT                         = (1 << 11),
    __GL_AUTONORMAL_ENDISABLE_BIT                   = (1 << 12),
    __GL_COLORSUM_ENDISABLE_BIT                     = (1 << 13),
    __GL_HINT_BIT                                   = (1 << 14)
};

#define __GL_ATTR3_ENABLE_BITS ( \
                __GL_NORMALIZE_ENDISABLE_BIT | \
                __GL_RESCALENORMAL_ENDISABLE_BIT | \
                __GL_MULTISAMPLE_ENDISABLE_BIT | \
                __GL_SAMPLE_ALPHA_TO_COVERAGE_ENDISABLE_BIT | \
                __GL_SAMPLE_ALPHA_TO_ONE_ENDISABLE_BIT | \
                __GL_SAMPLE_COVERAGE_ENDISABLE_BIT | \
                __GL_MAP1_ENDISABLE_BIT | \
                __GL_MAP2_ENDISABLE_BIT | \
                __GL_AUTONORMAL_ENDISABLE_BIT | \
                __GL_COLORSUM_ENDISABLE_BIT)

#define __GL_ATTR3_VERTEXPROGRAM_BITS ( \
    __GL_MODELVIEW_TRANSFORM_BIT | \
    __GL_PROJECTION_TRANSFORM_BIT | \
    __GL_NORMALIZE_ENDISABLE_BIT | \
    __GL_RESCALENORMAL_ENDISABLE_BIT)

#define __GL_MULTISAMPLE_ATTR_BITS ( \
    __GL_SAMPLECOVERAGE_BIT | \
    __GL_MULTISAMPLE_ENDISABLE_BIT | \
    __GL_SAMPLE_ALPHA_TO_COVERAGE_ENDISABLE_BIT    | \
    __GL_SAMPLE_ALPHA_TO_ONE_ENDISABLE_BIT | \
    __GL_SAMPLE_COVERAGE_ENDISABLE_BIT)

#define __GL_ATTR3_FRAGMENTPROGRAM_BITS  0

/* Referenced by "gc->globalDirtyState[__GL_LIGHTING_ATTRS]".
 */
enum {
    __GL_SHADEMODEL_BIT                         = (1 << 0),
    __GL_LIGHTING_ENDISABLE_BIT                 = (1 << 1),
    __GL_LIGHTMODEL_AMBIENT_BIT                 = (1 << 2),
    __GL_LIGHTMODEL_LOCALVIEWER_BIT             = (1 << 3),
    __GL_LIGHTMODEL_TWOSIDE_BIT                 = (1 << 4),
    __GL_LIGHTMODEL_COLORCONTROL_BIT            = (1 << 5),
    __GL_MATERIAL_COLORINDEX_FRONT_BIT          = (1 << 6),
    __GL_MATERIAL_EMISSION_FRONT_BIT            = (1 << 7),
    __GL_MATERIAL_SPECULAR_FRONT_BIT            = (1 << 8),
    __GL_MATERIAL_SHININESS_FRONT_BIT           = (1 << 9),
    __GL_MATERIAL_AMBIENT_FRONT_BIT             = (1 << 10),
    __GL_MATERIAL_DIFFUSE_FRONT_BIT             = (1 << 11),
    __GL_MATERIAL_COLORINDEX_BACK_BIT           = (1 << 12),
    __GL_MATERIAL_EMISSION_BACK_BIT             = (1 << 13),
    __GL_MATERIAL_SPECULAR_BACK_BIT             = (1 << 14),
    __GL_MATERIAL_SHININESS_BACK_BIT            = (1 << 15),
    __GL_MATERIAL_AMBIENT_BACK_BIT              = (1 << 16),
    __GL_MATERIAL_DIFFUSE_BACK_BIT              = (1 << 17),
    __GL_COLORMATERIAL_BIT                      = (1 << 18),
    __GL_COLORMATERIAL_ENDISABLE_BIT            = (1 << 19)
};

#define __GL_LIGHTING_ATTR_BITS    ( \
                __GL_SHADEMODEL_BIT | \
                __GL_LIGHTING_ENDISABLE_BIT | \
                __GL_LIGHTMODEL_AMBIENT_BIT | \
                __GL_LIGHTMODEL_LOCALVIEWER_BIT | \
                __GL_LIGHTMODEL_TWOSIDE_BIT | \
                __GL_LIGHTMODEL_COLORCONTROL_BIT | \
                __GL_MATERIAL_COLORINDEX_FRONT_BIT | \
                __GL_MATERIAL_EMISSION_FRONT_BIT | \
                __GL_MATERIAL_SPECULAR_FRONT_BIT | \
                __GL_MATERIAL_SHININESS_FRONT_BIT | \
                __GL_MATERIAL_AMBIENT_FRONT_BIT | \
                __GL_MATERIAL_DIFFUSE_FRONT_BIT | \
                __GL_MATERIAL_COLORINDEX_BACK_BIT | \
                __GL_MATERIAL_EMISSION_BACK_BIT | \
                __GL_MATERIAL_SPECULAR_BACK_BIT | \
                __GL_MATERIAL_SHININESS_BACK_BIT | \
                __GL_MATERIAL_AMBIENT_BACK_BIT | \
                __GL_MATERIAL_DIFFUSE_BACK_BIT | \
                __GL_COLORMATERIAL_BIT | \
                __GL_COLORMATERIAL_ENDISABLE_BIT)

#define __GL_LIGHTING_ENABLE_BITS ( \
                __GL_LIGHTING_ENDISABLE_BIT | \
                __GL_COLORMATERIAL_ENDISABLE_BIT)

#define __GL_LIGHTMODEL_ATTR_BITS    ( \
                __GL_LIGHTMODEL_AMBIENT_BIT | \
                __GL_LIGHTMODEL_LOCALVIEWER_BIT | \
                __GL_LIGHTMODEL_TWOSIDE_BIT | \
                __GL_LIGHTMODEL_COLORCONTROL_BIT)

#define __GL_FRONT_MATERIAL_BITS ( \
                __GL_MATERIAL_COLORINDEX_FRONT_BIT | \
                __GL_MATERIAL_EMISSION_FRONT_BIT | \
                __GL_MATERIAL_SPECULAR_FRONT_BIT | \
                __GL_MATERIAL_SHININESS_FRONT_BIT | \
                __GL_MATERIAL_AMBIENT_FRONT_BIT | \
                __GL_MATERIAL_DIFFUSE_FRONT_BIT)

#define __GL_BACK_MATERIAL_BITS ( \
                __GL_MATERIAL_COLORINDEX_BACK_BIT | \
                __GL_MATERIAL_EMISSION_BACK_BIT | \
                __GL_MATERIAL_SPECULAR_BACK_BIT | \
                __GL_MATERIAL_SHININESS_BACK_BIT | \
                __GL_MATERIAL_AMBIENT_BACK_BIT | \
                __GL_MATERIAL_DIFFUSE_BACK_BIT)

#define __GL_MATERIAL_BITS (__GL_FRONT_MATERIAL_BITS | __GL_BACK_MATERIAL_BITS)

/* all lighting attributes are vertex shader attributes */
#define __GL_LIGHTING_VERTEXPROGRAM_BITS 0xFFFFFFFF
#define __GL_LIGHTING_FRAGMENTPROGRAM_BITS  0

/* Referenced by "gc->lightAttrState[0 .. (__GL_MAX_LIGHT_NUM-1)]".
 */
enum {
    __GL_LIGHT_ENDISABLE_BIT                    = (1 << 0),
    __GL_LIGHT_AMBIENT_BIT                      = (1 << 1),
    __GL_LIGHT_DIFFUSE_BIT                      = (1 << 2),
    __GL_LIGHT_SPECULAR_BIT                     = (1 << 3),
    __GL_LIGHT_POSITION_BIT                     = (1 << 4),
    __GL_LIGHT_CONSTANTATT_BIT                  = (1 << 5),
    __GL_LIGHT_LINEARATT_BIT                    = (1 << 6),
    __GL_LIGHT_QUADRATICATT_BIT                 = (1 << 7),
    __GL_LIGHT_SPOTDIRECTION_BIT                = (1 << 8),
    __GL_LIGHT_SPOTEXPONENT_BIT                 = (1 << 9),
    __GL_LIGHT_SPOTCUTOFF_BIT                   = (1 << 10)
};

/* below defines are for vertex(fragment) program */
#define __GL_LIGHT_ATTENUATION_BIT ( \
    __GL_LIGHT_CONSTANTATT_BIT | \
    __GL_LIGHT_LINEARATT_BIT | \
    __GL_LIGHT_QUADRATICATT_BIT | \
    __GL_LIGHT_SPOTEXPONENT_BIT)

#define __GL_LIGHT_SPOTD_CUTOFF_BIT ( \
    __GL_LIGHT_SPOTDIRECTION_BIT |\
    __GL_LIGHT_SPOTCUTOFF_BIT)

#define __GL_LIGHT_STATEKEY_BITS ( \
    __GL_LIGHT_ENDISABLE_BIT | \
    __GL_LIGHT_SPECULAR_BIT | \
    __GL_LIGHT_POSITION_BIT | \
    __GL_LIGHT_SPOTCUTOFF_BIT)

#define __GL_LIGHT_CONSTANT_BITS ( \
    __GL_LIGHT_AMBIENT_BIT | \
    __GL_LIGHT_DIFFUSE_BIT | \
    __GL_LIGHT_SPECULAR_BIT | \
    __GL_LIGHT_POSITION_BIT | \
    __GL_LIGHT_CONSTANTATT_BIT | \
    __GL_LIGHT_LINEARATT_BIT | \
    __GL_LIGHT_QUADRATICATT_BIT | \
    __GL_LIGHT_SPOTDIRECTION_BIT | \
    __GL_LIGHT_SPOTEXPONENT_BIT | \
    __GL_LIGHT_SPOTCUTOFF_BIT)

#define __GL_LIGHT_SRC_BITS ( \
    __GL_LIGHT_ENDISABLE_BIT | \
    __GL_LIGHT_AMBIENT_BIT | \
    __GL_LIGHT_DIFFUSE_BIT | \
    __GL_LIGHT_SPECULAR_BIT | \
    __GL_LIGHT_POSITION_BIT | \
    __GL_LIGHT_CONSTANTATT_BIT | \
    __GL_LIGHT_LINEARATT_BIT | \
    __GL_LIGHT_QUADRATICATT_BIT | \
    __GL_LIGHT_SPOTDIRECTION_BIT | \
    __GL_LIGHT_SPOTEXPONENT_BIT | \
    __GL_LIGHT_SPOTCUTOFF_BIT)



/* all lighting attributes are vertex shader attributes */
#define __GL_LIGHTSRC_VERTEXPROGRAM_BITS    0xFFFFFFFF
#define __GL_LIGHTSRC_FRAGMENTPROGRAM_BITS  0

/* Referenced by "gc->globalDirtyState[__GL_CLIP_ATTRS]".
 */
enum {
    __GL_CLIPPLANE0_BIT                     = (1 << 0),
    __GL_CLIPPLANE1_BIT                     = (1 << 1),
    __GL_CLIPPLANE2_BIT                     = (1 << 2),
    __GL_CLIPPLANE3_BIT                     = (1 << 3),
    __GL_CLIPPLANE4_BIT                     = (1 << 4),
    __GL_CLIPPLANE5_BIT                     = (1 << 5),
    __GL_CLIPPLANE6_BIT                     = (1 << 6),
    __GL_CLIPPLANE7_BIT                     = (1 << 7),
    __GL_CLIPPLANE8_BIT                     = (1 << 8),
    __GL_CLIPPLANE9_BIT                     = (1 << 9),
    __GL_CLIPPLANE10_BIT                    = (1 << 10),
    __GL_CLIPPLANE11_BIT                    = (1 << 11),
    __GL_CLIPPLANE12_BIT                    = (1 << 12),
    __GL_CLIPPLANE13_BIT                    = (1 << 13),
    __GL_CLIPPLANE14_BIT                    = (1 << 14),

    __GL_CLIPPLANE0_ENDISABLE_BIT           = (1 << 16),
    __GL_CLIPPLANE1_ENDISABLE_BIT           = (1 << 17),
    __GL_CLIPPLANE2_ENDISABLE_BIT           = (1 << 18),
    __GL_CLIPPLANE3_ENDISABLE_BIT           = (1 << 19),
    __GL_CLIPPLANE4_ENDISABLE_BIT           = (1 << 20),
    __GL_CLIPPLANE5_ENDISABLE_BIT           = (1 << 21),
    __GL_CLIPPLANE6_ENDISABLE_BIT           = (1 << 22),
    __GL_CLIPPLANE7_ENDISABLE_BIT           = (1 << 23),
    __GL_CLIPPLANE8_ENDISABLE_BIT           = (1 << 24),
    __GL_CLIPPLANE9_ENDISABLE_BIT           = (1 << 25),
    __GL_CLIPPLANE10_ENDISABLE_BIT          = (1 << 26),
    __GL_CLIPPLANE11_ENDISABLE_BIT          = (1 << 27),
    __GL_CLIPPLANE12_ENDISABLE_BIT          = (1 << 28),
    __GL_CLIPPLANE13_ENDISABLE_BIT          = (1 << 29),
    __GL_CLIPPLANE14_ENDISABLE_BIT          = (1 << 30)
};


#define __GL_CLIPPLANE_BITS (               \
        __GL_CLIPPLANE0_BIT    |            \
        __GL_CLIPPLANE1_BIT    |            \
        __GL_CLIPPLANE2_BIT    |            \
        __GL_CLIPPLANE3_BIT    |            \
        __GL_CLIPPLANE4_BIT    |            \
        __GL_CLIPPLANE5_BIT    |            \
        __GL_CLIPPLANE6_BIT    |            \
        __GL_CLIPPLANE7_BIT    |            \
        __GL_CLIPPLANE8_BIT    |            \
        __GL_CLIPPLANE9_BIT    |            \
        __GL_CLIPPLANE10_BIT   |            \
        __GL_CLIPPLANE11_BIT   |            \
        __GL_CLIPPLANE12_BIT   |            \
        __GL_CLIPPLANE13_BIT   |            \
        __GL_CLIPPLANE14_BIT                )

#define __GL_CLIPPLANE_ENDISABLE_BITS (     \
        __GL_CLIPPLANE0_ENDISABLE_BIT   |   \
        __GL_CLIPPLANE1_ENDISABLE_BIT   |   \
        __GL_CLIPPLANE2_ENDISABLE_BIT   |   \
        __GL_CLIPPLANE3_ENDISABLE_BIT   |   \
        __GL_CLIPPLANE4_ENDISABLE_BIT   |   \
        __GL_CLIPPLANE5_ENDISABLE_BIT   |   \
        __GL_CLIPPLANE6_ENDISABLE_BIT   |   \
        __GL_CLIPPLANE7_ENDISABLE_BIT   |   \
        __GL_CLIPPLANE8_ENDISABLE_BIT   |   \
        __GL_CLIPPLANE9_ENDISABLE_BIT   |   \
        __GL_CLIPPLANE10_ENDISABLE_BIT  |   \
        __GL_CLIPPLANE11_ENDISABLE_BIT  |   \
        __GL_CLIPPLANE12_ENDISABLE_BIT  |   \
        __GL_CLIPPLANE13_ENDISABLE_BIT  |   \
        __GL_CLIPPLANE14_ENDISABLE_BIT  )

#define __GL_CLIP_VERTEXPROGRAM_BITS    0
#define __GL_CLIP_FRAGMENTPROGRAM_BITS  0

/* Referenced by "gc->texUnitAttrState[0 .. (__GL_TEXTURE_MAX_UNITS-1)]".
 */
#define __GL_TEX_ENABLE_DIM_CHANGED_BIT         (__GL_ONE_64 << 0)
#define __GL_TEX_IMAGE_CONTENT_CHANGED_BIT      (__GL_ONE_64 << 1)
#define __GL_TEX_IMAGE_FORMAT_CHANGED_BIT       (__GL_ONE_64 << 2)
#define __GL_TEXTURE_TRANSFORM_BIT              (__GL_ONE_64 << 3)
#define __GL_TEX_UNIT_LODBIAS_BIT               (__GL_ONE_64 << 4)

#define __GL_TEXGEN_S_ENDISABLE_BIT             (__GL_ONE_64 << 5)
#define __GL_TEXGEN_T_ENDISABLE_BIT             (__GL_ONE_64 << 6)
#define __GL_TEXGEN_R_ENDISABLE_BIT             (__GL_ONE_64 << 7)
#define __GL_TEXGEN_Q_ENDISABLE_BIT             (__GL_ONE_64 << 8)
#define __GL_TEXGEN_S_BIT                       (__GL_ONE_64 << 9)
#define __GL_TEXGEN_T_BIT                       (__GL_ONE_64 << 10)
#define __GL_TEXGEN_R_BIT                       (__GL_ONE_64 << 11)
#define __GL_TEXGEN_Q_BIT                       (__GL_ONE_64 << 12)

#define __GL_TEXENV_MODE_BIT                    (__GL_ONE_64 << 13)
#define __GL_TEXENV_COLOR_BIT                   (__GL_ONE_64 << 14)
#define __GL_TEXENV_COMBINE_ALPHA_BIT           (__GL_ONE_64 << 15)
#define __GL_TEXENV_COMBINE_RGB_BIT             (__GL_ONE_64 << 16)
#define __GL_TEXENV_SOURCE0_RGB_BIT             (__GL_ONE_64 << 17)
#define __GL_TEXENV_SOURCE1_RGB_BIT             (__GL_ONE_64 << 18)
#define __GL_TEXENV_SOURCE2_RGB_BIT             (__GL_ONE_64 << 19)
#define __GL_TEXENV_SOURCE0_ALPHA_BIT           (__GL_ONE_64 << 20)
#define __GL_TEXENV_SOURCE1_ALPHA_BIT           (__GL_ONE_64 << 21)
#define __GL_TEXENV_SOURCE2_ALPHA_BIT           (__GL_ONE_64 << 22)
#define __GL_TEXENV_OPERAND0_RGB_BIT            (__GL_ONE_64 << 23)
#define __GL_TEXENV_OPERAND1_RGB_BIT            (__GL_ONE_64 << 24)
#define __GL_TEXENV_OPERAND2_RGB_BIT            (__GL_ONE_64 << 25)
#define __GL_TEXENV_OPERAND0_ALPHA_BIT          (__GL_ONE_64 << 26)
#define __GL_TEXENV_OPERAND1_ALPHA_BIT          (__GL_ONE_64 << 27)
#define __GL_TEXENV_OPERAND2_ALPHA_BIT          (__GL_ONE_64 << 28)
#define __GL_TEXENV_RGB_SCALE_BIT               (__GL_ONE_64 << 29)
#define __GL_TEXENV_ALPHA_SCALE_BIT             (__GL_ONE_64 << 30)
#define __GL_TEXENV_COORD_REPLACE_BIT           (__GL_ONE_64 << 31)

#define __GL_TEXPARAM_WRAP_S_BIT                (__GL_ONE_64 << 32)
#define __GL_TEXPARAM_WRAP_T_BIT                (__GL_ONE_64 << 33)
#define __GL_TEXPARAM_WRAP_R_BIT                (__GL_ONE_64 << 34)
#define __GL_TEXPARAM_MIN_FILTER_BIT            (__GL_ONE_64 << 35)
#define __GL_TEXPARAM_MAG_FILTER_BIT            (__GL_ONE_64 << 36)
#define __GL_TEXPARAM_BORDER_COLOR_BIT          (__GL_ONE_64 << 37)
#define __GL_TEXPARAM_PRIORITY_BIT              (__GL_ONE_64 << 38)
#define __GL_TEXPARAM_MIN_LOD_BIT               (__GL_ONE_64 << 39)
#define __GL_TEXPARAM_MAX_LOD_BIT               (__GL_ONE_64 << 40)
#define __GL_TEXPARAM_BASE_LEVEL_BIT            (__GL_ONE_64 << 41)
#define __GL_TEXPARAM_MAX_LEVEL_BIT             (__GL_ONE_64 << 42)
#define __GL_TEXPARAM_LOD_BIAS_BIT              (__GL_ONE_64 << 43)
#define __GL_TEXPARAM_DEPTH_TEX_MODE_BIT        (__GL_ONE_64 << 44)
#define __GL_TEXPARAM_COMPARE_MODE_BIT          (__GL_ONE_64 << 45)
#define __GL_TEXPARAM_COMPARE_FUNC_BIT          (__GL_ONE_64 << 46)
#define __GL_TEXPARAM_COMPARE_FAIL_VALUE_BIT    (__GL_ONE_64 << 47)
#define __GL_TEXPARAM_GENERATE_MIPMAP_BIT       (__GL_ONE_64 << 48)
#define __GL_TEXPARAM_MAX_ANISOTROPY_BIT        (__GL_ONE_64 << 49)




#define __GL_TEXGEN_BITS ( \
                __GL_TEXGEN_S_ENDISABLE_BIT | \
                __GL_TEXGEN_T_ENDISABLE_BIT | \
                __GL_TEXGEN_R_ENDISABLE_BIT | \
                __GL_TEXGEN_Q_ENDISABLE_BIT | \
                __GL_TEXGEN_S_BIT | \
                __GL_TEXGEN_T_BIT | \
                __GL_TEXGEN_R_BIT | \
                __GL_TEXGEN_Q_BIT)

#define __GL_TEXENV_BITS ( \
                __GL_TEXENV_MODE_BIT | \
                __GL_TEXENV_COLOR_BIT | \
                __GL_TEXENV_COMBINE_ALPHA_BIT | \
                __GL_TEXENV_COMBINE_RGB_BIT | \
                __GL_TEXENV_SOURCE0_RGB_BIT | \
                __GL_TEXENV_SOURCE1_RGB_BIT | \
                __GL_TEXENV_SOURCE2_RGB_BIT | \
                __GL_TEXENV_SOURCE0_ALPHA_BIT | \
                __GL_TEXENV_SOURCE1_ALPHA_BIT | \
                __GL_TEXENV_SOURCE2_ALPHA_BIT | \
                __GL_TEXENV_OPERAND0_RGB_BIT | \
                __GL_TEXENV_OPERAND1_RGB_BIT | \
                __GL_TEXENV_OPERAND2_RGB_BIT | \
                __GL_TEXENV_OPERAND0_ALPHA_BIT | \
                __GL_TEXENV_OPERAND1_ALPHA_BIT | \
                __GL_TEXENV_OPERAND2_ALPHA_BIT | \
                __GL_TEXENV_RGB_SCALE_BIT | \
                __GL_TEXENV_ALPHA_SCALE_BIT | \
                __GL_TEXENV_COORD_REPLACE_BIT)

#define __GL_TEXPARAMETER_BITS ( \
                __GL_TEXPARAM_WRAP_S_BIT | \
                __GL_TEXPARAM_WRAP_T_BIT | \
                __GL_TEXPARAM_WRAP_R_BIT | \
                __GL_TEXPARAM_MIN_FILTER_BIT | \
                __GL_TEXPARAM_MAG_FILTER_BIT | \
                __GL_TEXPARAM_BORDER_COLOR_BIT | \
                __GL_TEXPARAM_PRIORITY_BIT | \
                __GL_TEXPARAM_MIN_LOD_BIT | \
                __GL_TEXPARAM_MAX_LOD_BIT | \
                __GL_TEXPARAM_BASE_LEVEL_BIT | \
                __GL_TEXPARAM_MAX_LEVEL_BIT | \
                __GL_TEXPARAM_LOD_BIAS_BIT | \
                __GL_TEXPARAM_DEPTH_TEX_MODE_BIT | \
                __GL_TEXPARAM_COMPARE_MODE_BIT | \
                __GL_TEXPARAM_COMPARE_FUNC_BIT | \
                __GL_TEXPARAM_GENERATE_MIPMAP_BIT | \
                __GL_TEXPARAM_COMPARE_FAIL_VALUE_BIT | \
                __GL_TEXPARAM_MAX_ANISOTROPY_BIT)

#define __GL_TEXIMAGE_BITS ( \
                __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | \
                __GL_TEX_IMAGE_FORMAT_CHANGED_BIT )

/* Used when push/pop GL_TEXTURE_BIT */
#define __GL_TEX_UNIT_ALL_BITS ( \
                __GL_TEXGEN_BITS | \
                __GL_TEXENV_BITS | \
                __GL_TEX_UNIT_LODBIAS_BIT | \
                __GL_TEXPARAMETER_BITS | \
                __GL_TEXIMAGE_BITS)

/* Used when push/pop GL_ENABLE_BIT */
#define __GL_TEXTURE_ENABLE_BITS ( \
                __GL_TEX_ENABLE_DIM_CHANGED_BIT | \
                __GL_TEXGEN_S_ENDISABLE_BIT | \
                __GL_TEXGEN_T_ENDISABLE_BIT | \
                __GL_TEXGEN_R_ENDISABLE_BIT | \
                __GL_TEXGEN_Q_ENDISABLE_BIT)

#define __GL_TEX_VERTEXPROGRAM_BITS (__GL_TEXGEN_BITS|__GL_TEXTURE_TRANSFORM_BIT)
#define __GL_TEX_FRAGMENTPROGRAM_BITS  (__GL_TEXENV_BITS)

/* Referenced by "gc->swpDirtyState[__GL_PIPELINE_ATTRS]".
 */
enum {
    __GL_SWP_DRAWBUFFER_BIT                 = (1 << 0),
    __GL_SWP_READBUFFER_BIT                 = (1 << 1),
    __GL_SWP_PIXELSTORE_BIT                 = (1 << 2),
    __GL_SWP_PIXELTRANSFER_BIT              = (1 << 3),
    __GL_SWP_PIXELZOOM_BIT                  = (1 << 4),
    __GL_SWP_COLORTABLE_BIT                 = (1 << 5),
    __GL_SWP_POST_CONV_COLORTABLE_BIT       = (1 << 6),
    __GL_SWP_POST_MATRIX_COLORTABLE_BIT     = (1 << 7),
    __GL_SWP_CONVOLUTION_1D_BIT             = (1 << 8),
    __GL_SWP_CONVOLUTION_2D_BIT             = (1 << 9),
    __GL_SWP_SEPARABLE_2D_BIT               = (1 << 10),
    __GL_SWP_HISTOGRAM_BIT                  = (1 << 11),
    __GL_SWP_MINMAX_BIT                     = (1 << 12)
};

/* Referenced by gc->globalDirtyState[__GL_PROGRAM_ATTRS]
*/
enum{
    __GL_DIRTY_VERTEX_PROGRAM_ENABLE        = (1<<0), /* enable, disable  */
    __GL_DIRTY_VP_POINT_SIZE_ENABLE         = (1<<1),
    __GL_DIRTY_VP_TWO_SIDE_ENABLE           = (1<<2),
    __GL_DIRTY_FRAGMENT_PROGRAM_ENABLE      = (1<<3),
    __GL_DIRTY_VERTEX_PROGRAM_SWITCH        = (1<<4), /* program switched  */
    __GL_DIRTY_FRAGMENT_PROGRAM_SWITCH      = (1<<5),
    __GL_DIRTY_VERTEX_PROGRAM_ENV           = (1<<6), /* env or local changed  */
    __GL_DIRTY_FRAGMENT_PROGRAM_ENV         = (1<<7),
    __GL_DIRTY_VERTEX_PROGRAM_LOCAL         = (1<<8),
    __GL_DIRTY_FRAGMENT_PROGRAM_LOCAL       = (1<<9),
    __GL_DIRTY_PROGRAM_MATRIX               = (1<<10),
    __GL_DIRTY_GLSL_PROGRAM_SWITCH          = (1<<11), /* glsl program switch */
    __GL_DIRTY_GLSL_VS_ENABLE               = (1<<12), /* glsl vertex shader enabled */
    __GL_DIRTY_GLSL_FS_ENABLE               = (1<<13), /* glsl fragment shader enabled */
    __GL_DIRTY_GLSL_VS_SWITCH               = (1<<14), /* glsl vertex shader switched */
    __GL_DIRTY_GLSL_FS_SWITCH               = (1<<15), /* glsl fragment shader switched */
    __GL_DIRTY_GLSL_SAMPLER                 = (1<<16), /* glsl sampler mapping changed */
    __GL_DIRTY_VS_POINT_SIZE_ENABLE         = (1<<17),
    __GL_DIRTY_VS_TWO_SIDE_ENABLE           = (1<<18),
    __GL_DIRTY_GLSL_FS_OUTPUT               = (1<<19),
    __GL_DIRTY_GLSL_GS_ENABLE               = (1<<20), /* glsl geometry shader enabled */
    __GL_DIRTY_GLSL_GS_SWITCH               = (1<<21), /* glsl geometry shader switched */
    __GL_DIRTY_GLSL_UNIFORM                 = (1<<22), /* glsl uniform update */
};

#define __GL_PROGRAM_VP_FULLUPDATE \
    (__GL_DIRTY_VERTEX_PROGRAM_ENABLE | __GL_DIRTY_VERTEX_PROGRAM_SWITCH)

#define __GL_PROGRAM_FP_FULLUPDATE \
    (__GL_DIRTY_FRAGMENT_PROGRAM_ENABLE | __GL_DIRTY_FRAGMENT_PROGRAM_SWITCH)

#define __GL_GLSL_VS_FULLUPDATE \
    (__GL_DIRTY_GLSL_VS_ENABLE | __GL_DIRTY_GLSL_VS_SWITCH)

#define __GL_GLSL_GS_FULLUPDATE \
    (__GL_DIRTY_GLSL_GS_ENABLE | __GL_DIRTY_GLSL_GS_SWITCH)

#define __GL_GLSL_FS_FULLUPDATE \
    (__GL_DIRTY_GLSL_FS_ENABLE | __GL_DIRTY_GLSL_FS_SWITCH)

#define __GL_DIRTY_PROGRAM_VS_SWITCH \
    (__GL_DIRTY_GLSL_VS_ENABLE | __GL_DIRTY_GLSL_VS_SWITCH | \
     __GL_DIRTY_VERTEX_PROGRAM_ENABLE | __GL_DIRTY_VERTEX_PROGRAM_SWITCH)

#define __GL_DIRTY_PROGRAM_GS_SWITCH \
    (__GL_DIRTY_GLSL_GS_ENABLE | __GL_DIRTY_GLSL_GS_SWITCH)

#define __GL_DIRTY_PROGRAM_PS_SWITCH \
    (__GL_DIRTY_GLSL_FS_ENABLE | __GL_DIRTY_GLSL_FS_SWITCH | \
     __GL_DIRTY_FRAGMENT_PROGRAM_ENABLE | __GL_DIRTY_FRAGMENT_PROGRAM_SWITCH)


/* cache type flag */
#define __GL_IM_CACHE               0
#define __GL_DL_CACHE               1
#define __GL_VBO_CACHE              2

/* use for extra tex env, define it same as GL_POLYGON_STIPPLE, and it will not conflict with existing one */
#define __GL_STIPPLE GL_POLYGON_STIPPLE

struct __GLcontextRec {
    /*
    ** OS related interfaces; these *must* be the first members of this
    ** structure, because they are exposed to the outside world.
    */
    __GLimports imports;
    __GLexports exports;

    /* Pointer to the GL readable and drawable currently bound to this context.
    */
    __GLdrawablePrivate *readablePrivate;
    __GLdrawablePrivate *drawablePrivate;

    /*
    ** Mode information that describes the kind of buffers and rendering
    ** modes that this context manages.
    */
    __GLcontextModes modes;

    /* Implementation dependent constants.
    */
    __GLdeviceConstants constants;

    /*
    ** Current dispatch tables state.
    */
    GLuint currentDispatchOffset;
    GLuint savedDispatchOffset;
    __GLdispatchState immediateDispatchTable;
    __GLdispatchState listCompileDispatchTable;
    __GLdispatchState *currentImmediateTable;

    /*
    ** Current GL rendering mode GL_RENDER, GL_FEEDBACK, or GL_SELECT.
    */
    GLenum renderMode;

    /*
    ** GL states that are stackable (PushAttrib, PopAttrib).
    ** All of the current user controllable GL states are in "state".
    ** All of the committed GL states (device shadow states) are in "commitState".
    */
    __GLattribute state;
    __GLattribute commitState;
    __GLclientAttribute clientState;
    __GLattributeMachine attribute;

    /* GL attribute dirty masks for device pipeline.
     */
    GLuint64 texUnitAttrDirtyMask;
    GLuint64 texUnitAttrState[__GL_MAX_TEXTURE_UNITS];
    GLbitfield lightAttrState[__GL_MAX_LIGHT_NUMBER];
    GLbitfield globalDirtyState[__GL_DIRTY_ATTRS_END];

    /* GL attribute dirty masks for SW pipeline.
     */
    GLuint64 swpTexUnitAttrDirtyMask;
    GLuint64 swpTexUnitAttrState[__GL_MAX_TEXTURE_UNITS];
    GLbitfield swpLightAttrState[__GL_MAX_LIGHT_NUMBER];
    GLbitfield swpDirtyState[__GL_DIRTY_ATTRS_END];

    /*
    ** GL state management data structures.
    */
    __GLvertexMachine input;
    __GLevaluatorMachine eval;
    __GLtransformMachine transform;
    __GLqueryMachine query;
    __GLfeedbackMachine feedback;
    __GLselectMachine select;
    __GLpixelMachine pixel;
    __GLvertexArrayMachine vertexArray;
    __GLvertexStreamMachine vertexStreams;

    /*
    ** GL context sharable objects.
    */
    __GLdlistMachine dlist;
    __GLtextureMachine texture;
    __GLbufferObjectMachine bufferObject;
    /* lower level shader program management */
    __GLProgramMachine program;
    /* high level shader progream management */
    __GLshaderProgramMachine shaderProgram;
    /* framebuffer objects */
    __GLframebufObjMachine frameBuffer;

    /* The device pipeline interface.
    */
    __GLdevicePipeline dp;

#if SWP_PIPELINE_ENABLED
    /* The SW pipeline interface.
    */
    __GLswPipeline swp;
#endif

    /* The current pipeline pointer to either dp.ctx or swp.ctx.
    */
    __GLpipeline *pipeline;

    /* This structure is used  to store SWVS output result, share by SWP and DP.
    */
    __GLVSOutput vsOutputContainer;

    /* glmaterial (in_begin/end) emulation
    */
    __GLTnlAccumMachine tnlAccum;

    /* This pointers to buffer region list for this context.
    */
    GLvoid *bufferList;

    /* Thread hash id that can be used for thread specific data lookup.
    */
    GLuint thrHashId;

    /* Flag to indicate if multi-threaded rendering is ON or OFF.
    */
    GLuint multiThreadOn;

    /* Context bitmask flag for misc purposes.
    */
    GLuint flags;

    /* Most recent error code, or GL_NO_ERROR if no error has occurred
    ** since the last glGetError.
    */
    GLint error;

    /* The drawable change mask: resize, move, and etc*/
    GLuint changeMask;

};

extern GLvoid __glSetError(GLenum code);


#define __GL_VALIDATE_VERTEX_ARRAYS(gc) \
    (gc)->immediateDispatchTable.dispatch.ArrayElement = __glim_ArrayElement_Validate; \
    (gc)->immediateDispatchTable.dispatch.DrawArrays = __glim_DrawArrays_Validate; \
    (gc)->immediateDispatchTable.dispatch.DrawElements = __glim_DrawElements_Validate; \
    (gc)->vertexArray.formatChanged = GL_TRUE;


/* Macro to set inputMaskChanged */
#define __GL_INPUTMASK_CHANGED(gc) \
    if (GL_FALSE == (gc)->input.inputMaskChanged) \
    { \
        (gc)->input.inputMaskChanged = GL_TRUE; \
        (gc)->immediateDispatchTable.dispatch.ArrayElement = __glim_ArrayElement_Validate; \
        (gc)->immediateDispatchTable.dispatch.DrawArrays = __glim_DrawArrays_Validate; \
        (gc)->immediateDispatchTable.dispatch.DrawElements = __glim_DrawElements_Validate; \
    } \

#ifdef __cplusplus
extern "C" {
#endif

#if defined(USE_LENDIAN)

typedef union Mask64 {
    struct {
        long mask_0;
        long mask_1;
    };
    GLint64 value;
} Mask64;

#else

typedef union Mask64 {
    struct {
        long mask_1;
        long mask_0;
    };
    GLint64 value;
} Mask64;

#endif

/*
** NOTE: this self-defined _BitScanForward_64 has no return value, which is different from
** intrinsic _BitScanForward64. The reason is this function's return value is never used in our
** driver code. _bittestandreset_64 is the same.
*/
__GL_INLINE GLvoid _BitScanForward_64(unsigned long * Index, GLuint64 Mask)
{
    Mask64 mask64;

    mask64.value = Mask;
    if (!_BitScanForward(Index, mask64.mask_0) && _BitScanForward(Index, mask64.mask_1))
    {
        *Index += sizeof(GLint) * 8;
    }
}

__GL_INLINE GLvoid _bittestandreset_64(GLint64 *a, long b)
{
    Mask64 mask64;

    mask64.value = *a;
    if (b < sizeof(GLint) * 8)
        _bittestandreset(&(mask64.mask_0), b);
    else
        _bittestandreset(&(mask64.mask_1), b - sizeof(GLint) * 8);
    *a = mask64.value;
}

/*
** Compiler intrinsic to speed bit operation.
** Note : Linux need redefine those intrinsics to proper instructions.
*/

#define __GL_BitTest(x, y)              _bittest((long*)(x), (y))
#define __GL_BitTest64(x, y)            _bittest64((GLint64*)(x), (y))
#define __GL_BitTestAndSet(x, y)        _bittestandset((long*)(x), (y))
#define __GL_BitTestAndSet64(x, y)      _bittestandset64((GLint64*)(x), (y))
#define __GL_BitTestAndReset(x, y)      _bittestandreset((long*)(x), (y))

#ifdef _WIN64
#define __GL_BitTestAndReset64(x, y)    _bittestandreset64((GLint64*)(x), (y))
#else
#define __GL_BitTestAndReset64(x, y)    _bittestandreset_64((GLint64*)(x), (y))
#endif

#define __GL_BitScanForward(x, y)       _BitScanForward((unsigned long*)(x), (y))

#ifdef _WIN64
#define __GL_BitScanForward64(x, y)     _BitScanForward64((DWORD *)(x), (y))
#else
#define __GL_BitScanForward64(x, y)     _BitScanForward_64((unsigned long*)(x), (y))
#endif

#define __GL_BitScanReverse(x, y)       _BitScanReverse((unsigned long*)(x), (y))
#define __GL_BitScanReverse64(x, y)     _BitScanReverse((GLuint64*)(x), (y))

GLvoid __glDispatchDrawableChange(__GLcontext *gc);
GLvoid __glEvaluateFramebufferChange(__GLcontext *gc);

#ifdef __cplusplus
}
#endif

__GL_INLINE GLvoid __glEvaluateAttribDrawableChange(__GLcontext *gc)
{
    /*
    ** Drawable change must be dispatched before evaluating the attribute change.
    */
    __glEvaluateFramebufferChange(gc);

    __glDispatchDrawableChange(gc);

    /*
    ** When app really want to draw to front buffer, sync front buffer to fake front buffer.
    */
    (*gc->dp.syncFrontToFakeFront)(gc);

    if (gc->globalDirtyState[__GL_ALL_ATTRS])
    {
        __glEvaluateAttributeChange(gc);
    }
}

#endif /* __gc_gl_context_h_ */
