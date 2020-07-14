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


#ifndef __gc_vsc_utils_base_h_
#define __gc_vsc_utils_base_h_

BEGIN_EXTERN_C()

#define VSC_NOTHING                                  do {} while (0)

#define VSC_UTILS_ALIGN(base, al)                    (((base) + (al) - 1) & ~((al) - 1))

#define SET_VALUE        0xFFFFFFFF
#define CLR_VALUE        0x00000000
#define SET_BIT_VALUE    1
#define CLR_BIT_VALUE    0

#define VSC_UTILS_MAKE_BIT_MASK(bitLoc)                       (((gctUINT)1) << (bitLoc))
#define VSC_UTILS_MAKE_LOW_MASK(highUnmaskBitCount)           ((gctUINT)0xFFFFFFFF >> (highUnmaskBitCount))
#define VSC_UTILS_MAKE_HIGH_MASK(lowUnmaskBitCount)           ((gctUINT)0xFFFFFFFF << (lowUnmaskBitCount))

/* startBitLoc should be LE endBitLoc */
#define VSC_UTILS_MAKE_BIT_RANGE_MASK(startBitLoc, endBitLoc) \
    (VSC_UTILS_MAKE_LOW_MASK(31 - (endBitLoc)) & VSC_UTILS_MAKE_HIGH_MASK((startBitLoc)))

#define VSC_UTILS_SET(uint, mask)                    ((uint) |= (mask))
#define VSC_UTILS_CLR(uint, mask)                    ((uint) &= ~(mask))
#define VSC_UTILS_TST(uint, mask)                    ((uint) & (mask))

#define VSC_UTILS_SET_BIT(uint, bitIdx)              VSC_UTILS_SET((uint), VSC_UTILS_MAKE_BIT_MASK((bitIdx)))
#define VSC_UTILS_CLR_BIT(uint, bitIdx)              VSC_UTILS_CLR((uint), VSC_UTILS_MAKE_BIT_MASK((bitIdx)))
#define VSC_UTILS_TST_BIT(uint, bitIdx)              VSC_UTILS_TST((uint), VSC_UTILS_MAKE_BIT_MASK((bitIdx)))

#define VSC_UTILS_MASK(uint, mask)                   VSC_UTILS_TST((uint), (mask))

/* Nodes of all VSC data structure (except array which is fully maintained by user) can have following
   extra VSC_BASE_NODE node member, such as list, tree, graph, hash and so on. When these basic data
   structures has no such extra member, we call them NODE which is made of concrete info standing for
   that data structure, and when such extra memmber is added, we call them as EXT_NODE which has common
   extended info, such as user-data, refcount or some debug info. Note that in EXT_NODE, NODE must be
   the FIRST member of EXT_NODE. EXT_NODE is demonstrated as following figure
   -----------------
   |      NODE     |
   -----------------
   |   BASE_NODE   |
   -----------------
   */
typedef struct _VSC_BASE_NODE
{
    /* User data that this base node contains when EXT_NODE acts as a container. Two concepts of this
       container (suppose A is EXT_NODE, and B is derived from A):
       1. If A is the first member of B, then user data can store any user side data or NULL because
          A and B can be simply casted each other.
       2. Otherwise, if A is NOT the first member of B, the user data must be used to store pointer of
          B for the purpose of that A can be easily converted to B, so it can not store other special
          user data any more */
    void*                     pUserData;
}VSC_BASE_NODE;

void  vscBSNODE_Initialize(VSC_BASE_NODE* pBaseNode, void* pOwner);
void* vscBSNODE_GetContainedUserData(VSC_BASE_NODE* pBaseNode);

/* Some basic bit operations */
#define INVALID_BIT_LOC   (-1)
gctINT vscFindPopulation(gctUINT uInt);
gctINT vscFindLeastSigBit(gctUINT uInt);
gctINT vscFindMostSigBit(gctUINT uInt);

/* Some data-digest algorithms */
gctUINT vscEvaluateCRC32(void *pData, gctUINT dataSizeInByte);

#define vscMIN(a,b) (((a) < (b)) ? (a) : (b))
#define vscMAX(a,b) (((a) > (b)) ? (a) : (b))

#define MAX_EFFECT_POW2_EXPONENT     31
static gcmINLINE gctUINT vscAlignToPow2(gctUINT base, gctUINT maxPow2Exponent)
{
    gctUINT i, maxPow2;

    maxPow2 = (1 << maxPow2Exponent);
    for (i = 1; i < maxPow2; i <<= 1)
    {
        if (i >= base)
        {
            return i;
        }
    }

    return 0;
}

END_EXTERN_C()

#endif /* __gc_vsc_utils_base_h_ */

