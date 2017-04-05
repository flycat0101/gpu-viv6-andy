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


#ifndef __gc_cl_command_h_
#define __gc_cl_command_h_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************\
|************************* Command Object Definition *************************|
\*****************************************************************************/

typedef struct _cl_sync_point   clsSyncPoint;
typedef clsSyncPoint *          clsSyncPoint_PTR;

struct _cl_sync_point
{
    gctUINT64               enqueueNo;
    clsSyncPoint_PTR        next;
    clsSyncPoint_PTR        previous;
};

typedef struct _cl_commit_request   clsCommitRequest;
typedef clsCommitRequest *          clsCommitRequest_PTR;

struct _cl_commit_request
{
    gctBOOL                     stall;
    gctUINT64                   nextEnqueueNo;
    gctSIGNAL                   signal;
    clsCommitRequest_PTR        next;
    clsCommitRequest_PTR        previous;
};

typedef struct clsPrivateBuffer   clsPrivateBuffer;
typedef clsPrivateBuffer *          clsPrivateBuffer_PTR;

struct clsPrivateBuffer
{
    clsMemAllocInfo_PTR         buffer;
    clsKernel_PTR               kernel;
    gcUNIFORM                   uniform;
    clsPrivateBuffer_PTR        next;
};

typedef struct _cl_command_queue
{
    clsIcdDispatch_PTR          dispatch;
    cleOBJECT_TYPE              objectType;
    gctUINT                     id;
    gcsATOM_PTR                 referenceCount;

    clsContext_PTR              context;
    clsDeviceId_PTR             device;
    gctUINT                     numCommands;
    clsSyncPoint_PTR            syncPointList;
    gctPOINTER                  syncPointListMutex;
    clsCommand_PTR              commandHead;
    clsCommand_PTR              commandTail;
    clsCommand_PTR              deferredReleaseCommandHead;
    clsCommand_PTR              deferredReleaseCommandTail;
    gctUINT64                   nextEnqueueNo;
    clsCommitRequest_PTR        commitRequestList;
    gctPOINTER                  commandListMutex;

    cl_command_queue_properties properties;

    clsCommandQueue_PTR         next;
    clsCommandQueue_PTR         previous;

    gctPOINTER                  workerThread;
    gctSIGNAL                   workerStartSignal;
    gctSIGNAL                   workerStopSignal;

    clsPrivateBuffer_PTR        privateBufList;
}
clsCommandQueue;

typedef enum _cleCOMMAND_TYPE
{
    clvCOMMAND_UNKNOWN,
    clvCOMMAND_READ_BUFFER,
    clvCOMMAND_READ_BUFFER_RECT,
    clvCOMMAND_WRITE_BUFFER,
#if BUILD_OPENCL_12
    clvCOMMAND_FILL_BUFFER,
#endif
    clvCOMMAND_WRITE_BUFFER_RECT,
    clvCOMMAND_COPY_BUFFER,
    clvCOMMAND_COPY_BUFFER_RECT,
    clvCOMMAND_READ_IMAGE,
    clvCOMMAND_WRITE_IMAGE,
#if BUILD_OPENCL_12
    clvCOMMAND_FILL_IMAGE,
#endif
    clvCOMMAND_COPY_IMAGE,
    clvCOMMAND_COPY_IMAGE_TO_BUFFER,
    clvCOMMAND_COPY_BUFFER_TO_IMAGE,
#if BUILD_OPENCL_12
    clvCOMMAND_MIGRATE_MEM_OBJECTS,
#endif
    clvCOMMAND_MAP_BUFFER,
    clvCOMMAND_MAP_IMAGE,
    clvCOMMAND_UNMAP_MEM_OBJECT,
    clvCOMMAND_NDRANGE_KERNEL,
    clvCOMMAND_TASK,
    clvCOMMAND_NATIVE_KERNEL,
    clvCOMMAND_MARKER,
    clvCOMMAND_WAIT_FOR_EVENTS,
    clvCOMMAND_BARRIER,
    clvCOMMAND_ACQUIRE_GL_OBJECTS,
    clvCOMMAND_RELEASE_GL_OBJECTS,
}
cleCOMMAND_TYPE;

