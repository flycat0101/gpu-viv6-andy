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


#include "gc_es_context.h"
#include "gc_es_device.h"
#include "gc_es_object_inline.c"

/*
** Enable/Disable
*/
#define _GC_OBJ_ZONE gcdZONE_ES30_CORE

__GL_INLINE GLvoid __glEnableDisable(__GLcontext *gc, GLenum cap, GLboolean val)
{
    __GLenableState *es = &gc->state.enables;

    switch (cap)
    {
    case GL_BLEND:
        {
            GLuint i;

            if (val && gc->dp.patchBlend)
            {
                (*gc->dp.patchBlend)(gc, gcvTRUE);
            }

            for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
            {
                if (es->colorBuffer.blend[i] ^ val)
                {
                    break;
                }
            }
            if (i < gc->constants.shaderCaps.maxDrawBuffers)
            {
                for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
                {
                    es->colorBuffer.blend[i] = val;
                }
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLEND_ENDISABLE_BIT);
            }
        }
        break;

    case GL_CULL_FACE:
        if (es->polygon.cullFace ^ val)
        {
            es->polygon.cullFace = val;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_CULLFACE_ENDISABLE_BIT);
        }
        break;

    case GL_DEPTH_TEST:
        if (es->depthTest ^ val)
        {
            es->depthTest = val;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1,  __GL_DEPTHTEST_ENDISABLE_BIT);
        }
        break;

    case GL_DITHER:
        if (es->colorBuffer.dither ^ val)
        {
            es->colorBuffer.dither = val;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_DITHER_ENDISABLE_BIT);
        }
        break;

    case GL_SCISSOR_TEST:
        if (es->scissorTest ^ val)
        {
            es->scissorTest = val;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_SCISSORTEST_ENDISABLE_BIT);
        }
        break;

    case GL_STENCIL_TEST:
        if (es->stencilTest ^ val)
        {
            es->stencilTest = val;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILTEST_ENDISABLE_BIT);
        }
        break;

    case GL_POLYGON_OFFSET_FILL:
        if (es->polygon.polygonOffsetFill ^ val)
        {
            es->polygon.polygonOffsetFill = val;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_POLYGONOFFSET_FILL_ENDISABLE_BIT);
        }
        break;

    case GL_SAMPLE_ALPHA_TO_COVERAGE:
        if (es->multisample.alphaToCoverage ^ val)
        {
            es->multisample.alphaToCoverage = val;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_SAMPLE_ALPHA_TO_COVERAGE_ENDISABLE_BIT);
        }
        break;

    case GL_SAMPLE_COVERAGE:
        if (es->multisample.coverage ^ val)
        {
            es->multisample.coverage = val;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_SAMPLE_COVERAGE_ENDISABLE_BIT);
        }
        break;

    case GL_PRIMITIVE_RESTART_FIXED_INDEX:
        if (es->primitiveRestart ^ val)
        {
            es->primitiveRestart = val;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_PRIMITIVE_RESTART_BIT);
        }
        break;

    case GL_RASTERIZER_DISCARD:
        if (es->rasterizerDiscard ^ val)
        {
            es->rasterizerDiscard = val;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1,  __GL_RASTERIZER_DISCARD_ENDISABLE_BIT);
        }
      break;

    case GL_PROFILE_VIV:
#if VIVANTE_PROFILER
        if (gc)
            gc->profiler.enableOutputCounters = val;
#endif
        break;

    case GL_SAMPLE_MASK:
        if (es->multisample.sampleMask ^ val)
        {
            es->multisample.sampleMask = val;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_SAMPLE_MASK_ENDISABLE_BIT);
        }
        break;

    case GL_SAMPLE_SHADING_OES:
        if (es->multisample.sampleShading ^ val)
        {
            es->multisample.sampleShading = val;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_SAMPLE_SHADING_ENDISABLE_BIT);
        }
        break;

    case GL_DEBUG_OUTPUT_KHR:
        gc->debug.dbgOut = val;
        break;

    case GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR:
        gc->debug.dbgOutSync = val;
        break;

    default:
        __GL_ERROR_RET(GL_INVALID_ENUM);
    }
}


GLvoid GL_APIENTRY __gles_Enable(__GLcontext *gc, GLenum cap)
{
    __GL_HEADER();

    __glEnableDisable(gc, cap, GL_TRUE);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_Disable(__GLcontext *gc, GLenum cap)
{
    __GL_HEADER();

    __glEnableDisable(gc, cap, GL_FALSE);

    __GL_FOOTER();
}

GLboolean GL_APIENTRY __gles_IsEnabled(__GLcontext *gc, GLenum cap)
{
    __GLenableState *es = &gc->state.enables;
    GLboolean ret;

    __GL_HEADER();

    switch (cap)
    {
    case GL_BLEND:
        ret = es->colorBuffer.blend[0];
        break;
    case GL_CULL_FACE:
        ret = es->polygon.cullFace;
        break;
    case GL_DEPTH_TEST:
        ret = es->depthTest;
        break;
    case GL_DITHER:
        ret = es->colorBuffer.dither;
        break;
    case GL_SCISSOR_TEST:
        ret = es->scissorTest;
        break;
    case GL_STENCIL_TEST:
        ret = es->stencilTest;
        break;
    case GL_POLYGON_OFFSET_FILL:
        ret = es->polygon.polygonOffsetFill;
        break;
    case GL_SAMPLE_ALPHA_TO_COVERAGE:
        ret = es->multisample.alphaToCoverage;
        break;
    case GL_SAMPLE_COVERAGE:
        ret = es->multisample.coverage;
        break;
    case GL_PRIMITIVE_RESTART_FIXED_INDEX:
        ret = es->primitiveRestart;
        break;
    case GL_RASTERIZER_DISCARD:
        ret = es->rasterizerDiscard;
        break;
    case GL_SAMPLE_MASK:
        ret = es->multisample.sampleMask;
        break;
    case GL_SAMPLE_SHADING_OES:
        ret = es->multisample.sampleShading;
        break;
    case GL_DEBUG_OUTPUT_KHR:
        ret = gc->debug.dbgOut;
        break;
    case GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR:
        ret = gc->debug.dbgOutSync;
        break;
    case GL_PROFILE_VIV:
#if VIVANTE_PROFILER
        ret = gc ? (GLboolean)gc->profiler.enableOutputCounters : GL_FALSE;
#else
        ret = GL_FALSE;
#endif
        break;
    default:
        ret = GL_FALSE;
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
        break;
    }

OnError:
    __GL_FOOTER();

    return ret;
}

/*
** Clear
*/
__GL_INLINE GLboolean __glClearBegin(__GLcontext *gc, GLbitfield *mask)
{
    if (gc->flags & __GL_CONTEXT_SKIP_DRAW_INVALID_RENDERBUFFER)
    {
        return GL_FALSE;
    }
    else
    {
        return (*gc->dp.clearBegin)(gc, mask);
    }
}

__GL_INLINE GLvoid __glClearValidateState(__GLcontext *gc, GLbitfield mask)
{
    (*gc->dp.clearValidateState)(gc, mask);
}


__GL_INLINE GLvoid __glClearEnd(__GLcontext *gc, GLbitfield mask, GLint drawbuffer)
{
    if ((*gc->dp.clearEnd)(gc, mask))
    {
        /* Mark FBO tex attach point dirty */
        if (gc->frameBuffer.drawFramebufObj->name)
        {
            __glSetFBOAttachedTexDirty(gc, mask, drawbuffer);
        }
    }
    else
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }
}


