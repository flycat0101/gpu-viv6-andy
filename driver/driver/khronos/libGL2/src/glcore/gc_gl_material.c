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


/*
** Lighting and coloring code.
**
*/
#include "gc_gl_context.h"
#include "g_asmoff.h"
#include "dri/viv_lock.h"
#include "gc_gl_debug.h"

extern GLvoid __glConvertResult(__GLcontext *gc, GLint fromType, const GLvoid *rawdata,
                              GLint toType, GLvoid *result, GLint size);

#define __GL_VERTEX_BUFFER_FLUSH_MATERIAL(gc)               \
    switch (gc->input.beginMode) {                          \
      case __GL_SMALL_LIST_BATCH:                           \
        __glDisplayListBatchEnd(gc);                        \
        break;                                              \
      case __GL_IN_BEGIN:                                   \
        __glImmediateFlushBuffer(gc);                       \
        break;                                              \
      default: ;                                            \
    }                                                       \



/* s_prim.c */
extern GLvoid __glComputePrimitiveData(__GLcontext *gc);
extern GLvoid __glConfigImmedVertexStream(__GLcontext *gc, GLenum mode);


/* For glMaterial in begin/end
** This Function calls Swp to do TnL and accumulate TnLed vertices in Swp pipeline.
** First it TnL vertices (gc->tnlAccum.prevVertexIndex to gc->input.vertex.index).
** Then if bFlushPipe is TRUE, all accumulated vertices will be flushed to draw.
*/
GLvoid __glImmedFlushPrim_Material(__GLcontext *gc, GLboolean bFlushPipe)
{
    GLint vertexCount;

    vertexCount = gc->input.vertex.index - gc->tnlAccum.preVertexIndex;
    /* First glMaterial inside begin/end */
    if (gc->tnlAccum.vertexCount == 0)
    {
        if (gc->input.lastVertexIndex > 0)
        {
            GLuint saveIndex = gc->input.vertex.index;

            /* Flush all previous primitives (0 to gc->input.lastVertexIndex) */
            gc->input.vertex.index = gc->input.lastVertexIndex;
            __glComputePrimitiveData(gc);
            __glDrawImmedPrimitive(gc);
            __glImmedUpdateVertexState(gc);

            gc->input.vertex.index = saveIndex;
            gc->tnlAccum.preVertexIndex = gc->input.lastVertexIndex;
        }

        gc->tnlAccum.firstVertexIndex = gc->tnlAccum.preVertexIndex;

        /* Switch dispatch entry dispatchTable->End
        Current dispatch table should be info table */
        if(vertexCount > 0 && (gc->currentImmediateTable->dispatch.End != __glim_End_Material))
        {
            gc->input.indexCount = 0;
            gc->tnlAccum.glimEnd = gc->currentImmediateTable->dispatch.End;
            gc->currentImmediateTable->dispatch.End = __glim_End_Material;
        }
    }

    GL_ASSERT(vertexCount >= 0);

    if (vertexCount == 0)
        return;

    if (gc->input.inconsistentFormat == GL_FALSE)
    {
        gc->input.primitiveFormat = gc->input.preVertexFormat;
    }
    __glComputePrimitiveData(gc);

    /*
    ** Prepare gc stream for tnl emulation. Draw with none index mode.
    */
    __glConfigImmedVertexStream(gc, gc->input.currentPrimMode);

    /* Reset startVertex to last tnled one */
    gc->vertexStreams.startVertex = gc->tnlAccum.preVertexIndex;

    /*
    ** update primMode, in normal path it will be updated in draw function.
    ** but we need check it in evaluate attribute.
    */
    if (gc->vertexStreams.primMode != gc->input.currentPrimMode)
    {
        gc->vertexStreams.primMode = gc->input.currentPrimMode;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_PRIMMODE_BIT);
    }

    /* Get the latest drawable information */
    LINUX_LOCK_FRAMEBUFFER(gc);

    /*
    ** Check all GL attributes for redundancy before notify device pipeline.
    */
    __glEvaluateAttribDrawableChange(gc);

    /*
    ** Tnl and accume the tnled vertex for final draw
    */
    (*gc->dp.ctx.tnLAndAccumInPipeline)(gc, bFlushPipe);

    LINUX_UNLOCK_FRAMEBUFFER(gc);

    if (bFlushPipe)
    {
        gc->tnlAccum.preVertexIndex = 0;
        gc->tnlAccum.vertexCount = 0;
        gc->tnlAccum.firstVertexIndex = 0;
    }
    else
    {
        gc->tnlAccum.vertexCount += vertexCount;
        gc->tnlAccum.preVertexIndex = gc->input.vertex.index;
    }
}


