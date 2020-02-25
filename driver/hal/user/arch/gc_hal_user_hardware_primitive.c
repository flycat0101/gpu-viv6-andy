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


#include "gc_hal_user_hardware_precomp.h"

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcvZONE_HARDWARE

typedef enum __SPLIT_RECT_MODE
{
    SPLIT_RECT_MODE_NONE,
    SPLIT_RECT_MODE_COLUMN,
    SPLIT_RECT_MODE_LINE
} SPLIT_RECT_MODE;

#define SPLIT_COLUMN            4
#define SPLIT_COLUMN_WIDTH      (1 << SPLIT_COLUMN)
#define SPLIT_COLUMN_WIDTH_MASK (~(SPLIT_COLUMN_WIDTH - 1))

#if gcdENABLE_2D
/******************************************************************************
 *
 *  deRop3
 *
 *  Perform the specified raster operation on 32-bit data.
 *
 *  Input parameters:
 *
 *      gctUINT8 Rop3
 *          Raster operation code.
 *
 *      gctUINT32 Destination
 *          Destination pixel value.
 *
 *      gctUINT32 Source
 *          Source pixel values.
 *
 *      gctUINT32 Pattern
 *          Pattern value.
 *
 *  Return value:
 *
 *      gctUINT32
 *          32-bit result.
 *
 */
static gctUINT32 _Rop3(
    gctUINT8 Rop3,
    gctUINT32 Source,
    gctUINT32 Pattern,
    gctUINT32 Destination
    )
{
    gctUINT32 i;
    gctUINT32 result = 0;

    for (i = 0; i < 32; i++)
    {
        /* Extract data bits. */
        gctUINT32 sourceBit      = Source      & 1;
        gctUINT32 patternBit     = Pattern     & 1;
        gctUINT32 destinationBit = Destination & 1;

        /* Construct rop bit index. */
        gctUINT32 index
            = (sourceBit  << 1)
            | (patternBit << 2)
            | destinationBit;

        /* Determine the result bit. */
        gctUINT32 resultBit = (Rop3 >> index) & 1;

        /* Set the result. */
        result |= (resultBit << i);

        /* Advance to the next bit. */
        Source      >>= 1;
        Pattern     >>= 1;
        Destination >>= 1;
    }

    return result;
}
#endif

/*******************************************************************************
**
**  _RenderRectangle
**
**  2D software renderer.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gce2D_COMMAND Command
**          2D engine command to be executed.
**
**      gctUINT32 RectCount
**          Number of destination rectangles to be operated on.
**
**      gcsRECT_PTR DestRect
**          Pointer to an array of destination rectangles.
**
**      gctUINT32 FgRop
**      gctUINT32 BgRop
**          Foreground and background ROP codes.
**
**  OUTPUT:
**
**      Nothing .
*/
#if gcdENABLE_2D
static gceSTATUS _RenderRectangle(
    IN gcoHARDWARE Hardware,
    IN gcs2D_State_PTR State,
    IN gce2D_COMMAND Command,
    IN gctUINT32 RectCount,
    IN gcsRECT_PTR DestRect
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Command=%d RectCount=%u DestRect=0x%x",
                  Hardware, Command, RectCount, DestRect);

    /* Verify arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(DestRect != gcvNULL);

    do
    {
        gctINT32 initSrcLeft, initTrgLeft;
        gctUINT srcPixelSize, trgPixelSize;
        gctUINT srcStep, trgStep;
        gctINT32 x, y;
        gctINT32 width, height;
        gctUINT8_PTR srcLinePtr;
        gctUINT8_PTR trgLinePtr;
        gcsSURF_FORMAT_INFO_PTR srcFormatInfo;
        gcsSURF_FORMAT_INFO_PTR intFormatInfo;
        gcsSURF_FORMAT_INFO_PTR trgFormatInfo;
        gctUINT32 srcColorMask;
        gctUINT32 transparentColor;
        gctBOOL srcOddStart, trgOddStart;
        gctUINT32 srcPixel[2] = {0};
        gctUINT32 trgPixel[2] = {0};
        gcs2D_MULTI_SOURCE_PTR curSrc = &State->multiSrc[State->currentSrcIndex];
        gctUINT32 FgRop = curSrc->fgRop;
        gctUINT32 BgRop = curSrc->bgRop;
        gcoSURF srcSurface = &curSrc->srcSurface;
        gcsRECT_PTR sourceRect = &curSrc->srcRect;

        /* Only limited support for now. */
        if ((Command != gcv2D_BLT) ||
            (RectCount != 1))
        {
            gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
        }

        /* Commit any commands buffer and stall. */
        gcmERR_BREAK(gcoHARDWARE_Commit(Hardware));
        gcmERR_BREAK(gcoHARDWARE_Stall(Hardware));

        /* Get format specifics. */
        gcmERR_BREAK(gcoHARDWARE_QueryFormat(
            srcSurface->format, &srcFormatInfo
            ));
        srcSurface->formatInfo = *srcFormatInfo;

        gcmERR_BREAK(gcoHARDWARE_QueryFormat(
            State->dstSurface.format, &trgFormatInfo
            ));
        State->dstSurface.formatInfo = *trgFormatInfo;

        gcmERR_BREAK(gcoHARDWARE_QueryFormat(
            gcvSURF_A8R8G8B8, &intFormatInfo
            ));

        /* Determine the initial source and target left coordinates. */
        initSrcLeft = srcFormatInfo->interleaved
            ? ~1 & sourceRect->left
            :      sourceRect->left;

        initTrgLeft = trgFormatInfo->interleaved
            ? ~1 & DestRect->left
            :      DestRect->left;

        /* Determine odd start flags. */
        srcOddStart = 1 & sourceRect->left;
        trgOddStart = 1 & DestRect->left;

        /* Determine pixel sizes. */
        srcPixelSize = srcFormatInfo->bitsPerPixel / 8;
        trgPixelSize = trgFormatInfo->bitsPerPixel / 8;

        /* Determine horizontal steps. */
        srcStep = srcFormatInfo->interleaved
            ? srcPixelSize * 2
            : srcPixelSize;

        trgStep = trgFormatInfo->interleaved
            ? trgPixelSize * 2
            : trgPixelSize;

        /* Compute the rectangle size. */
        width  = DestRect->right  - DestRect->left;
        height = DestRect->bottom - DestRect->top;

        /* Compute initial position. */
        srcLinePtr
            = ((gctUINT8_PTR) srcSurface->node.logical)
            + sourceRect->top * srcSurface->stride
            + initSrcLeft * srcPixelSize;

        trgLinePtr
            = ((gctUINT8_PTR) State->dstSurface.node.logical)
            + DestRect->top * State->dstSurface.stride
            + initTrgLeft * trgPixelSize;

        /* Commpute source pixel mask. */
        gcmERR_BREAK(gcoSURF_ComputeColorMask(srcFormatInfo, &srcColorMask));

        /* Determine the transparency color. */
        transparentColor = curSrc->srcColorKeyLow & srcColorMask;

        /* Loop through pixels one at a time. */
        for (y = 0; (y < height) && gcmIS_SUCCESS(status); y++)
        {
            gctUINT8_PTR srcPixelPtr = srcLinePtr;
            gctUINT8_PTR trgPixelPtr = trgLinePtr;

            /* Determine initial even/odd. */
            gctBOOL srcOdd = srcFormatInfo->interleaved && srcOddStart;
            gctBOOL trgOdd = trgFormatInfo->interleaved && trgOddStart;

            for (x = 0; x < width; x++)
            {
                gctBOOL transparent = gcvFALSE;

                gctUINT32 convSrcPixel = 0;
                gctUINT32 resultPixel;
                gctUINT8 rop;

                /* Read the source and destination pixels. */
                if (!srcOdd || srcOddStart)
                {
                    gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                        srcPixelPtr, &srcPixel[0], 0, 0,
                        srcFormatInfo, srcFormatInfo,
                        gcvNULL, gcvNULL,
                        0, 0
                        ));

                    if (srcFormatInfo->interleaved)
                    {
                        gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                            srcPixelPtr, &srcPixel[1], 0, 0,
                            srcFormatInfo, srcFormatInfo,
                            gcvNULL, gcvNULL,
                            1, 1
                            ));
                    }
                }

                if (!trgOdd || trgOddStart)
                {
                    gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                        trgPixelPtr, &trgPixel[0], 0, 0,
                        trgFormatInfo, intFormatInfo,
                        gcvNULL, gcvNULL,
                        0, 0
                        ));

                    if (trgFormatInfo->interleaved)
                    {
                        gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                            trgPixelPtr, &trgPixel[1], 0, 0,
                            trgFormatInfo, intFormatInfo,
                            gcvNULL, gcvNULL,
                            1, 0
                            ));
                    }
                }

                /* Determine transparency. */
                if (curSrc->srcTransparency == gcv2D_KEYED)
                {
                    transparent = ((srcPixel[srcOdd] & srcColorMask) == transparentColor)
                        ? gcvTRUE
                        : gcvFALSE;
                }

                /* Determine the ROP code to be used. */
                rop = transparent ? (gctUINT8) BgRop : (gctUINT8) FgRop;

                /* Convert the source pixel to the intermediate format. */
                gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                    &srcPixel[srcOdd],
                    &convSrcPixel,
                    0, 0,
                    srcFormatInfo,
                    intFormatInfo,
                    gcvNULL, gcvNULL,
                    srcOdd, 0
                    ));

                /* Perform ROP. */
                resultPixel = _Rop3(rop, convSrcPixel, 0, trgPixel[trgOdd]);

                /* Write the result back. */
                gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                    &resultPixel,
                    trgPixelPtr,
                    0, 0,
                    intFormatInfo,
                    trgFormatInfo,
                    gcvNULL, gcvNULL,
                    0, trgOdd
                    ));

                /* Advance to the next pixel. */
                if (!srcFormatInfo->interleaved || srcOdd)
                    srcPixelPtr += srcStep;

                if (!trgFormatInfo->interleaved || trgOdd)
                    trgPixelPtr += trgStep;

                /* Update the odd flags. */
                srcOdd = (srcOdd + srcFormatInfo->interleaved) & 1;
                trgOdd = (trgOdd + trgFormatInfo->interleaved) & 1;
            }

            /* Advance to the next line. */
            srcLinePtr += srcSurface->stride;
            trgLinePtr += State->dstSurface.stride;
        }

        /* Dump the results. */
        gcmDUMP_BUFFER(gcvNULL,
                       gcvDUMP_BUFFER_MEMORY,
                       gcsSURF_NODE_GetHWAddress(&State->dstSurface.node),
                       State->dstSurface.node.logical,
                       0,
                       State->dstSurface.node.size);
    }
    while (gcvFALSE);

    /* Return status. */
    gcmFOOTER_NO();
    return status;
}

/******************************************************************************
 *
 *  _IsValidRect
 *
 *  Chech whether the rect is valid for HW operation.
 *
 *  Input parameters:
 *
 *      gcsRECT_PTR Rect
 *
 *  Return value:
 *
 *      gctBOOL
 *          gcvTRUE if valid; or else gcvFALSE.
 *
 */
static gctBOOL _IsValidRect(const gcsRECT_PTR Rect)
{
    return ((Rect->left   >= 0)
         && (Rect->top    >= 0)
         && (Rect->left   < Rect->right)
         && (Rect->top    < Rect->bottom)
         && (Rect->right  <= 65535)
         && (Rect->bottom <= 65535));
}

static gctUINT _ReserveBufferSize(
    IN gcoHARDWARE Hardware,
    IN gcs2D_State_PTR State,
    IN gce2D_COMMAND Command
    )
{
    gctUINT srcMask = 0, srcCurrent = 0, i;
    gctBOOL usePattern = gcvFALSE, usePallete = gcvFALSE;
    gcs2D_MULTI_SOURCE_PTR src;
    gctUINT size, getCmdSize, srcCompressNum = 0;
    gctBOOL inCSC, outCSC, deGamma = gcvFALSE;

    inCSC = outCSC = Hardware->features[gcvFEATURE_2D_COLOR_SPACE_CONVERSION] ?
        gcoHARDWARE_NeedUserCSC(State->dstYUVMode, State->dstSurface.format) : gcvFALSE;

    if (Command == gcv2D_MULTI_SOURCE_BLT)
    {
        srcMask = State->srcMask;
    }
    else
    {
        srcMask = 1 << State->currentSrcIndex;
    }

    for (i = 0; i < gcdMULTI_SOURCE_NUM; i++)
    {
        gctBOOL source, pattern;

        if (!(srcMask & (1 << i)))
        {
            continue;
        }

        src = State->multiSrc + i;

        /* Determine the resource usage. */
        gcoHARDWARE_Get2DResourceUsage(
            src->fgRop, src->bgRop,
            src->srcTransparency,
            &source, &pattern, gcvNULL
            );

        if (source)
        {
            usePallete = usePallete || (src->srcSurface.format == gcvSURF_INDEX8);
            srcCurrent++;
        }

        if (!inCSC && Hardware->features[gcvFEATURE_2D_COLOR_SPACE_CONVERSION])
        {
            inCSC = gcoHARDWARE_NeedUserCSC(src->srcYUVMode, src->srcSurface.format);
        }

        if (!deGamma && Hardware->features[gcvFEATURE_2D_GAMMA])
        {
            deGamma = src->srcDeGamma;
        }

        usePattern |= pattern;

        if (Hardware->features[gcvFEATURE_TPC_COMPRESSION] ||
            Hardware->features[gcvFEATURE_TPCV11_COMPRESSION])
        {
            if (src->srcSurface.tileStatusConfig & gcv2D_TSC_TPC_COMPRESSED)
            {
                srcCompressNum++;
            }
        }
    }

    size =
        /* common states. */
        46
        /* pattern. */
        + (usePattern ? 54 : 0)
        /* source. */
        + ((srcCurrent > 0) ? (srcCurrent * (Hardware->features[gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2] ? 72 : 68)) : 24)
        /* pallete. */
        + (usePallete ? 258 : 0)
        /* input CSC. */
        + (inCSC ? 10 : 0)
        /* output CSC. */
        + (outCSC ? 12 : 0)
        /* DeGAMMA.*/
        + (deGamma ? 258 : 0)
        /* EnGAMMA. */
        + (State->dstEnGamma && Hardware->features[gcvFEATURE_2D_GAMMA] ? 258 : 0)
        /* Initialize tile format. */
        + 8
        /* tail. */
        + 12;

    if (Hardware->hw3DEngine)
    {
        Hardware->hw2DCmdIndex += 16;
    }

    gcoHARDWARE_GetCompressionCmdSize(
        Hardware,
        State,
        gcvNULL, gcvNULL,
        srcCompressNum,
        Command,
        &getCmdSize);

    size += getCmdSize;

    if (Hardware->hw2DAppendCacheFlush && !srcCurrent)
    {
        size += 44;
    }

    if (Hardware->features[gcvFEATURE_2D_FC_SOURCE] ||
        Hardware->features[gcvFEATURE_2D_FAST_CLEAR] ||
        Hardware->features[gcvFEATURE_2D_V4COMPRESSION])
    {
        size += 10;
    }

    return size;
}