typedef struct {
    clsMem_PTR          buffer;
    cl_bool             blockingRead;
    size_t              offset;
    size_t              cb;
    gctPOINTER          ptr;
} clsCommandReadBuffer,
* clsCommandReadBuffer_PTR;

typedef struct {
    clsMem_PTR          buffer;
    cl_bool             blockingRead;
    size_t              bufferOrigin[3];
    size_t              hostOrigin[3];
    size_t              region[3];
    size_t              bufferRowPitch;
    size_t              bufferSlicePitch;
    size_t              hostRowPitch;
    size_t              hostSlicePitch;
    gctPOINTER          ptr;
} clsCommandReadBufferRect,
* clsCommandReadBufferRect_PTR;

typedef struct {
    clsMem_PTR          buffer;
    cl_bool             blockingWrite;
    size_t              offset;
    size_t              cb;
    gctCONST_POINTER    ptr;
} clsCommandWriteBuffer,
* clsCommandWriteBuffer_PTR;

#if BUILD_OPENCL_12
typedef struct {
    clsMem_PTR          buffer;
    size_t              offset;
    size_t              size;
    size_t              pattern_size;
    gctCONST_POINTER    pattern;
} clsCommandFillBuffer,
* clsCommandFillBuffer_PTR;
#endif

typedef struct {
    clsMem_PTR          buffer;
    cl_bool             blockingWrite;
    size_t              bufferOrigin[3];
    size_t              hostOrigin[3];
    size_t              region[3];
    size_t              bufferRowPitch;
    size_t              bufferSlicePitch;
    size_t              hostRowPitch;
    size_t              hostSlicePitch;
    gctCONST_POINTER    ptr;
} clsCommandWriteBufferRect,
* clsCommandWriteBufferRect_PTR;

typedef struct {
    clsMem_PTR          srcBuffer;
    clsMem_PTR          dstBuffer;
    size_t              srcOffset;
    size_t              dstOffset;
    size_t              cb;
} clsCommandCopyBuffer,
* clsCommandCopyBuffer_PTR;

typedef struct {
    clsMem_PTR          srcBuffer;
    clsMem_PTR          dstBuffer;
    size_t              srcOrigin[3];
    size_t              dstOrigin[3];
    size_t              region[3];
    size_t              srcRowPitch;
    size_t              srcSlicePitch;
    size_t              dstRowPitch;
    size_t              dstSlicePitch;
} clsCommandCopyBufferRect,
* clsCommandCopyBufferRect_PTR;

typedef struct {
    clsMem_PTR          image;
    cl_bool             blockingRead;
    size_t              origin[3];
    size_t              region[3];
    size_t              rowPitch;
    size_t              slicePitch;
    gctPOINTER          ptr;
} clsCommandReadImage,
* clsCommandReadImage_PTR;

typedef struct {
    clsMem_PTR          image;
    cl_bool             blockingWrite;
    size_t              origin[3];
    size_t              region[3];
    size_t              inputRowPitch;
    size_t              inputSlicePitch;
    gctCONST_POINTER    ptr;
} clsCommandWriteImage,
* clsCommandWriteImage_PTR;

#if BUILD_OPENCL_12
typedef struct {
    clsMem_PTR          image;
    size_t              origin[3];
    size_t              region[3];
    size_t              inputRowPitch;
    size_t              inputSlicePitch;
    size_t              elementSize;
    gctCONST_POINTER    fillColorPtr;
} clsCommandFillImage,
* clsCommandFillImage_PTR;
#endif

typedef struct {
    clsMem_PTR          srcImage;
    clsMem_PTR          dstImage;
    size_t              srcOrigin[3];
    size_t              dstOrigin[3];
    size_t              region[3];
} clsCommandCopyImage,
* clsCommandCopyImage_PTR;

