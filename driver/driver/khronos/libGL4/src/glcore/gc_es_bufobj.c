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


#include "gc_es_context.h"
#include "gc_es_object_inline.c"

#define __GL_GET_BUFFER_TARGET_INDEX(target, targetIndex)   \
    switch (target)                                         \
    {                                                       \
    case GL_ARRAY_BUFFER:                                   \
        targetIndex = __GL_ARRAY_BUFFER_INDEX;              \
        break;                                              \
    case GL_ELEMENT_ARRAY_BUFFER:                           \
        targetIndex = __GL_ELEMENT_ARRAY_BUFFER_INDEX;      \
        break;                                              \
    case GL_COPY_READ_BUFFER:                               \
        targetIndex = __GL_COPY_READ_BUFFER_INDEX;          \
        break;                                              \
    case GL_COPY_WRITE_BUFFER:                              \
        targetIndex = __GL_COPY_WRITE_BUFFER_INDEX;         \
        break;                                              \
    case GL_PIXEL_PACK_BUFFER:                              \
        targetIndex = __GL_PIXEL_PACK_BUFFER_INDEX;         \
        break;                                              \
    case GL_PIXEL_UNPACK_BUFFER:                            \
        targetIndex = __GL_PIXEL_UNPACK_BUFFER_INDEX;       \
        break;                                              \
    case GL_UNIFORM_BUFFER:                                 \
        targetIndex = __GL_UNIFORM_BUFFER_INDEX;            \
        break;                                              \
    case GL_TRANSFORM_FEEDBACK_BUFFER:                      \
        targetIndex = __GL_XFB_BUFFER_INDEX;                \
        break;                                              \
    case GL_DRAW_INDIRECT_BUFFER:                           \
        targetIndex = __GL_DRAW_INDIRECT_BUFFER_INDEX;      \
        break;                                              \
    case GL_DISPATCH_INDIRECT_BUFFER:                       \
        targetIndex = __GL_DISPATCH_INDIRECT_BUFFER_INDEX;  \
        break;                                              \
    case GL_ATOMIC_COUNTER_BUFFER:                          \
        targetIndex = __GL_ATOMIC_COUNTER_BUFFER_INDEX;     \
        break;                                              \
    case GL_SHADER_STORAGE_BUFFER:                          \
        targetIndex = __GL_SHADER_STORAGE_BUFFER_INDEX;     \
        break;                                              \
    case GL_TEXTURE_BUFFER_EXT:                             \
        targetIndex = __GL_TEXTURE_BUFFER_BINDING_EXT;      \
        break;                                              \
    default:                                                \
        __GL_ERROR_EXIT(GL_INVALID_ENUM);                   \
    }

#define _GC_OBJ_ZONE gcdZONE_GL40_CORE

GLvoid __glInitBufferObject(__GLcontext *gc, __GLbufferObject *bufObj, GLuint name)
{
    bufObj->bindCount           = 0;
    bufObj->accessFlags         = 0;
    bufObj->access              = gc->imports.conformGLSpec? GL_READ_WRITE:GL_WRITE_ONLY_OES;
    bufObj->bufferMapped        = GL_FALSE;
    bufObj->mapPointer          = gcvNULL;
    bufObj->mapOffset           = 0;
    bufObj->mapLength           = 0;
    bufObj->usage               = GL_STATIC_DRAW;
    bufObj->size                = 0;
    bufObj->privateData         = gcvNULL;
    bufObj->vaoList             = gcvNULL;
    bufObj->texList             = gcvNULL;
    bufObj->name                = name;
}

