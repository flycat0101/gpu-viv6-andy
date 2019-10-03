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


#include "gc_es_context.h"
#include "gc_chip_context.h"

#define _GC_OBJ_ZONE    gcdZONE_GL40_CODEC

/*************************************************************************
**
** SW Decompress Functions
**
*************************************************************************/

/************************************************************************/
/* Implementation for internal functions                                */
/************************************************************************/


/*************************************************************************
** ETC1 Decompress
*************************************************************************/

static void
gcChipDecodeETC1Block(
    GLubyte * Output,
    gctSIZE_T Stride,
    gctSIZE_T Width,
    gctSIZE_T Height,
    const GLubyte * Data
    )
{
    GLubyte base[2][3];
    GLboolean flip, diff;
    GLbyte index[2];
    gctSIZE_T i, j, x, y, offset;
    static GLubyte table[][2] =
    {
        {  2,   8 },
        {  5,  17 },
        {  9,  29 },
        { 13,  42 },
        { 18,  60 },
        { 24,  80 },
        { 33, 106 },
        { 47, 183 },
    };
    gcmHEADER_ARG("Output=0x%x Stride=%d Width=%d Height=%d Data=0x%x",
                  Output, Stride, Width, Height, Data);

    diff = Data[3] & 0x2;
    flip = Data[3] & 0x1;

    if (diff)
    {
        GLbyte delta[3];

        /* Need to extend sign bit to entire byte. */
        delta[0] = (GLbyte)((Data[0] & 0x7) << 5) >> 5;
        delta[1] = (GLbyte)((Data[1] & 0x7) << 5) >> 5;
        delta[2] = (GLbyte)((Data[2] & 0x7) << 5) >> 5;

        base[0][0] = Data[0] >> 3;
        base[0][1] = Data[1] >> 3;
        base[0][2] = Data[2] >> 3;

        base[1][0] = base[0][0] + delta[0];
        base[1][1] = base[0][1] + delta[1];
        base[1][2] = base[0][2] + delta[2];

        base[0][0] = (base[0][0] << 3) | (base[0][0] >> 2);
        base[0][1] = (base[0][1] << 3) | (base[0][1] >> 2);
        base[0][2] = (base[0][2] << 3) | (base[0][2] >> 2);

        base[1][0] = (base[1][0] << 3) | (base[1][0] >> 2);
        base[1][1] = (base[1][1] << 3) | (base[1][1] >> 2);
        base[1][2] = (base[1][2] << 3) | (base[1][2] >> 2);
    }
    else
    {
        base[0][0] = (Data[0] & 0xF0) | (Data[0] >> 4  );
        base[0][1] = (Data[1] & 0xF0) | (Data[1] >> 4  );
        base[0][2] = (Data[2] & 0xF0) | (Data[2] >> 4  );
        base[1][0] = (Data[0] << 4  ) | (Data[0] & 0x0F);
        base[1][1] = (Data[1] << 4  ) | (Data[1] & 0x0F);
        base[1][2] = (Data[2] << 4  ) | (Data[2] & 0x0F);
    }

    index[0] = (Data[3] & 0xE0) >> 5;
    index[1] = (Data[3] & 0x1C) >> 2;

    for (i = x = y = offset = 0; i < 2; ++i)
    {
        GLubyte msb = Data[5 - i];
        GLubyte lsb = Data[7 - i];

        for (j = 0; j < 8; ++j)
        {
            GLuint delta = 0;
            GLint r, g, b;
            GLint block = flip
                        ? (y < 2) ? 0 : 1
                        : (x < 2) ? 0 : 1;

            switch (((msb & 1) << 1) | (lsb & 1))
            {
            case 0x3: delta = -table[index[block]][1]; break;
            case 0x2: delta = -table[index[block]][0]; break;
            case 0x0: delta =  table[index[block]][0]; break;
            case 0x1: delta =  table[index[block]][1]; break;
            }

            r = base[block][0] + delta; r = __GL_MAX(0x00, __GL_MIN(r, 0xFF));
            g = base[block][1] + delta; g = __GL_MAX(0x00, __GL_MIN(g, 0xFF));
            b = base[block][2] + delta; b = __GL_MAX(0x00, __GL_MIN(b, 0xFF));

            if ((x < Width) && (y < Height))
            {
                Output[offset + 0] = (GLubyte) r;
                Output[offset + 1] = (GLubyte) g;
                Output[offset + 2] = (GLubyte) b;
            }

            offset += Stride;
            if (++y == 4)
            {
                y = 0;
                ++x;

                offset += 3 - 4 * Stride;
            }

            msb >>= 1;
            lsb >>= 1;
        }
    }
    gcmFOOTER_NO();
}

/*************************************************************************
** ASTC Decompress
*************************************************************************/

#define MAX_WEIGHTS_PER_BLOCK       64
#define MIN_WEIGHT_BITS_PER_BLOCK   24
#define MAX_WEIGHT_BITS_PER_BLOCK   96
#define MAX_TEXEL_COUNT             144

typedef struct __GLblockModeRect
{
    GLubyte weightPrecision;        /* Weight precision bit. */
    GLubyte dualPlane;              /* Dual-plane mode bit. */
    GLubyte widthOfGridWeights;     /* Width of the grid of weights. */
    GLubyte heightOfGridWeights;    /* Height of the grid of weights. */
    GLubyte numberOfWeights;        /* Number of weights in the block. */
    GLubyte weightBits;             /* The size of the area in bits occupied by the weigths. */
    GLubyte weightRangIndex;        /* Weight range index (precision). */
} __GLblockMode;

typedef struct __GLcolors8888Rect
{
    GLushort r;
    GLushort g;
    GLushort b;
    GLushort a;
} __GLcolors8888;

typedef struct __GLrangeEncodingRect
{
    GLubyte trits;
    GLubyte quints;
    GLubyte bits;
    GLubyte size;
} __GLrangeEncoding;

static const __GLrangeEncoding rangeEncodings[21] =
{
    /* trit  quint  bits  size */
    {  0,    0,     1,    1  },     /*  #0: 0..1 */
    {  1,    0,     0,    8  },     /*  #1: 0..2 */
    {  0,    0,     2,    2  },     /*  #2: 0..3 */
    {  0,    1,     0,    7  },     /*  #3: 0..4 */
    {  1,    0,     1,    13 },     /*  #4: 0..5 */
    {  0,    0,     3,    3  },     /*  #5: 0..7 */
    {  0,    1,     1,    10 },     /*  #6: 0..9 */
    {  1,    0,     2,    18 },     /*  #7: 0..11 */
    {  0,    0,     4,    4  },     /*  #8: 0..15 */
    {  0,    1,     2,    13 },     /*  #9: 0..19 */
    {  1,    0,     3,    23 },     /* #10: 0..23 */
    {  0,    0,     5,    5  },     /* #11: 0..31 */
    {  0,    1,     3,    16 },     /* #12: 0..39 */
    {  1,    0,     4,    28 },     /* #13: 0..47 */
    {  0,    0,     6,    6  },     /* #14: 0..63 */
    {  0,    1,     4,    19 },     /* #15: 0..79 */
    {  1,    0,     5,    33 },     /* #16: 0..95 */
    {  0,    0,     7,    7  },     /* #17: 0..127 */
    {  0,    1,     5,    22 },     /* #18: 0..159 */
    {  1,    0,     6,    38 },     /* #19: 0..191 */
    {  0,    0,     8,    8  }      /* #20: 0..255 */
};

/* scale = (1024 + blockWidth / 2) / (blockwidth -1) */
static const GLushort weightScale[] =
{
    0,
    0,
    0,
    0,
    342, /* block width 4. */
    256, /* block width 5. */
    205, /* block width 6. */
    171, /* block width 7. */
    146, /* block width 8. */
    128, /* block width 9. */
    114, /* block width 10. */
    102, /* block width 11. */
    93   /* block width 12. */
};

static const GLubyte rangeIndices[82][9] =
{
    {  5,  0,  0,  0,  0,  0,  0,  0,  0 },     /* 6 bits. */
    {  6,  1,  0,  0,  0,  0,  0,  0,  0 },
    {  8,  2,  0,  0,  0,  0,  0,  0,  0 },
    {  9,  2,  0,  0,  0,  0,  0,  0,  0 },
    { 11,  3,  1,  0,  0,  0,  0,  0,  0 },
    { 12,  4,  1,  0,  0,  0,  0,  0,  0 },     /* 11 bits. */
    { 14,  5,  2,  0,  0,  0,  0,  0,  0 },
    { 15,  5,  2,  1,  0,  0,  0,  0,  0 },
    { 17,  6,  3,  1,  0,  0,  0,  0,  0 },
    { 18,  7,  3,  1,  0,  0,  0,  0,  0 },
    { 20,  8,  4,  2,  1,  0,  0,  0,  0 },     /* 16 bits. */
    { 20,  8,  4,  2,  1,  0,  0,  0,  0 },
    { 20,  9,  5,  2,  1,  0,  0,  0,  0 },
    { 20, 10,  5,  3,  1,  0,  0,  0,  0 },
    { 20, 11,  6,  3,  2,  1,  0,  0,  0 },
    { 20, 11,  6,  4,  2,  1,  0,  0,  0 },     /* 21 bits. */
    { 20, 12,  7,  4,  2,  1,  0,  0,  0 },
    { 20, 13,  7,  4,  2,  1,  1,  0,  0 },
    { 20, 14,  8,  5,  3,  2,  1,  0,  0 },
    { 20, 14,  8,  5,  3,  2,  1,  0,  0 },
    { 20, 15,  9,  5,  4,  2,  1,  1,  0 },     /* 26 bits. */
    { 20, 16,  9,  6,  4,  2,  1,  1,  0 },
    { 20, 17, 10,  6,  4,  3,  2,  1,  0 },
    { 20, 17, 10,  7,  4,  3,  2,  1,  1 },
    { 20, 18, 11,  7,  5,  3,  2,  1,  1 },
    { 20, 19, 11,  7,  5,  3,  2,  1,  1 },
    { 20, 20, 12,  8,  5,  4,  2,  2,  1 },     /* 32 bits. */
    { 20, 20, 12,  8,  5,  4,  3,  2,  1 },
    { 20, 20, 13,  8,  6,  4,  3,  2,  1 },
    { 20, 20, 13,  9,  6,  4,  3,  2,  1 },
    { 20, 20, 14,  9,  7,  5,  3,  2,  2 },
    { 20, 20, 14, 10,  7,  5,  4,  2,  2 },     /* 37 bits. */
    { 20, 20, 15, 10,  7,  5,  4,  3,  2 },
    { 20, 20, 15, 10,  7,  5,  4,  3,  2 },
    { 20, 20, 16, 11,  8,  6,  4,  3,  2 },
    { 20, 20, 16, 11,  8,  6,  4,  3,  2 },
    { 20, 20, 17, 11,  8,  6,  5,  4,  3 },     /* 42 bits. */
    { 20, 20, 17, 12,  8,  6,  5,  4,  3 },
    { 20, 20, 18, 12,  9,  7,  5,  4,  3 },
    { 20, 20, 18, 13,  9,  7,  5,  4,  3 },
    { 20, 20, 19, 13, 10,  7,  5,  4,  3 },
    { 20, 20, 19, 13, 10,  7,  6,  4,  4 },     /* 47 bits. */
    { 20, 20, 20, 14, 10,  8,  6,  5,  4 },
    { 20, 20, 20, 14, 10,  8,  6,  5,  4 },
    { 20, 20, 20, 14, 11,  8,  6,  5,  4 },
    { 20, 20, 20, 15, 11,  8,  7,  5,  4 },
    { 20, 20, 20, 15, 11,  9,  7,  5,  4 },
    { 20, 20, 20, 16, 11,  9,  7,  5,  4 },
    { 20, 20, 20, 16, 12,  9,  7,  6,  5 },
    { 20, 20, 20, 16, 12,  9,  7,  6,  5 },
    { 20, 20, 20, 17, 13, 10,  8,  6,  5 },
    { 20, 20, 20, 17, 13, 10,  8,  6,  5 },
    { 20, 20, 20, 17, 13, 10,  8,  7,  5 },
    { 20, 20, 20, 18, 13, 10,  8,  7,  5 },
    { 20, 20, 20, 18, 14, 11,  8,  7,  6 },
    { 20, 20, 20, 19, 14, 11,  9,  7,  6 },
    { 20, 20, 20, 19, 14, 11,  9,  7,  6 },
    { 20, 20, 20, 19, 14, 11,  9,  7,  6 },
    { 20, 20, 20, 20, 15, 12,  9,  8,  6 },
    { 20, 20, 20, 20, 15, 12, 10,  8,  7 },
    { 20, 20, 20, 20, 16, 12, 10,  8,  7 },
    { 20, 20, 20, 20, 16, 12, 10,  8,  7 },
    { 20, 20, 20, 20, 16, 13, 10,  8,  7 },
    { 20, 20, 20, 20, 16, 13, 10,  8,  7 },
    { 20, 20, 20, 20, 17, 13, 11,  9,  7 },
    { 20, 20, 20, 20, 17, 13, 11,  9,  7 },
    { 20, 20, 20, 20, 17, 14, 11,  9,  8 },
    { 20, 20, 20, 20, 17, 14, 11,  9,  8 },
    { 20, 20, 20, 20, 18, 14, 11, 10,  8 },
    { 20, 20, 20, 20, 18, 14, 12, 10,  8 },
    { 20, 20, 20, 20, 19, 15, 12, 10,  8 },
    { 20, 20, 20, 20, 19, 15, 12, 10,  8 },
    { 20, 20, 20, 20, 19, 15, 12, 10,  9 },
    { 20, 20, 20, 20, 19, 15, 13, 10,  9 },
    { 20, 20, 20, 20, 20, 16, 13, 11,  9 },
    { 20, 20, 20, 20, 20, 16, 13, 11,  9 },
    { 20, 20, 20, 20, 20, 16, 13, 11,  9 },
    { 20, 20, 20, 20, 20, 16, 13, 11, 10 },
    { 20, 20, 20, 20, 20, 17, 14, 11, 10 },
    { 20, 20, 20, 20, 20, 17, 14, 11, 10 },
    { 20, 20, 20, 20, 20, 17, 14, 12, 10 },
    { 20, 20, 20, 20, 20, 17, 14, 12, 10 }
};

/* Set bits in a value. */
static GLubyte
__glUtilSetBits(
    GLubyte *Data,
    const GLuint Start,
    const GLuint End,
    const GLubyte Value
    )
{
    if (End >= Start)
    {
        /* Compute the mask. */
        const GLubyte _Mask =  (GLubyte)((~0ULL >> (63 - End + Start)) << Start);

        /* Zero out the destination. */
        *Data &= ~_Mask;

        /* Set the destination to the value. */
        *Data |= (Value << Start) & _Mask;

        /* Return the result value. */
        return *Data;
    }
    else
    {
        /* Compute the mask. */
        const GLubyte _Mask =  (GLubyte)((~0ULL >> (63 - Start + End)) << End);

        /* Zero out the destination. */
        *Data &= ~_Mask;

        /* Set the destination to the value. */
        *Data |= (Value << End) & _Mask;

        /* Return the result value. */
        return *Data;
    }
}

static GLushort
__glUtilSetShortBits(
    GLushort *Data,
    const GLuint Start,
    const GLuint End,
    const GLushort Value
    )
{
    if (End >= Start)
    {
        /* Compute the mask. */
        const GLushort _Mask =  (GLushort)((~0ULL >> (63 - End + Start)) << Start);

        /* Zero out the destination. */
        *Data &= ~_Mask;

        /* Set the destination to the value. */
        *Data |= (Value << Start) & _Mask;

        /* Return the result value. */
        return *Data;
    }
    else
    {
        /* Compute the mask. */
        const GLushort _Mask =  (GLushort)((~0ULL >> (63 - Start + End)) << End);

        /* Zero out the destination. */
        *Data &= ~_Mask;

        /* Set the destination to the value. */
        *Data |= (Value << End) & _Mask;

        /* Return the result value. */
        return *Data;
    }
}

static GLuint64
__glUtilSet64Bits(
    GLuint64 *Data,
    const GLuint Start,
    const GLuint End,
    const GLuint64 Value
    )
{
    if (End >= Start)
    {
        /* Compute the mask. */
        const GLuint64 _Mask =  (GLuint64)((~0ULL >> (63 - End + Start)) << Start);

        /* Zero out the destination. */
        *Data &= ~_Mask;

        /* Set the destination to the value. */
        *Data |= (Value << Start) & _Mask;

        /* Return the result value. */
        return *Data;
    }
    else
    {
        /* Compute the mask. */
        const GLuint64 _Mask =  (GLuint64)((~0ULL >> (63 - Start + End)) << End);

        /* Zero out the destination. */
        *Data &= ~_Mask;

        /* Set the destination to the value. */
        *Data |= (Value << End) & _Mask;

        /* Return the result value. */
        return *Data;
    }
}

static GLuint
__glUtilSetNumOfBits(
    GLvoid *Data,
    const GLuint Start,
    const GLuint Count,
    const GLuint Value
    )
{
    /* Handle special case when Count == 0 */
    /* because ~0ULL cannot be shifted 64 bits. */
    if (Count > 0)
    {
        /* Compute the mask. */
        const GLuint _Mask =  (GLuint)((~0ULL >> (64 - Count)) << Start);

        /* Zero out the destination. */
        *(GLuint*)Data &= ~_Mask;

        /* Set the destination to the value. */
        *(GLuint*)Data |= (((GLuint) Value) << Start) & _Mask;
    }

    /* Return the result value. */
    return *(GLuint*)Data;
}

/* Extract bits from a value. */
static GLuint
__glUtilGetBits(
    const GLuint64 Data,
    const GLuint Start,
    const GLuint End
    )
{
    if (End >= Start)
    {
        const GLuint64 Mask = (GLuint64)(~0ULL >> (63 - End + Start));
        return (GLuint)((Data >> Start) & Mask);
    }
    else
    {
        const GLuint64 Mask = (GLuint64)(~0ULL >> (63 - Start + End));
        return (GLuint)((Data >> End) & Mask);
    }
}

