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


#include <stdio.h>
#include "gc_es_context.h"
#include "memmgr.h"

/*
** Arena style memory management.
**
** This memory manager works as follows:
**
** __glNewArena is called once to allocate an arena from which memory
**   will be allocated.
** __glArenaAlloc is called many times to allocate blocks of memory from
**   the arena.
** __glArenaFreeAll is called once to free every block ever allocated from
**   the arena.
** __glCondDeleteArena is called once to deallocate the arena.
** only if no memory is allocated from the arena.  To ensure the freeing
** of all memory allocated by the arena, call glArenaFreeAll followed by
** glCondDeleteArena.
**
** The value to this memory management style is that it can be implemented
** very efficiently.
*/

/*
** Structure for arena style memory management.  Currently this structure
** is defined here because this memory manager fits into one file, and this
** structure should be private.  Later it might make sense to break the
** memory manager into a few files, at which point this structure may need
** to be exposed.
*/

__GLarenaBlock *NewBlock(__GLcontext *gc, GLuint size)
{
    __GLarenaBlock *block;

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(__GLarenaBlock), (gctPOINTER*)&block)))
    {
        return NULL;
    }

    block->next = NULL;
    block->size = size;
    block->allocated = 0;
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, size, (gctPOINTER*)&block->data)))
    {
        gcmOS_SAFE_FREE(gcvNULL, block);
        return NULL;
    }
    return block;
}

GLvoid DeleteBlock(__GLcontext *gc, __GLarenaBlock *block)
{
    if (block->data)
    {
        gcmOS_SAFE_FREE(gcvNULL, block->data);
    }

    if (block)
    {
        gcmOS_SAFE_FREE(gcvNULL, block);
    }
}

/*
** Allocate a brand new arena.
*/
__GLarena *__glNewArena(__GLcontext *gc)
{
    __GLarena *arena;
    __GLarenaBlock *block;

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(__GLarena), (gctPOINTER*)&arena)))
    {
        return NULL;
    }

    arena->gc = gc;
    block = NewBlock(gc, MINBLOCKSIZE);
    if (block == NULL) {
        gcmOS_SAFE_FREE(gcvNULL, arena);
        return NULL;
    }
    arena->firstBlock = arena->lastBlock = block;
    return arena;
}

/*
** Delete an old arena (and free all memory ever allocated from it).
*/
GLvoid __glCondDeleteArena(__GLarena *arena)
{
    __GLcontext *gc;

    gc = arena->gc;
    if (arena->firstBlock == arena->lastBlock
       && !arena->firstBlock->allocated)
    {
        DeleteBlock(gc, arena->firstBlock);
        gcmOS_SAFE_FREE(gcvNULL, arena);
    }
}

/*
** Allocate block of memory from an arena.  This function needs to be
** as fast as possible.
*/
GLvoid *__glArenaAlloc(__GLarena *arena, GLuint size)
{
    __GLcontext *gc;
    __GLarenaBlock *block, *newblock;
    GLuint bsize, allocated;

    block = arena->lastBlock;
    bsize = block->size;
    allocated = block->allocated;

    /* increase size to next highest double word aligned value    *
     * to maintain double word alignment. This assumes that the   *
     * base address of each block is at least double word aligned *
     * this will increase blocks allocated, and waste some memory *
     * but it shouldn't cause a memory leak.                      */

    size = size + (8 - (size & 0x7));

    if (size <= (bsize-allocated)) {
        block->allocated = allocated+size;
        return ((GLbyte *) block->data) + allocated;
    }

    /*
    ** Need to allocate a new block.
    */

    bsize = size;
    if (bsize < MINBLOCKSIZE) bsize = MINBLOCKSIZE;

    gc = arena->gc;
    newblock = NewBlock(gc, bsize);
    block->next = newblock;
    arena->lastBlock = newblock;
    newblock->allocated = size;
    return newblock->data;
}

/*
** Free all memory ever allocated from this arena.  This function should
** be reasonably fast.
*/
GLvoid __glArenaFreeAll(__GLarena *arena)
{
    __GLcontext *gc;
    __GLarenaBlock *block, *next;

    gc = arena->gc;
    block = arena->firstBlock;
    next = block->next;
    block->next = NULL;
    block->allocated = 0;
    arena->lastBlock = block;
    for (block = next; block; block = next) {
        next = block->next;
        DeleteBlock(gc, block);
    }
}