GLvoid __glBindBufferToGeneralPoint(__GLcontext *gc, GLuint targetIndex, GLuint buffer)
{
    __GLbufferObject *newBufObj, *oldBufObj;
    __GLBufGeneralBindPoint *bindingPoint = &gc->bufferObject.generalBindingPoint[targetIndex];
    __GLvertexArrayState *vertexArrayState = &gc->vertexArray.boundVAO->vertexArray;

    __GL_HEADER();

    if (buffer)
    {
        /* Retrieve the buffer object from the "gc->bufferObject.shared" structure.*/
        newBufObj = (__GLbufferObject *)__glGetObject(gc, gc->bufferObject.shared, buffer);

        if (newBufObj == gcvNULL)
        {
            newBufObj = (__GLbufferObject*)(*gc->imports.calloc)(gc, 1, sizeof(__GLbufferObject));
            if (!newBufObj)
            {
                __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
            }

            __glInitBufferObject(gc, newBufObj, buffer);

            /* Add this buffer object to the "gc->bufferObject.shared" structure. */
            __glAddObject(gc, gc->bufferObject.shared, buffer, newBufObj);

            /* Mark the name "buffer" used in the buffer object nameArray.*/
            __glMarkNameUsed(gc, gc->bufferObject.shared, buffer);
        }
        else
        {
            GL_ASSERT(buffer == newBufObj->name);
        }
    }
    else
    {
        /* Bind the buffer target to 0 which means conventional vertex array */
        newBufObj = gcvNULL;
    }

    oldBufObj = bindingPoint->boundBufObj;
    if (oldBufObj != newBufObj)
    {
        /* Unbound the originally bound buffer object and bind to the new object */
        bindingPoint->boundBufName = buffer;
        bindingPoint->boundBufObj  = newBufObj;

        /* Decrease bindCount. Delete boundBufObj if there is nothing bound to the object
        */
        if (oldBufObj != gcvNULL)
        {
            --oldBufObj->bindCount;
            if (!oldBufObj->bindCount && !oldBufObj->vaoList &&
                !oldBufObj->texList && (oldBufObj->flag & __GL_OBJECT_IS_DELETED))
            {
                __glDeleteBufferObject(gc, oldBufObj);
            }
        }

        /* bindCount includes both single context and shared context bindings.
        */
        if (buffer)
        {
            GLuint binding;
            GLboolean retVal;

            newBufObj->bindCount++;

            /* Call the dp interface */
            retVal = (*gc->dp.bindBuffer)(gc, newBufObj, targetIndex);

            if (!retVal)
            {
                __GL_ERROR((*gc->dp.getError)(gc));
            }

            /* Make sure targetIdex is within 16bits range */
            GL_ASSERT(targetIndex < (1 << 16));
            /* Encoded targetIndex at high 16bits, and 0xFFFF at low 16bits. */
            binding = (targetIndex << 16 | 0xFFFF);
            __glAddImageUser(gc, &newBufObj->bindList, (GLvoid*)(GLintptr)binding);
        }
    }

    oldBufObj = vertexArrayState->boundIdxObj;
    if (targetIndex == __GL_ELEMENT_ARRAY_BUFFER_INDEX && oldBufObj != newBufObj)
    {
        if (gc->vertexArray.boundVertexArray)
        {
            /* Remove current VAO from old buffer object's vaoList */
            if (oldBufObj)
            {
                __glRemoveImageUser(gc, &oldBufObj->vaoList, gc->vertexArray.boundVAO);
                if (!oldBufObj->bindCount && !oldBufObj->vaoList &&
                    !oldBufObj->texList && (oldBufObj->flag & __GL_OBJECT_IS_DELETED))
                {
                    __glDeleteBufferObject(gc, oldBufObj);
                }
            }

            /* Add current VAO into new buffer object's vaoList */
            if (newBufObj)
            {
                __glAddImageUser(gc, &newBufObj->vaoList, gc->vertexArray.boundVAO);
            }
        }

        vertexArrayState->boundIdxName = buffer;
        vertexArrayState->boundIdxObj  = newBufObj;
    }

#ifdef OPENGL40
    if (targetIndex == __GL_ARRAY_BUFFER_INDEX)
    {
        /* Set ARRAY_BUFFER binding point in vertex array client state */
        vertexArrayState->arrayBufBinding = buffer;
    }
#endif

OnError:
    __GL_FOOTER();
    return;
}

GLvoid __glBindBufferToArrayPoint(__GLcontext *gc, GLuint targetIndex, GLuint bindingPointIdx,
                                  GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    __GLbufferObject *bufObj;
    __GLBufBindPoint *pBindingPoint = gc->bufferObject.bindingPoints[targetIndex];

    __GL_HEADER();
    /* Ensure the target buffer obj has an array of binding point */
    GL_ASSERT(pBindingPoint);
    if (buffer)
    {
        bufObj = (__GLbufferObject*)__glGetObject(gc, gc->bufferObject.shared, buffer);

        if (!bufObj || bufObj->name != buffer)
        {
            GL_ASSERT(0);
        }
    }
    else
    {
        bufObj = gcvNULL;
    }

    pBindingPoint = &pBindingPoint[bindingPointIdx];

    pBindingPoint->boundBufName = buffer;
    pBindingPoint->boundBufObj  = bufObj;
    pBindingPoint->bufOffset    = offset;
    pBindingPoint->bufSize      = size;

    __glBitmaskSet(&gc->bufferObject.bindingDirties[targetIndex], bindingPointIdx);

    if (bufObj)
    {
        GLuint binding;

        /* Make sure targetIdex and bindingPointIdx are within 16bits range */
        GL_ASSERT(targetIndex < (1 << 16));
        GL_ASSERT(bindingPointIdx < (1 << 16) - 1);

        /* Encoded targetIndex at high 16bits, while bindingPointIdx at low 16bits. */
        binding = (targetIndex << 16 | bindingPointIdx);
        __glAddImageUser(gc, &bufObj->bindList, (GLvoid*)(GLintptr)binding);
    }

    __GL_FOOTER();
}


