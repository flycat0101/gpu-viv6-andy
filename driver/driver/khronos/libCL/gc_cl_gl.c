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

#define __NEXT_MSG_ID__     011076

#if (defined(UNDER_CE) || defined(__QNXNTO__) || defined(LINUX))
#define MAP_TO_DEVICE 1
#else
#define MAP_TO_DEVICE 0
#endif

#define GL_GLEXT_PROTOTYPES

#if gcvVERSION_MAJOR >= 5
/* GLES3 */
#include <GLES3/gl32.h>
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
        gcmASSERT(-1);/*unsupport data type */
        return sizeof(GLfloat);
    };
}

size_t
clfGetCLTypeSize(GLenum type)
{
    switch( type )
    {
    case CL_FLOAT:
        return sizeof(cl_float);
    case CL_UNSIGNED_INT32:
    case CL_SIGNED_INT32:
        return sizeof(cl_int);
    case CL_UNSIGNED_INT16:
    case CL_SIGNED_INT16:
        return sizeof(cl_short);
    case CL_UNSIGNED_INT8:
    case CL_SIGNED_INT8:
        return sizeof(cl_char);
    case CL_HALF_FLOAT:
        return sizeof(cl_half);
    default:
        gcmASSERT(-1);/*unsupport data type */
        return sizeof(GLfloat);
    };
}

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

    for (i = 0; i < acquireGLObjects->numObjects; i++)
    {
        switch (acquireGLObjects->memObjects[i]->glObjType)
        {
        case CL_GL_OBJECT_BUFFER:
            {
                /* do nothing */
            }
            break;
        case CL_GL_OBJECT_TEXTURE2D:
            {
                clfCLGLShareCreateImageData(Command->commandQueue->context, acquireGLObjects->memObjects[i], acquireGLObjects->objectsDatas[i]);
            }
            break;
        case CL_GL_OBJECT_TEXTURE3D:
            {
                clfCLGLShareCreateImageData(Command->commandQueue->context, acquireGLObjects->memObjects[i], acquireGLObjects->objectsDatas[i]);
            }
            break;
        case CL_GL_OBJECT_RENDERBUFFER:
            {
                clfCLGLShareCreateImageData(Command->commandQueue->context, acquireGLObjects->memObjects[i], acquireGLObjects->objectsDatas[i]);
            }
            break;
        case CL_GL_OBJECT_TEXTURE2D_ARRAY:
            break;
        case CL_GL_OBJECT_TEXTURE1D:
            break;
        case CL_GL_OBJECT_TEXTURE1D_ARRAY:
            break;
        case CL_GL_OBJECT_TEXTURE_BUFFER:
            break;
        default:
            break;
        }
    }

    for (i = 0; i < acquireGLObjects->numObjects; i++)
    {
        clRetainMemObject(acquireGLObjects->memObjects[i]);
    }

    gcoOS_Free(gcvNULL, acquireGLObjects->w);
    gcoOS_Free(gcvNULL, acquireGLObjects->h);
    gcoOS_Free(gcvNULL, acquireGLObjects->d);
    gcoOS_Free(gcvNULL, acquireGLObjects->imFormats);
    gcoOS_Free(gcvNULL, acquireGLObjects->objectsDatas);

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

    for (i = 0; i < releaseGLObjects->numObjects; i++)
    {
        switch (releaseGLObjects->memObjects[i]->glObjType)
        {
        case CL_GL_OBJECT_BUFFER:
            break;
        case CL_GL_OBJECT_TEXTURE2D:
            {
                size_t origin[3] = { 0, 0, 0/*releaseGLObjects->x[i], releaseGLObjects->y[i], releaseGLObjects->z[i]*/};
                size_t region[3] = {releaseGLObjects->w[i], releaseGLObjects->h[i], releaseGLObjects->d[i]};
                clsCommand      command;
                clsCommandReadImage_PTR readImage;
                command.objectType             = clvOBJECT_COMMAND;
                command.type                   = clvCOMMAND_READ_IMAGE;
                command.handler                = gcvNULL;
                command.outEvent               = gcvNULL;
                command.numEventsInWaitList    = 0;
                command.eventWaitList          = gcvNULL;

                readImage                       = &command.u.readImage;
                readImage->image                = releaseGLObjects->memObjects[i];
                readImage->blockingRead         = CL_TRUE;
                readImage->origin[0]            = origin[0];
                readImage->origin[1]            = origin[1];
                readImage->origin[2]            = origin[2];
                readImage->region[0]            = region[0];
                readImage->region[1]            = region[1];
                readImage->region[2]            = region[2];
                readImage->rowPitch             = region[0]*releaseGLObjects->elementSize[i];
                readImage->slicePitch           = 0;
                readImage->ptr                  = releaseGLObjects->objectsDatas[i];
                clfExecuteCommandReadImage(&command);
            }
            break;
        case CL_GL_OBJECT_TEXTURE3D:
            {
                size_t origin[3] = { 0, 0, 0/*releaseGLObjects->x[i], releaseGLObjects->y[i], releaseGLObjects->z[i]*/};
                size_t region[3] = {releaseGLObjects->w[i], releaseGLObjects->h[i], releaseGLObjects->d[i]};
                clsCommand      command;
                clsCommandReadImage_PTR readImage;
                command.objectType             = clvOBJECT_COMMAND;
                command.type                   = clvCOMMAND_READ_IMAGE;
                command.handler                = gcvNULL;
                command.outEvent               = gcvNULL;
                command.numEventsInWaitList    = 0;
                command.eventWaitList          = gcvNULL;

                readImage                       = &command.u.readImage;
                readImage->image                = releaseGLObjects->memObjects[i];
                readImage->blockingRead         = CL_TRUE;
                readImage->origin[0]            = origin[0];
                readImage->origin[1]            = origin[1];
                readImage->origin[2]            = origin[2];
                readImage->region[0]            = region[0];
                readImage->region[1]            = region[1];
                readImage->region[2]            = region[2];
                readImage->rowPitch             = region[0]*releaseGLObjects->elementSize[i];
                readImage->slicePitch           = readImage->rowPitch*region[1];
                readImage->ptr                  = releaseGLObjects->objectsDatas[i];
                clfExecuteCommandReadImage(&command);
            }
            break;
        case CL_GL_OBJECT_RENDERBUFFER:
            {
                size_t origin[3] = { 0, 0, 0/*releaseGLObjects->x[i], releaseGLObjects->y[i], releaseGLObjects->z[i]*/};
                size_t region[3] = {releaseGLObjects->w[i], releaseGLObjects->h[i], releaseGLObjects->d[i]};
                clsCommand      command;
                clsCommandReadImage_PTR readImage;
                command.objectType             = clvOBJECT_COMMAND;
                command.type                   = clvCOMMAND_READ_IMAGE;
                command.handler                = gcvNULL;
                command.outEvent               = gcvNULL;
                command.numEventsInWaitList    = 0;
                command.eventWaitList          = gcvNULL;

                readImage                       = &command.u.readImage;
                readImage->image                = releaseGLObjects->memObjects[i];
                readImage->blockingRead         = CL_TRUE;
                readImage->origin[0]            = origin[0];
                readImage->origin[1]            = origin[1];
                readImage->origin[2]            = origin[2];
                readImage->region[0]            = region[0];
                readImage->region[1]            = region[1];
                readImage->region[2]            = region[2];
                readImage->rowPitch             = region[0]*releaseGLObjects->elementSize[i];
                readImage->slicePitch           = 0;
                readImage->ptr                  = releaseGLObjects->objectsDatas[i];
                clfExecuteCommandReadImage(&command);
            }
            break;
        case CL_GL_OBJECT_TEXTURE2D_ARRAY:
            break;
        case CL_GL_OBJECT_TEXTURE1D:
            break;
        case CL_GL_OBJECT_TEXTURE1D_ARRAY:
            break;
        case CL_GL_OBJECT_TEXTURE_BUFFER:
            break;
        default:
            break;
        }
    }

    for (i = 0; i < releaseGLObjects->numObjects; i++)
    {
        clReleaseMemObject(releaseGLObjects->memObjects[i]);
    }

    gcoOS_Free(gcvNULL, releaseGLObjects->w);
    gcoOS_Free(gcvNULL, releaseGLObjects->h);
    gcoOS_Free(gcvNULL, releaseGLObjects->d);
    gcoOS_Free(gcvNULL, releaseGLObjects->imFormats);
    gcoOS_Free(gcvNULL, releaseGLObjects->elementSize);
    gcoOS_Free(gcvNULL, releaseGLObjects->objectsDatas);

