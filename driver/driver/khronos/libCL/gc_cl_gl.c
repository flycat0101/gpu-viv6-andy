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


#include "gc_cl_precomp.h"

#define __NEXT_MSG_ID__     011056

#define GL_GLEXT_PROTOTYPES

#if gcvVERSION_MAJOR >= 5
/* GLES3 */
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES2/gl2ext.h>
#else
/* GLES2 */
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/

gctINT
clfExecuteCommandAcquireGLObjects(
    clsCommand_PTR  Command
    )
{
    clsCommandAcquireGLObjects_PTR  acquireGLObjects;
    gctUINT                         i;
    gctINT                          status = gcvSTATUS_OK;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_ACQUIRE_GL_OBJECTS, CL_INVALID_VALUE);

    acquireGLObjects = &Command->u.acquireGLObjects;

    /* TODO - reference GL objects. */

    for (i = 0; i < acquireGLObjects->numObjects; i++)
    {
        clRetainMemObject(acquireGLObjects->memObjects[i]);
    }

OnError:
    gcmFOOTER_ARG("%d", status);

    /* Return the status. */
    return status;
}

gctINT
clfExecuteCommandReleaseGLObjects(
    clsCommand_PTR  Command
    )
{
    clsCommandReleaseGLObjects_PTR  releaseGLObjects;
    gctUINT                         i;
    gctINT                          status = gcvSTATUS_OK;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_RELEASE_GL_OBJECTS, CL_INVALID_VALUE);

    releaseGLObjects = &Command->u.releaseGLObjects;

    /* TODO - upload image if the texture/renderbuffer is not read-only. */

    /* TODO - dereference GL objects. */

    for (i = 0; i < releaseGLObjects->numObjects; i++)
    {
        clReleaseMemObject(releaseGLObjects->memObjects[i]);
    }

OnError:
    gcmFOOTER_ARG("%d", status);

    /* Return the status. */
    return status;
}

size_t
clfGetGLTypeSize(GLenum type)
{
    switch( type )
    {
    case GL_FLOAT:
        return sizeof(GLfloat);

    case GL_INT:
        return sizeof(GLint);

    case GL_UNSIGNED_INT:
        return sizeof(GLuint);

    case GL_SHORT:
        return sizeof(GLshort);

    case GL_UNSIGNED_SHORT:
        return sizeof(GLushort);

    case GL_BYTE:
        return sizeof(GLbyte);

    case GL_UNSIGNED_BYTE:
        return sizeof(GLubyte);

    case GL_HALF_FLOAT:
        return sizeof(GLhalf);

    default:
        return sizeof(GLfloat);
    };
}
/*****************************************************************************\
|*                      OCL11 extension API                                  *|
\*****************************************************************************/
CL_API_ENTRY cl_event CL_API_CALL clCreateEventFromGLsyncKHR(cl_context context,
    cl_GLsync sync,
    cl_int * errcode_ret)
{
    /* note: empty now just to pass build */
    return gcvNULL;
}


/*****************************************************************************\
|*                      OpenCL GL_Sharing Extension API                      *|
\*****************************************************************************/
CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromGLBuffer(
    cl_context      Context,
    cl_mem_flags    Flags,
    cl_GLuint       BufObj,
    int *           ErrcodeRet
    )
{
    clsMem_PTR      buffer     = gcvNULL;
    GLint           bufferSize = 0;
    gctSIZE_T       size       = 0;
    gctPOINTER      data;
    gctINT          status;

    gcmHEADER_ARG("Context=0x%x Flags=0x%x BufObj=%u ",
                   Context, Flags, BufObj);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011000: (clCreateFromGLBuffer) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if ((Flags & CL_MEM_USE_HOST_PTR) &&
        (Flags & (CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR)))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011001: (clCreateFromGLBuffer) invalid Flags.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    glBindBuffer(GL_ARRAY_BUFFER, BufObj);

    /* TODO - Need to check if BufObj is valid GL buffer object. */

    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);

    if (bufferSize <= 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011002: (clCreateFromGLBuffer) BufObj has no data.\n");
        clmRETURN_ERROR(CL_INVALID_GL_OBJECT);
    }

    /* GLES3 */
    /*data = glMapBufferRange(GL_ARRAY_BUFFER, 0, bufferSize, GL_MAP_READ_BIT | GL_MAP_BUFFER_OBJ_VIV);*/
    /* GLES2 */
    data = glMapBufferOES(GL_ARRAY_BUFFER, GL_MAP_BUFFER_OBJ_VIV);

    if (data == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011003: (clCreateFromGLBuffer) BufObj is not mapped.\n");
        clmRETURN_ERROR(CL_INVALID_GL_OBJECT);
    }

    /* New buffer object. */
    clmONERROR(clfNewBuffer(Context, &buffer),
               CL_OUT_OF_HOST_MEMORY);

    /* Share with GL buffer. */
