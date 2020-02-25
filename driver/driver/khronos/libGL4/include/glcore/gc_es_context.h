/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_gl_context_h__
#define __gc_gl_context_h__

#define OPENGL40    1

/* Some platform (win64) predefined such macro, which make driver code confused. */
#if defined(MemoryBarrier)
#undef MemoryBarrier
#endif

#include "gc_es_types.h"
#include "gc_es_consts.h"
#include "gc_es_utils.h"
#include "gc_es_bitmask.h"
#include "gc_es_core.h"
#include "gc_es_thread.h"
#include "gc_es_dispatch.h"
#include "gc_es_debug.h"
#include "gc_es_sharedobj.h"
#include "gc_es_bufobj.h"
#include "gc_es_texture.h"
#include "gc_es_query.h"
#include "gc_es_shader.h"
#include "gc_es_extensions.h"
#include "gc_es_fbo.h"
#include "gc_es_vertex.h"
#ifdef OPENGL40
#include "gc_gl_pixel.h"
#include "gc_gl_lighting.h"
#include "gc_gl_eval.h"
#include "gc_gl_enable.h"
#include "gc_gl_vertex.h"
#include "gc_gl_feedback.h"
#include "gc_gl_xform.h"
#include "gc_gl_dlist.h"
#include "gc_gl_select.h"
#include "gc_gl_image.h"
#endif
#include "gc_es_attrib.h"
#include "gc_es_devicepipe.h"
#include "gc_es_profiler.h"
#include "gc_egl.h"
#include "gc_egl_common.h"
#ifdef OPENGL40
#include "wintogl.h"
#include "viv_lock.h"
#endif

#ifdef OPENGL40
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

/*
** Current glBegin mode.
*/
typedef enum __GLbeginModeEnum {
    __GL_NOT_IN_BEGIN        = 0,
    __GL_IN_BEGIN            = 1,
    __GL_SMALL_LIST_BATCH    = 2,
} __GLbeginMode;

typedef struct __GLvertexInputRec {
    GLubyte *pointer;
    GLfloat *currentPtrDW;
    GLuint offsetDW;
    GLuint index;
    GLuint sizeDW;
} __GLvertexInput;

/* use for extra tex env, define it same as GL_POLYGON_STIPPLE, and it will not conflict with existing one */
#define __GL_STIPPLE GL_POLYGON_STIPPLE

/*
** State for managing the vertex machinery.
*/
typedef struct __GLvertexMachineRec {
    GLuint indexCount;
    GLuint lastVertexIndex;
    GLuint64 currentInputMask;
    GLuint inputMaskChanged;
    GLuint64 requiredInputMask;
    GLuint64 primInputMask;
    GLuint numberOfElements;
    __GLbeginMode beginMode;
    GLuint64 primElemSequence;
    GLuint64 primitiveFormat;
    GLuint64 preVertexFormat;
    GLuint64 vertexFormat;
    GLushort deferredAttribDirty;

    GLboolean inconsistentFormat;
    GLboolean indexPrimEnabled;

    GLuint connectVertexIndex[4];

    /* Primitive type for a primitive batch (The first primitive's primType) */
    GLenum primMode;

    /* Primitive type for the current primitive in the primitive batch */
    GLenum currentPrimMode;

    GLfloat *vertexDataBuffer;
    GLfloat *defaultDataBuffer;
    GLfloat *defaultDataBufEnd;
    GLfloat *currentDataBufPtr;
    GLfloat *primBeginAddr;
    GLushort *defaultIndexBuffer;
    GLushort *indexBuffer;

    GLint vertTotalStrideDW;

    union
    {
        struct
        {
            __GLvertexInput vertex;
            __GLvertexInput weight;
            __GLvertexInput normal;
            __GLvertexInput color;
            __GLvertexInput color2;
            __GLvertexInput fog;
            __GLvertexInput edgeflag;
            __GLvertexInput filler3;
            __GLvertexInput texture[__GL_MAX_TEXTURE_COORDS];
            __GLvertexInput attribute[__GL_MAX_PROGRAM_VERTEX_ATTRIBUTES];
        };
        __GLvertexInput currentInput[__GL_TOTAL_VERTEX_ATTRIBUTES];
    };

    __GLcurrentState shadowCurrent;

    /* instance number for drawArrayInstanceEXT or drawElementInstanceEXT */
   GLsizei primCount;

} __GLvertexMachine;

#endif
/*
** Referenced by "gc->globalDirtyState[]".
 */
enum
{
    __GL_ALL_ATTRS        = 0,      /* Non-zero following word => bit position */
    __GL_DIRTY_ATTRS_1    = 1,
    __GL_DIRTY_ATTRS_2    = 2,
#ifdef OPENGL40
    __GL_DIRTY_ATTRS_3    = 3,

    __GL_LIGHTING_ATTRS   = 4,

    __GL_LIGHT_SRC_ATTRS  = 5,      /* Light# attribute changed => bit position */

    __GL_CLIP_ATTRS       = 6,      /* bit 0~15: plane param; bit 16~31: endisable */

    __GL_PIPELINE_ATTRS   = 7,      /* Pipeline private dirty bits */

    __GL_PROGRAM_ATTRS    = 8,      /* program dirty bits  */

    __GL_TEX_UNIT_ATTRS   = 9,      /* __GL_TEX_UNIT_ATTRS does not occupy a UINT in globalDirtyState[] */

    __GL_IMG_UNIT_ATTRS   = 10,

    __GL_DIRTY_ATTRS_END  = __GL_TEX_UNIT_ATTRS
#else
    __GL_PROGRAM_ATTRS    = 3,      /* program dirty bits  */
    __GL_TEX_UNIT_ATTRS   = 4,      /* __GL_TEX_UNIT_ATTRS does not occupy a UINT in globalDirtyState[] */
    __GL_IMG_UNIT_ATTRS   = 5,      /* __GL_IMG_UNIT_ATTRS does not occupy a UNIT in globalDirtyState[] */
    __GL_DIRTY_ATTRS_END  = __GL_IMG_UNIT_ATTRS
#endif
};

/*
 * Macros that set global dirty attribute bits.
 */
#define __GL_SET_ATTR_DIRTY_BIT(gc, index, bit)                                 \
    (gc)->globalDirtyState[(index)] |= (bit);                                   \
    (gc)->globalDirtyState[__GL_ALL_ATTRS] |= (__GL_ONE_32 << (index))

#ifdef OPENGL40
#define __GL_SET_LIGHT_SRC_BIT(gc, lighti, bit)                                 \
    (gc)->lightAttrState[(lighti)] |= (bit);                                    \
    (gc)->globalDirtyState[__GL_LIGHT_SRC_ATTRS] |= (1 << (lighti));            \
    (gc)->globalDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_LIGHT_SRC_ATTRS)

#define __GL_SET_SWP_DIRTY_ATTR(gc, bit)                                        \
    (gc)->swpDirtyState[__GL_PIPELINE_ATTRS] |= (bit);                          \
    (gc)->swpDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_PIPELINE_ATTRS)
#endif

#define __GL_SET_TEX_UNIT_BIT(gc, unit, bit)                                    \
    (gc)->texUnitAttrState[(unit)] |= (bit);                                    \
    __glBitmaskSet(&((gc)->texUnitAttrDirtyMask), unit);                         \
    (gc)->globalDirtyState[__GL_ALL_ATTRS] |= (__GL_ONE_32 << __GL_TEX_UNIT_ATTRS)