OnError:
    gcmFOOTER_ARG("%d", status);

    /* Return the status. */
    return status;
}

static void clfQueryGLEnum2Enum(GLint internalFormat, GLint textureType, cl_channel_type* retCLdataType, cl_channel_order* retCLdataFormat, cl_int* retChannelCount,
                                  GLenum *retGLType, GLenum *retGLFormat, cl_mem_object_type* retImType)
{
    cl_channel_type dataType = 0;
    cl_channel_order dataFormat = 0;
    cl_mem_object_type imType = 0;
    cl_int channelCount = 0;
    GLenum glType = 0;
    GLenum glFormat = 0;
    /* Translate format information. */
    if ( internalFormat != 0)
    switch (internalFormat)
    {
    case GL_R8:
    case GL_R8UI:
        dataFormat = CL_R;
        dataType = CL_UNSIGNED_INT8;
        channelCount = 1;
        glFormat = GL_RED;
        glType = GL_UNSIGNED_BYTE;
        break;
    case GL_R8_SNORM:
    case GL_R8I:
        dataFormat = CL_R;
        dataType = CL_SIGNED_INT8;
        channelCount = 1;
        glFormat = GL_RED;
        glType = GL_BYTE;
        break;
    case GL_R16F:
        dataFormat = CL_R;
        dataType = CL_HALF_FLOAT;
        channelCount = 1;
        glFormat = GL_RED;
        glType = GL_SHORT;
        break;
    case GL_R32F:
        dataFormat = CL_R;
        dataType = CL_FLOAT;
        channelCount = 1;
        glFormat = GL_RED;
        glType = GL_FLOAT;
        break;
    case GL_R16UI:
        dataFormat = CL_R;
        dataType = CL_UNSIGNED_INT16;
        channelCount = 1;
        glFormat = GL_RED;
        glType = GL_UNSIGNED_SHORT;
        break;
    case GL_R16I:
        dataFormat = CL_R;
        dataType = CL_SIGNED_INT16;
        channelCount = 1;
        glFormat = GL_RED;
        glType = GL_SHORT;
        break;
    case GL_R32UI:
        dataFormat = CL_R;
        dataType = CL_UNSIGNED_INT32;
        channelCount = 1;
        glFormat = GL_RED;
        glType = GL_UNSIGNED_INT;
        break;
    case GL_R32I:
        dataFormat = CL_R;
        dataType = CL_SIGNED_INT32;
        channelCount = 1;
        glFormat = GL_RED;
        glType = GL_INT;
        break;
    case GL_RG8:
    case GL_RG8UI:
        dataFormat = CL_RG;
        dataType = CL_UNSIGNED_INT8;
        channelCount = 2;
        glFormat = GL_RG;
        glType = GL_UNSIGNED_BYTE;
        break;
    case GL_RG8_SNORM:
    case GL_RG8I:
        dataFormat = CL_RG;
        dataType = CL_SIGNED_INT8;
        channelCount = 2;
        glFormat = GL_RG;
        glType = GL_BYTE;
        break;
    case GL_RG16F:
        dataFormat = CL_RG;
        dataType = CL_HALF_FLOAT;
        channelCount = 2;
        glFormat = GL_RG;
        glType = GL_HALF_FLOAT;
        break;
    case GL_RG32F:
        dataFormat = CL_RG;
        dataType = CL_FLOAT;
        channelCount = 2;
        glFormat = GL_RG;
        glType = GL_FLOAT;
        break;
    case GL_RG16UI:
        dataFormat = CL_RG;
        dataType = CL_UNSIGNED_INT16;
        channelCount = 2;
        glFormat = GL_RG;
        glType = GL_UNSIGNED_SHORT;
        break;
    case GL_RG16I:
        dataFormat = CL_RG;
        dataType = CL_SIGNED_INT16;
        channelCount = 2;
        glFormat = GL_RG;
        glType = GL_SHORT;
        break;
    case GL_RG32UI:
        dataFormat = CL_RG;
        dataType = CL_UNSIGNED_INT32;
        channelCount = 2;
        glFormat = GL_RG;
        glType = GL_UNSIGNED_INT;
        break;
    case GL_RG32I:
        dataFormat = CL_RG;
        dataType = CL_SIGNED_INT32;
        channelCount = 2;
        glFormat = GL_RG;
        glType = GL_INT;
        break;
    case GL_RGB8:
    case GL_RGB8UI:
        dataFormat = CL_RGB;
        dataType = CL_UNSIGNED_INT8;
        channelCount = 3;
        glFormat = GL_RGB;
        glType = GL_UNSIGNED_BYTE;
        break;
    case GL_SRGB8:
    case GL_RGB8I:
        dataFormat = CL_RGB;
        dataType = CL_SIGNED_INT8;
        channelCount = 3;
        glFormat = GL_RGB;
        glType = GL_BYTE;
        break;
    case GL_RGB565:
        dataFormat = CL_RGBx;
        dataType = CL_UNORM_SHORT_565;
        channelCount = 1;
        glFormat = GL_RGB;
        glType = GL_UNSIGNED_SHORT_5_6_5;
        break;
    case GL_RGB8_SNORM:
        dataFormat = CL_RGB;
        dataType = CL_SIGNED_INT8;
        channelCount = 3;
        glFormat = GL_RGB;
        glType = GL_BYTE;
        break;
    case GL_RGB16F:
        dataFormat = CL_RGB;
        dataType = CL_HALF_FLOAT;
        channelCount = 3;
        glFormat = GL_RGB;
        glType = GL_HALF_FLOAT;
        break;
    case GL_RGB32F:
        dataFormat = CL_RGB;
        dataType = CL_FLOAT;
        channelCount = 3;
        glFormat = GL_RGB;
        glType = GL_FLOAT;
        break;
    case GL_RGB16UI:
        dataFormat = CL_RGB;
        dataType = CL_UNSIGNED_INT16;
        channelCount = 3;
        glFormat = GL_RGB;
        glType = GL_UNSIGNED_SHORT;
        break;
    case GL_RGB16I:
        dataFormat = CL_RGB;
        dataType = CL_SIGNED_INT16;
        glFormat = GL_RGB;
        glType = GL_SHORT;
        channelCount = 3;
        break;
    case GL_RGB32UI:
        dataFormat = CL_RGB;
        dataType = CL_UNSIGNED_INT32;
        channelCount = 3;
        break;
    case GL_RGB32I:
        dataFormat = CL_RGB;
        dataType = CL_SIGNED_INT32;
        channelCount = 3;
        glFormat = GL_RGB;
        glType = GL_INT;
        break;
    case GL_RGBA8:
    case GL_SRGB8_ALPHA8:
    case GL_RGBA8UI:
        dataFormat = CL_RGBA;
        dataType = CL_UNSIGNED_INT8;
        channelCount = 4;
        glFormat = GL_RGBA;
        glType = GL_UNSIGNED_BYTE;
        break;
    case GL_RGBA8_SNORM:
    case GL_RGBA8I:
        dataFormat = CL_RGBA;
        dataType = CL_SIGNED_INT8;
        channelCount = 4;
        glFormat = GL_RGBA;
        glType = GL_BYTE;
        break;
    case GL_RGBA16F:
        dataFormat = CL_RGBA;
        dataType = CL_HALF_FLOAT;
        channelCount = 4;
        glFormat = GL_RGBA;
        glType = GL_HALF_FLOAT;
        break;
    case GL_RGBA32F:
        dataFormat = CL_RGBA;
        dataType = CL_FLOAT;
        channelCount = 4;
        glFormat = GL_RGBA;
        glType = GL_FLOAT;
        break;
    case GL_RGBA16UI:
        dataFormat = CL_RGBA;
        dataType = CL_UNSIGNED_INT16;
        channelCount = 4;
        glFormat = GL_RGBA;
        glType = GL_UNSIGNED_SHORT;
        break;
    case GL_RGBA16I:
        dataFormat = CL_RGBA;
        dataType = CL_SIGNED_INT16;
        channelCount = 4;
        glFormat = GL_RGBA;
        glType = GL_SHORT;
        break;
    case GL_RGBA32UI:
        dataFormat = CL_RGBA;
        dataType = CL_UNSIGNED_INT32;
        channelCount = 4;
        glFormat = GL_RGBA;
        glType = GL_UNSIGNED_INT;
        break;
    case GL_RGBA32I:
        dataFormat = CL_RGBA;
        dataType = CL_SIGNED_INT32;
        channelCount = 4;
        glFormat = GL_RGBA;
        glType = GL_INT;
        break;
    case GL_RGB10_A2UI:
    case GL_RGB10_A2:
    case GL_RGBA4:
    case GL_RGB5_A1:
    case GL_R11F_G11F_B10F:
    case GL_RGB9_E5:
    default:
        gcmASSERT(internalFormat); /*unsupport format in CL*/
        break;
    }
#define GL_TEXTURE_1D               0x0DE0
#define GL_TEXTURE_1D_ARRAY               0x8C18
    if ( textureType != 0 )
    switch (textureType)
    {
    case GL_TEXTURE_1D:
        imType = CL_MEM_OBJECT_IMAGE1D;
        break;
    case GL_TEXTURE_1D_ARRAY:
        imType = CL_MEM_OBJECT_IMAGE1D_ARRAY;
        break;
    case GL_TEXTURE_2D:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        imType = CL_MEM_OBJECT_IMAGE2D;
        break;
    case GL_TEXTURE_2D_ARRAY:
        imType = CL_MEM_OBJECT_IMAGE2D_ARRAY;
        break;
    case GL_TEXTURE_3D:
        imType = CL_MEM_OBJECT_IMAGE3D;
        break;
    default:
        gcmASSERT(textureType); /*unsupport textureType in CL*/
        break;
    }

    if(textureType != 0 && retImType != gcvNULL)
        *retImType = imType;
    if(internalFormat != 0)
    {
        if ( retCLdataType != gcvNULL)
            *retCLdataType = dataType;
        if ( retCLdataFormat != gcvNULL)
            *retCLdataFormat = dataFormat;
        if ( retChannelCount != gcvNULL)
            *retChannelCount = channelCount;
        if ( retGLType != gcvNULL)
            *retGLType = glType;
        if ( retGLFormat != gcvNULL)
            *retGLFormat = glFormat;
    }

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

    /* New buffer object. */
    clmONERROR(clfNewBuffer(Context, &buffer),
               CL_OUT_OF_HOST_MEMORY);
    buffer->flags = Flags;
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
clCreateFromGLTexture(
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
    GLint           realDepth = 1;
    GLint           textureFormat      = 0;
    cl_channel_order  realCLFormat;
    cl_channel_type   realCLType;
    clsMem_PTR      image = gcvNULL;
    cl_image_format imageFormat;
    cl_image_desc   imageDesc;
    cl_mem_object_type clImageType;
    cl_gl_object_type  glObjType;
    GLint          origTex = 0;

    gcmHEADER_ARG("Context=0x%x Flags=%u Target=%u MipLevel=%u Texture=%u",
                   Context, Flags, Target, MipLevel, Texture);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011057: (clCreateFromGLTexture) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (Flags & ~(CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011058: (clCreateFromGLTexture) invalid Flags (%u).\n", Flags);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Check Target type and bind texture. */
    switch(Target)
    {
    case GL_TEXTURE_2D:
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &origTex);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glObjType = CL_GL_OBJECT_TEXTURE2D;
        break;

    case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &origTex);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Texture);
        glObjType = CL_GL_OBJECT_TEXTURE2D;
        break;
    case GL_TEXTURE_3D:
        glGetIntegerv(GL_TEXTURE_BINDING_3D, &origTex);
        glBindTexture(GL_TEXTURE_3D, Texture);
        glObjType = CL_GL_OBJECT_TEXTURE3D;
        break;

    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011059: (clCreateFromGLTexture) invalid Target (%u).\n", Target);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Check mipmap level */
    if (MipLevel < 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011060: (clCreateFromGLTexture) invalid MipLevel (%d).\n", MipLevel);
        clmRETURN_ERROR(CL_INVALID_MIP_LEVEL);
    }
    else
    {
        GLint maxLevel = 0;
        if (  Target == GL_TEXTURE_CUBE_MAP_POSITIVE_X
            ||Target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y
            ||Target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z
            ||Target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X
            ||Target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
            ||Target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
        {
            glGetTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, &maxLevel );
        }
        else
        {
            glGetTexParameteriv(Target, GL_TEXTURE_MAX_LEVEL, &maxLevel );
        }
        if (MipLevel > maxLevel)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-011061: (clCreateFromGLTexture) invalid MipLevel (%d).  MaxLevel is %d.\n",
                MipLevel, maxLevel);
            clmRETURN_ERROR(CL_INVALID_MIP_LEVEL);
        }
    }

    /* Check Texture */
    glGetTexLevelParameteriv(Target, MipLevel, GL_TEXTURE_WIDTH, &realWidth );
    glGetTexLevelParameteriv(Target, MipLevel, GL_TEXTURE_HEIGHT, &realHeight );
    if(Target == GL_TEXTURE_3D)
    {
        glGetTexLevelParameteriv(Target, MipLevel, GL_TEXTURE_DEPTH, &realDepth );
    }
    glGetTexLevelParameteriv(Target, MipLevel, GL_TEXTURE_INTERNAL_FORMAT, &textureFormat );

    clfQueryGLEnum2Enum(textureFormat, Target, &realCLType, &realCLFormat, gcvNULL,gcvNULL, gcvNULL, &clImageType);


    imageFormat.image_channel_order = realCLFormat;
    imageFormat.image_channel_data_type = realCLType;

    imageDesc.image_width = realWidth;
    imageDesc.image_height = realHeight;
    imageDesc.image_depth = realDepth;
    imageDesc.image_array_size = 0;
    imageDesc.buffer = NULL;
    imageDesc.num_mip_levels = MipLevel;
    imageDesc.num_samples = 0;
    imageDesc.image_type = clImageType;
    imageDesc.image_row_pitch = 0;
    imageDesc.image_slice_pitch = 0;

    image = clfCLGLShareCreateImageWrapper(Context, Flags, &imageFormat, &imageDesc, gcvNULL, ErrcodeRet);

    image->fromGL               = gcvTRUE;
    image->glObj                = Texture;
    image->glObjType            = glObjType;
    image->glTexTarget          = Target;

    if (  Target == GL_TEXTURE_CUBE_MAP_POSITIVE_X
        ||Target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y
        ||Target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z
        ||Target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X
        ||Target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
        ||Target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, origTex);
    }
    else
    {
        glBindTexture(Target, origTex);
    }
    gcmFOOTER_ARG("%d image=0x%x",
                  CL_SUCCESS, image);
    return image;

