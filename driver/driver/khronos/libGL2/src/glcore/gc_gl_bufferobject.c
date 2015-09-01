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


#include "gc_gl_context.h"
#include "gc_gl_names_inline.c"
#include "dri/viv_lock.h"
#include "gc_gl_debug.h"

GLvoid __glInitBufObjTemplate(__GLcontext *gc)
{
    __GLbufferObject * bufObj = &gc->bufferObject.bufObjTemplate;
    bufObj->bindCount           = 0;
    bufObj->access              = GL_READ_WRITE ;
    bufObj->bufferMapped        = GL_FALSE;
    bufObj->mapPointer          = NULL;
    bufObj->usage               = GL_STATIC_DRAW ;
    bufObj->size                = 0;
    bufObj->privateData         = NULL;
    bufObj->systemCacheUptodate = GL_FALSE;
    bufObj->bufferObjData       = NULL;
}


static GLvoid __glInitBufferObject(__GLcontext *gc, __GLbufferObject *bufObj, GLuint name, GLint targetIndex)
{
    __GL_MEMCOPY(bufObj, &gc->bufferObject.bufObjTemplate, sizeof(__GLbufferObject));
    bufObj->name = name;
}

/*
** Used to share buffer objects between two different contexts.
*/
GLvoid __glShareBufferObjects(__GLcontext *dst, __GLcontext *src)
{
    if (dst->bufferObject.shared)
    {
        __glFreeSharedObjectState(dst, dst->bufferObject.shared);
    }

    dst->bufferObject.shared = src->bufferObject.shared;
    dst->bufferObject.shared->refcount++;
}

GLvoid __glBindBuffer(__GLcontext *gc, GLuint targetIndex, GLuint buffer)
{
    __GLbufferObject *bufObj, *boundBufObj;

#if (defined(DEBUG) || (defined(_DEBUG)))
    if (buffer == gdbg_nameTarget)
        gdbg_nameTarget = gdbg_nameTarget;
#endif


    if(gc->bufferObject.boundBuffer[targetIndex] == buffer)
        return;

    if( buffer )
    {
        /* Retrieve the buffer object from the "gc->bufferObject.shared" structure.*/
        bufObj = (__GLbufferObject *)__glGetObject(gc, gc->bufferObject.shared, buffer);

        if( bufObj == NULL )
        {
            bufObj = (__GLbufferObject*)(*gc->imports.calloc)(gc, 1, sizeof(__GLbufferObject) );

            if(!bufObj)
            {
                __glSetError(GL_OUT_OF_MEMORY);
                return;
            }

            __glInitBufferObject(gc,  bufObj, buffer, targetIndex);

            /* Add this buffer object to the "gc->bufferObject.shared" structure.
            */
            __glAddObject(gc, gc->bufferObject.shared, buffer, bufObj);

            /* Mark the name "buffer" used in the buffer object nameArray.*/
            __glMarkNameUsed(gc, gc->bufferObject.shared, buffer);

        }
        else
        {
            GL_ASSERT(buffer == bufObj->name);
        }
    }
    else /*bind the buffer target to 0, which means conventional vertex array*/
    {
        bufObj = NULL;
    }

    /* Unbound the originally bound buffer object and bind to the new object */
    boundBufObj = gc->bufferObject.boundTarget[targetIndex];
    gc->bufferObject.boundBuffer[targetIndex] = buffer;
    gc->bufferObject.boundTarget[targetIndex] = bufObj;

    switch (targetIndex)
    {
    case __GL_ARRAY_BUFFER_INDEX:
        /* Set ARRAY_BUFFER binding point in vertex array client state */
        gc->clientState.vertexArray.arrayBufBinding = buffer;
        break;
    case __GL_ELEMENT_ARRAY_BUFFER_INDEX:
        /* Set ELEMENT_ARRAY_BUFFER binding point in vertex array client state */
        gc->clientState.vertexArray.elementBufBinding = buffer;
        break;
    case __GL_PIXEL_PACK_BUFFER_INDEX:
        /* Set PIXEL_PACK_BUFFER binding point in pixel store client state */
        gc->clientState.pixel.packBufBinding = buffer;
        break;
    case __GL_PIXEL_UNPACK_BUFFER_INDEX:
        /* Set PIXEL_UNPACK_BUFFER binding point in pixel store client state */
        gc->clientState.pixel.unpackBufBinding = buffer;
        break;
    case __GL_TEXTURE_BUFFER_EXT_INDEX:
        /* Set TEXTURE_BUFFER_EXT binding point in __GLtextureState{} */
        gc->state.texture.textureBufBinding = buffer;
        break;
    }

    /* Remove gc from boundBufObj->userList if there are no other target bound to boundBufObj */
    if (boundBufObj != NULL)
    {
        GL_ASSERT(boundBufObj->name != 0 );
        boundBufObj->bindCount--;

        if (boundBufObj->bindCount == 0 && (boundBufObj->flag & __GL_OBJECT_IS_DELETED))
        {
            __glDeleteObject(gc, gc->bufferObject.shared, boundBufObj->name);
        }
    }

    /* Add gc to bufObj->userList if bufObj is not default buffer object.
    */
    if (buffer)
    {
        bufObj->bindCount++;

        /* Call the dp interface */
        (*gc->dp.bindBuffer)(gc, bufObj, targetIndex);
    }
}