#define __GL_SET_GLSL_SAMPLER_BIT(gc, samplerRegIdx)                            \
    __glBitmaskSet(&((gc)->shaderProgram.samplerMapDirty), samplerRegIdx);       \
    (gc)->globalDirtyState[__GL_PROGRAM_ATTRS] |= (__GL_DIRTY_GLSL_SAMPLER);    \
    (gc)->globalDirtyState[__GL_ALL_ATTRS] |= (__GL_ONE_32 << __GL_PROGRAM_ATTRS)


#define __GL_SET_IMG_UNIT_BIT(gc, unit)                                        \
    __glBitmaskSet(&((gc)->imageUnitDirtyMask), unit);                          \
    (gc)->globalDirtyState[__GL_ALL_ATTRS] |= (__GL_ONE_32 << __GL_IMG_UNIT_ATTRS)

/*
** Referenced by "gc->globalDirtyState[__GL_DIRTY_ATTRS_1]".
 */
enum
{
    __GL_BLENDCOLOR_BIT                         = (1 << 0),
    __GL_BLENDFUNC_BIT                          = (1 << 1),
    __GL_BLENDEQUATION_BIT                      = (1 << 2),
    __GL_BLEND_ENDISABLE_BIT                    = (1 << 3),
    __GL_COLORMASK_BIT                          = (1 << 5),
    __GL_DEPTHRANGE_BIT                         = (1 << 6),
    __GL_DEPTHFUNC_BIT                          = (1 << 7),
    __GL_DEPTHMASK_BIT                          = (1 << 8),
    __GL_DEPTHTEST_ENDISABLE_BIT                = (1 << 9),
    __GL_STENCILFUNC_FRONT_BIT                  = (1 << 10),
    __GL_STENCILFUNC_BACK_BIT                   = (1 << 11),
    __GL_STENCILOP_FRONT_BIT                    = (1 << 12),
    __GL_STENCILOP_BACK_BIT                     = (1 << 13),
    __GL_STENCILMASK_FRONT_BIT                  = (1 << 14),
    __GL_STENCILMASK_BACK_BIT                   = (1 << 15),
    __GL_STENCILTEST_ENDISABLE_BIT              = (1 << 16),
    __GL_FRONTFACE_BIT                          = (1 << 17),
    __GL_CULLFACE_BIT                           = (1 << 18),
    __GL_CULLFACE_ENDISABLE_BIT                 = (1 << 19),
    __GL_POLYGONOFFSET_BIT                      = (1 << 20),
    __GL_POLYGONOFFSET_FILL_ENDISABLE_BIT       = (1 << 21),
    __GL_RASTERIZER_DISCARD_ENDISABLE_BIT       = (1 << 22),
#ifdef OPENGL40
    __GL_POLYGONMODE_BIT                        = (1 << 23),
    __GL_POLYGONOFFSET_POINT_ENDISABLE_BIT      = (1 << 24),
    __GL_POLYGONOFFSET_LINE_ENDISABLE_BIT       = (1 << 25),
    __GL_POLYGONSMOOTH_ENDISABLE_BIT            = (1 << 26),
    __GL_POLYGONSTIPPLE_BIT                     = (1 << 27),
    __GL_POLYGONSTIPPLE_ENDISABLE_BIT           = (1 << 28),
    __GL_ALPHAFUNC_BIT                          = (1 << 29),
    __GL_ALPHATEST_ENDISABLE_BIT                = (1 << 30),
#endif
};

#define __GL_ALPHABLEND_ATTR_BITS ( \
                __GL_BLENDCOLOR_BIT | \
                __GL_BLENDFUNC_BIT | \
                __GL_BLENDEQUATION_BIT | \
                __GL_BLEND_ENDISABLE_BIT)

#define __GL_COLORBUF_ATTR_BITS ( \
                __GL_ALPHAFUNC_BIT | \
                __GL_ALPHATEST_ENDISABLE_BIT | \
                __GL_BLENDCOLOR_BIT | \
                __GL_BLENDFUNC_BIT | \
                __GL_BLENDEQUATION_BIT | \
                __GL_BLEND_ENDISABLE_BIT | \
                __GL_COLORMASK_BIT)


#define __GL_DEPTHBUF_ATTR_BITS ( \
                __GL_DEPTHFUNC_BIT | \
                __GL_DEPTHMASK_BIT | \
                __GL_DEPTHTEST_ENDISABLE_BIT)


#define __GL_STENCIL_ATTR_BITS ( \
                __GL_STENCILFUNC_FRONT_BIT | \
                __GL_STENCILFUNC_BACK_BIT | \
                __GL_STENCILOP_FRONT_BIT | \
                __GL_STENCILOP_BACK_BIT | \
                __GL_STENCILMASK_FRONT_BIT | \
                __GL_STENCILMASK_BACK_BIT | \
                __GL_STENCILTEST_ENDISABLE_BIT)

#ifndef OPENGL40
#define __GL_POLYGON_ATTR_BITS ( \
                __GL_FRONTFACE_BIT | \
                __GL_CULLFACE_BIT | \
                __GL_CULLFACE_ENDISABLE_BIT | \
                __GL_POLYGONOFFSET_BIT | \
                __GL_POLYGONOFFSET_FILL_ENDISABLE_BIT)
#else
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
#endif

#ifdef OPENGL40
#define __GL_ATTR1_ENABLE_BITS    ( \
                __GL_POLYGONOFFSET_POINT_ENDISABLE_BIT | \
                __GL_POLYGONOFFSET_LINE_ENDISABLE_BIT | \
                __GL_POLYGONOFFSET_FILL_ENDISABLE_BIT | \
                __GL_POLYGONSMOOTH_ENDISABLE_BIT | \
                __GL_POLYGONSTIPPLE_ENDISABLE_BIT | \
                __GL_BLEND_ENDISABLE_BIT | \
                __GL_DEPTHTEST_ENDISABLE_BIT | \
                __GL_ALPHATEST_ENDISABLE_BIT | \
                __GL_CULLFACE_ENDISABLE_BIT | \
                __GL_STENCILTEST_ENDISABLE_BIT )

#endif

/*
** Referenced by "gc->globalDirtyState[__GL_DIRTY_ATTRS_2]".
 */
