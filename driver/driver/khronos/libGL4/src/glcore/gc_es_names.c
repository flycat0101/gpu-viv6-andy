/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_es_context.h"

/*
** The following four functions are generic ID management functions that
** are used to generate IDs for GenTextures, etc.
*/
#define _GC_OBJ_ZONE gcdZONE_GL40_CORE

GLuint __glGenerateNames(__GLcontext *gc, __GLsharedObjectMachine *shared, GLsizei range)
{
    GLint genName = 0;

    if (shared->lock)
    {
        (*gc->imports.lockMutex)(shared->lock);
    }

    do
    {
        __GLnameAllocation *temp;
        __GLnameAllocation *allocated = shared->nameArray;

        if (!allocated)
        {
            /* There is no IDs allocated yet */
            allocated = (__GLnameAllocation *)(*gc->imports.malloc)(gc, sizeof(__GLnameAllocation));
            allocated->next = gcvNULL;
            allocated->start = 1;
            allocated->number = range;
            shared->nameArray = allocated;
            genName = 1;
            break;
        }

        /*
        ** Fill the gap in the beginning.
        */
        if (allocated->start > (GLuint)range)
        {
            if (allocated->start == (GLuint)range + 1)
            {
                allocated->start = 1;
                allocated->number += range;
            }
            else
            {
                temp = (__GLnameAllocation *)(*gc->imports.malloc)(gc, sizeof(__GLnameAllocation));
                temp->next = allocated;
                temp->start = 1;
                temp->number = range;
                shared->nameArray = temp;
            }
        }

        /*
        ** Search through already allocated lists to find a place to
        ** slip some new IDs in.
        */
        for (;;)
        {
            GLuint start = allocated->start + allocated->number;
            GLuint end = start + range;

            /* Check word wrap */
            if (end < start)
            {
                genName = 0;
                break;
            }

            if (!allocated->next || (end < allocated->next->start))
            {
                allocated->number += range;
                genName = start;
                break;
            }

            /* Collapse two adjacent nodes into one. */
            if (end == allocated->next->start)
            {
                allocated->number += range + allocated->next->number;
                temp = allocated->next;
                allocated->next = allocated->next->next;
                if (temp)
                {
                    (*gc->imports.free)(gc, temp);
                }
                genName = start;
                break;
            }

            allocated = allocated->next;
        }
    } while (GL_FALSE);

    if (shared->lock)
    {
        (*gc->imports.unlockMutex)(shared->lock);
    }
    return genName;
}

GLboolean __glIsNameDefined(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id)
{
    __GLnameAllocation *allocated;
    GLboolean result = GL_FALSE;

    if (shared->lock)
    {
        (*gc->imports.lockMutex)(shared->lock);
    }

    allocated = shared->nameArray;

    if (id == 0)
    {
        result = GL_FALSE;
    }
    else
    {
        while (allocated)
        {
            if (id < allocated->start)
            {
                result = GL_FALSE;
                break;
            }
            if (id < allocated->start + allocated->number)
            {
                result = GL_TRUE;
                break;
            }
            allocated = allocated->next;
        }
    }

    if (shared->lock)
    {
        (*gc->imports.unlockMutex)(shared->lock);
    }
    return result;
}

