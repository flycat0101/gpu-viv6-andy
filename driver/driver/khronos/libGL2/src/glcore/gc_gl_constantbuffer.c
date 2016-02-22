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


#include "gltypes.h"
#include "gc_gl_context.h"
#include "gc_gl_names_inline.c"


/********************************************************************
**
**  __glCreateConstantBuffer
**
**  Create an constant buffer structure.
**
**  Parameters:     size: constant buffer size, based on __GLcoord
**                        only valid for unbindable constant\uniform
**                  bindable: if it's a bindable buffer
**
**  Return Value:   return GL_TRUE\GL_FALSE if func succeed\fail
**
********************************************************************/
__GLconstantBuffer* __glCreateConstantBuffer(__GLcontext* gc, GLuint size, GLboolean bindable)
{
    __GLconstantBuffer* pBuffer = NULL;
    __GLbufferObject* bufObj;

    GL_ASSERT(gc);
    pBuffer = (__GLconstantBuffer*)(*gc->imports.calloc)(gc, 1, sizeof(__GLconstantBuffer) );

    if(pBuffer)
    {
        if(bindable)
        {
            pBuffer->bindable = GL_TRUE;
            /* constBufObj will point to the bound vbo later. */
            pBuffer->constBufObj = NULL;
        }
        else
        {
            pBuffer->bindable = GL_FALSE;

            /* initialize constBufObj*/
            bufObj = (__GLbufferObject*)(*gc->imports.calloc)(gc, 1, sizeof(__GLbufferObject) );
            bufObj->bufferObjData = (__GLbufferObjData*)
                (*gc->imports.calloc)(gc, 1, sizeof(__GLbufferObjData) );

            if (!bufObj || !bufObj->bufferObjData)
            {
                __glDestroyConstantBuffer(gc, pBuffer);
                pBuffer = NULL;
            }

            /* parameter size is based on __GLcoord */
            bufObj->size = size * sizeof(__GLcoord);

            bufObj->systemMemCache = (*gc->imports.malloc)(gc, sizeof(__GLcoord)*size );
            bufObj->bufferObjData->bufferObjDataDirty = (*gc->imports.malloc)(gc, sizeof(GLboolean)*size );
            bufObj->bufferObjData->bufferObjUserList = NULL;
            bufObj->privateData = (*gc->dp.createConstantBuffer)(gc, sizeof(__GLcoord)*size);

            pBuffer->constBufObj = bufObj;
            if (bufObj->systemMemCache &&
                bufObj->bufferObjData->bufferObjDataDirty &&
                bufObj->privateData)
            {
                __GL_MEMZERO(bufObj->systemMemCache, sizeof(__GLcoord)*size);
                __GL_MEMZERO(bufObj->bufferObjData->bufferObjDataDirty, sizeof(GLboolean)*size);
            }
            else
            {
                __glDestroyConstantBuffer(gc, pBuffer);
                pBuffer = NULL;
            }
        }
    }

    return pBuffer;
}

GLvoid __glCleanupConstantBuffer(__GLcontext* gc, GLvoid *buf)
{
    __GLconstantBuffer* pBuffer = buf;

    if(pBuffer)
    {
        pBuffer->bindable = GL_TRUE;
        if (pBuffer->constBufObj)
        {
            /* reseting constBufObj need to upload constant */
            pBuffer->globalDirty = GL_TRUE;
        }

        pBuffer->constBufObj = NULL;
    }
}

/********************************************************************
**
**  __glSetBindableConstantBuffer
**
**  Bind\Unbind an uniform buffer object to a constant buffer
**
**  Parameters:     gc: current gc
**                  pBuffer: constant buffer need to be bound
**                  bufObj: vbo need to be bound
**
**  Return Value: return GL_TRUE\GL_FALSE if function success\fail
**
********************************************************************/

GLboolean __glSetBindableConstantBuffer(__GLcontext* gc,
                                        __GLconstantBuffer* pBuffer,
                                        __GLbufferObject* bufObj)
{
    GL_ASSERT( pBuffer || bufObj);
    if (pBuffer)
    {
        /* must be a bindable constant buffer */
        pBuffer->bindable = GL_TRUE;
        if (pBuffer->constBufObj && (pBuffer->constBufObj!= bufObj))
        {
            /* reseting constBufObj need to upload constant */
            pBuffer->globalDirty = GL_TRUE;

            /* if bound before, need to disconnect original buffer object */
            if (pBuffer->constBufObj->bufferObjData)
            {
                __glRemoveImageUser(gc, &(pBuffer->constBufObj->bufferObjData->bufferObjUserList), pBuffer);
            }
        }

        pBuffer->constBufObj = bufObj;

        if (bufObj)
        {
            if (!bufObj->bufferObjData)
            {
                /* initalize bufferObjData when first bound */
                GLuint dirtySize = (bufObj->size + sizeof(__GLcoord)-1)/sizeof(__GLcoord);
                bufObj->bufferObjData = (*gc->imports.malloc)(gc, sizeof(__GLbufferObjData) );
                if (!bufObj->bufferObjData)
                {
                    __glSetError(GL_OUT_OF_MEMORY);
                    return GL_FALSE;
                }
                bufObj->bufferObjData->bufferObjDataDirty = (*gc->imports.malloc)(gc, dirtySize );
                if (!bufObj->bufferObjData->bufferObjDataDirty)
                {
                    __glSetError(GL_OUT_OF_MEMORY);
                    return GL_FALSE;
                }
                __GL_MEMSET(bufObj->bufferObjData->bufferObjDataDirty, GL_TRUE, dirtySize);
            }

            /* bind constant buffer to buffer object */
            __glAddImageUser(gc, &(bufObj->bufferObjData->bufferObjUserList), pBuffer, __glCleanupConstantBuffer);
        }
    }
    else
    {
        GL_ASSERT( bufObj);
        /* disconnect all constant buffer bound */
        if (bufObj->bufferObjData)
        {
            __glFreeImageUserList(gc, &(bufObj->bufferObjData->bufferObjUserList));
        }
    }

    return GL_TRUE;
}

/********************************************************************
**
**  __glDestroyConstantBuffer
**
**  Destroy an constant buffer structure
**
**  Parameters:     gc: current gc
**                  pBuffer: constant buffer need to be destroyed
**
**  Return Value:   none
**
********************************************************************/
GLvoid __glDestroyConstantBuffer(__GLcontext* gc, __GLconstantBuffer* pBuffer)
{
    __GLbufferObject* bufObj;
    if(pBuffer)
    {
        bufObj = pBuffer->constBufObj;
        if(pBuffer->bindable)
        {
            /* bindalbe const buf just need to disconnect bound buffer object */
            if (bufObj && bufObj->bufferObjData)
            {
                __glRemoveImageUser(gc, &(bufObj->bufferObjData->bufferObjUserList), bufObj);
            }
        }
        else
        {
            if(bufObj->systemMemCache)
            {
                (*gc->imports.free)(gc, bufObj->systemMemCache);
            }

            if(bufObj->bufferObjData)
            {
                if(bufObj->bufferObjData->bufferObjDataDirty)
                {
                    (*gc->imports.free)(gc, bufObj->bufferObjData->bufferObjDataDirty);
                }
                (*gc->imports.free)(gc, bufObj->bufferObjData);
            }

            if(bufObj->privateData)
            {
                gc->dp.destroyConstantBuffer(gc, bufObj->privateData);
            }
        }

        (*gc->imports.free)(gc, bufObj);
        (*gc->imports.free)(gc, pBuffer);
    }
}