enum
{
    __GL_VIEWPORT_BIT                           = (1 << 0),
    __GL_SCISSOR_BIT                            = (1 << 1),
    __GL_SCISSORTEST_ENDISABLE_BIT              = (1 << 2),
    __GL_DITHER_ENDISABLE_BIT                   = (1 << 3),
    __GL_LINEWIDTH_BIT                          = (1 << 4),
    __GL_SAMPLECOVERAGE_BIT                     = (1 << 5),
    __GL_SAMPLE_ALPHA_TO_COVERAGE_ENDISABLE_BIT = (1 << 6),
    __GL_SAMPLE_COVERAGE_ENDISABLE_BIT          = (1 << 7),
    __GL_HINT_BIT                               = (1 << 8),
    __GL_PRIMITIVE_RESTART_BIT                  = (1 << 9),
    __GL_SAMPLE_MASK_BIT                        = (1 << 10),
    __GL_SAMPLE_MASK_ENDISABLE_BIT              = (1 << 11),
    __GL_SAMPLE_SHADING_ENDISABLE_BIT           = (1 << 12),
    __GL_SAMPLE_MIN_SHADING_VALUE_BIT           = (1 << 13),
#ifdef OPENGL40
    __GL_FOGCOLOR_BIT                           = (1 << 14),
    __GL_FOGINDEX_BIT                           = (1 << 15),
    __GL_FOGDENSITY_BIT                         = (1 << 16),
    __GL_FOGSTART_BIT                           = (1 << 17),
    __GL_FOGEND_BIT                             = (1 << 18),
    __GL_FOGMODE_BIT                            = (1 << 19),
    __GL_FOGCOORDSRC_BIT                        = (1 << 20),
    __GL_FOG_ENDISABLE_BIT                      = (1 << 21),
    __GL_PRIMMODE_BIT                           = (1 << 22),
    __GL_CLEARCOLOR_BIT                         = (1 << 23),
    __GL_CLEARACCUM_BIT                         = (1 << 24),
    __GL_DEPTHBOUNDTEST_BIT                     = (1 << 25),
    __GL_DEPTHBOUNDTESTENABLE_BIT               = (1 << 26),
    __GL_LOGICOP_BIT                            = (1 << 27),
    __GL_LOGICOP_ENDISABLE_BIT                  = (1 << 28),
    __GL_LINESMOOTH_ENDISABLE_BIT               = (1 << 29),
    __GL_LINESTIPPLE_BIT                        = (1 << 30),
    __GL_LINESTIPPLE_ENDISABLE_BIT              = (1 << 31),
#endif
};

#ifdef OPENGL40
#define __GL_ATTR2_ENABLE_BITS ( \
                __GL_SCISSORTEST_ENDISABLE_BIT | \
                __GL_DITHER_ENDISABLE_BIT | \
                __GL_FOG_ENDISABLE_BIT | \
                __GL_LOGICOP_ENDISABLE_BIT | \
                __GL_LINESMOOTH_ENDISABLE_BIT | \
                __GL_LINESTIPPLE_ENDISABLE_BIT | \
                __GL_SAMPLE_ALPHA_TO_COVERAGE_ENDISABLE_BIT | \
                __GL_SAMPLE_COVERAGE_ENDISABLE_BIT | \
                __GL_DEPTHBOUNDTESTENABLE_BIT)

#define __GL_DEPTHBUF_ATTR2_BITS ( \
                __GL_DEPTHBOUNDTEST_BIT | \
                __GL_DEPTHBOUNDTESTENABLE_BIT)

#define __GL_COLORBUF_ATTR2_BITS ( \
                __GL_LOGICOP_BIT | \
                __GL_LOGICOP_ENDISABLE_BIT | \
                __GL_CLEARCOLOR_BIT | \
                __GL_DITHER_ENDISABLE_BIT )

/* Referenced by "gc->globalDirtyState[__GL_DIRTY_ATTRS_3]".
 */
enum {
    __GL_RENDERMODE_BIT                             = (1 << 0),
    __GL_MODELVIEW_TRANSFORM_BIT                    = (1 << 1),
    __GL_PROJECTION_TRANSFORM_BIT                   = (1 << 2),
    __GL_NORMALIZE_ENDISABLE_BIT                    = (1 << 3),
    __GL_RESCALENORMAL_ENDISABLE_BIT                = (1 << 4),
    __GL_MULTISAMPLE_ENDISABLE_BIT                  = (1 << 5),
    __GL_MAP1_ENDISABLE_BIT                         = (1 << 6),
    __GL_MAP2_ENDISABLE_BIT                         = (1 << 7),
    __GL_AUTONORMAL_ENDISABLE_BIT                   = (1 << 8),
    __GL_COLORSUM_ENDISABLE_BIT                     = (1 << 9),
    __GL_SAMPLE_ALPHA_TO_ONE_ENDISABLE_BIT          = (1 << 10),
    __GL_POINTSIZE_BIT                              = (1 << 11),
    __GL_POINTSMOOTH_ENDISABLE_BIT                  = (1 << 12),
    __GL_POINT_SIZE_MIN_BIT                         = (1 << 13),
    __GL_POINT_SIZE_MAX_BIT                         = (1 << 14),
    __GL_POINT_FADE_THRESHOLD_SIZE_BIT              = (1 << 15),
    __GL_POINT_DISTANCE_ATTENUATION_BIT             = (1 << 16),
    __GL_POINTSPRITE_ENDISABLE_BIT                  = (1 << 17),
    __GL_POINTSPRITE_COORD_ORIGIN_BIT               = (1 << 18),
    __GL_CLAMP_VERTEX_COLOR_BIT                     = (1 << 19),
    __GL_CLAMP_FRAG_COLOR_BIT                       = (1 << 20),
};

#define __GL_ATTR3_ENABLE_BITS ( \
                __GL_NORMALIZE_ENDISABLE_BIT | \
                __GL_RESCALENORMAL_ENDISABLE_BIT | \
                __GL_MULTISAMPLE_ENDISABLE_BIT | \
                __GL_SAMPLE_ALPHA_TO_ONE_ENDISABLE_BIT | \
                __GL_MAP1_ENDISABLE_BIT | \
                __GL_MAP2_ENDISABLE_BIT | \
                __GL_AUTONORMAL_ENDISABLE_BIT | \
                __GL_COLORSUM_ENDISABLE_BIT | \
                __GL_POINTSMOOTH_ENDISABLE_BIT)

#define __GL_ATTR3_VERTEXPROGRAM_BITS ( \
    __GL_MODELVIEW_TRANSFORM_BIT | \
    __GL_PROJECTION_TRANSFORM_BIT | \
    __GL_NORMALIZE_ENDISABLE_BIT | \
    __GL_RESCALENORMAL_ENDISABLE_BIT)

#define __GL_FOG_ATTR_BITS    ( \
                __GL_FOGCOLOR_BIT | \
                __GL_FOGINDEX_BIT | \
                __GL_FOGDENSITY_BIT | \
                __GL_FOGSTART_BIT | \
                __GL_FOGEND_BIT | \
                __GL_FOGMODE_BIT | \
                __GL_FOGCOORDSRC_BIT | \
                __GL_FOG_ENDISABLE_BIT)

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

#endif

#define __GL_MULTISAMPLE_ATTR_BITS ( \
                __GL_SAMPLECOVERAGE_BIT | \
                __GL_SAMPLE_ALPHA_TO_COVERAGE_ENDISABLE_BIT | \
                __GL_SAMPLE_COVERAGE_ENDISABLE_BIT | \
                __GL_SAMPLE_MASK_BIT | \
                __GL_SAMPLE_MASK_ENDISABLE_BIT | \
                __GL_SAMPLE_SHADING_ENDISABLE_BIT | \
                __GL_SAMPLE_MIN_SHADING_VALUE_BIT)


/*
** Referenced by "gc->texUnitAttrState[0 .. (__GL_TEXTURE_MAX_UNITS-1)]".
 */