gceSTATUS gcoHARDWARE_Reset2DCmdBuffer(
    IN gcoHARDWARE Hardware,
    IN gctBOOL CleanCmd)
{
    gcmHEADER_ARG("Hardware=0x%x, CleanCmd=%d",
                    Hardware, CleanCmd);

    if (CleanCmd)
    {
        /* Clean command buffer. */
        if (Hardware->hw2DCmdBuffer != gcvNULL)
        {

            Hardware->hw2DCmdIndex = 0;

            /* Fill redundant buffer with NOP */
            if (Hardware->hw2DCmdSize > Hardware->hw2DCmdIndex)
            {
                gcmVERIFY_OK(gcoHARDWARE_2DAppendNop(Hardware));
            }

            gcmASSERT(Hardware->hw2DCmdSize == Hardware->hw2DCmdIndex);
        }
    }

    Hardware->hw2DCmdBuffer = gcvNULL;
    Hardware->hw2DCmdIndex = Hardware->hw2DCmdSize = 0;

    /* Return the status. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS gcoHARDWARE_Set2DState(
    IN gcoHARDWARE Hardware,
    IN gcs2D_State_PTR State,
    IN gce2D_COMMAND Command,
    IN gctBOOL  MultiDstRect
    )
{
    gceSTATUS status;
    gceSURF_FORMAT dstFormat;
    gctUINT32 command, destConfig;
    gctBOOL useSource, useDest, usePattern;
    gcs2D_MULTI_SOURCE_PTR src;
    gctUINT8 fgRop, bgRop;
    gctBOOL flushCache = gcvTRUE, setPattern = gcvTRUE, uploadPaletteTable = gcvTRUE;
    gctBOOL uploadCSC = gcvFALSE, uploadDeGamma = Hardware->features[gcvFEATURE_2D_GAMMA];
    gctUINT srcMask = 0, srcCurrent = 0, i;
    gctBOOL anyRot = gcvFALSE, anySrcTiled = gcvFALSE, anyDstTiled = gcvFALSE;
    gctUINT32 dstBpp, dstAddress;
    gctBOOL anyStretch = gcvFALSE, anyP010 = gcvFALSE, srcYUV420 = gcvFALSE, dstYUV420 = gcvFALSE;
    gctUINT32 anyUnalignedTo64 = 0, anyCompress = 0;
    gctUINT8 anyMultiSrcUnalignYTo64 = 0;
    gctUINT8 srcCacheType = 0;
    gcsSURF_FORMAT_INFO_PTR srcFormatInfo, dstFormatInfo;
    gctBOOL blf = Hardware->features[gcvFEATURE_2D_MULTI_SRC_BLT_BILINEAR_FILTER] ||
                  Hardware->features[gcvFEATURE_2D_MULTI_SRC_BLT_1_5_ENHANCEMENT];

    gcmHEADER_ARG("Hardware=0x%x State=%x Command=0x%x",
                    Hardware, State, Command);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    Hardware->enableXRGB = State->enableXRGB;

    /* Convert the command. */
    gcmONERROR(gcoHARDWARE_TranslateCommand(
        Command, &command
        ));

    /* Add Command into State. */
    State->command = Command;

    dstFormat = State->dstSurface.format;
    /* Compute bits per pixel. */
    gcmONERROR(gcoHARDWARE_ConvertFormat(dstFormat,
                                         &dstBpp,
                                         gcvNULL));

    gcmONERROR(gcoHARDWARE_Begin2DRender(
        Hardware,
        State));

    gcmONERROR(gcoHARDWARE_QueryFormat(State->dstSurface.format, &dstFormatInfo));

    if (Hardware->features[gcvFEATURE_2D_YUV420_OUTPUT_LINEAR])
    {
        if (dstFormatInfo->fmtClass == gcvFORMAT_CLASS_YUV)
        {
            for (i = 0; i < State->dstRectCount; i++)
            {
                /* Dest rect horizontal with YUV422 format must be even. */
                if ((State->dstRect[i].left & 0x1) || (State->dstRect[i].right & 0x1))
                {
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }

                /* Dest rect horizontal and vertical with YUV420 format must be even. */
                if (dstFormatInfo->closestTXFormat != gcvSURF_YUY2 &&
                    dstFormatInfo->closestTXFormat != gcvSURF_UYVY &&
                    ((State->dstRect[i].top & 0x1) || (State->dstRect[i].bottom & 0x1)))
                {
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
            }
        }
    }

    /* Set target surface */
    gcmONERROR(gcoHARDWARE_SetTarget(
        Hardware,
        &State->dstSurface,
        gcvFALSE,
        State->dstYUVMode,
        State->cscRGB2YUV,
        State->dstEnGamma ? State->enGamma : gcvNULL,
        State->multiSrc[State->currentSrcIndex].enableGDIStretch,
        &destConfig
        ));

    anyP010 = State->dstSurface.format == gcvSURF_P010;
    anyDstTiled = State->dstSurface.tiling != gcvLINEAR;

    gcmGETHARDWAREADDRESS(State->dstSurface.node, anyUnalignedTo64);
    anyUnalignedTo64 &= 0x3F;
    anyRot =
        (gcmGET_PRE_ROTATION(State->dstSurface.rotation) == gcvSURF_270_DEGREE)
        || (gcmGET_PRE_ROTATION(State->dstSurface.rotation) == gcvSURF_90_DEGREE);

    anyCompress = gcmHASCOMPRESSION(&State->dstSurface) ? 0x100 : 0;

    gcmGETHARDWAREADDRESS(State->dstSurface.node, dstAddress);

    destConfig |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ?
 15:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:12) - (0 ?
 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (command) & ((gctUINT32) ((((1 ?
 15:12) - (0 ?
 15:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));

    if (Hardware->hw2DBlockSize || Hardware->hw2DQuad)
    {
        if (Hardware->hw2DBlockSize)
        {
            if (Hardware->features[gcvFEATURE_BLOCK_SIZE_16x16])
            {
                gcmONERROR(gcoHARDWARE_Load2DState32(
                    Hardware,
                    0x01324,
                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (4) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (4) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                    ));
            }
            else if (Hardware->features[gcvFEATURE_TPCV11_COMPRESSION] && (anyCompress & 0x100))
            {
                gcmONERROR(gcoHARDWARE_Load2DState32(
                    Hardware,
                    0x01324,
                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (5) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (3) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                    ));
            }
            else
            {
                gcmONERROR(gcoHARDWARE_Load2DState32(
                    Hardware,
                    0x01324,
                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (3) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (3) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                    ));
            }
        }

        if (Hardware->hw2DQuad)
        {
            destConfig = ((((gctUINT32) (destConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28)));

            destConfig = ((((gctUINT32) (destConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:27) - (0 ?
 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ? 27:27)));
        }
    }

    /* Reset tile format. */
    {
        gctUINT32 kk = 0;

        for (kk = 0; kk < 8; kk++)
        {
            gcmONERROR(gcoHARDWARE_Load2DState32(
                Hardware,
                0x12A60 + 0x4 * kk,
                0x00000000
                ));
        }
    }

    if ((Command == gcv2D_MULTI_SOURCE_BLT) &&
        !Hardware->features[gcvFEATURE_2D_YUV420_OUTPUT_LINEAR] &&
        (State->dstSurface.format == gcvSURF_I420 || State->dstSurface.format == gcvSURF_YV12
        || State->dstSurface.format == gcvSURF_NV21 || State->dstSurface.format == gcvSURF_NV61
        || State->dstSurface.format == gcvSURF_NV12 || State->dstSurface.format == gcvSURF_NV16))
    {
        gcmONERROR(gcoHARDWARE_SetMultiTarget(
            Hardware,
            &State->dstSurface,
            State->multiSrc[State->currentSrcIndex].srcSurface.format
            ));
    }

    /* Set target color key range. */
    gcmONERROR(gcoHARDWARE_SetTargetColorKeyRange(
        Hardware,
        State->dstColorKeyLow,
        State->dstColorKeyHigh
        ));

    if (Command == gcv2D_CLEAR)
    {
        /* Set clear color. */
        gcmONERROR(gcoHARDWARE_Set2DClearColor(
            Hardware,
            State->clearColor,
            dstFormat
            ));

        gcmONERROR(gcoHARDWARE_Load2DState32(
            Hardware,
            0x012BC,
            ((((gctUINT32) (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))))) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
            ));
    }

    gcmONERROR(gcoHARDWARE_SetClipping(
        Hardware,
        &State->dstClipRect));

    /* Set 2D dithering. */
    gcmONERROR(gcoHARDWARE_SetDither2D(
        Hardware,
        State->dither
        ));

    gcmONERROR(gcoHARDWARE_SetSuperTileVersion(
        Hardware,
        State->superTileVersion
        ));

    if (Command == gcv2D_MULTI_SOURCE_BLT)
    {
        srcMask = State->srcMask;
    }
    else
    {
        srcMask = 1 << State->currentSrcIndex;
    }

    for (i = 0; i < gcdMULTI_SOURCE_NUM; i++)
    {
        gctBOOL forceSrc = gcvFALSE;
        gctUINT32 srcAddress;
        gce2D_TRANSPARENCY srcTransparency;

        if (!(srcMask & (1 << i)))
        {
            continue;
        }

        src = State->multiSrc + i;

        fgRop = src->fgRop;
        bgRop = src->bgRop;

        if (Hardware->features[gcvFEATURE_ANDROID_ONLY])
        {
            if (fgRop != bgRop ||
                (fgRop != 0xCC && fgRop != 0xF0 && fgRop != 0xAA))
            {
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }
        }

        srcTransparency = (Command == gcv2D_LINE ?
                           gcv2D_OPAQUE : src->srcTransparency);

        /* Determine the resource usage. */
        gcoHARDWARE_Get2DResourceUsage(
            fgRop, bgRop,
            srcTransparency,
            &useSource, &usePattern, &useDest
            );

        if (!useSource && Command == gcv2D_STRETCH)
        {
            destConfig = ((((gctUINT32) (destConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ?
 15:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:12) - (0 ?
 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 15:12) - (0 ?
 15:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
        }

        if (Hardware->hw2DAppendCacheFlush && !useSource)
        {
            bgRop |= 0xCC;
            forceSrc = gcvTRUE;
        }

        gcmGETHARDWAREADDRESS(src->srcSurface.node, srcAddress);

        if ((useSource || forceSrc) &&
            (Command == gcv2D_BLT || Command == gcv2D_STRETCH) &&
            srcAddress == dstAddress)
        {
            if (Hardware->features[gcvFEATURE_SEPARATE_SRC_DST] &&
                State->dstRectCount > 1)
            {
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }
        }

        if (Command != gcv2D_CLEAR &&
            Command != gcv2D_LINE  &&
            useSource)
        {
            gcmONERROR(gcoHARDWARE_QueryFormat(src->srcSurface.format, &srcFormatInfo));

            srcYUV420 = (srcFormatInfo->fmtClass == gcvFORMAT_CLASS_YUV &&
                         srcFormatInfo->txFormat == gcvINVALID_TEXTURE_FORMAT);
        }
        else
        {
            srcYUV420 = gcvFALSE;
        }

        if ((Hardware->features[gcvFEATURE_2D_SEPARATE_CACHE]
            || Hardware->features[gcvFEATURE_DEC400_COMPRESSION]) &&
            !srcCacheType)
        {
            if (Hardware->features[gcvFEATURE_DEC400_COMPRESSION] && srcYUV420)
            {
                srcCacheType = 1;
            }
            else if (gcmHASCOMPRESSION(&src->srcSurface) &&
                (Hardware->features[gcvFEATURE_2D_V4COMPRESSION] ||
                 Hardware->features[gcvFEATURE_DEC400_COMPRESSION]))
            {
                srcCacheType = 1;
            }
            else if (src->srcSurface.tiling != gcvLINEAR)
            {
                srcCacheType = 2;
            }
            else if (!Hardware->hw2DQuad ||
                     srcYUV420 ||
                     (src->srcSurface.stride & 0x3F) ||
                     (srcAddress & 0x3F))
            {
                srcCacheType = 3;
            }
        }

        if (flushCache)
        {
            /*******************************************************************
            ** Chips with byte write capability don't fetch the destiantion
            ** if it's not needed for the current operation. If the primitive(s)
            ** that follow overlap with the previous primitive causing a cache
            ** hit and they happen to use the destination to ROP with, it will
            ** cause corruption since the destination was not fetched in the
            ** first place.
            **
            ** Currently the hardware does not track this kind of case so we
            ** have to flush whenever we see a use of source or destination.
            */

            /* Flush 2D cache if needed. */
            if ((Hardware->byteWrite && (useSource || useDest))
                || Hardware->bigEndian)
            {
                /* Flush the current pipe. */
                gcmONERROR(gcoHARDWARE_Load2DState32(
                                              Hardware,
                                              0x0380C,
                                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))));

                flushCache = gcvFALSE;
            }
        }

        if (Command == gcv2D_STRETCH)
        {
            /* Set stretch factors. */
            gcmONERROR(gcoHARDWARE_SetStretchFactors(
                Hardware,
                src->horFactor,
                src->verFactor
                ));
        }

        if (usePattern && setPattern)
        {
            setPattern = gcvFALSE;

            if (!useSource)
            {
                gcmONERROR(gcoHARDWARE_Load2DState32(
                    Hardware,
                    0x012D8,
                    (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11))))
                    ));
            }

            switch (State->brushType)
            {
            case gcv2D_PATTERN_SOLID:
                gcmONERROR(gcoHARDWARE_LoadSolidColorPattern(
                    Hardware,
                    State->brushColorConvert,
                    State->brushFgColor,
                    State->brushMask,
                    dstFormat
                    ));
                break;

            case gcv2D_PATTERN_MONO:
                gcmONERROR(gcoHARDWARE_LoadMonochromePattern(
                    Hardware,
                    State->brushOriginX,
                    State->brushOriginY,
                    State->brushColorConvert,
                    State->brushFgColor,
                    State->brushBgColor,
                    State->brushBits,
                    State->brushMask,
                    dstFormat
                    ));
                break;

            case gcv2D_PATTERN_COLOR:
                gcmONERROR(gcoHARDWARE_LoadColorPattern(
                    Hardware,
                    State->brushOriginX,
                    State->brushOriginY,
                    State->brushAddress,
                    State->brushFormat,
                    State->brushMask
                    ));
                break;

            default:
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }
        }

        if (uploadDeGamma && src->srcDeGamma)
        {
            gcmONERROR(gcoHARDWARE_Load2DState(
                Hardware,
                0x13800,
                256,
                State->deGamma
                ));

            uploadDeGamma = gcvFALSE;
        }

        if (!uploadCSC)
        {
            uploadCSC = gcoHARDWARE_NeedUserCSC(src->srcYUVMode, src->srcSurface.format);
        }

        /* the old src registers. */
        if (Command != gcv2D_MULTI_SOURCE_BLT)
        {
            /* Set transparency mode */
            gcmONERROR(gcoHARDWARE_SetTransparencyModesEx(
                Hardware,
                srcTransparency,
                src->dstTransparency,
                src->patTransparency,
                fgRop,
                bgRop,
                src->enableDFBColorKeyMode
                ));

            /* Set YUV2RGB */
            if (Hardware->features[gcvFEATURE_2D_10BIT_OUTPUT_LINEAR])
            {
                gcmONERROR(gcoHARDWARE_Load2DState32(
                    Hardware,
                    0x012D8,
                    (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11))))
                ));
            }
            else
            {
                gcmONERROR(gcoHARDWARE_Load2DState32(
                    Hardware,
                    0x012D8,
                    (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11))))
                ));
            }

            /*******************************************************************
            ** Setup ROP.
            */
            gcmONERROR(gcoHARDWARE_Load2DState32(
                Hardware,
                0x0125C,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:20) - (0 ?
 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:8) - (0 ?
 15:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:8) - (0 ?
 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (bgRop) & ((gctUINT32) ((((1 ?
 15:8) - (0 ?
 15:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:0) - (0 ?
 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (fgRop) & ((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0)))
                ));

            /* Set target global color. */
            gcmONERROR(gcoHARDWARE_SetTargetGlobalColor(
                Hardware,
                src->dstGlobalColor
                ));

            if (Hardware->features[gcvFEATURE_TPC_COMPRESSION] && anyCompress)
            {
                gcmONERROR(gcoHARDWARE_Load2DState32(
                    Hardware,
                    0x01324,
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (4) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (4) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                ));
            }

            if (useSource)
            {
                if (Command == gcv2D_CLEAR)
                {
                    gcmONERROR(gcoHARDWARE_Load2DState32(
                        Hardware,
                        0x0120C,
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))
                        ));

                    gcmONERROR(gcoHARDWARE_SetSource(
                        Hardware,
                        Hardware->hw2DClearDestRect
                        ));

                    if (Hardware->hw2DClearDummySurf == gcvNULL)
                    {
                        gcmONERROR(gcoHARDWARE_Alloc2DSurface(
                            Hardware, 4, 4, gcvSURF_A8R8G8B8, 0, &Hardware->hw2DClearDummySurf));
                    }

                    gcmONERROR(gcoHARDWARE_SetColorSource(
                        Hardware,
                        Hardware->hw2DClearDummySurf,
                        gcvFALSE,
                        0,
                        gcvFALSE,
                        gcvFALSE,
                        gcvFALSE
                        ));
                }
                else
                {
                    gctUINT32 transparency;
                    gctUINT32 address;

                    /* Load the palette for Index8 source. */
                    if (src->srcSurface.format == gcvSURF_INDEX8)
                    {
                        gcmONERROR(gcoHARDWARE_LoadPalette(
                            Hardware,
                            State->paletteFirstIndex,
                            State->paletteIndexCount,
                            State->paletteTable,
                            State->paletteConvert,
                            dstFormat,
                            &State->paletteProgram,
                            &State->paletteConvertFormat
                            ));
                    }

                    /* Set src global color for A8 source. */
                    if (src->srcSurface.format == gcvSURF_A8)
                    {
                        gcmONERROR(gcoHARDWARE_SetSourceGlobalColor(
                            Hardware,
                            src->srcGlobalColor
                            ));
                    }

                    /* Use the src rect for the parameter setting stage */
                    gcmONERROR(gcoHARDWARE_SetSource(
                        Hardware,
                        &src->srcRect
                        ));

                    /* Get PE 1.0 transparency from new transparency modes. */
                    gcmONERROR(gcoHARDWARE_TranslateTransparencies(
                        Hardware,
                        srcTransparency,
                        src->dstTransparency,
                        src->patTransparency,
                        &transparency
                        ));

                    gcmGETHARDWAREADDRESS(src->srcSurface.node, address);
                    anyUnalignedTo64 |= address & 0x3F;
                    anyMultiSrcUnalignYTo64 |= src->srcRect.top & 0x3F;
                    anyP010 |= src->srcSurface.format == gcvSURF_P010;

                    if (gcmHASCOMPRESSION(&src->srcSurface))
                    {
                        anyCompress |= 1 << i;
                    }

                    if (!anyRot)
                    {
                        anyRot =
                            (gcmGET_PRE_ROTATION(src->srcSurface.rotation) == gcvSURF_270_DEGREE)
                             || (gcmGET_PRE_ROTATION(src->srcSurface.rotation) == gcvSURF_90_DEGREE);
                    }

                    anySrcTiled = (src->srcSurface.tiling != gcvLINEAR);

                    switch (src->srcType)
                    {
                    case gcv2D_SOURCE_MONO:
                        gcmONERROR(gcoHARDWARE_SetMonochromeSource(
                            Hardware,
                            (gctUINT8)src->srcMonoTransparencyColor,
                            src->srcMonoPack,
                            src->srcRelativeCoord,
                            src->srcFgColor,
                            src->srcBgColor,
                            src->srcColorConvert,
                            dstFormat,
                            src->srcStream,
                            transparency
                            ));
                        break;

                    case gcv2D_SOURCE_MASKED:
                        gcmONERROR(gcoHARDWARE_SetMaskedSource(
                            Hardware,
                            &src->srcSurface,
                            src->srcRelativeCoord,
                            src->srcMonoPack,
                            transparency
                            ));
                        break;

                    case gcv2D_SOURCE_COLOR:
                        gcmONERROR(gcoHARDWARE_SetColorSource(
                            Hardware,
                            &src->srcSurface,
                            src->srcRelativeCoord,
                            transparency,
                            src->srcYUVMode,
                            src->srcDeGamma,
                            gcvFALSE
                            ));

                        if ((Command == gcv2D_BLT)
                            || (Command == gcv2D_STRETCH))
                        {
                            if (Hardware->hw2DBlockSize || blf)
                            {
                                gctUINT32 power2BlockWidth = 3;
                                gctUINT32 power2BlockHeight = 3;
                                gctUINT32 srcBpp;

                                gcmONERROR(gcoHARDWARE_ConvertFormat(
                                    src->srcSurface.format,
                                    &srcBpp,
                                    gcvNULL));

                                /* change YUV bpp to 16 anyway. */
                                srcBpp = srcBpp == 12 ? 16 : srcBpp;
                                dstBpp = dstBpp == 12 ? 16 : dstBpp;

                                if (Hardware->features[gcvFEATURE_TPC_COMPRESSION] && anyCompress)
                                {
                                    power2BlockWidth = 4;
                                    power2BlockHeight = 4;
                                }
                                else if (Hardware->features[gcvFEATURE_TPCV11_COMPRESSION] &&
                                         ((anyCompress & 0x100) || anyP010))
                                {
                                    power2BlockWidth = 5;
                                    power2BlockHeight = 3;
                                }
                                else if (Hardware->features[gcvFEATURE_DEC400_COMPRESSION] &&
                                         (anyCompress & 0x100) &&
                                         State->dstSurface.tiling != gcvTILED_8X8_YMAJOR)
                                {
                                    switch (State->dstSurface.tiling)
                                    {
                                        case gcvTILED_8X8_XMAJOR:
                                            power2BlockWidth = 4;
                                            power2BlockHeight = 3;
                                            break;
                                        case gcvTILED_8X4:
                                        case gcvTILED_4X8:
                                            power2BlockWidth = 3;
                                            power2BlockHeight = 3;
                                            break;
                                        case gcvSUPERTILED:
                                        case gcvSUPERTILED_128B:
                                        case gcvSUPERTILED_256B:
                                            power2BlockWidth = 6;
                                            power2BlockHeight = 6;
                                            break;
                                        case gcvTILED_64X4:
                                            power2BlockWidth = 6;
                                            power2BlockHeight = 3;
                                            break;
                                        case gcvTILED_32X4:
                                            power2BlockWidth = 5;
                                            power2BlockHeight = 3;
                                            break;
                                        default:
                                            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                                    }
                                }
                                else if (Hardware->features[gcvFEATURE_2D_COMPRESSION])
                                {
                                    if (anySrcTiled)
                                    {
                                        if (anyRot)
                                        {
                                            power2BlockWidth = 4;
                                            power2BlockHeight = 4;
                                        }
                                        else
                                        {
                                            power2BlockWidth = 3;
                                            power2BlockHeight = 3;
                                        }
                                    }
                                    else if (anyRot)
                                    {
                                        if (anyCompress)
                                        {
                                            if (Hardware->config->chipModel == gcv320)
                                            {
                                                power2BlockWidth = 4;
                                                power2BlockHeight = 4;
                                            }
                                            else
                                            {
                                                power2BlockWidth = 3;
                                                power2BlockHeight = 4;
                                            }
                                        }
                                        else if (srcYUV420)
                                        {
                                            power2BlockWidth = 4;
                                            power2BlockHeight = 5;
                                        }
                                        else
                                        {
                                            power2BlockWidth = 4;
                                            power2BlockHeight = 4;
                                        }
                                    }
                                    else
                                    {
                                        power2BlockWidth = 6;
                                        power2BlockHeight = 3;
                                    }
                                }
                                else
                                {
                                    if (Hardware->features[gcvFEATURE_2D_V4COMPRESSION] && anyCompress)
                                    {
                                        if (dstBpp == 16)
                                        {
                                            if (State->dstSurface.tiling == gcvTILED)
                                            {
                                                power2BlockWidth = 5;
                                                power2BlockHeight = 2;
                                            }
                                            else if (State->dstSurface.tiling == gcvSUPERTILED)
                                            {
                                                power2BlockWidth = 4;
                                                power2BlockHeight = 3;
                                            }
                                            else if (State->dstSurface.tiling == gcvYMAJOR_SUPERTILED)
                                            {
                                                power2BlockWidth = 3;
                                                power2BlockHeight = 4;
                                            }
                                            else
                                            {
                                                /* default linear */
                                                power2BlockWidth = 4;
                                                power2BlockHeight = 1;
                                            }
                                        }
                                        else
                                        {
                                            if (State->dstSurface.tiling != gcvLINEAR)
                                            {
                                                power2BlockWidth = 3;
                                                power2BlockHeight = 3;
                                            }
                                            else
                                            {
                                                /* default linear */
                                                power2BlockWidth = 4;
                                                power2BlockHeight = 1;
                                            }
                                        }
                                    }
                                    else if (anyRot)
                                    {
                                        if ((srcBpp == 16) && (dstBpp == 16))
                                        {
                                            power2BlockWidth = 5;
                                            power2BlockHeight = 5;
                                        }
                                        else if ((srcBpp == 16) && (dstBpp == 32))
                                        {
                                            power2BlockWidth = 5;
                                            power2BlockHeight = 4;
                                        }
                                        else if ((srcBpp == 32) && (dstBpp == 32))
                                        {
                                            power2BlockWidth = 4;
                                            power2BlockHeight = 4;
                                        }
                                        else if ((srcBpp == 32) && (dstBpp == 16))
                                        {
                                            power2BlockWidth = 4;
                                            power2BlockHeight = 5;
                                        }
                                    }
                                    else if (anySrcTiled)
                                    {
                                        power2BlockWidth = 3;
                                        power2BlockHeight = 3;
                                    }
                                    else
                                    {
                                        power2BlockWidth = 7;
                                        power2BlockHeight = 3;
                                    }
                                }

                                if (!anyCompress &&
                                    Hardware->features[gcvFEATURE_BLOCK_SIZE_16x16])
                                {
                                    power2BlockWidth = 4;
                                    power2BlockHeight = 4;
                                }

                                /* Config block size. */
                                gcmONERROR(gcoHARDWARE_Load2DState32(
                                    Hardware,
                                    0x01324,
                                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (power2BlockWidth) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (power2BlockHeight) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                                    ));
                            }

                            if (Hardware->hw2DQuad && anyRot)
                            {
                                /* Config quad. */
                                if (Command == gcv2D_STRETCH)
                                {
                                    destConfig = ((((gctUINT32) (destConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:27) - (0 ?
 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ? 27:27)))
                                                | ((((gctUINT32) (destConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28)));
                                }
                                else
                                {
                                    if (dstFormat != gcvSURF_YUY2 &&
                                        dstFormat != gcvSURF_UYVY)
                                    {
                                        destConfig = ((((gctUINT32) (destConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28)))
                                                    | ((((gctUINT32) (destConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:27) - (0 ?
 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ? 27:27)));
                                    }
                                    else
                                    {
                                        destConfig = ((((gctUINT32) (destConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28)))
                                                    | ((((gctUINT32) (destConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:27) - (0 ?
 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ? 27:27)));
                                    }
                                }
                            }
                        }

                        /* Set src color key range */
                        if (srcTransparency == gcv2D_KEYED)
                        {
                            gcmONERROR(gcoHARDWARE_SetSourceColorKeyRange(
                                Hardware,
                                src->srcColorKeyLow,
                                src->srcColorKeyHigh,
                                gcvTRUE,
                                src->srcSurface.format
                                ));
                        }
                        break;

                    default:
                        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                    }
                }
            }
            else if (forceSrc)
            {
                gcsRECT srcRect;
                gctUINT32 transparency;

                srcRect.left = srcRect.top = 0;
                srcRect.right = gcmMAX(State->dstSurface.alignedW, 1024);
                srcRect.bottom = gcmMAX(State->dstSurface.alignedH, 768);

                if (!Hardware->hw2DDummySurf ||
                    (Hardware->hw2DDummySurf->node.size < (gctSIZE_T)(srcRect.right * srcRect.bottom * 2)))
                {
                    if (Hardware->hw2DDummySurf)
                    {
                        gcmONERROR(gcoHARDWARE_Free2DSurface(Hardware, Hardware->hw2DDummySurf));
                        Hardware->hw2DDummySurf = gcvNULL;
                    }

                    gcmONERROR(gcoHARDWARE_Alloc2DSurface(
                        Hardware, srcRect.right, srcRect.bottom, gcvSURF_A4R4G4B4, State->dstSurface.hints, &Hardware->hw2DDummySurf));

                    gcoOS_ZeroMemory(Hardware->hw2DDummySurf->node.logical, Hardware->hw2DDummySurf->node.size);

                    gcmONERROR(gcoOS_CacheFlush(
                        gcvNULL,
                        Hardware->hw2DDummySurf->node.u.normal.node,
                        Hardware->hw2DDummySurf->node.logical,
                        Hardware->hw2DDummySurf->node.size));
                }

                Hardware->hw2DDummySurf->alignedW = srcRect.right;
                Hardware->hw2DDummySurf->alignedH = srcRect.bottom;
                Hardware->hw2DDummySurf->stride = Hardware->hw2DDummySurf->alignedW * 2;

                srcRect.right = srcRect.bottom = 16;
                gcmONERROR(gcoHARDWARE_SetSource(
                    Hardware,
                    &srcRect
                    ));

                gcmONERROR(gcoHARDWARE_TranslateTransparencies(
                    Hardware,
                    srcTransparency,
                    src->dstTransparency,
                    src->patTransparency,
                    &transparency
                    ));

                gcmONERROR(gcoHARDWARE_SetColorSource(
                    Hardware,
                    Hardware->hw2DDummySurf,
                    gcvFALSE,
                    transparency,
                    gcvFALSE,
                    gcvFALSE,
                    gcvFALSE
                    ));
            }

            if (src->enableAlpha)
            {
                /* Set alphablend states */
                gcmONERROR(gcoHARDWARE_EnableAlphaBlend(
                    Hardware,
                    src->srcAlphaMode,
                    src->dstAlphaMode,
                    src->srcGlobalAlphaMode,
                    src->dstGlobalAlphaMode,
                    src->srcFactorMode,
                    src->dstFactorMode,
                    src->srcColorMode,
                    src->dstColorMode,
                    src->srcGlobalColor,
                    src->dstGlobalColor
                    ));
            }
            else
            {
                gcmONERROR(gcoHARDWARE_DisableAlphaBlend(Hardware));
            }

            /* Set multiply modes. */
            gcmONERROR(gcoHARDWARE_SetMultiplyModes(
                Hardware,
                src->srcPremultiplyMode,
                src->dstPremultiplyMode,
                src->srcPremultiplyGlobalMode,
                src->dstDemultiplyMode,
                src->srcGlobalColor
                ));

            /* Set mirror state. */
            gcmONERROR(gcoHARDWARE_SetBitBlitMirror(
                Hardware,
                State->multiSrc[State->currentSrcIndex].horMirror,
                State->multiSrc[State->currentSrcIndex].verMirror,
                anyCompress ? gcvFALSE : gcvTRUE
                ));
        }
        /* the multi src registers. */
        else
        {
            if (useSource)
            {
                gctUINT32 address;
                gcmGETHARDWAREADDRESS(src->srcSurface.node, address);
                anyUnalignedTo64 |= address & 0x3F;

                if (gcmHASCOMPRESSION(&src->srcSurface))
                {
                    anyCompress |= 1 << i;
                }

                if (!anyRot)
                {
                    anyRot =
                        (gcmGET_PRE_ROTATION(src->srcSurface.rotation) == gcvSURF_270_DEGREE)
                         || (gcmGET_PRE_ROTATION(src->srcSurface.rotation) == gcvSURF_90_DEGREE);
                }
            }

            /* Load the palette for Index8 source.
               It is shared by all the multi source. */
            if (uploadPaletteTable
                && (src->srcSurface.format == gcvSURF_INDEX8))
            {
                gcmONERROR(gcoHARDWARE_LoadPalette(
                    Hardware,
                    State->paletteFirstIndex,
                    State->paletteIndexCount,
                    State->paletteTable,
                    State->paletteConvert,
                    dstFormat,
                    &State->paletteProgram,
                    &State->paletteConvertFormat
                    ));

                uploadPaletteTable = gcvFALSE;
            }

            if (Hardware->features[gcvFEATURE_2D_MULTI_SOURCE_BLT_EX]
                || Hardware->features[gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2]
                || Hardware->features[gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT]
                || Hardware->features[gcvFEATURE_2D_MULTI_SRC_BLT_1_5_ENHANCEMENT]
                || Hardware->features[gcvFEATURE_2D_MULTI_SRC_BLT_BILINEAR_FILTER])
            {
                /* 8x source setting. */
                gcmONERROR(gcoHARDWARE_SetMultiSourceEx(
                    Hardware,
                    srcCurrent,
                    i,
                    State,
                    MultiDstRect));

                if (Hardware->features[gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2] &&
                    (src->srcRect.right  - src->srcRect.left != src->dstRect.right  - src->dstRect.left ||
                     src->srcRect.bottom  - src->srcRect.top != src->dstRect.bottom  - src->dstRect.top))
                {
                    anyStretch = gcvTRUE;
                }
            }
            else
            {
                /* 4x source setting. */
                gcmONERROR(gcoHARDWARE_SetMultiSource(
                    Hardware,
                    srcCurrent,
                    i,
                    State
                    ));
            }

            anySrcTiled = src->srcSurface.tiling != gcvLINEAR;
        }

        srcCurrent++;
    }

    gcmONERROR(gcoHARDWARE_CheckConstraint(
        Hardware,
        State,
        Command,
        anyCompress
        ));

    if (Command == gcv2D_MULTI_SOURCE_BLT)
    {
        gctUINT8 horBlk = 0, verBlk = 0;
        gctUINT32 config = ~0U;

        if (Hardware->features[gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT] || blf)
        {
            config = (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (State->unifiedDstRect || blf ?
 0x0 : 0x1) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:7) - (0 ?
 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7))));

            gcmONERROR(gcoHARDWARE_Load2DState32(
                Hardware,
                0x01328,
                config
                ));
        }

        if (Hardware->features[gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT] || blf)
        {
            gcmONERROR(gcoHARDWARE_Load2DState32(
                Hardware,
                0x01380,
                (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (State->unifiedDstRect || blf ?
 0x1 : 0x0) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))))
                ));
        }

        if (!anyCompress && Hardware->features[gcvFEATURE_BLOCK_SIZE_16x16])
        {
            horBlk = 0x0;
            verBlk = 0x4;
        }
        else if (Hardware->features[gcvFEATURE_TPC_COMPRESSION])
        {
            if (anyCompress)
            {
                horBlk = 0x0;
                verBlk = 0x4;
            }
            else
            {
                if (anySrcTiled || anyDstTiled)
                {
                    horBlk = 0x0;
                    verBlk = 0x4;
                }
                else if (anyRot)
                {
                    horBlk = 0x6;
                    verBlk = 0x4;
                }
                else
                {
                    horBlk = 0x3;
                    verBlk = 0x0;
                }
            }
        }
        else if (Hardware->features[gcvFEATURE_TPCV11_COMPRESSION] &&
                 ((anyCompress & 0x100) || anyP010))
        {
            horBlk = 0x1;
            verBlk = 0x3;
        }
        else if (Hardware->features[gcvFEATURE_DEC400_COMPRESSION] &&
                 (anyCompress & 0x100) &&
                 State->dstSurface.tiling != gcvTILED_8X8_YMAJOR)
        {
            switch (State->dstSurface.tiling)
            {
                case gcvTILED_8X8_XMAJOR:
                    horBlk = 0x0;
                    verBlk = 0x3;
                    break;
                case gcvTILED_8X4:
                case gcvTILED_4X8:
                    horBlk = 0x6;
                    verBlk = 0x3;
                    break;
                case gcvSUPERTILED:
                case gcvSUPERTILED_128B:
                case gcvSUPERTILED_256B:
                    horBlk = 0x2;
                    verBlk = 0x6;
                    break;
                case gcvTILED_64X4:
                    horBlk = 0x2;
                    verBlk = 0x3;
                    break;
                case gcvTILED_32X4:
                    horBlk = 0x1;
                    verBlk = 0x3;
                    break;
                default:
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }
        }
        else if (Hardware->features[gcvFEATURE_2D_COMPRESSION])
        {
            if (anyCompress)
            {
                if (anyRot)
                {
                    horBlk = 0x7;
                    verBlk = 0x4;
                }
                else if (anySrcTiled || anyDstTiled)
                {
                    if (Hardware->config->chipModel == gcv320 &&
                        anyMultiSrcUnalignYTo64)
                    {
                        horBlk = 0x2;
                        verBlk = 0x2;
                    }
                    else
                    {
                        horBlk = 0x2;
                        verBlk = 0x3;
                    }
                }
                else
                {
                    if (Hardware->config->chipModel == gcv320)
                    {
                        horBlk = 0x2;
                        verBlk = 0x0;
                    }
                    else
                    {
                        horBlk = 0x3;
                        verBlk = 0x0;
                    }
                }
            }
            else
            {
                if (anySrcTiled || anyDstTiled)
                {
                    horBlk = 0x0;
                    verBlk = 0x4;
                }
                else if (anyRot)
                {
                    horBlk = 0x0;
                    verBlk = 0x4;
                }
                else
                {
                    if (Hardware->config->chipModel == gcv320)
                    {
                        horBlk = 0x2;
                        verBlk = 0x0;
                    }
                    else
                    {
                        horBlk = 0x3;
                        verBlk = 0x0;
                    }
                }
            }
        }
        else
        {
            if (Hardware->features[gcvFEATURE_2D_V4COMPRESSION] &&
                State->dstSurface.tileStatusConfig & gcv2D_TSC_V4_COMPRESSED)
            {
                if (dstBpp == 16)
                {
                    if (State->dstSurface.tiling == gcvTILED)
                    {
                        horBlk = 0x1;
                        verBlk = 0x2;
                    }
                    else if (State->dstSurface.tiling == gcvSUPERTILED)
                    {
                        horBlk = 0x0;
                        verBlk = 0x3;
                    }
                    else if (State->dstSurface.tiling == gcvYMAJOR_SUPERTILED)
                    {
                        horBlk = 0x6;
                        verBlk = 0x4;
                    }
                    else
                    {
                        /* default linear */
                        horBlk = 0x0;
                        verBlk = 0x0;
                    }
                }
                else
                {
                    if (State->dstSurface.tiling != gcvLINEAR)
                    {
                        horBlk = 0x6;
                        verBlk = 0x3;
                    }
                    else
                    {
                        /* default linear */
                        horBlk = 0x0;
                        verBlk = 0x0;
                    }
                }
            }
            else if (anySrcTiled || anyDstTiled)
            {
                horBlk = 0x0;
                verBlk = 0x4;
            }
            else if (anyRot)
            {
                if (anyUnalignedTo64)
                {
                    horBlk = 0x0;
                    verBlk = 0x3;
                }
                else
                {
                    horBlk = 0x0;
                    verBlk = 0x4;
                }
            }
            else
            {
                if (Hardware->config->chipModel == gcv320)
                {
                    horBlk = 0x2;
                    verBlk = 0x0;
                }
                else
                {
                    horBlk = 0x3;
                    verBlk = 0x0;
                }
            }
        }

        if (Hardware->config->chipModel == gcv300 && (anySrcTiled || anyDstTiled))
        {
            horBlk = 0x0;
            verBlk = 0x3;
        }

        config = ((((gctUINT32) (0x00000000)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (srcCurrent - 1) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                 | ((((gctUINT32) (0x00000000)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:8) - (0 ?
 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (horBlk) & ((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ? 10:8)))
                | ((((gctUINT32) (0x00000000)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (verBlk) & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)));

        if (Hardware->features[gcvFEATURE_2D_MULTI_SRC_BLT_1_5_ENHANCEMENT] ||
            Hardware->features[gcvFEATURE_2D_MULTI_SRC_BLT_BILINEAR_FILTER])
        {
            if (Hardware->features[gcvFEATURE_2D_MULTI_SRC_BLT_BILINEAR_FILTER] &&
                State->multiBilinearFilter)
            {
                config |= ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:21) - (0 ?
 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ? 21:21)))
                          | ((((gctUINT32) (0x00000000)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22)));
            }
            else
            {
                config |= ((((gctUINT32) (config)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:21) - (0 ?
 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ? 21:21)))
                          | ((((gctUINT32) (0x00000000)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22)));
            }
        }

        gcmONERROR(gcoHARDWARE_Load2DState32(
            Hardware,
            0x01308,
            config
            ));

        if (Hardware->hw2DQuad)
        {
            if (anySrcTiled || anyDstTiled)
            {
                destConfig = ((((gctUINT32) (destConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28)));
            }
            else if (anyRot)
            {
                if (anyStretch)
                {
                    destConfig = ((((gctUINT32) (destConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28)))
                                | ((((gctUINT32) (destConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:27) - (0 ?
 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ? 27:27)));
                }
                else
                {
                    destConfig = ((((gctUINT32) (destConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28)))
                                | ((((gctUINT32) (destConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:27) - (0 ?
 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ? 27:27)));
                }
            }
        }
    }

    {
        /* Always enable new walker.*/
        destConfig = ((((gctUINT32) (destConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)));
    }

    gcmONERROR(gcoHARDWARE_SetCompression(
        Hardware,
        State,
        &State->multiSrc[State->currentSrcIndex].srcSurface,
        &State->dstSurface,
        Command,
        anyCompress & 0xFF,
        gcmHASCOMPRESSION(&State->dstSurface)));

    /* Set overlap feature for gc520. */
    if (Hardware->features[gcvFEATURE_SEPARATE_SRC_DST])
    {
        gcmONERROR(gcoHARDWARE_Load2DState32(
            Hardware,
            0x01328,
            (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:9) - (0 ?
 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9)))) &
            (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:7) - (0 ?
 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7))))
            ));
    }

    if (Hardware->features[gcvFEATURE_2D_SEPARATE_CACHE])
    {
        dstYUV420 = (dstFormatInfo->fmtClass == gcvFORMAT_CLASS_YUV &&
                     dstFormatInfo->txFormat == gcvINVALID_TEXTURE_FORMAT);

        if (srcCacheType == 1)
        {
            gcmONERROR(gcoHARDWARE_Load2DState32(
                Hardware,
                0x01328,
                (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:15) - (0 ?
 16:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:15) - (0 ?
 16:15) + 1))))))) << (0 ?
 16:15))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 16:15) - (0 ?
 16:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:15) - (0 ?
 16:15) + 1))))))) << (0 ?
 16:15))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:14) - (0 ?
 14:14) + 1))))))) << (0 ?
 14:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14))))
                ));
        }
        else if (Hardware->hw2DQuad &&
                 srcCacheType != 3 &&
                 !dstYUV420 &&
                 !(dstAddress & 0x3F) &&
                 !(State->dstSurface.stride & 0x3F))
        {
            gcmONERROR(gcoHARDWARE_Load2DState32(
                Hardware,
                0x01328,
                (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:15) - (0 ?
 16:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:15) - (0 ?
 16:15) + 1))))))) << (0 ?
 16:15))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 16:15) - (0 ?
 16:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:15) - (0 ?
 16:15) + 1))))))) << (0 ?
 16:15))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:14) - (0 ?
 14:14) + 1))))))) << (0 ?
 14:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14))))
                ));
        }
        else
        {
            gcmONERROR(gcoHARDWARE_Load2DState32(
                Hardware,
                0x01328,
                (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:15) - (0 ?
 16:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:15) - (0 ?
 16:15) + 1))))))) << (0 ?
 16:15))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 16:15) - (0 ?
 16:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:15) - (0 ?
 16:15) + 1))))))) << (0 ?
 16:15))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:14) - (0 ?
 14:14) + 1))))))) << (0 ?
 14:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14))))
                ));
        }

        if ((State->dstSurface.tileStatusConfig & gcv2D_TSC_V4_COMPRESSED) ||
            dstYUV420)
        {
            gcmONERROR(gcoHARDWARE_Load2DState32(
                Hardware,
                0x01328,
                (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11))))
                ));
        }
        else if (!Hardware->hw2DQuad ||
                 (State->dstSurface.stride & 0x3F) ||
                 ((State->dstSurface.tiling == gcvLINEAR) && (dstAddress & 0x3F)) ||
                 ((State->dstSurface.tiling != gcvLINEAR) && (dstAddress & 0x7F)))
        {
            gcmONERROR(gcoHARDWARE_Load2DState32(
                Hardware,
                0x01328,
                (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11))))
                ));
        }
        else
        {
            gcmONERROR(gcoHARDWARE_Load2DState32(
                Hardware,
                0x01328,
                (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11))))
                ));
        }
    }
    else if (Hardware->features[gcvFEATURE_2D_CACHE_128B256BPERLINE])
    {
        if (anySrcTiled || anyDstTiled)
        {
            gcmONERROR(gcoHARDWARE_Load2DState32(
                Hardware,
                0x01328,
                (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11))))
                ));
        }
        else
        {
            gcmONERROR(gcoHARDWARE_Load2DState32(
                Hardware,
                0x01328,
                (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11))))
                ));
        }
    }
    else if (Hardware->features[gcvFEATURE_DEC400_COMPRESSION])
    {
        gcmONERROR(gcoHARDWARE_Load2DState32(
            Hardware,
            0x01328,
            (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:18) - (0 ?
 20:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:18) - (0 ?
 20:18) + 1))))))) << (0 ?
 20:18))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 20:18) - (0 ?
 20:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:18) - (0 ?
 20:18) + 1))))))) << (0 ?
 20:18))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:17) - (0 ?
 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17))))
            ));

        gcmONERROR(gcoHARDWARE_Load2DState32(
            Hardware,
            0x01328,
            (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:23) - (0 ?
 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23))))
            ));
    }

    if (Hardware->features[gcvFEATURE_2D_COLOR_SPACE_CONVERSION] && (uploadCSC
        || gcoHARDWARE_NeedUserCSC(State->dstYUVMode, State->dstSurface.format)))
    {
        gcmONERROR(gcoHARDWARE_UploadCSCTable(
            Hardware,
            gcvTRUE,
            State->cscYUV2RGB
            ));
    }

    gcmONERROR(gcoHARDWARE_Load2DState32(
        Hardware,
        0x01234,
        destConfig
        ));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}
