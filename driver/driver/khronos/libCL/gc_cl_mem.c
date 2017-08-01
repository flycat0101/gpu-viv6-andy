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


#include "gc_cl_precomp.h"
#include <limits.h>

#define __NEXT_MSG_ID__     004068

#if (defined(UNDER_CE) || defined(__QNXNTO__) || defined(LINUX))
#if (0 || defined(ANDROID))
#define MAP_TO_DEVICE 0
#else
#define MAP_TO_DEVICE 1
#endif
#else
#define MAP_TO_DEVICE 0
#endif

#if cldTUNING
void clfSortHaarRects(gctPOINTER Destination, gctPOINTER Source, gctSIZE_T Size)
{
    typedef struct clsHaarRect
    {
        int x;
        int y;
        int width;
        int height;
        float weight;
    }
    cltHaarRect;

    cltHaarRect * src = (cltHaarRect *)Source;
    cltHaarRect * dst = (cltHaarRect *)Destination;
    gctSIZE_T n = Size / sizeof(cltHaarRect);

    while (n > 0)
    {
        gctINT x = 0, i, hSize;

        for (hSize = 1; hSize < (gctINT) n; hSize++)
        {
            if (src[hSize].y == src->y)
            {
                break;
            }
        }

        for (i = 0; i < hSize; i++)
        {
            for (x = 0; (src[x].y == src->y) && (src[x].width == src->width) && (src[x].height == src->height); x += hSize)
            {
                *dst++ = src[x];
                n--;
            }

            src++;
        }

        src += x - hSize;
    }

    gcoOS_MemCopy(Source, Destination, Size);
}
#endif

/*****************************************************************************\
|*                         Local      functions                              *|
\*****************************************************************************/
/* Sync between host and device (internal) memory. */
static void
clfSyncHostMemory(
    gctBOOL     FromHost,
    void       *HostPtr,
    gctSIZE_T   HostStride,
    gctSIZE_T   HostSlice,
    gctSIZE_T  *HostOrig,
    gctPOINTER  DevicePtr,
    gctSIZE_T   DeviceStride,
    gctSIZE_T   DeviceSlice,
    gctSIZE_T  *DeviceOrigin,
    gctSIZE_T  *Area,
    gctSIZE_T   DataSize
    )
{
    gctPOINTER  toRead, toWrite;
    gctSIZE_T   readStride, writeStride;
    gctSIZE_T   readSlice, writeSlice;
    gctUINTPTR_T readBaseSlice, writeBaseSlice;
    gctUINTPTR_T targetHost, targetDevice;
    gctSIZE_T   lineSize;
    gctSIZE_T   j, k;

    /* Get starting location in the host memory. */
    targetHost = gcmPTR2INT(HostPtr) +
        HostOrig[2] * HostSlice +
        HostOrig[1] * HostStride +
        HostOrig[0] * DataSize;

    /* Get starting location in the device memory. */
    targetDevice = gcmPTR2INT(DevicePtr) +
        DeviceOrigin[2] * DeviceSlice +
        DeviceOrigin[1] * DeviceStride +
        DeviceOrigin[0] * DataSize;

    if (FromHost)
    {
        toRead      = gcmINT2PTR(targetHost);
        toWrite     = gcmINT2PTR(targetDevice);
        readStride  = HostStride;
        readSlice   = HostSlice;
        writeStride = DeviceStride;
        writeSlice  = DeviceSlice;
    }
    else
    {
        toRead      = gcmINT2PTR(targetDevice);
        toWrite     = gcmINT2PTR(targetHost);
        readStride  = DeviceStride;
        readSlice   = DeviceSlice;
        writeStride = HostStride;
        writeSlice  = HostSlice;
    }

    /* Copy. */
    if (Area[1] <= 0)   Area[1] = 1;
    if (Area[2] <= 0)   Area[2] = 1;
    lineSize = DataSize * Area[0];
    readBaseSlice   = gcmPTR2INT(toRead);
    writeBaseSlice  = gcmPTR2INT(toWrite);

    for (k = 0; k < Area[2]; k++)
    {
        toRead = gcmINT2PTR(readBaseSlice);
        toWrite = gcmINT2PTR(writeBaseSlice);

        for (j = 0; j < Area[1]; j++)
        {
            gcoOS_MemCopy(toWrite, toRead, lineSize);

            toRead = gcmINT2PTR(gcmPTR2INT(toRead) + readStride);
            toWrite = gcmINT2PTR(gcmPTR2INT(toWrite) + writeStride);
        }

        readBaseSlice += readSlice;
        writeBaseSlice += writeSlice;
    }
}

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/
gctINT
clfRetainMemObject(
    cl_mem MemObj
    )
{
    gctINT status = CL_SUCCESS;

    gcmHEADER_ARG("MemObj=0x%x", MemObj);

    if (MemObj == gcvNULL ||
        MemObj->objectType != clvOBJECT_MEM)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004025: (clfRetainMemObject) invalid MemObj.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, MemObj->referenceCount, gcvNULL));

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfReleaseMemObject(
    cl_mem MemObj
    )
{
    clsMemObjCallback_PTR   memObjCallback, nextMemObjCallback;
    gctINT                  status = CL_SUCCESS;
    gctINT32                oldReference;

    gcmHEADER_ARG("MemObj=0x%x", MemObj);

    if (MemObj == gcvNULL ||
        MemObj->objectType != clvOBJECT_MEM)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004026: (clfReleaseMemObject) invalid MemObj.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL, MemObj->referenceCount, &oldReference));

    if (oldReference == 1)
    {
        gcmASSERT(MemObj->mapCount == 0);

        if (MemObj->waitEvent.event)
        {
            clsBufferWaitEvent_PTR  tempWaitEvent        = &MemObj->waitEvent;
            clsBufferWaitEvent_PTR  tempNextWaitEvent    = gcvNULL;
            /* header node */
            clfReleaseEvent(tempWaitEvent->event);
            tempWaitEvent = tempWaitEvent->next;

            while(tempWaitEvent)
            {
                tempNextWaitEvent = tempWaitEvent->next;
                clfReleaseEvent(tempWaitEvent->event);
                gcoOS_Free(gcvNULL, tempWaitEvent);
                tempWaitEvent = tempNextWaitEvent;
            }
        }

        if (MemObj->type == CL_MEM_OBJECT_BUFFER)
        {
            if (MemObj->u.buffer.createType == CL_BUFFER_CREATE_TYPE_REGION) {
                clsMem_PTR parentBuffer = MemObj->u.buffer.parentBuffer;

                /* Release parent buffer. */
                clfReleaseMemObject(parentBuffer);
            }

            if(MemObj->tmpNode)
            {
                gctUINT32 physical = 0;

                gcsSURF_NODE_GetHardwareAddress(MemObj->tmpNode,&physical,gcvNULL, gcvNULL, gcvNULL);
                gcoCL_FreeMemory((gctPHYS_ADDR)(gctUINTPTR_T) physical,
                    (gctPOINTER)MemObj->tmpNode->logical,
                    MemObj->u.buffer.allocatedSize,
                    MemObj->tmpNode);

            }

            if (MemObj->mapCount == 0)
            {
                if (MemObj->fromGL)
                {
                    /* do nothing, it already release in releaseGLObject */
                    /* gcmVERIFY_OK(gcoCL_UnshareMemory(MemObj->u.buffer.node)); */
                }
                else if (MemObj->u.buffer.createType != CL_BUFFER_CREATE_TYPE_REGION)
                {
                    gcoCL_FreeMemory(MemObj->u.buffer.physical,
                                     MemObj->u.buffer.logical,
                                     MemObj->u.buffer.allocatedSize,
                                     MemObj->u.buffer.node);
                }

#if cldSYNC_MEMORY
                gcmVERIFY_OK(gcoCL_Commit(gcvTRUE));
#endif

                /* Invoke and free callbacks */
                memObjCallback = MemObj->memObjCallback;
                while (memObjCallback != gcvNULL)
                {
                    nextMemObjCallback = memObjCallback->next;
                    memObjCallback->pfnNotify(MemObj, memObjCallback->userData);
                    gcoOS_Free(gcvNULL, memObjCallback);
                    memObjCallback = nextMemObjCallback;
                }

                gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, MemObj->mutex));
                MemObj->mutex = gcvNULL;

                /* Destroy the reference count object */
                gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, MemObj->referenceCount));
                MemObj->referenceCount = gcvNULL;

                gcoOS_Free(gcvNULL, MemObj);

                gcmFOOTER_NO();
                return gcvSTATUS_OK;
            }
        }
        else if (MemObj->type == CL_MEM_OBJECT_IMAGE2D ||
                 MemObj->type == CL_MEM_OBJECT_IMAGE3D ||
                 MemObj->type == CL_MEM_OBJECT_IMAGE2D_ARRAY ||
                 MemObj->type == CL_MEM_OBJECT_IMAGE1D ||
                 MemObj->type == CL_MEM_OBJECT_IMAGE1D_ARRAY ||
                 MemObj->type == CL_MEM_OBJECT_IMAGE1D_BUFFER)
        {
            if (MemObj->mapCount == 0)
            {
                /* Release image object */
                gcoCL_FreeMemory(MemObj->u.image.physical,
                                 MemObj->u.image.logical,
                                 MemObj->u.image.allocatedSize,
                                 MemObj->u.image.node);

                gcoCL_DestroyTexture(MemObj->u.image.texture,
                                     MemObj->u.image.surface);

#if cldSYNC_MEMORY
                gcmVERIFY_OK(gcoCL_Commit(gcvTRUE));
#endif

                MemObj->u.image.texture = gcvNULL;
                MemObj->u.image.surface = gcvNULL;
                MemObj->u.image.surfaceMapped = gcvFALSE;

                /* Invoke and free callbacks */
                memObjCallback = MemObj->memObjCallback;
                while (memObjCallback != gcvNULL)
                {
                    nextMemObjCallback = memObjCallback->next;
                    memObjCallback->pfnNotify(MemObj, memObjCallback->userData);
                    gcoOS_Free(gcvNULL, memObjCallback);
                    memObjCallback = nextMemObjCallback;
                }

                gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, MemObj->mutex));
                MemObj->mutex = gcvNULL;

                /* Destroy the reference count object */
                gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, MemObj->referenceCount));
                MemObj->referenceCount = gcvNULL;

                gcoOS_Free(gcvNULL, MemObj);

                gcmFOOTER_NO();
                return gcvSTATUS_OK;
            }
        }
    }


