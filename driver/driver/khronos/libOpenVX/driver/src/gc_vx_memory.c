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


#include <gc_vx_common.h>
#include <gc_hal_user_precomp.h>
#include <gc_hal_vx.h>

VX_INTERNAL_API vx_status vxoMemory_CAllocate(vx_context context, void** memory, vx_uint32 size)
{
    gcoOS_AllocateMemory(gcvNULL, size, memory);

    context->memoryCount ++;

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoMemory_CFree(vx_context context, void** memory)
{
    vxAcquireMutex(context->base.lock);

    gcoOS_FreeMemory(gcvNULL, *memory);

    *memory = NULL;

    context->memoryCount --;

    vxReleaseMutex(context->base.lock);

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_bool vxoMemory_Allocate(vx_context context, vx_memory memory)
{
    vx_int32 planeIndex, dimIndex;

    vxmASSERT(context);
    vxmASSERT(memory);

    if (memory->allocated) return vx_true_e;

    memory->allocated = vx_true_e;

    for (planeIndex = 0; (vx_uint32) planeIndex < memory->planeCount; planeIndex++)
    {
        vx_size     size = sizeof(vx_uint8);
        gceSTATUS   status;

        if (memory->strides[planeIndex][VX_DIM_CHANNEL] != 0)
        {
            size = (vx_size)abs(memory->strides[planeIndex][VX_DIM_CHANNEL]);
        }

        for (dimIndex = 0; (vx_uint32)dimIndex < memory->dimCount; dimIndex++)
        {
            memory->strides[planeIndex][dimIndex] = (vx_int32)size;
            size *= (vx_size)abs(memory->dims[planeIndex][dimIndex]);
        }

        status = gcoVX_AllocateMemory((gctUINT32)size, (gctPOINTER*)&memory->logicals[planeIndex],
                                        &memory->physicals[planeIndex],
                                        &memory->nodePtrs[planeIndex]);
        context->memoryCount++;

        if (gcmIS_ERROR(status)) goto ErrorExit;

        vxmASSERT(memory->logicals[planeIndex]);

        if (!vxCreateMutex(OUT &memory->writeLocks[planeIndex]))
        {
            memory->writeLocks[planeIndex] = VX_NULL;
            planeIndex++;
            goto ErrorExit;
        }
    }

    memory->allocated = vx_true_e;

    vxoMemory_Dump(memory);

    return vx_true_e;

ErrorExit:
    for (planeIndex = planeIndex - 1; planeIndex >= 0; planeIndex--)
    {
        if (memory->logicals[planeIndex] != VX_NULL)
        {
            gcoVX_FreeMemory((gcsSURF_NODE_PTR)memory->nodePtrs[planeIndex]);
            memory->logicals[planeIndex]    = VX_NULL;
            memory->nodePtrs[planeIndex]    = VX_NULL;
        }

        if (memory->writeLocks[planeIndex] != VX_NULL)
        {
            vxDestroyMutex(memory->writeLocks[planeIndex]);
            memory->writeLocks[planeIndex]  = VX_NULL;
        }
    }

    memory->allocated = vx_false_e;

    return vx_false_e;
}

VX_INTERNAL_API vx_bool vxoMemory_Free(vx_context context, vx_memory memory)
{
    vx_uint32 planeIndex;

    vxmASSERT(context);
    vxmASSERT(memory);

    if (!memory->allocated) return vx_true_e;

    vxoMemory_Dump(memory);

    for (planeIndex = 0; planeIndex < memory->planeCount; planeIndex++)
    {
        if (memory->logicals[planeIndex] != VX_NULL)
        {
            gcoVX_FreeMemory((gcsSURF_NODE_PTR)memory->nodePtrs[planeIndex]);
            memory->logicals[planeIndex]    = VX_NULL;
            memory->nodePtrs[planeIndex]    = VX_NULL;

            context->memoryCount --;
        }

        if (memory->writeLocks[planeIndex] != VX_NULL)
        {
            vxDestroyMutex(memory->writeLocks[planeIndex]);
            memory->writeLocks[planeIndex]  = VX_NULL;
        }
    }

    memory->allocated = vx_false_e;

    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoMemory_WrapUserMemory(vx_context context, vx_memory memory)
{
    vx_int32 planeIndex, dimIndex;

    vxmASSERT(context);
    vxmASSERT(memory);

    if (memory->allocated) return vx_true_e;

    memory->allocated = vx_true_e;

    for (planeIndex = 0; (vx_uint32) planeIndex < memory->planeCount; planeIndex++)
    {
        gctUINT32    size = sizeof(vx_uint8);
        gceSTATUS   status;

        gcsUSER_MEMORY_DESC desc;

        if (memory->strides[planeIndex][VX_DIM_CHANNEL] != 0)
        {
            size = (gctUINT32)abs(memory->strides[planeIndex][VX_DIM_CHANNEL]);
        }

        for (dimIndex = 0; (vx_uint32)dimIndex < memory->dimCount; dimIndex++)
        {
            memory->strides[planeIndex][dimIndex] = (gctUINT32)size;
            size *= (gctUINT32)abs(memory->dims[planeIndex][dimIndex]);
        }

        gcoOS_ZeroMemory(&desc, gcmSIZEOF(desc));

        desc.flag     = memory->wrapFlag;
        desc.logical  = gcmPTR_TO_UINT64(memory->logicals[planeIndex]);
        desc.physical = gcvINVALID_ADDRESS;
        desc.size     = (gctUINT32)size;

        memory->wrappedSize[planeIndex] = (gctUINT32)size;

        /* Map the host ptr to a vidmem node. */
        status = gcoHAL_WrapUserMemory(&desc,
                              &memory->wrappedNode[planeIndex]);

        if (gcmIS_ERROR(status)) goto ErrorExit;

        /* Get the physical address. */
        status = gcoHAL_LockVideoMemory(memory->wrappedNode[planeIndex],
                               gcvFALSE,
                               gcvENGINE_RENDER,
                               &memory->physicals[planeIndex],
                               gcvNULL);

        if (gcmIS_ERROR(status)) goto ErrorExit;

        if (!vxCreateMutex(OUT &memory->writeLocks[planeIndex]))
        {
            memory->writeLocks[planeIndex] = VX_NULL;
            planeIndex++;
            goto ErrorExit;
        }
    }

    memory->allocated = vx_true_e;

    vxoMemory_Dump(memory);

    return vx_true_e;

ErrorExit:
    for (planeIndex = planeIndex - 1; planeIndex >= 0; planeIndex--)
    {
        if (memory->wrappedNode[planeIndex] != 0)
        {
            gcmVERIFY_OK(gcoHAL_UnlockVideoMemory(
                            memory->wrappedNode[planeIndex],
                            gcvSURF_BITMAP,
                            gcvENGINE_RENDER));

            gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(
                            memory->wrappedNode[planeIndex]));

            memory->logicals[planeIndex]     = VX_NULL;
            memory->wrappedNode[planeIndex]  = 0;
        }

        if (memory->writeLocks[planeIndex] != VX_NULL)
        {
            vxDestroyMutex(memory->writeLocks[planeIndex]);
            memory->writeLocks[planeIndex]  = VX_NULL;
        }
    }

    memory->allocated = vx_false_e;

    return vx_false_e;
}

VX_INTERNAL_API vx_bool vxoMemory_FreeWrappedMemory(vx_context context, vx_memory memory)
{
    vx_uint32 planeIndex;

    vxmASSERT(context);
    vxmASSERT(memory);

    if (!memory->allocated) return vx_true_e;

    vxoMemory_Dump(memory);

    for (planeIndex = 0; planeIndex < memory->planeCount; planeIndex++)
    {
        if (memory->wrappedNode[planeIndex] != 0)
        {
            gcmVERIFY_OK(gcoHAL_UnlockVideoMemory(
                            memory->wrappedNode[planeIndex],
                            gcvSURF_BITMAP,
                            gcvENGINE_RENDER));

            gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(
                            memory->wrappedNode[planeIndex]));

            memory->logicals[planeIndex]     = VX_NULL;
            memory->wrappedNode[planeIndex]  = 0;
        }

        if (memory->writeLocks[planeIndex] != VX_NULL)
        {
            vxDestroyMutex(memory->writeLocks[planeIndex]);
            memory->writeLocks[planeIndex]  = VX_NULL;
        }
    }

    memory->allocated = vx_false_e;

    return vx_true_e;
}

VX_INTERNAL_API void vxoMemory_Dump(vx_memory memory)
{
    vx_uint32 planeIndex, dimIndex;

    if (memory == VX_NULL)
    {
        vxTrace(VX_TRACE_MEMORY, "<memory>null</memory>\n");
    }
    else
    {
        vxTrace(VX_TRACE_MEMORY,
                "<memory>\n"
                "    <address>"VX_FORMAT_HEX"</address>\n"
                "    <planeCount>%u</planeCount>\n"
                "    <planes>",
                memory, memory->planeCount);

        for (planeIndex = 0; planeIndex < memory->planeCount; planeIndex++)
        {
            vxTrace(VX_TRACE_MEMORY, "        <plane%d>\n", planeIndex);

            for (dimIndex = 0; dimIndex < memory->dimCount; dimIndex++)
            {
                vxTrace(VX_TRACE_MEMORY,
                        "            dims[%d]=%d\n",
                        dimIndex, memory->dims[planeIndex][dimIndex]);

                vxTrace(VX_TRACE_MEMORY,
                        "            strides[%d]=%d\n",
                        dimIndex, memory->strides[planeIndex][dimIndex]);
            }

            vxTrace(VX_TRACE_MEMORY, "        </plane%d>\n", planeIndex);
        }

        vxTrace(VX_TRACE_MEMORY, "    </planes>");

        vxTrace(VX_TRACE_MEMORY,
                "    <allocated>%s</allocated>",
                vxmBOOL_TO_STRING(memory->allocated));

        vxTrace(VX_TRACE_MEMORY, "</memory>");
    }
}

VX_INTERNAL_API vx_size vxoMemory_ComputeSize(vx_memory memory, vx_uint32 planeIndex)
{
    vxmASSERT(memory);
    vxmASSERT(planeIndex < memory->planeCount);

    if (memory->dimCount == 0) return 0;

    return memory->dims[planeIndex][memory->dimCount - 1] * memory->strides[planeIndex][memory->dimCount - 1];
}

VX_INTERNAL_API vx_size vxoMemory_ComputeElementCount(vx_memory memory, vx_uint32 planeIndex)
{
    vx_uint32 index;
    vx_uint32 elementCount = 1;

    vxmASSERT(memory);
    vxmASSERT(planeIndex < memory->planeCount);

    if (memory->dimCount == 0) return 0;

    for (index = 0; index < memory->dimCount; index++)
    {
        elementCount *= memory->dims[planeIndex][index];
    }

    return elementCount;
}