#endif  /* gcdENABLE_2D*/

#if gcdENABLE_3D
gceSTATUS
gcoHARDWARE_FlushDrawID(
    IN gcoHARDWARE Hardware,
    IN gctBOOL  ForComputing,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gctUINT32 drawID;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

#if gcdFRAMEINFO_STATISTIC
    if (ForComputing)
    {
        gctUINT32  computeCount;
        gcoHAL_FrameInfoOps(gcvNULL,
                            gcvFRAMEINFO_COMPUTE_NUM,
                            gcvFRAMEINFO_OP_GET,
                            &computeCount);
        computeCount--;
        drawID = computeCount;

    }
    else
    {
        gctUINT32 drawCount;
        gctUINT32 frameCount;
        gcoHAL_FrameInfoOps(gcvNULL,
                            gcvFRAMEINFO_FRAME_NUM,
                            gcvFRAMEINFO_OP_GET,
                            &frameCount);

        gcoHAL_FrameInfoOps(gcvNULL,
                            gcvFRAMEINFO_DRAW_NUM,
                            gcvFRAMEINFO_OP_GET,
                            &drawCount);

        drawCount --;
        drawID = (frameCount << 16) | drawCount;

    }
#else
    {
        gctUINT32 programID;
        gcoHAL_FrameInfoOps(gcvNULL,
                            gcvFRAMEINFO_PROGRAM_ID,
                            gcvFRAMEINFO_OP_GET,
                            &programID);
        drawID = programID;
    }
#endif

    gcmONERROR(gcoHARDWARE_LoadCtrlStateNEW(Hardware, 0x0389C, drawID, Memory));

    gcmDUMP(gcvNULL, "#[info: drawID=0x%x]", drawID);

 OnError:

    gcmFOOTER();
    return status;
}

static gceSTATUS _CheckTargetDeferDither(
    IN gcoHARDWARE Hardware
    )
{
    /* If the draw required dither, while PE cannot support, defer until resolve time.
    ** If the draw didn't require, do not reset the flag, unless we are sure it's the
    ** draw can overwrite all of the surface.
    */
    gcsCOLOR_TARGET *pColorTarget = &Hardware->PEStates->colorStates.target[0];
    gcoSURF rtSurf = pColorTarget->surface;
    gctBOOL fakeFmt = rtSurf && rtSurf->formatInfo.fakedFormat;

    /* Disable dither for fake formats */
    if (!Hardware->features[gcvFEATURE_PE_DITHER_FIX] &&
        Hardware->PEStates->ditherEnable &&
        (rtSurf && !(rtSurf->flags & gcvSURF_FLAG_DITHER_DISABLED)) &&
        !fakeFmt)
    {
        gctUINT32 i;

        if (rtSurf)
        {
            rtSurf->deferDither3D = gcvTRUE;
        }

        for (i = 1; i < Hardware->PEStates->colorOutCount; ++i)
        {
            gcmASSERT(Hardware->PEStates->colorStates.target[i].surface);
            Hardware->PEStates->colorStates.target[i].surface->deferDither3D = gcvTRUE;
        }
    }

    return gcvSTATUS_OK;
}

static gceSTATUS gcoHARDWARE_FlushStates(
    IN gcoHARDWARE Hardware,
    IN gcePRIMITIVE Type,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status;
    gctPOINTER saveMemory = *Memory;

    /* Reset flush states. */
    Hardware->flushedColor = gcvFALSE;
    Hardware->flushedDepth = gcvFALSE;

    /* if PE dither enable and trigger draw(go though PE pipe), we need do dither in PE or resolve, no matter RT changed or not.*/
    _CheckTargetDeferDither(Hardware);

    if (Hardware->PEDirty->depthConfigDirty ||
        Hardware->PEDirty->colorConfigDirty ||
        Hardware->MsaaDirty->msaaConfigDirty  ||
        Hardware->SHDirty->shaderDirty      ||
        Hardware->PEDirty->alphaDirty)
    {
        /* Evaluate real depth only mode at the very beginning */
        gcmVERIFY_OK(gcoHARDWARE_FlushDepthOnly(Hardware));
    }

    /*If hardware support R8 format, these codes can be discarded.*/
    if (!Hardware->features[gcvFEATURE_R8_UNORM])
    {
        gctUINT32 i;

        for (i = 0; i < Hardware->PEStates->colorOutCount; ++i)
        {
            gcoSURF surface = Hardware->PEStates->colorStates.target[i].surface;

            if (surface && surface->paddingFormat &&
                Hardware->PEStates->alphaStates.blend[i] &&
                Hardware->PEStates->colorStates.target[i].colorWrite)
            {
                surface->garbagePadded = gcvTRUE;
            }
        }
    }

    /* switch to 3D pipe */
    gcmONERROR(gcoHARDWARE_SelectPipe(Hardware, gcvPIPE_3D, Memory));

    if (Hardware->QUERYStates->queryStatus[gcvQUERY_OCCLUSION] == gcvQUERY_Enabled &&
        !Hardware->features[gcvFEATURE_BUG_FIXES18] &&
        (Hardware->PEStates->depthStates.mode != gcvDEPTH_NONE))
    {
        Hardware->flushedDepth = gcvTRUE;

        gcmONERROR(gcoHARDWARE_LoadCtrlStateNEW(
            Hardware,
            0x0380C,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))),
            Memory
            ));
    }

    if (Hardware->FEDirty->indexDirty)
    {
        /* Program index states. */
        gcmONERROR(gcoHARDWARE_ProgramIndex(Hardware, Memory));
    }

#if gcdSECURITY
    /* Always mark full textures dirty for security mode */
    Hardware->TXDirty->textureDirty = gcvTRUE;
    Hardware->TXDirty->hwTxSamplerDirty = 0xFFFFFFFF;
    gcoOS_MemFill(Hardware->TXDirty->hwTxSamplerAddressDirty, 0xFF, gcmSIZEOF(Hardware->TXDirty->hwTxSamplerAddressDirty));
#endif

    if (Hardware->PAAndSEDirty->viewportDirty)
    {
        /* Flush viewport states. */
        gcmONERROR(gcoHARDWARE_FlushViewport(Hardware, Memory));
    }

    if (Hardware->PAAndSEDirty->scissorDirty)
    {
        /* Flush scissor states. */
        gcmONERROR(gcoHARDWARE_FlushScissor(Hardware, Memory));
    }

    if (Hardware->PEDirty->colorConfigDirty)
    {
        /* Flush target states. */
        gcmONERROR(gcoHARDWARE_FlushTarget(Hardware, Memory));
    }

    if (Hardware->PEDirty->alphaDirty)
    {
        /* Flush alpha states. */
        gcmONERROR(gcoHARDWARE_FlushAlpha(Hardware, Memory));
    }

    if (Hardware->PEDirty->depthConfigDirty ||
        Hardware->PEDirty->depthRangeDirty  ||
        Hardware->PEDirty->depthNormalizationDirty)
    {
        /* Flush depth states after FlushTarget */
        gcmONERROR(gcoHARDWARE_FlushDepth(Hardware, Memory));
    }

    if (Hardware->PEDirty->stencilDirty)
    {
        /* Flush stencil states. */
        gcmONERROR(gcoHARDWARE_FlushStencil(Hardware, Memory));
    }

    if (Hardware->MsaaDirty->msaaConfigDirty)
    {
        /* Flush anti-alias states. */
        gcmONERROR(gcoHARDWARE_FlushSampling(Hardware, Memory));
    }

    if (Hardware->PAAndSEDirty->paConfigDirty || Hardware->PAAndSEDirty->paLineDirty)
    {
        /* Flush primitive assembly states. */
        gcmONERROR(gcoHARDWARE_FlushPA(Hardware, Memory));
    }

    if (Hardware->SHDirty->uniformDirty)
    {
        /* Flush uniform states*/
        gcmONERROR(gcoHARDWARE_FlushUniform(Hardware, Memory));
    }

    /* Initialize some video memories that allocated by compiler. */
    gcmONERROR(gcoHARDWARE_InitVidMemAllocatedByCompiler(Hardware));

    if (Hardware->SHDirty->shaderDirty)
    {
        /* Flush shader states. */
        gcmONERROR(gcoHARDWARE_FlushShaders(Hardware, Type, Memory));

        if (Hardware->features[gcvFEATURE_SH_INSTRUCTION_PREFETCH])
        {
            gcmONERROR(gcoHARDWARE_FlushPrefetchInst(Hardware, Memory));
        }
    }

    if (Hardware->TXDirty->textureDirty)
    {
        /* Program texture states. */
        gcmONERROR((*Hardware->funcPtr->programTexture)(Hardware, Memory));
    }

    if (Hardware->features[gcvFEATURE_HW_TFB] && Hardware->XFBDirty->xfbDirty)
    {
        /* Flush Xfb states*/
        gcmONERROR(gcoHARDWARE_FlushXfb(Hardware, Memory));
    }

    if (Hardware->flushL2)
    {
        /* Flush L2 cache. */
        gcmONERROR(gcoHARDWARE_FlushL2Cache(Hardware, Memory));
    }

    if (Hardware->flushVtxData && Hardware->features[gcvFEATURE_MULTI_CLUSTER])
    {
        /* Flush Vertex data cache. */
        gcmONERROR(gcoHARDWARE_FlushVtxDataCache(Hardware, Memory));
    }

    if (Hardware->multiGPURenderingModeDirty && (Hardware->config->gpuCoreCount > 1))
    {
        /* TODO: select optimum rendering mode for different statemetn */
        gcmONERROR(gcoHARDWARE_FlushMultiGPURenderingMode(Hardware, Memory, gcvMULTI_GPU_RENDERING_MODE_INTERLEAVED_128x64));
    }

    if (Hardware->features[gcvFEATURE_DRAW_ID])
    {
        gcmONERROR(gcoHARDWARE_FlushDrawID(Hardware, gcvFALSE, Memory));
    }

#if gcdENABLE_TRUST_APPLICATION
    if (Hardware->features[gcvFEATURE_SECURITY] && Hardware->GPUProtecedModeDirty)
    {
        gcmONERROR(gcoHARDWARE_FlushProtectMode(Hardware, Memory));
    }
#endif

    if (Hardware->stallSource < Hardware->stallDestination)
    {
        /* Stall if we have to. */
        gcmONERROR(gcoHARDWARE_Semaphore(Hardware,
                                         Hardware->stallSource,
                                         Hardware->stallDestination,
                                         gcvHOW_STALL,
                                         Memory));
    }

    return gcvSTATUS_OK;

OnError:

    /* roll back all commands */
    *Memory = saveMemory;

    /* Return the status. */
    return status;
}
#endif