#define __GL_TEX_ENABLE_DIM_CHANGED_BIT         (__GL_ONE_64 <<  0)
#define __GL_TEX_IMAGE_CONTENT_CHANGED_BIT      (__GL_ONE_64 <<  1)
#define __GL_TEX_IMAGE_FORMAT_CHANGED_BIT       (__GL_ONE_64 <<  2)
#define __GL_TEXPARAM_WRAP_S_BIT                (__GL_ONE_64 <<  3)
#define __GL_TEXPARAM_WRAP_T_BIT                (__GL_ONE_64 <<  4)
#define __GL_TEXPARAM_WRAP_R_BIT                (__GL_ONE_64 <<  5)
#define __GL_TEXPARAM_MIP_HINT_BIT              (__GL_ONE_64 <<  6)
#define __GL_TEXPARAM_MIN_FILTER_BIT            (__GL_ONE_64 <<  7)
#define __GL_TEXPARAM_MAG_FILTER_BIT            (__GL_ONE_64 <<  8)
#define __GL_TEXPARAM_MIN_LOD_BIT               (__GL_ONE_64 <<  9)
#define __GL_TEXPARAM_MAX_LOD_BIT               (__GL_ONE_64 << 10)
#define __GL_TEXPARAM_BASE_LEVEL_BIT            (__GL_ONE_64 << 11)
#define __GL_TEXPARAM_MAX_LEVEL_BIT             (__GL_ONE_64 << 12)
#define __GL_TEXPARAM_COMPARE_MODE_BIT          (__GL_ONE_64 << 13)
#define __GL_TEXPARAM_COMPARE_FUNC_BIT          (__GL_ONE_64 << 14)
#define __GL_TEXPARAM_SWIZZLE_R_BIT             (__GL_ONE_64 << 15)
#define __GL_TEXPARAM_SWIZZLE_G_BIT             (__GL_ONE_64 << 16)
#define __GL_TEXPARAM_SWIZZLE_B_BIT             (__GL_ONE_64 << 17)
#define __GL_TEXPARAM_SWIZZLE_A_BIT             (__GL_ONE_64 << 18)
#define __GL_TEXPARAM_MAX_ANISOTROPY_BIT        (__GL_ONE_64 << 19)
#define __GL_TEXPARAM_DS_TEXMODE_BIT            (__GL_ONE_64 << 20)
#define __GL_TEXPARAM_SRGB_BIT                  (__GL_ONE_64 << 21)
#define __GL_TEXPARAM_BORDER_COLOR_BIT          (__GL_ONE_64 << 22)
#ifdef OPENGL40
#define __GL_TEXPARAM_PRIORITY_BIT              (__GL_ONE_64 << 23)
#define __GL_TEXPARAM_LOD_BIAS_BIT              (__GL_ONE_64 << 24)
#define __GL_TEXPARAM_DEPTH_TEX_MODE_BIT        (__GL_ONE_64 << 25)
#define __GL_TEXPARAM_COMPARE_FAIL_VALUE_BIT    (__GL_ONE_64 << 26)
#define __GL_TEXPARAM_GENERATE_MIPMAP_BIT       (__GL_ONE_64 << 27)
#define __GL_TEXTURE_TRANSFORM_BIT              (__GL_ONE_64 << 28)
#define __GL_TEXGEN_S_ENDISABLE_BIT             (__GL_ONE_64 << 29)
#define __GL_TEXGEN_T_ENDISABLE_BIT             (__GL_ONE_64 << 30)
#define __GL_TEXGEN_R_ENDISABLE_BIT             (__GL_ONE_64 << 31)
#define __GL_TEXGEN_Q_ENDISABLE_BIT             (__GL_ONE_64 << 32)
#define __GL_TEXGEN_S_BIT                       (__GL_ONE_64 << 33)
#define __GL_TEXGEN_T_BIT                       (__GL_ONE_64 << 34)
#define __GL_TEXGEN_R_BIT                       (__GL_ONE_64 << 35)
#define __GL_TEXGEN_Q_BIT                       (__GL_ONE_64 << 36)
#define __GL_TEXENV_MODE_BIT                    (__GL_ONE_64 << 37)
#define __GL_TEXENV_COLOR_BIT                   (__GL_ONE_64 << 38)
#define __GL_TEXENV_COMBINE_ALPHA_BIT           (__GL_ONE_64 << 39)
#define __GL_TEXENV_COMBINE_RGB_BIT             (__GL_ONE_64 << 40)
#define __GL_TEXENV_SOURCE0_RGB_BIT             (__GL_ONE_64 << 41)
#define __GL_TEXENV_SOURCE1_RGB_BIT             (__GL_ONE_64 << 42)
#define __GL_TEXENV_SOURCE2_RGB_BIT             (__GL_ONE_64 << 43)
#define __GL_TEXENV_SOURCE0_ALPHA_BIT           (__GL_ONE_64 << 44)
#define __GL_TEXENV_SOURCE1_ALPHA_BIT           (__GL_ONE_64 << 45)
#define __GL_TEXENV_SOURCE2_ALPHA_BIT           (__GL_ONE_64 << 46)
#define __GL_TEXENV_OPERAND0_RGB_BIT            (__GL_ONE_64 << 47)
#define __GL_TEXENV_OPERAND1_RGB_BIT            (__GL_ONE_64 << 48)
#define __GL_TEXENV_OPERAND2_RGB_BIT            (__GL_ONE_64 << 49)
#define __GL_TEXENV_OPERAND0_ALPHA_BIT          (__GL_ONE_64 << 50)
#define __GL_TEXENV_OPERAND1_ALPHA_BIT          (__GL_ONE_64 << 51)
#define __GL_TEXENV_OPERAND2_ALPHA_BIT          (__GL_ONE_64 << 52)
#define __GL_TEXENV_RGB_SCALE_BIT               (__GL_ONE_64 << 53)
#define __GL_TEXENV_ALPHA_SCALE_BIT             (__GL_ONE_64 << 54)
#define __GL_TEXENV_COORD_REPLACE_BIT           (__GL_ONE_64 << 55)
#define __GL_TEX_UNIT_LODBIAS_BIT               (__GL_ONE_64 << 56)
#define __GL_TEXTURE_BORDER_BIT                 (__GL_ONE_64 << 57)
#endif


#define __GL_TEXPARAMETER_SWIZZLE_BITS ( \
                __GL_TEXPARAM_SWIZZLE_R_BIT | \
                __GL_TEXPARAM_SWIZZLE_G_BIT | \
                __GL_TEXPARAM_SWIZZLE_B_BIT | \
                __GL_TEXPARAM_SWIZZLE_A_BIT)

#define __GL_TEXIMAGE_BITS ( \
                __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | \
                __GL_TEX_IMAGE_FORMAT_CHANGED_BIT )

#ifdef OPENGL40
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

#define __GL_SAMPLERPARAMETER_BITS ( \
                __GL_TEXPARAM_WRAP_S_BIT | \
                __GL_TEXPARAM_WRAP_T_BIT | \
                __GL_TEXPARAM_WRAP_R_BIT | \
                __GL_TEXPARAM_MIN_FILTER_BIT | \
                __GL_TEXPARAM_MAG_FILTER_BIT | \
                __GL_TEXPARAM_MIN_LOD_BIT | \
                __GL_TEXPARAM_MAX_LOD_BIT | \
                __GL_TEXPARAM_LOD_BIAS_BIT | \
                __GL_TEXPARAM_COMPARE_MODE_BIT |  \
                __GL_TEXPARAM_COMPARE_FUNC_BIT |  \
                __GL_TEXPARAM_MAX_ANISOTROPY_BIT | \
                __GL_TEXPARAM_SRGB_BIT | \
                __GL_TEXPARAM_BORDER_COLOR_BIT)