GLvoid GL_APIENTRY __gles_Clear(__GLcontext *gc, GLbitfield mask)
{
    GLboolean retVal;

    __GL_HEADER();

    if (mask & ~(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (gc->state.enables.rasterizerDiscard)
    {
        __GL_EXIT();
    }

    /* Make sure the clear include color buffer and all channel masked. */
    if ((mask & GL_COLOR_BUFFER_BIT) &&
        gc->state.raster.colorMask[0].redMask   &&
        gc->state.raster.colorMask[0].greenMask &&
        gc->state.raster.colorMask[0].blueMask  &&
        gc->state.raster.colorMask[0].alphaMask)
    {
        gc->flags |= __GL_CONTEXT_SKIP_PRESERVE_CLEAR_RECT;
    }

    __glEvaluateDrawableChange(gc, __GL_BUFFER_DRAW_BIT);

    gc->flags &= ~__GL_CONTEXT_SKIP_PRESERVE_CLEAR_RECT;

    if (GL_TRUE == __glClearBegin(gc, &mask))
    {
        __glClearValidateState(gc, mask);
        retVal = (*gc->dp.clear)(gc, mask);
        __glClearEnd(gc, mask, -1);

        if(!retVal)
        {
            __GL_ERROR((*gc->dp.getError)(gc));
        }
    }

OnExit:

OnError:
    __GL_FOOTER();
}


void __glClearBuffer(__GLcontext *gc, GLenum buffer, GLint drawbuffer, GLvoid *value, GLenum type)
{
    GLbitfield mask = 0;
    GLfloat *pf;
    GLboolean retVal;
    switch (buffer)
    {
    case GL_COLOR:
        if (drawbuffer < 0 || drawbuffer > (GLint)(gc->constants.shaderCaps.maxDrawBuffers - 1))
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        mask = GL_COLOR_BUFFER_BIT;

        pf = (GLfloat *)value;
        gc->state.raster.clearColor.clear.r = pf[0];
        gc->state.raster.clearColor.clear.g = pf[1];
        gc->state.raster.clearColor.clear.b = pf[2];
        gc->state.raster.clearColor.clear.a = pf[3];
        break;

    case GL_DEPTH:
        if (drawbuffer != 0)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        mask = GL_DEPTH_BUFFER_BIT;
        GL_ASSERT(type == GL_FLOAT);
        gc->state.depth.clear = *(GLfloat*)value;
        break;

    case GL_STENCIL:
        if (drawbuffer != 0)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        mask = GL_STENCIL_BUFFER_BIT;
        GL_ASSERT(type == GL_INT);
        gc->state.stencil.clear = *(GLint*)value;
        break;

    default:
        __GL_ERROR_RET(GL_INVALID_ENUM);
    }

    if (gc->state.enables.rasterizerDiscard)
    {
        return;
    }

    if (buffer == GL_DEPTH && !gc->state.depth.writeEnable)
    {
        return;
    }

    __glEvaluateDrawableChange(gc, __GL_BUFFER_DRAW_BIT);

    if (GL_TRUE == __glClearBegin(gc, &mask))
    {
        __glClearValidateState(gc, mask);

        retVal = (*gc->dp.clearBuffer)(gc, buffer, drawbuffer, value, type);

        __glClearEnd(gc, mask, drawbuffer);

        if(!retVal)
        {
            __GL_ERROR((*gc->dp.getError)(gc));
        }
    }
}

GLvoid GL_APIENTRY __gles_ClearBufferfi(__GLcontext *gc, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    GLbitfield mask;
    GLboolean retVal;

    __GL_HEADER();

    if ((buffer != GL_DEPTH_STENCIL))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if(drawbuffer != 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    mask = GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;

    /* Assign values to state machine*/
    gc->state.stencil.clear = stencil;
    gc->state.depth.clear = depth;

    __glEvaluateDrawableChange(gc, __GL_BUFFER_DRAW_BIT);

    if (GL_TRUE == __glClearBegin(gc, &mask))
    {
        __glClearValidateState(gc, mask);

        retVal = (*gc->dp.clearBufferfi)(gc, depth, stencil);

        __glClearEnd(gc, mask, -1);

        if (!retVal)
        {
            __GL_ERROR((*gc->dp.getError)(gc));
        }
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ClearBufferiv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLint* value)
{
    __GL_HEADER();

    if (buffer == GL_DEPTH)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if ((buffer == GL_STENCIL) && (drawbuffer != 0))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    __glClearBuffer(gc, buffer, drawbuffer,(GLvoid*)value, GL_INT);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ClearBufferuiv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLuint* value)
{
    __GL_HEADER();

    if (buffer != GL_COLOR)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    __glClearBuffer(gc, buffer, drawbuffer,(GLvoid*)value, GL_UNSIGNED_INT);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ClearBufferfv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
    __GL_HEADER();

    if (buffer == GL_STENCIL)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if ((buffer == GL_DEPTH) && (drawbuffer != 0))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    __glClearBuffer(gc, buffer, drawbuffer,(GLvoid*)value, GL_FLOAT);

OnError:
    __GL_FOOTER();
}


/*
** Flush/Finish
*/

GLvoid GL_APIENTRY __gles_Finish(__GLcontext *gc)
{
    GLboolean retVal;

    __GL_HEADER();

    retVal = (*gc->dp.finish)(gc);

    if(!retVal)
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    __GL_FOOTER();
}


GLvoid GL_APIENTRY __gles_Flush(__GLcontext *gc)
{
    __GL_HEADER();

    (*gc->dp.flush)(gc);

    __GL_FOOTER();
}


/*
** Hint
*/

GLvoid GL_APIENTRY __gles_Hint(__GLcontext *gc, GLenum target, GLenum mode)
{
    __GL_HEADER();

    switch (mode)
    {
    case GL_DONT_CARE:
    case GL_FASTEST:
    case GL_NICEST:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (target)
    {
    case GL_GENERATE_MIPMAP_HINT:
        gc->state.hints.generateMipmap = mode;
        break;

    case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
        gc->state.hints.fsDerivative = mode;
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_HINT_BIT);

OnError:
    __GL_FOOTER();
}


/*
** Fence/Sync
*/

__GL_INLINE GLvoid __glInitSyncObj(__GLcontext *gc, __GLsyncObject *syncObj, GLuint name,
                                   GLenum condition, GLbitfield flags)
{
    syncObj->name = name;
    syncObj->type = GL_SYNC_FENCE;
    syncObj->status = GL_UNSIGNALED;
    syncObj->condition = condition;
    syncObj->flags = flags;
    syncObj->privateData = gcvNULL;
    syncObj->waitCount = 0;
    syncObj->objFlag = 0;
}

GLboolean __glDeleteSyncObj(__GLcontext *gc, __GLsyncObject *syncObj)
{
    GLboolean retVal;

    if (syncObj->waitCount)
    {
        syncObj->objFlag |= __GL_OBJECT_IS_DELETED;
        return GL_FALSE;
    }

    if (syncObj->label)
    {
        gcmOS_SAFE_FREE(gcvNULL, syncObj->label);
    }

    retVal = (*gc->dp.deleteSync)(gc, syncObj);

    if(!retVal)
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    gcmOS_SAFE_FREE(gcvNULL, syncObj);

    return GL_TRUE;
}

GLvoid __glInitSyncState(__GLcontext *gc)
{
    /* Sync objects can be shared across contexts */
    if (gc->shareCtx)
    {
        GL_ASSERT(gc->shareCtx->sync.shared);
        gc->sync.shared = gc->shareCtx->sync.shared;
        gcoOS_LockPLS();
        gc->sync.shared->refcount++;

        /* Allocate VEGL lock */
        if (gcvNULL == gc->sync.shared->lock)
        {
            gc->sync.shared->lock = (*gc->imports.calloc)(gc, 1, sizeof(VEGLLock));
            (*gc->imports.createMutex)(gc->sync.shared->lock);
        }
        gcoOS_UnLockPLS();
    }
    else
    {
        GL_ASSERT(gcvNULL == gc->sync.shared);

        gc->sync.shared = (__GLsharedObjectMachine*)(*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine));

        /* Initialize a linear lookup table for sync object */
        gc->sync.shared->maxLinearTableSize = __GL_MAX_SYNCOBJ_LINEAR_TABLE_SIZE;
        gc->sync.shared->linearTableSize = __GL_DEFAULT_SYNCOBJ_LINEAR_TABLE_SIZE;
        gc->sync.shared->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, gc->sync.shared->linearTableSize * sizeof(GLvoid*));

        gc->sync.shared->hashSize = __GL_SYNC_HASH_TABLE_SIZE;
        gc->sync.shared->hashMask = __GL_SYNC_HASH_TABLE_SIZE - 1;
        gc->sync.shared->refcount = 1;
        gc->sync.shared->deleteObject = (__GLdeleteObjectFunc)__glDeleteSyncObj;
        gc->sync.shared->immediateInvalid = GL_TRUE;
    }
}

GLvoid __glFreeSyncState(__GLcontext *gc)
{
    /* Free shared Sync object table */
    __glFreeSharedObjectState(gc, gc->sync.shared);
}

GLsync GL_APIENTRY __gles_FenceSync(__GLcontext *gc, GLenum condition, GLbitfield flags)
{
    GLint sync = 0;
    GLboolean retVal;
    __GLsyncObject *syncObject;

    __GL_HEADER();

    if (GL_SYNC_GPU_COMMANDS_COMPLETE != condition)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }
    if (0 != flags)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    GL_ASSERT(gc->sync.shared);
    sync = __glGenerateNames(gc, gc->sync.shared, 1);
    if (sync < 0)
    {
        __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
    }
    if (__glMarkNameUsed(gc, gc->sync.shared, (GLuint)sync) < 0)
    {
        __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
    }

    syncObject = (__GLsyncObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLsyncObject));
    if (!syncObject)
    {
        sync = 0;
        __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
    }

    __glInitSyncObj(gc, syncObject, sync, condition, flags);
    __glAddObject(gc, gc->sync.shared, sync, syncObject);

    retVal = (*gc->dp.createSync)(gc, syncObject);

    if(!retVal)
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

OnError:
    __GL_FOOTER();

    return (GLsync)(gctUINTPTR_T)sync;
}

GLboolean GL_APIENTRY __gles_IsSync(__GLcontext *gc, GLsync sync)
{
    return (gcvNULL != __glGetObject(gc, gc->sync.shared, (GLuint)(gctUINTPTR_T)sync));
}

GLvoid GL_APIENTRY __gles_DeleteSync(__GLcontext *gc, GLsync sync)
{
    __GL_HEADER();

    /* Silently ignore sync value of 0 */
    if (0 == (GLuint)(gctUINTPTR_T)sync)
    {
        __GL_EXIT();
    }
    if (__glIsNameDefined(gc, gc->sync.shared, (GLuint)(gctUINTPTR_T)sync) == GL_FALSE)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    __glDeleteObject(gc, gc->sync.shared, (GLuint)(gctUINTPTR_T)sync);

OnExit:
OnError:
    __GL_FOOTER();
}

GLenum GL_APIENTRY __gles_ClientWaitSync(__GLcontext *gc, GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    __GLsyncObject *syncObj;
    GLenum ret = GL_TIMEOUT_EXPIRED;

    __GL_HEADER();

    /* If flag contain any other bits */
    if (flags & ~(GL_SYNC_FLUSH_COMMANDS_BIT))
    {
        ret = GL_WAIT_FAILED;
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    syncObj = (__GLsyncObject *)__glGetObject(gc, gc->sync.shared, (GLuint)(gctUINTPTR_T)sync);
    if (!syncObj)
    {
        ret = GL_WAIT_FAILED;
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (GL_SIGNALED == syncObj->status)
    {
        ret = GL_ALREADY_SIGNALED;
        __GL_EXIT();
    }

    syncObj->waitCount++;

    if (flags & GL_SYNC_FLUSH_COMMANDS_BIT)
    {
        (*gc->dp.flush)(gc);
    }

    /* Call dp function to wait sync */
    ret = (*gc->dp.waitSync)(gc, syncObj, timeout);

    if ((--syncObj->waitCount) == 0 && (syncObj->objFlag & __GL_OBJECT_IS_DELETED))
    {
        __glDeleteSyncObj(gc, syncObj);
    }

OnExit:
OnError:
    __GL_FOOTER();

    return ret;
}

GLvoid GL_APIENTRY __gles_WaitSync(__GLcontext *gc, GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    __GLsyncObject *syncObj;

    __GL_HEADER();

    /* If flag contain any other bits */
    if (0 != flags || (GLuint64)GL_TIMEOUT_IGNORED != timeout)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    syncObj = (__GLsyncObject *)__glGetObject(gc, gc->sync.shared, (GLuint)(gctUINTPTR_T)sync);
    if (!syncObj)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (GL_SIGNALED == syncObj->status)
    {
        __GL_EXIT();
    }

    syncObj->waitCount++;

    /* Call dp function to wait sync */
    (*gc->dp.waitSync)(gc, syncObj, 0);

    /* FIXME: glWaitSync is a nonblocking call. Need to register a server fence callback function
       and execute the following code the callback function */
    if ((--syncObj->waitCount) == 0 && (syncObj->objFlag & __GL_OBJECT_IS_DELETED))
    {
        __glDeleteSyncObj(gc, syncObj);
    }

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetSynciv(__GLcontext *gc, GLsync sync, GLenum pname, GLsizei bufSize,
                                    GLsizei* length, GLint* values)
{
    __GLsyncObject *syncObj;

    __GL_HEADER();

    if (bufSize <= 0 || !values)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    syncObj = (__GLsyncObject *)__glGetObject(gc, gc->sync.shared, (GLuint)(gctUINTPTR_T)sync);
    if (!syncObj)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    switch (pname)
    {
    case GL_OBJECT_TYPE:
        *values = syncObj->type;
        break;
    case GL_SYNC_STATUS:
        *values = syncObj->status;
        break;
    case GL_SYNC_CONDITION:
        *values = syncObj->condition;
        break;
    case GL_SYNC_FLAGS:
        *values = syncObj->flags;
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (length)
    {
        *length = 1;
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_MemoryBarrier(__GLcontext *gc, GLbitfield barriers)
{
    __GL_HEADER();

    if ((barriers != GL_ALL_BARRIER_BITS) &&
        (barriers & (~(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT |
                       GL_ELEMENT_ARRAY_BARRIER_BIT       |
                       GL_UNIFORM_BARRIER_BIT             |
                       GL_TEXTURE_FETCH_BARRIER_BIT       |
                       GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
                       GL_COMMAND_BARRIER_BIT             |
                       GL_PIXEL_BUFFER_BARRIER_BIT        |
                       GL_TEXTURE_UPDATE_BARRIER_BIT      |
                       GL_BUFFER_UPDATE_BARRIER_BIT       |
                       GL_FRAMEBUFFER_BARRIER_BIT         |
                       GL_TRANSFORM_FEEDBACK_BARRIER_BIT  |
                       GL_ATOMIC_COUNTER_BARRIER_BIT      |
                       GL_SHADER_STORAGE_BARRIER_BIT))))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }


    (*gc->dp.memoryBarrier)(gc, barriers);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_MemoryBarrierByRegion(__GLcontext *gc, GLbitfield barriers)
{
    __GL_HEADER();

    if ((barriers != GL_ALL_BARRIER_BITS) &&
        (barriers & (~(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT |
                       GL_ELEMENT_ARRAY_BARRIER_BIT       |
                       GL_UNIFORM_BARRIER_BIT             |
                       GL_TEXTURE_FETCH_BARRIER_BIT       |
                       GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
                       GL_COMMAND_BARRIER_BIT             |
                       GL_PIXEL_BUFFER_BARRIER_BIT        |
                       GL_TEXTURE_UPDATE_BARRIER_BIT      |
                       GL_BUFFER_UPDATE_BARRIER_BIT       |
                       GL_FRAMEBUFFER_BARRIER_BIT         |
                       GL_TRANSFORM_FEEDBACK_BARRIER_BIT  |
                       GL_ATOMIC_COUNTER_BARRIER_BIT      |
                       GL_SHADER_STORAGE_BARRIER_BIT))))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    (*gc->dp.memoryBarrier)(gc, barriers);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_BlendBarrier(__GLcontext *gc)
{
    __GL_HEADER();

    (*gc->dp.blendBarrier)(gc);

    __GL_FOOTER();
}


GLvoid __glInitDebugState(__GLcontext *gc)
{
    __GLdbgSrc srcIdx;
    __GLdbgType typeIdx;
    __GLdbgSeverity severityIdx;
    __GLdbgGroupCtrl *groupCtrl;
    __GLdebugMachine *dbgMachine = &gc->debug;

    dbgMachine->maxStackDepth = 64;
    dbgMachine->maxMsgLen = 256;
    dbgMachine->maxLogMsgs = 1024;

    dbgMachine->dbgOut = (GLboolean)gc->imports.debuggable;
    dbgMachine->dbgOutSync = GL_FALSE;
    dbgMachine->callback = NULL;
    dbgMachine->userParam = NULL;

    dbgMachine->current = 0;
    dbgMachine->msgCtrlStack = (__GLdbgGroupCtrl**)gc->imports.calloc(gc, dbgMachine->maxStackDepth, sizeof(__GLdbgGroupCtrl*));
    groupCtrl = (__GLdbgGroupCtrl*)gc->imports.calloc(gc, 1, sizeof(__GLdbgGroupCtrl));
    groupCtrl->message = NULL; /* No message for default group */

    for (srcIdx = 0; srcIdx < __GL_DEBUG_SRC_NUM; ++srcIdx)
    {
        for (typeIdx = 0; typeIdx < __GL_DEBUG_TYPE_NUM; ++typeIdx)
        {
            for (severityIdx = 0; severityIdx < __GL_DEBUG_SEVERITY_NUM; ++severityIdx)
            {
                groupCtrl->spaces[srcIdx][typeIdx].enables[severityIdx] = severityIdx != __GL_DEBUG_SEVERITY_LOW
                                                                        ? GL_TRUE
                                                                        : GL_FALSE;
            }
        }
    }
    dbgMachine->msgCtrlStack[dbgMachine->current] = groupCtrl;
}

GLvoid __glFreeDebugState(__GLcontext *gc)
{
    GLint stackIdx;
    __GLdebugMachine *dbgMachine = &gc->debug;
    __GLdbgMsgLog *msgLog = dbgMachine->msgLogHead;

    for (stackIdx = dbgMachine->current; stackIdx >= 0; --stackIdx)
    {
        __GLdbgSrc srcIdx;
        __GLdbgType typeIdx;
        __GLdbgGroupCtrl *groupCtrl = dbgMachine->msgCtrlStack[stackIdx];

        for (srcIdx = 0; srcIdx < __GL_DEBUG_SRC_NUM; ++srcIdx)
        {
            for (typeIdx = 0; typeIdx < __GL_DEBUG_TYPE_NUM; ++typeIdx)
            {
                __GLdbgMsgCtrl *msgCtrl = groupCtrl->spaces[srcIdx][typeIdx].msgs;
                while (msgCtrl)
                {
                    __GLdbgMsgCtrl *next = msgCtrl->next;
                    gcmOS_SAFE_FREE(gcvNULL, msgCtrl);
                    msgCtrl = next;
                }
            }
        }

        if (groupCtrl->message)
        {
            gcmOS_SAFE_FREE(gcvNULL, groupCtrl->message);
        }
        gcmOS_SAFE_FREE(gcvNULL, groupCtrl);
    }
    gcmOS_SAFE_FREE(gcvNULL, dbgMachine->msgCtrlStack);


    while (msgLog)
    {
        __GLdbgMsgLog *next = msgLog->next;
        if (msgLog->message)
        {
            gcmOS_SAFE_FREE(gcvNULL, msgLog->message);
        }
        gcmOS_SAFE_FREE(gcvNULL, msgLog);
        msgLog = next;
    }
    dbgMachine->msgLogHead = NULL;
    dbgMachine->msgLogTail = NULL;
}

__GL_INLINE __GLdbgSrc __glDebugGetSourceIdx(GLenum source)
{
    __GLdbgSrc srcIdx = __GL_DEBUG_SRC_NUM;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API_KHR:
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM_KHR:
    case GL_DEBUG_SOURCE_SHADER_COMPILER_KHR:
    case GL_DEBUG_SOURCE_THIRD_PARTY_KHR:
    case GL_DEBUG_SOURCE_APPLICATION_KHR:
    case GL_DEBUG_SOURCE_OTHER_KHR:
        srcIdx = (__GLdbgSrc)(source - GL_DEBUG_SOURCE_API_KHR);
        break;
    default:
        break;
    }

    return srcIdx;
}

__GL_INLINE __GLdbgType __glDebugGetTypeIdx(GLenum type)
{
    __GLdbgType typeIdx = __GL_DEBUG_TYPE_NUM;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR_KHR:
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_KHR:
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_KHR:
    case GL_DEBUG_TYPE_PORTABILITY_KHR:
    case GL_DEBUG_TYPE_PERFORMANCE_KHR:
    case GL_DEBUG_TYPE_OTHER_KHR:
        typeIdx = (__GLdbgType)(type - GL_DEBUG_TYPE_ERROR_KHR);
        break;
    case GL_DEBUG_TYPE_MARKER_KHR:
        typeIdx = __GL_DEBUG_TYPE_MARKER;
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP_KHR:
        typeIdx = __GL_DEBUG_TYPE_PUSH;
        break;
    case GL_DEBUG_TYPE_POP_GROUP_KHR:
        typeIdx = __GL_DEBUG_TYPE_POP;
        break;
    default:
        break;
    }

    return typeIdx;
}

__GL_INLINE __GLdbgSeverity __glDebugGetSeverityIdx(GLenum severity)
{
    __GLdbgSeverity severityIdx = __GL_DEBUG_SEVERITY_NUM;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH_KHR:
        severityIdx = __GL_DEBUG_SEVERITY_HIGH;
        break;
    case GL_DEBUG_SEVERITY_MEDIUM_KHR:
        severityIdx = __GL_DEBUG_SEVERITY_MEDIUM;
        break;
    case GL_DEBUG_SEVERITY_LOW_KHR:
        severityIdx = __GL_DEBUG_SEVERITY_LOW;
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION_KHR:
        severityIdx = __GL_DEBUG_SEVERITY_NOTICE;
        break;
    default:
        break;
    }

    return severityIdx;
}

GLboolean __glDebugIsLogEnabled(__GLcontext *gc, GLenum source, GLenum type, GLenum severity, GLuint id)
{
    __GLdbgSrc srcIdx = __glDebugGetSourceIdx(source);
    __GLdbgType typeIdx = __glDebugGetTypeIdx(type);
    __GLdbgSeverity severityIdx = __glDebugGetSeverityIdx(severity);
    GLboolean enabled = GL_FALSE;

    if (srcIdx < __GL_DEBUG_SRC_NUM && typeIdx < __GL_DEBUG_TYPE_NUM && severityIdx < __GL_DEBUG_SEVERITY_NUM)
    {
        __GLdebugMachine *dbgMachine = &gc->debug;
        __GLdbgSpaceCtrl *spaceCtrl = &dbgMachine->msgCtrlStack[dbgMachine->current]->spaces[srcIdx][typeIdx];
        __GLdbgMsgCtrl *iter = spaceCtrl->msgs;
        __GLdbgMsgCtrl *msgCtrl = NULL;

        while (iter)
        {
            if (iter->id == id)
            {
                msgCtrl = iter;
            }
            iter = iter->next;
        }

        if (msgCtrl)
        {
            enabled = msgCtrl->enables[severityIdx];
        }
        else
        {
            enabled = spaceCtrl->enables[severityIdx];
        }
    }

    return enabled;
}

GLboolean __glDebugInsertLogMessage(__GLcontext *gc, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLboolean needCopy)
{
    GLboolean inserted = GL_FALSE;

    do
    {
        __GLdebugMachine *dbgMachine = &gc->debug;

        if (!dbgMachine->dbgOut)
        {
            break;
        }

        if (!__glDebugIsLogEnabled(gc, source, type, severity, id))
        {
            break;
        }

        if (dbgMachine->callback)
        {
            GLsizei msgLen = (length < 0) ? (GLsizei)strlen(message) : length;
            dbgMachine->callback(source, type, id, severity, msgLen, message, dbgMachine->userParam);
        }
        /* If the message log is not full */
        else if (dbgMachine->loggedMsgs < dbgMachine->maxLogMsgs)
        {
            __GLdbgMsgLog *msgLog;
            GLsizei msgLen = (length < 0 || needCopy) ? (GLsizei)strlen(message) : length;

            if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(__GLdbgMsgLog), (gctPOINTER *)&msgLog)))
            {
                break;
            }

            msgLen = __GL_MIN(dbgMachine->maxMsgLen - 1, msgLen);

            msgLog->src = source;
            msgLog->type = type;
            msgLog->severity = severity;
            msgLog->id = id;
            msgLog->length = msgLen + 1;
            if (needCopy)
            {
                if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, (msgLen + 1) * sizeof(GLchar), (gctPOINTER *)&msgLog->message)))
                {
                    gcmOS_SAFE_FREE(gcvNULL, msgLog);
                    break;
                }

                __GL_MEMCOPY(msgLog->message, message, msgLen * sizeof(GLchar));
                msgLog->message[msgLen] = '\0';
            }
            else
            {
                msgLog->message = (GLchar*)message;
                inserted = GL_TRUE;
            }
            msgLog->next = NULL;

            /* Insert to tail */
            if (dbgMachine->msgLogHead)
            {
                GL_ASSERT(dbgMachine->msgLogTail);
                dbgMachine->msgLogTail->next = msgLog;
            }
            else
            {
                dbgMachine->msgLogHead = msgLog;
            }
            dbgMachine->msgLogTail = msgLog;

            ++dbgMachine->loggedMsgs;
        }
    } while (GL_FALSE);

    return inserted;
}