typedef struct {
    clsMem_PTR          srcImage;
    clsMem_PTR          dstBuffer;
    size_t              srcOrigin[3];
    size_t              region[3];
    size_t              dstOffset;
} clsCommandCopyImageToBuffer,
* clsCommandCopyImageToBuffer_PTR;

typedef struct {
    clsMem_PTR          srcBuffer;
    clsMem_PTR          dstImage;
    size_t              srcOffset;
    size_t              dstOrigin[3];
    size_t              region[3];
} clsCommandCopyBufferToImage,
* clsCommandCopyBufferToImage_PTR;

typedef struct {
    clsMem_PTR          buffer;
    cl_bool             blockingMap;
    cl_map_flags        mapFlags;
    size_t              offset;
    size_t              cb;
    gctPOINTER          mappedPtr;
} clsCommandMapBuffer,
* clsCommandMapBuffer_PTR;

typedef struct {
    clsMem_PTR          image;
    cl_bool             blockingMap;
    cl_map_flags        mapFlags;
    size_t              origin[3];
    size_t              region[3];
    size_t              imageRowPitch;
    size_t              imageSlicePitch;
    gctPOINTER          mappedPtr;
} clsCommandMapImage,
* clsCommandMapImage_PTR;

typedef struct {
    clsMem_PTR          memObj;
    gctPOINTER **       mappedPtr;
} clsCommandUnmapMemObject,
* clsCommandUnmapMemObject_PTR;

typedef struct {
    clsKernel_PTR       kernel;
    clsKernelStates_PTR states;
    gctUINT             numArgs;
    clsArgument_PTR     args;
    gctUINT             workDim;
    size_t              globalWorkOffset[3];
    size_t              globalScale[3];
    size_t              globalWorkSize[3];
    size_t              localWorkSize[3];
} clsCommandNDRangeKernel,
* clsCommandNDRangeKernel_PTR;

typedef struct {
    clsKernel_PTR       kernel;
    clsKernelStates_PTR states;
    gctUINT             numArgs;
    clsArgument_PTR     args;
} clsCommandTask,
* clsCommandTask_PTR;

typedef struct {
#if BUILD_OPENCL_12
    void                (CL_CALLBACK * userFunc)(void *);
#else
    void                (*userFunc)(void *);
#endif
    void *              args;
    size_t              cbArgs;
    gctUINT             numMemObjects;
    const clsMem_PTR *  memList;
    const void **       argsMemLoc;
} clsCommandNativeKernel,
* clsCommandNativeKernel_PTR;

typedef struct {
    gctUINT             numObjects;
    clsMem_PTR *  memObjects;
    void**              objectsDatas;
} clsCommandAcquireGLObjects,
* clsCommandAcquireGLObjects_PTR;

typedef struct {
    gctUINT             numObjects;
    clsMem_PTR *  memObjects;
    void**              objectsDatas;
} clsCommandReleaseGLObjects,
* clsCommandReleaseGLObjects_PTR;

typedef struct _cl_command
{
    cleOBJECT_TYPE          objectType;
    gctUINT                 id;
    gcsATOM_PTR             referenceCount;

    gctUINT64               enqueueNo;
    clsCommand_PTR          next;
    clsCommand_PTR          previous;
    clsCommandQueue_PTR     commandQueue;
    cleCOMMAND_TYPE         type;
    clsEvent_PTR *          outEvent;
    clsEvent_PTR            event;
    gctUINT                 numEventsInWaitList;
    const clsEvent_PTR *    eventWaitList;
    gctUINT                 internalNumEventsInWaitList;
    clsEvent_PTR            internalEventWaitList[2];
    gctINT                  (* handler)(clsCommand_PTR);
    gctSIGNAL               releaseSignal;

    union {
        clsCommandReadBuffer        readBuffer;
        clsCommandReadBufferRect    readBufferRect;
        clsCommandWriteBuffer       writeBuffer;
#if BUILD_OPENCL_12
        clsCommandFillBuffer        fillBuffer;
#endif
        clsCommandWriteBufferRect   writeBufferRect;
        clsCommandCopyBuffer        copyBuffer;
        clsCommandCopyBufferRect    copyBufferRect;
        clsCommandReadImage         readImage;
        clsCommandWriteImage        writeImage;
#if BUILD_OPENCL_12
        clsCommandFillImage         fillImage;
#endif
        clsCommandCopyImage         copyImage;
        clsCommandCopyImageToBuffer copyImageToBuffer;
        clsCommandCopyBufferToImage copyBufferToImage;
        clsCommandMapBuffer         mapBuffer;
        clsCommandMapImage          mapImage;
        clsCommandUnmapMemObject    unmapMemObject;
        clsCommandNDRangeKernel     NDRangeKernel;
        clsCommandTask              task;
        clsCommandNativeKernel      nativeKernel;
        clsCommandAcquireGLObjects  acquireGLObjects;
        clsCommandReleaseGLObjects  releaseGLObjects;
    } u;
}
clsCommand;