#if gcdENABLE_2D
/*******************************************************************************
**
**  gcoHARDWARE_Get2DResourceUsage
**
**  Determines the usage of 2D resources (source/pattern/destination).
**
**  INPUT:
**
**      gctUINT8 FgRop
**      gctUINT8 BgRop
**          Foreground and background ROP codes.
**
**      gctUINT32 SrcTransparency
**          Current source transparency mode in hardware encoding.
**
**  OUTPUT:
**
**      gctBOOL_PTR UseSource
**      gctBOOL_PTR UsePattern
**      gctBOOL_PTR UseDestination
**          Resource usage flags to be determined and returned.
**          gcvNULL is allowed for the unwanted flags.
**
*/
void gcoHARDWARE_Get2DResourceUsage(
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop,
    IN gce2D_TRANSPARENCY SrcTransparency,
    OUT gctBOOL_PTR UseSource,
    OUT gctBOOL_PTR UsePattern,
    OUT gctBOOL_PTR UseDestination
    )
{
    gcmHEADER_ARG("FgRop=%x BgRop=%x SrcTransparency=%d "
                    "UseSource=0x%x UsePattern=0x%x UseDestination=0x%x",
                    FgRop, BgRop, SrcTransparency,
                    UseSource, UsePattern, UseDestination);

    /* Determine whether we need the source for the operation. */
    if (UseSource != gcvNULL)
    {
        if (SrcTransparency == gcv2D_KEYED)
        {
            *UseSource = gcvTRUE;
        }
        else
        {
            /* Determine whether this is target only operation. */
            gctBOOL targetOnly
                =  ((FgRop == 0x00) && (BgRop == 0x00))     /* Blackness.    */
                || ((FgRop == 0x55) && (BgRop == 0x55))     /* Invert.       */
                || ((FgRop == 0xAA) && (BgRop == 0xAA))     /* No operation. */
                || ((FgRop == 0xFF) && (BgRop == 0xFF));    /* Whiteness.    */

            *UseSource
                = !targetOnly
                && ((((FgRop >> 2) & 0x33) != (FgRop & 0x33))
                ||  (((BgRop >> 2) & 0x33) != (BgRop & 0x33)));
        }
    }

    /* Determine whether we need the pattern for the operation. */
    if (UsePattern != gcvNULL)
    {
        *UsePattern
            =  (((FgRop >> 4) & 0x0F) != (FgRop & 0x0F))
            || (((BgRop >> 4) & 0x0F) != (BgRop & 0x0F));
    }

    /* Determine whether we need the destiantion for the operation. */
    if (UseDestination != gcvNULL)
    {
        *UseDestination
            =  (((FgRop >> 1) & 0x55) != (FgRop & 0x55))
            || (((BgRop >> 1) & 0x55) != (BgRop & 0x55));
    }

    gcmFOOTER_NO();
}
#endif  /* gcdENABLE_2D */

/*******************************************************************************
**
**  gco2D_GetMaximumDataCount
**
**  Retrieve the maximum number of 32-bit data chunks for a single DE command.
**
**  INPUT:
**
**      Nothing
**
**  OUTPUT:
**
**      gctUINT32
**          Data count value.
*/
gctUINT32 gco2D_GetMaximumDataCount(
    void
    )
{
    gctUINT32 result = 0;

#if gcdENABLE_2D
    gcmHEADER();

    result = ((gctUINT32) ((((1 ? 26:16) - (0 ? 26:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:16) - (0 ? 26:16) + 1)))));

    gcmFOOTER_ARG("return=%d", result);
#endif  /* gcdENABLE_2D */

    return result;
}

/*******************************************************************************
**
**  gco2D_GetMaximumRectCount
**
**  Retrieve the maximum number of rectangles, that can be passed in a single DE command.
**
**  INPUT:
**
**      Nothing
**
**  OUTPUT:
**
**      gctUINT32
**          Rectangle count value.
*/
gctUINT32 gco2D_GetMaximumRectCount(
    void
    )
{
    gctUINT32 result = 0;

#if gcdENABLE_2D
    gcmHEADER();

    result = ((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:8) - (0 ? 15:8) + 1)))));

    gcmFOOTER_ARG("return=%d", result);
#endif  /* gcdENABLE_2D */

    return result;
}

/*******************************************************************************
**
**  gco2D_GetPixelAlignment
**
**  Returns the pixel alignment of the surface.
**
**  INPUT:
**
**      gceSURF_FORMAT Format
**          Pixel format.
**
**  OUTPUT:
**
**      gcsPOINT_PTR Alignment
**          Pointer to the pixel alignment values.
*/
gceSTATUS gco2D_GetPixelAlignment(
    gceSURF_FORMAT Format,
    gcsPOINT_PTR Alignment
    )
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

#if gcdENABLE_2D
    const gctUINT32 BITS_PER_CACHELINE = 64 * 8;

    gcmHEADER_ARG("Format=%d Alignment=0x%x", Format, Alignment);

    /* Verify the argument. */
    gcmDEBUG_VERIFY_ARGUMENT(Alignment != gcvNULL);

    do
    {
        /* Get format's specifics. */
        gcsSURF_FORMAT_INFO_PTR formatInfo;
        gcmERR_BREAK(gcoHARDWARE_QueryFormat(Format, &formatInfo));

        /* Determine horizontal alignment. */
        Alignment->x = BITS_PER_CACHELINE / formatInfo->bitsPerPixel;

        /* Vertical alignment for GC600 is simple. */
        Alignment->y = 1;
    }
    while (gcvFALSE);

    /* Return the status. */
    gcmFOOTER();
#endif  /* gcdENABLE_2D */

    return status;
}

#if gcdENABLE_2D
static int _DrawRectangle(gcoHARDWARE Hardware, gctUINT32_PTR memory, gcsRECT_PTR SrcRect, gcsRECT_PTR DestRect)
{
    int size = 0;

    if (SrcRect)
    {
        /* 0x01210 */
        memory[size++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x01210 >> 2) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

        /* 0x01210 */
        memory[size++]
            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (SrcRect->left) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (SrcRect->top) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

        gcmDUMP(gcvNULL,
                "@[state 0x%04X 0x%08X]",
                0x0484, memory[size - 1]);

        /* 0x01214 */
        memory[size++]
            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (SrcRect->right  - SrcRect->left) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (SrcRect->bottom - SrcRect->top) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

        gcmDUMP(gcvNULL,
                "@[state 0x%04X 0x%08X]",
                0x0485, memory[size - 1]);

        size++;
    }

    /* StartDE(RectCount). */
    memory[size++]
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x04 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:8) - (0 ?
 15:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:8) - (0 ?
 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 15:8) - (0 ?
 15:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)));
    size++;

    /* Append the rectangle. */
    if (DestRect)
    {
        memory[size++]
            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (DestRect->left) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (DestRect->top) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
        memory[size++]
            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (DestRect->right) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (DestRect->bottom) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
    }
    else
    {
        /* Set the max rect for multi src blit v2. */
        memory[size++] = 0;
        memory[size++] = 0x3FFF3FFF;
    }

    if (DestRect != gcvNULL)
    {
        gcmDUMP_LOCK();
        gcmDUMP(gcvNULL, "@[draw2d 1 0x00000000");
        gcmDUMP(gcvNULL,
                "  %d,%d %d,%d",
                DestRect->left, DestRect->top,
                DestRect->right, DestRect->bottom);
        gcmDUMP(gcvNULL, "] -- draw2d");
        gcmDUMP_UNLOCK();
    }

    /*******************************************************************
    ** Program 3 dummy state load at addr 0x0001.
    */
    memory[size++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x00004 >> 2) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

    memory[size++] = 0;

    memory[size++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x00004 >> 2) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

    memory[size++] = 0;

    memory[size++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x00004 >> 2) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

    memory[size++] = 0;

    if (Hardware->hw2DAppendCacheFlush)
    {
        memory[size++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
                        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

        memory[size++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));

        /* Semaphore & Stall. */
        memory[size++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
                        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

        memory[size++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)));

        memory[size++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

        memory[size++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)));
    }

    return size;
}

static SPLIT_RECT_MODE
_PreSplitRectangle(
    IN gcs2D_State_PTR State,
    IN gcoHARDWARE Hardware,
    IN gcsRECT_PTR DestRect,
    IN gctUINT32 DestRectCount,
    OUT gctUINT32_PTR CmdSize)
{
    gctBOOL useSrc = gcvFALSE, useBrush = gcvFALSE, swap = gcvFALSE;
    gceSURF_ROTATION rot;
    gctUINT32 i;
    gctUINT32 rectNum = 0;
    SPLIT_RECT_MODE smode = SPLIT_RECT_MODE_NONE;
    gctBOOL hMirror = State->multiSrc[State->currentSrcIndex].horMirror;
    gctBOOL vMirror = State->multiSrc[State->currentSrcIndex].verMirror;

    gcoHARDWARE_Get2DResourceUsage(
        State->multiSrc[State->currentSrcIndex].fgRop,
        State->multiSrc[State->currentSrcIndex].bgRop,
        State->multiSrc[State->currentSrcIndex].srcTransparency,
        &useSrc, &useBrush, gcvNULL
        );

    if (!Hardware->features[gcvFEATURE_2D_BITBLIT_FULLROTATION]
        || Hardware->hw2DBlockSize
        || !useSrc || useBrush)
    {
        return SPLIT_RECT_MODE_NONE;
    }

    Hardware->srcRot = State->multiSrc[State->currentSrcIndex].srcSurface.rotation;
    Hardware->dstRot = State->dstSurface.rotation;
    Hardware->horMirror = State->multiSrc[State->currentSrcIndex].horMirror;
    Hardware->verMirror = State->multiSrc[State->currentSrcIndex].verMirror;
    Hardware->clipRect = State->dstClipRect;
    Hardware->srcRelated = State->multiSrc[State->currentSrcIndex].srcRelativeCoord;

    rot = gcmGET_PRE_ROTATION(State->multiSrc[State->currentSrcIndex].srcSurface.rotation);

    if (gcmGET_PRE_ROTATION(State->dstSurface.rotation) == gcvSURF_FLIP_X)
    {
        hMirror = !hMirror;
    }
    else if (gcmGET_PRE_ROTATION(State->dstSurface.rotation) == gcvSURF_FLIP_Y)
    {
        vMirror = !vMirror;
    }

    if (rot == gcvSURF_FLIP_X)
    {
        hMirror = !hMirror;
        rot = gcvSURF_0_DEGREE;
    }
    else if (rot == gcvSURF_FLIP_Y)
    {
        vMirror = !vMirror;
        rot = gcvSURF_0_DEGREE;
    }

    if (gcmIS_SUCCESS(gcsRECT_RelativeRotation(State->dstSurface.rotation, &rot)))
    {
        if ((rot == gcvSURF_90_DEGREE) || (rot == gcvSURF_270_DEGREE))
        {
            smode = SPLIT_RECT_MODE_COLUMN;
        }
        else
        {
            gctUINT32 srcAddress, dstAddress;

            gcmGETHARDWAREADDRESS(State->dstSurface.node, dstAddress);
            gcmGETHARDWAREADDRESS(State->multiSrc[State->currentSrcIndex].srcSurface.node, srcAddress);

            if (Hardware->features[gcvFEATURE_MMU] || (!(dstAddress & 0x80000000)
                && !(srcAddress & 0x80000000)))
            {
                return SPLIT_RECT_MODE_NONE;
            }

            smode = SPLIT_RECT_MODE_LINE;
        }
    }
    else
    {
        return SPLIT_RECT_MODE_NONE;
    }

    if ((gcmGET_PRE_ROTATION(State->dstSurface.rotation) == gcvSURF_90_DEGREE)
        || (gcmGET_PRE_ROTATION(State->dstSurface.rotation) == gcvSURF_270_DEGREE))
    {
        gctBOOL tmp = hMirror;
        hMirror = vMirror;
        vMirror = tmp;

        swap = gcvTRUE;
    }

    for (i = 0; i < DestRectCount; i++)
    {
        if (smode == SPLIT_RECT_MODE_COLUMN)
        {
            if (swap)
            {
                rectNum += ((DestRect[i].bottom - DestRect[i].top) >> SPLIT_COLUMN) + 2;
            }
            else
            {
                rectNum += ((DestRect[i].right - DestRect[i].left) >> SPLIT_COLUMN) + 2;
            }
        }
        else
        {
            if (swap)
            {
                rectNum += DestRect[i].right - DestRect[i].left;
            }
            else
            {
                rectNum += DestRect[i].bottom - DestRect[i].top;
            }
        }
    }

    if (rectNum * 14 > (gcdCMD_BUFFER_SIZE >> 1))
    {
        return SPLIT_RECT_MODE_NONE;
    }

    State->dstSurface.rotation = gcvSURF_0_DEGREE | gcmGET_POST_ROTATION(Hardware->dstRot);
    State->multiSrc[State->currentSrcIndex].srcSurface.rotation = rot | gcmGET_POST_ROTATION(Hardware->srcRot);
    State->multiSrc[State->currentSrcIndex].horMirror = hMirror;
    State->multiSrc[State->currentSrcIndex].verMirror = vMirror;
    State->dstClipRect.left = 0;
    State->dstClipRect.top = 0;
    State->dstClipRect.right = 0x7FFF;
    State->dstClipRect.bottom = 0x7FFF;
    State->multiSrc[State->currentSrcIndex].clipRect = State->dstClipRect;
    State->multiSrc[State->currentSrcIndex].srcRelativeCoord = gcvFALSE;

    *CmdSize = rectNum * (Hardware->hw2DAppendCacheFlush ? 20 : 14);

    return smode;
}

static int _SplitRectangle(
    gcs2D_State_PTR State,
    gcoHARDWARE Hardware,
    SPLIT_RECT_MODE Mode,
    gctUINT32_PTR Memory,
    gcsRECT_PTR SrcRect,
    gcsRECT_PTR DestRect)
{
    gctINT32 n, size = 0;
    gcsRECT srcRect, dstRect;

    dstRect.left = gcmMAX(DestRect->left, Hardware->clipRect.left);
    dstRect.top = gcmMAX(DestRect->top, Hardware->clipRect.top);
    dstRect.right = gcmMIN(DestRect->right, Hardware->clipRect.right);
    dstRect.bottom = gcmMIN(DestRect->bottom, Hardware->clipRect.bottom);

    if (Hardware->srcRelated)
    {
        srcRect.left = SrcRect->left + dstRect.left;
        srcRect.top = SrcRect->top + dstRect.top;
    }
    else
    {
        srcRect.left = SrcRect->left + dstRect.left - DestRect->left;
        srcRect.top = SrcRect->top + dstRect.top - DestRect->top;
    }

    srcRect.right = dstRect.right - dstRect.left + srcRect.left;
    srcRect.bottom = dstRect.bottom - dstRect.top + srcRect.top;

    if (Hardware->dstRot == gcvSURF_FLIP_X)
    {
        n = dstRect.left;
        dstRect.left = State->dstSurface.alignedW - dstRect.right;
        dstRect.right = State->dstSurface.alignedW - n;
    }
    else if (Hardware->dstRot == gcvSURF_FLIP_Y)
    {
        n = dstRect.top;
        dstRect.top = State->dstSurface.alignedH - dstRect.bottom;
        dstRect.bottom = State->dstSurface.alignedH - n;
    }

    if (Hardware->srcRot == gcvSURF_FLIP_X)
    {
        n = srcRect.left;
        srcRect.left = State->multiSrc[State->currentSrcIndex].srcSurface.alignedW - srcRect.right;
        srcRect.right = State->multiSrc[State->currentSrcIndex].srcSurface.alignedW - n;
    }
    else if (Hardware->srcRot == gcvSURF_FLIP_Y)
    {
        n = srcRect.top;
        srcRect.top = State->multiSrc[State->currentSrcIndex].srcSurface.alignedH - srcRect.bottom;
        srcRect.bottom = State->multiSrc[State->currentSrcIndex].srcSurface.alignedH - n;
    }

    if (gcvSTATUS_OK != gcsRECT_Rotate(
            &dstRect,
            Hardware->dstRot,
            State->dstSurface.rotation,
            State->dstSurface.alignedW,
            State->dstSurface.alignedH))
    {
        return 0;
    }

    if (gcvSTATUS_OK != gcsRECT_Rotate(
            &srcRect,
            Hardware->srcRot,
            State->multiSrc[State->currentSrcIndex].srcSurface.rotation,
            State->multiSrc[State->currentSrcIndex].srcSurface.alignedW,
            State->multiSrc[State->currentSrcIndex].srcSurface.alignedH))
    {
        return 0;
    }

    if (Mode == SPLIT_RECT_MODE_COLUMN)
    {
        gctINT32 dLeft = dstRect.left;
        gctINT32 dRight = dstRect.right;
        gctINT32 sLeft = srcRect.left;
        gctINT32 sRight = srcRect.right;

        gctBOOL reverse = dLeft > sLeft;

        n = (gcmALIGN(dRight, SPLIT_COLUMN_WIDTH) - (dLeft & SPLIT_COLUMN_WIDTH_MASK)) / SPLIT_COLUMN_WIDTH;

        while (n-- > 0)
        {
            if (reverse)
            {
                dstRect.left = gcmMAX(dLeft, (dstRect.right - SPLIT_COLUMN_WIDTH) & SPLIT_COLUMN_WIDTH_MASK);
            }
            else
            {
                dstRect.right = gcmMIN(dRight, (dstRect.left + SPLIT_COLUMN_WIDTH) & SPLIT_COLUMN_WIDTH_MASK);
            }

            if (State->multiSrc[State->currentSrcIndex].horMirror)
            {
                srcRect.right = dLeft + sRight - dstRect.left;
                srcRect.left = srcRect.right - (dstRect.right - dstRect.left);
            }
            else
            {
                srcRect.left = dstRect.left - dLeft + sLeft;
                srcRect.right = srcRect.left + dstRect.right - dstRect.left;
            }

            size += _DrawRectangle(Hardware, Memory + size, &srcRect, &dstRect);

            if (reverse)
            {
                dstRect.right = dstRect.left;
            }
            else
            {
                dstRect.left = dstRect.right;
            }
        }
    }
    else
    {
        gctBOOL reverse = dstRect.top > srcRect.top;
        n = dstRect.bottom - dstRect.top;

        while (n-- > 0)
        {
            if (reverse)
            {
                dstRect.top = dstRect.bottom - 1;
            }
            else
            {
                dstRect.bottom = dstRect.top + 1;
            }

            if (State->multiSrc[State->currentSrcIndex].verMirror ^ reverse)
            {
                srcRect.top = srcRect.bottom - 1;
            }
            else
            {
                srcRect.bottom = srcRect.top + 1;
            }

            size += _DrawRectangle(Hardware, Memory + size, &srcRect, &dstRect);

            if (reverse)
            {
                --dstRect.bottom;
            }
            else
            {
                ++dstRect.top;
            }

            if (State->multiSrc[State->currentSrcIndex].verMirror ^ reverse)
            {
                --srcRect.bottom;
            }
            else
            {
                ++srcRect.top;
            }
        }
    }

    return size;
}

