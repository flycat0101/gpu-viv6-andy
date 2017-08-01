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


#ifndef __gc_gl_context_h__
#define __gc_gl_context_h__

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
#include "gc_es_attrib.h"
#include "gc_es_protos.h"
#include "gc_es_devicepipe.h"
#include "gc_es_profiler.h"

#include "gc_egl.h"
#include "gc_egl_common.h"

/* Some platform (win64) predefined such macro, which make driver code confused. */
#if defined(MemoryBarrier)
#undef MemoryBarrier
#endif

/*
** Referenced by "gc->globalDirtyState[]".
 */
enum
{
    __GL_ALL_ATTRS        = 0,      /* Non-zero following word => bit position */
    __GL_DIRTY_ATTRS_1    = 1,
    __GL_DIRTY_ATTRS_2    = 2,
    __GL_PROGRAM_ATTRS    = 3,      /* program dirty bits  */
    __GL_TEX_UNIT_ATTRS   = 4,      /* __GL_TEX_UNIT_ATTRS does not occupy a UINT in globalDirtyState[] */
    __GL_IMG_UNIT_ATTRS   = 5,      /* __GL_IMG_UNIT_ATTRS does not occupy a UNIT in globalDirtyState[] */
    __GL_DIRTY_ATTRS_END  = __GL_IMG_UNIT_ATTRS
};

/*
 * Macros that set global dirty attribute bits.
 */
#define __GL_SET_ATTR_DIRTY_BIT(gc, index, bit)                                 \
    (gc)->globalDirtyState[(index)] |= (bit);                                   \
    (gc)->globalDirtyState[__GL_ALL_ATTRS] |= (__GL_ONE_32 << (index))

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
};

#define __GL_ALPHABLEND_ATTR_BITS ( \
                __GL_BLENDCOLOR_BIT | \
                __GL_BLENDFUNC_BIT | \
                __GL_BLENDEQUATION_BIT | \
                __GL_BLEND_ENDISABLE_BIT)

#define __GL_COLORBUF_ATTR_BITS ( \
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

#define __GL_POLYGON_ATTR_BITS ( \
                __GL_FRONTFACE_BIT | \
                __GL_CULLFACE_BIT | \
                __GL_CULLFACE_ENDISABLE_BIT | \
                __GL_POLYGONOFFSET_BIT | \
                __GL_POLYGONOFFSET_FILL_ENDISABLE_BIT)

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
};

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
enum
{
    __GL_TEX_ENABLE_DIM_CHANGED_BIT     = (1 << 0),
    __GL_TEX_IMAGE_CONTENT_CHANGED_BIT  = (1 << 1),
    __GL_TEX_IMAGE_FORMAT_CHANGED_BIT   = (1 << 2),
    __GL_TEXPARAM_WRAP_S_BIT            = (1 << 3),
    __GL_TEXPARAM_WRAP_T_BIT            = (1 << 4),
    __GL_TEXPARAM_WRAP_R_BIT            = (1 << 5),
    __GL_TEXPARAM_MIP_HINT_BIT          = (1 << 6),
    __GL_TEXPARAM_MIN_FILTER_BIT        = (1 << 7),
    __GL_TEXPARAM_MAG_FILTER_BIT        = (1 << 8),
    __GL_TEXPARAM_MIN_LOD_BIT           = (1 << 9),
    __GL_TEXPARAM_MAX_LOD_BIT           = (1 << 10),
    __GL_TEXPARAM_BASE_LEVEL_BIT        = (1 << 11),
    __GL_TEXPARAM_MAX_LEVEL_BIT         = (1 << 12),
    __GL_TEXPARAM_COMPARE_MODE_BIT      = (1 << 13),
    __GL_TEXPARAM_COMPARE_FUNC_BIT      = (1 << 14),
    __GL_TEXPARAM_SWIZZLE_R_BIT         = (1 << 15),
    __GL_TEXPARAM_SWIZZLE_G_BIT         = (1 << 16),
    __GL_TEXPARAM_SWIZZLE_B_BIT         = (1 << 17),
    __GL_TEXPARAM_SWIZZLE_A_BIT         = (1 << 18),
    __GL_TEXPARAM_MAX_ANISTROPY_BIT     = (1 << 19),
    __GL_TEXPARAM_D_ST_TEXMODE_BIT      = (1 << 20),
    __GL_TEXPARAM_SRGB_BIT              = (1 << 21),
    __GL_TEXPARAM_BORDER_COLOR_BIT      = (1 << 22)
};

