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
    GLvoid    (*xor)(GLbitmask_PTR BitmaskResult, GLbitmask_PTR Bitmask1, GLbitmask_PTR Bitmask2);
    GLboolean (*testAndClear)(GLbitmask_PTR Bitmask,  GLuint Loc);
    GLboolean (*isAllZero)(GLbitmask_PTR Bitmask);
    GLvoid    (*init)(GLbitmask_PTR Bitmask, GLboolean AllOne);
    GLvoid    (*clear)(GLbitmask_PTR Bitmask, GLuint Loc);
    GLvoid    (*setAll)(GLbitmask_PTR Bitmask, GLboolean AllOne);
    GLvoid    (*setValue)(GLbitmask_PTR Bitmask, GLuint Value);
} GLbitmaskFUNCS;

__GL_INLINE GLboolean seMaskTest(GLbitmask_PTR Bitmask, GLuint Loc)
{
    GL_ASSERT(Loc < Bitmask->size);
    return ((Bitmask->me[0] & ((__GL_BITMASK_ELT_TYPE) 1 << Loc)) ? gcvTRUE: gcvFALSE);
}

__GL_INLINE GLvoid seMaskSet(GLbitmask_PTR Bitmask, GLuint Loc)
{
    GL_ASSERT(Loc < Bitmask->size);
    Bitmask->me[0] |= (__GL_BITMASK_ELT_TYPE) 1 << Loc;
}

__GL_INLINE GLvoid seMaskOR(GLbitmask_PTR BitmaskResult, GLbitmask_PTR Bitmask1, GLbitmask_PTR Bitmask2)
{
    BitmaskResult->me[0] = Bitmask1->me[0] | Bitmask2->me[0];
}

__GL_INLINE GLvoid seMaskXOR(GLbitmask_PTR BitmaskResult, GLbitmask_PTR Bitmask1, GLbitmask_PTR Bitmask2)
{
    BitmaskResult->me[0] = Bitmask1->me[0] ^ Bitmask2->me[0];
}

__GL_INLINE GLboolean seMaskTestAndClear(GLbitmask_PTR Bitmask, GLuint Loc)
{
    GL_ASSERT(Loc < Bitmask->size);
    if (Bitmask->me[0] & ((__GL_BITMASK_ELT_TYPE) 1 << Loc))
    {
        Bitmask->me[0] &= ~((__GL_BITMASK_ELT_TYPE) 1 << Loc);
        return gcvTRUE;
    }
    return gcvFALSE;
}

__GL_INLINE GLboolean seMaskIsAllZero(GLbitmask_PTR Bitmask)
{
    return (Bitmask->me[0]== (__GL_BITMASK_ELT_TYPE) 0);
}

__GL_INLINE GLvoid seMaskInit(GLbitmask_PTR Bitmask, GLboolean AllOne)
{
    Bitmask->numOfElts = 1;
    Bitmask->me[0] = AllOne ?  ((__GL_BITMASK_ELT_TYPE) ~0 >> (__GL_BITMASK_ELT_BITS - Bitmask->size))
                            :  (__GL_BITMASK_ELT_TYPE) 0;
}

__GL_INLINE GLvoid seMaskSetAll(GLbitmask_PTR Bitmask, GLboolean AllOne)
{
    Bitmask->me[0] = AllOne ?  ((__GL_BITMASK_ELT_TYPE) ~0 >> (__GL_BITMASK_ELT_BITS - Bitmask->size))
                            :  (__GL_BITMASK_ELT_TYPE) 0;
}

__GL_INLINE GLvoid seMaskClear(GLbitmask_PTR Bitmask, GLuint Loc)
{
    GL_ASSERT(Loc < Bitmask->size);
    Bitmask->me[0] &= ~((__GL_BITMASK_ELT_TYPE) 1 << Loc);
}

__GL_INLINE GLvoid seMaskSetValue(GLbitmask_PTR Bitmask, GLuint Value)
{
    GL_ASSERT(Bitmask->size >= 32);
    Bitmask->me[0] = (__GL_BITMASK_ELT_TYPE) Value;
}


static GLbitmaskFUNCS seMaskFuncs =
{
    seMaskTest,
    seMaskSet,
    seMaskOR,
    seMaskXOR,
    seMaskTestAndClear,
    seMaskIsAllZero,
    seMaskInit,
    seMaskClear,
    seMaskSetAll,
    seMaskSetValue,
};

__GL_INLINE GLboolean meMaskTest(GLbitmask_PTR Bitmask, GLuint Loc)
{
    GL_ASSERT(Loc < Bitmask->size);
    return ((Bitmask->me[Loc / __GL_BITMASK_ELT_BITS] & ((__GL_BITMASK_ELT_TYPE) 1 << (Loc % __GL_BITMASK_ELT_BITS))) ? gcvTRUE: gcvFALSE);
}


__GL_INLINE GLvoid meMaskSet(GLbitmask_PTR Bitmask, GLuint Loc)
{
    GL_ASSERT(Loc < Bitmask->size);
    Bitmask->me[Loc / __GL_BITMASK_ELT_BITS] |= ((__GL_BITMASK_ELT_TYPE) 1 << (Loc % __GL_BITMASK_ELT_BITS));
}