/*******************************************************************************
**
**  gcoHARDWARE_StartDE
**
**  Start a DE command for one or more source and destination rectangles.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gce2D_COMMAND Command
**          2D engine command to be executed.
**
**      gctUINT32 SrcRectCount
**          Set as 1, for single source rectangle.
**          Set as DestRectCount, if each blit has its own source rectangle.
**
**      gcsRECT_PTR SrcRect
**          Pointer to an array of source rectangles.
**
**      gctUINT32 DestRectCount
**          Number of destination rectangles to be operated on.
**
**      gcsRECT_PTR DestRect
**          Pointer to an array of destination rectangles.
**
**      gctUINT32 FgRop
**      gctUINT32 BgRop
**          Foreground and background ROP codes.
**
**  OUTPUT:
**
**      gceSTATUS
**          Returns gcvSTATUS_OK if successful.
*/
gceSTATUS gcoHARDWARE_StartDE(
    IN gcoHARDWARE Hardware,
    IN gcs2D_State_PTR State,
    IN gce2D_COMMAND Command,
    IN gctUINT32 SrcRectCount,
    IN gcsRECT_PTR SrcRect,
    IN gctUINT32 DestRectCount,
    IN gcsRECT_PTR DestRect
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    SPLIT_RECT_MODE smode = SPLIT_RECT_MODE_NONE;

    gcmHEADER_ARG("Hardware=0x%x State=0x%x Command=0x%x SrcRectCount=%d "
                    "SrcRect=0x%x DestRectCount=%d DestRect=0x%x "
                    "FgRop=%x BgRop=%x",
                    Hardware, State, Command, SrcRectCount,
                    SrcRect, DestRectCount, DestRect);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (DestRect != gcvNULL)
    {
        if ((DestRectCount < 1) || ((Command != gcv2D_CLEAR)
            && (Command != gcv2D_BLT) && (Command != gcv2D_STRETCH)
            && (Command != gcv2D_MULTI_SOURCE_BLT))
            || ((!SrcRect || ((SrcRectCount != DestRectCount)))
                && (SrcRect || SrcRectCount != 0)))
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }
    else if (Hardware->features[gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2])
    {
        if (DestRectCount != 0)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }
    else
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    do
    {
        gctUINT32 idx;

        /* Validate rectangle coordinates. */
        if (SrcRect)
        {
            for (idx = 0; idx < SrcRectCount; idx++)
            {
                if (!_IsValidRect(SrcRect + idx))
                {
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
            }
        }

        for (idx = 0; idx < DestRectCount; idx++)
        {
            if (!_IsValidRect(DestRect + idx))
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
        }

        if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
        {
            gctUINT32 loopCount = 0;
            gcoCMDBUF reserve;
            gctUINT32_PTR memory = gcvNULL;
            gctUINT32 dataSize = DestRectCount * ((SrcRect == gcvNULL) ? 10 : 14);

            if (Hardware->hw2DAppendCacheFlush)
            {
                dataSize += DestRectCount * 6;
            }

            if ((Command == gcv2D_BLT) && Hardware->hw2DSplitRect)
            {
                smode = _PreSplitRectangle(State, Hardware, DestRect, DestRectCount, &dataSize);
            }

            Hardware->hw2DCmdBuffer = gcvNULL;
            Hardware->hw2DCmdSize = 0;
            Hardware->hw2DCmdIndex = _ReserveBufferSize(Hardware, State, Command);

            if (Hardware->hw2DCmdIndex)
            {
                Hardware->hw2DCmdIndex += dataSize;
            }

            while (Hardware->hw2DCmdBuffer == gcvNULL)
            {
                if (Hardware->hw2DCmdIndex)
                {
                    gcmONERROR(gcoBUFFER_Reserve(
                        Hardware->engine[gcvENGINE_RENDER].buffer,
                        Hardware->hw2DCmdIndex * gcmSIZEOF(gctUINT32),
                        gcvTRUE,
                        gcvCOMMAND_2D,
                        &reserve
                        ));

                    Hardware->hw2DCmdBuffer = gcmUINT64_TO_PTR(reserve->lastReserve);
                    Hardware->hw2DCmdSize = Hardware->hw2DCmdIndex;
                    Hardware->hw2DCmdIndex = 0;

                    reserve->using2D = gcvTRUE;
                }

            if (DestRect != gcvNULL)
            {
                State->dstRect = DestRect;
            }

            State->dstRectCount = DestRectCount;

            gcmONERROR(gcoHARDWARE_Set2DState(Hardware, State, Command, DestRect == gcvNULL));

            /*******************************************************************
            ** Allocate and configure StartDE command buffer.
            */

            if (SrcRect == gcvNULL)
            {
                if (Hardware->hw2DCmdBuffer != gcvNULL)
                {
                memory = Hardware->hw2DCmdBuffer + Hardware->hw2DCmdIndex;
                do
                {
                    if (smode != SPLIT_RECT_MODE_NONE)
                    {
                        memory += _SplitRectangle(
                            State,
                            Hardware,
                            smode,
                            memory,
                            &State->multiSrc[State->currentSrcIndex].srcRect,
                            DestRect);
                    }
                    else
                    {
                        /* Draw a dest rectangle. */
                        memory += _DrawRectangle(Hardware, memory, gcvNULL, DestRect);
                    }

                    if (DestRect)
                    {
                        --DestRectCount;
                        ++DestRect;
                    }

                    loopCount++;
                }
                while (DestRectCount && loopCount < MAX_LOOP_COUNT);
                }

                Hardware->hw2DCmdIndex +=
                    (!memory) ? dataSize
                    : (gctUINT32)((memory - Hardware->hw2DCmdBuffer - Hardware->hw2DCmdIndex));
            }
            else
            {
                /* Force to draw one rectange at a time. */
                if (Hardware->hw2DCmdBuffer != gcvNULL)
                {
                memory = Hardware->hw2DCmdBuffer + Hardware->hw2DCmdIndex;
                do
                {
                    /* Program source rectangle for each destination rectangle. */
                    if (smode != SPLIT_RECT_MODE_NONE)
                    {
                        memory += _SplitRectangle(
                            State,
                            Hardware,
                            smode,
                            memory,
                            SrcRect,
                            DestRect);
                    }
                    else
                    {
                        memory += _DrawRectangle(Hardware, memory, SrcRect, DestRect);
                    }

                    /* Move to next SrcRect and DestRect. */
                    SrcRect++;
                    DestRect++;
                    loopCount++;
                }
                while (--DestRectCount && loopCount < MAX_LOOP_COUNT);
                }

                Hardware->hw2DCmdIndex +=
                    (!memory) ? dataSize
                    : (gctUINT32)((memory - Hardware->hw2DCmdBuffer - Hardware->hw2DCmdIndex));
            }

            gcmONERROR(gcoHARDWARE_End2DRender(Hardware, State));

            }
        }
        else
        {
            /* Call software renderer. */
            gcmONERROR(_RenderRectangle(
                Hardware,
                State,
                Command,
                DestRectCount,
                DestRect
                ));
        }
    }
    while (gcvFALSE);

OnError:

    if (smode != SPLIT_RECT_MODE_NONE && Hardware!= gcvNULL)
    {
        State->multiSrc[State->currentSrcIndex].srcSurface.rotation
            = Hardware->srcRot;
        State->dstSurface.rotation = Hardware->dstRot;
        State->multiSrc[State->currentSrcIndex].horMirror = Hardware->horMirror;
        State->multiSrc[State->currentSrcIndex].verMirror = Hardware->verMirror;
        State->multiSrc[State->currentSrcIndex].clipRect = State->dstClipRect = Hardware->clipRect;
        State->multiSrc[State->currentSrcIndex].srcRelativeCoord = Hardware->srcRelated;
    }

    if (Hardware != gcvNULL)
    {
        if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
        {
            /* Reset command buffer. */
            gcoHARDWARE_Reset2DCmdBuffer(Hardware, gcmIS_ERROR(status));
        }
    }

    /* Return result. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_StartDELine
**
**  Start a DE command to draw one or more Lines, with a common or
**  individual color.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gce2D_COMMAND Command
**          2D engine command to be executed.
**
**      gctUINT32 RectCount
**          Number of destination rectangles to be operated on.
**
**      gcsRECT_PTR DestRect
**          Pointer to an array of destination rectangles.
**
**      gctUINT32 ColorCount
**          Set as 0, if using brush.
**          Set as 1, if single color for all lines.
**          Set as LineCount, if each line has its own color.
**
**      gctUINT32_PTR Color32
**          Source color array in A8R8G8B8 format.
**
**      gctUINT32 FgRop
**      gctUINT32 BgRop
**          Foreground and background ROP codes.
**
**  OUTPUT:
**
**      gceSTATUS
**          Returns gcvSTATUS_OK if successful.
*/
gceSTATUS gcoHARDWARE_StartDELine(
    IN gcoHARDWARE Hardware,
    IN gcs2D_State_PTR State,
    IN gce2D_COMMAND Command,
    IN gctUINT32 LineCount,
    IN gcsRECT_PTR DestRect,
    IN gctUINT32 ColorCount,
    IN gctUINT32_PTR Color32
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Command=0x%x LineCount=%d "
                    "DestRect=0x%x ColorCount=%d Color32=%x",
                    Hardware, Command, LineCount,
                    DestRect, ColorCount, Color32);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmASSERT((ColorCount == 1) || (ColorCount == 0) || (ColorCount == LineCount));
    gcmASSERT(LineCount);

    do
    {
        gctUINT32 idx;

        /* Validate line coordinates. */
        for (idx = 0; idx < LineCount; idx++)
        {
            if ((DestRect[idx].left < 0)
                || (DestRect[idx].top < 0)
                || (DestRect[idx].right < 0)
                || (DestRect[idx].bottom < 0)
                || (DestRect[idx].left > 65535)
                || (DestRect[idx].top > 65535)
                || (DestRect[idx].right > 65535)
                || (DestRect[idx].bottom > 65535))
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
        }

        if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
        {
            gctUINT32 i, maxLineCount, leftLineCount;
            gctUINT32 colorConfig[2], lastProgrammedColor = 0;
            gcoCMDBUF reserve;
            gctUINT32_PTR memory;
            gcsRECT_PTR currentDestRect;
            gctUINT32 loopCount = 0;

            Hardware->hw2DCmdBuffer = gcvNULL;
            Hardware->hw2DCmdSize = 0;
            Hardware->hw2DCmdIndex = _ReserveBufferSize(Hardware, State, Command);

            if (Hardware->hw2DCmdIndex)
            {
                Hardware->hw2DCmdIndex += (LineCount + ColorCount) * 4;
            }

            while (Hardware->hw2DCmdBuffer == gcvNULL)
            {
                if (Hardware->hw2DCmdIndex)
                {
                    gcmONERROR(gcoBUFFER_Reserve(
                        Hardware->engine[gcvENGINE_RENDER].buffer,
                        Hardware->hw2DCmdIndex * gcmSIZEOF(gctUINT32),
                        gcvTRUE,
                        gcvCOMMAND_2D,
                        &reserve
                        ));

                    Hardware->hw2DCmdBuffer = gcmUINT64_TO_PTR(reserve->lastReserve);
                    Hardware->hw2DCmdSize = Hardware->hw2DCmdIndex;
                    Hardware->hw2DCmdIndex = 0;

                    reserve->using2D = gcvTRUE;
                }
            gcmONERROR(gcoHARDWARE_Set2DState(Hardware, State, Command, gcvFALSE));

            /*******************************************************************
            ** Allocate and configure StartDE command buffer.  Subdivide into
            ** multiple commands if leftLineCount exceeds maxLineCount.
            */

            maxLineCount = gco2D_GetMaximumRectCount();
            leftLineCount = LineCount;
            currentDestRect = DestRect;

            if (ColorCount)
            {
                /* Set last programmed color different from *Color32,
                   so that the first color always gets programmed. */
                lastProgrammedColor = *Color32 + 1;
            }

            do
            {
                /* Render upto maxRectCount rectangles. */
                gctUINT32 lineCount = (leftLineCount < maxLineCount)
                                    ? leftLineCount
                                    : maxLineCount;

                /* Program color for each line. */
                if (ColorCount && (lastProgrammedColor != *Color32))
                {
                    if (Hardware->hw2DAppendCacheFlush)
                    {
                        gcmONERROR(gcoHARDWARE_Load2DState32(
                            Hardware,
                            0x01254,
                            *Color32
                            ));

                        gcmONERROR(gcoHARDWARE_Load2DState32(
                            Hardware,
                            0x0123C,
                              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
                            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (gcvTRUE) & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
                            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:6) - (0 ?
 7:6) + 1))))))) << (0 ?
 7:6))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6)))
                            ));
                    }
                    else
                    {
                        /* Backgroud color. */
                        colorConfig[0] = *Color32;

                        /* Foreground color. */
                        colorConfig[1] = *Color32;

                        /* Save last programmed color. */
                        lastProgrammedColor = *Color32;

                        /* LoadState(AQDE_SRC_COLOR_BG, 2), BgColor, FgColor. */
                        gcmONERROR(gcoHARDWARE_Load2DState(
                            Hardware,
                            0x01218, 2,
                            colorConfig
                            ));
                    }
                }

                /* Find the number of following lines with same color. */
                if (ColorCount > 1)
                {
                    gctUINT32 sameColoredLines = 1;

                    Color32++;

                    while (sameColoredLines < lineCount)
                    {
                        if (lastProgrammedColor != *Color32)
                        {
                            break;
                        }

                        Color32++;
                        sameColoredLines++;
                    }

                    lineCount = sameColoredLines;
                }

                if (Hardware->hw2DCmdBuffer != gcvNULL)
                {
                memory = Hardware->hw2DCmdBuffer + Hardware->hw2DCmdIndex;
                /* StartDE(RectCount). */
                *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x04 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:8) - (0 ?
 15:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:8) - (0 ?
 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (lineCount) & ((gctUINT32) ((((1 ?
 15:8) - (0 ?
 15:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)));
                memory++;

                gcmDUMP_LOCK();
                gcmDUMP(gcvNULL, "@[draw2d %u 0x00000000", lineCount);

                /* Append the rectangles. */
                for (i = 0; i < lineCount; i++)
                {
                    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (currentDestRect[i].left) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (currentDestRect[i].top) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
                    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (currentDestRect[i].right) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (currentDestRect[i].bottom) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

                    gcmDUMP(gcvNULL,
                            "  %d,%d %d,%d",
                            currentDestRect[i].left, currentDestRect[i].top,
                            currentDestRect[i].right, currentDestRect[i].bottom);
                }
                }
                Hardware->hw2DCmdIndex += 2 + lineCount * 2;
                gcmDUMP(gcvNULL, "] -- draw2d");
                gcmDUMP_UNLOCK();

                leftLineCount -= lineCount;
                currentDestRect += lineCount;
                loopCount++;
            }
            while (leftLineCount && loopCount < MAX_LOOP_COUNT);

            /*******************************************************************
            ** Program a dummy state load at addr 0x0001.
            */
            gcmONERROR(gcoHARDWARE_Load2DState32(
                    Hardware,
                    0x00004,
                    0x0
                    ));

            gcmONERROR(gcoHARDWARE_Load2DState32(
                    Hardware,
                    0x00004,
                    0x0
                    ));

            gcmONERROR(gcoHARDWARE_End2DRender(Hardware, State));

            }
        }
        else
        {
            /* Call software renderer. */
            gcmONERROR(_RenderRectangle(
                Hardware,
                State,
                Command,
                LineCount,
                DestRect
                ));
        }
    }
    while (gcvFALSE);

OnError:

    if (Hardware != gcvNULL)
    {
        if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
        {
            /* Reset command buffer. */
            gcoHARDWARE_Reset2DCmdBuffer(Hardware, gcmIS_ERROR(status));
        }
    }

    /* Return result. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_StartDEStream
**
**  Start a DE command with a monochrome stream source.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsRECT_PTR DestRect
**          Pointer to the destination rectangles.
**
**      gctUINT32 FgRop
**      gctUINT32 BgRop
**          Foreground and background ROP codes.
**
**      gctUINT32 StreamSize
**          Size of the stream in bytes.
**
**  OUTPUT:
**
**      gctPOINTER * StreamBits
**          Pointer to an allocated buffer for monochrome data.
*/
gceSTATUS gcoHARDWARE_StartDEStream(
    IN gcoHARDWARE Hardware,
    IN gcs2D_State_PTR State,
    IN gcsRECT_PTR DestRect,
    IN gctUINT32 StreamSize,
    OUT gctPOINTER * StreamBits
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x DestRect=0x%x StreamSize=%d StreamBits=0x%x",
                  Hardware, DestRect, StreamSize, StreamBits);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmDEBUG_VERIFY_ARGUMENT(DestRect != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(StreamBits != gcvNULL);

    do
    {
        if (Hardware->hw2DAppendCacheFlush)
        {
            /* gc320 5007 board does not support mono blit. */
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        if (!_IsValidRect(DestRect))
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
        {
            gctUINT32 dataCount;
            gcoCMDBUF reserve;
            gctUINT32_PTR memory;
            gctUINT32 dataSize = (2 + 2) * sizeof(gctUINT32) + StreamSize;/* Determine the command size. */

            Hardware->hw2DCmdBuffer = gcvNULL;
            Hardware->hw2DCmdSize = 0;
            Hardware->hw2DCmdIndex = _ReserveBufferSize(Hardware, State, gcv2D_BLT);

            if (Hardware->hw2DCmdIndex)
            {
                Hardware->hw2DCmdIndex += gcmALIGN(dataSize, 8) >> 2;
            }

            while (Hardware->hw2DCmdBuffer == gcvNULL)
            {
                if (Hardware->hw2DCmdIndex)
                {
                    gcmONERROR(gcoBUFFER_Reserve(
                        Hardware->engine[gcvENGINE_RENDER].buffer,
                        Hardware->hw2DCmdIndex * gcmSIZEOF(gctUINT32),
                        gcvTRUE,
                        gcvCOMMAND_2D,
                        &reserve
                        ));

                    Hardware->hw2DCmdBuffer = gcmUINT64_TO_PTR(reserve->lastReserve);
                    Hardware->hw2DCmdSize = Hardware->hw2DCmdIndex;
                    Hardware->hw2DCmdIndex = 0;

                    reserve->using2D = gcvTRUE;
                }

            gcmONERROR(gcoHARDWARE_Set2DState(Hardware, State, gcv2D_BLT, gcvFALSE));

            /*******************************************************************
            ** Allocate and configure StartDE command buffer.
            */
            if (Hardware->hw2DCmdBuffer != gcvNULL)
            {
            memory = Hardware->hw2DCmdBuffer + Hardware->hw2DCmdIndex;
            /* Determine the data count. */
            dataCount = StreamSize >> 2;

            /* StartDE(DataCount). */
            *memory++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x04 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:8) - (0 ?
 15:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:8) - (0 ?
 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 15:8) - (0 ?
 15:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:16) - (0 ?
 26:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:16) - (0 ?
 26:16) + 1))))))) << (0 ?
 26:16))) | (((gctUINT32) ((gctUINT32) (dataCount) & ((gctUINT32) ((((1 ?
 26:16) - (0 ?
 26:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:16) - (0 ? 26:16) + 1))))))) << (0 ? 26:16)));
            memory++;

            gcmDUMP(gcvNULL, "@[prim2d 1 0x%08X", dataCount);

            /* Append the rectangle. */
            *memory++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (DestRect->left) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (DestRect->top) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
            *memory++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (DestRect->right) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (DestRect->bottom) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

            gcmDUMP(gcvNULL, "  %d,%d %d,%d",
                    DestRect->left, DestRect->top,
                    DestRect->right, DestRect->bottom);

            /* Set the stream location. */
            *StreamBits = memory;
            }
            Hardware->hw2DCmdIndex += gcmALIGN(dataSize, 8) >> 2;

            /*******************************************************************
            ** Program a dummy state load at addr 0x0001.
            */
            gcmONERROR(gcoHARDWARE_Load2DState32(
                    Hardware,
                    0x00004,
                    0x0
                    ));

            gcmONERROR(gcoHARDWARE_Load2DState32(
                    Hardware,
                    0x00004,
                    0x0
                    ));

            gcmONERROR(gcoHARDWARE_End2DRender(Hardware, State));

            }
        }
        else
        {
            /* Monochrome operations are not currently supported. */
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }
    while (gcvFALSE);

OnError:

    if (Hardware != gcvNULL)
    {
        if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
        {
            /* Reset command buffer. */
            gcoHARDWARE_Reset2DCmdBuffer(Hardware, gcmIS_ERROR(status));
        }
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}
#endif  /* gcdENABLE_2D */

#if gcdENABLE_3D
static gcmINLINE gceSTATUS
_InternalTFBSwitch(
    gcoHARDWARE Hardware,
    gctBOOL Enable,
    gctPOINTER *Memory
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmASSERT(Hardware->features[gcvFEATURE_HW_TFB]);

    if ((Hardware->XFBStates->status != gcvXFB_Enabled) &&
        (Hardware->QUERYStates->queryStatus[gcvQUERY_PRIM_GENERATED] == gcvQUERY_Enabled))
    {
        gctUINT32 tfbCmd = Enable
                         ?
 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                         : ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));

        /* Define state buffer variables. */
        gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

        gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

        if (Enable)
        {
            gctUINT32 physical;

            gcmASSERT(Hardware->XFBStates->internalXFB == gcvFALSE);

            if (gcvNULL == Hardware->XFBStates->internalXFBNode)
            {
                gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsSURF_NODE), (gctPOINTER *)&Hardware->XFBStates->internalXFBNode);
                gcoOS_ZeroMemory((gctPOINTER)Hardware->XFBStates->internalXFBNode, gcmSIZEOF(gcsSURF_NODE));
                gcmONERROR(gcsSURF_NODE_Construct(Hardware->XFBStates->internalXFBNode,
                                                  64,
                                                  64,
                                                  gcvSURF_TFBHEADER,
                                                  0,
                                                  gcvPOOL_DEFAULT
                                                  ));

                gcmONERROR(gcoSURF_LockNode(Hardware->XFBStates->internalXFBNode, gcvNULL, &Hardware->XFBStates->internalXFBLocked));

                gcoOS_ZeroMemory(Hardware->XFBStates->internalXFBLocked, 64);

                gcmASSERT(Hardware->XFBStates->internalXFBLocked);
            }

            gcmGETHARDWAREADDRESSP(Hardware->XFBStates->internalXFBNode, physical);

            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7002) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x7002, physical);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7020) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x7020, 0);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7030) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x7030, 16);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            Hardware->XFBStates->internalXFB = gcvTRUE;
        }
        else
        {
            gcmASSERT(Hardware->XFBStates->internalXFB);


            Hardware->XFBStates->internalXFB = gcvFALSE;
            Hardware->XFBDirty->s.headerDirty = gcvTRUE;
            Hardware->XFBDirty->s.bufferDirty = gcvTRUE;
        }

        /* Switch to single GPU mode */
        if (Hardware->config->gpuCoreCount > 1)
        {
            gcoHARDWARE_MultiGPUSync(Hardware, &memory);
            { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0); memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0));
 } };

        }

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7001) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x7001, tfbCmd);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        /* Resume to multiple GPU mode */
        if (Hardware->config->gpuCoreCount > 1)
        {
            { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK; memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };

            gcoHARDWARE_MultiGPUSync(Hardware, &memory);
        }

        gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);
    }
OnError:
    return status;
}


static gcmINLINE gceSTATUS
_SinglePEPipeSwitch(
    gcoHARDWARE Hardware,
    gctBOOL Single,
    gctPOINTER *Memory
    )
{

    gctUINT32 flush = 0;
    gceSTATUS status = gcvSTATUS_OK;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    /* switch from dual-pipe to single-pipe */
    if (Single)
    {
        if (!Hardware->flushedColor)
        {
            flush |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));
        }
        if (!Hardware->flushedDepth)
        {
            flush |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));
        }
    }

    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    if (flush)
    {
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E03, flush);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x052F) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x052F, ((((gctUINT32) (Hardware->PEStates->peConfigExReg)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (Single ?
 0x1 : 0x0) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);
OnError:
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_DrawPrimitives
**
**  Draw a number of primitives.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcePRIMITIVE Type
**          Type of the primitives to draw.  Can be one of the following:
**
**              gcvPRIMITIVE_POINT_LIST     - List of points.
**              gcvPRIMITIVE_LINE_LIST      - List of lines.
**              gcvPRIMITIVE_LINE_STRIP     - List of connecting lines.
**              gcvPRIMITIVE_LINE_LOOP      - List of connecting lines where the
**                                           first and last line will also be
**                                           connected.
**              gcvPRIMITIVE_TRIANGLE_LIST  - List of triangles.
**              gcvPRIMITIVE_TRIANGLE_STRIP - List of connecting triangles layed
**                                           out in a strip.
**              gcvPRIMITIVE_TRIANGLE_FAN   - List of connecting triangles layed
**                                           out in a fan.
**
**      gctINT StartVertex
**          Starting vertex number to start drawing.  The starting vertex is
**          multiplied by the stream stride to compute the starting offset.
**
**      gctSIZE_T PrimitiveCount
**          Number of primitives to draw.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_DrawPrimitives(
    IN gcoHARDWARE Hardware,
    IN gcePRIMITIVE Type,
    IN gctINT StartVertex,
    IN gctSIZE_T PrimitiveCount
    )
{
    gceSTATUS status;
    gctPOINTER *outside = gcvNULL;
    gctBOOL useOneCore = gcvFALSE;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    static const gctINT32 xlate[] =
    {
        /* gcvPRIMITIVE_POINT_LIST */
        0x1,
        /* gcvPRIMITIVE_LINE_LIST */
        0x2,
        /* gcvPRIMITIVE_LINE_STRIP */
        0x3,
        /* gcvPRIMITIVE_LINE_LOOP */
        0x7,
        /* gcvPRIMITIVE_TRIANGLE_LIST */
        0x4,
        /* gcvPRIMITIVE_TRIANGLE_STRIP */
        0x5,
        /* gcvPRIMITIVE_TRIANGLE_FAN */
        0x6,
        /* gcvPRIMITIVE_RECTANGLE */
        0x8
    };

    gcmHEADER_ARG("Hardware=0x%x Type=%d StartVertex=%d PrimitiveCount=%u",
                  Hardware, Type, StartVertex, PrimitiveCount);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmDEBUG_VERIFY_ARGUMENT(PrimitiveCount > 0);

#if gcdDEBUG_OPTION && gcdDEBUG_OPTION_NO_DRAW_PRIMITIVES
    {
        gcePATCH_ID patchId = gcvPATCH_INVALID;

        gcoHARDWARE_GetPatchID(gcvNULL, &patchId);

        if (patchId == gcvPATCH_DEBUG)
        {
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }
    }
#endif

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, outside);

    /* Flush pipe states. */
    gcmONERROR(gcoHARDWARE_FlushStates(Hardware, Type, (gctPOINTER *)&memory));

    if (Hardware->gpuRenderingMode == gcvMULTI_GPU_RENDERING_MODE_OFF)
    {
        useOneCore = gcvTRUE;
    }

    if (!Hardware->features[gcvFEATURE_NEW_GPIPE])
    {
        switch (Type)
        {
        case gcvPRIMITIVE_LINE_STRIP:
        case gcvPRIMITIVE_LINE_LOOP:
        case gcvPRIMITIVE_TRIANGLE_STRIP:
        case gcvPRIMITIVE_TRIANGLE_FAN:
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E05) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E05, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            break;

        default:
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E05) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E05, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            break;
        }
    }

    if (Hardware->features[gcvFEATURE_HALTI5] &&
        !Hardware->features[gcvFEATURE_MRT_8BIT_DUAL_PIPE_FIX] &&
        (Hardware->config->pixelPipes > 1) &&
        (Hardware->PEStates->singlePEpipe))
    {
        _SinglePEPipeSwitch(Hardware, gcvTRUE, (gctPOINTER *)&memory);
    }

    if (useOneCore)
    {
        gcoHARDWARE_MultiGPUSync(Hardware, &memory);
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0); memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0));
 } };

    }

    /* Program the AQCommandDX8Primitive.Command data. */
    memory[0] =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

    /* Program the AQCommandDX8Primitive.Primtype data. */
    memory[1] =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:0) - (0 ?
 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (xlate[Type]) & ((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)));

    /* Program the AQCommandDX8Primitive.Vertexstart data. */
    memory[2] = StartVertex;

    /* Program the AQCommandDX8Primitive.Primcount data. */
    gcmSAFECASTSIZET(memory[3], PrimitiveCount);

    memory += 4;

    /* Dump the draw primitive. */
    gcmDUMP(gcvNULL,
            "#[draw 0x%08X 0x%08X 0x%08X]",
            xlate[Type],
            StartVertex,
            PrimitiveCount);

    if (useOneCore)
    {
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK; memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };

        gcoHARDWARE_MultiGPUSync(Hardware, &memory);
    }

    if (Hardware->features[gcvFEATURE_HALTI5] &&
        !Hardware->features[gcvFEATURE_MRT_8BIT_DUAL_PIPE_FIX] &&
        (Hardware->config->pixelPipes > 1) &&
        (Hardware->PEStates->singlePEpipe))
    {
        _SinglePEPipeSwitch(Hardware, gcvFALSE, (gctPOINTER *)&memory);
    }
    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, outside);

#if gcdFPGA_BUILD
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                       0x00A18,
                                       0x00000000));
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                       0x00A18,
                                       0x00000000));
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                       0x00A18,
                                       0x00000000));
