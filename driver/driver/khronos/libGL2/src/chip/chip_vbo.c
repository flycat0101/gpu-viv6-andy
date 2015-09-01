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
#include "chip_context.h"
#include "../glcore/gc_gl_names_inline.c"

#define _GC_OBJ_ZONE    gcvZONE_API_GL

GLvoid reResidentVertexBuffer(glsCHIPCONTEXT_PTR chipCtx, glsVERTEXBUFFERINFO **bufInfo)
{
}

GLvoid destroyVertexBufferObject(glsCHIPCONTEXT_PTR chipCtx, glsVERTEXBUFFERINFO *bufInfo)
{
    gceSTATUS status = gcvSTATUS_OK;
    do
    {
        if (bufInfo->flags.indexBuffer)
        {
            gcmERR_BREAK(gcoINDEX_Destroy(bufInfo->bufObject));
            bufInfo->bufObject = gcvNULL;
        }

        if (bufInfo->flags.vertexBuffer)
        {
            gcmERR_BREAK(gcoSTREAM_Destroy(bufInfo->bufObject));
            bufInfo->bufObject = gcvNULL;
        }

        /* Implement later */
        if (bufInfo->flags.constantBuffer)
        {
        }
        if (bufInfo->flags.pixelBuffer)
        {
        }
        if (bufInfo->flags.outputBuffer)
        {
        }
        if (bufInfo->flags.textureBuffer)
        {
        }
    }
    while (gcvFALSE);
}

/********************************************************************
**
**  bindPBOToTexture
**
**  Maintain a list of the textures whose image should be from the buffer object.
**  To defer the copy from buffer object to texture, we save the texture name in
**  the list when teximage2D is called.
**
**  We will kick off the copy in two conditions:
**      1. texture validation
**      2. the data of this buffer object is changed.
**
**
**  Parameters:      Describe the calling sequence
**  Return Value:    Describe the returning sequence
**
********************************************************************/
GLboolean bindPBOToTexture(__GLcontext* gc, GLuint PBOName, GLuint objectName)
{
    glsVERTEXBUFFERINFO        *bufInfo;
    glsCHIPBUFFEROBJECTPENDINGLIST *entry;
    __GLbufferObject               *bufObj;

    bufObj = (__GLbufferObject *)__glGetObject(gc, gc->bufferObject.shared, PBOName);

    if (bufObj == NULL)
    {
        /* The buffer object is dead */
        return GL_FALSE;
    }

    bufInfo = (glsVERTEXBUFFERINFO *)bufObj->privateData;

    /* Create one new entry */
    entry = (glsCHIPBUFFEROBJECTPENDINGLIST *)gc->imports.calloc(NULL, 1, sizeof(glsCHIPBUFFEROBJECTPENDINGLIST));

    if (entry == NULL)
    {
        GL_ASSERT(0);
        return GL_FALSE;
    }

    entry->name = objectName;
    entry->type = __GL_PBO_PENDING_TYPE_TEXTURE;

    /* Insert to the list */
    /* Should we check whether this texture is in the list already? */
    entry->next = bufInfo->toCopyList;
    bufInfo->toCopyList = entry;

    return GL_TRUE;
}

/********************************************************************
**
**  unbindPBOToTexture
**
**  Remove one entry from the toCopyList.
**
**
**  Parameters:      Describe the calling sequence
**  Return Value:    Describe the returning sequence
**
********************************************************************/
GLvoid unbindPBOToTexture(__GLcontext* gc, GLuint PBOName, GLuint objectName)
{
    __GLbufferObject               *bufObj;
    glsCHIPBUFFEROBJECTPENDINGLIST *entry, *prev;
    glsVERTEXBUFFERINFO        *bufInfo;

    bufObj  = (__GLbufferObject *)__glGetObject(gc, gc->bufferObject.shared, PBOName);
    bufInfo = (glsVERTEXBUFFERINFO *)bufObj->privateData;

    if (bufObj == NULL)
    {
        /* The buffer object is dead */
        return;
    }

    /* Remove from the list */
    prev = entry = bufInfo->toCopyList;

    while (entry)
    {
        if (entry->name == objectName)
        {
            if (entry == bufInfo->toCopyList)
            {
                bufInfo->toCopyList = entry->next;
            }
            else
            {
                prev->next = entry->next;
            }

            /* Free the entry */
            gc->imports.free(gc, (GLvoid *)entry);
            return;
        }

        prev  = entry;
        entry = entry->next;
    }
}


