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

/*
** The following four functions are generic ID management functions that
** are used to generate IDs for GenLists, GenTextures, etc.
*/

GLuint __glGenerateNames(__GLcontext *gc, __GLsharedObjectMachine *shared, GLsizei range)
{
    __GLnameAllocation *allocated = shared->nameArray;
    __GLnameAllocation *temp;
    GLuint start, end;

    if (!allocated) {
        /* There is no IDs allocated yet */
        allocated = (__GLnameAllocation *)
                (*gc->imports.malloc)(gc, sizeof(__GLnameAllocation) );
        allocated->next = NULL;
        allocated->start = 1;
        allocated->number = range;
        shared->nameArray = allocated;
        return 1;
    }

    /*
    ** Fill the gap in the beginning.
    ** FarCry assumes the name returned by driver is from 1. It's a game issue, but a game patch is even more
    ** expensive than the below code.
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
            temp = (__GLnameAllocation *)
                (*gc->imports.malloc)(gc, sizeof(__GLnameAllocation) );
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
    for (;;) {
        start = allocated->start + allocated->number;
        end = start + range;

        /* Check word wrap */
        if (end < start) {
            return 0;
        }

        if (!allocated->next || (end < allocated->next->start)) {
            allocated->number += range;
            return start;
        }

        /* Collapse two adjacent nodes into one. */
        if (end == allocated->next->start) {
            allocated->number += range + allocated->next->number;
            temp = allocated->next;
            allocated->next = allocated->next->next;
            if (temp) {
                (*gc->imports.free)(gc, temp);
            }
            return start;
        }
        allocated = allocated->next;
    }
}

GLboolean __glIsNameDefined(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id)
{
    __GLnameAllocation *allocated = shared->nameArray;

    if (id == 0) {
        return GL_FALSE;
    }

    while (allocated) {
        if (id < allocated->start) return(GL_FALSE);
        if (id < allocated->start + allocated->number) return(GL_TRUE);
        allocated = allocated->next;
    }
    return(GL_FALSE);
}

GLvoid __glDeleteNamesFrList(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id, GLsizei n)
{
    __GLnameAllocation **allocated = &shared->nameArray;
    __GLnameAllocation *temp;
    GLuint end, delstart, delend, blstart, blend;

    if (n <= 0) {
        return;
    }

    /* Delete it from the allocated struct */
    end = id + n;
    while (*allocated) {
        /* The start and end for this allocated block
        */
        blstart = (*allocated)->start;
        blend = blstart + (*allocated)->number;

        /*
        ** If we passed that part of our allocation id that might be
        ** affected, we stop (blstart will only keep getting larger).
        */
        if (end <= blstart) break;

        if (id < blend) {
            /*
            ** Since end > blstart and id < blend,
            ** The deletion affects this part of this block.
            */
            delstart = id;
            if (delstart < blstart) delstart = blstart;
            delend = end;
            if (delend > blend) delend = blend;

            /* Need to delete any ids from delstart to delend */

            if (delstart > blstart) {
                /*
                ** The first part of this block remains.  Might as well
                ** use the same allocated block, and just change "number".
                */
                (*allocated)->number = delstart - blstart;
                if (delend < blend) {
                    /*
                    ** Ugh.  Need to create a new block for the second
                    ** half of this newly fragmented one.
                    */
                    temp = (__GLnameAllocation*)
                    (*gc->imports.malloc)(gc, sizeof(__GLnameAllocation) );
                    temp->next = (*allocated)->next;
                    temp->start = delend;
                    temp->number = blend - delend;
                    (*allocated)->next = temp;
                    allocated = &((*allocated)->next);
                }
            } else if (delend < blend) {
                /*
                ** The last part of this block remains.  Might as well
                ** use the same allocated block, and just change the fields.
                */
                (*allocated)->number = blend - delend;
                (*allocated)->start = delend;
            } else {
                /* The whole block is gone.  Need to delete it. */
                temp = *allocated;
                *allocated = (*allocated)->next;
                if (temp) {
                    (*gc->imports.free)(gc, temp);
                }
                continue;
            }
        }

        allocated = &((*allocated)->next);
    }
}