GLboolean __glDeleteBufferObject(__GLcontext *gc, GLvoid *obj)
{
    GLuint targetIndex, arrayIndex;
    __GLvertexArrayState *vertexArrayState = &gc->clientState.vertexArray;
    __GLbufferObject *bufObj = obj;

    /*
    ** If the buffer object that is being deleted is currently bound to gc,
    ** bind 0 to the target.
    */
    for( targetIndex = 0; targetIndex < __GL_MAX_VERTEX_BUFFER_BINDINGS; targetIndex++ )
    {
        if(gc->bufferObject.boundBuffer[targetIndex] == bufObj->name)
        {
            /*
            ** __GL_OBJECT_IS_DELETED is cleared because we do not want this object
            ** deleted in the following function, otherwise there will be recursion:
            ** __glDeleteObject-->__glBindObject-->glDeleteObject, and the object will
            ** be deleted twice.
            */
            bufObj->flag &= ~__GL_OBJECT_IS_DELETED;
            __glBindBuffer(gc, targetIndex, 0);
        }
    }

    /* Check all the binding points in vertexArrayState. */
    for( arrayIndex = 0; arrayIndex < __GL_TOTAL_VERTEX_ATTRIBUTES; arrayIndex++ )
    {
        if(vertexArrayState->currentArrays[arrayIndex].bufBinding == bufObj->name )
        {
            vertexArrayState->currentArrays[arrayIndex].bufBinding = 0;
            gc->bufferObject.boundArrays[arrayIndex] = NULL;
        }
    }

    /* Do not delete the buffer object if there are other contexts bound to it. */
    if (bufObj->bindCount != 0)
    {
        /* Set the flag to indicate the object is marked for delete */
        bufObj->flag |= __GL_OBJECT_IS_DELETED;
        return GL_FALSE;
    }

    /* The object is truly deleted here, so delete the object name from name list. */
    __glDeleteNamesFrList(gc, gc->bufferObject.shared, bufObj->name, 1);

    /* Free system memory cache */
#if __GL_ENABLE_VBO_SYSTEM_CACHE
    if(bufObj->systemMemCache)
    {
        (*gc->imports.free)(gc, bufObj->systemMemCache);
        bufObj->systemMemCache = NULL;
    }
#endif

    /* Notify Dp that this buffer object is deleted.
    */
    (*gc->dp.deleteBuffer)(gc, bufObj);

    /* Free all objects related data which are attached to this buffer object */
    if (bufObj->bufferObjData)
    {
        /*
        ** Release all objects bound to VBO  and
        ** Free constant buffer user list
        */
        __glFreeImageUserList(gc, &(bufObj->bufferObjData->bufferObjUserList));

        /* Free constant dirty flag */
        if (bufObj->bufferObjData->bufferObjDataDirty)
        {
            (*gc->imports.free)(gc, bufObj->bufferObjData->bufferObjDataDirty);
        }

        /* Free bufferObjData structure */
        (*gc->imports.free)(gc, bufObj->bufferObjData);
    }

    /* Delete the buffer object structure */
    (*gc->imports.free)(gc, bufObj);

    return GL_TRUE;
}

