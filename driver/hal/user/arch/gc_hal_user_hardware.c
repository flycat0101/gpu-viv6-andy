/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_hal_user_hardware_precomp.h"
#include "gc_hal_user.h"
#include "gc_hal_user_brush.h"

#if (gcdENABLE_3D || gcdENABLE_2D)
#include "gc_feature_database.h"
#endif

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcvZONE_HARDWARE

/* Multi-sample grid selection:
**
**  0   Square grid only.
**  1   Rotated diamonds but no jitter.
**  2   Rotated diamonds in a 2x2 jitter matrix.
*/
#define gcdGRID_QUALITY 1

/******************************************************************************\
********************************* Support Code *********************************
\******************************************************************************/

/*
 * SuperTileMode 2:
 * 21'{ X[21-1:6], Y[5],X[5],Y[4],X[4], Y[3],X[3],Y[2],X[2],Y[1:0],X[1:0] }
 *
 * SuperTileMode 1:
 * 21'{ X[21-1:6], Y[5:4],X[5:3],Y[3:2], X[2],Y[1:0],X[1:0] }
 *
 * SuperTileMode 0:
 * 21'{ X[21-1:6], Y[5:2],X[5:2], Y[1:0],X[1:0] }
 */
#define gcmTILE_OFFSET_X(X, Y, SuperTiled, SuperTileMode) \
        ((SuperTiled) ? \
            (SuperTileMode == 2) ? \
                (((X) &  0x03) << 0) | \
                (((Y) &  0x03) << 2) | \
                (((X) &  0x04) << 2) | \
                (((Y) &  0x04) << 3) | \
                (((X) &  0x08) << 3) | \
                (((Y) &  0x08) << 4) | \
                (((X) &  0x10) << 4) | \
                (((Y) &  0x10) << 5) | \
                (((X) &  0x20) << 5) | \
                (((Y) &  0x20) << 6) | \
                (((X) & ~0x3F) << 6)   \
            : \
                (SuperTileMode == 1) ? \
                (((X) &  0x03) << 0) | \
                (((Y) &  0x03) << 2) | \
                (((X) &  0x04) << 2) | \
                (((Y) &  0x0C) << 3) | \
                (((X) &  0x38) << 4) | \
                (((Y) &  0x30) << 6) | \
                (((X) & ~0x3F) << 6)   \
                : \
                    (((X) &  0x03) << 0) | \
                    (((Y) &  0x03) << 2) | \
                    (((X) &  0x3C) << 2) | \
                    (((Y) &  0x3C) << 6) | \
                    (((X) & ~0x3F) << 6)   \
        : \
            ((X &  0x03) << 0) | \
            ((Y &  0x03) << 2) | \
            ((X & ~0x03) << 2) \
            )

#define gcmTILE_OFFSET_Y(X, Y, SuperTiled) \
    ((SuperTiled) ? ((Y) & ~0x3F) : ((Y) & ~0x03))

#if gcdENABLE_3D
gceSTATUS _DestroyBlitDraw(
    gcoHARDWARE Hardware
    );
#endif

#if (gcdENABLE_3D || gcdENABLE_2D)
static gceSTATUS _ResetDelta(
    IN gcsSTATE_DELTA_PTR StateDelta
    )
{
    /* The delta should not be attached to any context. */
    gcmASSERT(StateDelta->refCount == 0);

    /* Not attached yet, advance the ID. */
    StateDelta->id += 1;

    /* Did ID overflow? */
    if (StateDelta->id == 0)
    {
        /* Reset the map to avoid erroneous ID matches. */
        gcoOS_ZeroMemory(gcmUINT64_TO_PTR(StateDelta->mapEntryID), StateDelta->mapEntryIDSize);

        /* Increment the main ID to avoid matches after reset. */
        StateDelta->id += 1;
    }

    /* Reset the vertex element count. */
    StateDelta->elementCount = 0;

    /* Reset the record count. */
    StateDelta->recordCount = 0;

    /* Success. */
    return gcvSTATUS_OK;
}

static gceSTATUS _MergeDelta(
    IN gcsSTATE_DELTA_PTR StateDelta
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsSTATE_DELTA_PTR prevDelta;
    gcsSTATE_DELTA_RECORD_PTR record;
    gctUINT i, count;

    /* Get the record count. */
    count = StateDelta->recordCount;

    /* Set the first record. */
    record = gcmUINT64_TO_PTR(StateDelta->recordArray);

    /* Get the previous delta. */
    prevDelta = gcmUINT64_TO_PTR(StateDelta->prev);

    /* Go through all records. */
    for (i = 0; i < count; i += 1)
    {
        /* Update the delta. */
        gcoHARDWARE_UpdateDelta(
            prevDelta, record->address, record->mask, record->data
            );

        /* Advance to the next state. */
        record += 1;
    }

    /* Update the element count. */
    if (StateDelta->elementCount != 0)
    {
        prevDelta->elementCount = StateDelta->elementCount;
    }

    /* Return the status. */
    return status;
}
#endif


#if (gcdENABLE_3D || gcdENABLE_2D)
static gceSTATUS _LoadStates(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctBOOL FixedPoint,
    IN gctUINT32 Count,
    IN gctUINT32 Mask,
    IN gctPOINTER Data
    )
{
    gceSTATUS status;
    gctUINT32_PTR source;
    gctUINT i;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER(reserve, stateDelta, memory, reserveSize);

    gcmHEADER_ARG("Hardware=0x%x Address=0x%x Count=%d Data=0x%x",
                  Hardware, Address, Count, Data);

    /* Verify the arguments. */
    gcmGETHARDWARE(Hardware);
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Check address range. */
    gcmASSERT(Address + Count < Hardware->maxState);

    /* Cast the pointers. */
    source = (gctUINT32_PTR) Data;

    /* Determine the size of the buffer to reserve. */
    reserveSize = gcmALIGN((1 + Count) * gcmSIZEOF(gctUINT32), 8);

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER(Hardware, reserve, stateDelta, memory, reserveSize);

    {    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve, gctUINT32_PTR)) & 1) == 0);
    gcmASSERT((gctUINT32)Count  <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, Address, Count );
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (FixedPoint) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (Count ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (Address) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};


    for (i = 0; i < Count; i ++)
    {
        gcmSETSTATEDATAWITHMASK(
            stateDelta, reserve, memory, FixedPoint, Address + i, Mask,
            *source++
            );
    }

    if ((Count & 1) == 0)
    {
        gcmSETFILLER(
            reserve, memory
            );
    }

    gcmENDSTATEBATCH(
        reserve, memory
        );

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER(Hardware, reserve, memory, reserveSize);

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gceSTATUS _LoadStatesNEW(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctBOOL FixedPoint,
    IN gctUINT32 Count,
    IN gctUINT32 Mask,
    IN gctPOINTER Data,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32_PTR source;
    gctUINT i;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x Address=0x%x Count=%d Data=0x%x Memory=0x%x",
                  Hardware, Address, Count, Data, Memory);

    /* Verify the arguments. */
    gcmGETHARDWARE(Hardware);
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Check address range. */
    gcmASSERT(Address + Count < Hardware->maxState);

    /* Cast the pointers. */
    source = (gctUINT32_PTR) Data;

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)Count  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (FixedPoint) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (Count ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (Address) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


    for (i = 0; i < Count; i ++)
    {
        gcmSETSTATEDATAWITHMASK_NEW(
            stateDelta, reserve, memory, FixedPoint, Address + i, Mask,
            *source++
            );
    }

    if ((Count & 1) == 0)
    {
        gcmSETFILLER_NEW(
            reserve, memory
            );
    }

    gcmENDSTATEBATCH_NEW(
        reserve, memory
        );

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}


/*
 * Support loading more than 1024 states in a batch.
 */
static gceSTATUS _LoadStatesEx(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctBOOL FixedPoint,
    IN gctUINT32 Count,
    IN gctUINT32 Mask,
    IN gctPOINTER Data
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 i = 0;
    gctUINT32 count;
    gctUINT32 limit;

    gcmHEADER_ARG("Hardware=0x%x, Address=%x Count=%d Data=0x%x",
                  Hardware, Address, Count, Data);

    limit = 2 << (25 - 16);

    while (Count)
    {
        count = gcmMIN(Count, limit);

        gcmONERROR(
            _LoadStates(Hardware,
                        Address + i,
                        FixedPoint,
                        count,
                        Mask,
                        (gctUINT8_PTR)Data + i));

        i += count;
        Count -= count;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gctUINT32 _SetBitWidth(
    gctUINT32 Value,
    gctINT8 CurWidth,
    gctINT8 NewWidth
    )
{
    gctUINT32 result;
    gctINT8 widthDiff;

    /* Mask source bits. */
    Value &= ((gctUINT64) 1 << CurWidth) - 1;

    /* Init the result. */
    result = Value;

    /* Determine the difference in width. */
    widthDiff = NewWidth - CurWidth;

    /* Until the difference is not zero... */
    while (widthDiff)
    {
        /* New value is thiner then current? */
        if (widthDiff < 0)
        {
            result >>= -widthDiff;
            widthDiff = 0;
        }

        /* Full source replication? */
        else if (widthDiff >= CurWidth)
        {
            result = (CurWidth == 32) ? Value
                                      : ((result << CurWidth) | Value);
            widthDiff -= CurWidth;
        }

        /* Partial source replication. */
        else
        {
            result = (result << widthDiff) | (Value >> (CurWidth - widthDiff));
            widthDiff = 0;
        }
    }

    /* Return result. */
    return result;
}

static gceSTATUS
_ConvertComponent(
    gctUINT8* SrcPixel,
    gctUINT8* DstPixel,
    gctUINT SrcBit,
    gctUINT DstBit,
    gcsFORMAT_COMPONENT* SrcComponent,
    gcsFORMAT_COMPONENT* DstComponent,
    gcsBOUNDARY_PTR SrcBoundary,
    gcsBOUNDARY_PTR DstBoundary,
    gctUINT32 Default
    )
{
    gctUINT32 srcValue;
    gctUINT8 srcWidth;
    gctUINT8 dstWidth;
    gctUINT32 dstMask;
    gctUINT32 bits;

    /* Exit if target is beyond the boundary. */
    if ((DstBoundary != gcvNULL) &&
        ((DstBoundary->x < 0) || (DstBoundary->x >= DstBoundary->width) ||
         (DstBoundary->y < 0) || (DstBoundary->y >= DstBoundary->height)))
    {
        return gcvSTATUS_SKIP;
    }

    /* Exit if target component is not present. */
    if (DstComponent->width == gcvCOMPONENT_NOTPRESENT)
    {
        return gcvSTATUS_SKIP;
    }

    /* Extract target width. */
    dstWidth = DstComponent->width & gcvCOMPONENT_WIDTHMASK;

    /* Extract the source. */
    if ((SrcComponent == gcvNULL) ||
        (SrcComponent->width == gcvCOMPONENT_NOTPRESENT) ||
        (SrcComponent->width &  gcvCOMPONENT_DONTCARE)   ||
        ((SrcBoundary != gcvNULL) &&
         ((SrcBoundary->x < 0) || (SrcBoundary->x >= SrcBoundary->width) ||
          (SrcBoundary->y < 0) || (SrcBoundary->y >= SrcBoundary->height))))
    {
        srcValue = Default;
        srcWidth = 32;
    }
    else
    {
        /* Extract source width. */
        srcWidth = SrcComponent->width & gcvCOMPONENT_WIDTHMASK;

        /* Compute source position. */
        SrcBit += SrcComponent->start;
        SrcPixel += SrcBit >> 3;
        SrcBit &= 7;

        /* Compute number of bits to read from source. */
        bits = SrcBit + srcWidth;

        /* Read the value. */
        srcValue = SrcPixel[0] >> SrcBit;

        if (bits > 8)
        {
            /* Read up to 16 bits. */
            srcValue |= SrcPixel[1] << (8 - SrcBit);
        }

        if (bits > 16)
        {
            /* Read up to 24 bits. */
            srcValue |= SrcPixel[2] << (16 - SrcBit);
        }

        if (bits > 24)
        {
            /* Read up to 32 bits. */
            srcValue |= SrcPixel[3] << (24 - SrcBit);
        }
    }

    /* Make the source component the same width as the target. */
    srcValue = _SetBitWidth(srcValue, srcWidth, dstWidth);

    /* Compute destination position. */
    DstBit += DstComponent->start;
    DstPixel += DstBit >> 3;
    DstBit &= 7;

    /* Determine the target mask. */
    dstMask = (gctUINT32) (((gctUINT64) 1 << dstWidth) - 1);
    dstMask <<= DstBit;

    /* Align the source value. */
    srcValue <<= DstBit;

    /* Loop while there are bits to set. */
    while (dstMask != 0)
    {
        /* Set 8 bits of the pixel value. */
        if ((dstMask & 0xFF) == 0xFF)
        {
            /* Set all 8 bits. */
            *DstPixel = (gctUINT8) srcValue;
        }
        else
        {
            /* Set the required bits. */
            *DstPixel = (gctUINT8) ((*DstPixel & ~dstMask) | srcValue);
        }

        /* Next 8 bits. */
        DstPixel ++;
        dstMask  >>= 8;
        srcValue >>= 8;
    }

    return gcvSTATUS_OK;
}
#endif  /* gcdENABLE_3D/2D */

#if gcdENABLE_3D

gceSTATUS
_DisableTileStatus(
    IN gcoHARDWARE Hardware,
    IN gceTILESTATUS_TYPE Type
    )
{
    gceSTATUS status;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER(reserve, stateDelta, memory, reserveSize);

    /* Determine the size of the buffer to reserve. */
    reserveSize = gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32) * 3, 8);

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER(Hardware, reserve, stateDelta, memory, reserveSize);

    if (Type == gcvTILESTATUS_DEPTH)
    {
        /* Flush the depth cache. */
        {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0E03, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
gcmSETCTRLSTATE(stateDelta, reserve, memory, 0x0E03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );gcmENDSTATEBATCH(reserve, memory);
};


        {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0xFFFF, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0xFFFF) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
gcmSETCTRLSTATE(stateDelta, reserve, memory, 0xFFFF, 0);
gcmENDSTATEBATCH(reserve, memory);
};


        /* Disable depth tile status. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        /* Make sure auto-disable is turned off as well. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));

        /* Make sure compression is turned off as well. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));

        /* Make sure hierarchical turned off as well. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 12:12) - (0 ? 12:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)));
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:13) - (0 ? 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ?
 13:13))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 13:13) - (0 ? 13:13) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ? 13:13)));
    }
    else
    {
        /* Flush the color cache. */
        {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0E03, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
gcmSETCTRLSTATE(stateDelta, reserve, memory, 0x0E03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) );gcmENDSTATEBATCH(reserve, memory);
};


        {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0xFFFF, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0xFFFF) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
gcmSETCTRLSTATE(stateDelta, reserve, memory, 0xFFFF, 0);
gcmENDSTATEBATCH(reserve, memory);
};


        /* Disable color tile status. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));

        /* Make sure auto-disable is turned off as well. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)));

        /* Make sure compression is turned off as well. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)));
    }

    /* Program memory configuration register. */
    {    {    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0595, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0595) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA(stateDelta, reserve, memory, gcvFALSE, 0x0595, Hardware->MCStates->memoryConfig );
     gcmENDSTATEBATCH(reserve, memory);
};


    /* Validate the state buffer. */
    gcmENDSTATEBUFFER(Hardware, reserve, memory, reserveSize);

     /* Make sure raster won't start reading Z values until tile status is
     ** actually disabled. */
     gcmONERROR(gcoHARDWARE_Semaphore(
         Hardware, gcvWHERE_RASTER, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE, gcvNULL
         ));


OnError:
    return status;
}
gceSTATUS
_DisableTileStatusMRT(
    IN gcoHARDWARE Hardware,
    IN gceTILESTATUS_TYPE Type,
    IN gctUINT RtIndex
    )
{
    gceSTATUS status;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER(reserve, stateDelta, memory, reserveSize);

    /* Determine the size of the buffer to reserve. */
    reserveSize = gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32) * 3, 8);

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER(Hardware, reserve, stateDelta, memory, reserveSize);

    gcmASSERT (Type == gcvTILESTATUS_COLOR);

    /* Flush the color cache. */
    {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve, gctUINT32_PTR)) & 1) == 0);
    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0E03, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
gcmSETCTRLSTATE(stateDelta, reserve, memory, 0x0E03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) );gcmENDSTATEBATCH(reserve, memory);
};


    {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve, gctUINT32_PTR)) & 1) == 0);
    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0xFFFF, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0xFFFF) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
gcmSETCTRLSTATE(stateDelta, reserve, memory, 0xFFFF, 0);
gcmENDSTATEBATCH(reserve, memory);
};


    /* Disable color tile status. */
    Hardware->MCStates->memoryConfigMRT[RtIndex] = ((((gctUINT32) (Hardware->MCStates->memoryConfigMRT[RtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

    /* Make sure auto-disable is turned off as well. */
    Hardware->MCStates->memoryConfigMRT[RtIndex] = ((((gctUINT32) (Hardware->MCStates->memoryConfigMRT[RtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));

    /* Make sure compression is turned off as well. */
    Hardware->MCStates->memoryConfigMRT[RtIndex] = ((((gctUINT32) (Hardware->MCStates->memoryConfigMRT[RtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)));
    /*
    ** 0x05E8 slot 0 is useless on HW.
    */
    if (RtIndex == 0)
    {
        /* Disable color tile status. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));

        /* Make sure auto-disable is turned off as well. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)));

        /* Make sure compression is turned off as well. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)));
        /* Program memory configuration register. */
        {    {    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0595, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0595) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA(stateDelta, reserve, memory, gcvFALSE, 0x0595, Hardware->MCStates->memoryConfig );
     gcmENDSTATEBATCH(reserve, memory);
};


    }
    else
    {
        /* Program memory configuration register. */
        {    {    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x05E8 + RtIndex, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05E8 + RtIndex) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA(stateDelta, reserve, memory, gcvFALSE, 0x05E8 + RtIndex,
 Hardware->MCStates->memoryConfigMRT[RtIndex] );     gcmENDSTATEBATCH(reserve, memory);
};

    }

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER(Hardware, reserve, memory, reserveSize);

     /* Make sure raster won't start reading Z values until tile status is
     ** actually disabled. */
     gcmONERROR(gcoHARDWARE_Semaphore(
         Hardware, gcvWHERE_RASTER, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE, gcvNULL
         ));


OnError:
    return status;
}

gceSTATUS
_DestroyTempRT(
    IN gcoHARDWARE Hardware)
{
    gcoSURF tmpRT;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    tmpRT = &Hardware->tmpRT;
    /* Is there a surface allocated? */
    if (tmpRT->node.pool != gcvPOOL_UNKNOWN)
    {
        /* Unlock the surface. */
        gcmONERROR(gcoHARDWARE_Unlock(&tmpRT->node, tmpRT->type));

        gcmONERROR(gcsSURF_NODE_Destroy(&tmpRT->node));

        /* Reset the temporary surface. */
        gcoOS_ZeroMemory(&Hardware->tmpRT, sizeof(Hardware->tmpRT));
    }

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
_ResizeTempRT(
    IN gcoHARDWARE Hardware,
    IN gcoSURF depthSurf
    )
{
    gcoSURF tmpRT;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x depthSurf=0x%x", Hardware, depthSurf);

    if (!depthSurf)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    tmpRT = &Hardware->tmpRT;
    if (tmpRT->alignedW < depthSurf->alignedW)
    {
        const gceSURF_FORMAT format = gcvSURF_A8R8G8B8;
        gcsSURF_FORMAT_INFO_PTR formatInfo;

        /* Destroy temp RT if ever allocated */
        gcmONERROR(_DestroyTempRT(Hardware));

        /* Recreate new temp RT */
        gcmONERROR(gcoHARDWARE_QueryFormat(format, &formatInfo));

        tmpRT->requestW   = depthSurf->requestW;
        tmpRT->requestH   = 4;
        tmpRT->requestD   = 1;
        tmpRT->allocedW   = depthSurf->allocedW;
        tmpRT->allocedH   = 4 * 2; /* Consider tiled msaa */
        tmpRT->alignedW   = gcmALIGN_NP2(tmpRT->requestW, 4) * depthSurf->sampleInfo.x;
        tmpRT->alignedH   = 4 * 2;

        tmpRT->sampleInfo      = depthSurf->sampleInfo;
        tmpRT->type            = gcvSURF_RENDER_TARGET;
        tmpRT->format          = format;
        tmpRT->formatInfo      = *formatInfo;
        tmpRT->bitsPerPixel    = formatInfo->bitsPerPixel;
        tmpRT->tiling          = gcvTILED;
        tmpRT->superTiled      = 0;
        tmpRT->colorSpace      = gcd_QUERY_COLOR_SPACE(format);

        tmpRT->stride          = tmpRT->alignedW * tmpRT->bitsPerPixel / 8;
        tmpRT->sliceSize       =
        tmpRT->layerSize       =
        tmpRT->size            = tmpRT->stride * tmpRT->alignedH;

        gcmONERROR(gcsSURF_NODE_Construct(
            &tmpRT->node,
            tmpRT->size,
            256,
            tmpRT->type,
            gcvALLOC_FLAG_NONE,
            gcvPOOL_DEFAULT
            ));

        gcmONERROR(gcoHARDWARE_Lock(&tmpRT->node, gcvNULL, gcvNULL));

        tmpRT->pfGetAddr = gcoHARDWARE_GetProcCalcPixelAddr(Hardware, tmpRT);
    }
    else
    {
        status = gcvSTATUS_CACHED;
    }

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

#endif

#if gcdENABLE_2D
/* Init 2D related members in gcoHARDWARE. */
static gceSTATUS
_Init2D(
    IN gcoHARDWARE Hardware
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT i;

    /***********************************************************************
    ** Determine available features for 2D Hardware.
    */
    /* Determine whether 2D Hardware is present. */
    Hardware->hw2DEngine = Hardware->features[gcvFEATURE_PIPE_2D];

    /* Don't force software by default. */
    Hardware->sw2DEngine = gcvFALSE;
    Hardware->hw3DEngine = gcvFALSE;

    Hardware->hw2DAppendCacheFlush = 0;
    Hardware->hw2DCacheFlushAfterCompress = 0;
    Hardware->hw2DCacheFlushCmd    = gcvNULL;
    Hardware->hw2DCacheFlushSurf   = gcvNULL;

    /* Default enable splitting rectangle.*/
    Hardware->hw2DSplitRect = gcvTRUE;

    /* Determine whether byte write feature is present in the chip. */
    Hardware->byteWrite = Hardware->features[gcvFEATURE_BYTE_WRITE_2D];

    /* MASK register is missing on 4.3.1_rc0. */
    if (Hardware->config->chipRevision == 0x4310)
    {
        Hardware->shadowRotAngleReg = gcvTRUE;
        Hardware->rotAngleRegShadow = 0x00000000;
    }
    else
    {
        Hardware->shadowRotAngleReg = gcvFALSE;
    }

    Hardware->hw2D420L2Cache = Hardware->features[gcvFEATURE_L2_CACHE_FOR_2D_420];

    Hardware->hw2DDoMultiDst = gcvFALSE;

    for (i = 0; i < gcdTEMP_SURFACE_NUMBER; i += 1)
    {
        Hardware->temp2DSurf[i] = gcvNULL;
    }

    /***********************************************************************
    ** Initialize filter blit states.
    */
    Hardware->loadedKernel[gcvFILTER_BLIT_KERNEL_UNIFIED].type = gcvFILTER_SYNC;
    Hardware->loadedKernel[gcvFILTER_BLIT_KERNEL_UNIFIED].kernelSize = 0;
    Hardware->loadedKernel[gcvFILTER_BLIT_KERNEL_UNIFIED].scaleFactor = 0;
    Hardware->loadedKernel[gcvFILTER_BLIT_KERNEL_UNIFIED].kernelAddress = 0x01800;

    Hardware->loadedKernel[gcvFILTER_BLIT_KERNEL_VERTICAL].type = gcvFILTER_SYNC;
    Hardware->loadedKernel[gcvFILTER_BLIT_KERNEL_VERTICAL].kernelSize = 0;
    Hardware->loadedKernel[gcvFILTER_BLIT_KERNEL_VERTICAL].scaleFactor = 0;
    Hardware->loadedKernel[gcvFILTER_BLIT_KERNEL_VERTICAL].kernelAddress = 0x02A00;

    Hardware->loadedKernel[gcvFILTER_BLIT_KERNEL_HORIZONTAL].type = gcvFILTER_SYNC;
    Hardware->loadedKernel[gcvFILTER_BLIT_KERNEL_HORIZONTAL].kernelSize = 0;
    Hardware->loadedKernel[gcvFILTER_BLIT_KERNEL_HORIZONTAL].scaleFactor = 0;
    Hardware->loadedKernel[gcvFILTER_BLIT_KERNEL_HORIZONTAL].kernelAddress = 0x02800;

    if (Hardware->config->chipModel == gcv320 && Hardware->config->chipRevision == 0x5007)
    {
        /* Check to enable 2D Flush. */
        Hardware->hw2DAppendCacheFlush = gcvTRUE;
    }
    else
    {
        /* Check to enable splitting rectangle. */
        Hardware->hw2DSplitRect = gcvTRUE;
    }

    /* gcvFEATURE_2D_COMPRESSION */
    if (Hardware->features[gcvFEATURE_2D_COMPRESSION])
    {
        gctUINT address;

        const gctUINT32 _2DCacheFlushCmd[] =
        {
            0x08010483, 0x00000000,
            0x080104B0, 0x00000000,
            0x080104C9, 0x00030003,
            0x080104AD, 0x00000000,
            0x0804048A, 0x00000000,
            0x00000020, 0x00000010,
            0xA0000004, 0x00000000,
            0x08030497, 0x0030CCCC,
            0x00000000, 0x00100010,
            0x0801049F, 0x00000000,
            0x080104AF, 0x00000CC0,
            0x080104CA, 0xFFFFFFCF,
            0x080104CB, 0xFFF0FFFF,
            0x080204B4, 0x00000000,
            0x00000000, 0x00000000,
            0x20000100, 0x00000000,
            0x00000000, 0x00040004,
            0x08010E03, 0x00000008,
        };

        gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(_2DCacheFlushCmd), (gctPOINTER*)&Hardware->hw2DCacheFlushCmd));
        gcoOS_MemCopy(Hardware->hw2DCacheFlushCmd, _2DCacheFlushCmd, gcmSIZEOF(_2DCacheFlushCmd));
        Hardware->hw2DCacheFlushAfterCompress = gcmCOUNTOF(_2DCacheFlushCmd);

        gcmONERROR(gcoHARDWARE_Alloc2DSurface(Hardware,
            16, 16, gcvSURF_R5G6B5, gcvSURF_TYPE_UNKNOWN, &Hardware->hw2DCacheFlushSurf));

        gcmGETHARDWAREADDRESS(Hardware->hw2DCacheFlushSurf->node, address);

        for (i = 0; i < gcmCOUNTOF(_2DCacheFlushCmd); ++i)
        {
            if (Hardware->hw2DCacheFlushCmd[i] == 0x0804048A)
            {
                Hardware->hw2DCacheFlushCmd[i + 1] = address;
            }
        }
    }

OnError:
    return status;
}

static void
Fill2DFeaturesByDatabase(
    IN gcoHARDWARE  Hardware,
    OUT gctBOOL *    Features
    )
{
    gceCHIPMODEL    chipModel          = Hardware->config->chipModel;
    gctUINT32       chipRevision       = Hardware->config->chipRevision;

    gcsFEATURE_DATABASE * database     = Hardware->featureDatabase;

    /* 2D hardware Pixel Engine 2.0 availability flag. */
    gctBOOL hw2DPE20 = database->REG_2DPE20;

    /* Support one pass filter blit and tiled/YUV input&output for Blit/StretchBlit. */
    gctBOOL hw2DOPF = database->REG_OnePass2DFilter;

    /* Support 9tap one pass filter blit. */
    gctBOOL hw2DOPFTAP = database->REG_OPF9Tap;

    gctBOOL hw2DSuperTile = database->REG_DESupertile;

    /* Support Multi-source blit (4 src). */
    gctBOOL hw2DMultiSrcBlit = database->REG_MultiSourceBlt;

    /* This feature including 8 multi source, 2x2 minor tile, U/V separate stride,
        AXI reorder, RGB non-8-pixel alignement. */
    gctBOOL hw2DNewFeature0 =database->REG_NewFeatures0;

    gctBOOL hw2DDEEnhance1 = database->REG_DEEnhancements1;

    gctBOOL hw2DDEEnhance5 = database->REG_DEEnhancements5;

    gctBOOL hw2DCompression = database->REG_Compression2D;

    /*************************************************************************/
    /* 2D Features.                                                          */
    Features[gcvFEATURE_PIPE_2D] = database->REG_Pipe2D;

    if (((chipModel == gcv500) && (chipRevision <= 2))
    ||   (chipModel == gcv300)
    )
    {
        Features[gcvFEATURE_PIPE_2D] = gcvTRUE;
    }

    Features[gcvFEATURE_2DPE20] = hw2DPE20;

    /* Determine whether we support full DFB features. */
    Features[gcvFEATURE_FULL_DIRECTFB] = database->REG_FullDirectFB;
    Features[gcvFEATURE_YUV420_SCALER] = database->REG_YUV420Filter;

    Features[gcvFEATURE_2D_TILING]   = hw2DOPF || hw2DSuperTile;
    Features[gcvFEATURE_2D_ONE_PASS_FILTER] =
    Features[gcvFEATURE_2D_YUV_BLIT] = hw2DOPF;
    Features[gcvFEATURE_2D_ONE_PASS_FILTER_TAP] = hw2DOPFTAP;

    Features[gcvFEATURE_2D_MULTI_SOURCE_BLT] = hw2DMultiSrcBlit;

    Features[gcvFEATURE_2D_MULTI_SOURCE_BLT_EX] = hw2DNewFeature0;
    Features[gcvFEATURE_2D_PIXEL_ALIGNMENT] =
    Features[gcvFEATURE_2D_MINOR_TILING] =
    Features[gcvFEATURE_2D_YUV_SEPARATE_STRIDE] = hw2DNewFeature0;

    Features[gcvFEATURE_2D_FILTERBLIT_PLUS_ALPHABLEND] =
    Features[gcvFEATURE_2D_DITHER] = database->REG_DitherAndFilterPlusAlpha2D;

    Features[gcvFEATURE_2D_A8_TARGET] =
        database->REG_A8TargetSupport
     || database->REG_YUVConversion;

    /* Determine whether full rotation is present in the chip. */
    Features[gcvFEATURE_2D_FILTERBLIT_FULLROTATION] =
    Features[gcvFEATURE_2D_BITBLIT_FULLROTATION] = hw2DPE20;
    if (chipRevision < 0x4310)
    {
        Features[gcvFEATURE_2D_BITBLIT_FULLROTATION]    = gcvFALSE;
        Features[gcvFEATURE_2D_FILTERBLIT_FULLROTATION] = gcvFALSE;
    }
    else if (chipRevision == 0x4310)
    {
        Features[gcvFEATURE_2D_BITBLIT_FULLROTATION]    = gcvTRUE;
        Features[gcvFEATURE_2D_FILTERBLIT_FULLROTATION] = gcvFALSE;
    }
    else if (chipRevision >= 0x4400)
    {
        Features[gcvFEATURE_2D_BITBLIT_FULLROTATION]    = gcvTRUE;
        Features[gcvFEATURE_2D_FILTERBLIT_FULLROTATION] = gcvTRUE;
    }

    Features[gcvFEATURE_2D_NO_COLORBRUSH_INDEX8] = database->REG_NoIndexPattern;

    Features[gcvFEATURE_2D_FC_SOURCE] = database->REG_DEEnhancements2;

    if (chipModel > gcv520 && Features[gcvFEATURE_2D_FC_SOURCE])
    {
        Features[gcvFEATURE_2D_CC_NOAA_SOURCE] = gcvTRUE;
    }

    Features[gcvFEATURE_2D_GAMMA]= database->REG_DEEnhancements4;

    Features[gcvFEATURE_2D_COLOR_SPACE_CONVERSION] = hw2DDEEnhance5;

    Features[gcvFEATURE_2D_SUPER_TILE_VERSION] = hw2DDEEnhance5 || hw2DSuperTile;

    Features[gcvFEATURE_2D_MIRROR_EXTENSION] =
        database->REG_New2D
     || database->REG_DEMirrorRotate;

    if (hw2DDEEnhance5 || hw2DSuperTile)
    {
        Features[gcvFEATURE_2D_SUPER_TILE_V1] =
        Features[gcvFEATURE_2D_SUPER_TILE_V2] =
        Features[gcvFEATURE_2D_SUPER_TILE_V3] = gcvTRUE;
    }
    else if (hw2DOPF)
    {
        if (hw2DDEEnhance1
            || ((chipModel == gcv320)
            && (chipRevision >= 0x5303)))
        {
            Features[gcvFEATURE_2D_SUPER_TILE_V1] = gcvFALSE;
            Features[gcvFEATURE_2D_SUPER_TILE_V2] = gcvFALSE;
            Features[gcvFEATURE_2D_SUPER_TILE_V3] = gcvTRUE;
        }
        else
        {
            Features[gcvFEATURE_2D_SUPER_TILE_V1] = gcvFALSE;
            Features[gcvFEATURE_2D_SUPER_TILE_V2] = gcvTRUE;
            Features[gcvFEATURE_2D_SUPER_TILE_V3] = gcvFALSE;
        }
    }
    else
    {
        Features[gcvFEATURE_2D_SUPER_TILE_V1] =
        Features[gcvFEATURE_2D_SUPER_TILE_V2] =
        Features[gcvFEATURE_2D_SUPER_TILE_V3] = gcvFALSE;
    }

    Features[gcvFEATURE_2D_POST_FLIP] =
    Features[gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2] = database->REG_MultiSrcV2;

    /* Rotation stall fix. */
    Features[gcvFEATURE_2D_ROTATION_STALL_FIX] = database->REG_DERotationStallFix;

    Features[gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT] = database->REG_MultiSrcV15
        || hw2DCompression;

    Features[gcvFEATURE_2D_COMPRESSION] = hw2DCompression;

#if gcdENABLE_THIRD_PARTY_OPERATION
    Features[gcvFEATURE_TPC_COMPRESSION] = database->REG_ThirdPartyCompression;
#elif gcdENABLE_DEC_COMPRESSION
    Features[gcvFEATURE_DEC_COMPRESSION] = database->REG_DEC;

    Features[gcvFEATURE_DEC_COMPRESSION_TILE_NV12_8BIT] = database->REG_VSTileNV12;

    Features[gcvFEATURE_DEC_COMPRESSION_TILE_NV12_10BIT] = database->REG_VSTileNV1210Bit;

    Features[gcvFEATURE_DEC_TPC_COMPRESSION] =
        Features[gcvFEATURE_DEC_COMPRESSION_TILE_NV12_8BIT] |
        Features[gcvFEATURE_DEC_COMPRESSION_TILE_NV12_10BIT];

    Features[gcvFEATURE_2D_FC_SOURCE] |= Features[gcvFEATURE_DEC_COMPRESSION];
#endif

    Features[gcvFEATURE_2D_OPF_YUV_OUTPUT] = database->REG_DualPipeOPF;

    Features[gcvFEATURE_2D_YUV_MODE] = database->REG_YUVStandard;

    Hardware->hw2DQuad =
    Hardware->hw2DBlockSize = hw2DDEEnhance1 || hw2DCompression;

    Features[gcvFEATURE_2D_FILTERBLIT_A8_ALPHA] = hw2DDEEnhance1;

    Features[gcvFEATURE_2D_ALL_QUAD] = Hardware->hw2DQuad;

    Features[gcvFEATURE_SEPARATE_SRC_DST] = database->REG_SeperateSRCAndDstCache;

    Features[gcvFEATURE_NO_USER_CSC] = database->REG_CSCV2;

    Features[gcvFEATURE_ANDROID_ONLY] = database->REG_AndroidOnly;

    Features[gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT] &= !Features[gcvFEATURE_ANDROID_ONLY];

    Features[gcvFEATURE_2D_MULTI_SRC_BLT_BILINEAR_FILTER] = database->MULTI_SRC_BLT_BILINEAR_FILTER;

    Features[gcvFEATURE_2D_CACHE_128B256BPERLINE] = database->CACHE128B256BPERLINE;
    Features[gcvFEATURE_2D_MAJOR_SUPER_TILE] = database->PE2D_MAJOR_SUPER_TILE;
    Features[gcvFEATURE_2D_V4COMPRESSION] = database->V4Compression;
}
#endif

#if gcdENABLE_2D || gcdENABLE_3D
/*******************************************************************************
**
**  _FillInFeatureTable
**
**  Verifies whether the specified feature is available in hardware.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to Hardware object.
**
**      gctBOOL * Features
**          Features to be filled.
*/
static gceSTATUS
_FillInFeatureTableByDatabase(
    IN gcoHARDWARE  Hardware,
    IN gctBOOL *    Features
    )
{
    gceCHIPMODEL    chipModel      = Hardware->config->chipModel;
    gctUINT32       chipRevision   = Hardware->config->chipRevision;

    gcsFEATURE_DATABASE * database = Hardware->featureDatabase;

    gctBOOL         useHZ = gcvFALSE;

    gcmHEADER_ARG("Hardware=0x%x Features=0x%x", Hardware, Features);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

#if gcdENABLE_3D
    /* 3D and VG features. */
    Features[gcvFEATURE_PIPE_3D] = database->REG_Pipe3D;

    Features[gcvFEATURE_PIPE_VG] = database->REG_PipeVG;

    Features[gcvFEATURE_DC] = database->REG_DC;

    Features[gcvFEATURE_HIGH_DYNAMIC_RANGE] = database->REG_HighDynamicRange;

    Features[gcvFEATURE_MODULE_CG] = database->REG_ModuleCG;
    Features[gcvFEATURE_MIN_AREA] = database->REG_MinArea;

    Features[gcvFEATURE_BUFFER_INTERLEAVING] = database->REG_BufferInterleaving;

    Features[gcvFEATURE_BYTE_WRITE_2D] = Hardware->byteWrite;

    Features[gcvFEATURE_ENDIANNESS_CONFIG] = database->REG_EndiannessConfig;

    Features[gcvFEATURE_DUAL_RETURN_BUS] = database->REG_DualReturnBus;

    Features[gcvFEATURE_DEBUG_MODE] = database->REG_DebugMode;

    Features[gcvFEATURE_FAST_CLEAR] = database->REG_FastClear;

    if (chipModel == gcv700)
    {
        Features[gcvFEATURE_FAST_CLEAR] = gcvFALSE;
    }

    Features[gcvFEATURE_YUV420_TILER] = database->REG_YUV420Tiler;

    Features[gcvFEATURE_YUY2_AVERAGING] = database->REG_YUY2Averaging;

    Features[gcvFEATURE_FLIP_Y] = database->REG_FlipY;

    Features[gcvFEATURE_EARLY_Z] = database->REG_NoEZ == 0;

    Features[gcvFEATURE_COMPRESSION] = database->REG_ZCompression;

    Features[gcvFEATURE_MSAA] = database->REG_MSAA;

    Features[gcvFEATURE_SPECIAL_ANTI_ALIASING] = database->REG_SpecialAntiAliasing;

    Features[gcvFEATURE_SPECIAL_MSAA_LOD] = database->REG_SpecialMsaaLod;

    Features[gcvFEATURE_422_TEXTURE_COMPRESSION] = database->REG_No422Texture == 0;

    Features[gcvFEATURE_DXT_TEXTURE_COMPRESSION] = database->REG_DXTTextureCompression;

    Features[gcvFEATURE_ETC1_TEXTURE_COMPRESSION] = database->REG_ETC1TextureCompression;

    Features[gcvFEATURE_CORRECT_TEXTURE_CONVERTER] = database->REG_CorrectTextureConverter;

    Features[gcvFEATURE_TEXTURE_8K] = database->REG_Texture8K;

    Features[gcvFEATURE_SHADER_HAS_W] = database->REG_ShaderGetsW;

    Features[gcvFEATURE_HZ] = database->REG_HierarchicalZ;

    Features[gcvFEATURE_CORRECT_STENCIL] = database->REG_CorrectStencil;

    Features[gcvFEATURE_MC20] = database->REG_MC20;

    if (Hardware->patchID == gcvPATCH_DUOKAN)
    {
        /*Disable super-tile */
        Features[gcvFEATURE_SUPER_TILED] = gcvFALSE;
    }
    else
    {
         Features[gcvFEATURE_SUPER_TILED] = database->REG_SuperTiled32x32;
    }

    Features[gcvFEATURE_FAST_CLEAR_FLUSH] = database->REG_FastClearFlush;

    Features[gcvFEATURE_WIDE_LINE] = database->REG_WideLine;

    Features[gcvFEATURE_FC_FLUSH_STALL] = database->REG_FcFlushStall;

    if ((chipModel == gcv2000) && (chipRevision == 0x5118 || chipRevision == 0x5140))
    {
         Features[gcvFEATURE_FC_FLUSH_STALL] = gcvTRUE;
    }

    Features[gcvFEATURE_TEXTURE_FLOAT_HALF_FLOAT] =
    Features[gcvFEATURE_HALF_FLOAT_PIPE] = database->REG_HalfFloatPipe;

    Features[gcvFEATURE_NON_POWER_OF_TWO] = database->REG_NonPowerOfTwo;

    Features[gcvFEATURE_HALTI0] = database->REG_Halti0;

    if (Features[gcvFEATURE_HALTI0])
    {
        Features[gcvFEATURE_VERTEX_10_10_10_2] = gcvTRUE;
        Features[gcvFEATURE_TEXTURE_ANISOTROPIC_FILTERING] = !database->NO_ANISTRO_FILTER;
        Features[gcvFEATURE_TEXTURE_10_10_10_2] = gcvTRUE;
        Features[gcvFEATURE_3D_TEXTURE] = gcvTRUE;
        Features[gcvFEATURE_TEXTURE_ARRAY] = gcvTRUE;
        Features[gcvFEATURE_TEXTURE_SWIZZLE] = gcvTRUE;
        Features[gcvFEATURE_OCCLUSION_QUERY] = gcvTRUE;
    }

    Features[gcvFEATURE_HALTI1] = database->REG_Halti1;

    if (Features[gcvFEATURE_HALTI1] ||
        ((Hardware->config->chipModel == gcv880) && ((Hardware->config->chipRevision & 0xfff0) == 0x5120)))
    {
        Features[gcvFEATURE_SUPPORT_GCREGTX] = gcvTRUE;
    }

    Features[gcvFEATURE_PRIMITIVE_RESTART] =
    Features[gcvFEATURE_BUG_FIXED_IMPLICIT_PRIMITIVE_RESTART] =
    Features[gcvFEATURE_HALTI2] = database->REG_Halti2;

    Features[gcvFEATURE_LINE_LOOP] = database->REG_LineLoop;

    Features[gcvFEATURE_TILE_FILLER] = database->REG_TileFiller;

    Features[gcvFEATURE_LOGIC_OP] = database->REG_LogicOp;

    Features[gcvFEATURE_COMPOSITION] = database->REG_Composition;

    Features[gcvFEATURE_MIXED_STREAMS] = database->REG_MixedStreams;

    Features[gcvFEATURE_TEX_COMPRRESSION_SUPERTILED] = database->REG_TexCompressionSupertiled;

    if ((chipModel == gcv1000) && (chipRevision == 0x5036))
    {
        Features[gcvFEATURE_TEX_COMPRRESSION_SUPERTILED] = gcvFALSE;
    }

    Features[gcvFEATURE_TEXTURE_TILE_STATUS_READ] = database->REG_TextureTileStatus
                                                | database->REG_Composition;

    if (((chipModel == gcv2000)
         && ((chipRevision == 0x5020)
            || (chipRevision == 0x5026)
            || (chipRevision == 0x5113)
            || (chipRevision == 0x5108)))

       || ((chipModel == gcv2100)
           && ((chipRevision == 0x5108)
              || (chipRevision == 0x5113)))

       || ((chipModel == gcv4000)
           && ((chipRevision == 0x5222)
              || (chipRevision == 0x5208)))
       )
    {
        Features[gcvFEATURE_DEPTH_BIAS_FIX] = gcvFALSE;
    }
    else
    {
        Features[gcvFEATURE_DEPTH_BIAS_FIX] = gcvTRUE;
    }

    Features[gcvFEATURE_RECT_PRIMITIVE] = database->REG_RectPrimitive;

    if ((chipModel == gcv880) && (chipRevision == 0x5106))
    {
        Features[gcvFEATURE_RECT_PRIMITIVE] = gcvFALSE;
    }

    Features[gcvFEATURE_SUPERTILED_TEXTURE] = database->REG_SuperTiledTexture;

    Features[gcvFEATURE_RS_YUV_TARGET] = database->REG_RsYuvTarget;

    /* We dont have a specific feature bit for this,
       there is a disable index cache feature in bug fix 8,
       but it's best also to check the chip revisions for this. */
    if (database->REG_BugFixes8)
    {
        Features[gcvFEATURE_BUG_FIXED_INDEXED_TRIANGLE_STRIP] = gcvTRUE;
    }
    else if ((chipModel == gcv880 && chipRevision >= 0x5100 && chipRevision < 0x5130)
    || (chipModel == gcv1000 && chipRevision >= 0x5100 && chipRevision < 0x5130)
    || (chipModel == gcv2000 && chipRevision >= 0x5100 && chipRevision < 0x5130)
    || (chipModel == gcv4000 && chipRevision >= 0x5200 && chipRevision < 0x5220))
    {
        Features[gcvFEATURE_BUG_FIXED_INDEXED_TRIANGLE_STRIP] = gcvFALSE;
    }
    else
    {
        Features[gcvFEATURE_BUG_FIXED_INDEXED_TRIANGLE_STRIP] = gcvTRUE;
    }

    Features[gcvFEATURE_PE_DITHER_FIX] = database->REG_BugFixes15;

    if (Hardware->specialHint & (1 << 2))
    {
        Features[gcvFEATURE_PE_DITHER_FIX] = gcvTRUE;
    }

    Features[gcvFEATURE_FRUSTUM_CLIP_FIX] = database->REG_PAEnhancements1;

    Features[gcvFEATURE_TEXTURE_LINEAR] = database->REG_LinearTextureSupport;

    Features[gcvFEATURE_TEXTURE_YUV_ASSEMBLER] = database->REG_TX_YUVAssembler;

    Features[gcvFEATURE_LINEAR_RENDER_TARGET] = database->REG_LinearPE;

    Features[gcvFEATURE_SHADER_HAS_ATOMIC] = database->REG_SHEnhancements1;

    Features[gcvFEATURE_SHADER_ENHANCEMENTS2] =
    Features[gcvFEATURE_SHADER_HAS_RTNE] = database->REG_SHEnhancements2;

    Features[gcvFEATURE_SHADER_HAS_INSTRUCTION_CACHE] = database->REG_InstructionCache;

    Features[gcvFEATURE_SHADER_ENHANCEMENTS3] = database->REG_SHEnhancements3;

    Features[gcvFEATURE_BUG_FIXES7] = database->REG_BugFixes7;

    Features[gcvFEATURE_SHADER_HAS_EXTRA_INSTRUCTIONS2] = database->REG_ExtraShaderInstructions2;

    Features[gcvFEATURE_FAST_MSAA] = database->REG_FastMSAA;

    Features[gcvFEATURE_SMALL_MSAA] = database->REG_SmallMSAA;

    Features[gcvFEATURE_SINGLE_BUFFER] = database->REG_PEEnhancements3;

    /* Load/Store L1 cache hang fix. */
    Features[gcvFEATURE_BUG_FIXES10] = database->REG_BugFixes10;

    Features[gcvFEATURE_BUG_FIXES11] = database->REG_BugFixes11;

    Features[gcvFEATURE_TEXTURE_ASTC] = database->REG_TXEnhancements4 && !database->NO_ASTC;

    Features[gcvFEATURE_TX_DXT] = !database->NO_DXT;
#endif


    Features[gcvFEATURE_YUY2_RENDER_TARGET] = database->REG_YUY2RenderTarget;

    Features[gcvFEATURE_FRAGMENT_PROCESSOR] = gcvFALSE;


    /* Filter Blit. */
    Features[gcvFEATURE_SCALER] = database->REG_NoScaler == 0;

    Features[gcvFEATURE_NEW_RA] = database->REG_Rasterizer2;
    if (Features[gcvFEATURE_HALTI2])
    {
        Features[gcvFEATURE_32BPP_COMPONENT_TEXTURE_CHANNEL_SWIZZLE] = gcvTRUE;
        Features[gcvFEATURE_64BPP_HW_CLEAR_SUPPORT] = gcvTRUE;
        Features[gcvFEATURE_PE_MULTI_RT_BLEND_ENABLE_CONTROL] = gcvTRUE;
        Features[gcvFEATURE_VERTEX_INST_ID_AS_ATTRIBUTE] = gcvTRUE;
        Features[gcvFEATURE_VERTEX_INST_ID_AS_INTEGER] = gcvTRUE;
        Features[gcvFEATURE_DUAL_16] = gcvTRUE;
        Features[gcvFEATURE_BRANCH_ON_IMMEDIATE_REG] = gcvTRUE;
        Features[gcvFEATURE_V2_COMPRESSION_Z16_FIX] = gcvTRUE;
        Features[gcvFEATURE_MRT_TILE_STATUS_BUFFER] = gcvTRUE;
        Features[gcvFEATURE_STENCIL_TEXTURE] = gcvTRUE;
        Features[gcvFEATURE_FE_START_VERTEX_SUPPORT] = gcvTRUE;
        Features[gcvFEATURE_PE_DISABLE_COLOR_PIPE] = gcvTRUE;
        Features[gcvFEATURE_TEX_BASELOD] = gcvTRUE;
        Features[gcvFEATURE_TEX_SEAMLESS_CUBE] = gcvTRUE;
        Features[gcvFEATURE_TEX_ETC2] = gcvTRUE;
    }

    if (chipModel == gcv900 && chipRevision == 0x5250)
    {
        Features[gcvFEATURE_TEX_BASELOD] = gcvTRUE;
        Features[gcvFEATURE_TEX_SEAMLESS_CUBE] = gcvTRUE;
        Features[gcvFEATURE_TEX_ETC2] = gcvTRUE;
        Features[gcvFEATURE_FE_START_VERTEX_SUPPORT] = gcvTRUE;
        Features[gcvFEATURE_VERTEX_INST_ID_AS_ATTRIBUTE] = gcvTRUE;
        Features[gcvFEATURE_TEX_CUBE_BORDER_LOD] = gcvTRUE;
        Features[gcvFEATURE_PE_B2B_PIXEL_FIX] = gcvTRUE;
    }

    Features[gcvFEATURE_TX_LERP_PRECISION_FIX] = database->REG_BugFixes16;
    Features[gcvFEATURE_COMPRESSION_V2] = database->REG_V2Compression;

    Features[gcvFEATURE_COMPRESSION_V3] = gcvFALSE;

    Features[gcvFEATURE_COMPRESSION_V4] = database->V4Compression;

    Features[gcvFEATURE_128BTILE] = database->CACHE128B256BPERLINE;

    Features[gcvFEATURE_32F_COLORMASK_FIX] = database->PE_32BPC_COLORMASK_FIX;

    Features[gcvFEATURE_COLOR_COMPRESSION] = gcvFALSE;

    Features[gcvFEATURE_ADVANCED_BLEND_OPT] = database->ALPHA_BLENDING_OPT;

    Features[gcvFEATURE_SH_SUPPORT_V4] = database->SH_SUPPORT_V4;

    Features[gcvFEATURE_SH_SUPPORT_ALPHA_KILL] = database->SH_SUPPORT_ALPHA_KILL;

    Features[gcvFEATURE_VMSAA] = database->VMSAA;

    if (Features[gcvFEATURE_128BTILE])
    {
        Features[gcvFEATURE_COLOR_COMPRESSION] = gcvTRUE;
    }

    if (!Features[gcvFEATURE_COMPRESSION_V3] &&
        !Features[gcvFEATURE_COMPRESSION_V2] &&
        !Features[gcvFEATURE_COMPRESSION_V4] &&
        Features[gcvFEATURE_COMPRESSION])
    {
        Features[gcvFEATURE_COMPRESSION_V1] = gcvTRUE;
    }

    Features[gcvFEATURE_MMU] = database->REG_MMU;

    Features[gcvFEATURE_V1_COMPRESSION_Z16_DECOMPRESS_FIX] = database->REG_DecompressZ16;

    Features[gcvFEATURE_TX_HOR_ALIGN_SEL] = database->REG_TextureHorizontalAlignmentSelect;

    /* TODO: replace RTT check with surface check later rather than feature bit.
    */
    if (Hardware->config->resolvePipes == 1)
    {
        if (Features[gcvFEATURE_TX_HOR_ALIGN_SEL])
        {
            Features[gcvFEATURE_RTT] = gcvTRUE;
        }

    }
    else
    {
        /*
        ** gcvFEATURE_TEXTURE_TILE_STATUS_READ for HISI special case.
        */
        if ((Features[gcvFEATURE_SINGLE_BUFFER]) ||
            (Features[gcvFEATURE_TEXTURE_TILE_STATUS_READ]))
        {
            Features[gcvFEATURE_RTT] = gcvTRUE;
        }
    }

    Features[gcvFEATURE_GENERIC_ATTRIB] = database->REG_Generics;

    Features[gcvFEATURE_CORRECT_AUTO_DISABLE_COUNT] = database->REG_CorrectAutoDisable1;

    Features[gcvFEATURE_CORRECT_AUTO_DISABLE_COUNT_WIDTH] = database->REG_CorrectAutoDisableCountWidth;

    Features[gcvFEATURE_8K_RT] = database->REG_Render8K;

    Features[gcvFEATURE_HALTI3] = database->REG_Halti3;

    /* VIV:
    ** This feature (TRUE) means EEZ is there, but FALSE doesn't mean EEZ is not there.
    ** We ONLY can use TRUE.
    */
    Features[gcvFEATURE_EEZ] = database->REG_EEZ;

    if (database->REG_BugFixes22 &&
        database->REG_PAEnhancements3)
    {
        Features[gcvFEATURE_PA_FARZCLIPPING_FIX] = gcvTRUE;
        Features[gcvFEATURE_ZSCALE_FIX] = gcvTRUE;
    }

    if (Features[gcvFEATURE_HALTI3])
    {
        Features[gcvFEATURE_INTEGER_SIGNEXT_FIX] = gcvTRUE;
        Features[gcvFEATURE_PSOUTPUT_MAPPING] = gcvTRUE;
        Features[gcvFEATURE_8K_RT_FIX] = gcvTRUE;
        Features[gcvFEATURE_TX_TILE_STATUS_MAPPING] = gcvTRUE;
        Features[gcvFEATURE_SRGB_RT_SUPPORT] = gcvTRUE;
        Features[gcvFEATURE_TEXTURE_16K] = gcvTRUE;
        Features[gcvFEATURE_PE_DITHER_COLORMASK_FIX] = gcvTRUE;
        Features[gcvFEATURE_TX_LOD_GUARDBAND] = gcvTRUE;
        Features[gcvFEATURE_IMG_INSTRUCTION] = gcvTRUE;
    }
    Features[gcvFEATURE_SH_INSTRUCTION_PREFETCH] = database->SH_ICACHE_PREFETCH;

    Features[gcvFEATURE_PROBE] = database->REG_Probe;

    Features[gcvFEATURE_MULTI_PIXELPIPES] = (Hardware->config->pixelPipes > 1);

    Features[gcvFEATURE_PIPE_CL] = ((chipModel != gcv880) && database->REG_Halti0);

    Features[gcvFEATURE_BUG_FIXES18] = database->REG_BugFixes18;

    Features[gcvFEATURE_BUG_FIXES8] = database->REG_BugFixes8;

    Features[gcvFEATURE_UNIFIED_SAMPLERS] = database->REG_UnifiedSamplers;

    Features[gcvFEATURE_CL_PS_WALKER] = database->REG_ThreadWalkerInPS;

    Features[gcvFEATURE_NEW_HZ] = database->REG_NewHZ;

    /* Need original value of gcvFEATURE_NEW_HZ, gcvFEATURE_FAST_MAAA
     * and gcvFEATURE_EARLY_Z, don't change them before here.  */
    if (((chipModel == gcv4000) && (chipRevision == 0x5245))
     || ((chipModel == gcv5000) && (chipRevision == 0x5309))
     || ((chipModel == gcv2500) && (chipRevision == 0x5410))
     || ((chipModel == gcv5200) && (chipRevision == 0x5410))
     || ((chipModel == gcv6400) && (chipRevision == 0x5420))
     || ((chipModel == gcv6400) && (chipRevision == 0x5422))
     || ((chipModel == gcv5000) && (chipRevision == 0x5434))
       )
    {
        useHZ = Features[gcvFEATURE_NEW_HZ] || Features[gcvFEATURE_FAST_MSAA];
    }

    /*
    ** Make sure we don't use HZ if EEZ feature bit is there.
    */
    if (Features[gcvFEATURE_EEZ])
    {
        useHZ = gcvFALSE;
    }

    /* If new HZ is available, disable other early z modes. */
    if (useHZ)
    {
        /* Disable EZ. */
        Features[gcvFEATURE_EARLY_Z] = gcvFALSE;
    }

    /* Disable HZ when EZ is present for older chips. */
    else if (Features[gcvFEATURE_EARLY_Z])
    {
        /* Disable HIERARCHICAL_Z. */
        Features[gcvFEATURE_HZ] = gcvFALSE;
    }

    if (chipModel == gcv320 && chipRevision <= 0x5340)
    {
        Features[gcvFEATURE_2D_A8_NO_ALPHA] = gcvTRUE;
    }

#if gcdENABLE_3D
#if !VIV_STAT_ENABLE_STATISTICS
    if ((chipModel == gcv2000) && (chipRevision == 0x5108) && (Hardware->patchID == gcvPATCH_QUADRANT))
    {
        Features[gcvFEATURE_HZ] = gcvFALSE;
    }
#endif

#if defined(ANDROID)
    if (Hardware->patchID == gcvPATCH_FISHNOODLE)
    {
        Features[gcvFEATURE_HZ] = gcvFALSE;
    }
#endif

#if !VIV_STAT_ENABLE_STATISTICS
    /* Disable EZ for gc880. */
    if ((chipModel == gcv880) && (chipRevision == 0x5106) && (Hardware->patchID == gcvPATCH_QUADRANT))
    {
        Features[gcvFEATURE_EARLY_Z] = gcvFALSE;
    }
#endif
#endif

    /* single pipe with halti1 feature set (gc1500)*/
    if (Features[gcvFEATURE_HALTI1] && (Hardware->config->pixelPipes == 1))
    {
        Features[gcvFEATURE_SINGLE_PIPE_HALTI1] = gcvTRUE;
    }

    Features[gcvFEATURE_HAS_PRODUCTID] = database->REG_HasChipProductReg;


    if ((chipModel == gcv420) && (chipRevision == 0x5337))
    {
         Features[gcvFEATURE_BLOCK_SIZE_16x16] = gcvTRUE;
    }

    Features[gcvFEATURE_HALTI4] = database->REG_Halti4;

    if (Features[gcvFEATURE_HALTI4])
    {
        Features[gcvFEATURE_DRAW_INDIRECT] = gcvTRUE;
        Features[gcvFEATURE_COMPUTE_INDIRECT] = gcvTRUE;
        Features[gcvFEATURE_D24S8_SAMPLE_STENCIL] = gcvTRUE;
        Features[gcvFEATURE_MSAA_TEXTURE] = gcvTRUE;
        Features[gcvFEATURE_ADVANCED_BLEND_MODE_PART0] = gcvTRUE;
        Features[gcvFEATURE_MSAA_FRAGMENT_OPERATION] = gcvTRUE;
        Features[gcvFEATURE_ZERO_ATTRIB_SUPPORT] = gcvTRUE;
        Features[gcvFEATURE_DIVISOR_STREAM_ADDR_FIX] = gcvTRUE;
        Features[gcvFEATURE_FE_12bit_stride] = gcvTRUE;
        Features[gcvFEATURE_INTEGER32_FIX] = gcvTRUE;
        Features[gcvFEATURE_TEXTURE_GATHER] = gcvTRUE;
        Features[gcvFEATURE_HELPER_INVOCATION] = gcvTRUE;
        Features[gcvFEATURE_TILEFILLER_32TILE_ALIGNED] = gcvTRUE;
        Features[gcvFEATURE_TEXTURE_ASTC_FIX] = gcvTRUE;
    }

    Features[gcvFEATURE_S8_MSAA_COMPRESSION] = database->REG_S8MSAACompression;

    Features[gcvFEATURE_TX_FRAC_PRECISION_6BIT] = database->REG_TX6bitFrac;

    Features[gcvFEATURE_MRT_FC_FIX] = database->REG_BugFixes21;

    Features[gcvFEATURE_TESSELLATION] = database->REG_TessellationShaders;

    Features[gcvFEATURE_TX_DECOMPRESSOR] =  (Features[gcvFEATURE_TEXTURE_TILE_STATUS_READ] &&
                                             Features[gcvFEATURE_COMPRESSION_V1]) ||
                                            (Features[gcvFEATURE_HALTI4] &&
                                             Features[gcvFEATURE_COMPRESSION_V2]) ||
                                             Features[gcvFEATURE_COMPRESSION_V4];

    Features[gcvFEATURE_RA_DEPTH_WRITE] = database->REG_RAWriteDepth;

    if (database->REG_RSS8)
    {
        Features[gcvFEATURE_S8_ONLY_RENDERING] = gcvTRUE;

        Features[gcvFEATURE_RS_DS_DOWNSAMPLE_NATIVE_SUPPORT] = gcvTRUE;
    }
    else if ((chipModel == gcv1500 && chipRevision == 0x5246) ||
             (chipRevision >= 0x5451))
    {
        Features[gcvFEATURE_S8_ONLY_RENDERING] = gcvTRUE;
    }

    if ((!Features[gcvFEATURE_FAST_MSAA] && !Features[gcvFEATURE_SMALL_MSAA]) ||
        database->RSBLT_MSAA_DECOMPRESSION
       )
    {
        Features[gcvFEATURE_RSBLT_MSAA_DECOMPRESSION] = gcvTRUE;
    }

    Features[gcvFEATURE_V2_MSAA_COHERENCY_FIX] = database->REG_MSAACoherencyCheck
                                              || (Hardware->config->chipFlags & gcvCHIP_FLAG_MSAA_COHERENCEY_ECO_FIX);


    Features[gcvFEATURE_HALTI5] = database->REG_Halti5;
    if (Features[gcvFEATURE_HALTI5])
    {
        Features[gcvFEATURE_USC] = gcvTRUE;
        /* If turn TXDESC off, need take care of state map accordingly*/
        Features[gcvFEATURE_TX_DESCRIPTOR] = gcvTRUE;
        Features[gcvFEATURE_CUBEMAP_ARRAY] = gcvTRUE;
        Features[gcvFEATURE_SEPARATE_RT_CTRL] = gcvTRUE;
        Features[gcvFEATURE_SMALLDRAW_BATCH] = gcvTRUE;
        Features[gcvFEATURE_TX_BORDER_CLAMP] = gcvTRUE;
        Features[gcvFEATURE_LOD_FIX_FOR_BASELEVEL] = gcvTRUE;
        Features[gcvFEATURE_NEW_STEERING_AND_ICACHE_FLUSH] = gcvTRUE;
        Features[gcvFEATURE_TX_8BPP_TS_FIX] = gcvTRUE;
        Features[gcvFEATURE_FENCE] = gcvTRUE;
        Features[gcvFEATURE_TX_DEFAULT_VALUE_FIX] = gcvTRUE;
        Features[gcvFEATURE_TX_MIPFILTER_NONE_FIX] = gcvTRUE;
        Features[gcvFEATURE_MC_STENCIL_CTRL] = gcvTRUE;
        Features[gcvFEATURE_R8_UNORM] = gcvTRUE;
        Features[gcvFEATURE_DEPTH_MATH_FIX] = gcvTRUE;
        Features[gcvFEATURE_PE_B2B_PIXEL_FIX] = gcvTRUE;
        Features[gcvFEATURE_MULTIDRAW_INDIRECT] = gcvTRUE;
        Features[gcvFEATURE_SAMPLER_BASE_OFFSET] = gcvTRUE;
        Features[gcvFEATURE_DRAW_ELEMENTS_BASE_VERTEX] = gcvTRUE;
        Features[gcvFEATURE_COMMAND_PREFETCH] = gcvTRUE;
        Features[gcvFEATURE_TEX_CACHE_FLUSH_FIX] = gcvTRUE;
        Features[gcvFEATURE_TEXTURE_BUFFER] = gcvTRUE;
    }

    Features[gcvFEATURE_TX_8bit_UVFrac] = database->TX_8bit_UVFrac;
    Features[gcvFEATURE_GS_SUPPORT_EMIT] = gcvFALSE;
    Features[gcvFEATURE_WIDE_LINE_FIX] = gcvFALSE;
    Features[gcvFEATURE_LINE_DIAMOND_RULE_FIX] = gcvFALSE;
    Features[gcvFEATURE_SAMPLEPOS_SWIZZLE_FIX] = gcvFALSE;
    Features[gcvFEATURE_SELECTMAP_SRC0_SWIZZLE_FIX] = gcvFALSE;
    Features[gcvFEATURE_LOADATTR_OOB_FIX] = gcvFALSE;
    Features[gcvFEATURE_RA_DEPTH_WRITE_MSAA1X_FIX] = database->RA_DEPTH_WRITE_MSAA1X_FIX;

    Features[gcvFEATURE_MRT_8BIT_DUAL_PIPE_FIX] = (Hardware->config->pixelPipes == 1)
                                                || database->PE_8bpp_DUALPIPE_FIX;

    Features[gcvFEATURE_NEW_GPIPE] = database->NEW_GPIPE;
    Features[gcvFEATURE_PIPELINE_32_ATTRIBUTES] = database->PIPELINE_32_ATTRIBUTES;
    Features[gcvFEATURE_SNAPPAGE_CMD_FIX] = database->SH_SNAP2PAGE_FIX;
    Features[gcvFEATURE_ADVANCED_SH_INST] = database->SH_ADVANCED_INSTR;
    Features[gcvFEATURE_HW_TFB] = database->HWTFB;
    Features[gcvFEATURE_MSAA_SHADING] = database->MSAA_SHADING;

    Features[gcvFEATURE_SH_FLAT_INTERPOLATION_DUAL16_FIX] = database->SH_FLAT_INTERPOLATION_DUAL16_FIX ||
                                                            !database->REG_Halti5;

    Features[gcvFEATURE_EVIS] = database->REG_Evis;

#if gcdENABLE_3D
    /* Turn off HELPER_INVOCATION for dEQP if this chip doesn't have WIDE_LINE_FIX. */
    if (Hardware->patchID == gcvPATCH_DEQP &&
        Features[gcvFEATURE_WIDE_LINE_FIX] == gcvFALSE)
    {
        Features[gcvFEATURE_HELPER_INVOCATION] = gcvFALSE;
    }
#endif

    /* If turn it off, need take care of state map accordingly */
    Features[gcvFEATURE_BLT_ENGINE] = database->REG_BltEngine;
    Features[gcvFEATURE_RENDER_ARRAY] =
    Features[gcvFEATURE_GEOMETRY_SHADER] = database->REG_GeometryShader;

    Features[gcvFEATURE_IMAGE_OUT_BOUNDARY_FIX] = database->REG_BugFixes23;

    if (database->REG_BugFixesIn544)
    {
        Features[gcvFEATURE_PE_DITHER_FIX2] = gcvTRUE;
    }

    if (((chipModel == gcv5000) && (chipRevision == 0x5309))
     || ((chipModel == gcv5000) && (chipRevision == 0x5434))
     || ((chipModel == gcv3000) && (chipRevision == 0x5435))
     || ((chipModel == gcv1500) && (chipRevision == 0x5246))
       )
    {
        Features[gcvFEATURE_INDEX_FETCH_FIX] = database->REG_BugFixes24;
    }
    else
    {
        Features[gcvFEATURE_INDEX_FETCH_FIX] = gcvTRUE;
    }

    if ((chipRevision >= 0x5400) && (chipRevision < 0x6000))
    {
        Features[gcvFEATURE_ROBUST_ATOMIC] = gcvTRUE;
    }

#if gcdENABLE_3D
    if (Hardware->patchID == gcvPATCH_REALRACING)
    {
        Features[gcvFEATURE_INDEX_FETCH_FIX] = gcvTRUE;
    }
#endif

    Features[gcvFEATURE_MULTIGPU_SYNC_V2] = database->MultiCoreSemaphoreStallV2;

    Features[gcvFEATURE_CHIPENABLE_LINK] = database->ChipEnableLink;

    /* Features that are used for compiler. */
    /* 5_0_3_gc1000 and 5_1_0_gc880
       5_0_3_rc7 will be read as 32h00005037 from chipRevision register.
       We can ignore the last digit for the revision. Just detect 5_0_3
       for existing gc1000 and 5_1_0 for existing 880.
    */
    if ((chipModel == gcv1000 && (chipRevision & 0xFFF0) == 0x5030)
        ||
        (chipModel == gcv880  && (chipRevision & 0xFFF0) == 0x5100))
    {
        Features[gcvFEATURE_TEXTURE_BIAS_LOD_FIX] = gcvFALSE;
    }
    else
    {
        Features[gcvFEATURE_TEXTURE_BIAS_LOD_FIX] = gcvTRUE;
    }

    /* Chip needs GL_Z fix. */
    if (chipModel >= gcv1000 || chipModel == gcv880 ||
        (chipModel == gcv900 && chipRevision == 0x5250))
    {
        Features[gcvFEATURE_USE_GL_Z] = gcvFALSE;
    }
    else
    {
        Features[gcvFEATURE_USE_GL_Z] = gcvTRUE;
    }

    /* Chip can support integer branch. */
    if (chipRevision >= 0x5220)
    {
        Features[gcvFEATURE_PARTLY_SUPPORT_INTEGER_BRANCH] = gcvTRUE;
        if (chipRevision >= 0x5250)
        {
            Features[gcvFEATURE_FULLLY_SUPPORT_INTEGER_BRANCH] = gcvTRUE;
        }
        else
        {
            Features[gcvFEATURE_FULLLY_SUPPORT_INTEGER_BRANCH] = gcvFALSE;
        }
    }
    else
    {
        Features[gcvFEATURE_PARTLY_SUPPORT_INTEGER_BRANCH] = gcvFALSE;
        Features[gcvFEATURE_FULLLY_SUPPORT_INTEGER_BRANCH] = gcvFALSE;
    }

    /* Chip can support integer attributes. */
    if (chipModel >= gcv1000)
    {
        Features[gcvFEATURE_SUPPORT_INTEGER_ATTRIBUTE] = gcvTRUE;
    }
    else
    {
        Features[gcvFEATURE_SUPPORT_INTEGER_ATTRIBUTE] = gcvFALSE;
    }

    /* Chip can support MOVAI. */
    Features[gcvFEATURE_SUPPORT_MOVAI ] = database->SUPPORT_MOVAI;

    /* Need special handle for CL_X. */
    if ((chipModel == gcv2100) ||
        (chipModel == gcv2000 && chipRevision == 0x5108)    ||
        (chipModel == gcv880 && chipRevision == 0x5106)     ||
        (chipModel == gcv880 && chipRevision == 0x5121)     ||
        (chipModel == gcv880 && chipRevision == 0x5122)     ||
        (chipModel == gcv880 && chipRevision == 0x5124))
    {
        Features[gcvFEATURE_NEED_FIX_FOR_CL_X] = gcvTRUE;
    }
    else
    {
        Features[gcvFEATURE_NEED_FIX_FOR_CL_X] = gcvFALSE;
    }

    /* Need special handle for CL_XE. */
    if (chipModel > gcv2100 || chipModel == gcv1500 ||
        (chipModel == gcv900 && chipRevision == 0x5250))
    {
        Features[gcvFEATURE_NEED_FIX_FOR_CL_XE] = gcvTRUE;
    }
    else
    {
        Features[gcvFEATURE_NEED_FIX_FOR_CL_XE] = gcvFALSE;
    }

    /* Chip can support integer operations. */
    if (chipModel > gcv2000 || chipModel == gcv880 || chipModel == gcv1500 ||
        (chipModel == gcv900 && chipRevision == 0x5250) ||
        Features[gcvFEATURE_NEED_FIX_FOR_CL_X])
    {
        Features[gcvFEATURE_SUPPORT_INTEGER] = gcvTRUE;
    }
    else
    {
        Features[gcvFEATURE_SUPPORT_INTEGER] = gcvFALSE;
    }

    /* Chip has fix for output pack. */
    if (chipRevision >= 0x5240)
    {
        Features[gcvFEATURE_HAS_OUTPUT_COUNT_FIX] = gcvTRUE;
    }
    else
    {
        Features[gcvFEATURE_HAS_OUTPUT_COUNT_FIX] = gcvFALSE;
    }

    /*
    ** HW has a limitation that each varying pack must be start with X channel,
    ** and need to program 0x0E08.
    */
    if (chipModel < gcv1000)
    {
        Features[gcvFEATURE_VARYING_PACKING_LIMITATION] = gcvTRUE;
    }
    else
    {
        Features[gcvFEATURE_VARYING_PACKING_LIMITATION] = gcvFALSE;
    }

    /*
    ** In arch_version_5_4_4 (v55, i.e., halti3) and beyond,
    ** +1 temp for each highp varying with 3 or 4 components for dual16.
    */
    if (Features[gcvFEATURE_HALTI3])
    {
        Features[gcvFEATURE_HIGHP_VARYING_SHIFT] = gcvTRUE;
    }
    else
    {
        Features[gcvFEATURE_HIGHP_VARYING_SHIFT] = gcvFALSE;
    }

    Features[gcvFEATURE_BUG_FIXES2] = database->REG_BugFixes2;

    Features[gcvFEATURE_L2_CACHE_FOR_2D_420] = database->REG_L2CacheFor2D420;

    Features[gcvFEATURE_TILE_STATUS_2BITS] = database->REG_TileStatus2Bits;

    if (Features[gcvFEATURE_128BTILE])
    {
        Features[gcvFEATURE_TILE_STATUS_2BITS] = gcvFALSE;
        Features[gcvFEATURE_S8_MSAA_COMPRESSION] = gcvFALSE;
    }

    Features[gcvFEATURE_EXTRA_SHADER_INSTRUCTIONS0] = database->REG_ExtraShaderInstructions0;
    Features[gcvFEATURE_EXTRA_SHADER_INSTRUCTIONS1] = database->REG_ExtraShaderInstructions1;
    Features[gcvFEATURE_EXTRA_SHADER_INSTRUCTIONS2] = database->REG_ExtraShaderInstructions2;
    Features[gcvFEATURE_MEDIUM_PRECISION] = database->REG_MediumPrecision;
    Features[gcvFEATURE_FE20_BIT_INDEX] = database->REG_FE20BitIndex;
    Features[gcvFEATURE_RS_NEW_BASEADDR] = database->RS_NEW_BASEADDR;
    Features[gcvFEATURE_SH_SUPPORT_V4] = database->SH_SUPPORT_V4;
    Features[gcvFEATURE_PE_NO_ALPHA_TEST] = database->PE_NO_ALPHA_TEST;
    Features[gcvFEATURE_SH_SNAP2PAGE_MAXPAGES_FIX] = database->SH_SNAP2PAGE_MAXPAGES_FIX;
    Features[gcvFEATURE_USC_FULLCACHE_FIX] = database->USC_FULL_CACHE_FIX;
    Features[gcvFEATURE_PE_64bit_FENCE_FIX] = database->PE_64bit_FENCE_FIX;
    Features[gcvFEATURE_PE_RGBA16I_FIX] = database->PE_RGBA16I_FIX;
    Features[gcvFEATURE_BLT_8bit_256TILE_FC_FIX] = database->BLT_8bpp_256TILE_FC_FIX;
    Features[gcvFEATURE_BLT_64bpp_MASKED_CLEAR_FIX] = database->BLT_64bpp_MASKED_CLEAR_FIX;
    Features[gcvFEATURE_SH_PSO_MSAA1x_FIX] = database->SH_PSO_MSAA1x_FIX;
    Features[gcvFEATURE_USC_ATOMIC_FIX] = database->USC_ATOMIC_FIX;

    Features[gcvFEATURE_EVIS_NO_ABSDIFF] = database->EVIS_NO_ABSDIFF;
    Features[gcvFEATURE_EVIS_NO_BITREPLACE] = database->EVIS_NO_BITREPLACE;
    Features[gcvFEATURE_EVIS_NO_BOXFILTER] = database->EVIS_NO_BOXFILTER;
    Features[gcvFEATURE_EVIS_NO_CORDIAC] = database->EVIS_NO_CORDIAC;
    Features[gcvFEATURE_EVIS_NO_DP32] = database->EVIS_NO_DP32;
    Features[gcvFEATURE_EVIS_NO_FILTER] = database->EVIS_NO_FILTER;
    Features[gcvFEATURE_EVIS_NO_IADD] = database->EVIS_NO_IADD;
    Features[gcvFEATURE_EVIS_NO_SELECTADD] = database->EVIS_NO_SELECTADD;
    Features[gcvFEATURE_EVIS_LERP_7OUTPUT] = database->EVIS_LERP_7OUTPUT;
    Features[gcvFEATURE_EVIS_ACCSQ_8OUTPUT] = database->EVIS_ACCSQ_8OUTPUT;

    Features[gcvFEATURE_TX_YUV_ASSEMBLER_10BIT] = database->TX_YUV_ASSEMBLER_10BIT;
    Features[gcvFEATURE_USC_GOS_ADDR_FIX] = database->USC_GOS_ADDR_FIX;
    Features[gcvFEATURE_SUPPORT_MSAA2X] = 0;

    Features[gcvFEATURE_TX_INTEGER_COORDINATE] = database->TX_INTEGER_COORDINATE;
    Features[gcvFEATURE_DRAW_ID] = database->DRAWID;
    Features[gcvFEATURE_PSIO_SAMPLEMASK_IN_R0ZW_FIX] = database->PSIO_SAMPLEMASK_IN_R0ZW_FIX;
    Features[gcvFEATURE_SECURITY] = database->SECURITY;
    Features[gcvFEATURE_ROBUSTNESS] = database->ROBUSTNESS;
    Features[gcvFEATURE_MULTI_CORE_BLOCK_SET_CONFIG] = database->MULTI_CORE_BLOCK_SET_CONFIG;
    Features[gcvFEATURE_TX_INTEGER_COORDINATE_V2] = database->TX_INTEGER_COORDINATE_V2;
    Features[gcvFEATURE_SNAPPAGE_CMD] = database->SNAPPAGE_CMD;
    Features[gcvFEATURE_SH_NO_INDEX_CONST_ON_A0] = database->SH_NO_INDEX_CONST_ON_A0;
    Features[gcvFEATURE_SH_NO_ONECONST_LIMIT] = database->SH_NO_ONECONST_LIMIT;
    Features[gcvFEATURE_SH_IMG_LDST_ON_TEMP] = database->SH_IMG_LDST_ON_TEMP;
    Features[gcvFEATURE_TX_INTEGER_COORDINATE_V2] = database->TX_INTEGER_COORDINATE_V2;
    Features[gcvFEATURE_COMPUTE_ONLY] = database->COMPUTE_ONLY;
    Features[gcvFEATURE_SH_IMG_LDST_CLAMP] = database->SH_IMG_LDST_CLAMP;
    Features[gcvFEATURE_SH_ICACHE_ALLOC_COUNT_FIX] = database->SH_ICACHE_ALLOC_COUNT_FIX;

    Features[gcvFEATURE_MSAA_OQ_FIX] = database->PE_MSAA_OQ_FIX;

    Features[gcvFEATURE_PE_ENHANCEMENTS2] = database->REG_PEEnhancements2;
    Features[gcvFEATURE_PSIO_MSAA_CL_FIX] = database->PSIO_MSAA_CL_FIX;

#if gcdENABLE_2D
    Fill2DFeaturesByDatabase(Hardware, Features);
#endif

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}
#endif

#if gcdENABLE_2D || gcdENABLE_3D


static void
_QueryCoreCount(
    IN gcoHARDWARE Hardware,
    IN gcsHARDWARE_CONFIG * Config
    )
{
    gceHARDWARE_TYPE type;

    gcmGETCURRENTHARDWARE(type);

    if (type == gcvHARDWARE_3D2D || type == gcvHARDWARE_3D)
    {
        gcoHAL_QueryCoreCount(gcvNULL, type, &Config->gpuCoreCount, Hardware->chipIDs);

        if (Config->gpuCoreCount == 0)
        {
            type = type == gcvHARDWARE_3D2D
                 ? gcvHARDWARE_3D
                 : gcvHARDWARE_3D2D
                 ;

            gcoHAL_QueryCoreCount(gcvNULL, type, &Config->gpuCoreCount, Hardware->chipIDs);

            gcmASSERT(Config->gpuCoreCount);
        }
    }
}

void
_AdjustChipRevision(
    IN gcsHARDWARE_CONFIG * Config
    )
{
    if ((Config->chipModel == gcv2000) && (Config->chipRevision & 0xffff0000) == 0xffff0000)
    {
        Config->chipModel = gcv3000;
        Config->chipRevision &= 0xffff;
        Config->chipFlags |= gcvCHIP_FLAG_GC2000_R2;
    }
}

#if gcdENABLE_3D
static void
_DetermineSuperTileModeByDatabase(
    IN gcsFEATURE_DATABASE * FeatureDatabase,
    IN gcsHARDWARE_CONFIG * Config
    )
{
    /* Get the Supertile layout in the hardware. */
    if (FeatureDatabase->REG_NewHZ || FeatureDatabase->REG_FastMSAA)
    {
        Config->superTileMode = 2;
    }
    else if (FeatureDatabase->REG_HierarchicalZ)
    {
        Config->superTileMode = 1;
    }
    else
    {
        Config->superTileMode = 0;
    }

    /* Exception for GC1000, revision 5035 &  GC800, revision 4612 */
    if (((Config->chipModel == gcv1000) && ((Config->chipRevision == 0x5035)
                                           || (Config->chipRevision == 0x5036)
                                           || (Config->chipRevision == 0x5037)
                                           || (Config->chipRevision == 0x5039)
                                           || (Config->chipRevision >= 0x5040)))
    || ((Config->chipModel == gcv800) && (Config->chipRevision == 0x4612))
    || ((Config->chipModel == gcv600) && (Config->chipRevision >= 0x4650))
    || ((Config->chipModel == gcv860) && (Config->chipRevision == 0x4647))
    || ((Config->chipModel == gcv400) && (Config->chipRevision >= 0x4633)))
    {
        Config->superTileMode = 1;
    }
}
#endif

static gceSTATUS
_FillInConfigTableByDatabase(
    IN gcoHARDWARE          Hardware,
    IN gcsHARDWARE_CONFIG * Config
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsHAL_INTERFACE iface;
    gcsFEATURE_DATABASE * featureDatabase;

#if gcdENABLE_3D
    gctBOOL halti5;
    gctBOOL halti0;
#endif

    gcmHEADER_ARG("Hardware=0x%x Config=0x%x", Hardware, Config);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /***********************************************************************
    ** Query chip identity.
    */

    iface.command = gcvHAL_QUERY_CHIP_IDENTITY;

    gcmONERROR(gcoOS_DeviceControl(
        gcvNULL,
        IOCTL_GCHAL_INTERFACE,
        &iface, gcmSIZEOF(iface),
        &iface, gcmSIZEOF(iface)
        ));

    Config->chipModel              = iface.u.QueryChipIdentity.chipModel;
    Config->chipRevision           = iface.u.QueryChipIdentity.chipRevision;
    Config->productID              = iface.u.QueryChipIdentity.productID;
    Config->chipFlags              = iface.u.QueryChipIdentity.chipFlags;

    Hardware->featureDatabase = gcQueryFeatureDB(
        Config->chipModel,
        Config->chipRevision,
        iface.u.QueryChipIdentity.productID,
        iface.u.QueryChipIdentity.ecoID,
        iface.u.QueryChipIdentity.customerID);

    gcmASSERT(Hardware->featureDatabase != gcvNULL);

    _AdjustChipRevision(Config);

    featureDatabase = Hardware->featureDatabase;

    Config->pixelPipes             = featureDatabase->NumPixelPipes;
    Config->resolvePipes           = featureDatabase->NumResolvePipes;
    Config->gpuCoreCount           = featureDatabase->CoreCount;

    /* Multiple core information may be not reported by feature bit, try to get it from gcoHAL */
    _QueryCoreCount(Hardware, Config);

#if gcdENABLE_3D
    /* Determine super tile mode by features, chipModel and chipRevision. */
    _DetermineSuperTileModeByDatabase(featureDatabase, Config);

    /* Multi render target. */
    if (featureDatabase->REG_Halti2 ||
        (Config->chipModel == gcv900 && Config->chipRevision == 0x5250)
       )
    {
        Config->renderTargets = 8;
    }
    else if (featureDatabase->REG_Halti0)
    {
        Config->renderTargets = 4;
    }
    else
    {
        Config->renderTargets = 1;
    }

    halti5 = featureDatabase->REG_Halti5;
    halti0 = featureDatabase->REG_Halti0;

    Config->streamCount            = featureDatabase->Streams;
    Config->registerMax            = featureDatabase->TempRegisters;
    Config->threadCount            = featureDatabase->ThreadCount;
    Config->shaderCoreCount        = featureDatabase->NumShaderCores;
    Config->vertexCacheSize        = featureDatabase->VertexCacheSize;
    Config->vertexOutputBufferSize = featureDatabase->VertexOutputBufferSize;
    Config->instructionCount       = featureDatabase->InstructionCount;
    Config->numConstants           = featureDatabase->NumberOfConstants;
    Config->bufferSize             = featureDatabase->BufferSize;
    Config->attribCount            = halti5 ? 32 : (halti0 ? 16 : 10);
    Config->varyingsCount          = featureDatabase->VaryingCount;
    Config->uscPagesMaxInKbyte     = featureDatabase->USC_MAX_PAGES;
    Config->resultWindowMaxSize    = featureDatabase->RESULT_WINDOW_MAX_SIZE;
    Config->l1CacheSizeInKbyte      = featureDatabase->L1CacheSize;
    Config->localStorageSizeInKbyte = featureDatabase->LocalStorageSize;

    gcmASSERT((Config->uscPagesMaxInKbyte == 0) /* hardwares w/o USC */
        || (Config->uscPagesMaxInKbyte == Config->localStorageSizeInKbyte));

    /* Determine shader parameters. */

    /* Determine constant parameters. */
    {if (Config->numConstants > 256){    Config->unifiedConst = gcvTRUE;
if (halti5){    Config->vsConstBase  = 0xD000;
    Config->psConstBase  = 0xD800;
}else{    Config->vsConstBase  = 0xC000;
    Config->psConstBase  = 0xC000;
}if ((Config->chipModel == gcv880) && ((Config->chipRevision & 0xfff0) == 0x5120)){    Config->vsConstMax   = 512;
    Config->psConstMax   = 64;
    Config->constMax     = 576;
}else{    Config->vsConstMax   = gcmMIN(512, Config->numConstants - 64);
    Config->psConstMax   = gcmMIN(512, Config->numConstants - 64);
    Config->constMax     = Config->numConstants;
}}else if (Config->numConstants == 256){    if (Config->chipModel == gcv2000 && (Config->chipRevision == 0x5118 || Config->chipRevision == 0x5140))    {        Config->unifiedConst = gcvFALSE;
        Config->vsConstBase  = 0x1400;
        Config->psConstBase  = 0x1C00;
        Config->vsConstMax   = 256;
        Config->psConstMax   = 64;
        Config->constMax     = 320;
    }    else    {        Config->unifiedConst = gcvFALSE;
        Config->vsConstBase  = 0x1400;
        Config->psConstBase  = 0x1C00;
        Config->vsConstMax   = 256;
        Config->psConstMax   = 256;
        Config->constMax     = 512;
    }}else{    Config->unifiedConst = gcvFALSE;
    Config->vsConstBase  = 0x1400;
    Config->psConstBase  = 0x1C00;
    Config->vsConstMax   = 168;
    Config->psConstMax   = 64;
    Config->constMax     = 232;
}};


    /* Determine instruction parameters. */
    /* TODO - need to add code for iCache. */
    if (Config->instructionCount > 1024)
    {
        Config->unifiedInst   = gcvTRUE;
        Config->vsInstBase    = 0x8000;
        Config->psInstBase    = 0x8000;
        Config->vsInstMax     =
        Config->psInstMax     =
        Config->instMax       = Config->instructionCount;
    }
    else if (Config->instructionCount > 256)
    {
        Config->unifiedInst   = gcvTRUE;
        Config->vsInstBase    = 0x3000;
        Config->psInstBase    = 0x2000;
        Config->vsInstMax     =
        Config->psInstMax     =
        Config->instMax       = Config->instructionCount;
    }
    else
    {
        gcmASSERT(Config->instructionCount == 256);
        Config->unifiedInst   = gcvFALSE;
        Config->vsInstBase    = 0x1000;
        Config->psInstBase    = 0x1800;
        Config->vsInstMax     = 256;
        Config->psInstMax     = 256;
        Config->instMax       = 256 + 256;
    }
#endif

    gcmFOOTER_NO();
    return status;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gceSTATUS
_FillInSWWATable(
    IN gcoHARDWARE  Hardware,
    IN gctBOOL *    SWWA
    )
{
    gceCHIPMODEL    chipModel          = Hardware->config->chipModel;
    gctUINT32       chipRevision       = Hardware->config->chipRevision;

    gcmHEADER_ARG("Hardware=0x%x SWWA=0x%x", Hardware, SWWA);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if ((chipModel == gcv2000) && (chipRevision == 0x5118 || chipRevision == 0x5140))
    {
        SWWA[gcvSWWA_1165] = gcvTRUE;
    }

    if ((chipModel == gcv4000) &&
        (((chipRevision & 0xFFF0) == 0x5240) ||
         ((chipRevision & 0xFFF0) == 0x5300) ||
         ((chipRevision & 0xFFF0) == 0x5310) ||
         ((chipRevision & 0xFFF0) == 0x5410) ||
         (chipRevision == 0x5421)))
    {
        SWWA[gcvSWWA_1163] = gcvTRUE;
    }

    if  ((chipModel == gcv2200 && chipRevision == 0x5244) ||
         (chipModel == gcv1500 && chipRevision == 0x5246)
        )
    {
        SWWA[gcvSWWA_1163] = gcvTRUE;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}
#endif

#if gcdENABLE_3D
typedef struct _gcoPATCH_BENCH
{
    gcePATCH_ID patchId;
    gctCONST_STRING name;

    /* If replaced by symbol checking, please set gcvTRUE(just android platform), else gcvFALSE. */
    gctBOOL symbolFlag;
}
gcoPATCH_BENCH;
#endif /* gcdENABLE_3D */

gceSTATUS
gcoHARDWARE_DetectProcess(
    IN gcoHARDWARE Hardware
    )
{
#if gcdENABLE_3D
    gctUINT i = 0;
    gctCHAR curProcessName[gcdMAX_PATH];
    gceSTATUS status = gcvSTATUS_OK;

    gcoPATCH_BENCH benchList[] =
    {
        /* Benchmark List */
        {
            gcvPATCH_GLBM21,
            "\xb8\xb3\xbd\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xcd\xce",
            gcvFALSE
        },
        {
            gcvPATCH_GLBM25,
            "\xb8\xb3\xbd\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xcd\xca",
            gcvFALSE
        },
        {
            gcvPATCH_GLBM27,
            "\xb8\xb3\xbd\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xcd\xc8",
            gcvFALSE
        },


#if defined(ANDROID)
        {
            gcvPATCH_GLBM21,
            "\x9c\x90\x92\xd1\x98\x93\x9d\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xd1"
            "\x98\x93\x9d\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xcd\xce",
            gcvTRUE
        },
        {
            gcvPATCH_GLBM25,
            "\x9c\x90\x92\xd1\x98\x93\x9d\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xd1"
            "\x98\x93\x9d\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xcd\xca",
            gcvTRUE
        },
        {
            gcvPATCH_GLBM27,
            "\x9c\x90\x92\xd1\x98\x93\x9d\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xd1"
            "\x98\x93\x9d\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xcd\xc8",
            gcvTRUE
        },
        {
             gcvPATCH_GFXBENCH,
             "\x91\x9a\x8b\xd1\x94\x96\x8c\x97\x90\x91\x8b\x96\xd1\x98\x99"
             "\x87\x9d\x9a\x91\x9c\x97\xd1\x98\x93\xd1\x89\xcc\xcf\xce\xcf"
             "\xcf\xd1\x9c\x90\x8d\x8f\x90\x8d\x9e\x8b\x9a",
             gcvTRUE
        },
        {
             gcvPATCH_GFXBENCH4,
             "\x91\x9a\x8b\xd1\x94\x96\x8c\x97\x90\x91\x8b\x96\xd1\x98\x99"
             "\x87\x9d\x9a\x91\x9c\x97\xd1\x98\x93\xd1\x89\xcb\xcf\xcf\xcf"
             "\xcf\xd1\x9c\x90\x8d\x8f\x90\x8d\x9e\x8b\x9a",
             gcvTRUE
        },
        {
            gcvPATCH_GLBMGUI,
            "\x9c\x90\x92\xd1\x98\x93\x9d\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xd1"
            "\x98\x93\x9d\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xb8\xaa\xb6",
            gcvTRUE
        },
        {
            gcvPATCH_RIPTIDEGP2,
            "\x9c\x90\x92\xd1\x89\x9a\x9c\x8b\x90\x8d\x8a\x91\x96\x8b\xd1\x8d"
            "\x9a\x9b",
            gcvFALSE
        },
#endif
        {
            gcvPATCH_GTFES30,
            "\x9c\x8b\x8c\xd2\x8d\x8a\x91\x91\x9a\x8d",
            gcvFALSE
        },
        {
            gcvPATCH_GTFES30,
            "\x98\x93\x9c\x8b\x8c",
            gcvFALSE
        },
#if defined(ANDROID)
        {
            gcvPATCH_GTFES30,
            "\x90\x8d\x98\xd1\x94\x97\x8d\x90\x91\x90\x8c\xd1\x98\x93\xa0\x9c\x8b\x8c",
            gcvFALSE
        },
#endif
        {
            gcvPATCH_GTFES30,
            "\xb8\xab\xb9",
            gcvFALSE
        },
#if defined(ANDROID)
        {
            gcvPATCH_GTFES30,
            "\x9c\x90\x92\xd1\x9e\x91\x9b\x8d\x90\x96\x9b\xd1\x8e\x9e\xd1\x90"
            "\x9a\x8c\xcc\xcf\x9c\x90\x91\x99\x90\x8d\x92",
            gcvFALSE
        },
#endif
        {
            gcvPATCH_CTGL20,
#if defined(ANDROID)
            "\x9c\x90\x92\xd1\x9e\x91\x9b\x8d\x90\x96\x9b\xd1\x9c\x8b\x98\x93\xcd\xcf",
#else
            "\x9c\x8b\x98\x93\xaf\x93\x9e\x86\x9a\x8d\xb3\xb1\xa7\x92\x8f",
#endif
            gcvFALSE
        },
        {
            gcvPATCH_CTGL11,
#if defined(ANDROID)
            "\x9c\x90\x92\xd1\x9e\x91\x9b\x8d\x90\x96\x9b\xd1\x9c\x8b\x98\x93\xce\xce",
#else
            "\x9c\x8b\x98\x93\xaf\x93\x9e\x86\x9a\x8d\xb3\xb1\xa7",
#endif
            gcvFALSE
        },
#if !defined(ANDROID)
        {
            gcvPATCH_BM21,
            "\x9d\x9e\x8c\x9a\x92\x9e\x8d\x94",
            gcvFALSE
        },
#endif
#if defined(ANDROID)
        {
            gcvPATCH_BM21,
            "\x9c\x90\x92\xd1\x8d\x96\x98\x97\x8b\x88\x9e\x8d\x9a\xd1\x8b\x9b\x92\x92\xcd",
            gcvTRUE
        },
#endif
        {
            gcvPATCH_MM06,
#if defined(ANDROID)
            "\x9c\x90\x92\xd1\x89\x96\x89\x9e\x91\x8b\x9a\x9c\x90\x8d\x8f\xd1"
            "\x98\x8d\x9e\x8f\x97\x96\x9c\x8c\xd1\x92\x92\xcf\xc9",
#else
            "\x92\x92\xcf\xc9",
#endif
            gcvFALSE
        },
        {
            gcvPATCH_MM07,
#if defined(ANDROID)
            "\x9c\x90\x92\xd1\x89\x96\x89\x9e\x91\x8b\x9a\x9c\x90\x8d\x8f\xd1"
            "\x98\x8d\x9e\x8f\x97\x96\x9c\x8c\xd1\x92\x92\xcf\xc8",
#else
            "\x92\x92\xcf\xc8",
#endif
            gcvFALSE
        },
#if defined(ANDROID)
        {
            gcvPATCH_QUADRANT,
            "\x9c\x90\x92\xd1\x9e\x8a\x8d\x90\x8d\x9e\x8c\x90\x99\x8b\x88\x90"
            "\x8d\x94\x8c\xd1\x8e\x8a\x9e\x9b\x8d\x9e\x91\x8b",
            gcvFALSE
        },
        {
            gcvPATCH_ANTUTUGL3,
			"\x9c\x90\x92\xd1\x9e\x91\x8b\x8a\x8b\x8a\xd1\xbe\xbd\x9a\x91\x9c\x97\xb2\x9e\x8d\x94\xd1\xb8\xb3\xcc",
            gcvFALSE
        },
#endif
        {
            gcvPATCH_GPUBENCH,
            "\x98\x8f\x8a\xbd\x9a\x91\x9c\x97",
            gcvFALSE
        },
#if defined(ANDROID)
        {
            gcvPATCH_SMARTBENCH,
            "\x9c\x90\x92\xd1\x8c\x92\x9e\x8d\x8b\x9d\x9a\x91\x9c\x97",
            gcvFALSE
        },
        {
            gcvPATCH_JPCT,
            "\x9c\x90\x92\xd1\x8b\x97\x8d\x9a\x9a\x9b\xd1\x95\x8f\x9c\x8b\xd1"
            "\x9d\x9a\x91\x9c\x97",
            gcvFALSE
        },
        {
            gcvPATCH_FISHNOODLE,
            "\x99\x96\x8c\x97\x91\x90\x90\x9b\x93\x9a\xd1\x9d\x9a\x91\x9c\x97"
            "\x92\x9e\x8d\x94",
            gcvTRUE
        },
        {
            gcvPATCH_NENAMARK2,
            "\x8c\x9a\xd1\x91\x9a\x91\x9e\xd1\x91\x9a\x91\x9e\x92\x9e\x8d\x94"
            "\xcd",
            gcvTRUE
        },
        {
            gcvPATCH_NENAMARK,
            "\x8c\x9a\xd1\x91\x9a\x91\x9e\xd1\x91\x9a\x91\x9e\x92\x9e\x8d\x94"
            "\xce",
            gcvTRUE
        },
        {
            gcvPATCH_NEOCORE,
            "\x9c\x90\x92\xd1\x8e\x8a\x9e\x93\x9c\x90\x92\x92\xd1\x8e\x87\xd1"
            "\x91\x9a\x90\x9c\x90\x8d\x9a",
            gcvTRUE
        },
        {
            gcvPATCH_GLBM11,
            "\x9c\x90\x92\xd1\x98\x93\x9d\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xd1"
            "\xb8\xb3\xbd\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xce\xce",
            gcvTRUE
        },
        {
            gcvPATCH_BMGUI,
            "\x9c\x90\x92\xd1\x8d\x96\x98\x97\x8b\x88\x9e\x8d\x9a\xd1\x9d\x9e"
            "\x8c\x9a\x92\x9e\x8d\x94\x98\x8a\x96",
            gcvTRUE
        },
        {
            gcvPATCH_RTESTVA,
            "\x9c\x90\x92\xd1\x8b\x97\x96\x91\x94\x88\x9e\x8d\x9a\xd1\xad\xab"
            "\x9a\x8c\x8b\xa9\xbe",
            gcvFALSE
        },
        {
            gcvPATCH_BUSPARKING3D,
            "\x9c\x90\x92\xd1\x98\x9e\x92\x9a\x8c\x97\x9a\x93\x93\xd1\x9d\x8a"
            "\x8c\x8f\x9e\x8d\x94\x96\x91\x98\xcc\x9b",
            gcvFALSE
        },
        {
            gcvPATCH_PREMIUM,
            "\x9c\x90\x92\xd1\x9d\x8a\x93\x94\x86\xd1\xa6\x9a\x8c\x8b\x9a\x8d"
            "\x9b\x9e\x86\xd1\x8f\x8d\x9a\x92\x96\x8a\x92",
            gcvFALSE
        },
        {
            gcvPATCH_RACEILLEGAL,
            "\x9c\x90\x92\xd1\x97\x9a\x8d\x90\x9c\x8d\x9e\x99\x8b\xd1\x98\x9e"
            "\x92\x9a\xd1\x8d\x9e\x9c\x9a\x96\x93\x93\x9a\x98\x9e\x93",
            gcvFALSE
        },
        {
            gcvPATCH_MEGARUN,
            "\x9c\x90\x92\xd1\x98\x9a\x8b\x8c\x9a\x8b\x98\x9e\x92\x9a\x8c\xd1"
            "\x92\x9a\x98\x9e\x8d\x8a\x91",
            gcvFALSE
        },
        {
            gcvPATCH_GLOFTSXHM,
            "\x9c\x90\x92\xd1\x98\x9e\x92\x9a\x93\x90\x99\x8b\xd1\x9e\x91\x9b"
            "\x8d\x90\x96\x9b\xd1\xbe\xb1\xb2\xaf\xd1\xb8\x93\x90\x99\x8b\xac"
            "\xa7\xb7\xb2",
            gcvFALSE
        },
        {
            gcvPATCH_XRUNNER,
            "\x9c\x90\x92\xd1\x9b\x8d\x90\x96\x9b\x97\x9a\x91\xd1\x98\x9e\x92"
            "\x9a\xd1\x87\x8d\x8a\x91\x91\x9a\x8d",
            gcvFALSE
        },
        {
            gcvPATCH_SIEGECRAFT,
            "\x9c\x90\x92\xd1\x9d\x93\x90\x88\x99\x96\x8c\x97\x8c\x8b\x8a\x9b"
            "\x96\x90\x8c\xd1\x8c\x96\x9a\x98\x9a\x9c\x8d\x9e\x99\x8b",
            gcvFALSE
        },
        {
            gcvPATCH_DUOKAN,
            "\x9c\x90\x92\xd1\x9b\x8a\x90\x94\x9e\x91\xd1\x9b\x8a\x90\x94\x9e"
            "\x91\x8b\x89\x8b\x9a\x8c\x8b",
            gcvFALSE
        },
        {
            gcvPATCH_NBA2013,
            "\x9c\x90\x92\xd1\x8b\xcd\x94\x8c\x8f\x90\x8d\x8b\x8c\xd1\x91\x9d"
            "\x9e\xcd\x94\xce\xcc\x9e\x91\x9b\x8d\x90\x96\x9b",
            gcvFALSE
        },
        {
            gcvPATCH_BARDTALE,
            "\x9c\x90\x92\xd1\x96\x91\x87\x96\x93\x9a\xd1\xbd\x9e\x8d\x9b\xab"
            "\x9e\x93\x9a",
            gcvFALSE
        },
        {
            gcvPATCH_F18,
            "\x96\x8b\xd1\x8d\x90\x8d\x8b\x90\x8c\xd1\x99\xce\xc7\x9c\x9e\x8d"
            "\x8d\x96\x9a\x8d\x93\x9e\x91\x9b\x96\x91\x98\x93\x96\x8b\x9a",
            gcvFALSE
        },
        {
            gcvPATCH_CARPARK,
            "\x9c\x90\x92\xd1\x96\x9c\x93\x90\x8a\x9b\x85\x90\x91\x9a\xd1\xbc"
            "\x9e\x8d\xaf\x9e\x8d\x94",
            gcvFALSE
        },
        {
            gcvPATCH_CARCHALLENGE,
            "\x91\x9a\x8b\xd1\x99\x96\x8c\x97\x93\x9e\x9d\x8c\xd1\xac\x8f\x90"
            "\x8d\x8b\x8c\xbc\x9e\x8d\xbc\x97\x9e\x93\x93\x9a\x91\x98\x9a",
            gcvFALSE
        },
        {
            gcvPATCH_HEROESCALL,
            "\x9c\x90\x92\xd1\xbb\x9a\x99\x96\x9e\x91\x8b\xbb\x9a\x89\xd1\x97"
            "\x9a\x8d\x90\x9a\x8c\x9c\x9e\x93\x93\xd1\xab\xb7\xbb",
            gcvFALSE
        },
        {
            gcvPATCH_GLOFTF3HM,
            "\x9c\x90\x92\xd1\x98\x9e\x92\x9a\x93\x90\x99\x8b\xd1\x9e\x91\x9b"
            "\x8d\x90\x96\x9b\xd1\xbe\xb1\xb2\xaf\xd1\xb8\x93\x90\x99\x8b\xb9"
            "\xcc\xb7\xb2",
            gcvFALSE
        },
        {
            gcvPATCH_CRAZYRACING,
            "\x9c\x90\x92\xd1\x88\x96\x91\x9b\xd1\xbc\x8d\x9e\x85\x86\xad\x9e"
            "\x9c\x96\x91\x98",
            gcvFALSE
        },
        {
            gcvPATCH_CTS_TEXTUREVIEW,
            /* "com.android.cts.textureview" */
            "\x9c\x90\x92\xd1\x9e\x91\x9b\x8d\x90\x96\x9b\xd1\x9c\x8b\x8c\xd1"
            "\x8b\x9a\x87\x8b\x8a\x8d\x9a\x89\x96\x9a\x88",
            gcvFALSE
        },
#endif
        {
            gcvPATCH_FIREFOX,
#if defined(ANDROID)
            "\x90\x8d\x98\xd1\x92\x90\x85\x96\x93\x93\x9e\xd1\x99\x96\x8d\x9a"
            "\x99\x90\x87",
#else
            "\x99\x96\x8d\x9a\x99\x90\x87",
#endif
            gcvFALSE
        },
        {
            gcvPATCH_CHROME,
#if defined(ANDROID)
            "\x9c\x90\x92\xd1\x9e\x91\x9b\x8d\x90\x96\x9b\xd1\x9c\x97\x8d\x90"
            "\x92\x9a",
#else
            "\x9c\x97\x8d\x90\x92\x9a",
#endif
            gcvFALSE
        },
#if defined(ANDROID)
        {
            gcvPATCH_CHROME,
            "\x9c\x90\x92\xd1\x9c\x97\x8d\x90\x92\x9a\xd1\x8b\x89\xd1\x8c\x8b"
            "\x9e\x9d\x93\x9a",
            gcvFALSE
        },
        {
            gcvPATCH_CHROME,
            "\x9c\x90\x92\xd1\x9c\x97\x8d\x90\x92\x9a\xd1\x9d\x9a\x8b\x9e",
            gcvFALSE
        },
        {
            gcvPATCH_MONOPOLY,
            "\x9c\x90\x92\xd1\x9a\x9e\xd1\x92\x90\x91\x90\x8f\x90\x93\x86\x92"
            "\x96\x93\x93\x96\x90\x91\x9e\x96\x8d\x9a\xa0\x91\x9e",
            gcvFALSE
        },
        {
            gcvPATCH_SNOWCOLD,
            "\x9c\x90\x92\xd1\x8c\x91\x90\x88\x9c\x90\x93\x9b\xd1\x9d\x9a\x91"
            "\x9c\x97\x92\x9e\x8d\x94",
            gcvFALSE
        },
        {
            gcvPATCH_BM3,
            "\x9c\x90\x92\xd1\x8d\x96\x98\x97\x8b\x88\x9e\x8d\x9a\xd1\x94\x9e"
            "\x91\x85\x96\xd1\x9d\x9e\x8c\x9a\x92\x9e\x8d\x94\x9a\x8c\xcc",
            gcvTRUE
        },
        {
            gcvPATCH_DEQP,
            "\x9c\x90\x92\xd1\x9b\x8d\x9e\x88\x9a\x93\x9a\x92\x9a\x91\x8b\x8c"
            "\xd1\x9b\x9a\x8e\x8f\xd1\x9a\x87\x9a\x9c\x8c\x9a\x8d\x89\x9a\x8d"
            "\xc5\x8b\x9a\x8c\x8b\x9a\x8d\x9c\x90\x8d\x9a",
            gcvFALSE
        },
        {
            gcvPATCH_DEQP,
            "\x9c\x90\x92\xd1\x9b\x8d\x9e\x88\x9a\x93\x9a\x92\x9a\x91\x8b\x8c"
            "\xd1\x9b\x9a\x8e\x8f\xc5\x8b\x9a\x8c\x8b\x9a\x8d\x9c\x90\x8d\x9a",
            gcvFALSE
        },
        {
            gcvPATCH_SF4,
            "\x95\x8f\xd1\x9c\x90\xd1\x9c\x9e\x8f\x9c\x90\x92\xd1\x9e\x91\x9b"
            "\x8d\x90\x96\x9b\xd1\x8c\x99\xcb\xa0\x91\x90\xa0\x9c\x90\x92\x8f"
            "\x8d\x9a\x8c\x8c\xa0\x98\x90\x90\x98\x93\x9a\x8f\x93\x9e\x86",
            gcvFALSE
        },

        {
            gcvPATCH_MGOHEAVEN2,
            "\x9c\x90\x92\xd1\x9b\x9e\x89\x96\x9b\x9e\x92\x9e\x9b\x90\xd1\xb2"
            "\x9a\x8b\x9e\x93\xb8\x9a\x9e\x8d\xb0\x8a\x8b\x9a\x8d\xb7\x9a\x9e"
            "\x89\x9a\x91\xcd",
            gcvFALSE
        },
        {
            gcvPATCH_SILIBILI,
            "\x9c\x90\x92\xd1\x8c\x96\x93\x96\xd1\x9d\x96\x93\x96",
            gcvFALSE
        },
        {
            gcvPATCH_ELEMENTSDEF,
            "\x9c\x90\x92\xd1\x99\x9a\x9e\x92\x9d\x9a\x8d\xd1\x9a\x93\x9a\x92"
            "\x9a\x91\x8b\x8c\x9b\x9a\x99",
            gcvFALSE
        },
        {
            gcvPATCH_GLOFTKRHM,
            "\x9c\x90\x92\xd1\x98\x9e\x92\x9a\x93\x90\x99\x8b\xd1\x9e\x91\x9b"
            "\x8d\x90\x96\x9b\xd1\xbe\xb1\xb2\xaf\xd1\xb8\x93\x90\x99\x8b\xb4"
            "\xad\xb7\xb2",
            gcvFALSE
        },
        {
            gcvPATCH_AIRNAVY,
            "\x96\x8b\xd1\x8d\x90\x8d\x8b\x90\x8c\xd1\x9e\x96\x8d\x91\x9e\x89"
            "\x86\x99\x96\x98\x97\x8b\x9a\x8d\x8c",
            gcvFALSE
        },
        {
            gcvPATCH_F18NEW,
            "\x96\x8b\xd1\x8d\x90\x8d\x8b\x90\x8c\xd1\x99\xce\xc7\x9c\x9e\x8d"
            "\x8d\x96\x9a\x8d\x93\x9e\x91\x9b\x96\x91\x98",
            gcvFALSE
        },
        {
            gcvPATCH_SUMSUNG_BENCH,
            "\x9c\x90\x92\xd1\x8c\x9e\x92\x8c\x8a\x91\x98\xd1\x9d\x9a\x91\x9c"
            "\x97\x92\x9e\x8d\x94\x8c\xc5\x9a\x8c\xce",
            gcvFALSE
        },

        {
            gcvPATCH_ROCKSTAR_MAXPAYNE,
            /* "com.rockstar.maxpayne" */
            "\x9c\x90\x92\xd1\x8d\x90\x9c\x94\x8c\x8b\x9e\x8d\xd1\x92\x9e\x87"
            "\x8f\x9e\x86\x91\x9a",
            gcvFALSE
        },
        {
            gcvPATCH_TITANPACKING,
            /* "com.vascogames.titanicparking" */
            "\x9c\x90\x92\xd1\x89\x9e\x8c\x9c\x90\x98\x9e\x92\x9a\x8c\xd1\x8b"
            "\x96\x8b\x9e\x91\x96\x9c\x8f\x9e\x8d\x94\x96\x91\x98",
            gcvFALSE
        },
#endif

#if defined(UNDER_CE)
        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x9e\x93\x93"
            "\x90\x9c\x9e\x8b\x96\x90\x91\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x9e\x8f\x96",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x9e\x8b\x90"
            "\x92\x96\x9c\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x9d\x9e\x8c\x96\x9c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x9d\x9e\x8c"
            "\x96\x9c\xa0\x94\x9a",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x9d\x8a\x99"
            "\x99\x9a\x8d\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x9c\x93\xbc"
            "\x90\x8f\x86\xb6\x92\x9e\x98\x9a",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x9c\x93\xb8"
            "\x9a\x8b\xb6\x91\x99\x90",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x9c\x93\xad"
            "\x9a\x9e\x9b\xa8\x8d\x96\x8b\x9a\xb6\x92\x9e\x98\x9a",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x9c\x90\x92"
            "\x92\x90\x91\x99\x91\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x9c\x90\x92"
            "\x8f\x96\x93\x9a\x8d",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x9c\x90\x92"
            "\x8f\x8a\x8b\x9a\x96\x91\x99\x90",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x9c\x90\x91"
            "\x8b\x8d\x9e\x9c\x8b\x96\x90\x91\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x9c\x90\x91"
            "\x89\x9a\x8d\x8c\x96\x90\x91\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x9a\x89\x9a"
            "\x91\x8b\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x98\x9a\x90"
            "\x92\x9a\x8b\x8d\x96\x9c\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x97\x9e\x93\x99",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x96\x91\x8b"
            "\x9a\x98\x9a\x8d\xa0\x90\x8f\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x94\x9a\x8d"
            "\x91\x9a\x93\xa0\x96\x92\x9e\x98\x9a\xa0\x92\x9a\x8b\x97\x90\x9b"
            "\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x94\x9a\x8d"
            "\x91\x9a\x93\xa0\x8d\x9a\x9e\x9b\xa0\x88\x8d\x96\x8b\x9a",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x92\x9e\x8b"
            "\x97\xa0\x9d\x8d\x8a\x8b\x9a\xa0\x99\x90\x8d\x9c\x9a",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x92\x8a\x93"
            "\x8b\x96\x8f\x93\x9a\xa0\x9b\x9a\x89\x96\x9c\x9a\xa0\x9c\x90\x91"
            "\x8b\x9a\x87\x8b",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x91\x9a\x88"
            "\xa0\x8b\x9a\x8c\x8b\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x8f\x8d\x90"
            "\x99\x96\x93\x96\x91\x98",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x8d\x9a\x93"
            "\x9e\x8b\x96\x90\x91\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x8c\x9a\x93"
            "\x9a\x9c\x8b",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x8b\x9a\x8c"
            "\x8b\xa0\x9c\x93\xa0\x98\x93\xa0\x97",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x8b\x9a\x8c"
            "\x8b\xa0\x9c\x93\xa0\x97",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x8b\x9a\x8c"
            "\x8b\xa0\x9c\x93\xa0\x8f\x93\x9e\x8b\x99\x90\x8d\x92\xa0\x97",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x8b\x9a\x8c"
            "\x8b\xa0\x97\x9a\x9e\x9b\x9a\x8d\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x8b\x9a\x8c"
            "\x8b\xa0\x90\x8f\x9a\x91\x9c\x93\xa0\x97",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x8b\x97\x8d"
            "\x9a\x9e\x9b\xa0\x9b\x96\x92\x9a\x91\x8c\x96\x90\x91\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x89\x9a\x9c"
            "\xa0\x9e\x93\x96\x98\x91",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x93\xce\xce\xa0\x9c\x90\x91\x99\x90\x8d\x92\xa0\x89\x9a\x9c"
            "\xa0\x8c\x8b\x9a\x8f",
            gcvFALSE
        },
        {
            gcvPATCH_FM_OES_PLAYER,
            "\x99\x92\xa0\x90\x9a\x8c\xa0\x8f\x93\x9e\x86\x9a\x8d\xd1\x9a\x87"
            "\x9a",
            gcvFALSE
        },
#else
        {
            gcvPATCH_OCLCTS,
            "\x9c\x90\x92\x8f\x8a\x8b\x9a\x96\x91\x99\x90",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x9d\x9e\x8c\x96\x9c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x9e\x8f\x96",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x9c\x90\x92\x8f\x96\x93\x9a\x8d",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x9c\x90\x92\x92\x90\x91\x99\x91\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x98\x9a\x90\x92\x9a\x8b\x8d\x96\x9c\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x8d\x9a\x93\x9e\x8b\x96\x90\x91\x9e\x93\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x8b\x97\x8d\x9a\x9e\x9b\xa0\x9b\x96\x92\x9a"
            "\x91\x8c\x96\x90\x91\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x92\x8a\x93\x8b\x96\x8f\x93\x9a\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x9e\x8b\x90\x92\x96\x9c\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x8f\x8d\x90\x99\x96\x93\x96\x91\x98",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x9a\x89\x9a\x91\x8b\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x9e\x93\x93\x90\x9c\x9e\x8b\x96\x90\x91\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x89\x9a\x9c\x9e\x93\x96\x98\x91",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x89\x9a\x9c\x8c\x8b\x9a\x8f",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x9d\x8a\x99\x99\x9a\x8d\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x9c\x93\xa0\x98\x9a\x8b\xa0\x96\x91\x99\x90",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x94\x9a\x8d\x91\x9a\x93\xa0\x96\x92\x9e\x98"
            "\x9a\xa0\x92\x9a\x8b\x97\x90\x9b\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x96\x92\x9e\x98\x9a\xa0\x8c\x8b\x8d\x9a\x9e"
            "\x92\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x9c\x93\xa0\x9c\x90\x8f\x86\xa0\x96\x92\x9e"
            "\x98\x9a\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x9c\x93\xa0\x8d\x9a\x9e\x9b\xa0\x88\x8d\x96"
            "\x8b\x9a\xa0\x96\x92\x9e\x98\x9a\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x97\x9a\x9e\x9b\x9a\x8d\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x9c\x93\xa0\x97",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x9c\x93\xa0\x8f\x93\x9e\x8b\x99\x90\x8d\x92"
            "\xa0\x97",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x9c\x93\xa0\x98\x93\xa0\x97",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x90\x8f\x9a\x91\x9c\x93\xa0\x97",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x98\x93",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x8c\x9a\x93\x9a\x9c\x8b",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x9c\x90\x91\x89\x9a\x8d\x8c\x96\x90\x91\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9c\x90\x91\x8b\x8d\x9e\x9c\x8b\x96\x90\x91\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x9d\x8d\x8a\x8b\x9a\x99\x90\x8d\x9c\x9a",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\x8b\x9a\x8c\x8b\xa0\x96\x91\x8b\x9a\x98\x9a\x8d\xa0\x90\x8f\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_OCLCTS,
            "\xab\x9a\x8c\x8b\xa0\x97\x9e\x93\x99",
            gcvFALSE
        },
#endif

        {
            gcvPATCH_DEQP,
            /* deqp-gles */
            "\x9b\x9a\x8e\x8f\xd2\x98\x93\x9a\x8c",
            gcvFALSE
        },

        {
            gcvPATCH_DEQP,
            /* deqp-egl */
            "\x9b\x9a\x8e\x8f\xd2\x9a\x98\x93",
            gcvFALSE
        },

#if defined(ANDROID)
        {
            gcvPATCH_A8HP,
            "\x9c\x90\x92\xd1\x98\x9e\x92\x9a\x93\x90\x99\x8b\xd1\x9e\x91\x9b"
            "\x8d\x90\x96\x9b\xd1\xb8\xbe\xb1\xbb\xd1\xb8\x93\x90\x99\x8b\xbe"
            "\xc7\xb7\xaf",
            gcvFALSE
        },
        {
            gcvPATCH_BASEMARKX,
            "\xbd\x9e\x8c\x9a\x92\x9e\x8d\x94\xa7",
            gcvFALSE
        },
        {
            gcvPATCH_WISTONESG,
            "\x9c\x91\xd1\x88\x96\x8c\x8b\x90\x91\x9a\xd1\x8c\x98",
            gcvFALSE
        },
        {
            gcvPATCH_SPEEDRACE,
            "\x9a\x8a\xd1\x9b\x8d\x9a\x9e\x92\x8a\x8f\xd1\x8c\x8f\x9a\x9a\x9b"
            "\x8d\x9e\x9c\x96\x91\x98\x8a\x93\x8b\x96\x92\x9e\x8b\x9a\x99\x8d"
            "\x9a\x9a",
            gcvFALSE
        },
        {
            gcvPATCH_FSBHAWAIIF,
            "\x9c\x90\x92\xd1\x8b\x97\x9a\x8b\x96\x8c\x98\x9e\x92\x9a\x8c\xd1"
            "\x99\x8c\x9d\x90\x9a\x96\x91\x98\x97\x9e\x88\x9e\x96\x96\x99\x8d"
            "\x9a\x9a",
            gcvFALSE
        },
        {
            gcvPATCH_CKZOMBIES2,
            "\x9c\x90\x92\xd1\x98\x93\x8a\xd1\x9c\x94\x85\x90\x92\x9d\x96\x9a"
            "\x8c\xcd",
            gcvFALSE
        },
        {
            gcvPATCH_A8CN,
            "\x9c\x90\x92\xd1\x98\x9e\x92\x9a\x93\x90\x99\x8b\xd1\x9e\x91\x9b"
            "\x8d\x90\x96\x9b\xd1\xbe\xb1\xb2\xaf\xd1\xb8\x93\x90\x99\x8b\xbe"
            "\xc7\xbc\xb1",
            gcvFALSE
        },
        {
            gcvPATCH_EADGKEEPER,
            "\x9c\x90\x92\xd1\x9a\x9e\xd1\x98\x9e\x92\x9a\xd1\x9b\x8a\x91\x98"
            "\x9a\x90\x91\x94\x9a\x9a\x8f\x9a\x8d\xa0\x91\x9e",
            gcvFALSE
        },
        {
            gcvPATCH_OESCTS,
            /*"com.android.cts.stub" */
            "\x9c\x90\x92\xd1\x9e\x91\x9b\x8d\x90\x96\x9b\xd1\x9c\x8b\x8c\xd1"
            "\x8c\x8b\x8a\x9d",
            gcvFALSE
        },
        {
            gcvPATCH_GANGSTAR,
            /* "com.gameloft.android.ANMP.GloftGGHM" */
            "\x9c\x90\x92\xd1\x98\x9e\x92\x9a\x93\x90\x99\x8b\xd1\x9e\x91\x9b"
            "\x8d\x90\x96\x9b\xd1\xbe\xb1\xb2\xaf\xd1\xb8\x93\x90\x99\x8b\xb8"
            "\xb8\xb7\xb2",
            gcvFALSE
        },
        {
            gcvPATCH_TRIAL,
            /* "com.turbowarsoft.trial" */
            "\x9c\x90\x92\xd1\x8b\x8a\x8d\x9d\x90\x88\x9e\x8d\x8c\x90\x99\x8b"
            "\xd1\x8b\x8d\x96\x9e\x93",
            gcvFALSE
        },
        {
            gcvPATCH_WHRKYZIXOVAN,
            /* "com.thetisgames.whrkyzixovan" */
            "\x9c\x90\x92\xd1\x8b\x97\x9a\x8b\x96\x8c\x98\x9e\x92\x9a\x8c\xd1"
            "\x88\x97\x8d\x94\x86\x85\x96\x87\x90\x89\x9e\x91",
            gcvFALSE
        },
        {
            gcvPATCH_GMMY16MAPFB,
            /* "gm-my16-map-fb" */
            "\x98\x92\xd2\x92\x86\xce\xc9\xd2\x92\x9e\x8f\xd2\x99\x9d",
            gcvFALSE
        },
        {
            gcvPATCH_UIMARK,
            /* "com.rightware.uimark" */
            "\x9c\x90\x92\xd1\x8d\x96\x98\x97\x8b\x88\x9e\x8d\x9a\xd1\x8a\x96"
            "\x92\x9e\x8d\x94",
            gcvFALSE
        },
        {
            gcvPATCH_NAMESGAS,
            /* "com.namcobandaigames.sgas" */
            "\x9c\x90\x92\xd1\x91\x9e\x92\x9c\x90\x9d\x9e\x91\x9b\x9e\x96\x98"
            "\x9e\x92\x9a\x8c\xd1\x8c\x98\x9e\x8c",
            gcvFALSE
        },
        {
            gcvPATCH_NETFLIX,
            /* "com.netflix.ninja" */ "\x9c\x90\x92\xd1\x91\x9a\x8b\x99\x93\x96\x87\xd1\x91\x96\x91\x95\x9e",
            gcvFALSE
        },
        {
            gcvPATCH_AFTERBURNER,
            /* "com.sega.afterburner" */
            "\x9c\x90\x92\xd1\x8c\x9a\x98\x9e\xd1\x9e\x99\x8b\x9a\x8d\x9d\x8a"
            "\x8d\x91\x9a\x8d",
            gcvFALSE
        },
        {
            gcvPATCH_OES20SFT,
#if defined(ANDROID)
 "\x9c\x90\x92\xd1\x9e\x91\x9b\x8d\x90\x96\x9b\xd1\x8e\x9e\xd1\x90\x9a\x8c\xcd\xcf\x8c\x99\x8b",
#elif defined(UNDER_CE)
          "\xac\xb9\xab\xa0\xcd\xd1\xcf\xa0\xbc\xba\xd1\x9a\x87\x9a",
#else /* #elif defined(__QNXNTO__) */
                  "\xac\xb9\xab\xcd\xd1\xcf",
#endif
            gcvFALSE
        },
        {
            gcvPATCH_OES30SFT,
            /* "com.android.qa.oes30sft" */
            "\x9c\x90\x92\xd1\x9e\x91\x9b\x8d\x90\x96\x9b\xd1\x8e\x9e\xd1\x90"
            "\x9a\x8c\xcc\xcf\x8c\x99\x8b",
            gcvFALSE
        },
        {
            gcvPATCH_BASEMARKOSIICN,
            /* "com.rightware.BasemarkOSIICN" */
            "\x9c\x90\x92\xd1\x8d\x96\x98\x97\x8b\x88\x9e\x8d\x9a\xd1\xbd\x9e"
            "\x8c\x9a\x92\x9e\x8d\x94\xb0\xac\xb6\xb6\xbc\xb1",
            gcvFALSE
        },
        {
             gcvPATCH_ANDROID_WEBGL,
             /* "android.webgl.cts" */ "\x9e\x91\x9b\x8d\x90\x96\x9b\xd1\x88\x9a\x9d\x98\x93\xd1\x9c\x8b\x8c",
             gcvFALSE
        },
        {
            gcvPATCH_ANDROID_CTS_MEDIA,
            /* "com.android.cts.media" */
            "\x9c\x90\x92\xd1\x9e\x91\x9b\x8d\x90\x96\x9b\xd1\x9c\x8b\x8c\xd1"
            "\x92\x9a\x9b\x96\x9e",
            gcvFALSE
        },
        {
            gcvPATCH_ANDROID_COMPOSITOR,
            "\x8c\x8a\x8d\x99\x9e\x9c\x9a\x99\x93\x96\x91\x98\x9a\x8d",
            gcvFALSE
        },
        {
             gcvPATCH_WATER2_CHUKONG,
             /* "com.chukong.watertwo_chukong.wdj" */
              "\x9c\x90\x92\xd1\x9c\x97\x8a\x94\x90\x91\x98\xd1\x88\x9e\x8b\x9a\x8d\x8b\x88\x90\xa0\x9c\x97\x8a\x94\x90\x91\x98\xd1\x88\x9b\x95",
             gcvFALSE
        },
        /* google earth */
        {
            gcvPATCH_GOOGLEEARTH,
            /* "com.google.earth" */
            "\x9c\x90\x92\xd1\x98\x90\x90\x98\x93\x9a\xd1\x9a\x9e\x8d\x8b\x97",
            gcvFALSE
        },
        {
            gcvPATCH_LEANBACK,
            /* "com.google.android.leanbacklauncher" */
            "\x9c\x90\x92\xd1\x98\x90\x90\x98\x93\x9a\xd1\x9e\x91\x9b\x8d\x90\x96\x9b\xd1\x93\x9a\x9e\x91\x9d\x9e\x9c\x94\x93\x9e\x8a\x91\x9c\x97\x9a\x8d",
            gcvFALSE
        },
        {
            gcvPATCH_YOUTUBE_TV,
            /* "com.google.android.youtube.tv" */ "\x9c\x90\x92\xd1\x98\x90\x90\x98\x93\x9a\xd1\x9e\x91\x9b\x8d\x90\x96\x9b\xd1\x86\x90\x8a\x8b\x8a\x9d\x9a\xd1\x8b\x89",
            gcvFALSE
        },
        {
            gcvPATCH_YOUILABS_SHADERTEST,
            /* "com.youilabs.shadertest" */
            "\x9c\x90\x92\xd1\x86\x90\x8a\x96\x93\x9e\x9d\x8c\xd1\x8c\x97\x9e\x9b\x9a\x8d\x8b\x9a\x8c\x8b",
            gcvFALSE
        },
        {
            gcvPATCH_AXX_SAMPLE,
            /* "com.android.glaxx" */
            "\x9c\x90\x92\xd1\x9e\x91\x9b\x8d\x90\x96\x9b\xd1\x98\x93\x9e\x87\x87",
            gcvFALSE
        },
#endif
        {
            gcvPATCH_ANGRYBIRDS,
            /* com.rovio.angrybirds */
            "\x9c\x90\x92\xd1\x8d\x90\x89\x96\x90\xd1\x9e\x91\x98\x8d\x86\x9d\x96\x8d\x9b\x8c",
            gcvFALSE
        },
        {
            gcvPATCH_REALRACING,
            /* com.ea.games.r3_row */
            "\x9c\x90\x92\xd1\x9a\x9e\xd1\x98\x9e\x92\x9a\x8c\xd1\x8d\xcc\xa0\x8d\x90\x88",
            gcvFALSE,
        },
        {
            gcvPATCH_TEMPLERUN,
            /* com.imangi.templerun2 */
            "\x9c\x90\x92\xd1\x96\x92\x9e\x91\x98\x96\xd1\x8b\x9a\x92\x8f\x93\x9a\x8d\x8a\x91\xcd",
            gcvFALSE
        },
        {
            gcvPATCH_SBROWSER,
            /* com.sec.android.app.sbrowser */
            "\x9c\x90\x92\xd1\x8c\x9a\x9c\xd1\x9e\x91\x9b\x8d\x90\x96\x9b\xd1\x9e\x8f\x8f\xd1\x8c\x9d\x8d\x90\x88\x8c\x9a\x8d",
            gcvFALSE
        },
        {
            gcvPATCH_CLASHOFCLAN,
            /* com.supercell.clashofclans */
            "\x9c\x90\x92\xd1\x8c\x8a\x8f\x9a\x8d\x9c\x9a\x93\x93\xd1\x9c\x93\x9e\x8c\x97\x90\x99\x9c\x93\x9e\x91\x8c",
            gcvFALSE
        },
        {
            gcvPATCH_3DMARKSS,
            /* com.futuremark.dmandroid.slingshot:workload */
            "\x9c\x90\x92\xd1\x99\x8a\x8b\x8a\x8d\x9a\x92\x9e\x8d\x94\xd1\x9b\x92\x9e\x91\x9b\x8d\x90\x96\x9b\xd1\x8c\x93\x96\x91\x98\x8c\x97\x90\x8b\xc5\x88\x90\x8d\x94\x93\x90\x9e\x9b",
            gcvFALSE
        },



    };

#if defined(ANDROID)
    gcsSYMBOLSLIST programList[] =
    {
        {
            gcvPATCH_GLBM21,
            {
 "\xb5\x9e\x89\x9e\xa0\x9c\x90\x92\xa0\x98\x93\x9d\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xa0\x9c\x90\x92\x92\x90\x91\xa0\xb8\xb3\xbd\xb1\x9e\x8b\x96\x89\x9a\xb9\x8a\x91\x9c\x8b\x96\x90\x91\x8c\xa0\x92\x9e\x96\x91",
 "\xa0\xa5\xb1\xce\xc7\xab\xba\xac\xab\xa0\xb6\xb1\xab\xba\xad\xb9\xbe\xbc\xba\xa0\xcd\xa0\xce\xce\xcf\xbc\x8d\x9a\x9e\x8b\x9a\xab\x9a\x8c\x8b\xba\xad\xb4\xc9\xab\x9a\x8c\x8b\xb6\x9b",
 "\xa0\xa5\xb1\xce\xc7\xab\xba\xac\xab\xa0\xb6\xb1\xab\xba\xad\xb9\xbe\xbc\xba\xa0\xcd\xa0\xce\xce\xcc\xac\x9a\x8b\xad\x9a\x8c\x90\x93\x8a\x8b\x96\x90\x91\xba\x96\x96",
 "\xa0\xa5\xb1\xb4\xcc\xb8\xb3\xbd\xce\xce\xbe\x8f\x8f\x93\x96\x9c\x9e\x8b\x96\x90\x91\xcd\xc7\xbb\x9a\x99\x9e\x8a\x93\x8b\xb0\x91\x8c\x9c\x8d\x9a\x9a\x91\xa9\x96\x9a\x88\x8f\x90\x8d\x8b\xa8\x96\x9b\x8b\x97\xba\x89",
 "\xa0\xa5\xb1\xcc\xb8\xb3\xbd\xc8\xab\x9a\x87\x8b\x8a\x8d\x9a\xce\xca\x8c\x9a\x8b\xb2\x96\x91\xb2\x9e\x98\xb9\x96\x93\x8b\x9a\x8d\xba\x95\x95",
 "\xa0\xa5\xb1\xcc\xb8\xb3\xbd\xc9\xb0\xac\xb6\x92\x8f\x93\xce\xcd\xb8\x9a\x8b\xb8\x93\x90\x9d\x9e\x93\xb9\xbd\xb0\xba\x89",
 "\xa0\xa5\xb1\xcc\xb8\xb3\xbd\xcc\xb9\xbd\xb0\xce\xca\x96\x91\x96\x8b\xa0\x9b\x9a\x8f\x8b\x97\xa0\x90\x91\x93\x86\xba\x95\x95",
 "\xa0\xa5\xb1\xcc\xb8\xb3\xbd\xce\xce\xbe\x8f\x8f\x93\x96\x9c\x9e\x8b\x96\x90\x91\xce\xcf\xbc\x93\x9a\x9e\x8d\xab\x9a\x8c\x8b\x8c\xba\x89",
 "\xa0\xa5\xb1\xcc\xb8\xb3\xbd\xce\xcf\xae\x8a\x9e\x8b\x9a\x8d\x91\x96\x90\x91\xce\xcc\x99\x8d\x90\x92\xbe\x91\x98\x93\x9a\xbe\x87\x96\x8c\xba\x99\xad\xb4\xb1\xac\xa0\xc7\xa9\x9a\x9c\x8b\x90\x8d\xcc\xbb\xba",
            }
        },

        {
            gcvPATCH_GLBM25,
            {
 "\xb5\x9e\x89\x9e\xa0\x9c\x90\x92\xa0\x98\x93\x9d\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xa0\x9c\x90\x92\x92\x90\x91\xa0\xb8\xb3\xbd\xb1\x9e\x8b\x96\x89\x9a\xb9\x8a\x91\x9c\x8b\x96\x90\x91\x8c\xa0\x92\x9e\x96\x91",
 "\xb5\x9e\x89\x9e\xa0\x9c\x90\x92\xa0\x98\x93\x9d\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xa0\x9c\x90\x92\x92\x90\x91\xa0\xb8\xb3\xbd\xb1\x9e\x8b\x96\x89\x9a\xb9\x8a\x91\x9c\x8b\x96\x90\x91\x8c\xa0\xb6\x8c\xbd\x9e\x8b\x8b\x9a\x8d\x86\xab\x9a\x8c\x8b\xaa\x8f\x93\x90\x9e\x9b\x9e\x9d\x93\x9a",
 "\xab\x8d\x96\x9e\x91\x98\x93\x9a\xab\x9a\x87\xb9\x8d\x9e\x98\x92\x9a\x91\x8b\xb3\x96\x8b\xab\x9a\x8c\x8b\xa0\x99\x8d\x9e\x98\x92\x9a\x91\x8b\xac\x97\x9e\x9b\x9a\x8d",
 "\xa0\xa5\xb1\xcd\xcb\xab\x8d\x96\x9e\x91\x98\x93\x9a\xab\x9a\x87\xa9\x9a\x8d\x8b\x9a\x87\xb3\x96\x8b\xab\x9a\x8c\x8b\xce\xc6\x98\x9a\x8b\xaa\x91\x96\x99\x90\x8d\x92\xb3\x90\x9c\x9e\x8b\x96\x90\x91\x8c\xba\x89",
            }
        },

        {
            gcvPATCH_GLBM27,
            {
 "\xa0\xa5\xb1\xcc\xb8\xb3\xbd\xce\xcc\xb8\xb3\xbd\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xcd\xc8\xbc\xce\xba\x89",
 "\xa0\xa5\xb1\xcc\xb8\xb3\xbd\xce\xcc\xb8\xb3\xbd\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xcd\xc8\xbc\xcd\xba\x89",
 "\xa0\xa5\xb1\xcc\xb8\xb3\xbd\xce\xcc\xb8\xb3\xbd\x9a\x91\x9c\x97\x92\x9e\x8d\x94\xcd\xc8\xce\xcf\xbc\x8d\x9a\x9e\x8b\x9a\xab\x9a\x8c\x8b\xba\x96",
 "\xab\x8d\x96\x9e\x91\x98\x93\x9a\xab\x9a\x87\xb9\x8d\x9e\x98\x92\x9a\x91\x8b\xb3\x96\x8b\xab\x9a\x8c\x8b\xa0\x99\x8d\x9e\x98\x92\x9a\x91\x8b\xac\x97\x9e\x9b\x9a\x8d",
            }
        },

        {
            gcvPATCH_GFXBENCH,
            {
 "\xa0\xa5\xb1\xce\xce\xbd\x9e\x8b\x8b\x9a\x8d\x86\xab\x9a\x8c\x8b\xbc\xcd\xba\xad\xb4\xce\xcb\xab\x9a\x8c\x8b\xbb\x9a\x8c\x9c\x8d\x96\x8f\x8b\x90\x8d\xaf\xb1\xcc\xb8\xb3\xbd\xc7\xab\x9a\x8c\x8b\xbd\x9e\x8c\x9a\xba\x95",
 "\xa0\xa5\xb1\xce\xce\xab\x9a\x8c\x8b\xa8\x8d\x9e\x8f\x8f\x9a\x8d\xbc\xce\xba\xad\xb4\xce\xcb\xab\x9a\x8c\x8b\xbb\x9a\x8c\x9c\x8d\x96\x8f\x8b\x90\x8d\xaf\xb1\xcc\xb8\xb3\xbd\xc7\xab\x9a\x8c\x8b\xbd\x9e\x8c\x9a\xba",
 "\xa0\xa5\xb1\xce\xcd\xae\x8a\x9e\x93\x96\x8b\x86\xb2\x9e\x8b\x9c\x97\xbc\xce\xba\xad\xb4\xce\xcb\xab\x9a\x8c\x8b\xbb\x9a\x8c\x9c\x8d\x96\x8f\x8b\x90\x8d\xaf\xb1\xcc\xb8\xb3\xbd\xc7\xab\x9a\x8c\x8b\xbd\x9e\x8c\x9a\xba",
 "\xa0\xa5\xb1\xce\xcc\xb8\xb3\xbd\xa0\xac\x9c\x9a\x91\x9a\xa0\xba\xac\xcc\xce\xcc\xb2\x90\x89\x9a\xaf\x9e\x8d\x8b\x96\x9c\x93\x9a\x8c\xba\x89",
 "\xa0\xa5\xb1\xce\xca\xbc\xaf\xaa\xb0\x89\x9a\x8d\x97\x9a\x9e\x9b\xab\x9a\x8c\x8b\xbc\xce\xba\xad\xb4\xce\xcb\xab\x9a\x8c\x8b\xbb\x9a\x8c\x9c\x8d\x96\x8f\x8b\x90\x8d",
 "\xa0\xa5\xb1\xcc\xb8\xb3\xbd\xce\xcc\xb8\xb3\xbd\xab\x9a\x87\x8b\x8a\x8d\x9a\xba\xac\xcd\xbc\xce\xba\xaf\xb4\xb1\xcc\xb4\xbc\xb3\xca\xb6\x92\x9e\x98\x9a\xba\xb1\xac\xce\xa0\xce\xcd\xa0\xab\x9a\x87\x8b\x8a\x8d\x9a\xab\x86\x8f\x9a\xba\x9d",
 "\xa0\xa5\xb1\xcc\xb8\xb3\xbd\xcc\xb9\xbd\xb0\xce\xc7\xb8\x9a\x8b\xac\x9c\x8d\x9a\x9a\x91\x8c\x97\x90\x8b\xb6\x92\x9e\x98\x9a\xba\xad\xb1\xcc\xb4\xbc\xb3\xca\xb6\x92\x9e\x98\x9a\xba",
            }
        },

        /* Support GFXBench 3.0.16 */
        {
            gcvPATCH_GFXBENCH,
            {
 "\xb5\x9e\x89\x9e\xa0\x91\x9a\x8b\xa0\x94\x96\x8c\x97\x90\x91\x8b\x96\xa0\x8c\x88\x96\x98\xa0\x8b\x9a\x8c\x8b\x99\x88\xb5\xb1\xb6\xa0\xba\xb8\xb3\xb8\x8d\x9e\x8f\x97\x96\x9c\x8c\xbc\x90\x91\x8b\x9a\x87\x8b\xa0\xce\x89\x9a\x8d\x8c\x96\x90\x91\xac\x8a\x8f\x8f\x90\x8d\x8b\x9a\x9b",
 "\xa0\xa5\xb1\xc7\x94\x96\x8c\x97\x90\x91\x8b\x96\xc8\x9e\x91\x9b\x8d\x90\x96\x9b\xc7\xaf\x93\x9e\x8b\x99\x90\x8d\x92\xce\xcd\x9d\x9e\x8b\x8b\x9a\x8d\x86\xb3\x9a\x89\x9a\x93\xba\x89",
 "\xa0\xa5\xac\x8b\xcb\x9a\x91\x9b\x93\xb6\x9c\xac\x8b\xce\xce\x9c\x97\x9e\x8d\xa0\x8b\x8d\x9e\x96\x8b\x8c\xb6\x9c\xba\xba\xad\xac\x8b\xce\xcc\x9d\x9e\x8c\x96\x9c\xa0\x90\x8c\x8b\x8d\x9a\x9e\x92\xb6\xab\xa0\xab\xcf\xa0\xba\xac\xc9\xa0",
 "\xb5\x9e\x89\x9e\xa0\x91\x9a\x8b\xa0\x94\x96\x8c\x97\x90\x91\x8b\x96\xa0\x8c\x88\x96\x98\xa0\x8b\x9a\x8c\x8b\x99\x88\xb5\xb1\xb6\xa0\xab\x9a\x8c\x8b\xb9\x9e\x9c\x8b\x90\x8d\x86\xa0\xce\x9c\x8d\x9a\x9e\x8b\x9a\xa0\xce\x8b\x9a\x8c\x8b",
 "\x9c\x8d\x9a\x9e\x8b\x9a\xa0\x8b\x9a\x8c\x8b\xa0\x98\x93\xa0\x9a\x98\x86\x8f\x8b",
 "\x9c\x8d\x9a\x9e\x8b\x9a\xa0\x8b\x9a\x8c\x8b\xa0\x98\x93\xa0\x9b\x8d\x96\x89\x9a\x8d",
 "\x9c\x8d\x9a\x9e\x8b\x9a\xa0\x8b\x9a\x8c\x8b\xa0\x98\x93\xa0\x9e\x93\x8a",
 "\x9c\x8d\x9a\x9e\x8b\x9a\xa0\x8b\x9a\x8c\x8b\xa0\x98\x93\xa0\x8b\x8d\x9a\x87",
 "\x9c\x8d\x9a\x9e\x8b\x9a\xa0\x8b\x9a\x8c\x8b\xa0\x98\x93\xa0\x92\x9e\x91\x97\x9e\x8b\x8b\x9e\x91\xcc\xce",
 "\x9c\x8d\x9a\x9e\x8b\x9a\xa0\x8b\x9a\x8c\x8b\xa0\x98\x93\xa0\x92\x9e\x91\x97\x9e\x8b\x8b\x9e\x91",
            }
        },

        {
            gcvPATCH_BM21,
            {
 "\x9c\x90\x93\x93\x9a\x9c\x8b\xa0\x8a\x91\x96\x99\x96\x9a\x9b\xa0\x8c\x97\x9e\x9b\x9a\x8d\xa0\x8b\x9a\x8c\x8b\xa0\x8f\x9e\x8d\x9e\x92\x9a\x8b\x9a\x8d\x8c\xa0\x99\x8d\x90\x92\xa0\x93\x8a\x9e",
 "\x8c\x9a\x8b\x8a\x8f\xa0\x8b\x8d\x96\x9e\x91\x98\x93\x9a\xa0\x9c\x90\x8a\x91\x8b\xa0\x8b\x9a\x8c\x8b\xa0\x8d\x9a\x91\x9b\x9a\x8d\x8c\x8b\x9e\x8b\x9a",
 "\x90\x9a\x8c\xa0\x8c\x9c\x9a\x91\x9a\xa0\x8f\x93\x9e\x86\x9a\x8d\xa0\x9d\x9a\x98\x96\x91\xa0\x8d\x9a\x91\x9b\x9a\x8d\xa0\x8f\x9e\x8c\x8c",
 "\x92\x9e\x94\x9a\xa0\x9e\x9b\x89\x9e\x91\x9c\x9a\x9b\xa0\x88\x90\x8d\x93\x9b\xa0\x8b\x9a\x8c\x8b\xa0\x98\x9a\x90\x92\x9a\x8b\x8d\x86",
 "\x8a\x8f\x9b\x9e\x8b\x9a\xa0\x8b\x8d\x96\x9e\x91\x98\x93\x9a\xa0\x9c\x90\x8a\x91\x8b\xa0\x8b\x9a\x8c\x8b",
 "\xa0\xa5\xcd\xcc\x8c\x8b\x8d\x9c\x90\x91\x89\xa0\x96\x8c\xa0\x97\x96\x9d\x96\x8b\xa0\x8c\x8b\x8d\x96\x91\x98\xaf\xb4\x9c",
 "\x90\x9a\x8c\xa0\x8c\x9c\x9a\x91\x9a\xa0\x8f\x93\x9e\x86\x9a\x8d\xa0\x9c\x8d\x9a\x9e\x8b\x9a",
            }
        },

        {
            gcvPATCH_MM07,
            {
 "\x9c\x90\x93\x93\x9a\x9c\x8b\xa0\x8a\x91\x96\x99\x96\x9a\x9b\xa0\x8c\x97\x9e\x9b\x9a\x8d\xa0\x8b\x9a\x8c\x8b\xa0\x8f\x9e\x8d\x9e\x92\x9a\x8b\x9a\x8d\x8c\xa0\x99\x8d\x90\x92\xa0\x93\x8a\x9e",
 "\x8c\x9a\x8b\x8a\x8f\xa0\x8b\x8d\x96\x9e\x91\x98\x93\x9a\xa0\x9c\x90\x8a\x91\x8b\xa0\x8b\x9a\x8c\x8b\xa0\x8d\x9a\x91\x9b\x9a\x8d\x8c\x8b\x9e\x8b\x9a",
 "\x90\x9a\x8c\xa0\x8c\x9c\x9a\x91\x9a\xa0\x8f\x93\x9e\x86\x9a\x8d\xa0\x9d\x9a\x98\x96\x91\xa0\x8d\x9a\x91\x9b\x9a\x8d\xa0\x8f\x9e\x8c\x8c",
 "\x92\x9e\x94\x9a\xa0\x9e\x9b\x89\x9e\x91\x9c\x9a\x9b\xa0\x88\x90\x8d\x93\x9b\xa0\x8b\x9a\x8c\x8b\xa0\x98\x9a\x90\x92\x9a\x8b\x8d\x86",
 "\x8a\x8f\x9b\x9e\x8b\x9a\xa0\x8b\x8d\x96\x9e\x91\x98\x93\x9a\xa0\x9c\x90\x8a\x91\x8b\xa0\x8b\x9a\x8c\x8b",
 "\x90\x9a\x8c\xa0\x8c\x9c\x9a\x91\x9a\xa0\x8f\x93\x9e\x86\x9a\x8d\xa0\x9c\x8d\x9a\x9e\x8b\x9a",
 "\x8c\x96\xa0\x90\x8f\x9a\x91\xa0\x9c\x90\x91\x8c\x90\x93\x9a",
 "\x8c\x8b\x8d\x9c\x90\x91\x89\xa0\x96\x8c\xa0\x97\x96\x9d\x96\x8b\xa0\x8c\x8b\x8d\x96\x91\x98",
            }
        },

        {
            gcvPATCH_MM06,
            {
 "\x8c\x9c\x9a\x91\x9a\xa0\x8f\x93\x9e\x86\x9a\x8d\xa0\x8c\x9a\x8b\xa0\x98\x9e\x92\x9a\x8b\x9a\x8c\x8b",
 "\x89\x96\x9a\x88\xa0\x9c\x9e\x9c\x97\x9a\x9b\xa0\x9e\x93\x8f\x97\x9e\xa0\x8b\x9a\x8c\x8b",
 "\x8c\x96\xa0\x9b\x9a\x99\x9e\x8a\x93\x8b\xa0\x9b\x96\x8c\x8f\x93\x9e\x86\xa0\x9c\x90\x91\x99\x96\x98\x8a\x8d\x9e\x8b\x96\x90\x91",
 "\xa0\xa5\xce\xc6\x9a\x87\x8b\x9a\x91\x8c\x96\x90\x91\xa0\x8c\x8a\x8f\x8f\x90\x8d\x8b\x9a\x9b\xaf\xb4\x9c",
 "\xa0\xa5\xcd\xc7\x8f\x9e\x8d\x8c\x9a\xa0\x9c\x90\x92\x92\x9e\x91\x9b\xa0\x93\x96\x91\x9a\xa0\x99\x8d\x90\x92\xa0\x92\x9e\x96\x91\x96\xaf\xaf\x9c",
 "\x98\x9a\x90\x92\x9a\x8b\x8d\x86\xa0\x9c\x8d\x9a\x9e\x8b\x9a\xa0\x99\x8a\x93\x93\x8c\x9c\x8d\x9a\x9a\x91\xa0\x8b\x8d\x96\x9e\x91\x98\x93\x9a\x8c",
 "\x93\x96\x98\x97\x8b\xa0\x8c\x9a\x8b\xa0\x9b\x96\x99\x99\x8a\x8c\x9a\xa0\x8c\x8f\x9a\x9c\x8a\x93\x9e\x8d",
            }
        },

        {
            gcvPATCH_BMGUI,
            {
 "\x9d\x9a\x8c\x8b\xac\x9c\x90\x8d\x9a\xac\x9c\x9a\x91\x9a\xb2\x9a\x91\x8a\xbd\xb8\xac\x8b\x8d\x96\x8f\xb2\x9e\x8b\x9a\x8d\x96\x9e\x93\x8c",
 "\x9d\x99\xb2\x9e\x8b\x9a\x8d\x96\x9e\x93\xbc\x90\x93\x93\x9a\x9c\x8b\x96\x90\x91\xb8\x9a\x8b\xbc\x90\x91\x99\x96\x98\x8a\x8d\x9e\x8b\x96\x90\x91\xaf\x9e\x8d\x9e\x92\x9a\x8b\x9a\x8d\x8c\xa0\x96\x91\x8b\x9a\x8d\x91\x9e\x93",
 "\x98\x9a\x90\x92\x9a\x8b\x8d\x86\xbc\x97\x8a\x91\x94\xb6\x91\x96\x8b\x96\x9e\x93\x96\x85\x9a\xb8\x9a\x90\x92\x9a\x8b\x8d\x86\xb9\x8d\x90\x92\xac\x8b\x8d\x9a\x9e\x92\xbd\x8a\x99\x99\x9a\x8d\x8c",
 "\x94\x85\x9e\xbe\x8f\x8f\x93\x96\x9c\x9e\x8b\x96\x90\x91\xac\x9a\x8b\xbd\x90\x8a\x91\x9b\x96\x91\x98\xbd\x90\x87\xa9\x96\x8c\x8a\x9e\x93\x96\x85\x9e\x8b\x96\x90\x91\xba\x91\x9e\x9d\x93\x9a\x9b",
 "\x94\x85\x9c\xbb\x86\x91\x9e\x92\x96\x9c\xbe\x8d\x8d\x9e\x86\xb2\x8a\x8b\x9e\x9d\x93\x9a\xb6\x8b\x9a\x8d\x9e\x8b\x90\x8d\xb8\x9a\x8b\xa9\x9e\x93\x8a\x9a\xa0\x8f\x8d\x96\x89\x9e\x8b\x9a",
 "\x94\x85\x8a\xbe\x87\x96\x8c\xbe\x93\x96\x98\x91\x9a\x9b\xbd\x90\x8a\x91\x9b\x96\x91\x98\xbd\x90\x87\xb9\x8d\x90\x92\xab\x8d\x9e\x91\x8c\x99\x90\x8d\x92\x9a\x9b\xbe\xbe\xbd\xbd",
 "\x92\x9e\x96\x91\xbe\x8f\x8f\x93\x96\x9c\x9e\x8b\x96\x90\x91\xaa\x96\xba\x89\x9a\x91\x8b\xb7\x9e\x91\x9b\x93\x9a\x8d",
 "\x8d\x9a\x9d\x8a\x96\x93\x9b\xaf\x93\x9e\x8b\x8b\x9a\x8d\xbe\x91\x9b\xbe\x8c\x8c\x96\x98\x91\xba\x91\x8b\x8d\x86\xbe\x91\x96\x92\x9e\x8b\x96\x90\x91",
 "\x8c\x9c\x8d\x9a\x9a\x91\xbc\x9e\x8f\x8b\x8a\x8d\x9a\xac\x9a\x8b\x8b\x96\x91\x98\x8c\xb9\x90\x8d\x88\x9e\x8d\x9b\xb1\x9a\x87\x8b\xb6\x91\x9b\x9a\x87",
 "\x8c\x90\x8d\x8b\xac\x9a\x8e\x8a\x9a\x91\x9c\x9a\xaf\x90\x96\x91\x8b\x9a\x8d\x8c\xbe\x9c\x9c\x90\x8d\x9b\x96\x91\x98\xab\x90\xac\x96\x85\x9a\xb3\x9e\x8d\x98\x9a\x8c\x8b\xb9\x96\x8d\x8c\x8b",
            }
        },

        {
            gcvPATCH_FISHNOODLE,
            {
 "\xa0\xa5\xce\xcf\xba\xb8\xb3\xba\x87\x8b\xb6\x91\x96\x8b\xaf\xc8\xa0\xb5\xb1\xb6\xba\x91\x89",
 "\xa0\xa5\xce\xce\xb8\xb3\xcd\xcf\xb5\xb1\xb6\xb6\x91\x96\x8b\xaf\xc8\xa0\xb5\xb1\xb6\xba\x91\x89",
 "\xa0\xa5\xce\xce\xaa\x8b\x96\x93\x96\x8b\x86\xb6\x91\x96\x8b\xaf\xc8\xa0\xb5\xb1\xb6\xba\x91\x89",
 "\xa0\xa5\xc7\xba\xab\xbc\xce\xb6\x91\x96\x8b\xaf\xc8\xa0\xb5\xb1\xb6\xba\x91\x89",
 "\xa0\xa5\xb1\xc8\xa0\xb5\xb1\xb6\xba\x91\x89\xce\xcc\xbc\x9e\x93\x93\xb6\x91\x8b\xb2\x9a\x8b\x97\x90\x9b\xba\xaf\xc7\xa0\x95\x90\x9d\x95\x9a\x9c\x8b\xaf\xce\xcf\xa0\x95\x92\x9a\x8b\x97\x90\x9b\xb6\xbb\x85",
 "\xa0\xa5\xb1\xc8\xa0\xb5\xb1\xb6\xba\x91\x89\xce\xc9\xbc\x9e\x93\x93\xb0\x9d\x95\x9a\x9c\x8b\xb2\x9a\x8b\x97\x90\x9b\xba\xaf\xc7\xa0\x95\x90\x9d\x95\x9a\x9c\x8b\xaf\xce\xcf\xa0\x95\x92\x9a\x8b\x97\x90\x9b\xb6\xbb\x85",
 "\xa0\xa5\xb1\xc8\xa0\xb5\xb1\xb6\xba\x91\x89\xce\xc6\xbc\x9e\x93\x93\xac\x8b\x9e\x8b\x96\x9c\xb6\x91\x8b\xb2\x9a\x8b\x97\x90\x9b\xba\xaf\xc8\xa0\x95\x9c\x93\x9e\x8c\x8c\xaf\xce\xcf\xa0\x95\x92\x9a\x8b\x97\x90\x9b\xb6\xbb\x85",
 "\xa0\xa5\xb1\xc8\xa0\xb5\xb1\xb6\xba\x91\x89\xcd\xcf\xbc\x9e\x93\x93\xac\x8b\x9e\x8b\x96\x9c\xb3\x90\x91\x98\xb2\x9a\x8b\x97\x90\x9b\xba\xaf\xc8\xa0\x95\x9c\x93\x9e\x8c\x8c\xaf\xce\xcf\xa0\x95\x92\x9a\x8b\x97\x90\x9b\xb6\xbb\x85",
 "\xa0\xa5\xb1\xc8\xa0\xb5\xb1\xb6\xba\x91\x89\xcd\xcd\xbc\x9e\x93\x93\xac\x8b\x9e\x8b\x96\x9c\xb0\x9d\x95\x9a\x9c\x8b\xb2\x9a\x8b\x97\x90\x9b\xba\xaf\xc8\xa0\x95\x9c\x93\x9e\x8c\x8c\xaf\xce\xcf\xa0\x95\x92\x9a\x8b\x97\x90\x9b\xb6\xbb\x85",
 "\xa0\xa5\xb1\xc8\xa0\xb5\xb1\xb6\xba\x91\x89\xc7\xab\x97\x8d\x90\x88\xb1\x9a\x88\xba\xaf\xc8\xa0\x95\x9c\x93\x9e\x8c\x8c\xaf\xb4\x9c",
            }
        },

        /* Symbol list for Antutu 3.x */
        {
            gcvPATCH_ANTUTU,
            {
 "\xa0\xa5\xb1\xce\xcf\xa0\xa0\x9c\x87\x87\x9e\x9d\x96\x89\xce\xcd\xcc\xa0\xa0\x99\x8a\x91\x9b\x9e\x92\x9a\x91\x8b\x9e\x93\xa0\x8b\x86\x8f\x9a\xa0\x96\x91\x99\x90\xbb\xcd\xba\x89",
 "\xa0\xa5\xb1\xce\xcf\xa0\xa0\x9c\x87\x87\x9e\x9d\x96\x89\xce\xce\xc6\xa0\xa0\x8f\x90\x96\x91\x8b\x9a\x8d\xa0\x8b\x86\x8f\x9a\xa0\x96\x91\x99\x90\xbb\xcd\xba\x89",
 "\xa0\xa5\xb1\xce\xcf\xa0\xa0\x9c\x87\x87\x9e\x9d\x96\x89\xce\xce\xc8\xa0\xa0\x9c\x93\x9e\x8c\x8c\xa0\x8b\x86\x8f\x9a\xa0\x96\x91\x99\x90\xbb\xce\xba\x89",
 "\xa0\xa5\xb1\xcc\xb9\xcc\xbb\xcc\xb9\x90\x98\xce\xcc\x8c\x9a\x8b\xb9\x90\x98\xbb\x9a\x91\x8c\x96\x8b\x86\xba\x99",
 "\xa0\xa5\xb1\xcc\xb9\xcc\xbb\xcb\xb9\x90\x91\x8b\xce\xcf\x9b\x8d\x9e\x88\xac\x8b\x8d\x96\x91\x98\xba\x96\x96\x96\x96\xaf\xb4\x9c\xce\xcf\xbb\x8d\x9e\x88\xbe\x91\x9c\x97\x90\x8d",
 "\xa0\xa5\xb1\xcc\xb9\xcc\xbb\xca\xb6\x92\x9e\x98\x9a\xce\xcd\x99\x9a\x8b\x9c\x97\xaf\x9e\x93\x93\x9a\x8b\x9a\xba\xaf\xc8\xa0\xa0\x8c\xb9\xb6\xb3\xba\xaf\xca\xbc\x90\x93\x90\x8d\x96",
 "\xa0\xa5\xb1\xb4\xce\xcf\xa0\xa0\x9c\x87\x87\x9e\x9d\x96\x89\xce\xcd\xce\xa0\xa0\x89\x92\x96\xa0\x9c\x93\x9e\x8c\x8c\xa0\x8b\x86\x8f\x9a\xa0\x96\x91\x99\x90\xce\xcd\xa0\xa0\x9b\x90\xa0\x9b\x86\x91\x9c\x9e\x8c\x8b\xba\x96\xb1\xac\xa0\xce\xc8\xa0\xa0\x9c\x93\x9e\x8c\x8c\xa0\x8b\x86\x8f\x9a\xa0\x96\x91\x99\x90\xce\xcf\xa0\xa0\x8c\x8a\x9d\xa0\x94\x96\x91\x9b\xba\xaf\xb4\xac\xce\xa0\xaf\xb4\x89\xac\xcb\xa0\xac\xc9\xa0\xad\xb1\xac\xce\xa0\xce\xc9\xa0\xa0\x9b\x86\x91\x9c\x9e\x8c\x8b\xa0\x8d\x9a\x8c\x8a\x93\x8b\xba",
 "\xa0\xa5\xb1\xb4\xce\xcf\xa0\xa0\x9c\x87\x87\x9e\x9d\x96\x89\xce\xce\xc8\xa0\xa0\x9c\x93\x9e\x8c\x8c\xa0\x8b\x86\x8f\x9a\xa0\x96\x91\x99\x90\xcd\xcf\xa0\xa0\x9b\x90\xa0\x99\x96\x91\x9b\xa0\x8f\x8a\x9d\x93\x96\x9c\xa0\x8c\x8d\x9c\xba\x96\xaf\xb4\x89\xaf\xb4\xac\xcf\xa0\xac\xcd\xa0",
 "\xa0\xa5\xac\x8b\xce\xcb\xa0\xa0\x9c\x90\x91\x89\x9a\x8d\x8b\xa0\x8b\x90\xa0\x89\xb6\x9b\xba\x89\xaf\xb4\x9c\xad\xab\xa0\xad\xac\x8b\xce\xcd\xa0\xb6\x90\x8c\xa0\xb6\x90\x8c\x8b\x9e\x8b\x9a\xad\xb4\xaf\x96",
            }
        },

        /* Symbol list for Antutu 4.x */
        {
            gcvPATCH_ANTUTU4X,
            {
 "\x9d\x9a\x91\x9c\x97\xa0\x9b\x9e\x8b\x9e\xa0\x8f\x8d\x90\x9c\x9a\x8c\x8c\x96\x91\x98",
 "\x9e\x91\x9b\x8d\x90\x96\x9b\xa0\x98\x9a\x8b\xbc\x8f\x8a\xb6\x9b\xbe\x8d\x92",
 "\x9d\x9a\x91\x9c\x97\xa0\x8c\x9c\x90\x8d\x9a\xa0\x97\x86\x9d\x8d\x96\x9b",
 "\x9b\x9a\x9c\xa0\x8c\x8b\x8d\x96\x91\x98\xa0\x90\x8f\x9a\x91\x98\x93\x9a\x8c\xcc",
 "\x9b\x9a\x8c\xa0\x9a\x91\x9c\x8d\x86\x8f\x8b\x96\x90\x91",
 "\x98\x9a\x91\x9a\x8d\x9e\x8b\x9a\xa0\x8b\x9a\x8c\x8b\xa0\x9b\x9e\x8b\x9e\xa0\x99\x96\x93\x9a",
 "\xb5\x9e\x89\x9e\xa0\x9c\x90\x92\xa0\x9d\x9e\x9b\x93\x90\x98\x96\x9c\xa0\x98\x9b\x87\xa0\x98\x8d\x9e\x8f\x97\x96\x9c\x8c\xa0\x98\xcd\x9b\xa0\xb8\x9b\x87\xcd\xbb\xaf\x96\x87\x92\x9e\x8f\xa0\x9b\x8d\x9e\x88\xbc\x96\x8d\x9c\x93\x9a",
 "\xa0\xa5\xab\xa9\xb1\xce\xcf\xa0\xa0\x9c\x87\x87\x9e\x9d\x96\x89\xce\xcd\xcc\xa0\xa0\x99\x8a\x91\x9b\x9e\x92\x9a\x91\x8b\x9e\x93\xa0\x8b\x86\x8f\x9a\xa0\x96\x91\x99\x90\xba",
            }
        },

        /* Symbol list for Antutu 5.x */
        {
            gcvPATCH_ANTUTU5X,
            {
 "\x9d\x9a\x91\x9c\x97\xa0\x9b\x9e\x8b\x9e\xa0\x8f\x8d\x90\x9c\x9a\x8c\x8c\x96\x91\x98",
 "\x9e\x91\x9b\x8d\x90\x96\x9b\xa0\x98\x9a\x8b\xbc\x8f\x8a\xb6\x9b\xbe\x8d\x92",
 "\x9d\x9a\x91\x9c\x97\xa0\x8c\x9c\x90\x8d\x9a\xa0\x97\x86\x9d\x8d\x96\x9b",
 "\x9b\x9a\x9c\xa0\x8c\x8b\x8d\x96\x91\x98\xa0\x90\x8f\x9a\x91\x98\x93\x9a\x8c\xcc",
 "\x9b\x9a\x8c\xa0\x9a\x91\x9c\x8d\x86\x8f\x8b\x96\x90\x91",
 "\x98\x9a\x91\x9a\x8d\x9e\x8b\x9a\xa0\x8b\x9a\x8c\x8b\xa0\x9b\x9e\x8b\x9e\xa0\x99\x96\x93\x9a",
 "\xb5\x9e\x89\x9e\xa0\x9c\x90\x92\xa0\x9e\x91\x8b\x8a\x8b\x8a\xa0\xbe\xbd\x9a\x91\x9c\x97\xb2\x9e\x8d\x94\xa0\xb5\xb1\xb6\xb3\xb6\xbd\xa0\x9c\x97\x9a\x9c\x94\xaf\x9e\x9c\x94\x9e\x98\x9a",
 "\xb5\x9e\x89\x9e\xa0\x9c\x90\x92\xa0\x9e\x91\x8b\x8a\x8b\x8a\xa0\xbe\xbd\x9a\x91\x9c\x97\xb2\x9e\x8d\x94\xa0\xb5\xb1\xb6\xb3\xb6\xbd\xa0\x8c\x8a\x9d\x92\x96\x8b\xc9\xcb\x9d\x96\x8b",
            }
        },

        {
            gcvPATCH_NENAMARK2,
            {
 "\xa0\xa5\xce\xcc\x91\x9a\x88\xb5\x9e\x89\x9e\xb0\x9d\x95\x9a\x9c\x8b\xaf\xc8\xa0\xb5\xb1\xb6\xba\x91\x89\xaf\xc8\xa0\x95\x9c\x93\x9e\x8c\x8c\xaf\xce\xcf\xa0\x95\x92\x9a\x8b\x97\x90\x9b\xb6\xbb\x85",
 "\xa0\xa5\xce\xca\x8c\x9a\x8b\xb8\x93\x90\x9d\x9e\x93\xb3\x90\x98\x98\x9a\x8d\xaf\xb9\x89\xaf\x89\xc7\xb3\x90\x98\xb3\x9a\x89\x9a\x93\xaf\xb4\x9c\xac\xcd\xa0\xac\x8b\xc6\xa0\xa0\x89\x9e\xa0\x93\x96\x8c\x8b\xba\xac\xa0",
 "\xa0\xa5\xcd\xcd\xb1\x9a\x91\x9e\xb2\x9e\x8d\x94\xb5\xb1\xb6\xa0\x90\x91\xb9\x96\x91\x96\x8c\x97\x9a\x9b\xaf\xc8\xa0\xb5\xb1\xb6\xba\x91\x89\xaf\xc7\xa0\x95\x90\x9d\x95\x9a\x9c\x8b\xaf\xce\xcf\xa0\x95\x92\x9a\x8b\x97\x90\x9b\xb6\xbb\x99\xad\xb4\xc8\xb7\x9e\x8c\x97\xb2\x9e\x8f\xb6\xc9\xac\x8b\x8d\x96\x91\x98\xac\xc9\xa0\xba\xad\xb4\xce\xce\xad\x9a\x91\x9b\x9a\x8d\xab\x8d\x9e\x9c\x9a",
 "\xa0\xa5\xc6\x9d\x8a\x96\x93\x9b\xab\x8d\x9a\x9a\xad\xce\xca\xad\x9e\x91\x9b\x90\x92\xb8\x9a\x91\x9a\x8d\x9e\x8b\x90\x8d\xaf\xb4\xce\xcf\xbd\x8d\x9e\x91\x9c\x97\xac\x8f\x9a\x9c\x96\xad\xb4\xc8\xb2\x9e\x8b\xcb\x87\xcb\x99\xad\xc9\xa9\x9a\x9c\x8b\x90\x8d\xb6\xce\xcf\xbd\x8d\x9e\x91\x9c\x97\xb6\x91\x99\x90\xb3\x96\xce\xcf\xba\xba\xad\xac\xc8\xa0\xb6\xac\xcb\xa0\xb3\x96\xce\xcf\xba\xba",
 "\xa0\xa5\xb1\xce\xcf\xb9\x9b\xad\x9a\x8c\x90\x8a\x8d\x9c\x9a\xce\xce\x98\x9a\x8b\xb6\x91\x8b\x9a\x8d\x91\x9e\x93\xba\xaf\x96\xaf\x95\xac\xce\xa0",
 "\xa0\xa5\xb1\xce\xce\xbd\x8a\x8b\x8b\x90\x91\xb8\x8d\x90\x8a\x8f\xcb\x96\x91\x96\x8b\xba\xaf\xc9\xac\x86\x8c\x8b\x9a\x92\xaf\xc6\xb8\xb3\xbc\x90\x91\x8b\x9a\x87\x8b\xad\xb4\xc9\xac\x8b\x8d\x96\x91\x98\xad\xb4\xc9\xa9\x9a\x9c\x8b\x90\x8d\xb6\xce\xcf\xbd\x8a\x8b\x8b\x90\x91\xac\x8f\x9a\x9c\xb3\x96\xce\xcf\xba\xba",
 "\xa0\xa5\xb1\xce\xce\xbd\x8a\x8b\x8b\x90\x91\xb8\x8d\x90\x8a\x8f\xc8\x96\x91\x96\x8b\xac\x9a\x8b\xba\xaf\xc9\xac\x86\x8c\x8b\x9a\x92\xaf\xc6\xb8\xb3\xbc\x90\x91\x8b\x9a\x87\x8b\xad\xb4\xc9\xa9\x9a\x9c\x8b\x90\x8d\xb6\xce\xcf\xbd\x8a\x8b\x8b\x90\x91\xac\x8f\x9a\x9c\xb3\x96\xce\xcf\xba\xba",
 "\xa0\xa5\xb1\xce\xce\xbb\x8a\x92\x92\x86\xac\x86\x8c\x8b\x9a\x92\xcd\xcf\x8d\x9a\x98\x96\x8c\x8b\x9a\x8d\xb6\x92\x9e\x98\x9a\xbb\x9a\x9c\x90\x9b\x9a\x8d\xba\xad\xb4\xc9\xac\x8b\x8d\x96\x91\x98\xaf\xb9\xaf\xce\xcd\xb6\x92\x9e\x98\x9a\xbb\x9a\x9c\x90\x9b\x9a\x8d\xc9\xad\x9a\x99\xaf\x8b\x8d\xb6\xc7\xad\x9a\x8c\x90\x8a\x8d\x9c\x9a\xba\xba",
 "\xa0\xa5\xb1\xce\xce\xb8\xb3\xac\x8b\x9e\x8b\x9a\xb6\x92\x8f\x93\xce\xce\x93\x90\x9e\x9b\xab\x9a\x87\x8b\x8a\x8d\x9a\xba\xaf\xc9\xac\x86\x8c\x8b\x9a\x92\xad\xb4\xc9\xac\x8b\x8d\x96\x91\x98\xce\xcd\xab\x9a\x87\x8b\x8a\x8d\x9a\xb9\x93\x9e\x98\x8c\xac\xcb\xa0",
 "\xa0\xa5\xb1\xce\xce\xac\x9a\x8b\x8a\x8f\xac\x97\x9e\x9b\x9a\x8d\xb6\xb1\xce\xce\xbc\x93\x90\x8a\x9b\xac\x86\x8c\x8b\x9a\x92\xce\xce\xbc\x93\x90\x8a\x9b\xac\x97\x9e\x9b\x9a\x8d\xba\xba\xcb\x9e\x8b\x8b\x8d\xb6\xce\xca\xac\x97\x9e\x9b\x9a\x8d\xbe\x8b\x8b\x8d\x96\x9d\x8a\x8b\x9a\xb6\xb3\x96\xca\xce\xcd\xce\xba\xb3\x96\xcb\xba\xb3\x96\xcf\xba\xba\xba\xba\xad\xac\xcd\xa0\xad\xb4\xc9\xac\x8b\x8d\x96\x91\x98\xad\xab\xa0",
            }
        },

        {
            gcvPATCH_BM3,
            {
 "\x8b\x8d\x86\x9e\x93\x93\x8b\x9e\x9d\x93\x9a\x8c\xa0\xcc\x9d\x96\x8b\x8b\x9e\x9d\x93\x9a\xa0\x9e\x93\x93\xa0\x8c\x8a\x9d\x9d\x93\x90\x9c\x94\x8c\xa0\x8a\x8c\x96\x91\x98\xa0\x8f\x8d\x9a\x9c\x9e\x93\x9c\xa0\x8f\x9a\x8d\x9c\x9a\x8f\x8b\x8a\x9e\x93\xce\xcf\xcf\xcf",
 "\x94\x85\x8a\xba\x91\x98\x96\x91\x9a\xb6\x8c\xbe\x93\x93\x90\x9c\x9e\x8b\x9a\x9b\xac\x96\x85\x9a\xad\x9a\x91\x9b\x9a\x8d\x96\x91\x98\xba\x91\x9e\x9d\x93\x9a\x9b",
 "\x94\x85\x8a\xbb\x96\x8c\x8f\x9e\x8b\x9c\x97\xb2\x9a\x8c\x8c\x9e\x98\x9a\xbe\x9c\x8b\x96\x90\x91\xbe\x9b\x9b\xb0\x9d\x95\x9a\x9c\x8b\xb1\x90\x9b\x9a\xaf\x8d\x90\x8f\x9a\x8d\x8b\x86\xbd\x96\x91\x9b\x96\x91\x98",
 "\x94\x85\x8c\xac\x8a\x8d\x99\x9e\x9c\x9a\xb8\x9a\x8b\xaa\x8c\x9a\x9b\xac\x8a\x8d\x99\x9e\x9c\x9a\xb1\x9e\x8b\x96\x89\x9a\xaf\x8d\x90\x8f\x9a\x8d\x8b\x96\x9a\x8c",
 "\x94\x85\x8c\xb6\x91\x8f\x8a\x8b\xba\x89\x9a\x91\x8b\xae\x8a\x9a\x8a\x9a\xb6\x8b\x9a\x8d\x9e\x8b\x90\x8d\xb8\x9a\x8b\xa9\x9e\x93\x8a\x9a\xa0\x8f\x8d\x96\x89\x9e\x8b\x9a",
 "\x94\x85\x9c\xad\x9a\x8c\x90\x8a\x8d\x9c\x9a\xb2\x9e\x91\x9e\x98\x9a\x8d\xac\x9a\x8b\xb6\x92\x92\x9a\x9b\x96\x9e\x8b\x9a\xb8\xaf\xaa\xbb\x9a\x8f\x93\x90\x86\x92\x9a\x91\x8b\xba\x91\x9e\x9d\x93\x9a\x9b",
 "\x94\x85\x9c\xad\x9a\x91\x9b\x9a\x8d\x9a\x8d\xb6\x8c\xbc\x90\x89\x9a\x8d\x9e\x98\x9a\xbd\x8a\x99\x99\x9a\x8d\xac\x8a\x8f\x8f\x90\x8d\x8b\x9a\x9b",
 "\x94\x85\x9c\xad\x9e\x86\xbd\x90\x8a\x91\x9b\x96\x91\x98\xbd\x90\x87\xbd\x9e\x9c\x94\x99\x9e\x9c\x9a\xb6\x91\x8b\x9a\x8d\x8c\x9a\x9c\x8b\x96\x90\x91",
 "\x94\x85\x9e\xbe\x8f\x8f\x93\x96\x9c\x9e\x8b\x96\x90\x91\xac\x9a\x8b\xbe\x93\x93\x90\x9c\x9e\x8b\x9a\x9b\xb3\x9e\x86\x90\x8a\x8b\xbd\x90\x8a\x91\x9b\x96\x91\x98\xbd\x90\x87\xa9\x96\x8c\x8a\x9e\x93\x96\x85\x9e\x8b\x96\x90\x91\xba\x91\x9e\x9d\x93\x9a\x9b",
 "\x9d\x99\xac\x9c\x9a\x91\x9a\xbc\x90\x91\x99\x96\x98\x8a\x8d\x9e\x8b\x96\x90\x91\xb6\x8c\xa9\x9e\x93\x96\x9b",
            }
        },

        {
            gcvPATCH_NENAMARK,
            {
 "\xa0\xa5\xce\xce\x8c\x9a\x8b\x8a\x8f\xac\x97\x9e\x9b\x9a\x8d\xb6\xb1\xce\xce\xbc\x93\x90\x8a\x9b\xac\x86\x8c\x8b\x9a\x92\xce\xce\xbc\x93\x90\x8a\x9b\xac\x97\x9e\x9b\x9a\x8d\xba\xba\xce\xce\xac\x9a\x8b\x8a\x8f\xac\x97\x9e\x9b\x9a\x8d\xb6\xab\xa0\xba\xaf\xc6\xb8\xb3\xbc\x90\x91\x8b\x9a\x87\x8b\xad\xac\xcc\xa0\xad\xb4\xc9\xac\x8b\x8d\x96\x91\x98",
 "\xa0\xa5\xce\xca\x8c\x9a\x8b\xb8\x93\x90\x9d\x9e\x93\xb3\x90\x98\x98\x9a\x8d\xaf\xb9\x89\xaf\x89\xc7\xb3\x90\x98\xb3\x9a\x89\x9a\x93\xaf\xb4\x9c\xac\xcd\xa0\xac\x8b\xc6\xa0\xa0\x89\x9e\xa0\x93\x96\x8c\x8b\xba\xac\xa0",
 "\xa0\xa5\xcd\xcd\xb1\x9a\x91\x9e\xb2\x9e\x8d\x94\xb5\xb1\xb6\xa0\x90\x91\xb9\x96\x91\x96\x8c\x97\x9a\x9b\xaf\xc8\xa0\xb5\xb1\xb6\xba\x91\x89\xaf\xc7\xa0\x95\x90\x9d\x95\x9a\x9c\x8b\xaf\xce\xcf\xa0\x95\x92\x9a\x8b\x97\x90\x9b\xb6\xbb\x99\xad\xb4\xc8\xb7\x9e\x8c\x97\xb2\x9e\x8f\xb6\xc9\xac\x8b\x8d\x96\x91\x98\xac\xc9\xa0\xba\xad\xb4\xce\xce\xad\x9a\x91\x9b\x9a\x8d\xab\x8d\x9e\x9c\x9a",
 "\xa0\xa5\xb1\xce\xcf\xbe\x8f\x8f\xa8\x8d\x9e\x8f\x8f\x9a\x8d\xb6\xc7\xb1\x9a\x91\x9e\xb2\x9e\x8d\x94\xce\xc9\xb1\x9a\x91\x9e\xb2\x9e\x8d\x94\xbc\x9e\x93\x93\x9d\x9e\x9c\x94\xaf\xce\xcc\xb1\x9a\x91\x9e\xb2\x9e\x8d\x94\xac\x9c\x9a\x91\x9a\xba\xce\xcd\x96\x91\x96\x8b\x96\x9e\x93\x96\x85\x9a\xb8\xb3\xba\x89",
 "\xa0\xa5\xb1\xce\xce\xbd\x8a\x8b\x8b\x90\x91\xb8\x8d\x90\x8a\x8f\xcb\x96\x91\x96\x8b\xba\xaf\xc9\xac\x86\x8c\x8b\x9a\x92\xaf\xc6\xb8\xb3\xbc\x90\x91\x8b\x9a\x87\x8b\xad\xb4\xc9\xac\x8b\x8d\x96\x91\x98\xad\xb4\xc9\xa9\x9a\x9c\x8b\x90\x8d\xb6\xce\xcf\xbd\x8a\x8b\x8b\x90\x91\xac\x8f\x9a\x9c\xba",
 "\xa0\xa5\xb1\xce\xce\xac\x9a\x8b\x8a\x8f\xac\x97\x9e\x9b\x9a\x8d\xb6\xb1\xce\xce\xbc\x93\x90\x8a\x9b\xac\x86\x8c\x8b\x9a\x92\xce\xce\xbc\x93\x90\x8a\x9b\xac\x97\x9e\x9b\x9a\x8d\xba\xba\xcb\x9e\x8b\x8b\x8d\xb6\xce\xca\xac\x97\x9e\x9b\x9a\x8d\xbe\x8b\x8b\x8d\x96\x9d\x8a\x8b\x9a\xb6\xb3\x96\xca\xce\xcd\xce\xba\xb3\x96\xcb\xba\xba\xba\xba\xad\xac\xcd\xa0\xad\xb4\xc9\xac\x8b\x8d\x96\x91\x98\xad\xab\xa0",
 "\xa0\xa5\xb1\xc7\xb1\x9a\x91\x9e\xb2\x9e\x8d\x94\xce\xce\x98\x9a\x8b\xac\x9a\x8b\x8b\x96\x91\x98\x8c\xba\xad\xc8\xb7\x9e\x8c\x97\xb2\x9e\x8f\xb6\xc9\xac\x8b\x8d\x96\x91\x98\xce\xcc\xac\x9a\x8b\x8b\x96\x91\x98\x8c\xa9\x9e\x93\x8a\x9a\xba",
 "\xa0\xa5\xb1\xc7\xb1\x9a\x91\x9e\xb2\x9e\x8d\x94\xbc\xce\xba\xaf\xc9\xac\x86\x8c\x8b\x9a\x92\xaf\xce\xc9\xb1\x9a\x91\x9e\xb2\x9e\x8d\x94\xbc\x9e\x93\x93\x9d\x9e\x9c\x94\xaf\xc6\xb8\xb3\xbc\x90\x91\x8b\x9a\x87\x8b\xaf\xce\xcc\xb8\x8d\x9e\x8f\x97\x96\x9c\x8c\xbc\x9e\x9c\x97\x9a",
 "\xa0\xa5\xb1\xc6\xbd\x93\x9a\x91\x9b\xb2\x9a\x8c\x97\xcb\xb2\x9a\x8c\x97\xce\xcf\x8c\x9a\x8b\xab\x9a\x87\x8b\x8a\x8d\x9a\xba\xad\xb4\xc9\xac\x8b\x8d\x96\x91\x98",
            }
        },

        {
            gcvPATCH_NEOCORE,
            {
 "\xa0\xa5\xcc\xc7\xbe\xab\xb6\xa0\xab\xbc\xa0\xba\x91\x9c\x90\x9b\x9a\xb6\x92\x9e\x98\x9a\xb9\x9e\x8c\x8b\xa0\xac\x9a\x8f\x9a\x8d\x9e\x8b\x9a\xa0\xb7\x9a\x9e\x9b\x9a\x8d\xaf\xb4\x97\x95\x95\x95\x95\x96\x96\x96\x95\x95\xaf\xce\xcc\xa0\xbe\xab\xb6\xab\xbc\xa0\xb7\xba\xbe\xbb\xba\xad\xaf\x97\xaf\x95",
 "\xa0\xa5\xb1\xce\xc7\xae\xa7\xab\x9a\x87\x8b\x8a\x8d\x9a\xbc\x90\x91\x89\x9a\x8d\x8b\x9a\x8d\xcd\xce\xbd\x8a\x92\x8f\x92\x9e\x8f\xa0\xb0\x99\x99\x8c\x9a\x8b\xbe\x91\x9b\xbd\x96\x9e\x8c\xba\x99",
 "\xa0\xa5\xb1\xc8\x9e\x91\x9b\x8d\x90\x96\x9b\xcd\xca\x9e\x8c\x8c\x9a\x8b\xb2\x9e\x91\x9e\x98\x9a\x8d\xb9\x90\x8d\xb5\x9e\x89\x9e\xb0\x9d\x95\x9a\x9c\x8b\xba\xaf\xc8\xa0\xb5\xb1\xb6\xba\x91\x89\xaf\xc7\xa0\x95\x90\x9d\x95\x9a\x9c\x8b",
 "\xbe\xab\xb6\xa0\xab\xbc\xa0\xbb\x9a\x9c\x90\x9b\x9a\xb6\x92\x9e\x98\x9a\xa0\xac\x9a\x8f\x9a\x8d\x9e\x8b\x9a\xa0\xb7\x9a\x9e\x9b\x9a\x8d",
 "\xbc\x90\x91\x99\x96\x98\xb0\x8f\x8b\x96\x90\x91\x8c\xa0\xb3\x90\x98\xbc\x90\x91\x99\x96\x98\x8a\x8d\x9e\x8b\x96\x90\x91",
 "\xb1\xbc\xbc\x90\x91\x99\x96\x98\xb2\x9a\x91\x8a\xa0\xbe\x9b\x9b\xad\x9a\x8c\x90\x93\x8a\x8b\x96\x90\x91",
 "\xb1\xbc\xb3\x90\x9e\x9b\x96\x91\x98\xac\x9c\x8d\x9a\x9a\x91\xa0\xbc\x8d\x9a\x9e\x8b\x9a",
 "\xb1\xbc\xa8\x90\x8d\x93\x9b\xa0\xac\x9a\x8b\xb6\x91\x8b\x9a\x8d\x9e\x9c\x8b\x96\x89\x9a\xb2\x90\x9b\x9a\xad\x90\x8b\x9e\x8b\x96\x90\x91",
            }
        },

        {
            gcvPATCH_GLBM11,
            {
 "\x8d\x9a\x91\x9b\x9a\x8d\xaf\x9e\x8d\x8b\x96\x9c\x93\x9a\xac\x86\x8c\x8b\x9a\x92\xa8\x96\x8b\x97\xaf\x90\x96\x91\x8b\xac\x96\x85\x9a\xbe\x8d\x8d\x9e\x86",
 "\x98\x93\x9d\xa0\x8b\x9a\x8c\x8b\x8d\x9a\x8c\x8a\x93\x8b\xa0\x91\x9a\x88\xa0\x99\x8d\x90\x92\xa0\x9b\x9e\x8b\x9e",
 "\x98\x93\x9d\xa0\x92\xcc\x98\x9c\x90\x91\x99\x96\x98\xa0\x91\x9a\x88\xa0\x99\x8d\x90\x92\xa0\x9b\x9e\x8b\x9e",
 "\x98\x93\x9d\xa0\x92\xcc\x98\x8c\x9a\x8b\x8b\x96\x91\x98\x8c\xa0\x96\x91\x96\x8b",
 "\x9c\x90\x91\x89\x9a\x8d\x8b\xae\x8a\x9e\x8b\x9a\x8d\x91\x96\x90\x91\xab\x90\xbe\x91\x98\x93\x9a\xbe\x87\x96\x8c\xa7",
 "\x9e\x9b\x9b\xab\x9a\x87\x8b\x8a\x8d\x9a\xab\x90\xad\x9a\x91\x9b\x9a\x8d\xb3\x96\x8c\x8b",
 "\xa0\x98\x93\x9d\xa0\x9a\x91\x98\x96\x91\x9a\xa0\x96\x91\x96\x8b\xa0\x8f\x8d\x90\x95\x9a\x9c\x8b\x96\x90\x91\xa0\x92\x9e\x8b\x8d\x96\x87",
 "\x9c\x90\x92\x8f\x8a\x8b\x9a\xb6\x91\x96\x8b\x96\x9e\x93\xa8\x90\x8d\x93\x9b\xaf\x90\x8c\x96\x8b\x96\x90\x91",
            }
        },

    };
#endif

    gcmHEADER();

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    Hardware->patchID = gcvPATCH_INVALID;

    /* If global patch is set, pick it */
    if (gcPLS.patchID > gcvPATCH_NOTINIT)
    {
        Hardware->patchID = gcPLS.patchID;

        gcmDUMP(gcvNULL, "#[patchID 0x%04x]", gcPLS.patchID);
        gcmFOOTER();
        return status;
    }

#if gcdDEBUG_OPTION
    if (gcoOS_DetectProcessByName(gcdDEBUG_OPTION_KEY))
    {
        gcPLS.patchID     =
        Hardware->patchID = gcvPATCH_DEBUG;

        gcmDUMP(gcvNULL, "#[patchID 0x%04x]", gcPLS.patchID);
        gcmFOOTER();
        return status;
    }
#endif

#if defined(ANDROID)

    for (i = 0; i < gcmCOUNTOF(programList); i++)
    {
        status = gcoOS_DetectProgrameByEncryptedSymbols(&programList[i]);
        if (status == gcvSTATUS_SKIP)
        {
            gcPLS.patchID     =
            Hardware->patchID = gcvPATCH_INVALID;

            break;
        }
        else if (status == gcvSTATUS_MISMATCH)
        {
            gcPLS.patchID     =
            Hardware->patchID = gcvPATCH_INVALID;

            continue;
        }
        else if (status == gcvSTATUS_TRUE)
        {
            gcPLS.patchID     =
            Hardware->patchID = programList[i].patchId;
            gcmDUMP(gcvNULL, "#[patchID 0x%04x]", gcPLS.patchID);
            gcmFOOTER();
            return status;
        }
    }

#endif

    /* get current process name by pid */
    gcmONERROR(gcoOS_QueryCurrentProcessName(curProcessName, gcdMAX_PATH));

    for (i = 0; i < gcmCOUNTOF(benchList); i++)
    {
        gctCHAR *p, buff[gcdMAX_PATH];
        gctSTRING pos = gcvNULL;
        p = buff;

        gcoOS_StrCopySafe(buff, gcdMAX_PATH, benchList[i].name);
        while (*p)
        {
            *p = ~(*p);
            p++;
        }

        /* compare the patch name with current process name */
        gcoOS_StrStr(curProcessName, buff, &pos);

        if (pos)
        {
            gcPLS.patchID     =
            Hardware->patchID = benchList[i].patchId;

            if (benchList[i].symbolFlag)
            {
                gcmPRINT(" Symbol checking %d is invalid!", benchList[i].patchId);
            }

            gcmDUMP(gcvNULL, "#[patchID 0x%04x]", gcPLS.patchID);
            gcmFOOTER();
            return status;
        }
    }

    gcPLS.patchID = gcvPATCH_INVALID;

OnError:
    gcmDUMP(gcvNULL, "#[patchID 0x%04x]", gcPLS.patchID);
    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_OK;
#endif
}

#if gcdENABLE_3D
gceSTATUS
gcoHARDWARE_SetPatchID(
    IN gcoHARDWARE Hardware,
    IN gcePATCH_ID   PatchID
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER();

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_ARGUMENT(PatchID);

    gcPLS.patchID     =
    Hardware->patchID = PatchID;

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_GetPatchID(
    IN gcoHARDWARE Hardware,
    OUT gcePATCH_ID *PatchID
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER();

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_ARGUMENT(PatchID);

    *PatchID = Hardware->patchID;

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetSpecialHintData(
    IN gcoHARDWARE Hardware
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER();

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    Hardware->specialHintData = 0;

    switch(Hardware->patchID)
    {
        case gcvPATCH_GLBM27:
        Hardware->specialHintData = 200;
            break;
        case gcvPATCH_GFXBENCH:
            Hardware->specialHintData = 420;
            break;
        default:
            break;
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_GetSpecialHintData(
    IN gcoHARDWARE Hardware,
    OUT gctINT * Hint
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER();

    gcmGETHARDWARE(Hardware);
    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    *Hint = (Hardware->specialHintData < 0) ? (Hardware->specialHintData) : (-- Hardware->specialHintData);

OnError:
    gcmFOOTER();
    return status;
}
#endif

#if gcdENABLE_3D

static gcsHARDWARE_ExeFuncs _v60Funcs =
{
    gcoHARDWARE_ProgramTextureDesc,
};

static gcsHARDWARE_ExeFuncs _Funcs =
{
    gcoHARDWARE_ProgramTexture,
};

#endif

#if (gcdENABLE_3D || gcdENABLE_2D)
static gceSTATUS
_Attach(
    IN gcoHARDWARE Hardware,
    IN gctUINT Core
    )
{
    gceSTATUS status;
    gcsHAL_INTERFACE iface;
    gcoHARDWARE hardware = Hardware;
    gctUINT i;
    gctPOINTER pointer;
    gctUINT32 coreIndex;
    gctBOOL coreIndexChanged = gcvFALSE;

    gcoHAL_GetCurrentCoreIndex(gcvNULL, &coreIndex);

    /* FIXME: Convert core index in this gcoHARDWARE to gloabl core index.
    * For now, they are same.
    */
    gcoHAL_SetCoreIndex(gcvNULL, Core);
    coreIndexChanged = gcvTRUE;

    iface.command = gcvHAL_ATTACH;

#if gcdDUMP
    iface.u.Attach.map = gcvTRUE;
#else
    iface.u.Attach.map = gcvFALSE;
#endif

    gcmONERROR(gcoOS_DeviceControl(
        gcvNULL,
        IOCTL_GCHAL_INTERFACE,
        &iface, gcmSIZEOF(iface),
        &iface, gcmSIZEOF(iface)
        ));

    gcmONERROR(iface.status);

    gcoHAL_SetCoreIndex(gcvNULL, coreIndex);
    coreIndexChanged = gcvFALSE;

    /* Store the allocated context buffer object. */
    hardware->contexts[Core] = iface.u.Attach.context;
    gcmASSERT(hardware->contexts[Core] != 0);

    /* Store the number of states in the context. */
    hardware->maxState = (gctUINT32) iface.u.Attach.maxState;

    hardware->numStates  = iface.u.Attach.numStates;

#if gcdDUMP
    hardware->currentContext = 0;
    hardware->contextBytes = iface.u.Attach.bytes;

    for (i = 0; i < gcdCONTEXT_BUFFER_COUNT; i++)
    {
        hardware->contextPhysical[i] = iface.u.Attach.physicals[i];
        hardware->contextLogical[i] = gcmUINT64_TO_PTR(iface.u.Attach.logicals[i]);
    }
#endif

    /**************************************************************************/
    /* Allocate the context and state delta buffers. **************************/

    for (i = 0; i < gcdCONTEXT_BUFFER_COUNT + 1; i += 1)
    {
        /* Allocate a state delta. */
        gcsSTATE_DELTA_PTR delta;
        gcsSTATE_DELTA_PTR prev;
        gctUINT32 bytes;

        /* Allocate the state delta structure. */
        gcmONERROR(gcoOS_AllocateSharedMemory(
            gcvNULL, gcmSIZEOF(gcsSTATE_DELTA), (gctPOINTER *) &delta
            ));

        /* Reset the context buffer structure. */
        gcoOS_ZeroMemory(delta, gcmSIZEOF(gcsSTATE_DELTA));

        /* Append to the list. */
        if (hardware->deltas[Core] == gcvNULL)
        {
            delta->prev     = gcmPTR_TO_UINT64(delta);
            delta->next     = gcmPTR_TO_UINT64(delta);
            hardware->deltas[Core] = delta;
        }
        else
        {
            delta->next = gcmPTR_TO_UINT64(hardware->deltas[Core]);
            delta->prev = hardware->deltas[Core]->prev;

            prev = gcmUINT64_TO_PTR(hardware->deltas[Core]->prev);
            prev->next = gcmPTR_TO_UINT64(delta);
            hardware->deltas[Core]->prev = gcmPTR_TO_UINT64(delta);

        }

        /* Set the number of delta in the order of creation. */
#if gcmIS_DEBUG(gcdDEBUG_CODE)
        delta->num = i;
#endif
        if (hardware->maxState > 0)
        {
            /* Compute UINT array size. */
            bytes = gcmSIZEOF(gctUINT) * hardware->maxState;

            /* Allocate map ID array. */
            gcmONERROR(gcoOS_AllocateSharedMemory(
                gcvNULL, bytes, &pointer
                ));

            delta->mapEntryID = gcmPTR_TO_UINT64(pointer);

            /* Set the map ID size. */
            delta->mapEntryIDSize = bytes;

            /* Reset the record map. */
            gcoOS_ZeroMemory(gcmUINT64_TO_PTR(delta->mapEntryID), bytes);

            /* Allocate map index array. */
            gcmONERROR(gcoOS_AllocateSharedMemory(
                gcvNULL, bytes, &pointer
                ));

            delta->mapEntryIndex = gcmPTR_TO_UINT64(pointer);
        }

        if (hardware->numStates > 0)
        {
            /* Allocate state record array. */
            gcmONERROR(gcoOS_AllocateSharedMemory(
                gcvNULL,
                gcmSIZEOF(gcsSTATE_DELTA_RECORD) * hardware->numStates,
                &pointer
                ));

            delta->recordArray = gcmPTR_TO_UINT64(pointer);
        }

        /* Reset the new state delta. */
        _ResetDelta(delta);
    }

    return gcvSTATUS_OK;

OnError:
    if (coreIndexChanged)
    {
        /* Restore hardware type. */
        gcoHAL_SetCoreIndex(gcvNULL, coreIndex);
    }

    return status;
}

static gceSTATUS
_InitializeFlatMappingRange(
    IN gcoHARDWARE Hardware
    )
{
    gceSTATUS status;

    gcmHEADER();

    if (Hardware->features[gcvFEATURE_MMU])
    {
        gcsHAL_INTERFACE iface;
        iface.command = gcvHAL_GET_BASE_ADDRESS;

        gcmONERROR(gcoHAL_Call(gcvNULL, &iface));

        Hardware->flatMappingStart = iface.u.GetBaseAddress.flatMappingStart;
        Hardware->flatMappingEnd   = iface.u.GetBaseAddress.flatMappingEnd;
    }
    else
    {
        /* HW with old MMU always can access 0 - 0x800000000 directly. */
        Hardware->flatMappingStart = 0;
        Hardware->flatMappingEnd   = 0x80000000;
    }

    gcmFOOTER_ARG("flatMappingStart=0x%X, flatMappingEnd=0x%X",
                  Hardware->flatMappingStart, Hardware->flatMappingEnd);
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

void
_UpdateDelta(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Core
    )
{
    if (Hardware->deltas[Core])
    {
        /* Did the delta become associated? */
        if (Hardware->deltas[Core]->refCount == 0)
        {
            /* No, merge with the previous. */
            _MergeDelta(Hardware->deltas[Core]);
        }
        else
        {
            /* The delta got associated, move to the next one. */
            Hardware->deltas[Core] = gcmUINT64_TO_PTR(Hardware->deltas[Core]->next);
            gcmASSERT(Hardware->deltas[Core]->refCount == 0);

#if gcdDUMP && !gcdDUMP_COMMAND && !gcdDUMP_IN_KERNEL
            if (Core == 0)
            {
                /* Dump current context buffer. */
                gcmDUMP_BUFFER(gcvNULL,
                    "context",
                    Hardware->context,
                    Hardware->contextLogical[Hardware->currentContext],
                    0,
                    Hardware->contextBytes);

                /* Advance to next context buffer. */
                Hardware->currentContext = (Hardware->currentContext + 1) % gcdCONTEXT_BUFFER_COUNT;
            }
#endif
        }

        /* Reset the current. */
        _ResetDelta(Hardware->deltas[Core]);
    }
}

#endif

/******************************************************************************\
****************************** gcoHARDWARE API code ****************************
\******************************************************************************/

/*******************************************************************************
**
**  gcoHARDWARE_Construct
**
**  Construct a new gcoHARDWARE object.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to an gcoHAL object.
**
**      gctBOOL ThreadDefault
**          It's default hardware object of current thread or not.
**
**  OUTPUT:
**
**      gcoHARDWARE * Hardware
**          Pointer to a variable that will hold the gcoHARDWARE object.
*/
gceSTATUS
gcoHARDWARE_Construct(
    IN gcoHAL Hal,
    IN gctBOOL ThreadDefault,
    IN gctBOOL Robust,
    OUT gcoHARDWARE * Hardware
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_3D || gcdENABLE_2D
    gcoHARDWARE hardware = gcvNULL;
    gctUINT16 data = 0xff00;
    gctPOINTER pointer;
#if gcdENABLE_3D
    gctUINT i;
    gctUINT j;
#endif
    gceHARDWARE_TYPE type;
    gcmHEADER_ARG("Hal=0x%x", Hal);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
    gcmDEBUG_VERIFY_ARGUMENT(Hardware != gcvNULL);

    /***************************************************************************
    ** Allocate and reset the gcoHARDWARE object.
    */

    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              gcmSIZEOF(struct _gcoHARDWARE),
                              &pointer));
    hardware = pointer;

    /* Reset the object. */
    gcoOS_ZeroMemory(hardware, gcmSIZEOF(struct _gcoHARDWARE));

    /* Initialize the gcoHARDWARE object. */
    hardware->object.type = gcvOBJ_HARDWARE;

    hardware->engine[gcvENGINE_RENDER].buffer = gcvNULL;
    hardware->engine[gcvENGINE_RENDER].queue  = gcvNULL;

    hardware->engine[gcvENGINE_BLT].buffer = gcvNULL;
    hardware->engine[gcvENGINE_BLT].queue  = gcvNULL;

    /* Check if big endian */
    hardware->bigEndian = (*(gctUINT8 *)&data == 0xff);

    /* Don't stall before primitive. */
    hardware->stallDestination =
        hardware->features[gcvFEATURE_COMMAND_PREFETCH] ?
            gcvWHERE_COMMAND_PREFETCH : gcvWHERE_COMMAND;

    hardware->stallSource =
        hardware->features[gcvFEATURE_BLT_ENGINE] ?
            gcvWHERE_BLT : gcvWHERE_PIXEL;


    gcmONERROR(gcoHARDWARE_DetectProcess(hardware));

    hardware->threadDefault = ThreadDefault;

#if gcdENABLE_3D
    gcmONERROR(gcSHADER_SpecialHint(hardware->patchID, &hardware->specialHint));

    gcmONERROR(gcoHARDWARE_SetSpecialHintData(hardware));

    /* Set default API. */
    hardware->api = gcvAPI_OPENGL;

    /***************************************************************************
    ** Allocate and reset the state and dirty module in the gcoHARDWARE object.
    */
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsFESTATES), (gctPOINTER *) &hardware->FEStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsPAANDSESTATES), (gctPOINTER *) &hardware->PAAndSEStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsMSAASTATES), (gctPOINTER *) &hardware->MsaaStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsSHSTATES), (gctPOINTER *) &hardware->SHStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsPESTATES), (gctPOINTER *) &hardware->PEStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsTXSTATES), (gctPOINTER *) &hardware->TXStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsMCSTATES), (gctPOINTER *) &hardware->MCStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsXFBSTATES), (gctPOINTER *) &hardware->XFBStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsQUERYSTATES), (gctPOINTER *) &hardware->QUERYStates));

    gcoOS_ZeroMemory(hardware->FEStates, gcmSIZEOF(gcsFESTATES));
    gcoOS_ZeroMemory(hardware->PAAndSEStates, gcmSIZEOF(gcsPAANDSESTATES));
    gcoOS_ZeroMemory(hardware->MsaaStates, gcmSIZEOF(gcsMSAASTATES));
    gcoOS_ZeroMemory(hardware->SHStates, gcmSIZEOF(gcsSHSTATES));
    gcoOS_ZeroMemory(hardware->PEStates, gcmSIZEOF(gcsPESTATES));
    gcoOS_ZeroMemory(hardware->TXStates, gcmSIZEOF(gcsTXSTATES));
    gcoOS_ZeroMemory(hardware->MCStates, gcmSIZEOF(gcsMCSTATES));
    gcoOS_ZeroMemory(hardware->XFBStates, gcmSIZEOF(gcsXFBSTATES));
    gcoOS_ZeroMemory(hardware->QUERYStates, gcmSIZEOF(gcsQUERYSTATES));

    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsFEDIRTY), (gctPOINTER *) &hardware->FEDirty));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsPAANDSEDIRTY), (gctPOINTER *) &hardware->PAAndSEDirty));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsMSAADIRTY), (gctPOINTER *) &hardware->MsaaDirty));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsSHDIRTY), (gctPOINTER *) &hardware->SHDirty));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsPEDIRTY), (gctPOINTER *) &hardware->PEDirty));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsTXDIRTY), (gctPOINTER *) &hardware->TXDirty));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsMCDIRTY), (gctPOINTER *) &hardware->MCDirty));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsXFBDIRTY), (gctPOINTER *) &hardware->XFBDirty));

    gcoOS_ZeroMemory(hardware->FEDirty, gcmSIZEOF(gcsFEDIRTY));
    gcoOS_ZeroMemory(hardware->PAAndSEDirty, gcmSIZEOF(gcsPAANDSEDIRTY));
    gcoOS_ZeroMemory(hardware->MsaaDirty, gcmSIZEOF(gcsMSAADIRTY));
    gcoOS_ZeroMemory(hardware->SHDirty, gcmSIZEOF(gcsSHDIRTY));
    gcoOS_ZeroMemory(hardware->PEDirty, gcmSIZEOF(gcsPEDIRTY));
    gcoOS_ZeroMemory(hardware->TXDirty, gcmSIZEOF(gcsTXDIRTY));
    gcoOS_ZeroMemory(hardware->MCDirty, gcmSIZEOF(gcsMCDIRTY));
    gcoOS_ZeroMemory(hardware->XFBDirty, gcmSIZEOF(gcsXFBDIRTY));

    hardware->PEStates->stencilStates.mode = gcvSTENCIL_NONE;
    hardware->PEStates->stencilEnabled     = gcvFALSE;

    hardware->PEStates->earlyDepth = gcvFALSE;

    hardware->SHStates->rtneRounding = gcvFALSE;

    /* Disable anti-alias. */
    hardware->MsaaStates->sampleMask   = 0xF;
    hardware->MsaaStates->sampleEnable = 0xF;
    hardware->MsaaStates->sampleInfo   = g_sampleInfos[1];

    /* Table for disable dithering. */
    hardware->PEStates->ditherTable[0][0] = hardware->PEStates->ditherTable[0][1] = ~0U;
    /* Table for enable dithering. */
    hardware->PEStates->ditherTable[1][0] = 0x6E4CA280;
    hardware->PEStates->ditherTable[1][1] = 0x5D7F91B3;
    hardware->PEDirty->peDitherDirty = gcvTRUE;
    hardware->PEStates->ditherEnable = gcvFALSE;

    hardware->MsaaStates->MsaaFragmentOp = 0;

    /***************************************************************************
    ** Determine multi-sampling constants.
    */

    /* 4x MSAA jitter index. */

#if gcdGRID_QUALITY == 0

    /* Square only. */
    hardware->MsaaStates->jitterIndex
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:2) - (0 ?
 3:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ?
 3:2))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 3:2) - (0 ?
 3:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ?
 3:2)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:4) - (0 ?
 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 5:4) - (0 ?
 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:6) - (0 ?
 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ?
 7:6))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 7:6) - (0 ?
 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ?
 7:6)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:8) - (0 ?
 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 9:8) - (0 ?
 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:10) - (0 ?
 11:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 11:10) - (0 ?
 11:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ?
 11:10)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 13:12) - (0 ?
 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 13:12) - (0 ?
 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ?
 13:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:14) - (0 ?
 15:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 15:14) - (0 ?
 15:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ?
 15:14)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 17:16) - (0 ?
 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 17:16) - (0 ?
 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:18) - (0 ?
 19:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 19:18) - (0 ?
 19:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ?
 19:18)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ?
 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 21:20) - (0 ?
 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:22) - (0 ?
 23:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ?
 23:22))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 23:22) - (0 ?
 23:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ?
 23:22)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:24) - (0 ?
 25:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ?
 25:24))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 25:24) - (0 ?
 25:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ?
 25:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:26) - (0 ?
 27:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 27:26) - (0 ?
 27:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ?
 27:26)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 29:28) - (0 ?
 29:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 29:28) - (0 ?
 29:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:30) - (0 ?
 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 31:30) - (0 ?
 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));

#elif gcdGRID_QUALITY == 1

    /* No jitter. */
    hardware->MsaaStates->jitterIndex = 0;

#else

    /* Rotated diamonds. */
    hardware->MsaaStates->jitterIndex
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:2) - (0 ?
 3:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ?
 3:2))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 3:2) - (0 ?
 3:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ?
 3:2)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:4) - (0 ?
 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 5:4) - (0 ?
 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:6) - (0 ?
 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ?
 7:6))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 7:6) - (0 ?
 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ?
 7:6)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:8) - (0 ?
 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 9:8) - (0 ?
 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:10) - (0 ?
 11:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 11:10) - (0 ?
 11:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ?
 11:10)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 13:12) - (0 ?
 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 13:12) - (0 ?
 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ?
 13:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:14) - (0 ?
 15:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 15:14) - (0 ?
 15:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ?
 15:14)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 17:16) - (0 ?
 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 17:16) - (0 ?
 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:18) - (0 ?
 19:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 19:18) - (0 ?
 19:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ?
 19:18)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ?
 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 21:20) - (0 ?
 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:22) - (0 ?
 23:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ?
 23:22))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 23:22) - (0 ?
 23:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ?
 23:22)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:24) - (0 ?
 25:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ?
 25:24))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:24) - (0 ?
 25:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ?
 25:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:26) - (0 ?
 27:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 27:26) - (0 ?
 27:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ?
 27:26)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 29:28) - (0 ?
 29:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 29:28) - (0 ?
 29:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:30) - (0 ?
 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 31:30) - (0 ?
 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));

#endif


    /* 2x MSAA sample coordinates. */

    hardware->MsaaStates->sampleCoords2
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28)));


    /* 4x MSAA sample coordinates.
    **
    **                        1 1 1 1 1 1                        1 1 1 1 1 1
    **    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  0| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  1| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  2| | | | | | |X| | | | | | | | | |  | | | | | | | | | | |X| | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  3| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  4| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  5| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  6| | | | | | | | | | | | | | |X| |  | | |X| | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  7| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  8| | | | | | | | |o| | | | | | | |  | | | | | | | | |o| | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  9| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 10| | |X| | | | | | | | | | | | | |  | | | | | | | | | | | | | | |X| |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 11| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 12| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 13| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 14| | | | | | | | | | |X| | | | | |  | | | | | | |X| | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 15| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */

    /* Diamond. */
    hardware->MsaaStates->sampleCoords4[0]
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (6) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (14) & ((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (6) & ((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28))) | (((gctUINT32) ((gctUINT32) (14) & ((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28)));

    /* Mirrored diamond. */
    hardware->MsaaStates->sampleCoords4[1]
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (6) & ((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (14) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (6) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28))) | (((gctUINT32) ((gctUINT32) (14) & ((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28)));

    /* Square. */
    hardware->MsaaStates->sampleCoords4[2]
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28)));


    /* Compute centroids. */
    gcmONERROR(gcoHARDWARE_ComputeCentroids(
        hardware,
        1, &hardware->MsaaStates->sampleCoords2, &hardware->MsaaStates->centroids2
        ));

    gcmONERROR(gcoHARDWARE_ComputeCentroids(
        hardware,
        3, hardware->MsaaStates->sampleCoords4, hardware->MsaaStates->centroids4
        ));
#endif /* gcdENABLE_3D */

    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsHARDWARE_CONFIG), &pointer));
    gcoOS_ZeroMemory(pointer,gcmSIZEOF(gcsHARDWARE_CONFIG));
    hardware->config = pointer;
    gcmVERIFY_OK(_FillInConfigTableByDatabase(hardware, hardware->config));
    gcmVERIFY_OK(_FillInFeatureTableByDatabase(hardware, hardware->features));
    gcmVERIFY_OK(_FillInSWWATable(hardware, hardware->swwas));

#if gcdENABLE_3D
    hardware->unifiedConst = hardware->config->unifiedConst;
    hardware->vsConstBase  = hardware->config->vsConstBase;
    hardware->psConstBase  = hardware->config->psConstBase;
    hardware->vsConstMax   = hardware->config->vsConstMax;
    hardware->psConstMax   = hardware->config->psConstMax;
    hardware->constMax     = hardware->config->constMax;

    hardware->unifiedInst   = hardware->config->unifiedInst;
    hardware->vsInstBase    = hardware->config->vsInstBase;
    hardware->psInstBase    = hardware->config->psInstBase;
    hardware->vsInstMax     = hardware->config->vsInstMax;
    hardware->psInstMax     = hardware->config->psInstMax;
    hardware->instMax       = hardware->config->instMax;

    /* Initialize reset status for unified settings */
    hardware->prevProgramUnfiedStatus.useIcache = hardware->features[gcvFEATURE_SH_INSTRUCTION_PREFETCH];

    hardware->prevProgramUnfiedStatus.instruction = hardware->prevProgramUnfiedStatus.useIcache ?
                                                    gcvFALSE :
                                                    hardware->unifiedInst;

    hardware->prevProgramUnfiedStatus.constant          = hardware->unifiedConst;
    hardware->prevProgramUnfiedStatus.sampler           = gcvFALSE;
    hardware->prevProgramUnfiedStatus.instVSEnd         = -1;
    hardware->prevProgramUnfiedStatus.instPSStart       = -1;
    hardware->prevProgramUnfiedStatus.constGPipeEnd     = -1;
    hardware->prevProgramUnfiedStatus.constPSStart      = -1;
    hardware->prevProgramUnfiedStatus.samplerGPipeStart = -1;
    hardware->prevProgramUnfiedStatus.samplerPSEnd      = -1;

    hardware->prevProgramStageBits = 0;

    hardware->prevProgramBarrierUsed = gcvFALSE;
    hardware->previousPEDepth = gcvTRUE;

    if (hardware->config->gpuCoreCount > 1)
    {
        /* Set the default multi GPU rendering mode. */
        hardware->gpuRenderingMode = gcvMULTI_GPU_RENDERING_MODE_INVALID;
        /* Set default multi-GPU operating mode as combined. */
        hardware->gpuMode          = gcvMULTI_GPU_MODE_COMBINED;
        /* In the combined mode, the core ID is ignored. */
        hardware->chipEnable       = gcvCORE_3D_ALL_MASK;
        /* This is the semaphore ID
           TODO: We need to move this out of hardware layer. Because all processes
                 need to share this id. It may be proper to move this counter to
                 kernel layer and add an ioctl to fetch the next available id instead of
                 gcoHARDWARE_MultiGPUSemaphoreId.
         */
        hardware->interGpuSemaphoreId   = 0;
        hardware->prevSingleCore = -1;
    }

    /* Default depth near and far plane. */
    hardware->PEStates->depthNear.f = 0.0f;
    hardware->PEStates->depthFar.f  = 1.0f;

    if ((hardware->config->pixelPipes > 1) &&
        (!hardware->features[gcvFEATURE_SINGLE_BUFFER]))
    {
        hardware->multiPipeResolve = gcvTRUE;
        gcmASSERT(hardware->config->resolvePipes > 1);
    }
    else
    {
        hardware->multiPipeResolve = gcvFALSE;
    }

    hardware->resolveAlignmentX = 16;
    hardware->resolveAlignmentY = (hardware->multiPipeResolve)
                                ? (4 * hardware->config->resolvePipes) : 4;

    gcmTRACE(
        gcvLEVEL_INFO,
        "%s: resolvePipes=%d multiPipeResolve=%d resolveAlignment=%d,%d",
        __FUNCTION__,
        hardware->config->resolvePipes,
        hardware->multiPipeResolve,
        hardware->resolveAlignmentX,
        hardware->resolveAlignmentY
        );

    hardware->PEStates->singlePEpipe = (hardware->config->pixelPipes == 1);

#endif

#if gcdENABLE_2D
    gcmVERIFY_OK(_Init2D(hardware));
#endif


    /***************************************************************************
    ** Allocate the gckCONTEXT object.
    */
    gcmGETCURRENTHARDWARE(type);

    hardware->constructType = type;

#if gcdENABLE_3D || gcdENABLE_2D
    /* 2D indpendent HW doesn't need context. */
    if (type != gcvHARDWARE_2D)
    {
        gctUINT i;
        gctUINT coreCount = hardware->config->gpuCoreCount;

        gcmONERROR(gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(gcsSTATE_DELTA_PTR) * coreCount,
            (gctPOINTER *)&hardware->deltas
            ));

        gcoOS_ZeroMemory(hardware->deltas, gcmSIZEOF(gcsSTATE_DELTA_PTR) * coreCount);

        gcmONERROR(gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(gctUINT32) * coreCount,
            (gctPOINTER *)&hardware->contexts
            ));

        gcoOS_ZeroMemory(hardware->contexts, gcmSIZEOF(gctUINT32) * coreCount);

        for (i = 0; i < coreCount; i++)
        {
            gcmONERROR(_Attach(hardware, i));
        }

        /* TODO : Remove this. */
        hardware->delta = hardware->deltas[0];
        hardware->context = hardware->contexts[0];
    }
#endif

    /***********************************************************************
    ** Construct the command buffer and event queue.
    */
    gcmONERROR(gcoBUFFER_Construct(Hal, hardware, gcvENGINE_RENDER, hardware->context,
                                   gcdCMD_BUFFER_SIZE,
                                   hardware->threadDefault,
                                   &hardware->engine[gcvENGINE_RENDER].buffer));

    if (gcoHAL_GetOption(gcvNULL, gcvOPTION_KERNEL_FENCE) &&
        hardware->features[gcvFEATURE_BLT_ENGINE])
    {
        gcmONERROR(gcoBUFFER_Construct(Hal, hardware, gcvENGINE_BLT, hardware->context,
                                       gcdCMD_BLT_BUFFER_SIZE,
                                       hardware->threadDefault,
                                       &hardware->engine[gcvENGINE_BLT].buffer));
    }

    gcmONERROR(gcoQUEUE_Construct(gcvNULL, gcvENGINE_RENDER, &hardware->engine[gcvENGINE_RENDER].queue));

    if (gcoHAL_GetOption(gcvNULL, gcvOPTION_KERNEL_FENCE) &&
        hardware->features[gcvFEATURE_BLT_ENGINE])
    {
        gcmONERROR(gcoQUEUE_Construct(gcvNULL, gcvENGINE_BLT, &hardware->engine[gcvENGINE_BLT].queue));
    }

#if gcdENABLE_3D
    /***********************************************************************
    ** Reset the temporary surface.
    */
    hardware->tmpSurf.node.pool = gcvPOOL_UNKNOWN;

    /***********************************************************************
    ** Initialize texture states.
    */

    for (i = 0; i < gcdSAMPLERS; i += 1)
    {
        hardware->TXStates->hwTxSamplerMode[i]            = gcvINVALID_VALUE;
        hardware->TXStates->hwTxSamplerModeEx[i]          = gcvINVALID_VALUE;
        hardware->TXStates->hwTxSamplerMode2[i]           = gcvINVALID_VALUE;
        hardware->TXStates->hwTxSamplerSize[i]            = gcvINVALID_VALUE;
        hardware->TXStates->hwTxSamplerSizeLog[i]         = gcvINVALID_VALUE;
        hardware->TXStates->hwTxSampler3D[i]              = gcvINVALID_VALUE;
        hardware->TXStates->hwTxSamplerLOD[i]             = gcvINVALID_VALUE;
        hardware->TXStates->hwTxSamplerBaseLOD[i]         = gcvINVALID_VALUE;
        hardware->TXStates->hwTxSamplerYUVControl[i]      = gcvINVALID_VALUE;
        hardware->TXStates->hwTxSamplerYUVStride[i]       = gcvINVALID_VALUE;
        hardware->TXStates->hwTxSamplerLinearStride[i]    = gcvINVALID_VALUE;
        hardware->TXStates->hwTxSamplerSizeLogExt[i]      = gcvINVALID_VALUE;
        hardware->TXStates->hwTxSampler3DExt[i]           = gcvINVALID_VALUE;
        hardware->TXStates->hwTxSamplerLodExt[i]          = gcvINVALID_VALUE;
        hardware->TXStates->hwTxSamplerLodBiasExt[i]      = gcvINVALID_VALUE;
        hardware->TXStates->hwTxSamplerAnisoCtrl[i]       = gcvINVALID_VALUE;
        hardware->TXStates->hwTxSamplerConfig3[i]         = gcvINVALID_VALUE;
        hardware->TXStates->hwTxSamplerSliceSize[i]       = gcvINVALID_VALUE;

        for (j = 0; j < gcdLOD_LEVELS; j += 1)
        {
            hardware->TXStates->hwTxSamplerAddress[j][i]  = gcvINVALID_VALUE;
            hardware->TXStates->hwTxSamplerASTCSize[j][i] = gcvINVALID_VALUE;
            hardware->TXStates->hwTxSamplerASTCSRGB[j][i] = gcvINVALID_VALUE;
        }
    }

    for (i = 0; i < gcdSAMPLER_TS; i += 1)
    {
        hardware->MCStates->hwTXSampleTSConfig[i]          = gcvINVALID_VALUE;
        hardware->MCStates->hwTXSampleTSBuffer[i]          = gcvINVALID_VALUE;
        hardware->MCStates->hwTXSampleTSClearValue[i]      = gcvINVALID_VALUE;
        hardware->MCStates->hwTXSampleTSClearValueUpper[i] = gcvINVALID_VALUE;
        hardware->MCStates->hwTxSamplerTxBaseBuffer[i]     = gcvINVALID_VALUE;

        hardware->MCStates->texTileStatusSlotUser[i]       = -1;
    }

    /* Intialized texture descriptor states */
    for (i = 0; i < gcdTXDESCRIPTORS; i++)
    {
        hardware->TXStates->hwSamplerControl0[i] = gcvINVALID_VALUE;
        hardware->TXStates->hwSamplerControl1[i] = gcvINVALID_VALUE;
        hardware->TXStates->hwSamplerLodMinMax[i] = gcvINVALID_VALUE;
        hardware->TXStates->hwSamplerLODBias[i] = gcvINVALID_VALUE;
        hardware->TXStates->hwSamplerAnisCtrl[i] = gcvINVALID_VALUE;
        hardware->TXStates->hwTxDescAddress[i] = gcvINVALID_ADDRESS;
        hardware->TXStates->hwTextureControl[i] = gcvINVALID_ADDRESS;
        hardware->MCStates->texHasTileStatus[i] = gcvFALSE;
        hardware->MCStates->texBaseLevelSurfInfoWithTS[i] = gcvNULL;
        hardware->MCStates->texTileStatusSlotIndex[i] = -1;
    }

    gcsBITMASK_InitAllZero(&hardware->TXDirty->hwSamplerControl0Dirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllZero(&hardware->TXDirty->hwSamplerControl1Dirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllZero(&hardware->TXDirty->hwSamplerLodMinMaxDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllZero(&hardware->TXDirty->hwSamplerLODBiasDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllZero(&hardware->TXDirty->hwSamplerAnisCtrlDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllZero(&hardware->TXDirty->hwTxDescAddressDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllZero(&hardware->TXDirty->hwTxDescDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllZero(&hardware->TXDirty->hwTextureControlDirty, gcdTXDESCRIPTORS);

    /***********************************************************************
    ** Determine available features for 3D hardware.
    */

    hardware->hw3DEngine = hardware->features[gcvFEATURE_PIPE_3D];

    /* Determine whether threadwalker is in PS for OpenCL. */
    hardware->threadWalkerInPS = hardware->features[gcvFEATURE_CL_PS_WALKER];

    /* Determine whether composition engine is supported. */
    hardware->hwComposition = hardware->features[gcvFEATURE_COMPOSITION];

    /* Initialize composition shader parameters. */
    if (hardware->hwComposition)
    {
        gcmONERROR(gcoHARDWARE_InitializeComposition(hardware));
    }

    /* Initialize variables for bandwidth optimization. */
    for (i = 0; i < gcdMAX_DRAW_BUFFERS; ++i)
    {
        hardware->PEStates->colorStates.target[i].colorWrite = 0xF;
    }
    hardware->PEStates->colorStates.colorCompression = gcvFALSE;
    hardware->PEStates->colorStates.destinationRead  = ~0U;
    hardware->PEStates->colorStates.rop              = 0xC;
    hardware->preColorCacheMode128  = DEFAULT_CACHE_MODE;

    hardware->preDepthCacheMode128  = DEFAULT_CACHE_MODE;

    /* Determine striping. */
    if ((hardware->config->chipModel >= gcv860)
    && !hardware->features[gcvFEATURE_BUG_FIXES2]
    )
    {
        /* Get number of cache lines. */
        hardware->needStriping = hardware->hw2DEngine ? 32 : 16;
    }
    else
    {
        /* No need for striping. */
        hardware->needStriping = 0;
    }

    /* Initialize virtual stream state. */
    if ((hardware->config->streamCount == 1)
        ||  hardware->features[gcvFEATURE_MIXED_STREAMS])
    {
        hardware->mixedStreams = gcvTRUE;
    }
    else
    {
        hardware->mixedStreams = gcvFALSE;
    }

    for (i = 0; i < 4; i++)
    {
        hardware->SHStates->psOutputMapping[i] = i;
    }

    if (hardware->features[gcvFEATURE_TX_DESCRIPTOR])
    {
        hardware->funcPtr = &_v60Funcs;
    }
    else
    {
        hardware->funcPtr = &_Funcs;
    }

    hardware->robust = Robust && hardware->features[gcvFEATURE_ROBUSTNESS];

    hardware->QUERYStates->queryStatus[gcvQUERY_OCCLUSION] = gcvQUERY_Disabled;
#if gcdENABLE_APPCTXT_BLITDRAW
    hardware->QUERYStates->queryCmd[gcvQUERY_OCCLUSION] = gcvQUERYCMD_INVALID;
#endif

    if (hardware->features[gcvFEATURE_HW_TFB])
    {
        hardware->XFBStates->status =
        hardware->XFBStates->statusInCmd = gcvXFB_Disabled;

        hardware->QUERYStates->queryStatus[gcvQUERY_XFB_WRITTEN] =
        hardware->QUERYStates->queryStatus[gcvQUERY_PRIM_GENERATED] = gcvQUERY_Disabled;
#if gcdENABLE_APPCTXT_BLITDRAW
        hardware->QUERYStates->queryCmd[gcvQUERY_PRIM_GENERATED] =
        hardware->QUERYStates->queryCmd[gcvQUERY_XFB_WRITTEN] = gcvQUERYCMD_INVALID;
        hardware->XFBStates->cmd = gcvXFBCMD_INVALID;
#endif
    }

#endif

    hardware->currentPipe = (hardware->hw2DEngine) ? gcvPIPE_2D : gcvPIPE_3D;

    gcmONERROR(gcoOS_CreateSignal(gcvNULL, gcvFALSE, &hardware->stallSignal));

    gcmVERIFY_OK(gcoHARDWARE_InitializeFormatArrayTable(hardware));

#if gcdENABLE_3D
    /*Set a invalid value when init*/
    *(gctUINT32 *)&(hardware->PEStates->alphaStates.floatReference) = 0xFFFFFFFF;
#endif

#if gcdENABLE_3D
#if gcdSYNC
    hardware->fence = gcvNULL;
#endif
#endif

#if gcdENABLE_3D
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsHARDWARE_SLOT), &pointer));

    hardware->hwSlot = pointer;

    gcmONERROR(gcoHARDWARE_ResetHWSlot(hardware));
#endif

    gcmONERROR(_InitializeFlatMappingRange(hardware));

    gcmTRACE_ZONE(
        gcvLEVEL_INFO, gcvZONE_SIGNAL,
        "%s(%d): stall created signal 0x%08X\n",
        __FUNCTION__, __LINE__,
        hardware->stallSignal);

    /* Return pointer to the gcoHARDWARE object. */
    *Hardware = hardware;

    /* Success. */
    gcmFOOTER_ARG("*Hardware=0x%x", *Hardware);
    return gcvSTATUS_OK;

OnError:
    if (hardware != gcvNULL)
    {
        gcmVERIFY_OK(gcoHARDWARE_Destroy(hardware, ThreadDefault));
    }

    /* Return pointer to the gcoHARDWARE object. */
    *Hardware = gcvNULL;

    /* Return the status. */
    gcmFOOTER();
#endif  /* gcdENABLE_3D*/

    return status;
}

#if gcdENABLE_3D
#if gcdSYNC
static gceSTATUS _DestroyFence(gcoFENCE fence);
#endif
#endif

/*******************************************************************************
**
**  gcoHARDWARE_Destroy
**
**  Destroy an gcoHARDWARE object.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object that needs to be destroyed.
**
**      gctBOOL ThreadDefault
**          Indicate it's thread default hardware or not.
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gcoHARDWARE_Destroy(
    IN gcoHARDWARE Hardware,
    IN gctBOOL    ThreadDefault
    )
{
    gceSTATUS status;
    gcsSTATE_DELTA_PTR deltaHead;
    gctUINT i;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->hw2DDummySurf)
    {
        gcmONERROR(gcoHARDWARE_Free2DSurface(Hardware, Hardware->hw2DDummySurf));
        Hardware->hw2DDummySurf = gcvNULL;
    }

    if (Hardware->hw2DClearDummySurf)
    {
        gcmONERROR(gcoHARDWARE_Free2DSurface(Hardware, Hardware->hw2DClearDummySurf));
        Hardware->hw2DClearDummySurf = gcvNULL;
    }

    if(Hardware->tempBuffer.valid)
    {
        gcmONERROR(gcoHARDWARE_Unlock(&Hardware->tempBuffer, gcvSURF_VERTEX));
        gcmONERROR(gcsSURF_NODE_Destroy(&Hardware->tempBuffer));
    }

#if gcdENABLE_3D
#if gcdSYNC
    _DestroyFence(Hardware->fence);
    Hardware->fence = gcvNULL;
#endif
#endif

    if (Hardware->engine[gcvENGINE_RENDER].queue != gcvNULL && Hardware->stallSignal != gcvNULL)
    {
        gcmONERROR(gcoHARDWARE_Stall(Hardware));
    }

    gcmASSERT(ThreadDefault == Hardware->threadDefault);

    if (Hardware->hw2DCacheFlushSurf)
    {
        gcmONERROR(gcoHARDWARE_Free2DSurface(
            Hardware,
            Hardware->hw2DCacheFlushSurf));

        Hardware->hw2DCacheFlushSurf = gcvNULL;
    }

    if (Hardware->blitTmpSurf)
    {
        gcmONERROR(gcoHARDWARE_Free2DSurface(
            Hardware,
            Hardware->blitTmpSurf));

        Hardware->blitTmpSurf = gcvNULL;
    }

    if (Hardware->hw2DCacheFlushCmd)
    {
        gcmONERROR(gcoOS_Free(gcvNULL, Hardware->hw2DCacheFlushCmd));
    }

#if gcdENABLE_3D
    _DestroyTempRT(Hardware);
#endif

    if (Hardware->clearSrcSurf)
    {
        gcmONERROR(gcoHARDWARE_Free2DSurface(
            Hardware, Hardware->clearSrcSurf));

        Hardware->clearSrcSurf = gcvNULL;
    }

    /* Destroy temporary surface */
    if (Hardware->tempSurface != gcvNULL)
    {
        gcmONERROR(gcoSURF_Destroy(Hardware->tempSurface));
        Hardware->tempSurface = gcvNULL;
    }

    for (i = 0; i < gcdTEMP_SURFACE_NUMBER; i += 1)
    {
        gcoSURF surf = Hardware->temp2DSurf[i];

        if (surf != gcvNULL)
        {
            if (surf->node.valid)
            {
                gcmONERROR(gcoHARDWARE_Unlock(
                    &surf->node, gcvSURF_BITMAP
                    ));
            }

            /* Free the video memory by event. */
            if (surf->node.u.normal.node != 0)
            {
                gcmONERROR(gcsSURF_NODE_Destroy(
                    &Hardware->temp2DSurf[i]->node
                    ));

                surf->node.u.normal.node = 0;
            }

            gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->temp2DSurf[i]));
        }
    }

#if gcdENABLE_3D
    /* Destroy compositon objects. */
    gcmONERROR(gcoHARDWARE_DestroyComposition(Hardware));

    if (Hardware->blitDraw)
    {
#if !gcdENABLE_APPCTXT_BLITDRAW
        /* Only default hardware has blit draw */
        gcmASSERT(Hardware->threadDefault);
#endif

        gcmONERROR(_DestroyBlitDraw(Hardware));
    }

    for (i = 0; i < 2; i++)
    {
        if (Hardware->TXStates->nullTxDescNode[i])
        {
            if (Hardware->TXStates->nullTxDescLocked[i])
            {
                gcmONERROR(gcoSURF_UnLockNode(Hardware->TXStates->nullTxDescNode[i], gcvSURF_TXDESC));
                Hardware->TXStates->nullTxDescLocked[i] = gcvNULL;
            }

            gcmONERROR(gcsSURF_NODE_Destroy(Hardware->TXStates->nullTxDescNode[i]));
            gcmOS_SAFE_FREE(gcvNULL, Hardware->TXStates->nullTxDescNode[i]);
            Hardware->TXStates->nullTxDescNode[i] = gcvNULL;
        }
    }

#if gcdENABLE_3D
    /* Free the state and dirty module in hardware object. */
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->FEStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->PAAndSEStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->MsaaStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->SHStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->PEStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->TXStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->MCStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->QUERYStates));

    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->FEDirty));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->PAAndSEDirty));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->MsaaDirty));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->SHDirty));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->PEDirty));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->TXDirty));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->MCDirty));
#endif

    if (Hardware->XFBStates->internalXFB)
    {
        if (Hardware->XFBStates->internalXFBLocked)
        {
            gcmONERROR(gcoSURF_UnLockNode(Hardware->XFBStates->internalXFBNode, gcvSURF_TFBHEADER));
            Hardware->XFBStates->internalXFBLocked = gcvNULL;
        }
        gcmONERROR(gcsSURF_NODE_Destroy(Hardware->XFBStates->internalXFBNode));
        Hardware->XFBStates->internalXFBNode = gcvNULL;
    }

    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->XFBDirty));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->XFBStates));
#endif

#if (gcdENABLE_3D || gcdENABLE_2D)
    /* Destroy the command buffer. */
    if (Hardware->engine[gcvENGINE_BLT].buffer != gcvNULL)
    {
        gcmONERROR(gcoBUFFER_Destroy(Hardware->engine[gcvENGINE_BLT].buffer));
        Hardware->engine[gcvENGINE_BLT].buffer = gcvNULL;
    }

    if (Hardware->engine[gcvENGINE_BLT].queue != gcvNULL)
    {
        gcmONERROR(gcoQUEUE_Destroy(Hardware->engine[gcvENGINE_BLT].queue));
        Hardware->engine[gcvENGINE_BLT].queue = gcvNULL;
    }

    if (Hardware->engine[gcvENGINE_RENDER].buffer != gcvNULL)
    {
        gcmONERROR(gcoBUFFER_Destroy(Hardware->engine[gcvENGINE_RENDER].buffer));
        Hardware->engine[gcvENGINE_RENDER].buffer = gcvNULL;
    }

    if (Hardware->engine[gcvENGINE_RENDER].queue != gcvNULL)
    {
        gcmONERROR(gcoQUEUE_Destroy(Hardware->engine[gcvENGINE_RENDER].queue));
        Hardware->engine[gcvENGINE_RENDER].queue = gcvNULL;
    }
#endif

#if gcdENABLE_3D
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->hwSlot));
#endif

    /* Free state deltas. */
    for (deltaHead = Hardware->delta; Hardware->delta != gcvNULL;)
    {
        /* Get a shortcut to the current delta. */
        gcsSTATE_DELTA_PTR delta = Hardware->delta;
        gctUINT_PTR mapEntryIndex = gcmUINT64_TO_PTR(delta->mapEntryIndex);
        gctUINT_PTR mapEntryID = gcmUINT64_TO_PTR(delta->mapEntryID);
        gcsSTATE_DELTA_RECORD_PTR recordArray = gcmUINT64_TO_PTR(delta->recordArray);

        /* Get the next delta. */
        gcsSTATE_DELTA_PTR next = gcmUINT64_TO_PTR(delta->next);

        /* Last item? */
        if (next == deltaHead)
        {
            next = gcvNULL;
        }

        /* Free map index array. */
        if (mapEntryIndex != gcvNULL)
        {
            gcmONERROR(gcmOS_SAFE_FREE_SHARED_MEMORY(gcvNULL, mapEntryIndex));
        }

        /* Allocate map ID array. */
        if (mapEntryID != gcvNULL)
        {
            gcmONERROR(gcmOS_SAFE_FREE_SHARED_MEMORY(gcvNULL, mapEntryID));
        }

        /* Free state record array. */
        if (recordArray != gcvNULL)
        {
            gcmONERROR(gcmOS_SAFE_FREE_SHARED_MEMORY(gcvNULL, recordArray));
        }

        /* Free state delta. */
        gcmONERROR(gcmOS_SAFE_FREE_SHARED_MEMORY(gcvNULL, delta));

        /* Remove from the list. */
        Hardware->delta = next;
    }

    if (Hardware->deltas)
    {
        gcmOS_SAFE_FREE(gcvNULL, Hardware->deltas);
    }

#if gcdENABLE_3D
    if (Hardware->contexts)
    {
        for (i = 0; i < Hardware->config->gpuCoreCount; i++)
        {
            /* Detach the process. */
            if (Hardware->contexts[i] != 0)
            {
                gcsHAL_INTERFACE iface;
                iface.command = gcvHAL_DETACH;
                iface.u.Detach.context = Hardware->contexts[i];

                gcmONERROR(gcoOS_DeviceControl(
                    gcvNULL,
                    IOCTL_GCHAL_INTERFACE,
                    &iface, gcmSIZEOF(iface),
                    &iface, gcmSIZEOF(iface)
                    ));

                Hardware->contexts[i] = 0;
            }
        }

        gcmOS_SAFE_FREE(gcvNULL, Hardware->contexts);
    }
#endif

    /* Free temporary buffer allocated by filter blit operation. */
    /* Is there a surface allocated? */
    if (Hardware->tmpSurf.node.pool != gcvPOOL_UNKNOWN)
    {
        gcmONERROR(gcsSURF_NODE_Destroy(&Hardware->tmpSurf.node));

        /* Reset the temporary surface. */
        gcoOS_ZeroMemory(&Hardware->tmpSurf, sizeof(Hardware->tmpSurf));
    }

    /* Destroy the stall signal. */
    if (Hardware->stallSignal != gcvNULL)
    {
        gcmONERROR(gcoOS_DestroySignal(gcvNULL, Hardware->stallSignal));
        Hardware->stallSignal = gcvNULL;
    }

    if (Hardware->config)
    {
        gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware->config));
        Hardware->config = gcvNULL;
    }

    /* Mark the gcoHARDWARE object as unknown. */
    Hardware->object.type = gcvOBJ_UNKNOWN;

    /* Free the gcoHARDWARE object. */
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Hardware));

OnError:

    /* Return the status. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_LoadState
**
**  Load a state buffer.
**
**  INPUT:
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 Address
**          Starting register address of the state buffer.
**
**      gctUINT32 Count
**          Number of states in state buffer.
**
**      gctPOINTER Data
**          Pointer to state buffer.
**
**  OUTPUT:
**
**      Nothing.
*/
#if (gcdENABLE_3D || gcdENABLE_2D)
gceSTATUS
gcoHARDWARE_LoadState(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctUINT32 Count,
    IN gctPOINTER Data
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Address=%x Count=%d Data=0x%x",
                  Hardware, Address, Count, Data);

    /* Call generic function. */
    gcmONERROR(_LoadStatesEx(Hardware, Address >> 2, gcvFALSE, Count, 0, Data));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}
#endif

#if (gcdENABLE_3D || gcdENABLE_2D)
gceSTATUS
gcoHARDWARE_LoadCtrlState(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctUINT32 Data
    )
{
    gceSTATUS status;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER(reserve, stateDelta, memory, reserveSize);

    gcmHEADER_ARG("Hardware=0x%x Address=0x%x Data=0x%x",
                  Hardware, Address, Data);

    /* Verify the arguments. */
    gcmGETHARDWARE(Hardware);
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Determine the size of the buffer to reserve. */
    reserveSize = gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32), 8);

    /* Determine the size of the buffer to reserve. */
    gcmBEGINSTATEBUFFER(Hardware, reserve, stateDelta, memory, reserveSize);

    {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve, gctUINT32_PTR)) & 1) == 0);
    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, (Address >> 2), 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) ((Address >> 2)) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
gcmSETCTRLSTATE(stateDelta, reserve, memory, (Address >> 2), Data );
gcmENDSTATEBATCH(reserve, memory);
};


    stateDelta = stateDelta; /* Keep the compiler happy. */

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER(Hardware, reserve, memory, reserveSize);


OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_LoadCtrlStateNEW(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctUINT32 Data,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x Address=0x%x Data=0x%x Memory=0x%x",
                  Hardware, Address, Data, Memory);

    /* Verify the arguments. */
    gcmGETHARDWARE(Hardware);
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Determine the size of the buffer to reserve. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) ((Address >> 2)) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, (Address >> 2), Data );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    stateDelta = stateDelta; /* Keep the compiler happy. */

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);


OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  gcoHARDWARE_LoadStateX
**
**  Load a state buffer with fixed point states.  The states are meant for the
**  3D pipe.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 Address
**          Starting register address of the state buffer.
**
**      gctUINT32 Count
**          Number of states in state buffer.
**
**      gctPOINTER Data
**          Pointer to state buffer.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_LoadStateX(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctUINT32 Count,
    IN gctPOINTER Data
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Address=%x Count=%d Data=0x%x",
                  Hardware, Address, Count, Data);

    /* Switch to the 3D pipe. */
    gcmONERROR(gcoHARDWARE_SelectPipe(gcvNULL, gcvPIPE_3D, gcvNULL));

    /* Call LoadState function. */
    gcmONERROR(_LoadStatesEx(Hardware, Address >> 2, gcvTRUE, Count, 0, Data));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}
#endif

/*******************************************************************************
**
**  gcoHARDWARE_LoadState32
**
**  Load one 32-bit state.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 Address
**          Register address of the state.
**
**      gctUINT32 Data
**          Value of the state.
**
**  OUTPUT:
**
**      Nothing.
*/
#if (gcdENABLE_3D || gcdENABLE_2D)
gceSTATUS
gcoHARDWARE_LoadState32(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctUINT32 Data
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Address=%x Data=0x%x",
                  Address, Data);

    /* Call generic function. */
    gcmONERROR(_LoadStates(Hardware, Address >> 2, gcvFALSE, 1, 0, &Data));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_LoadState32NEW
**
**  Load one 32-bit state.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 Address
**          Register address of the state.
**
**      gctUINT32 Data
**          Value of the state.
**
**       gctPOINTER *Memory
**          Pointer to the temp command buffer pointer
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_LoadState32NEW(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctUINT32 Data,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Address=%x Data=0x%x Memory=0x%x",
                  Address, Data, Memory);

    /* Call generic function. */
    gcmONERROR(_LoadStatesNEW(Hardware, Address >> 2, gcvFALSE, 1, 0, &Data, Memory));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  gcoHARDWARE_LoadState32WithMask
**
**  Load one 32-bit state.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 Address
**          Register address of the state.
**
**      gctUINT32 Mask
**          Register mask of the state
**
**      gctUINT32 Data
**          Value of the state.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_LoadState32WithMask(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctUINT32 Mask,
    IN gctUINT32 Data
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Address=%x Data=0x%x",
                  Address, Data);

    /* Call generic function. */
    gcmONERROR(_LoadStates(Hardware, Address >> 2, gcvFALSE, 1, Mask, &Data));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_LoadState32WithMaskNEW
**
**  Load one 32-bit state with optional external command buffer.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 Address
**          Register address of the state.
**
**      gctUINT32 Mask
**          Register mask of the state
**
**      gctUINT32 Data
**          Value of the state.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_LoadState32WithMaskNEW(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctUINT32 Mask,
    IN gctUINT32 Data,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Address=%x Data=0x%x",
                  Address, Data);

    /* Call generic function. */
    gcmONERROR(_LoadStatesNEW(Hardware, Address >> 2, gcvFALSE, 1, Mask, &Data, Memory));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}


#endif

/*******************************************************************************
**
**  gcoHARDWARE_LoadState32x
**
**  Load one 32-bit state, which is represented in 16.16 fixed point.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 Address
**          Register address of the state.
**
**      gctFIXED_POINT Data
**          Value of the state in 16.16 fixed point format.
**
**  OUTPUT:
**
**      Nothing.
*/
#if (gcdENABLE_3D || gcdENABLE_2D)
gceSTATUS
gcoHARDWARE_LoadState32x(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctFIXED_POINT Data
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Address=%x Data=0x%x",
                  Hardware, Address, Data);

    /* Call generic function. */
    gcmONERROR(_LoadStates(Hardware, Address >> 2, gcvTRUE, 1, 0, &Data));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}
#endif

/*******************************************************************************
**
**  gcoHARDWARE_LoadState64
**
**  Load one 64-bit state.
**
**  INPUT:
**
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**
**      gctUINT32 Address
**          Register address of the state.
**
**      gctUINT64 Data
**          Value of the state.
**
**  OUTPUT:
**
**      Nothing.
*/
#if (gcdENABLE_3D || gcdENABLE_2D)
gceSTATUS
gcoHARDWARE_LoadState64(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctUINT64 Data
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Address=%x Data=0x%x",
                  Hardware, Address, Data);

    /* Call generic function. */
    gcmONERROR(_LoadStates(Hardware, Address >> 2, gcvFALSE, 2, 0, &Data));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}
#endif

#if gcdENABLE_3D
#if gcdSYNC
static gctBOOL  _GetFenceCtx(
                gcsSYNC_CONTEXT_PTR head,
                gcoFENCE fence,
                gcsSYNC_CONTEXT_PTR * ctx
                )
{
     gcsSYNC_CONTEXT_PTR ptr;

     if (fence == gcvNULL ||
         head == gcvNULL  ||
         ctx == gcvNULL)
     {
         return gcvFALSE;
     }

     ptr = head;
     while(ptr)
     {
         if (ptr->fence == fence)
         {
             *ctx = ptr;
             return gcvTRUE;
         }

         ptr = ptr->next;
     }

     return gcvFALSE;
}

static gctBOOL _SetFenceCtx(
                gcsSYNC_CONTEXT_PTR * head,
                gcoFENCE fence,
                gceFENCE_TYPE type
                )
{
     gcsSYNC_CONTEXT_PTR ptr;

     if (fence == gcvNULL ||
         head  == gcvNULL)
     {
         return gcvFALSE;
     }

     ptr = *head;

     while(ptr)
     {
         if (ptr->fence == fence)
         {
             if (type & gcvFENCE_TYPE_WRITE)
             {
                ptr->writeFenceID = fence->fenceID;
             }

             if (type & gcvFENCE_TYPE_READ)
             {
                 ptr->readFenceID = fence->fenceID;
             }

             ptr->mark    = gcvTRUE;

             return gcvTRUE;
         }

         ptr = ptr->next;
     }

     if(gcmIS_ERROR(gcoOS_Allocate(gcvNULL,sizeof(gcsSYNC_CONTEXT),(gctPOINTER *)&ptr)))
     {
         fence->fenceEnable = gcvFALSE;
         return gcvFALSE;
     }

     ptr->fence     = fence;
     ptr->writeFenceID = 0;
     ptr->readFenceID  = 0;

     if (type & gcvFENCE_TYPE_WRITE)
     {
         ptr->writeFenceID = fence->fenceID;
     }

     if (type & gcvFENCE_TYPE_READ)
     {
         ptr->readFenceID = fence->fenceID;
     }

     ptr->mark      = gcvTRUE;
     ptr->next      = *head;
     *head           = ptr;

     return gcvTRUE;
}


static gceSTATUS
_DestroyFence(gcoFENCE fence)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER();

    if (fence)
    {
        if (fence->type == gcvFENCE_RLV)
        {
            if (fence->u.rlvFence.fenceSurface != gcvNULL)
            {
                gcmONERROR(gcoSURF_Unlock(fence->u.rlvFence.fenceSurface,gcvNULL));
                gcmONERROR(gcoSURF_Destroy(fence->u.rlvFence.fenceSurface));
                fence->u.rlvFence.fenceSurface = gcvNULL;
            }

            if (fence->u.rlvFence.srcIDSurface != gcvNULL)
            {
                gcmONERROR(gcoSURF_Unlock(fence->u.rlvFence.srcIDSurface,gcvNULL));
                gcmONERROR(gcoSURF_Destroy(fence->u.rlvFence.srcIDSurface));
                fence->u.rlvFence.srcIDSurface = gcvNULL;
            }
        }
        else if (fence->type == gcvFENCE_HW)
        {
            if (fence->u.hwFence.dstSurfNode != gcvNULL)
            {
                gcmONERROR(gcoSURF_UnLockNode(fence->u.hwFence.dstSurfNode, gcvSURF_FENCE));
                gcmONERROR(gcsSURF_NODE_Destroy(fence->u.hwFence.dstSurfNode));
                gcmOS_SAFE_FREE(gcvNULL, fence->u.hwFence.dstSurfNode);
                fence->u.hwFence.dstSurfNode = gcvNULL;
            }
        }

        gcmONERROR(gcoOS_Free(gcvNULL,fence));
    }

OnError:
    /* Return the status. */
    gcmFOOTER_NO();
    return status;
}

static gceSTATUS
_ConstructFence(
     IN gcoHARDWARE Hardware,
     IN OUT gcoFENCE * Fence
     )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoHARDWARE hardware;
    gctPOINTER ptr[3];
    gcoFENCE fence = gcvNULL;
    gcsHAL_INTERFACE iface;

    gcmHEADER();

    hardware = Hardware;


    if (hardware == gcvNULL)
    {
        gcmFOOTER();
        return gcvSTATUS_FALSE;
    }

    gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof(struct _gcoFENCE), (gctPOINTER *)&fence));

    gcoOS_ZeroMemory(fence,sizeof(struct _gcoFENCE));

    if (Hardware->features[gcvFEATURE_HALTI2])
    {
        fence->type = gcvFENCE_HW;
    }
    else
    {
        fence->type = gcvFENCE_RLV;
    }

    if (fence->type == gcvFENCE_RLV)
    {
        fence->u.rlvFence.srcWidth  = hardware->resolveAlignmentX * 16;
        fence->u.rlvFence.srcHeight = hardware->resolveAlignmentY * 16;

        /* Aligned to super tile */
        fence->u.rlvFence.srcWidth = gcmALIGN(fence->u.rlvFence.srcWidth, 64);
        fence->u.rlvFence.srcHeight = gcmALIGN(fence->u.rlvFence.srcHeight, 64);

        fence->u.rlvFence.srcX      = 0;
        fence->u.rlvFence.srcY      = 0;

        gcmONERROR(gcoSURF_Construct(gcvNULL,
                                     hardware->resolveAlignmentX, hardware->resolveAlignmentY, 1,
                                     gcvSURF_TEXTURE | gcvSURF_TILE_RLV_FENCE,
                                     gcvSURF_A8R8G8B8,
                                     gcvPOOL_DEFAULT,
                                     &fence->u.rlvFence.fenceSurface));

        gcmONERROR(gcoSURF_Construct(gcvNULL,
                                     fence->u.rlvFence.srcWidth, fence->u.rlvFence.srcHeight, 1,
                                     gcvSURF_TEXTURE | gcvSURF_TILE_RLV_FENCE,
                                     gcvSURF_A8R8G8B8,
                                     gcvPOOL_DEFAULT,
                                     &fence->u.rlvFence.srcIDSurface));

        gcmONERROR(gcoSURF_Lock(fence->u.rlvFence.srcIDSurface,gcvNULL, ptr));

        gcoOS_ZeroMemory(ptr[0],fence->u.rlvFence.srcWidth * fence->u.rlvFence.srcHeight * 4);

        gcoSURF_CPUCacheOperation(fence->u.rlvFence.srcIDSurface, gcvCACHE_CLEAN);

        gcmONERROR(gcoSURF_Lock(fence->u.rlvFence.fenceSurface,gcvNULL, ptr));

        gcoOS_ZeroMemory(ptr[0],hardware->resolveAlignmentX * hardware->resolveAlignmentY * 4);

        gcoSURF_CPUCacheOperation(fence->u.rlvFence.fenceSurface, gcvCACHE_CLEAN);
    }
    else if (fence->type == gcvFENCE_HW)
    {
        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                  gcmSIZEOF(gcsSURF_NODE),
                                  (gctPOINTER *)&fence->u.hwFence.dstSurfNode));


        gcsSURF_NODE_Construct(fence->u.hwFence.dstSurfNode,
                               8, 64, gcvSURF_FENCE, 0, gcvPOOL_DEFAULT);


        gcmONERROR(gcoSURF_LockNode(fence->u.hwFence.dstSurfNode, &fence->u.hwFence.dstPhysic, &fence->u.hwFence.dstAddr));

        gcoOS_ZeroMemory(fence->u.hwFence.dstAddr,8);

        gcoSURF_NODE_CPUCacheOperation(fence->u.hwFence.dstSurfNode,
                                       gcvSURF_FENCE,
                                       0,
                                       8,
                                       gcvCACHE_CLEAN);
    }

    fence->fenceID   = 1;
    fence->commitID  = 0;

    fence->fenceIDSend = 0;

    fence->addSync      = gcvFALSE;
    fence->fromCommit   = gcvFALSE;
    fence->fenceEnable  = gcvTRUE;

    if (hardware->patchID == gcvPATCH_BASEMARK2V2)
    {
        if ((hardware->config->chipModel == gcv2000) &&
            (hardware->config->chipRevision == 0x5108))
        {
            fence->loopCount    = 100;
        }
        else
        {
            fence->loopCount    = 20000;
        }
    }
    else
    {
        fence->loopCount    = gcdFENCE_WAIT_LOOP_COUNT;
    }

#if gcdFPGA_BUILD
    fence->delayCount   = 20000000;
#else
    fence->delayCount   = 20000;
#endif

    iface.u.QueryResetTimeStamp.timeStamp = 0;

    iface.command = gcvHAL_QUERY_RESET_TIME_STAMP;

    gcoOS_DeviceControl(gcvNULL,
        IOCTL_GCHAL_INTERFACE,
        &iface, gcmSIZEOF(iface),
        &iface, gcmSIZEOF(iface));

    fence->resetTimeStamp = iface.u.QueryResetTimeStamp.timeStamp;

    *Fence = fence;

    gcmFOOTER();
    return status;

OnError:
    _DestroyFence(fence);

    *Fence = gcvNULL;
    /* Return result. */
    gcmFOOTER();
    return status;

}

static void _ResetFence(gcoFENCE fence)
{
    if (fence == gcvNULL)
        return;

    gcmPRINT("Reset Fence!");
    if (fence->type == gcvFENCE_RLV)
    {
        /* Clear the Rlv dst to the hight sendID */
        *(gctUINT64 *)fence->u.rlvFence.fenceSurface->node.logical = fence->fenceID;
    }
    else if (fence->type == gcvFENCE_HW)
    {
        /* Clear the Rlv dst to the hight sendID */
        *(gctUINT32 *)fence->u.hwFence.dstAddr = (gctUINT32)fence->fenceID;
    }

    fence->fenceIDSend = fence->fenceID;
    fence->addSync = gcvFALSE;
    fence->fromCommit = gcvFALSE;
    fence->commitID = fence->fenceID;

    return;
}

#define OQ_FENCE_BACK(ptr) ((*(gctUINT32*)ptr) == 0)

static gctBOOL _IsRlvFenceBack(gcsSYNC_CONTEXT_PTR ctx, gcoFENCE fence, gctUINT64 waitID)
{
    gctUINT64   *backFenceAddr;

    backFenceAddr = (gctUINT64 *)fence->u.rlvFence.fenceSurface->node.logical;

    if (waitID <= *backFenceAddr)
    {
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}

#define HW_FENCE_LESS_EQUAL(id0,id1) (((id0 <= id1) || ((id0 > id1) && ((id0 - id1) > 0xf0000000))) ? gcvTRUE : gcvFALSE)

static gctBOOL _IsHWFenceBack(gcsSYNC_CONTEXT_PTR ctx, gcoFENCE fence, gctUINT64 waitID)
{
    gctUINT32   backID;

    /* HW only have 32bit data, there is no way that we take all 2^32 times send without query once.
       So, compare with SendID to get the 64bit back ID
    */
    backID = *(gctUINT32 *)fence->u.hwFence.dstAddr;

    if (HW_FENCE_LESS_EQUAL((gctUINT32)(waitID), backID))
    {
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}

static gctBOOL _IsFenceBack(gcsSYNC_CONTEXT_PTR ctx, gcoFENCE fence, gctUINT64 waitID)
{
    if (fence->type == gcvFENCE_RLV)
    {
        return _IsRlvFenceBack(ctx, fence, waitID);
    }
    else if (fence->type == gcvFENCE_HW)
    {
        return _IsHWFenceBack(ctx, fence, waitID);
    }

    return gcvTRUE;
}

#if defined(EMULATOR)
#define gcdFENCE_WAIT_INTERVAL 10
#else
#define gcdFENCE_WAIT_INTERVAL 1
#endif

static void _WaitRlvFenceBack(gctUINT64 id, gcoFENCE fence)
{
#if !gcdNULL_DRIVER
    gctUINT32 i,delayCount;
    gctUINT64 * backAddr;

    backAddr = (gctUINT64 *)fence->u.rlvFence.fenceSurface->node.logical;
    delayCount = fence->delayCount;

    while(gcvTRUE)
    {
        i = fence->loopCount;

        do {
            /* Flush cach when read */
            gcoSURF_CPUCacheOperation(fence->u.rlvFence.fenceSurface, gcvCACHE_INVALIDATE);

            if (id <= *backAddr)
            {
                return;
            }
        }
        while(i--);

        gcoOS_Delay(gcvNULL, gcdFENCE_WAIT_INTERVAL);

        delayCount--;

        if (delayCount == 0)
            break;
    }

    /* Time Out */
    gcmPRINT("Fence Wait TimeOut!");
    {
        gcsHAL_INTERFACE iface;
        iface.u.QueryResetTimeStamp.timeStamp = 0;

        iface.command = gcvHAL_QUERY_RESET_TIME_STAMP;

        gcoOS_DeviceControl(gcvNULL,
            IOCTL_GCHAL_INTERFACE,
            &iface, gcmSIZEOF(iface),
            &iface, gcmSIZEOF(iface));

        if (iface.u.QueryResetTimeStamp.timeStamp != fence->resetTimeStamp)
        {
            fence->resetTimeStamp = iface.u.QueryResetTimeStamp.timeStamp;
            _ResetFence(fence);
        }
    }
#endif
    return;
}

static void _WaitHWFenceBack(gctUINT64 id, gcoFENCE fence)
{
#if !gcdNULL_DRIVER
    gctUINT32 i;
    gctUINT32 * backAddr = (gctUINT32 *)fence->u.hwFence.dstAddr;
    gctUINT32 backID;
#ifndef EMULATOR
    gctUINT32 delayCount = fence->delayCount;
#endif

    while(gcvTRUE)
    {
        i = fence->loopCount;

        do {
            /* Flush cach when read */
            gcoSURF_NODE_CPUCacheOperation(fence->u.hwFence.dstSurfNode,
                                           gcvSURF_FENCE, 0, 8, gcvCACHE_INVALIDATE);
            backID = *backAddr;

            if (HW_FENCE_LESS_EQUAL((gctUINT32)id ,backID))
            {
                return;
            }
        }
        while(i--);

        gcoOS_Delay(gcvNULL, gcdFENCE_WAIT_INTERVAL);

#ifndef EMULATOR

        delayCount--;

        if (delayCount == 0)
            break;
#endif

    }

    /* Time Out */
    gcmPRINT("Fence Wait TimeOut!");
    {
        gcsHAL_INTERFACE iface;
        iface.u.QueryResetTimeStamp.timeStamp = 0;

        iface.command = gcvHAL_QUERY_RESET_TIME_STAMP;

        gcoOS_DeviceControl(gcvNULL,
            IOCTL_GCHAL_INTERFACE,
            &iface, gcmSIZEOF(iface),
            &iface, gcmSIZEOF(iface));

        if (iface.u.QueryResetTimeStamp.timeStamp != fence->resetTimeStamp)
        {
            fence->resetTimeStamp = iface.u.QueryResetTimeStamp.timeStamp;
            _ResetFence(fence);
        }
    }
#endif
    return;
}

static gceSTATUS _WaitFenceBack(gcsSYNC_CONTEXT_PTR ctx, gcoFENCE fence,gctBOOL commit, gctUINT64 waitID)
{
    gceSTATUS status = gcvSTATUS_OK;

    if (commit)
    {
        gcmONERROR(gcoHARDWARE_SendFence(gcvNULL, gcvFALSE, gcvNULL));
        /* We have a stupid commit in Resolve, I am not commiting again here */
        gcmONERROR(gcoHARDWARE_Commit(gcvNULL));
    }

    if (fence->type == gcvFENCE_RLV)
    {
        _WaitRlvFenceBack(waitID, fence);
    }
    else if (fence->type == gcvFENCE_HW)
    {
        _WaitHWFenceBack(waitID, fence);
    }

OnError:
    return status;
}


static gceSTATUS
_SendHWFence(
    gcoHARDWARE Hardware,
    gctUINT32 physic,
    gctUINT32 data
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER *outside = gcvNULL;

    gcmDEFINECTRLSTATEBUFFER(reserve, memory);

    gcmHEADER_ARG("Hardware=0x%x Physic=%d", Hardware, physic);

    gcmGETHARDWARE(Hardware);

    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Switch to 3D pipe. */
    gcmONERROR(gcoHARDWARE_SelectPipe(Hardware, gcvPIPE_3D, gcvNULL));

    gcmBEGINCTRLSTATEBUFFER(Hardware, reserve, memory, outside);

    gcoHARDWARE_MultiGPUSync(Hardware, &memory);

    { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0);
 memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0));
 } };


    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E1A) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E1A, physic);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E1B) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E1B, data);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};



    { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK;
 memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };

    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, outside);
OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_GetFence(
    IN gcoHARDWARE Hardware,
    IN gcsSYNC_CONTEXT_PTR *Ctx,
    IN gceFENCE_TYPE Type
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoFENCE fence;

    gcmHEADER_ARG("Hardware=0x%x Ctx=0x%x",Hardware, Ctx);

    gcmGETHARDWARE(Hardware);

    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->fence == gcvNULL)
    {
        _ConstructFence(Hardware,&Hardware->fence);
    }

    fence = Hardware->fence;
    if (!fence || !fence->fenceEnable || (Ctx == gcvNULL))
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    if (!_SetFenceCtx(Ctx,fence, Type))
    {
        gcmFOOTER();
        return gcvSTATUS_FALSE;
    }

    fence->addSync = gcvTRUE;

OnError:
    /* Return result. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SendFence(
    IN gcoHARDWARE Hardware,
    IN gctBOOL IsFromCommit,
    OUT gctPOINTER *Memory
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoFENCE   fence = gcvNULL;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    gcmGETHARDWARE(Hardware);

    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->fence == gcvNULL)
    {
        _ConstructFence(Hardware,&Hardware->fence);
    }

    fence = Hardware->fence;

    if (!fence || !fence->fenceEnable)
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    /*Send fence/OQ/(Set commit update id)*/
    if (fence->addSync)
    {
        if (fence->type == gcvFENCE_RLV)
        {
            gctUINT64 * ptrFence;
            gctUINT64 * ptrSrc;
            gctPOINTER  srcAddrs[gcdMAX_SURF_LAYERS] = {gcvNULL};
            gcsSURF_VIEW srcView = {fence->u.rlvFence.srcIDSurface, 0, 1};
            gcsSURF_VIEW fenceView = {fence->u.rlvFence.fenceSurface, 0, 1};
            gcsSURF_RESOLVE_ARGS rlvArgs = {0};

            if (IsFromCommit && Memory)
            {
                gcmFOOTER();
                return gcvSTATUS_OK;
            }

            /* fill ID to srcIDSurface */
            ptrFence = (gctUINT64 *)fence->u.rlvFence.fenceSurface->node.logical;

            fence->u.rlvFence.srcIDSurface->pfGetAddr(fence->u.rlvFence.srcIDSurface,
                                                      fence->u.rlvFence.srcX,
                                                      fence->u.rlvFence.srcY,
                                                      0,
                                                      srcAddrs);
            ptrSrc = (gctUINT64 *)(srcAddrs[0]);

            /* flush cache when read */
             gcoSURF_CPUCacheOperation(fence->u.rlvFence.srcIDSurface, gcvCACHE_INVALIDATE);
             gcoSURF_CPUCacheOperation(fence->u.rlvFence.fenceSurface, gcvCACHE_INVALIDATE);
            /* Can we use the src slot? */
            if (*ptrSrc > *ptrFence)
            {
                if (*ptrSrc > fence->commitID)
                {
                    gcoHARDWARE_Commit(Hardware);
                }

                _WaitRlvFenceBack(*ptrSrc,fence);
            }

            *ptrSrc = fence->fenceID;

            /*Flush after write */
            gcoSURF_CPUCacheOperation(fence->u.rlvFence.srcIDSurface, gcvCACHE_CLEAN);

            rlvArgs.version = gcvHAL_ARG_VERSION_V2;
            rlvArgs.uArgs.v2.srcOrigin.x = fence->u.rlvFence.srcX;
            rlvArgs.uArgs.v2.srcOrigin.y = fence->u.rlvFence.srcY;
            rlvArgs.uArgs.v2.rectSize.x  = Hardware->resolveAlignmentX;
            rlvArgs.uArgs.v2.rectSize.y  = Hardware->resolveAlignmentY;
            rlvArgs.uArgs.v2.numSlices   = 1;
            gcmONERROR(gcoSURF_ResolveRect(&srcView, &fenceView, &rlvArgs));

            fence->fenceIDSend = fence->fenceID;

            fence->u.rlvFence.srcX += Hardware->resolveAlignmentX;
            if (fence->u.rlvFence.srcX >= fence->u.rlvFence.srcWidth)
            {
                fence->u.rlvFence.srcX = 0;
                fence->u.rlvFence.srcY += Hardware->resolveAlignmentY;
                if (fence->u.rlvFence.srcY >= fence->u.rlvFence.srcHeight)
                {
                    fence->u.rlvFence.srcY = 0;
                }
            }
        }
        else if (fence->type == gcvFENCE_HW)
        {
            if (IsFromCommit)
            {
                if (Memory)
                {
                    gctUINT32_PTR memory = (gctUINT32_PTR)*Memory;

                    if (!fence->fromCommit)
                    {
                        gcmFOOTER();
                        return gcvSTATUS_OK;
                    }

                    gcoHARDWARE_MultiGPUSync(Hardware, &memory);

                    { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0);
 memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0));
 } };


                    gcoHARDWARE_LoadCtrlStateNEW(Hardware, 0x03868, fence->u.hwFence.dstPhysic, (gctPOINTER *)&memory);
                    gcoHARDWARE_LoadCtrlStateNEW(Hardware, 0x0386C, (gctUINT32)fence->fenceID, (gctPOINTER *)&memory);

                    { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK;
 memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };


                    *Memory = memory;
                    fence->fromCommit = gcvFALSE;
                }
                else
                {
                    fence->fromCommit = gcvTRUE;
                    gcmFOOTER();
                    return gcvSTATUS_OK;
                }
            }
            else
            {
                _SendHWFence(Hardware,fence->u.hwFence.dstPhysic, (gctUINT32)fence->fenceID);
            }
            fence->fenceIDSend = fence->fenceID;
        }

        fence->addSync = gcvFALSE;
        fence->fenceID++;
    }

    status = gcvSTATUS_OK;

    /* Return result. */
    gcmFOOTER();
    return status;

OnError:
    /* Return result. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_WaitFence(
    IN gcoHARDWARE Hardware,
    IN gcsSYNC_CONTEXT_PTR Ctx,
    IN gceFENCE_TYPE Type
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoFENCE    fence = gcvNULL;
    gcsSYNC_CONTEXT_PTR fenceCtx = gcvNULL;
    gctUINT64 waitID = 0;

    gcmHEADER_ARG("Hardware=0x%x Ctx=0x%x",Hardware, Ctx);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->fence == gcvNULL)
    {
        _ConstructFence(Hardware, &Hardware->fence);
    }

    fence = Hardware->fence;

    if (!fence || !fence->fenceEnable || (Ctx == gcvNULL))
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    if (!_GetFenceCtx(Ctx,fence,&fenceCtx))
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    if (!fenceCtx->mark)
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    /* no matter what happend, mark to false*/
    fenceCtx->mark = gcvFALSE;

    if(fence->type == gcvFENCE_RLV)
    {
         gcoSURF_CPUCacheOperation(fence->u.rlvFence.fenceSurface, gcvCACHE_INVALIDATE);
    }
    else if(fence->type == gcvFENCE_HW)
    {
        gcoSURF_NODE_CPUCacheOperation(fence->u.hwFence.dstSurfNode,
                                       gcvSURF_FENCE, 0, 8, gcvCACHE_INVALIDATE);
    }

    if (Type == gcvFENCE_TYPE_WRITE)
    {
        waitID = fenceCtx->writeFenceID;
    }
    else if (Type == gcvFENCE_TYPE_READ)
    {
        waitID = fenceCtx->readFenceID;
    }
    else if (Type == gcvFENCE_TYPE_ALL)
    {
        waitID = gcmMAX(fenceCtx->writeFenceID, fenceCtx->readFenceID);
    }

    if (_IsFenceBack(fenceCtx, fence, waitID))
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }
    else if(waitID < fence->commitID)
    {
        gcmONERROR(_WaitFenceBack(fenceCtx, fence, gcvFALSE, waitID));
    }
    else
    {
        gcmONERROR(_WaitFenceBack(fenceCtx, fence, gcvTRUE, waitID));
    }

OnError:
    /* Return result. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_GetFenceEnabled(
    IN gcoHARDWARE Hardware,
    OUT gctBOOL_PTR Enabled
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enabled=0x%x",Hardware, Enabled);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmVERIFY_ARGUMENT(Enabled != gcvNULL);

    *Enabled = Hardware->fenceEnabled;

OnError:
    /* Return result. */
    gcmFOOTER();
    return status;

}

gceSTATUS
gcoHARDWARE_SetFenceEnabled(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enabled
    )

{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enabled=%d",Hardware, Enabled);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    Hardware->fenceEnabled = Enabled;

OnError:

    /* Return result. */
    gcmFOOTER();
    return status;

}

#endif


#endif

#if gcdENABLE_3D
gceSTATUS
gcoHARDWARE_SetHWSlot(
    IN gcoHARDWARE Hardware,
    IN gceENGINE Engine,
    IN gctUINT type,
    IN gctUINT32 node,
    IN gctUINT32 slot
)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmGETHARDWARE(Hardware);

    if (!gcoHAL_GetOption(gcvNULL, gcvOPTION_KERNEL_FENCE))
    {
        return status;
    }

    switch (type)
    {
    case gcvHWSLOT_INDEX:
        if (Hardware->hwSlot->renderEngine.index != node)
        {
            gcoBUFFER_InsertToPatchList(Hardware->engine[gcvENGINE_RENDER].buffer, Hardware->hwSlot->renderEngine.index, gcvENGINE_RENDER);
            Hardware->hwSlot->renderEngine.index = node;
        }
        break;

    case gcvHWSLOT_STREAM:
        if (slot < Hardware->hwSlot->renderEngine.streamMaxCount &&
            Hardware->hwSlot->renderEngine.streams[slot] != node)
        {
            if (Hardware->hwSlot->renderEngine.streams[slot] != 0)
            {
                gcoBUFFER_InsertToPatchList(Hardware->engine[gcvENGINE_RENDER].buffer, Hardware->hwSlot->renderEngine.streams[slot], gcvENGINE_RENDER);
                Hardware->hwSlot->renderEngine.streams[slot] = 0;
                Hardware->hwSlot->renderEngine.streamCount--;
            }

            Hardware->hwSlot->renderEngine.streams[slot] = node;

            if (node != 0)
            {
                Hardware->hwSlot->renderEngine.streamCount++;
            }
        }
        break;

    case gcvHWSLOT_TEXTURE:
        if (slot < Hardware->hwSlot->renderEngine.textureMaxCount &&
            Hardware->hwSlot->renderEngine.textures[slot] != node)
        {
            if (Hardware->hwSlot->renderEngine.textures[slot] != 0)
            {
                gcoBUFFER_InsertToPatchList(Hardware->engine[gcvENGINE_RENDER].buffer, Hardware->hwSlot->renderEngine.textures[slot], gcvENGINE_RENDER);
                Hardware->hwSlot->renderEngine.textures[slot] = 0;
                Hardware->hwSlot->renderEngine.textureCount--;
            }

            Hardware->hwSlot->renderEngine.textures[slot] = node;

            if (node != 0)
            {
                Hardware->hwSlot->renderEngine.textureCount++;
            }
        }
        break;

    case gcvHWSLOT_RT:
        if (slot < Hardware->hwSlot->renderEngine.rtMaxCount &&
            Hardware->hwSlot->renderEngine.rt[slot] != node)
        {
            if (Hardware->hwSlot->renderEngine.rt[slot] != 0)
            {
                gcoBUFFER_InsertToPatchList(Hardware->engine[gcvENGINE_RENDER].buffer, Hardware->hwSlot->renderEngine.rt[slot], gcvENGINE_RENDER);
                Hardware->hwSlot->renderEngine.rt[slot] = 0;
                Hardware->hwSlot->renderEngine.rtCount--;
            }

            Hardware->hwSlot->renderEngine.rt[slot] = node;

            if (node != 0)
            {
                Hardware->hwSlot->renderEngine.rtCount++;
            }
        }
        break;

    case gcvHWSLOT_DEPTH_STENCIL:
        if (Hardware->hwSlot->renderEngine.depthStencil != node)
        {
            gcoBUFFER_InsertToPatchList(Hardware->engine[gcvENGINE_RENDER].buffer, Hardware->hwSlot->renderEngine.depthStencil, gcvENGINE_RENDER);
            Hardware->hwSlot->renderEngine.depthStencil = node;
        }
        break;

    case gcvHWSLOT_BLT_SRC:
    case gcvHWSLOT_BLT_DST:
        if (node != 0)
        {
            gcoBUFFER_InsertToPatchList(Hardware->engine[Engine].buffer, node, Engine);
        }
        break;

    default:
        break;
    }

OnError:
    return status;
}

void _buildRenderPatchList(gcoBUFFER buffer, gctUINT32 count, gctUINT32 maxCount, gctUINT32 *slots)
{
        gctUINT i, j;

        if (count > 0)
        {
            for (i = 0, j = 0; i < maxCount; i++)
            {
                if (slots[i] != 0)
                {
                    gcoBUFFER_InsertToPatchList(buffer, slots[i], gcvENGINE_RENDER);

                    j++;

                    if (j == count)
                        break;
                }
            }
        }
}
/*******************************************************************************
**
**  gcoHardware_BuildPatchList
**
**  Build patchList of current command buffer.
**
**  INPUT:
**
**      gcoCMDBUF CommandBuffer
**          Pointer to an gcoCMDBUF object.
**
**  OUTPUT:
**
**      None.
*/
gceSTATUS
gcoHARDWARE_BuildPatchList(
    IN gcoHARDWARE Hardware,
    IN gcoBUFFER Buffer,
    IN gcoCMDBUF CommandBuffer,
    IN gceENGINE Engine
    )
{
    gcmHEADER_ARG("CommandBuffer=0x%x", CommandBuffer);

    /* Loop all hw slot, add the node to patchList */
    if (gcoHAL_GetOption(gcvNULL, gcvOPTION_KERNEL_FENCE) &&
        Engine == gcvENGINE_RENDER)
    {
        if (Hardware->hwSlot->renderEngine.index != 0)
        {
            gcoBUFFER_InsertToPatchList(Buffer, Hardware->hwSlot->renderEngine.index, gcvENGINE_RENDER);
        }

        if (Hardware->hwSlot->renderEngine.depthStencil != 0)
        {
            gcoBUFFER_InsertToPatchList(Buffer, Hardware->hwSlot->renderEngine.depthStencil, gcvENGINE_RENDER);
        }

        _buildRenderPatchList(Buffer, Hardware->hwSlot->renderEngine.streamCount,
                              Hardware->hwSlot->renderEngine.streamMaxCount,
                              Hardware->hwSlot->renderEngine.streams);

        _buildRenderPatchList(Buffer,Hardware->hwSlot->renderEngine.rtCount,
                              Hardware->hwSlot->renderEngine.rtMaxCount,
                              Hardware->hwSlot->renderEngine.rt);

        _buildRenderPatchList(Buffer,Hardware->hwSlot->renderEngine.textureCount,
                              Hardware->hwSlot->renderEngine.textureMaxCount,
                              Hardware->hwSlot->renderEngine.textures);
    }

    /* Success. */
    gcmFOOTER_NO();

    return gcvSTATUS_OK;
}

gceSTATUS
gcoHARDWARE_ResetHWSlot(
    IN gcoHARDWARE Hardware
)
{
    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    gcoOS_ZeroMemory(Hardware->hwSlot, gcmSIZEOF(gcsHARDWARE_SLOT));

    Hardware->hwSlot->renderEngine.rtMaxCount = 8;
    Hardware->hwSlot->renderEngine.streamMaxCount  = gcdSTREAMS;
    Hardware->hwSlot->renderEngine.textureMaxCount = gcdSAMPLERS;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}
#endif

/*******************************************************************************
**
**  gcoHARDWARE_Commit
**
**  Commit the current command buffer to the hardware.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_Commit(
    IN gcoHARDWARE Hardware
    )
{
#if gcdENABLE_3D
    gctUINT i;
#endif
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if (gcdENABLE_3D || gcdENABLE_2D)
    gctPOINTER dumpCommandLogical;
    gctUINT32 dumpCommandBytes = 0;
    gctPOINTER dumpAsyncCommandLogical = gcvNULL;
    gctUINT32 dumpAsyncCommandBytes = 0;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

#if gcdENABLE_3D
    if (Hardware->deltas)
    {
        for (i = 1; i < Hardware->config->gpuCoreCount; i++)
        {
            gcoHARDWARE_CopyDelta(Hardware->deltas[i], Hardware->deltas[0]);
        }
    }
#endif

    /* Commit command buffer and return status. */
    if (gcoHAL_GetOption(gcvNULL, gcvOPTION_KERNEL_FENCE) &&
        Hardware->features[gcvFEATURE_BLT_ENGINE])
    {
        gcmVERIFY_OK(gcoBUFFER_Commit(
            Hardware->engine[gcvENGINE_BLT].buffer,
            gcvPIPE_INVALID,
            gcvNULL,
            gcvNULL,
            gcvNULL,
            Hardware->engine[gcvENGINE_BLT].queue,
            &dumpAsyncCommandLogical,
            &dumpAsyncCommandBytes
            ));
    }

    status = gcoBUFFER_Commit(
        Hardware->engine[gcvENGINE_RENDER].buffer,
        Hardware->currentPipe,
        Hardware->delta,
        Hardware->deltas,
        Hardware->contexts,
        Hardware->engine[gcvENGINE_RENDER].queue,
        &dumpCommandLogical,
        &dumpCommandBytes
        );

#if gcdENABLE_3D
    if (Hardware->deltas)
    {
        for (i = 0; i < Hardware->config->gpuCoreCount; i++)
        {
            _UpdateDelta(Hardware, i);
        }

        /* TODO: Remove it. For now, all states updating happen in this delta, so keep it.*/
        Hardware->delta = Hardware->deltas[0];
    }
#endif

#if gcdDUMP && !gcdDUMP_COMMAND && !gcdDUMP_IN_KERNEL
    if (dumpCommandBytes)
    {
        /* Dump current command buffer. */
        gcmDUMP_BUFFER(gcvNULL,
                       "command",
                       Hardware->context,
                       dumpCommandLogical,
                       0,
                       dumpCommandBytes);
    }

    if (dumpAsyncCommandBytes)
    {
        gcmASSERT(dumpAsyncCommandLogical);
        /* Dump async command buffer. */
        gcmDUMP_BUFFER(gcvNULL,
                       "async",
                       Hardware->context,
                       dumpAsyncCommandLogical,
                       0,
                       dumpAsyncCommandBytes);
    }
#endif

    /* Dump the commit. */
    gcmDUMP(gcvNULL, "@[commit]");

#if gcdSYNC
    if (!gcmIS_ERROR(status))
    {
        if (Hardware->fence)
        {
            Hardware->fence->commitID = Hardware->fence->fenceIDSend;
        }
    }
#endif

OnError:
    /* Return the status. */
    gcmFOOTER();
#endif  /* (gcdENABLE_3D || gcdENABLE_2D) */

    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_Stall
**
**  Stall the thread until the hardware is finished.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_Stall(
    IN gcoHARDWARE Hardware
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if (gcdENABLE_3D || gcdENABLE_2D)
    gcsHAL_INTERFACE iface;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Dump the stall. */
    gcmDUMP(gcvNULL, "@[stall]");

    /* Create a signal event. */
    iface.command            = gcvHAL_SIGNAL;
    iface.u.Signal.signal    = gcmPTR_TO_UINT64(Hardware->stallSignal);
    iface.u.Signal.auxSignal = 0;
    iface.u.Signal.process   = gcmPTR_TO_UINT64(gcoOS_GetCurrentProcessID());
    iface.u.Signal.fromWhere = Hardware->features[gcvFEATURE_BLT_ENGINE] ? gcvKERNEL_BLT : gcvKERNEL_PIXEL;

    /* Send the event. */
    gcmONERROR(gcoHARDWARE_CallEvent(Hardware, &iface));

    /* Commit the event queue. */
    gcmONERROR(gcoQUEUE_Commit(Hardware->engine[gcvENGINE_RENDER].queue, gcvFALSE));

    /* Wait for the signal. */
    gcmONERROR(gcoOS_WaitSignal(gcvNULL, Hardware->stallSignal, gcvINFINITE));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
#endif  /* (gcdENABLE_3D || gcdENABLE_2D) */
    return status;
}

#if gcdENABLE_3D
/*******************************************************************************
**
**  _ConvertResolveFormat
**
**  Converts HAL resolve format into its hardware equivalent.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to a gcoHARDWARE object.
**
**      gceSURF_FORMAT Format
**          HAL format value.
**
**  OUTPUT:
**
**      gctUINT32 * HardwareFormat
**          Hardware format value.
**
**      gctBOOL * Flip
**          RGB component flip flag.
**
**      OUT gceMSAA_DOWNSAMPLE_MODE *DownsampleMode
**          Multisample downsample mode
*/
static gceSTATUS
_ConvertResolveFormat(
    IN gcoHARDWARE Hardware,
    IN gceSURF_FORMAT SrcFormat,
    IN gceSURF_FORMAT DstFormat,
    OUT gctUINT32 * SrcHardwareFormat,
    OUT gctUINT32 * DstHardwareFormat,
    OUT gctBOOL * FlipRB,
    OUT gceMSAA_DOWNSAMPLE_MODE *DownsampleMode
    )

{
    gceSURF_FORMAT format[2] = {SrcFormat, DstFormat};
    gctUINT32 hwFormat[2];
    gctBOOL flipRB[2];
    gctINT i;
    gctBOOL fakeFormat = gcvFALSE;
    gceMSAA_DOWNSAMPLE_MODE downsampleMode = gcvMSAA_DOWNSAMPLE_AVERAGE;

    for (i = 0; i < 2; i++)
    {
        switch (format[i])
        {
        case gcvSURF_X4R4G4B4:
            hwFormat[i] = 0x0;
            flipRB[i]   = gcvFALSE;
            break;

        case gcvSURF_A4R4G4B4:
            hwFormat[i] = 0x1;
            flipRB[i]   = gcvFALSE;
            break;

        case gcvSURF_X1R5G5B5:
            hwFormat[i] = 0x2;
            flipRB[i]   = gcvFALSE;
            break;

        case gcvSURF_A1R5G5B5:
            hwFormat[i] = 0x3;
            flipRB[i]   = gcvFALSE;
            break;

        case gcvSURF_X1B5G5R5:
            hwFormat[i] = 0x2;
            flipRB[i]   = gcvTRUE;
            break;

        case gcvSURF_A1B5G5R5:
            hwFormat[i] = 0x3;
            flipRB[i]   = gcvTRUE;
            break;

        case gcvSURF_R5G6B5:
            hwFormat[i] = 0x4;
            flipRB[i]   = gcvFALSE;
            break;

        case gcvSURF_X8R8G8B8:
            hwFormat[i] = 0x5;
            flipRB[i]   = gcvFALSE;
            break;

        /* Fake format. */
        case gcvSURF_R8_1_X8R8G8B8:
        case gcvSURF_G8R8_1_X8R8G8B8:
            hwFormat[i] = 0x5;
            flipRB[i]   = gcvFALSE;
            fakeFormat  = gcvTRUE;
            break;

        case gcvSURF_A8R8G8B8:
            hwFormat[i] = 0x6;
            flipRB[i]   = gcvFALSE;
            break;

        case gcvSURF_X8B8G8R8:
            hwFormat[i] = 0x5;
            flipRB[i]   = gcvTRUE;
            break;

        case gcvSURF_A8B8G8R8:
            hwFormat[i] = 0x6;
            flipRB[i]   = gcvTRUE;
            break;

        case gcvSURF_YUY2:
            if (Hardware->features[gcvFEATURE_YUY2_AVERAGING])
            {
                hwFormat[i] = 0x7;
                flipRB[i]   = gcvFALSE;
                break;
            }
            return gcvSTATUS_INVALID_ARGUMENT;

        /* Fake 16-bit formats. */
        case gcvSURF_D16:
            if (Hardware->features[gcvFEATURE_RS_DS_DOWNSAMPLE_NATIVE_SUPPORT])
            {
                hwFormat[i] = 0x18;
                flipRB[i]   = gcvFALSE;
                break;
            }
            /* else fall through */
        /* Fake format. */
        case gcvSURF_UYVY:
        case gcvSURF_YVYU:
        case gcvSURF_VYUY:
            hwFormat[i] = 0x1;
            flipRB[i]   = gcvFALSE;
            fakeFormat  = gcvTRUE;
            break;

        /* Fake 32-bit formats. */
        case gcvSURF_D24S8:
        case gcvSURF_D24X8:
        case gcvSURF_X24S8:
        case gcvSURF_D32:
            if (Hardware->features[gcvFEATURE_RS_DS_DOWNSAMPLE_NATIVE_SUPPORT])
            {
                hwFormat[i] = 0x17;
                flipRB[i]   = gcvFALSE;
            }
            else
            {
                /* Fake format. */
                hwFormat[i] = 0x6;
                flipRB[i]   = gcvFALSE;
                fakeFormat  = gcvTRUE;
            }
            break;


        case gcvSURF_A2B10G10R10:

            /* RS has decoding error for this hwFormat[i], which swapped RB channel, But PE is correct.
            */
            hwFormat[i] = 0x16;
            flipRB[i]   = gcvTRUE;
            break;

        /*
        ** we still keep tile status buffer for 1-layer faked formats.
        ** only for disable tile status buffer.
        ** i.e.resolve to itself. other RS-blit will not work,
        ** which should go gcoSURF_CopyPixels path.
        */
        case gcvSURF_A2B10G10R10UI_1_A8R8G8B8:
        case gcvSURF_A8B8G8R8I_1_A8R8G8B8:
        case gcvSURF_A8B8G8R8UI_1_A8R8G8B8:
        case gcvSURF_G16R16I_1_A8R8G8B8:
        case gcvSURF_G16R16UI_1_A8R8G8B8:
        case gcvSURF_X32R32I_1_A8R8G8B8:
        case gcvSURF_X32R32UI_1_A8R8G8B8:
        case gcvSURF_B8G8R8I_1_A8R8G8B8:
        case gcvSURF_B8G8R8UI_1_A8R8G8B8:
        case gcvSURF_R32I_1_A8R8G8B8:
        case gcvSURF_R32UI_1_A8R8G8B8:
            hwFormat[i] = 0x6;
            flipRB[i]   = gcvFALSE;
            fakeFormat  = gcvTRUE;
            break;

        case gcvSURF_R8I_1_A4R4G4B4:
        case gcvSURF_R8UI_1_A4R4G4B4:
        case gcvSURF_R16I_1_A4R4G4B4:
        case gcvSURF_R16UI_1_A4R4G4B4:
        case gcvSURF_X8R8I_1_A4R4G4B4:
        case gcvSURF_X8R8UI_1_A4R4G4B4:
        case gcvSURF_G8R8I_1_A4R4G4B4:
        case gcvSURF_G8R8UI_1_A4R4G4B4:
        case gcvSURF_X16R16I_1_A4R4G4B4:
        case gcvSURF_X16R16UI_1_A4R4G4B4:
        case gcvSURF_X8G8R8I_1_A4R4G4B4:
        case gcvSURF_X8G8R8UI_1_A4R4G4B4:
            hwFormat[i] = 0x1;
            flipRB[i]   = gcvFALSE;
            fakeFormat  = gcvTRUE;
            break;

        case gcvSURF_S8:
            if (Hardware->features[gcvFEATURE_RS_DS_DOWNSAMPLE_NATIVE_SUPPORT])
            {
                hwFormat[i] = 0x10;
                flipRB[i]   = gcvFALSE;
                downsampleMode = gcvMSAA_DOWNSAMPLE_SAMPLE;
                break;
            }
            /* else fall through */

        default:
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        if (fakeFormat)
        {
            /* Do not need check more when src format is faked. */
            break;
        }
    }

    if (fakeFormat)
    {
        /* Fake format only works when same format. */
        if (SrcFormat == DstFormat)
        {
            hwFormat[1] = hwFormat[0];
            flipRB[1]   = flipRB[0];
        }
        else
        {
            return gcvSTATUS_NOT_SUPPORTED;
        }
    }

    if (SrcHardwareFormat != gcvNULL)
    {
        *SrcHardwareFormat = hwFormat[0];
    }

    if (DstHardwareFormat != gcvNULL)
    {
        *DstHardwareFormat = hwFormat[1];
    }

    if (FlipRB != gcvNULL)
    {
        *FlipRB = (flipRB[0] != flipRB[1]);
    }

    if (DownsampleMode != gcvNULL)
    {
        *DownsampleMode = downsampleMode;
    }

    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  _AlignResolveRect
**
**  Align the specified rectangle to the resolve block requirements.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcoSURF Surface
**          Pointer to surface object.
**
**      gcsPOINT_PTR RectOrigin
**          Unaligned origin of the rectangle.
**
**      gcsPOINT_PTR RectSize
**          Unaligned size of the rectangle.
**
**  OUTPUT:
**
**      gcsPOINT_PTR AlignedOrigin
**          Resolve-aligned origin of the rectangle.
**
**      gcsPOINT_PTR AlignedSize
**          Resolve-aligned size of the rectangle.
*/
static void _AlignResolveRect(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface,
    IN gcsPOINT_PTR RectOrigin,
    IN gcsPOINT_PTR RectSize,
    OUT gcsPOINT_PTR AlignedOrigin,
    OUT gcsPOINT_PTR AlignedSize
    )
{
    gctUINT originX = 0;
    gctUINT originY = 0;
    gctUINT sizeX   = 0;
    gctUINT sizeY   = 0;
    gctUINT32 maskX;
    gctUINT32 maskY;

    /* Determine region's right and bottom coordinates. */
    gctINT32 right  = RectOrigin->x + RectSize->x;
    gctINT32 bottom = RectOrigin->y + RectSize->y;

    /* Determine the outside or "external" coordinates aligned for resolve
       to completely cover the requested rectangle. */
    gcoHARDWARE_GetSurfaceResolveAlignment(Hardware, Surface, &originX, &originY, &sizeX, &sizeY);

    maskX = originX - 1;
    maskY = originY - 1;

    AlignedOrigin->x = RectOrigin->x & ~maskX;
    AlignedOrigin->y = RectOrigin->y & ~maskY;

    AlignedSize->x   = gcmALIGN(right  - AlignedOrigin->x, sizeX);
    AlignedSize->y   = gcmALIGN(bottom - AlignedOrigin->y, sizeY);
}

/*******************************************************************************
**
**  gcoHARDWARE_AlignResolveRect
**
**  Align the specified rectangle to the resolve block requirements.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to surface object.
**
**      gcsPOINT_PTR RectOrigin
**          Unaligned origin of the rectangle.
**
**      gcsPOINT_PTR RectSize
**          Unaligned size of the rectangle.
**
**  OUTPUT:
**
**      gcsPOINT_PTR AlignedOrigin
**          Resolve-aligned origin of the rectangle.
**
**      gcsPOINT_PTR AlignedSize
**          Resolve-aligned size of the rectangle.
*/
gceSTATUS
gcoHARDWARE_AlignResolveRect(
    IN gcoSURF Surface,
    IN gcsPOINT_PTR RectOrigin,
    IN gcsPOINT_PTR RectSize,
    OUT gcsPOINT_PTR AlignedOrigin,
    OUT gcsPOINT_PTR AlignedSize
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoHARDWARE Hardware = gcvNULL;

    gcmHEADER_ARG("SurfInfo=0x%x RectOrigin=0x%x RectSize=0x%x, AlignedOrigin=0x%x AlignedSize=0x%x",
                  Surface, RectOrigin, RectSize, AlignedOrigin, AlignedSize);

    gcmGETHARDWARE(Hardware);

    _AlignResolveRect(
            Hardware, Surface, RectOrigin, RectSize, AlignedOrigin, AlignedSize
            );

OnError:
    gcmFOOTER_NO();
    return status;
}

/*******************************************************************************
**
**  _ComputePixelLocation
**
**  Compute the offset of the specified pixel and determine its format.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT X, Y
**          Pixel coordinates.
**
**      gctUINT Stride
**          Surface stride.
**
**      gcsSURF_FORMAT_INFO_PTR * Format
**          Pointer to the pixel format (even/odd pair).
**
**      gctBOOL Tiled
**          Surface tiled vs. linear flag.
**
**      gctBOOL SuperTiled
**          Surface super tiled vs. normal tiled flag.
**
**  OUTPUT:
**
**      gctUINT32 PixelOffset
**          Offset of the pixel from the beginning of the surface.
**
**      gcsSURF_FORMAT_INFO_PTR * PixelFormat
**          Specific pixel format of this pixel.
*/
static void _ComputePixelLocation(
    IN gcoHARDWARE Hardware,
    IN gctUINT X,
    IN gctUINT Y,
    IN gctUINT Stride,
    IN gcsSURF_FORMAT_INFO_PTR FormatInfo,
    IN gctBOOL Tiled,
    IN gctBOOL SuperTiled,
    OUT gctUINT32_PTR PixelOffset,
    OUT gctUINT32_PTR OddPixel
    )
{
    gctUINT8 bitsPerPixel = FormatInfo->bitsPerPixel;

    if (FormatInfo->interleaved)
    {
        /* Determine whether the pixel is at even or odd location. */
        *OddPixel = X & 1;

        /* Force to the even location for interleaved pixels. */
        X &= ~1;
    }
    else
    {
        *OddPixel = 0;
    }

    if (Tiled)
    {
        *PixelOffset
            /* Skip full rows of 64x64 tiles to the top. */
            = (Stride
            * gcmTILE_OFFSET_Y(X, Y, SuperTiled))

            + ((bitsPerPixel
               * gcmTILE_OFFSET_X(X, Y, SuperTiled, Hardware->config->superTileMode))
               / 8);
    }
    else
    {
        *PixelOffset
            = Y * Stride
            + X * bitsPerPixel / 8;
    }
}

#if gcdENABLE_2D
/*******************************************************************************
**
**  _BitBlitCopy
**
**  Make a copy of the specified rectangular area using 2D hardware.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcoSURF SrcSurf
**          Pointer to source surface object.
**
**      gcoSURF DstSurf
**          Pointer to destination surface object.
**
**      gcsPOINT_PTR SrcOrigin
**          The origin of the source area to be copied.
**
**      gcsPOINT_PTR DstOrigin
**          The origin of the destination area to be copied.
**
**      gcsPOINT_PTR RectSize
**          The size of the rectangular area to be copied.
**
**  OUTPUT:
**
**      Nothing.
*/
static gceSTATUS
_BitBlitCopy(
    IN gcoHARDWARE Hardware,
    IN gcoSURF SrcSurf,
    IN gcoSURF DstSurf,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DstOrigin,
    IN gcsPOINT_PTR RectSize
    )
{
    gceSTATUS status;
    gctUINT32 format, swizzle, isYUVformat;
    gctINT destRight, destBottom;

    gctUINT32 reserveSize;
    gcoCMDBUF reserve;

    gcmHEADER_ARG("Hardware=0x%x SrcSurf=0x%x DstSurf=0x%x SrcOrigin=%d,%d "
                  "DstOrigin=%d,%d RectSize=%d,%d",
                  Hardware, SrcSurf, DstSurf, SrcOrigin->x, SrcOrigin->y,
                  DstOrigin->x, DstOrigin->y, RectSize->x, RectSize->y);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Convert source format. */
    gcmONERROR(gcoHARDWARE_TranslateSourceFormat(
        Hardware, SrcSurf->format, &format, &swizzle, &isYUVformat
        ));

    /* Convert destination format. */
    gcmONERROR(gcoHARDWARE_TranslateDestinationFormat(
        Hardware, DstSurf->format, gcvTRUE, &format, &swizzle, &isYUVformat
        ));

    /* Verify that the surfaces are locked. */
    gcmVERIFY_LOCK(SrcSurf);
    gcmVERIFY_LOCK(DstSurf);

    Hardware->enableXRGB = gcvTRUE;

    /* Determine the size of the buffer to reserve. */
    reserveSize
        /* Switch to 2D pipes. */
        = gcmALIGN(8 * gcmSIZEOF(gctUINT32), 8)

        /* Source setup. */
        + gcmALIGN((1 + 6) * gcmSIZEOF(gctUINT32), 8)

        /* Destination setup. */
        + gcmALIGN((1 + 4) * gcmSIZEOF(gctUINT32), 8)

        /* Clipping window setup. */
        + gcmALIGN((1 + 2) * gcmSIZEOF(gctUINT32), 8)

        /* ROP setup. */
        + gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32), 8)

        /* Rotation setup. */
        + gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32), 8)

        /* Blit commands. */
        + 4 * gcmSIZEOF(gctUINT32)

        /* Flush. */
        + gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32), 8)

#if gcdENABLE_3D
        /* Switch to 3D pipes. */
        + gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32), 8)
#endif
        ;

    Hardware->hw2DCmdBuffer = gcvNULL;
    Hardware->hw2DCmdIndex = 0;
    Hardware->hw2DCmdSize = reserveSize >> 2;

    gcmONERROR(gcoBUFFER_Reserve(
        Hardware->engine[gcvENGINE_RENDER].buffer,
        reserveSize,
        gcvTRUE,
        gcvCOMMAND_2D,
        &reserve
        ));

    Hardware->hw2DCmdBuffer = gcmUINT64_TO_PTR(reserve->lastReserve);

    reserve->using2D = gcvTRUE;

    /* Flush the 3D pipe. */
    gcmONERROR(gcoHARDWARE_Load2DState32(
        Hardware,
        0x0380C,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:1) - (0 ?
 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
        ));

    gcmONERROR(gcoHARDWARE_Load2DState32(
        Hardware,
        0x03808,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ?
 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ?
 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
        ));

    Hardware->hw2DCmdBuffer[Hardware->hw2DCmdIndex++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));
    Hardware->hw2DCmdBuffer[Hardware->hw2DCmdIndex++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)));

    gcmONERROR(gcoHARDWARE_Load2DState32(
        Hardware,
        0x03800,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (gcvPIPE_2D) & ((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0)))
        ));
    {
        gctUINT32 data[6];

        /***************************************************************************
        ** Setup source.
        */
        gcmGETHARDWAREADDRESS(SrcSurf->node, data[0]);

        data[1] = SrcSurf->stride;

        data[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));

        data[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:24) - (0 ? 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ?
 28:24))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 28:24) - (0 ?
 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ?
 28:24)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));

        data[4] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (SrcOrigin->x) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (SrcOrigin->y) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16)));

        data[5] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (RectSize->x) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (RectSize->y) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16)));

        gcmONERROR(gcoHARDWARE_Load2DState(
            Hardware,
            0x01200,
            6,
            data
            ));

        /***************************************************************************
        ** Setup destination.
        */
        gcmGETHARDWAREADDRESS(DstSurf->node, data[0]);

        data[1] = DstSurf->stride;

        data[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));

        data[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 4:0) - (0 ?
 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));

        gcmONERROR(gcoHARDWARE_Load2DState(
            Hardware,
            0x01228,
            4,
            data
            ));

        /***************************************************************************
        ** Setup clipping window.
        */
        destRight  = DstOrigin->x + RectSize->x;
        destBottom = DstOrigin->y + RectSize->y;

        data[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
 14:0))) | (((gctUINT32) ((gctUINT32) (DstOrigin->x) & ((gctUINT32) ((((1 ?
 14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
 14:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:16) - (0 ? 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
 30:16))) | (((gctUINT32) ((gctUINT32) (DstOrigin->y) & ((gctUINT32) ((((1 ?
 30:16) - (0 ? 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
 30:16)));

        data[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
 14:0))) | (((gctUINT32) ((gctUINT32) (destRight) & ((gctUINT32) ((((1 ?
 14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
 14:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:16) - (0 ? 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
 30:16))) | (((gctUINT32) ((gctUINT32) (destBottom) & ((gctUINT32) ((((1 ?
 30:16) - (0 ? 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
 30:16)));

        gcmONERROR(gcoHARDWARE_Load2DState(
            Hardware,
            0x01260,
            2,
            data
            ));

        /***************************************************************************
        ** Blit the data.
        */

        /* Set ROP. */
        gcmONERROR(gcoHARDWARE_Load2DState32(
            Hardware,
            0x0125C,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (0xCC) & ((gctUINT32) ((((1 ? 7:0) - (0 ?
 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0)))
            ));

        /* Set Rotation.*/
        if (Hardware->features[gcvFEATURE_2D_MIRROR_EXTENSION])
        {
            gcmONERROR(gcoHARDWARE_Load2DState32(
                Hardware,
                0x012BC,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ? 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 13:12) - (0 ? 13:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 17:16) - (0 ? 17:16) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16)))
                ));
        }
        else
        {
            gcmONERROR(gcoHARDWARE_Load2DState32(
                Hardware,
                0x0126C,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                ));
        }

        /* START_DE. */
        Hardware->hw2DCmdBuffer[Hardware->hw2DCmdIndex++] =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ?
 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x04 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 15:8) - (0 ?
 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:16) - (0 ? 26:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:16) - (0 ? 26:16) + 1))))))) << (0 ?
 26:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 26:16) - (0 ?
 26:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:16) - (0 ? 26:16) + 1))))))) << (0 ?
 26:16)));
        Hardware->hw2DCmdIndex++;

        /* DestRectangle. */
        Hardware->hw2DCmdBuffer[Hardware->hw2DCmdIndex++] =
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (DstOrigin->x) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (DstOrigin->y) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16)));

        Hardware->hw2DCmdBuffer[Hardware->hw2DCmdIndex++] =
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (destRight) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (destBottom) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16)));
    }

    gcmDUMP(gcvNULL, "@[prim2d 1 0x00000000");
    gcmDUMP(gcvNULL,
            "  %d,%d %d,%d",
            DstOrigin->x, DstOrigin->y, destRight, destBottom);
    gcmDUMP(gcvNULL, "] -- prim2d");

    /* Flush the 2D cache. */
    gcmONERROR(gcoHARDWARE_Load2DState32(
        Hardware,
        0x0380C,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:3) - (0 ?
 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))));

#if gcdENABLE_3D
    /* Select the 3D pipe. */
    gcmONERROR(gcoHARDWARE_Load2DState32(
        Hardware,
        0x03800,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (gcvPIPE_3D) & ((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0)))
        ));
#endif

    gcmASSERT(Hardware->hw2DCmdSize == Hardware->hw2DCmdIndex);


    /* Commit the command buffer. */
    gcmONERROR(gcoHARDWARE_Commit(Hardware));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    /* Return the status. */
    return status;
}
#endif

/*******************************************************************************
**
**  _SoftwareCopy
**
**  Make a copy of the specified rectangular area using CPU.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsSURF_VIEW *SrcView,
**          Pointer to the source surface view.
**
**      gcsSURF_VIEW *DstView
**          Pointer to the destination surface view.
**
**      gcsSURF_RESOLVE_ARGS *Args
**          Pointer to the resolve arguments.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS _SoftwareCopy(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_VIEW *SrcView,
    IN gcsSURF_VIEW *DstView,
    IN gcsSURF_RESOLVE_ARGS *Args
    )
{
    gctINT x, y;
    gctBOOL srcTiled, dstTiled;
    gctUINT8_PTR pSrcBase, pDstBase;
    gcoSURF srcSurf = SrcView->surf;
    gcoSURF dstSurf = DstView->surf;
    gcsPOINT_PTR srcOrigin = &Args->uArgs.v2.srcOrigin;
    gcsPOINT_PTR dstOrigin = &Args->uArgs.v2.dstOrigin;
    gcsPOINT_PTR rectSize = &Args->uArgs.v2.rectSize;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x SrcView=0x%x DstView=0x%x Args=0x%x", Hardware, SrcView, DstView, Args);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmDEBUG_VERIFY_ARGUMENT(rectSize->x > 0);
    gcmDEBUG_VERIFY_ARGUMENT(rectSize->y > 0);

    if (Args->version != gcvHAL_ARG_VERSION_V2)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Verify that the surfaces are locked. */
    gcmVERIFY_LOCK(srcSurf);
    gcmVERIFY_LOCK(dstSurf);

    /* Flush and stall the pipe. */
    gcmONERROR(gcoHARDWARE_FlushPipe(Hardware, gcvNULL));
    gcmONERROR(gcoHARDWARE_Commit(Hardware));
    gcmONERROR(gcoHARDWARE_Stall(Hardware));

    /* Invalidate surface cache before CPU memcopy */
    gcmONERROR(gcoSURF_NODE_Cache(&srcSurf->node, srcSurf->node.logical, srcSurf->size, gcvCACHE_INVALIDATE));
    gcmONERROR(gcoSURF_NODE_Cache(&dstSurf->node, dstSurf->node.logical, dstSurf->size, gcvCACHE_INVALIDATE));

    /* Cannot support mult-layer format */
    gcmASSERT(srcSurf->formatInfo.layers == 1 && dstSurf->formatInfo.layers == 1);

    pSrcBase = srcSurf->node.logical + SrcView->firstSlice * srcSurf->sliceSize;
    pDstBase = dstSurf->node.logical + DstView->firstSlice * dstSurf->sliceSize;

    /* Determine whether the destination is tiled. */
    srcTiled = (srcSurf->type != gcvSURF_BITMAP);
    dstTiled = (dstSurf->type != gcvSURF_BITMAP);

   /* Test for fast copy. */
   if (srcTiled && dstTiled
   &&  (srcSurf->tiling == dstSurf->tiling)
   &&  (srcSurf->format == dstSurf->format)
   &&  (srcOrigin->x == 0)
   &&  (srcOrigin->y == 0)
   &&  (rectSize->x == (gctINT)dstSurf->alignedW)
   &&  (rectSize->y == (gctINT)dstSurf->alignedH)
   )
   {
       gctUINT32 srcOffset = 0;
       gctUINT32 dstOffset = 0;

       for (y = 0; y < rectSize->y; y += 4)
       {
           gcoOS_MemCopy(pDstBase + dstOffset, pSrcBase + srcOffset, dstSurf->stride * 4);

           srcOffset += srcSurf->stride * 4;
           dstOffset += dstSurf->stride * 4;
       }
   }
   else if (!srcTiled && !dstTiled
    && srcSurf->format == dstSurf->format
    && srcSurf->stride == dstSurf->stride
    && srcOrigin->x == 0 && srcOrigin->y == 0
    && dstOrigin->x == 0 && dstOrigin->y == 0
   )
   {
       /* For same logical, it is not necessary to do the next */
       if (pDstBase != pSrcBase)
       {
           gcoOS_MemCopy(pDstBase, pSrcBase, dstSurf->stride * rectSize->y);
       }
   }
   else if (!srcTiled && !dstTiled
    && srcSurf->format == dstSurf->format
    && srcOrigin->x == 0 && srcOrigin->y == 0
    && dstOrigin->x == 0 && dstOrigin->y == 0
   )
   {
       gctUINT32 srcOffset = 0;
       gctUINT32 dstOffset = 0;

       for (y = 0; y < rectSize->y; y++)
       {
           gcoOS_MemCopy(pDstBase + dstOffset,
                         pSrcBase + srcOffset,
                         (dstSurf->stride > srcSurf->stride ? srcSurf->stride : dstSurf->stride));
           srcOffset += srcSurf->stride;
           dstOffset += dstSurf->stride;
       }
   }
   else
   {
       gctUINT32 srcOffset, dstOffset, srcOdd, dstOdd;
       gcsSURF_FORMAT_INFO_PTR srcFormatInfo = &srcSurf->formatInfo;
       gcsSURF_FORMAT_INFO_PTR dstFormatInfo = &dstSurf->formatInfo;

       for (y = 0; y < rectSize->y; ++y)
       {
           for (x = 0; x < rectSize->x; ++x)
           {
               _ComputePixelLocation(
                   Hardware, srcOrigin->x + x, srcOrigin->y + y, srcSurf->stride,
                   srcFormatInfo, srcTiled, srcSurf->superTiled,
                   &srcOffset, &srcOdd
                   );

               _ComputePixelLocation(
                   Hardware, dstOrigin->x + x, dstOrigin->y + y, dstSurf->stride,
                   dstFormatInfo, dstTiled, dstSurf->superTiled,
                   &dstOffset, &dstOdd
                   );

               gcmONERROR(gcoHARDWARE_ConvertPixel(
                   pSrcBase + srcOffset,
                   pDstBase + dstOffset,
                   0, 0,
                   srcFormatInfo,
                   dstFormatInfo,
                   gcvNULL,
                   gcvNULL,
                   srcOdd,
                   dstOdd
                   ));
           }
       }
   }

    gcmONERROR(gcoSURF_NODE_Cache(&dstSurf->node, dstSurf->node.logical, dstSurf->size, gcvCACHE_CLEAN));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gceSTATUS _SourceCopy(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_VIEW *SrcView,
    IN gcsSURF_VIEW *DstView,
    IN gcsSURF_RESOLVE_ARGS *Args
    )
{
    gcoSURF srcSurf = SrcView->surf;
    gcoSURF dstSurf = DstView->surf;
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

    gcmHEADER_ARG("Hardware=0x%x SrcView=0x%x DstView=0x%x Args=0x%x", Hardware, SrcView, DstView, Args);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Args->version != gcvHAL_ARG_VERSION_V2)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    do
    {
        /* Only support BITMAP to BITMAP or TEXTURE to TEXTURE copy for now. */
        if (!(((srcSurf->type == gcvSURF_BITMAP)  && (dstSurf->type == gcvSURF_BITMAP))
        ||    ((srcSurf->type == gcvSURF_TEXTURE) && (dstSurf->type == gcvSURF_TEXTURE)))
        )
        {
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Is 2D pipe present? */
        if (Hardware->hw2DEngine && !Hardware->sw2DEngine
        /* GC500 needs properly aligned surfaces. */
        &&  ((Hardware->config->chipModel != gcv500) || ((dstSurf->allocedH & 7) == 0))
        )
        {
#if gcdENABLE_2D
            status = _BitBlitCopy(
                Hardware,
                srcSurf,
                dstSurf,
                &Args->uArgs.v2.srcOrigin,
                &Args->uArgs.v2.dstOrigin,
                &Args->uArgs.v2.rectSize
                );
#else
            status = gcvSTATUS_NOT_SUPPORTED;
#endif
        }

        if (status != gcvSTATUS_OK)
        {
            status = _SoftwareCopy(Hardware, SrcView, DstView, Args);
        }
    }
    while (gcvFALSE);

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  _Tile420Surface
**
**  Tile linear 4:2:0 source surface.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcoSURF SrcSurf
**          Pointer to the source surface object.
**
**      gcoSURF DstSurf
**          Pointer to the destination surface object.
**
**      gcsPOINT_PTR SrcOrigin
**          The origin of the source area to be copied.
**
**      gcsPOINT_PTR DstOrigin
**          The origin of the destination area to be copied.
**
**      gcsPOINT_PTR RectSize
**          The size of the rectangular area to be copied.
**
**  OUTPUT:
**
**      Nothing.
*/
static gceSTATUS _Tile420Surface(
    IN gcoHARDWARE Hardware,
    IN gcoSURF SrcSurf,
    IN gcoSURF DstSurf,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DstOrigin,
    IN gcsPOINT_PTR RectSize
    )
{
    gceSTATUS status;
    gctUINT32 srcFormat;
    gctBOOL tilerAvailable;
    gctUINT32 uvSwizzle;
    gctUINT32 srcYAddress;
    gctUINT32 srcUAddress;
    gctUINT32 srcVAddress;
    gctUINT32 destAddress;
    gctPOINTER *cmdBuffer = gcvNULL;
    gctINT srcTileMode = 0x0;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x SrcSurf=0x%x DstSurf=0x%x SrcOrigin=%d,%d "
                  "DstOrigin=%d,%d RectSize=%d,%d",
                  Hardware, SrcSurf, DstSurf, SrcOrigin->x, SrcOrigin->y,
                  DstOrigin->x, DstOrigin->y, RectSize->x, RectSize->y);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Verify that the surfaces are locked. */
    gcmVERIFY_LOCK(SrcSurf);
    gcmVERIFY_LOCK(DstSurf);

    /* Determine hardware support for 4:2:0 tiler. */
    tilerAvailable = Hardware->features[gcvFEATURE_YUV420_TILER];

    /* Available? */
    if (!tilerAvailable)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Input limitations until more support is required. */
    if ((SrcOrigin->x != 0) || (SrcOrigin->y != 0) ||
        (DstOrigin->x != 0) || (DstOrigin->y != 0)
        )

    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }
    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, cmdBuffer);

#if gcdENABLE_TRUST_APPLICATION
    if (Hardware->features[gcvFEATURE_SECURITY])
    {
        gcoHARDWARE_SetProtectMode(
            Hardware,
            ((SrcSurf->hints & gcvSURF_PROTECTED_CONTENT) ||
             (DstSurf->hints & gcvSURF_PROTECTED_CONTENT)),
            (gctPOINTER *)&memory);

        Hardware->GPUProtecedModeDirty = gcvTRUE;
    }
#endif

    /* Append FLUSH to the command buffer. */
    gcmONERROR(gcoHARDWARE_FlushPipe(Hardware, (gctPOINTER *)&memory));

    /* Determine the format. */
    if ((SrcSurf->format == gcvSURF_YV12) ||
        (SrcSurf->format == gcvSURF_I420))
    {
        /* No need to swizzle as we set the U and V addresses correctly. */
        uvSwizzle = 0x0;
        srcFormat = 0x0;
    }
    else if (SrcSurf->format == gcvSURF_NV12)
    {
        uvSwizzle = 0x0;
        srcFormat = 0x1;
    }
    else
    {
        uvSwizzle = 0x1;
        srcFormat = 0x1;
    }


    /***************************************************************************
    ** Set tiler configuration.
    */

    if (Hardware->config->gpuCoreCount > 1)
    {
        /* Sync between GPUs */
        gcoHARDWARE_MultiGPUSync(Hardware, &memory);
        /* Select GPU 3D_0. */
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0);
 memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0));
 } };

    }

    gcmGETHARDWAREADDRESS(SrcSurf->node, srcYAddress);

    if (SrcSurf->flags & gcvSURF_FLAG_MULTI_NODE)
    {
        gctUINT32 srcAddress2 = ~0U;
        gctUINT32 srcAddress3 = ~0U;

        if (SrcSurf->node2.pool != gcvPOOL_UNKNOWN)
        {
            gcmGETHARDWAREADDRESS(SrcSurf->node2, srcAddress2);
        }

        if (SrcSurf->node3.pool != gcvPOOL_UNKNOWN)
        {
            gcmGETHARDWAREADDRESS(SrcSurf->node3, srcAddress3);
        }

        /* Y,V,U order for YV12, otherwise Y,U,V. */
        if (SrcSurf->format == gcvSURF_YV12)
        {
            srcUAddress = srcAddress3;
            srcVAddress = srcAddress2;
        }
        else
        {
            srcUAddress = srcAddress2;
            srcVAddress = srcAddress3;
        }
    }
    else
    {
        /* The surface must be locked for these addresses. */
        srcUAddress = srcYAddress + SrcSurf->uOffset;
        srcVAddress = srcYAddress + SrcSurf->vOffset;
    }

    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)10  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (10 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x059E) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


        /* Set tiler configuration. */
        gcmSETCTRLSTATE_NEW(
            stateDelta, reserve, memory, 0x059E,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (srcFormat) & ((gctUINT32) ((((1 ?
 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (uvSwizzle) & ((gctUINT32) ((((1 ?
 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8)))
            );

        /* Set window size. */
        gcmSETCTRLSTATE_NEW(
            stateDelta, reserve, memory, 0x059F,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (RectSize->x) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (RectSize->y) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16)))
            );

        /* Set Y plane. */
        gcmSETCTRLSTATE_NEW(
            stateDelta, reserve, memory, 0x05A0,
            srcYAddress
            );

        gcmSETCTRLSTATE_NEW(
            stateDelta, reserve, memory, 0x05A1,
            SrcSurf->stride
            );

        /* Set U plane. */
        gcmSETCTRLSTATE_NEW(
            stateDelta, reserve, memory, 0x05A2,
            srcUAddress
            );

        gcmSETCTRLSTATE_NEW(
            stateDelta, reserve, memory, 0x05A3,
            SrcSurf->uStride
            );

        /* Set V plane. */
        gcmSETCTRLSTATE_NEW(
            stateDelta, reserve, memory, 0x05A4,
            srcVAddress
            );

        gcmSETCTRLSTATE_NEW(
            stateDelta, reserve, memory, 0x05A5,
            SrcSurf->vStride
            );

        gcmGETHARDWAREADDRESS(DstSurf->node, destAddress);

        /* Set destination. */
        gcmSETCTRLSTATE_NEW(
            stateDelta, reserve, memory, 0x05A6,
            destAddress
            );

        gcmSETCTRLSTATE_NEW(
            stateDelta, reserve, memory, 0x05A7,
            DstSurf->stride
            );

        gcmSETFILLER_NEW(
            reserve, memory
            );

    gcmENDSTATEBATCH_NEW(
        reserve, memory
        );

    if (Hardware->robust)
    {
        gctUINT32 YendAddress, UendAddress, VendAddress, dstEndAddress;
        gctUINT32 YbufSize, dstBufSize;

        gcmSAFECASTSIZET(YbufSize, SrcSurf->node.size);
        YendAddress = srcYAddress + YbufSize - 1;

        if (SrcSurf->flags & gcvSURF_FLAG_MULTI_NODE)
        {
            gctUINT32 srcAddress2 = ~0U;
            gctUINT32 srcAddress3 = ~0U;
            gctUINT32 bufSize2 = 0;
            gctUINT32 bufSize3 = 0;

            if (SrcSurf->node2.pool != gcvPOOL_UNKNOWN)
            {
                gcmGETHARDWAREADDRESS(SrcSurf->node2, srcAddress2);
                gcmSAFECASTSIZET(bufSize2, SrcSurf->node2.size);
            }

            if (SrcSurf->node3.pool != gcvPOOL_UNKNOWN)
            {
                gcmGETHARDWAREADDRESS(SrcSurf->node3, srcAddress3);
                gcmSAFECASTSIZET(bufSize3, SrcSurf->node3.size);
            }

            /* Y,V,U order for YV12, otherwise Y,U,V. */
            if (SrcSurf->format == gcvSURF_YV12)
            {
                UendAddress = srcAddress3 + bufSize3 - 1;
                VendAddress = srcAddress2 + bufSize2 - 1;
            }
            else
            {
                UendAddress = srcAddress2 + bufSize2 - 1;
                VendAddress = srcAddress3 + bufSize3 - 1;
            }
        }
        else
        {
            UendAddress = VendAddress = YendAddress;
        }
        gcmSAFECASTSIZET(dstBufSize, DstSurf->node.size);
        dstEndAddress = destAddress + dstBufSize - 1;

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x069A) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x069A, YendAddress );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x069B) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x069B, UendAddress );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x06B8) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x06B8, VendAddress );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x06B9) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x06B9, dstEndAddress );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }


    /***************************************************************************
    ** Disable clear.
    */

    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x058F) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x058F, 0 );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    if (SrcSurf->tiling == gcvSUPERTILED)
    {
        srcTileMode = 0x1;
    }
    else if (SrcSurf->tiling == gcvYMAJOR_SUPERTILED)
    {
        srcTileMode = 0x2;
    }

    /***************************************************************************
    ** Disable 256B cache setting
    */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0583) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0583, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 29:29) - (0 ? 29:29) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ? 28:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ?
 28:27))) | (((gctUINT32) ((gctUINT32) (srcTileMode) & ((gctUINT32) ((((1 ?
 28:27) - (0 ? 28:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ?
 28:27))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    /***************************************************************************
    ** Trigger resolve.
    */

    gcmASSERT(!Hardware->features[gcvFEATURE_BLT_ENGINE]);

    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0580) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0580, 0xBADABEEB );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};



    /***************************************************************************
    ** Disable tiler.
    */

    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x059E) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x059E, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    if (Hardware->config->gpuCoreCount > 1)
    {
        /* Enable all 3D GPUs. */
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK;
 memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };

        /* Sync between GPUs */
        gcoHARDWARE_MultiGPUSync(Hardware, &memory);
    }

    stateDelta = stateDelta;

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, cmdBuffer);

    /* Commit the command buffer. */
    gcmONERROR(gcoHARDWARE_Commit(Hardware));

    /* Return the status. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

typedef struct
{
    gctUINT32 mode;
    gctUINT32 horFactor;
    gctUINT32 verFactor;
}
gcsSUPERENTRY, *gcsSUPERENTRY_PTR;

typedef struct
{
    /* Source information. */
    gcsSURF_FORMAT_INFO_PTR srcFormatInfo;
    gceTILING srcTiling;
    gctBOOL srcMultiPipe;

    /* Destination information. */
    gcsSURF_FORMAT_INFO_PTR dstFormatInfo;
    gceTILING dstTiling;
    gctBOOL dstMultiPipe;

    /* Resolve information. */
    gctBOOL flipY;
    gcsSUPERENTRY_PTR superSampling;
}
gcsRESOLVE_VARS,
* gcsRESOLVE_VARS_PTR;

static gceSTATUS
_StripeResolve(
    IN gcoHARDWARE Hardware,
    IN gcoSURF SrcSurf,
    IN gcoSURF DstSurf,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DstOrigin,
    IN gcsPOINT_PTR RectSize,
    IN gcsRESOLVE_VARS_PTR Vars,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status;
    gcsRECT srcRect, dstRect;
    gctINT32 x, xStep, y, yStep;
    gctINT32 width, height;
    gctUINT32 srcOffset, dstOffset;
    gctINT hShift, vShift;
    gctINT32 dstY;
    gctUINT  loopCountX, loopCountY;
    gctUINT32 srcAddress = 0, dstAddress = 0;
    gctUINT32 windowSize = 0;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    /* Copy super-sampling factor. */
    hShift = (Vars->superSampling->horFactor == 1) ? 0 : 1;
    vShift = (Vars->superSampling->verFactor == 1) ? 0 : 1;

    /* Compute source bounding box. */
    srcRect.left   = SrcOrigin->x & ~15;
    srcRect.right  = gcmALIGN(SrcOrigin->x + (RectSize->x << hShift), 16);
    srcRect.top    = SrcOrigin->y & ~3;
    srcRect.bottom = gcmALIGN(SrcOrigin->y + (RectSize->y << vShift), 4);

    /* Compute destination bounding box. */
    dstRect.left   = DstOrigin->x & ~15;
    dstRect.right  = gcmALIGN(DstOrigin->x + RectSize->x, 16);
    dstRect.top    = DstOrigin->y & ~3;
    dstRect.bottom = gcmALIGN(DstOrigin->y + RectSize->y, 4);

    /* no split buffer fall into this function */
    gcmASSERT((SrcSurf->tiling & gcvTILING_SPLIT_BUFFER) == 0);
    gcmASSERT((DstSurf->tiling & gcvTILING_SPLIT_BUFFER) == 0);

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    /* Walk all stripes horizontally. */
    for (x = srcRect.left, loopCountX = 0; (x < srcRect.right) && (loopCountX < MAX_LOOP_COUNT); x += xStep, loopCountX++)
    {
        /* Compute horizontal step. */
        xStep = (x & 63) ? (x & 31) ? 16 : 32 : 64;
        gcmASSERT((x & ~63) == ((x + xStep - 1) & ~63));
        yStep = 16 * Hardware->needStriping / xStep;

        /* Compute width. */
        width = gcmMIN(srcRect.right - x, xStep);

        /* Walk the stripe vertically. */
        for (y = srcRect.top, loopCountY = 0; (y < srcRect.bottom) && (loopCountY < MAX_LOOP_COUNT); y += yStep, loopCountY++)
        {
            /* Compute vertical step. */
            yStep = 16 * Hardware->needStriping / xStep;
            if ((y & ~63) != ((y + yStep - 1) & ~63))
            {
                /* Don't overflow a super tile. */
                yStep = gcmALIGN(y, 64) - y;
            }

            /* Compute height. */
            height = gcmMIN(srcRect.bottom - y, yStep);

            /* Compute destination y. */
            dstY = Vars->flipY ? (dstRect.bottom - (y >> vShift) - height)
                               : (y >> vShift);

            /* Compute offsets. */
            gcmONERROR(
                gcoHARDWARE_ComputeOffset(Hardware,
                                          x, y,
                                          SrcSurf->stride,
                                          Vars->srcFormatInfo->bitsPerPixel / 8,
                                          Vars->srcTiling, &srcOffset));
            gcmONERROR(
                gcoHARDWARE_ComputeOffset(Hardware,
                                          x >> hShift, dstY,
                                          DstSurf->stride,
                                          Vars->dstFormatInfo->bitsPerPixel / 8,
                                          Vars->dstTiling, &dstOffset));

            /* Determine state values. */
            gcmGETHARDWAREADDRESS(SrcSurf->node, srcAddress);
            srcAddress += srcOffset;

            gcmGETHARDWAREADDRESS(DstSurf->node, dstAddress);
            dstAddress += dstOffset;

            windowSize = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (width) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)))
                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (height) & ((gctUINT32) ((((1 ? 31:16) - (0 ?
 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16)));

            /* Resolve one part of the stripe. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0582) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0582, srcAddress );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0584) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0584, dstAddress );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0588) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0588, windowSize );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            gcmASSERT(!Hardware->features[gcvFEATURE_BLT_ENGINE]);

            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0580) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0580, 0xBADABEEB );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }
    }

    stateDelta = stateDelta; /* keep compiler happy */

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    /* Return the error. */
    return status;
}

/*******************************************************************************
**
**  _ResolveRect
**
**  Perform a resolve on a surface.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsSURF_VIEW *SrcView,
**          Pointer to the source surface view.
**
**      gcsSURF_VIEW *DstView
**          Pointer to the destination surface view.
**
**      gcsSURF_RESOLVE_ARGS *Args
**          Pointer to the resolve arguments.
**
**  OUTPUT:
**
**      Nothing.
*/
static gceSTATUS _ResolveRect(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_VIEW *SrcView,
    IN gcsSURF_VIEW *DstView,
    IN gcsSURF_RESOLVE_ARGS *Args
    )
{
#   define _AA(HorFactor, VerFactor) \
    { \
        AQ_RS_CONFIG_RS_SRC_SUPER_SAMPLE_ENABLE \
            ## HorFactor ## X ## VerFactor, \
        HorFactor, \
        VerFactor \
    }

#   define _NOAA \
    { \
        0x0, \
        1, \
        1 \
    }

#   define _INVALIDAA \
    { \
        ~0U, \
        ~0U, \
        ~0U \
    }

    static gcsSUPERENTRY superSamplingTable[16] =
    {
        /*  SOURCE 1x1                 SOURCE 2x1 */

            /* DEST 1x1  DEST 2x1      DEST 1x1  DEST 2x1 */
            _NOAA, _INVALIDAA, { 0x1, 2, 1}, _NOAA,

            /* DEST 1x2  DEST 2x2      DEST 1x2  DEST 2x2 */
            _INVALIDAA, _INVALIDAA, _INVALIDAA, _INVALIDAA,

        /*  SOURCE 1x2                 SOURCE 2x2 */

            /* DEST 1x1  DEST 2x1      DEST 1x1  DEST 2x1 */
            { 0x2, 1, 2}, _INVALIDAA, { 0x3, 2, 2}, { 0x2, 1, 2},

            /* DEST 1x2  DEST 2x2      DEST 1x2  DEST 2x2 */
            _NOAA, _INVALIDAA, { 0x1, 2, 1}, _NOAA,

    };

    gceSTATUS status = gcvSTATUS_OK;
    gcsRESOLVE_VARS vars;
    gcsPOINT srcSize;
    gctUINT32 config;
    gctUINT32 srcAddress, dstAddress;
    gctUINT32 srcAddress2 = 0, dstAddress2 = 0;
    gctUINT32 srcStride, dstStride;
    gctUINT32 srcFormat, dstFormat;
    gctSIZE_T srcX, srcY, dstX, dstY;
    gctPOINTER srcMem[gcdMAX_SURF_LAYERS], dstMem[gcdMAX_SURF_LAYERS];
    gctUINT superSamplingIndex;
    gctBOOL flipRB;
    gctUINT32 endian;
    gctBOOL needStriping;
    gctUINT32 *pDitherTable;
    gctBOOL gammarCorrection = gcvFALSE;
    gctBOOL multiPipe;
    gceMSAA_DOWNSAMPLE_MODE srcDownsampleMode;
    gctPOINTER  *cmdBuffer = gcvNULL;

    gcoSURF srcSurf = SrcView->surf;
    gcoSURF dstSurf = DstView->surf;
    gcsPOINT_PTR srcOrigin = &Args->uArgs.v2.srcOrigin;
    gcsPOINT_PTR dstOrigin = &Args->uArgs.v2.dstOrigin;
    gcsPOINT_PTR rectSize = &Args->uArgs.v2.rectSize;
    gctBOOL yInverted = Args->uArgs.v2.yInverted;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x SrcView=0x%x DstView=0x%x Args=0x%x", Hardware, SrcView, DstView, Args);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT((srcOrigin->x & 3) == 0);
    gcmDEBUG_VERIFY_ARGUMENT((srcOrigin->y & 3) == 0);

    gcmDEBUG_VERIFY_ARGUMENT((dstOrigin->x & 3) == 0);
    gcmDEBUG_VERIFY_ARGUMENT((dstOrigin->y & 3) == 0);

    gcmDEBUG_VERIFY_ARGUMENT((rectSize->x & 15) == 0);
    gcmDEBUG_VERIFY_ARGUMENT((rectSize->y &  3) == 0);

    /* Verify that the surfaces are locked. */
    gcmVERIFY_LOCK(srcSurf);
    gcmVERIFY_LOCK(dstSurf);

    gcmASSERT(!Hardware->features[gcvFEATURE_BLT_ENGINE]);

    if (Args->version != gcvHAL_ARG_VERSION_V2)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if ((srcSurf->format == gcvSURF_D16 || srcSurf->format == gcvSURF_D24X8) &&
        (srcSurf->format != dstSurf->format)
       )
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

	if(srcSurf->colorSpace == gcvSURF_COLOR_SPACE_NONLINEAR || dstSurf->colorSpace == gcvSURF_COLOR_SPACE_NONLINEAR)
	{
		gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
	}

    /* Convert source and destination formats. */
    gcmONERROR(
        _ConvertResolveFormat(Hardware,
                              srcSurf->format,
                              dstSurf->format,
                              &srcFormat,
                              &dstFormat,
                              &flipRB,
                              &srcDownsampleMode));

    if (srcSurf->formatInfo.format == gcvSURF_UNKNOWN)
    {
        gcmONERROR(gcoSURF_QueryFormat(srcSurf->format, &vars.srcFormatInfo));
    }
    else
    {
        vars.srcFormatInfo = &srcSurf->formatInfo;
    }

    if (dstSurf->formatInfo.format == gcvSURF_UNKNOWN)
    {
        gcmONERROR(gcoSURF_QueryFormat(dstSurf->format, &vars.dstFormatInfo));
    }
    else
    {
        vars.dstFormatInfo = &dstSurf->formatInfo;
    }

    /*
    ** Actually RS gamma correct has precision issue. so we will not use this feature any more.
    **
    if ((dstSurf->colorSpace == gcvSURF_COLOR_SPACE_NONLINEAR) && (srcSurf->colorSpace == gcvSURF_COLOR_SPACE_LINEAR))
    {
        gammarCorrection = gcvTRUE;
    }
    */

    /* Determine if y flipping is required. */
    vars.flipY = (srcSurf->orientation != dstSurf->orientation) || yInverted;

    if (vars.flipY && !Hardware->features[gcvFEATURE_FLIP_Y])
    {
        dstSurf->orientation = srcSurf->orientation;
        vars.flipY           = gcvFALSE;
    }

    /* Determine source tiling. */
    vars.srcTiling = srcSurf->tiling;

    vars.srcMultiPipe = (srcSurf->tiling & gcvTILING_SPLIT_BUFFER);

    /* Determine destination tiling. */
    vars.dstTiling = dstSurf->tiling;

    vars.dstMultiPipe = (dstSurf->tiling & gcvTILING_SPLIT_BUFFER);

    multiPipe = vars.srcMultiPipe || vars.dstMultiPipe || Hardware->multiPipeResolve;

    superSamplingIndex = (srcSurf->sampleInfo.y  << 3)
                       + (dstSurf->sampleInfo.y << 2)
                       + (srcSurf->sampleInfo.x  << 1)
                       + dstSurf->sampleInfo.x
                       - 15;

    vars.superSampling = &superSamplingTable[superSamplingIndex];

    /* Supported mode? */
    if (vars.superSampling->mode == ~0U)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "Supersampling mode is not defined for the given configuration:\n"
            );

        gcmTRACE(
            gcvLEVEL_ERROR,
            "  srcSurf->samples  = %dx%d\n",
            srcSurf->sampleInfo.x, srcSurf->sampleInfo.y
            );

        gcmTRACE(
            gcvLEVEL_ERROR,
            "  dstSurf->samples = %dx%d\n",
            dstSurf->sampleInfo.x, dstSurf->sampleInfo.y
            );

        gcmTRACE(
            gcvLEVEL_ERROR,
            "superSamplingIndex  = %d\n",
            superSamplingIndex
            );

        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    srcX = srcOrigin->x * srcSurf->sampleInfo.x;
    srcY = srcOrigin->y * srcSurf->sampleInfo.y;
    dstX = dstOrigin->x * dstSurf->sampleInfo.x;
    dstY = dstOrigin->y * dstSurf->sampleInfo.y;

    srcSurf->pfGetAddr(srcSurf,
                       srcX,
                       srcY,
                       SrcView->firstSlice,
                       srcMem);
    dstSurf->pfGetAddr(dstSurf,
                       dstX,
                       dstY,
                       DstView->firstSlice,
                       dstMem);

    /* Determine base addresses. */
    gcmGETHARDWAREADDRESS(srcSurf->node, srcAddress);
    srcAddress += (gctUINT32)((gctUINT8_PTR)srcMem[0] - srcSurf->node.logical);
    if (((srcX >> 3) ^ (srcY >> 2)) & 0x01)
    {
        /* Move to the top buffer */
        srcAddress -= srcSurf->bottomBufferOffset;
    }
    gcmGETHARDWAREADDRESS(dstSurf->node, dstAddress);
    dstAddress += (gctUINT32)((gctUINT8_PTR)dstMem[0] - dstSurf->node.logical);
    if (((dstX >> 3) ^ (dstY >> 2)) & 0x01)
    {
        /* Move to the top buffer */
        dstAddress -= dstSurf->bottomBufferOffset;
    }

    /* Linear address must be 64 byte aligned. */
    if ((srcSurf->tiling == gcvLINEAR && (srcAddress & 0x3F)) ||
        (dstSurf->tiling == gcvLINEAR && (dstAddress & 0x3F)))
    {
        gcmONERROR(gcvSTATUS_NOT_ALIGNED);
    }

    /* Construct configuration state. */
    config

        /* Configure source. */
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ?
 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) ((gctUINT32) (srcFormat) & ((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:7) - (0 ?
 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) (vars.srcTiling != gcvLINEAR) & ((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7)))

        /* Configure destination. */
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ?
 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) ((gctUINT32) (dstFormat) & ((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:14) - (0 ?
 14:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ?
 14:14))) | (((gctUINT32) ((gctUINT32) (vars.dstTiling != gcvLINEAR) & ((gctUINT32) ((((1 ?
 14:14) - (0 ? 14:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ?
 14:14)))

        /* Configure flipping. */
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 29:29) - (0 ?
 29:29) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) (flipRB) & ((gctUINT32) ((((1 ? 29:29) - (0 ?
 29:29) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ?
 29:29)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 30:30) - (0 ?
 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (vars.flipY) & ((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30)))

        /* Configure supersampling enable. */
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 6:5) - (0 ?
 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) | (((gctUINT32) ((gctUINT32) (vars.superSampling->mode) & ((gctUINT32) ((((1 ?
 6:5) - (0 ? 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5)))
        /* gamma correction */
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 13:13) - (0 ?
 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ?
 13:13))) | (((gctUINT32) ((gctUINT32) (gammarCorrection) & ((gctUINT32) ((((1 ?
 13:13) - (0 ? 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ?
 13:13)));


    srcStride = srcSurf->stride * (vars.srcTiling == gcvLINEAR ? 1 : 4);
    dstStride = dstSurf->stride * (vars.dstTiling == gcvLINEAR ? 1 : 4);

    /* Make sure the stride not exceed HW caps */
    gcmASSERT(srcStride <= ((gctUINT32) ((((1 ? 19:0) - (0 ? 19:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:0) - (0 ? 19:0) + 1))))));
    gcmASSERT(dstStride <= ((gctUINT32) ((((1 ? 19:0) - (0 ? 19:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:0) - (0 ? 19:0) + 1))))));

    srcStride = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:0) - (0 ? 19:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:0) - (0 ? 19:0) + 1))))))) << (0 ?
 19:0))) | (((gctUINT32) ((gctUINT32) (srcStride) & ((gctUINT32) ((((1 ?
 19:0) - (0 ? 19:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:0) - (0 ? 19:0) + 1))))))) << (0 ?
 19:0)));
    if (vars.srcTiling & gcvSUPERTILED)
    {
        srcStride = ((((gctUINT32) (srcStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));
    }
    if (vars.srcMultiPipe)
    {
        srcStride = ((((gctUINT32) (srcStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)));
    }

    if (Hardware->features[gcvFEATURE_128BTILE])
    {
        if (srcSurf->tiling == gcvSUPERTILED)
        {
            srcStride = ((((gctUINT32) (srcStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ? 28:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ?
 28:27))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 28:27) - (0 ? 28:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ? 28:27)));
        }
        else if (srcSurf->tiling == gcvYMAJOR_SUPERTILED)
        {
            srcStride = ((((gctUINT32) (srcStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ? 28:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ?
 28:27))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 28:27) - (0 ? 28:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ? 28:27)));
        }
        else
        {
            srcStride = ((((gctUINT32) (srcStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ? 28:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ?
 28:27))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 28:27) - (0 ? 28:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ? 28:27)));
        }
    }

    dstStride = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:0) - (0 ? 19:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:0) - (0 ? 19:0) + 1))))))) << (0 ?
 19:0))) | (((gctUINT32) ((gctUINT32) (dstStride) & ((gctUINT32) ((((1 ?
 19:0) - (0 ? 19:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:0) - (0 ? 19:0) + 1))))))) << (0 ?
 19:0)));
    if (vars.dstTiling & gcvSUPERTILED)
    {
        dstStride = ((((gctUINT32) (dstStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));
    }
    if (vars.dstMultiPipe)
    {
        dstStride = ((((gctUINT32) (dstStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)));
    }

    if (Hardware->features[gcvFEATURE_128BTILE])
    {
        if (dstSurf->tiling == gcvSUPERTILED)
        {
            dstStride = ((((gctUINT32) (dstStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ? 28:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ?
 28:27))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 28:27) - (0 ? 28:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ? 28:27)));
        }
        else if (dstSurf->tiling == gcvYMAJOR_SUPERTILED)
        {
            dstStride = ((((gctUINT32) (dstStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ? 28:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ?
 28:27))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 28:27) - (0 ? 28:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ? 28:27)));
        }
        else
        {
            dstStride = ((((gctUINT32) (dstStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ? 28:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ?
 28:27))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 28:27) - (0 ? 28:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ? 28:27)));
        }
    }

    /* If source rendered in 256B, RS need enable CACHE256 */
    if (Hardware->features[gcvFEATURE_128BTILE])
    {
        /*gcmONERROR(gcoHARDWARE_AdjustCacheMode(Hardware, srcSurf));*/

        srcStride = ((((gctUINT32) (srcStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) (srcSurf->cacheMode == gcvCACHE_256) & ((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ?
 29:29)));
    }
    else if (Hardware->features[gcvFEATURE_FAST_MSAA] ||
             Hardware->features[gcvFEATURE_SMALL_MSAA])
    {
        /*
        ** Adjust cache mode according to source surface
        */
        gcmONERROR(gcoHARDWARE_AdjustCacheMode(Hardware, srcSurf));

        if (srcSurf->isMsaa)
        {
            srcStride = ((((gctUINT32) (srcStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 29:29) - (0 ? 29:29) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)));
        }
        else
        {
            srcStride = ((((gctUINT32) (srcStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 29:29) - (0 ? 29:29) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)));
        }

        if (dstSurf->sampleInfo.product > srcSurf->sampleInfo.product)
        {
            /* With fast msaa, we cannot resolve onto itself.
               The code path leading to this point should be changed. */
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }

    if (((srcSurf->formatInfo.fmtClass == gcvFORMAT_CLASS_DEPTH) ||
         (dstSurf->formatInfo.fmtClass == gcvFORMAT_CLASS_DEPTH))
       &&
        (((srcSurf->formatInfo.fmtClass == gcvFORMAT_CLASS_DEPTH) !=
          (dstSurf->formatInfo.fmtClass == gcvFORMAT_CLASS_DEPTH)) ||
         (srcSurf->formatInfo.bitsPerPixel != dstSurf->formatInfo.bitsPerPixel))
       )
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    if (srcSurf->isMsaa && srcSurf->compressed &&
        (dstSurf->sampleInfo.product == srcSurf->sampleInfo.product) &&
        !Hardware->features[gcvFEATURE_RSBLT_MSAA_DECOMPRESSION])
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Set endian control */
    endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) (0x0  & ((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)));

    if (Hardware->bigEndian &&
        (srcSurf->type != gcvSURF_TEXTURE) &&
        (dstSurf->type == gcvSURF_BITMAP))
    {
        if (dstSurf->formatInfo.bitsPerPixel == 16)
        {
            endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) (0x1  & ((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)));
        }
        else if (dstSurf->formatInfo.bitsPerPixel == 32)
        {
            endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) (0x2  & ((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)));
        }
    }

    /* Determine whether resolve striping is needed. */
    needStriping
        =  Hardware->needStriping
        && (((((gctUINT32) (Hardware->MCStates->memoryConfig)) >> (0 ? 1:1)) & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1)))))) );

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, cmdBuffer);

#if gcdENABLE_TRUST_APPLICATION
    if (Hardware->features[gcvFEATURE_SECURITY])
    {
        gcoHARDWARE_SetProtectMode(
            Hardware,
            ((srcSurf->hints & gcvSURF_PROTECTED_CONTENT) ||
             (dstSurf->hints & gcvSURF_PROTECTED_CONTENT)),
            (gctPOINTER *)&memory);

        Hardware->GPUProtecedModeDirty = gcvTRUE;
    }
#endif
    /* Flush the pipe. */
    gcmONERROR(gcoHARDWARE_FlushPipe(Hardware, (gctPOINTER *)&memory));

    /* Switch to 3D pipe. */
    gcmONERROR(gcoHARDWARE_SelectPipe(Hardware, gcvPIPE_3D, (gctPOINTER *)&memory));

    /* Append RESOLVE_CONFIG state. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0581) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0581, config );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    /* Set source and destination stride. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0583) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0583, srcStride );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0585) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0585, dstStride );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    /* Determine dithering. */
    if (srcSurf->deferDither3D &&
        (vars.srcFormatInfo->bitsPerPixel > vars.dstFormatInfo->bitsPerPixel))
    {
        pDitherTable = &Hardware->PEStates->ditherTable[gcvTRUE][0];
    }
    else
    {
        pDitherTable = &Hardware->PEStates->ditherTable[gcvFALSE][0];
    }

    /* Append RESOLVE_DITHER commands. */
    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)2  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x058C) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


        gcmSETCTRLSTATE_NEW(
            stateDelta, reserve, memory, 0x058C,
            pDitherTable[0]
            );

        gcmSETCTRLSTATE_NEW(
            stateDelta, reserve, memory, 0x058C + 1,
            pDitherTable[1]
            );

        gcmSETFILLER_NEW(
            reserve, memory
            );

    gcmENDSTATEBATCH_NEW(
        reserve, memory
        );

    /* Append RESOLVE_CLEAR_CONTROL state. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x058F) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x058F, 0 );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    /* Append new configuration register. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05A8) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x05A8, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) (!multiPipe) & ((gctUINT32) ((((1 ?
 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ?
 20:20))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:6) - (0 ?
 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ?
 7:6))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 7:6) - (0 ?
 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ?
 7:6))) | endian );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    {
        gctUINT32 dstBase;
        gcmGETHARDWAREADDRESS(dstSurf->node, dstBase);
        gcmDUMP(gcvNULL, "#[surface 0x%x 0x%x]", dstBase, dstSurf->node.size);
    }

    if (needStriping)
    {
        /* Stripe the resolve. */
        gcmONERROR(_StripeResolve(Hardware,
                                  srcSurf,
                                  dstSurf,
                                  srcOrigin,
                                  dstOrigin,
                                  rectSize,
                                  &vars,
                                  (gctPOINTER *)&memory));
    }
    else
    {
        /* Determine source rectangle size. */
        srcSize.x = rectSize->x * vars.superSampling->horFactor;
        srcSize.y = rectSize->y * vars.superSampling->verFactor;

        if (srcSurf->tiling & gcvTILING_SPLIT_BUFFER)
        {
             srcAddress2 = srcAddress + srcSurf->bottomBufferOffset;

            /* Append RESOLVE_SOURCE addresses. */
             {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)2  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05B0) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


                 gcmSETCTRLSTATE_NEW(
                     stateDelta, reserve, memory, 0x05B0,
                     srcAddress
                     );

                 gcmSETCTRLSTATE_NEW(
                     stateDelta, reserve, memory, 0x05B0 + 1,
                     srcAddress2
                     );

                 gcmSETFILLER_NEW(
                     reserve, memory
                     );

             gcmENDSTATEBATCH_NEW(
                 reserve, memory
                 );
        }
        else
        {
            /* Append RESOLVE_SOURCE commands. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0582) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0582, srcAddress );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            if (Hardware->features[gcvFEATURE_RS_NEW_BASEADDR])
            {
                /* Append RESOLVE_SOURCE commands. */
                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05B0) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x05B0, srcAddress );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            }
        }

        if (dstSurf->tiling & gcvTILING_SPLIT_BUFFER)
        {
            dstAddress2 = dstAddress + dstSurf->bottomBufferOffset;

            /* Append RESOLVE_DESTINATION addresses. */
            {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)2  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05B8) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


                gcmSETCTRLSTATE_NEW(
                    stateDelta, reserve, memory, 0x05B8,
                    dstAddress
                    );

                gcmSETCTRLSTATE_NEW(
                    stateDelta, reserve, memory, 0x05B8 + 1,
                    dstAddress2
                    );

                gcmSETFILLER_NEW(
                    reserve, memory
                    );

            gcmENDSTATEBATCH_NEW(
                reserve, memory
                );
        }
        else
        {
            /* Append RESOLVE_DESTINATION commands. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0584) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0584, dstAddress );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            if (Hardware->features[gcvFEATURE_RS_NEW_BASEADDR])
            {
                /* Append RESOLVE_DESTINATION commands. */
                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05B8) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x05B8, dstAddress );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            }
        }

        if (Hardware->robust)
        {
            gctUINT32 bufSize;
            gctUINT32 surfaceBase;
            gctUINT32 endAddress;

            gcmSAFECASTSIZET(bufSize, srcSurf->node.size);
            gcmGETHARDWAREADDRESS(srcSurf->node, surfaceBase);
            endAddress = surfaceBase + bufSize - 1;
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x069A) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x069A, endAddress );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            gcmSAFECASTSIZET(bufSize, dstSurf->node.size);
            gcmGETHARDWAREADDRESS(dstSurf->node, surfaceBase);
            endAddress = surfaceBase + bufSize - 1;
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x06B9) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x06B9, endAddress );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }

        if (!srcSurf->superTiled && !dstSurf->superTiled && (dstSurf->tiling == gcvLINEAR) &&
            (Hardware->config->chipModel == gcv2000) && (Hardware->config->chipRevision == 0x5108) &&
            (!Hardware->features[gcvFEATURE_SUPER_TILED]))
        {
             gctINT32 i, h = 8;
             gctUINT32 dstBufSize = (dstSurf->alignedH - h) * dstSurf->stride;
             if(vars.flipY)
             {
                 for (i = 0; i < srcSize.y/h; i++)
                 {
                    gcsPOINT newSize;
                    newSize.x = srcSize.x;
                    newSize.y = h;

                    gcoHARDWARE_LoadCtrlStateNEW(Hardware,
                            0x16C0,
                            srcAddress  + i*(h/2)*srcSurf->stride,
                            (gctPOINTER *)&memory);
                    gcoHARDWARE_LoadCtrlStateNEW(Hardware,
                            0x16C4,
                            srcAddress2 + i*(h/2)*srcSurf->stride,
                            (gctPOINTER *)&memory);

                    gcoHARDWARE_LoadCtrlStateNEW(Hardware,
                            0x16E0,
                            dstAddress + dstBufSize - (i*h*dstSurf->stride)/srcSurf->sampleInfo.y,
                            (gctPOINTER *)&memory);
                    gcoHARDWARE_LoadCtrlStateNEW(Hardware,
                            0x16E4,
                            dstAddress + dstBufSize - (i*h*dstSurf->stride + (h/2)*dstSurf->stride)/srcSurf->sampleInfo.y,
                            (gctPOINTER *)&memory);
                    /* Program the resolve window and trigger it. */
                    gcmONERROR(gcoHARDWARE_ProgramResolve(Hardware, newSize, multiPipe, srcDownsampleMode, (gctPOINTER *)&memory));
                 }
             }
             else
             {
                 for (i = 0; i < srcSize.y/h; i++)
                 {
                    gcsPOINT newSize;
                    newSize.x = srcSize.x;
                    newSize.y = h;

                    gcoHARDWARE_LoadCtrlStateNEW(Hardware,
                            0x16C0,
                            srcAddress  + i*(h/2)*srcSurf->stride,
                            (gctPOINTER *)&memory);
                    gcoHARDWARE_LoadCtrlStateNEW(Hardware,
                            0x16C4,
                            srcAddress2 + i*(h/2)*srcSurf->stride,
                            (gctPOINTER *)&memory);

                    gcoHARDWARE_LoadCtrlStateNEW(Hardware,
                            0x16E0,
                            dstAddress + (i*h*dstSurf->stride)/srcSurf->sampleInfo.y,
                            (gctPOINTER *)&memory);
                    gcoHARDWARE_LoadCtrlStateNEW(Hardware,
                            0x16E4,
                            dstAddress + (i*h*dstSurf->stride + (h/2)*dstSurf->stride)/srcSurf->sampleInfo.y,
                            (gctPOINTER *)&memory);
                    /* Program the resolve window and trigger it. */
                    gcmONERROR(gcoHARDWARE_ProgramResolve(Hardware, newSize, multiPipe, srcDownsampleMode, (gctPOINTER *)&memory));
                 }
             }
        }
        else
        {
             /* Program the resolve window and trigger it. */
             gcmONERROR(gcoHARDWARE_ProgramResolve(Hardware, srcSize, multiPipe, srcDownsampleMode, (gctPOINTER *)&memory));
        }
    }
    /* Make compiler happy */
    stateDelta = stateDelta;

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, cmdBuffer);

    /* Tile status cache is dirty. */
    Hardware->MCDirty->cacheDirty = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_ComputeOffset
**
**  Compute the offset of the specified pixel location.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctINT32 X, Y
**          Coordinates of the pixel.
**
**      gctUINT Stride
**          Surface stride.
**
**      gctINT BytesPerPixel
**          The number of bytes per pixel in the surface.
**
**      gctINT Tiling
**          Tiling type.
**
**  OUTPUT:
**
**      Computed pixel offset.
*/
gceSTATUS gcoHARDWARE_ComputeOffset(
    IN gcoHARDWARE Hardware,
    IN gctINT32 X,
    IN gctINT32 Y,
    IN gctUINT Stride,
    IN gctINT BytesPerPixel,
    IN gceTILING Tiling,
    OUT gctUINT32_PTR Offset
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if ((X == 0) && (Y == 0))
    {
        *Offset = 0;
    }
    else
    {
        if (Tiling == gcvLINEAR)
        {
            /* Linear. */
            * Offset

                /* Skip full rows of pixels. */
                = Y * Stride

                /* Skip pixels to the left. */
                + X * BytesPerPixel;
        }
        else
        {
            gctBOOL superTiled = (Tiling & gcvSUPERTILED) > 1;

            gcmGETHARDWARE(Hardware);

            if (Tiling & gcvTILING_SPLIT_BUFFER)
            {
                /* Calc offset in the tile part of one PE. */
                /* Adjust coordinates from split PE into one PE. */
                X = (X  & ~0x8) | ((Y & 0x4) << 1);
                Y = ((Y & ~0x7) >> 1) | (Y & 0x3);
            }

            * Offset

                /* Skip full rows of 64x64 tiles to the top. */
                = (Stride
                   * gcmTILE_OFFSET_Y(X, Y, superTiled))

                + (BytesPerPixel
                   * gcmTILE_OFFSET_X(X, Y, superTiled, Hardware->config->superTileMode));
        }
    }

OnError:
    /* Return status. */
    return status;
}


/* We need to program those pesky 256-byte cache configuration bits inside
 * the Pixel Engine - because that is where the Tile Status is looking at
 * as well. And who guarantees this is the same surface as the current one?
 * And then - we also have to make the states dirty so we can reprogram them
 * if required.
 */
gceSTATUS
gcoHARDWARE_AdjustCacheMode(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x, Surface=0x%x", Hardware, Surface);

    if (Hardware->features[gcvFEATURE_FAST_MSAA] ||
        Hardware->features[gcvFEATURE_SMALL_MSAA])
    {
        if ((Surface->format <= gcvSURF_YUY2) ||    /* Skip unsupported format by FlushTarget. */
            (Surface->format > gcvSURF_VYUY))
        {
            gctBOOL msaaEnable = Surface->isMsaa;
            gctUINT cacheMode  = msaaEnable ? 1 : 0;

            /* Check if the cache mode is different. */
            if ((cacheMode != Hardware->PEStates->colorStates.cacheMode) ||
                (cacheMode != Hardware->PEStates->depthStates.cacheMode))
            {
                gctUINT32 msaaMode;
                /* Define state buffer variables. */
                gcmDEFINESTATEBUFFER(reserve, stateDelta, memory, reserveSize);

                msaaMode = msaaEnable ? 0x2 :
                                        0x0;

                reserveSize = /* Flush cache */
                              gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32), 8)       +
                              /* semaphore stall */
                              (gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32), 8) * 2) +
                              /* cache control */
                              gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32), 8)       +
                              /* MSAA mode */
                              gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32), 8);

                /* Switch to 3D pipe. */
                gcmONERROR(gcoHARDWARE_SelectPipe(gcvNULL, gcvPIPE_3D, gcvNULL));

                /* Reserve space in the command buffer. */
                gcmBEGINSTATEBUFFER(Hardware, reserve, stateDelta, memory, reserveSize);


                {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0E03, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
gcmSETCTRLSTATE(stateDelta, reserve, memory, 0x0E03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) );gcmENDSTATEBATCH(reserve, memory);
};


                /* We also have to sema stall between FE and PE to make sure the flush
                ** has happened. */
                {     {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0E02, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
gcmSETCTRLSTATE(stateDelta, reserve, memory, 0x0E02, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));gcmENDSTATEBATCH(reserve, memory);
};
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));    gcmDUMP(gcvNULL, "@[stall 0x%08X 0x%08X]",    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ?
 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))),    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));    gcmSKIPSECUREUSER();
    }

                /* We need to program fast msaa config if the cache mode is changing. */
                {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0529, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0529) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATAWITHMASK(stateDelta, reserve, memory, gcvFALSE, 0x0529, (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 24:24) - (0 ?
 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:25) - (0 ? 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 25:25) - (0 ?
 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ?
 25:25)))) | (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 26:26) - (0 ?
 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:27) - (0 ? 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 27:27) - (0 ?
 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ?
 27:27)))), (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (msaaEnable ? 1 : 0) & ((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:25) - (0 ?
 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 25:25) - (0 ? 25:25) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25)))) & (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (msaaEnable ? 1 : 0) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:27) - (0 ?
 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 27:27) - (0 ? 27:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ? 27:27)))));     gcmENDSTATEBATCH(reserve, memory);
};


                /* Set multi-sample mode. */
                {    {    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0E06, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E06) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA(stateDelta, reserve, memory, gcvFALSE, 0x0E06, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (msaaMode) & ((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:12) - (0 ?
 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 14:12) - (0 ? 14:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 17:16) - (0 ? 17:16) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (msaaEnable) & ((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) );     gcmENDSTATEBATCH(reserve, memory);
};



                gcmENDSTATEBUFFER(Hardware, reserve, memory, reserveSize);

                Hardware->PEStates->colorStates.cacheMode = cacheMode;
                Hardware->PEStates->depthStates.cacheMode = cacheMode;

                Hardware->MsaaDirty->msaaModeDirty    = gcvTRUE;
                Hardware->MsaaDirty->msaaConfigDirty  = gcvTRUE;

                /* Test for error. */
                gcmONERROR(status);
            }
        }
    }
OnError:
    /* Return result. */
    gcmFOOTER();
    return status;

}

/*******************************************************************************
**
**  gcoHARDWARE_ResolveRect
**
**  Resolve a rectangluar area of a surface to another surface.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsSURF_VIEW *SrcView,
**          Pointer to the source surface view.
**
**      gcsSURF_VIEW *DstView
**          Pointer to the destination surface view.
**
**      gcsSURF_RESOLVE_ARGS *Args
**          Pointer to the resolve arguments.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_ResolveRect(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_VIEW *SrcView,
    IN gcsSURF_VIEW *DstView,
    IN gcsSURF_RESOLVE_ARGS *Args
    )
{
    gceSTATUS status;
    gctBOOL locked = gcvFALSE;
#if gcdENABLE_3D
    gctBOOL switchedTileStatus = gcvFALSE;
    gctBOOL pausedTileStatus = gcvFALSE;
    gcoSURF saved = gcvNULL;
#endif
    gctBOOL srcLocked = gcvFALSE;
    gctBOOL dstLocked = gcvFALSE;
    gctBOOL resampling;
    gctBOOL dithering;
    gctBOOL srcTiled;
    gctBOOL dstTiled;
    gcsPOINT alignedSrcOrigin, alignedSrcSize;
    gcsPOINT alignedDstOrigin, alignedDstSize;
    gcsPOINT alignedSrcOriginExpand = {0, 0};
    gcsPOINT alignedDstOriginExpand = {0, 0};
    gctBOOL originsMatch;
    gcsPOINT alignedRectSize;
    gcsSURF_FORMAT_INFO_PTR srcFormatInfo;
    gcsSURF_FORMAT_INFO_PTR dstFormatInfo;
    gcsSURF_FORMAT_INFO_PTR tmpFormatInfo;
    gctBOOL flip = gcvFALSE;
    gctBOOL expandRectangle = gcvTRUE;
    gctUINT32 srcAlignmentX = 0;
    gctUINT32 dstAlignmentX = 0;
    gctUINT32 srcFormat, dstFormat;
    gctBOOL flipRB = gcvFALSE;
    gctBOOL swcopy = gcvFALSE;
    gctUINT32 tileStatusAddress;

    gcoSURF tmpSurf = gcvNULL;
    gcoSURF srcSurf = SrcView->surf;
    gcoSURF dstSurf = DstView->surf;
    gcsPOINT_PTR srcOrigin = &Args->uArgs.v2.srcOrigin;
    gcsPOINT_PTR dstOrigin = &Args->uArgs.v2.dstOrigin;
    gcsPOINT_PTR rectSize  = &Args->uArgs.v2.rectSize;

    gcmHEADER_ARG("Hardware=0x%x SrcView=0x%x DstView=0x%x Args=0x%x", Hardware, SrcView, DstView, Args);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Args->version != gcvHAL_ARG_VERSION_V2)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    tmpSurf = &Hardware->tmpSurf;
#if gcdENABLE_3D

    /* Resolve destination surface and disable its tile status buffer . */
    if (!Hardware->MCStates->inFlush && !dstSurf->tileStatusDisabled)
    {
        gcmONERROR(gcoHARDWARE_DisableTileStatus(Hardware, dstSurf, gcvTRUE));
    }

    if (Hardware->features[gcvFEATURE_BLT_ENGINE])
    {
        gcmONERROR(gcoHARDWARE_AdjustCacheMode(Hardware, srcSurf));

        Args->uArgs.v2.engine = gcvENGINE_RENDER;
        if (gcvSTATUS_OK == gcoHARDWARE_3DBlitBlt(Hardware, SrcView, DstView, Args))
        {
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }
    }

    /*
    ** Halti2 RS can look depth FC register, before we add to support it,
    ** RS only look color FC registers. For depth surface, we will fake it
    ** to color surface and program it on color FC registers, which should
    ** be done before calling to this function. So just assert here.
    */
    if (!(srcSurf->tileStatusDisabled) && (srcSurf->type == gcvSURF_DEPTH))
    {
        gcmASSERT(0);
    }

    /* RS engine only look tile status buffer for source, so we can enable it for source
    ** instead of resolving it before blit.
    ** If the src surface is not current one, we can try to enable its tile status buffer
    ** for RS blit.
    */
    if ((srcSurf != Hardware->PEStates->colorStates.target[0].surface) &&
        (srcSurf->type == gcvSURF_RENDER_TARGET))
    {
        /* Save the current surface. */
        saved = Hardware->PEStates->colorStates.target[0].surface;

        /* Start working in the requested surface. */
        Hardware->PEStates->colorStates.target[0].surface = srcSurf;

        gcmGETHARDWAREADDRESS(srcSurf->tileStatusNode, tileStatusAddress);

        /* Enable the tile status for the requested surface. */
        status = gcoHARDWARE_EnableTileStatus(
            Hardware,
            srcSurf,
            tileStatusAddress,
            &srcSurf->hzTileStatusNode,
            0);

        /* Start working in the requested surface. */
        Hardware->PEStates->colorStates.target[0].surface = saved;

        switchedTileStatus = gcvTRUE;
    }
    else if (srcSurf != Hardware->PEStates->colorStates.target[0].surface)
    {
        gcoHARDWARE_PauseTileStatus(Hardware, gcvTILE_STATUS_PAUSE);
        pausedTileStatus = gcvTRUE;
    }

    flip = (srcSurf->orientation != dstSurf->orientation) || (Args->uArgs.v2.yInverted);
#endif

    /***********************************************************************
    ** Determine special functions.
    */

    srcTiled = (srcSurf->type != gcvSURF_BITMAP);
    dstTiled = (dstSurf->type != gcvSURF_BITMAP);

    resampling = Args->uArgs.v2.resample;

    dithering = srcSurf->deferDither3D && (srcSurf->bitsPerPixel > dstSurf->bitsPerPixel);

    /* check if the format is supported by hw. */
    if (gcmIS_ERROR(_ConvertResolveFormat(Hardware,
                              srcSurf->format,
                              dstSurf->format,
                              &srcFormat,
                              &dstFormat,
                              &flipRB,
                              gcvNULL)))
    {
        swcopy = gcvTRUE;
    }

    /* RS can't support tile<->tile flip y */
    if (flip &&
        (((srcSurf->tiling != gcvLINEAR) && (dstSurf->tiling != gcvLINEAR)) ||
         (!Hardware->features[gcvFEATURE_FLIP_Y])
        )
       )
    {
        swcopy = gcvTRUE;
    }

    /***********************************************************************
    ** Since 2D bitmaps live in linear space, we don't need to resolve them.
    ** We can just copy them using the 2D engine.  However, we need to flush
    ** and stall after we are done with the copy since we have to make sure
    ** the bits are there before we blit them to the screen.
    */
    if (!srcTiled && !dstTiled && !resampling && !dithering && !flip)
    {
        gcmONERROR(_SourceCopy(Hardware, SrcView, DstView, Args));
    }
    /* YUV 4:2:0 tiling case */
    else if ((srcSurf->format == gcvSURF_YV12) ||
             (srcSurf->format == gcvSURF_I420) ||
             (srcSurf->format == gcvSURF_NV12) ||
             (srcSurf->format == gcvSURF_NV21))
    {
        if (!srcTiled && dstTiled && (dstSurf->format == gcvSURF_YUY2))
        {
            gcmONERROR(_Tile420Surface(
                Hardware,
                srcSurf, dstSurf,
                srcOrigin, dstOrigin,
                rectSize
                ));
        }
        else
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }
    /* Unsupported cases */
    else if ((srcSurf->sampleInfo.x < dstSurf->sampleInfo.x) ||
             (srcSurf->sampleInfo.y < dstSurf->sampleInfo.y) ||  /* Upsampling. */
             swcopy /* the format that hw do not support*/
    )
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }
    /***********************************************************************
    ** Calculate the aligned source and destination rectangles; aligned to
    ** completely cover the specified source and destination areas.
    */
    else
    {
        gcsSURF_RESOLVE_ARGS newArgs;
        gctBOOL srcMultiPipe = (srcSurf->tiling & gcvTILING_SPLIT_BUFFER);
        gctBOOL dstMultiPipe = (dstSurf->tiling & gcvTILING_SPLIT_BUFFER);

        gcoOS_MemCopy(&newArgs, Args, gcmSIZEOF(newArgs));
        newArgs.uArgs.v2.yInverted  = flip;

        _AlignResolveRect(
            Hardware, srcSurf, srcOrigin, rectSize, &alignedSrcOrigin, &alignedSrcSize
            );

        _AlignResolveRect(
            Hardware, dstSurf, dstOrigin, rectSize, &alignedDstOrigin, &alignedDstSize
            );


        /* Use the maximum rectangle. */
        alignedRectSize.x = gcmMAX(alignedSrcSize.x, alignedDstSize.x);
        alignedRectSize.y = gcmMAX(alignedSrcSize.y, alignedDstSize.y);

        /***********************************************************************
        ** If specified and aligned rectangles are the same, then the requested
        ** rectangle is perfectly aligned and we can do it in one shot.
        */
        originsMatch
            =  (alignedSrcOrigin.x == srcOrigin->x)
            && (alignedSrcOrigin.y == srcOrigin->y)
            && (alignedDstOrigin.x == dstOrigin->x)
            && (alignedDstOrigin.y == dstOrigin->y);

        if (!originsMatch)
        {
            /***********************************************************************
            ** The origins are changed.
            */
            gctINT srcDeltaX, srcDeltaY;
            gctINT dstDeltaX, dstDeltaY;
            gctINT maxDeltaX, maxDeltaY;

            srcDeltaX = srcOrigin->x - alignedSrcOrigin.x;
            srcDeltaY = srcOrigin->y - alignedSrcOrigin.y;

            dstDeltaX = dstOrigin->x - alignedDstOrigin.x;
            dstDeltaY = dstOrigin->y - alignedDstOrigin.y;

            maxDeltaX = gcmMAX(srcDeltaX, dstDeltaX);
            maxDeltaY = gcmMAX(srcDeltaY, dstDeltaY);

            if (srcDeltaX == maxDeltaX)
            {
                /* The X coordinate of dst rectangle is changed. */
                alignedDstOriginExpand.x = dstOrigin->x - maxDeltaX;
                alignedSrcOriginExpand.x = alignedSrcOrigin.x;
            }
            else
            {
                /* The X coordinate of src rectangle is changed. */
                alignedSrcOriginExpand.x = srcOrigin->x - maxDeltaX;
                alignedDstOriginExpand.x = alignedDstOrigin.x;
            }

            if (srcDeltaY == maxDeltaY)
            {
                /* Expand the Y coordinate of dest rectangle. */
                alignedDstOriginExpand.y = dstOrigin->y - maxDeltaY;
                alignedSrcOriginExpand.y = alignedSrcOrigin.y;
            }
            else
            {
                /* Expand the Y coordinate of src rectangle. */
                alignedSrcOriginExpand.y = srcOrigin->y - maxDeltaY;
                alignedDstOriginExpand.y = alignedDstOrigin.y;
            }

            if ((alignedSrcOriginExpand.x < 0) ||
                (alignedSrcOriginExpand.y < 0) ||
                (alignedDstOriginExpand.x < 0) ||
                (alignedDstOriginExpand.y < 0))
            {
                expandRectangle = gcvFALSE;
            }
            else
            {
                gcoHARDWARE_GetSurfaceResolveAlignment(Hardware,
                                                       srcSurf,
                                                       &srcAlignmentX,
                                                       gcvNULL,
                                                       gcvNULL,
                                                       gcvNULL);

                gcoHARDWARE_GetSurfaceResolveAlignment(Hardware,
                                                       dstSurf,
                                                       &dstAlignmentX,
                                                       gcvNULL,
                                                       gcvNULL,
                                                       gcvNULL);
            }
        }

        /* Fully aligned Origin and RectSize. */
        if (originsMatch &&
            (alignedRectSize.x == rectSize->x) &&
            (alignedRectSize.y == rectSize->y))
        {
            gcmONERROR(_ResolveRect(Hardware, SrcView, DstView, Args));
        }
        else if (resampling)
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
        /* Handle supertiling */
        else if ((originsMatch) &&
                 (dstTiled) &&
                 ((rectSize->x > 64) && !(rectSize->x & 3)) &&
                 (!srcMultiPipe && !dstMultiPipe && !(rectSize->y & 3)) &&
                 (srcSurf->superTiled && !dstSurf->superTiled) &&
                 (!(srcOrigin->x  & 63) && !(srcOrigin->y & 63))
                )
        {
            const gctUINT32 tileSize = 64;
            gctINT interiorWidth = (rectSize->x / tileSize) * tileSize;

            if (Args->uArgs.v2.gpuOnly)
            {
                /* Software copy now allowed. */
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }

            /* First resolve: Resolve tile aligned interior region. */
            newArgs.uArgs.v2.srcOrigin.x = srcOrigin->x;
            newArgs.uArgs.v2.srcOrigin.y = srcOrigin->y;
            newArgs.uArgs.v2.dstOrigin.x = dstOrigin->x;
            newArgs.uArgs.v2.dstOrigin.y = dstOrigin->y;
            newArgs.uArgs.v2.rectSize.x  = interiorWidth;
            newArgs.uArgs.v2.rectSize.y  = rectSize->y;
            gcmONERROR(_ResolveRect(Hardware, SrcView, DstView, &newArgs));

            /* Second resolve: Resolve the right end. */
            newArgs.uArgs.v2.srcOrigin.x = srcOrigin->x + interiorWidth;
            newArgs.uArgs.v2.srcOrigin.y = srcOrigin->y;
            newArgs.uArgs.v2.dstOrigin.x = dstOrigin->x + interiorWidth;
            newArgs.uArgs.v2.dstOrigin.y = dstOrigin->y;
            newArgs.uArgs.v2.rectSize.x  = rectSize->x - interiorWidth;
            newArgs.uArgs.v2.rectSize.y  = rectSize->y;
            gcmONERROR(gcoHARDWARE_CopyPixels(Hardware, SrcView, DstView, &newArgs));
        }
        /* Aligned Origin and tile aligned RectSize. */
        /* Assuming tile size of 4x4 pixels, and resolve size requirement of 4 tiles. */
        else if (originsMatch &&
                 (rectSize->x > 16) &&
                ((rectSize->x & 3) == 0) &&
                (!srcMultiPipe && !dstMultiPipe && ((rectSize->y & 3) == 0)))
        {
            gctUINT32 extraTiles;
            gctUINT32 tileSize = 4, numTiles = 4;
            gctUINT32 loopCount = 0;

            extraTiles = numTiles - (alignedRectSize.x - rectSize->x) / tileSize;

            /* First resolve: Resolve 4 tile aligned interior region. */
            newArgs.uArgs.v2.srcOrigin.x = srcOrigin->x;
            newArgs.uArgs.v2.srcOrigin.y = srcOrigin->y;
            newArgs.uArgs.v2.dstOrigin.x = dstOrigin->x;
            newArgs.uArgs.v2.dstOrigin.y = dstOrigin->y;
            newArgs.uArgs.v2.rectSize.x  = rectSize->x - extraTiles * tileSize;
            newArgs.uArgs.v2.rectSize.y  = rectSize->y;
            gcmONERROR(_ResolveRect(Hardware, SrcView, DstView, &newArgs));

            /* Second resolve: Resolve last 4 tiles at the right end. */
            newArgs.uArgs.v2.srcOrigin.x = srcOrigin->x + rectSize->x - (4 * tileSize);
            newArgs.uArgs.v2.srcOrigin.y = srcOrigin->y;
            newArgs.uArgs.v2.dstOrigin.x = dstOrigin->x + rectSize->x - (4 * tileSize);
            newArgs.uArgs.v2.dstOrigin.y = dstOrigin->y;
            newArgs.uArgs.v2.rectSize.x  = 4 * tileSize;
            newArgs.uArgs.v2.rectSize.y  = 4 * Hardware->config->resolvePipes;

            do
            {
                gcmASSERT(newArgs.uArgs.v2.srcOrigin.y < (srcOrigin->y + rectSize->y));
                gcmONERROR(_ResolveRect(Hardware, SrcView, DstView, &newArgs));
                newArgs.uArgs.v2.srcOrigin.y += 4 * Hardware->config->resolvePipes;
                newArgs.uArgs.v2.dstOrigin.y += 4 * Hardware->config->resolvePipes;
                loopCount++;
            }
            while (newArgs.uArgs.v2.srcOrigin.y != (srcOrigin->y + rectSize->y) && (loopCount < MAX_LOOP_COUNT));
        }

        /***********************************************************************
        ** Not matched origins.
        ** We expand the rectangle to guarantee the alignment requirements.
        */
        else if (!originsMatch && expandRectangle &&
                 !(alignedSrcOriginExpand.x & (srcAlignmentX -1)) &&
                 !(alignedSrcOriginExpand.y & 3) &&
                 !(alignedDstOriginExpand.x & (dstAlignmentX - 1)) &&
                 !(alignedDstOriginExpand.y & 3))
        {
            /* TODO: log print for debug. */
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        /***********************************************************************
        ** Handle all cases when source and destination are both tiled.
        */
        else if ((srcSurf->type != gcvSURF_BITMAP) &&
                 (dstSurf->type != gcvSURF_BITMAP))
        {
            static gctBOOL printed = gcvFALSE;

            if (Args->uArgs.v2.gpuOnly)
            {
                /* Software copy now allowed. */
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }

            /* Flush the pipe. */
            gcmONERROR(gcoHARDWARE_FlushPipe(Hardware, gcvNULL));

#if gcdENABLE_3D
            /* Disable the tile status. */
            gcmONERROR(gcoHARDWARE_DisableTileStatus(
                Hardware, srcSurf, gcvTRUE
                ));
#endif

            /* Lock the source surface. */
            gcmONERROR(gcoHARDWARE_Lock(
                &srcSurf->node, gcvNULL, gcvNULL
                ));

            srcLocked = gcvTRUE;

            /* Lock the destination surface. */
            gcmONERROR(gcoHARDWARE_Lock(
                &dstSurf->node, gcvNULL, gcvNULL
                ));

            dstLocked = gcvTRUE;

            if (!printed)
            {
                printed = gcvTRUE;

                gcmPRINT("libGAL: Performing a software resolve!");
            }

            /* Perform software copy. why???*/
            if (Hardware->config->resolvePipes > 1)
            {
                gcmONERROR(gcoHARDWARE_CopyPixels(Hardware, SrcView, DstView, Args));
            }
            else
            {
                gcmONERROR(_SoftwareCopy(Hardware, SrcView, DstView, Args));
            }
        }

        /***********************************************************************
        ** At least one side of the rectangle is not aligned. In this case we
        ** will allocate a temporary buffer to resolve the aligned rectangle
        ** to and then use a source copy to complete the operation.
        */
        else
        {
            gcsSURF_VIEW tmpView;
            gctPOINTER memory = gcvNULL;
            gceORIENTATION orientation;

            if (Args->uArgs.v2.gpuOnly)
            {
                /* Software copy now allowed. */
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }

            /* Query format specifics. */
            srcFormatInfo = &srcSurf->formatInfo;
            dstFormatInfo = &dstSurf->formatInfo;

            /* Pick the most compact format for the temporary surface. */
            tmpFormatInfo
                = (srcFormatInfo->bitsPerPixel < dstFormatInfo->bitsPerPixel)
                    ? srcFormatInfo
                    : dstFormatInfo;

            /* Allocate the temporary surface */
            gcmONERROR(gcoHARDWARE_AllocTmpSurface(
                Hardware,
                (gctUINT)alignedSrcSize.x,
                (gctUINT)alignedSrcSize.y,
                tmpFormatInfo,
                dstSurf->type,
                dstSurf->hints
                ));

            /* Lock the temporary surface. */
            gcmONERROR(gcoHARDWARE_Lock(
                &tmpSurf->node,
                gcvNULL,
                &memory
                ));

            /* Mark as locked. */
            locked = gcvTRUE;

            /* Use same orientation as source. */
            orientation = tmpSurf->orientation;
            tmpSurf->orientation = srcSurf->orientation;

            tmpView.surf = &Hardware->tmpSurf;
            tmpView.firstSlice = 0;
            tmpView.numSlices = 1;

            newArgs.uArgs.v2.srcOrigin   = alignedSrcOrigin;
            newArgs.uArgs.v2.dstOrigin.x = 0;
            newArgs.uArgs.v2.dstOrigin.y = 0;
            newArgs.uArgs.v2.rectSize    = alignedSrcSize;

            /* Resolve the aligned rectangle into the temporary surface. */
            gcmONERROR(_ResolveRect(Hardware, SrcView, &tmpView, &newArgs));

            /* Restore orientation. */
            tmpSurf->orientation = orientation;

            /* Invalidate temporary surface cache, as it could be being re-used. */
            gcmONERROR(gcoSURF_NODE_Cache(&tmpSurf->node, memory, tmpSurf->size, gcvCACHE_INVALIDATE));

            /* Copy the unaligned area to the final destination. */
            newArgs.uArgs.v2.srcOrigin.x = srcOrigin->x - alignedSrcOrigin.x;
            newArgs.uArgs.v2.srcOrigin.y = flip
                                         ? (alignedSrcSize.y - (srcOrigin->y - alignedSrcOrigin.y) - rectSize->y)
                                         : (srcOrigin->y - alignedSrcOrigin.y);
            newArgs.uArgs.v2.dstOrigin   = *dstOrigin;
            newArgs.uArgs.v2.rectSize    = *rectSize;
            gcmONERROR(_SourceCopy(Hardware, &tmpView, DstView, &newArgs));
        }
    }

    if (dstSurf->paddingFormat)
    {
        gctBOOL srcHasExChannel = gcvFALSE;

        if (srcSurf->formatInfo.fmtClass == gcvFORMAT_CLASS_RGBA)
        {
            srcHasExChannel = (srcSurf->formatInfo.u.rgba.red.width   != 0 && dstSurf->formatInfo.u.rgba.red.width   == 0) ||
                              (srcSurf->formatInfo.u.rgba.green.width != 0 && dstSurf->formatInfo.u.rgba.green.width == 0) ||
                              (srcSurf->formatInfo.u.rgba.blue.width  != 0 && dstSurf->formatInfo.u.rgba.blue.width  == 0) ||
                              (srcSurf->formatInfo.u.rgba.alpha.width != 0 && dstSurf->formatInfo.u.rgba.alpha.width == 0);

            /* The source paddings are default value, is not from gcvSURF_G8R8_1_X8R8G8B8 to gcvSURF_R8_1_X8R8G8B8 and dst was full overwritten, the dst will be default value. */
            if (srcSurf->paddingFormat && !srcSurf->garbagePadded &&
                !(srcSurf->format == gcvSURF_G8R8_1_X8R8G8B8 && dstSurf->format == gcvSURF_R8_1_X8R8G8B8) &&
                dstOrigin->x == 0 && rectSize->x >= (gctINT)dstSurf->requestW &&
                dstOrigin->y == 0 && rectSize->y >= (gctINT)dstSurf->requestH)
            {
                dstSurf->garbagePadded = gcvFALSE;
            }

            /* The source has extra channel:
            ** 1, Source is not paddingformat,the dst may be garbage value.
            ** 2, source is paddingformat but not default value, the dst will be garbage value.
            */
            if (srcHasExChannel && (!srcSurf->paddingFormat || srcSurf->garbagePadded))
            {
                dstSurf->garbagePadded = gcvTRUE;
            }
        }
        else
        {
            dstSurf->garbagePadded = gcvTRUE;
        }
    }

OnError:
    if (Hardware != gcvNULL)
    {
#if gcdENABLE_3D
        /* Resume tile status if it was paused. */
        if (switchedTileStatus)
        {
            if (saved)
            {
                if (saved->tileStatusNode.pool != gcvPOOL_UNKNOWN)
                {
                    gcmGETHARDWAREADDRESS(saved->tileStatusNode, tileStatusAddress);
                }
                else
                {
                    tileStatusAddress = 0;
                }

                /* Reprogram the tile status to the current surface. */
                gcmVERIFY_OK(gcoHARDWARE_EnableTileStatus(Hardware, saved,
                                                          tileStatusAddress,
                                                          &saved->hzTileStatusNode,
                                                          0));
            }
            else
            {
                gcmVERIFY_OK(
                    gcoHARDWARE_DisableHardwareTileStatus(Hardware,
                                                          (srcSurf->type == gcvSURF_DEPTH) ?
                                                                        gcvTILESTATUS_DEPTH : gcvTILESTATUS_COLOR,
                                                          0));
            }


        }
        else if (pausedTileStatus)
        {
            gcoHARDWARE_PauseTileStatus(Hardware,gcvTILE_STATUS_RESUME);
        }
#endif

        /* Unlock. */
        if (locked)
        {
            /* Unlock the temporary surface. */
            gcmVERIFY_OK(gcoHARDWARE_Unlock(&tmpSurf->node, tmpSurf->type));
        }

        if (srcLocked)
        {
            /* Unlock the source. */
            gcmVERIFY_OK(gcoHARDWARE_Unlock(
                &srcSurf->node, srcSurf->type
                ));
        }

        if (dstLocked)
        {
            /* Unlock the source. */
            gcmVERIFY_OK(gcoHARDWARE_Unlock(
                &dstSurf->node, dstSurf->type
                ));
        }
    }

    /* Return result. */
    gcmFOOTER();
    return status;
}


gceSTATUS
gcoHARDWARE_IsHWResolveable(
    IN gcoSURF SrcSurf,
    IN gcoSURF DstSurf,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DstOrigin,
    IN gcsPOINT_PTR RectSize
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 srcFormat, dstFormat;
    gctBOOL flipRB;
    gcsPOINT alignedSrcOrigin, alignedSrcSize;
    gcsPOINT alignedDstOrigin, alignedDstSize;
    gctBOOL originsMatch;
    gcoHARDWARE hardware = gcvNULL;

    gcmHEADER_ARG("SrcSurf=0x%x DstSurf=0x%x "
                    "SrcOrigin=0x%x DstOrigin=0x%x RectSize=0x%x",
                    SrcSurf, DstSurf,
                    SrcOrigin, DstOrigin, RectSize);

    gcmGETHARDWARE(hardware);


    /* check if the format is supported by hw. */
    if (gcmIS_ERROR(_ConvertResolveFormat(hardware,
                              SrcSurf->format,
                              DstSurf->format,
                              &srcFormat,
                              &dstFormat,
                              &flipRB,
                              gcvNULL)))
    {
        gcmFOOTER();
        return gcvSTATUS_FALSE;
    }

    status = gcvSTATUS_TRUE;

    if ((SrcSurf->format == gcvSURF_YV12) ||
             (SrcSurf->format == gcvSURF_I420) ||
             (SrcSurf->format == gcvSURF_NV12) ||
             (SrcSurf->format == gcvSURF_NV21))
    {
        status = gcvSTATUS_FALSE;
    }
    else if (
            /* Upsampling. */
               (SrcSurf->sampleInfo.x < DstSurf->sampleInfo.x)
            || (SrcSurf->sampleInfo.y < DstSurf->sampleInfo.y)
    )
    {
        status = gcvSTATUS_FALSE;
    }
    else
    {
        _AlignResolveRect(
            hardware, SrcSurf, SrcOrigin, RectSize, &alignedSrcOrigin, &alignedSrcSize
            );

        _AlignResolveRect(
            hardware, DstSurf, DstOrigin, RectSize, &alignedDstOrigin, &alignedDstSize
            );

        /* Use the maximum rectangle. */

        originsMatch
            =  (alignedSrcOrigin.x  == SrcOrigin->x)
            && (alignedSrcOrigin.y  == SrcOrigin->y)
            && (alignedDstOrigin.x == DstOrigin->x)
            && (alignedDstOrigin.y == DstOrigin->y);

        if (!originsMatch)
        {
            status = gcvSTATUS_FALSE;
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_PreserveRects(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_VIEW *SrcView,
    IN gcsSURF_VIEW *DstView,
    IN gcsRECT Rects[],
    IN gctUINT RectCount
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT i;
    gcoSURF savedCurrent = gcvNULL;
    gctBOOL savedDestState = gcvFALSE;
    gctBOOL restoreDestState   = gcvFALSE;
    gctBOOL switchedTileStatus = gcvFALSE;
    gctBOOL pausedTileStatus   = gcvFALSE;
    gcoSURF srcSurf = SrcView->surf;
    gcoSURF dstSurf = DstView->surf;
    gcsSURF_RESOLVE_ARGS rlvArgs = {0};

    gcmHEADER_ARG("SrcView=0x%x DstView=0x%x Rects=0x%x RectCount=%d",
                   SrcView, DstView, Rects, RectCount);

    gcmGETHARDWARE(Hardware);

    gcmVERIFY_OBJECT(srcSurf, gcvOBJ_SURF);
    gcmVERIFY_OBJECT(dstSurf, gcvOBJ_SURF);

    if (srcSurf->type == gcvSURF_DEPTH)
    {
        /* Can not handle depth for now. */
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /***********************************************************************
    ** Stage 1: Diable dest tile status
    */

    /*
     * We are to copy into dest surface. We don't need to care dest tile status
     * and set tile status to disabled state, that is because
     * * If fully copied into dest surface, the raw surface is completely clean.
     * * If partially copied, we do not need to care the not copied pixels (ie,
     *   in masked rectangle) because these pixels will be overwriten by clear.
     */
    if ((dstSurf == Hardware->PEStates->colorStates.target[0].surface) &&
        (dstSurf->type == gcvSURF_RENDER_TARGET) &&
        (dstSurf->tileStatusDisabled == gcvFALSE))
    {
        /*
         * Dest is current color buffer and tile status is now enabled.
         * Need flush and turn off dest tile status.
         */
        gcmONERROR(gcoHARDWARE_DisableTileStatus(Hardware, dstSurf, gcvFALSE));
    }

    /* Save dest tileStatusDisable state. */
    savedDestState = dstSurf->tileStatusDisabled;

    /* Set dest tile status as disabled. */
    dstSurf->tileStatusDisabled = gcvTRUE;

    /* Tag dest tile status state as changed. */
    restoreDestState = gcvTRUE;

    /***********************************************************************
    ** Stage 2: Enable source tile status
    */
    if ((srcSurf != Hardware->PEStates->colorStates.target[0].surface) &&
        (srcSurf->type == gcvSURF_RENDER_TARGET))
    {
        gctUINT32 tileStatusAddress = 0;

        /* Save the current surface. */
        savedCurrent = Hardware->PEStates->colorStates.target[0].surface;

        /* Start working in the requested surface. */
        Hardware->PEStates->colorStates.target[0].surface = srcSurf;

        if (srcSurf->tileStatusNode.pool != gcvPOOL_UNKNOWN)
        {
            gcmGETHARDWAREADDRESS(srcSurf->tileStatusNode, tileStatusAddress);
        }

        /* Enable the tile status for the requested surface. */
        status = gcoHARDWARE_EnableTileStatus(
            Hardware,
            srcSurf,
            tileStatusAddress,
            &srcSurf->hzTileStatusNode,
            0);

        /* Start working in the requested surface. */
        Hardware->PEStates->colorStates.target[0].surface = savedCurrent;

        /* Tag tile status switched as true. */
        switchedTileStatus = gcvTRUE;
    }
    else if (srcSurf != Hardware->PEStates->colorStates.target[0].surface)
    {
        /* Src is not current, and source is not render target. */
        gcoHARDWARE_PauseTileStatus(Hardware, gcvTILE_STATUS_PAUSE);
        pausedTileStatus = gcvTRUE;
    }

    /***********************************************************************
    ** Stage 3: Copy rectangles
    */
    rlvArgs.version = gcvHAL_ARG_VERSION_V2;
    rlvArgs.uArgs.v2.numSlices   = 1;

    if (Hardware->features[gcvFEATURE_BLT_ENGINE])
    {
        for (i = 0; i < RectCount; i++)
        {
            rlvArgs.uArgs.v2.srcOrigin.x = Rects[i].left;
            rlvArgs.uArgs.v2.srcOrigin.y = Rects[i].top;
            rlvArgs.uArgs.v2.dstOrigin.x = Rects[i].left;
            rlvArgs.uArgs.v2.dstOrigin.y = Rects[i].top;
            rlvArgs.uArgs.v2.rectSize.x  = Rects[i].right  - Rects[i].left;
            rlvArgs.uArgs.v2.rectSize.y  = Rects[i].bottom - Rects[i].top;

            /* Go through all rectangles. */
            gcmONERROR(gcoHARDWARE_3DBlitBlt(Hardware, SrcView, DstView, &rlvArgs));
        }
    }
    else
    {
        for (i = 0; i < RectCount; i++)
        {
            rlvArgs.uArgs.v2.srcOrigin.x = Rects[i].left;
            rlvArgs.uArgs.v2.srcOrigin.y = Rects[i].top;
            rlvArgs.uArgs.v2.dstOrigin.x = Rects[i].left;
            rlvArgs.uArgs.v2.dstOrigin.y = Rects[i].top;
            rlvArgs.uArgs.v2.rectSize.x  = Rects[i].right  - Rects[i].left;
            rlvArgs.uArgs.v2.rectSize.y  = Rects[i].bottom - Rects[i].top;

            /* Go through all rectangles. */
            gcmONERROR(_ResolveRect(Hardware, SrcView, DstView, &rlvArgs));
        }
    }

    /* Success: do NOT restore dest tile status state. */
    restoreDestState = gcvFALSE;

OnError:
    if (switchedTileStatus)
    {
        if (savedCurrent)
        {
            gctUINT32 tileStatusAddress = 0;

            if (srcSurf->tileStatusNode.pool != gcvPOOL_UNKNOWN)
            {
                gcmGETHARDWAREADDRESS(srcSurf->tileStatusNode, tileStatusAddress);
            }

            /* Reprogram the tile status to the current surface. */
            gcmVERIFY_OK(
                gcoHARDWARE_EnableTileStatus(
                    Hardware,
                    savedCurrent,
                    tileStatusAddress,
                    &savedCurrent->hzTileStatusNode,
                    0));
        }
        else
        {
            /* No current tile status, turn on hardware tile status. */
            gcmVERIFY_OK(
                gcoHARDWARE_DisableHardwareTileStatus(
                    Hardware,
                    (srcSurf->type == gcvSURF_DEPTH)
                    ? gcvTILESTATUS_DEPTH : gcvTILESTATUS_COLOR,
                    0));
        }
    }
    else if (pausedTileStatus)
    {
        /* Resume tile status. */
        gcmVERIFY_OK(gcoHARDWARE_PauseTileStatus(Hardware, gcvTILE_STATUS_RESUME));
    }

    if (restoreDestState)
    {
        /* Restore dest tile status state. */
        dstSurf->tileStatusDisabled = savedDestState;
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_AddFEFence(
    IN gcoHARDWARE Hardware,
    IN gctUINT64 fenceID
)
{
    gceSTATUS status;
    gctUINT32_PTR memory;
    gcoCMDBUF reserve;
    gctSIZE_T reserveSize;

    reserveSize = gcmALIGN((2 + 4) * sizeof(gctUINT32), 8);

    gcmONERROR(gcoBUFFER_Reserve(
        Hardware->engine[gcvENGINE_RENDER].buffer, reserveSize, gcvTRUE, gcvCOMMAND_3D, &reserve
        ));

    memory = (gctUINT32_PTR) gcmUINT64_TO_PTR(reserve->lastReserve);

    memory[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x100) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)))
                                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 17:16) - (0 ? 17:16) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16)));

    memory[1] = 0;

    memory += 2;

    memory[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x01FA) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

    memory[1] = (gctUINT32)fenceID;

    memory[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x01FD) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

    memory[3] = (gctUINT32)(fenceID >> 32);

    memory += 4;

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}

#endif

/*******************************************************************************
**
**  gcoHARDWARE_LockEx
**
**  Lock video memory.
**
**  INPUT:
**
**      gcsSURF_NODE_PTR Node
**          Pointer to a gcsSURF_NODE structure that describes the video
**          memory to lock.
**
**      gceENGINE Engine
**          Engine by which node is locked, it is only used for gcvHARDWARE_3D
**          hardware.
**
**  OUTPUT:
**
**      gctUINT32 * Address
**          Physical address of the surface.
**
**      gctPOINTER * Memory
**          Logical address of the surface.
*/
gceSTATUS
gcoHARDWARE_LockEx(
    IN gcsSURF_NODE_PTR Node,
    IN gceENGINE Engine,
    OUT gctUINT32 * Address,
    OUT gctPOINTER * Memory
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gceHARDWARE_TYPE type;

    gcmHEADER_ARG("Node=0x%x Engine=0x%x, Address=0x%x Memory=0x%x",
                  Node, Engine, Address, Memory);

    gcmGETCURRENTHARDWARE(type);

    if (Node->lockCounts[type][Engine] == 0)
    {
        gcsHAL_INTERFACE iface;
        gctUINT32 handle = Node->u.normal.node;
        gctBOOL cacheable = Node->u.normal.cacheable;

        if (Node->pool == gcvPOOL_USER)
        {
            gctUINT32 physical;
            gctUINT32 baseAddress;

            gcmSAFECASTPHYSADDRT(physical, Node->u.wrapped.physical);

            if (physical != gcvINVALID_ADDRESS)
            {
                gcmVERIFY_OK(gcoOS_GetBaseAddress(gcvNULL, &baseAddress));

                physical -= baseAddress;

                gcoOS_CPUPhysicalToGPUPhysical(physical, &physical);
            }

            /*
            ** Although we want to treat user memory in the same way as video memory,
            ** user memory still has some distinguishing feature to allow some optimization.
            */
            if (handle == 0)
            {
                /*
                ** Some software access only surface has no kernel video node.
                ** Since 'physical' is only for one hardware, it can't be used by other hardware type.
                ** TODO: Remove usage of gcoSURF_WrapSurface to avoid this path.
                */
                gcmASSERT(Node->logical != gcvNULL);
                gcsSURF_NODE_SetHardwareAddress(Node, physical + (gctUINT32) Node->bufferOffset);
            }
            else
            if (gcoHARDWARE_IsFlatMapped(gcvNULL, physical))
            {
                /*
                ** If physical address is in flat mapping range of current hardware,
                ** use physical address as hardware address instead of lock in kernel.
                */
                gcsSURF_NODE_SetHardwareAddress(Node, physical + (gctUINT32) Node->bufferOffset);

                handle = 0;

                /* TODO: Use reference count of each type as valid flag. */
                Node->valid = gcvTRUE;
            }

            Node->u.wrapped.lockedInKernel[type] = handle ? gcvTRUE : gcvFALSE;

            cacheable = gcvFALSE;
        }

        if (handle)
        {
            /* Fill in the kernel call structure. */
            iface.engine = Engine;
            iface.command = gcvHAL_LOCK_VIDEO_MEMORY;
            iface.u.LockVideoMemory.node = handle;
            iface.u.LockVideoMemory.cacheable = cacheable;

            gcmONERROR(gcoHAL_Call(gcvNULL, &iface));

            /* Validate the node. */
            Node->valid = gcvTRUE;

            /* VIV: This flag is needed by bg2ct specific patch, if it is not set
            ** all cache function is skipped. */
            if (Node->pool != gcvPOOL_USER)
            {
                Node->lockedInKernel = gcvTRUE;
            }

            /* Store hardware address. */
            gcsSURF_NODE_SetHardwareAddress(Node, iface.u.LockVideoMemory.address + (gctUINT32) Node->bufferOffset);

            /* Store logical address. */
            Node->logical = gcmUINT64_TO_PTR(iface.u.LockVideoMemory.memory + Node->bufferOffset);
        }
    }

    if ((Node->lockCounts[type][Engine] == 0)
     && (type == gcvHARDWARE_2D || type == gcvHARDWARE_3D2D)
    )
    {
        gctUINT32 address;

        gcmGETHARDWAREADDRESS(*Node, address);
        gcmDUMP_ADD_MEMORY_INFO(address, Node->logical, gcvINVALID_ADDRESS, Node->size);
    }

    /* Increment the lock count per hardware type. */
    Node->lockCounts[type][Engine]++;

    /* Set the result. */
    if (Address != gcvNULL)
    {
        gcsSURF_NODE_GetHardwareAddress(Node, Address, gcvNULL, gcvNULL, gcvNULL);
    }

    if (Memory != gcvNULL)
    {
        *Memory = Node->logical;
    }

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_UnlockEx
**
**  Unlock video memory.
**
**  INPUT:
**
**      gcsSURF_NODE_PTR Node
**          Pointer to a gcsSURF_NODE structure that describes the video
**          memory to unlock.
**
**      gceENGINE Engine
**          Engine by which node is locked, it is only used for gcvHARDWARE_3D
**          hardware.
**
**      gceSURF_TYPE Type
**          Type of surface to unlock.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_UnlockEx(
    IN gcsSURF_NODE_PTR Node,
    IN gceENGINE Engine,
    IN gceSURF_TYPE Type
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsHAL_INTERFACE iface;
    gceHARDWARE_TYPE type;
#if gcdDUMP || gcdDUMP_2D
    gctUINT32 address = 0;
#endif

    gcmHEADER_ARG("Node=0x%x Type=%d", Node, Type);

    gcmGETCURRENTHARDWARE(type);

    /* Verify whether the node is valid. */
    if (Node->lockCounts[type][Engine] <= 0)
    {
        gcmTRACE_ZONE(
            gcvLEVEL_WARNING, gcvZONE_SURFACE,
            "gcoHARDWARE_Unlock: Node=0x%x; unlock called on an unlocked surface.",
            Node
            );
    }

    /* Locked more then once? */
    else
    {
        Node->lockCounts[type][Engine]--;

        if (Node->lockCounts[type][Engine] == 0)
        {
            gctUINT32 handle = Node->u.normal.node;

            if ((Node->pool == gcvPOOL_USER)
              && (Node->u.wrapped.lockedInKernel[type] == gcvFALSE)
            )
            {
                handle = 0;
            }

#if gcdDUMP_2D
            gcsSURF_NODE_GetHardwareAddress(Node, &address, gcvNULL, gcvNULL, gcvNULL);

            /* Delete the memory info. */
            gcmDUMP_DEL_MEMORY_INFO(address);
#endif

            if (handle)
            {
#if gcdDUMP
                gcsSURF_NODE_GetHardwareAddress(Node, &address, gcvNULL, gcvNULL, gcvNULL);

                gcmDUMP(gcvNULL, "#[unlock 0x%08x]", address);
#endif

                /* Unlock the video memory node. */
                iface.engine = Engine;
                iface.command = gcvHAL_UNLOCK_VIDEO_MEMORY;
                iface.u.UnlockVideoMemory.node = handle;
                iface.u.UnlockVideoMemory.type = Type & ~gcvSURF_NO_VIDMEM;

                /* Call the kernel. */
                gcmONERROR(gcoHAL_Call(gcvNULL, &iface));

                /* Schedule an event for the unlock. */
                gcmONERROR(gcoHARDWARE_CallEvent(gcvNULL, &iface));
            }
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_Lock
**
**  Lock video memory.
**
**  INPUT:
**
**      gcsSURF_NODE_PTR Node
**          Pointer to a gcsSURF_NODE structure that describes the video
**          memory to lock.
**
**  OUTPUT:
**
**      gctUINT32 * Address
**          Physical address of the surface.
**
**      gctPOINTER * Memory
**          Logical address of the surface.
*/
gceSTATUS
gcoHARDWARE_Lock(
    IN gcsSURF_NODE_PTR Node,
    OUT gctUINT32 * Address,
    OUT gctPOINTER * Memory
    )
{
    return gcoHARDWARE_LockEx(Node, gcvENGINE_RENDER, Address, Memory);
}

/*******************************************************************************
**
**  gcoHARDWARE_Unlock
**
**  Unlock video memory.
**
**  INPUT:
**
**      gcsSURF_NODE_PTR Node
**          Pointer to a gcsSURF_NODE structure that describes the video
**          memory to unlock.
**
**      gceSURF_TYPE Type
**          Type of surface to unlock.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_Unlock(
    IN gcsSURF_NODE_PTR Node,
    IN gceSURF_TYPE Type
    )
{
    return gcoHARDWARE_UnlockEx(Node, gcvENGINE_RENDER, Type);
}

/*******************************************************************************
**
**  gcoHARDWARE_CallEvent
**
**  Send an event to the kernel and append the required synchronization code to
**  the command buffer..
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsHAL_INTERFACE * Interface
**          Pointer to an gcsHAL_INTERFACE structure the defines the event to
**          send.
**
**  OUTPUT:
**
**      gcsHAL_INTERFACE * Interface
**          Pointer to an gcsHAL_INTERFACE structure the received information
**          from the kernel.
*/
gceSTATUS
gcoHARDWARE_CallEvent(
    IN gcoHARDWARE Hardware,
    IN OUT gcsHAL_INTERFACE * Interface
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if (gcdENABLE_3D || gcdENABLE_2D)
    gcoQUEUE queue = gcvNULL;
    gceENGINE engine = gcvENGINE_RENDER;

    gcmHEADER_ARG("Hardware=0x%x Interface=0x%x", Hardware, Interface);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmDEBUG_VERIFY_ARGUMENT(Interface != gcvNULL);

    if (Interface->command == gcvHAL_UNLOCK_VIDEO_MEMORY)
    {
        engine = Interface->engine;
    }

    /* Get the queue */
    queue = Hardware->engine[engine].queue;

    /* Append event to queue */
    gcmONERROR(gcoQUEUE_AppendEvent(queue, Interface));

#if gcdIN_QUEUE_RECORD_LIMIT
    if (Hardware->engine[engine].queue->recordCount >= gcdIN_QUEUE_RECORD_LIMIT)
    {
        gcmONERROR(gcoHARDWARE_Commit(Hardware));
    }
#endif

OnError:
    /* Return status. */
    gcmFOOTER();
#endif  /* (gcdENABLE_3D || gcdENABLE_2D) */
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_ScheduleVideoMemory
**
**  Schedule destruction for the specified video memory node.
**
**  INPUT:
**
**      gcsSURF_NODE_PTR Node
**          Pointer to the video momory node to be destroyed.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_ScheduleVideoMemory(
    IN gcsSURF_NODE_PTR Node
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsHAL_INTERFACE iface;

    gcmHEADER_ARG("Node=0x%x", Node);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_HARDWARE,
                  "node=0x%08x",
                  Node->u.normal.node);

    /* Release the allocated video memory asynchronously. */
    iface.command = gcvHAL_RELEASE_VIDEO_MEMORY;
    iface.u.ReleaseVideoMemory.node = Node->u.normal.node;

    /* Call kernel HAL. */
    gcmONERROR(gcoHAL_Call(gcvNULL, &iface));

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_AllocTmpSurface
**
**  Allocates a temporary surface with specified parameters.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to a gcoHARDWARE object.
**
**      gctUINT Width, Height
**          The aligned size of the surface to be allocated.
**
**      gcsSURF_FORMAT_INFO_PTR Format
**          The format of the surface to be allocated.
**
**      gceSURF_TYPE Type
**          The type of the surface to be allocated.
**
**      gceSURF_FLAG Flags
**          The flags of dest surface info.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_AllocTmpSurface(
    IN gcoHARDWARE Hardware,
    IN gctUINT Width,
    IN gctUINT Height,
    IN gcsSURF_FORMAT_INFO_PTR FormatInfo,
    IN gceSURF_TYPE Type,
    IN gceSURF_TYPE Hints
    )
{
    gceSTATUS status;
    gctBOOL superTiled = gcvFALSE;
    gceSURF_ALIGNMENT hAlignment = gcvSURF_FOUR;
    gcoSURF tmpSurf;

    gcmHEADER_ARG("Hardware=0x%x Width=%d Height=%d "
                  "FormatInfo=0x%x Type=%d",
                  Hardware, Width, Height, *FormatInfo, Type);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    tmpSurf = &Hardware->tmpSurf;
    /* Do we have a compatible surface? */
    if ((tmpSurf->type == Type) &&
        (tmpSurf->format == FormatInfo->format) &&
        (tmpSurf->requestW == Width) &&
        (tmpSurf->requestH == Height))
    {
        status = gcvSTATUS_OK;
    }
    else
    {
        gctUINT32 size;

        /* Delete existing buffer. */
        gcmONERROR(gcoHARDWARE_FreeTmpSurface(Hardware, gcvTRUE));

        Hardware->tmpSurf.object.type = gcvOBJ_SURF;
        tmpSurf->requestW = Width;
        tmpSurf->requestH = Height;
        tmpSurf->requestD = 1;
        tmpSurf->allocedW = Width;
        tmpSurf->allocedH = Height;
        tmpSurf->alignedW = Width;
        tmpSurf->alignedH = Height;
        tmpSurf->cacheMode = gcvCACHE_NONE;
        tmpSurf->colorSpace = (FormatInfo->fmtDataType == gcvFORMAT_DATATYPE_SRGB) ?
                                gcvSURF_COLOR_SPACE_NONLINEAR : gcvSURF_COLOR_SPACE_LINEAR;

        /* Align the width and height. */
        gcmONERROR(gcoHARDWARE_AlignToTileCompatible(
            Hardware,
            Type,
            0,
            FormatInfo->format,
            &tmpSurf->alignedW,
            &tmpSurf->alignedH,
            1,
            &tmpSurf->tiling,
            &superTiled,
            &hAlignment));

        size = tmpSurf->alignedW
             * FormatInfo->bitsPerPixel / 8
             * tmpSurf->alignedH;

        gcmONERROR(gcsSURF_NODE_Construct(
            &tmpSurf->node,
            size,
            64,
            Type,
            Hints & gcvSURF_PROTECTED_CONTENT
            ? gcvALLOC_FLAG_SECURITY
            : gcvALLOC_FLAG_NONE,
            gcvPOOL_DEFAULT
            ));

        /* Set the new parameters. */
        tmpSurf->type       = Type;
        tmpSurf->format     = FormatInfo->format;
        tmpSurf->formatInfo = *FormatInfo;
        tmpSurf->bitsPerPixel = FormatInfo->bitsPerPixel;
        tmpSurf->stride     = tmpSurf->alignedW * FormatInfo->bitsPerPixel / 8;
        tmpSurf->sliceSize  =
        tmpSurf->layerSize  =
        tmpSurf->size       = size;

#if gcdENABLE_3D
        tmpSurf->sampleInfo = g_sampleInfos[1];
        tmpSurf->isMsaa       = gcvFALSE;
        tmpSurf->vMsaa       = gcvFALSE;
        tmpSurf->superTiled = superTiled;
        tmpSurf->hAlignment = hAlignment;
        tmpSurf->colorSpace = gcd_QUERY_COLOR_SPACE(FormatInfo->format);
#endif /* gcdENABLE_3D */

        tmpSurf->pfGetAddr = gcoHARDWARE_GetProcCalcPixelAddr(Hardware, tmpSurf);
    }

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_FreeTmpSurface
**
**  Free the temporary surface.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to a gcoHARDWARE object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_FreeTmpSurface(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Synchronized
    )
{
    gcoSURF tmpSurf;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Synchronized=%d", Hardware, Synchronized);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    tmpSurf = &Hardware->tmpSurf;
    /* Is there a surface allocated? */
    if (tmpSurf->node.pool != gcvPOOL_UNKNOWN)
    {
        gcmVERIFY_OK(gcsSURF_NODE_Destroy(&tmpSurf->node));

        /* Reset the temporary surface. */
        gcoOS_ZeroMemory(&Hardware->tmpSurf, sizeof(Hardware->tmpSurf));
    }

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  gcoHARDWARE_Alloc2DSurface
**
**  Allocates a 2D surface with specified parameters.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to a gcoHARDWARE object.
**
**      gctUINT Width, Height
**          The aligned size of the surface to be allocated.
**
**      gceSURF_FORMAT Format
**          The format of the surface to be allocated.
**
**      gceSURF_FLAG Flags
**          The flags of dest surface info.
**  OUTPUT:
**
**      gcoSURF *Surface
*/
gceSTATUS
gcoHARDWARE_Alloc2DSurface(
    IN gcoHARDWARE Hardware,
    IN gctUINT Width,
    IN gctUINT Height,
    IN gceSURF_FORMAT Format,
    IN gceSURF_TYPE Hints,
    OUT gcoSURF *Surface
    )
{
    gceSTATUS status;
    gcoSURF surf = gcvNULL;
    gcsSURF_FORMAT_INFO_PTR formatInfo[2];
    gctUINT alignedWidth, alignedHeight, size;

    gcmHEADER_ARG("Width=%d Height=%d Format=%d",
                    Width, Height, Format);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Surface != gcvNULL);

    alignedWidth  = Width;
    alignedHeight = Height;

    /* Align the width and height. */
    gcmONERROR(gcoHARDWARE_AlignToTile(
        Hardware, gcvSURF_BITMAP, 0, Format, &alignedWidth, &alignedHeight, 1, gcvNULL, gcvNULL, gcvNULL
        ));

    gcmONERROR(gcoSURF_QueryFormat(Format, formatInfo));

    size = alignedWidth * formatInfo[0]->bitsPerPixel  / 8 * alignedHeight;

    gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof(struct _gcoSURF), (gctPOINTER *)&surf));
    gcoOS_ZeroMemory(surf, sizeof(struct _gcoSURF));

    gcmONERROR(gcsSURF_NODE_Construct(
        &surf->node,
        size,
        64,
        gcvSURF_BITMAP,
        Hints & gcvSURF_PROTECTED_CONTENT
        ? gcvALLOC_FLAG_SECURITY
        : gcvALLOC_FLAG_NONE,
        gcvPOOL_DEFAULT
        ));

    /* Set the new parameters. */
    surf->type         = gcvSURF_BITMAP;
    surf->format       = Format;
    surf->requestW     = Width;
    surf->requestH     = Height;
    surf->requestD     = 1;
    surf->allocedW     = Width;
    surf->allocedH     = Height;
    surf->alignedW     = alignedWidth;
    surf->alignedH     = alignedHeight;
    surf->bitsPerPixel = formatInfo[0]->bitsPerPixel;
    surf->stride       = alignedWidth * surf->bitsPerPixel / 8;
    surf->size         = size;;

#if gcdENABLE_3D
    surf->sampleInfo   = g_sampleInfos[1];
    surf->isMsaa       = gcvFALSE;
    surf->vMsaa       = gcvFALSE;
#endif

    surf->rotation     = gcvSURF_0_DEGREE;
    surf->orientation  = gcvORIENTATION_TOP_BOTTOM;

    surf->tiling       = gcvLINEAR;

    gcmONERROR(gcoHARDWARE_Lock(&surf->node, gcvNULL, gcvNULL));

    *Surface = surf;

OnError:
    if (gcmIS_ERROR(status) && (surf != gcvNULL))
    {
        if (surf->node.valid)
        {
            gcmVERIFY_OK(gcoHARDWARE_Unlock(
                &surf->node,
                gcvSURF_BITMAP
                ));
        }

        if (surf->node.u.normal.node != 0)
        {
            gcmVERIFY_OK(gcsSURF_NODE_Destroy(&surf->node));
        }

        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, surf));
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_Free2DSurface
**
**  Free the 2D surface.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to a gcoHARDWARE object.
**
**      gcoSURF Surface
**          Pointer to the surface to be free'd.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_Free2DSurface(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Surface->node.valid)
    {
        gcmONERROR(gcoHARDWARE_Unlock(
            &Surface->node,
            gcvSURF_BITMAP
            ));
    }

    /* Schedule deletion. */
    gcmONERROR(gcsSURF_NODE_Destroy(&Surface->node));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Surface));

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_Get2DTempSurface
**
**  Allocates a temporary surface with specified parameters.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT Width, Height
**          The aligned size of the surface to be allocated.
**
**      gceSURF_FORMAT Format
**          The format of the surface to be allocated.
**
**      gceSURF_FLAG Flags
**          The flags of dest surface info.
**  OUTPUT:
**
**      gcoSURF *Surface
*/
gceSTATUS
gcoHARDWARE_Get2DTempSurface(
    IN gcoHARDWARE Hardware,
    IN gctUINT Width,
    IN gctUINT Height,
    IN gceSURF_FORMAT Format,
    IN gceSURF_TYPE Hints,
    OUT gcoSURF *Surface
    )
{
    gceSTATUS status;
    gcsSURF_FORMAT_INFO_PTR formatInfo;
    gctUINT alignedWidth, alignedHeight, size;
    gctSIZE_T delta = 0;
    gctINT  i, idx = -1;
    gceSURF_TYPE dstSurfHints = Hints;

    gcmHEADER_ARG("Hardware=0x%x Width=%d Height=%d "
                    "Format=%d",
                    Hardware, Width, Height,
                    Format);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    alignedWidth = Width;
    alignedHeight = Height;

    /* Align the width and height. */
    gcmONERROR(gcoHARDWARE_AlignToTile(
        Hardware, gcvSURF_BITMAP, 0, Format, &alignedWidth, &alignedHeight, 1, gcvNULL, gcvNULL, gcvNULL
        ));

    gcmONERROR(gcoSURF_QueryFormat(Format, &formatInfo));

    size = alignedWidth * formatInfo->bitsPerPixel  / 8 * alignedHeight;

    /* Do we have a fit surface? */
    for (i = 0; i < gcdTEMP_SURFACE_NUMBER; i += 1)
    {
        if ((Hardware->temp2DSurf[i] != gcvNULL)
            && (Hardware->temp2DSurf[i]->node.size >= size)
            && ((Hardware->temp2DSurf[i]->hints & gcvSURF_PROTECTED_CONTENT)
                 == (dstSurfHints & gcvSURF_PROTECTED_CONTENT)))
        {
            if (idx == -1)
            {
                delta = Hardware->temp2DSurf[i]->node.size - size;
                idx = i;
            }
            else if (Hardware->temp2DSurf[i]->node.size - size < delta)
            {
                delta = Hardware->temp2DSurf[i]->node.size - size;
                idx = i;
            }
        }
    }

    if (idx != -1)
    {
        gcmASSERT((idx >= 0) && (idx < gcdTEMP_SURFACE_NUMBER));

        *Surface = Hardware->temp2DSurf[idx];
        Hardware->temp2DSurf[idx] = gcvNULL;

        (*Surface)->format          = Format;
        (*Surface)->alignedW        = alignedWidth;
        (*Surface)->alignedH        = alignedHeight;
        (*Surface)->bitsPerPixel    = formatInfo->bitsPerPixel;
        (*Surface)->stride          = alignedWidth * formatInfo->bitsPerPixel / 8;
        (*Surface)->rotation        = gcvSURF_0_DEGREE;
        (*Surface)->orientation     = gcvORIENTATION_TOP_BOTTOM;
        (*Surface)->tiling          = gcvLINEAR;

        (*Surface)->requestW        = Width;
        (*Surface)->requestH        = Height;
        (*Surface)->requestD        = 1;
        (*Surface)->allocedW        = Width;
        (*Surface)->allocedH        = Height;
    }
    else
    {
        gcmONERROR(gcoHARDWARE_Alloc2DSurface(Hardware, Width, Height, Format, dstSurfHints, Surface));
    }

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_Put2DTempSurface
**
**  Put back the temporary surface.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcoSURF Surface
**          Pointer to the surface to be free'd.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_Put2DTempSurface(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoSURF surf = Surface;
    gctINT i;

    gcmHEADER_ARG("Hardware=0x%x Surface=0x%x", Hardware, Surface);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    for (i = 0; i < gcdTEMP_SURFACE_NUMBER; i++)
    {
        /* Is there an empty slot? */
        if (Hardware->temp2DSurf[i] == gcvNULL)
        {
            Hardware->temp2DSurf[i] = surf;
            break;
        }

        /* Swap the smaller one. */
        else if (Hardware->temp2DSurf[i]->node.size < surf->node.size)
        {
            gcoSURF temp = surf;

            surf = Hardware->temp2DSurf[i];
            Hardware->temp2DSurf[i] = temp;
        }
    }

    if (i == gcdTEMP_SURFACE_NUMBER)
    {
        gcmONERROR(gcoHARDWARE_Free2DSurface(Hardware, surf));
    }

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_ConvertPixel
**
**  Convert source pixel from source format to target pixel' target format.
**  The source format class should be either identical or convertible to
**  the target format class.
**
**  INPUT:
**
**      gctPOINTER SrcPixel, DstPixel,
**          Pointers to source and target pixels.
**
**      gctUINT SrcBitOffset, DstBitOffset
**          Bit offsets of the source and target pixels relative to their
**          respective pointers.
**
**      gcsSURF_FORMAT_INFO_PTR SrcFormat, DstFormat
**          Pointers to source and target pixel format descriptors.
**
**      gcsBOUNDARY_PTR SrcBoundary, DstBoundary
**          Pointers to optional boundary structures to verify the source
**          and target position. If the source is found to be beyond the
**          defined boundary, it will be assumed to be 0. If the target
**          is found to be beyond the defined boundary, the write will
**          be ignored. If boundary checking is not needed, gcvNULL can be
**          passed.
**
**  OUTPUT:
**
**      gctPOINTER DstPixel + DstBitOffset
**          Converted pixel.
*/
#if (gcdENABLE_3D || gcdENABLE_2D)
gceSTATUS
gcoHARDWARE_ConvertPixel(
    IN gctPOINTER SrcPixel,
    OUT gctPOINTER DstPixel,
    IN gctUINT SrcBitOffset,
    IN gctUINT DstBitOffset,
    IN gcsSURF_FORMAT_INFO_PTR SrcFormat,
    IN gcsSURF_FORMAT_INFO_PTR DstFormat,
    IN OPTIONAL gcsBOUNDARY_PTR SrcBoundary,
    IN OPTIONAL gcsBOUNDARY_PTR DstBoundary,
    IN gctBOOL SrcPixelOdd,
    IN gctBOOL DstPixelOdd
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcuPIXEL_FORMAT_CLASS srcFmtClass, dstFmtClass;

    gcmHEADER_ARG("SrcPixel=0x%x DstPixel=0x%x "
                    "SrcBitOffset=%d DstBitOffset=%d SrcFormat=0x%x "
                    "DstFormat=0x%x SrcBoundary=0x%x DstBoundary=0x%x",
                    SrcPixel, DstPixel,
                    SrcBitOffset, DstBitOffset, SrcFormat,
                    DstFormat, SrcBoundary, DstBoundary);

    if (SrcFormat->interleaved && SrcPixelOdd)
    {
        srcFmtClass = SrcFormat->uOdd;
    }
    else
    {
        srcFmtClass = SrcFormat->u;
    }

    if (DstFormat->interleaved && DstPixelOdd)
    {
        dstFmtClass = DstFormat->uOdd;
    }
    else
    {
        dstFmtClass = DstFormat->u;
    }

    if (SrcFormat->fmtClass == gcvFORMAT_CLASS_RGBA)
    {
        if (DstFormat->fmtClass == gcvFORMAT_CLASS_RGBA)
        {
            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &SrcFormat->u.rgba.alpha,
                &DstFormat->u.rgba.alpha,
                SrcBoundary, DstBoundary,
                ~0U
                ));

            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &SrcFormat->u.rgba.red,
                &DstFormat->u.rgba.red,
                SrcBoundary, DstBoundary,
                0
                ));

            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &SrcFormat->u.rgba.green,
                &DstFormat->u.rgba.green,
                SrcBoundary, DstBoundary,
                0
                ));

            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &SrcFormat->u.rgba.blue,
                &DstFormat->u.rgba.blue,
                SrcBoundary, DstBoundary,
                0
                ));
        }

        else if (DstFormat->fmtClass == gcvFORMAT_CLASS_YUV)
        {
            gctUINT8 r[4] = {0};
            gctUINT8 g[4] = {0};
            gctUINT8 b[4] = {0};
            gctUINT8 y[4] = {0};
            gctUINT8 u[4] = {0};
            gctUINT8 v[4] = {0};

            /*
                Get RGB value.
            */

            gcmONERROR(_ConvertComponent(
                SrcPixel, r, SrcBitOffset, 0,
                &srcFmtClass.rgba.red, &gcvPIXEL_COMP_XXX8,
                SrcBoundary, gcvNULL, 0
                ));

            gcmONERROR(_ConvertComponent(
                SrcPixel, g, SrcBitOffset, 0,
                &srcFmtClass.rgba.green, &gcvPIXEL_COMP_XXX8,
                SrcBoundary, gcvNULL, 0
                ));

            gcmONERROR(_ConvertComponent(
                SrcPixel, b, SrcBitOffset, 0,
                &srcFmtClass.rgba.blue, &gcvPIXEL_COMP_XXX8,
                SrcBoundary, gcvNULL, 0
                ));

            /*
                Convert to YUV.
            */

            gcoHARDWARE_RGB2YUV(
                 r[0], g[0], b[0],
                 y, u, v
                );

            /*
                Average Us and Vs for odd pixels.
            */
            if ((DstFormat->interleaved & DstPixelOdd) != 0)
            {
                gctUINT8 curU[4] = {0}, curV[4] = {0};

                gcmONERROR(_ConvertComponent(
                    DstPixel, curU, DstBitOffset, 0,
                    &dstFmtClass.yuv.u, &gcvPIXEL_COMP_XXX8,
                    DstBoundary, gcvNULL, 0
                    ));

                gcmONERROR(_ConvertComponent(
                    DstPixel, curV, DstBitOffset, 0,
                    &dstFmtClass.yuv.v, &gcvPIXEL_COMP_XXX8,
                    DstBoundary, gcvNULL, 0
                    ));


                u[0] = (gctUINT8) (((gctUINT16) u[0] + (gctUINT16) curU[0]) >> 1);
                v[0] = (gctUINT8) (((gctUINT16) v[0] + (gctUINT16) curV[0]) >> 1);
            }

            /*
                Convert to the final format.
            */

            gcmONERROR(_ConvertComponent(
                y, DstPixel, 0, DstBitOffset,
                &gcvPIXEL_COMP_XXX8, &dstFmtClass.yuv.y,
                gcvNULL, DstBoundary, 0
                ));

            gcmONERROR(_ConvertComponent(
                u, DstPixel, 0, DstBitOffset,
                &gcvPIXEL_COMP_XXX8, &dstFmtClass.yuv.u,
                gcvNULL, DstBoundary, 0
                ));

            gcmONERROR(_ConvertComponent(
                v, DstPixel, 0, DstBitOffset,
                &gcvPIXEL_COMP_XXX8, &dstFmtClass.yuv.v,
                gcvNULL, DstBoundary, 0
                ));
        }

        else if (DstFormat->fmtClass == gcvFORMAT_CLASS_LUMINANCE)
        {
            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &SrcFormat->u.rgba.red,
                &DstFormat->u.lum.value,
                SrcBoundary, DstBoundary,
                0
                ));

            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &SrcFormat->u.rgba.alpha,
                &DstFormat->u.lum.alpha,
                SrcBoundary, DstBoundary,
                ~0U
                ));
        }

        else
        {
            /* Not supported combination. */
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }

    else if (SrcFormat->fmtClass == gcvFORMAT_CLASS_DEPTH)
    {
        if (DstFormat->fmtClass == gcvFORMAT_CLASS_DEPTH)
        {
            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &SrcFormat->u.depth.depth,
                &DstFormat->u.depth.depth,
                SrcBoundary, DstBoundary,
                ~0U
                ));

            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &SrcFormat->u.depth.stencil,
                &DstFormat->u.depth.stencil,
                SrcBoundary, DstBoundary,
                0
                ));
        }

        else
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }

    else if (SrcFormat->fmtClass == gcvFORMAT_CLASS_YUV)
    {
        if (DstFormat->fmtClass == gcvFORMAT_CLASS_YUV)
        {
            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &srcFmtClass.yuv.y,
                &dstFmtClass.yuv.y,
                SrcBoundary, DstBoundary,
                0
                ));

            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &srcFmtClass.yuv.u,
                &dstFmtClass.yuv.u,
                SrcBoundary, DstBoundary,
                0
                ));

            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &srcFmtClass.yuv.v,
                &dstFmtClass.yuv.v,
                SrcBoundary, DstBoundary,
                0
                ));
        }

        else if (DstFormat->fmtClass == gcvFORMAT_CLASS_RGBA)
        {
            gctUINT8 y[4]={0}, u[4]={0}, v[4]={0};
            gctUINT8 r[4], g[4], b[4];

            /*
                Get YUV value.
            */

            gcmONERROR(_ConvertComponent(
                SrcPixel, y, SrcBitOffset, 0,
                &srcFmtClass.yuv.y, &gcvPIXEL_COMP_XXX8,
                SrcBoundary, gcvNULL, 0
                ));

            gcmONERROR(_ConvertComponent(
                SrcPixel, u, SrcBitOffset, 0,
                &srcFmtClass.yuv.u, &gcvPIXEL_COMP_XXX8,
                SrcBoundary, gcvNULL, 0
                ));

            gcmONERROR(_ConvertComponent(
                SrcPixel, v, SrcBitOffset, 0,
                &srcFmtClass.yuv.v, &gcvPIXEL_COMP_XXX8,
                SrcBoundary, gcvNULL, 0
                ));

            /*
                Convert to RGB.
            */

            gcoHARDWARE_YUV2RGB(
                 y[0], u[0], v[0],
                 r, g, b
                );

            /*
                Convert to the final format.
            */

            gcmONERROR(_ConvertComponent(
                gcvNULL, DstPixel, 0, DstBitOffset,
                gcvNULL, &dstFmtClass.rgba.alpha,
                gcvNULL, DstBoundary, ~0U
                ));

            gcmONERROR(_ConvertComponent(
                r, DstPixel, 0, DstBitOffset,
                &gcvPIXEL_COMP_XXX8, &dstFmtClass.rgba.red,
                gcvNULL, DstBoundary, 0
                ));

            gcmONERROR(_ConvertComponent(
                g, DstPixel, 0, DstBitOffset,
                &gcvPIXEL_COMP_XXX8, &dstFmtClass.rgba.green,
                gcvNULL, DstBoundary, 0
                ));

            gcmONERROR(_ConvertComponent(
                b, DstPixel, 0, DstBitOffset,
                &gcvPIXEL_COMP_XXX8, &dstFmtClass.rgba.blue,
                gcvNULL, DstBoundary, 0
                ));
        }

        else
        {
            /* Not supported combination. */
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }

    else if (SrcFormat->fmtClass == gcvFORMAT_CLASS_INDEX)
    {
        if (DstFormat->fmtClass == gcvFORMAT_CLASS_INDEX)
        {
            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &SrcFormat->u.index.value,
                &DstFormat->u.index.value,
                SrcBoundary, DstBoundary,
                0
                ));
        }

        else
        {
            /* Not supported combination. */
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }

    else if (SrcFormat->fmtClass == gcvFORMAT_CLASS_LUMINANCE)
    {
        if (DstFormat->fmtClass == gcvFORMAT_CLASS_LUMINANCE)
        {
            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &SrcFormat->u.lum.alpha,
                &DstFormat->u.lum.alpha,
                SrcBoundary, DstBoundary,
                ~0U
                ));

            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &SrcFormat->u.lum.value,
                &DstFormat->u.lum.value,
                SrcBoundary, DstBoundary,
                0
                ));
        }

        else
        {
            /* Not supported combination. */
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }

    else if (SrcFormat->fmtClass == gcvFORMAT_CLASS_BUMP)
    {
        if (DstFormat->fmtClass == gcvFORMAT_CLASS_BUMP)
        {
            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &SrcFormat->u.bump.alpha,
                &DstFormat->u.bump.alpha,
                SrcBoundary, DstBoundary,
                ~0U
                ));

            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &SrcFormat->u.bump.l,
                &DstFormat->u.bump.l,
                SrcBoundary, DstBoundary,
                0
                ));

            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &SrcFormat->u.bump.v,
                &DstFormat->u.bump.v,
                SrcBoundary, DstBoundary,
                0
                ));

            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &SrcFormat->u.bump.u,
                &DstFormat->u.bump.u,
                SrcBoundary, DstBoundary,
                0
                ));

            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &SrcFormat->u.bump.q,
                &DstFormat->u.bump.q,
                SrcBoundary, DstBoundary,
                0
                ));

            gcmONERROR(_ConvertComponent(
                SrcPixel, DstPixel,
                SrcBitOffset, DstBitOffset,
                &SrcFormat->u.bump.w,
                &DstFormat->u.bump.w,
                SrcBoundary, DstBoundary,
                0
                ));
        }

        else
        {
            /* Not supported combination. */
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }

    else
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Return status. */
    gcmFOOTER();
    return gcvSTATUS_OK;

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}
#endif  /* gcdENABLE_3D/2D */

#if gcdENABLE_3D
/*******************************************************************************
**
**  gcoHARDWARE_CopyPixels
**
**  Copy a rectangular area from one surface to another with format conversion.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsSURF_VIEW *SrcView
**          Pointer to the source surface view.
**
**      gcsSURF_VIEW *DstView
**          Pointer to the destination surface view.
**
**      gcsSURF_RESOLVE_ARGS *Args
**          Pointer to the arguments.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_CopyPixels(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_VIEW *SrcView,
    IN gcsSURF_VIEW *DstView,
    IN gcsSURF_RESOLVE_ARGS *Args
    )
{
    gctUINT tmpSliceOffset = 0;
    gcsPOINT tmpOrigin = {0};
    gcoSURF tmpSurf = gcvNULL;
    gcoSURF srcSurf = SrcView->surf;
    gcoSURF dstSurf = DstView->surf;
    gcsPOINT_PTR srcOrigin = &Args->uArgs.v2.srcOrigin;
    gcsPOINT_PTR dstOrigin = &Args->uArgs.v2.dstOrigin;
    gcsPOINT_PTR rectSize = &Args->uArgs.v2.rectSize;
    gctUINT8_PTR tmpBase = gcvNULL;
    gctUINT8_PTR srcBase = gcvNULL;
    gctUINT8_PTR dstBase = gcvNULL;
    gcsSURF_FORMAT_INFO_PTR srcFmtInfo;
    gcsSURF_FORMAT_INFO_PTR dstFmtInfo;
    gcsSURF_FORMAT_INFO_PTR tmpFmtInfo;
    gctBOOL stalled = gcvFALSE;
#if gcdDUMP
    gctUINT32 address;
#endif
    gctINT x, y;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x SrcView=0x%x DstView=0x%x Args=0x%x", Hardware, SrcView, DstView, Args);

    gcmGETHARDWARE(Hardware);

    if (Args->version != gcvHAL_ARG_VERSION_V2)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Get surface formats. */
    srcFmtInfo = &srcSurf->formatInfo;
    dstFmtInfo = &dstSurf->formatInfo;

    if ((srcSurf->tileStatusDisabled && dstSurf->tileStatusDisabled) && /* Both tile status must be disabled */
        (srcSurf->sampleInfo.product == 1) &&                           /* No msaa */
        (srcFmtInfo->bitsPerPixel == dstFmtInfo->bitsPerPixel) &&       /* Same bpp */
        ((srcSurf->format == dstSurf->format) ||                        /* No format conversion except X */
         (srcSurf->format == gcvSURF_A8R8G8B8 && dstSurf->format == gcvSURF_X8R8G8B8) ||
         (srcSurf->format == gcvSURF_A8B8G8R8 && dstSurf->format == gcvSURF_X8B8G8R8)
        )
       )
    {
        /***********************************************************************
        ** Fast SW path.
        */

        gcmONERROR(gcoHARDWARE_Lock(&srcSurf->node, gcvNULL, (gctPOINTER*)&srcBase));
        gcmONERROR(gcoHARDWARE_Lock(&dstSurf->node, gcvNULL, (gctPOINTER*)&dstBase));

        /* Invalidate surface cache. */
        gcmONERROR(gcoSURF_NODE_Cache(&srcSurf->node, srcBase, srcSurf->size, gcvCACHE_INVALIDATE));
        gcmONERROR(gcoSURF_NODE_Cache(&dstSurf->node, dstBase, dstSurf->size, gcvCACHE_INVALIDATE));

        /* Make sure HW finished before CPU copy */
        gcmONERROR(gcoHARDWARE_Commit(Hardware));
        gcmONERROR(gcoHARDWARE_Stall(Hardware));

        for (y = 0; y < rectSize->y; ++y)
        {
            for (x = 0; x < rectSize->x; ++x)
            {
                gctPOINTER srcAddr[gcdMAX_SURF_LAYERS] = {gcvNULL};
                gctPOINTER dstAddr[gcdMAX_SURF_LAYERS] = {gcvNULL};

                gctUINT32 dstY = (Args->uArgs.v2.yInverted)
                               ? (rectSize->y - y - 1) + dstOrigin->y
                               : dstOrigin->y + y;

                srcSurf->pfGetAddr(srcSurf, srcOrigin->x + x, srcOrigin->y + y, SrcView->firstSlice, srcAddr);
                dstSurf->pfGetAddr(dstSurf, dstOrigin->x + x, dstY, DstView->firstSlice, dstAddr);

                if (srcFmtInfo->bitsPerPixel == 32)
                {
                    *(gctUINT32_PTR)dstAddr[0] = *(gctUINT32_PTR)srcAddr[0];
                }
                else
                {
                    switch (srcFmtInfo->bitsPerPixel)
                    {
                    case 64:
                        *(gctUINT64_PTR)dstAddr[0] = *(gctUINT64_PTR)srcAddr[0];
                        break;

                    case 16:
                        *(gctUINT16_PTR)dstAddr[0] = *(gctUINT16_PTR)srcAddr[0];
                        break;

                    case 8:
                        *(gctUINT8_PTR)dstAddr[0] = *(gctUINT8_PTR)srcAddr[0];
                        break;
                    }
                }
            }
        }

        goto OnExit;
    }

    if (/* Check whether the source is multi-sampled and is larger than the resolve size. */
        (srcSurf->alignedW / srcSurf->sampleInfo.x >= Hardware->resolveAlignmentX) &&
        (srcSurf->alignedH / srcSurf->sampleInfo.y >= Hardware->resolveAlignmentY) &&
        /* Check whether this is a multi-pixel render target or depth buffer. */
        (srcSurf->type == gcvSURF_RENDER_TARGET || srcSurf->type == gcvSURF_DEPTH) &&
        gcmIS_SUCCESS(_ConvertResolveFormat(Hardware, srcSurf->format, dstSurf->format, gcvNULL, gcvNULL, gcvNULL, gcvNULL))
        )
    {
        gcsSURF_VIEW tmpView;
        gcsSURF_RESOLVE_ARGS newArgs = {0};
        gceORIENTATION orientation;
        gcsPOINT tileSize, origin, size;
        gctUINT tmpOffset = 0;
        gctUINT dstOffset = DstView->firstSlice * dstSurf->sliceSize;
        gctUINT8_PTR tmpAddr;

        /* Tmp surface follows dstFormat for bitmap, srcFormat otherwise. */
        gcoSURF surface = (dstSurf->type == gcvSURF_BITMAP) ? dstSurf : srcSurf;

        gcmASSERT(dstFmtInfo->layers == 1);

        if (srcSurf->superTiled)
        {
            tileSize.x = 64;
            tileSize.y = (srcSurf->tiling & gcvTILING_SPLIT_BUFFER) ? (64 * Hardware->config->resolvePipes) : 64;
        }
        else
        {
            tileSize.x = Hardware->resolveAlignmentX;
            tileSize.y = Hardware->resolveAlignmentY;
        }

        /* Align origin to top right. */
        origin.x = gcmALIGN_BASE(srcOrigin->x, tileSize.x);
        origin.y = gcmALIGN_BASE(srcOrigin->y, tileSize.y);

        /* Align size to end of bottom right tile corner. */
        size.x = gcmALIGN((srcOrigin->x + rectSize->x), tileSize.x) - origin.x;
        size.y = gcmALIGN((srcOrigin->y + rectSize->y), tileSize.y) - origin.y;

        /* origin relative to tmpSurf. */
        tmpOrigin.x = srcOrigin->x - origin.x;
        tmpOrigin.y = srcOrigin->y - origin.y;

        /* Create a temporary surface. */
        gcmONERROR(gcoHARDWARE_AllocTmpSurface(Hardware,
                                                size.x,
                                                size.y,
                                                &surface->formatInfo,
                                                gcvSURF_BITMAP,
                                                surface->hints));

        /* Use temporary buffer as source after HW resolve */
        tmpSurf = &Hardware->tmpSurf;
        tmpSliceOffset = 0;
        tmpFmtInfo = &tmpSurf->formatInfo;

        gcmASSERT(size.x == (gctINT)tmpSurf->requestW && size.y == (gctINT)tmpSurf->requestH);

        /* Lock it. */
        gcmONERROR(gcoHARDWARE_Lock(&tmpSurf->node, gcvNULL, (gctPOINTER*)&tmpBase));

        /* Use same orientation as source. */
        orientation = tmpSurf->orientation;
        tmpSurf->orientation = srcSurf->orientation;

        tmpView.surf = &Hardware->tmpSurf;
        tmpView.firstSlice = tmpSliceOffset;
        tmpView.numSlices = 1;
        newArgs.version = gcvHAL_ARG_VERSION_V2;
        newArgs.uArgs.v2.srcOrigin = origin;
        newArgs.uArgs.v2.rectSize  = size;
        newArgs.uArgs.v2.numSlices = 1;
        /* Resolve the source into the temporary surface.
        ** Should NOT call _ResolveRect directly, as we need handle(switch/pause) tile status in gcoHARDWARE_ResolveRect.
        ** In gcoSURF_CopyPixel, we will disable source surface tile status, but which doesn't mean we really
        ** turn off the HW FC as source is probably always FC-disabled.
        */
        if ((srcSurf->type == gcvSURF_DEPTH) && (srcSurf->tileStatusNode.pool != gcvPOOL_UNKNOWN))
        {
            gcmONERROR(gcoHARDWARE_ResolveDepth(Hardware, SrcView, &tmpView, &newArgs));
        }
        else
        {
            gcmONERROR(gcoHARDWARE_ResolveRect(Hardware, SrcView, &tmpView, &newArgs));
        }

        /* Restore orientation. */
        tmpSurf->orientation = orientation;

        /* Stall the hardware. */
        gcmONERROR(gcoHARDWARE_Commit(Hardware));
        gcmONERROR(gcoHARDWARE_Stall(Hardware));

        stalled = gcvTRUE;

        /* Invalidate CPU cache of temp surface, as gcoHARDWARE_ResolveRect would warm up
        ** CPU cache by CPU reading, but then write by GPU. So need to force the coming CPU
        ** copy to read directly from memory.
        */
        gcmONERROR(gcoSURF_NODE_Cache(&tmpSurf->node, tmpBase, tmpSurf->size, gcvCACHE_INVALIDATE));

        tmpOffset = tmpOrigin.y * tmpSurf->stride + tmpOrigin.x * tmpSurf->formatInfo.bitsPerPixel / 8;
        tmpAddr = tmpBase + tmpOffset;

        if (dstSurf->type == gcvSURF_BITMAP)
        {
            gctINT32 i;
            gctUINT8_PTR dstAddr;
            gctUINT dstBpp = dstFmtInfo->bitsPerPixel / 8;
            gctUINT copySize = rectSize->x * dstBpp;

            if (Args->uArgs.v2.yInverted)
            {
                dstOffset += (rectSize->y + dstOrigin->y - 1) * dstSurf->stride + dstOrigin->x * dstBpp;
                dstAddr = dstSurf->node.logical + dstOffset;
                for (i = 0; i < rectSize->y; i++)
                {
                    gcoOS_MemCopy(dstAddr, tmpAddr, copySize);
                    tmpAddr += tmpSurf->stride;
                    dstAddr -= dstSurf->stride;
                }
            }
            else
            {
                dstOffset += dstOrigin->y * dstSurf->stride + dstOrigin->x * dstBpp;
                dstAddr = dstSurf->node.logical + dstOffset;

                if (dstSurf->allocedW == tmpSurf->alignedW &&
                    dstSurf->allocedH == tmpSurf->alignedH &&
                    dstSurf->stride   == tmpSurf->stride)
                {
                    gcoOS_MemCopy(dstAddr, tmpAddr, tmpSurf->alignedH * tmpSurf->stride);
                }
                else
                {
                    for (i = 0; i < rectSize->y; i++)
                    {
                        gcoOS_MemCopy(dstAddr, tmpAddr, copySize);
                        tmpAddr += tmpSurf->stride;
                        dstAddr += dstSurf->stride;
                    }
                }
            }

            goto OnExit;
        }

#if gcdENABLE_3D
        if ((dstSurf->type == gcvSURF_TEXTURE) && (!Args->uArgs.v2.yInverted))
        {
            /* Try using faster UploadTexture function. */
            status = gcoHARDWARE_UploadTexture(dstSurf,
                                               0,
                                               dstOrigin->x,
                                               dstOrigin->y,
                                               rectSize->x,
                                               rectSize->y,
                                               (gctPOINTER)tmpAddr,
                                               tmpSurf->stride,
                                               tmpSurf->format);

            if (status == gcvSTATUS_OK)
            {
                goto OnExit;
            }
        }
#endif
    }
    else
    {
        /* Use source as-is. */
        tmpSurf = srcSurf;
        tmpFmtInfo = srcFmtInfo;
        tmpSliceOffset = SrcView->firstSlice;
        tmpOrigin = *srcOrigin;
    }

    if (!stalled)
    {
        /* Synchronize with the GPU. */
        gcmONERROR(gcoHARDWARE_Commit(Hardware));
        gcmONERROR(gcoHARDWARE_Stall(Hardware));
    }

    do
    {
        /***********************************************************************
        ** Slow path
        */
        if (tmpSurf->sampleInfo.product > 1)
        {
            /* Cannot process multi-sampling. */
            break;
        }

        for (y = 0; y < rectSize->y; ++y)
        {
            for (x = 0; x < rectSize->x; ++x)
            {
                gctPOINTER tmpAddr[gcdMAX_SURF_LAYERS] = {gcvNULL};
                gctPOINTER dstAddr[gcdMAX_SURF_LAYERS] = {gcvNULL};

                gctUINT32 dstY = (Args->uArgs.v2.yInverted)
                               ? (rectSize->y - y - 1) + dstOrigin->y
                               : dstOrigin->y + y;

                tmpSurf->pfGetAddr(tmpSurf, tmpOrigin.x  + x, tmpOrigin.y + y, tmpSliceOffset, tmpAddr);
                dstSurf->pfGetAddr(dstSurf, dstOrigin->x + x, dstY, DstView->firstSlice, dstAddr);

                gcmONERROR(gcoHARDWARE_ConvertPixel(
                        tmpAddr[0], dstAddr[0],
                        0, 0,
                        tmpFmtInfo,
                        dstFmtInfo,
                        gcvNULL, gcvNULL,
                        0, 0
                        ));
            }
        }

        goto OnExit;
    } while (gcvFALSE);

    {
        gcsSURF_BLIT_ARGS blitArgs;

        gcoOS_ZeroMemory(&blitArgs, gcmSIZEOF(blitArgs));
        blitArgs.srcSurface = SrcView->surf;
        blitArgs.srcX       = Args->uArgs.v2.srcOrigin.x;
        blitArgs.srcY       = Args->uArgs.v2.srcOrigin.y;
        blitArgs.srcZ       = SrcView->firstSlice;
        blitArgs.srcWidth   = Args->uArgs.v2.rectSize.x;
        blitArgs.srcHeight  = Args->uArgs.v2.rectSize.y;
        blitArgs.srcDepth   = 1;
        blitArgs.dstSurface = DstView->surf;
        blitArgs.dstX       = Args->uArgs.v2.dstOrigin.x;
        blitArgs.dstY       = Args->uArgs.v2.dstOrigin.y;
        blitArgs.dstZ       = DstView->firstSlice;
        blitArgs.dstWidth   = Args->uArgs.v2.rectSize.x;
        blitArgs.dstHeight  = Args->uArgs.v2.rectSize.y;
        blitArgs.dstDepth   = 1;
        blitArgs.yReverse   = Args->uArgs.v2.yInverted;
        gcmONERROR(gcoSURF_BlitCPU(&blitArgs));
    }



OnExit:
    if (dstSurf->paddingFormat)
    {
        gctBOOL srcHasExChannel = gcvFALSE;

        if (srcFmtInfo->fmtClass == gcvFORMAT_CLASS_RGBA)
        {
            srcHasExChannel = (srcFmtInfo->u.rgba.red.width   != 0 && dstFmtInfo->u.rgba.red.width   == 0) ||
                              (srcFmtInfo->u.rgba.green.width != 0 && dstFmtInfo->u.rgba.green.width == 0) ||
                              (srcFmtInfo->u.rgba.blue.width  != 0 && dstFmtInfo->u.rgba.blue.width  == 0) ||
                              (srcFmtInfo->u.rgba.alpha.width != 0 && dstFmtInfo->u.rgba.alpha.width == 0);

            /* The source paddings are default value, is not from gcvSURF_G8R8_1_X8R8G8B8 to gcvSURF_R8_1_X8R8G8B8 and dst was full overwritten, the dst will be default value. */
            if (srcSurf->paddingFormat && !srcSurf->garbagePadded &&
                !(srcSurf->format == gcvSURF_G8R8_1_X8R8G8B8 && dstSurf->format == gcvSURF_R8_1_X8R8G8B8) &&
                dstOrigin->x == 0 && rectSize->x >= (gctINT)dstSurf->requestW &&
                dstOrigin->y == 0 && rectSize->y >= (gctINT)dstSurf->requestH)
            {
                dstSurf->garbagePadded = gcvFALSE;
            }

            /* The source has extra channel:
            ** 1, Source is not paddingformat,the dst may be garbage value.
            ** 2, source is paddingformat but not default value, the dst will be garbage value.
            */
            if (srcHasExChannel && (!srcSurf->paddingFormat || srcSurf->garbagePadded))
            {
                dstSurf->garbagePadded = gcvTRUE;
            }
        }
        else
        {
            dstSurf->garbagePadded = gcvTRUE;
        }
    }

    /* Flush CPU cache after writting */
    gcmVERIFY_OK(gcoSURF_NODE_Cache(&dstSurf->node,
                                    dstSurf->node.logical,
                                    dstSurf->size,
                                    gcvCACHE_CLEAN));

#if gcdDUMP
    if (dstSurf->node.pool == gcvPOOL_USER)
    {
        gctUINT offset, bytes, size;
        gctUINT step;

        if (tmpSurf)
        {
            if (tmpSurf->type == gcvSURF_BITMAP)
            {
                gctINT8 bpp = srcFmtInfo->bitsPerPixel / 8;
                offset  = srcOrigin->y * tmpSurf->stride + srcOrigin->x * bpp;
                size    = rectSize->x * rectSize->y * bpp;
                bytes   = rectSize->x * bpp;
                step    = tmpSurf->stride;

                if (bytes == step)
                {
                    bytes *= rectSize->y;
                    step   = 0;
                }
            }
            else
            {
                offset = 0;
                size   = tmpSurf->stride * tmpSurf->alignedH;
                bytes  = size;
                step   = 0;
            }

            while (size > 0)
            {
                gcmGETHARDWAREADDRESS(tmpSurf->node, address);
                gcmDUMP(gcvNULL,"#[info: verify copypixel source from user pool-0");
                gcmDUMP_BUFFER(gcvNULL,
                               "verify",
                               address,
                               tmpSurf->node.logical,
                               offset,
                               bytes);

                offset += step;
                size   -= bytes;
            }
        }
        /* Verify Source directly */
        else if (srcSurf)
        {
            gcmGETHARDWAREADDRESS(srcSurf->node, address);
            gcmDUMP(gcvNULL,"#[info: verify copypixel source from user pool-1");
            gcmDUMP_BUFFER(gcvNULL,
                           "verify",
                           address,
                           srcSurf->node.logical,
                           0,
                           srcSurf->sliceSize);
        }
    }
    else
    {
        /* verify the source directly */
        if (srcSurf)
        {
            gcmGETHARDWAREADDRESS(srcSurf->node, address);
            gcmDUMP(gcvNULL,"#[info: verify copypixel source");
            gcmDUMP_BUFFER(gcvNULL,
                           "verify",
                           address,
                           srcSurf->node.logical,
                           0,
                           srcSurf->sliceSize);
        }
    }

    gcmGETHARDWAREADDRESS(dstSurf->node, address);
    /* upload destination buffer to avoid uninitialized data */
    gcmDUMP_BUFFER(gcvNULL,
                   "memory",
                   address,
                   dstSurf->node.logical,
                   0,
                   dstSurf->sliceSize);
#endif

OnError:
    if (Hardware)
    {
        if (tmpBase)
        {
            gcmVERIFY_OK(gcoHARDWARE_Unlock(&tmpSurf->node, tmpSurf->type));
        }
        if (srcBase)
        {
            gcmVERIFY_OK(gcoHARDWARE_Unlock(&srcSurf->node, srcSurf->type));
        }
        if (dstBase)
        {
            gcmVERIFY_OK(gcoHARDWARE_Unlock(&dstSurf->node, dstSurf->type));
        }
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}
#endif

/*******************************************************************************
**
**  gcoHARDWARE_FlushPipe
**
**  Flush the current graphics pipe.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_FlushPipe(
    IN gcoHARDWARE Hardware,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;
#if (gcdENABLE_3D || gcdENABLE_2D)
    gctUINT32 flush;
    gctBOOL flushFixed;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    gcmGETHARDWARE(Hardware);

    if (Hardware->currentPipe == 0x1)
    {
        /* Flush 2D cache. */
        flush = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));
    }
    else
    {
        /* Flush Z and Color caches. */
        flush = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 10:10) - (0 ? 10:10) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));
    }

    /* RTL bug in older chips - flush leaks the next state.
       There is no actial bit for the fix. */
    flushFixed = Hardware->features[gcvFEATURE_FC_FLUSH_STALL];

    flushFixed = gcvFALSE;

    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E03, flush );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        if (!flushFixed)
        {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E03, flush );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }

    flush = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0594) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0594, flush );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    /* Validate the state Hardware. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

    stateDelta = stateDelta; /* Keep the compiler happy. */

#if gcdENABLE_3D
    if (Hardware->config->chipModel == gcv700
        || Hardware->config->gpuCoreCount > 1
       )
    {
        /* Flush the L2 cache. */
        gcmONERROR(gcoHARDWARE_FlushL2Cache(Hardware, Memory));
    }

    gcmONERROR(gcoHARDWARE_Semaphore(
        Hardware, gcvWHERE_RASTER, gcvWHERE_PIXEL,
        gcvHOW_SEMAPHORE,
        Memory
        ));
#endif
    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
#endif  /*(gcdENABLE_3D || gcdENABLE_2D) */
    return status;
}

#if gcdENABLE_3D
/*******************************************************************************
**
**  gcoHARDWARE_Semaphore
**
**  Send sempahore and stall until sempahore is signalled.
**
**  INPUT:
**
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**
**      gceWHERE From
**          Semaphore source.
**
**      GCHWERE To
**          Sempahore destination.
**
**      gceHOW How
**          What to do.  Can be a one of the following values:
**
**              gcvHOW_SEMAPHORE            Send sempahore.
**              gcvHOW_STALL                Stall.
**              gcvHOW_SEMAPHORE_STALL  Send semaphore and stall.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_Semaphore(
    IN gcoHARDWARE Hardware,
    IN gceWHERE From,
    IN gceWHERE To,
    IN gceHOW How,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status;
    gctBOOL semaphore, stall;
    gctUINT32 source, destination;
    gctBOOL isBlt = gcvFALSE;

    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x From=%d To=%d How=%d",
                  Hardware, From, To, How);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmASSERT(Hardware->features[gcvFEATURE_COMMAND_PREFETCH] ||
              (From != gcvWHERE_COMMAND_PREFETCH));

    semaphore = (How == gcvHOW_SEMAPHORE) || (How == gcvHOW_SEMAPHORE_STALL);
    stall     = (How == gcvHOW_STALL)     || (How == gcvHOW_SEMAPHORE_STALL);

    if (How == gcvHOW_SEMAPHORE)
    {
        if (From < Hardware->stallSource)
        {
            Hardware->stallSource = From;
        }

        if (To > Hardware->stallDestination)
        {
            Hardware->stallDestination = To;
        }

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    else if (How == gcvHOW_STALL)
    {
        /* Make sure we do a semaphore/stall here. */
        semaphore = gcvTRUE;
        stall     = gcvTRUE;
    }

    /* send semaphore-stall here */
    switch (From)
    {
    case gcvWHERE_COMMAND_PREFETCH:
        source = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ? 29:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 29:28) - (0 ? 29:28) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28)));
        break;
    case gcvWHERE_COMMAND:
        source = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)));
        break;

    case gcvWHERE_RASTER:
        source = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)));
        break;

    default:
        gcmASSERT(gcvFALSE);
        gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    switch (To)
    {
    case gcvWHERE_PIXEL:
        destination = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));
        break;

    case gcvWHERE_BLT:
        gcmASSERT(Hardware->features[gcvFEATURE_BLT_ENGINE]);
        destination = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));
        isBlt = gcvTRUE;
        break;

    default:
        gcmASSERT(gcvFALSE);
        gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    if (isBlt)
    {
        *memory++
            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x502E) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

        *memory++
            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));
    }


    if (semaphore)
    {
        /* Send sempahore token. */
        *memory++
            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

        *memory++
            = source
            | destination;

        /* Dump the semaphore. */
        gcmDUMP(gcvNULL, "#[semaphore 0x%08X 0x%08X]", source, destination);
    }

    if (stall)
    {
        if ((From == gcvWHERE_COMMAND) || (From == gcvWHERE_COMMAND_PREFETCH))
        {
            /* Stall command processor until semaphore is signalled. */
            *memory++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));
        }
        else
        {
            /* Send stall token. */
            *memory++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0F00) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));
        }

        *memory++
            = source
            | destination;

        /* Dump the stall. */
        gcmDUMP(gcvNULL, "#[stall 0x%08X 0x%08X]", source, destination);
    }

    if (isBlt)
    {
        *memory++
            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x502E) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

        *memory++
            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));
    }

    stateDelta = stateDelta; /* make compiler happy */

    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

    /* The full range is covered here, we don't need stall it before next draw */
    if ((From <= Hardware->stallSource) && (To >= Hardware->stallDestination))
    {
        Hardware->stallSource = Hardware->features[gcvFEATURE_BLT_ENGINE] ?
                                    gcvWHERE_BLT : gcvWHERE_PIXEL;
        Hardware->stallDestination = Hardware->features[gcvFEATURE_COMMAND_PREFETCH]?
                                    gcvWHERE_COMMAND_PREFETCH: gcvWHERE_COMMAND;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}



/*******************************************************************************
**
**  gcoHARDWARE_SnapPages
**
**  Send Snap to page command to free last page for idle stage.
**
**  INPUT:
**
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**
**      gcePROGRAM_STAGE_BIT Stages
**          Stages which we don't need page any more.
**
**      gctPOINTER *Memory
**          Pointer to external command buffer
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_SnapPages(
    IN gcoHARDWARE Hardware,
    IN gcePROGRAM_STAGE_BIT Stages,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status;

    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x Stages=%d",
                  Hardware, Stages);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);


    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    *memory++ =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ?
 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x13 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ?
 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) ((gctUINT32) (Stages) & ((gctUINT32) ((((1 ? 4:0) - (0 ?
 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0)));

    *memory++ = 0;

    gcmDUMP(gcvNULL, "#[snapPage 0x%08X]", Stages);

    stateDelta = stateDelta; /* make compiler happy */

    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}


gceSTATUS gcoHARDWARE_PauseTileStatus(
    IN gcoHARDWARE Hardware,
    IN gceTILE_STATUS_CONTROL Control
    )
{
    gceSTATUS status;
    gctUINT32 config;

    gcmHEADER_ARG("Hardware=0x%x Control=%d", Hardware, Control);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Determine configuration. */
    config = (Control == gcvTILE_STATUS_PAUSE) ? 0
                                               : Hardware->MCStates->memoryConfig;

    gcmONERROR(gcoHARDWARE_SelectPipe(Hardware, gcvPIPE_3D, gcvNULL));

    gcmONERROR(gcoHARDWARE_FlushPipe(Hardware, gcvNULL));

    gcmONERROR(_LoadStates(
        Hardware, 0x0595, gcvFALSE, 1, 0, &config
        ));

    Hardware->MCStates->paused = (Control == gcvTILE_STATUS_PAUSE);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the error. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_DisableHardwareTileStatus
**
**  Disable tile status hardware setting
**
**  INPUT:
**
**      IN gcoHARDWARE Hardware,
**          Hardware object
**
**      IN gceTILESTATUS_TYPE Type,
**          depth or color will be set
**
**      IN gctUINT  RtIndex
**          For color slot, which RT index will be set
**
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_DisableHardwareTileStatus(
    IN gcoHARDWARE Hardware,
    IN gceTILESTATUS_TYPE Type,
    IN gctUINT  RtIndex
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Type=%d RtIndex=%d",
                  Hardware, Type, RtIndex);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if ((Type == gcvTILESTATUS_DEPTH) ||
        (!Hardware->features[gcvFEATURE_MRT_TILE_STATUS_BUFFER]))
    {
       gcmONERROR(_DisableTileStatus(Hardware, Type));
    }
    else
    {
       gcmONERROR(_DisableTileStatusMRT(Hardware, Type, RtIndex));
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static
gceSTATUS
_AutoSetColorCompression(
    IN gcoHARDWARE Hardware
    )
{
    gctUINT i;
    gctBOOL compression = gcvFALSE;

    for (i = 0; i < Hardware->config->renderTargets; ++i)
    {
        gcoSURF surface = Hardware->PEStates->colorStates.target[i].surface;

        if ((surface != gcvNULL) &&
            (surface->tileStatusDisabled == gcvFALSE) &&
            (surface->tileStatusNode.pool != gcvPOOL_UNKNOWN))
        {
            compression |= surface->compressed;
        }
    }

    if (Hardware->PEStates->colorStates.colorCompression != compression)
    {
        Hardware->PEStates->colorStates.colorCompression = compression;
        Hardware->PEDirty->colorConfigDirty = gcvTRUE;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_EnableTileStatus(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface,
    IN gctUINT32 TileStatusAddress,
    IN gcsSURF_NODE_PTR HzTileStatus
    )
{
    gceSTATUS status;
    gctBOOL autoDisable;
    gctBOOL hierarchicalZ = gcvFALSE;
    gcsSURF_FORMAT_INFO_PTR info;
    gctUINT32 tileCount;
    gctUINT32 physicalBaseAddress;
    gctBOOL   halti2Support;
    gctUINT32 hzNodeSize;
    gctUINT32 surfaceAddress;
    gctPOINTER *cmdBuffer = gcvNULL;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x Surface=0x%x TileStatusAddress=0x%08x "
                  "HzTileStatus=0x%x",
                  Hardware, Surface, TileStatusAddress, HzTileStatus);


    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Surface->tileStatusDisabled || (TileStatusAddress == 0))
    {
        /* This surface has the tile status disabled - disable tile status. */
        status = _DisableTileStatus(Hardware, (Surface->type == gcvSURF_DEPTH) ? gcvTILESTATUS_DEPTH : gcvTILESTATUS_COLOR);
        gcmFOOTER();
        return status;
    }

    if (!Hardware->features[gcvFEATURE_TILE_FILLER] &&
        gcmIS_ERROR(_ConvertResolveFormat(Hardware, Surface->format, Surface->format, gcvNULL, gcvNULL, gcvNULL, gcvNULL)))
    {
        /* This surface has the tile status disabled - disable tile status. */
        status = _DisableTileStatus(Hardware, (Surface->type == gcvSURF_DEPTH)? gcvTILESTATUS_DEPTH :
                                                                                gcvTILESTATUS_COLOR);

        Surface->tileStatusDisabled = gcvTRUE;

        gcmFOOTER();
        return status;
    }

    halti2Support = Hardware->features[gcvFEATURE_HALTI2];

    /* Verify that the surface is locked. */
    gcmVERIFY_LOCK(Surface);

    /* Convert the format into bits per pixel. */
    info = &Surface->formatInfo;

    if (Hardware->features[gcvFEATURE_128BTILE])
    {
        if (Surface->cacheMode == gcvCACHE_256)
        {
            tileCount = Surface->size / 256;
        }
        else
        {
            tileCount = Surface->size / 128;
        }
    }
    else
    {
        if ((info->bitsPerPixel  == 16)
        &&  (Hardware->config->chipModel == gcv500)
        &&  (Hardware->config->chipRevision < 3)
        )
        {
            /* 32-bytes per tile. */
            tileCount = Surface->size / 32;
        }
        else
        {
            /* 64-bytes per tile. */
            tileCount = Surface->size / 64;
        }
    }

    /* Cannot auto disable TS for compressed case, because the TS buffer
    ** contains info for compress.
    */
    autoDisable = ((Surface->compressed == gcvFALSE) &&
                   ((Hardware->features[gcvFEATURE_CORRECT_AUTO_DISABLE_COUNT_WIDTH]) ||
                    ((Hardware->features[gcvFEATURE_CORRECT_AUTO_DISABLE_COUNT]) &&
                     (tileCount < (1 << 20))
                    )
                   )
                  );

    if (Surface->type == gcvSURF_DEPTH)
    {
        if (Surface != Hardware->PEStates->depthStates.surface)
        {
            /* Depth buffer is not current - no need to enable. */
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }

        /* Determine whether hierarchical Z is supported. */
        hierarchicalZ = Surface->hzDisabled
                      ? gcvFALSE
                      : Hardware->features[gcvFEATURE_HZ]
                      ;
    }
    else
    {
        if (Surface != Hardware->PEStates->colorStates.target[0].surface)
        {
            /* Render target is not current - no need to enable. */
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }
    }

#if gcdSECURE_USER
    physicalBaseAddress = 0;
#else
    if (Hardware->features[gcvFEATURE_MC20])
    {
        physicalBaseAddress = 0;
    }
    else
    {
        gcmONERROR(gcoOS_GetBaseAddress(gcvNULL, &physicalBaseAddress));
    }
#endif

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, cmdBuffer);

    /* Check for depth surfaces. */
    if (Surface->type == gcvSURF_DEPTH)
    {
        /* Flush the depth cache. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0xFFFF) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0xFFFF, 0);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        /* Enable fast clear. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        /* Set depth format. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (info->bitsPerPixel == 16) & ((gctUINT32) ((((1 ?
 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ?
 3:3)));

        /* Enable compression or not. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (Surface->compressed) & ((gctUINT32) ((((1 ?
 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ?
 6:6)));

        if (Hardware->features[gcvFEATURE_COMPRESSION_V4])
        {
            /* in case it's depth only, we need set this too */
            if (Surface->isMsaa)
            {
                Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));
            }
            else
            {
                Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));
            }
        }

        /* Automatically disable fast clear or not. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (autoDisable) & ((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4)));


        if (Hardware->features[gcvFEATURE_MC_STENCIL_CTRL])
        {
            gctBOOL hasStencil = (info->format == gcvSURF_D24S8 ||
                                  info->format == gcvSURF_X24S8 ||
                                  info->format == gcvSURF_S8);

            Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ? 14:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ?
 14:14))) | (((gctUINT32) ((gctUINT32) (hasStencil) & ((gctUINT32) ((((1 ?
 14:14) - (0 ? 14:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ?
 14:14)));
        }

        if (autoDisable)
        {
            /* Program surface auto disable counter. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x059C) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x059C, tileCount );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }

        /* Hierarchical Z. */
        if (hierarchicalZ)
        {
            gctUINT32 hzTileStatusAddress;

            gctBOOL hasHz = (HzTileStatus->pool != gcvPOOL_UNKNOWN);

            Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (hasHz) & ((gctUINT32) ((((1 ? 12:12) - (0 ?
 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ?
 12:12)));

            Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:13) - (0 ? 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ?
 13:13))) | (((gctUINT32) ((gctUINT32) (hasHz) & ((gctUINT32) ((((1 ? 13:13) - (0 ?
 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ?
 13:13)));

            gcmGETHARDWAREADDRESS(*HzTileStatus, hzTileStatusAddress);

            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05A9) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x05A9, hzTileStatusAddress + physicalBaseAddress );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            gcmSAFECASTSIZET(hzNodeSize, Surface->hzNode.size / 16);

            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05AB) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x05AB, hzNodeSize );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05AA) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x05AA, Surface->fcValueHz );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            gcmDUMP(gcvNULL, "#[surface 0x%x 0x%x]", hzTileStatusAddress, HzTileStatus->size);


        }

        /* Program fast clear registers. */
        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)3  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (3 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0599) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            /* Program tile status base address register. */
            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0599,
                TileStatusAddress + physicalBaseAddress
                );

            gcmGETHARDWAREADDRESS(Surface->node, surfaceAddress);

            /* Program surface base address register. */
            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x059A,
                surfaceAddress + physicalBaseAddress
                );

            /* Program clear value register. */
            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x059B,
                Surface->fcValue
                );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );

        gcmDUMP(gcvNULL, "#[surface 0x%x 0x%x]", TileStatusAddress, Surface->tileStatusNode.size);


    }

    /* Render target surfaces. */
    else
    {
        /* Flush the color cache. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0xFFFF) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0xFFFF, 0);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        /* Enable fast clear or not. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));

        /* Turn on color compression when in MSAA mode. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) (Surface->compressed) & ((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7)));

        if (Surface->compressed)
        {

            if (Surface->compressFormat == -1)
            {
                gcmFATAL("color pipe is taking invalid compression format for surf=0x%x, format=%d",
                          Surface, Surface->format);
            }

            Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (Surface->compressFormat) & ((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)));
        }

        if (Hardware->features[gcvFEATURE_COMPRESSION_V4])
        {
            if (Surface->isMsaa)
            {
                Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));
            }
            else
            {
                Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));
            }
        }

        if (Hardware->features[gcvFEATURE_128BTILE])
        {
            Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:27) - (0 ? 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) ((gctUINT32) (Surface->tiling == gcvYMAJOR_SUPERTILED) & ((gctUINT32) ((((1 ?
 27:27) - (0 ? 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ?
 27:27)));
        }

        /* Automatically disable fast clear or not. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (autoDisable) & ((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5)));

        if (halti2Support)
        {
            Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) ((info->bitsPerPixel == 64)) & ((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30)));
        }

        if (autoDisable)
        {
            /* Program surface auto disable counter. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x059D) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x059D, tileCount );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }

        /* Program fast clear registers. */
        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)3  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (3 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0596) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            /* Program tile status base address register. */
            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0596,
                TileStatusAddress + physicalBaseAddress
                );

            gcmGETHARDWAREADDRESS(Surface->node, surfaceAddress);

            /* Program surface base address register. */
            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0597,
                surfaceAddress + physicalBaseAddress
                );

            /* Program clear value register. */
            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0598,
                Surface->fcValue
                );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );


        gcmDUMP(gcvNULL, "#[surface 0x%x 0x%x]", TileStatusAddress, Surface->tileStatusNode.size);

        if (halti2Support)
        {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05AF) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x05AF, Surface->fcValueUpper );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }

        _AutoSetColorCompression(Hardware);
    }

    /* Program memory configuration register. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0595) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0595, Hardware->MCStates->memoryConfig );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};



    gcmONERROR(gcoHARDWARE_Semaphore(
        Hardware, gcvWHERE_RASTER, gcvWHERE_PIXEL,
        gcvHOW_SEMAPHORE,
        (gctPOINTER *)&memory
        ));

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, cmdBuffer);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}
static gceSTATUS
_EnableTileStatusMRT(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface,
    IN gctUINT32 TileStatusAddress,
    IN gcsSURF_NODE_PTR HzTileStatus,
    IN gctUINT  RtIndex
    )
{
    gceSTATUS status;
    gctBOOL autoDisable;
    gcsSURF_FORMAT_INFO_PTR info;
    gctUINT32 tileCount;
    gctUINT32 physicalBaseAddress;
    gctBOOL   halti2Support;
    gctUINT32 surfaceAddress;
    gctPOINTER *cmdBuffer = gcvNULL;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x Surface=0x%x TileStatusAddress=0x%08x "
                  "HzTileStatus=0x%x RtIndex=%d",
                  Hardware, Surface, TileStatusAddress, HzTileStatus, RtIndex);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    gcmASSERT(Hardware->features[gcvFEATURE_MRT_TILE_STATUS_BUFFER]);
    gcmASSERT(RtIndex < Hardware->config->renderTargets);
    gcmASSERT(Surface->type != gcvSURF_DEPTH);

    if (Surface->tileStatusDisabled || (TileStatusAddress == 0))
    {
        /* This surface has the tile status disabled - disable tile status. */
        status = _DisableTileStatusMRT(Hardware, gcvTILESTATUS_COLOR, RtIndex);

        gcmFOOTER();

        return status;
    }

    if (!Hardware->features[gcvFEATURE_TILE_FILLER] &&
        gcmIS_ERROR(_ConvertResolveFormat(Hardware, Surface->format, Surface->format, gcvNULL, gcvNULL, gcvNULL, gcvNULL)))
    {
        /* This surface has the tile status disabled - disable tile status. */
        status = _DisableTileStatusMRT(Hardware, gcvTILESTATUS_COLOR, RtIndex);

        Surface->tileStatusDisabled = gcvTRUE;

        gcmFOOTER();
        return status;
    }

    halti2Support = Hardware->features[gcvFEATURE_HALTI2];

    /* Verify that the surface is locked. */
    gcmVERIFY_LOCK(Surface);

    /* Convert the format into bits per pixel. */
    info = &Surface->formatInfo;

    if (Hardware->features[gcvFEATURE_128BTILE])
    {
        if (Surface->cacheMode == gcvCACHE_256)
        {
            tileCount = Surface->size / 256;
        }
        else
        {
            tileCount = Surface->size /128;
        }
    }
    else
    {
        if ((info->bitsPerPixel  == 16)
        &&  (Hardware->config->chipModel    == gcv500)
        &&  (Hardware->config->chipRevision < 3)
        )
        {
            /* 32-bytes per tile. */
            tileCount = Surface->size / 32;
        }
        else
        {
            /* 64-bytes per tile. */
            tileCount = Surface->size / 64;
        }
    }

    autoDisable = ((Surface->compressed == gcvFALSE) &&
                   ((Hardware->features[gcvFEATURE_CORRECT_AUTO_DISABLE_COUNT_WIDTH]) ||
                    ((Hardware->features[gcvFEATURE_CORRECT_AUTO_DISABLE_COUNT]) &&
                     (tileCount < (1 << 20))
                    )
                   )
                  );

    if (Surface != Hardware->PEStates->colorStates.target[RtIndex].surface)
    {
        /* Render target is not current - no need to enable. */
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

#if gcdSECURE_USER
    physicalBaseAddress = 0;
#else
    if (Hardware->features[gcvFEATURE_MC20])
    {
        physicalBaseAddress = 0;
    }
    else
    {
        gcmONERROR(gcoOS_GetBaseAddress(gcvNULL, &physicalBaseAddress));
    }
#endif

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, cmdBuffer);

    /* Flush the color cache. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0xFFFF) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0xFFFF, 0);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    /* Enable fast clear or not. */
    Hardware->MCStates->memoryConfigMRT[RtIndex] = ((((gctUINT32) (Hardware->MCStates->memoryConfigMRT[RtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        /* Turn on color compression when in MSAA mode. */
    Hardware->MCStates->memoryConfigMRT[RtIndex] = ((((gctUINT32) (Hardware->MCStates->memoryConfigMRT[RtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) ((gctUINT32) (Surface->compressed) & ((gctUINT32) ((((1 ?
 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ?
 2:2)));

    if (Surface->compressed)
    {
        Hardware->MCStates->memoryConfigMRT[RtIndex] = ((((gctUINT32) (Hardware->MCStates->memoryConfigMRT[RtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) ((gctUINT32) (Surface->compressFormat) & ((gctUINT32) ((((1 ?
 6:3) - (0 ? 6:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ?
 6:3)));
    }

    if (Hardware->features[gcvFEATURE_COMPRESSION_V4])
    {
        if (Surface->isMsaa)
        {
            Hardware->MCStates->memoryConfigMRT[RtIndex] = ((((gctUINT32) (Hardware->MCStates->memoryConfigMRT[RtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ? 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9)));
        }
        else
        {
            Hardware->MCStates->memoryConfigMRT[RtIndex] = ((((gctUINT32) (Hardware->MCStates->memoryConfigMRT[RtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ? 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9)));
        }
    }

    if (Hardware->features[gcvFEATURE_128BTILE])
    {
        Hardware->MCStates->memoryConfigMRT[RtIndex] = ((((gctUINT32) (Hardware->MCStates->memoryConfigMRT[RtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) ((gctUINT32) (Surface->tiling == gcvYMAJOR_SUPERTILED) & ((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10)));
    }

    /* Automatically disable fast clear or not. */
    Hardware->MCStates->memoryConfigMRT[RtIndex] = ((((gctUINT32) (Hardware->MCStates->memoryConfigMRT[RtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (autoDisable) & ((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1)));

    if (halti2Support)
    {
        Hardware->MCStates->memoryConfigMRT[RtIndex] = ((((gctUINT32) (Hardware->MCStates->memoryConfigMRT[RtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) ((info->bitsPerPixel == 64)) & ((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7)));
    }

    /*
    ** 1. 0x0690 slot 0 is uesless on RTL impelmetation.
    ** 2. we need keep Hardware->memoryConfig always has correct information for RT0,
    **    as it's combined with depth information.
    */
    if (RtIndex == 0)
    {
            /* Enable fast clear or not. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));

            /* Turn on color compression when in MSAA mode. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) (Surface->compressed) & ((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7)));

        if (Surface->compressed)
        {
            Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (Surface->compressFormat) & ((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)));
        }

        if (Hardware->features[gcvFEATURE_COMPRESSION_V4])
        {
            if (Surface->isMsaa)
            {
                Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));
            }
            else
            {
                Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));
            }
        }

        if (Hardware->features[gcvFEATURE_128BTILE])
        {
            Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:27) - (0 ? 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) ((gctUINT32) (Surface->tiling == gcvYMAJOR_SUPERTILED) & ((gctUINT32) ((((1 ?
 27:27) - (0 ? 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ?
 27:27)));
        }

        /* Automatically disable fast clear or not. */
        Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (autoDisable) & ((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5)));

        if (halti2Support)
        {
            Hardware->MCStates->memoryConfig = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) ((info->bitsPerPixel == 64)) & ((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30)));
        }

        if (autoDisable)
        {
            /* Program surface auto disable counter. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x059D) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x059D, tileCount );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }

        /* Program tile status base address register. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0596) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0596, TileStatusAddress + physicalBaseAddress );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        gcmGETHARDWAREADDRESS(Surface->node, surfaceAddress);

        /* Program surface base address register. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0597) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0597, surfaceAddress + physicalBaseAddress );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        /* Program clear value register. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0598) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0598, Surface->fcValue );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        gcmDUMP(gcvNULL, "#[surface 0x%x 0x%x]", TileStatusAddress, Surface->tileStatusNode.size);

        if (halti2Support)
        {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05AF) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x05AF, Surface->fcValueUpper );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }

        /* Program memory configuration register. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0595) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0595, Hardware->MCStates->memoryConfig );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    }
    else
    {
        if (autoDisable)
        {
            /* Program surface auto disable counter. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0690 + RtIndex) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0690 + RtIndex,
 tileCount );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }

        /* Program tile status base address register. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05F0 + RtIndex) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x05F0 + RtIndex,
 TileStatusAddress + physicalBaseAddress );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        gcmGETHARDWAREADDRESS(Surface->node, surfaceAddress);

        /* Program surface base address register. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05F8 + RtIndex) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x05F8 + RtIndex,
 surfaceAddress + physicalBaseAddress );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        /* Program clear value register. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0680 + RtIndex) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0680 + RtIndex,
 Surface->fcValue );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        gcmDUMP(gcvNULL, "#[surface 0x%x 0x%x]", TileStatusAddress, Surface->tileStatusNode.size);

        if (halti2Support)
        {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0688 + RtIndex) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0688 + RtIndex,
 Surface->fcValueUpper );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }

         /* Program memory configuration register. */
         {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05E8 + RtIndex) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x05E8 + RtIndex,
 Hardware->MCStates->memoryConfigMRT[RtIndex] );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    }
    gcmONERROR(gcoHARDWARE_Semaphore(
        Hardware, gcvWHERE_RASTER, gcvWHERE_PIXEL,
        gcvHOW_SEMAPHORE,
        (gctPOINTER *)&memory
        ));

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, cmdBuffer);

    _AutoSetColorCompression(Hardware);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_EnableTileStatus(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface,
    IN gctUINT32 TileStatusAddress,
    IN gcsSURF_NODE_PTR HzTileStatus,
    IN gctUINT  RtIndex
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Surface=0x%x TileStatusAddress=0x%08x "
                  "HzTileStatus=0x%x RtIndex=%d",
                  Hardware, Surface, TileStatusAddress, HzTileStatus, RtIndex);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if ((Surface->type == gcvSURF_DEPTH) ||
        (!Hardware->features[gcvFEATURE_MRT_TILE_STATUS_BUFFER]))
    {
        gcmONERROR(_EnableTileStatus(Hardware, Surface, TileStatusAddress, HzTileStatus));
    }
    else
    {
        gcmONERROR(_EnableTileStatusMRT(Hardware, Surface, TileStatusAddress, HzTileStatus, RtIndex));
    }

    /* Any time tile status enable, we need set cache mode */
    if (Hardware->features[gcvFEATURE_128BTILE] &&
        TileStatusAddress != 0 &&
        !Surface->tileStatusDisabled)
    {
        /* When enable TS, we don't always trigger flushTarget,
            but cachemode could change
        */
        if (Surface->type == gcvSURF_RENDER_TARGET &&
            Surface == Hardware->PEStates->colorStates.target[RtIndex].surface)
        {
            Hardware->PEDirty->colorConfigDirty = gcvTRUE;
        }
        else if (Surface->type == gcvSURF_DEPTH &&
                   Surface == Hardware->PEStates->depthStates.surface)
        {
            Hardware->PEDirty->colorConfigDirty = gcvTRUE;
        }

        if (Surface->cacheMode == gcvCACHE_NONE)
        {
            Surface->cacheMode = DEFAULT_CACHE_MODE;
        }

    }

    if (!Surface->tileStatusDisabled)
    {
        Surface->dirty = gcvTRUE;
    }

OnError:

    gcmFOOTER_NO();
    return status;

}

gceSTATUS
gcoHARDWARE_FillFromTileStatus(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface
    )
{
    gceSTATUS status;
    gctUINT32 physicalBaseAddress, tileCount, srcStride;
    gctBOOL halti2Support;
    gcoSURF current;
    gctUINT32 tileStatusAddress;
    gctUINT32 surfaceAddress;
    gctUINT32 sizeAlignment;
    gctBOOL savedMsaa = gcvFALSE;
    gctBOOL needRestore = gcvFALSE;
    gctUINT32 fillerBpp = 0;
    gctPOINTER *cmdBuffer = gcvNULL;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x Surface=0x%x", Hardware, Surface);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /*
    ** Adjust MSAA setting for tile filler.
    */
    savedMsaa = Surface->isMsaa;
    needRestore = gcvTRUE;
    Surface->isMsaa = gcvFALSE;
    gcmONERROR(gcoHARDWARE_AdjustCacheMode(Hardware, Surface));
    Surface->isMsaa = savedMsaa;
    needRestore = gcvFALSE;

    if (Hardware->features[gcvFEATURE_BLT_ENGINE])
    {
        gcmONERROR(gcoHARDWARE_3DBlitTileFill(Hardware, gcvENGINE_RENDER, Surface));

        gcmFOOTER();
        return status;
    }

    gcoHARDWARE_SetHWSlot(Hardware, gcvENGINE_RENDER, gcvHWSLOT_BLT_DST, Surface->node.u.normal.node,0);

    current = Hardware->PEStates->colorStates.target[0].surface;

#if gcdSECURE_USER
    physicalBaseAddress = 0;
#else
    /* Get physical base address. */
    if (Hardware->features[gcvFEATURE_MC20])
    {
        physicalBaseAddress = 0;
    }
    else
    {
        gcmONERROR(gcoOS_GetBaseAddress(gcvNULL, &physicalBaseAddress));
    }
#endif

    halti2Support = Hardware->features[gcvFEATURE_HALTI2];

    if (Hardware->features[gcvFEATURE_TILEFILLER_32TILE_ALIGNED])
    {
        sizeAlignment = 0x7ff;
    }
    else
    {
        sizeAlignment = 0x3fff;
    }

    if (Hardware->features[gcvFEATURE_128BTILE])
    {
        if (Surface->cacheMode == gcvCACHE_256)
        {
            tileCount = Surface->size / 256;
        }
        else
        {
            tileCount = Surface->size /128;
        }

        switch (Surface->bitsPerPixel)
        {
        case 8:
            fillerBpp = 0;
            break;
        case 16:
            fillerBpp = 1;
            break;
        case 32:
            fillerBpp = 2;
            break;
        case 64:
            fillerBpp = 3;
            break;
        case 128:
            fillerBpp = 4;
            break;
        default:
            gcmASSERT(0);
            fillerBpp = 2;
            break;
        }
    }
    else
    {
        /* Program TileFiller. */
        if ((Surface->node.size & sizeAlignment) == 0)
        {
            tileCount = (gctUINT32)(Surface->node.size / (Surface->isMsaa ? 256 : 64));
        }
        else
        {
            tileCount = 0;
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }

    /* Determine the source stride. */
    srcStride = (Surface->tiling == gcvLINEAR)

              /* Linear. */
              ? Surface->stride

              /* Tiled. */
              : ((((gctUINT32) (Surface->stride * 4)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (Surface->superTiled) & ((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

    if ((Hardware->config->resolvePipes > 1)
        && (Surface->tiling & gcvTILING_SPLIT_BUFFER))
    {
        srcStride = ((((gctUINT32) (srcStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)));
    }

    if (Hardware->features[gcvFEATURE_128BTILE])
    {
        if (Surface->compressed)
        {
            /* Tile filler doesnt compressed surface. */
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        if (Surface->tiling == gcvSUPERTILED)
        {
            srcStride = ((((gctUINT32) (srcStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ? 28:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ?
 28:27))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ? 28:27) - (0 ?
 28:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ?
 28:27)));
        }
        else if (Surface->tiling == gcvYMAJOR_SUPERTILED)
        {
            srcStride = ((((gctUINT32) (srcStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ? 28:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ?
 28:27))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ? 28:27) - (0 ?
 28:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ?
 28:27)));
        }
        else
        {
            srcStride = ((((gctUINT32) (srcStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ? 28:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ?
 28:27))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ? 28:27) - (0 ?
 28:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ?
 28:27)));
        }

        srcStride = ((((gctUINT32) (srcStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) (Surface->cacheMode == gcvCACHE_256) & ((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ?
 29:29)));
    }
    /* Determine Fast MSAA mode. */
    else if (Hardware->features[gcvFEATURE_FAST_MSAA] ||
        Hardware->features[gcvFEATURE_SMALL_MSAA])
    {
        if (Surface->compressed)
        {
            /* Tile filler doesnt compressed surface. */
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
        else
        {
            srcStride = ((((gctUINT32) (srcStride)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 29:29) - (0 ? 29:29) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)));
        }
    }

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, cmdBuffer);

#if gcdENABLE_TRUST_APPLICATION
    if (Hardware->features[gcvFEATURE_SECURITY])
    {
        gcoHARDWARE_SetProtectMode(
            Hardware,
            (Surface->hints & gcvSURF_PROTECTED_CONTENT),
            (gctPOINTER *)&memory);

        Hardware->GPUProtecedModeDirty = gcvTRUE;
    }
#endif

    if (Hardware->config->gpuCoreCount == 1)
    {
        gcmONERROR(gcoHARDWARE_Semaphore(
            Hardware, gcvWHERE_RASTER, gcvWHERE_PIXEL,
            gcvHOW_SEMAPHORE_STALL,
            (gctPOINTER*) &memory
            ));
    }

    if (Hardware->config->gpuCoreCount > 1)
    {
        /* Invalidate the tile status cache. */
        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0594) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        /* Sync the GPUs. */
        gcmONERROR(gcoHARDWARE_MultiGPUSync(Hardware, &memory));

        /* Select core 3D 0 to do the tile fill operation. */
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0);
 memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0));
 } };

    }

    /* Reprogram tile status. */
    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)3  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (3 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0596) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


        gcmGETHARDWAREADDRESS(Surface->tileStatusNode, tileStatusAddress);
        gcmGETHARDWAREADDRESS(Surface->node, surfaceAddress);

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0596,
            tileStatusAddress + physicalBaseAddress
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0597,
            surfaceAddress + physicalBaseAddress
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0598,
            Surface->fcValue
            );

    gcmENDSTATEBATCH_NEW(
        reserve, memory
        );

    if (halti2Support || Hardware->features[gcvFEATURE_128BTILE])
    {
        gctUINT32 rsConfig;

        if (Hardware->features[gcvFEATURE_128BTILE])
        {

            rsConfig = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ? 28:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 28:28) - (0 ? 28:28) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28)))
                         | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (Surface->cacheMode == gcvCACHE_256) & ((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24)))
                         | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (fillerBpp) & ((gctUINT32) ((((1 ?
 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25)));
        }
        else
        {
            rsConfig = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ? 28:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 28:28) - (0 ? 28:28) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28)))
                         | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (Surface->isMsaa ? 1: 0) & ((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24)))
                         | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (fillerBpp) & ((gctUINT32) ((((1 ?
 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ?
 27:25)));
        }

        /* Set upper color data. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05AF) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x05AF, Surface->fcValueUpper );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05A8) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x05A8, rsConfig );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    }

    /* Set source and destination stride. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0583) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0583, srcStride );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    /* Trigger the fill. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05AC) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x05AC, tileCount );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    /* Restore tile status. */
    if ((Surface != current) &&
        (current != gcvNULL) &&
        (current->tileStatusNode.pool != gcvPOOL_UNKNOWN))
    {
        gcmGETHARDWAREADDRESS(current->tileStatusNode, tileStatusAddress);
        gcmGETHARDWAREADDRESS(current->node, surfaceAddress);

        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)3  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (3 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0596) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0596,
                tileStatusAddress + physicalBaseAddress
                );

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0597,
                surfaceAddress + physicalBaseAddress
                );

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0598,
                current->fcValue
                );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );

        if (halti2Support)
        {
            /* Set upper color data. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05AF) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x05AF, current->fcValueUpper );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }
    }

    if (Hardware->config->gpuCoreCount > 1)
    {
        /* Enable all 3D GPUs. */
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK;
 memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };


        /* Invalidate the tile status cache. */
        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0594) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        /* Sync the GPUs. */
        gcmONERROR(gcoHARDWARE_MultiGPUSync(Hardware, &memory));
    }

    if (Hardware->config->gpuCoreCount == 1)
    {
        gcmONERROR(gcoHARDWARE_Semaphore(
            Hardware, gcvWHERE_RASTER, gcvWHERE_PIXEL,
            gcvHOW_SEMAPHORE,
            (gctPOINTER *)&memory
            ));
    }

    /* Make compiler happy */

    stateDelta = stateDelta;
    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, cmdBuffer);

    /* All pixels have been resolved. */
    Surface->dirty = gcvFALSE;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (needRestore)
    {
        Surface->isMsaa = savedMsaa;
    }
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gceSTATUS
_FlushTileStatusCache(
    IN gcoHARDWARE Hardware
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 physical[3] = {0};
    gctINT stride;
    gctPOINTER logical[3] = {gcvNULL};
    gctUINT32 format;
    gctUINT32 physicalBaseAddress;
    gcsPOINT rectSize;
    gctUINT32 tileStatusAddress;
    gctUINT32 surfaceAddress;
    gctPOINTER *cmdBuffer = gcvNULL;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x", Hardware);
    /* This function can't work with MRT tile status buffer */
    gcmASSERT(Hardware->features[gcvFEATURE_MRT_TILE_STATUS_BUFFER] == gcvFALSE);

    gcmASSERT(Hardware->features[gcvFEATURE_BLT_ENGINE] == gcvFALSE);

    /* Get physical base address. */
    if (Hardware->features[gcvFEATURE_MC20])
    {
        physicalBaseAddress = 0;
    }
    else
    {
        gcmONERROR(gcoOS_GetBaseAddress(gcvNULL, &physicalBaseAddress));
    }

    if (Hardware->tempSurface == gcvNULL)
    {
        /* Construct a temporary surface. */
        gcmONERROR(gcoSURF_Construct(gcvNULL,
                                     64, 64, 1,
                                     gcvSURF_RENDER_TARGET,
                                     gcvSURF_A8R8G8B8,
                                     gcvPOOL_DEFAULT,
                                     &Hardware->tempSurface));
    }

    /* Set rect size as 16x8. */
    rectSize.x = 16;
    rectSize.y =  8;

    /* Lock the temporary surface. */
    gcmONERROR(gcoSURF_Lock(Hardware->tempSurface, physical, logical));

    /* Get the stride of the temporary surface. */
    gcmONERROR(gcoSURF_GetAlignedSize(Hardware->tempSurface,
                                      gcvNULL, gcvNULL,
                                      &stride));

    /* Convert the source format. */
    gcmONERROR(_ConvertResolveFormat(Hardware,
                                     Hardware->tempSurface->format,
                                     Hardware->tempSurface->format,
                                     &format,
                                     gcvNULL,
                                     gcvNULL,
                                     gcvNULL));

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, cmdBuffer);

    /* Flush the depth/color cache of current RT to update ts cache */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    /* Reprogram tile status. */
    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)2  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0596) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


        gcmGETHARDWAREADDRESS(Hardware->tempSurface->tileStatusNode,
            tileStatusAddress);

        gcmGETHARDWAREADDRESS(Hardware->tempSurface->node,
            surfaceAddress);

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0596,
            tileStatusAddress + physicalBaseAddress
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0597,
            surfaceAddress + physicalBaseAddress
            );

        gcmSETFILLER_NEW(
            reserve, memory
            );

    gcmENDSTATEBATCH_NEW(
        reserve, memory
        );

    /* Flush the tile status. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0594) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0594, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    /* Append RESOLVE_CONFIG state. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0581) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0581, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 4:0) - (0 ?
 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:7) - (0 ?
 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:5) - (0 ? 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 6:5) - (0 ? 6:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ? 6:5))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x06 & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ? 14:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ?
 14:14))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 14:14) - (0 ? 14:14) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    /* Set source and destination stride. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0583) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0583, stride << 2 );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0585) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0585, stride << 2 );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    /* TODO: for singleBuffer+MultiPipe case, no need to program the 2nd address */
    if (Hardware->config->resolvePipes == 2)
    {
        gctUINT32 physical2 = physical[0] + Hardware->tempSurface->bottomBufferOffset;

        /* Append RESOLVE_SOURCE addresses. */
        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)2  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05B0) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            gcmSETCTRLSTATE_NEW(
                stateDelta, reserve, memory, 0x05B0,
                physical[0]
                );

            gcmSETCTRLSTATE_NEW(
                stateDelta, reserve, memory, 0x05B0 + 1,
                physical2
                );

            gcmSETFILLER_NEW(
                reserve, memory
                );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );

        /* Append RESOLVE_DESTINATION addresses. */
        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)2  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05B8) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            gcmSETCTRLSTATE_NEW(
                stateDelta, reserve, memory, 0x05B8,
                physical[0]
                );

            gcmSETCTRLSTATE_NEW(
                stateDelta, reserve, memory, 0x05B8 + 1,
                physical2
                );

            gcmSETFILLER_NEW(
                reserve, memory
                );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );

        /* Append offset. */
        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)2  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05C0) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            gcmSETCTRLSTATE_NEW(
                stateDelta, reserve, memory, 0x05C0,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:16) - (0 ? 28:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:16) - (0 ? 28:16) + 1))))))) << (0 ?
 28:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 28:16) - (0 ?
 28:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:16) - (0 ? 28:16) + 1))))))) << (0 ?
 28:16)))
                );

            gcmSETCTRLSTATE_NEW(
                stateDelta, reserve, memory, 0x05C0 + 1,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:16) - (0 ? 28:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:16) - (0 ? 28:16) + 1))))))) << (0 ?
 28:16))) | (((gctUINT32) ((gctUINT32) (rectSize.y) & ((gctUINT32) ((((1 ?
 28:16) - (0 ? 28:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:16) - (0 ? 28:16) + 1))))))) << (0 ?
 28:16)))
                );

            gcmSETFILLER_NEW(
                reserve, memory
                );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );
    }
    else
    {
        /* Append RESOLVE_SOURCE commands. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0582) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0582, physical[0] );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        /* Append RESOLVE_DESTINATION commands. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0584) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0584, physical[0] );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        if (Hardware->features[gcvFEATURE_RS_NEW_BASEADDR])
        {
            /* Append RESOLVE_SOURCE commands. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05B0) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x05B0, physical[0] );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            /* Append RESOLVE_DESTINATION commands. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05B8) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x05B8, physical[0] );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }
    }

    /* Program the window size. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0588) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0588, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (rectSize.x) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ?
 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (rectSize.y) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    /* Make sure it is a resolve and not a clear. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x058F) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x058F, 0 );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    gcmASSERT(!Hardware->features[gcvFEATURE_BLT_ENGINE]);

    /* Start the resolve. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0580) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0580, 0xBADABEEB );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    if ((Hardware->PEStates->colorStates.target[0].surface != gcvNULL) &&
        (Hardware->PEStates->colorStates.target[0].surface->tileStatusNode.pool != gcvPOOL_UNKNOWN))
    {

        /* Reprogram tile status. */
        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)2  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0596) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            gcmGETHARDWAREADDRESS(Hardware->PEStates->colorStates.target[0].surface->tileStatusNode,
                tileStatusAddress
                );

            gcmGETHARDWAREADDRESS(Hardware->PEStates->colorStates.target[0].surface->node,
                surfaceAddress
                );

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0596,
                tileStatusAddress + physicalBaseAddress
                );

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0597,
                surfaceAddress + physicalBaseAddress
                );

            gcmSETFILLER_NEW(
                reserve, memory
                );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );
    }

    gcmONERROR(gcoHARDWARE_Semaphore(
        Hardware, gcvWHERE_RASTER, gcvWHERE_PIXEL,
        gcvHOW_SEMAPHORE,
        (gctPOINTER *)&memory
        ));

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, cmdBuffer);


OnError:
    if (logical[0] != gcvNULL)
    {
        /* Unlock the temporary surface. */
        gcmVERIFY_OK(gcoSURF_Unlock(Hardware->tempSurface, logical[0]));
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}



gceSTATUS
_FlushAndDisableTileStatus(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface,
    IN gctBOOL Decompress,
    IN gctBOOL Disable
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL flushed            = gcvFALSE;
    gctBOOL flushedTileStatus  = gcvFALSE;
    gctBOOL switchedTileStatus = gcvFALSE;
    gcoSURF saved = gcvNULL;
    gctPOINTER logical[3] = {gcvNULL};
    gctUINT32 tileStatusAddress;
    gctUINT32 surfaceAddress;
    gctBOOL current = gcvTRUE;


    gcmHEADER_ARG("Hardware=0x%x Surface=0x%x Decompress=%d, Disable=%d",
                  Hardware, Surface, Decompress, Disable);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* If tilestatus was already disabled */
    if (Surface->tileStatusDisabled ||
        (Surface->tileStatusNode.pool == gcvPOOL_UNKNOWN))
    {
        goto OnError;
    }

    /* Nothing to do when there is no fast clear. */
    if (Hardware->features[gcvFEATURE_FAST_CLEAR] == gcvFALSE
        /* Already in a flush - don't recurse! */
    ||  Hardware->MCStates->inFlush
        /* Need to be in 3D pipe. */
    ||  (Hardware->currentPipe != 0x0)
        /* Skip this if the tile status got paused. */
    ||  Hardware->MCStates->paused
    )
    {
        goto OnError;
    }
    /* Mark that we are in a flush. */
    Hardware->MCStates->inFlush = gcvTRUE;

    if (Surface->type == gcvSURF_DEPTH)
    {
        if (Surface != Hardware->PEStates->depthStates.surface)
        {
            current = gcvFALSE;
        }
    }
    else
    {
        if (Hardware->PEStates->colorStates.target[0].surface != Surface)
        {
            current = gcvFALSE;
        }
    }

    /* The surface has not yet been de-tiled and is not the current one,
    ** enable its tile status buffer first.
    */
    if (Decompress)
    {
        if (Surface->type == gcvSURF_DEPTH)
        {
            if (Surface != Hardware->PEStates->depthStates.surface)
            {
                saved = Hardware->PEStates->depthStates.surface;
                Hardware->PEStates->depthStates.surface = Surface;

                gcmGETHARDWAREADDRESS(Surface->tileStatusNode,
                    tileStatusAddress
                    );

                gcmONERROR(
                    gcoHARDWARE_EnableTileStatus(
                    Hardware,
                    Surface,
                    tileStatusAddress,
                    &Surface->hzTileStatusNode,
                        0));

                switchedTileStatus = gcvTRUE;
            }
        }
        else
        {
            if (Hardware->PEStates->colorStates.target[0].surface != Surface)
            {
                saved = Hardware->PEStates->colorStates.target[0].surface;

                Hardware->PEStates->colorStates.target[0].surface = Surface;

                gcmGETHARDWAREADDRESS(Surface->tileStatusNode,
                    tileStatusAddress
                    );

                gcmONERROR(
                    gcoHARDWARE_EnableTileStatus(
                        Hardware,
                        Hardware->PEStates->colorStates.target[0].surface,
                        tileStatusAddress,
                        &Surface->hzTileStatusNode,
                        0));
                switchedTileStatus = gcvTRUE;
            }
        }
    }

    /* If the hardware doesn't have a real flush and the cache is marked as
    ** dirty, we do need to do a partial resolve to make sure the dirty
    ** cache lines are pushed out. */
    if (!Hardware->features[gcvFEATURE_FAST_CLEAR_FLUSH])
    {
        /* Flush the cache the hard way. */
        gcsPOINT origin, size;

        gctBOOL fastClearC = ((((gctUINT32) (Hardware->MCStates->memoryConfig)) >> (0 ?
 1:1) & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 1:1) - (0 ? 1:1) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1)))))));

        /* Setup the origin. */
        origin.x = origin.y = 0;

        /* Setup the size. */
        size.x = 16;
        size.y = 16 * 4 * 2;
        /* Below code can't work with MRT tile status buffer */
        gcmASSERT(Hardware->features[gcvFEATURE_MRT_TILE_STATUS_BUFFER] == gcvFALSE);

        if ((Decompress) &&
            (Surface == Hardware->PEStates->colorStates.target[0].surface) &&
            (Hardware->PEStates->colorStates.target[0].surface->stride >= 256) &&
            (Hardware->PEStates->colorStates.target[0].surface->alignedH < (gctUINT) size.y))
        {
            size.x     = Hardware->PEStates->colorStates.target[0].surface->alignedW;
            size.y     = Hardware->PEStates->colorStates.target[0].surface->alignedH;
            Decompress = gcvFALSE;
        }
        /* Flush the pipe. */
        gcmONERROR(gcoHARDWARE_FlushPipe(Hardware, gcvNULL));

        /* The pipe has been flushed. */
        flushed = gcvTRUE;

        if ((Hardware->PEStates->colorStates.target[0].surface == gcvNULL)                   ||
            (Hardware->PEStates->colorStates.target[0].surface->stride < 256)                ||
            (Hardware->PEStates->colorStates.target[0].surface->alignedH < (gctUINT) size.y) ||
            (!fastClearC))
        {
            gctUINT32 address[3] = {0};
            gctUINT32 physicalBaseAddress = 0;
            gcoSURF target;
            gctUINT32 stride;

            /* Define state buffer variables. */
            gcmDEFINESTATEBUFFER(reserve, stateDelta, memory, reserveSize);

#if !gcdSECURE_USER
            /* Get physical base address. */
            if (Hardware->features[gcvFEATURE_MC20])
            {
                physicalBaseAddress = 0;
            }
            else
            {
                gcmONERROR(gcoOS_GetBaseAddress(gcvNULL, &physicalBaseAddress));
            }
#endif

            /* Save current target. */
            target = Hardware->PEStates->colorStates.target[0].surface;

            /* Construct a temporary surface. */
            if (Hardware->tempSurface == gcvNULL)
            {
                gcmONERROR(gcoSURF_Construct(
                    gcvNULL,
                    64, size.y, 1,
                    gcvSURF_RENDER_TARGET,
                    gcvSURF_A8R8G8B8,
                    gcvPOOL_DEFAULT,
                    &Hardware->tempSurface
                    ));
            }

            /* Lock the surface. */
            gcmONERROR(gcoSURF_Lock(
                Hardware->tempSurface, address, logical
                ));

            /* Stride. */
            gcmONERROR(gcoSURF_GetAlignedSize(
                Hardware->tempSurface, gcvNULL, gcvNULL, (gctINT_PTR) &stride
                ));

            /* Determine the size of the buffer to reserve. */
            reserveSize

                /* Grouped states. */
                = gcmALIGN((1 + 2) * gcmSIZEOF(gctUINT32), 8)

                /* Cache flush. */
                + gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32) * 2, 8)

                /* Grouped resolve states. */
                + gcmALIGN((1 + 5) * gcmSIZEOF(gctUINT32), 8)

                /* Ungrouped resolve states. */
                + gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32) * 3, 8);

            if (!fastClearC)
            {
                reserveSize
                    += gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32) * 2, 8);
            }

            if (target && target->tileStatusNode.pool != gcvPOOL_UNKNOWN)
            {
                reserveSize
                    += gcmALIGN((1 + 2) * gcmSIZEOF(gctUINT32), 8)
                    +  gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32), 8);
            }

            /* Determine the size of the buffer to reserve. */
            gcmBEGINSTATEBUFFER(Hardware, reserve, stateDelta, memory, reserveSize);

            /* Reprogram tile status. */
            {    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)2  <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0596, 2 );
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0596) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};


                gcmGETHARDWAREADDRESS(Hardware->tempSurface->tileStatusNode,
                    tileStatusAddress
                    );

                gcmGETHARDWAREADDRESS(Hardware->tempSurface->node,
                    surfaceAddress
                    );

                gcmSETSTATEDATA(
                    stateDelta, reserve, memory, gcvFALSE, 0x0596,
                      tileStatusAddress
                    + physicalBaseAddress
                    );

                gcmSETSTATEDATA(
                    stateDelta, reserve, memory, gcvFALSE, 0x0597,
                      surfaceAddress
                    + physicalBaseAddress
                    );

                gcmSETFILLER(
                    reserve, memory
                    );

            gcmENDSTATEBATCH(
                reserve, memory
                );

            {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0594, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0594) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
gcmSETCTRLSTATE(stateDelta, reserve, memory, 0x0594, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5))) );gcmENDSTATEBATCH(reserve, memory);
};


            if (!fastClearC)
            {
                {    {    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0595, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0595) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA(stateDelta, reserve, memory, gcvFALSE, 0x0595, ((((gctUINT32) (Hardware->MCStates->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))));     gcmENDSTATEBATCH(reserve, memory);
};

            }

            /* Resolve this buffer on top of itself. */
            {    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)5  <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0581, 5 );
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (5 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0581) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};


                gcmSETCTRLSTATE(
                    stateDelta, reserve, memory, 0x0581,
                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x06 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x06 & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ? 14:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ?
 14:14))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 14:14) - (0 ? 14:14) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14)))
                    );

                gcmSETCTRLSTATE(
                    stateDelta, reserve, memory, 0x0582,
                    address[0]
                    );

                gcmSETCTRLSTATE(
                    stateDelta, reserve, memory, 0x0583,
                    stride << 2
                    );

                gcmSETCTRLSTATE(
                    stateDelta, reserve, memory, 0x0584,
                    address[0]
                    );

                gcmSETCTRLSTATE(
                    stateDelta, reserve, memory, 0x0585,
                    stride << 2
                    );

            gcmENDSTATEBATCH(
                reserve, memory
                );

            {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0588, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0588) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
gcmSETCTRLSTATE(stateDelta, reserve, memory, 0x0588, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (size.x) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ?
 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (size.y) & ((gctUINT32) ((((1 ? 31:16) - (0 ?
 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))) );gcmENDSTATEBATCH(reserve, memory);
};


            {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x058F, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x058F) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
gcmSETCTRLSTATE(stateDelta, reserve, memory, 0x058F, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16))) );gcmENDSTATEBATCH(reserve, memory);
};


            gcmASSERT(!Hardware->features[gcvFEATURE_BLT_ENGINE]);

            {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0580, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0580) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
gcmSETCTRLSTATE(stateDelta, reserve, memory, 0x0580, 0xBADABEEB );
gcmENDSTATEBATCH(reserve, memory);
};


            if (target && target->tileStatusNode.pool != gcvPOOL_UNKNOWN)
            {
                /* Reprogram tile status. */
                {    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)2  <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0596, 2 );
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0596) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};


                    gcmGETHARDWAREADDRESS(target->tileStatusNode, tileStatusAddress);

                    gcmGETHARDWAREADDRESS(target->node, surfaceAddress);

                    gcmSETSTATEDATA(
                        stateDelta, reserve, memory, gcvFALSE, 0x0596,
                          tileStatusAddress + physicalBaseAddress
                        );

                    gcmSETSTATEDATA(
                        stateDelta, reserve, memory, gcvFALSE, 0x0597,
                          surfaceAddress + physicalBaseAddress
                        );

                    gcmSETFILLER(
                        reserve, memory
                        );

                gcmENDSTATEBATCH(
                    reserve, memory
                    );

                {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0594, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0594) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
gcmSETCTRLSTATE(stateDelta, reserve, memory, 0x0594, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5))) );gcmENDSTATEBATCH(reserve, memory);
};

            }

            if (!fastClearC)
            {
                {    {    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0595, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0595) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA(stateDelta, reserve, memory, gcvFALSE, 0x0595, Hardware->MCStates->memoryConfig );
     gcmENDSTATEBATCH(reserve, memory);
};

            }

            /* Invalidate the tile status cache. */
            {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0594, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0594) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
gcmSETCTRLSTATE(stateDelta, reserve, memory, 0x0594, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );gcmENDSTATEBATCH(reserve, memory);
};


            /* Validate the state buffer. */
            gcmENDSTATEBUFFER(Hardware, reserve, memory, reserveSize);

            /* Unlock the surface. */
            gcmONERROR(gcoSURF_Unlock(Hardware->tempSurface, logical[0]));
            logical[0] = gcvNULL;
        }
        else
        {
            gcsSURF_VIEW surfView = {gcvNULL, 0, 1};
            gcsSURF_RESOLVE_ARGS rlvArgs = {0};

            if (Decompress && Hardware->PEStates->colorStates.target[0].surface->dirty && fastClearC)
            {
                /* We need to decompress as well, so resolve the entire
                ** target on top of itself. */
                size.x = Hardware->PEStates->colorStates.target[0].surface->alignedW;
                size.y = Hardware->PEStates->colorStates.target[0].surface->alignedH;
            }

            surfView.surf = Hardware->PEStates->colorStates.target[0].surface;
            rlvArgs.version = gcvHAL_ARG_VERSION_V2;
            rlvArgs.uArgs.v2.srcOrigin = origin;
            rlvArgs.uArgs.v2.dstOrigin = origin;
            rlvArgs.uArgs.v2.rectSize  = size;
            rlvArgs.uArgs.v2.numSlices = 1;

            /* Resolve the current render target onto itself. */
            gcmONERROR(_ResolveRect(Hardware, &surfView, &surfView, &rlvArgs));

            /* The pipe has been flushed by resolve. */
            flushed = gcvTRUE;
        }

        /* The tile status buffer has been flushed */
        flushedTileStatus = gcvTRUE;
    }

    if (!flushed)
    {
        /* Flush the pipe. */
        gcmONERROR(gcoHARDWARE_FlushPipe(Hardware, gcvNULL));
    }

    /* Do we need to explicitly flush?. */
    if (flushedTileStatus ||
        Hardware->features[gcvFEATURE_FC_FLUSH_STALL])
    {
        gcmONERROR(gcoHARDWARE_LoadCtrlState(Hardware,
                                             0x01650,
                                             ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))));
    }
    else
    {
        gcmONERROR(_FlushTileStatusCache(Hardware));
    }

    /* Check for decompression request. */
    if (Decompress && Surface->dirty)
    {
        gcsPOINT origin, size;
        gctUINT sizeAlignment = Hardware->features[gcvFEATURE_TILEFILLER_32TILE_ALIGNED] ? 0x7ff : 0x3fff;

        /* Setup the origin. */
        origin.x = origin.y = 0;

        /* Setup the size. */
        size.x = Surface->alignedW;
        size.y = Surface->alignedH;

        if (Hardware->features[gcvFEATURE_TILE_FILLER] &&
            Hardware->features[gcvFEATURE_FC_FLUSH_STALL] &&
            (!Surface->compressed) &&
            ((Surface->node.size & sizeAlignment) == 0))
        {
            /* Define state buffer variables. */
            gcmDEFINESTATEBUFFER(reserve, stateDelta, memory, reserveSize);

            /* Determine the size of the buffer to reserve. */
            reserveSize = gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32), 8);

            /* Determine the size of the buffer to reserve. */
            gcmBEGINSTATEBUFFER(Hardware, reserve, stateDelta, memory, reserveSize);

            {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0594, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0594) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
gcmSETCTRLSTATE(stateDelta, reserve, memory, 0x0594, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );gcmENDSTATEBATCH(reserve, memory);
};


            stateDelta = stateDelta; /* Keep the compiler happy. */

            /* Validate the state buffer. */
            gcmENDSTATEBUFFER(Hardware, reserve, memory, reserveSize);

            /* Semaphore-stall before next primitive. */
            gcmONERROR(gcoHARDWARE_Semaphore(
                Hardware, gcvWHERE_RASTER, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE,
                gcvNULL
                ));


            /* Fast clear fill. */
            gcmONERROR(gcoHARDWARE_FillFromTileStatus(Hardware, Surface));
        }
        else
        {
            gcsSURF_VIEW surfView = {Surface, 0, 1};
            gcsSURF_RESOLVE_ARGS rlvArgs = {0};

            rlvArgs.version = gcvHAL_ARG_VERSION_V2;
            rlvArgs.uArgs.v2.srcOrigin = origin;
            rlvArgs.uArgs.v2.dstOrigin = origin;
            rlvArgs.uArgs.v2.rectSize  = size;
            rlvArgs.uArgs.v2.numSlices = 1;

            /* Resolve the current RT/depth onto itself */
            if (Hardware->features[gcvFEATURE_BLT_ENGINE])
            {
                /* for blt engine, need original rect param.*/
                rlvArgs.uArgs.v2.rectSize.x = Surface->requestW;
                rlvArgs.uArgs.v2.rectSize.y = Surface->requestH;

                rlvArgs.uArgs.v2.engine = gcvENGINE_RENDER;
                gcmONERROR(gcoHARDWARE_3DBlitBlt(Hardware, &surfView, &surfView, &rlvArgs));
            }
            else
            {
                if (Surface->type == gcvSURF_RENDER_TARGET)
                {
                    gcmONERROR(_ResolveRect(Hardware, &surfView, &surfView, &rlvArgs));
                    gcmONERROR(gcoHARDWARE_Commit(Hardware));
                }
                else if (Surface->type == gcvSURF_DEPTH)
                {
                    gcmONERROR(gcoHARDWARE_ResolveDepth(Hardware, &surfView, &surfView, &rlvArgs));
                }
            }
        }

        Surface->dirty = gcvFALSE;
    }

    /* Semaphore-stall before next primitive. */
    gcmONERROR(gcoHARDWARE_Semaphore(
        Hardware, gcvWHERE_RASTER, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE, gcvNULL
        ));

    /* Tile status cache is no longer dirty. */
    Hardware->MCDirty->cacheDirty = gcvFALSE;

    if (Disable)
    {
        if (Decompress &&
            Surface->type == gcvSURF_DEPTH &&
            Surface->hzNode.pool != gcvPOOL_UNKNOWN &&
            Surface->hzDisabled == gcvFALSE &&
            (((((gctUINT32) (Hardware->MCStates->memoryConfig)) >> (0 ? 12:12)) & ((gctUINT32) ((((1 ? 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1)))))) ))
        {
            Surface->hzDisabled = gcvTRUE;

            /* For current depth, need to toggle dirty for later update. */
            if (current)
            {
                Hardware->PEDirty->depthTargetDirty = gcvTRUE;
                Hardware->PEDirty->depthConfigDirty = gcvTRUE;
            }
        }

        if (current)
        {
            gcmONERROR(gcoHARDWARE_DisableHardwareTileStatus(
                Hardware,
                (Surface->type == gcvSURF_DEPTH) ? gcvTILESTATUS_DEPTH : gcvTILESTATUS_COLOR,
                0));
        }

        /* Disable the tile status for this surface. */
        Surface->tileStatusDisabled = Decompress;
    }

    /*  */
    if (Hardware->features[gcvFEATURE_128BTILE]
        && Surface->tileStatusDisabled)
    {
        Surface->cacheMode = gcvCACHE_NONE;
    }

OnError:
    if (Hardware != gcvNULL)
    {
        if (logical[0] != gcvNULL)
        {
            /* Unlock the surface. */
            gcmVERIFY_OK(gcoSURF_Unlock(Hardware->tempSurface, logical[0]));
        }

        /* Resume tile status if it was paused. */
        if (switchedTileStatus)
        {
            if (Surface->type == gcvSURF_DEPTH)
            {
                Hardware->PEStates->depthStates.surface = saved;
            }
            else
            {
                Hardware->PEStates->colorStates.target[0].surface = saved;
            }

            if (saved)
            {
                if (saved->tileStatusNode.pool != gcvPOOL_UNKNOWN)
                {
                    gcmGETHARDWAREADDRESS(saved->tileStatusNode, tileStatusAddress);
                }
                else
                {
                    tileStatusAddress = 0;
                }

                /* Reprogram the tile status to the current surface. */
                gcmVERIFY_OK(
                    gcoHARDWARE_EnableTileStatus(
                        Hardware,
                        saved,
                        tileStatusAddress,
                        &saved->hzTileStatusNode,
                        0));
            }
            else
            {
                gcmVERIFY_OK(
                    gcoHARDWARE_DisableHardwareTileStatus(
                        Hardware,
                        (Surface->type == gcvSURF_DEPTH) ? gcvTILESTATUS_DEPTH : gcvTILESTATUS_COLOR,
                        0));
            }
        }
        /* Flush finished. */
        Hardware->MCStates->inFlush = gcvFALSE;
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}
gceSTATUS
gcoHARDWARE_FlushTileStatus(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface,
    IN gctBOOL Decompress
    )
{
    return _FlushAndDisableTileStatus(Hardware, Surface, Decompress, gcvFALSE);
}

gceSTATUS
gcoHARDWARE_DisableTileStatus(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface,
    IN gctBOOL CpuAccess
    )
{
    return _FlushAndDisableTileStatus(Hardware, Surface, CpuAccess, gcvTRUE);
}

/*******************************************************************************
**
**  gcoHARDWARE_SetAntiAlias
**
**  Enable or disable anti-aliasing.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctBOOL Enable
**          Enable anti-aliasing when gcvTRUE and disable anti-aliasing when
**          gcvFALSE.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_SetAntiAlias(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Store value. */
    Hardware->MsaaStates->sampleMask      = Enable ? 0xF : 0x0;
    Hardware->MsaaDirty->msaaConfigDirty = gcvTRUE;
    Hardware->SHDirty->shaderDirty |= gcvPROGRAM_STAGE_FRAGMENT_BIT;

    if (Hardware->features[gcvFEATURE_RA_DEPTH_WRITE] &&
        gcoHAL_GetOption(gcvNULL, gcvOPTION_PREFER_RA_DEPTH_WRITE) &&
        !Hardware->features[gcvFEATURE_RA_DEPTH_WRITE_MSAA1X_FIX])
    {
        Hardware->PEDirty->depthConfigDirty = gcvTRUE;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_SetSamples
**
**  Enable or disable anti-aliasing.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsSAMPLER Samples
**          MSAA Samples
**
**  OUTPUT:
**
**      Nothing.
*/

gceSTATUS
gcoHARDWARE_SetSamples(
    IN gcoHARDWARE Hardware,
    IN gcsSAMPLES SampleInfo
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Samples.x=%d, Samples.y=%d", Hardware, SampleInfo.x, SampleInfo.y);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Store value. */
    Hardware->MsaaStates->sampleInfo = SampleInfo;
    Hardware->MsaaDirty->msaaConfigDirty = gcvTRUE;
    Hardware->MsaaDirty->msaaModeDirty = gcvTRUE;
    Hardware->SHDirty->shaderDirty |= gcvPROGRAM_STAGE_FRAGMENT_BIT;


OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  gcoHARDWARE_SetDither
**
**  Enable or disable dithering.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**
**      gctBOOL Enable
**          gcvTRUE to enable dithering or gcvFALSE to disable it.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_SetDither(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->PEStates->ditherEnable != Enable)
    {
        Hardware->PEStates->ditherEnable = Enable;
        Hardware->PEDirty->peDitherDirty = gcvTRUE;
        Hardware->PEDirty->colorConfigDirty = gcvTRUE;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}
#endif

/*******************************************************************************
**
**  gcoHARDWARE_InvalidateCache
**
**  Invalidate the cache.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_InvalidateCache(
    gcoHARDWARE Hardware
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    gcmGETHARDWARE(Hardware);

    gcmONERROR(gcoHARDWARE_FlushPipe(Hardware, gcvNULL));

#if gcdENABLE_3D
    if (Hardware->currentPipe == 0x0)
    {
        if (Hardware->features[gcvFEATURE_FC_FLUSH_STALL])
        {
            /* Define state buffer variables. */
            gcmDEFINESTATEBUFFER(reserve, stateDelta, memory, reserveSize);

            /* Determine the size of the buffer to reserve. */
            reserveSize = gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32), 8);

            /* Determine the size of the buffer to reserve. */
            gcmBEGINSTATEBUFFER(Hardware, reserve, stateDelta, memory, reserveSize);

            {{    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0594, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0594) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
gcmSETCTRLSTATE(stateDelta, reserve, memory, 0x0594, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );gcmENDSTATEBATCH(reserve, memory);
};


            stateDelta = stateDelta; /* Keep the compiler happy. */

            /* Validate the state buffer. */
            gcmENDSTATEBUFFER(Hardware, reserve, memory, reserveSize);
        }
        else
        {
            gcmONERROR(_FlushTileStatusCache(Hardware));
        }
    }
#endif

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_UseSoftware2D
**
**  Sets the software 2D renderer force flag.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctBOOL Enable
**          gcvTRUE to enable using software for 2D or
**          gcvFALSE to use underlying hardware.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_UseSoftware2D(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Set the force flag. */
    Hardware->sw2DEngine = Enable;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

#if gcdENABLE_3D
/*******************************************************************************
**
**  gcoHARDWARE_WriteBuffer
**
**  Write data into the command buffer.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**      gctCONST_POINTER Data
**          Pointer to a buffer that contains the data to be copied.
**
**      IN gctSIZE_T Bytes
**          Number of bytes to copy.
**
**      IN gctBOOL Aligned
**          gcvTRUE if the data needs to be aligned to 64-bit.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_WriteBuffer(
    IN gcoHARDWARE Hardware,
    IN gctCONST_POINTER Data,
    IN gctSIZE_T Bytes,
    IN gctBOOL Aligned
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Data=0x%x Bytes=%d Aligned=%d",
                    Hardware, Data, Bytes, Aligned);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Call hardware. */
    gcmONERROR(gcoBUFFER_Write(Hardware->engine[gcvENGINE_RENDER].buffer, Data, Bytes, Aligned));

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}
#endif

/*******************************************************************************
**
**  gcoHARDWARE_RGB2YUV
**
**  Convert RGB8 color value to YUV color space.
**
**  INPUT:
**
**      gctUINT8 R, G, B
**          RGB color value.
**
**  OUTPUT:
**
**      gctUINT8_PTR Y, U, V
**          YUV color value.
*/
void gcoHARDWARE_RGB2YUV(
    gctUINT8 R,
    gctUINT8 G,
    gctUINT8 B,
    gctUINT8_PTR Y,
    gctUINT8_PTR U,
    gctUINT8_PTR V
    )
{
    gcmHEADER_ARG("R=%d G=%d B=%d",
                    R, G, B);

    *Y = (gctUINT8) (((66 * R + 129 * G +  25 * B + 128) >> 8) +  16);
    *U = (gctUINT8) (((-38 * R -  74 * G + 112 * B + 128) >> 8) + 128);
    *V = (gctUINT8) (((112 * R -  94 * G -  18 * B + 128) >> 8) + 128);

    gcmFOOTER_ARG("*Y=%d *U=%d *V=%d",
                    *Y, *U, *V);
}

/*******************************************************************************
**
**  gcoHARDWARE_YUV2RGB
**
**  Convert YUV color value to RGB8 color space.
**
**  INPUT:
**
**      gctUINT8 Y, U, V
**          YUV color value.
**
**  OUTPUT:
**
**      gctUINT8_PTR R, G, B
**          RGB color value.
*/
void gcoHARDWARE_YUV2RGB(
    gctUINT8 Y,
    gctUINT8 U,
    gctUINT8 V,
    gctUINT8_PTR R,
    gctUINT8_PTR G,
    gctUINT8_PTR B
    )
{
    /* Clamp the input values to the legal ranges. */
    gctINT y = (Y < 16) ? 16 : ((Y > 235) ? 235 : Y);
    gctINT u = (U < 16) ? 16 : ((U > 240) ? 240 : U);
    gctINT v = (V < 16) ? 16 : ((V > 240) ? 240 : V);

    /* Shift ranges. */
    gctINT _y = y - 16;
    gctINT _u = u - 128;
    gctINT _v = v - 128;

    /* Convert to RGB. */
    gctINT r = (298 * _y            + 409 * _v + 128) >> 8;
    gctINT g = (298 * _y - 100 * _u - 208 * _v + 128) >> 8;
    gctINT b = (298 * _y + 516 * _u            + 128) >> 8;

    gcmHEADER_ARG("Y=%d U=%d V=%d",
                    Y, U, V);

    /* Clamp the result. */
    *R = (r < 0)? 0 : (r > 255)? 255 : (gctUINT8) r;
    *G = (g < 0)? 0 : (g > 255)? 255 : (gctUINT8) g;
    *B = (b < 0)? 0 : (b > 255)? 255 : (gctUINT8) b;

    gcmFOOTER_ARG("*R=%d *G=%d *B=%d",
                    *R, *G, *B);
}

#if gcdENABLE_3D
static gceSTATUS
_MultiGPUSync2(
    IN gcoHARDWARE Hardware,
    OUT gctUINT32_PTR *Memory
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%08x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    gcoHARDWARE_Semaphore(Hardware,
                          (Hardware->features[gcvFEATURE_COMMAND_PREFETCH] ?
                            gcvWHERE_COMMAND_PREFETCH : gcvWHERE_COMMAND),
                          (Hardware->features[gcvFEATURE_BLT_ENGINE] ?
                            gcvWHERE_BLT : gcvWHERE_PIXEL),
                          gcvHOW_SEMAPHORE_STALL,
                          (gctPOINTER *)&memory);

    /* Select core 3D 0. */
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
              | gcvCORE_3D_0_MASK;

    memory++;

    gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK);

    /* Send a semaphore token from FE to core 3D 1. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E02, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ?
 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    gcmDUMP(gcvNULL, "#[chip.semaphore 0x%04X]", gcvCORE_3D_1_ID);

    /* Await a semaphore token from core 3D 1. */
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)));

    gcmDUMP(gcvNULL, "#[chip.stall 0x%04X]", gcvCORE_3D_0_ID);

    /* Select core 3D 1. */
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
              | gcvCORE_3D_1_MASK;

    memory++;

    gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_1_MASK);

    /* Send a semaphore token from FE to core 3D 0. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E02, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ?
 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    gcmDUMP(gcvNULL, "#[chip.semaphore 0x%04X]", gcvCORE_3D_0_ID);

    /* Await a semaphore token from GPU 3D_0. */
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)));

    gcmDUMP(gcvNULL, "#[chip.stall 0x%04X]", gcvCORE_3D_1_ID);

    /* Enable all 3D GPUs. */
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
              | gcvCORE_3D_ALL_MASK;

    memory++;

    gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);

    stateDelta = stateDelta;

    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_MultiGPUSync(
    IN gcoHARDWARE Hardware,
    OUT gctUINT32_PTR *Memory
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT coreID;

    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%08x", Hardware);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_ARGUMENT(Hardware->constructType != gcvHARDWARE_2D);

    if (Hardware->config->gpuCoreCount <= 1)
    {
        gcmFOOTER();
        return status;
    }


    if (!Hardware->features[gcvFEATURE_MULTIGPU_SYNC_V2])
    {
        status = _MultiGPUSync2(Hardware, Memory);
        gcmFOOTER();

        return status;
    }

    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    /* Drain the whole pipe for each GPUs */
    gcoHARDWARE_Semaphore(Hardware,
                          (Hardware->features[gcvFEATURE_COMMAND_PREFETCH] ?
                            gcvWHERE_COMMAND_PREFETCH : gcvWHERE_COMMAND),
                          (Hardware->features[gcvFEATURE_BLT_ENGINE] ?
                            gcvWHERE_BLT : gcvWHERE_PIXEL),
                          gcvHOW_SEMAPHORE_STALL,
                          (gctPOINTER *)&memory);

    for (coreID = 0; coreID < Hardware->config->gpuCoreCount; coreID++)
    {
        /* Select ith core 3D. */
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(coreID);
 memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(coreID));
 } };


        /* For 1st Core 3D */
        if (coreID == 0)
        {
            /* Send a semaphore token from FE to next Core 3D. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E02, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (gcmTO_CHIP_ID(coreID)) & ((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ?
 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (gcmTO_CHIP_ID(coreID + 1)) & ((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


             gcmDUMP(gcvNULL, "#[chip.semaphore 0x%04X]", gcmTO_CHIP_ID((coreID + 1)));

            /* Await a semaphore token from next core 3D. */
            *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

            *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (gcmTO_CHIP_ID(coreID + 1)) & ((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (gcmTO_CHIP_ID(coreID)) & ((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)));

            gcmDUMP(gcvNULL, "#[chip.stall 0x%04X]", gcmTO_CHIP_ID(coreID));
        }
        /* For last Core 3D */
        else if (coreID == (Hardware->config->gpuCoreCount - 1))
        {
            /* Await a semaphore token from previous core 3D. */
            *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

            *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (gcmTO_CHIP_ID(coreID - 1)) & ((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (gcmTO_CHIP_ID(coreID)) & ((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)));

            gcmDUMP(gcvNULL, "#[chip.stall 0x%04X]", gcmTO_CHIP_ID(coreID));

            /* Send a semaphore token from FE to previous Core 3D. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E02, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (gcmTO_CHIP_ID(coreID)) & ((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ?
 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (gcmTO_CHIP_ID(coreID - 1)) & ((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


             gcmDUMP(gcvNULL, "#[chip.semaphore 0x%04X]", gcmTO_CHIP_ID((coreID - 1)));
        }
        /* For all other Core 3D sitting in the middle */
        else
        {
            /* Await a semaphore token from previous core 3D. */
            *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

            *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (gcmTO_CHIP_ID(coreID - 1)) & ((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (gcmTO_CHIP_ID(coreID)) & ((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)));

            gcmDUMP(gcvNULL, "#[chip.stall 0x%04X]", gcmTO_CHIP_ID(coreID));

            /* Send a semaphore token from FE to next Core 3D. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E02, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (gcmTO_CHIP_ID(coreID)) & ((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ?
 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (gcmTO_CHIP_ID(coreID + 1)) & ((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


             gcmDUMP(gcvNULL, "#[chip.semaphore 0x%04X]", gcmTO_CHIP_ID((coreID + 1)));

            /* Await a semaphore token from next core 3D. */
            *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

            *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (gcmTO_CHIP_ID(coreID + 1)) & ((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (gcmTO_CHIP_ID(coreID)) & ((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)));

            gcmDUMP(gcvNULL, "#[chip.stall 0x%04X]", gcmTO_CHIP_ID(coreID));

            /* Send a semaphore token from FE to previous Core 3D. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E02, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (gcmTO_CHIP_ID(coreID)) & ((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ?
 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (gcmTO_CHIP_ID(coreID - 1)) & ((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


             gcmDUMP(gcvNULL, "#[chip.semaphore 0x%04X]", gcmTO_CHIP_ID((coreID - 1)));

        }
    }

    /* Enable all 3D GPUs. */
    { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK;
 memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };


    stateDelta = stateDelta;

    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

OnError:
    gcmFOOTER();
    return status;
}

/*
** Flush shader L1 cache to drain write-back buffer
*/
gceSTATUS
gcoHARDWARE_FlushSHL1Cache(
    IN gcoHARDWARE Hardware
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the argments */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Flush the shader L1 cache. */
    gcmONERROR(
       gcoHARDWARE_LoadCtrlState(Hardware, 0x0380C,
           ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:5) - (0 ?
 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
         | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 10:10) - (0 ?
 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 10:10) - (0 ? 10:10) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))
         | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:11) - (0 ?
 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))));


OnError:
    /* Return the status */
    gcmFOOTER();
    return status;
}

gceSTATUS gcoHARDWARE_FlushL2Cache(
    IN gcoHARDWARE Hardware,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    if (Hardware->config->gpuCoreCount > 1)
    {
        gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

        /* Flush the L2 cache. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        stateDelta = stateDelta; /* Keep the compiler happy. */

        gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);
    }
    else
    {
        /* Idle the pipe. */
        gcmONERROR(
            gcoHARDWARE_Semaphore(Hardware,
                                  gcvWHERE_COMMAND,
                                  gcvWHERE_PIXEL,
                                  gcvHOW_SEMAPHORE_STALL,
                                  Memory));

        /* Determine the size of the buffer to reserve. */
        gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

        /* Flush the L2 cache. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0594) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0594, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        stateDelta = stateDelta; /* Keep the compiler happy. */

        /* Validate the state buffer. */
        gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

        /* Idle the pipe (again). */
        gcmONERROR(
            gcoHARDWARE_Semaphore(Hardware,
                                  gcvWHERE_COMMAND,
                                  gcvWHERE_PIXEL,
                                  gcvHOW_SEMAPHORE_STALL,
                                  Memory));

    }

    Hardware->flushL2 = gcvFALSE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}
#endif

/*******************************************************************************
**
**  gcoHARDWARE_SetAutoFlushCycles
**
**  Set the GPU clock cycles after which the idle 2D engine will keep auto-flushing.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**
**      UINT32 Cycles
**          Source color premultiply with Source Alpha.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gcoHARDWARE_SetAutoFlushCycles(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Cycles
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Cycles=%d", Hardware, Cycles);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
    {
        /* LoadState timeout value. */
        gcmONERROR(gcoHARDWARE_Load2DState32(
            Hardware,
            0x00670,
            Cycles
            ));
    }
    else
    {
        /* Auto-flush is not supported by the software renderer. */
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

#if gcdENABLE_3D
/* Resolve depth buffer. */
gceSTATUS
gcoHARDWARE_ResolveDepth(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_VIEW *SrcView,
    IN gcsSURF_VIEW *DstView,
    IN gcsSURF_RESOLVE_ARGS *Args
    )
{
    gcoSURF srcSurf = SrcView->surf;
    gcoSURF dstSurf = DstView->surf;
    gceSURF_TYPE originSurfType = gcvSURF_TYPE_UNKNOWN;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x SrcView=0x%x DstView=0x%x Args=0x%x", Hardware, SrcView, DstView, Args);
    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmDEBUG_VERIFY_ARGUMENT(srcSurf != gcvNULL);

    if (Args->version != gcvHAL_ARG_VERSION_V2)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    originSurfType = srcSurf->type;

#if gcdENABLE_3D
    /* If the tile status is disabled for the surface, just do a normal resolve. */
    if (srcSurf->tileStatusDisabled || (srcSurf->tileStatusNode.pool == gcvPOOL_UNKNOWN))
    {
        /* Use normal resolve. */
        gcmONERROR(gcoHARDWARE_ResolveRect(Hardware, SrcView, DstView, Args));
    }
    else
#endif
    {
        /* Resolve destination surface and disable its tile status buffer . */
        if (!Hardware->MCStates->inFlush)
        {
            gcmONERROR(gcoHARDWARE_DisableTileStatus(Hardware, dstSurf, gcvTRUE));
        }

        if (!Hardware->features[gcvFEATURE_BLT_ENGINE])
        {
            /* fake depth as RT for TS operation */
            srcSurf->type = gcvSURF_RENDER_TARGET;
        }

        /* Determine color format. */
        switch (srcSurf->format)
        {
        case gcvSURF_D24X8:
        case gcvSURF_D24S8:
        case gcvSURF_D16:
        case gcvSURF_S8:
        case gcvSURF_X24S8:
            /* Keep as it, gcoHARDWARE_EnableTileStatus can support it */
            break;
        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        /* Flush the pipe. */
        gcmONERROR(gcoHARDWARE_FlushPipe(Hardware, gcvNULL));

        /* Flush the src tile status. */
        gcmONERROR(gcoHARDWARE_FlushTileStatus(Hardware, srcSurf, gcvFALSE));

        /* Perform the resolve. */
        gcmONERROR(gcoHARDWARE_ResolveRect(Hardware, SrcView, DstView, Args));
    }

OnError:
    /* Restore surface type */
    srcSurf->type = originSurfType;

    /* Return the status. */
    gcmFOOTER();
    return status;
}

#endif

gceSTATUS
gcoHARDWARE_Load2DState32(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctUINT32 Data
    )
{
    /* Call buffered load state to do it. */
    return gcoHARDWARE_Load2DState(Hardware, Address, 1, &Data);

}

gceSTATUS
gcoHARDWARE_Load2DState(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctUINT32 Count,
    IN gctPOINTER Data
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Address=0x%08x Count=%lu Data=0x%08x",
                  Hardware, Address, Count, Data);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

     /* Verify the arguments. */
    if (Hardware->hw2DCmdIndex & 1)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (Hardware->hw2DCmdBuffer != gcvNULL)
    {
        gctUINT32_PTR memory;

        if (Hardware->hw2DCmdSize - Hardware->hw2DCmdIndex < gcmALIGN(1 + Count, 2))
        {
            gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }

        memory = Hardware->hw2DCmdBuffer + Hardware->hw2DCmdIndex;

        /* LOAD_STATE(Count, Address >> 2) */
        memory[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (Count) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (Address >> 2) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

        /* Copy the state buffer. */
        gcoOS_MemCopy(&memory[1], Data, Count * 4);
    }

    Hardware->hw2DCmdIndex += 1 + Count;

    /* Aligned */
    if (Hardware->hw2DCmdIndex & 1)
    {
        Hardware->hw2DCmdIndex += 1;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_2DAppendNop(
    IN gcoHARDWARE Hardware
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    if (Hardware->hw2DCmdIndex & 1)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if ((Hardware->hw2DCmdBuffer != gcvNULL)
        && (Hardware->hw2DCmdSize > Hardware->hw2DCmdIndex))
    {
        gctUINT32_PTR memory = Hardware->hw2DCmdBuffer + Hardware->hw2DCmdIndex;
        gctUINT32 i = 0;

        while (i < Hardware->hw2DCmdSize - Hardware->hw2DCmdIndex)
        {
            /* Append NOP. */
            memory[i++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x03 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));
            i++;
        }

        Hardware->hw2DCmdIndex = Hardware->hw2DCmdSize;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

#if gcdENABLE_3D

/*******************************************************************************
**  gcoHARDWARE_ComputeCentroids
**
**  Compute centroids.
**
**  INPUT:
**
*/
gceSTATUS
gcoHARDWARE_ComputeCentroids(
    IN gcoHARDWARE Hardware,
    IN gctUINT Count,
    IN gctUINT32_PTR SampleCoords,
    OUT gcsCENTROIDS_PTR Centroids
    )
{
    gctUINT i, j, count, sumX, sumY;

    gcmHEADER_ARG("Hardware=0x%08X", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    for (i = 0; i < Count; i += 1)
    {
        /* Zero out the centroid table. */
        gcoOS_ZeroMemory(&Centroids[i], sizeof(gcsCENTROIDS));

        /* Set the value for invalid pixels. */
        if (Hardware->api == gcvAPI_OPENGL)
        {
            Centroids[i].value[0]
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (8) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (8) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)));
        }

        /* Compute all centroids. */
        for (j = 1; j < 16; j += 1)
        {
            /* Initializ sums and count. */
            sumX = sumY = 0;
            count = 0;

            if ((j == 0x7) || (j == 0xB)
            ||  (j == 0xD) || (j == 0xE)
            )
            {
                sumX = sumY = 0x8;
            }
            else
            {
                if (j & 0x1)
                {
                    /* Add sample 1. */
                    sumX += (((((gctUINT32) (SampleCoords[i])) >> (0 ? 3:0)) & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1)))))) );
                    sumY += (((((gctUINT32) (SampleCoords[i])) >> (0 ? 7:4)) & ((gctUINT32) ((((1 ? 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1)))))) );
                    ++count;
                }

                if (j & 0x2)
                {
                    /* Add sample 2. */
                    sumX += (((((gctUINT32) (SampleCoords[i])) >> (0 ? 11:8)) & ((gctUINT32) ((((1 ? 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1)))))) );
                    sumY += (((((gctUINT32) (SampleCoords[i])) >> (0 ? 15:12)) & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1)))))) );
                    ++count;
                }

                if (j & 0x4)
                {
                    /* Add sample 3. */
                    sumX += (((((gctUINT32) (SampleCoords[i])) >> (0 ? 19:16)) & ((gctUINT32) ((((1 ? 19:16) - (0 ? 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1)))))) );
                    sumY += (((((gctUINT32) (SampleCoords[i])) >> (0 ? 23:20)) & ((gctUINT32) ((((1 ? 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1)))))) );
                    ++count;
                }

                if (j & 0x8)
                {
                    /* Add sample 4. */
                    sumX += (((((gctUINT32) (SampleCoords[i])) >> (0 ? 27:24)) & ((gctUINT32) ((((1 ? 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1)))))) );
                    sumY += (((((gctUINT32) (SampleCoords[i])) >> (0 ? 31:28)) & ((gctUINT32) ((((1 ? 31:28) - (0 ? 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1)))))) );
                    ++count;
                }

                /* Compute average. */
                if (count > 0)
                {
                    sumX /= count;
                    sumY /= count;
                }
            }

            switch (j & 3)
            {
            case 0:
                /* Set for centroid 0, 4, 8, or 12. */
                Centroids[i].value[j / 4]
                    = ((((gctUINT32) (Centroids[i].value[j / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (sumX) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
                    | ((((gctUINT32) (Centroids[i].value[j / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (sumY) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)));
                break;

            case 1:
                /* Set for centroid 1, 5, 9, or 13. */
                Centroids[i].value[j / 4]
                    = ((((gctUINT32) (Centroids[i].value[j / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (sumX) & ((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
                    | ((((gctUINT32) (Centroids[i].value[j / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (sumY) & ((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12)));
                break;

            case 2:
                /* Set for centroid 2, 6, 10, or 14. */
                Centroids[i].value[j / 4]
                    = ((((gctUINT32) (Centroids[i].value[j / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (sumX) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
                    | ((((gctUINT32) (Centroids[i].value[j / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (sumY) & ((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)));
                break;

            case 3:
                /* Set for centroid 3, 7, 11, or 15. */
                Centroids[i].value[j / 4]
                    = ((((gctUINT32) (Centroids[i].value[j / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (sumX) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
                    | ((((gctUINT32) (Centroids[i].value[j / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:28) - (0 ? 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28))) | (((gctUINT32) ((gctUINT32) (sumY) & ((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28)));
                break;
            }
        }
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_ProgramResolve
**
**  Program the Resolve offset, Window and then trigger the resolve.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**      gcsPOINT RectSize
**          The size of the rectangular area to be resolved.
**
**      gctBOOL MultiPipe
**         Use two engines to do resolve
**
**      gceMSAA_DOWNSAMPLE_MODE DownsampleMode
**         Downsample mode could average or one sample.
**
**      gctPOINTER *Memory
**          Pointer to external temporary command buffer.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_ProgramResolve(
    IN gcoHARDWARE Hardware,
    IN gcsPOINT RectSize,
    IN gctBOOL  MultiPipe,
    IN gceMSAA_DOWNSAMPLE_MODE DownsampleMode,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status;
    gctUINT32 useSingleResolveEngine = 0;
    gctUINT32 resolveEngines;
    gctBOOL programRScontrol = gcvFALSE;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%08x RectSize=%d,%d, MultiPipe=%d DownsampleMode=%d",
                  Hardware, RectSize.x, RectSize.y, MultiPipe, DownsampleMode);


    if (MultiPipe)
    {
        gcmASSERT(Hardware->config->resolvePipes > 1);
        resolveEngines = Hardware->config->resolvePipes;
    }
    else
    {
        /* Note: When resolving only from 1 engine,
           the rs config register needs to be set to use full_height. */
        resolveEngines = 1;
    }

    /* Make sure RS form a full command buffer */
    gcmASSERT(Memory);

    /* Resolve from either both the pixel pipes or 1 pipe. */
    gcmASSERT((resolveEngines == Hardware->config->resolvePipes)
           || (resolveEngines == 1));

    if ((resolveEngines != Hardware->config->resolvePipes) ||
         Hardware->features[gcvFEATURE_RS_DS_DOWNSAMPLE_NATIVE_SUPPORT])
    {
        /* Resolve engine enable/restore states. */
        useSingleResolveEngine = (resolveEngines == 1);
        programRScontrol = gcvTRUE;
    }

    /* Split the window. */
    if (resolveEngines > 1)
    {
        gcmASSERT((RectSize.y & 7) == 0);
        RectSize.y /= 2;
    }

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    if (Hardware->config->gpuCoreCount > 1)
    {
        /* Flush both caches. */
        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        /* Invalidate the tile status cache. */
        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0594) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        /* Sync the GPUs. */
        gcmONERROR(gcoHARDWARE_MultiGPUSync(Hardware, &memory));

        /* Select core 3D 0 to do the resolve operation. */
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0);
 memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0));
 } };

    }

    /* Append RESOLVE_WINDOW commands. */
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0588) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (RectSize.x) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (RectSize.y) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16)));

    gcmDUMP(gcvNULL,
            "#[state 0x%04X 0x%08X]",
            0x0588, memory[-1]);

    if (resolveEngines == 1)
    {
        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05C0) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:0) - (0 ? 12:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:0) - (0 ? 12:0) + 1))))))) << (0 ?
 12:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 12:0) - (0 ?
 12:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:0) - (0 ? 12:0) + 1))))))) << (0 ?
 12:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:16) - (0 ? 28:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:16) - (0 ? 28:16) + 1))))))) << (0 ?
 28:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 28:16) - (0 ?
 28:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:16) - (0 ? 28:16) + 1))))))) << (0 ?
 28:16)));

         gcmDUMP(gcvNULL,
             "#[state 0x%04X 0x%08X]",
             0x05C0, memory[-1]);

    }
    else
    {

        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05C0) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:0) - (0 ? 12:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:0) - (0 ? 12:0) + 1))))))) << (0 ?
 12:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 12:0) - (0 ?
 12:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:0) - (0 ? 12:0) + 1))))))) << (0 ?
 12:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:16) - (0 ? 28:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:16) - (0 ? 28:16) + 1))))))) << (0 ?
 28:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 28:16) - (0 ?
 28:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:16) - (0 ? 28:16) + 1))))))) << (0 ?
 28:16)));

        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:0) - (0 ? 12:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:0) - (0 ? 12:0) + 1))))))) << (0 ?
 12:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 12:0) - (0 ?
 12:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:0) - (0 ? 12:0) + 1))))))) << (0 ?
 12:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:16) - (0 ? 28:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:16) - (0 ? 28:16) + 1))))))) << (0 ?
 28:16))) | (((gctUINT32) ((gctUINT32) (RectSize.y) & ((gctUINT32) ((((1 ?
 28:16) - (0 ? 28:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:16) - (0 ? 28:16) + 1))))))) << (0 ?
 28:16)));
        gcmDUMP(gcvNULL,
                "#[state 0x%04X 0x%08X]",
                0x05C0, memory[-2]);

        gcmDUMP(gcvNULL,
                "#[state 0x%04X 0x%08X]",
                0x05C0+1, memory[-1]);

        memory++;

    }



    if (programRScontrol)
    {
        /*************************************************************/
        /* Enable Resolve Engine 0. */
        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05AE) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (useSingleResolveEngine) & ((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (DownsampleMode) & ((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1)));

        gcmDUMP(gcvNULL,
                "#[state 0x%04X 0x%08X]",
                0x05AE, memory[-1]);
    }

    gcmASSERT(!Hardware->features[gcvFEATURE_BLT_ENGINE]);

    /* Trigger resolve. */
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0580) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

    *memory++ = 0xBADABEEB;

    gcmDUMP(gcvNULL,
            "#[state 0x%04X 0x%08X]",
            0x0580, memory[-1]);

    if (programRScontrol)
    {
        /*************************************************************/
        /* Enable Resolve Engine 0. */
        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x05AE) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        gcmDUMP(gcvNULL,
                "#[state 0x%04X 0x%08X]",
                0x05AE, memory[-1]);
    }

    if (Hardware->config->gpuCoreCount > 1)
    {
        /* Enable all 3D GPUs. */
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK;
 memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };


        /* Flush both caches. */
        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        /* Invalidate the tile status cache. */
        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0594) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

        *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        /* Sync the GPUs. */
        gcmONERROR(gcoHARDWARE_MultiGPUSync(Hardware, &memory));
    }
    else
    {
        gcmONERROR(gcoHARDWARE_Semaphore(
            Hardware, gcvWHERE_RASTER, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE_STALL, (gctPOINTER *)&memory));
    }

    /* Make compiler happy */
    stateDelta = stateDelta;

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

OnError:
    gcmFOOTER();
    return status;
}
#endif

#if VIVANTE_PROFILER_CONTEXT
gceSTATUS
gcoHARDWARE_GetContext(
    IN gcoHARDWARE Hardware,
    OUT gctUINT32 * Context
    )
{
    gctUINT32 curCoreIndex;
    gcoHAL_GetCurrentCoreIndex(gcvNULL, &curCoreIndex);
    *Context = Hardware->contexts[curCoreIndex];
    return gcvSTATUS_OK;
}
#endif

gceSTATUS
gcoHARDWARE_EnableCounters(
    IN gcoHARDWARE Hardware
    )
{
#if (gcdENABLE_3D || gcdENABLE_2D)
    gctUINT32 data = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 20:20) - (0 ?
 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ?
 20:20))) |
                     ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 16:16) - (0 ?
 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) |
                     ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:19) - (0 ? 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 19:19) - (0 ?
 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ?
 19:19))) |
                     ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:18) - (0 ? 18:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:18) - (0 ? 18:18) + 1))))))) << (0 ?
 18:18))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 18:18) - (0 ?
 18:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:18) - (0 ? 18:18) + 1))))))) << (0 ?
 18:18))) |
                     ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ? 17:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 17:17) - (0 ?
 17:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ?
 17:17))) |
                     ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 22:22) - (0 ?
 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22))) |
                     ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 23:23) - (0 ?
 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) |
                     ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 24:24) - (0 ?
 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) |
                     ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:25) - (0 ? 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:25) - (0 ?
 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ?
 25:25))) |
                     ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 21:21) - (0 ?
 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ?
 21:21)));
    gcoHARDWARE_LoadCtrlState(Hardware, 0x03848, data);
#endif
   return gcvSTATUS_OK;
}

#if VIVANTE_PROFILER_PROBE
gceSTATUS
gcoHARDWARE_ProbeCounter(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 address,
    IN gctUINT32 module,
    IN gctUINT32 counter
    )
{
    gctUINT32 data;

    /* pass address to store counter */
    gcoHARDWARE_LoadState32(Hardware, 0x03870, (gctUINT32)address);

    /* probe counter and write to the address */
    data = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:24) - (0 ?
 25:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ?
 25:24))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 25:24) - (0 ? 25:24) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ? 25:24))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (module) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (counter) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));
    gcoHARDWARE_LoadState32(Hardware, 0x03878, data);

    /* reset counter */
    data = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:24) - (0 ?
 25:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ?
 25:24))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 25:24) - (0 ? 25:24) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ? 25:24))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (module) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (counter) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));
    gcoHARDWARE_LoadState32(Hardware, 0x03878, data);
    return gcvSTATUS_OK;
}
#endif


/**********************************************************************
*********
**
**  gcoHARDWARE_Get2DHardware
**
**  Get the pointer to the current gcoHARDWARE object of this thread.
**
**
**  OUTPUT:
**
**   gco2D * Hardware
**       Pointer to a variable receiving the gcoHARDWARE object pointer.
*/
gceSTATUS
gcoHARDWARE_Get2DHardware(
     OUT gcoHARDWARE * Hardware
     )
{
     gceSTATUS status;
     gcsTLS_PTR tls;

     gcmHEADER();

     /* Verify the arguments. */
     gcmDEBUG_VERIFY_ARGUMENT(Hardware != gcvNULL);

     gcmONERROR(gcoOS_GetTLS(&tls));

     /* Return pointer to the gco2D object. */
     if (gcPLS.hal->separated2D && gcPLS.hal->is3DAvailable)
        *Hardware = tls->hardware2D;
     else
        *Hardware = tls->currentHardware;

     /* Success. */
     gcmFOOTER_ARG("*Hardware=0x%x", *Hardware);
     return gcvSTATUS_OK;

OnError:
     /* Return the status. */
     gcmFOOTER();
     return status;
}


/**********************************************************************
*********
**
**  gcoHARDWARE_Set2DHardware
**
**  Set the gcoHARDWARE object.
**
**  INPUT:
**
**
**   gcoHARDWARE Hardware
**       The gcoHARDWARE object.
**
**  OUTPUT:
**
*/
gceSTATUS
gcoHARDWARE_Set2DHardware(
     IN gcoHARDWARE Hardware
     )
{
    gceSTATUS status;
    gcsTLS_PTR tls;

    gcmHEADER();

    gcmONERROR(gcoOS_GetTLS(&tls));

    /* Set current hardware as requested */
    if (gcPLS.hal->separated2D && gcPLS.hal->is3DAvailable)
    {
       tls->hardware2D = Hardware;
    }
    else
    {
       tls->currentHardware = Hardware;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

#if gcdENABLE_3D
/**********************************************************************
*********
**
**  gcoHARDWARE_Get3DHardware
**
**  Get the pointer to the current gcoHARDWARE object of this thread.
**
**
**  OUTPUT:
**
**   gco3D * Hardware
**       Pointer to a variable receiving the gcoHARDWARE object pointer.
*/
gceSTATUS
gcoHARDWARE_Get3DHardware(
     OUT gcoHARDWARE * Hardware
     )
{
     gceSTATUS status;
     gcsTLS_PTR tls;

     gcmHEADER();

     /* Verify the arguments. */
     gcmDEBUG_VERIFY_ARGUMENT(Hardware != gcvNULL);

     gcmONERROR(gcoOS_GetTLS(&tls));

     /* Return pointer to the gco3D object. */
     *Hardware = tls->currentHardware;

     /* Success. */
     gcmFOOTER_ARG("*Hardware=0x%x", *Hardware);
     return gcvSTATUS_OK;

OnError:
     /* Return the status. */
     gcmFOOTER();
     return status;
}


/**********************************************************************
*********
**
**  gcoHARDWARE_Set3DHardware
**
**  Set the gcoHARDWARE object.
**
**  INPUT:
**
**
**   gcoHARDWARE Hardware
**       The gcoHARDWARE object.
**
**  OUTPUT:
**
*/
gceSTATUS
gcoHARDWARE_Set3DHardware(
     IN gcoHARDWARE Hardware
     )
{
    gceSTATUS status;
    gcsTLS_PTR tls;

    gcmHEADER();

    gcmONERROR(gcoOS_GetTLS(&tls));

    if (tls->currentHardware && tls->currentHardware != Hardware)
    {
        gcmDUMP(gcvNULL,
                "#[end contextid %d, default=%d]",
                tls->currentHardware->context,
                tls->currentHardware->threadDefault);

        gcmONERROR(gcoHARDWARE_Commit(tls->currentHardware));
    }

    /* Set current hardware as requested */
    tls->currentHardware = Hardware;

#if gcdDUMP
    if (Hardware)
    {
        gcmDUMP(gcvNULL,
                "#[start contextid %d, default=%d]",
                Hardware->context,
                Hardware->threadDefault);
    }
#endif

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}


/*
** We must make sure below format definiton is exact same.
** 7:4
** 11:8
** 6:3
** 7:4
*/
gceSTATUS
gcoHARDWARE_QueryCompression(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface,
    OUT gctBOOL *Compressed,
    OUT gctINT32 *CompressedFormat,
    OUT gctINT32 *CompressedDecFormat
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL compressed;
    gctINT32 format = -1;
    gctINT32 decFormat = -1;

    gcmHEADER_ARG("Hardware = 0x%08X, Surface = 0x%08X, Compressed = 0x%08X, CompressedFormat = 0x%08X",
                  Hardware, Surface, Compressed, CompressedFormat);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmVERIFY_ARGUMENT(Compressed != gcvNULL);
    gcmVERIFY_ARGUMENT(CompressedFormat != gcvNULL);

    compressed = Hardware->features[gcvFEATURE_COMPRESSION];

    if (compressed)
    {
        /* Determine color compression format. */
        switch (Surface->format)
        {
        case gcvSURF_D16:
            if (Hardware->features[gcvFEATURE_COMPRESSION_V2] ||
                Hardware->features[gcvFEATURE_COMPRESSION_V3] ||
                Hardware->features[gcvFEATURE_128BTILE] ||
                (Hardware->features[gcvFEATURE_V1_COMPRESSION_Z16_DECOMPRESS_FIX] &&
                 Hardware->features[gcvFEATURE_COMPRESSION_V1]))
            {
                format = 0x8;
            }
            break;

        case gcvSURF_A4R4G4B4:
        case gcvSURF_X4R4G4B4:
            if (!Hardware->features[gcvFEATURE_128BTILE])
            {
                format = 0x0;
            }
            break;

        case gcvSURF_A1R5G5B5:
        case gcvSURF_X1R5G5B5:
            format = 0x1;
            break;

        case gcvSURF_R5G6B5:
            format = 0x2;
            break;

        case gcvSURF_X8R8G8B8:
        case gcvSURF_R8_1_X8R8G8B8:
        case gcvSURF_G8R8_1_X8R8G8B8:
            format = (Hardware->config->chipRevision < 0x4500)
                   ? 0x3
                   : 0x4;
            if((Surface->hints & gcvSURF_DEC) == gcvSURF_DEC)
            {
                decFormat = 0x1;
            }
            break;

        case gcvSURF_A8R8G8B8:
            format = 0x3;
            if((Surface->hints & gcvSURF_DEC) == gcvSURF_DEC)
            {
                decFormat = 0x0;
            }
            break;

        /* Depth will fake as RT when resolve */
        case gcvSURF_D24X8:
            format = 0x6;
            break;

        case gcvSURF_D24S8:
        case gcvSURF_X24S8:
            format = 0x5;
            break;

        case gcvSURF_YUY2:
            if((Surface->hints & gcvSURF_DEC) == gcvSURF_DEC)
            {
                decFormat = 0x4;
            }
            break;

        case gcvSURF_UYVY:
            if((Surface->hints & gcvSURF_DEC) == gcvSURF_DEC)
            {
                decFormat = 0x3;
            }
            break;

        case gcvSURF_AYUV:
            if((Surface->hints & gcvSURF_DEC) == gcvSURF_DEC)
            {
                decFormat = 0x2;
            }
            break;

        case gcvSURF_S8:
            if (Hardware->features[gcvFEATURE_S8_MSAA_COMPRESSION] && Surface->isMsaa)
            {
                format = 0x9;
                break;
            }
        default:
            break;
        }

        if (Surface->type == gcvSURF_DEPTH)
        {
            if ((Surface->format == gcvSURF_S8) &&
                (!Hardware->features[gcvFEATURE_S8_MSAA_COMPRESSION] ||
                (!Surface->isMsaa))
               )
            {
                compressed = gcvFALSE;
            }
            else if (Surface->format == gcvSURF_X24S8 &&
                     Surface->isMsaa &&
                     !Hardware->features[gcvFEATURE_RSBLT_MSAA_DECOMPRESSION])
            {
                compressed = gcvFALSE;
            }
            else if ((Surface->format == gcvSURF_D16) &&
                     (Hardware->features[gcvFEATURE_COMPRESSION_V2]) &&
                     (Surface->fcValue != 0xFFFFFFFF) &&
                     (!Hardware->features[gcvFEATURE_V2_COMPRESSION_Z16_FIX]))
            {
                compressed = gcvFALSE;
            }
        }
        else
        {
            gcmASSERT(Surface->formatInfo.fmtClass != gcvFORMAT_CLASS_DEPTH);

            compressed = ((format != -1) &&
                           (Surface->isMsaa || Hardware->features[gcvFEATURE_COLOR_COMPRESSION]));
        }

        /*
        ** If TX can't support decompressor, prefer to non-compressed as tile filler will help.
        */
        if ((Surface->hints & gcvSURF_CREATE_AS_TEXTURE) &&
            ((!Hardware->features[gcvFEATURE_TX_DECOMPRESSOR] &&
               Hardware->features[gcvFEATURE_TEXTURE_TILE_STATUS_READ]) ||
             (Hardware->swwas[gcvSWWA_1165])))
        {
            compressed = gcvFALSE;
        }

        if (Surface->isMsaa &&
            Hardware->features[gcvFEATURE_COMPRESSION_V2] &&
            !Hardware->features[gcvFEATURE_V2_MSAA_COHERENCY_FIX])
        {
            compressed = gcvFALSE;
        }

        if (Surface->hints & gcvSURF_NO_COMPRESSION)
        {
            compressed = gcvFALSE;
        }
    }

    *Compressed = compressed;
    *CompressedFormat = format;
    *CompressedDecFormat = decFormat;

OnError:
    gcmFOOTER_NO();
    return status;
}

/*********************************************************************************
**
** 3D Blit codes
**
*********************************************************************************/

static gcsVSC_APIS *g_vscAPIs = gcvNULL;

gceSTATUS
gcoHARDWARE_SetCompilerFuncTable(
    IN gcoHARDWARE Hardware,
    IN gcsVSC_APIS *VscAPIs
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%08x VscAPIs=0x%08x", Hardware, VscAPIs);

    g_vscAPIs = VscAPIs;

    gcmFOOTER_NO();
    return status;
}

static gceSTATUS _InitCompilerAPIs(
    gcsHARDWARE_BLITDRAW_PTR blitDraw
    )
{
    gceSTATUS status = gcvSTATUS_OK;

#if !gcdSTATIC_LINK && !defined(__CHROME_OS__)

    static gctCONST_STRING vscDll =
#if defined(__APPLE__)
                    "libVSC.dylib";
#elif defined(WIN32)
                    "libVSC";
#else
                    "libVSC.so";
#endif

    static gctCONST_STRING glslcDll =
#if defined(__APPLE__)
                    "libGLSLC.dylib";
#elif defined(WIN32)
                    "libGLSLC";
#else
                    "libGLSLC.so";
#endif

    gcsVSC_APIS *vscAPIs = &blitDraw->vscAPIs;

    gcmONERROR(gcoOS_LoadLibrary(gcvNULL, vscDll, &blitDraw->vscLib));

    gcmONERROR(gcoOS_LoadLibrary(gcvNULL, glslcDll, &blitDraw->glslcLib));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->glslcLib,
        "gcCompileShader",
        (gctPOINTER*) &vscAPIs->gcCompileShader));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcLinkShaders",
        (gctPOINTER*) &vscAPIs->gcLinkShaders));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcSHADER_Construct",
        (gctPOINTER*) &vscAPIs->gcSHADER_Construct));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcSHADER_AddAttribute",
        (gctPOINTER*) &vscAPIs->gcSHADER_AddAttribute));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcSHADER_AddUniform",
        (gctPOINTER*) &vscAPIs->gcSHADER_AddUniform));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcSHADER_AddOpcode",
        (gctPOINTER*) &vscAPIs->gcSHADER_AddOpcode));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcSHADER_AddOpcodeConditional",
        (gctPOINTER*) &vscAPIs->gcSHADER_AddOpcodeConditional));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcSHADER_AddSourceUniformIndexedFormattedWithPrecision",
        (gctPOINTER*) &vscAPIs->gcSHADER_AddSourceUniformIndexedFormattedWithPrecision));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcSHADER_AddSourceAttribute",
        (gctPOINTER*) &vscAPIs->gcSHADER_AddSourceAttribute));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcSHADER_AddSourceConstant",
        (gctPOINTER*) &vscAPIs->gcSHADER_AddSourceConstant));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcSHADER_AddOutput",
        (gctPOINTER*) &vscAPIs->gcSHADER_AddOutput));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcSHADER_AddOutputLocation",
        (gctPOINTER*) &vscAPIs->gcSHADER_AddOutputLocation));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcSHADER_SetCompilerVersion",
        (gctPOINTER*) &vscAPIs->gcSHADER_SetCompilerVersion));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcSHADER_Pack",
        (gctPOINTER*) &vscAPIs->gcSHADER_Pack));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcSHADER_Destroy",
        (gctPOINTER*) &vscAPIs->gcSHADER_Destroy));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcSHADER_Copy",
        (gctPOINTER*) &vscAPIs->gcSHADER_Copy));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcSHADER_DynamicPatch",
        (gctPOINTER*) &vscAPIs->gcSHADER_DynamicPatch));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcCreateOutputConversionDirective",
        (gctPOINTER*) &vscAPIs->gcCreateOutputConversionDirective));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcCreateInputConversionDirective",
        (gctPOINTER*) &vscAPIs->gcCreateInputConversionDirective));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcHINTS_Destroy",
        (gctPOINTER*) &vscAPIs->gcHINTS_Destroy));

    gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
        blitDraw->vscLib,
        "gcSetGLSLCompiler",
        (gctPOINTER*) &vscAPIs->gcSetGLSLCompiler));

#else

    /* Static link should already be set by client driver */
    if (!g_vscAPIs)
    {
        gcmONERROR(gcvSTATUS_INVALID_REQUEST);
    }

#endif

OnError:
    return status;
}

#if gcdENABLE_APPCTXT_BLITDRAW
static gceSTATUS _InitDefaultState(
    gcoHARDWARE Hardware
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i = 0;
    gctINT j = 0;
    gcoOS_ZeroMemory(Hardware->FEStates, gcmSIZEOF(gcsFESTATES));
    gcoOS_ZeroMemory(Hardware->PAAndSEStates, gcmSIZEOF(gcsPAANDSESTATES));
    gcoOS_ZeroMemory(Hardware->MsaaStates, gcmSIZEOF(gcsMSAASTATES));
    gcoOS_ZeroMemory(Hardware->SHStates, gcmSIZEOF(gcsSHSTATES));
    gcoOS_ZeroMemory(Hardware->PEStates, gcmSIZEOF(gcsPESTATES));
    gcoOS_ZeroMemory(Hardware->TXStates, gcmSIZEOF(gcsTXSTATES));
    gcoOS_ZeroMemory(Hardware->MCStates, gcmSIZEOF(gcsMCSTATES));
    gcoOS_ZeroMemory(Hardware->QUERYStates, gcmSIZEOF(gcsQUERYSTATES));
    gcoOS_ZeroMemory(Hardware->XFBStates, gcmSIZEOF(gcsXFBSTATES));

    /* Reset all the dirty to make sure the next time, it will flush the right states. */
    /* FE dirty reset. */
    if (Hardware->FEStates->indexHeadAddress)
    {
        Hardware->FEDirty->indexHeadDirty = gcvTRUE;
    }

    if (Hardware->FEStates->indexTailAddress)
    {
        Hardware->FEDirty->indexTailDirty = gcvTRUE;
    }
    Hardware->FEDirty->indexDirty = gcvTRUE;
    /* PA and SE dirty reset. */
    Hardware->PAAndSEDirty->paConfigDirty = Hardware->PAAndSEDirty->paLineDirty
                                          = Hardware->PAAndSEDirty->scissorDirty
                                          = Hardware->PAAndSEDirty->viewportDirty = gcvTRUE;
    /* Msaa dirty reset. */
    Hardware->MsaaDirty->centroidsDirty = Hardware->MsaaDirty->msaaConfigDirty = Hardware->MsaaDirty->msaaModeDirty = gcvTRUE;
    /* SH dirty reset. */
    Hardware->SHDirty->shaderDirty |= (gcvPROGRAM_STAGE_VERTEX_BIT | gcvPROGRAM_STAGE_FRAGMENT_BIT);

    /* PE dirty reset. */
    Hardware->PEDirty->alphaDirty = Hardware->PEDirty->colorConfigDirty
                                  = Hardware->PEDirty->colorTargetDirty
                                  = Hardware->PEDirty->depthConfigDirty
                                  = Hardware->PEDirty->depthNormalizationDirty
                                  = Hardware->PEDirty->depthRangeDirty
                                  = Hardware->PEDirty->depthTargetDirty
                                  = Hardware->PEDirty->peDitherDirty
                                  = Hardware->PEDirty->stencilDirty = gcvTRUE;

    /* Texture dirty reset. */
    Hardware->TXDirty->hwTxSamplerModeDirty = Hardware->TXDirty->hwTxSamplerSizeDirty
                                            = Hardware->TXDirty->hwTxSamplerSizeLogDirty
                                            = Hardware->TXDirty->hwTxSampler3DDirty
                                            = Hardware->TXDirty->hwTxSamplerLODDirty
                                            = Hardware->TXDirty->hwTxSamplerBaseLODDirty
                                            = Hardware->TXDirty->hwTxSamplerYUVControlDirty
                                            = Hardware->TXDirty->hwTxSamplerYUVStrideDirty
                                            = Hardware->TXDirty->hwTxSamplerLinearStrideDirty
                                            = Hardware->TXDirty->hwTxSamplerSizeLogExtDirty
                                            = Hardware->TXDirty->hwTxSampler3DExtDirty
                                            = Hardware->TXDirty->hwTxSamplerLodExtDirty
                                            = Hardware->TXDirty->hwTxSamplerLodBiasExtDirty
                                            = Hardware->TXDirty->hwTxSamplerAnisoCtrlDirty
                                            = Hardware->TXDirty->hwTxSamplerConfig3Dirty
                                            = Hardware->TXDirty->hwTxSamplerDirty = 0xFFFFFFFF;

    gcsBITMASK_InitAllOne(&Hardware->TXDirty->hwSamplerControl0Dirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&Hardware->TXDirty->hwSamplerControl1Dirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&Hardware->TXDirty->hwSamplerLodMinMaxDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&Hardware->TXDirty->hwSamplerLODBiasDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&Hardware->TXDirty->hwSamplerAnisCtrlDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&Hardware->TXDirty->hwTxDescAddressDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&Hardware->TXDirty->hwTxDescDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&Hardware->TXDirty->hwTextureControlDirty, gcdTXDESCRIPTORS);

    Hardware->TXDirty->hwTxDirty = Hardware->TXDirty->textureDirty
                                 = Hardware->TXDirty->hwTxFlushPS
                                 = Hardware->TXDirty->hwTxFlushVS = gcvTRUE;
    gcoOS_MemFill(Hardware->TXDirty->hwTxSamplerAddressDirty, 0xFF, gcmSIZEOF(Hardware->TXDirty->hwTxSamplerAddressDirty));
    gcoOS_MemFill(Hardware->TXDirty->hwTxSamplerASTCDirty, 0xFF, gcmSIZEOF(Hardware->TXDirty->hwTxSamplerASTCDirty));

    /* MC dirty reset. */
    Hardware->MCDirty->hwTxSamplerTSDirty = Hardware->MCDirty->texTileStatusSlotDirty = 0xFF;
    Hardware->MCDirty->cacheDirty= gcvTRUE;

    /* Query dirty reset. */
    Hardware->QUERYDirty->queryDirty[gcvQUERY_OCCLUSION] = gcvTRUE;

    if (Hardware->features[gcvFEATURE_HW_TFB])
    {
        Hardware->QUERYDirty->queryDirty[gcvQUERY_XFB_WRITTEN] = gcvTRUE;
        Hardware->QUERYDirty->queryDirty[gcvQUERY_PRIM_GENERATED] = gcvTRUE;
        /* XFB dirty reset. */
        Hardware->XFBDirty->xfbDirty = 0xFFFFFFFF;
    }

#if gcdENABLE_3D
    Hardware->PEStates->stencilStates.mode = gcvSTENCIL_NONE;
    Hardware->PEStates->stencilEnabled     = gcvFALSE;

    Hardware->PEStates->earlyDepth = gcvFALSE;

    Hardware->SHStates->rtneRounding = gcvFALSE;

    /* Disable anti-alias. */
    Hardware->MsaaStates->sampleMask   = 0xF;
    Hardware->MsaaStates->sampleEnable = 0xF;
    Hardware->MsaaStates->sampleInfo.x = 1;
    Hardware->MsaaStates->sampleInfo.y = 1;

    /* Table for disable dithering. */
    Hardware->PEStates->ditherTable[0][0] = Hardware->PEStates->ditherTable[0][1] = ~0U;
    /* Table for enable dithering. */
    Hardware->PEStates->ditherTable[1][0] = 0x6E4CA280;
    Hardware->PEStates->ditherTable[1][1] = 0x5D7F91B3;
    Hardware->PEDirty->peDitherDirty = gcvTRUE;
    Hardware->PEStates->ditherEnable = gcvFALSE;

    Hardware->MsaaStates->MsaaFragmentOp = 0;

    /***************************************************************************
    ** Determine multi-sampling constants.
    */

    /* 4x MSAA jitter index. */

#if gcdGRID_QUALITY == 0

    /* Square only. */
    Hardware->MsaaStates->jitterIndex
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:2) - (0 ?
 3:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ?
 3:2))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 3:2) - (0 ?
 3:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ?
 3:2)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:4) - (0 ?
 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 5:4) - (0 ?
 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:6) - (0 ?
 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ?
 7:6))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 7:6) - (0 ?
 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ?
 7:6)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:8) - (0 ?
 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 9:8) - (0 ?
 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:10) - (0 ?
 11:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 11:10) - (0 ?
 11:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ?
 11:10)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 13:12) - (0 ?
 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 13:12) - (0 ?
 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ?
 13:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:14) - (0 ?
 15:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 15:14) - (0 ?
 15:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ?
 15:14)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 17:16) - (0 ?
 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 17:16) - (0 ?
 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:18) - (0 ?
 19:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 19:18) - (0 ?
 19:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ?
 19:18)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ?
 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 21:20) - (0 ?
 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:22) - (0 ?
 23:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ?
 23:22))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 23:22) - (0 ?
 23:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ?
 23:22)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:24) - (0 ?
 25:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ?
 25:24))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 25:24) - (0 ?
 25:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ?
 25:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:26) - (0 ?
 27:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 27:26) - (0 ?
 27:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ?
 27:26)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 29:28) - (0 ?
 29:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 29:28) - (0 ?
 29:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:30) - (0 ?
 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 31:30) - (0 ?
 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));

#elif gcdGRID_QUALITY == 1

    /* No jitter. */
    Hardware->MsaaStates->jitterIndex = 0;

#else

    /* Rotated diamonds. */
    Hardware->MsaaStates->jitterIndex
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:2) - (0 ?
 3:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ?
 3:2))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 3:2) - (0 ?
 3:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ?
 3:2)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:4) - (0 ?
 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 5:4) - (0 ?
 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:6) - (0 ?
 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ?
 7:6))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 7:6) - (0 ?
 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ?
 7:6)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:8) - (0 ?
 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 9:8) - (0 ?
 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:10) - (0 ?
 11:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 11:10) - (0 ?
 11:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ?
 11:10)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 13:12) - (0 ?
 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 13:12) - (0 ?
 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ?
 13:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:14) - (0 ?
 15:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 15:14) - (0 ?
 15:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ?
 15:14)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 17:16) - (0 ?
 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 17:16) - (0 ?
 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:18) - (0 ?
 19:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 19:18) - (0 ?
 19:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ?
 19:18)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ?
 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 21:20) - (0 ?
 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:22) - (0 ?
 23:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ?
 23:22))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 23:22) - (0 ?
 23:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ?
 23:22)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:24) - (0 ?
 25:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ?
 25:24))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:24) - (0 ?
 25:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ?
 25:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:26) - (0 ?
 27:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 27:26) - (0 ?
 27:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ?
 27:26)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 29:28) - (0 ?
 29:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 29:28) - (0 ?
 29:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:30) - (0 ?
 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 31:30) - (0 ?
 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));

#endif


    /* 2x MSAA sample coordinates. */

    Hardware->MsaaStates->sampleCoords2
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28)));


    /* 4x MSAA sample coordinates.
    **
    **                        1 1 1 1 1 1                        1 1 1 1 1 1
    **    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  0| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  1| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  2| | | | | | |X| | | | | | | | | |  | | | | | | | | | | |X| | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  3| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  4| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  5| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  6| | | | | | | | | | | | | | |X| |  | | |X| | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  7| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  8| | | | | | | | |o| | | | | | | |  | | | | | | | | |o| | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  9| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 10| | |X| | | | | | | | | | | | | |  | | | | | | | | | | | | | | |X| |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 11| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 12| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 13| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 14| | | | | | | | | | |X| | | | | |  | | | | | | |X| | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 15| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */

    /* Diamond. */
    Hardware->MsaaStates->sampleCoords4[0]
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (6) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (14) & ((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (6) & ((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28))) | (((gctUINT32) ((gctUINT32) (14) & ((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28)));

    /* Mirrored diamond. */
    Hardware->MsaaStates->sampleCoords4[1]
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (6) & ((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (14) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (6) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28))) | (((gctUINT32) ((gctUINT32) (14) & ((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28)));

    /* Square. */
    Hardware->MsaaStates->sampleCoords4[2]
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28)));


    /* Compute centroids. */
    gcmONERROR(gcoHARDWARE_ComputeCentroids(
        Hardware,
        1, &Hardware->MsaaStates->sampleCoords2, &Hardware->MsaaStates->centroids2
        ));

    gcmONERROR(gcoHARDWARE_ComputeCentroids(
        Hardware,
        3, Hardware->MsaaStates->sampleCoords4, Hardware->MsaaStates->centroids4
        ));

    /* Default depth near and far plane. */
    Hardware->PEStates->depthNear.f = 0.0f;
    Hardware->PEStates->depthFar.f  = 1.0f;
    /* Initialize variables for bandwidth optimization. */
    for (i = 0; i < gcdMAX_DRAW_BUFFERS; ++i)
    {
        Hardware->PEStates->colorStates.target[i].colorWrite = 0xF;
    }
    Hardware->PEStates->colorStates.colorCompression = gcvFALSE;
    Hardware->PEStates->colorStates.destinationRead  = ~0U;
    Hardware->PEStates->colorStates.rop              = 0xC;

    /*Set a invalid value when init*/
    *(gctUINT32 *)&(Hardware->PEStates->alphaStates.floatReference) = 0xFFFFFFFF;

    Hardware->QUERYStates->queryStatus[gcvQUERY_OCCLUSION] =
    Hardware->QUERYStates->statusInCmd[gcvQUERY_OCCLUSION] = gcvQUERY_Disabled;
    Hardware->QUERYStates->queryCmd[gcvQUERY_OCCLUSION] = gcvQUERYCMD_INVALID;

    if (Hardware->features[gcvFEATURE_HW_TFB])
    {
        Hardware->XFBStates->status =
        Hardware->XFBStates->statusInCmd = gcvXFB_Disabled;

        Hardware->QUERYStates->queryStatus[gcvQUERY_XFB_WRITTEN] =
        Hardware->QUERYStates->statusInCmd[gcvQUERY_XFB_WRITTEN] = gcvQUERY_Disabled;
        Hardware->QUERYStates->queryStatus[gcvQUERY_PRIM_GENERATED] =
        Hardware->QUERYStates->statusInCmd[gcvQUERY_PRIM_GENERATED] = gcvQUERY_Disabled;
        Hardware->QUERYStates->queryCmd[gcvQUERY_PRIM_GENERATED] =
        Hardware->QUERYStates->queryCmd[gcvQUERY_XFB_WRITTEN] = gcvQUERYCMD_INVALID;
        Hardware->XFBStates->cmd = gcvXFBCMD_INVALID;
    }
#endif

    for (i = 0; i < 4; i++)
    {
        Hardware->SHStates->psOutputMapping[i] = i;
    }

     /***********************************************************************
    ** Initialize texture states.
    */

    for (i = 0; i < gcdSAMPLERS; i += 1)
    {
        Hardware->TXStates->hwTxSamplerMode[i]            = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxSamplerModeEx[i]          = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxSamplerMode2[i]           = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxSamplerSize[i]            = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxSamplerSizeLog[i]         = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxSampler3D[i]              = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxSamplerLOD[i]             = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxSamplerBaseLOD[i]         = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxSamplerYUVControl[i]      = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxSamplerYUVStride[i]       = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxSamplerLinearStride[i]    = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxSamplerSizeLogExt[i]      = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxSampler3DExt[i]           = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxSamplerLodExt[i]          = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxSamplerLodBiasExt[i]      = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxSamplerAnisoCtrl[i]       = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxSamplerConfig3[i]         = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxSamplerSliceSize[i]       = gcvINVALID_VALUE;

        for (j = 0; j < gcdLOD_LEVELS; j += 1)
        {
            Hardware->TXStates->hwTxSamplerAddress[j][i]  = gcvINVALID_VALUE;
            Hardware->TXStates->hwTxSamplerASTCSize[j][i] = gcvINVALID_VALUE;
            Hardware->TXStates->hwTxSamplerASTCSRGB[j][i] = gcvINVALID_VALUE;
        }
    }

    for (i = 0; i < gcdSAMPLER_TS; i += 1)
    {
        Hardware->MCStates->hwTXSampleTSConfig[i]          = gcvINVALID_VALUE;
        Hardware->MCStates->hwTXSampleTSBuffer[i]          = gcvINVALID_VALUE;
        Hardware->MCStates->hwTXSampleTSClearValue[i]      = gcvINVALID_VALUE;
        Hardware->MCStates->hwTXSampleTSClearValueUpper[i] = gcvINVALID_VALUE;
        Hardware->MCStates->hwTxSamplerTxBaseBuffer[i]     = gcvINVALID_VALUE;

        Hardware->MCStates->texTileStatusSlotUser[i]       = -1;
    }

    /* Intialized texture descriptor states */
    for (i = 0; i < gcdTXDESCRIPTORS; i++)
    {
        Hardware->TXStates->hwSamplerControl0[i] = gcvINVALID_VALUE;
        Hardware->TXStates->hwSamplerControl1[i] = gcvINVALID_VALUE;
        Hardware->TXStates->hwSamplerLodMinMax[i] = gcvINVALID_VALUE;
        Hardware->TXStates->hwSamplerLODBias[i] = gcvINVALID_VALUE;
        Hardware->TXStates->hwSamplerAnisCtrl[i] = gcvINVALID_VALUE;
        Hardware->TXStates->hwTxDescAddress[i] = gcvINVALID_ADDRESS;
        Hardware->TXStates->hwTextureControl[i] = gcvINVALID_ADDRESS;
        Hardware->MCStates->texHasTileStatus[i] = gcvFALSE;
        Hardware->MCStates->texBaseLevelSurfInfoWithTS[i] = gcvNULL;
        Hardware->MCStates->texTileStatusSlotIndex[i] = -1;
    }

OnError:
    return status;
}
#endif

static gceSTATUS _InitBlitDraw(
    gcoHARDWARE Hardware
    )
{
    gceSTATUS status = gcvSTATUS_OK;

#if !gcdENABLE_APPCTXT_BLITDRAW
    if (!Hardware->threadDefault)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }
#endif

    if (!Hardware->blitDraw)
    {
        gcsHARDWARE_BLITDRAW_PTR blitDraw = gcvNULL;

        gcmONERROR(gcoHARDWARE_Initialize3D(Hardware));
        gcmONERROR(gcoHARDWARE_SetAPI(Hardware, gcvAPI_OPENGL_ES30));

        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                  gcmSIZEOF(gcsHARDWARE_BLITDRAW),
                                  (gctPOINTER*)&blitDraw));

        gcoOS_ZeroMemory(blitDraw, gcmSIZEOF(gcsHARDWARE_BLITDRAW));

        gcmONERROR(_InitCompilerAPIs(blitDraw));

        /* Construct a cacheable stream. */
        gcmONERROR(gcoSTREAM_Construct(gcvNULL, &blitDraw->dynamicStream));

        if (Hardware->features[gcvFEATURE_TX_DESCRIPTOR])
        {
            blitDraw->descCurIndex = -1;
        }

        Hardware->blitDraw = blitDraw;
    }

OnError:
    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(_DestroyBlitDraw(Hardware));
    }
    return status;
}

gceSTATUS _DestroyBlitDraw(
    gcoHARDWARE Hardware
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsHARDWARE_BLITDRAW_PTR blitDraw;
    gcsVSC_APIS *vscAPIs;
    gctINT32 i, j;

#if gcdSTATIC_LINK || defined(__CHROME_OS__)
    vscAPIs = g_vscAPIs;
#else
    vscAPIs = &Hardware->blitDraw->vscAPIs;
#endif

    if (!Hardware->threadDefault)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    blitDraw = Hardware->blitDraw;

    if (!blitDraw || !vscAPIs)
    {
        return gcvSTATUS_OK;
    }

    for (i = 0; i < gcvBLITDRAW_NUM_TYPE; i++)
    {
        for (j = 0; j < gcmMAX_VARIATION_NUM; j++)
        {
            gcsPROGRAM_STATE_VARIATION *temp = &blitDraw->programState[i][j];
            if (temp->programState.hints)
            {
                (*vscAPIs->gcHINTS_Destroy)(temp->programState.hints);
                gcmOS_SAFE_FREE(gcvNULL, temp->programState.hints);
                temp->programState.hints = gcvNULL;
            }

            if (temp->programState.stateBuffer)
            {
                gcmOS_SAFE_FREE(gcvNULL, temp->programState.stateBuffer);
                temp->programState.stateBuffer = gcvNULL;
            }
        }

        if (blitDraw->psShader[i])
        {
            (*vscAPIs->gcSHADER_Destroy)(blitDraw->psShader[i]);
            blitDraw->psShader[i] = gcvNULL;
        }

        if (blitDraw->vsShader[i])
        {
            (*vscAPIs->gcSHADER_Destroy)(blitDraw->vsShader[i]);
            blitDraw->vsShader[i] = gcvNULL;
        }
    }

    if (blitDraw->dynamicStream != gcvNULL)
    {
        /* Destroy the dynamic stream. */
        gcmVERIFY_OK(gcoSTREAM_Destroy(blitDraw->dynamicStream));
        blitDraw->dynamicStream = gcvNULL;
    }

#if !gcdSTATIC_LINK && !defined(__CHROME_OS__)
    if (blitDraw->vscLib)
    {
        gcmVERIFY_OK(gcoOS_FreeLibrary(gcvNULL, blitDraw->vscLib));
    }

    if (blitDraw->glslcLib)
    {
        gcmVERIFY_OK(gcoOS_FreeLibrary(gcvNULL, blitDraw->glslcLib));
    }
#endif

    gcoHAL_FreeTXDescArray(blitDraw->descArray, blitDraw->descCurIndex);

    blitDraw->descCurIndex = -1;

    gcmOS_SAFE_FREE(gcvNULL, blitDraw);

    Hardware->blitDraw = gcvNULL;

OnError:
    return status;
}

static gceSTATUS _InitBlitProgram(
    gcoHARDWARE         Hardware,
    gceBLITDRAW_TYPE    type,
    gctBOOL             NeedRecompile
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsHARDWARE_BLITDRAW_PTR blitDraw = gcvNULL;
    gctUINT32 compilerVersion = NeedRecompile ? gcmCC('\0', '\0', '\0', '\3') : gcmCC('\0', '\0', '\1', '\1');
    gctUINT32 vsShaderVersion[2] = {(gcmCC('E', 'S', '\0', '\0') |  gcSHADER_TYPE_VERTEX << 16), compilerVersion};
    gctUINT32 psShaderVersion[2] = {(gcmCC('E', 'S', '\0', '\0') |  gcSHADER_TYPE_FRAGMENT << 16), compilerVersion};
    const gctSTRING outputColorName = NeedRecompile ? "Color" : "#Color";
    gcsVSC_APIS *vscAPIs;

#if gcdSTATIC_LINK || defined(__CHROME_OS__)
    vscAPIs = g_vscAPIs;
#else
    vscAPIs = &Hardware->blitDraw->vscAPIs;
#endif

    gcmASSERT(Hardware->blitDraw);

    blitDraw = Hardware->blitDraw;

    if (!vscAPIs)
    {
        gcmONERROR(gcvSTATUS_INVALID_REQUEST);
    }

    /* Initial blit program */
    if ((type == gcvBLITDRAW_BLIT) && (blitDraw->vsShader[gcvBLITDRAW_BLIT] == gcvNULL))
    {
        gcmONERROR((*vscAPIs->gcSHADER_Construct)(gcvNULL, gcSHADER_TYPE_VERTEX, &blitDraw->vsShader[gcvBLITDRAW_BLIT]));
        do
        {
            gcATTRIBUTE pos;
            gcATTRIBUTE texCoord;

            gcSHADER blitVSShader = blitDraw->vsShader[gcvBLITDRAW_BLIT];
            gcmONERROR((*vscAPIs->gcSHADER_AddAttribute)(blitVSShader, "in_position", gcSHADER_FLOAT_X3, 1, gcvFALSE, gcSHADER_SHADER_DEFAULT, gcSHADER_PRECISION_HIGH, &pos));
            gcmONERROR((*vscAPIs->gcSHADER_AddAttribute)(blitVSShader, "in_texCoord", gcSHADER_FLOAT_X2, 1, gcvFALSE, gcSHADER_SHADER_DEFAULT, gcSHADER_PRECISION_HIGH, &texCoord));
            gcmONERROR((*vscAPIs->gcSHADER_AddOpcode)(blitVSShader, gcSL_MOV, 1, gcSL_ENABLE_XYZ, gcSL_FLOAT, pos->precision));
            gcmONERROR((*vscAPIs->gcSHADER_AddSourceAttribute)(blitVSShader, pos, gcSL_SWIZZLE_XYZZ, 0));
            gcmONERROR((*vscAPIs->gcSHADER_AddOpcode)(blitVSShader, gcSL_MOV, 1, gcSL_ENABLE_W, gcSL_FLOAT, gcSHADER_PRECISION_HIGH));
            gcmONERROR((*vscAPIs->gcSHADER_AddSourceConstant)(blitVSShader, 1.0));
            gcmONERROR((*vscAPIs->gcSHADER_AddOutput)(blitVSShader, "#Position", gcSHADER_FLOAT_X4, 1, 1, gcSHADER_PRECISION_HIGH));
            gcmONERROR((*vscAPIs->gcSHADER_AddOpcode)(blitVSShader, gcSL_MOV, 2, gcSL_ENABLE_XY, gcSL_FLOAT, texCoord->precision));
            gcmONERROR((*vscAPIs->gcSHADER_AddSourceAttribute)(blitVSShader, texCoord, gcSL_SWIZZLE_XYYY, 0));
            gcmONERROR((*vscAPIs->gcSHADER_AddOutput)(blitVSShader, "vTexCoord", gcSHADER_FLOAT_X2, 1, 2, gcSHADER_PRECISION_HIGH));
            gcmONERROR((*vscAPIs->gcSHADER_Pack)(blitVSShader));

            gcmONERROR((*vscAPIs->gcSHADER_SetCompilerVersion)(blitVSShader, vsShaderVersion));
        } while(gcvFALSE);
    }

    if ((type == gcvBLITDRAW_BLIT) && (blitDraw->psShader[gcvBLITDRAW_BLIT] == gcvNULL))
    {
        gcATTRIBUTE texcoord0;

        gcmONERROR((*vscAPIs->gcSHADER_Construct)(gcvNULL, gcSHADER_TYPE_FRAGMENT, &blitDraw->psShader[gcvBLITDRAW_BLIT]));
        do
        {
            gcSHADER blitPSShader = blitDraw->psShader[gcvBLITDRAW_BLIT];
            gcmONERROR((*vscAPIs->gcSHADER_AddAttribute)(blitPSShader, "vTexCoord", gcSHADER_FLOAT_X2, 1, gcvTRUE, gcSHADER_SHADER_DEFAULT, gcSHADER_PRECISION_HIGH, &texcoord0));
            gcmONERROR((*vscAPIs->gcSHADER_AddUniform)(blitPSShader, "unit0", gcSHADER_SAMPLER_2D, 1, gcSHADER_PRECISION_HIGH, &blitDraw->bliterSampler));

            gcmONERROR((*vscAPIs->gcSHADER_AddOpcodeConditional)(blitPSShader, gcSL_KILL, gcSL_LESS, 0));
            gcmONERROR((*vscAPIs->gcSHADER_AddSourceAttribute)(blitPSShader, texcoord0, gcSL_SWIZZLE_XYYY, 0));
            gcmONERROR((*vscAPIs->gcSHADER_AddSourceConstant)(blitPSShader, 0.0f));
            gcmONERROR((*vscAPIs->gcSHADER_AddOpcodeConditional)(blitPSShader, gcSL_KILL, gcSL_GREATER, 0));
            gcmONERROR((*vscAPIs->gcSHADER_AddSourceAttribute)(blitPSShader, texcoord0, gcSL_SWIZZLE_XYYY, 0));
            gcmONERROR((*vscAPIs->gcSHADER_AddSourceConstant)(blitPSShader, 1.0f));

            gcmONERROR((*vscAPIs->gcSHADER_AddOpcode)(blitPSShader, gcSL_TEXLD, 1, gcSL_ENABLE_XYZW, gcSL_FLOAT, blitDraw->bliterSampler->precision));
            gcmONERROR((*vscAPIs->gcSHADER_AddSourceUniformIndexedFormattedWithPrecision)(
                blitPSShader,
                blitDraw->bliterSampler,
                gcSL_SWIZZLE_XYZW,
                0,
                gcSL_NOT_INDEXED,
                gcSL_NONE_INDEXED,
                0,
                gcSL_FLOAT,
                gcSHADER_PRECISION_HIGH));
            gcmONERROR((*vscAPIs->gcSHADER_AddSourceAttribute)(blitPSShader, texcoord0, gcSL_SWIZZLE_XYYY, 0));
            gcmONERROR((*vscAPIs->gcSHADER_AddOutput)(blitPSShader, outputColorName, gcSHADER_FLOAT_X4, 1, 1, gcSHADER_PRECISION_HIGH));
            if (NeedRecompile)
            {
                gcmONERROR((*vscAPIs->gcSHADER_AddOutputLocation)(blitPSShader, 0, 1));
            }
            gcmONERROR((*vscAPIs->gcSHADER_Pack)(blitPSShader));

            gcmONERROR((*vscAPIs->gcSHADER_SetCompilerVersion)(blitPSShader, psShaderVersion));
        } while(gcvFALSE);
    }

    /* Initial clear program */
    if ((type == gcvBLITDRAW_CLEAR) && (blitDraw->vsShader[gcvBLITDRAW_CLEAR] == gcvNULL))
    {
        gcmONERROR((*vscAPIs->gcSHADER_Construct)(gcvNULL, gcSHADER_TYPE_VERTEX, &blitDraw->vsShader[gcvBLITDRAW_CLEAR]));
        do
        {
            gcATTRIBUTE pos;
            gcSHADER clearVSShader = blitDraw->vsShader[gcvBLITDRAW_CLEAR];

            gcmONERROR((*vscAPIs->gcSHADER_AddAttribute)(clearVSShader, "in_position", gcSHADER_FLOAT_X3, 1, gcvFALSE, gcSHADER_SHADER_DEFAULT, gcSHADER_PRECISION_HIGH, &pos));
            gcmONERROR((*vscAPIs->gcSHADER_AddOpcode)(clearVSShader, gcSL_MOV, 1, gcSL_ENABLE_XYZ, gcSL_FLOAT, pos->precision));
            gcmONERROR((*vscAPIs->gcSHADER_AddSourceAttribute)(clearVSShader, pos, gcSL_SWIZZLE_XYZZ, 0));
            gcmONERROR((*vscAPIs->gcSHADER_AddOpcode)(clearVSShader, gcSL_MOV, 1, gcSL_ENABLE_W, gcSL_FLOAT, gcSHADER_PRECISION_HIGH));
            gcmONERROR((*vscAPIs->gcSHADER_AddSourceConstant)(clearVSShader, 1.0));
            gcmONERROR((*vscAPIs->gcSHADER_AddOutput)(clearVSShader, "#Position", gcSHADER_FLOAT_X4, 1, 1, gcSHADER_PRECISION_HIGH));
            gcmONERROR((*vscAPIs->gcSHADER_Pack)(clearVSShader));

            gcmONERROR((*vscAPIs->gcSHADER_SetCompilerVersion)(clearVSShader, vsShaderVersion));
        } while(gcvFALSE);
    }

    if ((type == gcvBLITDRAW_CLEAR) && (blitDraw->psShader[gcvBLITDRAW_CLEAR] == gcvNULL))
    {
        gcmONERROR((*vscAPIs->gcSHADER_Construct)(gcvNULL, gcSHADER_TYPE_FRAGMENT, &blitDraw->psShader[gcvBLITDRAW_CLEAR]));
        do
        {
            gcSHADER clearPSShader = blitDraw->psShader[gcvBLITDRAW_CLEAR];
            gcmONERROR((*vscAPIs->gcSHADER_AddUniform)(clearPSShader, "uColor", gcSHADER_FLOAT_X4, 1, gcSHADER_PRECISION_HIGH, &blitDraw->clearColorUnfiorm));
            gcmONERROR((*vscAPIs->gcSHADER_AddOpcode)(clearPSShader, gcSL_MOV, 1, gcSL_ENABLE_XYZW, gcSL_FLOAT, blitDraw->clearColorUnfiorm->precision));
            gcmONERROR((*vscAPIs->gcSHADER_AddSourceUniformIndexedFormattedWithPrecision)(
                clearPSShader,
                blitDraw->clearColorUnfiorm,
                gcSL_SWIZZLE_XYZW,
                0,
                gcSL_NOT_INDEXED,
                gcSL_NONE_INDEXED,
                0,
                gcSL_FLOAT,
                gcSHADER_PRECISION_HIGH));
            gcmONERROR((*vscAPIs->gcSHADER_AddOutput)(clearPSShader, outputColorName, gcSHADER_FLOAT_X4, 1, 1, gcSHADER_PRECISION_HIGH));
            if (NeedRecompile)
            {
                gcmONERROR((*vscAPIs->gcSHADER_AddOutputLocation)(clearPSShader, 0, 1));
            }
            gcmONERROR((*vscAPIs->gcSHADER_Pack)(clearPSShader));

            gcmONERROR((*vscAPIs->gcSHADER_SetCompilerVersion)(clearPSShader, psShaderVersion));
        } while(gcvFALSE);
    }

OnError:
    return status;
}

gceSTATUS
_PickBlitDrawShader(
    gcoHARDWARE Hardware,
    gceBLITDRAW_TYPE type,
    gcsSURF_FORMAT_INFO_PTR srcFmtInfo,
    gcsSURF_FORMAT_INFO_PTR dstFmtInfo,
    gcsPROGRAM_STATE ** programState
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsHARDWARE_BLITDRAW_PTR blitDraw = Hardware->blitDraw;
    gceSURF_FORMAT requestSrcFmt = gcvSURF_UNKNOWN;
    gceSURF_FORMAT requestDstFmt = gcvSURF_UNKNOWN;
    gcsVSC_APIS *vscAPIs;

#if gcdSTATIC_LINK || defined(__CHROME_OS__)
    vscAPIs = g_vscAPIs;
#else
    vscAPIs = &Hardware->blitDraw->vscAPIs;
#endif

    if (!vscAPIs)
    {
        gcmONERROR(gcvSTATUS_INVALID_REQUEST);
    }

    if (srcFmtInfo && srcFmtInfo->fakedFormat &&
        srcFmtInfo->format != gcvSURF_R8_1_X8R8G8B8 &&
        srcFmtInfo->format != gcvSURF_G8R8_1_X8R8G8B8
       )
    {
        requestSrcFmt = srcFmtInfo->format;
    }

    if (dstFmtInfo && dstFmtInfo->fakedFormat)
    {
        requestDstFmt = dstFmtInfo->format;
    }

    if (blitDraw)
    {
        gctINT32 i;
        gcsPROGRAM_STATE_VARIATION *variation = gcvNULL;

        for (i = 0; i < gcmMAX_VARIATION_NUM; i++)
        {
            variation = &blitDraw->programState[type][i];
            if ((variation->srcFmt == requestSrcFmt) &&
                (variation->destFmt == requestDstFmt) &&
                (variation->programState.stateBuffer != gcvNULL))
            {
                break;
            }
            variation = gcvNULL;
        }

        /* new format pair */
        if (variation == gcvNULL)
        {
            gcsPROGRAM_STATE_VARIATION *temp = gcvNULL;
            gcPatchDirective *patchDirective = gcvNULL;
            gctBOOL needRecompile = gcvFALSE;
            gctBOOL needFormatConvert = gcvTRUE;

            for (i = 0; i < gcmMAX_VARIATION_NUM; i++)
            {
                temp = &blitDraw->programState[type][i];
                if (temp->programState.stateBuffer == gcvNULL)
                {
                    break;
                }
            }

            if (i >= gcmMAX_VARIATION_NUM)
            {
                gcmASSERT(temp);
                /* TODO: add LRU support, now just free the last one */
                if (temp->programState.hints)
                {
                    gcmONERROR(g_vscAPIs->gcHINTS_Destroy(temp->programState.hints));
                    gcmOS_SAFE_FREE(gcvNULL, temp->programState.hints);
                    temp->programState.hints = gcvNULL;
                }

                if (temp->programState.stateBuffer)
                {
                    gcmOS_SAFE_FREE(gcvNULL, temp->programState.stateBuffer);
                    temp->programState.stateBuffer = gcvNULL;
                }
            }

            gcmASSERT(temp->programState.stateBuffer == gcvNULL);

            /* start to generate program variation for new format pair
            */
            if(blitDraw->vsShader[type] != gcvNULL)
            {
                gcmONERROR((*vscAPIs->gcSHADER_Destroy)(blitDraw->vsShader[type]));
                blitDraw->vsShader[type] = gcvNULL;
            }

            if(blitDraw->psShader[type] != gcvNULL)
            {
                gcmONERROR((*vscAPIs->gcSHADER_Destroy)(blitDraw->psShader[type]));
                blitDraw->psShader[type] = gcvNULL;
            }

            if (requestSrcFmt != gcvSURF_UNKNOWN ||
                requestDstFmt != gcvSURF_UNKNOWN)
            {
                needRecompile = gcvTRUE;
            }

            gcmONERROR(_InitBlitProgram(Hardware, type, needRecompile));

            if (requestSrcFmt != gcvSURF_UNKNOWN)
            {
                gceTEXTURE_SWIZZLE baseComponents_rgba[] =
                {
                    gcvTEXTURE_SWIZZLE_R,
                    gcvTEXTURE_SWIZZLE_G,
                    gcvTEXTURE_SWIZZLE_B,
                    gcvTEXTURE_SWIZZLE_A
                };

                gcmASSERT(type == gcvBLITDRAW_BLIT);
                gcmASSERT(srcFmtInfo);

                gcmONERROR((*vscAPIs->gcCreateInputConversionDirective)(blitDraw->bliterSampler,
                                                                          0,
                                                                          srcFmtInfo,
                                                                          baseComponents_rgba,
                                                                          0,
                                                                          gcTEXTURE_MODE_NONE,
                                                                          gcTEXTURE_MODE_NONE,
                                                                          gcTEXTURE_MODE_NONE,
                                                                          0.0,
                                                                          0,
                                                                          0,
                                                                          0,
                                                                          0,
                                                                          0,
                                                                          0,
                                                                          0,
                                                                          gcvFALSE,
                                                                          gcvFALSE,
                                                                          gcvFALSE,
                                                                          needFormatConvert,
                                                                          &patchDirective));


            }

            if (requestDstFmt != gcvSURF_UNKNOWN)
            {
                gcmASSERT(dstFmtInfo);
                gcmONERROR((*vscAPIs->gcCreateOutputConversionDirective)(0, dstFmtInfo, 0, gcvFALSE, &patchDirective));
            }


            /* patch vertex and fragment shader */
            if (patchDirective)
            {
                (*vscAPIs->gcSetGLSLCompiler)(vscAPIs->gcCompileShader);
                gcmONERROR((*vscAPIs->gcSHADER_DynamicPatch)(blitDraw->vsShader[type], patchDirective, 0));
                gcmONERROR((*vscAPIs->gcSHADER_DynamicPatch)(blitDraw->psShader[type], patchDirective, 0));
            }

            gcmONERROR((*vscAPIs->gcLinkShaders)(blitDraw->vsShader[type],
                                                   blitDraw->psShader[type],
                                                   (gceSHADER_FLAGS)
                                                   (gcvSHADER_DEAD_CODE              |
                                                    gcvSHADER_RESOURCE_USAGE         |
                                                    gcvSHADER_OPTIMIZER              |
                                                    gcvSHADER_USE_GL_Z               |
                                                    gcvSHADER_USE_GL_POINT_COORD     |
                                                    gcvSHADER_USE_GL_POSITION        |
                                                    gcvSHADER_REMOVE_UNUSED_UNIFORMS |
                                                    gcvSHADER_DISABLE_DEFAULT_UBO    |
                                                    gcvSHADER_FLUSH_DENORM_TO_ZERO),
                                                   &temp->programState.stateBufferSize,
                                                   &temp->programState.stateBuffer,
                                                   &temp->programState.hints));

            *programState = &temp->programState;
            temp->destFmt = requestDstFmt;
            temp->srcFmt = requestSrcFmt;
            return gcvSTATUS_OK;

        }
        else
        {
            /* found one */
            *programState = &variation->programState;
            return gcvSTATUS_OK;
        }
    }
    else
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

OnError:
    return status;
}

gceSTATUS
gcoHARDWARE_DrawClear(
    IN gcsSURF_VIEW *RtView,
    IN gcsSURF_VIEW *DsView,
    IN gcsSURF_CLEAR_ARGS_PTR args
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoHARDWARE hardware = gcvNULL;
    gcsHARDWARE_BLITDRAW_PTR blitDraw;
    gcoSURF rtSurface = RtView ? RtView->surf : gcvNULL;
    gcoSURF dsSurface = DsView ? DsView->surf : gcvNULL;
    gcsSURF_FORMAT_INFO_PTR dstFmtInfo = gcvNULL;
    gcsPROGRAM_STATE *programState = gcvNULL;
    gctINT32 i = 0;
    gctUINT32 tileStatusAddress;

    gctUINT srcWidth  = 1;
    gctUINT srcHeight = 1;

#if gcdENABLE_APPCTXT_BLITDRAW
    gcsFESTATES *FEStates;
    gcsPAANDSESTATES *PAAndSEStates;
    gcsMSAASTATES *MsaaStates;
    gcsSHSTATES *SHStates;
    gcsPESTATES *PEStates;
    gcsTXSTATES *TXStates;
    gcsMCSTATES *MCStates;
    gcsQUERYSTATES *QueryStates;
    gcsXFBSTATES *XfbStates;

    gcmGETHARDWARE(hardware);

    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsFESTATES), (gctPOINTER *) &FEStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsPAANDSESTATES), (gctPOINTER *) &PAAndSEStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsMSAASTATES), (gctPOINTER *) &MsaaStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsSHSTATES), (gctPOINTER *) &SHStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsPESTATES), (gctPOINTER *) &PEStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsTXSTATES), (gctPOINTER *) &TXStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsMCSTATES), (gctPOINTER *) &MCStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsQUERYSTATES), (gctPOINTER *) &QueryStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsXFBSTATES), (gctPOINTER *) &XfbStates));

    /* Save the states of the last draw. */
    gcoOS_MemCopy(FEStates, hardware->FEStates, gcmSIZEOF(gcsFESTATES));
    gcoOS_MemCopy(PAAndSEStates, hardware->PAAndSEStates, gcmSIZEOF(gcsPAANDSESTATES));
    gcoOS_MemCopy(MsaaStates, hardware->MsaaStates, gcmSIZEOF(gcsMSAASTATES));
    gcoOS_MemCopy(SHStates, hardware->SHStates, gcmSIZEOF(gcsSHSTATES));
    gcoOS_MemCopy(PEStates, hardware->PEStates, gcmSIZEOF(gcsPESTATES));
    gcoOS_MemCopy(TXStates, hardware->TXStates, gcmSIZEOF(gcsTXSTATES));
    gcoOS_MemCopy(MCStates, hardware->MCStates, gcmSIZEOF(gcsMCSTATES));
    gcoOS_MemCopy(QueryStates, hardware->QUERYStates, gcmSIZEOF(gcsQUERYSTATES));
    gcoOS_MemCopy(XfbStates, hardware->XFBStates, gcmSIZEOF(gcsXFBSTATES));

    gcmONERROR(_InitDefaultState(hardware));
#else
    gcoHARDWARE savedHardware = gcvNULL;

    gcmONERROR(gcoHARDWARE_Get3DHardware(&savedHardware));
    /* Kick off current hardware and switch to default hardware */
    gcmONERROR(gcoHARDWARE_Set3DHardware(gcvNULL));
    gcmGETHARDWARE(hardware);

    /* Currently it's a must that default hardware is current */
    gcmASSERT(hardware->threadDefault);
#endif

    gcmONERROR(_InitBlitDraw(hardware));

    blitDraw = hardware->blitDraw;

    if (args->flags & gcvCLEAR_COLOR)
    {
        gcmASSERT(rtSurface);

        dstFmtInfo = &rtSurface->formatInfo;

        for (i = 0; i < dstFmtInfo->layers; i++)
        {
            gcmONERROR(gcoHARDWARE_SetRenderTarget(hardware,
                                                   i,
                                                   rtSurface,
                                                   RtView->firstSlice,
                                                   i));

            gcmONERROR(gcoHARDWARE_SetColorWrite(hardware, i, args->colorMask));
        }

        if (rtSurface->tileStatusNode.pool != gcvPOOL_UNKNOWN)
        {
            gcmGETHARDWAREADDRESS(rtSurface->tileStatusNode, tileStatusAddress);
        }
        else
        {
            tileStatusAddress = 0;
        }

        gcmONERROR(gcoHARDWARE_EnableTileStatus(hardware,
                                                rtSurface,
                                                tileStatusAddress,
                                                &rtSurface->hzTileStatusNode,
                                                0));

        gcmONERROR(gcoHARDWARE_SetDepthOnly(hardware, gcvFALSE));

        gcmONERROR(gcoHARDWARE_SetViewport(hardware,
                                           0,
                                           rtSurface->requestH,
                                           rtSurface->requestW,
                                           0));

        gcmONERROR(gcoHARDWARE_SetScissors(hardware,
                                           0,
                                           0,
                                           rtSurface->requestW,
                                           rtSurface->requestH));

        srcWidth  = rtSurface->requestW;
        srcHeight = rtSurface->requestH;
        gcmONERROR(gcoHARDWARE_SetColorOutCount(hardware, dstFmtInfo->layers));

        if (rtSurface->isMsaa)
        {
            /* MSAA we need decide if we need a extra register, which set in flushDepth, so, dirty flush depth */
            hardware->PEDirty->depthConfigDirty        = gcvTRUE;
            hardware->PEDirty->depthTargetDirty        = gcvTRUE;
        }
    }
    else
    {
        gcmONERROR(gcoHARDWARE_SetColorWrite(hardware, 0, 0));
        gcmONERROR(gcoHARDWARE_SetColorOutCount(hardware, 0));
    }

    gcmONERROR(gcoHARDWARE_SetColorCacheMode(hardware));

    if (args->flags & (gcvCLEAR_DEPTH | gcvCLEAR_STENCIL))
    {
        gcsSTENCIL_INFO stencilInfo;
        gcmASSERT(dsSurface);
        dstFmtInfo = &dsSurface->formatInfo;

        gcmONERROR(gcoHARDWARE_SetDepthBuffer(hardware, dsSurface, DsView->firstSlice));

        if (dsSurface->tileStatusNode.pool != gcvPOOL_UNKNOWN)
        {
            gcmGETHARDWAREADDRESS(dsSurface->tileStatusNode, tileStatusAddress);
        }
        else
        {
            tileStatusAddress = 0;
        }

        gcmONERROR(gcoHARDWARE_EnableTileStatus(hardware,
                                                dsSurface,
                                                tileStatusAddress,
                                                &dsSurface->hzTileStatusNode,
                                                0));
        gcmONERROR(gcoHARDWARE_SetDepthMode(hardware, gcvDEPTH_Z));
        gcmONERROR(gcoHARDWARE_SetDepthOnly(hardware, gcvTRUE));
        gcmONERROR(gcoHARDWARE_SetDepthRangeF(hardware, gcvDEPTH_Z, 0.0f, 1.0f));
        gcmONERROR(gcoHARDWARE_SetDepthWrite(hardware, (args->flags & gcvCLEAR_DEPTH) ?
                                                        args->depthMask : 0));
        gcmONERROR(gcoHARDWARE_SetDepthCompare(hardware, gcvCOMPARE_ALWAYS));

        if (args->flags & gcvCLEAR_STENCIL)
        {
            stencilInfo.compareBack =
            stencilInfo.compareFront = gcvCOMPARE_ALWAYS;
            stencilInfo.maskBack =
            stencilInfo.maskFront = 0xff;
            stencilInfo.referenceBack =
            stencilInfo.referenceFront = (gctUINT8)args->stencil;
            stencilInfo.depthFailBack =
            stencilInfo.depthFailFront =
            stencilInfo.failBack =
            stencilInfo.failFront =
            stencilInfo.passBack =
            stencilInfo.passFront = gcvSTENCIL_REPLACE;
            stencilInfo.writeMaskBack =
            stencilInfo.writeMaskFront = args->stencilMask;
            stencilInfo.mode = gcvSTENCIL_DOUBLE_SIDED;
        }
        else
        {
            stencilInfo.mode = gcvSTENCIL_NONE;
            stencilInfo.compareBack =
            stencilInfo.compareFront = gcvCOMPARE_ALWAYS;
            stencilInfo.maskBack =
            stencilInfo.maskFront = 0xff;
            stencilInfo.referenceBack =
            stencilInfo.referenceFront = (gctUINT8)args->stencil;
            stencilInfo.depthFailBack =
            stencilInfo.depthFailFront =
            stencilInfo.failBack =
            stencilInfo.failFront =
            stencilInfo.passBack =
            stencilInfo.passFront = gcvSTENCIL_KEEP;
            stencilInfo.writeMaskBack =
            stencilInfo.writeMaskFront = 0x0;
            stencilInfo.mode = gcvSTENCIL_DOUBLE_SIDED;
        }
        gcmONERROR(gcoHARDWARE_SetStencilAll(hardware, &stencilInfo));

        gcmONERROR(gcoHARDWARE_SetViewport(hardware,
                                           0,
                                           dsSurface->requestH,
                                           dsSurface->requestW,
                                           0));

        gcmONERROR(gcoHARDWARE_SetScissors(hardware,
                                           0,
                                           0,
                                           dsSurface->requestW,
                                           dsSurface->requestH));

        srcWidth  = dsSurface->requestW;
        srcHeight = dsSurface->requestH;
    }
    else
    {
        gcmONERROR(gcoHARDWARE_SetDepthMode(hardware, gcvDEPTH_NONE));
        gcmONERROR(gcoHARDWARE_SetStencilMode(hardware, gcvSTENCIL_NONE));
    }

    gcmONERROR(gcoHARDWARE_SetCulling(hardware, gcvCULL_NONE));
    gcmONERROR(gcoHARDWARE_SetFill(hardware, gcvFILL_SOLID));


    gcmONERROR(_PickBlitDrawShader(hardware,
                                   gcvBLITDRAW_CLEAR,
                                   gcvNULL,
                                   dstFmtInfo,
                                   &programState));

    gcmONERROR(gcoHARDWARE_LoadProgram(hardware,
                                       programState->hints->stageBits,
                                       programState));

    /* Flush uniform */
    if (args->flags & gcvCLEAR_COLOR)
    {
        gctFLOAT fColor[4];
        gctUINT32 physicalAddress = 0, baseAddress = 0, shift = 0;

        switch (rtSurface->format)
        {
        case gcvSURF_R8_1_X8R8G8B8:
            fColor[0] = args->color.r.floatValue;
            fColor[1] = 0.0f;
            fColor[2] = 0.0f;
            fColor[3] = 1.0f;
            break;
        case gcvSURF_G8R8_1_X8R8G8B8:
            fColor[0] = args->color.r.floatValue;
            fColor[1] = args->color.g.floatValue;
            fColor[2] = 0.0f;
            fColor[3] = 1.0f;
            break;
        default:
            fColor[0] = args->color.r.floatValue;
            fColor[1] = args->color.g.floatValue;
            fColor[2] = args->color.b.floatValue;
            fColor[3] = args->color.a.floatValue;
            break;
        }

        baseAddress = programState->hints->psConstBase;
        shift = (gctUINT32)gcmExtractSwizzle(GetUniformSwizzle(blitDraw->clearColorUnfiorm), 0);

        physicalAddress = baseAddress + GetUniformPhysical(blitDraw->clearColorUnfiorm) * 16 + shift * 4;

        gcmONERROR(gcoHARDWARE_ProgramUniform(gcvNULL, physicalAddress,
                                              4, 1, fColor, 0, 0, GetUniformShaderKind(blitDraw->clearColorUnfiorm)));
    }

    /* Set up vertex stream.*/
    do
    {
        gcsVERTEXARRAY_BUFOBJ stream;
        gcsVERTEXARRAY_BUFOBJ_PTR streamPtr = &stream;
        gcsVERTEXARRAY_BUFOBJ_ATTRIBUTE attrib;
        gcsVERTEXARRAY_BUFOBJ_ATTRIBUTE_PTR attribPtr = &attrib;
        gctFLOAT left, top, right, bottom;
        gctFLOAT depth = 1.0f;

        gctFLOAT positionData[18];

        if (args->flags & gcvCLEAR_DEPTH)
        {
            depth = (args->depth.floatValue >= 1.0f) ?  1.0f
                  : (args->depth.floatValue <= 0.0f) ? -1.0f
                  : 2 * args->depth.floatValue - 1.0f;
        }

        /*
         * Generate two CCW triangles.
         * Clear Arg coordinate is Top-Bottom.
         * OpenGL vertex coordinate is Bottom-Top.
         */
        left   = 2.0f * args->clearRect->left   / srcWidth  - 1.0f;
        top    = 2.0f * args->clearRect->bottom / srcHeight - 1.0f;
        right  = 2.0f * args->clearRect->right  / srcWidth  - 1.0f;
        bottom = 2.0f * args->clearRect->top    / srcHeight - 1.0f;

        positionData[0]  = left;
        positionData[1]  = bottom;
        positionData[2]  = depth;

        positionData[3]  = right;
        positionData[4]  = bottom;
        positionData[5]  = depth;

        positionData[6]  = left;
        positionData[7]  = top;
        positionData[8]  = depth;

        positionData[9]  = left;
        positionData[10] = top;
        positionData[11] = depth;

        positionData[12] = right;
        positionData[13] = bottom;
        positionData[14] = depth;

        positionData[15] = right;
        positionData[16] = top;
        positionData[17] = depth;

        gcoOS_ZeroMemory(streamPtr, sizeof(gcsVERTEXARRAY_BUFOBJ));

        /* set steam. */
        streamPtr->stride         = 0xc;
        streamPtr->divisor        = 0x0;
        streamPtr->count          = 0x6;
        streamPtr->attributeCount = 0x1;

        /* set attrib.*/
        attribPtr->format     = gcvVERTEX_FLOAT;
        attribPtr->linkage    = 0x0; /* Only one attribute for clear shader*/
        attribPtr->size       = 0x3;
        attribPtr->normalized = 0x0;
        attribPtr->enabled    = gcvTRUE;
        attribPtr->offset     = 0x0;
        attribPtr->pointer    = &positionData;
        attribPtr->bytes      = 0xc;
        attribPtr->isPosition = 0x0;
        attribPtr->stride     = 0xc;
        attribPtr->next       = gcvNULL;

        streamPtr->attributePtr = attribPtr;

        gcmONERROR(gcoSTREAM_CacheAttributesEx(blitDraw->dynamicStream,
                                             0x1,
                                             streamPtr,
                                             0x0,
                                             gcvNULL
                                             ));

        gcmONERROR(gcoHARDWARE_SetVertexArrayEx(gcvNULL,
            1,
            0,
            1,
            streamPtr,
            0,
            0,
            0xffffffff
            ));

        if (hardware->features[gcvFEATURE_HALTI0])
        {
            /* Draw command.*/
            gcmONERROR(gcoHARDWARE_DrawInstancedPrimitives(hardware,
                                                           0,
                                                           gcvPRIMITIVE_TRIANGLE_LIST,
                                                           0,
                                                           0,
                                                           2,
                                                           6,
                                                           1));
        }
        else
        {
            /* Draw command.*/
            gcmONERROR(gcoHARDWARE_DrawPrimitives(hardware,
                                                  gcvPRIMITIVE_TRIANGLE_LIST,
                                                  0,
                                                  2));
        }

        /* reset rt. Should set gcvNULL to rt to triger MSAA read temp rt when clear depth.*/
        if (args->flags & gcvCLEAR_COLOR)
        {
            gcmONERROR(gcoHARDWARE_DisableTileStatus(hardware, rtSurface, gcvFALSE));
            for (i = 0; i < dstFmtInfo->layers; i++)
            {
                gcmONERROR(gcoHARDWARE_SetRenderTarget(hardware,
                                                       i,
                                                       gcvNULL,
                                                       0,
                                                       i));
            }
        }

        if (args->flags & (gcvCLEAR_DEPTH | gcvCLEAR_STENCIL))
        {
            gcmONERROR(gcoHARDWARE_DisableTileStatus(hardware, dsSurface, gcvFALSE));
            gcmONERROR(gcoHARDWARE_SetDepthBuffer(hardware, gcvNULL, 0));
        }

    }
    while(gcvFALSE);

#if gcdENABLE_APPCTXT_BLITDRAW
    /* Restore states. */
    gcoOS_MemCopy(hardware->FEStates, FEStates, gcmSIZEOF(gcsFESTATES));
    gcoOS_MemCopy(hardware->PAAndSEStates, PAAndSEStates, gcmSIZEOF(gcsPAANDSESTATES));
    gcoOS_MemCopy(hardware->MsaaStates, MsaaStates, gcmSIZEOF(gcsMSAASTATES));
    gcoOS_MemCopy(hardware->SHStates, SHStates, gcmSIZEOF(gcsSHSTATES));
    gcoOS_MemCopy(hardware->PEStates, PEStates, gcmSIZEOF(gcsPESTATES));
    gcoOS_MemCopy(hardware->TXStates, TXStates, gcmSIZEOF(gcsTXSTATES));
    gcoOS_MemCopy(hardware->MCStates, MCStates, gcmSIZEOF(gcsMCSTATES));
    gcoOS_MemCopy(hardware->QUERYStates, QueryStates, gcmSIZEOF(gcsQUERYSTATES));
    gcoOS_MemCopy(hardware->XFBStates, XfbStates, gcmSIZEOF(gcsXFBSTATES));

    /* Reset some intermediate generated states to invalid. Thus, in the next compasion, it will work.
    ** For example, clear1->blit->clear2, if clear2 has the same cacheMode with clear1, but the blit has the different one.
    ** If we didn't reset the caseMode after blit, clear2 will not program the correct value about cacheMode.
    */
    hardware->PEStates->colorStates.cacheMode = hardware->PEStates->singlePEpipe
                                              = hardware->PEStates->colorStates.destinationRead = gcvINVALID_VALUE;

    /* Reset all the dirty to make sure the next time, it will flush the right states. */
    /* FE dirty reset. */
    if (hardware->FEStates->indexHeadAddress)
    {
        hardware->FEDirty->indexHeadDirty = gcvTRUE;
    }

    if (hardware->FEStates->indexTailAddress)
    {
        hardware->FEDirty->indexTailDirty = gcvTRUE;
    }
    hardware->FEDirty->indexDirty = gcvTRUE;
    /* PA and SE dirty reset. */
    hardware->PAAndSEDirty->paConfigDirty = hardware->PAAndSEDirty->paLineDirty
                                          = hardware->PAAndSEDirty->scissorDirty
                                          = hardware->PAAndSEDirty->viewportDirty = gcvTRUE;
    /* Msaa dirty reset. */
    hardware->MsaaDirty->centroidsDirty = hardware->MsaaDirty->msaaConfigDirty = hardware->MsaaDirty->msaaModeDirty = gcvTRUE;
    /* SH dirty reset. */
    hardware->SHDirty->shaderDirty |= (gcvPROGRAM_STAGE_VERTEX_BIT | gcvPROGRAM_STAGE_FRAGMENT_BIT);
    hardware->SHDirty->programSwitched = gcvTRUE;
    /* PE dirty reset. */
    hardware->PEDirty->alphaDirty = hardware->PEDirty->colorConfigDirty
                                  = hardware->PEDirty->colorTargetDirty
                                  = hardware->PEDirty->depthConfigDirty
                                  = hardware->PEDirty->depthNormalizationDirty
                                  = hardware->PEDirty->depthRangeDirty
                                  = hardware->PEDirty->depthTargetDirty
                                  = hardware->PEDirty->peDitherDirty
                                  = hardware->PEDirty->stencilDirty = gcvTRUE;
    /* Texture dirty reset. */
    hardware->TXDirty->hwTxDirty = hardware->TXDirty->hwTxSamplerModeDirty
                                 = hardware->TXDirty->hwTxSamplerSizeDirty
                                 = hardware->TXDirty->hwTxSamplerSizeLogDirty
                                 = hardware->TXDirty->hwTxSampler3DDirty
                                 = hardware->TXDirty->hwTxSamplerLODDirty
                                 = hardware->TXDirty->hwTxSamplerBaseLODDirty
                                 = hardware->TXDirty->hwTxSamplerYUVControlDirty
                                 = hardware->TXDirty->hwTxSamplerYUVStrideDirty
                                 = hardware->TXDirty->hwTxSamplerLinearStrideDirty
                                 = hardware->TXDirty->hwTxSamplerSizeLogExtDirty
                                 = hardware->TXDirty->hwTxSampler3DExtDirty
                                 = hardware->TXDirty->hwTxSamplerLodExtDirty
                                 = hardware->TXDirty->hwTxSamplerLodBiasExtDirty
                                 = hardware->TXDirty->hwTxSamplerAnisoCtrlDirty
                                 = hardware->TXDirty->hwTxSamplerConfig3Dirty
                                 = hardware->TXDirty->hwTxSamplerDirty = 0xFFFFFFFF;

    gcsBITMASK_InitAllOne(&hardware->TXDirty->hwSamplerControl0Dirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&hardware->TXDirty->hwSamplerControl1Dirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&hardware->TXDirty->hwSamplerLodMinMaxDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&hardware->TXDirty->hwSamplerLODBiasDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&hardware->TXDirty->hwSamplerAnisCtrlDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&hardware->TXDirty->hwTxDescAddressDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&hardware->TXDirty->hwTxDescDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&hardware->TXDirty->hwTextureControlDirty, gcdTXDESCRIPTORS);

    hardware->TXDirty->hwTxDirty = hardware->TXDirty->textureDirty
                                 = hardware->TXDirty->hwTxFlushPS
                                 = hardware->TXDirty->hwTxFlushVS = gcvTRUE;
    gcoOS_MemFill(hardware->TXDirty->hwTxSamplerAddressDirty, 0xFF, gcmSIZEOF(hardware->TXDirty->hwTxSamplerAddressDirty));
    gcoOS_MemFill(hardware->TXDirty->hwTxSamplerASTCDirty, 0xFF, gcmSIZEOF(hardware->TXDirty->hwTxSamplerASTCDirty));

    /* MC dirty reset. */
    hardware->MCDirty->hwTxSamplerTSDirty = hardware->MCDirty->texTileStatusSlotDirty = 0xFF;
    hardware->MCDirty->cacheDirty= gcvTRUE;

    /* Query dirty reset. */
    hardware->QUERYDirty->queryDirty[gcvQUERY_OCCLUSION] = gcvTRUE;

    if (hardware->features[gcvFEATURE_HW_TFB])
    {
        hardware->QUERYDirty->queryDirty[gcvQUERY_XFB_WRITTEN] = gcvTRUE;
        hardware->QUERYDirty->queryDirty[gcvQUERY_PRIM_GENERATED] = gcvTRUE;

        /* XFB dirty reset. */
        hardware->XFBDirty->xfbDirty = 0xFFFFFFFF;
    }

    /* TODO: for clear, Reset rt is not need if no context switch,
       then, the following code can skip.*/
    if ((args->flags & gcvCLEAR_COLOR)                               &&
         hardware->PEStates->colorStates.target[0].surface != gcvNULL)
    {
        if (hardware->PEStates->colorStates.target[0].surface->tileStatusNode.pool != gcvPOOL_UNKNOWN)
        {
            gcmGETHARDWAREADDRESS(hardware->PEStates->colorStates.target[0].surface->tileStatusNode, tileStatusAddress);
        }
        else
        {
            tileStatusAddress = 0;
        }
        /* Enable the tile status for the requested surface. */
        status = gcoHARDWARE_EnableTileStatus(
            hardware,
            hardware->PEStates->colorStates.target[0].surface,
            tileStatusAddress,
            &hardware->PEStates->colorStates.target[0].surface->hzTileStatusNode,
            0);
    }

    if ((args->flags & (gcvCLEAR_DEPTH | gcvCLEAR_STENCIL)) &&
        hardware->PEStates->depthStates.surface != gcvNULL)
    {
        if (hardware->PEStates->depthStates.surface->tileStatusNode.pool != gcvPOOL_UNKNOWN)
        {
            gcmGETHARDWAREADDRESS(hardware->PEStates->depthStates.surface->tileStatusNode, tileStatusAddress);
        }
        else
        {
            tileStatusAddress = 0;
        }
        /* Enable the tile status for the requested surface. */
        status = gcoHARDWARE_EnableTileStatus(
            hardware,
            hardware->PEStates->depthStates.surface,
            tileStatusAddress,
            &hardware->PEStates->depthStates.surface->hzTileStatusNode,
            0);
    }

    /* Free temp states. */
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, FEStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, PAAndSEStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, MsaaStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, SHStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, PEStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, TXStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, MCStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, QueryStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, XfbStates));
#endif

OnError:
#if !gcdENABLE_APPCTXT_BLITDRAW
    if (savedHardware)
    {
        gcmVERIFY_OK(gcoHARDWARE_Set3DHardware(savedHardware));
    }
#endif
    return status;
}

gceSTATUS
gcoHARDWARE_DrawBlit(
    gcsSURF_VIEW *SrcView,
    gcsSURF_VIEW *DstView,
    gscSURF_BLITDRAW_BLIT *args
    )
{
    gceSTATUS status;
    gcoHARDWARE hardware = gcvNULL;
    gcsSAMPLER samplerInfo = {0};
    gcsTEXTURE texParamInfo = {0};
    gcoSURF srcSurf = SrcView->surf;
    gcoSURF dstSurf = DstView->surf;
    gcsHARDWARE_BLITDRAW_PTR blitDraw;
    gcsSURF_FORMAT_INFO_PTR dstFmtInfo = gcvNULL;
    gcsSURF_FORMAT_INFO_PTR srcFmtInfo = gcvNULL;
    gcsPROGRAM_STATE *programState = gcvNULL;
    gctINT32 i;
    gctUINT32 tileStatusAddress;

#if gcdENABLE_APPCTXT_BLITDRAW
    gcsFESTATES *FEStates;
    gcsPAANDSESTATES *PAAndSEStates;
    gcsMSAASTATES *MsaaStates;
    gcsSHSTATES *SHStates;
    gcsPESTATES *PEStates;
    gcsTXSTATES *TXStates;
    gcsMCSTATES *MCStates;
    gcsQUERYSTATES *QueryStates;
    gcsXFBSTATES *XfbStates;
#endif

    static const gceTEXTURE_SWIZZLE baseComponents_rgba[] =
    {
        gcvTEXTURE_SWIZZLE_R,
        gcvTEXTURE_SWIZZLE_G,
        gcvTEXTURE_SWIZZLE_B,
        gcvTEXTURE_SWIZZLE_A
    };

#if gcdENABLE_APPCTXT_BLITDRAW
    /* Get current hardware. */
    gcmGETHARDWARE(hardware);
#else
    gcoHARDWARE savedHardware = gcvNULL;

    /* Get current hardware. */
    gcmGETHARDWARE(hardware);
#endif
    srcFmtInfo = &srcSurf->formatInfo;
    dstFmtInfo = &dstSurf->formatInfo;

    /* explicit unsupported cases for source:
    ** 1, msaa source, We may support it later.
    ** 2, TX can't read it.
    */
    if (srcSurf->isMsaa ||
        (!hardware->features[gcvFEATURE_SUPERTILED_TEXTURE] && (srcSurf->tiling & gcvSUPERTILED)) ||
        (!hardware->features[gcvFEATURE_TEXTURE_LINEAR] && (srcSurf->tiling & gcvLINEAR))
       )
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Bail out if dst surface is not renderable*/
    if (gcvSTATUS_OK != gcoHARDWARE_QuerySurfaceRenderable(hardware, dstSurf))
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Disable source tile status if TX can not read. */
    if (
        (srcSurf->tileStatusDisabled == gcvFALSE &&
            !hardware->features[gcvFEATURE_TEXTURE_TILE_STATUS_READ]) ||
        (srcSurf->compressed &&
            !hardware->features[gcvFEATURE_TX_DECOMPRESSOR]) ||
        hardware->swwas[gcvSWWA_1165]
       )
    {
        /*
         * NOTICE: Must call disable tile status in rendering context before
         * switching back to blitdraw context, to make hardware states match in
         * the two contexts.
         */
        gcoHARDWARE_DisableTileStatus(hardware, srcSurf, gcvTRUE);
        samplerInfo.hasTileStatus = gcvFALSE;
    }
#if gcdENABLE_APPCTXT_BLITDRAW
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsFESTATES), (gctPOINTER *) &FEStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsPAANDSESTATES), (gctPOINTER *) &PAAndSEStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsMSAASTATES), (gctPOINTER *) &MsaaStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsSHSTATES), (gctPOINTER *) &SHStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsPESTATES), (gctPOINTER *) &PEStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsTXSTATES), (gctPOINTER *) &TXStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsMCSTATES), (gctPOINTER *) &MCStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsQUERYSTATES), (gctPOINTER *) &QueryStates));
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsXFBSTATES), (gctPOINTER *) &XfbStates));

    /* Save the states of the last draw. */
    gcoOS_MemCopy(FEStates, hardware->FEStates, gcmSIZEOF(gcsFESTATES));
    gcoOS_MemCopy(PAAndSEStates, hardware->PAAndSEStates, gcmSIZEOF(gcsPAANDSESTATES));
    gcoOS_MemCopy(MsaaStates, hardware->MsaaStates, gcmSIZEOF(gcsMSAASTATES));
    gcoOS_MemCopy(SHStates, hardware->SHStates, gcmSIZEOF(gcsSHSTATES));
    gcoOS_MemCopy(PEStates, hardware->PEStates, gcmSIZEOF(gcsPESTATES));
    gcoOS_MemCopy(TXStates, hardware->TXStates, gcmSIZEOF(gcsTXSTATES));
    gcoOS_MemCopy(MCStates, hardware->MCStates, gcmSIZEOF(gcsMCSTATES));
    gcoOS_MemCopy(QueryStates, hardware->QUERYStates, gcmSIZEOF(gcsQUERYSTATES));
    gcoOS_MemCopy(XfbStates, hardware->XFBStates, gcmSIZEOF(gcsXFBSTATES));

    gcmONERROR(_InitDefaultState(hardware));
#else
    gcmONERROR(gcoHARDWARE_Get3DHardware(&savedHardware));
    /* Kick off current hardware and switch to default hardware */
    gcmONERROR(gcoHARDWARE_Set3DHardware(gcvNULL));

    hardware = gcvNULL;
    gcmGETHARDWARE(hardware);

    /* Currently it's a must that default hardware is current */
    gcmASSERT(hardware->threadDefault);
#endif
    gcmONERROR(_InitBlitDraw(hardware));
    blitDraw = hardware->blitDraw;

    /* We must link program before bind texture so that we can know the base sampler offset. */
    gcmONERROR(
        _PickBlitDrawShader(hardware,
                            gcvBLITDRAW_BLIT,
                            srcFmtInfo,
                            dstFmtInfo,
                            &programState));

    gcmONERROR(gcoHARDWARE_LoadProgram(hardware,
                                       programState->hints->stageBits,
                                       programState));

    /* Prepare sampler info. */
    samplerInfo.width  = srcSurf->requestW;
    samplerInfo.height = srcSurf->requestH;
    samplerInfo.depth  = 1;
    samplerInfo.faces  = 1;

    samplerInfo.endianHint = ((srcFmtInfo->txFormat == 0x05)  ||
                              (srcFmtInfo->txFormat == 0x06)  ||
                              (srcFmtInfo->txFormat == 0x0C) ||
                              (srcFmtInfo->txFormat == 0x0D) ||
                              (srcFmtInfo->txFormat == 0x0B)) ?
                              gcvENDIAN_SWAP_WORD : gcvENDIAN_NO_SWAP;

    samplerInfo.filterable = gcvTRUE;
    samplerInfo.format = srcSurf->format;
    samplerInfo.formatInfo = &srcSurf->formatInfo;

    switch (srcSurf->tiling)
    {
    case gcvLINEAR:
        samplerInfo.hAlignment = gcvSURF_SIXTEEN;
        samplerInfo.addressing = gcvSURF_STRIDE_LINEAR;

        switch (samplerInfo.format)
        {
        case gcvSURF_YV12:
        case gcvSURF_I420:
        case gcvSURF_NV12:
        case gcvSURF_NV21:
        case gcvSURF_NV16:
        case gcvSURF_NV61:
            /* Need YUV assembler. */
            samplerInfo.lodNum = 3;
            break;

        case gcvSURF_YUY2:
        case gcvSURF_UYVY:
        case gcvSURF_YVYU:
        case gcvSURF_VYUY:
        default:
            /* Need linear texture. */
            samplerInfo.lodNum = 1;
            break;
        }
        break;

    case gcvTILED:
        samplerInfo.hAlignment = srcSurf->hAlignment;
        samplerInfo.lodNum = 1;
        break;

    case gcvSUPERTILED:
        samplerInfo.hAlignment = gcvSURF_SUPER_TILED;
        samplerInfo.lodNum = 1;
        break;

    case gcvMULTI_TILED:
        samplerInfo.hAlignment = gcvSURF_SPLIT_TILED;
        samplerInfo.lodNum = 2;
        break;

    case gcvMULTI_SUPERTILED:
        samplerInfo.hAlignment = gcvSURF_SPLIT_SUPER_TILED;
        samplerInfo.lodNum = 2;
        break;

    default:
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Set all the LOD levels. */
    gcmGETHARDWAREADDRESS(srcSurf->node, samplerInfo.lodAddr[0]);

    samplerInfo.baseLevelSurf = srcSurf;

    if (samplerInfo.lodNum == 3)
    {
        /* YUV-assembler needs 3 lods. */
        if (srcSurf->flags & gcvSURF_FLAG_MULTI_NODE)
        {
            gcmGETHARDWAREADDRESS(srcSurf->node2, samplerInfo.lodAddr[1]);
            gcmGETHARDWAREADDRESS(srcSurf->node3, samplerInfo.lodAddr[2]);
        }
        else
        {
            samplerInfo.lodAddr[1]   = samplerInfo.lodAddr[0] + srcSurf->uOffset;
            samplerInfo.lodAddr[2]   = samplerInfo.lodAddr[0] + srcSurf->vOffset;
        }

        /* Save strides. */
        samplerInfo.lodStride[0] = srcSurf->stride;
        samplerInfo.lodStride[1] = srcSurf->uStride;
        samplerInfo.lodStride[2] = srcSurf->vStride;
    }
    else if (samplerInfo.lodNum == 2)
    {
        samplerInfo.lodAddr[1] = srcSurf->node.physicalBottom;
    }

    samplerInfo.hasTileStatus = !srcSurf->tileStatusDisabled;

    samplerInfo.texType = gcvTEXTURE_2D;
    samplerInfo.textureInfo = &texParamInfo;
    texParamInfo.anisoFilter = 1;
    texParamInfo.baseLevel = 0;
    texParamInfo.compareMode = gcvTEXTURE_COMPARE_MODE_NONE;
    texParamInfo.lodBias = 0;
    texParamInfo.lodMin = 0;
    texParamInfo.lodMax = 0;
    texParamInfo.mipFilter = gcvTEXTURE_NONE;
    texParamInfo.minFilter =
    texParamInfo.magFilter = args->filterMode;
    texParamInfo.maxLevel = 0;
    gcoOS_MemCopy(&texParamInfo.swizzle, baseComponents_rgba, sizeof(baseComponents_rgba));
    texParamInfo.r =
    texParamInfo.s =
    texParamInfo.t = gcvTEXTURE_CLAMP;

    samplerInfo.unsizedDepthTexture = gcvTRUE;

    if (hardware->features[gcvFEATURE_TX_DESCRIPTOR])
    {
        gcsTXDESC_UPDATE_INFO updateInfo;
        gctINT arrayIdx;

        gcoOS_ZeroMemory((gctPOINTER)&updateInfo, gcmSIZEOF(updateInfo));

        updateInfo.baseLevelWidth = srcSurf->requestW;
        updateInfo.baseLevelHeight = srcSurf->requestH;
        updateInfo.baseLevelDepth = 1;
        updateInfo.type = gcvTEXTURE_2D;
        updateInfo.baseLevelSurf = srcSurf;
        updateInfo.endianHint = ((srcFmtInfo->txFormat == 0x05)  ||
                                 (srcFmtInfo->txFormat == 0x06)  ||
                                 (srcFmtInfo->txFormat == 0x0C) ||
                                 (srcFmtInfo->txFormat == 0x0D) ||
                                 (srcFmtInfo->txFormat == 0x0B)) ?
                                 gcvENDIAN_SWAP_WORD : gcvENDIAN_NO_SWAP;

        updateInfo.levels = 1;
        updateInfo.unsizedDepthTexture = gcvTRUE;

        gcmGETHARDWAREADDRESS(srcSurf->node, updateInfo.lodAddr[0]);

        if (srcSurf->formatInfo.fmtClass == gcvFORMAT_CLASS_ASTC)
        {
            updateInfo.astcSize[0] = srcSurf->format -
                                        (srcSurf->formatInfo.sRGB ? gcvSURF_ASTC4x4_SRGB : gcvSURF_ASTC4x4);
            updateInfo.astcSRGB[0] = srcSurf->formatInfo.sRGB;
        }

        if (blitDraw->descCurIndex >= (gcdMAX_TXDESC_ARRAY_SIZE - 1))
        {
            gcoHAL_FreeTXDescArray(blitDraw->descArray, blitDraw->descCurIndex);
            blitDraw->descCurIndex = -1;
        }

        arrayIdx = ++blitDraw->descCurIndex;

        for (i = 0; i < srcFmtInfo->layers; i++)
        {
            gcsTXDescNode *pDescNode = &blitDraw->descArray[arrayIdx];
            gcmASSERT(i < 2);

            updateInfo.lodAddr[0] += i * srcSurf->layerSize;

            if (!pDescNode->descNode[i])
            {
                gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsSURF_NODE), (gctPOINTER *)&pDescNode->descNode[i]);
                gcoOS_ZeroMemory((gctPOINTER)pDescNode->descNode[i], gcmSIZEOF(gcsSURF_NODE));
                gcmONERROR(gcsSURF_NODE_Construct(pDescNode->descNode[i],
                                                  256,
                                                  64,
                                                  gcvSURF_TXDESC,
                                                  0,
                                                  gcvPOOL_DEFAULT
                                                  ));
            }
            else
            {
                gcmASSERT(0);
            }

            if (!pDescNode->descLocked[i])
            {
                gcmONERROR(gcoSURF_LockNode(pDescNode->descNode[i],
                                            gcvNULL,
                                            &pDescNode->descLocked[i]));
            }

            updateInfo.desc = pDescNode->descLocked[i];

#if gcdDUMP
            gcmGETHARDWAREADDRESSP(pDescNode->descNode[i], updateInfo.physical);
#endif

            gcmONERROR(gcoHARDWARE_UpdateTextureDesc(hardware,
                                                     &texParamInfo,
                                                     &updateInfo));

            samplerInfo.descNode = pDescNode->descNode[i];

            gcmONERROR(gcoHARDWARE_BindTextureDesc(hardware,
                                                   i + programState->hints->samplerBaseOffset[gcvPROGRAM_STAGE_FRAGMENT],
                                                   &samplerInfo));
        }
    }
    else
    {
        if (srcSurf->formatInfo.fmtClass == gcvFORMAT_CLASS_ASTC)
        {
            samplerInfo.astcSize[0] = srcSurf->format -
                                        (srcSurf->formatInfo.sRGB ? gcvSURF_ASTC4x4_SRGB : gcvSURF_ASTC4x4);
            samplerInfo.astcSRGB[0] = srcSurf->formatInfo.sRGB;
        }
        for (i = 0; i < srcFmtInfo->layers; i++)
        {
            samplerInfo.lodAddr[0] += i * srcSurf->layerSize;

            if (samplerInfo.lodNum == 2)
            {
                samplerInfo.lodAddr[1] += i * srcSurf->layerSize;
            }

            gcmONERROR(gcoHARDWARE_BindTexture(hardware,
                                               i + programState->hints->samplerBaseOffset[gcvPROGRAM_STAGE_FRAGMENT],
                                               &samplerInfo));
        }
    }

    gcmONERROR(gcoHARDWARE_BindTextureTS(hardware));
#if gcdENABLE_APPCTXT_BLITDRAW
    /* For dispatchCompute dump texture, when switch RT, disable previous tileStatus.*/
    if (srcSurf->tileStatusNode.pool != gcvPOOL_UNKNOWN)
    {
        /* Disable tile status. */
        gcmONERROR(
            gcoHARDWARE_DisableTileStatus(hardware, srcSurf, gcvFALSE));
    }

    gcmONERROR(gcoHARDWARE_Semaphore(
        hardware, gcvWHERE_COMMAND, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE, gcvNULL
        ));
#endif

    for (i = 0; i < dstFmtInfo->layers; i++)
    {
        gcmONERROR(gcoHARDWARE_SetRenderTarget(hardware, i, dstSurf, DstView->firstSlice, i));
    }

    if (dstSurf->tileStatusNode.pool != gcvPOOL_UNKNOWN)
    {
        gcmGETHARDWAREADDRESS(dstSurf->tileStatusNode, tileStatusAddress);
    }
    else
    {
        tileStatusAddress = 0;
    }

    gcmONERROR(
        gcoHARDWARE_EnableTileStatus(hardware,
                                     dstSurf,
                                     tileStatusAddress,
                                     &dstSurf->hzTileStatusNode,
                                     0));

    gcmONERROR(gcoHARDWARE_SetDepthMode(hardware, gcvDEPTH_NONE));
    gcmONERROR(gcoHARDWARE_SetDepthOnly(hardware, gcvFALSE));
    gcmONERROR(gcoHARDWARE_SetStencilMode(hardware, gcvSTENCIL_NONE));

    gcmONERROR(gcoHARDWARE_SetColorWrite(hardware, 0, 0xff));
    gcmONERROR(gcoHARDWARE_SetColorOutCount(hardware, dstFmtInfo->layers));
    gcmONERROR(gcoHARDWARE_SetColorCacheMode(hardware));

    gcmONERROR(gcoHARDWARE_SetCulling(hardware, gcvCULL_NONE));
    gcmONERROR(gcoHARDWARE_SetFill(hardware, gcvFILL_SOLID));

    gcmONERROR(gcoHARDWARE_SetViewport(hardware,
                                       0,
                                       dstSurf->requestH,
                                       dstSurf->requestW,
                                       0));

    if (args->scissorEnabled)
    {
        gcmONERROR(
            gcoHARDWARE_SetScissors(hardware,
                                    gcmMAX(args->scissor.left, args->dstRect.left),
                                    gcmMAX(args->scissor.top, args->dstRect.top),
                                    gcmMIN(args->scissor.right, args->dstRect.right),
                                    gcmMIN(args->scissor.bottom, args->dstRect.bottom)));
    }
    else
    {
        gcmONERROR(
            gcoHARDWARE_SetScissors(hardware,
                                    gcmMAX(0, args->dstRect.left),
                                    gcmMAX(0, args->dstRect.top),
                                    gcmMIN((gctINT)dstSurf->requestW, args->dstRect.right),
                                    gcmMIN((gctINT)dstSurf->requestH, args->dstRect.bottom)));
    }

    /* Set up vertex stream.*/
    do
    {
        gcsVERTEXARRAY_BUFOBJ streamPos, streamTexCoord;
        gcsVERTEXARRAY_BUFOBJ_PTR streamPosPtr = &streamPos;
        gcsVERTEXARRAY_BUFOBJ_PTR streamTexCoordPtr = &streamTexCoord;
        gcsVERTEXARRAY_BUFOBJ_ATTRIBUTE attribPos, attribTexCoord;
        gcsVERTEXARRAY_BUFOBJ_ATTRIBUTE_PTR attribPosPtr = &attribPos;
        gcsVERTEXARRAY_BUFOBJ_ATTRIBUTE_PTR attribTexCoordPtr = &attribTexCoord;

        gctFLOAT positionData[3 * 2];
        gctFLOAT texCoordData[3 * 2];

        /* Rectangle coordinates. */
        gctFLOAT left, top, right, bottom;

        /* Transform coordinates. */
        left   = 2.0f * args->dstRect.left / dstSurf->requestW - 1.0f;
        top    = 2.0f * args->dstRect.top  / dstSurf->requestH - 1.0f;
        right  = 2.0f * (2.0f * args->dstRect.right  - args->dstRect.left) / dstSurf->requestW  - 1.0f;
        bottom = 2.0f * (2.0f * args->dstRect.bottom - args->dstRect.top)  / dstSurf->requestH - 1.0f;

        /* Create 2 triangles in a strip. */
        positionData[0] = left;
        positionData[1] = top;

        positionData[2] = right;
        positionData[3] = top;

        positionData[4] = left;
        positionData[5] = bottom;

        /* Normalize coordinates.*/
        left   = (gctFLOAT)args->srcRect.left / srcSurf->requestW;
        top    = (gctFLOAT)args->srcRect.top  / srcSurf->requestH;
        right  = (2 * (gctFLOAT)args->srcRect.right -  (gctFLOAT)args->srcRect.left) / srcSurf->requestW;
        bottom = (2 * (gctFLOAT)args->srcRect.bottom - (gctFLOAT)args->srcRect.top)  / srcSurf->requestH;

        if (args->xReverse)
        {
            left  = (gctFLOAT)args->srcRect.right / srcSurf->requestW;
            right = (2 * (gctFLOAT)args->srcRect.left - (gctFLOAT)args->srcRect.right) / srcSurf->requestW;
        }

        if (args->yReverse)
        {
            top    = (gctFLOAT)args->srcRect.bottom / srcSurf->requestH;
            bottom = (2 * (gctFLOAT)args->srcRect.top - (gctFLOAT)args->srcRect.bottom) / srcSurf->requestH;
        }

        /* Create two triangles. */
        texCoordData[0] = left;
        texCoordData[1] = top;

        texCoordData[2] = right;
        texCoordData[3] = top;

        texCoordData[4] = left;
        texCoordData[5] = bottom;

        gcoOS_ZeroMemory(streamPosPtr, sizeof(gcsVERTEXARRAY_BUFOBJ));
        gcoOS_ZeroMemory(streamTexCoordPtr, sizeof(gcsVERTEXARRAY_BUFOBJ));
        gcoOS_ZeroMemory(attribPosPtr, sizeof(gcsVERTEXARRAY_BUFOBJ_ATTRIBUTE));
        gcoOS_ZeroMemory(attribTexCoordPtr, sizeof(gcsVERTEXARRAY_BUFOBJ_ATTRIBUTE));

        /* set pos steam. */
        streamPosPtr->stride         = 0x8;
        streamPosPtr->divisor        = 0x0;
        streamPosPtr->count          = 0x3;
        /* vertex and texcoord.*/
        streamPosPtr->attributeCount = 0x1;

        /* set attrib.*/
        attribPosPtr->bytes      = 0x8;
        attribPosPtr->format     = gcvVERTEX_FLOAT;
        attribPosPtr->linkage    = 0x0; /* Pos.*/
        attribPosPtr->size       = 0x2;
        attribPosPtr->normalized = 0x0;
        attribPosPtr->offset     = gcmPTR2INT32(positionData);
        attribPosPtr->enabled    = gcvTRUE;
        attribPosPtr->isPosition = 0x0;
        attribPosPtr->stride     = 0x8;
        attribPosPtr->pointer    = &positionData;

        streamPosPtr->attributePtr = attribPosPtr;

        /* Set tex coord stream.*/
        streamTexCoordPtr->stride         = 0x8;
        streamTexCoordPtr->divisor        = 0x0;
        streamTexCoordPtr->count          = 0x3;
        streamTexCoordPtr->attributeCount = 0x1;

        /* set attrib.*/
        attribTexCoordPtr->bytes      = 0x8;
        attribTexCoordPtr->format     = gcvVERTEX_FLOAT;
        attribTexCoordPtr->linkage    = 0x1; /* tex coord */
        attribTexCoordPtr->size       = 0x2;
        attribTexCoordPtr->normalized = 0x0;
        attribTexCoordPtr->offset     = gcmPTR2INT32(texCoordData);
        attribTexCoordPtr->enabled    = gcvTRUE;
        attribTexCoordPtr->isPosition = 0x0;
        attribTexCoordPtr->stride     = 0x8;
        attribTexCoordPtr->pointer    = &texCoordData;

        streamTexCoordPtr->attributePtr = attribTexCoordPtr;
        streamTexCoordPtr->next = streamPosPtr;

        gcmONERROR(
            gcoSTREAM_CacheAttributesEx(blitDraw->dynamicStream,
                                        0x2,
                                        streamTexCoordPtr,
                                        0x0,
                                        gcvNULL));

        gcmONERROR(gcoHARDWARE_SetVertexArrayEx(gcvNULL,
                                                1,
                                                0,
                                                2,
                                                streamTexCoordPtr,
                                                0,
                                                0,
                                                0xffffffff
                                                ));

        if (hardware->features[gcvFEATURE_HALTI0])
        {
            /* Draw command.*/
            gcmONERROR(
                gcoHARDWARE_DrawInstancedPrimitives(hardware,
                                                    0,
                                                    gcvPRIMITIVE_TRIANGLE_STRIP,
                                                    0,
                                                    0,
                                                    1,
                                                    3,
                                                    1));
        }
        else
        {
            /* Draw command.*/
            gcmONERROR(
                gcoHARDWARE_DrawPrimitives(hardware,
                                           gcvPRIMITIVE_TRIANGLE_STRIP,
                                           0,
                                           1));
        }

        /* reset rt.*/
        for (i = 0; i < dstFmtInfo->layers; i++)
        {
            gcmONERROR(gcoHARDWARE_DisableTileStatus(hardware, dstSurf, gcvFALSE));
            gcmONERROR(gcoHARDWARE_SetRenderTarget(hardware, i, gcvNULL, 0, i));
        }

    }
    while(gcvFALSE);

#if gcdENABLE_APPCTXT_BLITDRAW
    /* Restore states. */
    gcoOS_MemCopy(hardware->FEStates, FEStates, gcmSIZEOF(gcsFESTATES));
    gcoOS_MemCopy(hardware->PAAndSEStates, PAAndSEStates, gcmSIZEOF(gcsPAANDSESTATES));
    gcoOS_MemCopy(hardware->MsaaStates, MsaaStates, gcmSIZEOF(gcsMSAASTATES));
    gcoOS_MemCopy(hardware->SHStates, SHStates, gcmSIZEOF(gcsSHSTATES));
    gcoOS_MemCopy(hardware->PEStates, PEStates, gcmSIZEOF(gcsPESTATES));
    gcoOS_MemCopy(hardware->TXStates, TXStates, gcmSIZEOF(gcsTXSTATES));
    gcoOS_MemCopy(hardware->MCStates, MCStates, gcmSIZEOF(gcsMCSTATES));
    gcoOS_MemCopy(hardware->QUERYStates, QueryStates, gcmSIZEOF(gcsQUERYSTATES));
    gcoOS_MemCopy(hardware->XFBStates, XfbStates, gcmSIZEOF(gcsXFBSTATES));


    /* Reset some intermediate generated states to invalid. Thus, in the next compasion, it will work.
    ** For example, clear1->blit->clear2, if clear2 has the same cacheMode with clear1, but the blit has the different one.
    ** If we didn't reset the caseMode after blit, clear2 will not program the correct value about cacheMode.
    */
    hardware->PEStates->colorStates.cacheMode = hardware->PEStates->singlePEpipe
                                              = hardware->PEStates->colorStates.destinationRead = gcvINVALID_VALUE;

    /* Reset all the dirty to make sure the next time, it will flush the right states. */
    /* FE dirty reset. */
    if (hardware->FEStates->indexHeadAddress)
    {
        hardware->FEDirty->indexHeadDirty = gcvTRUE;
    }

    if (hardware->FEStates->indexTailAddress)
    {
        hardware->FEDirty->indexTailDirty = gcvTRUE;
    }
    hardware->FEDirty->indexDirty = gcvTRUE;
    /* PA and SE dirty reset. */
    hardware->PAAndSEDirty->paConfigDirty = hardware->PAAndSEDirty->paLineDirty
                                          = hardware->PAAndSEDirty->scissorDirty
                                          = hardware->PAAndSEDirty->viewportDirty = gcvTRUE;
    /* Msaa dirty reset. */
    hardware->MsaaDirty->centroidsDirty = hardware->MsaaDirty->msaaConfigDirty = hardware->MsaaDirty->msaaModeDirty = gcvTRUE;
    /* SH dirty reset. */
    hardware->SHDirty->shaderDirty |= (gcvPROGRAM_STAGE_VERTEX_BIT | gcvPROGRAM_STAGE_FRAGMENT_BIT);
    hardware->SHDirty->programSwitched = gcvTRUE;
    /* PE dirty reset. */
    hardware->PEDirty->alphaDirty = hardware->PEDirty->colorConfigDirty
                                  = hardware->PEDirty->colorTargetDirty
                                  = hardware->PEDirty->depthConfigDirty
                                  = hardware->PEDirty->depthNormalizationDirty
                                  = hardware->PEDirty->depthRangeDirty
                                  = hardware->PEDirty->depthTargetDirty
                                  = hardware->PEDirty->peDitherDirty
                                  = hardware->PEDirty->stencilDirty = gcvTRUE;
    /* Texture dirty reset. */
    hardware->TXDirty->hwTxDirty = hardware->TXDirty->hwTxSamplerModeDirty
                                 = hardware->TXDirty->hwTxSamplerSizeDirty
                                 = hardware->TXDirty->hwTxSamplerSizeLogDirty
                                 = hardware->TXDirty->hwTxSampler3DDirty
                                 = hardware->TXDirty->hwTxSamplerLODDirty
                                 = hardware->TXDirty->hwTxSamplerBaseLODDirty
                                 = hardware->TXDirty->hwTxSamplerYUVControlDirty
                                 = hardware->TXDirty->hwTxSamplerYUVStrideDirty
                                 = hardware->TXDirty->hwTxSamplerLinearStrideDirty
                                 = hardware->TXDirty->hwTxSamplerSizeLogExtDirty
                                 = hardware->TXDirty->hwTxSampler3DExtDirty
                                 = hardware->TXDirty->hwTxSamplerLodExtDirty
                                 = hardware->TXDirty->hwTxSamplerLodBiasExtDirty
                                 = hardware->TXDirty->hwTxSamplerAnisoCtrlDirty
                                 = hardware->TXDirty->hwTxSamplerConfig3Dirty
                                 = hardware->TXDirty->hwTxSamplerDirty = 0xFFFFFFFF;

    gcsBITMASK_InitAllOne(&hardware->TXDirty->hwSamplerControl0Dirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&hardware->TXDirty->hwSamplerControl1Dirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&hardware->TXDirty->hwSamplerLodMinMaxDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&hardware->TXDirty->hwSamplerLODBiasDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&hardware->TXDirty->hwSamplerAnisCtrlDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&hardware->TXDirty->hwTxDescAddressDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&hardware->TXDirty->hwTxDescDirty, gcdTXDESCRIPTORS);
    gcsBITMASK_InitAllOne(&hardware->TXDirty->hwTextureControlDirty, gcdTXDESCRIPTORS);

    hardware->TXDirty->hwTxDirty = hardware->TXDirty->textureDirty
                                 = hardware->TXDirty->hwTxFlushPS
                                 = hardware->TXDirty->hwTxFlushVS = gcvTRUE;

    gcoOS_MemFill(hardware->TXDirty->hwTxSamplerAddressDirty, 0xFF, gcmSIZEOF(hardware->TXDirty->hwTxSamplerAddressDirty));
    gcoOS_MemFill(hardware->TXDirty->hwTxSamplerASTCDirty, 0xFF, gcmSIZEOF(hardware->TXDirty->hwTxSamplerASTCDirty));

    /* MC dirty reset. */
    hardware->MCDirty->hwTxSamplerTSDirty = hardware->MCDirty->texTileStatusSlotDirty = 0xFF;
    hardware->MCDirty->cacheDirty= gcvTRUE;

    /* Query dirty reset. */
    hardware->QUERYDirty->queryDirty[gcvQUERY_OCCLUSION] = gcvTRUE;

    if (hardware->features[gcvFEATURE_HW_TFB])
    {
        hardware->QUERYDirty->queryDirty[gcvQUERY_XFB_WRITTEN] = gcvTRUE;
        hardware->QUERYDirty->queryDirty[gcvQUERY_PRIM_GENERATED] = gcvTRUE;

        /* XFB dirty reset. */
        hardware->XFBDirty->xfbDirty = 0xFFFFFFFF;
    }

    /* Free temp states. */
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, FEStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, PAAndSEStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, MsaaStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, SHStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, PEStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, TXStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, MCStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, QueryStates));
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, XfbStates));

    if (hardware->PEStates->colorStates.target[0].surface != gcvNULL)
    {
        if (hardware->PEStates->colorStates.target[0].surface->tileStatusNode.pool != gcvPOOL_UNKNOWN)
        {
            gcmGETHARDWAREADDRESS(hardware->PEStates->colorStates.target[0].surface->tileStatusNode, tileStatusAddress);
        }
        else
        {
            tileStatusAddress = 0;
        }
        /* Enable the tile status for the requested surface. */
        status = gcoHARDWARE_EnableTileStatus(
            hardware,
            hardware->PEStates->colorStates.target[0].surface,
            tileStatusAddress,
            &hardware->PEStates->colorStates.target[0].surface->hzTileStatusNode,
            0);
    }
#endif

OnError:
#if !gcdENABLE_APPCTXT_BLITDRAW
    if (savedHardware)
    {
        gcmVERIFY_OK(gcoHARDWARE_Set3DHardware(savedHardware));
    }
#endif
    return status;
}

gceSTATUS
gcoHARDWARE_SetChipEnable(
    IN gcoHARDWARE Hardware,
    IN gceCORE_3D_MASK ChipEnable
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x ChipEnable=%d",
                  Hardware, ChipEnable);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    Hardware->chipEnable = ChipEnable;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_GetChipEnable(
    IN gcoHARDWARE Hardware,
    OUT gceCORE_3D_MASK *ChipEnable
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x ChipEnable=%d",
                  Hardware, ChipEnable);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    *ChipEnable = Hardware->chipEnable;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetMultiGPUMode(
    IN gcoHARDWARE Hardware,
    IN gceMULTI_GPU_MODE MultiGPUMode
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x MultiGPUMode=%d",
                  Hardware, MultiGPUMode);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    Hardware->gpuMode = MultiGPUMode;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_GetMultiGPUMode(
    IN gcoHARDWARE Hardware,
    OUT gceMULTI_GPU_MODE *MultiGPUMode
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Mode=%d",
                  Hardware, MultiGPUMode);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    *MultiGPUMode = Hardware->gpuMode;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_QueryReset(
    IN gcoHARDWARE Hardware,
    OUT gctBOOL_PTR Innocent
    )
{
    gceSTATUS status = gcvSTATUS_FALSE;
    gctUINT64 resetTimeStamp;
    gctUINT64 contextID;

    gcmHEADER_ARG("Hardware=0x%x Mode=0x%x",
                  Hardware, Innocent);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmONERROR(gcoHAL_QueryResetTimeStamp(&resetTimeStamp, &contextID));

    if (resetTimeStamp != Hardware->resetTimeStamp)
    {
        Hardware->resetTimeStamp = resetTimeStamp;

        if ((contextID == Hardware->context) && Innocent)
        {
            *Innocent = gcvFALSE;
        }

        status = gcvSTATUS_TRUE;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}


#endif

gceSTATUS
gcoHARDWARE_GetProductName(
    IN gcoHARDWARE Hardware,
    IN OUT gctSTRING *ProductName
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctSTRING chipName;
    gctUINT i;
    gctBOOL foundID = gcvFALSE;
    gctUINT32 chipID;
    gctPOINTER pointer;
    gctSTRING  chipNameBase;

    gcmHEADER_ARG("Hardware=0x%x ProductName=0x%x", Hardware, ProductName);
    gcmVERIFY_ARGUMENT(ProductName != gcvNULL);
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmONERROR(gcoOS_Allocate(gcvNULL, 32, &pointer));

    gcoOS_ZeroMemory(pointer, 32);

    chipNameBase =
    chipName     = (gctSTRING) pointer;

    if (Hardware->features[gcvFEATURE_HAS_PRODUCTID])
    {
        gctUINT32 productID = Hardware->config->productID;
        gctUINT32 type = (((((gctUINT32) (productID)) >> (0 ? 27:24)) & ((gctUINT32) ((((1 ? 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1)))))) );
        gctUINT32 grade = (((((gctUINT32) (productID)) >> (0 ? 3:0)) & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1)))))) );

        chipID = (((((gctUINT32) (productID)) >> (0 ? 23:4)) & ((gctUINT32) ((((1 ? 23:4) - (0 ? 23:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:4) - (0 ? 23:4) + 1)))))) );

        switch (type)
        {
        case 0:
            *chipName++ = 'G';
            *chipName++ = 'C';
            break;
        case 1:
            *chipName++ = 'D';
            *chipName++ = 'E';
            *chipName++ = 'C';
            break;
        case 2:
            *chipName++ = 'D';
            *chipName++ = 'C';
            break;
        case 3:
            *chipName++ = 'V';
            *chipName++ = 'G';
            break;
        case 4:
            *chipName++ = 'S';
            *chipName++ = 'C';
            break;
        case 5:
            *chipName++ = 'V';
            *chipName++ = 'P';
            break;
        default:
            *chipName++ = '?';
            *chipName++ = '?';
            gcmPRINT("GAL: Invalid product type");
            break;
        }

        /* Translate the ID. */
        for (i = 0; i < 8; i++)
        {
            /* Get the current digit. */
            gctUINT8 digit = (gctUINT8) ((chipID >> 28) & 0xFF);

            /* Append the digit to the string. */
            if (foundID || digit)
            {
                *chipName++ = '0' + digit;
                foundID = gcvTRUE;
            }

            /* Advance to the next digit. */
            chipID <<= 4;
        }

        switch (grade)
        {
        case 0:             /* Normal id */
        default:
            break;

        case 1:
            gcoOS_StrCatSafe(chipNameBase, 32, "Nano");
            break;

        case 2:
            gcoOS_StrCatSafe(chipNameBase, 32, "L");
            break;

        case 3:
            gcoOS_StrCatSafe(chipNameBase, 32, "UL");
            break;

        case 4:
            gcoOS_StrCatSafe(chipNameBase, 32, "XS");
            break;

        case 5:
            gcoOS_StrCatSafe(chipNameBase, 32, "NanoUltra");
            break;

        case 6:
            gcoOS_StrCatSafe(chipNameBase, 32, "NanoLite");
            break;

        case 7:
            gcoOS_StrCatSafe(chipNameBase, 32, "NanoUltra3");
            break;

        case 8:
            gcoOS_StrCatSafe(chipNameBase, 32, "XSVX");
            break;

        case 9:
            gcoOS_StrCatSafe(chipNameBase, 32, "NanoUltra2");
            break;

        case 10:
            gcoOS_StrCatSafe(chipNameBase, 32, "LVX");
            break;

        case 11:
            gcoOS_StrCatSafe(chipNameBase, 32, "LXSVX");
            break;

        case 12:
            gcoOS_StrCatSafe(chipNameBase, 32, "ULXS");
            break;

        case 13:
            gcoOS_StrCatSafe(chipNameBase, 32, "VX");
            break;

        case 14:
            gcoOS_StrCatSafe(chipNameBase, 32, "LVX");
            break;

        case 15:
            gcoOS_StrCatSafe(chipNameBase, 32, "ULVX");
            break;
        }
    }
    else
    {
        chipID = Hardware->config->chipModel;

        if (Hardware->config->chipFlags & gcvCHIP_FLAG_GC2000_R2)
        {
            chipID = gcv2000;
        }

        *chipName++ = 'G';
        *chipName++ = 'C';

        /* Translate the ID. */
        for (i = 0; i < 8; i++)
        {
            /* Get the current digit. */
            gctUINT8 digit = (gctUINT8) ((chipID >> 28) & 0xFF);

            /* Append the digit to the string. */
            if (foundID || digit)
            {
                *chipName++ = '0' + digit;
                foundID = gcvTRUE;
            }

            /* Advance to the next digit. */
            chipID <<= 4;
        }

        if (Hardware->config->chipFlags & gcvCHIP_FLAG_GC2000_R2)
        {
            *chipName++ ='+';
        }
    }

    gcoOS_StrDup(gcvNULL, pointer, ProductName);

    gcmOS_SAFE_FREE(gcvNULL, pointer);

    gcmFOOTER_ARG("ProductName=%s", *ProductName);
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER_ARG("status=%d", status);
    return status;
}



/****************************************************************************************************
**
** Following 9 functions are for calc addr of different memory layouts
** The surface are already locked before calling these function. Even no need to check their lockness
** insider because the function will be called lots of times.
**
****************************************************************************************************/

void _CalcPixelAddr_Linear(
    gcoSURF surface,
    gctSIZE_T x,
    gctSIZE_T y,
    gctSIZE_T z,
    gctPOINTER addr[gcdMAX_SURF_LAYERS]
    )
{
    gctUINT32 index;
    gctUINT32 layers = surface->formatInfo.layers;
    gctUINT8_PTR addr0 = surface->node.logical
                       + surface->sliceSize * z
                       + (x * surface->bitsPerPixel / 8 + y * surface->stride) / layers;

    for (index = 0; index < layers; index++)
    {
        addr[index] = addr0 + surface->layerSize * index;
    }

    return;
}

void _CalcPixelAddr_Tiled(
    gcoSURF surface,
    gctSIZE_T x,
    gctSIZE_T y,
    gctSIZE_T z,
    gctPOINTER addr[gcdMAX_SURF_LAYERS]
    )
{
    /* Tiled 0: { X[21-3], Y[1:0],X[1:0] } */
    gctSIZE_T xt = ((x & 0x03) << 0) | ((y & 0x03) << 2) | ((x & ~0x03) << 2);
    gctSIZE_T yt = (y & ~0x03);
    gctSIZE_T offsetInPixels = xt + yt * surface->alignedW;
    gctUINT32 index;
    gctUINT32 layers = surface->formatInfo.layers;
    gctUINT8_PTR addr0 = surface->node.logical
                       + surface->sliceSize * z
                       + offsetInPixels * (surface->bitsPerPixel / 8) / layers;

    for (index = 0; index < layers; index++)
    {
        addr[index] = addr0 + surface->layerSize * index;
    }

    return;
}

void _CalcPixelAddr_MultiTiled(
    gcoSURF surface,
    gctSIZE_T x,
    gctSIZE_T y,
    gctSIZE_T z,
    gctPOINTER addr[gcdMAX_SURF_LAYERS]
    )
{
    /* determine which part? top buffer or bottom. */
    gctUINT8_PTR baseAddr = (((x >> 3) ^ (y >> 2)) & 0x01)
                          ? surface->node.logicalBottom
                          : surface->node.logical;

    /* Calc coord in split surface */
    gctSIZE_T xs = (x & ~0x8) | ((y & 0x4) << 1);
    gctSIZE_T ys = ((y & ~0x7) >> 1) | (y & 0x3);

    /* Tiled 0: { X[21-3], Y[1:0],X[1:0] } */
    gctSIZE_T xt = ((xs & 0x03) << 0) | ((ys & 0x03) << 2) | ((xs & ~0x03) << 2);
    gctSIZE_T yt = (ys & ~0x03);

    gctSIZE_T offsetInPixels = xt + yt * surface->alignedW;

    gctUINT32 index;
    gctUINT32 layers = surface->formatInfo.layers;
    gctUINT8_PTR addr0 = baseAddr
                       + surface->sliceSize * z
                       + offsetInPixels * (surface->bitsPerPixel / 8) / layers;

    for (index = 0; index < layers; index++)
    {
        addr[index] = addr0 + surface->layerSize * index;
    }

    return;
}


/****************************************************************************************************
** Following 3 functions are for SuperTile w/o split.
****************************************************************************************************/

void _CalcPixelAddr_SuperTiled_Mode0(
    gcoSURF surface,
    gctSIZE_T x,
    gctSIZE_T y,
    gctSIZE_T z,
    gctPOINTER addr[gcdMAX_SURF_LAYERS]
    )
{
    /* SuperTileMode 0: { X[21-6], Y[5:2],X[5:2], Y[1:0],X[1:0] } */
    gctSIZE_T xt = ((x &  0x03) << 0) |
                   ((y &  0x03) << 2) |
                   ((x &  0x3C) << 2) |
                   ((y &  0x3C) << 6) |
                   ((x & ~0x3F) << 6);
    gctSIZE_T yt = (y & ~0x3F);
    gctSIZE_T offsetInPixels = xt + yt * surface->alignedW;
    gctUINT32 index;
    gctUINT32 layers = surface->formatInfo.layers;
    gctUINT8_PTR addr0 = surface->node.logical
                       + surface->sliceSize * z
                       + offsetInPixels * (surface->bitsPerPixel / 8) / layers;

    for (index = 0; index < layers; index++)
    {
        addr[index] = addr0 + surface->layerSize * index;
    }

    return;
}

void _CalcPixelAddr_SuperTiled_Mode1(
    gcoSURF surface,
    gctSIZE_T x,
    gctSIZE_T y,
    gctSIZE_T z,
    gctPOINTER addr[gcdMAX_SURF_LAYERS]
    )
{
    /* SuperTileMode 1: { X[21-1:6], Y[5:4],X[5:3], Y[3:2],X[2], Y[1:0],X[1:0] } */
    gctSIZE_T xt = ((x &  0x03) << 0) |
                   ((y &  0x03) << 2) |
                   ((x &  0x04) << 2) |
                   ((y &  0x0C) << 3) |
                   ((x &  0x38) << 4) |
                   ((y &  0x30) << 6) |
                   ((x & ~0x3F) << 6);
    gctSIZE_T yt = (y & ~0x3F);
    gctSIZE_T offsetInPixels = xt + yt * surface->alignedW;

    gctUINT32 index;
    gctUINT32 layers = surface->formatInfo.layers;
    gctUINT8_PTR addr0 = surface->node.logical
                       + surface->sliceSize * z
                       + offsetInPixels * (surface->bitsPerPixel / 8) / layers;

    for (index = 0; index < layers; index++)
    {
        addr[index] = addr0 + surface->layerSize * index;
    }

    return;
}

void _CalcPixelAddr_SuperTiled_Mode2(
    gcoSURF surface,
    gctSIZE_T x,
    gctSIZE_T y,
    gctSIZE_T z,
    gctPOINTER addr[gcdMAX_SURF_LAYERS]
    )
{
    /* SuperTileMode 2: { X[21-1:6], Y[5],X[5], Y[4],X[4], Y[3],X[3], Y[2],X[2], Y[1:0],X[1:0] } */
    gctSIZE_T xt = ((x &  0x03) << 0) |
                   ((y &  0x03) << 2) |
                   ((x &  0x04) << 2) |
                   ((y &  0x04) << 3) |
                   ((x &  0x08) << 3) |
                   ((y &  0x08) << 4) |
                   ((x &  0x10) << 4) |
                   ((y &  0x10) << 5) |
                   ((x &  0x20) << 5) |
                   ((y &  0x20) << 6) |
                   ((x & ~0x3F) << 6);
    gctSIZE_T yt = (y & ~0x3F);
    gctSIZE_T offsetInPixels = xt + yt * surface->alignedW;

    gctUINT32 index;
    gctUINT32 layers = surface->formatInfo.layers;
    gctUINT8_PTR addr0 = surface->node.logical
                       + surface->sliceSize * z
                       + offsetInPixels * (surface->bitsPerPixel / 8) / layers;

    for (index = 0; index < layers; index++)
    {
        addr[index] = addr0 + surface->layerSize * index;
    }

    return;
}


/****************************************************************************************************
** Following 3 functions are for MultiSuperTile
** They do NOT support 3D, because cannot program 3D texture with split layout .
****************************************************************************************************/

void _CalcPixelAddr_MultiSuperTiled_Mode0(
    gcoSURF surface,
    gctSIZE_T x,
    gctSIZE_T y,
    gctSIZE_T z,
    gctPOINTER addr[gcdMAX_SURF_LAYERS]
    )
{
    /* determine which part? top buffer or bottom. */
    gctUINT8_PTR baseAddr = (((x >> 3) ^ (y >> 2)) & 0x01)
                          ? surface->node.logicalBottom
                          : surface->node.logical;

    /* Calc coord in split surface */
    gctSIZE_T xs = (x & ~0x8) | ((y & 0x4) << 1);
    gctSIZE_T ys = ((y & ~0x7) >> 1) | (y & 0x3);

    /* SuperTileMode 0: { X[21-6], Y[5:2],X[5:2], Y[1:0],X[1:0] } */
    gctSIZE_T xt = ((xs &  0x03) << 0) |
                   ((ys &  0x03) << 2) |
                   ((xs &  0x3C) << 2) |
                   ((ys &  0x3C) << 6) |
                   ((xs & ~0x3F) << 6);
    gctSIZE_T yt = (ys & ~0x3F);

    gctSIZE_T offsetInPixels = xt + yt * surface->alignedW;

    gctUINT32 index;
    gctUINT32 layers = surface->formatInfo.layers;
    gctUINT8_PTR addr0 = baseAddr
                       + surface->sliceSize * z
                       + offsetInPixels * (surface->bitsPerPixel / 8) / layers;

    gcmASSERT(z==0);

    for (index = 0; index < layers; index++)
    {
        addr[index] = addr0 + surface->layerSize * index;
    }
    return;
}

void _CalcPixelAddr_MultiSuperTiled_Mode1(
    gcoSURF surface,
    gctSIZE_T x,
    gctSIZE_T y,
    gctSIZE_T z,
    gctPOINTER addr[gcdMAX_SURF_LAYERS]
    )
{
    /* determine which part? top buffer or bottom. */
    gctUINT8_PTR baseAddr = (((x >> 3) ^ (y >> 2)) & 0x01)
                          ? surface->node.logicalBottom
                          : surface->node.logical;

    /* Calc coord in split surface */
    gctSIZE_T xs = (x & ~0x8) | ((y & 0x4) << 1);
    gctSIZE_T ys = ((y & ~0x7) >> 1) | (y & 0x3);

    /* SuperTileMode 1: { X[21-1:6], Y[5:4],X[5:3], Y[3:2],X[2], Y[1:0],X[1:0] } */
    gctSIZE_T xt = ((xs &  0x03) << 0) |
                   ((ys &  0x03) << 2) |
                   ((xs &  0x04) << 2) |
                   ((ys &  0x0C) << 3) |
                   ((xs &  0x38) << 4) |
                   ((ys &  0x30) << 6) |
                   ((xs & ~0x3F) << 6);

    gctSIZE_T yt = (ys & ~0x3F);

    gctSIZE_T offsetInPixels = xt + yt * surface->alignedW;

    gctUINT32 index;
    gctUINT32 layers = surface->formatInfo.layers;
    gctUINT8_PTR addr0 = baseAddr
                       + surface->sliceSize * z
                       + offsetInPixels * (surface->bitsPerPixel / 8) / layers;

    gcmASSERT(z==0);

    for (index = 0; index < layers; index++)
    {
        addr[index] = addr0 + surface->layerSize * index;
    }

    return;
}

void _CalcPixelAddr_MultiSuperTiled_Mode2(
    gcoSURF surface,
    gctSIZE_T x,
    gctSIZE_T y,
    gctSIZE_T z,
    gctPOINTER addr[gcdMAX_SURF_LAYERS]
    )
{
    /* determine which part? top buffer or bottom. */
    gctUINT8_PTR baseAddr = (((x >> 3) ^ (y >> 2)) & 0x01)
                          ? surface->node.logicalBottom
                          : surface->node.logical;

    /* Calc coord in split surface */
    gctSIZE_T xs = (x & ~0x8) | ((y & 0x4) << 1);
    gctSIZE_T ys = ((y & ~0x7) >> 1) | (y & 0x3);

    /* SuperTileMode 2: { X[21-1:6], Y[5],X[5], Y[4],X[4], Y[3],X[3], Y[2],X[2], Y[1:0],X[1:0] } */
    gctSIZE_T xt = ((xs &  0x03) << 0) |
                   ((ys &  0x03) << 2) |
                   ((xs &  0x04) << 2) |
                   ((ys &  0x04) << 3) |
                   ((xs &  0x08) << 3) |
                   ((ys &  0x08) << 4) |
                   ((xs &  0x10) << 4) |
                   ((ys &  0x10) << 5) |
                   ((xs &  0x20) << 5) |
                   ((ys &  0x20) << 6) |
                   ((xs & ~0x3F) << 6);
    gctSIZE_T yt = (ys & ~0x3F);

    gctSIZE_T offsetInPixels = xt + yt * surface->alignedW;

    gctUINT32 index;
    gctUINT32 layers = surface->formatInfo.layers;
    gctUINT8_PTR addr0 = baseAddr
                       + surface->sliceSize * z
                       + offsetInPixels * (surface->bitsPerPixel / 8) / layers;

    gcmASSERT(z==0);

    for (index = 0; index < layers; index++)
    {
        addr[index] = addr0 + surface->layerSize * index;
    }
    return;
}


_PFNcalcPixelAddr gcoHARDWARE_GetProcCalcPixelAddr(
    IN gcoHARDWARE Hardware,
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    switch (Surface->tiling)
    {
    case gcvLINEAR:
    case gcvINVALIDTILED:   /* If surface is a wrapper surface, its tiling is not initialized. So it's treat as linear. */
        return _CalcPixelAddr_Linear;

    case gcvTILED:
        return _CalcPixelAddr_Tiled;

    case gcvMULTI_TILED:
        return _CalcPixelAddr_MultiTiled;

    case gcvSUPERTILED:
        gcmGETHARDWARE(Hardware);
        switch (Hardware->config->superTileMode)
        {
        case 0:
            return _CalcPixelAddr_SuperTiled_Mode0;
        case 1:
            return _CalcPixelAddr_SuperTiled_Mode1;
        case 2:
            return _CalcPixelAddr_SuperTiled_Mode2;
        default:
            gcmASSERT(0);
        }
        break;

    case gcvMULTI_SUPERTILED:
        gcmGETHARDWARE(Hardware);
        switch (Hardware->config->superTileMode)
        {
        case 0:
            return _CalcPixelAddr_MultiSuperTiled_Mode0;
        case 1:
            return _CalcPixelAddr_MultiSuperTiled_Mode1;
        case 2:
            return _CalcPixelAddr_MultiSuperTiled_Mode2;
        default:
            gcmASSERT(0);
        }
        break;

    case gcvYMAJOR_SUPERTILED:
        gcmASSERT(0);
        break;

    default:
        gcmASSERT(0);
    }

OnError:
    return gcvNULL;
}