OnError:
    if (status == CL_INVALID_MEM_OBJECT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004027: (clfReleaseMemObject) internal error.\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

gceSTATUS
clfExecuteHWCopy(
    clsCommand_PTR  Command
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 dstPhysical = 0;
    gctUINT32 srcPhysical = 0;
    gcsSURF_NODE_PTR node = gcvNULL;
    clsMem_PTR  srcBuffer = gcvNULL;
    clsMem_PTR  dstBuffer = gcvNULL;
    size_t size = 0;
    gctBOOL locked0 = gcvFALSE;
    gctBOOL locked1 = gcvFALSE;
    gcsSURF_NODE_PTR node0 = gcvNULL;
    gcsSURF_NODE_PTR node1 = gcvNULL;
    gceENGINE engine = gcvENGINE_RENDER;

    gcmHEADER_ARG("Command=0x%x", Command);

    switch (Command->type)
    {
        case clvCOMMAND_READ_BUFFER:
            {
                clsCommandReadBuffer_PTR readBuffer = gcvNULL;

                readBuffer  = &Command->u.readBuffer;
                srcBuffer = readBuffer->buffer;
                /* wrap dest */
                size = readBuffer->cb;

                /* src address */
                srcPhysical = gcmPTR2INT(srcBuffer->u.buffer.physical) + readBuffer->offset;
                gcoCL_ChooseBltEngine(srcBuffer->u.buffer.node, &engine);

                /* flush and invalid user ptr cache */
                gcoCL_FlushMemory(gcvNULL, readBuffer->ptr, size);

                gcmONERROR(gcoCL_WrapUserMemory(readBuffer->ptr, size, &dstPhysical, &node));

                EVENT_SET_GPU_RUNNING(Command, engine);
                gcoCL_MemWaitAndGetFence(srcBuffer->u.buffer.node, engine, gcvFENCE_TYPE_READ, gcvFENCE_TYPE_WRITE);
            }
            break;

        case clvCOMMAND_WRITE_BUFFER:
            {
                clsCommandWriteBuffer_PTR writeBuffer = gcvNULL;
                gctPOINTER logical = gcvNULL;
                writeBuffer  = &Command->u.writeBuffer;
                dstBuffer = writeBuffer->buffer;
                size = writeBuffer->cb;
                /*src address */
                if(dstBuffer->tmpNode == gcvNULL)
                {
                     gctUINT         bytes = dstBuffer->u.buffer.allocatedSize;
                     gctPHYS_ADDR    physical = 0;

                     gcmONERROR(gcoCL_AllocateMemory(&bytes,
                                        &physical,
                                        &logical,
                                        &dstBuffer->tmpNode,
                                        0));
                }

                gcoCL_MemWaitAndGetFence(dstBuffer->tmpNode, gcvENGINE_CPU, gcvFENCE_TYPE_WRITE, gcvFENCE_TYPE_READ);
                /*Src address */
                gcsSURF_NODE_GetHardwareAddress(dstBuffer->tmpNode,&srcPhysical,gcvNULL, gcvNULL, gcvNULL);
                logical = dstBuffer->tmpNode->logical;
                gcoOS_MemCopy(logical,writeBuffer->ptr,writeBuffer->cb);

                gcmDUMP_BUFFER(gcvNULL,
                               "memory",
                               gcmPTR2INT(srcPhysical),
                               (gctPOINTER)writeBuffer->ptr,
                               0,
                               writeBuffer->cb);

                /* dst address */
                dstPhysical = gcmPTR2INT(dstBuffer->u.buffer.physical) + writeBuffer->offset;
                gcoCL_ChooseBltEngine(dstBuffer->u.buffer.node, &engine);
                EVENT_SET_GPU_RUNNING(Command, engine);

                node0 = dstBuffer->u.buffer.node;
                node1 = dstBuffer->tmpNode;
                gcoCL_MemWaitAndGetFence(node0, engine, gcvFENCE_TYPE_WRITE, gcvFENCE_TYPE_ALL);
                gcoCL_MemWaitAndGetFence(node1, engine, gcvFENCE_TYPE_READ, gcvFENCE_TYPE_READ);

                gcmONERROR(gcsSURF_NODE_Lock(node0, engine, gcvNULL, gcvNULL));
                locked0 = gcvTRUE;
                gcmONERROR(gcsSURF_NODE_Lock(node1, engine, gcvNULL, gcvNULL));
                locked1 = gcvTRUE;
            }
            break;

        case clvCOMMAND_COPY_BUFFER:
            {
                clsCommandCopyBuffer_PTR copyBuffer = gcvNULL;
                gceENGINE engine1, engine2;

                copyBuffer = &Command->u.copyBuffer;
                srcBuffer   = copyBuffer->srcBuffer;
                dstBuffer   = copyBuffer->dstBuffer;
                size = copyBuffer->cb;
                srcPhysical = gcmPTR2INT(srcBuffer->u.buffer.physical) + copyBuffer->srcOffset;
                dstPhysical = gcmPTR2INT(dstBuffer->u.buffer.physical) + copyBuffer->dstOffset;

                gcoCL_ChooseBltEngine(srcBuffer->u.buffer.node, &engine1);
                gcoCL_ChooseBltEngine(dstBuffer->u.buffer.node, &engine2);

                engine = PRIORITY_ENGINE(engine1, engine2);
                EVENT_SET_GPU_RUNNING(Command, engine);

                node0 = srcBuffer->u.buffer.node;
                node1 = dstBuffer->u.buffer.node;
                gcoCL_MemWaitAndGetFence(node0, engine, gcvFENCE_TYPE_READ, gcvFENCE_TYPE_WRITE);
                gcoCL_MemWaitAndGetFence(node1, engine, gcvFENCE_TYPE_WRITE, gcvFENCE_TYPE_WRITE);

                gcmONERROR(gcsSURF_NODE_Lock(node0, engine, gcvNULL, gcvNULL));
                locked0 = gcvTRUE;
                gcmONERROR(gcsSURF_NODE_Lock(node1, engine, gcvNULL, gcvNULL));
                locked1 = gcvTRUE;

            }
            break;

        default:
            gcmONERROR(gcvSTATUS_INVALID_OBJECT);
    }

    gcmONERROR(gcoCL_MemBltCopy(srcPhysical, dstPhysical, size, engine));

OnError:
    if (node)
    {
        gcoCL_FreeMemory(gcvNULL, gcvNULL, 0, node);
    }

    if (locked0)
    {
        gcsSURF_NODE_Unlock(node0,engine);
    }

    if (locked1)
    {
        gcsSURF_NODE_Unlock(node1,engine);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandReadBuffer(
    clsCommand_PTR  Command
    )
{
    clsCommandReadBuffer_PTR readBuffer;
    clsMem_PTR      buffer;
    gctINT          status = CL_SUCCESS;
    gctPOINTER      src;
    gctBOOL         hwCopy = gcvFALSE;
    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);
    clmASSERT(Command->type == clvCOMMAND_READ_BUFFER, CL_INVALID_VALUE);

    readBuffer  = &Command->u.readBuffer;
    buffer      = readBuffer->buffer;

    hwCopy   = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_BLT_ENGINE);

    /* Tmp disable HW copy for read buffer before resolve wrap node issue */
    hwCopy = gcvFALSE;

    /*Try Hardware Copy*/
    if(hwCopy)
    {
        if (gcmIS_ERROR(clfExecuteHWCopy(Command)))
        {
            hwCopy = gcvFALSE;
        }
        else if (readBuffer->blockingRead)
        {
            gcoCL_MemWaitAndGetFence(buffer->u.buffer.node, gcvENGINE_CPU, gcvFENCE_TYPE_READ, gcvFENCE_TYPE_WRITE);
        }
    }

    /* HwCopy failed*/
    if (!hwCopy)
    {
        EVENT_SET_CPU_RUNNING(Command);

        src = (gctPOINTER) (gcmPTR2INT(buffer->u.buffer.logical) + readBuffer->offset);
#if !cldFSL_OPT
        /* CPU to wait fence back*/
        gcoCL_MemWaitAndGetFence(buffer->u.buffer.node, gcvENGINE_CPU, gcvFENCE_TYPE_READ, gcvFENCE_TYPE_WRITE);
#endif

        gcoCL_InvalidateMemoryCache(buffer->u.buffer.node, src, readBuffer->cb);

        gcoOS_MemCopy(readBuffer->ptr, src, readBuffer->cb);

        gcmDUMP(gcvNULL,
                "@[memory.read 0x%08X 0x%08X]",
                gcmPTR2INT(buffer->u.buffer.physical) + readBuffer->offset,
                readBuffer->cb);

        gcmDUMP_BUFFER(gcvNULL,
                       "verify",
                       gcmPTR2INT(buffer->u.buffer.physical) + readBuffer->offset,
                       src,
                       0,
                       readBuffer->cb);
    }

    status = CL_SUCCESS;

    /* Decrease reference count of memory object */
    clfReleaseMemObject(buffer);

OnError:

    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandReadBufferRect(
    clsCommand_PTR  Command
    )
{
    clsCommandReadBufferRect_PTR readBufferRect;
    clsMem_PTR      buffer;
    const size_t *  bufferOrigin;
    const size_t *  hostOrigin;
    const size_t *  region;
    size_t          bufferRowPitch;
    size_t          bufferSlicePitch;
    size_t          hostRowPitch;
    size_t          hostSlicePitch;
    gctPOINTER      ptr;
    size_t          src, dst, srcFirstByte, dstFirstByte;
    size_t          row, slice;
    gctINT          status;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_READ_BUFFER_RECT, CL_INVALID_VALUE);
#if !cldFSL_OPT
    /* Flush GPU cache and wait for GPU finish before CPU operations */
    gcmONERROR(gcoCL_Flush(gcvTRUE));
#endif
    EVENT_SET_CPU_RUNNING(Command);

    readBufferRect  = &Command->u.readBufferRect;
    buffer          = readBufferRect->buffer;
    bufferOrigin    = readBufferRect->bufferOrigin;
    hostOrigin      = readBufferRect->hostOrigin;
    region          = readBufferRect->region;
    bufferRowPitch  = readBufferRect->bufferRowPitch;
    bufferSlicePitch = readBufferRect->bufferSlicePitch;
    hostRowPitch    = readBufferRect->hostRowPitch;
    hostSlicePitch  = readBufferRect->hostSlicePitch;
    ptr             = readBufferRect->ptr;

    srcFirstByte = gcmPTR2INT(buffer->u.buffer.logical) +
                   bufferOrigin[0] +
                   bufferOrigin[1] * bufferRowPitch +
                   bufferOrigin[2] * bufferSlicePitch;

    dstFirstByte = gcmPTR2INT(ptr) +
                   hostOrigin[0] +
                   hostOrigin[1] * hostRowPitch +
                   hostOrigin[2] * hostSlicePitch;

    gcoCL_InvalidateMemoryCache(buffer->u.buffer.node, buffer->u.buffer.logical, buffer->u.buffer.allocatedSize);

    /* Copy row by row from buffer to host */
    for (slice=0; slice<region[2]; slice++)
    {
        for (row=0; row<region[1]; row++)
        {
            src = srcFirstByte + row*bufferRowPitch + slice*bufferSlicePitch;
            dst = dstFirstByte + row*hostRowPitch + slice*hostSlicePitch;
            gcoOS_MemCopy((gctPOINTER)dst, (gctPOINTER)src, region[0]);

            gcmDUMP(gcvNULL,
                    "@[memory.read 0x%08X 0x%08X]",
                    gcmPTR2INT( buffer->u.buffer.physical) + (src - gcmPTR2INT( buffer->u.buffer.logical)),
                    region[0]);
        }
    }

    status = CL_SUCCESS;

    /* Decrease reference count of memory object */
    clfReleaseMemObject(buffer);

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandWriteBuffer(
    clsCommand_PTR  Command
    )
{
    clsCommandWriteBuffer_PTR writeBuffer;
    clsMem_PTR      buffer;
    size_t          cb;
    size_t          offset;
    gctCONST_POINTER ptr;
    gctINT          status = CL_SUCCESS;
    gctPOINTER      logicalAddress;
    gctBOOL         hwCopy = gcvFALSE;
    gctBOOL          fenceBack = gcvFALSE;
    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_WRITE_BUFFER, CL_INVALID_VALUE);

    writeBuffer = &Command->u.writeBuffer;
    buffer      = writeBuffer->buffer;
    hwCopy   = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_BLT_ENGINE);


    fenceBack = gcoCL_MemIsFenceBack(buffer->u.buffer.node, gcvENGINE_RENDER, gcvFENCE_TYPE_ALL)  &&
                    gcoCL_MemIsFenceBack(buffer->u.buffer.node, gcvENGINE_BLT, gcvFENCE_TYPE_ALL);

   /* HwCopy will first copy to a tmpNode and do blit ,
     If dest Mem 's fence is back, we choose cpu copy to  reduce the bandwidth
   */
    if(fenceBack)
    {
        hwCopy = gcvFALSE;
    }

     /*Try Hardware Copy*/
    if (hwCopy)
    {
        if (gcmIS_ERROR(clfExecuteHWCopy(Command)))
        {
            hwCopy = gcvFALSE;
        }
    }

    if(!hwCopy)
    {
#if !cldFSL_OPT
    /*Waiting fence back before write */
     gcoCL_MemWaitAndGetFence(buffer->u.buffer.node, gcvENGINE_CPU, gcvFENCE_TYPE_WRITE, gcvFENCE_TYPE_ALL);
#endif
    EVENT_SET_CPU_RUNNING(Command);

    cb          = writeBuffer->cb;
    ptr         = writeBuffer->ptr;
    offset      = writeBuffer->offset;
    logicalAddress = (gctPOINTER) (gcmPTR2INT(buffer->u.buffer.logical) + offset);

    gcoOS_MemCopy(logicalAddress, ptr, cb);

    gcoCL_FlushMemory(buffer->u.buffer.node, buffer->u.buffer.logical, buffer->u.buffer.allocatedSize);

    gcmDUMP_BUFFER(gcvNULL,
                   "memory",
                   gcmPTR2INT( buffer->u.buffer.physical),
                   buffer->u.buffer.logical,
                   0,
                   cb);
    }
    status = CL_SUCCESS;

    /* Decrease reference count of memory object */
    clfReleaseMemObject(buffer);

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandFillBuffer(
    clsCommand_PTR  Command
    )
{
    clsCommandFillBuffer_PTR fillBuffer;
    clsMem_PTR      buffer;
    size_t          size;
    size_t          offset;
    size_t          pattern_size;
    gctCONST_POINTER pattern;
    gctINT          status;
    gctPOINTER      logicalAddress;
    gctSIZE_T       operAddress;
    size_t          i;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_FILL_BUFFER, CL_INVALID_VALUE);

#if !cldFSL_OPT
    /* Flush GPU cache and wait for GPU finish before CPU operations */
    gcmONERROR(gcoCL_Flush(gcvTRUE));
#endif
    EVENT_SET_CPU_RUNNING(Command);

    fillBuffer   = &Command->u.fillBuffer;
    buffer       = fillBuffer->buffer;
    size         = fillBuffer->size;
    pattern      = fillBuffer->pattern;
    offset       = fillBuffer->offset;
    pattern_size = fillBuffer->pattern_size;

    logicalAddress = (gctPOINTER) (gcmPTR2INT(buffer->u.buffer.logical) + offset);
    operAddress = (gctSIZE_T)logicalAddress;

    for ( i = 0; i < size; i+=pattern_size )
    {
        gcoOS_MemCopy((gctPOINTER) operAddress, pattern, pattern_size);
        operAddress += (size_t)pattern_size;
    }

    gcoCL_FlushMemory(buffer->u.buffer.node, buffer->u.buffer.logical, buffer->u.buffer.allocatedSize);

    gcmDUMP_BUFFER(gcvNULL,
                   "memory",
                   gcmPTR2INT( buffer->u.buffer.physical),
                   buffer->u.buffer.logical,
                   0,
                   buffer->u.buffer.allocatedSize);

    status = CL_SUCCESS;

    /* Decrease reference count of memory object */
    clfReleaseMemObject(buffer);

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandWriteBufferRect(
    clsCommand_PTR  Command
    )
{
    clsCommandWriteBufferRect_PTR writeBufferRect;
    clsMem_PTR      buffer;
    const size_t *  bufferOrigin;
    const size_t *  hostOrigin;
    const size_t *  region;
    size_t          bufferRowPitch;
    size_t          bufferSlicePitch;
    size_t          hostRowPitch;
    size_t          hostSlicePitch;
    gctCONST_POINTER ptr;
    size_t          src, dst, srcFirstByte, dstFirstByte;
    size_t          row, slice;
    gctINT          status;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_WRITE_BUFFER_RECT, CL_INVALID_VALUE);
#if !cldFSL_OPT
    /* Flush GPU cache and wait for GPU finish before CPU operations */
    gcmONERROR(gcoCL_Flush(gcvTRUE));
#endif
    EVENT_SET_CPU_RUNNING(Command);

    writeBufferRect = &Command->u.writeBufferRect;
    buffer          = writeBufferRect->buffer;
    bufferOrigin    = writeBufferRect->bufferOrigin;
    hostOrigin      = writeBufferRect->hostOrigin;
    region          = writeBufferRect->region;
    bufferRowPitch  = writeBufferRect->bufferRowPitch;
    bufferSlicePitch = writeBufferRect->bufferSlicePitch;
    hostRowPitch    = writeBufferRect->hostRowPitch;
    hostSlicePitch  = writeBufferRect->hostSlicePitch;
    ptr             = writeBufferRect->ptr;

    srcFirstByte = gcmPTR2INT(ptr) +
                   hostOrigin[0] +
                   hostOrigin[1] * hostRowPitch +
                   hostOrigin[2] * hostSlicePitch;

    dstFirstByte = gcmPTR2INT(buffer->u.buffer.logical) +
                   bufferOrigin[0] +
                   bufferOrigin[1] * bufferRowPitch +
                   bufferOrigin[2] * bufferSlicePitch;

    /* Copy row by row from host to buffer */
    for (slice=0; slice<region[2]; slice++)
    {
        for (row=0; row<region[1]; row++)
        {
            src = srcFirstByte + row*hostRowPitch + slice*hostSlicePitch;
            dst = dstFirstByte + row*bufferRowPitch + slice*bufferSlicePitch;
            gcoOS_MemCopy((gctPOINTER)dst, (gctPOINTER)src, region[0]);

            gcmDUMP_BUFFER(gcvNULL,
                           "memory",
                           gcmPTR2INT( buffer->u.buffer.physical),
                           buffer->u.buffer.logical,
                           dst - gcmPTR2INT(buffer->u.buffer.logical),
                           region[0]);
        }
    }

    gcoCL_FlushMemory(buffer->u.buffer.node, buffer->u.buffer.logical, buffer->u.buffer.allocatedSize);

    status = CL_SUCCESS;

    /* Decrease reference count of memory object */
    clfReleaseMemObject(buffer);

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandCopyBuffer(
    clsCommand_PTR  Command
    )
{
    clsCommandCopyBuffer_PTR copyBuffer;
    clsMem_PTR      srcBuffer = gcvNULL;
    clsMem_PTR      dstBuffer = gcvNULL;
    size_t          srcOffset;
    size_t          dstOffset;
    size_t          cb;
    gctINT          status = gcvSTATUS_OK;
    size_t          src, dst;
    gctBOOL         hwCopy = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_BLT_ENGINE);
    gctBOOL         block = gcvFALSE;
    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_COPY_BUFFER, CL_INVALID_VALUE);

    copyBuffer  = &Command->u.copyBuffer;
    srcBuffer   = copyBuffer->srcBuffer;
    dstBuffer   = copyBuffer->dstBuffer;

    if (hwCopy)
    {
        if (gcmIS_ERROR(clfExecuteHWCopy(Command)))
        {
            hwCopy = gcvFALSE;
        }
        else if (block)
        {
            /* only need wait one buffer */
            gcoCL_MemWaitAndGetFence(srcBuffer->u.buffer.node, gcvENGINE_CPU, gcvFNECE_TYPE_INVALID, gcvFENCE_TYPE_READ);
        }
    }

    if (!hwCopy)
    {
#if !cldFSL_OPT
        /* Flush GPU cache and wait for GPU finish before CPU operations */
        gcoCL_MemWaitAndGetFence(srcBuffer->u.buffer.node, gcvENGINE_CPU, gcvFNECE_TYPE_INVALID, gcvFENCE_TYPE_WRITE);
        gcoCL_MemWaitAndGetFence(dstBuffer->u.buffer.node, gcvENGINE_CPU, gcvFNECE_TYPE_INVALID, gcvFENCE_TYPE_ALL);
#endif

        EVENT_SET_CPU_RUNNING(Command);

        srcOffset   = copyBuffer->srcOffset;
        dstOffset   = copyBuffer->dstOffset;
        cb          = copyBuffer->cb;

        src = gcmPTR2INT(srcBuffer->u.buffer.logical) + srcOffset;
        dst = gcmPTR2INT(dstBuffer->u.buffer.logical) + dstOffset;

        gcoCL_InvalidateMemoryCache(srcBuffer->u.buffer.node, srcBuffer->u.buffer.logical, srcBuffer->u.buffer.allocatedSize);

        gcoOS_MemCopy((gctPOINTER)dst, (gctPOINTER)src, cb);

        gcoCL_FlushMemory(dstBuffer->u.buffer.node, dstBuffer->u.buffer.logical, dstBuffer->u.buffer.allocatedSize);

        gcmDUMP_BUFFER(gcvNULL,
                       "memory",
                       gcmPTR2INT( dstBuffer->u.buffer.physical),
                       dstBuffer->u.buffer.logical,
                       0,
                       cb);
    }

    status = CL_SUCCESS;

OnError:
    clfReleaseMemObject(srcBuffer);
    clfReleaseMemObject(dstBuffer);

    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandCopyBufferRect(
    clsCommand_PTR  Command
    )
{
    clsCommandCopyBufferRect_PTR copyBufferRect;
    clsMem_PTR      srcBuffer;
    clsMem_PTR      dstBuffer;
    const size_t *  srcOrigin;
    const size_t *  dstOrigin;
    const size_t *  region;
    size_t          srcRowPitch;
    size_t          srcSlicePitch;
    size_t          dstRowPitch;
    size_t          dstSlicePitch;
    size_t          src, dst, srcFirstByte, dstFirstByte;
    size_t          row, slice;
    gctINT          status;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_COPY_BUFFER_RECT, CL_INVALID_VALUE);
#if !cldFSL_OPT
    /* Flush GPU cache and wait for GPU finish before CPU operations */
    gcmONERROR(gcoCL_Flush(gcvTRUE));
#endif
    EVENT_SET_CPU_RUNNING(Command);

    copyBufferRect  = &Command->u.copyBufferRect;
    srcBuffer       = copyBufferRect->srcBuffer;
    dstBuffer       = copyBufferRect->dstBuffer;
    srcOrigin       = copyBufferRect->srcOrigin;
    dstOrigin       = copyBufferRect->dstOrigin;
    region          = copyBufferRect->region;
    srcRowPitch     = copyBufferRect->srcRowPitch;
    srcSlicePitch   = copyBufferRect->srcSlicePitch;
    dstRowPitch     = copyBufferRect->dstRowPitch;
    dstSlicePitch   = copyBufferRect->dstSlicePitch;

    srcFirstByte = gcmPTR2INT(srcBuffer->u.buffer.logical) +
                   srcOrigin[0] +
                   srcOrigin[1] * srcRowPitch +
                   srcOrigin[2] * srcSlicePitch;

    dstFirstByte = gcmPTR2INT(dstBuffer->u.buffer.logical) +
                   dstOrigin[0] +
                   dstOrigin[1] * dstRowPitch +
                   dstOrigin[2] * dstSlicePitch;

    gcoCL_InvalidateMemoryCache(srcBuffer->u.buffer.node, srcBuffer->u.buffer.logical, srcBuffer->u.buffer.allocatedSize);

    /* Copy row by row from src buffer to destination buffer */
    for (slice=0; slice<region[2]; slice++)
    {
        for (row=0; row<region[1]; row++)
        {
            src = srcFirstByte + row*srcRowPitch + slice*srcSlicePitch;
            dst = dstFirstByte + row*dstRowPitch + slice*dstSlicePitch;
            gcoOS_MemCopy((gctPOINTER)dst, (gctPOINTER)src, region[0]);

            gcmDUMP_BUFFER(gcvNULL,
                           "memory",
                           gcmPTR2INT( dstBuffer->u.buffer.physical),
                           dstBuffer->u.buffer.logical,
                           dst - gcmPTR2INT(dstBuffer->u.buffer.logical),
                           region[0]);
        }
    }

    gcoCL_FlushMemory(dstBuffer->u.buffer.node, dstBuffer->u.buffer.logical, dstBuffer->u.buffer.allocatedSize);

    status = CL_SUCCESS;

    clfReleaseMemObject(srcBuffer);
    clfReleaseMemObject(dstBuffer);

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfReadImage(
    clsCommand_PTR  Command
    )
{
    clsCommandReadImage_PTR readImage;
    gctINT          status;
    clsMem_PTR      image;
    gctUINT         elementSize;
    gctUINT         lineSize;
    gctUINT8 *      srcLine;
    gctUINT8 *      dstLine;
    gctUINT         y,z;
    gctUINT         xoff,yoff,zoff;
    gctUINT         width,height,depth;
    gctUINT         srcRowPitch, dstRowPitch;
    gctUINT         srcSlicePitch, dstSlicePitch;
    gctUINT8        *srcSliceBegin, *dstSliceBegin;


    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_READ_IMAGE, CL_INVALID_VALUE);

#if !cldFSL_OPT
    /* Flush GPU cache and wait for GPU finish before CPU operations */
    gcmONERROR(gcoCL_Flush(gcvTRUE));
