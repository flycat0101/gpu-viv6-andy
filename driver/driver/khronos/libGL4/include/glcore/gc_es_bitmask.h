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


/*
** The following are inline functions used for bitmask management
*/
#ifndef __gc_es_bitmask_h_
#define __gc_es_bitmask_h_

#define __GL_BITMASK_ELT_BITS     32
#define __GL_BITMASK_ELT_TYPE     GLuint

#define __GL_BITMASK_ELT_MAXNUM   4    /* (128 / __GL_BITMASK_ELT_BITS) */

struct __GLbitmaskFUNCS;

typedef struct __GLbitmaskRec
{
    /* Multiple element array */
    __GL_BITMASK_ELT_TYPE me[__GL_BITMASK_ELT_MAXNUM];
    /* Number of elements in elts pointers. */
    GLuint numOfElts;
    /* Size of bits */
    GLuint size;
    /* Remained size */
    GLuint remainedSize;
    /* op table */
    struct __GLbitmaskFUNCS *op;

} __GLbitmask,*GLbitmask_PTR;

typedef struct __GLbitmaskFUNCS
{
    GLboolean (*test) (GLbitmask_PTR Bitmask, GLuint Loc);
    GLvoid    (*set) (GLbitmask_PTR Bitmask, GLuint Loc);
    GLvoid    (*or)(GLbitmask_PTR BitmaskResult, GLbitmask_PTR Bitmask1, GLbitmask_PTR Bitmask2);
    GLboolean (*testAndClear)(GLbitmask_PTR Bitmask,  GLuint Loc);
    GLboolean (*isAllZero)(GLbitmask_PTR Bitmask);
    GLvoid    (*init)(GLbitmask_PTR Bitmask, GLboolean AllOne);
    GLvoid    (*clear)(GLbitmask_PTR Bitmask, GLuint Loc);
    GLvoid    (*setAll)(GLbitmask_PTR Bitmask, GLboolean AllOne);
    GLvoid    (*setValue)(GLbitmask_PTR Bitmask, GLuint Value);
} GLbitmaskFUNCS;

extern GLbitmaskFUNCS seMaskFuncs;
extern GLbitmaskFUNCS meMaskFuncs;

/*
** Initilialize size Bitmask with all bit armed.
*/
__GL_INLINE  GLvoid
__glBitmaskInitAllOne(
    GLbitmask_PTR Bitmask,
    GLuint size
    )
{
    Bitmask->size = size;

    GL_ASSERT(size <= __GL_BITMASK_ELT_BITS * __GL_BITMASK_ELT_MAXNUM);

    if (size <= __GL_BITMASK_ELT_BITS)
    {
        Bitmask->op = &seMaskFuncs;
    }
    else
    {
        Bitmask->op = &meMaskFuncs;
    }

    (*Bitmask->op->init)(Bitmask, gcvTRUE);
}

/*
** Initialize sized Bitmask with all zero set.
*/
__GL_INLINE GLvoid
__glBitmaskInitAllZero(
    GLbitmask_PTR Bitmask,
    GLuint size
    )
{
    Bitmask->size = size;

    GL_ASSERT(size <= __GL_BITMASK_ELT_BITS * __GL_BITMASK_ELT_MAXNUM);

    if (size <= __GL_BITMASK_ELT_BITS)
    {
        Bitmask->op = &seMaskFuncs;
    }
    else
    {
        Bitmask->op = &meMaskFuncs;
    }

    (*Bitmask->op->init)(Bitmask, gcvFALSE);
}

/*
** Initialize BitmaskResult with Bitmask1 or Bitmask2
** BitmaskResult can NOT be either of Bitmask1 or Bitmask2.
*/
__GL_INLINE GLvoid
__glBitmaskInitOR(
    GLbitmask_PTR BitmaskResult,
    GLbitmask_PTR Bitmask1,
    GLbitmask_PTR Bitmask2
    )
{
    GLuint size = __GL_MAX(Bitmask1->size, Bitmask2->size);

    __glBitmaskInitAllZero(BitmaskResult, size);

    (*BitmaskResult->op->or)(BitmaskResult, Bitmask1, Bitmask2);
}

/*
** Check Loc bit is armed or not in Bitmask
*/
__GL_INLINE GLboolean
__glBitmaskTest(
    GLbitmask_PTR Bitmask,
    GLuint Loc
    )
{
    return (*Bitmask->op->test)(Bitmask, Loc);
}

/*
** Check Loc bit is armed and clear it
*/
__GL_INLINE GLboolean
__glBitmaskTestAndClear(
    GLbitmask_PTR Bitmask,
    GLuint Loc
    )
{
    return (*Bitmask->op->testAndClear)(Bitmask, Loc);
}

/*
** Check all bits are armed in Bitmask or not
*/
__GL_INLINE GLboolean
__glBitmaskIsAllZero(
    GLbitmask_PTR Bitmask
    )
{
    return (*Bitmask->op->isAllZero)(Bitmask);
}

/*
** Arm Loc bit in Bitmask
*/
__GL_INLINE GLvoid
__glBitmaskSet(
    GLbitmask_PTR Bitmask,
    GLuint Loc
    )
{
    (*Bitmask->op->set)(Bitmask, Loc);
}

/*
** Clear Loc bit in Bitmask
*/
__GL_INLINE GLvoid
__glBitmaskClear(
    GLbitmask_PTR Bitmask,
    GLuint Loc
    )
{
    (*Bitmask->op->clear)(Bitmask, Loc);
}

/*
** Clear or set all bits in Bitmask
*/
__GL_INLINE GLvoid
__glBitmaskSetAll(
    GLbitmask_PTR Bitmask,
    GLboolean AllOne
    )
{
   (*Bitmask->op->setAll)(Bitmask, AllOne);
}

/*
**  Merge multiple[count] Bitmask in BitmaskArray into BitmaskResult
*/
__GL_INLINE GLvoid
__glBitmaskMergeBitMaskArray(
    GLbitmask_PTR BitmaskResult,
    GLbitmask_PTR *BitmaskArray,
    GLuint Count
    )
{
    GLuint i;
    for (i = 0; i < Count; i++)
    {
        (*BitmaskResult->op->or)(BitmaskResult, BitmaskResult, BitmaskArray[i]);
    }
    return;
}

/*
** Merge Bitmask into BitmaskResult
*/
__GL_INLINE GLvoid
__glBitmaskOR2(
    GLbitmask_PTR BitmaskResult,
    GLbitmask_PTR Bitmask
    )
{
    (*BitmaskResult->op->or)(BitmaskResult, BitmaskResult, Bitmask);
}

#endif /*#ifndef __gc_es_bitmask_h_*/