OnError:
    if (ErrcodeRet) {
        *ErrcodeRet = status;
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
    GLint           textureFormat      = 0;
    cl_channel_order  realCLFormat;
    cl_channel_type   realCLType;
    clsMem_PTR      image = gcvNULL;
    cl_image_format imageFormat;
    cl_image_desc   imageDesc;
    cl_gl_object_type  glObjType;
    GLint          origTex = 0;

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
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &origTex);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glObjType = CL_GL_OBJECT_TEXTURE2D;
        break;

    case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &origTex);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Texture);
        glObjType = CL_GL_OBJECT_TEXTURE2D;
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
        if (  Target == GL_TEXTURE_CUBE_MAP_POSITIVE_X
            ||Target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y
            ||Target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z
            ||Target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X
            ||Target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
            ||Target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
        {
            glGetTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, &maxLevel );
        }
        else
        {
            glGetTexParameteriv(Target, GL_TEXTURE_MAX_LEVEL, &maxLevel );
        }
        if (MipLevel > maxLevel)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-011007: (clCreateFromGLTexture2D) invalid MipLevel (%d).  MaxLevel is %d.\n",
                MipLevel, maxLevel);
            clmRETURN_ERROR(CL_INVALID_MIP_LEVEL);
        }
    }

    /* Check Texture */
    glGetTexLevelParameteriv(Target, MipLevel,GL_TEXTURE_WIDTH, &realWidth );
    glGetTexLevelParameteriv(Target, MipLevel, GL_TEXTURE_HEIGHT, &realHeight );
    glGetTexLevelParameteriv(Target, MipLevel, GL_TEXTURE_INTERNAL_FORMAT, &textureFormat );

    clfQueryGLEnum2Enum(textureFormat, 0, &realCLType, &realCLFormat, gcvNULL, gcvNULL, gcvNULL,gcvNULL);

    imageFormat.image_channel_order = realCLFormat;
    imageFormat.image_channel_data_type = realCLType;

    imageDesc.image_width = realWidth;
    imageDesc.image_height = realHeight;
    imageDesc.image_depth = 1;
    imageDesc.image_array_size = 0;
    imageDesc.buffer = NULL;
    imageDesc.num_mip_levels = MipLevel;
    imageDesc.num_samples = 0;
    imageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    imageDesc.image_row_pitch = 0;
    imageDesc.image_slice_pitch = 0;

    image = clfCLGLShareCreateImageWrapper(Context, Flags, &imageFormat, &imageDesc, gcvNULL, ErrcodeRet);

    image->fromGL               = gcvTRUE;
    image->glObj                = Texture;
    image->glObjType            = glObjType;
    image->glTexTarget          = Target;
    if (  Target == GL_TEXTURE_CUBE_MAP_POSITIVE_X
        ||Target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y
        ||Target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z
        ||Target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X
        ||Target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
        ||Target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, origTex);
    }
    else
    {
        glBindTexture(Target, origTex);
    }

    gcmFOOTER_ARG("%d image=0x%x",
                  CL_SUCCESS, image);
    return image;

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
    GLint           realWidth  = 0;
    GLint           realHeight = 0;
    GLint           realDepth = 0;
    GLint           textureFormat      = 0;
    cl_channel_order  realCLFormat;
    cl_channel_type   realCLType;
    clsMem_PTR      image = gcvNULL;
    cl_image_format imageFormat;
    cl_image_desc   imageDesc;
    GLint          origTex = 0;

    gcmHEADER_ARG("Context=0x%x Flags=%u Target=%u MipLevel=%u Texture=%u",
                   Context, Flags, Target, MipLevel, Texture);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011062: (clCreateFromGLTexture3D) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (Flags & ~(CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011063: (clCreateFromGLTexture3D) invalid Flags (%u).\n", Flags);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Check Target type and bind texture. */
    gcmASSERT(Target == GL_TEXTURE_3D);
    glGetIntegerv(GL_TEXTURE_BINDING_3D, &origTex);
    glBindTexture(Target, Texture);

    /* Check mipmap level */
    if (MipLevel < 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011064: (clCreateFromGLTexture3D) invalid MipLevel (%d).\n", MipLevel);
        clmRETURN_ERROR(CL_INVALID_MIP_LEVEL);
    }
    else
    {
        GLint maxLevel = 0;

        glGetTexParameteriv(Target, GL_TEXTURE_MAX_LEVEL, &maxLevel );
        if (MipLevel > maxLevel)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-011065: (clCreateFromGLTexture3D) invalid MipLevel (%d).  MaxLevel is %d.\n",
                MipLevel, maxLevel);
            clmRETURN_ERROR(CL_INVALID_MIP_LEVEL);
        }
    }

    /* Check Texture */
    glGetTexLevelParameteriv(Target, MipLevel,GL_TEXTURE_WIDTH, &realWidth );
    glGetTexLevelParameteriv(Target, MipLevel, GL_TEXTURE_HEIGHT, &realHeight );
    glGetTexLevelParameteriv(Target, MipLevel, GL_TEXTURE_DEPTH, &realDepth );
    glGetTexLevelParameteriv(Target, MipLevel, GL_TEXTURE_INTERNAL_FORMAT, &textureFormat );

    clfQueryGLEnum2Enum(textureFormat, 0, &realCLType, &realCLFormat, gcvNULL, gcvNULL, gcvNULL,gcvNULL);

    imageFormat.image_channel_order = realCLFormat;
    imageFormat.image_channel_data_type = realCLType;

    imageDesc.image_width = realWidth;
    imageDesc.image_height = realHeight;
    imageDesc.image_depth = realDepth;
    imageDesc.image_array_size = 0;
    imageDesc.buffer = NULL;
    imageDesc.num_mip_levels = MipLevel;
    imageDesc.num_samples = 0;
    imageDesc.image_type = CL_MEM_OBJECT_IMAGE3D;
    imageDesc.image_row_pitch = 0;
    imageDesc.image_slice_pitch = 0;

    image = clfCLGLShareCreateImageWrapper(Context, Flags, &imageFormat, &imageDesc, gcvNULL, ErrcodeRet);

    image->fromGL               = gcvTRUE;
    image->glObj                = Texture;
    image->glObjType            = CL_GL_OBJECT_TEXTURE3D;
    image->glTexTarget          = Target;

    glBindTexture(Target, origTex);

    gcmFOOTER_ARG("%d image=0x%x",
                  CL_SUCCESS, image);
    return image;

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
    GLint           renderbufferFormat      = 0;
    cl_channel_order clFormat;
    cl_channel_type  clType;
    cl_int           channelCount;
    GLenum          glFormat;
    GLenum          glType        = 0;
    clsMem_PTR      image               = gcvNULL;
    cl_image_format imageFormat;
    cl_image_desc   imageDesc;

    gcmHEADER_ARG("Context=0x%x Flags=%u Renderbuffer=%u",
                   Context, Flags, Renderbuffer);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011066: (clCreateFromGLRenderbuffer) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (Flags & ~(CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-011067: (clCreateFromGLRenderbuffer) invalid Flags (%u).\n", Flags);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &outWidth );
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &outHeight );
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &renderbufferFormat );

    clfQueryGLEnum2Enum(renderbufferFormat, 0, &clType, &clFormat, &channelCount, &glType, &glFormat, gcvNULL);

    imageFormat.image_channel_order = clFormat;
    imageFormat.image_channel_data_type = clType;


    imageDesc.image_width = outWidth;
    imageDesc.image_height = outHeight;
    imageDesc.image_depth = 1;
    imageDesc.image_array_size = 0;
    imageDesc.image_slice_pitch = 0;
    imageDesc.buffer = NULL;
    imageDesc.num_mip_levels = 0;
    imageDesc.num_samples = 0;
    imageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    imageDesc.image_row_pitch = 0;
    imageDesc.image_row_pitch = 0;
    /* don't copy host data, it may dirty when use, so update it at enqueueAquire */
    image = clfCLGLShareCreateImageWrapper(Context, Flags, &imageFormat, &imageDesc, gcvNULL, ErrcodeRet);

    image->fromGL               = gcvTRUE;
    image->glObj                = Renderbuffer;
    image->glObjType            = CL_GL_OBJECT_RENDERBUFFER;

    if (ErrcodeRet)
    {
        *ErrcodeRet = CL_SUCCESS;
    }

    gcmFOOTER_ARG("%d image=0x%x",
                  CL_SUCCESS, image);
    return image;