#endif
    readImage   = &Command->u.readImage;
    image       = readImage->image;
    xoff        = readImage->origin[0];
    yoff        = readImage->origin[1];
    zoff        = readImage->origin[2];
    width       = readImage->region[0];
    height      = readImage->region[1];
    depth       = readImage->region[2];

    srcRowPitch  = image->u.image.textureStride;
    dstRowPitch  = readImage->rowPitch;
    srcSlicePitch = image->u.image.textureSlicePitch;
    dstSlicePitch = readImage->slicePitch;
    elementSize = image->u.image.elementSize;
    lineSize = width * elementSize;


    gcoCL_FlushSurface(image->u.image.surface);

    /* set to the slice begin */
    srcLine = (gctUINT8_PTR) image->u.image.textureLogical + (srcSlicePitch * zoff);
    srcSliceBegin = (gctUINT8_PTR) image->u.image.textureLogical + (srcSlicePitch * zoff) + (yoff*srcRowPitch) + (xoff*elementSize);
    dstSliceBegin = (gctUINT8_PTR) readImage->ptr;


    for (z = 0; z < depth; z++)
    {
        srcLine = srcSliceBegin;
        dstLine = dstSliceBegin;
        for (y = 0; y < height; y++)
        {
            /* set to the pixel begin */
            gcoOS_MemCopy(dstLine, srcLine, lineSize);
            /* set to next line begin */
            srcLine += srcRowPitch;
            dstLine += dstRowPitch;
        }
        /* set to next slice begin */
        srcSliceBegin += srcSlicePitch;
        dstSliceBegin += dstSlicePitch;
    }

    clfReleaseMemObject(image);
    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandReadImage(
    clsCommand_PTR  Command
    )
{
    gctINT ret;

    EVENT_SET_CPU_RUNNING(Command);

    ret = clfReadImage(Command);

    return ret;
}

gctINT
clfWriteImage(
    clsCommand_PTR  Command
    )
{
    clsCommandWriteImage_PTR writeImage;
    gctINT           status;
    clsMem_PTR       image;
    gctUINT          elementSize;
    gctUINT          lineSize;
    const gctUINT8 * srcLine;
    gctUINT8 *       dstLine;
    gctUINT          y,z;
    gctUINT          xoff,yoff,zoff;
    gctUINT          width,height,depth;
    gctUINT          srcRowPitch, dstRowPitch;
    gctUINT          srcSlicePitch, dstSlicePitch;
    gctUINT8         *srcSliceBegin, *dstSliceBegin;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_WRITE_IMAGE, CL_INVALID_VALUE);
#if !cldFSL_OPT
    /* Flush GPU cache and wait for GPU finish before CPU operations */
    gcmONERROR(gcoCL_Flush(gcvTRUE));
#endif
    writeImage     = &Command->u.writeImage;
    image          = writeImage->image;
    xoff           = writeImage->origin[0];
    yoff           = writeImage->origin[1];
    zoff           = writeImage->origin[2];
    width          = writeImage->region[0];
    height         = writeImage->region[1];
    depth          = writeImage->region[2];
    srcRowPitch    = writeImage->inputRowPitch;
    dstRowPitch    = image->u.image.textureStride;
    srcSlicePitch  = writeImage->inputSlicePitch;
    dstSlicePitch  = image->u.image.textureSlicePitch;
    elementSize    = image->u.image.elementSize;
    lineSize       = elementSize * width;

    /* set to the slice begin */
    srcSliceBegin = (gctUINT8_PTR) writeImage->ptr;
    dstSliceBegin = (gctUINT8_PTR) image->u.image.textureLogical + (dstSlicePitch * zoff) + (dstRowPitch * yoff) + xoff*elementSize;

    for (z = 0; z < depth; z++)
    {
        /* set to the line begin */
        srcLine = srcSliceBegin;
        dstLine = dstSliceBegin;
        for (y = 0; y < height; y++)
        {
            /* set to the pixel begin */
            gcoOS_MemCopy(dstLine, srcLine, lineSize);
            gcmDUMP_BUFFER(gcvNULL,
                            "texture",
                            gcmPTR2INT(image->u.image.texturePhysical),
                            image->u.image.textureLogical,
                            gcmPTR2INT(dstLine) - gcmPTR2INT(image->u.image.textureLogical),
                            lineSize);

            srcLine += srcRowPitch;
            dstLine += dstRowPitch;
        }
        srcSliceBegin += srcSlicePitch;
        dstSliceBegin += dstSlicePitch;
    }

    gcoCL_FlushSurface(image->u.image.surface);

    clfReleaseMemObject(image);
    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandWriteImage(
    clsCommand_PTR  Command
    )
{
    gctINT ret;

    EVENT_SET_CPU_RUNNING(Command);

    ret = clfWriteImage(Command);

    return ret;
}

gctINT
clfExecuteCommandFillImage(
    clsCommand_PTR  Command
    )
{
    clsCommandFillImage_PTR fillImage;
    gctINT           status;
    clsMem_PTR       image;
    size_t           stride;
    size_t           slice;
    gctCONST_POINTER ptr;
    gctPOINTER       textureLogical;
    gctUINT          elementSize;
    gctUINT8 *       line;
    gctUINT          x, y, z;
    gctUINT          xoff,yoff,zoff,width,height,depth;
    gctUINT8         *srcSliceBegin, *srcLineBegin;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_FILL_IMAGE, CL_INVALID_VALUE);

#if !cldFSL_OPT
    /* Flush GPU cache and wait for GPU finish before CPU operations */
    gcmONERROR(gcoCL_Flush(gcvTRUE));
#endif
    EVENT_SET_CPU_RUNNING(Command);

    fillImage     = &Command->u.fillImage;
    image          = fillImage->image;
    ptr            = fillImage->fillColorPtr;
    stride         = image->u.image.textureStride;
    slice          = image->u.image.textureSlicePitch;
    elementSize    = image->u.image.elementSize;
    textureLogical = image->u.image.textureLogical;
    xoff = fillImage->origin[0];
    yoff = fillImage->origin[1];
    zoff = fillImage->origin[2];
    width = fillImage->region[0];
    height = fillImage->region[1];
    depth = fillImage->region[2];

    /* set to the slice begin */
    srcSliceBegin = (gctUINT8_PTR) textureLogical + (zoff*slice) + (yoff*stride) + (xoff*elementSize);
    for (z = 0; z < depth; z++)
    {
        /* set to the line begin */
        srcLineBegin = srcSliceBegin;
        for (y = 0; y < height; y++)
        {
            /* set to the pixel begin */
            line = srcLineBegin;
            for (x = 0; x < width; x++)
            {
                gcoOS_MemCopy(line, ptr, elementSize);

                gcmDUMP_BUFFER(gcvNULL,
                               "texture",
                               gcmPTR2INT( image->u.image.texturePhysical),
                               image->u.image.textureLogical,
                               gcmPTR2INT(line) - gcmPTR2INT(image->u.image.textureLogical),
                               elementSize);
                line += elementSize;
            }
            /* set to next line begin */
            srcLineBegin += stride;
        }
        /* set to next slice begin */
        srcSliceBegin += slice;
    }

    gcoCL_FlushSurface(image->u.image.surface);
    gcoOS_Free(gcvNULL, (gctPOINTER)fillImage->fillColorPtr);

    status = CL_SUCCESS;

    /* Decrease reference count of memory object */
    clfReleaseMemObject(image);

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandCopyImage(
    clsCommand_PTR  Command
    )
{
    clsCommandCopyImage_PTR copyImage;
    gctINT          status;
    clsMem_PTR      srcImage;
    clsMem_PTR      dstImage;
    gctUINT         elementSize;
    gctUINT         lineSize;
    gctUINT8 *      srcLine;
    gctUINT8 *      dstLine;
    gctUINT         y;
    gctUINT         z;
    gctUINT         sxoff,syoff,szoff,dxoff,dyoff,dzoff;
    gctUINT         width,height,depth;
    gctUINT         srcRowPitch, dstRowPitch;
    gctUINT         srcSlicePitch, dstSlicePitch;
    gctUINT8        *srcSliceBegin, *dstSliceBegin;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_COPY_IMAGE, CL_INVALID_VALUE);

    /* Flush GPU cache and wait for GPU finish before CPU operations */
    gcmONERROR(gcoCL_Flush(gcvTRUE));

    EVENT_SET_CPU_RUNNING(Command);

    copyImage   = &Command->u.copyImage;
    srcImage    = copyImage->srcImage;
    dstImage    = copyImage->dstImage;
    sxoff       = copyImage->srcOrigin[0];
    syoff       = copyImage->srcOrigin[1];
    szoff       = copyImage->srcOrigin[2];
    dxoff       = copyImage->dstOrigin[0];
    dyoff       = copyImage->dstOrigin[1];
    dzoff       = copyImage->dstOrigin[2];
    width       = copyImage->region[0];
    height      = copyImage->region[1];
    depth       = copyImage->region[2];
    elementSize = srcImage->u.image.elementSize;
    clmASSERT(srcImage->u.image.elementSize == dstImage->u.image.elementSize, CL_IMAGE_FORMAT_MISMATCH);


    gcoCL_FlushSurface(srcImage->u.image.surface);

    /* step 1, write to swap buffer */
    /* step 1.1, prepare data */
    lineSize     = elementSize * width;

    srcSlicePitch = srcImage->u.image.textureSlicePitch;
    dstSlicePitch = dstImage->u.image.textureSlicePitch;
    srcRowPitch   = srcImage->u.image.textureStride;
    dstRowPitch   = dstImage->u.image.textureStride;

    srcSliceBegin = (gctUINT8_PTR) srcImage->u.image.textureLogical + (srcSlicePitch * szoff) + (srcRowPitch*syoff) + (sxoff*elementSize);
    dstSliceBegin = (gctUINT8_PTR) dstImage->u.image.textureLogical + (dstSlicePitch * dzoff) + (dstRowPitch*dyoff) + (dxoff*elementSize);

    for (z = 0; z < depth; z++)
    {
        srcLine = srcSliceBegin;
        dstLine = dstSliceBegin;
        for (y = 0; y < height; y++)
        {
            gcoOS_MemCopy(dstLine, srcLine, lineSize);
            srcLine += srcRowPitch;
            dstLine += dstRowPitch;
        }
        srcSliceBegin += srcSlicePitch;
        dstSliceBegin += dstSlicePitch;
    }

    gcoCL_FlushSurface(dstImage->u.image.surface);
    status = CL_SUCCESS;

    /* Decrease reference count of memory object */
    clfReleaseMemObject(srcImage);
    clfReleaseMemObject(dstImage);

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandCopyImageToBuffer(
    clsCommand_PTR  Command
    )
{
    clsCommandCopyImageToBuffer_PTR copyImageToBuffer;
    gctINT          status;
    clsMem_PTR      srcImage;
    clsMem_PTR      dstBuffer;
    size_t          width;
    size_t          height;
    size_t          depth;
    size_t          srcStride;
    size_t          dstOffset;
    gctPOINTER      textureLogical;
    gctUINT         elementSize;
    gctUINT         lineSize;
    gctUINT8 *      srcLine;
    gctUINT8 *      dstLine;
    gctUINT8 *      imgSlice;
    gctUINT         slice;
    gctUINT         y, z;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_COPY_IMAGE_TO_BUFFER, CL_INVALID_VALUE);
#if !cldFSL_OPT
    /* Flush GPU cache and wait for GPU finish before CPU operations */
    gcmONERROR(gcoCL_Flush(gcvTRUE));
#endif
    EVENT_SET_CPU_RUNNING(Command);

    copyImageToBuffer = &Command->u.copyImageToBuffer;
    srcImage          = copyImageToBuffer->srcImage;
    dstBuffer         = copyImageToBuffer->dstBuffer;
    width             = copyImageToBuffer->region[0];
    height            = copyImageToBuffer->region[1];
    depth             = copyImageToBuffer->region[2] > 0 ?
                        copyImageToBuffer->region[2] :
                        1;
    dstOffset         = copyImageToBuffer->dstOffset;
    srcStride         = srcImage->u.image.textureStride;
    elementSize       = srcImage->u.image.elementSize;
    textureLogical    = srcImage->u.image.textureLogical;

    /* TODO - Need to handle 3D image. */

    dstLine = (gctUINT8_PTR) dstBuffer->u.buffer.logical + dstOffset;
    srcLine = (gctUINT8_PTR) textureLogical +
              srcStride * copyImageToBuffer->srcOrigin[1] +
              elementSize * copyImageToBuffer->srcOrigin[0];
    lineSize = elementSize * width;
    imgSlice = srcLine;
    slice    = srcImage->u.image.textureSlicePitch;

    gcoCL_FlushSurface(srcImage->u.image.surface);

    for (z = 0; z < depth; z++)
    {
        srcLine = imgSlice;
        for (y = 0; y < height; y++)
        {
            gcoOS_MemCopy(dstLine, srcLine, lineSize);

            gcmDUMP_BUFFER(gcvNULL,
                           "memory",
                           gcmPTR2INT(dstBuffer->u.buffer.physical),
                           dstBuffer->u.buffer.logical,
                           gcmPTR2INT(dstLine) - gcmPTR2INT(dstBuffer->u.buffer.logical),
                           lineSize);

            srcLine += srcStride;
            dstLine += lineSize;
        }
        imgSlice += slice;
    }

    gcoCL_FlushMemory(dstBuffer->u.buffer.node, dstBuffer->u.buffer.logical, dstBuffer->u.buffer.allocatedSize);

    status = CL_SUCCESS;

    /* Decrease reference count of memory object */
    clfReleaseMemObject(srcImage);
    clfReleaseMemObject(dstBuffer);

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandCopyBufferToImage(
    clsCommand_PTR  Command
    )
{
    clsCommandCopyBufferToImage_PTR copyBufferToImage;
    gctINT           status;
    clsMem_PTR       srcBuffer;
    clsMem_PTR       dstImage;
    size_t           width;
    size_t           height;
    size_t           depth;
    size_t           dstStride;
    size_t           srcOffset;
    gctPOINTER       textureLogical;
    gctUINT          elementSize;
    gctUINT          lineSize;
    const gctUINT8 * srcLine;
    gctUINT8 *       dstLine;
    gctUINT8 *       dstSlice;
    gctUINT          slice;
    gctUINT          y, z;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_COPY_BUFFER_TO_IMAGE, CL_INVALID_VALUE);
#if !cldFSL_OPT
    /* Flush GPU cache and wait for GPU finish before CPU operations */
    gcmONERROR(gcoCL_Flush(gcvTRUE));
#endif
    EVENT_SET_CPU_RUNNING(Command);

    copyBufferToImage = &Command->u.copyBufferToImage;
    srcBuffer         = copyBufferToImage->srcBuffer;
    dstImage          = copyBufferToImage->dstImage;
    width             = copyBufferToImage->region[0];
    height            = copyBufferToImage->region[1] > 0 ?
                        copyBufferToImage->region[1] :
                        1;
    depth             = copyBufferToImage->region[2] > 0 ?
                        copyBufferToImage->region[2] :
                        1;
    srcOffset         = copyBufferToImage->srcOffset;
    dstStride         = dstImage->u.image.textureStride;
    elementSize       = dstImage->u.image.elementSize;
    textureLogical    = dstImage->u.image.textureLogical;

    /* TODO - Need to handle 3D image. */

    srcLine = (gctUINT8_PTR) srcBuffer->u.buffer.logical + srcOffset;
    dstSlice = (gctUINT8_PTR) textureLogical +
              dstStride * copyBufferToImage->dstOrigin[1] +
              elementSize * copyBufferToImage->dstOrigin[0];
    lineSize = elementSize * width;
    slice    = dstImage->u.image.textureSlicePitch;

    gcoCL_FlushMemory(srcBuffer->u.buffer.node, srcBuffer->u.buffer.logical, srcBuffer->u.buffer.allocatedSize);

    for (z = 0; z < depth; z++)
    {
        dstLine = dstSlice;
        for (y = 0; y < height; y++)
        {
            gcoOS_MemCopy(dstLine, srcLine, lineSize);

            gcmDUMP_BUFFER(gcvNULL,
                           "texture",
                           gcmPTR2INT( dstImage->u.image.texturePhysical),
                           dstImage->u.image.textureLogical,
                           gcmPTR2INT(dstLine) - gcmPTR2INT(dstImage->u.image.textureLogical),
                           lineSize);

            srcLine += lineSize;
            dstLine += dstStride;
        }
        dstSlice += slice;
    }

    gcoCL_FlushSurface(dstImage->u.image.surface);

    status = CL_SUCCESS;

     /* Decrease reference count of memory object */
    clfReleaseMemObject(dstImage);
    clfReleaseMemObject(srcBuffer);

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandMigrateMemObjects(
    clsCommand_PTR  Command
    )
{
    gctINT          status;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_MIGRATE_MEM_OBJECTS, CL_INVALID_VALUE);

    EVENT_SET_CPU_RUNNING(Command);

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandMapBuffer(
    clsCommand_PTR  Command
    )
{
    clsCommandMapBuffer_PTR mapBuffer;
    clsMem_PTR          buffer;
    cl_map_flags        mapFlags;
    size_t              cb;
    gctPOINTER          mappedPtr;
    gctINT              status;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_MAP_BUFFER, CL_INVALID_VALUE);
#if !cldFSL_OPT
    /* Flush GPU cache and wait for GPU finish before CPU operations */
    gcmONERROR(gcoCL_Flush(gcvTRUE));
#endif
    EVENT_SET_CPU_RUNNING(Command);

    mapBuffer   = &Command->u.mapBuffer;
    buffer      = mapBuffer->buffer;
    mapFlags    = mapBuffer->mapFlags;
    cb          = mapBuffer->cb;
    mappedPtr   = mapBuffer->mappedPtr;
    buffer->mapFlag  = mapFlags;

    clfRetainMemObject(buffer);

    gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, buffer->mutex, gcvINFINITE));
    buffer->mapCount++;
    gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, buffer->mutex));

    if (mapFlags & CL_MAP_WRITE)
    {
        gcoCL_FlushMemory(buffer->u.buffer.node, mappedPtr, cb);
    }

    if (mapFlags & CL_MAP_READ)
    {
        gcoCL_InvalidateMemoryCache(buffer->u.buffer.node, mappedPtr, cb);
    }

    /* Anyway, we need to sync the content of the memory when mapping is called. */
    if ((buffer->flags & CL_MEM_USE_HOST_PTR) &&
        (buffer->host != gcvNULL))
    {
        gctSIZE_T   orig[3], area[3];
        orig[0] = orig[1] = orig[2] = 0;
        area[0] = buffer->u.buffer.size;
        area[1] = 1;
        area[2] = 1;
        clfSyncHostMemory(gcvFALSE,
            buffer->host,
            0,
            0,
            orig,
            buffer->u.buffer.logical,
            0,
            0,
            orig,
            area,
            1
            );
    }

    /* Sync is needed as well when 1d image buffer is associated. */
    if (buffer->u.buffer.image != gcvNULL)
    {
        gctSIZE_T   orig[3], area[3];
        orig[0] = orig[1] = orig[2] = 0;
        area[0] = buffer->u.buffer.size;
        area[1] = 1;
        area[2] = 1;
        clfSyncHostMemory(gcvTRUE,
            buffer->u.buffer.image->u.image.textureLogical,
            0,
            0,
            orig,
            buffer->u.buffer.logical,
            0,
            0,
            orig,
            area,
            1
            );
    }

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandMapImage(
    clsCommand_PTR  Command
    )
{
    clsCommandMapImage_PTR mapImage;
    gctINT          status;
    clsMem_PTR      image;
    cl_map_flags    mapFlags;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_MAP_IMAGE, CL_INVALID_VALUE);