#endif


    /* Mark tile status buffers as dirty. */
    Hardware->MCDirty->cacheDirty        = gcvTRUE;

    /* change xfb/query status in cmdbuffer */
    Hardware->XFBStates->statusInCmd = Hardware->XFBStates->status;

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
**  gcoHARDWARE_DrawIndirectPrimitives
**
**  Draw a number of primitives through indirect parameter buffer
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcePRIMITIVE Type
**          Type of the primitives to draw.  Can be one of the following:
**
**              gcvPRIMITIVE_POINT_LIST     - List of points.
**              gcvPRIMITIVE_LINE_LIST      - List of lines.
**              gcvPRIMITIVE_LINE_STRIP     - List of connecting lines.
**              gcvPRIMITIVE_LINE_LOOP      - List of connecting lines where the
**                                           first and last line will also be
**                                           connected.
**              gcvPRIMITIVE_TRIANGLE_LIST  - List of triangles.
**              gcvPRIMITIVE_TRIANGLE_STRIP - List of connecting triangles layed
**                                           out in a strip.
**              gcvPRIMITIVE_TRIANGLE_FAN   - List of connecting triangles layed
**                                           out in a fan.
**
**      gctBOOL DrawIndex
**          Whether or not to draw index mode.
**
**      gctUINT Address
**          Address to the draw parameter buffer.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_DrawIndirectPrimitives(
    IN gcoHARDWARE Hardware,
    IN gctBOOL DrawIndex,
    IN gcePRIMITIVE Type,
    IN gctUINT Address
    )
{
    gceSTATUS status;

    gctUINT32 drawMode = DrawIndex ?
                         0x1:
                         0x0;

    gctPOINTER *outside = gcvNULL;
    gctBOOL useOneCore = gcvFALSE;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    static const gctINT32 xlate[] =
    {
        /* gcvPRIMITIVE_POINT_LIST */
        0x1,
        /* gcvPRIMITIVE_LINE_LIST */
        0x2,
        /* gcvPRIMITIVE_LINE_STRIP */
        0x3,
        /* gcvPRIMITIVE_LINE_LOOP */
        0x7,
        /* gcvPRIMITIVE_TRIANGLE_LIST */
        0x4,
        /* gcvPRIMITIVE_TRIANGLE_STRIP */
        0x5,
        /* gcvPRIMITIVE_TRIANGLE_FAN */
        0x6,
        /* gcvPRIMITIVE_RECTANGLE */
        0x8,
        /* gcvPRIMITIVE_LINES_ADJACENCY */
        0x9,
        /* gcvPRIMITIVE_LINE_STRIP_ADJACENCY */
        0xA,
        /* gcvPRIMITIVE_TRIANGLES_ADJACENCY */
        0xB,
        /* gcvPRIMITIVE_TRIANGLE_STRIP_ADJACENCY */
        0xC,
        /* gcvPRIMITIVE_PATCH_LIST */
        0xD,
    };

    gcmHEADER_ARG("Hardware=0x%x Type=%d DrawIndex=%d Address=0x%x",
                  Hardware, Type, DrawIndex, Address);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

#if gcdDEBUG_OPTION && gcdDEBUG_OPTION_NO_DRAW_PRIMITIVES
    {
        gcePATCH_ID patchId = gcvPATCH_INVALID;

        gcoHARDWARE_GetPatchID(gcvNULL, &patchId);

        if (patchId == gcvPATCH_DEBUG)
        {
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }
    }
#endif

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, outside);

    /* Flush pipe states. */
    gcmONERROR(gcoHARDWARE_FlushStates(Hardware, Type, (gctPOINTER *)&memory));

    if (Hardware->gpuRenderingMode == gcvMULTI_GPU_RENDERING_MODE_OFF)
    {
        useOneCore = gcvTRUE;
    }

    if (!Hardware->features[gcvFEATURE_NEW_GPIPE])
    {
        switch (Type)
        {
        case gcvPRIMITIVE_LINE_STRIP:
        case gcvPRIMITIVE_LINE_LOOP:
        case gcvPRIMITIVE_TRIANGLE_STRIP:
        case gcvPRIMITIVE_TRIANGLE_FAN:
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E05) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E05, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            break;

        default:
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E05) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E05, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            break;
        }
    }

    if (Hardware->features[gcvFEATURE_HW_TFB])
    {
        _InternalTFBSwitch(Hardware, gcvTRUE, (gctPOINTER *)&memory);
    }

    if (Hardware->features[gcvFEATURE_HALTI5] &&
        !Hardware->features[gcvFEATURE_MRT_8BIT_DUAL_PIPE_FIX] &&
        (Hardware->config->pixelPipes > 1) &&
        (Hardware->PEStates->singlePEpipe))
    {
        _SinglePEPipeSwitch(Hardware, gcvTRUE, (gctPOINTER *)&memory);
    }

    if (useOneCore)
    {
        gcoHARDWARE_MultiGPUSync(Hardware, &memory);
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0); memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0));
 } };

    }


    /* Program the GCCMD_DRAW_INDIRECT_COMMAND.Command data. */
    memory[0] =
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (drawMode) & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:0) - (0 ?
 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (xlate[Type]) & ((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:14) - (0 ?
 14:14) + 1))))))) << (0 ?
 14:14))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14)));


    /* Program the GCCMD_DRAW_INDIRECT_ADDRESS data. */

    memory[1] = Address;

    memory += 2;

    /* Dump the draw primitive. */
    gcmDUMP(gcvNULL,
            "#[draw.indirectprim 0x%08X 0x%08X]",
            xlate[Type],
            Address);

    if (useOneCore)
    {
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK; memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };

        gcoHARDWARE_MultiGPUSync(Hardware, &memory);
    }

    if (Hardware->features[gcvFEATURE_HALTI5] &&
        !Hardware->features[gcvFEATURE_MRT_8BIT_DUAL_PIPE_FIX] &&
        (Hardware->config->pixelPipes > 1) &&
        (Hardware->PEStates->singlePEpipe))
    {
        _SinglePEPipeSwitch(Hardware, gcvFALSE, (gctPOINTER *)&memory);
    }
    if (Hardware->features[gcvFEATURE_HW_TFB])
    {
        _InternalTFBSwitch(Hardware, gcvFALSE, (gctPOINTER *)&memory);
    }

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, outside);

#if gcdFPGA_BUILD
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                       0x00A18,
                                       0x00000000));
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                       0x00A18,
                                       0x00000000));
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                       0x00A18,
                                       0x00000000));
#endif


    /* Mark tile status buffers as dirty. */
    Hardware->MCDirty->cacheDirty        = gcvTRUE;

    /* change xfb/query status in cmdbuffer */
    Hardware->XFBStates->statusInCmd = Hardware->XFBStates->status;

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
**  gcoHARDWARE_MultiDrawIndirectPrimitives
**
**  Draw a number of primitives through indirect parameter buffer
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcePRIMITIVE Type
**          Type of the primitives to draw.  Can be one of the following:
**
**              gcvPRIMITIVE_POINT_LIST     - List of points.
**              gcvPRIMITIVE_LINE_LIST      - List of lines.
**              gcvPRIMITIVE_LINE_STRIP     - List of connecting lines.
**              gcvPRIMITIVE_LINE_LOOP      - List of connecting lines where the
**                                           first and last line will also be
**                                           connected.
**              gcvPRIMITIVE_TRIANGLE_LIST  - List of triangles.
**              gcvPRIMITIVE_TRIANGLE_STRIP - List of connecting triangles layed
**                                           out in a strip.
**              gcvPRIMITIVE_TRIANGLE_FAN   - List of connecting triangles layed
**                                           out in a fan.
**
**      gctBOOL DrawIndex
**          Whether or not to draw index mode.
**
**      gctINT DrawCount
**          Draw command number.
**
**      gctINT Stride
**          Draw parameter stride.
**
**      gctUINT Address
**          Address to the draw parameter buffer.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_MultiDrawIndirectPrimitives(
    IN gcoHARDWARE Hardware,
    IN gctBOOL DrawIndex,
    IN gcePRIMITIVE Type,
    IN gctINT DrawCount,
    IN gctINT Stride,
    IN gctUINT Address
    )
{
    gceSTATUS status;

    gctUINT32 drawMode = DrawIndex ?
                         0x1:
                         0x0;

    gctPOINTER *outside = gcvNULL;

    gctBOOL useOneCore = gcvFALSE;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    static const gctINT32 xlate[] =
    {
        /* gcvPRIMITIVE_POINT_LIST */
        0x1,
        /* gcvPRIMITIVE_LINE_LIST */
        0x2,
        /* gcvPRIMITIVE_LINE_STRIP */
        0x3,
        /* gcvPRIMITIVE_LINE_LOOP */
        0x7,
        /* gcvPRIMITIVE_TRIANGLE_LIST */
        0x4,
        /* gcvPRIMITIVE_TRIANGLE_STRIP */
        0x5,
        /* gcvPRIMITIVE_TRIANGLE_FAN */
        0x6,
        /* gcvPRIMITIVE_RECTANGLE */
        0x8,
        /* gcvPRIMITIVE_LINES_ADJACENCY */
        0x9,
        /* gcvPRIMITIVE_LINE_STRIP_ADJACENCY */
        0xA,
        /* gcvPRIMITIVE_TRIANGLES_ADJACENCY */
        0xB,
        /* gcvPRIMITIVE_TRIANGLE_STRIP_ADJACENCY */
        0xC,
        /* gcvPRIMITIVE_PATCH_LIST */
        0xD,
    };

    gcmHEADER_ARG("Hardware=0x%x Type=%d DrawIndex=%d Address=0x%x DrawCount=%d Stride=%d",
        Hardware, Type, DrawIndex, DrawCount, Stride,Address);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

#if gcdDEBUG_OPTION && gcdDEBUG_OPTION_NO_DRAW_PRIMITIVES
    {
        gcePATCH_ID patchId = gcvPATCH_INVALID;

        gcoHARDWARE_GetPatchID(gcvNULL, &patchId);

        if (patchId == gcvPATCH_DEBUG)
        {
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }
    }
#endif
    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, outside);

    /* Flush pipe states. */
    gcmONERROR(gcoHARDWARE_FlushStates(Hardware, Type, (gctPOINTER *)&memory));

    if (Hardware->gpuRenderingMode == gcvMULTI_GPU_RENDERING_MODE_OFF)
    {
        useOneCore = gcvTRUE;
    }

    if (!Hardware->features[gcvFEATURE_NEW_GPIPE])
    {
        switch (Type)
        {
        case gcvPRIMITIVE_LINE_STRIP:
        case gcvPRIMITIVE_LINE_LOOP:
        case gcvPRIMITIVE_TRIANGLE_STRIP:
        case gcvPRIMITIVE_TRIANGLE_FAN:
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E05) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E05, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            break;

        default:
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E05) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E05, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            break;
        }
    }

    if (Hardware->features[gcvFEATURE_HW_TFB])
    {
        _InternalTFBSwitch(Hardware, gcvTRUE, (gctPOINTER *)&memory);
    }

    if (Hardware->features[gcvFEATURE_HALTI5] &&
        !Hardware->features[gcvFEATURE_MRT_8BIT_DUAL_PIPE_FIX] &&
        (Hardware->config->pixelPipes > 1) &&
        (Hardware->PEStates->singlePEpipe))
    {
        _SinglePEPipeSwitch(Hardware, gcvTRUE, (gctPOINTER *)&memory);
    }

    if (useOneCore)
    {
        gcoHARDWARE_MultiGPUSync(Hardware, &memory);
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0); memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0));
 } };

    }

    /* Program the GCCMD_DRAW_INDIRECT_COMMAND.Command data. */

    memory[0] =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (drawMode) & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:0) - (0 ?
 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (xlate[Type]) & ((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:14) - (0 ?
 14:14) + 1))))))) << (0 ?
 14:14))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14)));


    /* Program the GCCMD_DRAW_INDIRECT_ADDRESS data. */

    memory[1] = Address;
    memory[2] =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:0) - (0 ?
 17:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:0) - (0 ?
 17:0) + 1))))))) << (0 ?
 17:0))) | (((gctUINT32) ((gctUINT32) (DrawCount) & ((gctUINT32) ((((1 ?
 17:0) - (0 ?
 17:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:0) - (0 ? 17:0) + 1))))))) << (0 ? 17:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:18) - (0 ?
 31:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:18) - (0 ?
 31:18) + 1))))))) << (0 ?
 31:18))) | (((gctUINT32) ((gctUINT32) (Stride) & ((gctUINT32) ((((1 ?
 31:18) - (0 ?
 31:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:18) - (0 ? 31:18) + 1))))))) << (0 ? 31:18)));

    memory += 3;

    /* Dump the draw primitive. */
    gcmDUMP(gcvNULL,
            "#[multidraw.indirectprim count 0x%08X 0x%08X 0x%08X]",
            DrawCount,
            xlate[Type],
            Address);

    if (useOneCore)
    {
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK; memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };

        gcoHARDWARE_MultiGPUSync(Hardware, &memory);
    }

    if (Hardware->features[gcvFEATURE_HALTI5] &&
        !Hardware->features[gcvFEATURE_MRT_8BIT_DUAL_PIPE_FIX] &&
        (Hardware->config->pixelPipes > 1) &&
        (Hardware->PEStates->singlePEpipe))
    {
        _SinglePEPipeSwitch(Hardware, gcvFALSE, (gctPOINTER *)&memory);
    }
    if (Hardware->features[gcvFEATURE_HW_TFB])
    {
        _InternalTFBSwitch(Hardware, gcvFALSE, (gctPOINTER *)&memory);
    }

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, outside);

#if gcdFPGA_BUILD
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
        0x00A18,
        0x00000000));
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
        0x00A18,
        0x00000000));
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
        0x00A18,
        0x00000000));
#endif


    /* Mark tile status buffers as dirty. */
    Hardware->MCDirty->cacheDirty        = gcvTRUE;

    /* change xfb/query status in cmdbuffer */
    Hardware->XFBStates->statusInCmd = Hardware->XFBStates->status;

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
**  gcoHARDWARE_DrawInstancedPrimitives
**
**  Draw a number of primitives.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**
**      gcePRIMITIVE Type
**          Type of the primitives to draw.  Can be one of the following:
**
**              gcvPRIMITIVE_POINT_LIST     - List of points.
**              gcvPRIMITIVE_LINE_LIST      - List of lines.
**              gcvPRIMITIVE_LINE_STRIP     - List of connecting lines.
**              gcvPRIMITIVE_LINE_LOOP      - List of connecting lines where the
**                                           first and last line will also be
**                                           connected.
**              gcvPRIMITIVE_TRIANGLE_LIST  - List of triangles.
**              gcvPRIMITIVE_TRIANGLE_STRIP - List of connecting triangles layed
**                                           out in a strip.
**              gcvPRIMITIVE_TRIANGLE_FAN   - List of connecting triangles layed
**                                           out in a fan.
**
**      gctINT StartVertex
**          Starting vertex number to start drawing.  The starting vertex is
**          multiplied by the stream stride to compute the starting offset.
**
**      gctSIZE_T PrimitiveCount
**          Number of primitives to draw.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_DrawInstancedPrimitives(
    IN gcoHARDWARE Hardware,
    IN gctBOOL DrawIndex,
    IN gcePRIMITIVE Type,
    IN gctINT StartVertex,
    IN gctINT StartIndex,
    IN gctSIZE_T PrimitiveCount,
    IN gctSIZE_T VertexCount,
    IN gctSIZE_T InstanceCount
    )
{
    gceSTATUS status;
    gctSIZE_T vertexCount;
    gctUINT32 drawCommand;
    gctUINT32 drawCount;
    gctUINT32 drawMode = DrawIndex ?
                         0x1 :
                         0x0;
    gctPOINTER *outside = gcvNULL;
    gctBOOL useOneCore = gcvFALSE;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    static const gctINT32 xlate[] =
    {
        /* gcvPRIMITIVE_POINT_LIST */
        0x1,
        /* gcvPRIMITIVE_LINE_LIST */
        0x2,
        /* gcvPRIMITIVE_LINE_STRIP */
        0x3,
        /* gcvPRIMITIVE_LINE_LOOP */
        0x7,
        /* gcvPRIMITIVE_TRIANGLE_LIST */
        0x4,
        /* gcvPRIMITIVE_TRIANGLE_STRIP */
        0x5,
        /* gcvPRIMITIVE_TRIANGLE_FAN */
        0x6,
        /* gcvPRIMITIVE_RECTANGLE */
        0x8,
        /* gcvPRIMITIVE_LINES_ADJACENCY */
        0x9,
        /* gcvPRIMITIVE_LINE_STRIP_ADJACENCY */
        0xA,
        /* gcvPRIMITIVE_TRIANGLES_ADJACENCY */
        0xB,
        /* gcvPRIMITIVE_TRIANGLE_STRIP_ADJACENCY */
        0xC,
        /* gcvPRIMITIVE_PATCH_LIST */
        0xD,
    };

    gcmHEADER_ARG("Hardware=0x%x Type=%d StartVertex=%d PrimitiveCount=%u, VertexCount=%u, IntanceCount=%u",
                  Hardware, Type, StartVertex, PrimitiveCount, VertexCount, InstanceCount);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmDEBUG_VERIFY_ARGUMENT(PrimitiveCount > 0);
    gcmDEBUG_VERIFY_ARGUMENT(VertexCount > 0);
    gcmDEBUG_VERIFY_ARGUMENT(InstanceCount > 0);

#if gcdDEBUG_OPTION && gcdDEBUG_OPTION_NO_DRAW_PRIMITIVES
    {
        gcePATCH_ID patchId = gcvPATCH_INVALID;

        gcoHARDWARE_GetPatchID(gcvNULL, &patchId);

        if (patchId == gcvPATCH_DEBUG)
        {
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }
    }
#endif

    if (!Hardware->features[gcvFEATURE_HALTI2])
    {
        switch (Type)
        {
            case gcvPRIMITIVE_TRIANGLE_LIST:
                vertexCount = VertexCount - (VertexCount % 3);
                break;

            case gcvPRIMITIVE_LINE_LIST:
                vertexCount = VertexCount - (VertexCount % 2);
                break;

            case gcvPRIMITIVE_LINE_STRIP:
            case gcvPRIMITIVE_TRIANGLE_STRIP:
            case gcvPRIMITIVE_TRIANGLE_FAN:
            default:
                vertexCount = VertexCount;
                break;
        }
    }
    else
    {
        vertexCount = VertexCount;
    }

    /* Determine draw command. */
    drawCommand = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0C & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (drawMode) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (InstanceCount) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (xlate[Type]) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)));

    drawCount = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:0) - (0 ?
 23:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:0) - (0 ?
 23:0) + 1))))))) << (0 ?
 23:0))) | (((gctUINT32) ((gctUINT32) (vertexCount) & ((gctUINT32) ((((1 ?
 23:0) - (0 ?
 23:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:0) - (0 ? 23:0) + 1))))))) << (0 ? 23:0)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:24) - (0 ?
 31:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:24) - (0 ?
 31:24) + 1))))))) << (0 ?
 31:24))) | (((gctUINT32) ((gctUINT32) ((InstanceCount >> 16)) & ((gctUINT32) ((((1 ?
 31:24) - (0 ?
 31:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ? 31:24)));

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, outside);

    /* Flush pipe states. */
    gcmONERROR(gcoHARDWARE_FlushStates(Hardware, Type, (gctPOINTER *)&memory));

    if (Hardware->gpuRenderingMode == gcvMULTI_GPU_RENDERING_MODE_OFF)
    {
        useOneCore = gcvTRUE;
    }

    if (!Hardware->features[gcvFEATURE_NEW_GPIPE])
    {
        switch (Type)
        {
        case gcvPRIMITIVE_LINE_STRIP:
        case gcvPRIMITIVE_LINE_LOOP:
        case gcvPRIMITIVE_TRIANGLE_STRIP:
        case gcvPRIMITIVE_TRIANGLE_FAN:
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E05) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E05, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            break;
        default:
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E05) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E05, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            break;
        }
    }

    if (Hardware->features[gcvFEATURE_HW_TFB])
    {
        _InternalTFBSwitch(Hardware, gcvTRUE, (gctPOINTER *)&memory);
    }
    if (Hardware->features[gcvFEATURE_HALTI5] &&
        !Hardware->features[gcvFEATURE_MRT_8BIT_DUAL_PIPE_FIX] &&
        (Hardware->config->pixelPipes > 1) &&
        (Hardware->PEStates->singlePEpipe))
    {
        _SinglePEPipeSwitch(Hardware, gcvTRUE, (gctPOINTER *)&memory);
    }

    if (useOneCore)
    {
        gcoHARDWARE_MultiGPUSync(Hardware, &memory);
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0); memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0));
 } };

    }

    /* Program the AQCommandDX8Primitive.Command data. */
    *memory++ = drawCommand;
    *memory++ = drawCount;

    if (Hardware->features[gcvFEATURE_FE_START_VERTEX_SUPPORT])
    {
        *memory++ = StartVertex;
        *memory++ = 0;
    }
    else
    {
        /* start index is not set */
        *memory++ = 0;
        *memory++ = 0;
    }

    /* Dump the draw primitive. */
    gcmDUMP(gcvNULL,
            "#[draw.instanced 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X]",
            drawMode, /* 22:20 */
            xlate[Type], /*  19:16 */
            InstanceCount, /* 15:0 */
            vertexCount, /* 23:0 */
            StartVertex /* 2 */
            );

    if (useOneCore)
    {
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK; memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };

        gcoHARDWARE_MultiGPUSync(Hardware, &memory);
    }

    if (Hardware->features[gcvFEATURE_HALTI5] &&
        !Hardware->features[gcvFEATURE_MRT_8BIT_DUAL_PIPE_FIX] &&
        (Hardware->config->pixelPipes > 1) &&
        (Hardware->PEStates->singlePEpipe))
    {
        _SinglePEPipeSwitch(Hardware, gcvFALSE, (gctPOINTER *)&memory);
    }
    if (Hardware->features[gcvFEATURE_HW_TFB])
    {
        _InternalTFBSwitch(Hardware, gcvFALSE, (gctPOINTER *)&memory);
    }

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, outside);


    /* Mark tile status buffers as dirty. */
    Hardware->MCDirty->cacheDirty        = gcvTRUE;

    /* change xfb/query status in cmdbuffer */
    Hardware->XFBStates->statusInCmd = Hardware->XFBStates->status;

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
**  gcoHARDWARE_DrawNullPrimitives
**
**  Draw none of primitives.
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
gcoHARDWARE_DrawNullPrimitives(
    IN gcoHARDWARE Hardware
    )
{
    gceSTATUS status;
    gctPOINTER *outside = gcvNULL;
    gctBOOL useOneCore = gcvFALSE;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

#if gcdDEBUG_OPTION && gcdDEBUG_OPTION_NO_DRAW_PRIMITIVES
    {
        gcePATCH_ID patchId = gcvPATCH_INVALID;

        gcoHARDWARE_GetPatchID(gcvNULL, &patchId);

        if (patchId == gcvPATCH_DEBUG)
        {
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }
    }
#endif

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, outside);

    /* Flush pipe states. */
    gcmONERROR(gcoHARDWARE_FlushStates(Hardware, gcvPRIMITIVE_TRIANGLE_LIST, (gctPOINTER *)&memory));

    if (Hardware->gpuRenderingMode == gcvMULTI_GPU_RENDERING_MODE_OFF)
    {
        useOneCore = gcvTRUE;
    }

    if (Hardware->features[gcvFEATURE_HW_TFB])
    {
        _InternalTFBSwitch(Hardware, gcvTRUE, (gctPOINTER *)&memory);
    }

    if (useOneCore)
    {
        gcoHARDWARE_MultiGPUSync(Hardware, &memory);
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0); memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0));
 } };

    }
    /* null draw command */

    if (useOneCore)
    {
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK; memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };

        gcoHARDWARE_MultiGPUSync(Hardware, &memory);
    }

    if (Hardware->features[gcvFEATURE_HW_TFB])
    {
        _InternalTFBSwitch(Hardware, gcvFALSE, (gctPOINTER *)&memory);
    }

    /* Make compiler happy */
#ifndef __clang__
    stateDelta = stateDelta;
#endif

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, outside);



    /* Mark tile status buffers as dirty. */
    Hardware->MCDirty->cacheDirty        = gcvTRUE;

    /* change xfb/query status in cmdbuffer */
    Hardware->XFBStates->statusInCmd = Hardware->XFBStates->status;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}


