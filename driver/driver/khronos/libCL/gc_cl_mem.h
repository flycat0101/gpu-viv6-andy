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


#ifndef __gc_cl_memory_h_
#define __gc_cl_memory_h_

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************\
************************* Memory Object Definition *************************
\******************************************************************************/

/* cl_channel_order */
#define CLK_R                       CL_R
#define CLK_A                       CL_A
#define CLK_RG                      CL_RG
#define CLK_RA                      CL_RA
#define CLK_RGB                     CL_RGB
#define CLK_RGBA                    CL_RGBA
#define CLK_BGRA                    CL_BGRA
#define CLK_ARGB                    CL_ARGB
#define CLK_INTENSITY               CL_INTENSITY
#define CLK_LUMINANCE               CL_LUMINANCE
#define CLK_Rx                      CL_Rx
#define CLK_RGx                     CL_RGx
#define CLK_RGBx                    CL_RGBx

/* cl_channel_type */
#define CLK_SNORM_INT8              CL_SNORM_INT8
#define CLK_SNORM_INT16             CL_SNORM_INT16
#define CLK_UNORM_INT8              CL_UNORM_INT8
#define CLK_UNORM_INT16             CL_UNORM_INT16
#define CLK_UNORM_SHORT_565         CL_UNORM_SHORT_565
#define CLK_UNORM_SHORT_555         CL_UNORM_SHORT_555
#define CLK_UNORM_INT_101010        CL_UNORM_INT_101010
#define CLK_SIGNED_INT8             CL_SIGNED_INT8
#define CLK_SIGNED_INT16            CL_SIGNED_INT16
#define CLK_SIGNED_INT32            CL_SIGNED_INT32
#define CLK_UNSIGNED_INT8           CL_UNSIGNED_INT8
#define CLK_UNSIGNED_INT16          CL_UNSIGNED_INT16
#define CLK_UNSIGNED_INT32          CL_UNSIGNED_INT32
#define CLK_HALF_FLOAT              CL_HALF_FLOAT
#define CLK_FLOAT                   CL_FLOAT

typedef struct _cl_mem_callback     clsMemObjCallback;
typedef clsMemObjCallback *         clsMemObjCallback_PTR;
typedef struct _buffer_wait_event   clsBufferWaitEvent;
typedef clsBufferWaitEvent *        clsBufferWaitEvent_PTR;

struct _cl_mem_callback
{
    void                    (CL_CALLBACK * pfnNotify)(cl_mem, void *);
    void                    *userData;
    clsMemObjCallback_PTR   next;
};

typedef struct _cl_image_header
{
    gctINT                  width;
    gctINT                  height;
    gctINT                  depth;              /* 0: 2D image. */
    gctINT                  channelDataType;
    gctINT                  channelOrder;
    gctINT                  samplerValue;
    gctUINT                 rowPitch;
    gctUINT                 slicePitch;
    gctINT                  arraySize;
    gctUINT                 imageType;
    gceTILING               tiling;
    gctUINT                 physical;           /* GPU address: 32 bit only. */
}
clsImageHeader;

struct _buffer_wait_event
{
    /* commandQueueID is only used to identify event owner(do not access its member)
       so no need to maintain reference count. and it should be also safe in:
       1. commandQueueID is invalid(the commandQueue has been detroied)
       2. commandQueueID is reused(commandQueue destroied and re-created with the same value)
    */
    clsCommandQueue_PTR     commandQueueID;
    /* track buffer status event */
    clsEvent_PTR            event;
    clsBufferWaitEvent_PTR  next;
};