static GLuint64
__glUtilReadBlock(
    const GLubyte * data,
    GLuint start,
    GLuint count,
    GLboolean reverse)
{
    GLubyte offset;
    GLubyte shift;
    GLubyte bitpos;
    GLuint64 temp;
    GLuint64 mask;
    GLuint64 value;
    GLuint i;

    /* Reverse the start position if needed. */
    if (reverse)
    {
        start = 128 - (start + count);
    }

    /* Determine data position. */
    offset = (GLubyte)start >> 3;
    shift = start & 7;

    /* Extract the value. */
    /* For Android, if address is not aligned, it may crash. */
    {
        const GLubyte * ptr = &data[offset];
        temp =((GLuint64)ptr[0])            | (((GLuint64)ptr[1]) << 8)
              | (((GLuint64)ptr[2]) << 16)  | (((GLuint64)ptr[3]) << 24)
              | (((GLuint64) ptr[4]) << 32) | (((GLuint64) ptr[5]) << 40)
              | (((GLuint64) ptr[6]) << 48) | (((GLuint64) ptr[7]) << 56);
    }

    if (reverse)
    {
        value = 0;
        for (i = 0; i < count; i += 1)
        {
            bitpos = (GLubyte)(count - i - 1);
            __glUtilSet64Bits(&value, i, i, (GLuint64) __glUtilGetBits(temp, shift + bitpos, shift + bitpos));
        }
    }
    else
    {
        mask = (1LL << count) - 1;
        value = (temp >> shift) & mask;
    }

    return value;
}

/*** Returns the total number of bits that it takes to hold the specified
* number of items encoded with the specified precision using
* Integer Sequence Encoding (ISE). ***/
__GL_INLINE GLuint
__glUtilGetISEBitCount(
    GLuint Count,
    GLuint Precision)
{
    switch (Precision)
    {
    case 0:
        return Count;

    case 1:
        return (8 * Count + 4) / 5;

    case 2:
        return 2 * Count;

    case 3:
        return (7 * Count + 2) / 3;

    case 4:
        return (13 * Count + 4) / 5;

    case 5:
        return 3 * Count;

    case 6:
        return (10 * Count + 2) / 3;

    case 7:
        return (18 * Count + 4) / 5;

    case 8:
        return Count * 4;

    case 9:
        return (13 * Count + 2) / 3;

    case 10:
        return (23 * Count + 4) / 5;

    case 11:
        return 5 * Count;

    default:
        return 0;
    }
}

/* return 0 on invalid mode, 1 on valid mode. */
static GLint
__glUtilDecodeBlockMode(
    GLint BlockMode,
    __GLblockMode *ASTCBlock)
{
    GLubyte A, B;
    GLubyte R;

    /* Reserved mode? */
    if (__glUtilGetBits(BlockMode, 0, 3) == 0)
        return 0;

    ASTCBlock->weightPrecision = (GLubyte) __glUtilGetBits(BlockMode, 9, 9);
    ASTCBlock->dualPlane = (GLubyte) __glUtilGetBits(BlockMode, 10, 10);

    A = (GLubyte) __glUtilGetBits(BlockMode, 5, 6);
    R = (GLubyte) __glUtilGetBits (BlockMode, 4, 4);

    /* If true, this means we've got the first 5 lines (see table). */
    if (__glUtilGetBits(BlockMode, 0, 1) != 0)
    {
        __glUtilSetBits(&R, 1, 2, (GLubyte) __glUtilGetBits(BlockMode, 0, 1));
        B = (GLubyte) __glUtilGetBits(BlockMode, 7, 8);

        switch (__glUtilGetBits(BlockMode, 2, 3))
        {
        case 0:
            /* Line #1. */
            ASTCBlock->widthOfGridWeights = B + 4;
            ASTCBlock->heightOfGridWeights = A + 2;
            break;

        case 1:
            /* Line #2. */
            ASTCBlock->widthOfGridWeights = B + 8;
            ASTCBlock->heightOfGridWeights = A + 2;
            break;

        case 2:
            /* Line #3. */
            ASTCBlock->widthOfGridWeights = A + 2;
            ASTCBlock->heightOfGridWeights = B + 8;
            break;

        case 3:
            /* Lines #4 or #5. */
            B = (GLubyte) __glUtilGetBits(BlockMode, 7, 7);

            if (__glUtilGetBits(BlockMode, 8, 8) == 0)
            {
                /* Line #4. */
                ASTCBlock->widthOfGridWeights = A + 2;
                ASTCBlock->heightOfGridWeights = B + 6;
            }
            else
            {
                /* Line #5. */
                ASTCBlock->widthOfGridWeights = B + 2;
                ASTCBlock->heightOfGridWeights = A + 2;
            }
            break;
        }
    }
    else
    {
        if (__glUtilGetBits(BlockMode, 6, 8) == 7)
            return 0;

        __glUtilSetBits(&R, 1, 2, (GLubyte) __glUtilGetBits(BlockMode, 2, 3));
        B = (GLubyte) __glUtilGetBits(BlockMode, 9, 10);

        switch (__glUtilGetBits(BlockMode, 7, 8))
        {
        case 0:
            /* Line #6. */
            ASTCBlock->widthOfGridWeights = 12;
            ASTCBlock->heightOfGridWeights = A + 2;
            break;

        case 1:
            /* Line #7. */
            ASTCBlock->widthOfGridWeights = A + 2;
            ASTCBlock->heightOfGridWeights = 12;
            break;

        case 2:
            /* Line #10. */
            ASTCBlock->widthOfGridWeights = A + 6;
            ASTCBlock->heightOfGridWeights = B + 6;
            ASTCBlock->dualPlane = 0;
            ASTCBlock->weightPrecision = 0;
            break;

        case 3:
            switch (__glUtilGetBits(BlockMode, 5, 5))
            {
            case 0:
                /* Line #8. */
                ASTCBlock->widthOfGridWeights = 6;
                ASTCBlock->heightOfGridWeights = 10;
                break;

            case 1:
                /* Line #9. */
                ASTCBlock->widthOfGridWeights = 10;
                ASTCBlock->heightOfGridWeights = 6;
                break;
            }

            break;
        }
    }

    /* Compute the number of weights in the block. */
    ASTCBlock->numberOfWeights = ASTCBlock->widthOfGridWeights * ASTCBlock->heightOfGridWeights * (ASTCBlock->dualPlane ? 2: 1);
    if (ASTCBlock->numberOfWeights > MAX_WEIGHTS_PER_BLOCK)
        return 0;

    /* Compute the weight range index. */
    ASTCBlock->weightRangIndex = (R - 2) + 6 * ASTCBlock->weightPrecision;

    /* Compute the number of bits that hold the weights. */
    ASTCBlock->weightBits = (GLubyte) __glUtilGetISEBitCount(ASTCBlock->numberOfWeights, ASTCBlock->weightRangIndex);
    if ((ASTCBlock->weightBits < MIN_WEIGHT_BITS_PER_BLOCK) ||
        (ASTCBlock->weightBits > MAX_WEIGHT_BITS_PER_BLOCK))
        return 0;

    return 1;
}

static void
__glUtilDecodeISE(
    const GLubyte *Block,
    GLubyte Offset,
    GLboolean Reverse,
    GLushort RangeIndex,
    GLushort ItemCount,
    GLubyte *Items)
{
    GLubyte bitcount;
    GLushort itemsleft;
    const __GLrangeEncoding *rangeEncoding = &rangeEncodings[RangeIndex];

    /* Get range encoding entry. */
    if (RangeIndex >= (sizeof(rangeEncodings) / sizeof(rangeEncodings[0])))
    {
        GL_ASSERT(GL_FALSE);
    }

    if (rangeEncoding->trits)
    {
        static const GLubyte tritsize[] = { 0, 2, 4, 5, 7 };

        for (itemsleft = ItemCount; itemsleft != 0;)
        {
            GLuint64 packedbits;
            GLubyte T = 0;
            GLubyte C = 0;
            GLubyte t0, t1, t2, t3, t4;

            /* Determine the number of bits to read. */
            if (itemsleft >= 5)
            {
                bitcount = rangeEncoding->size;
                itemsleft -= 5;
            }
            else
            {
                bitcount = (GLubyte)(rangeEncoding->bits * itemsleft + tritsize[itemsleft]);
                itemsleft = 0;
            }

            /* Read the packed block. */
            packedbits = __glUtilReadBlock(Block, Offset, bitcount, Reverse);
            Offset += rangeEncoding->size;

            switch (rangeEncoding->bits)
            {
            case 0:
                T = (GLubyte) packedbits;

                Items[0] = 0;
                Items[1] = 0;
                Items[2] = 0;
                Items[3] = 0;
                Items[4] = 0;
                break;

            case 1:
                __glUtilSetBits(&T, 0, 1, (GLubyte) __glUtilGetBits(packedbits,  1,  2));
                __glUtilSetBits(&T, 2, 3, (GLubyte) __glUtilGetBits(packedbits,  4,  5));
                __glUtilSetBits(&T, 4, 4, (GLubyte) __glUtilGetBits(packedbits,  7,  7));
                __glUtilSetBits(&T, 5, 6, (GLubyte) __glUtilGetBits(packedbits,  9, 10));
                __glUtilSetBits(&T, 7, 7, (GLubyte) __glUtilGetBits(packedbits, 12, 12));

                Items[0] = (GLubyte) __glUtilGetBits((GLuint)packedbits,  0, 0);
                Items[1] = (GLubyte) __glUtilGetBits((GLuint)packedbits,  3, 3);
                Items[2] = (GLubyte) __glUtilGetBits((GLuint)packedbits,  6, 6);
                Items[3] = (GLubyte) __glUtilGetBits((GLuint)packedbits,  8, 8);
                Items[4] = (GLubyte) __glUtilGetBits((GLuint)packedbits, 11, 11);
                break;

            case 2:
                __glUtilSetBits(&T, 0, 1, (GLubyte) __glUtilGetBits(packedbits,  2,  3));
                __glUtilSetBits(&T, 2, 3, (GLubyte) __glUtilGetBits(packedbits,  6,  7));
                __glUtilSetBits(&T, 4, 4, (GLubyte) __glUtilGetBits(packedbits, 10, 10));
                __glUtilSetBits(&T, 5, 6, (GLubyte) __glUtilGetBits(packedbits, 13, 14));
                __glUtilSetBits(&T, 7, 7, (GLubyte) __glUtilGetBits(packedbits, 17, 17));

                Items[0] = (GLubyte) __glUtilGetBits(packedbits,  0,  1);
                Items[1] = (GLubyte) __glUtilGetBits(packedbits,  4,  5);
                Items[2] = (GLubyte) __glUtilGetBits(packedbits,  8,  9);
                Items[3] = (GLubyte) __glUtilGetBits(packedbits, 11, 12);
                Items[4] = (GLubyte) __glUtilGetBits(packedbits, 15, 16);
                break;

            case 3:
                __glUtilSetBits(&T, 0, 1, (GLubyte) __glUtilGetBits(packedbits,  3,  4));
                __glUtilSetBits(&T, 2, 3, (GLubyte) __glUtilGetBits(packedbits,  8,  9));
                __glUtilSetBits(&T, 4, 4, (GLubyte) __glUtilGetBits(packedbits, 13, 13));
                __glUtilSetBits(&T, 5, 6, (GLubyte) __glUtilGetBits(packedbits, 17, 18));
                __glUtilSetBits(&T, 7, 7, (GLubyte) __glUtilGetBits(packedbits, 22, 22));

                Items[0] = (GLubyte) __glUtilGetBits(packedbits,  0,  2);
                Items[1] = (GLubyte) __glUtilGetBits(packedbits,  5,  7);
                Items[2] = (GLubyte) __glUtilGetBits(packedbits, 10, 12);
                Items[3] = (GLubyte) __glUtilGetBits(packedbits, 14, 16);
                Items[4] = (GLubyte) __glUtilGetBits(packedbits, 19, 21);
                break;

            case 4:
                __glUtilSetBits(&T, 0, 1, (GLubyte) __glUtilGetBits(packedbits,  4,  5));
                __glUtilSetBits(&T, 2, 3, (GLubyte) __glUtilGetBits(packedbits, 10, 11));
                __glUtilSetBits(&T, 4, 4, (GLubyte) __glUtilGetBits(packedbits, 16, 16));
                __glUtilSetBits(&T, 5, 6, (GLubyte) __glUtilGetBits(packedbits, 21, 22));
                __glUtilSetBits(&T, 7, 7, (GLubyte) __glUtilGetBits(packedbits, 27, 27));

                Items[0] = (GLubyte) __glUtilGetBits(packedbits,  0,  3);
                Items[1] = (GLubyte) __glUtilGetBits(packedbits,  6,  9);
                Items[2] = (GLubyte) __glUtilGetBits(packedbits, 12, 15);
                Items[3] = (GLubyte) __glUtilGetBits(packedbits, 17, 20);
                Items[4] = (GLubyte) __glUtilGetBits(packedbits, 23, 26);
                break;

            case 5:
                __glUtilSetBits(&T, 0, 1, (GLubyte) __glUtilGetBits(packedbits,  5,  6));
                __glUtilSetBits(&T, 2, 3, (GLubyte) __glUtilGetBits(packedbits, 12, 13));
                __glUtilSetBits(&T, 4, 4, (GLubyte) __glUtilGetBits(packedbits, 19, 19));
                __glUtilSetBits(&T, 5, 6, (GLubyte) __glUtilGetBits(packedbits, 25, 26));
                __glUtilSetBits(&T, 7, 7, (GLubyte) __glUtilGetBits(packedbits, 32, 32));

                Items[0] = (GLubyte) __glUtilGetBits(packedbits,  0,  4);
                Items[1] = (GLubyte) __glUtilGetBits(packedbits,  7, 11);
                Items[2] = (GLubyte) __glUtilGetBits(packedbits, 14, 18);
                Items[3] = (GLubyte) __glUtilGetBits(packedbits, 20, 24);
                Items[4] = (GLubyte) __glUtilGetBits(packedbits, 27, 31);
                break;

            case 6:
                __glUtilSetBits(&T, 0, 1, (GLubyte) __glUtilGetBits(packedbits,  6,  7));
                __glUtilSetBits(&T, 2, 3, (GLubyte) __glUtilGetBits(packedbits, 14, 15));
                __glUtilSetBits(&T, 4, 4, (GLubyte) __glUtilGetBits(packedbits, 22, 22));
                __glUtilSetBits(&T, 5, 6, (GLubyte) __glUtilGetBits(packedbits, 29, 30));
                __glUtilSetBits(&T, 7, 7, (GLubyte) __glUtilGetBits(packedbits, 37, 37));

                Items[0] = (GLubyte) __glUtilGetBits(packedbits,  0,  5);
                Items[1] = (GLubyte) __glUtilGetBits(packedbits,  8, 13);
                Items[2] = (GLubyte) __glUtilGetBits(packedbits, 16, 21);
                Items[3] = (GLubyte) __glUtilGetBits(packedbits, 23, 28);
                Items[4] = (GLubyte) __glUtilGetBits(packedbits, 31, 36);
                break;

            default:
                GL_ASSERT(GL_FALSE);
            }

            /* Derive t0 to t4 from T. */
            if (__glUtilGetBits(T, 2, 4) == 7)
            {
                __glUtilSetBits(&C, 0, 1, (GLubyte) __glUtilGetBits(T, 0, 1));
                __glUtilSetBits(&C, 2, 4, (GLubyte) __glUtilGetBits(T, 5, 7));

                t3 = 2;
                t4 = 2;
            }
            else
            {
                __glUtilSetBits(&C, 0, 4, (GLubyte) __glUtilGetBits(T, 0, 4));

                if (__glUtilGetBits(T, 5, 6) == 3)
                {
                    t3 = (GLubyte) __glUtilGetBits(T, 7, 7);
                    t4 = 2;
                }
                else
                {
                    t3 = (GLubyte) __glUtilGetBits(T, 5, 6);
                    t4 = (GLubyte) __glUtilGetBits(T, 7, 7);
                }
            }

            if (__glUtilGetBits(C, 0, 1) == 3)
            {
                t0 = 0;
                __glUtilSetBits(&t0, 0, 0, (GLubyte) __glUtilGetBits(C, 2, 2) & ~__glUtilGetBits(C, 3, 3));
                __glUtilSetBits(&t0, 1, 1, (GLubyte) __glUtilGetBits(C, 3, 3));
                t1 = (GLubyte) __glUtilGetBits(C, 4, 4);
                t2 = 2;
            }
            else if (__glUtilGetBits(C, 2, 3) == 3)
            {
                t0 = (GLubyte) __glUtilGetBits(C, 0, 1);
                t1 = 2;
                t2 = 2;
            }
            else
            {
                t0 = 0;
                __glUtilSetBits(&t0, 0, 0, (GLubyte) __glUtilGetBits(C, 0, 0) & ~__glUtilGetBits(C, 1, 1));
                __glUtilSetBits(&t0, 1, 1, (GLubyte) __glUtilGetBits(C, 1, 1));
                t1 = (GLubyte) __glUtilGetBits(C, 2, 3);
                t2 = (GLubyte) __glUtilGetBits(C, 4, 4);
            }

            /* Set the MSBs. */
            __glUtilSetNumOfBits(&Items[0], rangeEncoding->bits, 2, t0);
            __glUtilSetNumOfBits(&Items[1], rangeEncoding->bits, 2, t1);
            __glUtilSetNumOfBits(&Items[2], rangeEncoding->bits, 2, t2);
            __glUtilSetNumOfBits(&Items[3], rangeEncoding->bits, 2, t3);
            __glUtilSetNumOfBits(&Items[4], rangeEncoding->bits, 2, t4);
            /* Advance to the next five items. */
            Items += 5;
        }
    }
    else if (rangeEncoding->quints)
    {
        static const GLubyte quintsize[] = { 0, 3, 5 };

        for (itemsleft = ItemCount; itemsleft != 0;)
        {
            GLubyte Q = 0;
            GLubyte C;
            GLubyte q0, q1, q2;
            GLuint packedbits;
            /* Determine the number of bits to read. */
            if (itemsleft >= 3)
            {
                bitcount = rangeEncoding->size;
                itemsleft -= 3;
            }
            else
            {
                bitcount = (GLubyte)(rangeEncoding->bits * itemsleft + quintsize[itemsleft]);
                itemsleft = 0;
            }

            /* Read the packed block. */
            packedbits = (GLuint) __glUtilReadBlock(Block, Offset, bitcount, Reverse);
            Offset += rangeEncoding->size;

            switch (rangeEncoding->bits)
            {
            case 0:
                Q = (GLubyte) packedbits;

                Items[0] = 0;
                Items[1] = 0;
                Items[2] = 0;
                break;

            case 1:
                __glUtilSetBits(&Q, 0, 2, (GLubyte) __glUtilGetBits(packedbits,  1, 3));
                __glUtilSetBits(&Q, 3, 4, (GLubyte) __glUtilGetBits(packedbits,  5, 6));
                __glUtilSetBits(&Q, 5, 6, (GLubyte) __glUtilGetBits(packedbits,  8, 9));

                Items[0] = (GLubyte) __glUtilGetBits(packedbits, 0, 0);
                Items[1] = (GLubyte) __glUtilGetBits(packedbits, 4, 4);
                Items[2] = (GLubyte) __glUtilGetBits(packedbits, 7, 7);
                break;

            case 2:
                __glUtilSetBits(&Q, 0, 2, (GLubyte) __glUtilGetBits(packedbits,  2,  4));
                __glUtilSetBits(&Q, 3, 4, (GLubyte) __glUtilGetBits(packedbits,  7,  8));
                __glUtilSetBits(&Q, 5, 6, (GLubyte) __glUtilGetBits(packedbits, 11, 12));

                Items[0] = (GLubyte) __glUtilGetBits(packedbits, 0,  1);
                Items[1] = (GLubyte) __glUtilGetBits(packedbits, 5,  6);
                Items[2] = (GLubyte) __glUtilGetBits(packedbits, 9, 10);
                break;

            case 3:
                __glUtilSetBits(&Q, 0, 2, (GLubyte) __glUtilGetBits(packedbits,  3,  5));
                __glUtilSetBits(&Q, 3, 4, (GLubyte) __glUtilGetBits(packedbits,  9, 10));
                __glUtilSetBits(&Q, 5, 6, (GLubyte) __glUtilGetBits(packedbits, 14, 15));

                Items[0] = (GLubyte) __glUtilGetBits(packedbits,  0,  2);
                Items[1] = (GLubyte) __glUtilGetBits(packedbits,  6,  8);
                Items[2] = (GLubyte) __glUtilGetBits(packedbits, 11, 13);
                break;

            case 4:
                __glUtilSetBits(&Q, 0, 2, (GLubyte) __glUtilGetBits(packedbits,  4,  6));
                __glUtilSetBits(&Q, 3, 4, (GLubyte) __glUtilGetBits(packedbits, 11, 12));
                __glUtilSetBits(&Q, 5, 6, (GLubyte) __glUtilGetBits(packedbits, 17, 18));

                Items[0] = (GLubyte) __glUtilGetBits(packedbits,  0,  3);
                Items[1] = (GLubyte) __glUtilGetBits(packedbits,  7, 10);
                Items[2] = (GLubyte) __glUtilGetBits(packedbits, 13, 16);
                break;

            case 5:
                __glUtilSetBits(&Q, 0, 2, (GLubyte) __glUtilGetBits(packedbits,  5,  7));
                __glUtilSetBits(&Q, 3, 4, (GLubyte) __glUtilGetBits(packedbits, 13, 14));
                __glUtilSetBits(&Q, 5, 6, (GLubyte) __glUtilGetBits(packedbits, 20, 21));

                Items[0] = (GLubyte) __glUtilGetBits(packedbits,  0,  4);
                Items[1] = (GLubyte) __glUtilGetBits(packedbits,  8, 12);
                Items[2] = (GLubyte) __glUtilGetBits(packedbits, 15, 19);
                break;

            default:
                GL_ASSERT(GL_FALSE);
            }

            /* Derive q0 to q2 from Q. */
            if ((__glUtilGetBits(Q, 1, 2) == 3) && (__glUtilGetBits(Q, 5, 6) == 0))
            {
                q0 = 4;
                q1 = 4;
                q2 = 0;
                __glUtilSetBits(&q2, 0, 0, (GLubyte) __glUtilGetBits(Q, 3, 3) & ~__glUtilGetBits(Q, 0, 0));
                __glUtilSetBits(&q2, 1, 1, (GLubyte) __glUtilGetBits(Q, 4, 4) & ~__glUtilGetBits(Q, 0, 0));
                __glUtilSetBits(&q2, 2, 2, (GLubyte) __glUtilGetBits(Q, 0, 0));
            }
            else
            {
                if (__glUtilGetBits(Q, 1, 2) == 3)
                {
                    __glUtilSetBits(&C, 0, 0,  (GLubyte) __glUtilGetBits(Q, 0, 0));
                    __glUtilSetBits(&C, 1, 2, ~((GLubyte) __glUtilGetBits(Q, 5, 6)));
                    __glUtilSetBits(&C, 3, 4,  (GLubyte) __glUtilGetBits(Q, 3, 4));
                    q2 = 4;
                }
                else
                {
                    C  = (GLubyte) __glUtilGetBits(Q, 0, 4);
                    q2 = (GLubyte) __glUtilGetBits(Q, 5, 6);
                }

                if (__glUtilGetBits(C, 0, 2) == 5)
                {
                    q0 = (GLubyte) __glUtilGetBits(C, 3, 4);
                    q1 = 4;
                }
                else
                {
                    q0 = (GLubyte) __glUtilGetBits(C, 0, 2);
                    q1 = (GLubyte) __glUtilGetBits(C, 3, 4);
                }
            }

            __glUtilSetNumOfBits(&Items[0], rangeEncoding->bits, 3, q0);
            __glUtilSetNumOfBits(&Items[1], rangeEncoding->bits, 3, q1);
            __glUtilSetNumOfBits(&Items[2], rangeEncoding->bits, 3, q2);

            /* Advance to the next three items. */
            Items += 3;
        }
    }
    else
    {
        GLubyte i;
        for (i = 0; i < ItemCount; i += 1)
        {
            Items[i] = (GLubyte) __glUtilReadBlock(Block, Offset,
                rangeEncoding->size, Reverse);
            Offset += rangeEncoding->size;
        }
    }
}