GLvoid __glInitBufferObjectState(__GLcontext *gc)
{
    if (gc->bufferObject.shared == NULL)
    {
        gc->bufferObject.shared = (__GLsharedObjectMachine *)
            (*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine) );

        /* Initialize a linear lookup table for vertex buffer object */
        gc->bufferObject.shared->maxLinearTableSize = __GL_MAX_BUFOBJ_LINEAR_TABLE_SIZE;
        gc->bufferObject.shared->linearTableSize = __GL_DEFAULT_BUFOBJ_LINEAR_TABLE_SIZE;
        gc->bufferObject.shared->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, gc->bufferObject.shared->linearTableSize * sizeof(GLvoid *) );

        gc->bufferObject.shared->hashSize = __GL_BUFOBJ_HASH_TABLE_SIZE;
        gc->bufferObject.shared->hashMask = __GL_BUFOBJ_HASH_TABLE_SIZE - 1;
        gc->bufferObject.shared->refcount = 1;
        gc->bufferObject.shared->deleteObject = __glDeleteBufferObject;
    }
}

GLvoid __glFreeBufferObjectState(__GLcontext *gc)
{
    /* Free shared buffer object table */
    __glFreeSharedObjectState(gc, gc->bufferObject.shared);
}

/* OpenGL buffer object APIs */

