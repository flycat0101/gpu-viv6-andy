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


#include <gc_vx_common.h>
#include <gc_hal_user_precomp.h>
#include <gc_hal_vx.h>

#define _GC_OBJ_ZONE            gcdZONE_VX_MEMORY

VX_PRIVATE_API vx_bool _pool_get_memory(vx_memory_pool pool, vx_size size, vx_uint8_ptr* logical, vx_uint32_ptr physical, gcsSURF_NODE_PTR* node);
VX_PRIVATE_API vx_bool _pool_put_memory(vx_memory_pool pool, gcsSURF_NODE_PTR node);

VX_PRIVATE_API vx_bool _AllocateMemory(vx_context context, vx_memory memory, vx_bool virt)
{
    vx_int32 planeIndex, dimIndex;

    gcmHEADER_ARG("context=%p, memory=%p, virt=0x%x", context, memory, virt);
    vxmASSERT(context);
    vxmASSERT(memory);

    if (memory->allocated)
    {
        gcmFOOTER_ARG("%d", vx_true_e);
        return vx_true_e;
    }
    memory->allocated = vx_true_e;

    for (planeIndex = 0; (vx_uint32) planeIndex < memory->planeCount; planeIndex++)
    {
        gceSTATUS status;

        if (memory->sizes[planeIndex] == 0)
        {
            vx_uint32 size = sizeof(vx_uint8);

            if (memory->strides[planeIndex][VX_DIM_CHANNEL] != 0)
            {
                size = (vx_size)abs(memory->strides[planeIndex][VX_DIM_CHANNEL]);
            }

            for (dimIndex = 0; (vx_uint32)dimIndex < memory->dimCount; dimIndex++)
            {
                memory->strides[planeIndex][dimIndex] = (vx_int32)size;
                size *= (vx_size)abs(memory->dims[planeIndex][dimIndex]);
            }

            memory->sizes[planeIndex] = size;
        }

        {
            if (context->options.enableMemPool && virt && memory->graph != VX_NULL && memory->graph->memoryPool != VX_NULL &&
                _pool_get_memory(memory->graph->memoryPool,
                                 gcmALIGN(memory->sizes[planeIndex], 64),
                                 &memory->logicals[planeIndex],
                                 &memory->physicals[planeIndex],
                                 &memory->nodePtrs[planeIndex]))
            {
                memory->allocType = VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR;
                status = (gceSTATUS)VX_SUCCESS;
            }
            else
            {
                status = gcoVX_AllocateMemory((gctUINT32)memory->sizes[planeIndex], (gctPOINTER*)&memory->logicals[planeIndex],
                                                &memory->physicals[planeIndex],
                                                &memory->nodePtrs[planeIndex]);

                if (gcmIS_ERROR(status)) goto ErrorExit;
                vxmASSERT(memory->logicals[planeIndex]);

                memory->allocType = VXNNE_MEM_POOL_TYPE_ORIG_DDR;
                gcoOS_ZeroMemory(memory->logicals[planeIndex], memory->sizes[planeIndex]);
            }
        }

        context->memoryCount++;

        if (!vxCreateMutex(OUT &memory->writeLocks[planeIndex]))
        {
            memory->writeLocks[planeIndex] = VX_NULL;
            planeIndex++;
            goto ErrorExit;
        }
    }

    memory->allocated = vx_true_e;

    vxoMemory_Dump(memory);

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;

ErrorExit:
    for (planeIndex = planeIndex - 1; planeIndex >= 0; planeIndex--)
    {
        if (memory->logicals[planeIndex] != VX_NULL)
        {
            {
                gcoVX_FreeMemory((gcsSURF_NODE_PTR)memory->nodePtrs[planeIndex]);
            }
            memory->logicals[planeIndex]    = VX_NULL;
            memory->nodePtrs[planeIndex]    = VX_NULL;
        }

        if (memory->writeLocks[planeIndex] != VX_NULL)
        {
            vxDestroyMutex(memory->writeLocks[planeIndex]);
            memory->writeLocks[planeIndex]  = VX_NULL;
        }

        memory->sizes[planeIndex] = 0;
    }

    memory->allocated = vx_false_e;

    gcmFOOTER_ARG("%d", vx_false_e);
    return vx_false_e;
}

VX_INTERNAL_API vx_bool vxoMemory_Allocate(vx_context context, vx_memory memory)
{
    return _AllocateMemory(context, memory, vx_false_e);
}

VX_INTERNAL_API vx_bool vxoMemory_AllocateEx(vx_context context, vx_memory memory)
{
    return _AllocateMemory(context, memory, vx_true_e);
}

VX_INTERNAL_API vx_bool vxoMemory_AllocateSize(vx_context context, vx_memory memory, vx_size size)
{
    gcmHEADER_ARG("context=%p, memory=%p, size=0x%lx", context, memory, size);
    vxmASSERT(size);
    vxmASSERT(memory);

    memory->planeCount = 1;
    memory->dimCount = 1;
    memory->dims[0][0] = (vx_int32)size;
    memory->strides[0][VX_DIM_CHANNEL] = 0;

    gcmFOOTER_NO();
    return vxoMemory_Allocate(context, memory);
}