static void
__glUtilUnquantizeWeights(
    GLubyte RangeIndex,
    GLushort ItemCount,
    GLubyte *Items)
{
    GLushort i ;
    /* Get range encoding entry. */
    if (RangeIndex >= (sizeof(rangeEncodings)/sizeof(rangeEncodings[0])))
    {
        GL_ASSERT(GL_FALSE);
    }

    for (i = 0; i < ItemCount; i += 1)
    {
        GLubyte A = 0, B = 0, C = 0;
        GLubyte D = 0;
        GLubyte T;

        switch (RangeIndex)
        {
        case 0:
            /* 0..1 (1 bit) */
            switch (__glUtilGetBits(Items[i], 0, 0))
            {
            case 0: __glUtilSetBits(&Items[i], 0, 5,  0); break;
            case 1: __glUtilSetBits(&Items[i], 0, 5, 63); break;
            }
            break;

        case 1:
            /* 0..2 (1 trit no bits) */
            switch (__glUtilGetBits(Items[i], 0, 1))
            {
            case 0: __glUtilSetBits(&Items[i], 0, 5,  0); break;
            case 1: __glUtilSetBits(&Items[i], 0, 5, 32); break;
            case 2: __glUtilSetBits(&Items[i], 0, 5, 63); break;
            }
            break;

        case 2:
            /* 0..3 (2 bits) */
            {
                GLubyte bits = (GLubyte) __glUtilGetBits(Items[i], 0, 1);
                __glUtilSetBits(&Items[i], 4, 5, bits);
                __glUtilSetBits(&Items[i], 2, 3, bits);
                __glUtilSetBits(&Items[i], 0, 1, bits);
            }
            break;

        case 3:
            /* 0..4 (1 quint) */
            switch (__glUtilGetBits(Items[i], 0, 2))
            {
            case 0: __glUtilSetBits(&Items[i], 0, 5,  0); break;
            case 1: __glUtilSetBits(&Items[i], 0, 5, 16); break;
            case 2: __glUtilSetBits(&Items[i], 0, 5, 32); break;
            case 3: __glUtilSetBits(&Items[i], 0, 5, 47); break;
            case 4: __glUtilSetBits(&Items[i], 0, 5, 63); break;
            }
            break;

        case 5:
            /* 0..7 (3 bits) */
            {
                GLubyte bits = (GLubyte) __glUtilGetBits(Items[i], 0, 2);
                __glUtilSetBits(&Items[i], 3, 5, bits);
                __glUtilSetBits(&Items[i], 0, 2, bits);
            }
            break;

        case 8:
            /* 0..15 (4 bits) */
            {
                GLubyte bits = (GLubyte) __glUtilGetBits(Items[i], 0, 3);
                __glUtilSetBits(&Items[i], 2, 5, bits);
                __glUtilSetBits(&Items[i], 0, 1, (GLubyte)__glUtilGetBits(bits, 2, 3));
            }
            break;

        case 11:
            /* 0..31 (5 bits) */
            {
                GLubyte bits = (GLubyte) __glUtilGetBits(Items[i], 0, 4);
                __glUtilSetBits(&Items[i], 1, 5, bits);
                __glUtilSetBits(&Items[i], 0, 0, (GLubyte)__glUtilGetBits(bits, 4, 4));
            }
            break;

        default:
            A = (__glUtilGetBits(Items[i], 0, 0) == 0) ? 0x00 : 0x7F;

            switch (RangeIndex)
            {
            case 4:
                /* 0..5 (1 trit + 1 bit) */
                B = 0;
                C = 50;
                D = (GLubyte) __glUtilGetBits(Items[i], 1, 2);
                break;

            case 6:
                /* 0..9 (1 quint + 1 bit) */
                B = 0;
                C = 28;
                D = (GLubyte) __glUtilGetBits(Items[i], 1, 3);
                break;

            case 7:
                /* 0..11 (1 trit + 2 bits) */
                B = 0;
                __glUtilSetBits(&B, 0, 0, (GLubyte) __glUtilGetBits(Items[i], 1, 1));
                __glUtilSetBits(&B, 2, 2, (GLubyte) __glUtilGetBits(Items[i], 1, 1));
                __glUtilSetBits(&B, 6, 6, (GLubyte) __glUtilGetBits(Items[i], 1, 1));
                C = 23;
                D = (GLubyte) __glUtilGetBits(Items[i], 2, 3);
                break;

            case 9:
                /* 0..19 (1 quint + 2 bits) */
                B = 0;
                __glUtilSetBits(&B, 1, 1, (GLubyte) __glUtilGetBits(Items[i], 1, 1));
                __glUtilSetBits(&B, 6, 6, (GLubyte) __glUtilGetBits(Items[i], 1, 1));
                C = 13;
                D = (GLubyte) __glUtilGetBits(Items[i], 2, 4);
                break;

            case 10:
                /* 0..23 (1 trit + 3 bits) */
                B = 0;
                __glUtilSetBits(&B, 0, 1, (GLubyte) __glUtilGetBits(Items[i], 1, 2));
                __glUtilSetBits(&B, 5, 6, (GLubyte) __glUtilGetBits(Items[i], 1, 2));
                C = 11;
                D = (GLubyte) __glUtilGetBits(Items[i], 3, 4);
                break;
            }

            T = D * C + B;
            T = T ^ A;
            T = (A & 0x20) | (T >> 2);

            Items[i] = T;
        }

        if (Items[i] > 32)
            Items[i] += 1;
    }
}

static void
__glUtilComputeWeights(
    GLubyte BlockWidth,
    GLubyte BlockHeight,
    __GLblockMode *ASTCBlock,
    GLubyte *Weights,
    GLushort *Effective)
{
    /* Compute the scale factors (93..342). */
    GLushort Ds = weightScale[BlockWidth];
    GLushort Dt = weightScale[BlockHeight];
    GLint i, j, k;
    GLushort p00, p01, p10, p11;
    GLint numWeightsPerTexel = ASTCBlock->dualPlane ? 2 : 1;

    for (j = 0; j < BlockHeight; j++)
    {
        for (i = 0; i < BlockWidth; i++)
        {
            /* Compute the hommogeneous coordinates (0..1026). */
            GLushort cs = (GLushort)(Ds * i);
            GLushort ct = (GLushort)(Dt * j);

            /* Compute the weight-grid coordinate in 4.4 fixed point format (0..176). */
            GLushort gs = (GLushort)((cs * (ASTCBlock->widthOfGridWeights - 1) + 32) >> 6);
            GLushort gt = (GLushort)((ct * (ASTCBlock->heightOfGridWeights - 1) + 32) >> 6);

            /* Separate the integers (0..11). */
            GLushort js = (GLushort) __glUtilGetBits(gs, 4, 7);
            GLushort jt = (GLushort) __glUtilGetBits(gt, 4, 7);

            /* Separate the fractions (0..15). */
            GLushort fs = (GLushort) __glUtilGetBits(gs, 0, 3);
            GLushort ft = (GLushort) __glUtilGetBits(gt, 0, 3);

            /* Compute fractional weights (0..16). */
            GLushort w11 = (fs * ft + 8) >> 4;
            GLushort w10 = ft - w11;
            GLushort w01 = fs - w11;
            GLushort w00 = 16 - fs - ft + w11;

            /* Compute weight indices (0..156). */
            GLushort v00 = js + jt * ASTCBlock->widthOfGridWeights;
            GLushort v01 = v00 + 1;
            GLushort v10 = v00 + ASTCBlock->widthOfGridWeights;
            GLushort v11 = v00 + ASTCBlock->widthOfGridWeights + 1;

            for (k = 0; k < numWeightsPerTexel; k++)
            {
                p00 = Weights[v00 * numWeightsPerTexel + k];
                p01 = Weights[v01 * numWeightsPerTexel + k];
                p10 = Weights[v10 * numWeightsPerTexel + k];
                p11 = Weights[v11 * numWeightsPerTexel + k];

                /* Compute the effective weight (0..64). */
                Effective[(j * BlockWidth + i) * 2 + k] = (p00 * w00 + p01 * w01 + p10 * w10 + p11 * w11 + 8) >> 4;
            }
        }
    }
}

static void
__glUtilUnquantizeCEM(
    GLushort RangeIndex,
    GLushort ItemCount,
    GLubyte *Items)
{
    GLushort i;
    /* Get range encoding entry. */
    if (RangeIndex >= (sizeof(rangeEncodings)/sizeof(rangeEncodings[0])))
    {
        GL_ASSERT(GL_FALSE);
    }

    for (i = 0; i < ItemCount; i += 1)
    {
        GLushort A = 0, B = 0, C = 0;
        GLubyte D = 0;
        GLushort T;

        switch (RangeIndex)
        {
        case 5:
            /* 0..7 (3 bits) */
            {
                GLubyte bits = (GLubyte) __glUtilGetBits(Items[i], 0, 2);
                __glUtilSetBits(&Items[i], 5, 7, bits);
                __glUtilSetBits(&Items[i], 2, 4, bits);
                __glUtilSetBits(&Items[i], 0, 1, (GLubyte) __glUtilGetBits(bits, 1, 2));
            }
            break;

        case 8:
            /* 0..15 (4 bits) */
            {
                GLubyte bits = (GLubyte) __glUtilGetBits(Items[i], 0, 3);
                __glUtilSetBits(&Items[i], 4, 7, bits);
                __glUtilSetBits(&Items[i], 0, 3, bits);
            }
            break;

        case 11:
            /* 0..31 (5 bits) */
            {
                GLubyte bits = (GLubyte) __glUtilGetBits(Items[i], 0, 4);
                __glUtilSetBits(&Items[i], 3, 7, bits);
                __glUtilSetBits(&Items[i], 0, 2, (GLubyte) __glUtilGetBits(bits, 2, 4));
            }
            break;

        case 14:
            /* 0..63 (6 bits) */
            {
                GLubyte bits = (GLubyte) __glUtilGetBits(Items[i], 0, 5);
                __glUtilSetBits(&Items[i], 2, 7, bits);
                __glUtilSetBits(&Items[i], 0, 1, (GLubyte) __glUtilGetBits(bits, 4, 5));
            }
            break;

        case 17:
            /* 0..127 (7 bits) */
            {
                GLubyte bits = (GLubyte) __glUtilGetBits(Items[i], 0, 6);
                __glUtilSetBits(&Items[i], 1, 7, bits);
                __glUtilSetBits(&Items[i], 0, 0, (GLubyte) __glUtilGetBits(bits, 6, 6));
            }
            break;

        case 20:
            /* 0..255 (9 bits) */
            break;

        default:
            A = (__glUtilGetBits(Items[i], 0, 0) == 0) ? 0x000 : 0x1FF;

            switch (RangeIndex)
            {
            case 4:
                /* 0..5 (1 trit + 1 bit) */
                B = 0;
                C = 204;
                D = (GLubyte) __glUtilGetBits(Items[i], 1, 2);
                break;

            case 6:
                /* 0..9 (1 quint + 1 bit) */
                B = 0;
                C = 113;
                D = (GLubyte) __glUtilGetBits(Items[i], 1, 3);
                break;

            case 7:
                /* 0..11 (1 trit + 2 bits) */
                B = 0;
                __glUtilSetShortBits(&B, 1, 1, (GLushort) __glUtilGetBits(Items[i], 1, 1));
                __glUtilSetShortBits(&B, 2, 2, (GLushort) __glUtilGetBits(Items[i], 1, 1));
                __glUtilSetShortBits(&B, 4, 4, (GLushort) __glUtilGetBits(Items[i], 1, 1));
                __glUtilSetShortBits(&B, 8, 8, (GLushort) __glUtilGetBits(Items[i], 1, 1));
                C = 93;
                D = (GLubyte) __glUtilGetBits(Items[i], 2, 3);
                break;

            case 9:
                /* 0..19 (1 quint + 2 bits) */
                B = 0;
                __glUtilSetShortBits(&B, 2, 2, (GLushort) __glUtilGetBits(Items[i], 1, 1));
                __glUtilSetShortBits(&B, 3, 3, (GLushort) __glUtilGetBits(Items[i], 1, 1));
                __glUtilSetShortBits(&B, 8, 8, (GLushort) __glUtilGetBits(Items[i], 1, 1));
                C = 54;
                D = (GLubyte) __glUtilGetBits(Items[i], 2, 4);
                break;

            case 10:
                /* 0..23 (1 trit + 3 bits) */
                B = 0;
                __glUtilSetShortBits(&B, 0, 1, (GLushort) __glUtilGetBits(Items[i], 1, 2));
                __glUtilSetShortBits(&B, 2, 3, (GLushort) __glUtilGetBits(Items[i], 1, 2));
                __glUtilSetShortBits(&B, 7, 8, (GLushort) __glUtilGetBits(Items[i], 1, 2));
                C = 44;
                D = (GLubyte) __glUtilGetBits(Items[i], 3, 4);
                break;

            case 12:
                /* 0..39 (1 quint + 3 bits) */
                B = 0;
                __glUtilSetShortBits(&B, 0, 0, (GLushort) __glUtilGetBits(Items[i], 2, 2));
                __glUtilSetShortBits(&B, 1, 2, (GLushort) __glUtilGetBits(Items[i], 1, 2));
                __glUtilSetShortBits(&B, 7, 8, (GLushort) __glUtilGetBits(Items[i], 1, 2));
                C = 26;
                D = (GLubyte) __glUtilGetBits(Items[i], 3, 5);
                break;

            case 13:
                /* 0..47 (1 trit + 4 bits) */
                B = 0;
                __glUtilSetShortBits(&B, 0, 2, (GLushort) __glUtilGetBits(Items[i], 1, 3));
                __glUtilSetShortBits(&B, 6, 8, (GLushort) __glUtilGetBits(Items[i], 1, 3));
                C = 22;
                D = (GLubyte) __glUtilGetBits(Items[i], 4, 5);
                break;

            case 15:
                /* 0..79 (1 quint + 4 bits) */
                B = 0;
                __glUtilSetShortBits(&B, 0, 1, (GLushort) __glUtilGetBits(Items[i], 2, 3));
                __glUtilSetShortBits(&B, 6, 8, (GLushort) __glUtilGetBits(Items[i], 1, 3));
                C = 13;
                D = (GLubyte) __glUtilGetBits(Items[i], 4, 6);
                break;

            case 16:
                /* 0..95 (1 trit + 5 bits) */
                B = 0;
                __glUtilSetShortBits(&B, 0, 1, (GLushort) __glUtilGetBits(Items[i], 3, 4));
                __glUtilSetShortBits(&B, 5, 8, (GLushort) __glUtilGetBits(Items[i], 1, 4));
                C = 11;
                D = (GLubyte) __glUtilGetBits(Items[i], 5, 6);
                break;

            case 18:
                /* 0..159 (1 quint + 5 bits) */
                B = 0;
                __glUtilSetShortBits(&B, 0, 0, (GLushort) __glUtilGetBits(Items[i], 4, 4));
                __glUtilSetShortBits(&B, 5, 8, (GLushort) __glUtilGetBits(Items[i], 1, 4));
                C = 6;
                D = (GLubyte) __glUtilGetBits(Items[i], 5, 7);
                break;

            case 19:
                /* 0..191 (1 trit + 6 bits) */
                B = 0;
                __glUtilSetShortBits(&B, 0, 0, (GLushort) __glUtilGetBits(Items[i], 5, 5));
                __glUtilSetShortBits(&B, 4, 8, (GLushort) __glUtilGetBits(Items[i], 1, 5));
                C = 5;
                D = (GLubyte) __glUtilGetBits(Items[i], 6, 7);
                break;
            }

            T = D * C + B;
            T = T ^ A;
            T = (A & 0x80) | (T >> 2);

            Items[i] = (GLubyte) T;
        }
    }
}