__GLobjItem **__glLookupObjectItem(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id)
{
    __GLobjItem **buckets;
    __GLobjItem **prevp;
    __GLobjItem *item;

    buckets = shared->hashBuckets;
    if (buckets == NULL) {
        /* This shared object machine has no object */
        return NULL;
    }

    /*
    ** The hash function is just a simple bitmask.
    */
    prevp = &buckets[id & shared->hashMask];
    item = *prevp;
    while (item) {
        if (item->name == id) {
            return prevp;
        }
        prevp = &item->next;
        item = item->next;
    }
    return NULL;
}

__GLobjItem *
__glFindObjItemNode(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id)
{
    __GLobjItem *item;
    __GLobjItem **buckets;
    __GLobjItem **bucket;

    buckets = shared->hashBuckets;
    if (buckets == NULL) {
        /*
        ** Create hash buckets on first use.
        */
        buckets = (__GLobjItem**)(*gc->imports.calloc)(gc, 1, shared->hashSize * sizeof(GLvoid *) );
        if (!buckets) {
            return NULL;
        }
        shared->hashBuckets = buckets;
    }

    /*
    ** Search item list to see if the item that has the same "id" is already in.
    */
    bucket = &buckets[id & shared->hashMask];
    item = *bucket;
    while (item) {
        if (item->name == id) {
            return item;
        }
        item = item->next;
    }

    /*
    ** Create a new item and add it to the list if the item with "id" does not exist.
    */
    item = (__GLobjItem *)(*gc->imports.malloc)(gc, sizeof(__GLobjItem) );
    if (item == NULL) {
        return NULL;
    }
    item->obj = NULL;
    item->name = id;
    item->next = *bucket;
    *bucket = item;

    return item;
}

GLvoid __glFreeSharedObjectState(__GLcontext *gc, __GLsharedObjectMachine *shared)
{
    __GLobjItem **buckets;
    __GLobjItem *hdr, *next;
    __GLnameAllocation *name;
    GLuint i;

    if (shared->refcount > 1) {
        shared->refcount--;
        return;
    }

    /* Free every name node on "shared->nameArray" chain.
    */
    name = shared->nameArray;
    while (name) {
        shared->nameArray = name->next;
        (*gc->imports.free)(gc, name);
        name = shared->nameArray;
    }

    /* Free every object in the linear lookup table.
    */
    if (shared->linearTable) {
        for (i = 0; i < shared->linearTableSize; i++) {
            GLvoid *obj = shared->linearTable[i];
            if (obj) {
                (*shared->deleteObject)(gc, obj);
            }
        }
        (*gc->imports.free)(gc, shared->linearTable);
    }

    /* Free every list on each hash bucket chain.
    */
    buckets = shared->hashBuckets;
    if (buckets) {
        for (i = 0; i < shared->hashSize; i++) {
            hdr = buckets[i];
            while (hdr) {
                next = hdr->next;
                (*shared->deleteObject)(gc, hdr->obj);
                (*gc->imports.free)(gc, hdr);
                hdr = next;
            }
        }
        (*gc->imports.free)(gc, buckets);
    }

    /* Free the shared structure itself.
    */
    (*gc->imports.free)(gc, shared);
}

GLvoid __glCheckLinearTableSize(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint size)
{
    GLvoid **oldLinearTable, *obj;
    GLuint oldTableSize, i, allocSize;

    if (size > shared->linearTableSize)
    {
        oldLinearTable = shared->linearTable;
        oldTableSize = shared->linearTableSize;

        if (size < shared->maxLinearTableSize)
        {
            /* Allocate a new linear table */
            allocSize = size + 500;
            if (allocSize > shared->maxLinearTableSize) {
                allocSize = shared->maxLinearTableSize;
            }
            shared->linearTable = (GLvoid **)(*gc->imports.calloc)(gc, 1, allocSize * sizeof(GLvoid *) );
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
                if (obj) {
                    /* Insert object to the hash table */
                    __GLobjItem *item = __glFindObjItemNode(gc, shared, i);
                    item->obj = obj;
                }
            }

            /* Free the old linear table and set shared->linearTable to NULL to indicates using hash table */
            (*gc->imports.free)(gc, shared->linearTable);
            shared->linearTable = NULL;
            shared->linearTableSize = 0;
        }
    }
}