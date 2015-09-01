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


#include "gc_cl_common.h"

#define HASH_MULTIPLIER        31        /* the multiplier of Hash */

/* a & b are the numbers whose LCM is to be found */
gctINT
clFindLCM(
gctINT A,
gctINT B
)
{
   int n;

   for(n = (A>B) ? A : B; ; n++) {
      if(n%A == 0 && n%B == 0)
         return n;
   }
}

/* check if an integer is power of 2 */
gctBOOL
clIsPowerOf2(
gctINT IntValue
)
{
   gctINT remainder = IntValue;

   while(remainder) {
     if(remainder == 1) break;
     if(remainder & 0x1) return gcvFALSE;
     remainder >>= 1;
   }
   return gcvTRUE;
}

/* find integer's nearest power of 2 */
gctINT
clFindNearestPowerOf2(
gctINT IntValue
)
{
   gctINT powerOf2 = 1;

   while(IntValue > powerOf2) powerOf2 <<= 1;
   return powerOf2;
}

gctHASH_VALUE
clHashString(
    IN gctCONST_STRING String
    )
{
    gctHASH_VALUE        hashValue = 0;
    gctCONST_CHAR_PTR    ch;

    for (ch = String; *ch != '\0'; ch++)
    {
        hashValue = HASH_MULTIPLIER * hashValue + *ch;
    }

    return hashValue;
}

#define clmQsortPivot(i, j)  (((i) + (j)) / 2)

static void
_clQsortSwap(
IN gctCHAR_PTR x,
IN gctCHAR_PTR y,
IN gctSIZE_T size
)
{
   gctCHAR temp;
   gctSIZE_T i;

   for(i = 0; i < size; i++) {
     temp = x[i];
     x[i] = y[i];
     y[i] = temp;
   }
}

static void
_clQsort(
IN void *array,
IN gctINT m,
IN gctINT n,
IN gctSIZE_T size,
IN int ( * comparator ) ( const void *, const void * )
)
{
   gctCHAR_PTR key;
   gctINT i, j, k;

   if(m < n) {
      k = clmQsortPivot(m, n);

      _clQsortSwap((gctCHAR_PTR)array + (m * size),
                   (gctCHAR_PTR)array + (k * size), size);

      key = (gctCHAR_PTR)array + (m * size);
      i = m+1;
      j = n;
      while(i <= j) {
         while((i <= n) &&
           comparator((gctCHAR_PTR)array + (i * size), key) <= 0)
                i++;
         while((j >= m) &&
           comparator((gctCHAR_PTR)array + (j * size), key) > 0)
                j--;
         if( i < j) {
            _clQsortSwap((gctCHAR_PTR)array + (i * size),
                         (gctCHAR_PTR)array + (j * size), size);
         }
      }
      /* swap two elements */
      _clQsortSwap(key, (gctCHAR_PTR)array + (j * size), size);
      /* recursively sort the lesser array */
      _clQsort(array, m, j-1, size, comparator);
      _clQsort(array, j+1, n, size, comparator);
   }
}

void
clQuickSort(
IN void * base,
IN gctSIZE_T num,
IN gctSIZE_T size,
IN int ( * comparator ) ( const void *, const void * )
)
{
   _clQsort(base, 0, num - 1, size, comparator);
}

void *
clBsearch(
IN void *key,
IN void *base,
IN gctSIZE_T num,
IN gctSIZE_T size,
IN int ( * comparator ) ( const void *, const void * )
)
{
    gctINT        low, mid, high;
    gceSTATUS    result;

    low = 0;
    high = num - 1;

    while (low <= high) {
        mid = (low + high) / 2;
        result = comparator(key, (gctCHAR_PTR) base + (mid * size));
        if (result < 0) {
            high    = mid - 1;
        }
        else if (result > 0) {
            low = mid + 1;
        }
        else {
            return  (gctCHAR_PTR) base + (mid * size);
        }
    }
    return gcvNULL;
}