__GL_INLINE void
__glUtilBitTransferSigned(
    GLubyte A,
    GLubyte *B,
    GLshort *Aout)
{
    *B >>= 1;
    *B  |= (A & 0x80);

    A >>= 1;
    A  &= 0x3F;

    *Aout = ((A & 0x20) == 0) ? A : (A - 0x40);
}

__GL_INLINE void
__glUtilClampUnorm8(
    GLshort A,
    GLubyte *Aout)
{
    if (A < 0)
        *Aout = 0;
    else if (A > 255)
        *Aout = 255;
    else
        *Aout = (GLubyte) A;
}

static GLubyte
__glUtilDecodeCEM(
    GLubyte PartCount,
    GLubyte *CEM,
    GLubyte *CEP,
    __GLcolors8888 *E0,
    __GLcolors8888 *E1)
{
    GLubyte success = 1;
    __GLcolors8888 e0 = { 0 };
    __GLcolors8888 e1 = { 0 };
    GLubyte CEPoffset = 0;
    GLubyte v0, v1, v2, v3;
    GLubyte v4, v5, v6, v7;
    GLshort v1signed, v3signed, v5signed, v7signed;
    GLushort L0;
    GLushort L1;
    GLushort s0, s1;
    GLubyte i;

    for (i = 0; i <= PartCount; i += 1)
    {
        switch (CEM[i])
        {
        case 0:
            /* LDR Luminance, direct. */
            v0 = CEP[CEPoffset + 0];
            v1 = CEP[CEPoffset + 1];
            CEPoffset += 2;

            e0.r = v0;
            e0.g = v0;
            e0.b = v0;
            e0.a = 0xFF;

            e1.r = v1;
            e1.g = v1;
            e1.b = v1;
            e1.a = 0xFF;
            break;

        case 1:
            /* LDR Luminance, base + offset. */
            v0 = CEP[CEPoffset + 0];
            v1 = CEP[CEPoffset + 1];
            CEPoffset += 2;

            L0 = (v0 >> 2) | (v1 & 0xC0);
            L1 = L0 + (v1 & 0x3F);

            if (L1 > 0xFF)
                L1 = 0xFF;

            e0.r = L0;
            e0.g = L0;
            e0.b = L0;
            e0.a = 0xFF;

            e1.r = (GLubyte) L1;
            e1.g = (GLubyte) L1;
            e1.b = (GLubyte) L1;
            e1.a = 0xFF;
            break;

        case 2:
            /* HDR Luminance, large range. */
            CEPoffset += 2;
            success = 0;
            break;

        case 3:
            /* HDR Luminance, small range. */
            CEPoffset += 2;
            success = 0;
            break;

        case 4:
            /* LDR Luminance + Alpha, direct. */
            v0 = CEP[CEPoffset + 0];
            v1 = CEP[CEPoffset + 1];
            v2 = CEP[CEPoffset + 2];
            v3 = CEP[CEPoffset + 3];
            CEPoffset += 4;

            e0.r = v0;
            e0.g = v0;
            e0.b = v0;
            e0.a = v2;

            e1.r = v1;
            e1.g = v1;
            e1.b = v1;
            e1.a = v3;
            break;

        case 5:
            /* LDR Luminance+Alpha, direct. */
            v0 = CEP[CEPoffset + 0];
            v1 = CEP[CEPoffset + 1];
            v2 = CEP[CEPoffset + 2];
            v3 = CEP[CEPoffset + 3];
            CEPoffset += 4;

            __glUtilBitTransferSigned(v1, &v0, &v1signed);
            __glUtilBitTransferSigned(v3, &v2, &v3signed);

            v1signed += v0;
            v3signed += v2;

            __glUtilClampUnorm8(v1signed, &v1);
            __glUtilClampUnorm8(v3signed, &v3);

            e0.r = v0;
            e0.g = v0;
            e0.b = v0;
            e0.a = v2;

            e1.r = v1;
            e1.g = v1;
            e1.b = v1;
            e1.a = v3;
            break;

        case 6:
            /* LDR RGB, base+scale. */
            v0 = CEP[CEPoffset + 0];
            v1 = CEP[CEPoffset + 1];
            v2 = CEP[CEPoffset + 2];
            v3 = CEP[CEPoffset + 3];
            CEPoffset += 4;

            e0.r = (GLushort) __glUtilGetBits(v0 * v3, 8, 15);
            e0.g = (GLushort) __glUtilGetBits(v1 * v3, 8, 15);
            e0.b = (GLushort) __glUtilGetBits(v2 * v3, 8, 15);
            e0.a = 0xFF;

            e1.r = v0;
            e1.g = v1;
            e1.b = v2;
            e1.a = 0xFF;
            break;

        case 7:
            /* HDR RGB, base+scale. */
            CEPoffset += 4;
            success = 0;
            break;

        case 8:
            /* LDR RGB, direct. */
            v0 = CEP[CEPoffset + 0];
            v1 = CEP[CEPoffset + 1];
            v2 = CEP[CEPoffset + 2];
            v3 = CEP[CEPoffset + 3];
            v4 = CEP[CEPoffset + 4];
            v5 = CEP[CEPoffset + 5];
            CEPoffset += 6;

            s0 = v0 + v2 + v4;
            s1 = v1 + v3 + v5;

            if (s1 >= s0)
            {
                e0.r = v0;
                e0.g = v2;
                e0.b = v4;
                e0.a = 0xFF;

                e1.r = v1;
                e1.g = v3;
                e1.b = v5;
                e1.a = 0xFF;
            }
            else
            {
                v1 = (GLubyte) __glUtilGetBits((GLushort) v1 + v5, 1, 8);
                v3 = (GLubyte) __glUtilGetBits((GLushort) v3 + v5, 1, 8);

                v0 = (GLubyte) __glUtilGetBits((GLushort) v0 + v4, 1, 8);
                v2 = (GLubyte) __glUtilGetBits((GLushort) v2 + v4, 1, 8);

                e0.r = v1;
                e0.g = v3;
                e0.b = v5;
                e0.a = 0xFF;

                e1.r = v0;
                e1.g = v2;
                e1.b = v4;
                e1.a = 0xFF;
            }
            break;

        case 9:
            /* LDR RGB, base + offset. */
            v0 = CEP[CEPoffset + 0];
            v1 = CEP[CEPoffset + 1];
            v2 = CEP[CEPoffset + 2];
            v3 = CEP[CEPoffset + 3];
            v4 = CEP[CEPoffset + 4];
            v5 = CEP[CEPoffset + 5];
            CEPoffset += 6;

            __glUtilBitTransferSigned(v1, &v0, &v1signed);
            __glUtilBitTransferSigned(v3, &v2, &v3signed);
            __glUtilBitTransferSigned(v5, &v4, &v5signed);

            if (v1signed + v3signed + v5signed >= 0)
            {
                v1signed += v0;
                v3signed += v2;
                v5signed += v4;

                __glUtilClampUnorm8(v1signed, &v1);
                __glUtilClampUnorm8(v3signed, &v3);
                __glUtilClampUnorm8(v5signed, &v5);

                e0.r = v0;
                e0.g = v2;
                e0.b = v4;
                e0.a = 0xFF;

                e1.r = v1;
                e1.g = v3;
                e1.b = v5;
                e1.a = 0xFF;
            }
            else
            {
                v1signed += v0;
                v3signed += v2;
                v5signed += v4;

                v1signed = (v1signed + v5signed) >> 1;
                v3signed = (v3signed + v5signed) >> 1;

                __glUtilClampUnorm8(v1signed, &v1);
                __glUtilClampUnorm8(v3signed, &v3);
                __glUtilClampUnorm8(v5signed, &v5);

                v0 = (GLubyte) __glUtilGetBits((GLushort) v0 + v4, 1, 8);
                v2 = (GLubyte) __glUtilGetBits((GLushort) v2 + v4, 1, 8);

                e0.r = v1;
                e0.g = v3;
                e0.b = v5;
                e0.a = 0xFF;

                e1.r = v0;
                e1.g = v2;
                e1.b = v4;
                e1.a = 0xFF;
            }
            break;

        case 10:
            /* LDR RGB, base+scale plus two A. */
            v0 = CEP[CEPoffset + 0];
            v1 = CEP[CEPoffset + 1];
            v2 = CEP[CEPoffset + 2];
            v3 = CEP[CEPoffset + 3];
            v4 = CEP[CEPoffset + 4];
            v5 = CEP[CEPoffset + 5];
            CEPoffset += 6;

            e0.r = (GLushort) __glUtilGetBits(v0 * v3, 8, 15);
            e0.g = (GLushort) __glUtilGetBits(v1 * v3, 8, 15);
            e0.b = (GLushort) __glUtilGetBits(v2 * v3, 8, 15);
            e0.a = v4;

            e1.r = v0;
            e1.g = v1;
            e1.b = v2;
            e1.a = v5;
            break;

        case 11:
            /* HDR RGB. */
            CEPoffset += 6;
            success = 0;
            break;

        case 12:
            /* LDR RGBA, direct. */
            v0 = CEP[CEPoffset + 0];
            v1 = CEP[CEPoffset + 1];
            v2 = CEP[CEPoffset + 2];
            v3 = CEP[CEPoffset + 3];
            v4 = CEP[CEPoffset + 4];
            v5 = CEP[CEPoffset + 5];
            v6 = CEP[CEPoffset + 6];
            v7 = CEP[CEPoffset + 7];
            CEPoffset += 8;

            s0 = v0 + v2 + v4;
            s1 = v1 + v3 + v5;

            if (s1 >= s0)
            {
                e0.r = v0;
                e0.g = v2;
                e0.b = v4;
                e0.a = v6;

                e1.r = v1;
                e1.g = v3;
                e1.b = v5;
                e1.a = v7;
            }
            else
            {
                v1 = (GLubyte) __glUtilGetBits((GLushort) v1 + v5, 1, 8);
                v3 = (GLubyte) __glUtilGetBits((GLushort) v3 + v5, 1, 8);

                v0 = (GLubyte) __glUtilGetBits((GLushort) v0 + v4, 1, 8);
                v2 = (GLubyte) __glUtilGetBits((GLushort) v2 + v4, 1, 8);

                e0.r = v1;
                e0.g = v3;
                e0.b = v5;
                e0.a = v7;

                e1.r = v0;
                e1.g = v2;
                e1.b = v4;
                e1.a = v6;
            }
            break;

        case 13:
            /* LDR RGBA, base+offset. */
            v0 = CEP[CEPoffset + 0];
            v1 = CEP[CEPoffset + 1];
            v2 = CEP[CEPoffset + 2];
            v3 = CEP[CEPoffset + 3];
            v4 = CEP[CEPoffset + 4];
            v5 = CEP[CEPoffset + 5];
            v6 = CEP[CEPoffset + 6];
            v7 = CEP[CEPoffset + 7];
            CEPoffset += 8;

            __glUtilBitTransferSigned(v1, &v0, &v1signed);
            __glUtilBitTransferSigned(v3, &v2, &v3signed);
            __glUtilBitTransferSigned(v5, &v4, &v5signed);
            __glUtilBitTransferSigned(v7, &v6, &v7signed);

            if (v1signed + v3signed + v5signed >= 0)
            {
                v1signed += v0;
                v3signed += v2;
                v5signed += v4;
                v7signed += v6;

                __glUtilClampUnorm8(v1signed, &v1);
                __glUtilClampUnorm8(v3signed, &v3);
                __glUtilClampUnorm8(v5signed, &v5);
                __glUtilClampUnorm8(v7signed, &v7);

                e0.r = v0;
                e0.g = v2;
                e0.b = v4;
                e0.a = v6;

                e1.r = v1;
                e1.g = v3;
                e1.b = v5;
                e1.a = v7;
            }
            else
            {
                v1signed += v0;
                v3signed += v2;
                v5signed += v4;
                v7signed += v6;

                v1signed = (v1signed + v5signed) >> 1;
                v3signed = (v3signed + v5signed) >> 1;

                __glUtilClampUnorm8(v1signed, &v1);
                __glUtilClampUnorm8(v3signed, &v3);
                __glUtilClampUnorm8(v5signed, &v5);
                __glUtilClampUnorm8(v7signed, &v7);

                v0 = (GLubyte) __glUtilGetBits(v0 + v4, 1, 8);
                v2 = (GLubyte) __glUtilGetBits(v2 + v4, 1, 8);

                e0.r = v1;
                e0.g = v3;
                e0.b = v5;
                e0.a = v7;

                e1.r = v0;
                e1.g = v2;
                e1.b = v4;
                e1.a = v6;
            }
            break;

        case 14:
            /* HDR RGB + LDR Alpha. */
            CEPoffset += 8;
            success = 0;
            break;

        case 15:
            /* HDR RGB + HDR Alpha. */
            CEPoffset += 8;
            success = 0;
            break;

        default:
            GL_ASSERT(GL_FALSE);
        }

        if (success==0) {
            e0.r = 0xFFFF;
            e0.g = 0;
            e0.b = 0xFFFF;
            e0.a = 0xFFFF;

            e1.r = 0xFFFF;
            e1.g = 0;
            e1.b = 0xFFFF;
            e1.a = 0xFFFF;
            success = 1;
        }

        /* Set result. */
        E0[i] = e0;
        E1[i] = e1;
    }

    return success;
}

__GL_INLINE GLuint
__glUtilExpandSeed(
    GLushort PartSeed)
{
    GLuint seed = PartSeed;

    seed ^= (seed >> 15);
    seed -= (seed << 17);
    seed += (seed <<  7);
    seed += (seed <<  4);
    seed ^= (seed >>  5);
    seed += (seed << 16);
    seed ^= (seed >>  7);
    seed ^= (seed >>  3);
    seed ^= (seed <<  6);
    seed ^= (seed >> 17);

    return seed;
}