/************************************************************************/
/* Implementation for DP API                                            */
/************************************************************************/
void __glChipBindBufferObject(__GLcontext* gc, __GLbufferObject* bufObj, GLuint targetIndex)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    glsVERTEXBUFFERINFO *bufInfo = NULL;
    GLushort prevBindFlags = 0;

    /*If this is the first time that the buffer object is bound*/
    if(bufObj->privateData == NULL)
    {
        bufInfo = (glsVERTEXBUFFERINFO*)gc->imports.calloc(NULL, 1, sizeof(glsVERTEXBUFFERINFO));
        GL_ASSERT(bufInfo);
        bufObj->privateData = bufInfo;
    }
    else
    {
        bufInfo = (glsVERTEXBUFFERINFO*)(bufObj->privateData);
    }


    prevBindFlags = bufInfo->flags.bindFlags;
    memset((void*)&(bufInfo->flags.bindFlags), 0, sizeof(bufInfo->flags.bindFlags));
    switch(targetIndex)
    {
    case __GL_ARRAY_BUFFER_INDEX:
        bufInfo->flags.vertexBuffer = 1;
        break;

    case __GL_ELEMENT_ARRAY_BUFFER_INDEX:
        bufInfo->flags.indexBuffer = 1;
        break;

    case __GL_PIXEL_PACK_BUFFER_INDEX:
    case __GL_PIXEL_UNPACK_BUFFER_INDEX:
        bufInfo->flags.pixelBuffer = 1;
        break;

    case __GL_UNIFORM_BUFFER_INDEX:
        bufInfo->flags.constantBuffer = 1;
        break;

    case __GL_TEXTURE_BUFFER_EXT_INDEX:
        bufInfo->flags.textureBuffer = 1;
        break;

    default :
        GL_ASSERT(0);
    }

    bufInfo->location = __GL_RESIDENT_IN_VIDEOMEMORY;
    if(prevBindFlags != bufInfo->flags.bindFlags)
    {
        /* If resource already exist and bind flag change, re-resident it */
        if(bufInfo->bufObject && bufInfo->size)
        {
            reResidentVertexBuffer(chipCtx, &bufInfo);
            bufObj->privateData = bufInfo;
        }
    }
}


GLvoid __glChipDeleteBufferObject(__GLcontext* gc, __GLbufferObject* bufObj)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    glsVERTEXBUFFERINFO* bufInfo = (glsVERTEXBUFFERINFO *)(bufObj->privateData);

    GL_ASSERT(bufInfo);

    destroyVertexBufferObject(chipCtx, bufInfo);
    gc->imports.free(gc, bufInfo);
    bufObj->privateData = NULL;
}

GLvoid*__glChiptMapBufferObject(__GLcontext* gc, __GLbufferObject* bufObj)
{
    glsVERTEXBUFFERINFO* bufInfo = (glsVERTEXBUFFERINFO*)(bufObj->privateData);

    /* set mapping status and return buffer client pointer. */
    gcoSTREAM_CPUCacheOperation(bufInfo->bufObject, gcvCACHE_INVALIDATE);

    gcoSTREAM_Lock(bufInfo->bufObject,
        &bufInfo->bufferMapPointer,
        gcvNULL);
    bufInfo->isMapped = gcvTRUE;
    return bufInfo->bufferMapPointer;
}


GLboolean __glChipUnMapBufferObject(__GLcontext* gc, __GLbufferObject* bufObj)
{
    glsVERTEXBUFFERINFO* bufInfo = (glsVERTEXBUFFERINFO*)(bufObj->privateData);

    GL_ASSERT(bufInfo);

    /* Check whether there is pending copy of pixel buffer object */
    if (bufInfo->toCopyList != NULL)
    {
        glsCHIPBUFFEROBJECTPENDINGLIST *entry, *next;

        entry = bufInfo->toCopyList;

        while (entry)
        {
            next = entry->next;

            /* Free the entry */
            gc->imports.free(gc, (GLvoid *)entry);

            entry = next;
        }

        bufInfo->toCopyList = NULL;
    }

    if (bufInfo->bufObject == NULL)
       return GL_TRUE;

    if (bufInfo->isMapped)
    {
        gcoSTREAM_CPUCacheOperation(bufInfo->bufObject, gcvCACHE_FLUSH);
        bufInfo->bufferMapPointer = gcvNULL;
        bufInfo->isMapped = gcvTRUE;
    }

    return gcvTRUE;;
}