GLvoid __glBindBufferToXfb(__GLcontext *gc, GLuint buffer)
{
    __GLbufferObject *newBufObj, *oldBufObj;
    __GLxfbObject *xfbObj = gc->xfb.boundXfbObj;

    __GL_HEADER();

    if (buffer)
    {
        /* Retrieve the buffer object from the "gc->bufferObject.shared" structure.*/
        newBufObj = (__GLbufferObject *)__glGetObject(gc, gc->bufferObject.shared, buffer);

        if (newBufObj == gcvNULL)
        {
            newBufObj = (__GLbufferObject*)(*gc->imports.calloc)(gc, 1, sizeof(__GLbufferObject));
            if (!newBufObj)
            {
                __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
            }

            __glInitBufferObject(gc, newBufObj, buffer);

            /* Add this buffer object to the "gc->bufferObject.shared" structure. */
            __glAddObject(gc, gc->bufferObject.shared, buffer, newBufObj);

            /* Mark the name "buffer" used in the buffer object nameArray.*/
            __glMarkNameUsed(gc, gc->bufferObject.shared, buffer);
        }
        else
        {
            GL_ASSERT(buffer == newBufObj->name);
        }
    }
    else
    {
        /* Bind the buffer target to 0 which means conventional vertex array */
        newBufObj = gcvNULL;
    }

    if (xfbObj->boundBufObj != newBufObj)
    {

        oldBufObj = xfbObj->boundBufObj;

        xfbObj->boundBufObj = newBufObj;
        xfbObj->boundBufName = buffer;
        /* Decrease bindCount. Delete boundBufObj if there is nothing bound to the object
        */
        if (oldBufObj != gcvNULL)
        {
            --oldBufObj->bindCount;
            if (!oldBufObj->bindCount && !oldBufObj->vaoList &&
                !oldBufObj->texList && (oldBufObj->flag & __GL_OBJECT_IS_DELETED))
            {
                __glDeleteBufferObject(gc, oldBufObj);
            }
        }

        /* bindCount includes both single context and shared context bindings.
        */
        if (newBufObj)
        {
            GLboolean retVal;

            newBufObj->bindCount++;

            /* Call the dp interface */
            retVal = (*gc->dp.bindBuffer)(gc, newBufObj, __GL_XFB_BUFFER_INDEX);

            if (!retVal)
            {
                __GL_ERROR((*gc->dp.getError)(gc));
            }
        }
    }

OnError:
    __GL_FOOTER();
    return;
}


GLvoid __glBindBufferToXfbStream(__GLcontext *gc, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    __GLbufferObject *bufObj;
    __GLxfbObject    *xfbObj = gc->xfb.boundXfbObj;
    __GLBufBindPoint *pBufBinding = &xfbObj->boundBufBinding[index];

    __GL_HEADER();
    /* Ensure the target buffer obj has an array of binding point */
    if (buffer)
    {
        bufObj = (__GLbufferObject*)__glGetObject(gc, gc->bufferObject.shared, buffer);

        if (!bufObj || bufObj->name != buffer)
        {
            GL_ASSERT(0);
        }
    }
    else
    {
        bufObj = gcvNULL;
    }

    pBufBinding->boundBufName = buffer;
    pBufBinding->boundBufObj  = bufObj;
    pBufBinding->bufOffset    = offset;
    pBufBinding->bufSize      = size;

    __GL_FOOTER();
    return;
}