#if !cldFSL_OPT
    /* Flush GPU cache and wait for GPU finish before CPU operations */
    gcmONERROR(gcoCL_Flush(gcvTRUE));
#endif
    EVENT_SET_CPU_RUNNING(Command);

    mapImage    = &Command->u.mapImage;
    image       = mapImage->image;
    mapFlags    = mapImage->mapFlags;
    image->mapFlag = mapFlags;
    clfRetainMemObject(image);

    gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, image->mutex, gcvINFINITE));

    /* Lock memory. */
    if (image->mapCount == 0)
    {
        gcmONERROR(gcoCL_LockSurface(image->u.image.surface,
                                     (gctUINT32 *) &image->u.image.texturePhysical,
                                     &image->u.image.textureLogical));
    }

    image->mapCount++;
    gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, image->mutex));

    /* TODO - Need to handle 3D image. */

    if (mapFlags & CL_MAP_WRITE)
    {
        gcoCL_FlushSurface(image->u.image.surface);
    }

    if (mapFlags & CL_MAP_READ)
    {
        /* TODO - Need to invalidate surface node. */
    }

    /* Anyway, we need to sync the content of the memory when mapping is called. */
    if ((image->flags & CL_MEM_USE_HOST_PTR) &&
        (image->host != gcvNULL))
    {
        gctSIZE_T   orig[3], area[3];
        orig[0] = orig[1] = orig[2] = 0;
        area[0] = image->u.image.width;
        area[1] = image->type == CL_MEM_OBJECT_IMAGE1D_ARRAY ?
                    image->u.image.arraySize :
                    image->u.image.height;
        area[2] = image->type == CL_MEM_OBJECT_IMAGE2D_ARRAY ?
                    image->u.image.arraySize :
                    image->u.image.depth;

        clfSyncHostMemory(gcvFALSE,
            image->host,
            image->u.image.rowPitch,
            image->u.image.slicePitch,
            orig,
            image->u.image.textureLogical,
            image->u.image.textureStride,
            image->u.image.textureSlicePitch,
            orig,
            area,
            image->u.image.elementSize
            );
    }

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandUnmapMemObject(
    clsCommand_PTR  Command
    )
{
    clsCommandUnmapMemObject_PTR unmapMemObject;
    clsMem_PTR          memObj = gcvNULL;
    /*gctPOINTER **       mappedPtr;*/
    gctINT              status;
    gctBOOL             acquired = gcvFALSE;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_UNMAP_MEM_OBJECT, CL_INVALID_VALUE);

    /* Flush GPU cache and wait for GPU finish before CPU operations */
    gcmONERROR(gcoCL_Flush(gcvTRUE));

    EVENT_SET_CPU_RUNNING(Command);

    unmapMemObject  = &Command->u.unmapMemObject;
    memObj          = unmapMemObject->memObj;
    /*mappedPtr     = unmapMemObject->mappedPtr;*/

    gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, memObj->mutex, gcvINFINITE));
    acquired = gcvTRUE;
    memObj->mapCount--;

    if (memObj->mapCount == 0)
    {
        if (memObj->type == CL_MEM_OBJECT_BUFFER)
        {
            /* Sync from host if necessary. */
            if ((memObj->flags & CL_MEM_USE_HOST_PTR) &&
                (memObj->host != gcvNULL) &&
                (memObj->mapFlag & CL_MAP_WRITE))
            {
                /* Host writting done, Sync host to device. */
                gctSIZE_T   orig[3], area[3];
                orig[0] = orig[1] = orig[2] = 0;
                area[0] = memObj->u.buffer.size;
                area[1] = 1;
                area[2] = 1;

                clfSyncHostMemory(gcvTRUE,
                    memObj->host,
                    0,
                    0,
                    orig,
                    memObj->u.buffer.logical,
                    0,
                    0,
                    orig,
                    area,
                    1
                    );
            }

            /* Update the associated 1d image buffer object. */
            if (memObj->u.buffer.image != gcvNULL)
            {
                /* Buffer writting done, Sync to the associated image. */
                gctSIZE_T   orig[3], area[3];
                orig[0] = orig[1] = orig[2] = 0;
                area[0] = memObj->u.buffer.size;
                area[1] = 1;
                area[2] = 1;

                clfSyncHostMemory(gcvFALSE,
                    memObj->u.buffer.image->u.image.textureLogical,
                    0,
                    0,
                    orig,
                    memObj->u.buffer.logical,
                    0,
                    0,
                    orig,
                    area,
                    1
                    );
            }
           /* TODO - if mapped for write, need to flush memory. */
            /*gcoCL_FlushMemory(memObj->u.buffer.node, memObj->u.buffer.logical, cb);*/
        }
        else if (memObj->type == CL_MEM_OBJECT_IMAGE2D ||
                 memObj->type == CL_MEM_OBJECT_IMAGE3D ||
                 memObj->type == CL_MEM_OBJECT_IMAGE1D ||
                 memObj->type == CL_MEM_OBJECT_IMAGE1D_ARRAY ||
                 memObj->type == CL_MEM_OBJECT_IMAGE1D_BUFFER ||
                 memObj->type == CL_MEM_OBJECT_IMAGE2D_ARRAY)
        {
            /* Sync from host if necessary. */
            if ((memObj->flags & CL_MEM_USE_HOST_PTR) &&
                (memObj->host != gcvNULL) &&
                (memObj->mapFlag & CL_MAP_WRITE))
            {
                /* Host writting done, Sync host to device. */
                gctSIZE_T   orig[3], area[3];
                orig[0] = orig[1] = orig[2] = 0;
                area[0] = memObj->u.image.width;
                area[1] = memObj->type == CL_MEM_OBJECT_IMAGE1D_ARRAY ?
                            memObj->u.image.arraySize :
                            memObj->u.image.height;
                area[2] = memObj->type == CL_MEM_OBJECT_IMAGE2D_ARRAY ?
                            memObj->u.image.arraySize :
                            memObj->u.image.depth;

                clfSyncHostMemory(gcvTRUE,
                    memObj->host,
                    memObj->u.image.rowPitch,
                    memObj->u.image.slicePitch,
                    orig,
                    memObj->u.image.textureLogical,
                    memObj->u.image.textureStride,
                    memObj->u.image.textureSlicePitch,
                    orig,
                    area,
                    memObj->u.image.elementSize
                    );
            }

            /* Unlock memory */
            gcmONERROR(gcoCL_UnlockSurface(memObj->u.image.surface, memObj->u.image.textureLogical));
        }
    }

    gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, memObj->mutex));
    acquired = gcvFALSE;

    clfReleaseMemObject(memObj);

    status = CL_SUCCESS;

OnError:
    if (acquired)
    {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, memObj->mutex));
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfImageFormat2GcFormat(
    const clsImageFormat *  Format,
    gctSIZE_T *             ElementSize,
    gceSURF_FORMAT *        InternalFormat,
    gctSIZE_T *             ChanelCount
    )
{
    gctSIZE_T           channelCount;
    gctSIZE_T           channelSize;
    gceSURF_FORMAT      internalFormat;
    gctINT              status;

    /* Given an image format, return number of bytes per pixel and internal format. */

    switch (Format->image_channel_order)
    {
    case CL_R:
        switch (Format->image_channel_data_type)
        {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            internalFormat = gcvSURF_R8;
            break;

        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_UNSIGNED_INT16:
            internalFormat = gcvSURF_R16;
            break;

        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
            internalFormat = gcvSURF_R32;
            break;

        case CL_HALF_FLOAT:
            internalFormat = gcvSURF_R16F;
            break;

        case CL_FLOAT:
            internalFormat = gcvSURF_R32F;
            break;

        default:
            status = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
            goto OnError;
        }
        channelCount = 1;
        break;

    case CL_Rx:
        switch (Format->image_channel_data_type)
        {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            internalFormat = gcvSURF_X8R8;
            break;

        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_UNSIGNED_INT16:
            internalFormat = gcvSURF_X16R16;
            break;

        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
            internalFormat = gcvSURF_X32R32;
            break;

        case CL_HALF_FLOAT:
            internalFormat = gcvSURF_X16R16F;
            break;

        case CL_FLOAT:
            internalFormat = gcvSURF_X32R32F;
            break;

        default:
            status = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
            goto OnError;
        }
        channelCount = 1;
        break;

    case CL_A:
        switch (Format->image_channel_data_type)
        {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            internalFormat = gcvSURF_A8;
            break;

        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_UNSIGNED_INT16:
            internalFormat = gcvSURF_A16;
            break;

        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
            internalFormat = gcvSURF_A32;
            break;

        case CL_HALF_FLOAT:
            internalFormat = gcvSURF_A16F;
            break;

        case CL_FLOAT:
            internalFormat = gcvSURF_A32F;
            break;

        default:
            status = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
            goto OnError;
        }
        channelCount = 1;
        break;

    case CL_RG:
        switch (Format->image_channel_data_type)
        {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            internalFormat = gcvSURF_G8R8;
            break;

        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_UNSIGNED_INT16:
            internalFormat = gcvSURF_G16R16;
            break;

        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
            internalFormat = gcvSURF_G32R32;
            break;

        case CL_HALF_FLOAT:
            internalFormat = gcvSURF_G16R16F;
            break;

        case CL_FLOAT:
            internalFormat = gcvSURF_G32R32F;
            break;

        default:
            status = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
            goto OnError;
        }
        channelCount = 2;
        break;

    case CL_RGx:
        switch (Format->image_channel_data_type)
        {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            internalFormat = gcvSURF_X8G8R8;
            break;

        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_UNSIGNED_INT16:
            internalFormat = gcvSURF_X16G16R16;
            break;

        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
            internalFormat = gcvSURF_X32G32R32;
            break;

        case CL_HALF_FLOAT:
            internalFormat = gcvSURF_X16G16R16F;
            break;

        case CL_FLOAT:
            internalFormat = gcvSURF_X32G32R32F;
            break;

        default:
            status = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
            goto OnError;
        }
        channelCount = 2;
        break;

    case CL_RA:
        switch (Format->image_channel_data_type)
        {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            internalFormat = gcvSURF_A8R8;
            break;

        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_UNSIGNED_INT16:
            internalFormat = gcvSURF_A16R16;
            break;

        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
            internalFormat = gcvSURF_A32R32;
            break;

        case CL_HALF_FLOAT:
            internalFormat = gcvSURF_A16R16F;
            break;

        case CL_FLOAT:
            internalFormat = gcvSURF_A32R32F;
            break;

        default:
            status = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
            goto OnError;
        }
        channelCount = 2;
        break;

    case CL_RGB:
        switch (Format->image_channel_data_type)
        {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            internalFormat = gcvSURF_B8G8R8;
            break;

        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_UNSIGNED_INT16:
            internalFormat = gcvSURF_B16G16R16;
            break;

        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
            internalFormat = gcvSURF_B32G32R32;
            break;

        case CL_HALF_FLOAT:
            internalFormat = gcvSURF_B16G16R16F;
            break;

        case CL_FLOAT:
            internalFormat = gcvSURF_B32G32R32F;
            break;

        case CL_UNORM_SHORT_565:
            internalFormat = gcvSURF_B5G6R5;
            break;

        case CL_UNORM_SHORT_555:
            internalFormat = gcvSURF_X1B5G5R5;
            break;

        default:
            status = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
            goto OnError;
        }
        channelCount = 3;
        break;

    case CL_RGBx:
        switch (Format->image_channel_data_type)
        {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            internalFormat = gcvSURF_X8B8G8R8;
            break;

        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_UNSIGNED_INT16:
            internalFormat = gcvSURF_X16B16G16R16;
            break;

        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
            internalFormat = gcvSURF_X32B32G32R32;
            break;

        case CL_HALF_FLOAT:
            internalFormat = gcvSURF_X16B16G16R16F;
            break;

        case CL_FLOAT:
            internalFormat = gcvSURF_X32B32G32R32F;
            break;

        case CL_UNORM_SHORT_555:
            internalFormat = gcvSURF_X1B5G5R5;
            break;

        case CL_UNORM_INT_101010:
            internalFormat = gcvSURF_X2B10G10R10;
            break;

        default:
            status = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
            goto OnError;
        }
        channelCount = 3;
        break;

    case CL_RGBA:
        switch (Format->image_channel_data_type)
        {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            internalFormat = gcvSURF_A8B8G8R8;
            break;

        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_UNSIGNED_INT16:
            internalFormat = gcvSURF_A16B16G16R16;
            break;

        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
            internalFormat = gcvSURF_A32B32G32R32;
            break;

        case CL_HALF_FLOAT:
            internalFormat = gcvSURF_A16B16G16R16F;
            break;

        case CL_FLOAT:
            internalFormat = gcvSURF_A32B32G32R32F;
            break;

        default:
            status = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
            goto OnError;
        }
        channelCount = 4;
        break;

        /*TODO: for BGRA, walkround is used if not directly support by HW,
         * thus it has the same surf format as RGBA.
         * Otherwise, BGRA has its own internal formats gcvSURF_ARGBXXXX
        */
    case CL_BGRA:
        switch (Format->image_channel_data_type)
        {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            internalFormat = gcvSURF_A8R8G8B8;
            break;

        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_UNSIGNED_INT16:
            internalFormat = gcvSURF_A16R16G16B16;
            break;

        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
            internalFormat = gcvSURF_A32R32G32B32;
            break;

        case CL_HALF_FLOAT:
            internalFormat = gcvSURF_A16B16G16R16F;
            break;

        case CL_FLOAT:
            internalFormat = gcvSURF_A32B32G32R32F;
            break;

        default:
            status = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
            goto OnError;
        }
        channelCount = 4;
        break;

    case CL_INTENSITY:
        /* Fall through - use swizzle. */
    case CL_LUMINANCE:
        switch (Format->image_channel_data_type)
        {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            internalFormat = gcvSURF_L8;
            break;

        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_UNSIGNED_INT16:
            internalFormat = gcvSURF_L16;
            break;

        case CL_HALF_FLOAT:
            internalFormat = gcvSURF_L16F;
            break;

        case CL_FLOAT:
            internalFormat = gcvSURF_L32F;
            break;

        default:
            status = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
            goto OnError;
        }
        channelCount = 1;
        break;

    default:
        status = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
        goto OnError;
    }

    switch (Format->image_channel_data_type)
    {
    case CL_SNORM_INT8:
    case CL_UNORM_INT8:
    case CL_SIGNED_INT8:
    case CL_UNSIGNED_INT8:
        channelSize = 1;
        break;

    case CL_UNORM_SHORT_565:
    case CL_UNORM_SHORT_555:
        /* Override channelCount. */
        channelCount = 1;
        /* Fall through. */
    case CL_SNORM_INT16:
    case CL_UNORM_INT16:
    case CL_SIGNED_INT16:
    case CL_UNSIGNED_INT16:
    case CL_HALF_FLOAT:
        channelSize = 2;
        break;

    case CL_UNORM_INT_101010:   /* 10-bit unsigned integer */
        /* Override channelCount. */
        channelCount = 1;
        /* Fall through. */
    case CL_SIGNED_INT32:
    case CL_UNSIGNED_INT32:
    case CL_FLOAT:
        channelSize = 4;
        break;

    default:
        status = CL_INVALID_VALUE;
        goto OnError;
    }

    if (ElementSize)
    {
        *ElementSize = channelCount * channelSize;
    }

    if (InternalFormat)
    {
        *InternalFormat = internalFormat;
    }

    if (ChanelCount)
    {
        *ChanelCount = channelCount;
    }

    return CL_SUCCESS;

OnError:
    return status;
}

#define NORMALIZE( v, max ) ( v < 0 ? 0 : ( v > 1.f ? max : clfRound2Even( v * max ) ) )
#define NORMALIZE_UNROUNDED( v, max ) ( v < 0 ? 0 : ( v > 1.f ? max :  v * max ) )
#define NORMALIZE_SIGNED( v, min, max ) ( v  < -1.0f ? min : ( v > 1.f ? max : clfRound2Even( v * max ) ) )
#define NORMALIZE_SIGNED_UNROUNDED( v, min, max ) ( v  < -1.0f ? min : ( v > 1.f ? max : v * max ) )
#define CONVERT_INT( v, min, max, max_val)  ( v < min ? min : ( v > max ? max_val : clfRound2Even( v ) ) )
#define CONVERT_UINT( v, max, max_val)  ( v < 0 ? 0 : ( v > max ? max_val : clfRound2Even( v ) ) )
#define MAKE_HEX_FLOAT(x,y,z)  ((float)ldexp( (float)(y), z))
#define MAKE_HEX_DOUBLE(x,y,z) ldexp( (double)(y), z)
#define MAKE_HEX_LONG(x,y,z)   ((long double) ldexp( (long double)(y), z))

#define SATURATE( v, min, max ) ( v < min ? min : ( v > max ? max : v ) )

#define CLBGRA2GRGA(TYPE) \
    if (imageFormat->image_channel_order == CL_BGRA) \
    { \
        TYPE tmpData = ptr[0]; \
        ptr[0] = ptr[2]; \
        ptr[2] = tmpData; \
    }