GLvoid APIENTRY __glim_End_Material(GLvoid)
{
    __GL_SETUP();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_End_Material", DT_GLnull);
#endif

    if (gc->input.inconsistentFormat == GL_FALSE)
    {
        gc->input.primitiveFormat = gc->input.preVertexFormat;
    }
    __glComputePrimitiveData(gc);

    __glImmedFlushPrim_Material(gc, GL_TRUE);
    __glImmedUpdateVertexState(gc);

     GL_ASSERT(gc->currentImmediateTable->dispatch.End == __glim_End_Material);
    /* Restore dispatchTable->End to the original function pointer */
    gc->currentImmediateTable->dispatch.End = gc->tnlAccum.glimEnd ;

    __glResetImmedVertexBuffer(gc);


    /* Switch to outside Begin/End dispatch table.
    */
    gc->currentImmediateTable = &gc->immediateDispatchTable;
    if (gc->dlist.mode == 0) {
        __GL_SET_API_DISPATCH(__GL_IMMEDIATE_TABLE_OFFSET);
    }

    gc->input.beginMode = __GL_NOT_IN_BEGIN;
}

__GL_INLINE GLvoid
__glMaterialfv(__GLcontext *gc, GLenum face, GLenum pname, GLfloat *pv)
{
    __GLmaterialState *m1, *m2;
    GLbitfield faceMask;

    switch(face) {
    case GL_FRONT:
        m1 = &gc->state.light.front;
        m2 = NULL;
        faceMask = __GL_FRONT_MATERIAL_BITS;
        break;
    case GL_BACK:
        m1 = &gc->state.light.back;
        m2 = NULL;
        faceMask = __GL_BACK_MATERIAL_BITS;
        break;
    case GL_FRONT_AND_BACK:
        m1 = &gc->state.light.front;
        m2 = &gc->state.light.back;
        faceMask = __GL_FRONT_MATERIAL_BITS | __GL_BACK_MATERIAL_BITS;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch(pname) {
    case GL_COLOR_INDEXES:
        m1->cmapa = pv[0];
        m1->cmapd = pv[1];
        m1->cmaps = pv[2];
        if (m2 != NULL) {
            m2->cmapa = pv[0];
            m2->cmapd = pv[1];
            m2->cmaps = pv[2];
        }

        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS,
            (__GL_MATERIAL_COLORINDEX_FRONT_BIT | __GL_MATERIAL_COLORINDEX_BACK_BIT) & faceMask);
        break;

    case GL_SHININESS:
        if (pv[0] < 0.0 || pv[0] > 128.0) {
            __glSetError(GL_INVALID_VALUE);
            return;
        }
        m1->specularExponent = pv[0];
        if (m2 != NULL) {
            m2->specularExponent = m1->specularExponent;
        }

        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS,
            (__GL_MATERIAL_SHININESS_FRONT_BIT | __GL_MATERIAL_SHININESS_BACK_BIT) & faceMask);
        break;

    case GL_EMISSION:
        m1->emissive.r = pv[0];
        m1->emissive.g = pv[1];
        m1->emissive.b = pv[2];
        m1->emissive.a = pv[3];
        if (m2 != NULL) {
            m2->emissive.r = m1->emissive.r;
            m2->emissive.g = m1->emissive.g;
            m2->emissive.b = m1->emissive.b;
            m2->emissive.a = m1->emissive.a;
        }

        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS,
            (__GL_MATERIAL_EMISSION_FRONT_BIT | __GL_MATERIAL_EMISSION_BACK_BIT) & faceMask);
        break;

    case GL_SPECULAR:
        m1->specular.r = pv[0];
        m1->specular.g = pv[1];
        m1->specular.b = pv[2];
        m1->specular.a = pv[3];
        if (m2 != NULL) {
            m2->specular.r = m1->specular.r;
            m2->specular.g = m1->specular.g;
            m2->specular.b = m1->specular.b;
            m2->specular.a = m1->specular.a;
        }

        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS,
            (__GL_MATERIAL_SPECULAR_FRONT_BIT | __GL_MATERIAL_SPECULAR_BACK_BIT) & faceMask);
        break;

    case GL_AMBIENT:
        m1->ambient.r = pv[0];
        m1->ambient.g = pv[1];
        m1->ambient.b = pv[2];
        m1->ambient.a = pv[3];
        if (m2 != NULL) {
            m2->ambient.r = m1->ambient.r;
            m2->ambient.g = m1->ambient.g;
            m2->ambient.b = m1->ambient.b;
            m2->ambient.a = m1->ambient.a;
        }

        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS,
            (__GL_MATERIAL_AMBIENT_FRONT_BIT | __GL_MATERIAL_AMBIENT_BACK_BIT) & faceMask);
        break;

    case GL_DIFFUSE:
        m1->diffuse.r = pv[0];
        m1->diffuse.g = pv[1];
        m1->diffuse.b = pv[2];
        m1->diffuse.a = pv[3];
        if (m2 != NULL) {
            m2->diffuse.r = m1->diffuse.r;
            m2->diffuse.g = m1->diffuse.g;
            m2->diffuse.b = m1->diffuse.b;
            m2->diffuse.a = m1->diffuse.a;
        }

        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS,
            (__GL_MATERIAL_DIFFUSE_FRONT_BIT | __GL_MATERIAL_DIFFUSE_BACK_BIT) & faceMask);
        break;

    case GL_AMBIENT_AND_DIFFUSE:
        m1->ambient.r = m1->diffuse.r = pv[0];
        m1->ambient.g = m1->diffuse.g = pv[1];
        m1->ambient.b = m1->diffuse.b = pv[2];
        m1->ambient.a = m1->diffuse.a = pv[3];
        if (m2 != NULL) {
            m2->ambient.r = m2->diffuse.r = m1->ambient.r;
            m2->ambient.g = m2->diffuse.g = m1->ambient.g;
            m2->ambient.b = m2->diffuse.b = m1->ambient.b;
            m2->ambient.a = m2->diffuse.a = m1->ambient.a;
        }

        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS,
            (__GL_MATERIAL_AMBIENT_FRONT_BIT | __GL_MATERIAL_AMBIENT_BACK_BIT |
            __GL_MATERIAL_DIFFUSE_FRONT_BIT | __GL_MATERIAL_DIFFUSE_BACK_BIT) & faceMask);
        break;

    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