GLvoid __glGetBufferParameteri64v(__GLcontext *gc, GLenum target, GLenum pname, GLint64 *params)
{
    GLuint targetIndex;
    __GLbufferObject * bufObj;

    __GL_HEADER();

    __GL_GET_BUFFER_TARGET_INDEX(target, targetIndex);

    if (!params)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    bufObj = __glGetBoundBufObj(gc, targetIndex);
    if (!bufObj)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    switch (pname)
    {
    case GL_BUFFER_SIZE:
        *params = (GLint64)(GLintptr)bufObj->size;
        break;
    case GL_BUFFER_USAGE:
        *params = (GLint64)bufObj->usage;
        break;
    case GL_BUFFER_ACCESS_FLAGS:
        *params = (GLint64)bufObj->accessFlags;
        break;
    case GL_BUFFER_ACCESS:
        *params = (GLint64)bufObj->access;
        break;
    case GL_BUFFER_MAPPED:
        *params = (GLint64)bufObj->bufferMapped;
        break;
    case GL_BUFFER_MAP_OFFSET:
        *params = (GLint64)bufObj->mapOffset;
        break;
    case GL_BUFFER_MAP_LENGTH:
        *params = (GLint64)bufObj->mapLength;
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
    return;

}

GLboolean __glDeleteBufferObject(__GLcontext *gc, __GLbufferObject *bufObj)
{
    GLboolean retVal;
    GLuint targetIndex, bindingIndex, activeUnit;
    __GLvertexArrayState *vertexArrayState = &gc->vertexArray.boundVAO->vertexArray;
    GLboolean ret = GL_TRUE;

    __GL_HEADER();
    /*
    ** __GL_OBJECT_IS_DELETED is cleared here because we do not want this object
    ** deleted in the following bind functions, __glBindBufferToGeneralPoint, __glBindBufferToArrayPoint.
    ** otherwise there will be recursion: __glDeleteBufferObject-->__glBindObject-->__glDeleteBufferObject,
    ** and the object will be deleted twice.
    */
    bufObj->flag &= ~__GL_OBJECT_IS_DELETED;

    /*
    ** If the buffer object that is being deleted is currently bound to gc,
    ** bind 0 to the target.
    */
    for (targetIndex = 0; targetIndex < __GL_MAX_BUFFER_INDEX; targetIndex++)
    {
        if (gc->bufferObject.generalBindingPoint[targetIndex].boundBufObj == bufObj)
        {
            __glBindBufferToGeneralPoint(gc, targetIndex, 0);

            /* Unmap if it is mapped */
            if (bufObj->bufferMapped)
            {
                gc->dp.unmapBuffer(gc, bufObj, targetIndex);
            }
        }

        /* Remove bufObj from the array binding points */
        for (bindingIndex = 0; bindingIndex < gc->bufferObject.maxBufBindings[targetIndex]; bindingIndex++)
        {
            __GLBufBindPoint *pBindingPoint = &gc->bufferObject.bindingPoints[targetIndex][bindingIndex];
            if (pBindingPoint->boundBufObj == bufObj)
            {
                __glBindBufferToArrayPoint(gc, targetIndex, bindingIndex, 0, 0, 0);
            }
        }
    }

    if (gc->xfb.boundXfbObj->boundBufObj == bufObj)
    {
        __glBindBufferToXfb(gc, 0);
    }

    /* Remove bufObj from the array binding points */
    for (bindingIndex = 0; bindingIndex < gc->constants.shaderCaps.maxXfbSeparateAttribs; bindingIndex++)
    {
        __GLBufBindPoint *pBindingPoint = &gc->xfb.boundXfbObj->boundBufBinding[bindingIndex];
        if (pBindingPoint->boundBufObj == bufObj)
        {
            __glBindBufferToXfbStream(gc, bindingIndex, 0, 0, 0);
        }
    }

    /* Check all the binding points in vertexArrayState. */
    for (bindingIndex = 0; bindingIndex < __GL_MAX_VERTEX_ATTRIBUTE_BINDINGS; bindingIndex++)
    {
        if(gc->imports.conformGLSpec)
        {
            if (vertexArrayState->attributeBinding[bindingIndex].boundArrayObj == bufObj)
            {
                if (gc->vertexArray.boundVertexArray)
                {
                    __glRemoveImageUser(gc, &bufObj->vaoList, gc->vertexArray.boundVAO);
                }

                vertexArrayState->attributeBinding[bindingIndex].boundArrayName = 0;
                vertexArrayState->attributeBinding[bindingIndex].boundArrayObj = gcvNULL;
            }
        }
        else
        {
            __GLvertexAttribBinding *pAttribBinding = &vertexArrayState->attributeBinding[bindingIndex];

            if (gc->vertexArray.boundVertexArray)
            {
                if (pAttribBinding->boundArrayObj == bufObj)
                {
                    __glRemoveImageUser(gc, &bufObj->vaoList, gc->vertexArray.boundVAO);
                    pAttribBinding->boundArrayName = 0;
                    pAttribBinding->boundArrayObj = gcvNULL;
                }
            }
            else
            {
                if (pAttribBinding->boundArrayName == bufObj->name)
                {
                    pAttribBinding->boundArrayName = 0;
                    pAttribBinding->boundArrayObj = gcvNULL;
                }
            }
        }
    }

    if (vertexArrayState->boundIdxObj == bufObj)
    {
        if (gc->vertexArray.boundVertexArray)
        {
            __glRemoveImageUser(gc, &bufObj->vaoList, gc->vertexArray.boundVAO);
        }

        vertexArrayState->boundIdxName = 0;
        vertexArrayState->boundIdxObj  = NULL;
    }

    for (activeUnit = 0; activeUnit < __GL_MAX_TEXTURE_UNITS; activeUnit++)
    {
        __GLtextureObject *tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_BINDING_BUFFER_EXT];
        if (tex && (tex->bufObj == bufObj))
        {
            __glRemoveImageUser(gc, &bufObj->texList, tex);
            tex->bufObj = gcvNULL;
            tex->bufOffset = 0;
            tex->bufSize = 0;
        }
    }

    /* Do not delete the buffer object if there are other contexts bound to it. */
    if (bufObj->bindCount != 0 || bufObj->vaoList || bufObj->texList)
    {
        /* Set the flag to indicate the object is marked for delete */
        bufObj->flag |= __GL_OBJECT_IS_DELETED;
        ret = GL_FALSE;
        __GL_EXIT();
    }

    if (bufObj->label)
    {
        gc->imports.free(gc, bufObj->label);
    }

    /* Notify Dp that this buffer object is deleted.
    */
    retVal = (*gc->dp.deleteBuffer)(gc, bufObj);

    if (!retVal)
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    __glFreeImageUserList(gc, &bufObj->bindList);

    /* Delete the buffer object structure */
    (*gc->imports.free)(gc, bufObj);

OnExit:
    __GL_FOOTER();
    return ret;
}