/******************************************************************************\
|*                                    Macros                                  *|
\******************************************************************************/

/* Get event command type from command type */
#if BUILD_OPENCL_12
#define clmGET_COMMANDTYPE(type) ( \
    (type) == clvCOMMAND_READ_BUFFER          ? CL_COMMAND_READ_BUFFER          : \
    (type) == clvCOMMAND_READ_BUFFER_RECT     ? CL_COMMAND_READ_BUFFER_RECT     : \
    (type) == clvCOMMAND_WRITE_BUFFER         ? CL_COMMAND_WRITE_BUFFER         : \
    (type) == clvCOMMAND_WRITE_BUFFER_RECT    ? CL_COMMAND_WRITE_BUFFER_RECT    : \
    (type) == clvCOMMAND_COPY_BUFFER          ? CL_COMMAND_COPY_BUFFER          : \
    (type) == clvCOMMAND_COPY_BUFFER_RECT     ? CL_COMMAND_COPY_BUFFER_RECT     : \
    (type) == clvCOMMAND_READ_IMAGE           ? CL_COMMAND_READ_IMAGE           : \
    (type) == clvCOMMAND_WRITE_IMAGE          ? CL_COMMAND_WRITE_IMAGE          : \
    (type) == clvCOMMAND_COPY_IMAGE           ? CL_COMMAND_COPY_IMAGE           : \
    (type) == clvCOMMAND_COPY_IMAGE_TO_BUFFER ? CL_COMMAND_COPY_IMAGE_TO_BUFFER : \
    (type) == clvCOMMAND_COPY_BUFFER_TO_IMAGE ? CL_COMMAND_COPY_BUFFER_TO_IMAGE : \
    (type) == clvCOMMAND_MIGRATE_MEM_OBJECTS  ? CL_COMMAND_MIGRATE_MEM_OBJECTS  : \
    (type) == clvCOMMAND_MAP_BUFFER           ? CL_COMMAND_MAP_BUFFER           : \
    (type) == clvCOMMAND_MAP_IMAGE            ? CL_COMMAND_MAP_IMAGE            : \
    (type) == clvCOMMAND_UNMAP_MEM_OBJECT     ? CL_COMMAND_UNMAP_MEM_OBJECT     : \
    (type) == clvCOMMAND_NDRANGE_KERNEL       ? CL_COMMAND_NDRANGE_KERNEL       : \
    (type) == clvCOMMAND_TASK                 ? CL_COMMAND_TASK                 : \
    (type) == clvCOMMAND_NATIVE_KERNEL        ? CL_COMMAND_NATIVE_KERNEL        : \
    (type) == clvCOMMAND_MARKER               ? CL_COMMAND_MARKER               : \
    (type) == clvCOMMAND_ACQUIRE_GL_OBJECTS   ? CL_COMMAND_ACQUIRE_GL_OBJECTS   : \
    (type) == clvCOMMAND_RELEASE_GL_OBJECTS   ? CL_COMMAND_RELEASE_GL_OBJECTS   : \
    (type) == clvCOMMAND_FILL_IMAGE           ? CL_COMMAND_FILL_IMAGE           : \
    (type) == clvCOMMAND_BARRIER              ? CL_COMMAND_BARRIER              : \
    (type) == clvCOMMAND_FILL_BUFFER          ? CL_COMMAND_FILL_BUFFER          : \
                                                CL_COMMAND_USER )