GLvoid __glUpdateMaterialfv(__GLcontext *gc, GLenum face, GLenum pname, GLfloat *pv)
{
    __glMaterialfv(gc, face, pname, pv);
}

GLvoid APIENTRY __glim_Materialfv(GLenum face, GLenum pname, const GLfloat *pv)
{
    __GL_SETUP();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Materialfv", DT_GLenum, face, DT_GLenum, pname, DT_GLfloat_ptr, pv, DT_GLnull);
#endif

    /* Flush immediate mode vertex buffer only if lighting is enabled.
    */
    if (gc->state.enables.lighting.lighting) {
        __GL_VERTEX_BUFFER_FLUSH_MATERIAL(gc);
    }

    __glMaterialfv(gc, face, pname, (GLfloat*)pv);
}

GLvoid APIENTRY __glim_Materialf(GLenum face, GLenum pname, GLfloat f)
{
    __GL_SETUP();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Materialf", DT_GLenum, face, DT_GLenum, pname, DT_GLfloat, f, DT_GLnull);
#endif

    /* Flush immediate mode vertex buffer only if lighting is enabled.
    */
    if (gc->state.enables.lighting.lighting) {
        __GL_VERTEX_BUFFER_FLUSH_MATERIAL(gc);
    }

    if (pname == GL_SHININESS) {
        __glMaterialfv(gc, face, pname, (GLfloat*)&f);
    }
    else {
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_Materialiv(GLenum face, GLenum pname, const GLint *pv)
{
    GLfloat tmpf[4]= {0.0, 0.0, 0.0, 0.0};
    __GL_SETUP();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Materialiv", DT_GLenum, face, DT_GLenum, pname, DT_GLint_ptr, pv, DT_GLnull);
#endif

    /* Flush immediate mode vertex buffer only if lighting is enabled.
    */
    if (gc->state.enables.lighting.lighting) {
        __GL_VERTEX_BUFFER_FLUSH_MATERIAL(gc);
    }

    switch(pname) {
    case GL_EMISSION:
    case GL_SPECULAR:
    case GL_AMBIENT:
    case GL_DIFFUSE:
    case GL_AMBIENT_AND_DIFFUSE:
        tmpf[0] = __GL_I_TO_FLOAT(pv[0]);
        tmpf[1] = __GL_I_TO_FLOAT(pv[1]);
        tmpf[2] = __GL_I_TO_FLOAT(pv[2]);
        tmpf[3] = __GL_I_TO_FLOAT(pv[3]);
        break;
    case GL_COLOR_INDEXES:
        tmpf[0] = (GLfloat)(pv[0]);
        tmpf[1] = (GLfloat)(pv[1]);
        tmpf[2] = (GLfloat)(pv[2]);
        break;
    case GL_SHININESS:
        tmpf[0] = (GLfloat)(pv[0]);
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    __glMaterialfv(gc, face, pname, (GLfloat*)tmpf);
}

GLvoid APIENTRY __glim_Materiali(GLenum face, GLenum pname, GLint i)
{
    GLfloat tmpf = (GLfloat)i;
    __GL_SETUP();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Materiali", DT_GLenum, face, DT_GLenum, pname, DT_GLint, i, DT_GLnull);
#endif

    /* Flush immediate mode vertex buffer only if lighting is enabled.
    */
    if (gc->state.enables.lighting.lighting) {
        __GL_VERTEX_BUFFER_FLUSH_MATERIAL(gc);
    }

    if (pname == GL_SHININESS) {
        __glMaterialfv(gc, face, pname, (GLfloat*)&tmpf);
    }
    else {
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_GetMaterialfv(GLenum face, GLenum pname, GLfloat *result)
{
    __GLmaterialState *mat;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetMaterialfv",DT_GLenum, face, DT_GLenum, pname, DT_GLfloat_ptr, result, DT_GLnull);
#endif

    switch (face)
    {
    case GL_FRONT:
        mat = &gc->state.light.front;
        break;
    case GL_BACK:
        mat = &gc->state.light.back;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch (pname)
    {
    case GL_COLOR_INDEXES:
        result[0] = mat->cmapa;
        result[1] = mat->cmapd;
        result[2] = mat->cmaps;
        break;
    case GL_SHININESS:
        result[0] = mat->specularExponent;
        break;
    case GL_EMISSION:
        result[0] = mat->emissive.r;
        result[1] = mat->emissive.g;
        result[2] = mat->emissive.b;
        result[3] = mat->emissive.a;
        break;
    case GL_AMBIENT:
        result[0] = mat->ambient.r;
        result[1] = mat->ambient.g;
        result[2] = mat->ambient.b;
        result[3] = mat->ambient.a;
        break;
    case GL_DIFFUSE:
        result[0] = mat->diffuse.r;
        result[1] = mat->diffuse.g;
        result[2] = mat->diffuse.b;
        result[3] = mat->diffuse.a;
        break;
    case GL_SPECULAR:
        result[0] = mat->specular.r;
        result[1] = mat->specular.g;
        result[2] = mat->specular.b;
        result[3] = mat->specular.a;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_GetMaterialiv(GLenum face, GLenum pname, GLint *result)
{
    __GLmaterialState *mat;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetMaterialiv", DT_GLenum, face, DT_GLenum, pname, DT_GLint_ptr, result, DT_GLnull);
#endif

    switch (face)
    {
    case GL_FRONT:
        mat = &gc->state.light.front;
        break;
    case GL_BACK:
        mat = &gc->state.light.back;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch (pname)
    {
    case GL_COLOR_INDEXES:
        __glConvertResult(gc, __GL_FLOAT, &mat->cmapa, __GL_INT32, result, 3);
        break;
    case GL_SHININESS:
        __glConvertResult(gc, __GL_FLOAT, &mat->specularExponent, __GL_INT32, result, 1);
        break;
    case GL_EMISSION:
        result[0] = __GL_FLOAT_TO_I(mat->emissive.r);
        result[1] = __GL_FLOAT_TO_I(mat->emissive.g);
        result[2] = __GL_FLOAT_TO_I(mat->emissive.b);
        result[3] = __GL_FLOAT_TO_I(mat->emissive.a);
        break;
    case GL_AMBIENT:
        result[0] = __GL_FLOAT_TO_I(mat->ambient.r);
        result[1] = __GL_FLOAT_TO_I(mat->ambient.g);
        result[2] = __GL_FLOAT_TO_I(mat->ambient.b);
        result[3] = __GL_FLOAT_TO_I(mat->ambient.a);
        break;
    case GL_DIFFUSE:
        result[0] = __GL_FLOAT_TO_I(mat->diffuse.r);
        result[1] = __GL_FLOAT_TO_I(mat->diffuse.g);
        result[2] = __GL_FLOAT_TO_I(mat->diffuse.b);
        result[3] = __GL_FLOAT_TO_I(mat->diffuse.a);
        break;
    case GL_SPECULAR:
        result[0] = __GL_FLOAT_TO_I(mat->specular.r);
        result[1] = __GL_FLOAT_TO_I(mat->specular.g);
        result[2] = __GL_FLOAT_TO_I(mat->specular.b);
        result[3] = __GL_FLOAT_TO_I(mat->specular.a);
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