gctINT clfPackImagePixeli( cl_int *srcVector, const cl_image_format *imageFormat, void *outData )
{
    size_t channelCount;
    unsigned int i;
    clfImageFormat2GcFormat(imageFormat, gcvNULL, gcvNULL, &channelCount);

    switch( imageFormat->image_channel_data_type )
    {
        case CL_SIGNED_INT8:
        {
            char *ptr = (char *)outData;
            for( i = 0; i < channelCount; i++ )
                ptr[ i ] = (char)SATURATE( srcVector[ i ], -128, 127 );
            CLBGRA2GRGA(char);
            break;
        }
        case CL_SIGNED_INT16:
        {
            short *ptr = (short *)outData;
            for( i = 0; i < channelCount; i++ )
                ptr[ i ] = (short)SATURATE( srcVector[ i ], -32768, 32767 );
            CLBGRA2GRGA(short);
            break;
        }
        case CL_SIGNED_INT32:
        {
            int *ptr = (int *)outData;
            for( i = 0; i < channelCount; i++ )
                ptr[ i ] = (int)srcVector[ i ];
            CLBGRA2GRGA(int);
            break;
        }
        default:
            break;
    }
    return CL_SUCCESS;
}

gctINT clfPackImagePixelui( cl_uint *srcVector, const cl_image_format *imageFormat, void *outData )
{
    size_t channelCount;
    unsigned int i;
    clfImageFormat2GcFormat(imageFormat, gcvNULL, gcvNULL, &channelCount);

    switch( imageFormat->image_channel_data_type )
    {
        case CL_UNSIGNED_INT8:
        {
            unsigned char *ptr = (unsigned char *)outData;
            for( i = 0; i < channelCount; i++ )
                ptr[ i ] = (unsigned char)SATURATE( srcVector[ i ], 0, 255 );
            CLBGRA2GRGA(unsigned char);
            break;
        }
        case CL_UNSIGNED_INT16:
        {
            unsigned short *ptr = (unsigned short *)outData;
            for( i = 0; i < channelCount; i++ )
                ptr[ i ] = (unsigned short)SATURATE( srcVector[ i ], 0, 65535 );
            CLBGRA2GRGA(unsigned short);
            break;
        }
        case CL_UNSIGNED_INT32:
        {
            unsigned int *ptr = (unsigned int *)outData;
            for( i = 0; i < channelCount; i++ )
                ptr[ i ] = (unsigned int)srcVector[ i ];
            CLBGRA2GRGA(unsigned int);
            break;
        }
        default:
            break;
    }
    return CL_SUCCESS;
}

/*fix build error under linux, will link fabsf error*/
#if defined(LINUX) && !defined(ANDROID)
#define fabsf(v) \
        (v > 0.0 ? v: -v)
#endif


static int clfRound2Even( float v )
{
    /* clamp overflow */
    if( v >= - (float) INT_MIN )
        return INT_MAX;
    if( v <= (float) INT_MIN )
        return INT_MIN;

    /* round fractional values to integer value */
    if( fabsf(v) < MAKE_HEX_FLOAT(0x1.0p23f, 0x1L, 23) )
    {
        float magic[2] = { MAKE_HEX_FLOAT(0x1.0p23f, 0x1L, 23), MAKE_HEX_FLOAT(-0x1.0p23f, -0x1L, 23) };
        float magicVal = magic[ v < 0.0f ];
        v += magicVal;
        v -= magicVal;
    }

    return (int) v;
}

static cl_ushort clfFloat2halfRTE( float f )
    {
    union{ float f; cl_uint u; } u = {f};
    cl_uint sign = (u.u >> 16) & 0x8000;
    float x = fabsf(f);

    /* Nan */
    if( x != x )
    {
        u.u >>= (24-11);
        u.u &= 0x7fff;
        u.u |= 0x0200;      /*silence the NaN */
        return (cl_ushort)(u.u | sign);
                }

    /* overflow */
    if( x >= MAKE_HEX_FLOAT(0x1.ffep15f, 0x1ffeL, 3) )
        return (cl_ushort)(0x7c00 | sign);

    /* underflow */
    if( x <= MAKE_HEX_FLOAT(0x1.0p-25f, 0x1L, -25) )
        return (cl_ushort)sign;    /* The halfway case can return 0x0001 or 0. 0 is even. */

    /* very small */
    if( x < MAKE_HEX_FLOAT(0x1.8p-24f, 0x18L, -28) )
        return (cl_ushort)(sign | 1);

    /* half denormal */
    if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
    {
        u.f = x * MAKE_HEX_FLOAT(0x1.0p-125f, 0x1L, -125);
        return (cl_ushort)(sign | u.u);
        }

    u.f *= MAKE_HEX_FLOAT(0x1.0p13f, 0x1L, 13);
    u.u &= 0x7f800000;
    x += u.f;
    u.f = x - u.f;
    u.f *= MAKE_HEX_FLOAT(0x1.0p-112f, 0x1L, -112);

    return (cl_ushort)((u.u >> (24-11)) | sign);
    }

gctINT clfPackImagePixelf( cl_float *srcVector, const cl_image_format *imageFormat, void *outData )
{
    size_t channelCount;
    unsigned int i;
    clfImageFormat2GcFormat(imageFormat, gcvNULL, gcvNULL, &channelCount);
    switch( imageFormat->image_channel_data_type )
    {
        case CL_HALF_FLOAT:
        {
            cl_ushort *ptr = (cl_ushort *)outData;

            for( i = 0; i < channelCount; i++ )
                        ptr[ i ] = clfFloat2halfRTE( srcVector[ i ] );
            CLBGRA2GRGA(cl_ushort);
            break;
        }

        case CL_FLOAT:
        {
            cl_float *ptr = (cl_float *)outData;
            for( i = 0; i < channelCount; i++ )
                ptr[ i ] = srcVector[ i ];
            CLBGRA2GRGA(cl_float);
            break;
        }

        case CL_SNORM_INT8:
        {
            cl_char *ptr = (cl_char *)outData;
            for( i = 0; i < channelCount; i++ )
                ptr[ i ] = (char)NORMALIZE_SIGNED( srcVector[ i ], -127.0f, 127.f );
            CLBGRA2GRGA(cl_char);
            break;
        }
        case CL_SNORM_INT16:
        {
            cl_short *ptr = (cl_short *)outData;
            for( i = 0; i < channelCount; i++ )
                ptr[ i ] = (short)NORMALIZE_SIGNED( srcVector[ i ], -32767.f, 32767.f  );
            CLBGRA2GRGA(cl_short);
            break;
        }
        case CL_UNORM_INT8:
        {
            cl_uchar *ptr = (cl_uchar *)outData;
            for( i = 0; i < channelCount; i++ )
                ptr[ i ] = (unsigned char)NORMALIZE( srcVector[ i ], 255.f );
            CLBGRA2GRGA(cl_uchar);
            break;
        }
        case CL_UNORM_INT16:
        {
            cl_ushort *ptr = (cl_ushort *)outData;
            for( i = 0; i < channelCount; i++ )
                ptr[ i ] = (unsigned short)NORMALIZE( srcVector[ i ], 65535.f );
            CLBGRA2GRGA(cl_ushort);
            break;
        }
        case CL_UNORM_SHORT_555:
        {
            cl_ushort *ptr = (cl_ushort *)outData;
            ptr[ 0 ] = ( ( (unsigned short)NORMALIZE( srcVector[ 0 ], 31.f ) & 31 ) << 10 ) |
            ( ( (unsigned short)NORMALIZE( srcVector[ 1 ], 31.f ) & 31 ) << 5 ) |
            ( ( (unsigned short)NORMALIZE( srcVector[ 2 ], 31.f ) & 31 ) << 0 );
            break;
        }
        case CL_UNORM_SHORT_565:
        {
            cl_ushort *ptr = (cl_ushort *)outData;
            ptr[ 0 ] = ( ( (unsigned short)NORMALIZE( srcVector[ 0 ], 31.f ) & 31 ) << 11 ) |
            ( ( (unsigned short)NORMALIZE( srcVector[ 1 ], 63.f ) & 63 ) << 5 ) |
            ( ( (unsigned short)NORMALIZE( srcVector[ 2 ], 31.f ) & 31 ) << 0 );
            break;
        }
        case CL_UNORM_INT_101010:
        {
            cl_uint *ptr = (cl_uint *)outData;
            ptr[ 0 ] = ( ( (unsigned int)NORMALIZE( srcVector[ 0 ], 1023.f ) & 1023 ) << 20 ) |
            ( ( (unsigned int)NORMALIZE( srcVector[ 1 ], 1023.f ) & 1023 ) << 10 ) |
            ( ( (unsigned int)NORMALIZE( srcVector[ 2 ], 1023.f ) & 1023 ) << 0 );
            break;
        }
        case CL_SIGNED_INT8:
        {
            cl_char *ptr = (cl_char *)outData;
            for( i = 0; i < channelCount; i++ )
                ptr[ i ] = (char)CONVERT_INT( srcVector[ i ], -127.0f, 127.f, 127 );
            CLBGRA2GRGA(cl_char);
            break;
        }
        case CL_SIGNED_INT16:
        {
            cl_short *ptr = (cl_short *)outData;
            for( i = 0; i < channelCount; i++ )
                ptr[ i ] = (short)CONVERT_INT( srcVector[ i ], -32767.f, 32767.f, 32767  );
            CLBGRA2GRGA(cl_short);
            break;
        }
        case CL_SIGNED_INT32:
        {
            cl_int *ptr = (cl_int *)outData;
            for( i = 0; i < channelCount; i++ )
                ptr[ i ] = (int)CONVERT_INT( srcVector[ i ], MAKE_HEX_FLOAT( -0x1.0p31f, -1, 31), MAKE_HEX_FLOAT( 0x1.fffffep30f, 0x1fffffe, 30-23), CL_INT_MAX  );
            CLBGRA2GRGA(cl_int);
            break;
        }
        case CL_UNSIGNED_INT8:
        {
            cl_uchar *ptr = (cl_uchar *)outData;
            for( i = 0; i < channelCount; i++ )
                ptr[ i ] = (cl_uchar)CONVERT_UINT( srcVector[ i ], 255.f, CL_UCHAR_MAX );
            CLBGRA2GRGA(cl_uchar);
            break;
        }
        case CL_UNSIGNED_INT16:
        {
            cl_ushort *ptr = (cl_ushort *)outData;
            for( i = 0; i < channelCount; i++ )
                ptr[ i ] = (cl_ushort)CONVERT_UINT( srcVector[ i ], 32767.f, CL_USHRT_MAX );
            CLBGRA2GRGA(cl_ushort);
            break;
        }
        case CL_UNSIGNED_INT32:
        {
            cl_uint *ptr = (cl_uint *)outData;
            for( i = 0; i < channelCount; i++ )
                ptr[ i ] = (cl_uint)CONVERT_UINT( srcVector[ i ], MAKE_HEX_FLOAT( 0x1.fffffep31f, 0x1fffffe, 31-23), CL_UINT_MAX  );
            CLBGRA2GRGA(cl_uint);
            break;
        }

        default:
            break;
    }
    return CL_SUCCESS;
}

gctINT
clfNewBuffer(
    clsContext_PTR  Context,
    clsMem_PTR *    Buffer
    )
{
    clsMem_PTR   buffer = gcvNULL;
    gctPOINTER   pointer = gcvNULL;
    gctINT       status;

    gcmHEADER_ARG("Context=0x%x ",
                   Context);

    /* Allocate buffer object. */
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsMem), &pointer),
               CL_OUT_OF_HOST_MEMORY);

    gcoOS_ZeroMemory(pointer, sizeof(clsMem));

    /* Mem info. */
    buffer                  = (clsMem_PTR) pointer;
    buffer->dispatch        = Context->dispatch;
    buffer->objectType      = clvOBJECT_MEM;
    buffer->context         = Context;
    buffer->type            = CL_MEM_OBJECT_BUFFER;
    buffer->host            = gcvNULL;
    buffer->memObjCallback  = gcvNULL;
    buffer->mapCount        = 0;
    buffer->fromGL          = gcvFALSE;
    buffer->flags           = 0;

    /* Buffer specific info. */
    buffer->u.buffer.size           = 0;
    buffer->u.buffer.node           = gcvNULL;
    buffer->u.buffer.parentBuffer   = gcvNULL;
    buffer->u.buffer.createType     = 0;
    buffer->u.buffer.wrapped        = gcvFALSE;
    buffer->u.buffer.image          = gcvNULL;

    /* Create a reference count object and set it to 1. */
    clmONERROR(gcoOS_AtomConstruct(gcvNULL, &buffer->referenceCount),
               CL_OUT_OF_HOST_MEMORY);

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, buffer->referenceCount, gcvNULL));

    clmONERROR(gcoOS_AtomIncrement(gcvNULL, clgGlobalId, (gctINT*)&buffer->id), CL_INVALID_VALUE);

    /* Create modify mutex lock. */
    clmONERROR(gcoOS_CreateMutex(gcvNULL,
                                 &buffer->mutex),
               CL_OUT_OF_HOST_MEMORY);

    *Buffer = buffer;

    gcmFOOTER_ARG("%d buffer=0x%x", CL_SUCCESS, buffer);
    return CL_SUCCESS;

OnError:
    if (buffer != gcvNULL)
    {
        gcoOS_Free(gcvNULL, buffer);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfNewImage(
    clsContext_PTR  Context,
    clsMem_PTR *    Image
    )
{
    clsMem_PTR   image   = gcvNULL;
    gctPOINTER   pointer = gcvNULL;
    gctINT       status;

    gcmHEADER_ARG("Context=0x%x ",
                   Context);

    /* Allocate image object. */
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsMem), &pointer),
               CL_OUT_OF_HOST_MEMORY);

    gcoOS_ZeroMemory(pointer, sizeof(clsMem));

    /* Mem info. */
    image                  = (clsMem_PTR) pointer;
    image->dispatch        = Context->dispatch;
    image->objectType      = clvOBJECT_MEM;
    image->context         = Context;
    image->type            = CL_MEM_OBJECT_IMAGE2D;
    image->host            = gcvNULL;
    image->memObjCallback  = gcvNULL;
    image->mapCount        = 0;
    image->fromGL          = gcvFALSE;

    /* Image specific info. */
    image->u.image.width        = 0;
    image->u.image.height       = 0;
    image->u.image.depth        = 0;
    image->u.image.rowPitch     = 0;
    image->u.image.slicePitch   = 0;
    image->u.image.elementSize  = 4;
    image->u.image.size         = 0;
    image->u.image.texture      = gcvNULL;
    image->u.image.node         = gcvNULL;
    image->u.image.internalFormat = gcvSURF_UNKNOWN;
    image->u.image.texturePhysical = 0;
    image->u.image.textureLogical = 0;
    image->u.image.buffer = gcvNULL;
    /* Create a reference count object and set it to 1. */
    clmONERROR(gcoOS_AtomConstruct(gcvNULL, &image->referenceCount),
               CL_OUT_OF_HOST_MEMORY);

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, image->referenceCount, gcvNULL));

    clmONERROR(gcoOS_AtomIncrement(gcvNULL, clgGlobalId, (gctINT*)&image->id), CL_INVALID_VALUE);

    /* Create modify mutex lock. */
    clmONERROR(gcoOS_CreateMutex(gcvNULL,
                                 &image->mutex),
               CL_OUT_OF_HOST_MEMORY);

    *Image = image;
    gcmFOOTER_ARG("%d Image=0x%x", CL_SUCCESS, image);
    return CL_SUCCESS;

OnError:
    if(image != gcvNULL)
    {
        gcoOS_Free(gcvNULL, image);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

/*****************************************************************************\
|*                       OpenCL Memory Object API                            *|
\*****************************************************************************/
CL_API_ENTRY cl_mem CL_API_CALL
clCreateBuffer(
    cl_context   Context,
    cl_mem_flags Flags,
    size_t       Size,
    void *       HostPtr,
    cl_int *     ErrcodeRet)
{
    clsMem_PTR   buffer = gcvNULL;
    gctINT       status;
#if MAP_TO_DEVICE
    gceCHIPMODEL  chipModel;
#endif
    gcmHEADER_ARG("Context=0x%x Size=%u HostPtr=0x%x Flags=0x%x ",
                   Context, Size, HostPtr, Flags);
    gcmDUMP_API("${OCL clCreateBuffer 0x%x, 0x%x}", Context, Flags);
    VCL_TRACE_API(CreateBuffer_Pre)(Context, Flags, Size, HostPtr, ErrcodeRet);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004000: (clCreateBuffer) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (Size == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004001: (clCreateBuffer) Size is 0.\n");
        clmRETURN_ERROR(CL_INVALID_BUFFER_SIZE);
    }

    if ((Flags & CL_MEM_USE_HOST_PTR) &&
        (Flags & (CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR)))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004002: (clCreateBuffer) invalid Flags.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if((Flags & CL_MEM_USE_UNCACHED_HOST_MEMORY_VIV) && ((Flags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR)) == 0))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004002: (clCreateBuffer) invalid Flags.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if ((HostPtr == gcvNULL && (Flags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR))) ||
        (HostPtr && (Flags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR)) == 0))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004003: (clCreateBuffer) invalid HostPtr.\n");
        clmRETURN_ERROR(CL_INVALID_HOST_PTR);
    }

    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);
    /* New buffer object. */
    clmONERROR(clfNewBuffer(Context, &buffer),
               CL_OUT_OF_HOST_MEMORY);

    /* Mem info. */
    buffer->host            = HostPtr;
    buffer->flags           = Flags ? Flags : CL_MEM_READ_WRITE /* default */;
    buffer->mapFlag         = 0;
    buffer->u.buffer.size   = Size; /* Buffer specific info. */

#if MAP_TO_DEVICE
    /*
     * Map host ptr to physical if flag has CL_MEM_USE_HOST_PTR
     * and host ptr is aligned.

     * This is not required for CL_MEM_ALLOC_HOST_PTR
     * We always allocate from host-accessible memory
     */
    chipModel = Context->devices[0]->deviceInfo.chipModel;
    if ((Flags & CL_MEM_USE_HOST_PTR)
         && !(gcmPTR2INT(HostPtr) & 0x3F)
         && !(chipModel == gcv3000 || chipModel == gcv5000))
    {
        gctUINT32 physical;

        gcoCL_WrapUserMemory(HostPtr, Size, &physical, &buffer->u.buffer.node);

        buffer->u.buffer.allocatedSize  = Size;
        buffer->u.buffer.physical       = (gctPHYS_ADDR)gcmINT2PTR(physical);
        buffer->u.buffer.logical        = HostPtr;
        buffer->u.buffer.wrapped        = gcvTRUE;

        gcoCL_FlushMemory(buffer->u.buffer.node, HostPtr, Size);
    }
    else