static void
__glUtilGetPartIndex(
    GLushort PartSeed,
    GLint PartCount,
    GLubyte X,
    GLubyte Y,
    GLushort smallBlock,
    GLushort *PartIndex)
{
    GLubyte seed1, seed2, seed3, seed4, seed5, seed6, seed7, seed8;
    GLuint rnum ;
    GLubyte a, b, c, d;
    GLubyte sh1, sh2;

    if (smallBlock)
    {
        X <<= 1;
        Y <<= 1;
    }

    PartSeed += (GLushort)(PartCount * 1024);

    rnum = (GLuint)__glUtilExpandSeed(PartSeed);

    seed1 = (GLubyte) __glUtilGetBits(rnum,  0,  3);
    seed2 = (GLubyte) __glUtilGetBits(rnum,  4,  7);
    seed3 = (GLubyte) __glUtilGetBits(rnum,  8, 11);
    seed4 = (GLubyte) __glUtilGetBits(rnum, 12, 15);
    seed5 = (GLubyte) __glUtilGetBits(rnum, 16, 19);
    seed6 = (GLubyte) __glUtilGetBits(rnum, 20, 23);
    seed7 = (GLubyte) __glUtilGetBits(rnum, 24, 27);
    seed8 = (GLubyte) __glUtilGetBits(rnum, 28, 31);

    /* Squaring all the seeds in order to bias their distribution towards lower values. */
    seed1 *= seed1;
    seed2 *= seed2;
    seed3 *= seed3;
    seed4 *= seed4;
    seed5 *= seed5;
    seed6 *= seed6;
    seed7 *= seed7;
    seed8 *= seed8;

    if (__glUtilGetBits(PartSeed, 0, 0) == 1)
    {
        sh1 = (__glUtilGetBits(PartSeed, 1, 1) == 1) ? 4 : 5;
        sh2 = (PartCount == 2) ? 6 : 5;
    }
    else
    {
        sh1 = (PartCount == 2) ? 6 : 5;
        sh2 = (__glUtilGetBits(PartSeed, 1, 1) == 1) ? 4 : 5;
    }

    seed1 >>= sh1;
    seed2 >>= sh2;
    seed3 >>= sh1;
    seed4 >>= sh2;
    seed5 >>= sh1;
    seed6 >>= sh2;
    seed7 >>= sh1;
    seed8 >>= sh2;

    a = (GLubyte) __glUtilGetBits(seed1 * X + seed2 * Y + __glUtilGetBits(rnum, 14, 31), 0, 5);
    b = (GLubyte) __glUtilGetBits(seed3 * X + seed4 * Y + __glUtilGetBits(rnum, 10, 31), 0, 5);
    c = (GLubyte) __glUtilGetBits(seed5 * X + seed6 * Y + __glUtilGetBits(rnum,  6, 31), 0, 5);
    d = (GLubyte) __glUtilGetBits(seed7 * X + seed8 * Y + __glUtilGetBits(rnum,  2, 31), 0, 5);

    /* Remove some of the components of we are to output < 4 partitions. */
    if (PartCount <= 2)
        d = 0;

    if (PartCount <= 1)
        c = 0;

    if (PartCount == 0)
        b = 0;

    /* Determine partition index. */
    if ((a >= b) && (a >= c) && (a >= d))
    {
        *PartIndex = 0;
    }
    else if ((b >= c) && (b >= d))
    {
        *PartIndex = 1;
    }
    else if (c >= d)
    {
        *PartIndex = 2;
    }
    else
    {
        *PartIndex = 3;
    }
}

static void
__glUtilSetASTCErrorColor(
    GLubyte *output,
    GLubyte blockWidth,
    GLubyte blockHeight)
{
    GLint i;
    for (i = 0; i < blockWidth * blockHeight; i++)
    {
        output[4 * i + 0] = 0xFF;
        output[4 * i + 1] = 0;
        output[4 * i + 2] = 0xFF;
        output[4 * i + 3] = 0xFF;
    }
}

/* sRGB texture, convert to Linear space. */
static const GLubyte sRGB2linear[] =
{
    0   ,0   ,0   ,0   ,0   ,0   ,0   ,1   ,1   ,1   ,1   ,1   ,1   ,1   ,1   ,1   ,
    1   ,1   ,2   ,2   ,2   ,2   ,2   ,2   ,2   ,2   ,3   ,3   ,3   ,3   ,3   ,3   ,
    4   ,4   ,4   ,4   ,4   ,5   ,5   ,5   ,5   ,6   ,6   ,6   ,6   ,7   ,7   ,7   ,
    8   ,8   ,8   ,8   ,9   ,9   ,9   ,10  ,10  ,10  ,11  ,11  ,12  ,12  ,12  ,13  ,
    13  ,13  ,14  ,14  ,15  ,15  ,16  ,16  ,17  ,17  ,17  ,18  ,18  ,19  ,19  ,20  ,
    20  ,21  ,22  ,22  ,23  ,23  ,24  ,24  ,25  ,25  ,26  ,27  ,27  ,28  ,29  ,29  ,
    30  ,30  ,31  ,32  ,32  ,33  ,34  ,35  ,35  ,36  ,37  ,37  ,38  ,39  ,40  ,41  ,
    41  ,42  ,43  ,44  ,45  ,45  ,46  ,47  ,48  ,49  ,50  ,51  ,51  ,52  ,53  ,54  ,
    55  ,56  ,57  ,58  ,59  ,60  ,61  ,62  ,63  ,64  ,65  ,66  ,67  ,68  ,69  ,70  ,
    71  ,72  ,73  ,74  ,76  ,77  ,78  ,79  ,80  ,81  ,82  ,84  ,85  ,86  ,87  ,88  ,
    90  ,91  ,92  ,93  ,95  ,96  ,97  ,99  ,100 ,101 ,103 ,104 ,105 ,107 ,108 ,109 ,
    111 ,112 ,114 ,115 ,116 ,118 ,119 ,121 ,122 ,124 ,125 ,127 ,128 ,130 ,131 ,133 ,
    134 ,136 ,138 ,139 ,141 ,142 ,144 ,146 ,147 ,149 ,151 ,152 ,154 ,156 ,157 ,159 ,
    161 ,163 ,164 ,166 ,168 ,170 ,171 ,173 ,175 ,177 ,179 ,181 ,183 ,184 ,186 ,188 ,
    190 ,192 ,194 ,196 ,198 ,200 ,202 ,204 ,206 ,208 ,210 ,212 ,214 ,216 ,218 ,220 ,
    222 ,224 ,226 ,229 ,231 ,233 ,235 ,237 ,239 ,242 ,244 ,246 ,248 ,250 ,253 ,255
};

static void
__glUtilDecodeVoidExtent(
    GLubyte *output,
    GLushort blockMode,
    const GLubyte *blockData,
    GLubyte blockWidth,
    GLubyte blockHeight,
    GLboolean sRGB)
{
    GLubyte constColor;
    GLushort s0, s1, t0, t1;
    GLint j;

    /* Make sure HDR is disabled. */
    if (blockMode & 0x200)
    {
        __glUtilSetASTCErrorColor(output, blockWidth, blockHeight);
        return;
    }

    /* Make sure that reserved bits are set correctly. */
    if (__glUtilReadBlock(blockData, 10, 2, GL_FALSE) != 3)
    {
        __glUtilSetASTCErrorColor(output, blockWidth, blockHeight);
        return;
    }

    /* Get void-extent coordinates. */
    s0 = (GLushort) __glUtilReadBlock(blockData, 12, 13, GL_FALSE);
    s1 = (GLushort) __glUtilReadBlock(blockData, 25, 13, GL_FALSE);
    t0 = (GLushort) __glUtilReadBlock(blockData, 38, 13, GL_FALSE);
    t1 = (GLushort) __glUtilReadBlock(blockData, 51, 13, GL_FALSE);

    /* not support hdr astc */
    if (__glUtilGetBits(blockMode, 9, 9))
    {
        __glUtilSetASTCErrorColor(output, blockWidth, blockHeight);
        return;
    }
    /* Determine whether this is a constant color block. */
    constColor = (s0 == 0x1FFF) && (s1 == 0x1FFF) &&
        (t0 == 0x1FFF) && (t1 == 0x1FFF);

    /* Validate the block. */
    if (((s0 >= s1) || (t0 >= t1)) && !constColor)
    {
        __glUtilSetASTCErrorColor(output, blockWidth, blockHeight);
        return;
    }

    for (j = 0; j < blockWidth*blockHeight; j++)
    {
        if (sRGB)
        {

            output[j*4 + 0] = sRGB2linear[(GLubyte) __glUtilReadBlock(blockData,  72, 8, GL_FALSE)];
            output[j*4 + 1] = sRGB2linear[(GLubyte) __glUtilReadBlock(blockData,  88, 8, GL_FALSE)];
            output[j*4 + 2] = sRGB2linear[(GLubyte) __glUtilReadBlock(blockData,  104, 8, GL_FALSE)];
            output[j*4 + 3] = (GLubyte) __glUtilReadBlock(blockData,  120, 8, GL_FALSE);
        }
        else
        {
            output[j*4 + 0] = (GLubyte) __glUtilReadBlock(blockData,  72, 8, GL_FALSE);
            output[j*4 + 1] = (GLubyte) __glUtilReadBlock(blockData,  88, 8, GL_FALSE);
            output[j*4 + 2] = (GLubyte) __glUtilReadBlock(blockData,  104, 8, GL_FALSE);
            output[j*4 + 3] = (GLubyte) __glUtilReadBlock(blockData,  120, 8, GL_FALSE);
        }
    }
}

static void
__glUtilComputeColor(
    GLubyte sRGB,
    __GLcolors8888 Color0,
    __GLcolors8888 Color1,
    GLushort *Effective,
    GLushort Plane2comp,
    GLubyte *output)
{
    GLushort w1r = Effective[0];
    GLushort w1g = Effective[0];
    GLushort w1b = Effective[0];
    GLushort w1a = Effective[0];
    GLushort w0r, w0g,w0b, w0a;
    GLushort color0r, color0g, color0b, color0a;
    GLushort color1r, color1g, color1b, color1a;
    GLushort r, g, b, a;

    color0r = color0g = color0b = color0a = 0;
    color1r = color1g = color1b = color1a = 0;

    switch (Plane2comp)
    {
    case 0:
        w1r = Effective[1];
        break;

    case 1:
        w1g = Effective[1];
        break;

    case 2:
        w1b = Effective[1];
        break;

    case 3:
        w1a = Effective[1];
        break;
    }

    w0r = 64 - w1r;
    w0g = 64 - w1g;
    w0b = 64 - w1b;
    w0a = 64 - w1a;


    if (sRGB)
    {
        __glUtilSetShortBits(&color0r, 0, 7, 0x80);
        __glUtilSetShortBits(&color0g, 0, 7, 0x80);
        __glUtilSetShortBits(&color0b, 0, 7, 0x80);
        __glUtilSetShortBits(&color0a, 0, 7, 0x80);

        __glUtilSetShortBits(&color1r, 0, 7, 0x80);
        __glUtilSetShortBits(&color1g, 0, 7, 0x80);
        __glUtilSetShortBits(&color1b, 0, 7, 0x80);
        __glUtilSetShortBits(&color1a, 0, 7, 0x80);

        __glUtilSetShortBits(&color0r, 8, 15, Color0.r);
        __glUtilSetShortBits(&color0g, 8, 15, Color0.g);
        __glUtilSetShortBits(&color0b, 8, 15, Color0.b);
        __glUtilSetShortBits(&color0a, 8, 15, Color0.a);

        __glUtilSetShortBits(&color1r, 8, 15, Color1.r);
        __glUtilSetShortBits(&color1g, 8, 15, Color1.g);
        __glUtilSetShortBits(&color1b, 8, 15, Color1.b);
        __glUtilSetShortBits(&color1a, 8, 15, Color1.a);
    }
    else
    {
        __glUtilSetShortBits(&color0r, 0, 7, Color0.r);
        __glUtilSetShortBits(&color0g, 0, 7, Color0.g);
        __glUtilSetShortBits(&color0b, 0, 7, Color0.b);
        __glUtilSetShortBits(&color0a, 0, 7, Color0.a);

        __glUtilSetShortBits(&color1r, 0, 7, Color1.r);
        __glUtilSetShortBits(&color1g, 0, 7, Color1.g);
        __glUtilSetShortBits(&color1b, 0, 7, Color1.b);
        __glUtilSetShortBits(&color1a, 0, 7, Color1.a);

        __glUtilSetShortBits(&color0r, 8, 15, Color0.r);
        __glUtilSetShortBits(&color0g, 8, 15, Color0.g);
        __glUtilSetShortBits(&color0b, 8, 15, Color0.b);
        __glUtilSetShortBits(&color0a, 8, 15, Color0.a);

        __glUtilSetShortBits(&color1r, 8, 15, Color1.r);
        __glUtilSetShortBits(&color1g, 8, 15, Color1.g);
        __glUtilSetShortBits(&color1b, 8, 15, Color1.b);
        __glUtilSetShortBits(&color1a, 8, 15, Color1.a);
    }

    r = ((color0r * w0r) + (color1r * w1r) + 32) >> 6;
    g = ((color0g * w0g) + (color1g * w1g) + 32) >> 6;
    b = ((color0b * w0b) + (color1b * w1b) + 32) >> 6;
    a = ((color0a * w0a) + (color1a * w1a) + 32) >> 6;

    if (sRGB)
    {

        output[0] = sRGB2linear[(GLubyte)((r & 0xff00) >> 8)];
        output[1] = sRGB2linear[(GLubyte)((g & 0xff00) >> 8)];
        output[2] = sRGB2linear[(GLubyte)((b & 0xff00) >> 8)];
        output[3] = (GLubyte)((a & 0xff00) >> 8);
    }
    else
    {
    output[0] = (GLubyte)((r & 0xff00) >> 8);
    output[1] = (GLubyte)((g & 0xff00) >> 8);
    output[2] = (GLubyte)((b & 0xff00) >> 8);
    output[3] = (GLubyte)((a & 0xff00) >> 8);
    }
}