static gceSTATUS _GetPrimitiveCount(
    IN gcePRIMITIVE PrimitiveMode,
    IN gctSIZE_T VertexCount,
    OUT gctSIZE_T * PrimitiveCount
    )
{
    gceSTATUS result = gcvSTATUS_OK;

    /* Translate primitive count. */
    switch (PrimitiveMode)
    {
    case gcvPRIMITIVE_POINT_LIST:
        *PrimitiveCount = VertexCount;
        break;

    case gcvPRIMITIVE_LINE_LIST:
        *PrimitiveCount = VertexCount / 2;
        break;

    case gcvPRIMITIVE_LINE_LOOP:
        *PrimitiveCount = VertexCount;
        break;

    case gcvPRIMITIVE_LINE_STRIP:
        *PrimitiveCount = VertexCount - 1;
        break;

    case gcvPRIMITIVE_TRIANGLE_LIST:
        *PrimitiveCount = VertexCount / 3;
        break;

    case gcvPRIMITIVE_TRIANGLE_STRIP:
    case gcvPRIMITIVE_TRIANGLE_FAN:
        *PrimitiveCount = VertexCount - 2;
        break;

    default:
        result = gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Return result. */
    return result;
}

/*******************************************************************************
**
**  gcoHARDWARE_DrawPrimitivesCount
**
**  Draw an array of primitives.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcePRIMITIVE Type
**          Type of the primitives to draw.  Can be one of the following:
**
**              gcvPRIMITIVE_POINT_LIST     - List of points.
**              gcvPRIMITIVE_LINE_LIST      - List of lines.
**              gcvPRIMITIVE_LINE_STRIP     - List of connecting lines.
**              gcvPRIMITIVE_LINE_LOOP      - List of connecting lines where the
**                                           first and last line will also be
**                                           connected.
**              gcvPRIMITIVE_TRIANGLE_LIST  - List of triangles.
**              gcvPRIMITIVE_TRIANGLE_STRIP - List of connecting triangles layed
**                                           out in a strip.
**              gcvPRIMITIVE_TRIANGLE_FAN   - List of connecting triangles layed
**                                           out in a fan.
**
**      gctINT StartVertex
**          Starting vertex array of number to start drawing.  The starting vertex is
**          multiplied by the stream stride to compute the starting offset.
**
**      gctSIZE_T VertexCount
**          Number of vertices to draw per primitive.
**
**      gctSIZE_T PrimitiveCount
**          Number of primitives to draw.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_DrawPrimitivesCount(
    IN gcoHARDWARE Hardware,
    IN gcePRIMITIVE Type,
    IN gctINT* StartVertex,
    IN gctSIZE_T* VertexCount,
    IN gctSIZE_T PrimitiveCount
    )
{
    gceSTATUS status;
    gctUINT32 i;
    gctPOINTER *outside = gcvNULL;
    gctBOOL useOneCore = gcvFALSE;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    static const gctINT32 xlate[] =
    {
        /* gcvPRIMITIVE_POINT_LIST */
        0x1,
        /* gcvPRIMITIVE_LINE_LIST */
        0x2,
        /* gcvPRIMITIVE_LINE_STRIP */
        0x3,
        /* gcvPRIMITIVE_LINE_LOOP */
        0x7,
        /* gcvPRIMITIVE_TRIANGLE_LIST */
        0x4,
        /* gcvPRIMITIVE_TRIANGLE_STRIP */
        0x5,
        /* gcvPRIMITIVE_TRIANGLE_FAN */
        0x6,
        /* gcvPRIMITIVE_RECTANGLE */
        0x8
    };

    gcmHEADER_ARG("Hardware=0x%x Type=%d StartVertex=%d PrimitiveCount=%u",
                  Hardware, Type, StartVertex, PrimitiveCount);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmDEBUG_VERIFY_ARGUMENT(PrimitiveCount > 0);
    gcmVERIFY_ARGUMENT(Type <= gcvPRIMITIVE_RECTANGLE);

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, outside);

    /* Flush pipe states. */
    gcmONERROR(gcoHARDWARE_FlushStates(Hardware, Type, (gctPOINTER *)&memory));

    if (Hardware->gpuRenderingMode == gcvMULTI_GPU_RENDERING_MODE_OFF)
    {
        useOneCore = gcvTRUE;
    }

    if (!Hardware->features[gcvFEATURE_NEW_GPIPE])
    {
        switch (Type)
        {
        case gcvPRIMITIVE_LINE_STRIP:
        case gcvPRIMITIVE_LINE_LOOP:
        case gcvPRIMITIVE_TRIANGLE_STRIP:
        case gcvPRIMITIVE_TRIANGLE_FAN:
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E05) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E05, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            break;

        default:
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E05) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E05, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            break;
        }
    }

    if (useOneCore)
    {
        gcoHARDWARE_MultiGPUSync(Hardware, &memory);
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0); memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0));
 } };

    }

    for (i = 0; i < PrimitiveCount; i++)
    {
        gctSIZE_T count = 0;

        gcmVERIFY_OK(_GetPrimitiveCount(Type, VertexCount[i], &count));

        /* Program the AQCommandDX8Primitive.Command data. */
        *memory++ =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

        /* Program the AQCommandDX8Primitive.Primtype data. */
        *memory++ =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:0) - (0 ?
 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (xlate[Type]) & ((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)));

        /* Program the AQCommandDX8Primitive.Vertexstart data. */
        *memory++ = StartVertex[i];

        /* Program the AQCommandDX8Primitive.Primcount data. */
        gcmSAFECASTSIZET(*memory++, count);
    }

    /* Dump the draw primitive. */
    gcmDUMP(gcvNULL,
            "#[draw 0x%08X 0x%08X 0x%08X]",
            xlate[Type],
            (gctUINTPTR_T) StartVertex,
            PrimitiveCount);

    if (useOneCore)
    {
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK; memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };

        gcoHARDWARE_MultiGPUSync(Hardware, &memory);
    }

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, outside);

#if gcdFPGA_BUILD
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                       0x00A18,
                                       0x00000000));
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                       0x00A18,
                                       0x00000000));
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                       0x00A18,
                                       0x00000000));
#endif


    /* Mark tile status buffers as dirty. */
    Hardware->MCDirty->cacheDirty        = gcvTRUE;

    /* change xfb/query status in cmdbuffer */
    Hardware->XFBStates->statusInCmd = Hardware->XFBStates->status;

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
**  gcoHARDWARE_DrawPrimitivesOffset
**
**  Draw a number of primitives using offsets.
**
**  INPUT:
**
**      gcePRIMITIVE Type
**          Type of the primitives to draw.  Can be one of the following:
**
**              gcvPRIMITIVE_POINT_LIST     - List of points.
**              gcvPRIMITIVE_LINE_LIST      - List of lines.
**              gcvPRIMITIVE_LINE_STRIP     - List of connecting lines.
**              gcvPRIMITIVE_LINE_LOOP      - List of connecting lines where the
**                                           first and last line will also be
**                                           connected.
**              gcvPRIMITIVE_TRIANGLE_LIST  - List of triangles.
**              gcvPRIMITIVE_TRIANGLE_STRIP - List of connecting triangles layed
**                                           out in a strip.
**              gcvPRIMITIVE_TRIANGLE_FAN   - List of connecting triangles layed
**                                           out in a fan.
**
**      gctINT32 StartOffset
**          Starting offset.
**
**      gctSIZE_T PrimitiveCount
**          Number of primitives to draw.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_DrawPrimitivesOffset(
    IN gcePRIMITIVE Type,
    IN gctINT32 StartOffset,
    IN gctSIZE_T PrimitiveCount
    )
{
    gcmHEADER_ARG("Type=%d StartOffset=%d PrimitiveCount=%u",
                   Type, StartOffset, PrimitiveCount);

    /* No supported on XAQ2. */
    gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**  gcoHARDWARE_DrawIndexedPrimitives
**
**  Draw a number of indexed primitives.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcePRIMITIVE Type
**          Type of the primitives to draw.  Can be one of the following:
**
**              gcvPRIMITIVE_POINT_LIST     - List of points.
**              gcvPRIMITIVE_LINE_LIST      - List of lines.
**              gcvPRIMITIVE_LINE_STRIP     - List of connecting lines.
**              gcvPRIMITIVE_LINE_LOOP      - List of connecting lines where the
**                                           first and last line will also be
**                                           connected.
**              gcvPRIMITIVE_TRIANGLE_LIST  - List of triangles.
**              gcvPRIMITIVE_TRIANGLE_STRIP - List of connecting triangles layed
**                                           out in a strip.
**              gcvPRIMITIVE_TRIANGLE_FAN   - List of connecting triangles layed
**                                           out in a fan.
**
**      gctINT BaseVertex
**          Base vertex number which will be added to any indexed vertex read
**          from the index buffer.
**
**      gctINT StartIndex
**          Starting index number to start drawing.  The starting index is
**          multiplied by the index buffer stride to compute the starting
**          offset.
**
**      gctSIZE_T PrimitiveCount
**          Number of primitives to draw.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_DrawIndexedPrimitives(
    IN gcoHARDWARE Hardware,
    IN gcePRIMITIVE Type,
    IN gctINT BaseVertex,
    IN gctINT StartIndex,
    IN gctSIZE_T PrimitiveCount
    )
{
    gceSTATUS status;

    gctPOINTER *outside = gcvNULL;

    gctBOOL useOneCore = gcvFALSE;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    static const gctINT32 xlate[] =
    {
        /* gcvPRIMITIVE_POINT_LIST */
        0x1,
        /* gcvPRIMITIVE_LINE_LIST */
        0x2,
        /* gcvPRIMITIVE_LINE_STRIP */
        0x3,
        /* gcvPRIMITIVE_LINE_LOOP */
        0x7,
        /* gcvPRIMITIVE_TRIANGLE_LIST */
        0x4,
        /* gcvPRIMITIVE_TRIANGLE_STRIP */
        0x5,
        /* gcvPRIMITIVE_TRIANGLE_FAN */
        0x6,
        /* gcvPRIMITIVE_RECTANGLE */
        0x8
    };

    gcmHEADER_ARG("Hardware=0x%x Type=%d BaseVertex=%d "
                    "StartIndex=%d PrimitiveCount=%u",
                    Hardware, Type, BaseVertex,
                    StartIndex, PrimitiveCount);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmDEBUG_VERIFY_ARGUMENT(PrimitiveCount > 0);
    gcmVERIFY_ARGUMENT(Type <= gcvPRIMITIVE_RECTANGLE);

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, outside);

    /* Flush pipe states. */
    gcmONERROR(gcoHARDWARE_FlushStates(Hardware, Type, (gctPOINTER *)&memory));

    if (Hardware->gpuRenderingMode == gcvMULTI_GPU_RENDERING_MODE_OFF)
    {
        useOneCore = gcvTRUE;
    }

    if (!Hardware->features[gcvFEATURE_NEW_GPIPE])
    {
        switch (Type)
        {
        case gcvPRIMITIVE_LINE_STRIP:
        case gcvPRIMITIVE_LINE_LOOP:
        case gcvPRIMITIVE_TRIANGLE_STRIP:
        case gcvPRIMITIVE_TRIANGLE_FAN:
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E05) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E05, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            break;

        default:
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E05) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E05, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            break;
        }
    }

    if (useOneCore)
    {
        gcoHARDWARE_MultiGPUSync(Hardware, &memory);
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0); memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0));
 } };

    }

    /* Program the AQCommandDX8IndexPrimitive.Command data. */
    memory[0] =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x06 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

    /* Program the AQCommandDX8IndexPrimitive.Primtype data. */
    memory[1] =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:0) - (0 ?
 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (xlate[Type]) & ((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)));

    /* Program the AQCommandDX8IndexPrimitive.Start data. */
    memory[2] = StartIndex;

    /* Program the AQCommandDX8IndexPrimitive.Primcount data. */
    gcmSAFECASTSIZET(memory[3], PrimitiveCount);

    /* Program the AQCommandDX8IndexPrimitive.Basevertex data. */
    memory[4] = BaseVertex;

    memory += 5;

    /* Dump the indexed draw primitive. */
    gcmDUMP(gcvNULL,
            "#[draw.index 0x%08X 0x%08X 0x%08X 0x%08X]",
            xlate[Type],
            StartIndex,
            PrimitiveCount,
            BaseVertex);

    if (useOneCore)
    {
        { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK; memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };

        gcoHARDWARE_MultiGPUSync(Hardware, &memory);
    }

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, outside);

#if gcdFPGA_BUILD
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                       0x00A18,
                                       0x00000000));
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                       0x00A18,
                                       0x00000000));
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                       0x00A18,
                                       0x00000000));
#endif


    /* Mark tile status bufefrs as dirty. */
    Hardware->MCDirty->cacheDirty        = gcvTRUE;

    /* change xfb/query status in cmdbuffer */
    Hardware->XFBStates->statusInCmd = Hardware->XFBStates->status;
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
**  gcoHARDWARE_DrawIndexedPrimitivesOffset
**
**  Draw a number of indexed primitives using offsets.
**
**  INPUT:
**
**      gcePRIMITIVE Type
**          Type of the primitives to draw.  Can be one of the following:
**
**              gcvPRIMITIVE_POINT_LIST     - List of points.
**              gcvPRIMITIVE_LINE_LIST      - List of lines.
**              gcvPRIMITIVE_LINE_STRIP     - List of connecting lines.
**              gcvPRIMITIVE_LINE_LOOP      - List of connecting lines where the
**                                           first and last line will also be
**                                           connected.
**              gcvPRIMITIVE_TRIANGLE_LIST  - List of triangles.
**              gcvPRIMITIVE_TRIANGLE_STRIP - List of connecting triangles layed
**                                           out in a strip.
**              gcvPRIMITIVE_TRIANGLE_FAN   - List of connecting triangles layed
**                                           out in a fan.
**
**      gctINT BaseOffset
**          Base offset which will be added to any indexed vertex offset read
**          from the index buffer.
**
**      gctINT StartOffset
**          Starting offset into the index buffer to start drawing.
**
**      gctSIZE_T PrimitiveCount
**          Number of primitives to draw.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_DrawIndexedPrimitivesOffset(
    IN gcePRIMITIVE Type,
    IN gctINT32 BaseOffset,
    IN gctINT32 StartOffset,
    IN gctSIZE_T PrimitiveCount
    )
{
    gcmHEADER_ARG("Type=%d BaseOffset=%d "
                  "StartOffset=%d PrimitiveCount=%u",
                  Type, BaseOffset,
                  StartOffset, PrimitiveCount);

    /* No supported on XAQ2. */
    gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
    return gcvSTATUS_NOT_SUPPORTED;
}

static gceSTATUS
_FastDrawIndexedPrimitive(
    IN gcoHARDWARE Hardware,
    IN gcsFAST_FLUSH_PTR FastFlushInfo,
    INOUT gctPOINTER *Memory)
{
    gceSTATUS status;
    gctUINT primCount = FastFlushInfo->drawCount / 3;
    gctBOOL useOneCore = gcvFALSE;

    gcmHEADER_ARG("Hardware=0x%x FastFlushInfo=0x%x", Hardware, FastFlushInfo);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->gpuRenderingMode == gcvMULTI_GPU_RENDERING_MODE_OFF)
    {
        useOneCore = gcvTRUE;
    }

    /* Send command */
    if (FastFlushInfo->hasHalti)
    {
        gctSIZE_T vertexCount;
        gctUINT32 drawCommand;
        gctUINT32 drawCount;
        gctUINT32 drawMode = (FastFlushInfo->indexCount != 0) ?
                0x1 :
                0x0;

        /* Define state buffer variables. */
        gcmDEFINESTATEBUFFER_NEW_FAST(reserve, memory);

        if (!Hardware->features[gcvFEATURE_HALTI2])
        {
            vertexCount = FastFlushInfo->drawCount - (FastFlushInfo->drawCount % 3);
        }
        else
        {
            vertexCount = FastFlushInfo->drawCount;
        }

        /* Determine draw command. */
        drawCommand = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0C & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (drawMode) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (FastFlushInfo->instanceCount) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)));

        drawCount = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:0) - (0 ?
 23:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:0) - (0 ?
 23:0) + 1))))))) << (0 ?
 23:0))) | (((gctUINT32) ((gctUINT32) (vertexCount) & ((gctUINT32) ((((1 ?
 23:0) - (0 ?
 23:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:0) - (0 ? 23:0) + 1))))))) << (0 ? 23:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:24) - (0 ?
 31:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:24) - (0 ?
 31:24) + 1))))))) << (0 ?
 31:24))) | (((gctUINT32) ((gctUINT32) ((FastFlushInfo->instanceCount >> 16)) & ((gctUINT32) ((((1 ?
 31:24) - (0 ?
 31:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ? 31:24)));

        /* Reserve space in the command buffer. */
        gcmBEGINSTATEBUFFER_NEW_FAST(Hardware, reserve, memory, Memory);

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E05) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW_FAST(stateDelta, reserve, memory, gcvFALSE, 0x0E05, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        if (Hardware->features[gcvFEATURE_HW_TFB])
        {
            _InternalTFBSwitch(Hardware, gcvTRUE, (gctPOINTER *)&memory);
        }

        if (Hardware->features[gcvFEATURE_HALTI5] &&
            !Hardware->features[gcvFEATURE_MRT_8BIT_DUAL_PIPE_FIX] &&
            (Hardware->config->pixelPipes > 1) &&
            (Hardware->PEStates->singlePEpipe))
        {
            _SinglePEPipeSwitch(Hardware, gcvTRUE, (gctPOINTER *)&memory);
        }
        if (useOneCore)
        {
            gcoHARDWARE_MultiGPUSync(Hardware, &memory);
            { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0); memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0));
 } };

        }

        /* Program the AQCommandDX8Primitive.Command data. */
        *memory++ = drawCommand;
        *memory++ = drawCount;

        if (Hardware->features[gcvFEATURE_HALTI2])
        {
            *memory++ = 0;
            *memory++ = 0;
        }
        else
        {
            /* start index is not set */
            *memory++ = 0;

            gcmSETFILLER_NEW(
                reserve,
                memory
                );
        }

        if (useOneCore)
        {
            { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK; memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };

            gcoHARDWARE_MultiGPUSync(Hardware, &memory);
        }

        if (Hardware->features[gcvFEATURE_HALTI5] &&
            !Hardware->features[gcvFEATURE_MRT_8BIT_DUAL_PIPE_FIX] &&
            (Hardware->config->pixelPipes > 1) &&
            (Hardware->PEStates->singlePEpipe))
        {
            _SinglePEPipeSwitch(Hardware, gcvFALSE, (gctPOINTER *)&memory);
        }
        if (Hardware->features[gcvFEATURE_HW_TFB])
        {
            _InternalTFBSwitch(Hardware, gcvFALSE, (gctPOINTER *)&memory);
        }

        /* Validate the state buffer. */
        gcmENDSTATEBUFFER_NEW_FAST(Hardware, reserve, memory, Memory);
    }
    else
    {
        gcmDEFINESTATEBUFFER_NEW_FAST(reserve, memory);

        /* Reserve space in the command buffer. */
        gcmBEGINSTATEBUFFER_NEW_FAST(Hardware, reserve, memory, Memory);

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E05) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));};
    gcmSETSTATEDATA_NEW_FAST(stateDelta, reserve, memory, gcvFALSE, 0x0E05, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        if (useOneCore)
        {
            gcoHARDWARE_MultiGPUSync(Hardware, &memory);
            { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0); memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0));
 } };

        }

        /* Program the AQCommandDX8IndexPrimitive.Command data. */
        memory[0] =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x06 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

        /* Program the AQCommandDX8IndexPrimitive.Primtype data. */
        memory[1] =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:0) - (0 ?
 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)));

        /* Program the AQCommandDX8IndexPrimitive.Start data. */
        memory[2] = 0;

        /* Program the AQCommandDX8IndexPrimitive.Primcount data. */
        gcmSAFECASTSIZET(memory[3], primCount);

        /* Program the AQCommandDX8IndexPrimitive.Basevertex data. */
        memory[4] = 0;

        memory += 5;

        if (5 & 1)
        {
            gcmSETFILLER_NEW(
                reserve,
                memory
                );
        }

        if (useOneCore)
        {
            { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK; memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };

            gcoHARDWARE_MultiGPUSync(Hardware, &memory);
        }

        /* Validate the state buffer. */
        gcmENDSTATEBUFFER_NEW_FAST(Hardware, reserve, memory, Memory);
    }

    /* Success */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status */
    gcmFOOTER();
    return status;
}


gceSTATUS
gcoHARDWARE_DrawPattern(
    IN gcoHARDWARE Hardware,
    IN gcsFAST_FLUSH_PTR FastFlushInfo
    )
{
    gceSTATUS status;

    gctPOINTER cmdBuffer = gcvNULL;

    gcsTEMPCMDBUF tempCMD;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    gcmONERROR(gcoBUFFER_StartTEMPCMDBUF(Hardware->engine[gcvENGINE_RENDER].buffer, Hardware->engine[gcvENGINE_RENDER].queue, &tempCMD));

    cmdBuffer = tempCMD->buffer;

    gcmONERROR(gcoHARDWARE_FastFlushUniforms(Hardware, FastFlushInfo, &cmdBuffer));

    gcmONERROR(gcoHARDWARE_FastFlushStream(Hardware, FastFlushInfo, &cmdBuffer));

    gcmONERROR(gcoHARDWARE_FastProgramIndex(Hardware, FastFlushInfo, &cmdBuffer));

    gcmONERROR(gcoHARDWARE_FastFlushAlpha(Hardware, FastFlushInfo, &cmdBuffer));

    gcmONERROR(gcoHARDWARE_FastFlushDepthCompare(Hardware, FastFlushInfo, &cmdBuffer));

    gcmONERROR(_FastDrawIndexedPrimitive(Hardware, FastFlushInfo, &cmdBuffer));

    tempCMD->currentByteSize =  (gctUINT32)((gctUINT8_PTR)cmdBuffer -
                                (gctUINT8_PTR)tempCMD->buffer);

    gcmONERROR(gcoBUFFER_EndTEMPCMDBUF(Hardware->engine[gcvENGINE_RENDER].buffer, gcvFALSE));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}
#endif

/*******************************************************************************
**
**  gcoHARDWARE_TranslateCommand
**
**  Translate API 2D command to its hardware value.
**
**  INPUT:
**
**      gce2D_COMMAND APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslateCommand(
    IN gce2D_COMMAND APIValue,
    OUT gctUINT32 * HwValue
    )
{
    gcmHEADER_ARG("APIValue=%d HwValue=0x%x", APIValue, HwValue);

    /* Dispatch on command. */
    switch (APIValue)
    {
    case gcv2D_CLEAR:
        *HwValue = 0x0;
        break;

    case gcv2D_LINE:
        *HwValue = 0x1;
        break;

    case gcv2D_BLT:
        *HwValue = 0x2;
        break;

    case gcv2D_STRETCH:
        *HwValue = 0x4;
        break;

    case gcv2D_HOR_FILTER:
        *HwValue = 0x5;
        break;

    case gcv2D_VER_FILTER:
        *HwValue = 0x6;
        break;

    case gcv2D_MULTI_SOURCE_BLT:
        *HwValue = 0x8;
        break;

    default:
        /* Not supported. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Success. */
    gcmFOOTER_ARG("*HwValue=%d", *HwValue);
    return gcvSTATUS_OK;
}