#endif
    {
        gctUINT32 memFlag = 0;

        if(Flags & CL_MEM_USE_UNCACHED_HOST_MEMORY_VIV)
            memFlag &= ~gcvALLOC_FLAG_CACHEABLE;

        /* Allocate physical buffer from device. */
        buffer->u.buffer.allocatedSize = Size;
        clmONERROR(gcoCL_AllocateMemory(&buffer->u.buffer.allocatedSize,
                                        &buffer->u.buffer.physical,
                                        &buffer->u.buffer.logical,
                                        &buffer->u.buffer.node,
                                        memFlag),
                   CL_MEM_OBJECT_ALLOCATION_FAILURE);

        if ((Flags & CL_MEM_COPY_HOST_PTR) ||
            (Flags & CL_MEM_USE_HOST_PTR))
        {
#if cldTUNING
            if ((Flags & CL_MEM_READ_ONLY) && Context->sortRects && (Size == 7731040))
            {
                clfSortHaarRects(buffer->u.buffer.logical, HostPtr, Size);
                Context->sortRects = gcvFALSE;
            }
            else
#endif
            {
                gcoOS_MemCopy(buffer->u.buffer.logical, HostPtr, Size);
            }

            gcoCL_FlushMemory(buffer->u.buffer.node, buffer->u.buffer.logical, buffer->u.buffer.allocatedSize);

            gcmDUMP_BUFFER(gcvNULL,
                           "memory",
                           gcmPTR2INT(buffer->u.buffer.physical),
                           buffer->u.buffer.logical,
                           0,
                           buffer->u.buffer.size);
        }
    }

    if (ErrcodeRet) {
        *ErrcodeRet = CL_SUCCESS;
    }

    VCL_TRACE_API(CreateBuffer_Post)(Context, Flags, Size, HostPtr, ErrcodeRet, buffer);
    gcmFOOTER_ARG("%d buffer=0x%x",
                  CL_SUCCESS, buffer);
    return buffer;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY || status == CL_MEM_OBJECT_ALLOCATION_FAILURE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004004: (clCreateBuffer) cannot create buffer.  Maybe run out of memory.\n");
    }

    if(buffer != gcvNULL) gcoOS_Free(gcvNULL,buffer);

    if (ErrcodeRet) {
        *ErrcodeRet = status;
    }

    gcmFOOTER_ARG("%d", status);
    return gcvNULL;
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateSubBuffer(
    cl_mem                   Buffer,
    cl_mem_flags             Flags,
    cl_buffer_create_type    BufferCreateType,
    const void *             BufferCreateInfo,
    cl_int *                 ErrcodeRet
    )
{
    clsMem_PTR   buffer = gcvNULL;
    gctINT       status;
    size_t       origin, size;

    gcmHEADER_ARG("Buffer=0x%x Flags=0x%x BufferCreateType=%u BufferCreateInfo=0x%x",
                   Buffer, Flags, BufferCreateType, BufferCreateInfo);
    gcmDUMP_API("${OCL clCreateSubBuffer 0x%x, 0x%x}", Buffer, Flags);
    VCL_TRACE_API(CreateSubBuffer_Pre)(Buffer, Flags, BufferCreateType, BufferCreateInfo, ErrcodeRet);

    if (Buffer == gcvNULL ||
        Buffer->objectType != clvOBJECT_MEM ||
        Buffer->type != CL_MEM_OBJECT_BUFFER)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004005: (clCreateSubBuffer) invaled Buffer.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    /*if the CL_MEM_READ_WRITE CL_MEM_READ_ONLY or CL_MEM_WRITE_ONLY not set, inherited from Buffer */
    if ( !(Flags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY)) )
    {
        Flags |= (Buffer->flags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY));
    }

    /*the CL_MEM_ALLOC_HOST_PTR CL_MEM_COPY_HOST_PTR and CL_MEM_USE_HOST_PTR inherited from Buffer */
    Flags |= Buffer->flags & (CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR);

    /*if the CL_MEM_HOST_WRITE_ONLY CL_MEM_HOST_READ_ONLY or CL_MEM_HOST_NO_ACCESS not set, inherited from Buffer */
    if ( !(Flags &  (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) )
    {
        Flags |= (Buffer->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) ;
    }

    if ((Buffer->flags & CL_MEM_WRITE_ONLY) &&
        ((Flags & CL_MEM_READ_WRITE) || (Flags &CL_MEM_READ_ONLY)))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004006: (clCreateSubBuffer) invaled flags.  Buffer is write only.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (BufferCreateType != CL_BUFFER_CREATE_TYPE_REGION)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004008: (clCreateSubBuffer) invaled BufferCreateType.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (BufferCreateInfo == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004009: (clCreateSubBuffer) BufferCreateInfo is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    origin = ((cl_buffer_region *)BufferCreateInfo)->origin;
    size   = ((cl_buffer_region *)BufferCreateInfo)->size;

    if (size == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004010: (clCreateSubBuffer) BufferCreateInfo->size is 0.\n");
        clmRETURN_ERROR(CL_INVALID_BUFFER_SIZE);
    }

    if (Buffer->u.buffer.size < origin+size)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004011: (clCreateSubBuffer) invalid BufferCreateInfo--out of bound.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);
    /* New buffer object. */
    clmONERROR(clfNewBuffer(Buffer->context, &buffer),
               CL_OUT_OF_HOST_MEMORY);

    /* Mem info. */
    buffer->host  = Buffer->host ? (gctPOINTER)(gcmPTR2INT(Buffer->host) + origin) : gcvNULL;
    buffer->flags = Flags ? Flags : Buffer->flags /* default - inherit from parent */;

    /* Buffer specific info. */
    buffer->u.buffer.size          = size;
    buffer->u.buffer.allocatedSize = size;
    buffer->u.buffer.parentBuffer  = Buffer;
    buffer->u.buffer.createType    = CL_BUFFER_CREATE_TYPE_REGION;
    buffer->u.buffer.logical       = Buffer->u.buffer.logical ?
        (gctPOINTER)(gcmPTR2INT(Buffer->u.buffer.logical) + origin) : gcvNULL;
    buffer->u.buffer.physical      = Buffer->u.buffer.physical ?
        (gctPOINTER)(gcmPTR2INT(Buffer->u.buffer.physical) + origin) : gcvNULL;

    buffer->u.buffer.bufferCreateInfo.origin    = origin;
    buffer->u.buffer.bufferCreateInfo.size      = size;
    buffer->u.buffer.node = Buffer->u.buffer.node;

    /* Retain Parent buffer. */
    clfRetainMemObject(Buffer);

    if (ErrcodeRet) {
        *ErrcodeRet = CL_SUCCESS;
    }

    VCL_TRACE_API(CreateSubBuffer_Post)(Buffer, Flags, BufferCreateType, BufferCreateInfo, ErrcodeRet, buffer);
    gcmFOOTER_ARG("%d buffer=0x%x",
                  CL_SUCCESS, buffer);
    return buffer;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004012: (clCreateSubBuffer) cannot create subbuffer.  Maybe run out of memory.\n");
    }

    gcmFOOTER_ARG("%d", status);
    if (ErrcodeRet) {
        *ErrcodeRet = status;
    }
    return gcvNULL;
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateImage(
    cl_context               Context ,
    cl_mem_flags             Flags ,
    const cl_image_format *  ImageFormat ,
    const cl_image_desc *    ImageDesc ,
    void *                   HostPtr ,
    cl_int *                 ErrcodeRet
    )
{
    clsMem_PTR              image = gcvNULL;
    clsImageHeader_PTR      imageHeader;
    gctINT                  status;
    gctSIZE_T               rowPitch;
    gctSIZE_T               slicePitch = 0;
    gctSIZE_T               elementSize;
    gceSURF_FORMAT          internalFormat;
    gctSIZE_T               size;
    gctSIZE_T               ImageWidth, ImageHeight, ImageDepth, ImageRowPitch, ImageSlicePitch, ImageArraySize;
    gctSIZE_T               dim1Size, dim2Size, dim3Size;


    gcmHEADER_ARG("Context=0x%x Flags=%u ImageFormat=0x%x ImageDesc=%u HostPtr=0x%x",
                   Context, Flags, ImageFormat, ImageDesc, HostPtr);
    gcmDUMP_API("${OCL clCreateImage 0x%x, 0x%x}", Context, Flags);
    VCL_TRACE_API(CreateImage_Pre)(Context, Flags, ImageFormat, ImageDesc, HostPtr, ErrcodeRet);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004040: (clCreateImage) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (ImageFormat == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004041: (clCreateImage) ImageFormat is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
    }

    if (ImageDesc == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004042: (clCreateImage) ImageDesc is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_IMAGE_DESCRIPTOR);
    }

    ImageWidth = ImageDesc->image_width;
    ImageHeight = ImageDesc->image_height;
    ImageDepth = ImageDesc->image_depth;
    ImageRowPitch = ImageDesc->image_row_pitch;
    ImageSlicePitch = ImageDesc->image_slice_pitch;
    ImageArraySize = ImageDesc->image_array_size;

    if (clfImageFormat2GcFormat(ImageFormat, &elementSize, &internalFormat, gcvNULL))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004043: (clCreateImage) invalid format descriptor.\n");
        clmRETURN_ERROR(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
    }

     if (ImageDesc->num_mip_levels != 0)
     {
         gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004044: (clCreateImage) invalid num_mip_levels (%d).\n",
            ImageDesc->num_mip_levels);
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
     }

     if (ImageDesc->num_samples != 0)
     {
         gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004045: (clCreateImage) invalid num_samples (%d).\n",
            ImageDesc->num_samples);
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
     }

     if (ImageRowPitch != 0 && HostPtr == gcvNULL)
     {
         gcmUSER_DEBUG_ERROR_MSG(
             "OCL-004046: (clCreateImage) ImageRowPitch (%d) is not 0, but HostPtr is NULL.\n",
             ImageRowPitch);
         clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
     }

    /* Get row pitch. */
    rowPitch = (ImageRowPitch ? ImageRowPitch
                              : ImageWidth * elementSize);
    dim1Size = ImageWidth;
    switch (ImageDesc->image_type) {
    case CL_MEM_OBJECT_IMAGE2D:
        if (ImageWidth == 0 || ImageWidth > Context->devices[0]->deviceInfo.image2DMaxWidth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004047: (clCreateImage) invalid ImageWidth (%d).\n",
            ImageWidth);
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
        }
        if (ImageHeight == 0 || ImageHeight > Context->devices[0]->deviceInfo.image2DMaxHeight)
        {
            gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004048: (clCreateImage) invalid ImageHeight (%d).\n",
            ImageHeight);
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
        }

        if (ImageRowPitch != 0 &&
               (ImageRowPitch < ImageWidth * elementSize ||
                ImageRowPitch % elementSize != 0 ))
        {
            gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004049: (clCreateImage) invalid image size.\n");
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
        }
        ImageDepth = 1;
        dim2Size = ImageHeight;
        dim3Size = ImageDepth;
        /* Calculate the size of the image. */
        size = rowPitch * ImageHeight;
        break;

    case CL_MEM_OBJECT_IMAGE1D:
        if (ImageWidth == 0 || ImageWidth > Context->devices[0]->deviceInfo.image2DMaxWidth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-004050: (clCreateImage) invalid ImageWidth (%d).\n",
                ImageWidth);
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
        }
        ImageHeight = 1;
        ImageDepth = 1;
        dim2Size = ImageHeight;
        dim3Size = ImageDepth;
        /* Calculate the size of the image. */
        size = rowPitch * ImageHeight;
        break;

    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        if (ImageWidth == 0 || ImageWidth > Context->devices[0]->deviceInfo.image2DMaxWidth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004051: (clCreateImage) invalid ImageWidth (%d).\n",
            ImageWidth);
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
        }
        if (ImageHeight == 0 || ImageHeight > Context->devices[0]->deviceInfo.image2DMaxHeight)
        {
            gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004052: (clCreateImage) invalid ImageHeight (%d).\n",
            ImageHeight);
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
        }

        if (ImageRowPitch != 0 &&
               (ImageRowPitch < ImageWidth * elementSize ||
                ImageRowPitch % elementSize != 0 ))
        {
            gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004053: (clCreateImage) invalid image size.\n");
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
        }

        if (ImageSlicePitch != 0 && HostPtr != gcvNULL &&
            (ImageSlicePitch < rowPitch * ImageHeight))
        {
            gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004054: (clCreateImage) invalid ImageSlicePitch (%d).\n",ImageSlicePitch);
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
        }
        dim2Size = ImageHeight;
        dim3Size = ImageArraySize;
        /* Get slice pitch. */
        slicePitch = (ImageSlicePitch ? ImageSlicePitch
                                      : rowPitch * ImageHeight);
        ImageDepth = 1;
        /* Calculate the size of the image. */
        size = slicePitch * ImageArraySize;
        break;
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
        if (ImageWidth == 0 || ImageWidth > Context->devices[0]->deviceInfo.image2DMaxWidth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-004055: (clCreateImage) invalid ImageWidth (%d).\n",
                ImageWidth);
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
        }
        ImageHeight = 1;
        if (ImageSlicePitch != 0 && HostPtr != gcvNULL &&
            (ImageSlicePitch < rowPitch * ImageHeight))
        {
            gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004056: (clCreateImage) invalid ImageSlicePitch (%d).\n",ImageSlicePitch);
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
        }
        /* Get slice pitch. */
        slicePitch = (ImageSlicePitch ? ImageSlicePitch
                                      : rowPitch * ImageHeight);
        ImageDepth = 1;
        /* Calculate the size of the image. */
        size = slicePitch * ImageArraySize;
        dim2Size = ImageArraySize;
        dim3Size = ImageDepth;
        break;
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        if (ImageWidth == 0 || ImageWidth > Context->devices[0]->deviceInfo.imageMaxBufferSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-004057: (clCreateImage) invalid ImageWidth (%d).\n",
                ImageWidth);
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
        }
        ImageHeight = 1;
        ImageDepth = 1;
        dim2Size = ImageHeight;
        dim3Size = ImageDepth;
        /* Calculate the size of the image. */
        size = rowPitch * ImageHeight;
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        if (ImageWidth == 0 || ImageWidth > Context->devices[0]->deviceInfo.image2DMaxWidth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004058: (clCreateImage) invalid ImageWidth (%d).\n",
            ImageWidth);
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
        }
        if (ImageHeight == 0 || ImageHeight > Context->devices[0]->deviceInfo.image2DMaxHeight)
        {
            gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004059: (clCreateImage) invalid ImageHeight (%d).\n",
            ImageHeight);
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
        }
        if (ImageDepth == 0 || ImageDepth > Context->devices[0]->deviceInfo.image3DMaxDepth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004060: (clCreateImage) invalid ImageHeight (%d).\n",
            ImageHeight);
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
        }

        if (ImageRowPitch != 0 &&
               (ImageRowPitch < ImageWidth * elementSize ||
                ImageRowPitch % elementSize != 0 ))
        {
            gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004061: (clCreateImage) invalid image size.\n");
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
        }

        if (ImageSlicePitch != 0 && HostPtr != gcvNULL &&
            (ImageSlicePitch < slicePitch * ImageDepth))
        {
            gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004062: (clCreateImage) invalid ImageSlicePitch (%d).\n",ImageSlicePitch);
            clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
        }
        /* Get slice pitch. */
        slicePitch = (ImageSlicePitch ? ImageSlicePitch
                                      : rowPitch * ImageHeight);
        /* Calculate the size of the image. */
        size = slicePitch * ImageDepth;
        dim2Size = ImageHeight;
        dim3Size = ImageDepth;
        break;

    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004064: (clCreateImage) invalid image type (%d).\n", ImageDesc->image_type);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }


    if ((Flags & CL_MEM_USE_HOST_PTR) &&
        (Flags & (CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR)))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004064: (clCreateImage) invalid Flags.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if ((HostPtr == gcvNULL && (Flags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR))) ||
        (HostPtr && (Flags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR)) == 0))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004065: (clCreateImage) invalid HostPtr.\n");
        clmRETURN_ERROR(CL_INVALID_HOST_PTR);
    }
    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);

    /* TODO - Set endian hint. */


    /* New image object. */
    clmONERROR(clfNewImage(Context, &image),
               CL_OUT_OF_HOST_MEMORY);

    image->type = ImageDesc->image_type; /* Update image type */

    /* Mem info. */
    image->host            = HostPtr;

    /* default image type*/
    if (!Flags && ImageDesc->image_type != CL_MEM_OBJECT_IMAGE1D)
    {
        image->flags = CL_MEM_READ_WRITE;
    }
    else if (ImageDesc->image_type == CL_MEM_OBJECT_IMAGE1D && ImageDesc->buffer)
    {
        image->flags = Flags;
        /* if not set CL_MEM_READ_WRITE CL_MEM_READ_ONLY CL_MEM_WRITE_ONLY inherite from parent */
        if (!(Flags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY)))
        {
            image->flags |= (ImageDesc->buffer->flags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY));
        }
        image->flags |= (ImageDesc->buffer->flags & (CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR));

        /* if not set CL_MEM_HOST_WRITE_ONLY CL_MEM_HOST_READ_ONLY CL_MEM_HOST_NO_ACCESS inherite from parent */
        if (!(Flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS)))
        {
            image->flags |= (ImageDesc->buffer->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS));
        }
    }
    else
    {
        image->flags = Flags;
    }

    /* Image specific info. */
    image->u.image.format           = *ImageFormat;
    image->u.image.width            = ImageWidth;
    image->u.image.height           = ImageHeight;
    image->u.image.depth            = ImageDepth;
    image->u.image.rowPitch         = rowPitch;
    image->u.image.slicePitch       = slicePitch;
    image->u.image.elementSize      = elementSize;
    image->u.image.size             = size;
    image->u.image.arraySize        = ImageArraySize;
    image->u.image.texture          = gcvNULL;
    image->u.image.node             = gcvNULL;
    image->u.image.internalFormat   = internalFormat;
    image->u.image.texturePhysical  = 0;
    image->u.image.textureLogical   = 0;
    image->u.image.surfaceMapped    = gcvFALSE;
    image->u.image.tiling           = gcvLINEAR;
    /* TODO: need to handle 1d buffer */
    image->u.image.buffer           = ImageDesc->buffer;
    image->mapFlag                  = 0;
    image->u.image.vxcaddressingMode = 0;
    image->u.image.vxcfilterMode    = 0;
    image->u.image.vxcnormalizedCoords = 0;

    /* Allocate physical buffer for image header. */
    image->u.image.allocatedSize = sizeof(clsImageHeader);
    clmONERROR(gcoCL_AllocateMemory(&image->u.image.allocatedSize,
                                    &image->u.image.physical,
                                    &image->u.image.logical,
                                    &image->u.image.node,
                                    0),
               CL_MEM_OBJECT_ALLOCATION_FAILURE);

    imageHeader = (clsImageHeader_PTR) image->u.image.logical;

    /* Get endian hint */
    /*endianHint = clfEndianHint(internalformat, type);*/