__GL_INLINE GLvoid meMaskOR(GLbitmask_PTR BitmaskResult, GLbitmask_PTR Bitmask1, GLbitmask_PTR Bitmask2)
{
    GLuint i;
    GLuint minIndex = __GL_MIN(Bitmask1->numOfElts, Bitmask2->numOfElts);
    for (i = 0; i < minIndex; i++)
    {
        BitmaskResult->me[i] = Bitmask1->me[i] | Bitmask2->me[i];
    }
}

__GL_INLINE GLvoid meMaskXOR(GLbitmask_PTR BitmaskResult, GLbitmask_PTR Bitmask1, GLbitmask_PTR Bitmask2)
{
    GLuint i;
    GLuint minIndex = __GL_MIN(Bitmask1->numOfElts, Bitmask2->numOfElts);
    for (i = 0; i < minIndex; i++)
    {
        BitmaskResult->me[i] = Bitmask1->me[i] ^ Bitmask2->me[i];
    }
}

__GL_INLINE GLboolean meMaskTestAndClear(GLbitmask_PTR Bitmask, GLuint Loc)
{
    GL_ASSERT(Loc < Bitmask->size);
    if (Bitmask->me[Loc / __GL_BITMASK_ELT_BITS] & ((__GL_BITMASK_ELT_TYPE) 1 << (Loc % __GL_BITMASK_ELT_BITS)))
    {
        Bitmask->me[Loc / __GL_BITMASK_ELT_BITS] &= ~((__GL_BITMASK_ELT_TYPE) 1 << (Loc % __GL_BITMASK_ELT_BITS));
        return gcvTRUE;
    }
    return gcvFALSE;
}

__GL_INLINE GLboolean meMaskIsAllZero(GLbitmask_PTR Bitmask)
{
    GLuint i;
    for (i = 0; i < Bitmask->numOfElts; i++)
    {
        if (Bitmask->me[i])
        {
            return gcvFALSE;
        }
    }

    return gcvTRUE;
}


__GL_INLINE GLvoid meMaskInit(GLbitmask_PTR Bitmask, GLboolean AllOne)
{
    GLuint i;
    Bitmask->numOfElts = (Bitmask->size + (__GL_BITMASK_ELT_BITS -1)) / __GL_BITMASK_ELT_BITS;
    Bitmask->remainedSize = Bitmask->size & (__GL_BITMASK_ELT_BITS -1);
    GL_ASSERT(Bitmask->numOfElts <= __GL_BITMASK_ELT_MAXNUM);

    for (i = 0; i < Bitmask->numOfElts; i++)
    {
        Bitmask->me[i] = AllOne ? (__GL_BITMASK_ELT_TYPE) ~0 : 0;
    }

    if (Bitmask->remainedSize)
    {
        Bitmask->me[Bitmask->numOfElts-1] >>= (__GL_BITMASK_ELT_BITS - Bitmask->remainedSize);
    }
}

__GL_INLINE GLvoid meMaskSetAll(GLbitmask_PTR Bitmask, GLboolean AllOne)
{
    GLuint i;
    for (i = 0; i < Bitmask->numOfElts; i++)
    {
        Bitmask->me[i] = AllOne ? (__GL_BITMASK_ELT_TYPE) ~0 : 0;
    }

    if (Bitmask->remainedSize)
    {
        Bitmask->me[Bitmask->numOfElts-1] >>= (__GL_BITMASK_ELT_BITS - Bitmask->remainedSize);
    }
}


__GL_INLINE GLvoid meMaskClear(GLbitmask_PTR Bitmask, GLuint Loc)
{
    Bitmask->me[Loc / __GL_BITMASK_ELT_BITS] &= ~((__GL_BITMASK_ELT_TYPE) 1 << (Loc % __GL_BITMASK_ELT_BITS));
}

__GL_INLINE GLvoid meMaskSetValue(GLbitmask_PTR Bitmask, GLuint Value)
{
    Bitmask->me[0] = (__GL_BITMASK_ELT_TYPE) Value;
}

static GLbitmaskFUNCS meMaskFuncs =
{
    meMaskTest,
    meMaskSet,
    meMaskOR,
    meMaskXOR,
    meMaskTestAndClear,
    meMaskIsAllZero,
    meMaskInit,
    meMaskClear,
    meMaskSetAll,
    meMaskSetValue,
};



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

/*
** Initialize BitmaskResult with Bitmask1 xor Bitmask2
** BitmaskResult can NOT be either of Bitmask1 or Bitmask2.
*/
__GL_INLINE GLvoid
__glBitmaskInitXOR(
    GLbitmask_PTR BitmaskResult,
    GLbitmask_PTR Bitmask1,
    GLbitmask_PTR Bitmask2
)
{
    GLuint size = __GL_MAX(Bitmask1->size, Bitmask2->size);

    __glBitmaskInitAllZero(BitmaskResult, size);

    (*BitmaskResult->op->xor)(BitmaskResult, Bitmask1, Bitmask2);
}

#endif /*#ifndef __gc_es_bitmask_h_*/