VX_PRIVATE_API vx_bool _FreeMemory(vx_context context, vx_memory memory, vx_bool virt)
{
    vx_uint32 planeIndex;
    gcmHEADER_ARG("context=%p, memory=%p, virt=0x%x", context, memory, virt);
    vxmASSERT(context);
    vxmASSERT(memory);

    if (!memory->allocated)
    {
        gcmFOOTER_ARG("%d", vx_true_e);
        return vx_true_e;
    }
    vxoMemory_Dump(memory);

    for (planeIndex = 0; planeIndex < memory->planeCount; planeIndex++)
    {
        if (memory->allocType == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR &&
            memory->graph != VX_NULL && memory->graph->memoryPool != VX_NULL)
        {
            _pool_put_memory(memory->graph->memoryPool, (gcsSURF_NODE_PTR)memory->nodePtrs[planeIndex]);
            context->memoryCount--;
        }
        else if (memory->logicals[planeIndex] != VX_NULL)
        {
            {
                gcoVX_FreeMemory((gcsSURF_NODE_PTR)memory->nodePtrs[planeIndex]);
            }
            memory->logicals[planeIndex]    = VX_NULL;
            memory->nodePtrs[planeIndex]    = VX_NULL;
            context->memoryCount --;
        }

        if (memory->writeLocks[planeIndex] != VX_NULL)
        {
            vxDestroyMutex(memory->writeLocks[planeIndex]);
            memory->writeLocks[planeIndex]  = VX_NULL;
        }

        memory->sizes[planeIndex] = 0;
    }

    memory->allocated = vx_false_e;

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoMemory_Free(vx_context context, vx_memory memory)
{
    return _FreeMemory(context, memory, vx_false_e);
}

VX_INTERNAL_API vx_bool vxoMemory_FreeEx(vx_context context, vx_memory memory)
{
    return _FreeMemory(context, memory, vx_true_e);
}

VX_INTERNAL_API vx_bool vxoMemory_WrapUserMemory(vx_context context, vx_memory memory)
{
    vx_int32 planeIndex;

    gcmHEADER_ARG("context=%p, memory=%p", context, memory);
    vxmASSERT(context);
    vxmASSERT(memory);

    if (memory->allocated)
    {
        gcmFOOTER_ARG("%d", vx_true_e);
        return vx_true_e;
    }
    memory->allocated = vx_true_e;

    for (planeIndex = 0; (vx_uint32) planeIndex < memory->planeCount; planeIndex++)
    {
        gceSTATUS status, dimIndex;

        gcsUSER_MEMORY_DESC desc;

        if (memory->sizes[planeIndex] == 0)
        {
            gctUINT32 size = sizeof(vx_uint8);

            if (memory->strides[planeIndex][VX_DIM_CHANNEL] != 0)
            {
                size = (gctUINT32)abs(memory->strides[planeIndex][VX_DIM_CHANNEL]);
            }

            for (dimIndex = 0; (vx_uint32)dimIndex < memory->dimCount; dimIndex++)
            {
                memory->strides[planeIndex][dimIndex] = (gctUINT32)size;
                size *= (gctUINT32)abs(memory->dims[planeIndex][dimIndex]);
            }

            memory->sizes[planeIndex] = size;
        }
        gcoOS_ZeroMemory(&desc, gcmSIZEOF(desc));

        desc.flag     = memory->wrapFlag;
        desc.logical  = gcmPTR_TO_UINT64(memory->logicals[planeIndex]);
        desc.physical = gcvINVALID_PHYSICAL_ADDRESS;

        /*TODO: add more check to make sure memory size 4k alignment*/
        desc.size     = gcmALIGN((gctUINT32)memory->sizes[planeIndex], 0x1000);

        /*page size alignment for CPU*/
        if(desc.logical & 0x3f || desc.size & 0xfff ) goto ErrorExit;
        memory->sizes[planeIndex] = desc.size;
        memory->wrappedSize[planeIndex] = desc.size;

        /* Map the host ptr to a vidmem node. */
        status = gcoHAL_WrapUserMemory(&desc,
                              gcvVIDMEM_TYPE_BITMAP,
                              &memory->wrappedNode[planeIndex]);

        if (gcmIS_ERROR(status)) goto ErrorExit;

        /* Get the physical address. */
        status = gcoHAL_LockVideoMemory(memory->wrappedNode[planeIndex],
                               gcvTRUE,
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

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;

ErrorExit:
    for (planeIndex = planeIndex - 1; planeIndex >= 0; planeIndex--)
    {
        if (memory->wrappedNode[planeIndex] != 0)
        {
            gcmVERIFY_OK(gcoHAL_UnlockVideoMemoryEX(
                            memory->wrappedNode[planeIndex],
                            gcvVIDMEM_TYPE_BITMAP,
                            gcvENGINE_RENDER, gcvTRUE));

            gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(
                            memory->wrappedNode[planeIndex]));

            memory->wrappedNode[planeIndex]  = 0;
        }

        if (memory->writeLocks[planeIndex] != VX_NULL)
        {
            vxDestroyMutex(memory->writeLocks[planeIndex]);
            memory->writeLocks[planeIndex]  = VX_NULL;
        }

        memory->sizes[planeIndex] = 0;
    }

    memory->allocated = vx_false_e;

    gcmFOOTER_ARG("%d", vx_false_e);
    return vx_false_e;
}

VX_INTERNAL_API vx_bool vxoMemory_FreeWrappedMemory(vx_context context, vx_memory memory)
{
    vx_uint32 planeIndex;

    gcmHEADER_ARG("context=%p, memory=%p", context, memory);
    vxmASSERT(context);
    vxmASSERT(memory);

    if (!memory->allocated)
    {
        gcmFOOTER_ARG("%d", vx_true_e);
        return vx_true_e;
    }
    vxoMemory_Dump(memory);

    for (planeIndex = 0; planeIndex < memory->planeCount; planeIndex++)
    {
        if (memory->wrappedNode[planeIndex] != 0)
        {
            gcmVERIFY_OK(gcoHAL_UnlockVideoMemoryEX(
                            memory->wrappedNode[planeIndex],
                            gcvVIDMEM_TYPE_BITMAP,
                            gcvENGINE_RENDER, gcvTRUE));

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

        memory->sizes[planeIndex] = 0;
    }

    memory->allocated = vx_false_e;

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

VX_INTERNAL_API void vxoMemory_Dump(vx_memory memory)
{
    vx_uint32 planeIndex, dimIndex;

    gcmHEADER_ARG("memory=%p", memory);

    if (memory == VX_NULL)
    {
        vxTrace(VX_TRACE_MEMORY, "<memory>null</memory>\n");
    }
    else
    {
        vxTrace(VX_TRACE_MEMORY,
                "<memory>\n"
                "    <address>%p</address>\n"
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
    gcmFOOTER_NO();
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

    gcmHEADER_ARG("memory=%p, planeIndex=0x%x", memory, planeIndex);
    vxmASSERT(memory);
    vxmASSERT(planeIndex < memory->planeCount);

    if (memory->dimCount == 0)
    {
        gcmFOOTER_ARG("%d", 0);
        return 0;
    }
    for (index = 0; index < memory->dimCount; index++)
    {
        elementCount *= memory->dims[planeIndex][index];
    }

    gcmFOOTER_ARG("0x%x", elementCount);
    return elementCount;
}

/*****************************************************************************************************/
/*****************************************************************************************************/
VX_PRIVATE_API vx_memory_pool_item _pool_get_emtpy_item(vx_memory_pool pool)
{
    gcmHEADER_ARG("pool=%p", pool);
    if (pool->count >= VX_MAX_MEMPOOL_ITEM_NUM)
    {
        /* re-arrange to reserve more empty space in pool array */
        vx_uint32 i, j;

        for (i = 0; i < VX_MAX_MEMPOOL_ITEM_NUM; i++)
        {
            if (pool->pool[i].index == VX_MEMPOOL_ITEM_INVALID)
                break;
        }

        if (i == VX_MAX_MEMPOOL_ITEM_NUM)
        {
            gcmFOOTER_NO();
            return VX_NULL;
        }
        else
        {
            for (i = i+1, j = i; i < VX_MAX_MEMPOOL_ITEM_NUM; i++)
            {
                if (pool->pool[i].index != VX_MEMPOOL_ITEM_INVALID && j < VX_MAX_MEMPOOL_ITEM_NUM)
                {
                    vx_memory_pool_item niptr = &pool->pool[j];
                    gcoOS_MemCopy(niptr, &pool->pool[i], sizeof(vx_memory_pool_item_s));
                    if (niptr->prev != VX_NULL)
                    {
                        niptr->prev->next = niptr;
                    }
                    else if (niptr->allocated)
                    {
                        pool->allocHead = niptr;
                    }
                    else
                    {
                        pool->freeHead = niptr;
                    }
                    if (niptr->next != VX_NULL)
                    {
                        niptr->next->prev = niptr;
                    }
                    niptr->index = j;
                    j++;
                    gcoOS_MemFill(&pool->pool[i], 0, sizeof(vx_memory_pool_item_s));
                }
            }
            pool->count = j;
        }
    }

    if (pool->count >= VX_MAX_MEMPOOL_ITEM_NUM)
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }

    gcoOS_MemFill(&pool->pool[pool->count], 0, sizeof(vx_memory_pool_item_s));
    pool->pool[pool->count].index = pool->count;
    pool->count++;
    gcmFOOTER_NO();
    return &pool->pool[pool->count-1];
}

VX_PRIVATE_API vx_bool _pool_get_memory(vx_memory_pool pool, vx_size size, vx_uint8_ptr* logical, vx_uint32_ptr physical, gcsSURF_NODE_PTR* node)
{
    vx_memory_pool_item piptr = VX_NULL, fiptr, aiptr;

    gcmHEADER_ARG("pool=%p, size=0x%x, logical=%p, physical=%p, node=%p", pool, size, logical, physical, node);
    vxmASSERT(size);
    vxmASSERT(node);

    if (pool->locked) {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }
    fiptr = pool->freeHead;
    aiptr = pool->allocHead;

    while (fiptr != VX_NULL)
    {
        if (fiptr->size >= size)
            break;
        piptr = fiptr;
        fiptr = fiptr->next;
    }

    if (fiptr == VX_NULL)
    {
        if (!pool->memExpandMode)
        {
            gcmFOOTER_ARG("%d", vx_false_e);
            return vx_false_e;
        }
        else
        {
            /* enlarge pool size in pre-allocated mode */
            if (piptr != VX_NULL && piptr->offset + piptr->size == pool->size)
            {
                pool->size += size - piptr->size;
            }
            else
            {
                if ((piptr = _pool_get_emtpy_item(pool)) == VX_NULL)
                {
                    gcmFOOTER_ARG("%d", vx_false_e);
                    return vx_false_e;
                }
                pool->freeHead = piptr;
                piptr->offset = pool->size;
                pool->size += size;
            }
            piptr->size = size;
            fiptr = piptr;
        }
    }

    /* remove from freed list */
    if (fiptr->prev != VX_NULL) fiptr->prev->next = fiptr->next;
    if (fiptr->next != VX_NULL) fiptr->next->prev = fiptr->prev;

    if (fiptr->size > size)
    {
        vx_memory_pool_item niptr;

        if ((niptr = _pool_get_emtpy_item(pool)) == VX_NULL)
        {
            gcmFOOTER_ARG("%d", vx_false_e);
            return vx_false_e;
        }

        niptr->offset = fiptr->offset + size;
        niptr->size = fiptr->size - size;
        niptr->allocated = vx_false_e;

        niptr->next = fiptr->next;
        niptr->prev = fiptr->prev;

        if (fiptr->prev != VX_NULL) fiptr->prev->next = niptr;
        else pool->freeHead = niptr;
        if (fiptr->next != VX_NULL) fiptr->next->prev = niptr;

        fiptr->size = size;
    }
    else
    {
        pool->freeHead = VX_NULL;
    }

    /* add to allocated list */
    fiptr->next = aiptr;
    fiptr->allocated = vx_true_e;
    if (aiptr != VX_NULL)
    {
        aiptr->prev = fiptr;
    }
    else
    {
        pool->allocHead = fiptr;
    }
    pool->allocHead = fiptr;

    if (!pool->memExpandMode)
    {
        if (logical != VX_NULL)
        {
            *logical = pool->logical + fiptr->offset;
            gcoOS_ZeroMemory(*logical, size);
        }
        if (physical != VX_NULL)
            *physical = pool->physical + (vx_uint32)fiptr->offset;
        *node = (gcsSURF_NODE_PTR)fiptr;
    }
    else
    {
        if (logical != VX_NULL)
            *logical = (vx_uint8_ptr)fiptr->offset;
        if (physical != VX_NULL)
            *physical = (vx_uint32)fiptr->offset;
        *node = (gcsSURF_NODE_PTR)fiptr;
    }

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

VX_PRIVATE_API vx_bool _pool_put_memory(vx_memory_pool pool, gcsSURF_NODE_PTR node)
{
    vx_memory_pool_item ciptr;

    gcmHEADER_ARG("pool=%p, node=%p", pool, node);
    vxmASSERT(node);

    if (pool->locked)
    {
        gcmFOOTER_ARG("%d", vx_true_e);
        return vx_true_e;
    }
    ciptr = (vx_memory_pool_item)node;

    /* remove from allocated list */
    if (ciptr->prev != VX_NULL)
    {
        ciptr->prev->next = ciptr->next;
        ciptr->prev = VX_NULL;
    }
    else
    {
        pool->allocHead = ciptr->next;
    }

    if (ciptr->next != VX_NULL)
    {
        ciptr->next->prev = ciptr->prev;
        ciptr->next = VX_NULL;
    }

    /* add to freed list and adjust */
    if (pool->freeHead == VX_NULL)
    {
        pool->freeHead = ciptr;
        ciptr->next = ciptr->prev = VX_NULL;
        ciptr->allocated = vx_false_e;
    }
    else
    {
        vx_memory_pool_item fiptr = pool->freeHead;
        vx_memory_pool_item nipter;

        if (ciptr->offset + ciptr->size < fiptr->offset)
        {
            pool->freeHead->prev = ciptr;
            ciptr->next = pool->freeHead;
            pool->freeHead = ciptr;
            ciptr->allocated = vx_false_e;
        }
        else if (ciptr->offset + ciptr->size == fiptr->offset)
        {
            /* merge and invalid ciptr */
            fiptr->offset = ciptr->offset;
            fiptr->size += ciptr->size;
            ciptr->index = VX_MEMPOOL_ITEM_INVALID;
        }
        else
        {
            while (fiptr->next != VX_NULL)
            {
                if (ciptr->offset >= fiptr->offset + fiptr->size &&
                    ciptr->offset + ciptr->size <= fiptr->next->offset)
                {
                    break;
                }
                fiptr = fiptr->next;
            }

            nipter = fiptr->next;

            /* check prev node and link/merge */
            if (ciptr->offset == fiptr->offset + fiptr->size)
            {
                fiptr->size += ciptr->size;
                ciptr->index = VX_MEMPOOL_ITEM_INVALID;
                ciptr = fiptr;
            }
            else
            {
                fiptr->next = ciptr;
                ciptr->prev = fiptr;
                ciptr->allocated = vx_false_e;
            }

            /* check next node and link/merge */
            fiptr = nipter;
            if (fiptr != VX_NULL && ciptr->offset + ciptr->size == fiptr->offset)
            {
                ciptr->size += fiptr->size;
                fiptr->index = VX_MEMPOOL_ITEM_INVALID;
                ciptr->next = fiptr->next;
                if (fiptr->next != VX_NULL) fiptr->next->prev = ciptr;
                fiptr->prev = fiptr->next = VX_NULL;
            }
            else
            {
                ciptr->next = fiptr;
                if (fiptr != VX_NULL) fiptr->prev = ciptr;
            }
        }
    }

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoMemoryPool_Deinitialize(vx_graph graph)
{
    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);
    vxmASSERT(graph->memoryPool);

    if (graph->memoryPool->logical != VX_NULL)
    {
        gcoVX_FreeMemory((gcsSURF_NODE_PTR)graph->memoryPool->nodePtr);
    }

    gcoOS_FreeMemory(gcvNULL, graph->memoryPool);
    graph->memoryPool = VX_NULL;

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoMemoryPool_Initialize(vx_graph graph, vx_size size)
{
    gceSTATUS status;
    vx_memory_pool pointer = gcvNULL;

    gcmHEADER_ARG("graph=%p, size=0x%lx", graph, size);
    vxmASSERT(graph);

    status = gcoOS_Allocate(gcvNULL, gcmSIZEOF(struct _vx_memory_pool_s), (gctPOINTER *)&pointer);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }
    gcoOS_MemFill(pointer, 0, gcmSIZEOF(struct _vx_memory_pool_s));

    if (size != 0)
    {
        size += VX_MEMPOOL_RESERVE_MEM_SIZE;
        status = gcoVX_AllocateMemory((gctUINT32)size, (gctPOINTER*)&pointer->logical,
                                      &pointer->physical, &pointer->nodePtr);
        if (gcmIS_ERROR(status))
        {
            if(pointer) gcoOS_Free(NULL, (gctPOINTER)pointer);
            gcmFOOTER_ARG("%d", vx_false_e);
            return vx_false_e;
        }
        pointer->memExpandMode = vx_false_e;
    }
    else pointer->memExpandMode = vx_true_e;

    pointer->size = size;
    pointer->count = 1;
    pointer->allocHead = VX_NULL;
    pointer->freeHead = &pointer->pool[0];
    pointer->pool[0].index = 0;
    pointer->pool[0].size = size;
    pointer->pool[0].allocated = vx_false_e;
    pointer->pool[0].prev = pointer->pool[0].next = VX_NULL;

    graph->memoryPool = pointer;

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoMemoryPool_LockDown(vx_graph graph, vx_size size)
{
    gceSTATUS status;
    vx_memory_pool pool;

    gcmHEADER_ARG("graph=%p, size=0x%lx", graph, size);

    vxmASSERT(graph);
    pool = graph->memoryPool;
    vxmASSERT(pool);

    if (pool->locked)
    {
        gcmFOOTER_ARG("%d", vx_true_e);
        return vx_true_e;
    }
    if (!pool->memExpandMode)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }
    if (pool->size && size)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }
    if (!pool->size && !size)
    {
        gcmFOOTER_ARG("%d", vx_true_e);
        return vx_true_e;
    }

    size += pool->size + VX_MEMPOOL_RESERVE_MEM_SIZE;
    status = gcoVX_AllocateMemory((gctUINT32)size, (gctPOINTER*)&pool->logical,
                                  &pool->physical, (gcsSURF_NODE_PTR*)&pool->nodePtr);
    if (gcmIS_ERROR(status))
    {
        vxoMemoryPool_Deinitialize(graph);
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }

    pool->size = size;
    pool->locked = vx_true_e;

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoMemoryPool_Reset(vx_graph graph, vx_bool freeMem)
{
    vx_memory_pool pool;

    gcmHEADER_ARG("graph=%p, freeMem=0x%x", graph, freeMem);

    vxmASSERT(graph);
    pool = graph->memoryPool;
    vxmASSERT(pool);

    if (!pool->locked)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }
    if (freeMem)
    {
        gcoVX_FreeMemory((gcsSURF_NODE_PTR)graph->memoryPool->nodePtr);
        pool->size = 0;
    }

    pool->count = 1;
    pool->allocHead = VX_NULL;
    pool->freeHead = &pool->pool[0];
    pool->pool[0].index = 0;
    pool->pool[0].size = pool->size;
    pool->pool[0].allocated = vx_false_e;
    pool->pool[0].prev = pool->pool[0].next = VX_NULL;

    pool->memExpandMode = vx_false_e;
    pool->locked = vx_false_e;

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoMemoryPool_GetBase(vx_graph graph, vx_uint8_ptr* logical, vx_uint32* physical)
{
    if (graph->memoryPool == VX_NULL ||
        graph->memoryPool->nodePtr == VX_NULL ||
        !graph->memoryPool->locked)
    {
        return vx_false_e;
    }

    if (logical != VX_NULL) *logical = graph->memoryPool->logical;
    if (physical != VX_NULL) *physical = graph->memoryPool->physical;

    return vx_true_e;
}

/*******************************/
VX_INTERNAL_API vx_size
vxoMemory_GetSize(
    vx_memory memory
    )
{
    return memory->sizes[0];
}

VX_INTERNAL_API void
vxoMemory_SetSize(
    vx_memory memory,
    vx_size   size
    )
{
    memory->sizes[0] = size;
}

VX_INTERNAL_API vx_enum
vxoMemory_GetType(
    vx_memory memory
    )
{
    return memory->allocType;
}

VX_INTERNAL_API void
vxoMemory_SetType(
    vx_memory memory,
    vx_enum   type
    )
{
    memory->allocType = type;
}

VX_INTERNAL_API void
vxoMemory_SetAddress(
    vx_memory    memory,
    vx_uint8_ptr logical,
    vx_uint32    physical
    )
{
    memory->logicals[0] = logical;
    memory->physicals[0] = physical;
}

VX_INTERNAL_API void
vxoMemory_ResetOffset(
    vx_memory memory
    )
{
    memory->lastUseId = VXNNE_MEM_ID_INIT_VALUE;
    memory->firstUseId = VXNNE_MEM_ID_INIT_VALUE;
    memory->memOffset = 0;
    memory->memReverse = vx_false_e;
    memory->logicals[0] = VX_NULL;
    memory->physicals[0] = 0;
}

VX_PRIVATE_API vx_status
vxoMemoryPool_InitStack(
    vx_mempool_stack stack,
    vx_uint32 count,
    vx_mempool_stack* backup,
    vx_uint32 bcount
    )
{
    vx_status status = VX_SUCCESS;
    vx_ptr buffer = VX_NULL;

    buffer = vxAllocateAndZeroMemory(gcmSIZEOF(vx_mempool_stack_item_s) * (count + 2));
    vxmONERROR_NULLPTR(buffer);

    stack->buffer = (vx_mempool_stack_item)buffer;
    stack->count = count;
    stack->first = 0;
    stack->last = count + 1;

    if (backup != VX_NULL)
    {
        *backup = (vx_mempool_stack)vxAllocateAndZeroMemory(gcmSIZEOF(vx_mempool_stack_s) * bcount);
        vxmONERROR_NULLPTR(*backup);
    }

OnError:
    return status;
}

VX_PRIVATE_API vx_status
vxoMemoryPool_DeInitStack(
    vx_mempool_stack stack,
    vx_mempool_stack* backup,
    vx_uint32 bcount
    )
{
    if (stack->buffer != VX_NULL)
    {
        vxFree(stack->buffer);
        stack->buffer = VX_NULL;
    }

    stack->count = 0;
    stack->first = 0;
    stack->last = 1;

    if (backup != VX_NULL && *backup != VX_NULL)
    {
        vx_uint32 i;
        vx_mempool_stack_s* b = *backup;
        for (i = 0; i < bcount; i++)
        {
            if (b[i].buffer != VX_NULL)
            {
                vxFree(b[i].buffer);
                b[i].buffer = VX_NULL;
            }
        }
        vxFree(b);
        *backup = VX_NULL;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status
vxoMemoryPool_BackupStack(
    vx_mempool_stack stack,
    vx_mempool_stack_s* backup,
    vx_uint32 id
    )
{
    vx_status status = VX_SUCCESS;
    vx_ptr buffer = VX_NULL;

    if (backup[id].buffer == VX_NULL)
    {
        buffer = vxAllocateAndZeroMemory(gcmSIZEOF(vx_mempool_stack_item_s) * (stack->count + 2));
        vxmONERROR_NULLPTR(buffer);
        backup[id].buffer = (vx_mempool_stack_item)buffer;
    }

    vxMemCopy(backup[id].buffer, stack->buffer, gcmSIZEOF(vx_mempool_stack_item_s) * (stack->count + 2));
    backup[id].count = stack->count;
    backup[id].first = stack->first;
    backup[id].last = stack->last;

OnError:
    return status;
}

VX_PRIVATE_API vx_size
vxoMemoryPool_GetStackSize(
    vx_mempool_stack stack
    )
{
    return stack->buffer[stack->first].offset + stack->buffer[stack->first].size +
           stack->buffer[stack->last].offset + stack->buffer[stack->last].size;
}

VX_PRIVATE_API vx_enum
vxoMemoryPool_GetStackStatus(
    vx_mempool_stack stack
    )
{
    if (!stack->first && stack->last == stack->count + 1 )
    {
        return VX_MEMPOOL_STACK_EMPTY;
    }
    else if (stack->first && stack->last != stack->count + 1)
    {
        return VX_MEMPOOL_STACK_HAS_HEADTAIL;
    }
    else if (stack->first)
    {
        return VX_MEMPOOL_STACK_HAS_HEAD;
    }
    else
    {
        return VX_MEMPOOL_STACK_HAS_TAIL;
    }
}

VX_PRIVATE_API vx_uint32
vxoMemoryPool_GetStackId(
    vx_mempool_stack stack,
    vx_bool          head
    )
{
    if (head && stack->first)
    {
        return stack->buffer[stack->first].memory->lastUseId;
    }
    else if (!head && stack->last != stack->count + 1)
    {
        return stack->buffer[stack->last].memory->lastUseId;
    }

    return 0;
}

VX_PRIVATE_API vx_status
vxoMemoryPool_SetStackItemSizeByPos(
    vx_mempool_stack stack,
    vx_size          size,
    vx_uint32        pos
    )
{
    gcmASSERT(pos == stack->first || pos == stack->last);

    stack->buffer[pos].size = size;
    stack->buffer[pos].memory->sizes[1] = size;
    if (stack->buffer[pos].memory->memReverse)
    {
        stack->buffer[pos].memory->memOffset = stack->buffer[pos].offset + size;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status
vxoMemoryPool_ResetStackItemByPos(
    vx_mempool_stack stack,
    vx_uint32        pos
    )
{
    gcmASSERT(pos == stack->first || pos == stack->last);

    if (pos == stack->first)
    {
        stack->first--;
    }
    else if (pos == stack->last)
    {
        stack->last++;
    }
    else
    {
        stack->buffer[pos].memory = VX_NULL;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_uint32
vxoMemoryPool_PushStack(
    vx_mempool_stack stack,
    vx_memory memory,
    vx_bool head
    )
{
    vx_uint32 pos = 0;
    vx_size size = memory->sizes[1];
    gcmHEADER_ARG("stack=%p, memory=%p, head=0x%x", stack, memory, head);

    if (head)
    {
        stack->first++;
        pos = stack->first;
        stack->buffer[pos].memory = memory;
        stack->buffer[pos].offset = stack->buffer[pos-1].offset + stack->buffer[pos-1].size;
        stack->buffer[pos].size = size;
        memory->memOffset = stack->buffer[pos].offset;
        memory->memReverse = vx_false_e;
    }
    else
    {
        stack->last--;
        pos = stack->last;
        stack->buffer[pos].memory = memory;
        stack->buffer[pos].offset = stack->buffer[pos+1].offset + stack->buffer[pos+1].size;
        stack->buffer[pos].size = size;
        memory->memOffset = stack->buffer[pos].offset + size;
        memory->memReverse = vx_true_e;
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return pos;
}

VX_PRIVATE_API vx_status
vxoMemoryPool_PopStack(
    vx_mempool_stack stack,
    vx_bool head
     )
{
    gcmHEADER_ARG("stack=%p, head=0x%x", stack, head);
    if (head)
    {
        if (stack->first)
        {
            stack->buffer[stack->first].memory = VX_NULL;
            stack->first--;
            while (stack->first && stack->buffer[stack->first].memory == VX_NULL) stack->first--;
        }
    }
    else
    {
        if (stack->last != stack->count+1)
        {
            stack->buffer[stack->last].memory = VX_NULL;
            stack->last++;
            while (stack->last != stack->count+1 && stack->buffer[stack->last].memory == VX_NULL) stack->last++;
        }
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status
vxoMemoryPool_RemoveFromStackById(
    vx_mempool_stack stack,
    vx_uint32 op_id,
    vx_bool from_last,
    vx_bool clear_mem
    )
{
    vx_bool hstop = vx_false_e, tstop = vx_false_e;
    gcmHEADER_ARG("stack=%p, op_id=0x%x", stack, op_id);

    do
    {
        vx_enum s = vxoMemoryPool_GetStackStatus(stack);

        if ((s & VX_MEMPOOL_STACK_HAS_HEAD) &&
            ((from_last && stack->buffer[stack->first].memory->lastUseId < op_id) ||
             (!from_last && stack->buffer[stack->first].memory->firstUseId >= op_id)))
        {
            if (clear_mem)
            {
                stack->buffer[stack->first].memory->allocated = vx_false_e;
            }
            vxoMemoryPool_PopStack(stack, vx_true_e);
        }
        else
        {
            hstop = vx_true_e;
        }

        if ((s & VX_MEMPOOL_STACK_HAS_TAIL) &&
            ((from_last && stack->buffer[stack->last].memory->lastUseId < op_id) ||
             (!from_last && stack->buffer[stack->last].memory->firstUseId >= op_id)))
        {
            if (clear_mem)
            {
                stack->buffer[stack->last].memory->allocated = vx_false_e;
            }
            vxoMemoryPool_PopStack(stack, vx_false_e);
        }
        else
        {
            tstop = vx_true_e;
        }
    }
    while (!hstop || !tstop);

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status
vxoMemoryPool_GetStackItemsByPriority(
    vx_context       context,
    vx_mempool_stack stack,
    vx_enum          priority,
    vx_memory        array[],
    vx_uint32 *      acount
    )
{
    vx_uint32 pos, count = 0;

    if ((pos = stack->first) != 0)
    {
        do
        {
            if (VXNNE_MEM_ALLOC_TYPE_WITHOUT_MUST_HAVE(stack->buffer[pos].memory->allocPriority) == priority)
            {
                array[count++] = stack->buffer[pos].memory;
            }
        } while (--pos != 0);
    }

    if ((pos = stack->last) != stack->count+1)
    {
        do
        {
            if (VXNNE_MEM_ALLOC_TYPE_WITHOUT_MUST_HAVE(stack->buffer[pos].memory->allocPriority) == priority)
            {
                array[count++] = stack->buffer[pos].memory;
            }
        } while (++pos != stack->count+1);
    }

    if (acount != VX_NULL)
    {
        *acount = count;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status
vxoMemoryPool_AdjustAddress(
    vx_graph  graph,
    vx_memory memory,
    vx_size   all_size
    )
{
    vx_uint32 offset;
    vx_uint32 pbase = 0;
    vx_uint8_ptr lbase = VX_NULL;
    vx_uint32 ntype = VXNNE_MEM_POOL_TYPE_WITHOUT_CACHE(memory->allocType);

    gcmHEADER_ARG("graph=%p, memory=%p, all_size=0x%lx", graph, memory, all_size);

    if (memory->memReverse)
    {
        memory->memOffset = all_size - memory->memOffset;
        memory->memReverse = vx_false_e;
    }

    offset = (vx_uint32)memory->memOffset;

    if (memory->physicals[0] == 0)
    {
        switch (ntype)
        {
            case VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR:
                if (graph->memoryPool != VX_NULL &&
                    graph->memoryPool->nodePtr != VX_NULL &&
                    graph->memoryPool->locked)
                {
                    pbase = graph->memoryPool->physical;
                }
                break;

            case VXNNE_MEM_POOL_TYPE_AXI_SRAM:
                pbase = graph->base.context->axiSRAM[graph->deviceID].physical;
                break;

            case VXNNE_MEM_POOL_TYPE_VIP_SRAM:
                pbase = graph->base.context->vipSRAM.physical;
                break;

            default:
                break;
        }

        memory->physicals[0] = pbase + offset;

        if (ntype == VXNNE_MEM_POOL_TYPE_VIP_SRAM && VXNNE_MEM_POOL_TYPE_IS_CACHE(memory->allocType))
        {
            memory->physicals[0] -= graph->base.context->vipSRAM.physBase;
        }
    }

    if (memory->logicals[0] == VX_NULL)
    {
        switch (ntype)
        {
            case VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR:
                if (graph->memoryPool != VX_NULL &&
                    graph->memoryPool->nodePtr != VX_NULL &&
                    graph->memoryPool->locked)
                {
                    lbase = graph->memoryPool->logical;
                }
                break;

            case VXNNE_MEM_POOL_TYPE_AXI_SRAM:
                lbase = (vx_uint8_ptr)graph->base.context->axiSRAM[graph->deviceID].logical;
                break;

            case VXNNE_MEM_POOL_TYPE_VIP_SRAM:
                lbase = (vx_uint8_ptr)graph->base.context->vipSRAM.logical;
                break;

            default:
                break;
        }

        memory->logicals[0] = lbase + offset;
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status
vxoMemoryPool_RestoreStack(
    vx_mempool_stack stack,
    vx_mempool_stack_s* backup,
    vx_uint32 id,
    vx_uint32 start,
    vx_uint32 fid
    )
{
    vx_uint32 i;
    vxmASSERT(backup[id].buffer != VX_NULL);

    for (i = start; (vx_int32)i >= (vx_int32)id; i--)
    {
        vxMemCopy(stack->buffer, backup[i].buffer, gcmSIZEOF(vx_mempool_stack_item_s) * (stack->count + 2));
        stack->count = backup[i].count;
        stack->first = backup[i].first;
        stack->last = backup[i].last;
        vxoMemoryPool_RemoveFromStackById(stack, fid, vx_false_e, vx_true_e);
    }

    vxMemCopy(stack->buffer, backup[id].buffer, gcmSIZEOF(vx_mempool_stack_item_s) * (stack->count + 2));
    stack->count = backup[id].count;
    stack->first = backup[id].first;
    stack->last = backup[id].last;

    return VX_SUCCESS;
}

#define TYPE_SWAP(type, ma, mb) \
    do \
    { \
        type tm; tm = ma; ma = mb; mb = tm; \
    } while(0)

VX_INTERNAL_API vx_status
vxoMemoryPool_RequestList(
    vx_graph            graph,
    vxnne_mem_request   list,
    vx_uint32           list_count,
    vx_uint32           start,
    vx_uint32           count,
    vx_uint32           *max_sizes,
    vx_uint32           *fail_id
    )
{
    vx_uint32 i, j, ii, s, tcount = 0, mcounts[VXNNE_MEM_POOL_TYPE_END] = {0}, failId = 0;
    vx_size size, msize[VXNNE_MEM_POOL_TYPE_END] = {0};
    vx_size maxs[VXNNE_MEM_POOL_TYPE_ALL] = {0};
    vx_memory * plists[VXNNE_MEM_POOL_TYPE_END] = {VX_NULL};
    vx_mempool_stack_s stacks[VXNNE_MEM_POOL_TYPE_END];
    vx_mempool_stack_s* sbackups[VXNNE_MEM_POOL_TYPE_END] = {VX_NULL};
    vx_enum sstate;
    vx_enum atype = VXNNE_MEM_POOL_TYPE_ORIG_DDR;
    vx_context context = graph->base.context;
    vx_status status = VX_SUCCESS;

    gcmHEADER_ARG("graph=%p, list=%p, list_count=0x%x, start=0x%x, count=0x%x, max_sizes=%p",
        graph, list, list_count, start, count, max_sizes);
    gcmASSERT(list_count >= start+count);

    gcoOS_ZeroMemory(stacks, sizeof(vx_mempool_stack_s) * VXNNE_MEM_POOL_TYPE_END);

    /* sort based on 1.firstUseId from late to early 2.priority from low to high 3.size from big to small */
#define NEED_SWAP(pa, pb, fa, fb, sa, sb) \
    (fa < fb || (fa == fb && MEM_PRIORITY_LEFT_HIGHER_RIGHT(pa, pb)) || (fa == fb && pa == pb && sa > sb))

    maxs[VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR] = 0xFFFFFFFF;
    maxs[VXNNE_MEM_POOL_TYPE_VIP_SRAM] = context->vipSRAM.size - context->vipSRAM.used;
    maxs[VXNNE_MEM_POOL_TYPE_AXI_SRAM] = context->axiSRAM[graph->deviceID].size - context->axiSRAM[graph->deviceID].used;
    maxs[VXNNE_MEM_POOL_TYPE_SRAM] = maxs[VXNNE_MEM_POOL_TYPE_VIP_SRAM] + maxs[VXNNE_MEM_POOL_TYPE_AXI_SRAM];

    /* set first/last access id for each memory */
    for (i = 0; i < list_count; i++)
    {
        for (j = 0; j < list[i].outputCount; j++)
        {
            if (list[i].outputMemory[j]->firstUseId == VXNNE_MEM_ID_INIT_VALUE)
            {
                list[i].outputMemory[j]->firstUseId = i;
            }
            list[i].outputMemory[j]->lastUseId = i;

            if (i >= start && i < start + count)
            {
                vx_uint32 ntype = VXNNE_MEM_POOL_TYPE_WITHOUT_CACHE(list[i].outputMemory[j]->allocType);

                if (ntype == VXNNE_MEM_POOL_TYPE_AXI_SRAM && !maxs[VXNNE_MEM_POOL_TYPE_AXI_SRAM])
                {
                    vxmASSERT(0);
                    status = VX_INVALID_VALUE;
                    goto exit;
                }

                ntype = VXNNE_MEM_POOL_TYPE_WITHOUT_CACHE(list[i].outputMemory[j]->allocType);
                if (ntype != VXNNE_MEM_POOL_TYPE_ORIG_DDR && list[i].outputMemory[j]->sizes[1] == 0)
                {
                    vxmASSERT(maxs[ntype] != 0);
                    tcount++;
                    if (ntype == VXNNE_MEM_POOL_TYPE_SRAM)
                    {
                        mcounts[VXNNE_MEM_POOL_TYPE_AXI_SRAM]++;
                        mcounts[VXNNE_MEM_POOL_TYPE_VIP_SRAM]++;
                        list[i].outputMemory[j]->allocTypeTmp = maxs[VXNNE_MEM_POOL_TYPE_VIP_SRAM] ?
                                                                    VXNNE_MEM_POOL_TYPE_VIP_SRAM : VXNNE_MEM_POOL_TYPE_AXI_SRAM;
                    }
                    else
                    {
                        mcounts[ntype]++;
                        list[i].outputMemory[j]->allocTypeTmp = ntype;
                    }
                    list[i].outputMemory[j]->sizes[1] = list[i].outputMemory[j]->sizes[0];
                }

                atype |= ntype;
                gcmASSERT(!VXNNE_MEM_ALLOC_TYPE_IS_MUST_HAVE(list[i].outputMemory[j]->allocPriority) ||
                          list[i].outputMemory[j]->allocPartial == vx_false_e);
            }
        }

        for (j = 0; j < list[i].inputCount; j++)
        {
            if (list[i].inputMemory[j]->firstUseId == VXNNE_MEM_ID_INIT_VALUE)
            {
                list[i].inputMemory[j]->firstUseId = i;
            }
            list[i].inputMemory[j]->lastUseId = i;

            if (i >= start && i < start + count)
            {
                vx_uint32 ntype = VXNNE_MEM_POOL_TYPE_WITHOUT_CACHE(list[i].inputMemory[j]->allocType);

                if (ntype == VXNNE_MEM_POOL_TYPE_AXI_SRAM && maxs[VXNNE_MEM_POOL_TYPE_AXI_SRAM] == 0)
                {
                    vxmASSERT(0);
                    status = VX_INVALID_VALUE;
                    goto exit;
                }
                ntype = VXNNE_MEM_POOL_TYPE_WITHOUT_CACHE(list[i].inputMemory[j]->allocType);
                if (ntype != VXNNE_MEM_POOL_TYPE_ORIG_DDR && list[i].inputMemory[j]->sizes[1] == 0)
                {
                    vxmASSERT(maxs[ntype] != 0);
                    tcount++;
                    if (ntype == VXNNE_MEM_POOL_TYPE_SRAM)
                    {
                        mcounts[VXNNE_MEM_POOL_TYPE_AXI_SRAM]++;
                        mcounts[VXNNE_MEM_POOL_TYPE_VIP_SRAM]++;
                        list[i].inputMemory[j]->allocTypeTmp = maxs[VXNNE_MEM_POOL_TYPE_VIP_SRAM] ?
                                                                   VXNNE_MEM_POOL_TYPE_VIP_SRAM : VXNNE_MEM_POOL_TYPE_AXI_SRAM;
                    }
                    else
                    {
                        mcounts[ntype]++;
                        list[i].inputMemory[j]->allocTypeTmp = ntype;
                    }
                    list[i].inputMemory[j]->sizes[1] = list[i].inputMemory[j]->sizes[0];

                }

                atype |= ntype;
                gcmASSERT(!VXNNE_MEM_ALLOC_TYPE_IS_MUST_HAVE(list[i].inputMemory[j]->allocPriority) ||
                          list[i].inputMemory[j]->allocPartial == vx_false_e);
            }
        }
    }

    gcmASSERT(!((atype & VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR) && (atype & VXNNE_MEM_POOL_TYPE_SRAM)));

    for (i = 0; i < VXNNE_MEM_POOL_TYPE_END; i++)
    {
        if (mcounts[i] == 0) continue;

        status = gcoOS_Allocate(gcvNULL, gcmSIZEOF(vx_memory) * mcounts[i], (gctPOINTER*)&plists[i]);
        if (gcmIS_ERROR(status)) goto exit;

        status = vxoMemoryPool_InitStack(&stacks[i], mcounts[i], &sbackups[i], count);
        if (status != VX_SUCCESS) goto exit;
    }

    /* calculate virtual buffer from first op to last op */
    for (i = start; i < start+count; i++)
    {
        for (s = 0; s < VXNNE_MEM_POOL_TYPE_END; s++)
        {
            if (mcounts[s] == 0) continue;
            if (i > start)
            {
                vxoMemoryPool_RemoveFromStackById(&stacks[s], i, vx_true_e, vx_false_e);
            }
        }

again:
        for (ii = 0; ii < 2; ii++)
        {
            tcount = ii == 0 ? list[i].outputCount : list[i].inputCount;

            for (j = 0; j < tcount; j++)
            {
                vx_memory mem = ii == 0 ? list[i].outputMemory[j] : list[i].inputMemory[j];
                vx_enum ntype = mem->allocTypeTmp;
                vx_enum npriority = VXNNE_MEM_ALLOC_TYPE_WITHOUT_MUST_HAVE(mem->allocPriority);
                vx_bool mustHave = VXNNE_MEM_ALLOC_TYPE_IS_MUST_HAVE(mem->allocPriority);

                if (ntype == VXNNE_MEM_POOL_TYPE_ORIG_DDR) continue;

                gcmASSERT(ntype < VXNNE_MEM_POOL_TYPE_END);
                gcmASSERT(mcounts[ntype] > 0);

                if (mem->allocated == vx_false_e && i > mem->firstUseId && mustHave)
                {
                    failId = i;
                    status = VX_FAILURE;
                    goto exit;
                }
                else if (mem->allocated == vx_false_e && mem->sizes[1] != 0)
                {
                    vx_uint32 pos = 0;
                    vx_uint32 sid = mem->lastUseId;
                    vx_uint32 hid = vxoMemoryPool_GetStackId(&stacks[ntype], vx_true_e);
                    vx_uint32 tid = vxoMemoryPool_GetStackId(&stacks[ntype], vx_false_e);

                    sstate = vxoMemoryPool_GetStackStatus(&stacks[ntype]);

                    gcmASSERT(mem->physicals[0] == 0);

                    if (sstate == VX_MEMPOOL_STACK_HAS_HEADTAIL)
                    {
                        if ((tid > hid && sid > hid)  || (hid > tid && sid <= tid))
                        {
                            pos = vxoMemoryPool_PushStack(&stacks[ntype], mem, vx_false_e);
                        }
                        else
                        {
                            pos = vxoMemoryPool_PushStack(&stacks[ntype], mem, vx_true_e);
                        }
                    }
                    else if (sstate == VX_MEMPOOL_STACK_EMPTY)
                    {
                        pos = vxoMemoryPool_PushStack(&stacks[ntype], mem, vx_true_e);
                    }
                    else
                    {
                        if ((sstate == VX_MEMPOOL_STACK_HAS_HEAD && hid == i) ||
                            (sstate == VX_MEMPOOL_STACK_HAS_TAIL && tid == sid))
                        {
                            pos = vxoMemoryPool_PushStack(&stacks[ntype], mem, vx_false_e);
                        }
                        else if ((sstate == VX_MEMPOOL_STACK_HAS_TAIL && tid == i) ||
                                 (sstate == VX_MEMPOOL_STACK_HAS_HEAD && hid == sid))
                        {
                            pos = vxoMemoryPool_PushStack(&stacks[ntype], mem, vx_true_e);
                        }
                        else
                        {
                            vx_uint32 ii;
                            vx_uint32 nid = 0;

                            if (i < count - 1)
                            {
                                for (ii = 0; ii < list[i+1].outputCount; ii++)
                                {
                                    vx_bool alloc = list[i+1].outputMemory[ii]->allocated;
                                    vx_uint32 id = list[i+1].outputMemory[ii]->lastUseId;

                                    if (!alloc && id > nid) nid = id;
                                }
                            }

                            if (sstate == VX_MEMPOOL_STACK_HAS_HEAD)
                            {
                                if (sid > nid)
                                {
                                    pos = vxoMemoryPool_PushStack(&stacks[ntype], mem, vx_false_e);
                                }
                                else
                                {
                                    pos = vxoMemoryPool_PushStack(&stacks[ntype], mem, vx_true_e);
                                }
                            }
                            else
                            {
                                if (sid > nid)
                                {
                                    pos = vxoMemoryPool_PushStack(&stacks[ntype], mem, vx_true_e);
                                }
                                else
                                {
                                    pos = vxoMemoryPool_PushStack(&stacks[ntype], mem, vx_false_e);
                                }
                            }
                        }
                    }

                    gcmASSERT(pos != 0);

                    size = vxoMemoryPool_GetStackSize(&stacks[ntype]);

                    if (size > maxs[ntype])
                    {
                        vx_uint32 k, pcounts[VXNNE_MEM_ALLOC_PRIORITY_TYPE_KIND] = {0}, acount = 0;
                        vx_bool find = vx_false_e;
                        vx_size asize = 0, dsize = size - maxs[ntype];
                        vx_memory * plist = plists[ntype];

                        if (npriority != VXNNE_MEM_ALLOC_LOWEST_PRIORITY)
                        {
                            vx_uint32 kk;
                            vx_enum p;

                            asize = 0;
                            LOOP_MEM_PRIORITY_LOWEST2HIGH(p, MEM_PRIORITY_DOWN_LEVEL(npriority))
                            {
                                vx_memory * cplist = plist + acount;

                                vxoMemoryPool_GetStackItemsByPriority(context, &stacks[ntype], p, cplist, &pcounts[p]);

                                if (pcounts[p] != 0)
                                {
                                    for (k = 0; k < pcounts[p]; k++)
                                    {
                                        asize += cplist[k]->sizes[1];
                                        acount++;

                                        /* sort current priority memory based on firstUseId from late to early */
                                        for (kk = k; kk > 0; kk--)
                                        {
                                            vx_enum oldP = VXNNE_MEM_ALLOC_TYPE_WITHOUT_MUST_HAVE(cplist[kk-1]->allocPriority);
                                            vx_enum newP = VXNNE_MEM_ALLOC_TYPE_WITHOUT_MUST_HAVE(cplist[kk]->allocPriority);
                                            if (NEED_SWAP(oldP, newP,
                                                          cplist[kk-1]->firstUseId, cplist[kk]->firstUseId,
                                                          cplist[kk-1]->sizes[1], cplist[kk]->sizes[1]))
                                            {
                                                TYPE_SWAP(vx_memory, cplist[kk-1], cplist[kk]);
                                            }
                                        }
                                    }

                                    if (asize >= dsize)
                                    {
                                        find = vx_true_e;
                                        break;
                                    }
                                }
                            }
                        }

                        if (find || (mem->allocPartial && asize > 0))
                        {
                            vx_uint32 fid = 0;

                            asize = 0;
                            for (k = 0; k < acount; k++)
                            {
                                asize += plist[k]->sizes[1];
                                plist[k]->allocated = vx_false_e;
                                if (asize >= dsize)
                                {
                                    break;
                                }
                                else
                                {
                                    if (VXNNE_MEM_POOL_TYPE_WITHOUT_CACHE(plist[k]->allocType) == VXNNE_MEM_POOL_TYPE_SRAM &&
                                        plist[k]->allocTypeTmp == VXNNE_MEM_POOL_TYPE_VIP_SRAM &&
                                        maxs[VXNNE_MEM_POOL_TYPE_AXI_SRAM] != 0)
                                    {
                                        plist[k]->allocTypeTmp = VXNNE_MEM_POOL_TYPE_AXI_SRAM;
                                    }
                                    else
                                    {
                                        if (VXNNE_MEM_ALLOC_TYPE_IS_MUST_HAVE(plist[k]->allocPriority))
                                        {
                                            failId = i;
                                            status = VX_FAILURE;
                                            goto exit;
                                        }
                                        plist[k]->sizes[1] = 0;
                                    }

                                    fid = plist[k]->firstUseId;
                                }
                            }
                            gcmASSERT(k < acount || mem->allocPartial);

                            if (k < acount)
                            {
                                fid = plist[k]->firstUseId;

                                if (plist[k]->allocPartial && asize >= dsize)
                                {
                                    plist[k]->sizes[1] = asize - dsize;
                                }
                                else
                                {
                                    if (VXNNE_MEM_POOL_TYPE_WITHOUT_CACHE(plist[k]->allocType) == VXNNE_MEM_POOL_TYPE_SRAM &&
                                        plist[k]->allocTypeTmp == VXNNE_MEM_POOL_TYPE_VIP_SRAM &&
                                        maxs[VXNNE_MEM_POOL_TYPE_AXI_SRAM] != 0)
                                    {
                                        plist[k]->allocTypeTmp = VXNNE_MEM_POOL_TYPE_AXI_SRAM;
                                    }
                                    else
                                    {
                                        if (VXNNE_MEM_ALLOC_TYPE_IS_MUST_HAVE(plist[k]->allocPriority))
                                        {
                                            failId = i;
                                            status = VX_FAILURE;
                                            goto exit;
                                        }
                                        plist[k]->sizes[1] = 0;
                                    }
                                }
                            }

                            for (s = 0; s < VXNNE_MEM_POOL_TYPE_END; s++)
                            {
                                if (mcounts[s] == 0) continue;
                                vxoMemoryPool_RemoveFromStackById(&stacks[s], fid, vx_false_e, vx_true_e);
                                if (i > fid)
                                {
                                    vxoMemoryPool_RestoreStack(&stacks[s], sbackups[s], fid - start, i - start - 1, fid);
                                    vxoMemoryPool_RemoveFromStackById(&stacks[s], fid, vx_false_e, vx_true_e);
                                }
                            }

                            i = fid;
                            goto again;
                        }
                        else if (VXNNE_MEM_POOL_TYPE_WITHOUT_CACHE(mem->allocType) == VXNNE_MEM_POOL_TYPE_SRAM &&
                                 mem->allocTypeTmp == VXNNE_MEM_POOL_TYPE_VIP_SRAM &&
                                 maxs[VXNNE_MEM_POOL_TYPE_AXI_SRAM] != 0)
                        {
                            mem->allocTypeTmp = VXNNE_MEM_POOL_TYPE_AXI_SRAM;
                            vxoMemoryPool_ResetStackItemByPos(&stacks[ntype], pos);
                            goto again;
                        }
                        else if (mem->allocPartial && mem->sizes[1] > dsize)
                        {
                            vxoMemoryPool_SetStackItemSizeByPos(&stacks[ntype], mem->sizes[1]-dsize, pos);
                            mem->allocated = vx_true_e;
                            msize[ntype] = maxs[ntype];
                            continue;
                        }
                        else if (!mustHave)
                        {
                            vxoMemoryPool_ResetStackItemByPos(&stacks[ntype], pos);
                            continue;
                        }
                        else
                        {
                            failId = i;
                            status = VX_FAILURE;
                            goto exit;
                        }
                    }
                    else
                    {
                        if (size > msize[ntype]) msize[ntype] = size;
                        mem->allocTypeTmp = ntype;
                        mem->allocated = vx_true_e;
                    }
                }
            }
        }

        for (s = 0; s < VXNNE_MEM_POOL_TYPE_END; s++)
        {
            if (mcounts[s] == 0) continue;
            vxoMemoryPool_BackupStack(&stacks[s], sbackups[s], i - start);
        }
    }

    for (i = 0; i < VXNNE_MEM_POOL_TYPE_END; i++)
    {
        if (mcounts[i] == 0) continue;

        vxoMemoryPool_RemoveFromStackById(&stacks[i], start+count, vx_true_e, vx_false_e);

        /* check if memory alloc/free is closure */
        sstate = vxoMemoryPool_GetStackStatus(&stacks[i]);
        if (sstate != VX_MEMPOOL_STACK_EMPTY)
        {
            status = VX_FAILURE;
            goto exit;
        }

        if (max_sizes != VX_NULL)
        {
            max_sizes[i] = (vx_uint32)msize[i];
        }
    }

    /* allocate virtual memory pool if necessary */
    if (atype == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR)
    {
        if (!context->options.memPoolSize && !vxoMemoryPool_LockDown(graph, msize[atype]))
        {
            vxError("Can't allocate memory for virtual memory pool");
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }
    }

    /* change memory offset to address */
    for (i = start; i < start+count; i++)
    {
        for (j = 0; j < list[i].outputCount; j++)
        {
            vx_memory outmem = list[i].outputMemory[j];
            vx_uint32 ntype = outmem->allocTypeTmp;

            if (ntype != VXNNE_MEM_POOL_TYPE_ORIG_DDR && outmem->allocated)
            {
                outmem->allocType = VXNNE_MEM_POOL_TYPE_IS_CACHE(outmem->allocType) ?
                                        VXNNE_MEM_POOL_TYPE_SET_CACHE(outmem->allocTypeTmp) : outmem->allocTypeTmp;
                vxoMemoryPool_AdjustAddress(graph, outmem, msize[ntype]);
                outmem->allocPartial = outmem->sizes[1] == outmem->sizes[0] ? vx_false_e : vx_true_e;
                outmem->sizes[0] = outmem->sizes[1];
                outmem->allocated = vx_false_e;
            }
        }

        for (j = 0; j < list[i].inputCount; j++)
        {
            vx_memory inmem = list[i].inputMemory[j];
            vx_uint32 ntype = inmem->allocTypeTmp;

            if (ntype != VXNNE_MEM_POOL_TYPE_ORIG_DDR && inmem->allocated)
            {
                inmem->allocType = VXNNE_MEM_POOL_TYPE_IS_CACHE(inmem->allocType) ?
                                        VXNNE_MEM_POOL_TYPE_SET_CACHE(inmem->allocTypeTmp) : inmem->allocTypeTmp;
                vxoMemoryPool_AdjustAddress(graph, inmem, msize[ntype]);
                inmem->allocPartial = inmem->sizes[1] == inmem->sizes[0] ? vx_false_e : vx_true_e;
                inmem->sizes[0] = inmem->sizes[1];
                inmem->allocated = vx_false_e;
            }
        }
    }

exit:
    for (i = 0; i < VXNNE_MEM_POOL_TYPE_END; i++)
    {
        if (mcounts[i] == 0) continue;

        vxoMemoryPool_DeInitStack(&stacks[i], &sbackups[i], count);

        if (plists[i] != VX_NULL)
        {
            gcoOS_FreeMemory(gcvNULL, plists[i]);
            plists[i] = VX_NULL;
        }
    }

    if (status == VX_FAILURE && fail_id != VX_NULL)
    {
        *fail_id = failId;
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