#else
#define clmGET_COMMANDTYPE(type) ( \
    (type) == clvCOMMAND_READ_BUFFER          ? CL_COMMAND_READ_BUFFER          : \
    (type) == clvCOMMAND_READ_BUFFER_RECT     ? CL_COMMAND_READ_BUFFER_RECT     : \
    (type) == clvCOMMAND_WRITE_BUFFER         ? CL_COMMAND_WRITE_BUFFER         : \
    (type) == clvCOMMAND_WRITE_BUFFER_RECT    ? CL_COMMAND_WRITE_BUFFER_RECT    : \
    (type) == clvCOMMAND_COPY_BUFFER          ? CL_COMMAND_COPY_BUFFER          : \
    (type) == clvCOMMAND_COPY_BUFFER_RECT     ? CL_COMMAND_COPY_BUFFER_RECT     : \
    (type) == clvCOMMAND_READ_IMAGE           ? CL_COMMAND_READ_IMAGE           : \
    (type) == clvCOMMAND_WRITE_IMAGE          ? CL_COMMAND_WRITE_IMAGE          : \
    (type) == clvCOMMAND_COPY_IMAGE           ? CL_COMMAND_COPY_IMAGE           : \
    (type) == clvCOMMAND_COPY_IMAGE_TO_BUFFER ? CL_COMMAND_COPY_IMAGE_TO_BUFFER : \
    (type) == clvCOMMAND_COPY_BUFFER_TO_IMAGE ? CL_COMMAND_COPY_BUFFER_TO_IMAGE : \
    (type) == clvCOMMAND_MAP_BUFFER           ? CL_COMMAND_MAP_BUFFER           : \
    (type) == clvCOMMAND_MAP_IMAGE            ? CL_COMMAND_MAP_IMAGE            : \
    (type) == clvCOMMAND_UNMAP_MEM_OBJECT     ? CL_COMMAND_UNMAP_MEM_OBJECT     : \
    (type) == clvCOMMAND_NDRANGE_KERNEL       ? CL_COMMAND_NDRANGE_KERNEL       : \
    (type) == clvCOMMAND_TASK                 ? CL_COMMAND_TASK                 : \
    (type) == clvCOMMAND_NATIVE_KERNEL        ? CL_COMMAND_NATIVE_KERNEL        : \
    (type) == clvCOMMAND_MARKER               ? CL_COMMAND_MARKER               : \
    (type) == clvCOMMAND_ACQUIRE_GL_OBJECTS   ? CL_COMMAND_ACQUIRE_GL_OBJECTS   : \
    (type) == clvCOMMAND_RELEASE_GL_OBJECTS   ? CL_COMMAND_RELEASE_GL_OBJECTS   : \
                                                CL_COMMAND_USER )
#endif

/* Return true if command requires hardware execution
 * Most commands are executed purely in driver stack
 */
#define clmCOMMAND_EXEC_HARDWARE(type) ( \
    (type) == clvCOMMAND_NDRANGE_KERNEL ? gcvTRUE : \
    (type) == clvCOMMAND_TASK           ? gcvTRUE : gcvFALSE )

/* Return true if command is a sync point
 * No commands enqueued after this command should execute before
 * this command is finished
 */
#define clmCOMMAND_SYNC_POINT(type) ( \
    (type) == clvCOMMAND_MARKER             ? gcvTRUE : \
    (type) == clvCOMMAND_BARRIER            ? gcvTRUE : \
    (type) == clvCOMMAND_WAIT_FOR_EVENTS    ? gcvTRUE : gcvFALSE )


/* Return true if command is a buffer operation */
#if BUILD_OPENCL_12