typedef struct _cl_mem
{
    clsIcdDispatch_PTR      dispatch;
    cleOBJECT_TYPE          objectType;
    gctUINT                 id;
    gcsATOM_PTR             referenceCount;

    clsContext_PTR          context;

    cl_mem_object_type      type;       /* Buffer or image. */
    cl_mem_flags            flags;
    void *                  host;
    clsMemObjCallback_PTR   memObjCallback;
    gctUINT                 mapCount;
#if BUILD_OPENCL_12
    cl_map_flags            mapFlag;
#endif

    /* GL sharing. */
    gctBOOL                 fromGL;
    cl_GLuint               glObj;
    cl_gl_object_type       glObjType;

    /* Modify mutex lock. */
    gctPOINTER              mutex;

    /* track buffer status event */
    clsBufferWaitEvent      waitEvent;

    union {
        struct {
            size_t                  size;
            clsMem_PTR              parentBuffer;
            cl_buffer_create_type   createType;  /* buffer or sub-buffer */
            cl_buffer_region        bufferCreateInfo;
            gctUINT                 allocatedSize;
            gctPHYS_ADDR            physical;
            gctPOINTER              logical;
            gcsSURF_NODE_PTR        node;
            gctBOOL                 wrapped;
            gctUINT32               wrappedNode;
#if BUILD_OPENCL_12
            clsMem_PTR              image;      /* The associated image object. Used in 1D image buffer. */
#endif

        } buffer;

        struct {
            void *                  image;
            clsImageFormat          format;
            size_t                  width;
            size_t                  height;
            size_t                  depth;
            size_t                  rowPitch;
            size_t                  slicePitch;
            gctSIZE_T               elementSize;
            gctSIZE_T               size;
            gctUINT                 allocatedSize;
            gctPHYS_ADDR            physical;           /* Image header. */
            gctPOINTER              logical;            /* Image header. */
            gcsSURF_NODE_PTR        node;
            gceSURF_FORMAT          internalFormat;
            gcoTEXTURE              texture;
            gcoSURF                 surface;
            gctBOOL                 surfaceMapped;
            gctUINT32               texturePhysical;    /* Texture data. */
            gctPOINTER              textureLogical;     /* Texture data. */
            gctUINT                 textureStride;      /* Texture data. */

            gceTILING               tiling;
            /* GL sharing. */
            cl_GLenum               textureTarget;
            cl_GLint                mipLevel;
            gctUINT                 textureSlicePitch;  /* Texture data. */
#if BUILD_OPENCL_12
            clsMem_PTR              buffer;
            size_t                  arraySize;
#endif
        } image;
    } u;
}
clsMem;

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/

gctINT
clfExecuteCommandReadBuffer(
    clsCommand_PTR              Command
    );

gctINT
clfExecuteCommandReadBufferRect(
    clsCommand_PTR              Command
    );

gctINT
clfExecuteCommandWriteBuffer(
    clsCommand_PTR              Command
    );

#if BUILD_OPENCL_12
gctINT
clfExecuteCommandFillBuffer(
    clsCommand_PTR              Command
    );
#endif

gctINT
clfExecuteCommandWriteBufferRect(
    clsCommand_PTR              Command
    );

gctINT
clfExecuteCommandCopyBuffer(
    clsCommand_PTR              Command
    );

gctINT
clfExecuteCommandCopyBufferRect(
    clsCommand_PTR              Command
    );

gctINT
clfExecuteCommandReadImage(
    clsCommand_PTR  Command
    );

gctINT
clfExecuteCommandWriteImage(
    clsCommand_PTR  Command
    );

gctINT
clfExecuteCommandFillImage(
    clsCommand_PTR  Command
    );

gctINT
clfExecuteCommandCopyImage(
    clsCommand_PTR  Command
    );

gctINT
clfExecuteCommandCopyImageToBuffer(
    clsCommand_PTR  Command
    );

gctINT
clfExecuteCommandCopyBufferToImage(
    clsCommand_PTR  Command
    );

gctINT
clfExecuteCommandMapBuffer(
    clsCommand_PTR  Command
    );

gctINT
clfExecuteCommandMapImage(
    clsCommand_PTR  Command
    );

gctINT
clfExecuteCommandUnmapMemObject(
    clsCommand_PTR  Command
    );

gctINT
clfNewBuffer(
    clsContext_PTR  Context,
    clsMem_PTR *    Buffer
    );

gctINT
clfNewImage(
    clsContext_PTR  Context,
    clsMem_PTR *    Image
    );

gctINT
clfImageFormat2GcFormat(
    const clsImageFormat *  Format,
    gctSIZE_T *             ElementSize,
    gceSURF_FORMAT *        InternalFormat,
    gctSIZE_T *             ChanelCount
    );

#if BUILD_OPENCL_12
gctINT
clfPackImagePixeli(
    cl_int *srcVector,
    const cl_image_format *imageFormat,
    void *outData );

gctINT
clfPackImagePixelui(
    cl_uint *srcVector,
    const cl_image_format *imageFormat,
    void *outData );

gctINT
clfPackImagePixelf(
    cl_float *srcVector,
    const cl_image_format *imageFormat,
    void *outData );
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __gc_cl_memory_h_ */