#define __GL_TEXPARAMETER_BITS ( \
                __GL_TEXPARAMETER_SWIZZLE_BITS | \
                __GL_SAMPLERPARAMETER_BITS | \
                __GL_TEXPARAM_PRIORITY_BIT | \
                __GL_TEXPARAM_BASE_LEVEL_BIT | \
                __GL_TEXPARAM_MAX_LEVEL_BIT | \
                __GL_TEXPARAM_DEPTH_TEX_MODE_BIT | \
                __GL_TEXPARAM_DS_TEXMODE_BIT | \
                __GL_TEXPARAM_GENERATE_MIPMAP_BIT | \
                __GL_TEXPARAM_MIP_HINT_BIT | \
                __GL_TEXPARAM_COMPARE_FAIL_VALUE_BIT)

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

#else

#define __GL_SAMPLERPARAMETER_BITS ( \
                __GL_TEXPARAM_WRAP_S_BIT | \
                __GL_TEXPARAM_WRAP_T_BIT | \
                __GL_TEXPARAM_WRAP_R_BIT | \
                __GL_TEXPARAM_MIN_FILTER_BIT | \
                __GL_TEXPARAM_MAG_FILTER_BIT | \
                __GL_TEXPARAM_MIN_LOD_BIT | \
                __GL_TEXPARAM_MAX_LOD_BIT | \
                __GL_TEXPARAM_COMPARE_MODE_BIT |  \
                __GL_TEXPARAM_COMPARE_FUNC_BIT |  \
                __GL_TEXPARAM_MAX_ANISOTROPY_BIT | \
                __GL_TEXPARAM_SRGB_BIT | \
                __GL_TEXPARAM_BORDER_COLOR_BIT)

#define __GL_TEXPARAMETER_BITS ( \
                __GL_TEXPARAMETER_SWIZZLE_BITS | \
                __GL_SAMPLERPARAMETER_BITS | \
                __GL_TEXPARAM_MIP_HINT_BIT | \
                __GL_TEXPARAM_BASE_LEVEL_BIT | \
                __GL_TEXPARAM_MAX_LEVEL_BIT |\
                __GL_TEXPARAM_DS_TEXMODE_BIT)

#endif


/*
** Referenced by gc->globalDirtyState[__GL_PROGRAM_ATTRS]
*/
enum
{
    __GL_DIRTY_GLSL_VS_SWITCH               = (1 << 0), /* glsl vs switched */
    __GL_DIRTY_GLSL_FS_SWITCH               = (1 << 1), /* glsl fs switched */
    __GL_DIRTY_GLSL_CS_SWITCH               = (1 << 2), /* glsl cs switched */
    __GL_DIRTY_GLSL_SAMPLER                 = (1 << 3), /* glsl sampler mapping changed */
    __GL_DIRTY_GLSL_UNIFORM                 = (1 << 4), /* glsl uniform update */
    __GL_DIRTY_GLSL_FS_OUTPUT               = (1 << 5), /* useless */
    __GL_DIRTY_GLSL_MODE_SWITCH             = (1 << 6), /* glsl mode switched */
    __GL_DIRTY_GLSL_TCS_SWITCH              = (1 << 7), /* glsl tcs switched */
    __GL_DIRTY_GLSL_TES_SWITCH              = (1 << 8), /* glsl tes siwtched */
    __GL_DIRTY_GLSL_PATCH_VERTICES          = (1 << 9), /* tcs input patch vertices count */
    __GL_DIRTY_GLSL_GS_SWITCH               = (1 << 10),/* glsl gs switched */
#ifdef OPENGL40
    __GL_DIRTY_PROGRAM_MATRIX               = (1 << 11),
    __GL_DIRTY_VERTEX_PROGRAM_ENABLE        = (1 << 12), /* enable, disable  */
    __GL_DIRTY_VP_POINT_SIZE_ENABLE         = (1 << 13),
    __GL_DIRTY_VP_TWO_SIDE_ENABLE           = (1 << 14),
    __GL_DIRTY_FRAGMENT_PROGRAM_ENABLE      = (1 << 15),
    __GL_DIRTY_VERTEX_PROGRAM_SWITCH        = (1 << 16), /* program switched  */
    __GL_DIRTY_FRAGMENT_PROGRAM_SWITCH      = (1 << 17),
    __GL_DIRTY_VERTEX_PROGRAM_ENV           = (1 << 18), /* env or local changed  */
    __GL_DIRTY_FRAGMENT_PROGRAM_ENV         = (1 << 19),
    __GL_DIRTY_VERTEX_PROGRAM_LOCAL         = (1 << 20),
    __GL_DIRTY_FRAGMENT_PROGRAM_LOCAL       = (1 << 21),
    __GL_DIRTY_GLSL_VS_ENABLE               = (1 << 22), /* glsl vertex shader enabled */
    __GL_DIRTY_GLSL_FS_ENABLE               = (1 << 23), /* glsl fragment shader enabled */
    __GL_DIRTY_VS_POINT_SIZE_ENABLE         = (1 << 24),
    __GL_DIRTY_VS_TWO_SIDE_ENABLE           = (1 << 25),
    __GL_DIRTY_GLSL_GS_ENABLE               = (1 << 26), /* glsl geometry shader enabled */
#endif
};

#define __GL_DIRTY_GLSL_PROGRAM_SWITCH ( \
                __GL_DIRTY_GLSL_VS_SWITCH  | \
                __GL_DIRTY_GLSL_FS_SWITCH  | \
                __GL_DIRTY_GLSL_CS_SWITCH  | \
                __GL_DIRTY_GLSL_TCS_SWITCH | \
                __GL_DIRTY_GLSL_TES_SWITCH | \
                __GL_DIRTY_GLSL_GS_SWITCH)

#define __GL_DIRTY_GLSL_GPIPE_SWITCH ( \
                __GL_DIRTY_GLSL_VS_SWITCH  | \
                __GL_DIRTY_GLSL_TCS_SWITCH | \
                __GL_DIRTY_GLSL_TES_SWITCH | \
                __GL_DIRTY_GLSL_GS_SWITCH)



#define __GL_PROGRAM_SET_PROGRAMMATRIX_DIRTY(gc,index) \
    gc->globalDirtyState[__GL_PROGRAM_ATTRS] |= (__GL_DIRTY_PROGRAM_MATRIX);\
    gc->globalDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_PROGRAM_ATTRS);\
    gc->program.programMatrix |= (index);

#ifdef OPENGL40
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
#endif
/*
** Referenced by gc->drawableDirtyMask
*/
enum
{
    __GL_BUFFER_DRAW_BIT = (1 << 0),
    __GL_BUFFER_READ_BIT = (1 << 1),
};

#define __GL_BUFFER_DRAW_READ_BITS (__GL_BUFFER_DRAW_BIT | __GL_BUFFER_READ_BIT)