GLvoid __glDeleteNamesFrList(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id, GLsizei n)
{
    __GLnameAllocation **allocated;
    __GLnameAllocation *temp;
    GLuint end, delstart, delend, blstart, blend;

    if (n <= 0)
    {
        return;
    }

    if (shared->lock)
    {
        (*gc->imports.lockMutex)(shared->lock);
    }

    allocated = &shared->nameArray;

    /* Delete it from the allocated struct */
    end = id + n;
    while (*allocated)
    {
        /* The start and end for this allocated block
        */
        blstart = (*allocated)->start;
        blend = blstart + (*allocated)->number;

        /*
        ** If we passed that part of our allocation id that might be
        ** affected, we stop (blstart will only keep getting larger).
        */
        if (end <= blstart)
        {
            break;
        }

        if (id < blend)
        {
            /*
            ** Since end > blstart and id < blend,
            ** The deletion affects this part of this block.
            */
            delstart = id;
            if (delstart < blstart)
            {
                delstart = blstart;
            }
            delend = end;
            if (delend > blend)
            {
                delend = blend;
            }

            /* Need to delete any ids from delstart to delend */

            if (delstart > blstart)
            {
                /*
                ** The first part of this block remains.  Might as well
                ** use the same allocated block, and just change "number".
                */
                (*allocated)->number = delstart - blstart;
                if (delend < blend)
                {
                    /*
                    ** Need to create a new block for the second
                    ** half of this newly fragmented one.
                    */
                    temp = (__GLnameAllocation*)(*gc->imports.malloc)(gc, sizeof(__GLnameAllocation));
                    temp->next = (*allocated)->next;
                    temp->start = delend;
                    temp->number = blend - delend;
                    (*allocated)->next = temp;
                    allocated = &((*allocated)->next);
                }
            }
            else if (delend < blend)
            {
                /*
                ** The last part of this block remains.  Might as well
                ** use the same allocated block, and just change the fields.
                */
                (*allocated)->number = blend - delend;
                (*allocated)->start = delend;
            }
            else
            {
                /* The whole block is gone.  Need to delete it. */
                temp = *allocated;
                *allocated = (*allocated)->next;
                if (temp)
                {
                    (*gc->imports.free)(gc, temp);
                }
                continue;
            }
        }

        allocated = &((*allocated)->next);
    }

    if (shared->lock)
    {
        (*gc->imports.unlockMutex)(shared->lock);
    }
}

__GLobjItem **__glLookupObjectItem(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id)
{
    __GLobjItem **buckets;
    __GLobjItem **prevp;
    __GLobjItem *item;
    __GLobjItem **result = gcvNULL;

    if (shared->lock)
    {
        (*gc->imports.lockMutex)(shared->lock);
    }

    buckets = shared->hashBuckets;
    if (buckets == gcvNULL)
    {
        /* This shared object machine has no object */
        result = gcvNULL;
    }
    else
    {
        /*
        ** The hash function is just a simple bitmask.
        */
        prevp = &buckets[id & shared->hashMask];
        item = *prevp;
        while (item)
        {
            if (item->name == id)
            {
                result = prevp;
                break;
            }
            prevp = &item->next;
            item = item->next;
        }
    }

    if (shared->lock)
    {
        (*gc->imports.unlockMutex)(shared->lock);
    }
    return result;
}

__GLobjItem *
__glFindObjItemNode(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id)
{
    __GLobjItem *item = gcvNULL;
    __GLobjItem **buckets;
    __GLobjItem **bucket;

    if (shared->lock)
    {
        (*gc->imports.lockMutex)(shared->lock);
    }

    buckets = shared->hashBuckets;
    if (buckets == gcvNULL)
    {
        /*
        ** Create hash buckets on first use.
        */
        buckets = (__GLobjItem**)(*gc->imports.calloc)(gc, 1, shared->hashSize * sizeof(GLvoid *));
        if (!buckets)
        {
            goto _Done;
        }
        shared->hashBuckets = buckets;
    }

    /*
    ** Search item list to see if the item that has the same "id" is already in.
    */
    bucket = &buckets[id & shared->hashMask];
    item = *bucket;
    while (item)
    {
        if (item->name == id)
        {
            goto _Done;
        }
        item = item->next;
    }

    /*
    ** Create a new item and add it to the list if the item with "id" does not exist.
    */
    item = (__GLobjItem *)(*gc->imports.malloc)(gc, sizeof(__GLobjItem));
    if (item != gcvNULL)
    {
        item->obj = gcvNULL;
        item->name = id;
        item->next = *bucket;
        *bucket = item;
    }

_Done:
    if (shared->lock)
    {
        (*gc->imports.unlockMutex)(shared->lock);
    }
    return item;
}