GLboolean __glChipBufferData(__GLcontext* gc, __GLbufferObject* bufObj, GLuint targetIndex, const void* data)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    glsVERTEXBUFFERINFO* bufInfo = (glsVERTEXBUFFERINFO*)(bufObj->privateData);
    gceSTATUS status = gcvSTATUS_OK;

    GL_ASSERT(bufInfo);

    /* Check whether there is pending copy of pixel buffer object */
    if (bufInfo->toCopyList != NULL)
    {
        glsCHIPBUFFEROBJECTPENDINGLIST *entry, *next;

        entry = bufInfo->toCopyList;

        while (entry)
        {

            next = entry->next;

            /* Free the entry */
            gc->imports.free(gc, (GLvoid *)entry);

            entry = next;
        }

        bufInfo->toCopyList = NULL;
    }

    do
    {
        /*
        **If memory has been allocated before, but the original size
        **is not enough. So destroy the old memory, and allocate
        **again.

        ** If (bufInfo->size != 0 && bufObj->size == 0 ), just destroy the original buffer object.
        */

        bufInfo->size = bufObj->size;
        if(bufObj->usage == GL_STATIC_DRAW)
        {
            bufInfo->flags.dynamic  = 0;
            bufInfo->location = __GL_RESIDENT_IN_VIDEOMEMORY;
        }
        else
        {
            bufInfo->flags.dynamic  = 1;
            bufInfo->location = __GL_RESIDENT_IN_NONLOCALVIDMEM;
        }

        if(bufInfo->bufObject)
        {
            destroyVertexBufferObject(chipCtx, bufInfo);
        }

        if(bufInfo->size != 0)
        {
            switch (targetIndex)
            {
                case __GL_ARRAY_BUFFER_INDEX:
                    /* Allocate the stream object. */
                    gcmERR_BREAK(gcoSTREAM_Construct(
                        chipCtx->hal,
                        (gcoSTREAM *)&bufInfo->bufObject
                        ));

                    /* Allocate the buffer. */
                    gcmERR_BREAK(gcoSTREAM_Reserve(
                        bufInfo->bufObject,
                        bufInfo->size
                        ));

                    /* Is the data provided? */
                    if (data != gcvNULL)
                    {
                        gcmERR_BREAK(gcoSTREAM_Upload(
                            bufInfo->bufObject,
                            data,
                            0,
                            bufInfo->size,
                            bufInfo->flags.dynamic
                            ));
                    }
                    break;

                case __GL_ELEMENT_ARRAY_BUFFER_INDEX:
                    /* Allocate the index object. */
                    gcmERR_BREAK(gcoINDEX_Construct(
                        chipCtx->hal,
                        (gcoINDEX *)&bufInfo->bufObject
                        ));

                    /* Allocate the buffer. */
                    gcmERR_BREAK(gcoINDEX_Upload(
                        bufInfo->bufObject,
                        gcvNULL,
                        bufInfo->size
                    ));

                    /* Is the data provided? */
                    if (data != gcvNULL)
                    {
                        gcmERR_BREAK(gcoINDEX_Upload(
                            bufInfo->bufObject,
                            data,
                            bufInfo->size
                            ));
                    }
                    break;

                case __GL_PIXEL_PACK_BUFFER_INDEX:
                case __GL_PIXEL_UNPACK_BUFFER_INDEX:
                    break;

                case __GL_UNIFORM_BUFFER_INDEX:
                    break;

                case __GL_TEXTURE_BUFFER_EXT_INDEX:
                    break;
            }
        }
    }
    while (gcvFALSE);

    return gcvTRUE;
}


GLboolean __glChipBufferSubData(__GLcontext* gc, __GLbufferObject* bufObj, GLuint targetIndex, GLintptr offset, GLsizeiptr size, const void* data)
{
    glsVERTEXBUFFERINFO* bufInfo = (glsVERTEXBUFFERINFO*)(bufObj->privateData);

    if (!bufInfo || !bufInfo->bufObject) {
        return gcvFALSE;
    }

    /* Upload user data. */
    switch( targetIndex )
    {
        case __GL_ARRAY_BUFFER_INDEX :
            /* Upload data. */
            gcoSTREAM_Upload(
                bufInfo->bufObject,
                data,
                offset,
                size,
                bufInfo->flags.dynamic
                );
            break;
        case __GL_ELEMENT_ARRAY_BUFFER_INDEX :
            gcoINDEX_UploadOffset(
                bufInfo->bufObject,
                offset,
                data,
                size
                );
            break;
        case __GL_PIXEL_PACK_BUFFER_INDEX:
           break;
        case __GL_PIXEL_UNPACK_BUFFER_INDEX:
            break;
        case __GL_UNIFORM_BUFFER_INDEX:
            break;
        case __GL_TEXTURE_BUFFER_EXT_INDEX:
            break;
    }
    return gcvTRUE;
}


GLvoid __glChipGetBufferSubData(__GLcontext* gc, __GLbufferObject* bufObj,
                                        GLintptr offset, GLsizeiptr size, void* data)
{
    glsVERTEXBUFFERINFO* bufInfo = (glsVERTEXBUFFERINFO*)(bufObj->privateData);

    /* set mapping status and return buffer client pointer. */
    gcoSTREAM_CPUCacheOperation(bufInfo->bufObject, gcvCACHE_INVALIDATE);

    gcoSTREAM_Lock(bufInfo->bufObject,
        &bufInfo->bufferMapPointer,
        gcvNULL);

    __GL_MEMCOPY(data, (GLbyte *)bufInfo->bufferMapPointer+offset, size);
}