#define __GL_CHECK_ATTR1(bit, attr1)                \
    if (localMask & bit)                            \
    {                                               \
        if (ds->attr1 != cs->attr1)                 \
        {                                           \
            ds->attr1 = cs->attr1;                  \
        }                                           \
        else                                        \
        {                                           \
            localMask &= ~bit;                      \
        }                                           \
    }

#define __GL_CHECK_ATTR2(bit, attr1, attr2)         \
    if (localMask & bit)                            \
    {                                               \
        if (ds->attr1 != cs->attr1 ||               \
            ds->attr2 != cs->attr2)                 \
        {                                           \
            ds->attr1 = cs->attr1;                  \
            ds->attr2 = cs->attr2;                  \
        }                                           \
        else                                        \
        {                                           \
            localMask &= ~bit;                      \
        }                                           \
    }

#define __GL_CHECK_ATTR3(bit, attr1, attr2, attr3)  \
    if (localMask & bit)                            \
    {                                               \
        if (ds->attr1 != cs->attr1 ||               \
            ds->attr2 != cs->attr2 ||               \
            ds->attr3 != cs->attr3)                 \
        {                                           \
            ds->attr1 = cs->attr1;                  \
            ds->attr2 = cs->attr2;                  \
            ds->attr3 = cs->attr3;                  \
        }                                           \
        else                                        \
        {                                           \
            localMask &= ~bit;                      \
        }                                           \
    }

#define __GL_CHECK_ATTR4(bit, attr1, attr2, attr3, attr4) \
    if (localMask & bit)                            \
    {                                               \
        if (ds->attr1 != cs->attr1 ||               \
            ds->attr2 != cs->attr2 ||               \
            ds->attr3 != cs->attr3 ||               \
            ds->attr4 != cs->attr4)                 \
        {                                           \
            ds->attr1 = cs->attr1;                  \
            ds->attr2 = cs->attr2;                  \
            ds->attr3 = cs->attr3;                  \
            ds->attr4 = cs->attr4;                  \
        }                                           \
        else                                        \
        {                                           \
            localMask &= ~bit;                      \
        }                                           \
    }

#define __GL_CHECK_TEX_PARAM1(bit, attr)            \
    if (localMask & bit)                            \
    {                                               \
        if (ds_params->attr != cs_params->attr)     \
        {                                           \
            ds_params->attr = cs_params->attr;      \
        }                                           \
        else                                        \
        {                                           \
            localMask &= ~bit;                      \
        }                                           \
    }

#define __GL_CHECK_SAMPLER_PARAM1(bit, attr)        \
if (localMask & bit)                                \
{                                                   \
    if (ds_params->sampler.attr != cs_params->attr) \
    {                                               \
        ds_params->sampler.attr = cs_params->attr;  \
    }                                               \
    else                                            \
    {                                               \
        localMask &= ~bit;                          \
    }                                               \
}

#define __GL_CHECK_SAMPLER_PARAM_ARRAY(bit, attr, arraySize, bpe)        \
if (localMask & bit)                                \
{ \
    if (__GL_MEMCMP(ds_params->sampler.attr, cs_params->attr, arraySize * bpe)) \
    { \
        __GL_MEMCOPY(ds_params->sampler.attr, cs_params->attr, arraySize * bpe); \
    }                                               \
    else                                            \
    {                                               \
        localMask &= ~bit;                          \
    }                                               \
}

/*
** Procedures which are exported by the GLcore to the surroundings,
** so that it can manage multiple GL context's.
*/
typedef struct __GLexportsRec
{
    /* Context management (return GL_FALSE on failure) */
    GLvoid* (*createContext)(GLint clientVersion, VEGLimports *imports, GLvoid* sharedCtx);
    GLboolean (*destroyContext)(GLvoid *gc);
    GLvoid  (*setDrawable)(__GLcontext* gc, __GLdrawablePrivate* drawable, __GLdrawablePrivate* readable);
    GLboolean (*makeCurrent)(__GLcontext *gc, __GLdrawablePrivate* drawable, __GLdrawablePrivate* readable, GLboolean flushDrawChange);
    GLboolean (*loseCurrent)(__GLcontext *gc, __GLdrawablePrivate* drawable, __GLdrawablePrivate* readable);
    __GLthreadPriv* (*getThreadData)(GLvoid *eglThreadData);

} __GLexports;

#if defined(OPENGL40) && defined(DRI_PIXMAPRENDER_GL)
/*
** This must be the first member of a __GLcontext structure.  This is the
** only part of a context that is exposed to the outside world; everything
** else is opaque.
*/
typedef struct __GLcontextInterfaceRec {
    /* The first Int of __GLcontext stores a unique context ID */
    GLuint magic;
    VEGLimports imports;
    __GLexports exports;
} __GLcontextInterface;
#endif

typedef enum __GLApiVersionRec
{
    __GL_API_VERSION_INVALID = 0,
    __GL_API_VERSION_ES20 = 200,
    __GL_API_VERSION_ES30 = 300,
    __GL_API_VERSION_ES31 = 310,
    __GL_API_VERSION_ES32 = 320,

    __GL_API_VERSION_OGL10  = 0x10000,
    __GL_API_VERSION_OGL11  = 0x10100,
    __GL_API_VERSION_OGL12  = 0x10200,
    __GL_API_VERSION_OGL13  = 0x10300,
    __GL_API_VERSION_OGL14  = 0x10400,
    __GL_API_VERSION_OGL15  = 0x10500,
    __GL_API_VERSION_OGL20  = 0x20000,
    __GL_API_VERSION_OGL21  = 0x20100,
    __GL_API_VERSION_OGL30  = 0x30000,
    __GL_API_VERSION_OGL31  = 0x30100,
    __GL_API_VERSION_OGL32  = 0x30200,
    __GL_API_VERSION_OGL33  = 0x30300,
    __GL_API_VERSION_OGL40  = 0x40000,
} __GLApiVersion;


#define __glApiEnum(apiname)  enum_gl##apiname

typedef enum
{
    __GL_API_ENTRIES(__glApiEnum)
} __glApiEnum;

#if gcdPATTERN_FAST_PATH
typedef struct __GLapiInfoRec
{
    __glApiEnum apiEnum;
    GLuint param[4];
} __GLapiInfo;

typedef struct __GLapiPatternRec
{
    __GLapiInfo api[32];
    GLuint apiCount;
} __GLapiPattern;

typedef enum
{
    GLES_PATTERN_STATE_CHECK,
    GLES_PATTERN_STATE_MATCHED,
}__GLpatternState;

typedef struct __GLapiPatternMatchineRec
{
    GLboolean enable;
    __GLapiPattern * patterns[2];
    /* if we add match check for all APIs, this don't need,
       or, this record the sub-APIs match count, which we need compare with apiCount,
       if not equal, it means it fall to some function not in match table.
    */
    GLuint matchCount;
    GLuint apiCount;
    __GLapiPattern * matchPattern;
    __GLapiPattern * lastPattern;
    GLuint patternMatchMask;
    __GLpatternState state;
}__GLapiPatternMatchine;

extern __GLapiPattern gfxPattern0;
extern __GLapiPattern gfxPattern1;

typedef enum
{
    GLES_PATTERN_GFX0  = 0,
    GLES_PATTERN_GFX1  = 1,
    GLES_PATTERN_COUNT = 2,
}__GLpatternEnum;
#endif