GLvoid __glFreeSharedObjectState(__GLcontext *gc, __GLsharedObjectMachine *shared)
{
    __GLobjItem **buckets = shared->hashBuckets;
    __GLnameAllocation *name = shared->nameArray;
    GLuint i;

    if (shared->lock)
    {
        (*gc->imports.lockMutex)(shared->lock);
    }

    if (shared->refcount > 1)
    {
        shared->refcount--;
        if (shared->lock)
        {
            (*gc->imports.unlockMutex)(shared->lock);
        }
        return;
    }

    /* Free every name node on "shared->nameArray" chain.
    */
    while (name)
    {
        shared->nameArray = name->next;
        (*gc->imports.free)(gc, name);
        name = shared->nameArray;
    }

    /* Free every object in the linear lookup table.
    */
    if (shared->linearTable)
    {
        for (i = 0; i < shared->linearTableSize; ++i)
        {
            GLvoid *obj = shared->linearTable[i];
            if (obj)
            {
                (*shared->deleteObject)(gc, obj);
            }
        }
    }

    /* Free every list on each hash bucket chain.
    */
    if (buckets)
    {
        for (i = 0; i < shared->hashSize; ++i)
        {
            __GLobjItem *hdr = buckets[i];
            while (hdr)
            {
                __GLobjItem *next = hdr->next;
                (*shared->deleteObject)(gc, hdr->obj);
                hdr = next;
            }
        }
    }

    /* Free the left list and hash table memory in the end
    */
    if (shared->linearTable)
    {
        (*gc->imports.free)(gc, shared->linearTable);
    }

    if (buckets)
    {
        for (i = 0; i < shared->hashSize; ++i)
        {
            __GLobjItem *hdr = buckets[i];
            while (hdr)
            {
                __GLobjItem *next = hdr->next;
                (*gc->imports.free)(gc, hdr);
                hdr = next;
            }
        }
        (*gc->imports.free)(gc, shared->hashBuckets);
    }

    if (shared->lock)
    {
        (*gc->imports.unlockMutex)(shared->lock);
        /* Destroy mutext from EGL */
        (*gc->imports.destroyMutex)(shared->lock);

        /* Free lock */
        (*gc->imports.free)(gc, shared->lock);
    }

    /* Free the shared structure itself.
    */
    (*gc->imports.free)(gc, shared);
}

GLvoid __glCheckLinearTableSize(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint size)
{
    GLvoid **oldLinearTable, *obj;
    GLuint oldTableSize, i, allocSize;

    if (shared->lock)
    {
        (*gc->imports.lockMutex)(shared->lock);
    }

    if (size > shared->linearTableSize)
    {
        oldLinearTable = shared->linearTable;
        oldTableSize = shared->linearTableSize;

        if (size < shared->maxLinearTableSize)
        {
            /* Allocate a new linear table */
            allocSize = size + 500;
            if (allocSize > shared->maxLinearTableSize)
            {
                allocSize = shared->maxLinearTableSize;
            }
            shared->linearTable = (GLvoid **)(*gc->imports.calloc)(gc, 1, allocSize * sizeof(GLvoid *));
            shared->linearTableSize = allocSize;

            /* Copy the data from the old linear table to the new linear table */
            __GL_MEMCOPY(shared->linearTable, oldLinearTable, oldTableSize * sizeof(GLvoid *));

            /* Free the old linear table */
            (*gc->imports.free)(gc, oldLinearTable);
        }
        else
        {
            /* If the linear table becomes too big then we switch to use hash table */
            for (i = 0; i < oldTableSize; i++)
            {
                obj = oldLinearTable[i];
                if (obj)
                {
                    /* Insert object to the hash table */
                    __GLobjItem *item = __glFindObjItemNode(gc, shared, i);
                    GL_ASSERT(item);
                    item->obj = obj;
                }
            }

            /* Free the old linear table and set shared->linearTable to NULL to indicates using hash table */
            (*gc->imports.free)(gc, shared->linearTable);
            shared->linearTable = gcvNULL;
            shared->linearTableSize = 0;
        }
    }

    if (shared->lock)
    {
        (*gc->imports.unlockMutex)(shared->lock);
    }
}
