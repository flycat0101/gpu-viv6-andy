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

VX_PRIVATE_API vx_bool _pool_get_memory(vx_memory_pool pool, vx_size size, vx_uint8_ptr* logical, vx_uint32_ptr physical, gcsSURF_NODE_PTR* node);
VX_PRIVATE_API vx_bool _pool_put_memory(vx_memory_pool pool, gcsSURF_NODE_PTR node);

VX_PRIVATE_API vx_bool _AllocateMemory(vx_context context, vx_memory memory, vx_bool virt)
{
    vx_int32 planeIndex, dimIndex;

    vxmASSERT(context);
    vxmASSERT(memory);

    if (memory->allocated) return vx_true_e;

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
    vxmASSERT(size);
    vxmASSERT(memory);

    memory->planeCount = 1;
    memory->dimCount = 1;
    memory->dims[0][0] = (vx_int32)size;
    memory->strides[0][VX_DIM_CHANNEL] = 0;

    return vxoMemory_Allocate(context, memory);
}

VX_PRIVATE_API vx_bool _FreeMemory(vx_context context, vx_memory memory, vx_bool virt)
{
    vx_uint32 planeIndex;

    vxmASSERT(context);
    vxmASSERT(memory);

    if (!memory->allocated) return vx_true_e;

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
    vx_int32 planeIndex, dimIndex;

    vxmASSERT(context);
    vxmASSERT(memory);

    if (memory->allocated) return vx_true_e;

    memory->allocated = vx_true_e;

    for (planeIndex = 0; (vx_uint32) planeIndex < memory->planeCount; planeIndex++)
    {
        gceSTATUS status;

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
        desc.size     = (gctUINT32)memory->sizes[planeIndex];

        memory->wrappedSize[planeIndex] = memory->sizes[planeIndex];

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

    return vx_true_e;

ErrorExit:
    for (planeIndex = planeIndex - 1; planeIndex >= 0; planeIndex--)
    {
        if (memory->wrappedNode[planeIndex] != 0)
        {
            gcmVERIFY_OK(gcoHAL_UnlockVideoMemory(
                            memory->wrappedNode[planeIndex],
                            gcvVIDMEM_TYPE_BITMAP,
                            gcvENGINE_RENDER));

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
                            gcvVIDMEM_TYPE_BITMAP,
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

        memory->sizes[planeIndex] = 0;
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

/*****************************************************************************************************/
/*****************************************************************************************************/
VX_PRIVATE_API vx_memory_pool_item _pool_get_emtpy_item(vx_memory_pool pool)
{
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
            return VX_NULL;
        }
        else
        {
            for (i = i+1, j = i; i < VX_MAX_MEMPOOL_ITEM_NUM; i++)
            {
                if (pool->pool[i].index != VX_MEMPOOL_ITEM_INVALID)
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

    gcoOS_MemFill(&pool->pool[pool->count], 0, sizeof(vx_memory_pool_item_s));
    pool->pool[pool->count].index = pool->count;
    pool->count++;
    return &pool->pool[pool->count-1];
}

VX_PRIVATE_API vx_bool _pool_get_memory(vx_memory_pool pool, vx_size size, vx_uint8_ptr* logical, vx_uint32_ptr physical, gcsSURF_NODE_PTR* node)
{
    vx_memory_pool_item piptr = VX_NULL, fiptr, aiptr;

    vxmASSERT(size);
    vxmASSERT(node);

    if (pool->locked)
        return vx_false_e;

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
                    return vx_false_e;
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
            return vx_false_e;

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
            *logical = pool->logical + fiptr->offset;
        if (physical != VX_NULL)
            *physical = pool->physical + (vx_uint32)fiptr->offset;
        gcoOS_ZeroMemory(*logical, size);
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

    return vx_true_e;
}

VX_PRIVATE_API vx_bool _pool_put_memory(vx_memory_pool pool, gcsSURF_NODE_PTR node)
{
    vx_memory_pool_item ciptr;

    vxmASSERT(node);

    if (pool->locked) return vx_true_e;

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

    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoMemoryPool_Deinitialize(vx_graph graph)
{
    vxmASSERT(graph);
    vxmASSERT(graph->memoryPool);

    if (graph->memoryPool->logical != VX_NULL)
    {
        gcoVX_FreeMemory((gcsSURF_NODE_PTR)graph->memoryPool->nodePtr);
    }

    gcoOS_FreeMemory(gcvNULL, graph->memoryPool);
    graph->memoryPool = VX_NULL;

    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoMemoryPool_Initialize(vx_graph graph, vx_size size)
{
    gceSTATUS status;
    vx_memory_pool pointer = gcvNULL;

    vxmASSERT(graph);

    status = gcoOS_Allocate(gcvNULL, gcmSIZEOF(struct _vx_memory_pool_s), (gctPOINTER *)&pointer);
    if (gcmIS_ERROR(status)) return vx_false_e;

    gcoOS_MemFill(pointer, 0, gcmSIZEOF(struct _vx_memory_pool_s));

    if (size != 0)
    {
        size += VX_MEMPOOL_RESERVE_MEM_SIZE;
        status = gcoVX_AllocateMemory((gctUINT32)size, (gctPOINTER*)&pointer->logical,
                                      &pointer->physical, &pointer->nodePtr);
        if (gcmIS_ERROR(status))
        {
            vxoMemoryPool_Deinitialize(graph);
            if(pointer) gcoOS_Free(NULL, (gctPOINTER)pointer);
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

    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoMemoryPool_LockDown(vx_graph graph, vx_size size)
{
    gceSTATUS status;
    vx_memory_pool pool;

    vxmASSERT(graph);
    pool = graph->memoryPool;
    vxmASSERT(pool);

    if (pool->locked) return vx_true_e;

    if (!pool->memExpandMode) return vx_false_e;
    if (pool->size && size) return vx_false_e;
    if (!pool->size && !size) return vx_true_e;

    size += pool->size + VX_MEMPOOL_RESERVE_MEM_SIZE;
    status = gcoVX_AllocateMemory((gctUINT32)size, (gctPOINTER*)&pool->logical,
                                  &pool->physical, (gcsSURF_NODE_PTR*)&pool->nodePtr);
    if (gcmIS_ERROR(status))
    {
        vxoMemoryPool_Deinitialize(graph);
        return vx_false_e;
    }

    pool->size = size;
    pool->locked = vx_true_e;

    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoMemoryPool_Reset(vx_graph graph, vx_bool freeMem)
{
    vx_memory_pool pool;

    vxmASSERT(graph);
    pool = graph->memoryPool;
    vxmASSERT(pool);

    if (!pool->locked) return vx_false_e;

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
    vx_uint32 count
    )
{
    vx_status status = VX_SUCCESS;

    status = gcoOS_Allocate(gcvNULL, gcmSIZEOF(vx_mempool_stack_item_s) * (count + 2), (gctPOINTER*)&stack->buffer);
    if (gcmIS_ERROR(status)) return status;

    stack->count = count;
    stack->first = 0;
    stack->last = count + 1;
    stack->buffer[stack->first].memory = stack->buffer[stack->last].memory = VX_NULL;
    stack->buffer[stack->first].offset = stack->buffer[stack->last].offset = 0;
    stack->buffer[stack->first].size   = stack->buffer[stack->last].size = 0;

    return status;
}

VX_PRIVATE_API vx_status
vxoMemoryPool_DeInitStack(
    vx_mempool_stack stack
    )
{
    if (stack->buffer != VX_NULL)
    {
        gcoOS_FreeMemory(gcvNULL, stack->buffer);
        stack->buffer = VX_NULL;
    }

    stack->count = 0;
    stack->first = 0;
    stack->last = 1;

    return VX_SUCCESS;
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

VX_PRIVATE_API vx_memory
vxoMemoryPool_GetStackHeadTailMem(
    vx_mempool_stack stack,
    vx_enum position
    )
{
    if (position == VX_MEMPOOL_STACK_HAS_HEAD && !stack->first)
    {
        return stack->buffer[stack->first].memory;
    }
    else if (position == VX_MEMPOOL_STACK_HAS_TAIL && stack->last != stack->count + 1)
    {
        return stack->buffer[stack->last].memory;
    }

    return VX_NULL;
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
vxoMemoryPool_PushStack(
    vx_mempool_stack stack,
    vx_memory memory,
    vx_bool head
    )
{
    vx_uint32 pos;
    vx_size size = memory->sizes[0];

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

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status
vxoMemoryPool_PopStack(
    vx_mempool_stack stack,
    vx_bool head
     )
{
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

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status
vxoMemoryPool_RemoveFromStackById(
    vx_mempool_stack stack,
    vx_uint32 op_id
    )
{
    vx_bool hstop = vx_false_e, tstop = vx_false_e;

    do
    {
        vx_enum s = vxoMemoryPool_GetStackStatus(stack);

        if ((s & VX_MEMPOOL_STACK_HAS_HEAD) && op_id > stack->buffer[stack->first].memory->lastUseId)
        {
            vxoMemoryPool_PopStack(stack, vx_true_e);
        }
        else
        {
            hstop = vx_true_e;
        }

        if ((s & VX_MEMPOOL_STACK_HAS_TAIL) && op_id > stack->buffer[stack->last].memory->lastUseId)
        {
            vxoMemoryPool_PopStack(stack, vx_false_e);
        }
        else
        {
            tstop = vx_true_e;
        }
    }
    while (!hstop || !tstop);

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

    if (memory->memReverse)
    {
        memory->memOffset = all_size - memory->memOffset;
        memory->memReverse = vx_false_e;
    }

    offset = (vx_uint32)memory->memOffset;

    if (memory->physicals[0] == 0)
    {
        switch (memory->allocType)
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
                pbase = graph->base.context->axiSRAM.physical;
                break;

            case VXNNE_MEM_POOL_TYPE_VIP_SRAM:
                pbase = graph->base.context->vipSRAM.physical;
                break;

            default:
                break;
        }

        memory->physicals[0] = pbase + offset;
    }

    if (memory->logicals[0] == VX_NULL)
    {
        switch (memory->allocType)
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
                lbase = (vx_uint8_ptr)graph->base.context->axiSRAM.logical;
                break;

            case VXNNE_MEM_POOL_TYPE_VIP_SRAM:
                lbase = (vx_uint8_ptr)graph->base.context->vipSRAM.logical;
                break;

            default:
                break;
        }

        memory->logicals[0] = lbase + offset;
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status
vxoMemoryPool_RequestList(
    vx_graph            graph,
    vxnne_mem_request   list,
    vx_uint32           list_count,
    vx_uint32           start,
    vx_uint32           count,
    vx_memory *         error_mem
    )
{
    vx_uint32 i, j, ii, tcount = 0;
    vx_size size, msize = 0;
    vx_mempool_stack_s stack;
    vx_enum sstate;
    vx_enum atype = VXNNE_MEM_POOL_TYPE_ORIG_DDR;
    vx_context context = graph->base.context;
    vx_status status = VX_SUCCESS;

    gcmASSERT(list_count >= start+count);

    /* set last access id for each memory */
    for (i = 0; i < list_count; i++)
    {
        for (j = 0; j < list[i].outputCount; j++)
        {
            if (list[i].outputMemory[j]->firstUseId == VXNNE_MEM_ID_INIT_VALUE)
            {
                list[i].outputMemory[j]->firstUseId = i;
            }
        }

        for (j = 0; j < list[i].inputCount; j++)
        {
            list[i].inputMemory[j]->lastUseId = i;
        }

        if (i >= start && i < start+count)
        {
            tcount += list[i].inputCount + list[i].outputCount;
        }
    }

    status = vxoMemoryPool_InitStack(&stack, tcount);
    if (status != VX_SUCCESS) goto exit;

    /* calculate virtual buffer from first op to last op */
    for (i = start; i < start+count; i++)
    {
        vxoMemoryPool_RemoveFromStackById(&stack, i);

        for (ii = 0; ii < 2; ii++)
        {
            tcount = ii == 0 ? list[i].outputCount : list[i].inputCount;

            for (j = 0; j < tcount; j++)
            {
                vx_memory mem = ii == 0 ? list[i].outputMemory[j] : list[i].inputMemory[j];

                if (mem->allocType == VXNNE_MEM_POOL_TYPE_ORIG_DDR) continue;

                if (mem->allocated == vx_false_e && i > mem->firstUseId)
                {
                    status = VX_FAILURE;
                    goto exit;
                }
                else if (mem->allocated == vx_false_e)
                {
                    vx_uint32 sid = mem->lastUseId;
                    vx_uint32 hid = vxoMemoryPool_GetStackId(&stack, vx_true_e);
                    vx_uint32 tid = vxoMemoryPool_GetStackId(&stack, vx_false_e);

                    sstate = vxoMemoryPool_GetStackStatus(&stack);

                    gcmASSERT(mem->physicals[0] == 0);

                    if (sstate == VX_MEMPOOL_STACK_HAS_HEADTAIL)
                    {
                        if ((tid > hid && sid > hid) || (hid > tid && sid <= tid))
                        {
                            vxoMemoryPool_PushStack(&stack, mem, vx_false_e);
                        }
                        else
                        {
                            vxoMemoryPool_PushStack(&stack, mem, vx_true_e);
                        }
                    }
                    else if (sstate == VX_MEMPOOL_STACK_EMPTY)
                    {
                        vxoMemoryPool_PushStack(&stack, mem, vx_true_e);
                    }
                    else
                    {
                        if ((sstate == VX_MEMPOOL_STACK_HAS_HEAD && hid == i) ||
                            (sstate == VX_MEMPOOL_STACK_HAS_TAIL && tid == sid))
                        {
                            vxoMemoryPool_PushStack(&stack, mem, vx_false_e);
                        }
                        else if ((sstate == VX_MEMPOOL_STACK_HAS_TAIL && tid == i) ||
                                 (sstate == VX_MEMPOOL_STACK_HAS_HEAD && hid == sid))
                        {
                            vxoMemoryPool_PushStack(&stack, mem, vx_true_e);
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
                                if (sid > nid) vxoMemoryPool_PushStack(&stack, mem, vx_false_e);
                                else vxoMemoryPool_PushStack(&stack, mem, vx_true_e);
                            }
                            else
                            {
                                if (sid > nid) vxoMemoryPool_PushStack(&stack, mem, vx_true_e);
                                else vxoMemoryPool_PushStack(&stack, mem, vx_false_e);
                            }
                        }
                    }

                    mem->allocated = vx_true_e;
                }

                gcmASSERT(atype == VXNNE_MEM_POOL_TYPE_ORIG_DDR || atype == mem->allocType);
                atype = mem->allocType;
            }
        }

        size = vxoMemoryPool_GetStackSize(&stack);
        if (size > msize) msize = size;

        if (atype == VXNNE_MEM_POOL_TYPE_AXI_SRAM && msize > context->axiSRAM.size)
        {
            status = VX_FAILURE;
            goto exit;
        }
    }

    vxoMemoryPool_RemoveFromStackById(&stack, start+count);

    /* check if memory alloc/free is closure */
    sstate = vxoMemoryPool_GetStackStatus(&stack);
    if (sstate != VX_MEMPOOL_STACK_EMPTY)
    {
        if (error_mem != VX_NULL)
        {
            if (sstate & VX_MEMPOOL_STACK_HAS_HEAD)
            {
                *error_mem = vxoMemoryPool_GetStackHeadTailMem(&stack, VX_MEMPOOL_STACK_HAS_HEAD);
            }
            else if (sstate & VX_MEMPOOL_STACK_HAS_TAIL)
            {
                *error_mem = vxoMemoryPool_GetStackHeadTailMem(&stack, VX_MEMPOOL_STACK_HAS_TAIL);
            }
        }

        status = VX_FAILURE;
        goto exit;
    }

    /* allocate virtual memory pool if necessary */
    if (atype == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR)
    {
        if (!context->options.memPoolSize && !vxoMemoryPool_LockDown(graph, msize))
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

            if (outmem->allocType != VXNNE_MEM_POOL_TYPE_ORIG_DDR)
            {
                vxoMemoryPool_AdjustAddress(graph, outmem, msize);
                outmem->allocated = vx_false_e;
            }
        }

        for (j = 0; j < list[i].inputCount; j++)
        {
            vx_memory inmem = list[i].inputMemory[j];

            if (inmem->allocType != VXNNE_MEM_POOL_TYPE_ORIG_DDR && inmem->allocated)
            {
                vxoMemoryPool_AdjustAddress(graph, inmem, msize);
                inmem->allocated = vx_false_e;
            }
        }
    }

exit:
    vxoMemoryPool_DeInitStack(&stack);

    return status;
}