#if MAP_TO_DEVICE
    image->u.image.surfaceMapped = (Flags & CL_MEM_USE_HOST_PTR) && !(gcmPTR2INT(HostPtr) & 0x3F);
#endif
    clmONERROR(gcoCL_CreateTexture(&image->u.image.surfaceMapped,
                                   dim1Size,
                                   dim2Size,
                                   dim3Size,
                                   HostPtr,
                                   rowPitch,
                                   slicePitch,
                                   internalFormat,
                                   gcvENDIAN_NO_SWAP,
                                   &image->u.image.texture,
                                   &image->u.image.surface,
                                   &image->u.image.texturePhysical,
                                   &image->u.image.textureLogical,
                                   &image->u.image.textureStride,
                                   &image->u.image.textureSlicePitch),
               CL_MEM_OBJECT_ALLOCATION_FAILURE);

    switch (image->type) {
    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
    case CL_MEM_OBJECT_IMAGE2D:
        image->u.image.textureSlicePitch = 0;
        break;
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
        image->u.image.textureSlicePitch = image->u.image.textureStride;
        break;
    }

    /* For 1D buffer, associate the buffer with the image and sync the data. */
    if (image->type == CL_MEM_OBJECT_IMAGE1D_BUFFER)
    {
        gcmASSERT((image->u.image.buffer != gcvNULL)
                    && (image->u.image.buffer->u.buffer.logical != gcvNULL));
        image->u.image.buffer->u.buffer.image = image;
        gcoOS_MemCopy(image->u.image.textureLogical, image->u.image.buffer->u.buffer.logical, size);
    }

    gcoCL_FlushSurface(image->u.image.surface);

    imageHeader->width              = ImageWidth;
    imageHeader->height             = ImageHeight;
    imageHeader->depth              = ImageDepth;
    imageHeader->arraySize          = ImageArraySize;
    imageHeader->rowPitch           = image->u.image.textureStride;
    imageHeader->slicePitch         = image->u.image.textureSlicePitch;
    imageHeader->channelDataType    = ImageFormat->image_channel_data_type;
    imageHeader->channelOrder       = ImageFormat->image_channel_order;
    imageHeader->samplerValue       = -1;
    imageHeader->tiling             = image->u.image.tiling;
    imageHeader->physical           = gcmPTR2INT32(image->u.image.texturePhysical);
    imageHeader->imageType          = ImageDesc->image_type;

    if (ErrcodeRet)
    {
        *ErrcodeRet = CL_SUCCESS;
    }

    VCL_TRACE_API(CreateImage_Post)(Context, Flags, ImageFormat, ImageDesc, HostPtr, ErrcodeRet, image);
    gcmFOOTER_ARG("%d image=0x%x",
                  CL_SUCCESS, image);
    return image;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY || status == CL_MEM_OBJECT_ALLOCATION_FAILURE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004066: (clCreateImage) cannot create image.  Maybe run out of memory.\n");
    }

    if(image != gcvNULL) gcoOS_Free(gcvNULL,image);
    if (ErrcodeRet)
    {
        *ErrcodeRet = status;
    }

    gcmFOOTER_ARG("%d", status);
    return gcvNULL;
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateImage2D(
    cl_context              Context,
    cl_mem_flags            Flags,
    const cl_image_format * ImageFormat,
    size_t                  ImageWidth,
    size_t                  ImageHeight,
    size_t                  ImageRowPitch,
    void *                  HostPtr,
    cl_int *                ErrcodeRet
    )
{
    clsMem_PTR              image = gcvNULL;
    clsImageHeader_PTR      imageHeader;
    gctINT                  status;
    gctSIZE_T               rowPitch;
    gctSIZE_T               elementSize;
    gceSURF_FORMAT          internalFormat;
    gctSIZE_T               size;
#if MAP_TO_DEVICE
    gceCHIPMODEL            chipModel;
#endif

    gcmHEADER_ARG("Context=0x%x Flags=%u ImageFormat=0x%x ImageWidth=%u ImageWidth=%u ImageRowPitch=%u HostPtr=0x%x",
                   Context, Flags, ImageFormat, ImageWidth, ImageHeight, ImageRowPitch, HostPtr);
    gcmDUMP_API("${OCL clCreateImage2D 0x%x, 0x%x}", Context, Flags);
    VCL_TRACE_API(CreateImage2D_Pre)(Context, Flags, ImageFormat, ImageWidth, ImageHeight, ImageRowPitch, HostPtr, ErrcodeRet);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004013: (clCreateImage2D) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (ImageFormat == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004014: (clCreateImage2D) ImageFormat is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
    }

    if (ImageWidth == 0 || ImageWidth > Context->devices[0]->deviceInfo.image2DMaxWidth)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004015: (clCreateImage2D) invalid ImageWidth (%d).\n",
            ImageWidth);
        clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
    }

    if (ImageHeight == 0 || ImageHeight > Context->devices[0]->deviceInfo.image2DMaxHeight)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004016: (clCreateImage2D) invalid ImageHeight (%d).\n",
            ImageHeight);
        clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
    }

    if (ImageRowPitch != 0 && HostPtr == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004017: (clCreateImage2D) ImageRowPitch (%d) is not 0, but HostPtr is NULL.\n",
            ImageRowPitch);
        clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
    }

    if ((Flags & CL_MEM_USE_HOST_PTR) &&
        (Flags & (CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR)))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004018: (clCreateImage2D) invalid Flags.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if ((HostPtr == gcvNULL && (Flags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR))) ||
        (HostPtr && (Flags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR)) == 0))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004019: (clCreateImage2D) invalid HostPtr.\n");
        clmRETURN_ERROR(CL_INVALID_HOST_PTR);
    }

    if (clfImageFormat2GcFormat(ImageFormat, &elementSize, &internalFormat, gcvNULL))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004020: (clCreateImage2D) invalid format descriptor.\n");
        clmRETURN_ERROR(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
    }

    if (ImageRowPitch != 0 &&
                   (ImageRowPitch < ImageWidth * elementSize ||
                    ImageRowPitch % elementSize != 0 ||
                    (ImageRowPitch / elementSize) % 4 != 0))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004021: (clCreateImage2D) invalid image size.\n");
        clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
    }

    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);
    /* TODO - Set endian hint. */

    /* Get row pitch. */
    rowPitch = (ImageRowPitch ? ImageRowPitch
                              : ImageWidth * elementSize);

    /* Calculate the size of the image. */
    size = rowPitch * ImageHeight;

    /* New image object. */
    clmONERROR(clfNewImage(Context, &image),
               CL_OUT_OF_HOST_MEMORY);

    /* Mem info. */
    image->host            = HostPtr;
    image->flags           = Flags ? Flags : CL_MEM_READ_WRITE /* default */;

    /* Image specific info. */
    image->u.image.format           = *ImageFormat;
    image->u.image.width            = ImageWidth;
    image->u.image.height           = ImageHeight;
    image->u.image.depth            = 1;
    image->u.image.rowPitch         = rowPitch;
    image->u.image.slicePitch       = 0;
    image->u.image.elementSize      = elementSize;
    image->u.image.size             = size;
    image->u.image.texture          = gcvNULL;
    image->u.image.node             = gcvNULL;
    image->u.image.internalFormat   = internalFormat;
    image->u.image.texturePhysical  = 0;
    image->u.image.textureLogical   = 0;
    image->u.image.surfaceMapped    = gcvFALSE;
    image->u.image.tiling           = gcvLINEAR;
    image->u.image.vxcaddressingMode = 0;
    image->u.image.vxcfilterMode    = 0;
    image->u.image.vxcnormalizedCoords = 0;

    /* Allocate physical buffer for image header. */
    image->u.image.allocatedSize = sizeof(clsImageHeader);
    clmONERROR(gcoCL_AllocateMemory(&image->u.image.allocatedSize,
                                    &image->u.image.physical,
                                    &image->u.image.logical,
                                    &image->u.image.node,
                                    0),
               CL_MEM_OBJECT_ALLOCATION_FAILURE);

    imageHeader = (clsImageHeader_PTR) image->u.image.logical;

    /* Get endian hint */
    /*endianHint = clfEndianHint(internalformat, type);*/

#if MAP_TO_DEVICE
    chipModel = Context->devices[0]->deviceInfo.chipModel;

    image->u.image.surfaceMapped =
            (Flags & CL_MEM_USE_HOST_PTR)
            && !(gcmPTR2INT(HostPtr) & 0x3F)
            && !(chipModel == gcv3000 || chipModel == gcv5000);
#endif
    clmONERROR(gcoCL_CreateTexture(&image->u.image.surfaceMapped,
                                   ImageWidth,
                                   ImageHeight,
                                   1,
                                   HostPtr,
                                   rowPitch,
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

    imageHeader->width              = ImageWidth;
    imageHeader->height             = ImageHeight;
    imageHeader->depth              = 0;
    imageHeader->rowPitch           = image->u.image.textureStride;
    imageHeader->channelDataType    = ImageFormat->image_channel_data_type;
    imageHeader->channelOrder       = ImageFormat->image_channel_order;
    imageHeader->samplerValue       = -1;
    imageHeader->tiling             = image->u.image.tiling;
    imageHeader->slicePitch         = 0;
    imageHeader->physical           = gcmPTR2INT32(image->u.image.texturePhysical);
    imageHeader->imageType          = CL_MEM_OBJECT_IMAGE2D;

    if (ErrcodeRet)
    {
        *ErrcodeRet = CL_SUCCESS;
    }

    VCL_TRACE_API(CreateImage2D_Post)(Context, Flags, ImageFormat, ImageWidth, ImageHeight, ImageRowPitch, HostPtr, ErrcodeRet, image);
    gcmFOOTER_ARG("%d image=0x%x",
                  CL_SUCCESS, image);
    return image;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY || status == CL_MEM_OBJECT_ALLOCATION_FAILURE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004022: (clCreateImage2D) cannot create image.  Maybe run out of memory.\n");
    }

    if(image != gcvNULL) gcoOS_Free(gcvNULL,image);
    if (ErrcodeRet)
    {
        *ErrcodeRet = status;
    }

    gcmFOOTER_ARG("%d", status);
    return gcvNULL;
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateImage3D(
    cl_context              Context,
    cl_mem_flags            Flags,
    const cl_image_format * ImageFormat,
    size_t                  ImageWidth,
    size_t                  ImageHeight,
    size_t                  ImageDepth,
    size_t                  ImageRowPitch,
    size_t                  ImageSlicePitch,
    void *                  HostPtr,
    cl_int *                ErrcodeRet
    )
{
    gctINT                  status;
    gctUINT                 i;
    clsMem_PTR              image = gcvNULL;
    clsImageHeader_PTR      imageHeader;
    gctSIZE_T               rowPitch, slicePitch;
    gctSIZE_T               elementSize;
    gceSURF_FORMAT          internalFormat;
    gctSIZE_T               size;
    gctBOOL                 imageSupport = gcvFALSE;

    gcmHEADER_ARG("Context=0x%x Flags=%u ImageFormat=0x%x ImageWidth=%u ImageHeight=%u ImageDepth=%u ImageRowPitch=%u ImageSlicePitch=%u HostPtr=0x%x",
                   Context, Flags, ImageFormat, ImageWidth, ImageHeight, ImageDepth, ImageRowPitch, ImageSlicePitch, HostPtr);
    gcmDUMP_API("${OCL clCreateImage3D 0x%x, 0x%x}", Context, Flags);
    VCL_TRACE_API(CreateImage3D_Pre)(Context, Flags, ImageFormat, ImageWidth, ImageHeight, ImageDepth, ImageRowPitch, ImageSlicePitch, HostPtr, ErrcodeRet);

    if (Context->devices[0]->deviceInfo.image3DMaxDepth == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004023: (clCreateImage3D) image3D is not supported.\n");
        clmRETURN_ERROR(CL_INVALID_OPERATION);
    }

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004013: (clCreateImage3D) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (ImageFormat == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004014: (clCreateImage3D) ImageFormat is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
    }

    if (ImageWidth == 0 || ImageWidth > Context->devices[0]->deviceInfo.image2DMaxWidth)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004015: (clCreateImage3D) invalid ImageWidth (%d).\n",
            ImageWidth);
        clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
    }

    if (ImageHeight == 0 || ImageHeight > Context->devices[0]->deviceInfo.image2DMaxHeight)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004016: (clCreateImage3D) invalid ImageHeight (%d).\n",
            ImageHeight);
        clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
    }

    if (ImageDepth <= 1 || ImageDepth > Context->devices[0]->deviceInfo.image2DMaxHeight)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004016: (clCreateImage3D) invalid ImageDepth (%d).\n",
            ImageDepth);
        clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
    }

    if (HostPtr == gcvNULL && (ImageRowPitch != 0 || ImageSlicePitch != 0) )
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004017: (clCreateImage3D) ImageRowPitch (%d) is not 0 or ImageSlicePitch(%d) is not 0, but HostPtr is NULL.\n",
            ImageRowPitch, ImageSlicePitch);
        clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
    }

    if ((Flags & CL_MEM_USE_HOST_PTR) &&
        (Flags & (CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR)))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004018: (clCreateImage3D) invalid Flags.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if ((HostPtr == gcvNULL && (Flags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR))) ||
        (HostPtr && (Flags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR)) == 0))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004019: (clCreateImage3D) invalid HostPtr.\n");
        clmRETURN_ERROR(CL_INVALID_HOST_PTR);
    }

    if (clfImageFormat2GcFormat(ImageFormat, &elementSize, &internalFormat, gcvNULL))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004020: (clCreateImage3D) invalid format descriptor.\n");
        clmRETURN_ERROR(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
    }
    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);
    /* Get row pitch. */
    rowPitch = (ImageRowPitch ? ImageRowPitch
                              : ImageWidth * elementSize);

    /* Get slice pitch. */
    slicePitch= (ImageSlicePitch ? ImageSlicePitch
                                 : rowPitch * ImageHeight);

    /* Calculate the size of the image. */
    size = slicePitch * ImageDepth;

    if (ImageRowPitch != 0 &&
                   (ImageRowPitch < ImageWidth * elementSize ||
                    ImageRowPitch % elementSize != 0 ||
                    (ImageRowPitch / elementSize) % 4 != 0))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004021: (clCreateImage3D) invalid image size.\n");
        clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
    }

    if(ImageSlicePitch != 0 &&
                    (ImageSlicePitch < rowPitch * ImageHeight ||
                     ImageSlicePitch % rowPitch != 0))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004021: (clCreateImage3D) invalid image size.\n");
        clmRETURN_ERROR(CL_INVALID_IMAGE_SIZE);
    }

    for(i = 0; i < Context->numDevices; i++)
    {
        if(Context->devices[i]->deviceInfo.imageSupport == gcvTRUE)
        {
            imageSupport = gcvTRUE;
        }
    }

    if(imageSupport == gcvFALSE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
                "OCL-004021: (clCreateImage3D) invalid operation.\n");
            clmRETURN_ERROR(CL_INVALID_OPERATION);
    }

    /* TODO - Set endian hint. */
    /* New image object. */
    clmONERROR(clfNewImage(Context, &image),
               CL_OUT_OF_HOST_MEMORY);

    /* Mem info. */
    image->type            = CL_MEM_OBJECT_IMAGE3D;
    image->host            = HostPtr;
    image->flags           = Flags ? Flags : CL_MEM_READ_WRITE /* default */;

    /* Image specific info. */
    image->u.image.format           = *ImageFormat;
    image->u.image.width            = ImageWidth;
    image->u.image.height           = ImageHeight;
    image->u.image.depth            = ImageDepth;
    image->u.image.rowPitch         = rowPitch;
    image->u.image.slicePitch       = slicePitch;
    image->u.image.elementSize      = elementSize;
    image->u.image.size             = size;
    image->u.image.texture          = gcvNULL;
    image->u.image.node             = gcvNULL;
    image->u.image.internalFormat   = internalFormat;
    image->u.image.texturePhysical  = 0;
    image->u.image.textureLogical   = 0;
    image->u.image.surfaceMapped    = gcvFALSE;
    image->u.image.tiling           = gcvLINEAR;
    image->u.image.vxcaddressingMode = 0;
    image->u.image.vxcfilterMode    = 0;
    image->u.image.vxcnormalizedCoords = 0;

    /* Allocate physical buffer for image header. */
    image->u.image.allocatedSize = sizeof(clsImageHeader);
    clmONERROR(gcoCL_AllocateMemory(&image->u.image.allocatedSize,
                                    &image->u.image.physical,
                                    &image->u.image.logical,
                                    &image->u.image.node,
                                    0),
               CL_MEM_OBJECT_ALLOCATION_FAILURE);

    imageHeader = (clsImageHeader_PTR) image->u.image.logical;

#if MAP_TO_DEVICE
    image->u.image.surfaceMapped = (Flags & CL_MEM_USE_HOST_PTR) && !(gcmPTR2INT(HostPtr) & 0x3F);