static void
__glUtilDecodeBlock(
    GLubyte *output,
    GLubyte blockWidth,
    GLubyte blockHeight,
    GLushort blockMode,
    const GLubyte *blockData,
    GLboolean sRGB)
{
    __GLblockMode astcBlock = {0, 0, 0, 0, 0, 0, 0};
    GLushort effective[MAX_TEXEL_COUNT * 2];
    GLubyte fixedConfig;
    GLubyte baseClass = 0;
    GLushort encodedCEM = 0;
    GLubyte partCount;
    GLubyte lowerBits;
    GLubyte CEM[4] = {0};
    GLbyte remainingBits;
    GLushort CEMindex;
    GLushort CEMpairs = 0;
    GLushort partSeed;
    GLushort partIndex;
    GLushort plane2comp;
    GLubyte i, j;

    /* Decode the color end points. */
    __GLcolors8888 e0[4], e1[4];

    /* Determine small block state. */
    GLushort smallBlock = ((blockWidth * blockHeight) < 32);

    /* Up to 66 weights are supported. */
    GLubyte weights[66];

    /* Up to 18 color end point interegs are supported. */
    GLubyte CEP[20];
    memset(CEP, 0, sizeof(CEP));
    gcoOS_ZeroMemory(&weights, gcmSIZEOF(weights));

    /* Decode the block mode. */
    if (!__glUtilDecodeBlockMode(blockMode, &astcBlock))
    {
        __glUtilSetASTCErrorColor(output, blockWidth, blockHeight);
        return;
    }

    /* Verify the grid size. */
    if ((astcBlock.widthOfGridWeights > blockWidth) || (astcBlock.heightOfGridWeights > blockHeight))
    {
        __glUtilSetASTCErrorColor(output, blockWidth, blockHeight);
        return;
    }

    /* Decode the weigth ISE. */
    __glUtilDecodeISE(blockData, 0, GL_TRUE, astcBlock.weightRangIndex, astcBlock.numberOfWeights, weights);

    /* Unquantize the weights. */
    __glUtilUnquantizeWeights(astcBlock.weightRangIndex, astcBlock.numberOfWeights, weights);

    /* Compute the effective weights. */
    __glUtilComputeWeights(blockWidth, blockHeight, &astcBlock, weights, &effective[0]);

    /* Get the partition count. */
    partCount = (GLubyte) __glUtilReadBlock(blockData, 11, 2, GL_FALSE);
    if (astcBlock.dualPlane && (partCount == 3))
    {
        __glUtilSetASTCErrorColor(output, blockWidth, blockHeight);
        return;
    }

    /* Determine the base class and the color endpoint mode (CEM). */
    if (partCount == 0)
    {
        /* Get CEM. */
        __glUtilSetShortBits(&encodedCEM, 0, 3, (GLushort) __glUtilReadBlock(blockData, 13, 4, GL_FALSE));

        /* Fixed configuration bits. */
        fixedConfig = 11 + 2 + 4;
    }
    else
    {
        /* Get base class. */
        __glUtilSetBits(&baseClass,  0, 1, (GLubyte) __glUtilReadBlock(blockData, 23, 2, GL_FALSE));

        /* Get CEM. */
        __glUtilSetShortBits(&encodedCEM, 0, 3, (GLushort) __glUtilReadBlock(blockData, 25, 4, GL_FALSE));

        /* Fixed configuration bits. */
        fixedConfig = 11 + 2 + 10 + 6;
    }

    /* Compute the number of bits before the weight array.
    * The number of bits in the weight array is in range 24..96.
    * Lower bits is in range 32..104. */
    lowerBits = 128 - astcBlock.weightBits;

    /* Decode CEM. */
    if (baseClass == 0)
    {
        switch (partCount)
        {
        case 3:
            /* Four partitions. */
            CEM[3] = (GLubyte) encodedCEM;

        case 2:
            /* Three partitions. */
            CEM[2] = (GLubyte) encodedCEM;

        case 1:
            /* Two partitions. */
            CEM[1] = (GLubyte) encodedCEM;

        case 0:
            /* One partition. */
            CEM[0] = (GLubyte) encodedCEM;
        }
    }
    else
    {
        GLubyte M0 = 0, M1 = 0, M2 = 0, M3 = 0;

        /* Get color modes. */
        switch (partCount)
        {
        case 1:
            /* Adjust for the extra CEM bits. */
            lowerBits -= 2;

            /* Get the rest of the bits. */
            __glUtilSetShortBits(&encodedCEM, 4, 5, (GLushort) __glUtilReadBlock(blockData, lowerBits, 2, GL_FALSE));

            /* Extract the color modes. */
            M0 = (GLubyte) __glUtilGetBits(encodedCEM, 2, 3);
            M1 = (GLubyte) __glUtilGetBits(encodedCEM, 4, 5);
            break;

        case 2:
            /* Adjust for the extra CEM bits. */
            lowerBits -= 5;

            /* Get the rest of the bits. */
            __glUtilSetShortBits(&encodedCEM, 4, 8, (GLushort) __glUtilReadBlock(blockData, lowerBits, 5, GL_FALSE));

            /* Extract the color modes. */
            M0 = (GLubyte) __glUtilGetBits(encodedCEM, 3, 4);
            M1 = (GLubyte) __glUtilGetBits(encodedCEM, 5, 6);
            M2 = (GLubyte) __glUtilGetBits(encodedCEM, 7, 8);
            break;

        case 3:
            /* Adjust for the extra CEM bits. */
            lowerBits -= 8;

            /* Get the rest of the bits.*/
            __glUtilSetShortBits(&encodedCEM, 4, 11, (GLushort) __glUtilReadBlock(blockData, lowerBits, 8, GL_FALSE));

            /* Extract the color modes. */
            M0 = (GLubyte) __glUtilGetBits(encodedCEM,  4,  5);
            M1 = (GLubyte) __glUtilGetBits(encodedCEM,  6,  7);
            M2 = (GLubyte) __glUtilGetBits(encodedCEM,  8,  9);
            M3 = (GLubyte) __glUtilGetBits(encodedCEM, 10, 11);
            break;
        }

        /* Rescale base class from 1..3 range to 0..2. */
        baseClass -= 1;

        /* Decode CEM. */
        switch (partCount)
        {
        case 3:
            /* Four partitions. */
            CEM[3] = 0;
            __glUtilSetBits(&CEM[3], 0, 1, M3);
            __glUtilSetBits(&CEM[3], 2, 3, baseClass + (GLubyte) __glUtilGetBits(encodedCEM, 3, 3));

        case 2:
            /* Three partitions. */
            CEM[2] = 0;
            __glUtilSetBits(&CEM[2], 0, 1, M2);
            __glUtilSetBits(&CEM[2], 2, 3, baseClass + (GLubyte) __glUtilGetBits(encodedCEM, 2, 2));

        case 1:
            /* Two partitions. */
            CEM[1] = 0;
            __glUtilSetBits(&CEM[1], 0, 1, M1);
            __glUtilSetBits(&CEM[1], 2, 3, baseClass + (GLubyte) __glUtilGetBits(encodedCEM, 1, 1));

            CEM[0] = 0;
            __glUtilSetBits(&CEM[0], 0, 1, M0);
            __glUtilSetBits(&CEM[0], 2, 3, baseClass + (GLubyte) __glUtilGetBits(encodedCEM, 0, 0));
        }
    }

    /* Determine the number of remaining bits available for color information. */
    remainingBits = lowerBits - fixedConfig;

    /* Determine final remaining bits available for color information
    * (account for the two dual plane config bits).
    * Number of remining bits is in range 0..87. */
    if (astcBlock.dualPlane)
        remainingBits -= 2;

    if (remainingBits < 0)
        remainingBits = 0;

    /* Verify the range. */
    if (remainingBits > 87)
    {
        GL_ASSERT(GL_FALSE);
    }

    /* Ranges 0..5 and smaller are not supported. */
    if (remainingBits < 6)
    {
        __glUtilSetASTCErrorColor(output, blockWidth, blockHeight);
        return;
    }
    remainingBits -= 6;

    /* Determine the number of CEM pairs. */
    switch (partCount)
    {
    case 3:
        /* Four partitions. */
        CEMpairs += (GLushort) __glUtilGetBits(CEM[3], 2, 3) + 1;

    case 2:
        /* Three partitions. */
        CEMpairs += (GLushort) __glUtilGetBits(CEM[2], 2, 3) + 1;

    case 1:
        /* Two partitions. */
        CEMpairs += (GLushort) __glUtilGetBits(CEM[1], 2, 3) + 1;

    case 0:
        /* One partition. */
        CEMpairs += (GLushort) __glUtilGetBits(CEM[0], 2, 3) + 1;
    }

    /* Can only have up to 9 pairs. */
    if (CEMpairs > 9)
    {
        __glUtilSetASTCErrorColor(output, blockWidth, blockHeight);
        return;
    }

    /* Get CEM range index. */
    CEMindex = rangeIndices[remainingBits][CEMpairs - 1];

    /* Ranges 0..5 and smaller are not supported. */
    if (CEMindex < 4)
    {
        __glUtilSetASTCErrorColor(output, blockWidth, blockHeight);
        return;
    }

    /* Decode the color endpoint ISE. */
    __glUtilDecodeISE(blockData, fixedConfig, GL_FALSE, CEMindex, CEMpairs * 2, CEP);

    /* Unquantize the color end points. */
    __glUtilUnquantizeCEM(CEMindex, CEMpairs * 2, CEP);

    __glUtilDecodeCEM(partCount, CEM, CEP, e0, e1);

    /* Get the color component for the second plane. */
    plane2comp = astcBlock.dualPlane
        ? (GLushort) __glUtilReadBlock(blockData, lowerBits - 2, 2, GL_FALSE)
        : (GLushort) -1;

    /* Get the partition index. */
    partSeed = (partCount == 0) ? 0 : (GLushort) __glUtilReadBlock(blockData, 13, 10, GL_FALSE);


    for (j = 0; j < blockHeight; j++)
    {
        for (i = 0; i < blockWidth; i++)
        {
            __glUtilGetPartIndex(partSeed, partCount, i, j, smallBlock, &partIndex);

            __glUtilComputeColor(sRGB, e0[partIndex], e1[partIndex], &effective[(j * blockWidth + i) * 2], plane2comp, &output[(j * blockWidth + i) * 4]);
        }
    }
}

static void
gcChipDecodeASTCBlock(
    GLubyte *Output,
    GLubyte blockWidth,
    GLubyte blockHeight,
    const GLubyte *blockData,
    GLboolean sRGB
    )
{
    GLushort blockMode;

    /* extract header fields */
    blockMode = (GLushort) __glUtilReadBlock(blockData, 0, 11, GL_FALSE);

    /* Void-extent*/
    if ((blockMode & 0x1FF) == 0x1FC)
    {
        __glUtilDecodeVoidExtent(Output, blockMode, blockData, blockWidth, blockHeight, sRGB);
    }
    else
    {
        __glUtilDecodeBlock(Output, blockWidth, blockHeight, blockMode, blockData, sRGB);
    }
}

GLvoid*
gcChipDecompressASTC(
    IN  __GLcontext *gc,
    IN  gctSIZE_T Width,
    IN  gctSIZE_T Height,
    IN  gctSIZE_T numSlices,
    IN  gctSIZE_T compressedSize,
    IN  const void *Data,
    IN  __GLformatInfo *formatInfo,
    OUT gceSURF_FORMAT *Format,
    OUT gctSIZE_T *pRowStride
    )
{
    GLubyte *pixels = gcvNULL;
    GLubyte *line = gcvNULL;
    GLubyte *pLine = gcvNULL;
    GLubyte *pOut = gcvNULL;
    const GLubyte * data;
    gctSIZE_T x, y, z, stride, j;
    GLubyte blockWidth = (GLubyte)formatInfo->blockWidth;
    GLubyte blockHeight = (GLubyte)formatInfo->blockHeight;
    gctSIZE_T xBlock, yBlock;
    GLboolean sRGB = GL_FALSE;
    gctSIZE_T offset;

    gcmHEADER_ARG("gc=0x%x Width=%d Height=%d Data=0x%x Format=0x%x pRowStride=0x%x",
                   gc, Width, Height, Data, Format, pRowStride);

    sRGB = (formatInfo->glFormat >= GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR) &&
           (formatInfo->glFormat <= GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR);

    xBlock = (Width + blockWidth - 1) / blockWidth;
    yBlock = (Height + blockHeight - 1) / blockHeight;
    stride = Width * 4;

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, xBlock * blockWidth * yBlock * blockHeight * 4 * numSlices, (gctPOINTER*)&pixels)))
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, blockWidth * blockHeight * 4, (gctPOINTER*)&line)))
    {
        /* free memory.*/
        if (pixels)
        {
            gcoOS_Free(gcvNULL, (gctPOINTER)pixels);
            pixels = gcvNULL;
        }
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    for (z = 0; z < numSlices; z++)
    {
        for (y = 0; y < yBlock; y++)
        {
            for (x = 0; x < (xBlock - 1); x++)
            {
                offset = z * compressedSize + (y * xBlock + x) * 16;
                data = (GLubyte*)Data + offset;
                gcChipDecodeASTCBlock(line, blockWidth, blockHeight, data, sRGB);
                pLine = line;

                for (j = 0; j  < blockHeight; j ++)
                {
                    pOut = pixels + (z * Width * Height + y * Width * blockHeight + x * blockWidth + j * Width) * 4;
                    __GL_MEMCOPY(pOut, pLine, blockWidth * 4);
                    pLine += blockWidth * 4;
                }
            }
            offset = z * compressedSize + (y * xBlock + x) * 16;
            data = (GLubyte*)Data + offset;
            gcChipDecodeASTCBlock(line, blockWidth, blockHeight, data, sRGB);
            pLine = line;
            for (j = 0; j  < blockHeight; j ++)
            {
                pOut = pixels + (z * Width * Height + y * Width * blockHeight + x * blockWidth + j * Width) * 4;
                __GL_MEMCOPY(pOut, pLine, (Width - x * blockWidth) * 4);
                pLine += blockWidth * 4;
            }
        }
    }

    if (line)
    {
        gcoOS_Free(gcvNULL, (gctPOINTER)line);
        line = gcvNULL;
    }

    GL_ASSERT(Format && pRowStride);
    *Format = gcvSURF_A8B8G8R8;
    *pRowStride = stride;

    gcmFOOTER_ARG("return=0x%x", pixels);
    return pixels;
}

/*************************************************************************
** DXT Decompress
*************************************************************************/

#define __GL_DXT_RED(c)   ( (c) >> 11 )
#define __GL_DXT_GREEN(c) ( ((c) >> 5) & 0x3F )
#define __GL_DXT_BLUE(c)  ( (c) & 0x1F )

#define __GL_DXT_EXPAND_RED(c)   ( (((c) & 0xF800) << 8) | (((c) & 0xE000) << 3) )
#define __GL_DXT_EXPAND_GREEN(c) ( (((c) & 0x07E0) << 5) | (((c) & 0x0600) >> 1) )
#define __GL_DXT_EXPAND_BLUE(c)  ( (((c) & 0x001F) << 3) | (((c) & 0x001C) >> 2) )

/* Decode 64-bits of color information. */
static void
gcChipDecodeDXTColor16(
    IN gctSIZE_T Width,
    IN gctSIZE_T Height,
    IN gctSIZE_T Stride,
    IN const GLubyte * Data,
    OUT GLubyte * Output
    )
{
    GLushort c0, c1;
    GLushort color[4];
    GLushort r, g, b;
    gctSIZE_T x, y;
    gcmHEADER_ARG("Width=%d Height=%d Stride=%d Data=0x%x Output=0x%x",
                  Width, Height, Stride, Data, Output);

    /* Decode color 0. */
    c0 = *(GLushort*)Data;
    color[0] = 0x8000 | (c0 & 0x001F) | ((c0 & 0xFFC0) >> 1);

    /* Decode color 1. */
    c1 = *(GLushort*)(Data + 2);
    color[1] = 0x8000 | (c1 & 0x001F) | ((c1 & 0xFFC0) >> 1);

    if (c0 > c1)
    {
        /* Compute color 2: (c0 * 2 + c1) / 3. */
        r = (2 * __GL_DXT_RED  (c0) + __GL_DXT_RED  (c1)) / 3;
        g = (2 * __GL_DXT_GREEN(c0) + __GL_DXT_GREEN(c1)) / 3;
        b = (2 * __GL_DXT_BLUE (c0) + __GL_DXT_BLUE (c1)) / 3;
        color[2] = 0x8000 | (r << 10) | ((g & 0x3E) << 4) | b;

        /* Compute color 3: (c0 + 2 * c1) / 3. */
        r = (__GL_DXT_RED  (c0) + 2 * __GL_DXT_RED  (c1)) / 3;
        g = (__GL_DXT_GREEN(c0) + 2 * __GL_DXT_GREEN(c1)) / 3;
        b = (__GL_DXT_BLUE (c0) + 2 * __GL_DXT_BLUE (c1)) / 3;
        color[3] = 0x8000 | (r << 10) | ((g & 0x3E) << 4) | b;
    }
    else
    {
        /* Compute color 2: (c0 + c1) / 2. */
        r = (__GL_DXT_RED  (c0) + __GL_DXT_RED  (c1)) / 2;
        g = (__GL_DXT_GREEN(c0) + __GL_DXT_GREEN(c1)) / 2;
        b = (__GL_DXT_BLUE (c0) + __GL_DXT_BLUE (c1)) / 2;
        color[2] = 0x8000 | (r << 10) | ((g & 0x3E) << 4) | b;

        /* Color 3 is opaque black. */
        color[3] = 0;
    }

    /* Walk all lines. */
    for (y = 0; y < Height; y++)
    {
        /* Get lookup for line. */
        GLubyte bits = Data[4 + y];

        /* Walk all pixels. */
        for (x = 0; x < Width; x++, bits >>= 2, Output += 2)
        {
            /* Copy the color. */
            *(GLshort *) Output = color[bits & 3];
        }

        /* Next line. */
        Output += Stride - Width * 2;
    }
    gcmFOOTER_NO();
}

/* Decode 64-bits of color information. */
static void
gcChipDecodeDXTColor32(
    IN gctSIZE_T Width,
    IN gctSIZE_T Height,
    IN gctSIZE_T Stride,
    IN const GLubyte * Data,
    IN const GLubyte *Alpha,
    OUT GLubyte * Output
    )
{
    GLuint color[4];
    GLushort c0, c1;
    gctSIZE_T x, y;
    GLuint r, g, b;
    gcmHEADER_ARG("Width=%d Height=%d Stride=%d Data=0x%x Alpha=0x%x Output=0x%x",
                  Width, Height, Stride, Data, Alpha, Output);

    /* Decode color 0. */
    c0       = Data[0] | (Data[1] << 8);
    color[0] = __GL_DXT_EXPAND_RED(c0) | __GL_DXT_EXPAND_GREEN(c0) | __GL_DXT_EXPAND_BLUE(c0);

    /* Decode color 1. */
    c1       = Data[2] | (Data[3] << 8);
    color[1] = __GL_DXT_EXPAND_RED(c1) | __GL_DXT_EXPAND_GREEN(c1) | __GL_DXT_EXPAND_BLUE(c1);

    /* Compute color 2: (c0 * 2 + c1) / 3. */
    r = (2 * (color[0] & 0xFF0000) + (color[1] & 0xFF0000)) / 3;
    g = (2 * (color[0] & 0x00FF00) + (color[1] & 0x00FF00)) / 3;
    b = (2 * (color[0] & 0x0000FF) + (color[1] & 0x0000FF)) / 3;
    color[2] = (r & 0xFF0000) | (g & 0x00FF00) | (b & 0x0000FF);

    /* Compute color 3: (c0 + 2 * c1) / 3. */
    r = ((color[0] & 0xFF0000) + 2 * (color[1] & 0xFF0000)) / 3;
    g = ((color[0] & 0x00FF00) + 2 * (color[1] & 0x00FF00)) / 3;
    b = ((color[0] & 0x0000FF) + 2 * (color[1] & 0x0000FF)) / 3;
    color[3] = (r & 0xFF0000) | (g & 0x00FF00) | (b & 0x0000FF);

    /* Walk all lines. */
    for (y = 0; y < Height; y++)
    {
        /* Get lookup for line. */
        GLubyte bits = Data[4 + y];
        gctSIZE_T a  = y << 2;

        /* Walk all pixels. */
        for (x = 0; x < Width; x++, bits >>= 2, Output += 4)
        {
            /* Combine the lookup color with the alpha value. */
            GLuint c = color[bits & 3] | (Alpha[a++] << 24);
            *(GLuint *) Output = c;
        }

        /* Next line. */
        Output += Stride - Width * 4;
    }
    gcmFOOTER_NO();
}

/* Decode 64-bits of alpha information. */
static void
gcChipDecodeDXT3Alpha(
    IN const GLubyte *Data,
    OUT GLubyte *Output
    )
{
    GLint i;
    GLubyte a;
    gcmHEADER_ARG("Data=0x%x Output=0x%x", Data, Output);

    /* Walk all alpha pixels. */
    for (i = 0; i < 8; i++, Data++)
    {
        /* Get even alpha and expand into 8-bit. */
        a = *Data & 0x0F;
        *Output++ = a | (a << 4);

        /* Get odd alpha and expand into 8-bit. */
        a = *Data >> 4;
        *Output++ = a | (a << 4);
    }
    gcmFOOTER_NO();
}

/* Decode 64-bits of alpha information. */
static void
gcChipDecodeDXT5Alpha(
    IN const GLubyte *Data,
    OUT GLubyte *Output
    )
{
    GLint i, j, n;
    GLubyte a[8];
    GLushort bits = 0;
    gcmHEADER_ARG("Data=0x%x Output=0x%x", Data, Output);

    /* Load alphas 0 and 1. */
    a[0] = Data[0];
    a[1] = Data[1];

    if (a[0] > a[1])
    {
        /* Interpolate alphas 2 through 7. */
        a[2] = (GLubyte) ((6 * a[0] + 1 * a[1]) / 7);
        a[3] = (GLubyte) ((5 * a[0] + 2 * a[1]) / 7);
        a[4] = (GLubyte) ((4 * a[0] + 3 * a[1]) / 7);
        a[5] = (GLubyte) ((3 * a[0] + 4 * a[1]) / 7);
        a[6] = (GLubyte) ((2 * a[0] + 5 * a[1]) / 7);
        a[7] = (GLubyte) ((1 * a[0] + 6 * a[1]) / 7);
    }
    else
    {
        /* Interpolate alphas 2 through 5. */
        a[2] = (GLubyte) ((4 * a[0] + 1 * a[1]) / 5);
        a[3] = (GLubyte) ((3 * a[0] + 2 * a[1]) / 5);
        a[4] = (GLubyte) ((2 * a[0] + 3 * a[1]) / 5);
        a[5] = (GLubyte) ((1 * a[0] + 4 * a[1]) / 5);

        /* Set alphas 6 and 7. */
        a[6] = 0;
        a[7] = 255;
    }

    /* Walk all pixels. */
    for (i = 0, j = 2, n = 0; i < 16; i++, bits >>= 3, n -= 3)
    {
        /* Test if we have enough bits in the accumulator. */
        if (n < 3)
        {
            /* Load another chunk of bits in the accumulator. */
            bits |= Data[j++] << n;
            n += 8;
        }

        /* Copy decoded alpha value. */
        Output[i] = a[bits & 0x7];
    }
    gcmFOOTER_NO();
}