#define clmCOMMAND_BUFFER_OPERATION(type) ( \
    (type) == clvCOMMAND_READ_BUFFER             ? gcvTRUE : \
    (type) == clvCOMMAND_READ_BUFFER_RECT        ? gcvTRUE : \
    (type) == clvCOMMAND_WRITE_BUFFER            ? gcvTRUE : \
    (type) == clvCOMMAND_FILL_BUFFER             ? gcvTRUE : \
    (type) == clvCOMMAND_WRITE_BUFFER_RECT       ? gcvTRUE : \
    (type) == clvCOMMAND_COPY_BUFFER             ? gcvTRUE : \
    (type) == clvCOMMAND_COPY_BUFFER_RECT        ? gcvTRUE : \
    (type) == clvCOMMAND_READ_IMAGE              ? gcvTRUE : \
    (type) == clvCOMMAND_WRITE_IMAGE             ? gcvTRUE : \
    (type) == clvCOMMAND_FILL_IMAGE              ? gcvTRUE : \
    (type) == clvCOMMAND_COPY_IMAGE              ? gcvTRUE : \
    (type) == clvCOMMAND_COPY_IMAGE_TO_BUFFER    ? gcvTRUE : \
    (type) == clvCOMMAND_COPY_BUFFER_TO_IMAGE    ? gcvTRUE : \
    (type) == clvCOMMAND_MIGRATE_MEM_OBJECTS     ? gcvTRUE : \
    (type) == clvCOMMAND_MAP_BUFFER              ? gcvTRUE : \
    (type) == clvCOMMAND_MAP_IMAGE               ? gcvTRUE : gcvFALSE )

#else

#define clmCOMMAND_BUFFER_OPERATION(type) ( \
    (type) == clvCOMMAND_READ_BUFFER             ? gcvTRUE : \
    (type) == clvCOMMAND_READ_BUFFER_RECT        ? gcvTRUE : \
    (type) == clvCOMMAND_WRITE_BUFFER            ? gcvTRUE : \
    (type) == clvCOMMAND_WRITE_BUFFER_RECT       ? gcvTRUE : \
    (type) == clvCOMMAND_COPY_BUFFER             ? gcvTRUE : \
    (type) == clvCOMMAND_COPY_BUFFER_RECT        ? gcvTRUE : \
    (type) == clvCOMMAND_READ_IMAGE              ? gcvTRUE : \
    (type) == clvCOMMAND_WRITE_IMAGE             ? gcvTRUE : \
    (type) == clvCOMMAND_COPY_IMAGE              ? gcvTRUE : \
    (type) == clvCOMMAND_COPY_IMAGE_TO_BUFFER    ? gcvTRUE : \
    (type) == clvCOMMAND_COPY_BUFFER_TO_IMAGE    ? gcvTRUE : \
    (type) == clvCOMMAND_MAP_BUFFER              ? gcvTRUE : \
    (type) == clvCOMMAND_MAP_IMAGE               ? gcvTRUE : gcvFALSE )

#endif

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/

gctINT
clfAllocateCommand(
    clsCommandQueue_PTR         CommandQueue,
    clsCommand_PTR *            Command
    );

gctINT
clfRetainCommand(
    clsCommand_PTR              Command
    );

gctINT
clfReleaseCommand(
    clsCommand_PTR              Command
    );

gctINT
clfSubmitCommand(
    clsCommandQueue_PTR         CommandQueue,
    clsCommand_PTR              Command,
    gctBOOL                     Blocking
    );

gctTHREAD_RETURN CL_API_CALL
clfCommandQueueWorker(
    gctPOINTER Data
    );

gctINT
clfExecuteCommandMarker(
    clsCommand_PTR  Command
    );

gctINT
clfExecuteCommandBarrier(
    clsCommand_PTR  Command
    );

gctINT
clfExecuteCommandWaitForEvents(
    clsCommand_PTR  Command
    );

void
clfWakeUpAllCommandQueueWorkers(
    clsContext_PTR      Context
    );

gceSTATUS
clfReleaseCommandQueue(
    cl_command_queue    CommandQueue
    );

gceSTATUS
clfRetainCommandQueue(
    cl_command_queue CommandQueue
    );

#ifdef __cplusplus
}
#endif

#endif  /* __gc_cl_command_h_ */