#endif
    clmONERROR(gcoCL_CreateTexture(&image->u.image.surfaceMapped,
                                   ImageWidth,
                                   ImageHeight,
                                   ImageDepth,
                                   HostPtr,
                                   rowPitch,
                                   slicePitch,
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

    imageHeader->width              = ImageWidth;
    imageHeader->height             = ImageHeight;
    imageHeader->depth              = ImageDepth;
    imageHeader->rowPitch           = image->u.image.textureStride;
    imageHeader->channelDataType    = ImageFormat->image_channel_data_type;
    imageHeader->channelOrder       = ImageFormat->image_channel_order;
    imageHeader->samplerValue       = -1;
    imageHeader->tiling             = image->u.image.tiling;
    imageHeader->slicePitch         = image->u.image.textureSlicePitch;
    imageHeader->physical           = gcmPTR2INT32(image->u.image.texturePhysical);
    imageHeader->imageType          = CL_MEM_OBJECT_IMAGE3D;

    if (ErrcodeRet)
    {
        *ErrcodeRet = CL_SUCCESS;
    }

    VCL_TRACE_API(CreateImage3D_Post)(Context, Flags, ImageFormat, ImageWidth, ImageHeight, ImageDepth, ImageRowPitch, ImageSlicePitch, HostPtr, ErrcodeRet, image);
    gcmFOOTER_ARG("%d image=0x%x",
                  CL_SUCCESS, image);
    return image;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY || status == CL_MEM_OBJECT_ALLOCATION_FAILURE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004022: (clCreateImage3D) cannot create image.  Maybe run out of memory.\n");
    }

    if(image != gcvNULL) gcoOS_Free(gcvNULL,image);
    if (ErrcodeRet)
    {
        *ErrcodeRet = status;
    }

    gcmFOOTER_ARG("%d", status);
    return gcvNULL;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainMemObject(
    cl_mem MemObj
    )
{
    gctINT status;

    gcmHEADER_ARG("MemObj=0x%x", MemObj);
    gcmDUMP_API("${OCL clRetainMemObject 0x%x}", MemObj);

    if (MemObj == gcvNULL ||
        MemObj->objectType != clvOBJECT_MEM)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004025: (clRetainMemObject) invalid MemObj.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    clfONERROR(clfRetainMemObject(MemObj));

    VCL_TRACE_API(RetainMemObject)(MemObj);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseMemObject(
    cl_mem MemObj
    )
{
    gctINT                  status;

    gcmHEADER_ARG("MemObj=0x%x", MemObj);
    gcmDUMP_API("${OCL clReleaseMemObject 0x%x}", MemObj);

    if (MemObj == gcvNULL ||
        MemObj->objectType != clvOBJECT_MEM)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004026: (clReleaseMemObject) invalid MemObj.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    clfONERROR(clfReleaseMemObject(MemObj));

    VCL_TRACE_API(ReleaseMemObject)(MemObj);
    gcmFOOTER_NO();
    return CL_SUCCESS;

OnError:
    if (status != CL_INVALID_MEM_OBJECT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004027: (clReleaseMemObject) internal error.\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}


CL_API_ENTRY cl_int CL_API_CALL
clGetSupportedImageFormats(
    cl_context           Context,
    cl_mem_flags         Flags,
    cl_mem_object_type   ImageType,
    cl_uint              NumEntries,
    cl_image_format *    ImageFormats,
    cl_uint *            NumImageFormats
    )
{
    gctINT           status;
    cl_image_format   supported[] =
    {
        {CL_RGBA,   CL_UNORM_INT8},
        {CL_RGBA,   CL_UNORM_INT16},
        {CL_RGBA,   CL_SIGNED_INT8},
        {CL_RGBA,   CL_SIGNED_INT16},
        {CL_RGBA,   CL_SIGNED_INT32},
        {CL_RGBA,   CL_UNSIGNED_INT8},
        {CL_RGBA,   CL_UNSIGNED_INT16},
        {CL_RGBA,   CL_UNSIGNED_INT32},
        {CL_RGBA,   CL_HALF_FLOAT},
        {CL_RGBA,   CL_FLOAT},
        {CL_BGRA,   CL_UNORM_INT8},
        {CL_BGRA,   CL_UNORM_INT16},
        {CL_BGRA,   CL_SIGNED_INT8},
        {CL_BGRA,   CL_SIGNED_INT16},
        {CL_BGRA,   CL_SIGNED_INT32},
        {CL_BGRA,   CL_UNSIGNED_INT8},
        {CL_BGRA,   CL_UNSIGNED_INT16},
        {CL_BGRA,   CL_UNSIGNED_INT32},
        {CL_BGRA,   CL_HALF_FLOAT},
        {CL_BGRA,   CL_FLOAT}
,
        /*{CL_R,      CL_SNORM_INT8},*/
        /*{CL_R,      CL_UNORM_INT8},*/
        /*{CL_R,      CL_SIGNED_INT8},*/
        {CL_R,      CL_UNSIGNED_INT8},
        /*{CL_R,      CL_SNORM_INT16},*/
        /*{CL_R,      CL_UNORM_INT16},*/
        {CL_R,      CL_SIGNED_INT16},
        /*{CL_R,      CL_UNSIGNED_INT16},*/
        /*{CL_R,      CL_SIGNED_INT32},*/
        /*{CL_R,      CL_UNSIGNED_INT32},*/
        /*{CL_R,      CL_HALF_FLOAT},*/
        {CL_R,      CL_FLOAT}
    };
    cl_uint count;

    gcmHEADER_ARG("Context=0x%x Flags=%u ImageType=0x%x NumEntries=%u",
                   Context, Flags, ImageType, NumEntries);
    gcmDUMP_API("${OCL clGetSupportedImageFormats 0x%x, 0x%x}", Context, Flags);

    if(!gcoOS_StrCmp(clgDefaultDevice->deviceVersion, cldVERSION11)) count = 10;
    else count = (cl_uint)(gcmSIZEOF(supported) / gcmSIZEOF(supported[0]));


    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004028: (clGetSupportedImageFormats) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (NumEntries == 0 && ImageFormats != gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004029: (clGetSupportedImageFormats) NumEntries is 0, but ImageFormats is not NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (ImageFormats)
    {
        if (NumEntries >= 10)
        {
            cl_uint mincount = gcmMIN(count, NumEntries);
            cl_uint i;

            for (i = 0; i <mincount; i++)
            {
                ImageFormats[i].image_channel_data_type = supported[i].image_channel_data_type;
                ImageFormats[i].image_channel_order     = supported[i].image_channel_order;
            }
        }
        else
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-004030: (clGetSupportedImageFormats) NumEntries (%d) is less than supported nubmer (10).\n",
                NumEntries);
        }
    }

    if (NumImageFormats)
    {
        *NumImageFormats = count;
    }

    VCL_TRACE_API(GetSupportedImageFormats)(Context, Flags, ImageType, NumEntries, ImageFormats, NumImageFormats);
    gcmFOOTER_ARG("%d *NumImageFormats=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(NumImageFormats));
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetMemObjectInfo(
    cl_mem          MemObj,
    cl_mem_info     ParamName,
    size_t          ParamValueSize,
    void *          ParamValue,
    size_t *        ParamValueSizeRet
    )
{
    gctSIZE_T       retParamSize = 0;
    gctPOINTER      retParamPtr = NULL;
    size_t          retValue_size_t;
    gctINT          status;
    static cl_uint  zerointSize = 0;
    static size_t   zerosizetSize = 0;
    static cl_mem   nullMemObj = gcvNULL;
    gctINT32        referenceCount;

    gcmHEADER_ARG("MemObj=0x%x ParamName=%u ParamValueSize=%lu ParamValue=0x%x",
                  MemObj, ParamName, ParamValueSize, ParamValue);
    gcmDUMP_API("${OCL clGetMemObjectInfo 0x%x, 0x%x}", MemObj, ParamName);

    if (MemObj == gcvNULL || MemObj->objectType != clvOBJECT_MEM)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004031: (clGetMemObjectInfo) invalid MemObj.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    switch (ParamName)
    {
        case CL_MEM_REFERENCE_COUNT:
            gcmVERIFY_OK(gcoOS_AtomGet(gcvNULL, MemObj->referenceCount, &referenceCount));
            retParamSize = gcmSIZEOF(referenceCount);
            retParamPtr = &referenceCount;
            break;

        case CL_MEM_TYPE:
            retParamSize = gcmSIZEOF(MemObj->type);
            retParamPtr = &MemObj->type;
            break;

        case CL_MEM_FLAGS:
            retParamSize = gcmSIZEOF(MemObj->flags);
            retParamPtr = &MemObj->flags;
            break;

        case CL_MEM_HOST_PTR:
            retParamSize = gcmSIZEOF(MemObj->host);
            retParamPtr = &MemObj->host;
            break;

        case CL_MEM_CONTEXT:
            retParamSize = gcmSIZEOF(MemObj->context);
            retParamPtr = &MemObj->context;
            break;

        case CL_MEM_MAP_COUNT:
            if (MemObj->type == CL_MEM_OBJECT_BUFFER)
            {
                retParamSize = gcmSIZEOF(MemObj->mapCount);
                retParamPtr = &(MemObj->mapCount);
            }
            else
            {
                retParamSize = gcmSIZEOF(zerointSize);
                retParamPtr = &zerointSize;
            }
            break;

        case CL_MEM_ASSOCIATED_MEMOBJECT:
            if (MemObj->type == CL_MEM_OBJECT_BUFFER)
            {
                retParamSize = gcmSIZEOF(MemObj->u.buffer.parentBuffer);
                retParamPtr = &(MemObj->u.buffer.parentBuffer);
            }
            else
            {
                retParamSize = gcmSIZEOF(nullMemObj);
                retParamPtr = &nullMemObj;
            }
            break;

        case CL_MEM_OFFSET:
            if (MemObj->type == CL_MEM_OBJECT_BUFFER &&
                MemObj->u.buffer.createType == CL_BUFFER_CREATE_TYPE_REGION) {
                /*retParamSize = gcmSIZEOF(MemObj->u.buffer.bufferCreateInfo.origin);
                retParamPtr = &(MemObj->u.buffer.bufferCreateInfo.origin);*/
                retValue_size_t = MemObj->u.buffer.bufferCreateInfo.origin;
            } else {
                /*retParamSize = gcmSIZEOF(zerosizetSize);
                retParamPtr = &zerosizetSize;*/
                retValue_size_t = zerosizetSize;
            }
            retParamSize = gcmSIZEOF(retValue_size_t);
            retParamPtr = &retValue_size_t;
            break;

        case CL_MEM_SIZE:
            if (MemObj->type == CL_MEM_OBJECT_BUFFER)
            {
                /*retParamSize = gcmSIZEOF(MemObj->u.buffer.size);
                retParamPtr = &(MemObj->u.buffer.size);*/
                retValue_size_t = MemObj->u.buffer.size;
            }
            else if (MemObj->type == CL_MEM_OBJECT_IMAGE2D ||
                       MemObj->type == CL_MEM_OBJECT_IMAGE3D)
            {
                /*retParamSize = gcmSIZEOF(MemObj->u.image.size);
                retParamPtr = &(MemObj->u.image.size);*/
                retValue_size_t = MemObj->u.image.size;
            }
            retParamSize = gcmSIZEOF(retValue_size_t);
            retParamPtr = &retValue_size_t;
            break;

        default:
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-004032: (clGetMemObjectInfo) invalid ParamName (0x%x).\n",
                ParamName);
            clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (ParamValue) {
        if (ParamValueSize < retParamSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-004033: (clGetMemObjectInfo) ParamValueSize (%d) is less than required size (%d).\n",
                ParamValueSize, retParamSize);
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (retParamSize)
        {
            gcoOS_MemCopy(ParamValue, retParamPtr, retParamSize);
        }
    }

    if (ParamValueSizeRet)
    {
        *ParamValueSizeRet = retParamSize;
    }

    VCL_TRACE_API(GetMemObjectInfo)(MemObj, ParamName, ParamValueSize, ParamValue, ParamValueSizeRet);
    gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(ParamValueSizeRet));
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetImageInfo(
    cl_mem           Image,
    cl_image_info    ParamName,
    size_t           ParamValueSize,
    void *           ParamValue,
    size_t *         ParamValueSizeRet
    )
{
    gctSIZE_T        retParamSize = 0;
    gctPOINTER       retParamPtr = NULL;
    size_t           retValue_size_t;
    gctINT           status;

    gcmHEADER_ARG("Image=0x%x ParamName=%u ParamValueSize=%lu ParamValue=0x%x",
                  Image, ParamName, ParamValueSize, ParamValue);
    gcmDUMP_API("${OCL clGetImageInfo 0x%x, 0x%x}", Image, ParamName);

    if (Image == gcvNULL ||
        Image->objectType != clvOBJECT_MEM ||
        (Image->type != CL_MEM_OBJECT_IMAGE2D &&
         Image->type != CL_MEM_OBJECT_IMAGE3D &&
         Image->type != CL_MEM_OBJECT_IMAGE1D &&
         Image->type != CL_MEM_OBJECT_IMAGE1D_ARRAY &&
         Image->type != CL_MEM_OBJECT_IMAGE2D_ARRAY &&
         Image->type != CL_MEM_OBJECT_IMAGE1D_BUFFER))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004067: (clGetImageInfo) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    switch (ParamName) {
        case CL_IMAGE_FORMAT:
            retParamSize = gcmSIZEOF(Image->u.image.format);
            retParamPtr = &(Image->u.image.format);
            break;

        case CL_IMAGE_ELEMENT_SIZE:
            /*retParamSize = gcmSIZEOF(Image->u.image.elementSize);
            retParamPtr = &(Image->u.image.elementSize);*/
            retValue_size_t = Image->u.image.elementSize;
            retParamSize = gcmSIZEOF(retValue_size_t);
            retParamPtr = &retValue_size_t;
            break;

        case CL_IMAGE_ROW_PITCH:
            /*retParamSize = gcmSIZEOF(Image->u.image.rowPitch);
            retParamPtr = &(Image->u.image.rowPitch);*/
            retValue_size_t = Image->u.image.rowPitch;
            retParamSize = gcmSIZEOF(retValue_size_t);
            retParamPtr = &retValue_size_t;
            break;

        case CL_IMAGE_SLICE_PITCH:
            /*retParamSize = gcmSIZEOF(Image->u.image.slicePitch);
            retParamPtr = &(Image->u.image.slicePitch);*/
            retValue_size_t = Image->u.image.slicePitch;
            if(Image->type == CL_MEM_OBJECT_IMAGE1D || Image->type == CL_MEM_OBJECT_IMAGE2D
                || Image->type == CL_MEM_OBJECT_IMAGE1D_BUFFER)
                retValue_size_t = 0;
            retParamSize = gcmSIZEOF(retValue_size_t);
            retParamPtr = &retValue_size_t;
            break;

        case CL_IMAGE_WIDTH:
            /*retParamSize = gcmSIZEOF(Image->u.image.width);
            retParamPtr = &(Image->u.image.width);*/
            retValue_size_t = Image->u.image.width;
            retParamSize = gcmSIZEOF(retValue_size_t);
            retParamPtr = &retValue_size_t;
            break;

        case CL_IMAGE_HEIGHT:
            /*retParamSize = gcmSIZEOF(Image->u.image.height);
            retParamPtr = &(Image->u.image.height);*/
            retValue_size_t = Image->u.image.height;
            if(Image->type == CL_MEM_OBJECT_IMAGE1D || Image->type == CL_MEM_OBJECT_IMAGE1D_ARRAY
                || Image->type == CL_MEM_OBJECT_IMAGE1D_BUFFER)
                retValue_size_t = 0;
            retParamSize = gcmSIZEOF(retValue_size_t);
            retParamPtr = &retValue_size_t;
            break;

        case CL_IMAGE_DEPTH:
            /*retParamSize = gcmSIZEOF(Image->u.image.depth);
            retParamPtr = &(Image->u.image.depth);*/
            retValue_size_t = Image->u.image.depth;
            if(Image->type == CL_MEM_OBJECT_IMAGE1D || Image->type == CL_MEM_OBJECT_IMAGE1D_ARRAY
                || Image->type == CL_MEM_OBJECT_IMAGE1D_BUFFER || Image->type == CL_MEM_OBJECT_IMAGE2D
                || Image->type == CL_MEM_OBJECT_IMAGE2D_ARRAY)
                retValue_size_t = 0 ;
            retParamSize = gcmSIZEOF(retValue_size_t);
            retParamPtr = &retValue_size_t;
            break;

        case CL_IMAGE_ARRAY_SIZE:
            retValue_size_t = Image->u.image.arraySize;
            if (Image->type != CL_MEM_OBJECT_IMAGE1D_ARRAY && Image->type != CL_MEM_OBJECT_IMAGE2D_ARRAY)
                retValue_size_t = 0;
            retParamSize = gcmSIZEOF(retValue_size_t);
            retParamPtr = &retValue_size_t;
            break;

        case CL_IMAGE_NUM_MIP_LEVELS:
        case CL_IMAGE_NUM_SAMPLES:
            retValue_size_t = 0;
            retParamSize = gcmSIZEOF((cl_uint)retValue_size_t);
            retParamPtr = &retValue_size_t;
            break;

        case CL_IMAGE_BUFFER:
            retParamSize = gcmSIZEOF(Image->u.image.buffer);
            retParamPtr = &(Image->u.image.buffer);
            break;

        default:
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-004035: (clGetImageInfo) invalid ParamName (0x%x).\n",
                ParamName);
            clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (ParamValue) {
        if (ParamValueSize < retParamSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-004036: (clGetImageInfo) ParamValueSize (%d) is less than required size (%d).\n",
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

    VCL_TRACE_API(GetImageInfo)(Image, ParamName, ParamValueSize, ParamValue, ParamValueSizeRet);
    gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(ParamValueSizeRet));
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clSetMemObjectDestructorCallback(
    cl_mem      MemObj,
    void        (CL_CALLBACK * PfnNotify)(cl_mem, void *),
    void *      UserData
    )
{
    clsMemObjCallback_PTR   memObjCallback;
    gctPOINTER              pointer;
    gctINT                  status;

    gcmHEADER_ARG("MemObj=0x%x PfnNotify=0x%x UserData=0x%x",
                  MemObj, PfnNotify, UserData);
    gcmDUMP_API("${OCL clSetMemObjectDestructorCallback 0x%x}", MemObj);

    if (MemObj == gcvNULL || MemObj->objectType != clvOBJECT_MEM)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004037: (clSetMemObjectDestructorCallback) invalid MemObj.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (PfnNotify == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004038: (clSetMemObjectDestructorCallback) PfnNotify is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    status = gcoOS_Allocate(gcvNULL, sizeof(clsMemObjCallback), &pointer);
    if (gcmIS_ERROR(status))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-004039: (clSetMemObjectDestructorCallback) Cannot allocate memory.\n");
        clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }

    memObjCallback              = (clsMemObjCallback_PTR) pointer;
    memObjCallback->pfnNotify   = PfnNotify;
    memObjCallback->userData    = UserData;
    memObjCallback->next        = MemObj->memObjCallback;
    MemObj->memObjCallback      = memObjCallback;

    VCL_TRACE_API(SetMemObjectDestructorCallback)(MemObj, PfnNotify, UserData);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}