/*************************************************************************
** PowerVR Decompress
*************************************************************************/

/* Decompress a DXT texture. */
GLvoid*
gcChipDecompressDXT(
    IN  __GLcontext* gc,
    IN  gctSIZE_T Width,
    IN  gctSIZE_T Height,
    IN  gctSIZE_T ImageSize,
    IN  const void * Data,
    IN  GLenum InternalFormat,
    OUT gceSURF_FORMAT *Format,
    OUT gctSIZE_T* pRowStride
    )
{
    GLubyte * pixels = gcvNULL;
    GLubyte * line;
    const GLubyte * data;
    gctSIZE_T x, y, stride;
    GLubyte alpha[16];
    gctSIZE_T bpp;

    gcmHEADER_ARG("gc=0x%x Width=%d Height=%d ImageSize=%d Data=0x%x InternalFormat=0x%04x Format=0x%x pRowStride=0x%x",
                   gc, Width, Height, ImageSize, Data, InternalFormat, Format, pRowStride);

    /* Determine bytes per pixel. */
    bpp = ((InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT) || (InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)) ? 4 : 2;

    /* Allocate the decompressed memory. */
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, Width * Height * bpp, (gctPOINTER*)&pixels)))
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    /* Initialize the variables. */
    stride = Width * bpp;
    data   = Data;

    GL_ASSERT(Format && pRowStride);

    /* Walk all lines, 4 lines per block. */
    for (y = 0, line = pixels; y < Height; y += 4, line += stride * 4)
    {
        GLubyte * p = line;

        /* Walk all pixels, 4 pixels per block. */
        for (x = 0; x < Width; x += 4)
        {
            /* Dispatch on format. */
            switch (InternalFormat)
            {
            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
                GL_ASSERT(ImageSize >= 16);

                /* Decode DXT3 alpha. */
                gcChipDecodeDXT3Alpha(data, alpha);
                /* Decompress one color block. */
                gcChipDecodeDXTColor32(__GL_MIN(Width - x, 4), __GL_MIN(Height - y, 4),
                                         stride, data + 8, alpha, p);

                /* 16 bytes per block. */
                data      += 16;
                ImageSize -= 16;
                *Format = gcvSURF_A8R8G8B8;
                *pRowStride = Width * 4;
                break;

            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
                GL_ASSERT(ImageSize >= 16);

                /* Decode DXT5 alpha. */
                gcChipDecodeDXT5Alpha(data, alpha);
                /* Decompress one color block. */
                gcChipDecodeDXTColor32(__GL_MIN(Width - x, 4), __GL_MIN(Height - y, 4),
                                         stride, data + 8, alpha, p);

                /* 16 bytes per block. */
                data      += 16;
                ImageSize -= 16;
                *Format = gcvSURF_A8R8G8B8;
                *pRowStride = Width * 4;
                break;

            default:
                GL_ASSERT(ImageSize >= 8);

                /* Decompress one color block. */
                gcChipDecodeDXTColor16(__GL_MIN(Width - x, 4), __GL_MIN(Height - y, 4), stride, data, p);

                /* 8 bytes per block. */
                data      += 8;
                ImageSize -= 8;
                *Format = gcvSURF_A1R5G5B5;
                *pRowStride = Width * 2;
                break;
            }

            /* Next block. */
            p += 4 * bpp;
        }
    }

    /* Return pointer to decompressed data. */
    gcmFOOTER_ARG("return=0x%x", pixels);
    return pixels;
}

GLvoid*
gcChipDecompressPalette(
    IN  __GLcontext* gc,
    IN  GLenum InternalFormat,
    IN  gctSIZE_T Width,
    IN  gctSIZE_T Height,
    IN  GLint Level,
    IN  gctSIZE_T ImageSize,
    IN  const void * Data,
    OUT gceSURF_FORMAT * Format,
    OUT gctSIZE_T *pRowStride)
{
    gctSIZE_T pixelBits = 0, paletteBytes = 0;
    const GLubyte * palette;
    const GLubyte * data;
    GLubyte * pixels = gcvNULL;
    gctSIZE_T x, y, bytes;
    gctSIZE_T offset;

    gcmHEADER_ARG("gc=0x%x InternalFormat=0x%04x Width=%d Height=%d Level=%d ImageSize=%d Data=0x%x Format=0x%x pRowStride=0x%x",
                   gc, InternalFormat, Width, Height, Level, ImageSize, Data, Format, pRowStride);

    GL_ASSERT(Format && pRowStride);

    switch (InternalFormat)
    {
    case GL_PALETTE4_RGBA4_OES:
        pixelBits    = 4;
        paletteBytes = 16 / 8;
        *Format      = gcvSURF_R4G4B4A4;
        *pRowStride  = Width * 2;
        break;

    case GL_PALETTE4_RGB5_A1_OES:
        pixelBits    = 4;
        paletteBytes = 16 / 8;
        *Format      = gcvSURF_R5G5B5A1;
        *pRowStride  = Width * 2;
        break;

    case GL_PALETTE4_R5_G6_B5_OES:
        pixelBits    = 4;
        paletteBytes = 16 / 8;
        *Format      = gcvSURF_R5G6B5;
        *pRowStride  = Width * 2;
        break;

    case GL_PALETTE4_RGB8_OES:
        pixelBits    = 4;
        paletteBytes = 24 / 8;
        *Format      = gcvSURF_B8G8R8;
        *pRowStride  = Width * 3;
        break;

    case GL_PALETTE4_RGBA8_OES:
        pixelBits    = 4;
        paletteBytes = 32 / 8;
        *Format      = gcvSURF_A8B8G8R8;
        *pRowStride  = Width * 4;
        break;

    case GL_PALETTE8_RGBA4_OES:
        pixelBits    = 8;
        paletteBytes = 16 / 8;
        *Format      = gcvSURF_R4G4B4A4;
        *pRowStride  = Width * 2;
        break;

    case GL_PALETTE8_RGB5_A1_OES:
        pixelBits    = 8;
        paletteBytes = 16 / 8;
        *Format      = gcvSURF_R5G5B5A1;
        *pRowStride  = Width * 2;
        break;

    case GL_PALETTE8_R5_G6_B5_OES:
        pixelBits    = 8;
        paletteBytes = 16 / 8;
        *Format      = gcvSURF_R5G6B5;
        *pRowStride  = Width * 2;
        break;

    case GL_PALETTE8_RGB8_OES:
        pixelBits    = 8;
        paletteBytes = 24 / 8;
        *Format      = gcvSURF_B8G8R8;
        *pRowStride  = Width * 3;
        break;

    case GL_PALETTE8_RGBA8_OES:
        pixelBits    = 8;
        paletteBytes = 32 / 8;
        *Format      = gcvSURF_A8B8G8R8;
        *pRowStride  = Width * 4;
        break;
    }

    palette = Data;

    bytes = paletteBytes << pixelBits;
    data  = (const GLubyte *) palette + bytes;

    gcmASSERT(ImageSize > bytes);
    ImageSize -= bytes;

    while (Level-- > 0)
    {
        bytes  = gcmALIGN(Width * pixelBits, 8) / 8 * Height;
        data  += bytes;

        GL_ASSERT(ImageSize > bytes);
        ImageSize -= bytes;

        Width  = Width / 2;
        Height = Height / 2;
    }

    bytes = gcmALIGN(Width * paletteBytes, (gctSIZE_T)gc->clientState.pixel.unpackModes.alignment) * Height;

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, bytes, (gctPOINTER*)&pixels)))
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    for (y = 0, offset = 0; y < Height; ++y)
    {
        for (x = 0; x < Width; ++x)
        {
            GLubyte pixel;

            GL_ASSERT(ImageSize > 0);
            if (pixelBits == 4)
            {
                if (x & 1)
                {
                    pixel = *data++ & 0xF;
                    --ImageSize;
                }
                else
                {
                    pixel = *data >> 4;
                }
            }
            else
            {
                pixel = *data++;
                --ImageSize;
            }

            __GL_MEMCOPY(pixels + offset, palette + pixel * paletteBytes, paletteBytes);
            offset += paletteBytes;
        }

        offset = gcmALIGN(offset, (gctSIZE_T)gc->clientState.pixel.unpackModes.alignment);

        if (x & 1)
        {
            ++data;
            --ImageSize;
        }
    }

    gcmFOOTER_ARG("return=0x%x", pixels);
    return pixels;
}

GLvoid*
gcChipDecompressETC1(
    IN  __GLcontext* gc,
    IN  gctSIZE_T Width,
    IN  gctSIZE_T Height,
    IN  gctSIZE_T ImageSize,
    IN  const void * Data,
    OUT gceSURF_FORMAT * Format,
    OUT gctSIZE_T *pRowStride
    )
{
    GLubyte * pixels = gcvNULL;
    GLubyte * line;
    const GLubyte * data;
    gctSIZE_T x, y, stride;

    gcmHEADER_ARG("gc=0x%x Width=%d Height=%d ImageSize=%d Data=0x%x Format=0x%x pRowStride=0x%x",
                   gc, Width, Height, ImageSize, Data, Format, pRowStride);

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, Width * Height * 3, (gctPOINTER*)&pixels)))
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    stride = Width * 3;
    data   = Data;

    for (y = 0, line = pixels; y < Height; y += 4, line += stride * 4)
    {
        GLubyte * p = line;

        for (x = 0; x < Width; x += 4)
        {
            GL_ASSERT(ImageSize >= 8);
            gcChipDecodeETC1Block(p, stride, __GL_MIN(Width - x, 4), __GL_MIN(Height - y, 4), data);
            p         += 4 * 3;
            data      += 8;
            ImageSize -= 8;
        }
    }

    GL_ASSERT(Format && pRowStride);
    *Format = gcvSURF_B8G8R8;
    *pRowStride = stride;

    gcmFOOTER_ARG("return=0x%x", pixels);
    return pixels;
}



/*************************************************************************
** EAC 11bit Decompress
*************************************************************************/

static GLint
__get3Bits(
    const GLubyte * Data,
    gctSIZE_T start
    )
{
    gctSIZE_T i, j;

    i = start / 8;
    j = start % 8;

    if (j < 6)
    {
        /* Grab 3 bits from Ith byte. */
        return (Data[i] >> j) & 0x7;
    }
    else
    {
        /* Grab j bits from Ith byte, and 8 - j bits from (I+1)th byte */
        return ((Data[i] >> j) | (Data[i+1] << (8 - j))) & 0x7;
    }
}

/* Decode 8 bytes of 16 compressed 11 bit pixels,
   into 16 16F float values. */
static void
gcChipDecodeEAC11Block(
    GLubyte * Output,
    gctSIZE_T Width,
    gctSIZE_T inX,
    gctSIZE_T inY,
    gctSIZE_T RequiredW,
    gctSIZE_T RequiredH,
    GLboolean signedFormat,
    GLboolean gPresent,
    const GLubyte * Data
    )
{
    gctSIZE_T i, x, y, offset, pixelStride;
    gctSIZE_T pixelIndex, fetchIndex;
    GLint modifier, A;
    GLfloat color;

    struct _eacBlock
    {
        GLbitfield skip1        : 8;
        GLbitfield skip2        : 8;
        GLbitfield skip3        : 8;
        GLbitfield skip4        : 8;
        GLbitfield skip5        : 8;
        GLbitfield skip6        : 8;

        GLbitfield tableIndex   : 4;
        GLbitfield multiplier   : 4;
        GLbitfield baseCodeWord : 8;
    } eacBlock;

    /* EAC modifier table. */
    static GLint EACModifierTable[] =
    {
         -3,  -6,  -9, -15,   2,   5,   8,  14,        /* codeword0. */
         -3,  -7, -10, -13,   2,   6,   9,  12,        /* codeword1. */
         -2,  -5,  -8, -13,   1,   4,   7,  12,        /* codeword2. */
         -2,  -4,  -6, -13,   1,   3,   5,  12,        /* codeword3. */
         -3,  -6,  -8, -12,   2,   5,   7,  11,        /* codeword4. */
         -3,  -7,  -9, -11,   2,   6,   8,  10,        /* codeword5. */
         -4,  -7,  -8, -11,   3,   6,   7,  10,        /* codeword6. */
         -3,  -5,  -8, -11,   2,   4,   7,  10,        /* codeword7. */
         -2,  -6,  -8, -10,   1,   5,   7,   9,        /* codeword8. */
         -2,  -5,  -8, -10,   1,   4,   7,   9,        /* codeword9. */
         -2,  -4,  -8, -10,   1,   3,   7,   9,        /* codeword10. */
         -2,  -5,  -7, -10,   1,   4,   6,   9,        /* codeword11. */
         -3,  -4,  -7, -10,   2,   3,   6,   9,        /* codeword12. */
         -1,  -2,  -3, -10,   0,   1,   2,   9,        /* codeword13. */
         -4,  -6,  -8,  -9,   3,   5,   7,   8,        /* codeword14. */
         -3,  -5,  -7,  -9,   2,   4,   6,   8,        /* codeword15. */
    };

    GLubyte *littleEndianArray = (GLubyte *) &eacBlock;

    /* Convert to little endian. */
    for (i = 0; i < 8; i++)
    {
        littleEndianArray[i] = Data[7 - i];
    }

    /* Stride in bytes. */
    pixelStride = gPresent ? 4 : 2;

    for (y = 0; y < RequiredH; y++)
    {
        offset = ((inY + y) * Width + inX) * pixelStride;

        for (x = 0; x < RequiredW; x++)
        {
            fetchIndex = (15 - (x << 2) - y) * 3;

            /* Get fetchIndex to fetchIndex + 2 from Data (64 bit block). */
            pixelIndex = __get3Bits(littleEndianArray, fetchIndex);

            modifier = EACModifierTable[(eacBlock.tableIndex << 3) + pixelIndex];

            if (signedFormat)
            {
                GLubyte baseCodeWord = (GLubyte)eacBlock.baseCodeWord;
                A = *((GLbyte*)&baseCodeWord);
                if (A == -128)
                {
                    A = -127;
                }
                A <<= 3;
                if (eacBlock.multiplier > 0)
                {
                    A += (eacBlock.multiplier * modifier) << 3;
                }
                else
                {
                    A += modifier;
                }

                /* Clamp to [-1023, 1023]. */
                A = (A < -1023) ? -1023 : ((A > 1023) ? 1023 : A);

                if ((A > 0) && (A < 1023))
                {
                    color = (A + 0.5f) / 1023.0f;
                }
                else
                {
                    color = A / 1023.0f;
                }
            }
            else
            {
                A = (eacBlock.baseCodeWord << 3) + 4;

                if (eacBlock.multiplier > 0)
                {
                    A += (eacBlock.multiplier * modifier) << 3;
                }
                else
                {
                    A += modifier;
                }

                /* Clamp to [0, 2047]. */
                A = (A < 0) ? 0 : ((A > 2047) ? 2047 : A);

                if ((A < 2047) && (A != 1023))
                {
                    color = (A + 0.5f) / 2047.0f;
                }
                else
                {
                    color = A / 2047.0f;
                }
            }

            /* Write the decoded color. */
            *(gctUINT16*)(Output + offset) = gcoMATH_FloatToFloat16(*(gctUINT32*)&color);
            offset += pixelStride;
        }
    }
}

/* Decompress EAC 11bit textures. */
GLvoid*
gcChipDecompress_EAC_11bitToR16F(
    IN  __GLcontext* gc,
    IN  gctSIZE_T Width,
    IN  gctSIZE_T Height,
    IN  gctSIZE_T Depth,
    IN  gctSIZE_T ImageSize,
    IN  const void * Data,
    IN  GLenum InternalFormat,
    OUT gceSURF_FORMAT *Format,
    OUT gctSIZE_T *pRowStride
    )
{
    GLubyte * pIn  = gcvNULL;
    GLubyte * pOut = gcvNULL;
    GLvoid  * decompressed = gcvNULL;
    gctSIZE_T offset, outputSize, x, y, slice;
    GLboolean gPresent = gcvFALSE;
    GLboolean signedFormat = gcvFALSE;

    gcmHEADER_ARG("gc=0x%x Width=%d Height=%d Depth=%d, ImageSize=%d Data=0x%x "
                  "InternalFormat=0x%04x Format=0x%x pRowStride=0x%x", gc, Width,
                  Height, Depth, ImageSize, Data, InternalFormat, Format, pRowStride);

    /* Determine bytes per pixel. */
    GL_ASSERT((ImageSize % 8) == 0 && (ImageSize % Depth) == 0);
    GL_ASSERT(Format && pRowStride);

    switch (InternalFormat)
    {
    case GL_COMPRESSED_SIGNED_R11_EAC:
        signedFormat = gcvTRUE;
        /* Fall through. */
    case GL_COMPRESSED_R11_EAC:
        *Format  = gcvSURF_R16F_1_A4R4G4B4;
        *pRowStride = Width * 2;
        break;

    case GL_COMPRESSED_SIGNED_RG11_EAC:
        signedFormat = gcvTRUE;
        /* Fall through. */
    case GL_COMPRESSED_RG11_EAC:
        gPresent = gcvTRUE;
        *Format  = gcvSURF_G16R16F_1_A8R8G8B8;
        *pRowStride = Width * 4;
        break;

    default:
        GL_ASSERT(GL_FALSE);
        return gcvNULL;
    }
    outputSize = (*pRowStride) * Height * Depth;

    /* Allocate the decompressed memory. */
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, outputSize, (gctPOINTER*)&decompressed)))
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    pIn  = (GLubyte*)Data;
    pOut = (GLubyte*)decompressed;
    for (slice = 0; slice < Depth; ++slice)
    {
        offset = 0;
        for (y = 0; y < Height; y += 4)
        {
            for (x = 0; x < Width; x += 4)
            {
                /* For some mip level, not the whole blocks need to decompressed */
                gctSIZE_T w = __GL_MIN(Width - x, 4);
                gctSIZE_T h = __GL_MIN(Height - y, 4);

                /* Decompress 8 byte R block into 16 pixels. */
                gcChipDecodeEAC11Block(pOut,
                                       Width,
                                       x, y,
                                       w, h,
                                       signedFormat,
                                       gPresent,
                                       pIn + offset);
                offset += 8;

                if (gPresent)
                {
                    /* Decompress 8 byte G block into 16 pixels. */
                    gcChipDecodeEAC11Block(pOut + 2,
                                           Width,
                                           x, y,
                                           w, h,
                                           signedFormat,
                                           gPresent,
                                           pIn + offset);
                    offset += 8;
                }
            }
        }

        pOut += (*pRowStride) * Height;
        pIn  += ImageSize / Depth;
    }


    /* Return pointer to converted data. */
    gcmFOOTER_ARG("return=0x%x", decompressed);
    return decompressed;
}