#define __GL_TEXPARAMETER_SWIZZLE_BITS ( \
                __GL_TEXPARAM_SWIZZLE_R_BIT | \
                __GL_TEXPARAM_SWIZZLE_G_BIT | \
                __GL_TEXPARAM_SWIZZLE_B_BIT | \
                __GL_TEXPARAM_SWIZZLE_A_BIT)

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
                __GL_TEXPARAM_MAX_ANISTROPY_BIT | \
                __GL_TEXPARAM_SRGB_BIT          | \
                __GL_TEXPARAM_BORDER_COLOR_BIT)

#define __GL_TEXPARAMETER_BITS ( \
                __GL_SAMPLERPARAMETER_BITS | \
                __GL_TEXPARAMETER_SWIZZLE_BITS | \
                __GL_TEXPARAM_MIP_HINT_BIT | \
                __GL_TEXPARAM_BASE_LEVEL_BIT | \
                __GL_TEXPARAM_MAX_LEVEL_BIT |\
                __GL_TEXPARAM_D_ST_TEXMODE_BIT)

#define __GL_TEXIMAGE_BITS ( \
                __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | \
                __GL_TEX_IMAGE_FORMAT_CHANGED_BIT )


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


typedef enum __GLApiVersionRec
{
    __GL_API_VERSION_INVALID = 0,
    __GL_API_VERSION_ES20 = 200,
    __GL_API_VERSION_ES30 = 300,
    __GL_API_VERSION_ES31 = 310,
    __GL_API_VERSION_ES32 = 320,
} __GLApiVersion;


#define __glesApiEnum(apiname)  enum_gl##apiname

typedef enum
{
    __GLES_API_ENTRIES(__glesApiEnum)
} __glesApiEnum;

#if gcdPATTERN_FAST_PATH
typedef struct __GLapiInfoRec
{
    __glesApiEnum apiEnum;
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

    /* Current dispatch table. */
    __GLesDispatchTable apiDispatchTable;

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
    GLbitfield texUnitAttrState[__GL_MAX_TEXTURE_UNITS];
    GLbitfield globalDirtyState[__GL_DIRTY_ATTRS_END];
    GLbitfield drawableDirtyMask;

    /*
    ** GL state management data structures.
    */
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
};


extern GLvoid __glSetError(__GLcontext *gc, GLenum code);

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

#define __GL_ERROR_RET_VAL(err, val)    \
    __glSetError(gc, err);              \
    return (val);

#define __GL_EXIT()                     \
    goto OnExit;

#define __GL_ERROR_EXIT2()             \
    goto OnError;



__GL_INLINE __GLcontext * __glGetGLcontext(GLvoid)
{
    gcsDRIVER_TLS_PTR tls;
    gcoOS_GetDriverTLS(gcvTLS_KEY_OPENGL_ES, &tls);
    return (__GLcontext *) tls;
}

__GL_INLINE GLvoid __glSetGLcontext(GLvoid *context)
{
    gcoOS_SetDriverTLS(gcvTLS_KEY_OPENGL_ES, (gcsDRIVER_TLS_PTR) context);
}

extern GLvoid __glEvaluateFramebufferChange(__GLcontext *gc, GLbitfield flags);

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


#define __GLES3_ZONE_TRACE               (gcvZONE_API_ES30 | (1 << 0))
#define __GLES3_ZONE_BUFFER              (gcvZONE_API_ES30 | (1 << 1))
#define __GLES3_ZONE_CLEAR               (gcvZONE_API_ES30 | (1 << 2))
#define __GLES3_ZONE_CODEC               (gcvZONE_API_ES30 | (1 << 3))
#define __GLES3_ZONE_CONTEXT             (gcvZONE_API_ES30 | (1 << 4))
#define __GLES3_ZONE_DEPTH               (gcvZONE_API_ES30 | (1 << 5))
#define __GLES3_ZONE_DEVICE              (gcvZONE_API_ES30 | (1 << 6))
#define __GLES3_ZONE_DRAW                (gcvZONE_API_ES30 | (1 << 7))
#define __GLES3_ZONE_FBO                 (gcvZONE_API_ES30 | (1 << 8))
#define __GLES3_ZONE_PIXEL               (gcvZONE_API_ES30 | (1 << 9))
#define __GLES3_ZONE_SHADER              (gcvZONE_API_ES30 | (1 << 10))
#define __GLES3_ZONE_STATE               (gcvZONE_API_ES30 | (1 << 11))
#define __GLES3_ZONE_TEXTURE             (gcvZONE_API_ES30 | (1 << 12))
#define __GLES3_ZONE_UTILS               (gcvZONE_API_ES30 | (1 << 13))
#define __GLES3_ZONE_PROFILER            (gcvZONE_API_ES30 | (1 << 14))
#define __GLES3_ZONE_CORE                (gcvZONE_API_ES30 | (1 << 15))



#define __GLES_MAX_FILENAME_LEN 256

#endif /* __gc_gl_context_h__ */