OnError:

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
    gctPOINTER*                     pointers = gcvNULL;
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
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER)*NumObjects, (gctPOINTER*)&acquireGLObjects->objectsDatas), CL_OUT_OF_HOST_MEMORY);
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(size_t)*NumObjects, (gctPOINTER*)&acquireGLObjects->w), CL_OUT_OF_HOST_MEMORY);
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(size_t)*NumObjects, (gctPOINTER*)&acquireGLObjects->h), CL_OUT_OF_HOST_MEMORY);
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(size_t)*NumObjects, (gctPOINTER*)&acquireGLObjects->d), CL_OUT_OF_HOST_MEMORY);
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(cl_image_format)*NumObjects, (gctPOINTER*)&acquireGLObjects->imFormats), CL_OUT_OF_HOST_MEMORY);
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER)*NumObjects, (gctPOINTER*)&pointers), CL_OUT_OF_HOST_MEMORY);
    gcoOS_MemFill(pointers, 0, sizeof(gctPOINTER)*NumObjects);

    for (i = 0; i < NumObjects; i++)
    {
        GLint internalFormat = 0;
        GLenum glType = 0;
        GLenum glFormat = 0;
        switch (MemObjects[i]->glObjType)
        {
            case CL_GL_OBJECT_BUFFER:
                {
                clsMem_PTR      buffer     = MemObjects[i];
                GLint           bufferSize = 0;
                gctSIZE_T       size       = 0;
                gctPOINTER      data;
                GLint           origBuffer;

                glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &origBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, buffer->glObj);

                /* TODO - Need to check if BufObj is valid GL buffer object. */

                glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);

                if (bufferSize <= 0)
                {
                    gcmUSER_DEBUG_ERROR_MSG(
                        "OCL-011068: (clEnqueueAcquireGLObjects) BufObj has no data.\n");
                    clmRETURN_ERROR(CL_INVALID_GL_OBJECT);
                }

                /* GLES3 */
                /*data = glMapBufferRange(GL_ARRAY_BUFFER, 0, bufferSize, GL_MAP_READ_BIT | GL_MAP_BUFFER_OBJ_VIV);*/
                /* GLES2 */
                data = glMapBufferOES(GL_ARRAY_BUFFER, GL_MAP_BUFFER_OBJ_VIV);

                if (data == gcvNULL)
                {
                    gcmUSER_DEBUG_ERROR_MSG(
                        "OCL-011069: (clEnqueueAcquireGLObjects) BufObj is not mapped.\n");
                    clmRETURN_ERROR(CL_INVALID_GL_OBJECT);
                }

                acquireGLObjects->objectsDatas[i] = data;
                /* use block mode to update data in current implement */

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
                glBindBuffer(GL_ARRAY_BUFFER, origBuffer);
            }
            break;
        case CL_GL_OBJECT_TEXTURE2D:
            {
                gctINT          status;
                GLint           realWidth  = MemObjects[i]->u.image.width;
                GLint           realHeight = MemObjects[i]->u.image.height;
                GLint           realDepth = 1;
                size_t          outBytes   = 0;
                GLint           origTex = 0;
                GLenum          target;
                if (  MemObjects[i]->glTexTarget == GL_TEXTURE_CUBE_MAP_POSITIVE_X
                    ||MemObjects[i]->glTexTarget == GL_TEXTURE_CUBE_MAP_POSITIVE_Y
                    ||MemObjects[i]->glTexTarget == GL_TEXTURE_CUBE_MAP_POSITIVE_Z
                    ||MemObjects[i]->glTexTarget == GL_TEXTURE_CUBE_MAP_NEGATIVE_X
                    ||MemObjects[i]->glTexTarget == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
                    ||MemObjects[i]->glTexTarget == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
                {
                    target = GL_TEXTURE_CUBE_MAP;
                }
                else
                {
                    target = GL_TEXTURE_2D;
                }

                glGetIntegerv(target == GL_TEXTURE_2D ? GL_TEXTURE_BINDING_2D : GL_TEXTURE_BINDING_CUBE_MAP, &origTex);
                glBindTexture(target, acquireGLObjects->memObjects[i]->glObj);

                outBytes = realWidth * realHeight * realDepth * acquireGLObjects->memObjects[i]->u.image.elementSize;

                status = gcoOS_Allocate(gcvNULL, outBytes, &pointers[i]);
                if (gcmIS_ERROR(status))
                {
                    gcmUSER_DEBUG_ERROR_MSG(
                        "OCL-011070: (clEnqueueAcquireGLObjects) Cannot allocate memory.\n");
                    clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
                }

                /*fill texture data into buffer */
                glGetTexLevelParameteriv(MemObjects[i]->glTexTarget, MemObjects[i]->u.image.mipLevel, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat );

                clfQueryGLEnum2Enum(internalFormat, 0, gcvNULL, gcvNULL, gcvNULL,&glType, &glFormat, gcvNULL);
                glGetTexImage(MemObjects[i]->glTexTarget, MemObjects[i]->u.image.mipLevel, glFormat, glType, pointers[i]);

                acquireGLObjects->objectsDatas[i] = (cl_char *) pointers[i];

                glBindTexture(target, origTex);

            }
            break;
        case CL_GL_OBJECT_TEXTURE3D:
            {
                gctINT          status;
                GLint           realWidth  = MemObjects[i]->u.image.width;
                GLint           realHeight = MemObjects[i]->u.image.height;
                GLint           realDepth = MemObjects[i]->u.image.depth;
                size_t          outBytes   = 0;
                GLint           origTex = 0;

                glGetIntegerv(GL_TEXTURE_BINDING_3D, &origTex);
                glBindTexture(GL_TEXTURE_3D, acquireGLObjects->memObjects[i]->glObj);

                outBytes = realWidth * realHeight * realDepth * acquireGLObjects->memObjects[i]->u.image.elementSize;
                status = gcoOS_Allocate(gcvNULL, outBytes, &pointers[i]);
                if (gcmIS_ERROR(status))
                {
                    gcmUSER_DEBUG_ERROR_MSG(
                        "OCL-011071: (clEnqueueAcquireGLObjects) Cannot allocate memory.\n");
                    clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
                }

                /*fill texture data into buffer */
                glGetTexLevelParameteriv(GL_TEXTURE_3D, MemObjects[i]->u.image.mipLevel, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat );

                clfQueryGLEnum2Enum(internalFormat, 0, gcvNULL, gcvNULL, gcvNULL,&glType, &glFormat, gcvNULL);
                glGetTexImage(GL_TEXTURE_3D, MemObjects[i]->u.image.mipLevel, glFormat, glType, pointers[i]);

                acquireGLObjects->objectsDatas[i] = (cl_char *) pointers[i];

                glBindTexture(GL_TEXTURE_3D, origTex);

            }
            break;
        case CL_GL_OBJECT_RENDERBUFFER:
            {
                GLint           realWidth  = MemObjects[i]->u.image.width;
                GLint           realHeight = MemObjects[i]->u.image.height;
                GLint           realDepth = MemObjects[i]->u.image.depth;
                GLint           elemenSize = MemObjects[i]->u.image.elementSize;
                size_t          outBytes   = 0;
                glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &internalFormat );

                clfQueryGLEnum2Enum(internalFormat, 0, gcvNULL, gcvNULL, gcvNULL, &glType, &glFormat, gcvNULL);
                outBytes = realWidth * realHeight * elemenSize;

                acquireGLObjects->w[i] = realWidth;
                acquireGLObjects->h[i] = realHeight;
                acquireGLObjects->d[i] = realDepth;
                acquireGLObjects->imFormats[i] = MemObjects[i]->u.image.format;

                {
                    status = gcoOS_Allocate(gcvNULL, outBytes, &pointers[i]);
                    if (gcmIS_ERROR(status))
                    {
                        gcmUSER_DEBUG_ERROR_MSG(
                            "OCL-011072: (clEnqueueAcquireGLObjects) Cannot allocate memory.\n");
                        clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
                    }
                    acquireGLObjects->objectsDatas[i] = (cl_char *) pointers[i];

                    /* TODO - Use resolve to speed up the conversion. */
                    glFinish();
                    glReadPixels(0, 0, (GLsizei)realWidth, (GLsizei)realHeight, glFormat, glType, acquireGLObjects->objectsDatas[i]);
                }
            }
            break;
        case CL_GL_OBJECT_TEXTURE2D_ARRAY:
            break;
        case CL_GL_OBJECT_TEXTURE1D:
            break;
        case CL_GL_OBJECT_TEXTURE1D_ARRAY:
            break;
        case CL_GL_OBJECT_TEXTURE_BUFFER:
            break;
        default:
            break;
        }
    }

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvTRUE),
               CL_OUT_OF_HOST_MEMORY);

    for (i = 0; i < NumObjects; i++)
    {
        if(pointers[i])
            gcoOS_Free(gcvNULL, pointers[i]);
    }
    gcoOS_Free(gcvNULL, pointers);

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
    gctPOINTER*                     pointers = gcvNULL;
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
                "OCL-011052: (clEnqueueReleaseGLObjects) MemObjects[i] is not created from OpenGL object.\n", i);
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
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER)*NumObjects, (gctPOINTER*)&releaseGLObjects->objectsDatas), CL_OUT_OF_HOST_MEMORY);
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(size_t)*NumObjects, (gctPOINTER*)&releaseGLObjects->w), CL_OUT_OF_HOST_MEMORY);
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(size_t)*NumObjects, (gctPOINTER*)&releaseGLObjects->h), CL_OUT_OF_HOST_MEMORY);
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(size_t)*NumObjects, (gctPOINTER*)&releaseGLObjects->d), CL_OUT_OF_HOST_MEMORY);
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(cl_image_format)*NumObjects, (gctPOINTER*)&releaseGLObjects->imFormats), CL_OUT_OF_HOST_MEMORY);
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(cl_int)*NumObjects, (gctPOINTER*)&releaseGLObjects->elementSize), CL_OUT_OF_HOST_MEMORY);
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER)*NumObjects, (gctPOINTER*)&pointers), CL_OUT_OF_HOST_MEMORY);
    gcoOS_MemFill(pointers, 0, sizeof(gctPOINTER)*NumObjects);

    for (i = 0; i < NumObjects; i++)
    {
        switch (MemObjects[i]->glObjType)
        {
        case CL_GL_OBJECT_BUFFER:
            {
                gcmVERIFY_OK(gcoCL_UnshareMemory(MemObjects[i]->u.buffer.node));
            }
            break;
        case CL_GL_OBJECT_TEXTURE2D:
            {
                size_t          outBytes            = 0;

                releaseGLObjects->w[i] = releaseGLObjects->memObjects[i]->u.image.width;
                releaseGLObjects->h[i] = releaseGLObjects->memObjects[i]->u.image.height;
                releaseGLObjects->d[i] = 1;

                releaseGLObjects->imFormats[i] = releaseGLObjects->memObjects[i]->u.image.format;
                releaseGLObjects->elementSize[i] = releaseGLObjects->memObjects[i]->u.image.elementSize;

                outBytes = releaseGLObjects->w[i]
                           * releaseGLObjects->h[i]
                           * releaseGLObjects->d[i]
                           * releaseGLObjects->elementSize[i];
                status = gcoOS_Allocate(gcvNULL, outBytes, &pointers[i]);
                if (gcmIS_ERROR(status))
                {
                    gcmUSER_DEBUG_ERROR_MSG(
                        "OCL-011073: (clEnqueueReleaseGLObjects) Cannot allocate memory.\n");
                    clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
                }
                releaseGLObjects->objectsDatas[i] = (cl_char *) pointers[i];
            }
            break;
        case CL_GL_OBJECT_TEXTURE3D:
            {
                size_t          outBytes            = 0;

                releaseGLObjects->w[i] = releaseGLObjects->memObjects[i]->u.image.width;
                releaseGLObjects->h[i] = releaseGLObjects->memObjects[i]->u.image.height;
                releaseGLObjects->d[i] = releaseGLObjects->memObjects[i]->u.image.depth;

                releaseGLObjects->imFormats[i] = releaseGLObjects->memObjects[i]->u.image.format;
                releaseGLObjects->elementSize[i] = releaseGLObjects->memObjects[i]->u.image.elementSize;

                outBytes = releaseGLObjects->w[i]
                           * releaseGLObjects->h[i]
                           * releaseGLObjects->d[i]
                           * releaseGLObjects->elementSize[i];
                status = gcoOS_Allocate(gcvNULL, outBytes, &pointers[i]);
                if (gcmIS_ERROR(status))
                {
                    gcmUSER_DEBUG_ERROR_MSG(
                        "OCL-011074: (clEnqueueReleaseGLObjects) Cannot allocate memory.\n");
                    clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
                }
                releaseGLObjects->objectsDatas[i] = (cl_char *) pointers[i];
            }
            break;
        case CL_GL_OBJECT_RENDERBUFFER:
            {
                size_t          outBytes            = 0;

                releaseGLObjects->w[i] = releaseGLObjects->memObjects[i]->u.image.width;
                releaseGLObjects->h[i] = releaseGLObjects->memObjects[i]->u.image.height;
                releaseGLObjects->d[i] = 1;

                releaseGLObjects->imFormats[i] = MemObjects[i]->u.image.format;
                releaseGLObjects->elementSize[i] = MemObjects[i]->u.image.elementSize;

                outBytes = releaseGLObjects->memObjects[i]->u.image.width
                         * releaseGLObjects->memObjects[i]->u.image.height
                         * releaseGLObjects->elementSize[i];
                status = gcoOS_Allocate(gcvNULL, outBytes, &pointers[i]);
                if (gcmIS_ERROR(status))
                {
                    gcmUSER_DEBUG_ERROR_MSG(
                        "OCL-011075: (clEnqueueReleaseGLObjects) Cannot allocate memory.\n");
                    clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
                }
                releaseGLObjects->objectsDatas[i] = (cl_char *) pointers[i];
            }
            break;
        case CL_GL_OBJECT_TEXTURE2D_ARRAY:
            break;
        case CL_GL_OBJECT_TEXTURE1D:
            break;
        case CL_GL_OBJECT_TEXTURE1D_ARRAY:
            break;
        case CL_GL_OBJECT_TEXTURE_BUFFER:
            break;
        default:
            break;
        }
    }

    /* Read back */
    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvTRUE),
               CL_OUT_OF_HOST_MEMORY);

    /* Update */
    for (i = 0; i < NumObjects; i++)
    {
        GLint width = 0;
        GLint height = 0;
        GLenum rbFormat; GLenum rbType;

        switch (MemObjects[i]->glObjType)
        {
        case CL_GL_OBJECT_BUFFER:
            break;
        case CL_GL_OBJECT_TEXTURE2D:
            {
                GLint textureFormat;
                GLenum format,type;
                GLint origTex;
                GLenum target;
                if (  MemObjects[i]->glTexTarget == GL_TEXTURE_CUBE_MAP_POSITIVE_X
                    ||MemObjects[i]->glTexTarget == GL_TEXTURE_CUBE_MAP_POSITIVE_Y
                    ||MemObjects[i]->glTexTarget == GL_TEXTURE_CUBE_MAP_POSITIVE_Z
                    ||MemObjects[i]->glTexTarget == GL_TEXTURE_CUBE_MAP_NEGATIVE_X
                    ||MemObjects[i]->glTexTarget == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
                    ||MemObjects[i]->glTexTarget == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
                {
                    target = GL_TEXTURE_CUBE_MAP;
                }
                else
                {
                    target = GL_TEXTURE_2D;
                }
                clfCLGLShareReleaseMemObjectData(MemObjects[i]);

                glGetIntegerv(target == GL_TEXTURE_2D ? GL_TEXTURE_BINDING_2D : GL_TEXTURE_BINDING_CUBE_MAP, &origTex);
                glBindTexture(target, MemObjects[i]->glObj);
                glGetTexLevelParameteriv(MemObjects[i]->glTexTarget, MemObjects[i]->u.image.mipLevel, GL_TEXTURE_INTERNAL_FORMAT, &textureFormat );
                clfQueryGLEnum2Enum(textureFormat, 0, gcvNULL, gcvNULL, gcvNULL, &type, &format, gcvNULL);
                glTexImage2D(MemObjects[i]->glTexTarget,
                             MemObjects[i]->u.image.mipLevel,
                             textureFormat,
                             MemObjects[i]->u.image.width,
                             MemObjects[i]->u.image.height,
                             0,
                             format,
                             type,
                             pointers[i]);
                glBindTexture(target, origTex);
            }
            break;
        case CL_GL_OBJECT_TEXTURE3D:
            {
                GLint textureFormat;
                GLenum format,type;
                GLint origTex;
                clfCLGLShareReleaseMemObjectData(MemObjects[i]);
                glGetIntegerv(GL_TEXTURE_BINDING_3D, &origTex);
                glBindTexture(GL_TEXTURE_3D, MemObjects[i]->glObj);
                glGetTexLevelParameteriv(GL_TEXTURE_3D, MemObjects[i]->u.image.mipLevel, GL_TEXTURE_INTERNAL_FORMAT, &textureFormat );
                clfQueryGLEnum2Enum(textureFormat, 0, gcvNULL, gcvNULL, gcvNULL, &type, &format, gcvNULL);
                glTexImage3D(GL_TEXTURE_3D,
                             MemObjects[i]->u.image.mipLevel,
                             textureFormat,
                             MemObjects[i]->u.image.width,
                             MemObjects[i]->u.image.height,
                             MemObjects[i]->u.image.depth,
                             0,
                             format,
                             type,
                             pointers[i]);
                glBindTexture(GL_TEXTURE_3D, origTex);
            }
            break;
        case CL_GL_OBJECT_RENDERBUFFER:
            {
                const char *vssrc =
                    "varying   mediump vec2 texCoord;\n"
                    "attribute vec2 inPosition;\n"
                    "void main() {\n"
                    "    texCoord    = vec2((inPosition.x+1.0)/2.0, (inPosition.y+1.0)/2.0);\n"
                    "    gl_Position = vec4(inPosition.x, inPosition.y, 0.0, 1.0);\n"
                    "}\n";
                const char *fssrc =
                    "uniform sampler2D tex;\n"
                    "varying mediump vec2      texCoord;\n"
                    "void main() {\n"
                    "    gl_FragColor =  texture2D(tex, texCoord);\n"
                    "}\n";
                GLuint vs, fs, program;
                GLuint positionIdx = 0;
                GLfloat x1 = -1.0f, x2 = 1.0f, y1 = -1.0f, y2 = 1.0f;
                GLfloat vertices[4][2];
                GLint renderbufferFormat;
                GLuint texture;
                GLint  origTex, origProgram;
                glGetIntegerv(GL_TEXTURE_BINDING_2D, &origTex);
                glGetIntegerv(GL_CURRENT_PROGRAM, &origProgram);
                glGenTextures( 1, &texture );
                glBindTexture( GL_TEXTURE_2D, texture );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
                glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width );
                glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height );
                glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &renderbufferFormat );

                clfQueryGLEnum2Enum(renderbufferFormat, 0, gcvNULL, gcvNULL, gcvNULL, &rbType, &rbFormat, gcvNULL);

                /* TODO - Use resolve to speed up the conversion. */

                glTexImage2D( GL_TEXTURE_2D, 0, renderbufferFormat, MemObjects[i]->u.image.width, MemObjects[i]->u.image.height, 0, rbFormat, rbType, pointers[i] );
                /* Render fullscreen textured quad */
                glViewport(0, 0, width, height);

                vertices[0][0] = x1; vertices[0][1] = y1;
                vertices[1][0] = x2; vertices[1][1] = y1;
                vertices[2][0] = x1; vertices[2][1] = y2;
                vertices[3][0] = x2; vertices[3][1] = y2;

                vs = glCreateShader(GL_VERTEX_SHADER);
                fs = glCreateShader(GL_FRAGMENT_SHADER);

                glShaderSource(vs, 1, &vssrc, NULL);
                glShaderSource(fs, 1, &fssrc, NULL);

                glCompileShader(vs);
                glCompileShader(fs);

                program = glCreateProgram();
                glAttachShader(program, vs);
                glAttachShader(program, fs);
                glLinkProgram(program);
                glUseProgram(program);

                positionIdx = glGetAttribLocation(program, "inPosition");
                glEnableVertexAttribArray(positionIdx);
                glVertexAttribPointer(positionIdx, 2, GL_FLOAT, GL_FALSE, 0, vertices);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                glUseProgram(origProgram);
                glBindTexture(GL_TEXTURE_2D, origTex);

                glDeleteProgram(program);
                glDeleteShader(vs);
                glDeleteShader(fs);
                glDeleteTextures(1, &texture);
            }
            break;
        case CL_GL_OBJECT_TEXTURE2D_ARRAY:
            break;
        case CL_GL_OBJECT_TEXTURE1D:
            break;
        case CL_GL_OBJECT_TEXTURE1D_ARRAY:
            break;
        case CL_GL_OBJECT_TEXTURE_BUFFER:
            break;
        default:
            break;
        }
        if(pointers[i])
            gcoOS_Free(gcvNULL, pointers[i]);
    }
    gcoOS_Free(gcvNULL, pointers);

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