#if gcvVERSION_MAJOR >= 5
    /* GLES3 */
    clmONERROR(gcoCL_ShareMemoryWithBufObj((gcoBUFOBJ) data,
                                           &size,
                                           &buffer->u.buffer.physical,
                                           &buffer->u.buffer.logical,
                                           &buffer->u.buffer.node),
               CL_MEM_OBJECT_ALLOCATION_FAILURE);
#else
    /* GLES2 */
    clmONERROR(gcoCL_ShareMemoryWithStream((gcoSTREAM) data,
                                           &size,
                                           &buffer->u.buffer.physical,
                                           &buffer->u.buffer.logical,
                                           &buffer->u.buffer.node),
               CL_MEM_OBJECT_ALLOCATION_FAILURE);
#endif
    buffer->u.buffer.allocatedSize = (gctUINT) size;

    /* GLES3 */
    /*glUnmapBuffer( GL_ARRAY_BUFFER );*/
    /* GLES2 */
    glUnmapBufferOES(GL_ARRAY_BUFFER);

    buffer->fromGL    = gcvTRUE;
    buffer->glObj     = BufObj;
    buffer->glObjType = CL_GL_OBJECT_BUFFER;

    if (ErrcodeRet)
    {
        *ErrcodeRet = CL_SUCCESS;
    }
    gcmFOOTER_ARG("%d buffer=0x%x",
                  CL_SUCCESS, buffer);
    return buffer;