/*************************************************************************
** RGTC Decompress
*************************************************************************/

/* Decode 64-bits of Unsigned RGTC (COMPRESSED_RED_RGTC1) to 128 bits of RED8. */
static void
gcChipDecodeRGTC1(
    IN gctSIZE_T Width,
    IN gctSIZE_T Height,
    IN gctSIZE_T Stride,
    IN const GLubyte * Data,
    OUT GLubyte * Output
    )
{
    GLubyte reds[8];
    gctSIZE_T x, y;
    GLushort bits = 0;
    GLint n, bitsStart;

    /* Data (8 bytes): red0; red1; bits0; bits1; bits2; bits3; bits4; bits5 */

    /* Decode RED 0: RED0 = red0 */
    reds[0] = *Data;

    /* Decode RED 1: RED1 = red1 */
    reds[1] = *(Data + 1);

    /* Interpolate RED 2 through RED 7. */
    if (reds[0] > reds[1])
    {
        reds[2] = (6 * reds[0] + 1 * reds[1]) / 7;
        reds[3] = (5 * reds[0] + 2 * reds[1]) / 7;
        reds[4] = (4 * reds[0] + 3 * reds[1]) / 7;
        reds[5] = (3 * reds[0] + 4 * reds[1]) / 7;
        reds[6] = (2 * reds[0] + 5 * reds[1]) / 7;
        reds[7] = (1 * reds[0] + 6 * reds[1]) / 7;
    }
    else
    {
        reds[2] = (4 * reds[0] + 1 * reds[1]) / 5;
        reds[3] = (3 * reds[0] + 2 * reds[1]) / 5;
        reds[4] = (2 * reds[0] + 3 * reds[1]) / 5;
        reds[5] = (1 * reds[0] + 4 * reds[1]) / 5;
        /* RED MIN */
        reds[6] = 0x00;
        /* RED MAX */
        reds[7] = 0xFF;
    }

    /* Walk all lines. */
    for (y = 0, n = 0, bitsStart = 2; y < Height; y++)
    {
        /* Walk all pixels. 3 bits per control code. */
        for (x = 0; x < Width; x++, bits >>= 3, n -= 3, Output += 1)
        {
            /* Test if we have enough bits in the accumulator. */
            if (n < 3)
            {
                /* Load another chunk of bits in the accumulator. */
                bits |= Data[bitsStart++] << n;
                n += 8;
            }

            /* Copy the color. */
            *Output = reds[bits & 0x7];
        }

        /* Next line. */
        Output += Stride - Width * 1;
    }
}

/* Decode 64-bits of Signed RGTC (COMPRESSED_SIGNED_RED_RGTC1) to 128 bits of RED8. */
static void
gcChipDecodeSignedRGTC1(
    IN gctSIZE_T Width,
    IN gctSIZE_T Height,
    IN gctSIZE_T Stride,
    IN const GLubyte * Data,
    OUT GLubyte * Output
    )
{
    GLfloat reds[8];
    gctSIZE_T x, y;
    GLushort bits = 0;
    GLint n, bitsStart;

    /* Data (8 bytes): red0; red1; bits0; bits1; bits2; bits3; bits4; bits5 */

    /* Decode RED 0: map signed red0 (-128, 127) to unsigned red (0, 255) */
    reds[0] = *Data + 128.0f;

    /* Decode RED 1: map signed red1 (-128, 127) to unsigned red (0, 255) */
    reds[1] = *(Data + 1) + 128.0f;

    /* Interpolate RED 2 through RED 7. */
    if (reds[0] > reds[1])
    {
        reds[2] = (6 * reds[0] + 1 * reds[1]) / 7.0f;
        reds[3] = (5 * reds[0] + 2 * reds[1]) / 7.0f;
        reds[4] = (4 * reds[0] + 3 * reds[1]) / 7.0f;
        reds[5] = (3 * reds[0] + 4 * reds[1]) / 7.0f;
        reds[6] = (2 * reds[0] + 5 * reds[1]) / 7.0f;
        reds[7] = (1 * reds[0] + 6 * reds[1]) / 7.0f;
    }
    else
    {
        reds[2] = (4 * reds[0] + 1 * reds[1]) / 5.0f;
        reds[3] = (3 * reds[0] + 2 * reds[1]) / 5.0f;
        reds[4] = (2 * reds[0] + 3 * reds[1]) / 5.0f;
        reds[5] = (1 * reds[0] + 4 * reds[1]) / 5.0f;
        /* RED MIN */
        reds[6] = 0x00;
        /* RED MAX */
        reds[7] = 0xFF;
    }

    /* Walk all lines. */
    for (y = 0, n = 0, bitsStart = 2; y < Height; y++)
    {
        /* Walk all pixels. 3 bits per control code. */
        for (x = 0; x < Width; x++, bits >>= 3, n -= 3, Output += 1)
        {
            /* Test if we have enough bits in the accumulator. */
            if (n < 3)
            {
                /* Load another chunk of bits in the accumulator. */
                bits |= Data[bitsStart++] << n;
                n += 8;
            }

            /* Copy the color. */
            *Output = (GLubyte)reds[bits & 0x7];
        }

        /* Next line. */
        Output += Stride - Width * 1;
    }
}

/* Decode 128-bits of Unsigned RGTC (COMPRESSED_RG_RGTC2) to 256 bits of RG8. */
static void
gcChipDecodeRGTC2(
    IN gctSIZE_T Width,
    IN gctSIZE_T Height,
    IN gctSIZE_T Stride,
    IN const GLubyte * Data,
    OUT GLubyte * Output
    )
{
    GLfloat reds[8];
    GLfloat greens[8];
    gctSIZE_T x, y;
    GLushort redBits = 0;
    GLushort greenBits = 0;
    GLint n, redBitsStart, greenBitsStart;

    /* Data (16 bytes): red0;   red1;   bits0; bits1; bits2; bits3; bits4; bits5; */
    /*                  green0; green1; bits0; bits1; bits2; bits3; bits4; bits5; */

    /* Decode RED 0: RED0 = red0 */
    reds[0] = *Data;

    /* Decode RED 1: RED1 = red1 */
    reds[1] = *(Data + 1);

    /* Decode GREEN 0: GREEN0 = green0 */
    greens[0] = *(Data + 8);

    /* Decode GREEN 1: GREEN1 = green1 */
    greens[1] = *(Data + 9);

    /* Interpolate RED 2 through RED 7. */
    if (reds[0] > reds[1])
    {
        reds[2] = (6 * reds[0] + 1 * reds[1]) / 7.0f;
        reds[3] = (5 * reds[0] + 2 * reds[1]) / 7.0f;
        reds[4] = (4 * reds[0] + 3 * reds[1]) / 7.0f;
        reds[5] = (3 * reds[0] + 4 * reds[1]) / 7.0f;
        reds[6] = (2 * reds[0] + 5 * reds[1]) / 7.0f;
        reds[7] = (1 * reds[0] + 6 * reds[1]) / 7.0f;
    }
    else
    {
        reds[2] = (4 * reds[0] + 1 * reds[1]) / 5.0f;
        reds[3] = (3 * reds[0] + 2 * reds[1]) / 5.0f;
        reds[4] = (2 * reds[0] + 3 * reds[1]) / 5.0f;
        reds[5] = (1 * reds[0] + 4 * reds[1]) / 5.0f;
        /* RED MIN */
        reds[6] = 0x00;
        /* RED MAX */
        reds[7] = 0xFF;
    }

    /* Interpolate GREEN 2 through GREEN 7. */
    if (greens[0] > greens[1])
    {
        greens[2] = (6 * greens[0] + 1 * greens[1]) / 7.0f;
        greens[3] = (5 * greens[0] + 2 * greens[1]) / 7.0f;
        greens[4] = (4 * greens[0] + 3 * greens[1]) / 7.0f;
        greens[5] = (3 * greens[0] + 4 * greens[1]) / 7.0f;
        greens[6] = (2 * greens[0] + 5 * greens[1]) / 7.0f;
        greens[7] = (1 * greens[0] + 6 * greens[1]) / 7.0f;
    }
    else
    {
        greens[2] = (4 * greens[0] + 1 * greens[1]) / 5.0f;
        greens[3] = (3 * greens[0] + 2 * greens[1]) / 5.0f;
        greens[4] = (2 * greens[0] + 3 * greens[1]) / 5.0f;
        greens[5] = (1 * greens[0] + 4 * greens[1]) / 5.0f;
        /* GREEN MIN */
        greens[6] = 0x00;
        /* GREEN MAX */
        greens[7] = 0xFF;
    }

    /* Walk all lines. */
    for (y = 0, n = 0, redBitsStart = 2, greenBitsStart = 10; y < Height; y++)
    {
        /* Walk all pixels. 3 bits per control code. */
        for (x = 0; x < Width; x++, redBits >>= 3, greenBits >>= 3, n -= 3, Output += 2)
        {
            /* Test if we have enough bits in the accumulator. */
            if (n < 3)
            {
                /* Load another chunk of bits in the accumulator. */
                redBits |= Data[redBitsStart++] << n;
                greenBits |= Data[greenBitsStart++] << n;
                n += 8;
            }

            /* Copy the color. */
            *Output = (GLubyte)reds[redBits & 0x7];
            *(Output+1) = (GLubyte)greens[greenBits & 0x7];
        }

        /* Next line. */
        Output += Stride - Width * 2;
    }
}


/* Decode 128-bits of Signed RGTC (COMPRESSED_SIGNED_RG_RGTC2) to 256 bits of RG8. */
static void
gcChipDecodeSignedRGTC2(
    IN gctSIZE_T Width,
    IN gctSIZE_T Height,
    IN gctSIZE_T Stride,
    IN const GLubyte * Data,
    OUT GLubyte * Output
    )
{
    GLfloat reds[8];
    GLfloat greens[8];
    gctSIZE_T x, y;
    GLushort redBits = 0;
    GLushort greenBits = 0;
    GLint n, redBitsStart, greenBitsStart;

    /* Data (16 bytes): red0;   red1;   bits0; bits1; bits2; bits3; bits4; bits5; */
    /*                  green0; green1; bits0; bits1; bits2; bits3; bits4; bits5; */

    /* Decode RED 0: RED0 = red0 */
    reds[0] = *Data + 128.0f;

    /* Decode RED 1: RED1 = red1 */
    reds[1] = *(Data + 1) + 128.0f;

    /* Decode GREEN 0: GREEN0 = green0 */
    greens[0] = *(Data + 8) + 128.0f;

    /* Decode GREEN 1: GREEN1 = green1 */
    greens[1] = *(Data + 9) + 128.0f;

    /* Interpolate RED 2 through RED 7. */
    if (reds[0] > reds[1])
    {
        reds[2] = (6 * reds[0] + 1 * reds[1]) / 7.0f;
        reds[3] = (5 * reds[0] + 2 * reds[1]) / 7.0f;
        reds[4] = (4 * reds[0] + 3 * reds[1]) / 7.0f;
        reds[5] = (3 * reds[0] + 4 * reds[1]) / 7.0f;
        reds[6] = (2 * reds[0] + 5 * reds[1]) / 7.0f;
        reds[7] = (1 * reds[0] + 6 * reds[1]) / 7.0f;
    }
    else
    {
        reds[2] = (4 * reds[0] + 1 * reds[1]) / 5.0f;
        reds[3] = (3 * reds[0] + 2 * reds[1]) / 5.0f;
        reds[4] = (2 * reds[0] + 3 * reds[1]) / 5.0f;
        reds[5] = (1 * reds[0] + 4 * reds[1]) / 5.0f;
        /* RED MIN */
        reds[6] = 0x00;
        /* RED MAX */
        reds[7] = 0xFF;
    }

    /* Interpolate GREEN 2 through GREEN 7. */
    if (greens[0] > greens[1])
    {
        greens[2] = (6 * greens[0] + 1 * greens[1]) / 7.0f;
        greens[3] = (5 * greens[0] + 2 * greens[1]) / 7.0f;
        greens[4] = (4 * greens[0] + 3 * greens[1]) / 7.0f;
        greens[5] = (3 * greens[0] + 4 * greens[1]) / 7.0f;
        greens[6] = (2 * greens[0] + 5 * greens[1]) / 7.0f;
        greens[7] = (1 * greens[0] + 6 * greens[1]) / 7.0f;
    }
    else
    {
        greens[2] = (4 * greens[0] + 1 * greens[1]) / 5.0f;
        greens[3] = (3 * greens[0] + 2 * greens[1]) / 5.0f;
        greens[4] = (2 * greens[0] + 3 * greens[1]) / 5.0f;
        greens[5] = (1 * greens[0] + 4 * greens[1]) / 5.0f;
        /* GREEN MIN */
        greens[6] = 0x00;
        /* GREEN MAX */
        greens[7] = 0xFF;
    }

    /* Walk all lines. */
    for (y = 0, n = 0, redBitsStart = 2, greenBitsStart = 10; y < Height; y++)
    {
        /* Walk all pixels. 3 bits per control code. */
        for (x = 0; x < Width; x++, redBits >>= 3, greenBits >>= 3, n -= 3, Output += 2)
        {
            /* Test if we have enough bits in the accumulator. */
            if (n < 3)
            {
                /* Load another chunk of bits in the accumulator. */
                redBits |= Data[redBitsStart++] << n;
                greenBits |= Data[greenBitsStart++] << n;
                n += 8;
            }

            /* Copy the color. */
            *Output = (GLubyte)reds[redBits & 0x7];
            *(Output+1) = (GLubyte)greens[greenBits & 0x7];
        }

        /* Next line. */
        Output += Stride - Width * 2;
    }
}

/* Decompress a RGTC texture. */
GLvoid*
gcChipDecompressRGTC(
    IN  __GLcontext* gc,
    IN  gctSIZE_T Width,
    IN  gctSIZE_T Height,
    IN  gctSIZE_T ImageSize,
    IN  const void * Data,
    IN  GLenum InternalFormat,
    OUT gceSURF_FORMAT *Format,
    OUT gctSIZE_T* pRowStride
    )
{
    GLubyte * pixels = gcvNULL;
    GLubyte * line;
    const GLubyte * data;
    gctSIZE_T x, y, stride;
    gctSIZE_T bpp;

    gcmHEADER_ARG("gc=0x%x Width=%d Height=%d ImageSize=%d Data=0x%x InternalFormat=0x%04x Format=0x%x pRowStride=0x%x",
                   gc, Width, Height, ImageSize, Data, InternalFormat, Format, pRowStride);

    /* Determine bytes per pixel. */
    bpp = ((InternalFormat == GL_COMPRESSED_RG_RGTC2) || (InternalFormat == GL_COMPRESSED_SIGNED_RG_RGTC2)) ? 4 : 2;

    /* Allocate the decompressed memory. */
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, Width * Height * bpp, (gctPOINTER*)&pixels)))
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    /* Initialize the variables. */
    stride = Width * bpp;
    data   = Data;

    /* Walk all lines, 4 lines per block. */
    for (y = 0, line = pixels; y < Height; y += 4, line += stride * 4)
    {
        GLubyte * p = line;

        /* Walk all pixels, 4 pixels per block. */
        for (x = 0; x < Width; x += 4)
        {
            /* Dispatch on format. */
            switch (InternalFormat)
            {
            case GL_COMPRESSED_RED_RGTC1:

                /* Decompress one color block. */
                gcChipDecodeRGTC1(__GL_MIN(Width - x, 4), __GL_MIN(Height - y, 4),
                                         stride, data, p);

                /* 8 bytes per block. */
                data      += 8;
                ImageSize -= 8;
                *Format = gcvSURF_R8;
                *pRowStride = Width * 4;
                break;

            case GL_COMPRESSED_SIGNED_RED_RGTC1:

                /* Decompress one color block. */
                gcChipDecodeSignedRGTC1(__GL_MIN(Width - x, 4), __GL_MIN(Height - y, 4),
                                         stride, data, p);

                /* 8 bytes per block. */
                data      += 8;
                ImageSize -= 8;
                *Format = gcvSURF_R8;
                *pRowStride = Width * 4;
                break;

            case GL_COMPRESSED_RG_RGTC2:

                /* Decompress one color block. */
                gcChipDecodeRGTC2(__GL_MIN(Width - x, 4), __GL_MIN(Height - y, 4),
                                         stride, data, p);

                /* 16 bytes per block. */
                data      += 16;
                ImageSize -= 16;
                *Format = gcvSURF_RG16;
                *pRowStride = Width * 4;
                break;

            case GL_COMPRESSED_SIGNED_RG_RGTC2:

                /* Decompress one color block. */
                gcChipDecodeSignedRGTC2(__GL_MIN(Width - x, 4), __GL_MIN(Height - y, 4),
                                         stride, data, p);

                /* 16 bytes per block. */
                data      += 16;
                ImageSize -= 16;
                *Format = gcvSURF_RG16;
                *pRowStride = Width * 4;
                break;

            default:
                /* unknown format */
                GL_ASSERT(GL_FALSE);
                break;
            }

            /* Next block. */
            p += 4 * bpp;
        }
    }

    /* Return pointer to decompressed data. */
    gcmFOOTER_ARG("return=0x%x", pixels);
    return pixels;
}