#if gcdENABLE_2D
static gceSTATUS
gcoHARDWARE_BrushStretchBlit(
    IN gcoHARDWARE Hardware,
    IN gcs2D_State_PTR State,
    IN gctUINT32 RectCount,
    IN gcsRECT_PTR Rect
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT32 surfSize = 640;
    gcs2D_State state;
    gcs2D_MULTI_SOURCE_PTR curSrc;
    gcoSURF surface;
    gctUINT8_PTR logical1, logical2;
    gctUINT32 address;
    gctUINT32 i, lastW, lastH, curW, curH, num = 1;
    gcsRECT_PTR rect, rectStart;

    gcmHEADER_ARG("RectCount=%d Rect=0x%x",
                  RectCount, Rect);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->clearSrcSurf == gcvNULL)
    {
        gcmONERROR(gcoHARDWARE_Alloc2DSurface(
            Hardware, surfSize, surfSize, gcvSURF_A8R8G8B8, State->dstSurface.hints, &Hardware->clearSrcSurf));

        Hardware->clearSrcRect.left = Hardware->clearSrcRect.top = 0;
        Hardware->clearSrcRect.right = Hardware->clearSrcRect.bottom = 2;
    }

    logical1 = (gctUINT8_PTR)Hardware->clearSrcSurf->node.logical +
                             Hardware->clearSrcRect.top * Hardware->clearSrcSurf->stride +
                             Hardware->clearSrcRect.left * 4;

    logical2 = logical1 + Hardware->clearSrcSurf->stride;

    *((gctUINT32_PTR)logical1) = *((gctUINT32_PTR)logical1 + 1) =
        *((gctUINT32_PTR)logical2) = *((gctUINT32_PTR)logical2 + 1) = State->clearColor;

    gcoOS_MemCopy(&state, State, sizeof(state));

    curSrc = state.multiSrc + state.currentSrcIndex;
    curSrc->srcColorKeyHigh =
    curSrc->srcColorKeyLow = 0;
    surface                     = &curSrc->srcSurface;
    surface->type               = gcvSURF_BITMAP;
    surface->format             = gcvSURF_A8R8G8B8;
    surface->alignedW           = Hardware->clearSrcSurf->alignedW;
    surface->alignedH           = Hardware->clearSrcSurf->alignedH;
    surface->rotation           = gcvSURF_0_DEGREE;
    surface->stride             = Hardware->clearSrcSurf->stride;
    curSrc->srcRelativeCoord    = gcvFALSE;
    curSrc->srcType             = gcv2D_SOURCE_COLOR;

    gcmGETHARDWAREADDRESS(Hardware->clearSrcSurf->node, address);
    gcsSURF_NODE_SetHardwareAddress(&surface->node, address);

    state.multiSrc[state.currentSrcIndex].srcRect = Hardware->clearSrcRect;

    state.multiSrc[state.currentSrcIndex].fgRop = 0xCC;
    state.multiSrc[state.currentSrcIndex].bgRop = 0xCC;

    curW = lastW = Rect->right - Rect->left;
    curH = lastH = Rect->bottom - Rect->top;
    rectStart = Rect;

    for (i = 1; i < RectCount; i++)
    {
        rect = Rect + i;
        curW = rect->right - rect->left;
        curH = rect->bottom - rect->top;

        /* Commit accumulated rects */
        if (curW != lastW || curH != lastH)
        {
            state.multiSrc[state.currentSrcIndex].horFactor =
                gcoHARDWARE_GetStretchFactor(
                        state.multiSrc[state.currentSrcIndex].enableGDIStretch,
                        2, lastW
                        );

            state.multiSrc[state.currentSrcIndex].verFactor =
                gcoHARDWARE_GetStretchFactor(
                        state.multiSrc[state.currentSrcIndex].enableGDIStretch,
                        2, lastH
                        );

            gcmONERROR(gcoHARDWARE_StartDE(
                  Hardware,
                  &state,
                  gcv2D_STRETCH,
                  0,
                  gcvNULL,
                  num,
                  rectStart
                  ));

            lastW = curW;
            lastH = curH;
            num = 1;
            rectStart = rect;
        }
        else
        {
            num++;
        }
    }

    /* Commit left rects */
    state.multiSrc[state.currentSrcIndex].horFactor =
        gcoHARDWARE_GetStretchFactor(
                state.multiSrc[state.currentSrcIndex].enableGDIStretch,
                2, lastW
                );

    state.multiSrc[state.currentSrcIndex].verFactor =
        gcoHARDWARE_GetStretchFactor(
                state.multiSrc[state.currentSrcIndex].enableGDIStretch,
                2, lastH
                );

    gcmONERROR(gcoHARDWARE_StartDE(
          Hardware,
          &state,
          gcv2D_STRETCH,
          0,
          gcvNULL,
          num,
          rectStart
          ));

    /* Move to next rect. (surfSize*surfSize)/(2*2) rects in all. */
    Hardware->clearSrcRect.left += 2;
    if (Hardware->clearSrcRect.left == surfSize)
    {
        Hardware->clearSrcRect.left = 0;
        Hardware->clearSrcRect.top += 2;
        if (Hardware->clearSrcRect.top == surfSize)
            Hardware->clearSrcRect.top = 0;
    }
    Hardware->clearSrcRect.right = Hardware->clearSrcRect.left + 2;
    Hardware->clearSrcRect.bottom = Hardware->clearSrcRect.top + 2;

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_Clear2D
**
**  Clear one or more rectangular areas.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 RectCount
**          The number of rectangles to draw. The array of line positions
**          pointed to by Position parameter must have at least RectCount
**          positions.
**
**      gcsRECT_PTR Rect
**          Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**      gctUINT32 Color
**          32-bit clear color value.
**
**      gctBOOL ColorConvert
**          If set to gcvTRUE, the 32-bit values in the table are assumed to be
**          in ARGB8 format and will be converted by the hardware to the
**          destination format as needed.
**          If set to gcvFALSE, the 32-bit values in the table are assumed to be
**          preconverted to the destination format.
**
**      gctUINT8 FgRop
**          Foreground ROP to use with opaque pixels.
**
**      gctUINT8 BgRop
**          Background ROP to use with transparent pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_Clear2D(
    IN gcoHARDWARE Hardware,
    IN gcs2D_State_PTR State,
    IN gctUINT32 RectCount,
    IN gcsRECT_PTR Rect
    )
{
    gctUINT8 fgRop = 0, bgRop = 0;
    gcsRECT dstRect = {0};
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x RectCount=%d Rect=0x%x",
                  Hardware, RectCount, Rect);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->hw2DAppendCacheFlush)
    {
        gcmONERROR(gcoHARDWARE_BrushStretchBlit(Hardware, State, RectCount, Rect));

        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    if (!Hardware->features[gcvFEATURE_2DPE20])
    {
        fgRop = State->multiSrc[State->currentSrcIndex].fgRop;
        bgRop = State->multiSrc[State->currentSrcIndex].bgRop;
        State->multiSrc[State->currentSrcIndex].fgRop = 0x0;
        State->multiSrc[State->currentSrcIndex].bgRop = 0x0;
    }

    if (Rect == gcvNULL)
    {
        /* Has target surface been set?. */
        if (State->dstSurface.type == gcvSURF_TYPE_UNKNOWN)
        {
            gcmONERROR(gcvSTATUS_INVALID_OBJECT);
        }

        dstRect.right = State->dstSurface.requestW;
        dstRect.bottom = State->dstSurface.requestH;

        /* Use target rectangle as clear rectangle. */
        Rect = &dstRect;
    }

    Hardware->hw2DClearDestRect = Rect;

    /* Kick off 2D engine. */
    gcmONERROR(gcoHARDWARE_StartDE(
        Hardware, State, gcv2D_CLEAR, 0, gcvNULL, RectCount, Rect
        ));

    if (!Hardware->features[gcvFEATURE_2DPE20])
    {
        State->multiSrc[State->currentSrcIndex].fgRop = fgRop;
        State->multiSrc[State->currentSrcIndex].bgRop = bgRop;
    }

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_Line2DEx
**
**  Draw one or more Bresenham lines using solid color(s).
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 LineCount
**          The number of lines to draw. The array of line positions pointed
**          to by Position parameter must have at least LineCount positions.
**
**      gcsRECT_PTR Position
**          Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**      gctUINT32 ColorCount
**          Set as 1, if single color for all lines.
**          Set as LineCount, if each line has its own color.
**
**      gctUINT32_PTR Color32
**          Source color array in A8R8G8B8 format.
**
**      gctUINT8 FgRop
**          Foreground ROP to use with opaque pixels.
**
**      gctUINT8 BgRop
**          Background ROP to use with transparent pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_Line2DEx(
    IN gcoHARDWARE Hardware,
    IN gcs2D_State_PTR State,
    IN gctUINT32 LineCount,
    IN gcsRECT_PTR Position,
    IN gctUINT32 ColorCount,
    IN gctUINT32_PTR Color32
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("LineCount=%d Position=0x%x "
                  "ColorCount=%d Color32=0x%x",
                  LineCount, Position,
                  ColorCount, Color32);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmASSERT((ColorCount == 1) || (ColorCount == LineCount));

    if (Hardware->hw2DAppendCacheFlush)
    {
        gce2D_PATTERN type = State->brushType;
        gctUINT8 rop = 0;

        State->brushType = gcv2D_PATTERN_SOLID;

        if (((State->multiSrc[State->currentSrcIndex].fgRop == 0xCC) ||
            (State->multiSrc[State->currentSrcIndex].fgRop == 0x66)) &&
            (State->multiSrc[State->currentSrcIndex].bgRop == State->multiSrc[State->currentSrcIndex].fgRop))
        {
            rop = State->multiSrc[State->currentSrcIndex].fgRop;
            State->multiSrc[State->currentSrcIndex].bgRop =
            State->multiSrc[State->currentSrcIndex].fgRop =
            (State->multiSrc[State->currentSrcIndex].fgRop == 0xCC) ? 0xF0 : 0x0F;
        }
        else
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        /* Kick off 2D engine. */
        gcmONERROR(gcoHARDWARE_StartDELine(
            Hardware, State, gcv2D_LINE, LineCount, Position, ColorCount, Color32
            ));

        State->multiSrc[State->currentSrcIndex].bgRop =
        State->multiSrc[State->currentSrcIndex].fgRop = rop;
        State->brushType = type;
    }
    else
    {
        gce2D_SOURCE type;
        gctBOOL stream;
        gcsRECT rect;

       /* Backup user setting. */
       type   = State->multiSrc[State->currentSrcIndex].srcType;
       stream = State->multiSrc[State->currentSrcIndex].srcStream;
       rect   = State->multiSrc[State->currentSrcIndex].srcRect;

       /* Set the solid color using a monochrome stream. */
       State->multiSrc[State->currentSrcIndex].srcType = gcv2D_SOURCE_MONO;
       State->multiSrc[State->currentSrcIndex].srcStream = gcvFALSE;
       State->multiSrc[State->currentSrcIndex].srcRect.left = 0;
       State->multiSrc[State->currentSrcIndex].srcRect.right = 0;
       State->multiSrc[State->currentSrcIndex].srcRect.top = 0;
       State->multiSrc[State->currentSrcIndex].srcRect.bottom = 0;

       /* Kick off 2D engine. */
       gcmONERROR(gcoHARDWARE_StartDELine(
           Hardware, State, gcv2D_LINE, LineCount, Position, ColorCount, Color32
           ));

       /* revert user setting. */
       State->multiSrc[State->currentSrcIndex].srcType = type;
       State->multiSrc[State->currentSrcIndex].srcStream = stream;
       State->multiSrc[State->currentSrcIndex].srcRect = rect;
    }

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_MonoBlit
**
**  Monochrome blit.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctPOINTER StreamBits
**          Pointer to the monochrome bitmap.
**
**      gcsPOINT_PTR StreamSize
**          Size of the monochrome bitmap in pixels.
**
**      gcsRECT_PTR StreamRect
**          Bounding rectangle of the area within the bitmap to render.
**
**      gceSURF_MONOPACK SrcStreamPack
**          Source bitmap packing.
**
**      gceSURF_MONOPACK DestStreamPack
**          Packing of the bitmap in the command stream.
**
**      gcsRECT_PTR DestRect
**          Pointer to an array of destination rectangles.
**
**      gctUINT32 FgRop
**          Foreground and background ROP codes.
**
**      gctUINT32 BgRop
**          Background ROP to use with transparent pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_MonoBlit(
    IN gcoHARDWARE Hardware,
    IN gcs2D_State_PTR State,
    IN gctPOINTER StreamBits,
    IN gcsPOINT_PTR StreamSize,
    IN gcsRECT_PTR StreamRect,
    IN gceSURF_MONOPACK SrcStreamPack,
    IN gceSURF_MONOPACK DestStreamPack,
    IN gcsRECT_PTR DestRect
    )
{
    gceSTATUS status;
    gctUINT32 srcStreamWidth, srcStreamHeight;
    gctUINT32 destStreamWidth, destStreamHeight;
    gctUINT32 srcPackWidth, srcPackHeight;
    gctUINT32 destPackWidth, destPackHeight;
    gctUINT32 destStreamSize;
    gctUINT32_PTR buffer;
    gctPOINTER pointer;

    gcmHEADER_ARG("Hardware=0x%x StreamBits=0x%x StreamSize=%d StreamRect=0x%x "
                    "SrcStreamPack=%d DestStreamPack=%d DestRect=0x%x",
                    Hardware, StreamBits, StreamSize, StreamRect,
                    SrcStreamPack, DestStreamPack, DestRect);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(StreamBits != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(StreamRect != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(StreamSize != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(DestRect != gcvNULL);

    /* Get stream pack sizes. */
    gcmONERROR(gco2D_GetPackSize(
        SrcStreamPack,
        &srcPackWidth,
        &srcPackHeight
        ));

    gcmONERROR(gco2D_GetPackSize(
        DestStreamPack,
        &destPackWidth,
        &destPackHeight
        ));

    /* Determine the size of the stream. */
    destStreamWidth  = StreamRect->right  - StreamRect->left;
    destStreamHeight = StreamRect->bottom - StreamRect->top;

    /* Align the height, verify alignment of the width. */
    gcmASSERT((destStreamWidth & (destPackWidth - 1)) == 0);
    destStreamHeight = gcmALIGN(destStreamHeight, destPackHeight);

    /* Determine the size of the stream in bytes and in 32-bitters. */
    destStreamSize = (destStreamWidth * destStreamHeight) >> 3;
    gcmASSERT((destStreamSize & 3) == 0);

    /* Determine the size of the source stream in pixels. */
    srcStreamWidth  = gcmALIGN(StreamSize->x, srcPackWidth);
    srcStreamHeight = gcmALIGN(StreamSize->y, srcPackHeight);

    /* Call lower layer to form a StartDE command. */
    gcmONERROR(gcoHARDWARE_StartDEStream(
        Hardware,
        State,
        DestRect,
        destStreamSize,
        &pointer
        ));

    buffer = pointer;

    /* Same packing and entire stream? */
    if ((SrcStreamPack   == DestStreamPack) &&
        (srcStreamWidth  == destStreamWidth) &&
        (srcStreamHeight == destStreamHeight) &&
        (StreamRect->left == 0) &&
        (StreamRect->top == 0))
    {
        gcmASSERT(
            (StreamRect->left == 0) && (StreamRect->top == 0)
            );

        gcoOS_MemCopy(
            buffer,
            StreamBits,
            destStreamSize
            );
    }
    else
    {
        gctUINT8_PTR srcStream;
        gctUINT8_PTR dstStream;

        /* Compute the offset into the source stream in bits/pixels. */
        gctUINT32 srcOffset = (SrcStreamPack == gcvSURF_UNPACKED)
            ?  StreamRect->top * srcStreamWidth + StreamRect->left
            : (StreamRect->left & ~(srcPackWidth - 1)) * srcStreamHeight +
              (StreamRect->left &  (srcPackWidth - 1)) +
               StreamRect->top  *   srcPackWidth;

        /* Adjust to get the offset in bytes. */
        gcmASSERT((srcOffset & 7) == 0);
        srcOffset >>= 3;

        /* Set stream bases. */
        srcStream = (gctUINT8_PTR) StreamBits + srcOffset;
        dstStream = (gctUINT8_PTR) buffer;

        /* Same packing? */
        if ((SrcStreamPack == DestStreamPack) &&
            ((StreamRect->left & (srcPackWidth - 1)) == 0) &&
            !Hardware->bigEndian)
        {
            gctUINT32 count;
            gctUINT32 passCount;
            gctUINT32 step;
            gctUINT32 copySize;
            gctUINT32 srcStride;

            /* Must be pack aligned. */
            gcmASSERT((srcOffset       & ((srcPackWidth >> 3) - 1)) == 0);
            gcmASSERT((srcStreamWidth  & (srcPackWidth       - 1)) == 0);
            gcmASSERT((destStreamWidth & (destPackWidth      - 1)) == 0);

            if (DestStreamPack == gcvSURF_UNPACKED)
            {
                srcStride = srcStreamWidth  >> 3;
                copySize  = destStreamWidth >> 3;

                /* One line at a time. */
                step      = 1;
                passCount = destStreamHeight;
            }
            else
            {
                srcStride = (srcPackWidth  * srcStreamHeight)  >> 3;
                copySize  = (destPackWidth * destStreamHeight) >> 3;

                /* One packed column at a time. */
                step      = destPackWidth;
                passCount = destStreamWidth;
            }

            for (count = 0; count < passCount; count += step)
            {
                gcoOS_MemCopy(dstStream, srcStream, copySize);
                srcStream += srcStride;
                dstStream += copySize;
            }
        }
        else
        {
            gctUINT32 destX, destY;

            gctUINT8_PTR srcLine = srcStream;
            gctUINT8_PTR destLine = dstStream;

            gctUINT32 srcLineStep, destLineStep;
            gctUINT32 srcByteIntStep, srcByteExtStep;
            gctUINT32 destByteIntStep, destByteExtStep;

            if (SrcStreamPack == gcvSURF_UNPACKED)
            {
                srcByteIntStep = srcByteExtStep = 1;
                srcLineStep    = srcStreamWidth >> 3;
            }
            else
            {
                srcByteIntStep = 1;
                srcByteExtStep = ((srcPackWidth * srcStreamHeight) - srcPackWidth + 8) >> 3;
                srcLineStep    = srcPackWidth >> 3;
            }

            if (DestStreamPack == gcvSURF_UNPACKED)
            {
                destByteIntStep = destByteExtStep = 1;
                destLineStep    = destStreamWidth >> 3;
            }
            else
            {
                destByteIntStep = 1;
                destByteExtStep = ((destPackWidth * destStreamHeight) - destPackWidth + 8) >> 3;
                destLineStep    = destPackWidth >> 3;
            }

            for (destY = 0; destY < destStreamHeight; destY++)
            {
                gctUINT32 srcX;
                gctUINT8_PTR srcByte  = srcLine;
                gctUINT8_PTR destByte = destLine;

                for (srcX = StreamRect->left, destX = 0;
                     destX < destStreamWidth;)
                {
                    /* Copy the current byte. */
                    if (Hardware->bigEndian)
                    {
                        * (gctUINT8_PTR) gcmINT2PTR((gcmPTR2INT32(destByte) & (~3)) + (3 - (gcmPTR2INT32(destByte) % 4))) =
                            * (gctUINT8_PTR) gcmINT2PTR((gcmPTR2INT32(srcByte) & (~3)) + (3 - (gcmPTR2INT32(srcByte) % 4)));
                    }
                    else
                    {
                        *destByte = *srcByte;
                    }

                    /* Advance the coordinates. */
                    srcX  += 8;
                    destX += 8;

                    /* Advance the pointers. */
                    srcByte += ((srcX % srcPackWidth) == 0)
                        ? srcByteExtStep
                        : srcByteIntStep;

                    destByte += ((destX % destPackWidth) == 0)
                        ? destByteExtStep
                        : destByteIntStep;
                }

                /* Advance the pointers. */
                srcLine  += srcLineStep;
                destLine += destLineStep;
            }
        }
    }

    /* Commit the data. */
    gcmONERROR(gcoHARDWARE_Commit(Hardware));

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_Blit
**
**  Generic blit.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 RectCount
**          The number of rectangles to draw. The array of line positions
**          pointed to by Position parameter must have at least RectCount
**          positions.
**
**      gcsRECT_PTR Rect
**          Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_Blit(
    IN gcoHARDWARE Hardware,
    IN gcs2D_State_PTR State,
    IN gctUINT32 SrcRectCount,
    IN gcsRECT_PTR SrcRect,
    IN gctUINT32 RectCount,
    IN gcsRECT_PTR Rect
    )
{
    gceSTATUS status;
    gcoSURF srcSurf, dstSurf;
    gcsRECT_PTR srcRect;
    gctBOOL doBlitTwice = gcvFALSE;
    gctUINT32 bpp = 0;

    gcmHEADER_ARG("Hardware=0x%x State=0x%x RectCount=%d Rect=0x%x",
                  Hardware, State, RectCount, Rect);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    srcSurf = &(State->multiSrc[State->currentSrcIndex].srcSurface);
    dstSurf = &(State->dstSurface);
    srcRect = &(State->multiSrc[State->currentSrcIndex].srcRect);

    if (Hardware->hw2DBlockSize)
    {
        gctBOOL useSource = gcvFALSE;
        gctUINT32 srcAddress, dstAddress;

        gcoHARDWARE_Get2DResourceUsage(
            State->multiSrc[State->currentSrcIndex].fgRop,
            State->multiSrc[State->currentSrcIndex].bgRop,
            State->multiSrc[State->currentSrcIndex].srcTransparency,
            &useSource, gcvNULL, gcvNULL
            );

        gcmGETHARDWAREADDRESS(srcSurf->node, srcAddress);
        gcmGETHARDWAREADDRESS(dstSurf->node, dstAddress);

        if (useSource && srcAddress == dstAddress)
        {
            gctUINT32 i, srcPos, dstPos;
            gcsSURF_FORMAT_INFO_PTR tempFormat[2];

            gcmONERROR(gcoHARDWARE_QueryFormat(srcSurf->format, tempFormat));
            bpp = tempFormat[0]->bitsPerPixel < 8 ? 1 : tempFormat[0]->bitsPerPixel / 8;

            if (SrcRectCount)
            {
                for (i = 0; i < SrcRectCount; i++)
                {
                    if (SrcRect[i].top == Rect[i].top)
                    {
                        doBlitTwice = gcvTRUE;
                        break;
                    }
                    else if (srcSurf->stride < 128)
                    {
                        srcPos = srcAddress + SrcRect[i].left * bpp + srcSurf->stride * SrcRect[i].top;
                        dstPos = dstAddress + Rect[i].left * bpp + dstSurf->stride * Rect[i].top;

                        if (gcmABS((gctINT32)(dstPos - srcPos)) < 128)
                        {
                            doBlitTwice = gcvTRUE;
                            break;
                        }
                    }
                }
            }
            else
            {
                srcPos = srcAddress + srcRect->left * bpp + srcSurf->stride * srcRect->top;

                for (i = 0; i < RectCount; i++)
                {
                    if (srcRect->top == Rect[i].top)
                    {
                        doBlitTwice = gcvTRUE;
                        break;
                    }
                    else if (srcSurf->stride < 128)
                    {
                        dstPos = dstAddress + Rect[i].left * bpp + dstSurf->stride * Rect[i].top;

                        if (gcmABS((gctINT32)(dstPos - srcPos)) < 128)
                        {
                            doBlitTwice = gcvTRUE;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (doBlitTwice)
    {
        gcs2D_State state;
        gctUINT32 width, height;
        gcsRECT tmpRect;

        if (SrcRectCount)
        {
            width = srcSurf->alignedW;
            height = srcSurf->alignedH;
        }
        else
        {
            width = srcRect->right - srcRect->left;
            height = srcRect->bottom - srcRect->top;
        }

        tmpRect.left = tmpRect.top = 0;
        tmpRect.right = width;
        tmpRect.bottom = height;

        if (!Hardware->blitTmpSurf ||
            (Hardware->blitTmpSurf->node.size <
                (gctSIZE_T)(width * height * bpp)))
        {
            if (Hardware->blitTmpSurf)
            {
                gcmONERROR(gcoHARDWARE_Free2DSurface(Hardware, Hardware->blitTmpSurf));
                Hardware->blitTmpSurf = gcvNULL;
            }

            gcmONERROR(gcoHARDWARE_Alloc2DSurface(
                Hardware,
                width,
                height,
                srcSurf->format,
                gcvSURF_BITMAP,
                &Hardware->blitTmpSurf));

            gcoOS_ZeroMemory(
                Hardware->blitTmpSurf->node.logical,
                Hardware->blitTmpSurf->node.size);

            gcmONERROR(gcoOS_CacheFlush(
                gcvNULL,
                Hardware->blitTmpSurf->node.u.normal.node,
                Hardware->blitTmpSurf->node.logical,
                Hardware->blitTmpSurf->node.size));
        }

        /* Blit src surface to tmp surface first. */
        gcoOS_MemCopy(&state, State, sizeof(state));

        state.dstSurface = *(Hardware->blitTmpSurf);
        state.currentSrcIndex = 0;
        state.srcMask = 1;
        state.multiSrc[0].enableAlpha = gcvFALSE;
        state.multiSrc[0].srcTransparency = gcv2D_OPAQUE;
        state.multiSrc[0].dstTransparency = gcv2D_OPAQUE;
        state.multiSrc[0].patTransparency = gcv2D_OPAQUE;
        state.multiSrc[0].fgRop = 0xCC;
        state.multiSrc[0].bgRop = 0xCC;
        state.multiSrc[0].srcSurface = *srcSurf;
        if (SrcRectCount)
        {
            state.multiSrc[0].srcRect = state.multiSrc[0].clipRect = state.dstClipRect = tmpRect;
        }
        else
        {
            state.multiSrc[0].srcRect = *srcRect;
            state.multiSrc[0].clipRect = state.dstClipRect = tmpRect;
        }

        gcmONERROR(gcoHARDWARE_StartDE(
            Hardware,
            &state,
            gcv2D_BLT,
            0,
            gcvNULL,
            1,
            &tmpRect
            ));

        gcoHAL_Commit(gcvNULL, gcvFALSE);

        /* Blit tmp surface to dst surface last. */
        gcoOS_MemCopy(&state, State, sizeof(state));

        state.multiSrc[State->currentSrcIndex].srcSurface = *(Hardware->blitTmpSurf);
        if (!SrcRectCount)
        {
            state.multiSrc[State->currentSrcIndex].srcRect = tmpRect;
        }

        gcmONERROR(gcoHARDWARE_StartDE(
            Hardware,
            &state,
            gcv2D_BLT,
            SrcRectCount,
            SrcRect,
            RectCount,
            Rect
            ));
    }
    else
    {
        /* Do original blit. */
        gcmONERROR(gcoHARDWARE_StartDE(
            Hardware,
            State,
            gcv2D_BLT,
            SrcRectCount,
            SrcRect,
            RectCount,
            Rect
            ));
    }

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}
#endif  /* gcdENABLE_2D */