OnError:
    /* Return the status. */
    if (ErrcodeRet)
    {
        *ErrcodeRet = status;
    }

    if(buffer != gcvNULL)
    {
        clReleaseMemObject(buffer);
    }

    gcmFOOTER_ARG("%d", status);
    return gcvNULL;
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromGLTexture2D(
    cl_context      Context,
    cl_mem_flags    Flags,
    cl_GLenum       Target,
    cl_GLint        MipLevel,
    cl_GLuint       Texture,
    cl_int *        ErrcodeRet
    )
{
    gctINT          status;
    GLint           realWidth  = 0;
    GLint           realHeight = 0;
    GLint           realInternalFormat;
    /*GLenum          readBackFormat;*/
    GLenum          readBackType;
    size_t          outBytes   = 0;
    cl_char *       outBuffer  = gcvNULL;
    gctPOINTER      pointer    = gcvNULL;

    gcmHEADER_ARG("Context=0x%x Flags=%u Target=%u MipLevel=%u Texture=%u",
                   Context, Flags, Target, MipLevel, Texture);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011004: (clCreateFromGLTexture2D) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (Flags & ~(CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011005: (clCreateFromGLTexture2D) invalid Flags (%u).\n", Flags);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Check Target type and bind texture. */
    switch(Target)
    {
    case GL_TEXTURE_2D:
        glBindTexture(GL_TEXTURE_2D, Texture);
        break;

    case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        glBindTexture(GL_TEXTURE_CUBE_MAP, Texture);
        break;

    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011006: (clCreateFromGLTexture2D) invalid Target (%u).\n", Target);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Check mipmap level */
    if (MipLevel < 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011006: (clCreateFromGLTexture2D) invalid MipLevel (%d).\n", MipLevel);
        clmRETURN_ERROR(CL_INVALID_MIP_LEVEL);
    }
    else
    {
        GLint maxLevel = 0;

        glGetTexParameteriv(Target, GL_TEXTURE_MAX_LEVEL, &maxLevel );
        if (MipLevel > maxLevel)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-011007: (clCreateFromGLTexture2D) invalid MipLevel (%d).  MaxLevel is %d.\n",
                MipLevel, maxLevel);
            clmRETURN_ERROR(CL_INVALID_MIP_LEVEL);
        }
    }

    /* Check Texture */

#define GL_TEXTURE_WIDTH            0x1000
#define GL_TEXTURE_HEIGHT           0x1001
#define GL_TEXTURE_INTERNAL_FORMAT  0x1003
    glGetTexParameteriv(Target, GL_TEXTURE_WIDTH, &realWidth );
    glGetTexParameteriv(Target, GL_TEXTURE_HEIGHT, &realHeight );
    glGetTexParameteriv(Target, GL_TEXTURE_INTERNAL_FORMAT, &realInternalFormat );

    /*readBackFormat = GL_RGBA;*/
    readBackType = GL_UNSIGNED_BYTE;
    outBytes = realWidth * realHeight * 4 * clfGetGLTypeSize(readBackType);

    status = gcoOS_Allocate(gcvNULL, outBytes, &pointer);
    if (gcmIS_ERROR(status))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011007: (clCreateFromGLTexture2D) Cannot allocate memory.\n");
        clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }
    outBuffer = (cl_char *) pointer;

#define GL_TEXTURE_IMAGE  0x1001002
    glGetTexParameterfv(Target, GL_TEXTURE_IMAGE, (GLfloat *) &outBuffer );

    /* TODO - Need to implement image3D when hal is ready. */
    gcmUSER_DEBUG_ERROR_MSG(
        "OCL-011004: (clCreateFromGLTexture2D) internal error.\n");
    status = CL_INVALID_OPERATION;

OnError:
    if (ErrcodeRet) {
        *ErrcodeRet = status;
    }

    gcmFOOTER_ARG("%d", status);
    return gcvNULL;
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromGLTexture3D(
    cl_context      Context,
    cl_mem_flags    Flags,
    cl_GLenum       Target,
    cl_GLint        MipLevel,
    cl_GLuint       Texture,
    cl_int *        ErrcodeRet
    )
{
    gctINT          status;

    gcmHEADER_ARG("Context=0x%x Flags=%u Target=%u MipLevel=%u Texture=%u",
                   Context, Flags, Target, MipLevel, Texture);

    if (Context->devices[0]->deviceInfo.image3DMaxDepth == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011010: (clCreateFromGLTexture3D) image3D is not supported.\n");
        clmRETURN_ERROR(CL_INVALID_OPERATION);
    }

    /* TODO - Need to implement image3D when hal is ready. */
    gcmUSER_DEBUG_ERROR_MSG(
        "OCL-011011: (clCreateFromGLTexture3D) internal error.\n");
    status = CL_INVALID_OPERATION;

OnError:
    if (ErrcodeRet) {
        *ErrcodeRet = status;
    }

    gcmFOOTER_ARG("%d", status);
    return gcvNULL;
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromGLRenderbuffer(
    cl_context      Context,
    cl_mem_flags    Flags,
    cl_GLuint       Renderbuffer,
    cl_int *        ErrcodeRet
    )
{
    gctINT          status;
    GLint           outWidth            = 0;
    GLint           outHeight           = 0;
    GLint           internalFormat      = 0;
    GLenum          readBackFormat;
    GLenum          readBackType        = 0;
    size_t          readBackTypeSize    = 0;
    size_t          outBytes            = 0;
    cl_char *       outBuffer           = gcvNULL;
    gctPOINTER      pointer             = gcvNULL;
    clsMem_PTR      image               = gcvNULL;
    cl_image_format imageFormat;
    clsImageHeader_PTR      imageHeader = gcvNULL;

    gcmHEADER_ARG("Context=0x%x Flags=%u Renderbuffer=%u",
                   Context, Flags, Renderbuffer);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011020: (clCreateFromGLTexture2D) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (Flags & ~(CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011021: (clCreateFromGLTexture2D) invalid Flags (%u).\n", Flags);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    glGetRenderbufferParameteriv(Renderbuffer, GL_RENDERBUFFER_WIDTH, &outWidth );
    glGetRenderbufferParameteriv(Renderbuffer, GL_RENDERBUFFER_HEIGHT, &outHeight );
    glGetRenderbufferParameteriv(Renderbuffer, GL_RENDERBUFFER_INTERNAL_FORMAT, &internalFormat );

    readBackFormat = GL_RGBA;
    readBackType = GL_UNSIGNED_BYTE;
    readBackTypeSize = clfGetGLTypeSize(readBackType);
    outBytes = outWidth * outHeight * 4 * readBackTypeSize;

    if ((Flags & CL_MEM_WRITE_ONLY) == 0)
    {
        /* TODO - If read-only, share the surface/surf_node and use texld for read_imagef. */

        status = gcoOS_Allocate(gcvNULL, outBytes, &pointer);
        if (gcmIS_ERROR(status))
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-011021: (clCreateFromGLTexture2D) Cannot allocate memory.\n");
            clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
        }
        outBuffer = (cl_char *) pointer;

        /* TODO - The data conversion should be deferred to clEnqueueAcquireGLObjects. */

        /* TODO - Use resolve to speed up the conversion. */
        glReadPixels(0, 0, (GLsizei)outWidth, (GLsizei)outHeight, readBackFormat, readBackType, outBuffer);
    }

    imageFormat.image_channel_order = CL_RGBA;
    imageFormat.image_channel_data_type = CL_UNORM_INT8;

    /* New image object. */
    status = clfNewImage(Context, &image);
    if (gcmIS_ERROR(status))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011022: (clCreateFromGLTexture2D) Cannot allocate memory.\n");
        clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }

    /* Mem info. */
    image->host                 = outBuffer;
    image->flags                = Flags ? Flags : CL_MEM_READ_WRITE /* default */;

    image->fromGL               = gcvTRUE;
    image->glObj                = Renderbuffer;
    image->glObjType            = CL_GL_OBJECT_RENDERBUFFER;

    /* Image specific info. */
    image->u.image.format           = imageFormat;
    image->u.image.width            = outWidth;
    image->u.image.height           = outHeight;
    image->u.image.depth            = 1;
    image->u.image.rowPitch         = outWidth * readBackTypeSize;
    image->u.image.slicePitch       = 0;
    image->u.image.elementSize      = readBackTypeSize;
    image->u.image.size             = outBytes;
    image->u.image.texture          = gcvNULL;
    image->u.image.node             = gcvNULL;
    image->u.image.internalFormat   = internalFormat;
    image->u.image.texturePhysical  = 0;
    image->u.image.textureLogical   = 0;
    image->u.image.surfaceMapped    = gcvFALSE;
    image->u.image.tiling           = gcvSUPERTILED;

    /* Allocate physical buffer for image header. */
    image->u.image.allocatedSize = sizeof(clsImageHeader);
    clmONERROR(gcoCL_AllocateMemory(&image->u.image.allocatedSize,
                                    &image->u.image.physical,
                                    &image->u.image.logical,
                                    &image->u.image.node),
               CL_MEM_OBJECT_ALLOCATION_FAILURE);

    imageHeader = (clsImageHeader_PTR) image->u.image.logical;

    clmONERROR(gcoCL_CreateTexture(gcvFALSE,
                                   outWidth,
                                   outHeight,
                                   0,
                                   outBuffer,
                                   image->u.image.rowPitch,
                                   0,
                                   internalFormat,
                                   gcvENDIAN_NO_SWAP,
                                   &image->u.image.texture,
                                   &image->u.image.surface,
                                   &image->u.image.texturePhysical,
                                   &image->u.image.textureLogical,
                                   &image->u.image.textureStride,
                                   &image->u.image.textureSlicePitch),
               CL_MEM_OBJECT_ALLOCATION_FAILURE);

    gcoCL_FlushSurface(image->u.image.surface);

    imageHeader->width              = outWidth;
    imageHeader->height             = outHeight;
    imageHeader->depth              = 0;
    imageHeader->rowPitch           = image->u.image.textureStride;
    imageHeader->channelDataType    = imageFormat.image_channel_data_type;
    imageHeader->channelOrder       = imageFormat.image_channel_order;
    imageHeader->samplerValue       = -1;
    imageHeader->tiling             = image->u.image.tiling;
    imageHeader->slicePitch         = 0;
    imageHeader->physical           = gcmPTR2INT32(image->u.image.texturePhysical);

    if (ErrcodeRet)
    {
        *ErrcodeRet = CL_SUCCESS;
    }

    gcmFOOTER_ARG("%d image=0x%x",
                  CL_SUCCESS, image);
    return image;

OnError:
    if (outBuffer){
        gcoOS_Free(gcvNULL,outBuffer);
    }
    if (image->u.image.logical){
        gcoCL_FreeMemory(image->u.image.physical,
                        image->u.image.logical,
                        image->u.image.allocatedSize,
                        image->u.image.node);
    }
    if(image->u.image.texture && image->u.image.surface){
        gcoCL_DestroyTexture(
        image->u.image.texture,
        image->u.image.surface);
    }

    gcmOS_SAFE_FREE(gcvNULL, image);

    if (ErrcodeRet) {
        *ErrcodeRet = status;
    }

    gcmFOOTER_ARG("%d", status);
    return gcvNULL;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetGLObjectInfo(
    cl_mem                MemObj,
    cl_gl_object_type *   GLObjectType,
    cl_GLuint *           GLObjectName
    )
{
    gctINT           status;

    gcmHEADER_ARG("MemObj=0x%x GLObjectType=%u GLObjectName=0x%x",
                  MemObj, GLObjectType, GLObjectName);

    if (MemObj == gcvNULL || MemObj->objectType != clvOBJECT_MEM)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011030: (clGetGLObjectInfo) invalid MemObj.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (! MemObj->fromGL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011031: (clGetGLObjectInfo) MemObj not associated with GL object.\n");
        clmRETURN_ERROR(CL_INVALID_GL_OBJECT);
    }

    if (GLObjectType) {
        *GLObjectType = MemObj->glObjType;
    }

    if (GLObjectName) {
        *GLObjectName = MemObj->glObj;
    }

    gcmFOOTER_ARG("%d *GLObjectType=%lu *GLObjectName=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(GLObjectType), gcmOPT_VALUE(GLObjectName));
    return CL_SUCCESS;

OnError:
    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetGLTextureInfo(
    cl_mem               MemObj,
    cl_gl_texture_info   ParamName,
    size_t               ParamValueSize,
    void *               ParamValue,
    size_t *             ParamValueSizeRet
    )
{
    gctSIZE_T        retParamSize = 0;
    gctPOINTER       retParamPtr = NULL;
    gctINT           status;

    gcmHEADER_ARG("MemObj=0x%x ParamName=%u ParamValueSize=%lu ParamValue=0x%x",
                  MemObj, ParamName, ParamValueSize, ParamValue);

    if (MemObj == gcvNULL || MemObj->objectType != clvOBJECT_MEM)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011032: (clGetGLTextureInfo) invalid MemObj.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (! MemObj->fromGL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011033: (clGetGLTextureInfo) MemObj not associated with GL object.\n");
        clmRETURN_ERROR(CL_INVALID_GL_OBJECT);
    }

    if (MemObj->type != CL_MEM_OBJECT_IMAGE2D && MemObj->type != CL_MEM_OBJECT_IMAGE3D)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011034: (clGetGLTextureInfo) MemObj not Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    switch (ParamName) {
    case CL_GL_TEXTURE_TARGET:
        retParamSize = gcmSIZEOF(MemObj->u.image.textureTarget);
        retParamPtr = &(MemObj->u.image.textureTarget);
        break;

    case CL_GL_MIPMAP_LEVEL:
        retParamSize = gcmSIZEOF(MemObj->u.image.mipLevel);
        retParamPtr = &(MemObj->u.image.mipLevel);
        break;

    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011035: (clGetGLTextureInfo) invalid ParamName (0x%x).\n",
            ParamName);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (ParamValue) {
        if (ParamValueSize < retParamSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-004036: (clGetGLTextureInfo) ParamValueSize (%d) is less than required size (%d).\n",
                ParamValueSize, retParamSize);
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (retParamSize) {
            gcoOS_MemCopy(ParamValue, retParamPtr, retParamSize);
        }
    }

    if (ParamValueSizeRet) {
        *ParamValueSizeRet = retParamSize;
    }

    gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(ParamValueSizeRet));
    return CL_SUCCESS;

OnError:
    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueAcquireGLObjects(
    cl_command_queue      CommandQueue,
    cl_uint               NumObjects,
    const cl_mem *        MemObjects,
    cl_uint               NumEventsInWaitList,
    const cl_event *      EventWaitList,
    cl_event *            Event
    )
{
    clsCommand_PTR                  command = gcvNULL;
    clsCommandAcquireGLObjects_PTR  acquireGLObjects;
    gctUINT                         i;
    gctPOINTER                      pointer = gcvNULL;
    gctINT                          status;

    gcmHEADER_ARG("CommandQueue=0x%x NumObjects=%d MemObjects=0x%x "
                  "NumEventsInWaitList=%u EventWaitList=0x%x Event=0x%x",
                  CommandQueue, NumObjects, MemObjects, NumEventsInWaitList, EventWaitList, Event);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011037: (clEnqueueAcquireGLObjects) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (NumObjects == 0 && MemObjects != gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011038: (clEnqueueAcquireGLObjects) NumObjects is zero, but MemObject is not NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (NumObjects != 0 && MemObjects == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011039: (clEnqueueAcquireGLObjects) NumObjects is not zero, but MemObject is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    for (i = 0; i < NumObjects; i++)
    {
        if (MemObjects[i] == NULL || MemObjects[i]->objectType != clvOBJECT_MEM)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-011040: (clEnqueueAcquireGLObjects) invalid MemObjects[i].\n", i);
            clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
        }

        if (CommandQueue->context != MemObjects[i]->context)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-011041: (clEnqueueAcquireGLObjects) MemObjects[i]'s context is not the same as CommandQueue's context.\n", i);
            clmRETURN_ERROR(CL_INVALID_CONTEXT);
        }

        if (! MemObjects[i]->fromGL)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-011042: (clEnqueueAcquireGLObjects) MemObjects[i] is not created from OpenGL object.\n", i);
            clmRETURN_ERROR(CL_INVALID_CONTEXT);
        }
    }

    if (CommandQueue->context == NULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011043: (clEnqueueAcquireGLObjects) CommandQueue's context is not from OpenGL context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011044: (clEnqueueAcquireGLObjects) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i=0; i<NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-011045: (clEnqueueAcquireGLObjects) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_ACQUIRE_GL_OBJECTS;
    command->handler                = &clfExecuteCommandAcquireGLObjects;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    acquireGLObjects                = &command->u.acquireGLObjects;
    acquireGLObjects->numObjects    = NumObjects;
    acquireGLObjects->memObjects    = MemObjects;

    for (i = 0; i < NumObjects; i++)
    {
        clRetainMemObject(MemObjects[i]);
    }

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011046: (clEnqueueAcquireGLObjects) Run out of memory.\n");
    }

    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReleaseGLObjects(
    cl_command_queue      CommandQueue,
    cl_uint               NumObjects,
    const cl_mem *        MemObjects,
    cl_uint               NumEventsInWaitList,
    const cl_event *      EventWaitList,
    cl_event *            Event
    )
{
    clsCommand_PTR                  command = gcvNULL;
    clsCommandReleaseGLObjects_PTR  releaseGLObjects;
    gctUINT                         i;
    gctPOINTER                      pointer = gcvNULL;
    gctINT                          status;

    gcmHEADER_ARG("CommandQueue=0x%x NumObjects=%d MemObjects=0x%x "
                  "NumEventsInWaitList=%u EventWaitList=0x%x Event=0x%x",
                  CommandQueue, NumObjects, MemObjects, NumEventsInWaitList, EventWaitList, Event);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011047: (clEnqueueReleaseGLObjects) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (NumObjects == 0 && MemObjects != gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011048: (clEnqueueReleaseGLObjects) NumObjects is zero, but MemObject is not NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (NumObjects != 0 && MemObjects == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011049: (clEnqueueReleaseGLObjects) NumObjects is not zero, but MemObject is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    for (i = 0; i < NumObjects; i++)
    {
        if (MemObjects[i] == NULL || MemObjects[i]->objectType != clvOBJECT_MEM)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-011050: (clEnqueueReleaseGLObjects) invalid MemObjects[i].\n", i);
            clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
        }

        if (CommandQueue->context != MemObjects[i]->context)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-011051: (clEnqueueReleaseGLObjects) MemObjects[i]'s context is not the same as CommandQueue's context.\n", i);
            clmRETURN_ERROR(CL_INVALID_CONTEXT);
        }

        if (! MemObjects[i]->fromGL)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-011052: (clEnqueueAcquireGLObjects) MemObjects[i] is not created from OpenGL object.\n", i);
            clmRETURN_ERROR(CL_INVALID_CONTEXT);
        }
    }

    if (CommandQueue->context == NULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011053: (clEnqueueReleaseGLObjects) CommandQueue's context is not from OpenGL context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011054: (clEnqueueReleaseGLObjects) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i=0; i<NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-011008: (clEnqueueReleaseGLObjects) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_RELEASE_GL_OBJECTS;
    command->handler                = &clfExecuteCommandReleaseGLObjects;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    releaseGLObjects                = &command->u.releaseGLObjects;
    releaseGLObjects->numObjects    = NumObjects;
    releaseGLObjects->memObjects    = MemObjects;

    for (i = 0; i < NumObjects; i++)
    {
        clRetainMemObject(MemObjects[i]);
    }

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011055: (clEnqueueReleaseGLObjects) Run out of memory.\n");
    }

    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}