GLvoid APIENTRY __glim_BindBuffer(GLenum target, GLuint buffer)
{
    GLuint targetIndex;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_BindBuffer", DT_GLenum, target, DT_GLuint, buffer, DT_GLnull);
#endif


    switch( target )
    {
    case GL_ARRAY_BUFFER :
        targetIndex = __GL_ARRAY_BUFFER_INDEX;
        break;
    case GL_ELEMENT_ARRAY_BUFFER :
        targetIndex = __GL_ELEMENT_ARRAY_BUFFER_INDEX;
        break;
    case GL_PIXEL_PACK_BUFFER_ARB:
        targetIndex = __GL_PIXEL_PACK_BUFFER_INDEX;
        break;
    case GL_PIXEL_UNPACK_BUFFER_ARB:
        targetIndex = __GL_PIXEL_UNPACK_BUFFER_INDEX;
        break;
    case GL_UNIFORM_BUFFER_EXT:
        targetIndex = __GL_UNIFORM_BUFFER_INDEX;
        break;
    case GL_TEXTURE_BUFFER_EXT:
        targetIndex = __GL_TEXTURE_BUFFER_EXT_INDEX;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glBindBuffer(gc, targetIndex, buffer);
}

GLvoid APIENTRY __glim_DeleteBuffers( GLsizei n, const GLuint *buffers)
{
    GLint i;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DeleteBuffers", DT_GLsizei, n, DT_GLuint_ptr, buffers, DT_GLnull);
#endif

    if(n < 0 )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    LINUX_LOCK_FRAMEBUFFER(gc);

    for (i = 0; i < n; i++)
    {
        if (buffers[i])
        {
            __glDeleteObject(gc, gc->bufferObject.shared, buffers[i]);
        }
    }
    LINUX_UNLOCK_FRAMEBUFFER(gc);
}

GLvoid APIENTRY __glim_GenBuffers( GLsizei n, GLuint * buffers)
{
    GLint start, i;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GenBuffers", DT_GLsizei, n, DT_GLuint_ptr, buffers, DT_GLnull);
#endif

    if(n < 0 )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (NULL == buffers){
        return;
    }

    GL_ASSERT(NULL != gc->bufferObject.shared);

    start = __glGenerateNames(gc, gc->bufferObject.shared, n);
    for (i = 0; i < n; i++)
    {
        buffers[i] = start + i;
    }

    if (gc->bufferObject.shared->linearTable)
    {
        __glCheckLinearTableSize(gc, gc->bufferObject.shared, (start + n));
    }
}

GLboolean APIENTRY __glim_IsBuffer( GLuint buffer)
{
    __GL_SETUP_NOT_IN_BEGIN_RET(0);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_IsBuffer", DT_GLuint, buffer, DT_GLnull);
#endif

    return __glIsNameDefined(gc, gc->bufferObject.shared, buffer);
}

GLvoid APIENTRY __glim_BufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
    GLuint targetIndex;
    __GLbufferObject * bufObj;

    GLboolean bRelocated;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_BufferData", DT_GLenum, target, DT_GLsizei_ptr, size, DT_GLvoid_ptr, data, DT_GLenum_ptr, usage, DT_GLnull);
#endif

    switch( target )
    {
    case GL_ARRAY_BUFFER :
        targetIndex = __GL_ARRAY_BUFFER_INDEX;
        break;
    case GL_ELEMENT_ARRAY_BUFFER :
        targetIndex = __GL_ELEMENT_ARRAY_BUFFER_INDEX;
        break;
    case GL_PIXEL_PACK_BUFFER_ARB:
        targetIndex = __GL_PIXEL_PACK_BUFFER_INDEX;
        break;
    case GL_PIXEL_UNPACK_BUFFER_ARB:
        targetIndex = __GL_PIXEL_UNPACK_BUFFER_INDEX;
        break;
    case GL_UNIFORM_BUFFER_EXT:
        targetIndex = __GL_UNIFORM_BUFFER_INDEX;
        break;
    case GL_TEXTURE_BUFFER_EXT:
        targetIndex = __GL_TEXTURE_BUFFER_EXT_INDEX;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if(size < 0 )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( gc->bufferObject.boundBuffer[targetIndex] == 0 )
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    switch( usage )
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
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    bufObj = gc->bufferObject.boundTarget[targetIndex];
    GL_ASSERT(bufObj);

    bufObj->usage = usage;

    /*relocate the system cache*/
    if( (bRelocated = (size != bufObj->size)) )
    {
#if __GL_ENABLE_VBO_SYSTEM_CACHE
        if (bufObj->systemMemCache)
        {
            (*gc->imports.free)(gc, bufObj->systemMemCache);
            bufObj->size = 0;
            bufObj->systemMemCache = NULL;
        }
        if( size )
        {
            bufObj->systemMemCache = (*gc->imports.malloc)(gc, size);
            if(!bufObj->systemMemCache)
            {
                /*
                ** delete the device buffer data
                */
                GL_ASSERT(bufObj->size == 0);
                (*gc->dp.bufferData)(gc, bufObj, targetIndex, data);
                bufObj->bufInDeviceMemory = GL_FALSE;
                __glSetError(GL_OUT_OF_MEMORY);
                return;
            }

            bufObj->size = size;
        }
#else
        bufObj->size = size;
#endif
    }

    /* Update system memory cache */
#if __GL_ENABLE_VBO_SYSTEM_CACHE
    if(data && size)
    {
        GL_ASSERT(bufObj->systemMemCache);
        __GL_MEMCOPY(bufObj->systemMemCache, data, size);
        bufObj->systemCacheUptodate = GL_TRUE;
    }
#endif

    LINUX_LOCK_FRAMEBUFFER(gc);

    if(GL_FALSE==(*gc->dp.bufferData)(gc, bufObj, targetIndex, data))
    {
        bufObj->bufInDeviceMemory = GL_FALSE;
        __glSetError(GL_OUT_OF_MEMORY);
    }
    else
    {
        bufObj->bufInDeviceMemory = GL_TRUE;
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);

    switch(targetIndex)
    {
        case GL_UNIFORM_BUFFER_EXT:
            if (bufObj->bufferObjData)
            {
                GLuint dirtySize = (bufObj->size + sizeof(__GLcoord)-1)/sizeof(__GLcoord);
                GLboolean dirty = data ? GL_TRUE : GL_FALSE;
                __GLimageUser* pUser = bufObj->bufferObjData->bufferObjUserList;
                __GLconstantBuffer* pBuffer;

                if (bRelocated)
                {
                    (*gc->imports.free)(gc, bufObj->bufferObjData->bufferObjDataDirty);
                    bufObj->bufferObjData->bufferObjDataDirty = (*gc->imports.malloc)(gc, dirtySize );
                    if (!bufObj->bufferObjData->bufferObjDataDirty)
                    {
                        __glSetError(GL_OUT_OF_MEMORY);
                        return;
                    }
                }

                __GL_MEMSET(bufObj->bufferObjData->bufferObjDataDirty, GL_FALSE, dirtySize);

                while(pUser)
                {
                    pBuffer = (__GLconstantBuffer*) pUser->imageUser;
                    pBuffer->globalDirty = dirty;
                    pUser = pUser->next;
                }
            }

            /* Set dirty to sync constant buffer */
            __GL_SET_ATTR_DIRTY_BIT(gc,__GL_PROGRAM_ATTRS,__GL_DIRTY_GLSL_UNIFORM);
            break;

        case GL_TEXTURE_BUFFER_EXT:
            if(bufObj->bufferObjData)
            {
                if(bRelocated)
                {
                    /* Add code to notify attached objects */
                }
            }
            break;
    }
}

GLvoid APIENTRY __glim_BufferSubData( GLenum target, GLintptr offset , GLsizeiptr size, const GLvoid *data)
{
    GLuint targetIndex;
    __GLbufferObject * bufObj;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_BufferSubData", DT_GLenum, target, DT_GLint_ptr, offset , DT_GLsizei_ptr, size, DT_GLvoid_ptr, data, DT_GLnull);
#endif

    switch( target )
    {
    case GL_ARRAY_BUFFER :
        targetIndex = __GL_ARRAY_BUFFER_INDEX;
        break;
    case GL_ELEMENT_ARRAY_BUFFER :
        targetIndex = __GL_ELEMENT_ARRAY_BUFFER_INDEX;
        break;
    case GL_PIXEL_PACK_BUFFER_ARB:
        targetIndex = __GL_PIXEL_PACK_BUFFER_INDEX;
        break;
    case GL_PIXEL_UNPACK_BUFFER_ARB:
        targetIndex = __GL_PIXEL_UNPACK_BUFFER_INDEX;
        break;
    case GL_UNIFORM_BUFFER_EXT:
        targetIndex = __GL_UNIFORM_BUFFER_INDEX;
        break;
    case GL_TEXTURE_BUFFER_EXT:
        targetIndex = __GL_TEXTURE_BUFFER_EXT_INDEX;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if( gc->bufferObject.boundBuffer[targetIndex] == 0 )
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    bufObj = gc->bufferObject.boundTarget[targetIndex];
    GL_ASSERT(bufObj);

    if ( offset < 0 || (size  + offset) > bufObj->size || size < 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( bufObj->bufferMapped )
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

#if __GL_ENABLE_VBO_SYSTEM_CACHE
    if(data && size && bufObj->systemMemCache)
    {
        /* Update system memory cache */
        __GL_MEMCOPY(bufObj->systemMemCache + offset, data, size);
#else
    if(data && size)
    {
#endif
        LINUX_LOCK_FRAMEBUFFER(gc);

        /* call the dp function to update the buffer object data in local memory*/
        if (!(*gc->dp.bufferSubData)(gc, bufObj, targetIndex, offset, size, data))
        {
            bufObj->bufInDeviceMemory = GL_FALSE;
        }
        else
        {
            bufObj->bufInDeviceMemory = GL_TRUE;
        }

        LINUX_UNLOCK_FRAMEBUFFER(gc);
    }

    if (targetIndex== GL_UNIFORM_BUFFER_EXT && bufObj->bufferObjData && data && size)
    {
        GLuint startIndex = offset / sizeof(__GLcoord);
        GLuint endIndex = (offset + size) / sizeof(__GLcoord);
        __GLimageUser* pUser = bufObj->bufferObjData->bufferObjUserList;
        __GLconstantBuffer* pBuffer;

        __GL_MEMSET(&(bufObj->bufferObjData->bufferObjDataDirty[startIndex]),
                    GL_FALSE,
                    (endIndex - startIndex + 1));

        while(pUser)
        {
            pBuffer = (__GLconstantBuffer*) pUser->imageUser;
            pBuffer->globalDirty = GL_TRUE;
            pUser = pUser->next;
        }

        /* Set dirty to sync constant buffer */
        __GL_SET_ATTR_DIRTY_BIT(gc,__GL_PROGRAM_ATTRS,__GL_DIRTY_GLSL_UNIFORM);
    }
}

GLvoid APIENTRY __glim_GetBufferSubData( GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data)
{
    GLuint targetIndex;
    __GLbufferObject * bufObj;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetBufferSubData", DT_GLenum, target, DT_GLint_ptr, offset, DT_GLsizei_ptr, size, DT_GLvoid_ptr, data, DT_GLnull);
#endif

    switch( target )
    {
    case GL_ARRAY_BUFFER :
        targetIndex = __GL_ARRAY_BUFFER_INDEX;
        break;
    case GL_ELEMENT_ARRAY_BUFFER :
        targetIndex = __GL_ELEMENT_ARRAY_BUFFER_INDEX;
        break;
    case GL_PIXEL_PACK_BUFFER_ARB:
        targetIndex = __GL_PIXEL_PACK_BUFFER_INDEX;
        break;
    case GL_PIXEL_UNPACK_BUFFER_ARB:
        targetIndex = __GL_PIXEL_UNPACK_BUFFER_INDEX;
        break;
    case GL_UNIFORM_BUFFER_EXT:
        targetIndex = __GL_UNIFORM_BUFFER_INDEX;
        break;
    case GL_TEXTURE_BUFFER_EXT:
        targetIndex = __GL_TEXTURE_BUFFER_EXT_INDEX;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if( gc->bufferObject.boundBuffer[targetIndex] == 0 )
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    bufObj = gc->bufferObject.boundTarget[targetIndex];
    GL_ASSERT(bufObj);

    if ( offset < 0 || (size  + offset) > bufObj->size || size < 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if( bufObj->bufferMapped )
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    /*call the dp function to initialize the buffer object data*/
    if(data && size )
        (*gc->dp.getBufferSubData)(gc, bufObj, offset, size, data);
}

GLvoid* APIENTRY __glim_MapBuffer(GLenum target, GLenum access)
{
    GLuint targetIndex;
    __GLbufferObject * bufObj;
    __GL_SETUP_NOT_IN_BEGIN_RET(0);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_MapBuffer", DT_GLenum, target, DT_GLenum, access, DT_GLnull);
#endif

    switch( target )
    {
    case GL_ARRAY_BUFFER :
        targetIndex = __GL_ARRAY_BUFFER_INDEX;
        break;
    case GL_ELEMENT_ARRAY_BUFFER :
        targetIndex = __GL_ELEMENT_ARRAY_BUFFER_INDEX;
        break;
    case GL_PIXEL_PACK_BUFFER_ARB:
        targetIndex = __GL_PIXEL_PACK_BUFFER_INDEX;
        break;
    case GL_PIXEL_UNPACK_BUFFER_ARB:
        targetIndex = __GL_PIXEL_UNPACK_BUFFER_INDEX;
        break;
    case GL_UNIFORM_BUFFER_EXT:
        targetIndex = __GL_UNIFORM_BUFFER_INDEX;
        break;
    case GL_TEXTURE_BUFFER_EXT:
        targetIndex = __GL_TEXTURE_BUFFER_EXT_INDEX;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return NULL;
    }

    if(gc->bufferObject.boundBuffer[targetIndex] == 0 )
    {
        __glSetError(GL_INVALID_OPERATION);
        return NULL;
    }

    bufObj = gc->bufferObject.boundTarget[targetIndex];
    GL_ASSERT(bufObj);

    if( bufObj->bufferMapped )
    {
        __glSetError(GL_INVALID_OPERATION);
        return NULL;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch( access )
    {
    case GL_READ_ONLY :
    case GL_WRITE_ONLY :
    case GL_READ_WRITE :
        bufObj->access = access;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return NULL;
    }

    /*call the dp function to get the mapPointer*/
    bufObj->mapPointer = (*gc->dp.mapBuffer)(gc, bufObj);
    bufObj->bufferMapped = GL_TRUE;

    return bufObj->mapPointer;
}

GLboolean APIENTRY __glim_UnmapBuffer(GLenum target)
{
    GLuint targetIndex;
    __GLbufferObject * bufObj;
    GLboolean retVal;
    __GL_SETUP_NOT_IN_BEGIN_RET(0);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_UnmapBuffer", DT_GLenum, target, DT_GLnull);
#endif

    switch( target )
    {
    case GL_ARRAY_BUFFER:
        targetIndex = __GL_ARRAY_BUFFER_INDEX;
        break;
    case GL_ELEMENT_ARRAY_BUFFER:
        targetIndex = __GL_ELEMENT_ARRAY_BUFFER_INDEX;
        break;
    case GL_PIXEL_PACK_BUFFER_ARB:
        targetIndex = __GL_PIXEL_PACK_BUFFER_INDEX;
        break;
    case GL_PIXEL_UNPACK_BUFFER_ARB:
        targetIndex = __GL_PIXEL_UNPACK_BUFFER_INDEX;
        break;
    case GL_UNIFORM_BUFFER_EXT:
        targetIndex = __GL_UNIFORM_BUFFER_INDEX;
        break;
    case GL_TEXTURE_BUFFER_EXT:
        targetIndex = __GL_TEXTURE_BUFFER_EXT_INDEX;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return GL_FALSE;
    }

    if(gc->bufferObject.boundBuffer[targetIndex] == 0 )
    {
        __glSetError(GL_INVALID_OPERATION);
        return GL_FALSE;
    }

    bufObj = gc->bufferObject.boundTarget[targetIndex];
    GL_ASSERT(bufObj);

    if( bufObj->bufferMapped == GL_FALSE)
    {
        __glSetError(GL_INVALID_OPERATION);
        return GL_FALSE;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    LINUX_LOCK_FRAMEBUFFER(gc);

    /*call the dp function to get the mapPointer*/
    retVal = (*gc->dp.unmapBuffer)(gc, bufObj);

    LINUX_UNLOCK_FRAMEBUFFER(gc);

    bufObj->bufferMapped = GL_FALSE;
    bufObj->mapPointer = NULL;

    if (retVal)
        bufObj->bufInDeviceMemory = GL_TRUE;
    else
        bufObj->bufInDeviceMemory = GL_FALSE;

    if (targetIndex== GL_UNIFORM_BUFFER_EXT && bufObj->bufferObjData)
    {
        GLuint dirtySize = (bufObj->size + sizeof(__GLcoord)-1)/sizeof(__GLcoord);
        __GLimageUser* pUser = bufObj->bufferObjData->bufferObjUserList;
        __GLconstantBuffer* pBuffer;

        __GL_MEMSET(bufObj->bufferObjData->bufferObjDataDirty, GL_FALSE, dirtySize);

        while(pUser)
        {
            pBuffer = (__GLconstantBuffer*) pUser->imageUser;
            pBuffer->globalDirty = GL_TRUE;
            pUser = pUser->next;
        }

        /* Set dirty to sync constant buffer */
        __GL_SET_ATTR_DIRTY_BIT(gc,__GL_PROGRAM_ATTRS,__GL_DIRTY_GLSL_UNIFORM);
    }

    return retVal;
}

GLvoid APIENTRY __glim_GetBufferParameteriv( GLenum target, GLenum pname, GLint *params)
{
    GLuint targetIndex;
    __GLbufferObject * bufObj;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetBufferParameteriv", DT_GLenum, target, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    switch( target )
    {
    case GL_ARRAY_BUFFER:
        targetIndex = __GL_ARRAY_BUFFER_INDEX;
        break;
    case GL_ELEMENT_ARRAY_BUFFER:
        targetIndex = __GL_ELEMENT_ARRAY_BUFFER_INDEX;
        break;
    case GL_PIXEL_PACK_BUFFER_ARB:
        targetIndex = __GL_PIXEL_PACK_BUFFER_INDEX;
        break;
    case GL_PIXEL_UNPACK_BUFFER_ARB:
        targetIndex = __GL_PIXEL_UNPACK_BUFFER_INDEX;
        break;
    case GL_UNIFORM_BUFFER_EXT:
        targetIndex = __GL_UNIFORM_BUFFER_INDEX;
        break;
    case GL_TEXTURE_BUFFER_EXT:
        targetIndex = __GL_TEXTURE_BUFFER_EXT_INDEX;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if( gc->bufferObject.boundBuffer[targetIndex] == 0 )
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    bufObj = gc->bufferObject.boundTarget[targetIndex];
    GL_ASSERT(bufObj);

    switch( pname )
    {
    case GL_BUFFER_SIZE :
        *params = (GLint)bufObj->size;
        return;
    case GL_BUFFER_USAGE :
        *params = bufObj->usage;
        return;
    case GL_BUFFER_ACCESS :
        *params = bufObj->access;
        return;
    case GL_BUFFER_MAPPED :
        *params = bufObj->bufferMapped;
        return;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_GetBufferPointerv( GLenum target, GLenum pname, GLvoid* *params)
{
    GLuint targetIndex;
    __GLbufferObject * bufObj;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetBufferPointerv", DT_GLenum, target, DT_GLenum, pname, DT_GLvoid_ptr, params, DT_GLnull);
#endif

    switch( target )
    {
    case GL_ARRAY_BUFFER:
        targetIndex = __GL_ARRAY_BUFFER_INDEX;
        break;
    case GL_ELEMENT_ARRAY_BUFFER:
        targetIndex = __GL_ELEMENT_ARRAY_BUFFER_INDEX;
        break;
    case GL_PIXEL_PACK_BUFFER_ARB:
        targetIndex = __GL_PIXEL_PACK_BUFFER_INDEX;
        break;
    case GL_PIXEL_UNPACK_BUFFER_ARB:
        targetIndex = __GL_PIXEL_UNPACK_BUFFER_INDEX;
        break;
    case GL_UNIFORM_BUFFER_EXT:
        targetIndex = __GL_UNIFORM_BUFFER_INDEX;
        break;
    case GL_TEXTURE_BUFFER_EXT:
        targetIndex = __GL_TEXTURE_BUFFER_EXT_INDEX;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch( pname )
    {
    case GL_BUFFER_MAP_POINTER:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if( gc->bufferObject.boundBuffer[targetIndex] == 0 )
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    bufObj= gc->bufferObject.boundTarget[targetIndex];
    GL_ASSERT(bufObj);

    *params = bufObj->mapPointer;
}
