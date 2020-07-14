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


#ifndef __gl_memmgr_h_
#define __gl_memmgr_h_
/*
** Structure for arena style memory management.  Currently this structure
** is defined here because this memory manager fits into one file, and this
** structure should be private.  Later it might make sense to break the
** memory manager into a few files, at which point this structure may need
** to be exposed.
*/

/*
** Quarter megabyte default block size.
*/
#define MINBLOCKSIZE    262144

/*
** An arena is implemented as a linked list of large blocks of memory from
** which smaller blocks are cut.  Each block has a block size, it has an
** amount of the block which has already been allocated, it has a data
** pointer to the block, and a link to the next block in the linked list.
*/
typedef struct __GLarenaBlockRec {
    GLuint size;
    GLuint allocated;
    GLvoid *data;
    struct __GLarenaBlockRec *next;
} __GLarenaBlock;

struct __GLarenaRec {
    __GLcontext *gc;
    __GLarenaBlock *firstBlock;
    __GLarenaBlock *lastBlock;
};

extern __GLarenaBlock *NewBlock(__GLcontext *gc, GLuint size);

extern GLvoid DeleteBlock(__GLcontext *gc, __GLarenaBlock *block);

/*
** Allocate a brand new arena.
*/
extern __GLarena *__glNewArena(__GLcontext *gc);

/*
** Delete an old arena (and free all memory ever allocated from it).
*/
extern GLvoid __glCondDeleteArena(__GLarena *arena);

/*
** Allocate block of memory from an arena.  This function needs to be
** as fast as possible.
*/
extern GLvoid *__glArenaAlloc(__GLarena *arena, GLuint size);

/*
** Free all memory ever allocated from this arena.  This function should
** be reasonably fast.
*/
extern GLvoid __glArenaFreeAll(__GLarena *arena);


#endif /* __gl_memmgr_h_ */