#define ES3X_MAGIC  gcmCC('e', 's', '3', 'x')

struct __GLcontextRec
{
    gcsDRIVER_TLS base;

    GLuint magic;

    /* EGL imported functions which might be OS specific */
    VEGLimports imports;

    /* GLcore exported functions for other layers' use */
    __GLexports exports;

    /* Pointer to the GL readable and drawable currently bound to this context. */
    __GLdrawablePrivate *readablePrivate;
    __GLdrawablePrivate *drawablePrivate;

    /* Record which context this one will share with */
    __GLcontext *shareCtx;

    /* GL_CONTEXT_FLAGS. */
    GLbitfield contextFlags;

    /*
    ** Mode information that describes the kind of buffers and rendering
    ** modes that this context manages.
    */
    __GLcontextModes modes;

    /* Original client version app requested when creating EGL context */
    __GLApiVersion apiVersion;

    /* Implementation dependent constants. */
    __GLdeviceConstants constants;


#ifdef OPENGL40
    __GLevaluatorMachine eval;
    __GLdlistMachine dlist;
    __GLattributeMachine attribute;
    __GLdispatchTable dlCompileDispatch;
#endif
    __GLdispatchTable immedModeDispatch;

    /* Enabled API level profiling? */
    GLboolean apiProfile;
    /* Current mode dispatch table:  can point to immediate or dlist-compile dispatch table */
    __GLdispatchTable *pModeDispatch;
    /* Current entry dispatch table: can point to mode, profiler or nop dispatch table. */
    __GLdispatchTable *pEntryDispatch;


    /*
    ** All of the current user controllable GL server states are in "state".
    ** All of the committed GL states (device shadow states) are in "commitState".
    */
    __GLattribute state;
    __GLattribute commitState;

    /* Sometimes commit states may be invalid and cannot be used to evaluate attrib changes.
    ** This happens when there is intermediate error in a draw, which cause part or all of
    ** the states in fact were not flushed to HW.
    */
    GLboolean invalidCommonCommit;  /* Mark invalid for committed states of both draw and compute */
    GLboolean invalidDrawCommit;    /* Mark invalid for committed states of draw only */

    /* GL client state for vertex array and pixels */
    __GLclientAttribute clientState;

    /* GL attribute dirty masks for device pipeline.
     */
    __GLbitmask texUnitAttrDirtyMask;
    __GLbitmask imageUnitDirtyMask;
#ifdef OPENGL40
    GLbitfield lightAttrState[__GL_MAX_LIGHT_NUMBER];
#endif
    GLuint64   texUnitAttrState[__GL_MAX_TEXTURE_UNITS];
    GLbitfield globalDirtyState[__GL_DIRTY_ATTRS_END];
    GLbitfield drawableDirtyMask;

    /*
    ** GL state management data structures.
    */
#ifdef OPENGL40

    GLbitfield swpLightAttrState[__GL_MAX_LIGHT_NUMBER];
    GLbitfield swpDirtyState[__GL_DIRTY_ATTRS_END];

     GLenum renderMode;
     GLboolean conditionalRenderDiscard;
    __GLvertexMachine input;
    __GLfeedbackMachine feedback;
    __GLvertexStreamMachine vertexStreams;
    __GLtransformMachine transform;
    /* lower level shader program management */

    __GLProgramMachine program;
    __GLTnlAccumMachine tnlAccum;
    __GLselectMachine select;
#endif
    __GLvertexArrayMachine vertexArray;
    __GLtextureMachine texture;
    __GLsamplerMachine sampler;
    __GLbufferObjectMachine bufferObject;
    __GLshaderProgramMachine shaderProgram;
    __GLframebufObjMachine frameBuffer;
    __GLxfbMachine xfb;
    __GLqueryMachine query;
    __GLsyncMachine sync;
    __GLcomputeMachine compute;

    __GLdebugMachine debug;

    /* The device pipeline interface */
    __GLdevicePipeline dp;

    /* Context bitmask flag for misc purposes.
    */
    GLuint flags;

    /* Most recent error code, or GL_NO_ERROR if no error has occurred
    ** since the last glGetError.
    */
    GLint error;

#if VIVANTE_PROFILER
    /* Profiler. */
    glsPROFILER         profiler;
#endif

#if gcdPATTERN_FAST_PATH
    __GLapiPatternMatchine      pattern;
#endif

#ifdef OPENGL40
    /* The drawable change mask: resize, move, and etc*/
    GLuint changeMask;
#endif
};


extern GLvoid __glSetError(__GLcontext *gc, GLenum code);

#ifdef OPENGL40
#define __GL_VALIDATE_VERTEX_ARRAYS(gc) \
    (gc)->vertexArray.formatChanged = GL_TRUE; \
    (gc)->immedModeDispatch.ArrayElement = __glim_ArrayElement_Validate; \
    (gc)->immedModeDispatch.DrawArrays = __glim_DrawArrays_Validate; \
    (gc)->immedModeDispatch.DrawElements = __glim_DrawElements_Validate; \


/* Macro to set inputMaskChanged */
#define __GL_INPUTMASK_CHANGED(gc) \
    if (GL_FALSE == (gc)->input.inputMaskChanged) \
    { \
        (gc)->input.inputMaskChanged = GL_TRUE; \
        (gc)->immedModeDispatch.ArrayElement = __glim_ArrayElement_Validate; \
        (gc)->immedModeDispatch.DrawArrays = __glim_DrawArrays_Validate; \
        (gc)->immedModeDispatch.DrawElements = __glim_DrawElements_Validate; \
    } \

#endif

#define __GL_HEADER         gcmHEADER
#define __GL_HEADER_ARG     gcmHEADER_ARG

#define __GL_FOOTER         gcmFOOTER_NO
#define __GL_FOTTER_ARG     gcmFOOTER_ARG

#define __GL_ERROR(err)                 \
    __glSetError(gc, err);

#define __GL_ERROR_EXIT(err)            \
    __glSetError(gc, err);              \
    goto OnError;

#define __GL_ERROR_RET(err)             \
    __glSetError(gc, err);              \
    return;

/* Pop stack before return, or will waste stack */
#define __GL_ERROR_RET_STACK(err)             \
    __GL_FOOTER();                      \
    __glSetError(gc, err);              \
    return;

#define __GL_ERROR_RET_VAL(err, val)    \
    __glSetError(gc, err);              \
    return (val);

#define __GL_EXIT()                     \
    goto OnExit;

#define __GL_ERROR_EXIT2()             \
    goto OnError;



__GL_INLINE __GLcontext * __glGetGLcontext(GLvoid)
{
    gcsDRIVER_TLS_PTR tls = NULL;
    gcoOS_GetDriverTLS(gcvTLS_KEY_OPENGL, &tls);
    return (__GLcontext *) tls;
}

__GL_INLINE GLvoid __glSetGLcontext(GLvoid *context)
{
    gcoOS_SetDriverTLS(gcvTLS_KEY_OPENGL, (gcsDRIVER_TLS_PTR) context);
}