GLvoid __glDebugPrintLogMessage(__GLcontext *gc, GLenum source, GLenum type, GLuint id, GLenum severity, const GLchar *format, ...)
{
    __GLdebugMachine *dbgMachine = &gc->debug;

    if (dbgMachine->dbgOut && __glDebugIsLogEnabled(gc, source, type, severity, id))
    {
        GLuint offset = 0;
        gctARGUMENTS args;

        GLchar *message;

        if (gcmIS_SUCCESS(gcoOS_Allocate(gcvNULL, dbgMachine->maxLogMsgs, (gctPOINTER *)&message)))
        {
            /* format the message string */
            gcmARGUMENTS_START(args, format);
            gcoOS_PrintStrVSafe(message, (gctSIZE_T)dbgMachine->maxLogMsgs, &offset, format, args);
            gcmARGUMENTS_END(args);

            if (!__glDebugInsertLogMessage(gc, source, type, id, severity, -1, message, GL_FALSE))
            {
                /* If inserting message to list failed, free it here to avoid memory leak. */
                gcmOS_SAFE_FREE(gcvNULL, message);
            }
        }
    }
}

GLvoid GL_APIENTRY __gles_DebugMessageControl(__GLcontext *gc, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)
{
    __GLdbgSrc srcIdx;
    __GLdbgType typeIdx;
    __GLdbgSeverity severityIdx;
    __GLdebugMachine *dbgMachine = &gc->debug;

    __GL_HEADER();

    if (count < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    srcIdx = __glDebugGetSourceIdx(source);
    if (srcIdx == __GL_DEBUG_SRC_NUM && source != GL_DONT_CARE)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    typeIdx = __glDebugGetTypeIdx(type);
    if (typeIdx == __GL_DEBUG_TYPE_NUM && type != GL_DONT_CARE)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    severityIdx = __glDebugGetSeverityIdx(severity);
    if (severityIdx == __GL_DEBUG_SEVERITY_NUM && severity != GL_DONT_CARE)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (count > 0 && ids)
    {
        GLsizei i;
        __GLdbgSpaceCtrl *spaceCtrl;

        if (source == GL_DONT_CARE || type == GL_DONT_CARE || severity != GL_DONT_CARE)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        spaceCtrl = &dbgMachine->msgCtrlStack[dbgMachine->current]->spaces[srcIdx][typeIdx];

        for (i = 0; i < count; ++i)
        {
            __GLdbgMsgCtrl *msgCtrl = NULL;
            __GLdbgMsgCtrl *iter = spaceCtrl->msgs;
            __GLdbgSeverity severityIdx;

            while (iter)
            {
                if (iter->id == ids[i])
                {
                    msgCtrl = iter;
                    break;
                }
                iter = iter->next;
            }

            if (!msgCtrl)
            {
                if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(__GLdbgMsgLog), (gctPOINTER *)&msgCtrl)))
                {
                    __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
                }
                msgCtrl->id = ids[i];
                msgCtrl->src = source;
                msgCtrl->type = type;

                /* Insert it to head */
                msgCtrl->next = spaceCtrl->msgs;
                spaceCtrl->msgs = msgCtrl;
            }

            for (severityIdx = 0; severityIdx < __GL_DEBUG_SEVERITY_NUM; ++severityIdx)
            {
                msgCtrl->enables[severityIdx] = enabled;
            }
        }
    }
    else if (count == 0 && ids == NULL)
    {
        __GLdbgSrc srcIdxBegin = (source == GL_DONT_CARE) ? 0 : srcIdx;
        __GLdbgSrc srcIdxEnd   = (source == GL_DONT_CARE) ? __GL_DEBUG_SRC_NUM : srcIdx + 1 ;
        __GLdbgType typeIdxBegin = (type == GL_DONT_CARE) ? 0 : typeIdx;
        __GLdbgType typeIdxEnd   = (type == GL_DONT_CARE) ? __GL_DEBUG_TYPE_NUM : typeIdx + 1 ;
        __GLdbgSeverity severityIdxBegin = (severity == GL_DONT_CARE) ? 0 : severityIdx;
        __GLdbgSeverity severityIdxEnd   = (severity == GL_DONT_CARE) ? __GL_DEBUG_SEVERITY_NUM : severityIdx + 1;
        __GLdbgGroupCtrl *groupCtrl = dbgMachine->msgCtrlStack[dbgMachine->current];

        for (srcIdx = srcIdxBegin; srcIdx < srcIdxEnd; ++srcIdx)
        {
            for (typeIdx = typeIdxBegin; typeIdx < typeIdxEnd; ++typeIdx)
            {
                __GLdbgSpaceCtrl *spaceCtrl = &groupCtrl->spaces[srcIdx][typeIdx];
                __GLdbgMsgCtrl *msgCtrl = spaceCtrl->msgs;

                /* Mark all msgCtrl already created */
                while (msgCtrl)
                {
                    for (severityIdx = severityIdxBegin; severityIdx < severityIdxEnd; ++severityIdx)
                    {
                        msgCtrl->enables[severityIdx] = enabled;
                    }
                    msgCtrl = msgCtrl->next;
                }

                /* Record enable state for later created message */
                for (severityIdx = severityIdxBegin; severityIdx < severityIdxEnd; ++severityIdx)
                {
                    spaceCtrl->enables[severityIdx] = enabled;
                }
            }
        }
    }
    else
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_DebugMessageInsert(__GLcontext *gc, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf)
{
    __GLdbgType typeIdx;
    __GLdbgSeverity severityIdx;
    __GLdebugMachine *dbgMachine = &gc->debug;

    __GL_HEADER();

    if (!dbgMachine->dbgOut)
    {
        __GL_EXIT();
    }

    if (source != GL_DEBUG_SOURCE_THIRD_PARTY_KHR && source != GL_DEBUG_SOURCE_APPLICATION_KHR)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    typeIdx = __glDebugGetTypeIdx(type);
    if (typeIdx == __GL_DEBUG_TYPE_NUM)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    severityIdx = __glDebugGetSeverityIdx(severity);
    if (severityIdx == __GL_DEBUG_SEVERITY_NUM )
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if ((length < 0 ? (GLsizei)strlen(buf) : length) >= dbgMachine->maxMsgLen)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    __glDebugInsertLogMessage(gc, source, type, id, severity, length, buf, GL_TRUE);

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_DebugMessageCallback(__GLcontext *gc, GLDEBUGPROCKHR callback, const GLvoid* userParam)
{
    __GL_HEADER();

    gc->debug.callback = callback;
    gc->debug.userParam = userParam;

    __GL_FOOTER();
}

GLuint GL_APIENTRY __gles_GetDebugMessageLog(__GLcontext *gc, GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog)
{
    GLuint i = 0;
    GLsizei loggedLen = 0;
    __GLdbgMsgLog *msgLog;
    __GLdebugMachine *dbgMachine = &gc->debug;

    __GL_HEADER();

    if (bufSize < 0 && messageLog)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    msgLog = dbgMachine->msgLogHead;
    for (i = 0; i < count && msgLog; ++i)
    {
        __GLdbgMsgLog *next = msgLog->next;

        if (messageLog)
        {
            /* If the messge cannot fully fit */
            if (loggedLen + msgLog->length > bufSize)
            {
                break;
            }

            __GL_MEMCOPY(messageLog + loggedLen, msgLog->message, msgLog->length);
            loggedLen += msgLog->length;
        }

        if (sources)
        {
            sources[i] = msgLog->src;
        }

        if (types)
        {
            types[i] = msgLog->type;
        }

        if (ids)
        {
            ids[i] = msgLog->id;
        }

        if (severities)
        {
            severities[i] = msgLog->severity;
        }

        if (lengths)
        {
            lengths[i] = msgLog->length;
        }

        /* Remove the msg from head of the list */
        dbgMachine->msgLogHead = next;
        if (dbgMachine->msgLogTail == msgLog)
        {
            dbgMachine->msgLogTail = NULL;
        }

        gcmOS_SAFE_FREE(gcvNULL, msgLog->message);
        gcmOS_SAFE_FREE(gcvNULL, msgLog);
        --dbgMachine->loggedMsgs;

        msgLog = next;
    }

OnError:
    __GL_FOOTER();

    return i;
}

GLvoid GL_APIENTRY __gles_GetPointerv(__GLcontext *gc, GLenum pname, GLvoid** params)
{
    __GL_HEADER();

    if (!params)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    switch (pname)
    {
    case GL_DEBUG_CALLBACK_FUNCTION_KHR:
        *params = (GLvoid*)gc->debug.callback;
        break;
    case GL_DEBUG_CALLBACK_USER_PARAM_KHR:
        *params = (GLvoid*)gc->debug.userParam;
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
        break;
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_PushDebugGroup(__GLcontext *gc, GLenum source, GLuint id, GLsizei length, const GLchar * message)
{
    GLsizei msgLen;
    __GLdbgSrc srcIdx;
    __GLdbgType typeIdx;
    __GLdbgGroupCtrl *groupCtrl, *groupCtrlPrev;
    __GLdebugMachine *dbgMachine = &gc->debug;

    __GL_HEADER();

    switch (source)
    {
    case GL_DEBUG_SOURCE_THIRD_PARTY_KHR:
    case GL_DEBUG_SOURCE_APPLICATION_KHR:
        srcIdx = (__GLdbgSrc)(source - GL_DEBUG_SOURCE_API_KHR);
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
        break;
    }

    if ((length < 0 ? (GLsizei)strlen(message) : length) >= dbgMachine->maxMsgLen)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (dbgMachine->current >= dbgMachine->maxStackDepth - 1)
    {
        __GL_ERROR_EXIT(GL_STACK_OVERFLOW_KHR);
    }

    groupCtrlPrev = dbgMachine->msgCtrlStack[dbgMachine->current];
    dbgMachine->msgCtrlStack[++dbgMachine->current] = groupCtrl
                                                    = (__GLdbgGroupCtrl*)gc->imports.calloc(gc, 1, sizeof(__GLdbgGroupCtrl));

    /* Inherit the control volume from previous group */
    for (srcIdx = 0; srcIdx < __GL_DEBUG_SRC_NUM; ++srcIdx)
    {
        for (typeIdx = 0; typeIdx < __GL_DEBUG_TYPE_NUM; ++typeIdx)
        {
            __GLdbgMsgCtrl *msgCtrlPrev = groupCtrlPrev->spaces[srcIdx][typeIdx].msgs;

            while (msgCtrlPrev)
            {
                __GLdbgMsgCtrl *msgCtrl;
                 if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(__GLdbgMsgCtrl), (gctPOINTER *)&msgCtrl)))
                 {
                     __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
                 }

                __GL_MEMCOPY(msgCtrl, msgCtrlPrev, sizeof(__GLdbgMsgCtrl));

                /* Insert to head of new group */
                msgCtrl->next = groupCtrl->spaces[srcIdx][typeIdx].msgs;
                groupCtrl->spaces[srcIdx][typeIdx].msgs = msgCtrl;

                msgCtrlPrev = msgCtrlPrev->next;
            }

            __GL_MEMCOPY(groupCtrl->spaces[srcIdx][typeIdx].enables,
                         groupCtrlPrev->spaces[srcIdx][typeIdx].enables,
                         __GL_DEBUG_SEVERITY_NUM * sizeof(GLboolean));
        }
    }

    groupCtrl->source = source;
    groupCtrl->id = id;
    msgLen = (length < 0) ? (GLsizei)strlen(message) : length;
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, (msgLen + 1) * sizeof(GLchar), (gctPOINTER *)&groupCtrl->message)))
    {
        __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
    }

    __GL_MEMCOPY(groupCtrl->message, message, msgLen);
    groupCtrl->message[msgLen] = '\0';

    __glDebugInsertLogMessage(gc, source, GL_DEBUG_TYPE_PUSH_GROUP_KHR, id,
                              GL_DEBUG_SEVERITY_NOTIFICATION_KHR, length, message, GL_TRUE);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_PopDebugGroup(__GLcontext *gc)
{
    __GLdbgSrc srcIdx;
    __GLdbgType typeIdx;
    __GLdbgGroupCtrl *groupCtrl;
    __GLdebugMachine *dbgMachine = &gc->debug;

    __GL_HEADER();

    if (dbgMachine->current == 0)
    {
        __GL_ERROR_EXIT(GL_STACK_UNDERFLOW_KHR);
    }

    groupCtrl = dbgMachine->msgCtrlStack[dbgMachine->current--];
    __glDebugInsertLogMessage(gc, groupCtrl->source, GL_DEBUG_TYPE_POP_GROUP_KHR, groupCtrl->id,
                              GL_DEBUG_SEVERITY_NOTIFICATION_KHR, -1, groupCtrl->message, GL_TRUE);

    for (srcIdx = 0; srcIdx < __GL_DEBUG_SRC_NUM; ++srcIdx)
    {
        for (typeIdx = 0; typeIdx < __GL_DEBUG_TYPE_NUM; ++typeIdx)
        {
            __GLdbgMsgCtrl *msgCtrl = groupCtrl->spaces[srcIdx][typeIdx].msgs;
            while (msgCtrl)
            {
                __GLdbgMsgCtrl *next = msgCtrl->next;
                gcmOS_SAFE_FREE(gcvNULL, msgCtrl);
                msgCtrl = next;
            }
        }
    }

    if (groupCtrl->message)
    {
        gcmOS_SAFE_FREE(gcvNULL, groupCtrl->message);
    }
    gcmOS_SAFE_FREE(gcvNULL, groupCtrl);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ObjectLabel(__GLcontext *gc, GLenum identifier, GLuint name, GLsizei length, const GLchar *label)
{
    GLchar **pLabel = NULL;

    __GL_HEADER();

    if (label)
    {
        if ((length < 0 ? (GLsizei)strlen(label) : length) >= gc->debug.maxMsgLen)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
    }

    switch (identifier)
    {
    case GL_BUFFER_KHR:
        {
            __GLbufferObject *bufObj = (__GLbufferObject*)__glGetObject(gc, gc->bufferObject.shared, name);
            if (!bufObj)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            pLabel = &bufObj->label;
        }
        break;

    case GL_SHADER_KHR:
        {
            __GLshaderObject *shaderObj = (__GLshaderObject*)__glGetObject(gc, gc->shaderProgram.spShared, name);
            if (!shaderObj)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            else if (shaderObj->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE)
            {
                __GL_ERROR_EXIT(GL_INVALID_OPERATION);
            }
            pLabel = &shaderObj->objectInfo.label;
        }
        break;

    case GL_PROGRAM_KHR:
        {
            __GLprogramObject *progObj = (__GLprogramObject*)__glGetObject(gc, gc->shaderProgram.spShared, name);
            if (!progObj)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            else if (progObj->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
            {
                __GL_ERROR_EXIT(GL_INVALID_OPERATION);
            }
            pLabel = &progObj->objectInfo.label;
        }
        break;

    case GL_VERTEX_ARRAY_KHR:
        {
            __GLvertexArrayObject *vao = (__GLvertexArrayObject*)__glGetObject(gc, gc->vertexArray.noShare, name);
            if (!vao)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            pLabel = &vao->label;
        }
        break;

    case GL_QUERY_KHR:
        {
            __GLqueryObject *queryObj = (__GLqueryObject*)__glGetObject(gc, gc->query.noShare, name);
            if (!queryObj)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            pLabel = &queryObj->label;
        }
        break;
    case GL_PROGRAM_PIPELINE_KHR:
        {
            __GLprogramPipelineObject *ppo = (__GLprogramPipelineObject*)__glGetObject(gc, gc->shaderProgram.ppNoShare, name);
            if (!ppo)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            pLabel = &ppo->label;
        }
        break;
    case GL_TRANSFORM_FEEDBACK:
        {
            __GLxfbObject *xfb = (__GLxfbObject*)__glGetObject(gc, gc->xfb.noShare, name);
            if (!xfb)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            pLabel = &xfb->label;
        }
        break;

    case GL_SAMPLER_KHR:
        {
            __GLsamplerObject *samplerObj = (__GLsamplerObject*)__glGetObject(gc, gc->sampler.shared, name);
            if (!samplerObj)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            pLabel = &samplerObj->label;
        }
        break;

    case GL_TEXTURE:
        {
            __GLtextureObject *texObj = (__GLtextureObject*)__glGetObject(gc, gc->texture.shared, name);
            if (!texObj)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            pLabel = &texObj->label;
        }
        break;

    case GL_RENDERBUFFER:
        {
            __GLrenderbufferObject *rbo = (__GLrenderbufferObject*)__glGetObject(gc, gc->frameBuffer.rboShared, name);
            if (!rbo)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            pLabel = &rbo->label;
        }
        break;

    case GL_FRAMEBUFFER:
        {
            __GLframebufferObject *fbo = (__GLframebufferObject*)__glGetObject(gc, gc->frameBuffer.fboManager, name);
            if (!fbo)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            pLabel = &fbo->label;
        }
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (*pLabel)
    {
        gcmOS_SAFE_FREE(gcvNULL, *pLabel);
        *pLabel = NULL;
    }

    if (label)
    {
        GLsizei msgLen = (length < 0) ? (GLsizei)strlen(label) : length;
        msgLen = __GL_MIN(gc->debug.maxMsgLen - 1, msgLen);

        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, (msgLen + 1) * sizeof(GLchar), (gctPOINTER *)pLabel)))
        {
            __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
        }
        __GL_MEMCOPY(*pLabel, label, msgLen * sizeof(GLchar));
        (*pLabel)[msgLen] = '\0';
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetObjectLabel(__GLcontext *gc, GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    GLsizei len = 0;
    GLchar *objLabel = NULL;

    __GL_HEADER();

    if (bufSize < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    switch (identifier)
    {
    case GL_BUFFER_KHR:
        {
            __GLbufferObject *bufObj = (__GLbufferObject*)__glGetObject(gc, gc->bufferObject.shared, name);
            if (!bufObj)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            objLabel = bufObj->label;
        }
        break;

    case GL_SHADER_KHR:
        {
            __GLshaderObject *shaderObj = (__GLshaderObject*)__glGetObject(gc, gc->shaderProgram.spShared, name);
            if (!shaderObj)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            else if (shaderObj->objectInfo.objectType != __GL_SHADER_OBJECT_TYPE)
            {
                __GL_ERROR_EXIT(GL_INVALID_OPERATION);
            }
            objLabel = shaderObj->objectInfo.label;
        }
        break;

    case GL_PROGRAM_KHR:
        {
            __GLprogramObject *progObj = (__GLprogramObject*)__glGetObject(gc, gc->shaderProgram.spShared, name);
            if (!progObj)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            else if (progObj->objectInfo.objectType != __GL_PROGRAM_OBJECT_TYPE)
            {
                __GL_ERROR_EXIT(GL_INVALID_OPERATION);
            }
            objLabel = progObj->objectInfo.label;
        }
        break;

    case GL_VERTEX_ARRAY_KHR:
        {
            __GLvertexArrayObject *vao = (__GLvertexArrayObject*)__glGetObject(gc, gc->vertexArray.noShare, name);
            if (!vao)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            objLabel = vao->label;
        }
        break;

    case GL_QUERY_KHR:
        {
            __GLqueryObject *queryObj = (__GLqueryObject*)__glGetObject(gc, gc->query.noShare, name);
            if (!queryObj)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            objLabel = queryObj->label;
        }
        break;
    case GL_PROGRAM_PIPELINE_KHR:
        {
            __GLprogramPipelineObject *ppo = (__GLprogramPipelineObject*)__glGetObject(gc, gc->shaderProgram.ppNoShare, name);
            if (!ppo)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            objLabel = ppo->label;
        }
        break;
    case GL_TRANSFORM_FEEDBACK:
        {
            __GLxfbObject *xfb = (__GLxfbObject*)__glGetObject(gc, gc->xfb.noShare, name);
            if (!xfb)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            objLabel = xfb->label;
        }
        break;

    case GL_SAMPLER_KHR:
        {
            __GLsamplerObject *samplerObj = (__GLsamplerObject*)__glGetObject(gc, gc->sampler.shared, name);
            if (!samplerObj)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            objLabel = samplerObj->label;
        }
        break;

    case GL_TEXTURE:
        {
            __GLtextureObject *texObj = (__GLtextureObject*)__glGetObject(gc, gc->texture.shared, name);
            if (!texObj)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            objLabel = texObj->label;
        }
        break;

    case GL_RENDERBUFFER:
        {
            __GLrenderbufferObject *rbo = (__GLrenderbufferObject*)__glGetObject(gc, gc->frameBuffer.rboShared, name);
            if (!rbo)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            objLabel = rbo->label;
        }
        break;

    case GL_FRAMEBUFFER:
        {
            __GLframebufferObject *fbo = (__GLframebufferObject*)__glGetObject(gc, gc->frameBuffer.fboManager, name);
            if (!fbo)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            objLabel = fbo->label;
        }
        break;

    default:
        __glDebugPrintLogMessage(gc, GL_DEBUG_SOURCE_API_KHR, GL_DEBUG_TYPE_ERROR_KHR, 0, GL_DEBUG_SEVERITY_HIGH_KHR,
                                 "glGetObjectLabelKHR generated INVALID_ENUM error because identifier is: 0x%04x", identifier);
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    len = objLabel ? (GLsizei)strlen(objLabel) : 0;
    if (label && bufSize > 0)
    {
        len = __GL_MIN(len, bufSize - 1);
        if (len > 0)
        {
            __GL_MEMCOPY(label, objLabel, len);
        }
        label[len] = '\0';
    }

    if (length)
    {
        *length = len;
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ObjectPtrLabel(__GLcontext *gc, const GLvoid* ptr, GLsizei length, const GLchar *label)
{
     __GLsyncObject *syncObj = (__GLsyncObject*)__glGetObject(gc, gc->sync.shared, __GL_PTR2UINT(ptr));

    __GL_HEADER();

    if (label)
    {
        if ((length < 0 ? (GLsizei)strlen(label) : length) >= gc->debug.maxMsgLen)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
    }

    if (!syncObj)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (syncObj->label)
    {
        gcmOS_SAFE_FREE(gcvNULL, syncObj->label);
        syncObj->label = NULL;
    }

    if (label)
    {
        GLsizei msgLen = (length < 0) ? (GLsizei)strlen(label) : length;
        msgLen = __GL_MIN(gc->debug.maxMsgLen - 1, msgLen);

        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, (msgLen + 1) * sizeof(GLchar), (gctPOINTER *)&syncObj->label)))
        {
            __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
        }

        __GL_MEMCOPY(syncObj->label, label, msgLen * sizeof(GLchar));
        syncObj->label[msgLen] = '\0';
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetObjectPtrLabel(__GLcontext *gc, const GLvoid* ptr, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    GLsizei len = 0;
    __GLsyncObject *syncObj = (__GLsyncObject*)__glGetObject(gc, gc->sync.shared, __GL_PTR2UINT(ptr));

    __GL_HEADER();

    if (!syncObj)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (bufSize < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    len = syncObj->label ? (GLsizei)strlen(syncObj->label) : 0;
    if (label && bufSize > 0)
    {
        len = __GL_MIN(len, bufSize - 1);
        if (len > 0)
        {
            __GL_MEMCOPY(label, syncObj->label, len);
        }
        label[len] = '\0';
    }

    if (length)
    {
        *length = len;
    }

OnError:
    __GL_FOOTER();
}


GLvoid GL_APIENTRY __gles_Enablei(__GLcontext *gc, GLenum target, GLuint index)
{
    __GL_HEADER();

    if (target != GL_BLEND)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (index >= gc->constants.shaderCaps.maxDrawBuffers)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (!gc->state.enables.colorBuffer.blend[index])
    {
        gc->state.enables.colorBuffer.blend[index] = gcvTRUE;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLEND_ENDISABLE_BIT);
    }

OnError:
    __GL_FOOTER();

    return;
}

GLvoid GL_APIENTRY __gles_Disablei(__GLcontext *gc, GLenum target, GLuint index)
{
    __GL_HEADER();

    if (target != GL_BLEND)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (index >= gc->constants.shaderCaps.maxDrawBuffers)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (gc->state.enables.colorBuffer.blend[index])
    {
        gc->state.enables.colorBuffer.blend[index] = gcvFALSE;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLEND_ENDISABLE_BIT);
    }

OnError:
    __GL_FOOTER();

    return;
}

GLboolean GL_APIENTRY __gles_IsEnabledi(__GLcontext * gc, GLenum target, GLuint index)
{
    GLboolean ret = GL_FALSE;

    __GL_HEADER();

    if (target != GL_BLEND)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (index >= gc->constants.shaderCaps.maxDrawBuffers)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    ret = gc->state.enables.colorBuffer.blend[index];

OnError:
    __GL_FOOTER();

    return ret;
}

GLvoid GL_APIENTRY __gles_PrimitiveBoundingBox(__GLcontext *gc, GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW,
                                               GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW)
{
    __GL_HEADER();

    gc->state.primBound.minX = minX;
    gc->state.primBound.minY = minY;
    gc->state.primBound.minZ = minZ;
    gc->state.primBound.minW = minW;
    gc->state.primBound.maxX = maxX;
    gc->state.primBound.maxY = maxY;
    gc->state.primBound.maxZ = maxZ;
    gc->state.primBound.maxW = maxW;

    __GL_FOOTER();

    return;
}