GLvoid __glInitBufferObjectState(__GLcontext *gc)
{
    GLuint bufIdx = 0;

    __GL_HEADER();

    __GL_MEMZERO(gc->bufferObject.maxBufBindings, __GL_MAX_BUFFER_INDEX * sizeof(GLuint));

    /* Only certain types have the array binding points */
    gc->bufferObject.maxBufBindings[__GL_UNIFORM_BUFFER_INDEX]        = gc->constants.shaderCaps.maxUniformBufferBindings;
    gc->bufferObject.maxBufBindings[__GL_XFB_BUFFER_INDEX]            = gc->constants.shaderCaps.maxXfbSeparateAttribs;
    gc->bufferObject.maxBufBindings[__GL_ATOMIC_COUNTER_BUFFER_INDEX] = gc->constants.shaderCaps.maxAtomicCounterBufferBindings;
    gc->bufferObject.maxBufBindings[__GL_SHADER_STORAGE_BUFFER_INDEX] = gc->constants.shaderCaps.maxShaderStorageBufferBindings;

    for (bufIdx = 0; bufIdx < __GL_MAX_BUFFER_INDEX; bufIdx++)
    {
        if (gc->bufferObject.maxBufBindings[bufIdx])
        {
            gc->bufferObject.bindingPoints[bufIdx] =
                (*gc->imports.calloc)(gc, gc->bufferObject.maxBufBindings[bufIdx], sizeof(__GLBufBindPoint));
        }
        else
        {
            gc->bufferObject.bindingPoints[bufIdx] = gcvNULL;
        }

        __glBitmaskInitAllZero(&gc->bufferObject.bindingDirties[bufIdx], gc->bufferObject.maxBufBindings[bufIdx]);
    }

    /* Buffer objects can be shared across contexts */
    if (gc->shareCtx)
    {
        GL_ASSERT(gc->shareCtx->bufferObject.shared);
        gc->bufferObject.shared = gc->shareCtx->bufferObject.shared;
        gcoOS_LockPLS();
        gc->bufferObject.shared->refcount++;

        /* Allocate VEGL lock */
        if (gcvNULL == gc->bufferObject.shared->lock)
        {
            gc->bufferObject.shared->lock = (*gc->imports.calloc)(gc, 1, sizeof(VEGLLock));
            (*gc->imports.createMutex)(gc->bufferObject.shared->lock);
        }
        gcoOS_UnLockPLS();
    }
    else
    {
        GL_ASSERT(gcvNULL == gc->bufferObject.shared);

        gc->bufferObject.shared = (__GLsharedObjectMachine*)(*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine));

        /* Initialize a linear lookup table for vertex buffer object */
        gc->bufferObject.shared->maxLinearTableSize = __GL_MAX_BUFOBJ_LINEAR_TABLE_SIZE;
        gc->bufferObject.shared->linearTableSize = __GL_DEFAULT_BUFOBJ_LINEAR_TABLE_SIZE;
        gc->bufferObject.shared->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, gc->bufferObject.shared->linearTableSize * sizeof(GLvoid *));

        gc->bufferObject.shared->hashSize = __GL_BUFOBJ_HASH_TABLE_SIZE;
        gc->bufferObject.shared->hashMask = __GL_BUFOBJ_HASH_TABLE_SIZE - 1;
        gc->bufferObject.shared->refcount = 1;
        gc->bufferObject.shared->deleteObject = (__GLdeleteObjectFunc)__glDeleteBufferObject;
        gc->bufferObject.shared->immediateInvalid = GL_TRUE;
    }

    __GL_FOOTER();
    return;
}