extern GLvoid __glEvaluateFramebufferChange(__GLcontext *gc, GLbitfield flags);
#if defined(OPENGL40) && defined(DRI_PIXMAPRENDER_GL)
extern GLvoid __glDispatchDrawableChange(__GLcontext *gc);
#endif
__GL_INLINE GLvoid __glEvaluateDrawableChange(__GLcontext *gc, GLbitfield flags)
{
    /* Evaluate context frame buffer object status, include default ones. */
    __glEvaluateFramebufferChange(gc, flags);

    if ((gc->drawableDirtyMask & __GL_BUFFER_DRAW_BIT)&& (flags & __GL_BUFFER_DRAW_BIT))
    {
        if (!(*gc->dp.changeDrawBuffers)(gc))
        {
            __GL_ERROR((*gc->dp.getError)(gc));
        }
        gc->drawableDirtyMask &= ~__GL_BUFFER_DRAW_BIT;
    }

    if ((gc->drawableDirtyMask & __GL_BUFFER_READ_BIT) && (flags & __GL_BUFFER_READ_BIT))
    {
        if (!(*gc->dp.changeReadBuffers)(gc))
        {
            __GL_ERROR((*gc->dp.getError)(gc));
        }
        gc->drawableDirtyMask &= ~__GL_BUFFER_READ_BIT;
    }

#if defined(OPENGL40) && defined(DRI_PIXMAPRENDER_GL)
    if (gc->imports.conformGLSpec)
    {
        /* Get the latest drawable information */
        LINUX_LOCK_FRAMEBUFFER(gc);
        __glDispatchDrawableChange(gc);
        /* Get the latest drawable information */
        LINUX_UNLOCK_FRAMEBUFFER(gc);
    }
#endif
}

__GL_INLINE GLboolean __glIsStageProgramActive(__GLcontext *gc, __GLprogramObject *progObj, GLbitfield stages)
{
    GLboolean active = GL_FALSE;

    if (gc->shaderProgram.currentProgram)
    {
        if (gc->shaderProgram.currentProgram == progObj)
        {
            active = GL_TRUE;
        }
    }
    else if (gc->shaderProgram.boundPPO)
    {
        __GLprogramPipelineObject *boundPPO = gc->shaderProgram.boundPPO;

        if ((!active) && (stages & GL_VERTEX_SHADER_BIT  ) && (boundPPO->stageProgs[__GLSL_STAGE_VS] == progObj))
        {
            active = GL_TRUE;
        }

        if ((!active) && (stages & GL_FRAGMENT_SHADER_BIT) && (boundPPO->stageProgs[__GLSL_STAGE_FS] == progObj))
        {
            active = GL_TRUE;
        }

        if ((!active) && (stages & GL_COMPUTE_SHADER_BIT ) && (boundPPO->stageProgs[__GLSL_STAGE_CS] == progObj))
        {
            active = GL_TRUE;
        }

        if ((!active) && (stages & GL_TESS_CONTROL_SHADER_BIT_EXT ) && (boundPPO->stageProgs[__GLSL_STAGE_TCS] == progObj))
        {
            active = GL_TRUE;
        }

        if ((!active) && (stages & GL_TESS_EVALUATION_SHADER_BIT_EXT ) && (boundPPO->stageProgs[__GLSL_STAGE_TES] == progObj))
        {
            active = GL_TRUE;
        }

        if ((!active) && (stages & GL_GEOMETRY_SHADER_BIT_EXT ) && (boundPPO->stageProgs[__GLSL_STAGE_GS] == progObj))
        {
            active = GL_TRUE;
        }
    }

    return active;
}

__GL_INLINE __GLprogramObject* __glGetCurrentStageProgram(__GLcontext *gc, __GLSLStage stage)
{
    __GLprogramObject *progObj = gc->shaderProgram.currentProgram;

    if (progObj)
    {
        if (progObj->bindingInfo.activeShaderID[stage] == 0)
        {
            progObj = NULL;
        }
    }
    else if (gc->shaderProgram.boundPPO)
    {
        progObj = gc->shaderProgram.boundPPO->stageProgs[stage];
    }

    return progObj;
}


__GL_INLINE __GLprogramObject* __glGetLastNonFragProgram(__GLcontext *gc)
{
    __GLprogramObject *progObj = gc->shaderProgram.currentProgram;

    if (progObj)
    {
        return progObj;
    }
    else if (gc->shaderProgram.boundPPO)
    {
        __GLSLStage stage = __GLSL_STAGE_GS;
        while (NULL == (progObj = gc->shaderProgram.boundPPO->stageProgs[stage]))
        {
            if (--stage == __GLSL_STAGE_INVALID)
            {
                break;
            }

        }
        return progObj;
    }

    return NULL;
}

__GL_INLINE __GLbufferObject* __glGetBoundBufObj(__GLcontext *gc, GLuint targetIndex)
{
    /* ibo is always from current VAO (default or named). */
    return  (targetIndex == __GL_ELEMENT_ARRAY_BUFFER_INDEX) ?
            gc->vertexArray.boundVAO->vertexArray.boundIdxObj :
            gc->bufferObject.generalBindingPoint[targetIndex].boundBufObj;
}

/*
** DEBUG mode needs frame information.
*/
#if (defined(DEBUG) || defined(_DEBUG) || gcdDUMP || gcdFRAMEINFO_STATISTIC)
#define __GL_FRAME_INFO       1
#define __GLES_PRINT(...)  gcmPRINT(__VA_ARGS__)

enum
{
    __GL_PERDRAW_DUMP_DRAW_RT       = 1 << 0,
    __GL_PERDRAW_DUMP_DRAW_DS       = 1 << 1,
    __GL_PERDRAW_DUMP_CLEAR_RT      = 1 << 2,
    __GL_PERDRAW_DUMP_CLEAR_DS      = 1 << 3,
    __GL_PERDRAW_DUMP_TEXTURE       = 1 << 4,
    __GL_PERDRAW_DUMP_BLITFBO_RT    = 1 << 5,
    __GL_PERDRAW_DUMP_BLITFBO_DS    = 1 << 6,

    __GL_PERDRAW_DUMP_AS_TGA        = 1 << 16,
    __GL_PERDRAW_DUMP_AS_RAW        = 1 << 17,
    __GL_PERDRAW_DUMP_AS_COMPRESSED = 1 << 18,

    __GL_PERDRAW_DUMP_NONE      = 0x0,
    __GL_PERDRAW_DUMP_ALL       = 0xFFFFFFFF,
};
#else
#define __GL_FRAME_INFO       0
#define __GLES_PRINT(...)
#endif

#define __GLES_MAX_FILENAME_LEN 256

#ifdef OPENGL40
extern GLvoid __glEvaluateAttributeChange(__GLcontext *gc);

__GL_INLINE GLvoid __glEvaluateAttribDrawableChange(__GLcontext *gc)
{
    /*
    ** Drawable change must be dispatched before evaluating the attribute change.
    */
//    __glEvaluateFramebufferChange(gc, __GL_BUFFER_DRAW_READ_BITS);
    __glEvaluateDrawableChange(gc, __GL_BUFFER_DRAW_READ_BITS);

//    __glDispatchDrawableChange(gc);

    /*
    ** When app really want to draw to front buffer, sync front buffer to fake front buffer.
    */
//    (*gc->dp.syncFrontToFakeFront)(gc);


    __glEvaluateAttributeChange(gc);

}
#endif


#if VIVANTE_PROFILER
extern GLint __glApiProfileMode;
#endif

#endif /* __gc_gl_context_h__ */