GLvoid __glFreeBufferObjectState(__GLcontext *gc)
{
    GLuint bufIdx = 0;

    __GL_HEADER();

    /* unbind buffer object from all targets */
    for (bufIdx = 0; bufIdx < __GL_MAX_BUFFER_INDEX; bufIdx++)
    {
        __glBindBufferToGeneralPoint(gc, bufIdx, 0);
    }

    /* Free shared buffer object table */
    __glFreeSharedObjectState(gc, gc->bufferObject.shared);

    for (bufIdx = 0; bufIdx < __GL_MAX_BUFFER_INDEX; bufIdx++)
    {
        if (gc->bufferObject.bindingPoints[bufIdx])
        {
            (*gc->imports.free)(gc, gc->bufferObject.bindingPoints[bufIdx]);
            gc->bufferObject.bindingPoints[bufIdx] = gcvNULL;
        }
    }

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_BindBuffer(__GLcontext *gc, GLenum target, GLuint buffer)
{
    GLuint targetIndex;

    __GL_HEADER();

    __GL_GET_BUFFER_TARGET_INDEX(target, targetIndex);

#ifdef OPENGL40
    if (gc->imports.conformGLSpec &&
        buffer && !__glIsNameDefined(gc, gc->bufferObject.shared, buffer))
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
#endif

    __glBindBufferToGeneralPoint(gc, targetIndex, buffer);

    if (targetIndex == __GL_XFB_BUFFER_INDEX)
    {
        __glBindBufferToXfb(gc, buffer);
    }

OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __glim_BindBufferBase(__GLcontext *gc, GLenum target, GLuint index, GLuint buffer)
{
    GLuint targetIndex;

    __GL_HEADER();

#ifdef OPENGL40
    if (gc->imports.conformGLSpec &&
        buffer && !__glIsNameDefined(gc, gc->bufferObject.shared, buffer))
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
#endif

    switch (target)
    {
    case GL_TRANSFORM_FEEDBACK_BUFFER:
        if (gc->xfb.boundXfbObj->active)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        targetIndex = __GL_XFB_BUFFER_INDEX;
        break;
    case GL_UNIFORM_BUFFER:
        targetIndex = __GL_UNIFORM_BUFFER_INDEX;
        break;
    case GL_ATOMIC_COUNTER_BUFFER:
        targetIndex = __GL_ATOMIC_COUNTER_BUFFER_INDEX;
        break;
    case GL_SHADER_STORAGE_BUFFER:
        targetIndex = __GL_SHADER_STORAGE_BUFFER_INDEX;
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (index >= gc->bufferObject.maxBufBindings[targetIndex])
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    /*
    ** Spec requires to bind the buffer object named by <buffer> to the general
    ** binding point, and additionally bind the buffer object to the binding point
    ** in the array given by <index>.
    */
    __glBindBufferToGeneralPoint(gc, targetIndex, buffer);

    /*
    ** The buffer's bindCount has been considered in the __glBindBufferToGeneralPoint,
    ** so __glBindBufferToArrayPoint doesn't handle bindCount.
    */
    __glBindBufferToArrayPoint(gc, targetIndex, index, buffer, 0, 0);

    if (targetIndex == __GL_XFB_BUFFER_INDEX)
    {
        __glBindBufferToXfb(gc, buffer);
        __glBindBufferToXfbStream(gc, index, buffer, 0, 0);
    }

OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __glim_BindBufferRange(__GLcontext *gc, GLenum target, GLuint index,
                                          GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    GLuint targetIndex;

    __GL_HEADER();

    if ((buffer != 0) && (size <= 0))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec &&
        buffer && !__glIsNameDefined(gc, gc->bufferObject.shared, buffer))
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
#endif

    switch (target)
    {
    case GL_TRANSFORM_FEEDBACK_BUFFER:
        if (gc->xfb.boundXfbObj->active)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        if ((size & 0x3) != 0 || (offset & 0x3) != 0)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        targetIndex = __GL_XFB_BUFFER_INDEX;
        break;

    case GL_UNIFORM_BUFFER:
        if (offset % gc->constants.shaderCaps.uniformBufferOffsetAlignment != 0)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        targetIndex = __GL_UNIFORM_BUFFER_INDEX;
        break;

    case GL_ATOMIC_COUNTER_BUFFER:
        if ((offset & 0x3) != 0)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        targetIndex = __GL_ATOMIC_COUNTER_BUFFER_INDEX;
        break;
    case GL_SHADER_STORAGE_BUFFER:
        if (offset % gc->constants.shaderCaps.shaderStorageBufferOffsetAlignment != 0)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        targetIndex = __GL_SHADER_STORAGE_BUFFER_INDEX;
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (index >= gc->bufferObject.maxBufBindings[targetIndex])
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    /*
    ** Spec requires to bind the buffer object named by <buffer> to the general
    ** binding point, and additionally bind the buffer object to the binding point
    ** in the array given by <index>.
    */
    __glBindBufferToGeneralPoint(gc, targetIndex, buffer);

    /*
    ** The buffer's bindCount has been considered in the __glBindBufferToGeneralPoint,
    ** so __glBindBufferToArrayPoint doesn't handle bindCount.
    */
    __glBindBufferToArrayPoint(gc, targetIndex, index, buffer, offset, size);

    if (targetIndex == __GL_XFB_BUFFER_INDEX)
    {
        __glBindBufferToXfb(gc, buffer);
        __glBindBufferToXfbStream(gc, index, buffer, offset, size);
    }

OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __glim_DeleteBuffers(__GLcontext *gc, GLsizei n, const GLuint *buffers)
{
    GLint i;

    __GL_HEADER();

    if (n < 0 )
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    for (i = 0; i < n; i++)
    {
        if (buffers[i])
        {
            __glDeleteObject(gc, gc->bufferObject.shared, buffers[i]);
        }
    }
OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __glim_GenBuffers(__GLcontext *gc, GLsizei n, GLuint * buffers)
{
    GLint start, i;

    __GL_HEADER();

    if (n < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (gcvNULL == buffers)
    {
        __GL_EXIT();
    }

    GL_ASSERT(gcvNULL != gc->bufferObject.shared);

    start = __glGenerateNames(gc, gc->bufferObject.shared, n);
    for (i = 0; i < n; i++)
    {
        buffers[i] = start + i;
    }

    if (gc->bufferObject.shared->linearTable)
    {
        __glCheckLinearTableSize(gc, gc->bufferObject.shared, (start + n));
    }

OnError:

OnExit:
    __GL_FOOTER();
    return;
}

GLboolean GL_APIENTRY __glim_IsBuffer(__GLcontext *gc, GLuint buffer)
{
    return (gcvNULL != __glGetObject(gc, gc->bufferObject.shared, buffer));
}

GLvoid GL_APIENTRY __glim_BufferData(__GLcontext *gc, GLenum target, GLsizeiptr size,
                                     const GLvoid *data, GLenum usage)
{
    GLuint targetIndex;
    __GLbufferObject *bufObj;
    __GLimageUser *bindUser;

    __GL_HEADER();

    __GL_GET_BUFFER_TARGET_INDEX(target, targetIndex);

    if (size < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    switch (usage)
    {
    case GL_STREAM_DRAW:
    case GL_STREAM_READ:
    case GL_STREAM_COPY:
    case GL_STATIC_DRAW:
    case GL_STATIC_READ:
    case GL_STATIC_COPY:
    case GL_DYNAMIC_DRAW:
    case GL_DYNAMIC_READ:
    case GL_DYNAMIC_COPY:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    bufObj = __glGetBoundBufObj(gc, targetIndex);
    if (!bufObj)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    bufObj->usage = usage;
    bufObj->size = size;

    if (!(*gc->dp.bufferData)(gc, bufObj, targetIndex, data))
    {
        __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
    }

    bindUser = bufObj->bindList;
    while (bindUser)
    {
        GLuint binding = __GL_PTR2UINT(bindUser->imageUser);

        GLuint targetIdx = (binding >> 16);
        GLuint arrayIdx = (binding & 0xFFFF);

        GL_ASSERT(targetIdx < __GL_MAX_BUFFER_INDEX);

        /* If the bufObj is really bound to the point, mark dirty so validation will program
        ** the new allocated address to HW
        */
        if (arrayIdx < gc->bufferObject.maxBufBindings[targetIdx] &&
            gc->bufferObject.bindingPoints[targetIdx][arrayIdx].boundBufObj == bufObj)
        {
            __glBitmaskSet(&gc->bufferObject.bindingDirties[targetIdx], arrayIdx);
        }

        bindUser = bindUser->next;
    }

OnError:
    __GL_FOOTER();
    return;

}

GLvoid GL_APIENTRY __glim_BufferSubData(__GLcontext *gc, GLenum target, GLintptr offset,
                                        GLsizeiptr size, const GLvoid *data)
{
    GLuint targetIndex;
    __GLbufferObject *bufObj;

    __GL_HEADER();

    __GL_GET_BUFFER_TARGET_INDEX(target, targetIndex);

    bufObj = __glGetBoundBufObj(gc, targetIndex);
    if (!bufObj)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (offset < 0 || size < 0 || (size + offset) > bufObj->size)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    /* If any part is mapped */
    if (bufObj->bufferMapped)
    {
        GLintptr mapStart = bufObj->mapOffset;
        GLintptr mapEnd   = mapStart + bufObj->mapLength;
        if ((offset >= mapStart && offset < mapEnd) ||
            (offset+size > mapStart && offset+size <= mapEnd) ||
            (offset < mapStart && offset+size > mapEnd))
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
    }

    if (data && size)
    {
        (*gc->dp.bufferSubData)(gc, bufObj, targetIndex, offset, size, data);
    }
OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __glim_CopyBufferSubData(__GLcontext *gc, GLenum readTarget, GLenum writeTarget,
                                            GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    GLuint readTargetIndex;
    GLuint writeTargetIndex;
    __GLbufferObject *readBufObj;
    __GLbufferObject *writeBufObj;

    __GL_HEADER();

    __GL_GET_BUFFER_TARGET_INDEX(readTarget, readTargetIndex);
    __GL_GET_BUFFER_TARGET_INDEX(writeTarget, writeTargetIndex);

    readBufObj  = __glGetBoundBufObj(gc, readTargetIndex);
    writeBufObj = __glGetBoundBufObj(gc, writeTargetIndex);
    if (!readBufObj || !writeBufObj)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (size < 0 || readOffset < 0 || writeOffset < 0 ||
        (size + readOffset) > readBufObj->size || (size + writeOffset) > writeBufObj->size ||
        (readBufObj == writeBufObj &&
         ((readOffset <= writeOffset && (readOffset + size) > writeOffset) ||
          (writeOffset <= readOffset && (writeOffset + size) > readOffset)
         )
        )
       )
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (readBufObj->bufferMapped || writeBufObj->bufferMapped)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    (*gc->dp.copyBufferSubData)(gc, readTargetIndex, readBufObj, writeTargetIndex, writeBufObj, readOffset, writeOffset, size);

OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __glim_GetBufferSubData(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data)
{
    GLuint targetIndex;
    __GLbufferObject *readBufObj;

    __GL_HEADER();
    __GL_GET_BUFFER_TARGET_INDEX(target, targetIndex);

    readBufObj  = __glGetBoundBufObj(gc, targetIndex);

    if (!readBufObj )
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (size < 0 || (size + offset) > readBufObj->size)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (readBufObj->bufferMapped)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    (*gc->dp.getBufferSubData)(gc, targetIndex, readBufObj, offset, size, data);

    OnError:
    __GL_FOOTER();
    return;
}

GLvoid* GL_APIENTRY __glim_MapBufferRange(__GLcontext *gc, GLenum target, GLintptr offset,
                                          GLsizeiptr length, GLbitfield access)
{
    GLuint targetIndex;
    __GLbufferObject *bufObj;
    void * result = gcvNULL;

    __GL_HEADER();

    __GL_GET_BUFFER_TARGET_INDEX(target, targetIndex);

    if (access & ~(GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT |
                   GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT |
                   GL_MAP_BUFFER_OBJ_VIV))
    {
        /* Bitfield not defined */
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    if (!(access & (GL_MAP_READ_BIT | GL_MAP_WRITE_BIT)))
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    if ((access & GL_MAP_READ_BIT) &&
        (access & (GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT)))
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    if ((access & GL_MAP_FLUSH_EXPLICIT_BIT) && (!(access & GL_MAP_WRITE_BIT)))
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    bufObj = __glGetBoundBufObj(gc, targetIndex);
    if (!bufObj)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (bufObj->bufferMapped)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    if ((offset < 0) || (length < 0) || ((offset + length) > bufObj->size))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
    if (0 == length)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    result = (*gc->dp.mapBufferRange)(gc, bufObj, targetIndex, offset, length, access);
    if (gcvNULL == result)
    {
        __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
    }
    bufObj->accessFlags = access;

    if(gc->imports.conformGLSpec)
    {
        if ((access & (GL_MAP_READ_BIT|GL_MAP_WRITE_BIT)) == GL_MAP_READ_BIT)
            bufObj->access =  GL_READ_ONLY;

        if ((access & (GL_MAP_READ_BIT|GL_MAP_WRITE_BIT)) == GL_MAP_WRITE_BIT)
            bufObj->access =  GL_WRITE_ONLY;

        if ((access & (GL_MAP_READ_BIT|GL_MAP_WRITE_BIT)) == (GL_MAP_READ_BIT|GL_MAP_WRITE_BIT))
            bufObj->access = GL_READ_WRITE;
    }
OnError:
    __GL_FOOTER();
    return result;
}

GLvoid* GL_APIENTRY __glim_MapBuffer(__GLcontext *gc, GLenum target, GLenum access)
{
    GLuint targetIndex;
    __GLbufferObject *bufObj;
    GLbitfield accessFlag = (GLbitfield)access;
    void *result = gcvNULL;

    __GL_HEADER();

    __GL_GET_BUFFER_TARGET_INDEX(target, targetIndex);

    switch( access )
    {
    case GL_WRITE_ONLY:
        accessFlag = GL_MAP_WRITE_BIT;
        break;
#ifdef OPENGL40
    case GL_READ_ONLY:
        accessFlag = GL_MAP_READ_BIT;
        break;
    case GL_READ_WRITE:
        accessFlag = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
        break;
#else
    case GL_MAP_BUFFER_OBJ_VIV:
        break;
#endif
    default:
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    bufObj = __glGetBoundBufObj(gc, targetIndex);
    if (!bufObj)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (bufObj->bufferMapped)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);
    result = (*gc->dp.mapBufferRange)(gc, bufObj, targetIndex, 0, bufObj->size, accessFlag);
    if (gcvNULL == result)
    {
        __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
    }
    bufObj->access = access;
    bufObj->accessFlags = accessFlag;

OnError:
    __GL_FOOTER();
    return result;
}

GLvoid GL_APIENTRY __glim_FlushMappedBufferRange(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr length)
{
    GLuint targetIndex;
    __GLbufferObject *bufObj;

    __GL_HEADER();

    __GL_GET_BUFFER_TARGET_INDEX(target, targetIndex);

    bufObj = __glGetBoundBufObj(gc, targetIndex);
    if (!bufObj)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (!bufObj->bufferMapped || !(bufObj->accessFlags & GL_MAP_FLUSH_EXPLICIT_BIT))
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    if ((offset < 0) || (length < 0) || ((offset + length) > bufObj->mapLength))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (!(*gc->dp.flushMappedBufferRange)(gc, bufObj, targetIndex, offset, length))
    {
        __GL_ERROR_EXIT((*gc->dp.getError)(gc));
    }
OnError:
    __GL_FOOTER();
    return;
}

GLboolean GL_APIENTRY __glim_UnmapBuffer(__GLcontext *gc, GLenum target)
{
    GLuint targetIndex;
    __GLbufferObject *bufObj;
    GLboolean retVal = GL_FALSE;

    __GL_HEADER();

    __GL_GET_BUFFER_TARGET_INDEX(target, targetIndex);

    bufObj = __glGetBoundBufObj(gc, targetIndex);
    if (!bufObj)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (bufObj->bufferMapped == GL_FALSE)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    /* Call the dp function to get the mapPointer */
    retVal = (*gc->dp.unmapBuffer)(gc, bufObj, targetIndex);

OnError:
    __GL_FOOTER();
    return retVal;
}

GLvoid GL_APIENTRY __glim_GetBufferParameteri64v(__GLcontext *gc, GLenum target, GLenum pname, GLint64* params)
{
    __glGetBufferParameteri64v(gc, target, pname, params);
}

GLvoid GL_APIENTRY __glim_GetBufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params)
{
    __GL_HEADER();
    if (params)
    {
        GLint64 param64 = 0;
        __glGetBufferParameteri64v(gc, target, pname, &param64);
        *params = (GLint)param64;
    }
    else
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }
OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __glim_GetBufferPointerv(__GLcontext *gc, GLenum target, GLenum pname, GLvoid **params)
{
    GLuint targetIndex;
    __GLbufferObject *bufObj;

    __GL_HEADER();

    __GL_GET_BUFFER_TARGET_INDEX(target, targetIndex);

    switch (pname)
    {
    case GL_BUFFER_MAP_POINTER:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    bufObj = __glGetBoundBufObj(gc, targetIndex);
    if (!bufObj)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    *params = bufObj->mapPointer;

OnError:
    __GL_FOOTER();
    return;
}

