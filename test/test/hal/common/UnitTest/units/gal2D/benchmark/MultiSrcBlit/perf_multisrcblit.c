/****************************************************************************
*
*    Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


#include <galUtil.h>
#include <stdio.h>
#include <string.h>


static gctCONST_STRING s_CaseDescription =
    "Case gal2DBenchmarkMultiSrcBlit : 2D MultiSrcBlit performance benchmark.\n"\
    "args \"-loop\" : loop number.\n";

typedef enum _PoolType
{
    BOTH= 0,
    VIRTONLY,
    SYSTEMONLY,
}PoolType;

static char *sSourceFile[] = {
    "resource/FilterBlit_1920x1920_A8R8G8B8.bmp",
    "resource/FilterBlit_1920x1920_R5G6B5.bmp",
};

typedef enum _SrcFileFormat
{
    YUV = 0,
    BMP,
    UNKNOWN_Format
}
SrcFileFormat;

#define SRCNUM 4

typedef struct Test2D
{
    GalTest         base;
    GalRuntime      *runtime;
    gctINT          count;

    // case type
    gctUINT32       loop;
    char*           srcFile[SRCNUM];
    SrcFileFormat   srcFileFormat[SRCNUM];

    T2D_SURF_PTR    srcSurf[SRCNUM];
    T2D_SURF_PTR    tmpSurf;

    // destination surface
    gcoSURF         dstSurf;
    gceSURF_FORMAT  dstFormat;
    gctUINT         dstWidth;
    gctUINT         dstHeight;
    gctINT          dstStride;
    gctUINT32       dstPhyAddr;
    gctPOINTER      dstLgcAddr;
}
Test2D;


typedef struct _AttriComb
{
    char*               itemName;
    gctUINT32           srcNum;
    gceSURF_FORMAT      srcFormat[SRCNUM];
    gceSURF_FORMAT      dstFormat;
    gcsRECT             srcResolution;
    gcsRECT             dstResolution;
    gctUINT32           align;
} AttriComb;

static AttriComb Array[] =
{
    {
     "multisrc_4src_RGB565_480p",
     4,
     {gcvSURF_R5G6B5, gcvSURF_R5G6B5, gcvSURF_R5G6B5, gcvSURF_R5G6B5},
     gcvSURF_R5G6B5,
     {0, 0, 640, 480}, {0, 0, 640, 480},
     64,
    },
    {
     "multisrc_4src_RGB565_720p",
     4,
     {gcvSURF_R5G6B5, gcvSURF_R5G6B5, gcvSURF_R5G6B5, gcvSURF_R5G6B5},
     gcvSURF_R5G6B5,
     {0, 0, 1280, 720}, {0, 0, 1280, 720},
     64,
    },
    {
     "multisrc_4src_RGB565_1080p",
     4,
     {gcvSURF_R5G6B5, gcvSURF_R5G6B5, gcvSURF_R5G6B5, gcvSURF_R5G6B5},
     gcvSURF_R5G6B5,
     {0, 0, 1920, 1080}, {0, 0, 1920, 1080},
     64,
    },

    {
     "multisrc_4src_ARGB8888_480p",
     4,
     {gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8},
     gcvSURF_A8R8G8B8,
     {0, 0, 640, 480}, {0, 0, 640, 480},
     64,
    },
    {
     "multisrc_4src_ARGB8888_720p",
     4,
     {gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8},
     gcvSURF_A8R8G8B8,
     {0, 0, 1280, 720}, {0, 0, 1280, 720},
     64,
    },
    {
     "multisrc_4src_ARGB8888_1080p",
     4,
     {gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8},
     gcvSURF_A8R8G8B8,
     {0, 0, 1920, 1080}, {0, 0, 1920, 1080},
     64,
    },
};


gctBOOL CopyTmpToDstSurf(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS   status;
    gco2D       egn2D   = t2d->runtime->engine2d;
    gcsRECT     dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
    gcsRECT     tmpRect = Array[frameNo].dstResolution;
    gctUINT32   horFactor, verFactor;
    gceSURF_FORMAT format = Array[frameNo].dstFormat;

    // set source color and rect
    gcmONERROR(gco2D_SetColorSource(egn2D,
                                    t2d->tmpSurf->address[0], t2d->tmpSurf->stride[0],
                                    format, gcvSURF_0_DEGREE, t2d->tmpSurf->width,
                                    gcvFALSE, gcvSURF_OPAQUE, 0));

    gcmONERROR(gco2D_SetSource(egn2D, &tmpRect));

    // set dst and clippint rect
    gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth));

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    /* Calculate the stretch factors. */
    gcmONERROR(gco2D_CalcStretchFactor(egn2D, tmpRect.right - tmpRect.left,
        dstRect.right - dstRect.left, &horFactor));

    gcmONERROR(gco2D_CalcStretchFactor(egn2D, tmpRect.bottom - tmpRect.top,
        dstRect.bottom - dstRect.top, &verFactor));

    /* Program the stretch factors. */
    gcmONERROR(gco2D_SetStretchFactors(egn2D, horFactor, verFactor));

    gcmONERROR(gco2D_StretchBlit(egn2D, 1, &dstRect, 0xCC, 0xCC, t2d->dstFormat));

    gcmONERROR(gco2D_Flush(egn2D));
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    return gcvTRUE;

OnError:

    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));

    return gcvFALSE;
}

gctBOOL GetSourceFileName(Test2D *t2d, gceSURF_FORMAT format, gctUINT32 index)
{
    gctUINT pos  = 0;

    switch (format)
    {
        case gcvSURF_A8R8G8B8:
            pos = 0;
            t2d->srcFileFormat[index] = BMP;
            break;

        case gcvSURF_R5G6B5:
            pos = 1;
            t2d->srcFileFormat[index] = BMP;
            break;

        case gcvSURF_I420:
            pos = 2;
            t2d->srcFileFormat[index] = YUV;
            break;

        case gcvSURF_YV12:
            pos = 3;
            t2d->srcFileFormat[index] = YUV;
            break;

        case gcvSURF_NV12:
            pos = 4;
            t2d->srcFileFormat[index] = YUV;
            break;

        case gcvSURF_NV21:
            pos = 5;
            t2d->srcFileFormat[index] = YUV;
            break;

        case gcvSURF_NV16:
            pos = 6;
            t2d->srcFileFormat[index] = YUV;
            break;

        case gcvSURF_NV61:
            pos = 7;
            t2d->srcFileFormat[index] = YUV;
            break;

        case gcvSURF_UYVY:
            pos = 8;
            t2d->srcFileFormat[index] = YUV;
            break;

        default:
            return gcvFALSE;
    }

    t2d->srcFile[index] = sSourceFile[pos];

    GalOutput(GalOutputType_Log,
            "source file: %s\n", sSourceFile[pos]);

    return gcvTRUE;
}

static char* GetFormatName(gceSURF_FORMAT format)
{
    char* f;

    switch(format)
    {
        case gcvSURF_A8R8G8B8:
            f = "A8R8B8G8";
            break;

       case gcvSURF_A1R5G5B5:
            f = "A1R5G5B5";
            break;

       case gcvSURF_A4R4G4B4:
            f = "A4R4G4B4";
            break;

       case gcvSURF_B4G4R4A4:
            f = "B4G4R4A4";
            break;

        case gcvSURF_R5G6B5:
            f = "R5G6B5";
            break;

        case gcvSURF_YUY2:
            f = "YUY2";
            break;

        case gcvSURF_UYVY:
            f = "UYVY";
            break;

        case gcvSURF_I420:
            f = "YUV420";
            break;

        case gcvSURF_YV12:
            f = "YV12";
            break;

        case gcvSURF_NV16:
            f = "NV16";
            break;

        case gcvSURF_NV12:
            f = "NV12";
            break;

        case gcvSURF_NV61:
            f = "NV61";
            break;

        case gcvSURF_NV21:
            f = "NV21";
            break;

        default:
            f = "errorformat";
            break;
    }

    return f;
}

gceSTATUS LoadSourceSurfaceFromFile(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS   status;
    gctUINT32   i;
    gctBOOL     bResult = gcvFALSE;
    gctUINT32   align = Array[frameNo].align;

    for (i=0; i<Array[frameNo].srcNum; i++)
    {
        bResult = GetSourceFileName(t2d,
                                    Array[frameNo].srcFormat[i],
                                    i);
        if (!bResult)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            return gcvFALSE;
        }

        if (t2d->srcSurf[i] != gcvNULL)
        {
            GalDeleteTSurf(gcvNULL, t2d->srcSurf[i]);
            t2d->srcSurf[i] = gcvNULL;
        }

        // create source surface
        gcmONERROR(GalLoadFileToTSurf(
            t2d->srcFile[i], &t2d->srcSurf[i]));

        if (!t2d->srcSurf[i])
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            return gcvFALSE;
        }

        if (align && t2d->srcSurf[i]->validAddressNum == 1)
        {
            //first align to 64 bytes
            t2d->srcSurf[i]->address[0] = (t2d->srcSurf[i]->address[0] + 63) & ~63;
            if (align != 64)
            {
                t2d->srcSurf[i]->address[0] = t2d->srcSurf[i]->address[0] + align;
            }

            gcmASSERT(!(t2d->srcSurf[i]->address[0] & (align - 1)));
            gcmASSERT(!(t2d->srcSurf[i]->stride[0] & (align - 1)));
        }
    }

    return gcvSTATUS_OK;

OnError:

    return status;
}

gceSTATUS CreateTmpSurface(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS   status;
    gctUINT     width, height;
    gctUINT32   align;
    gceSURF_FORMAT  format;

    align = Array[frameNo].align;
    width = Array[frameNo].srcResolution.right - Array[frameNo].srcResolution.left;
    height = Array[frameNo].srcResolution.bottom - Array[frameNo].srcResolution.top;
    format = Array[frameNo].dstFormat;

    if (t2d->tmpSurf != gcvNULL)
    {
        GalDeleteTSurf(gcvNULL, t2d->tmpSurf);
        t2d->tmpSurf = gcvNULL;
    }

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        format,
        gcvLINEAR,
        gcv2D_TSC_DISABLE,
        width > height ? width : height,
        height > width ? height : width,
        &t2d->tmpSurf));

    if (align && t2d->tmpSurf->validAddressNum == 1)
    {
        //first align to 64 bytes
        t2d->tmpSurf->address[0] = (t2d->tmpSurf->address[0] + 63) & ~63;
        if (align != 64)
        {
            t2d->tmpSurf->address[0] = t2d->tmpSurf->address[0] + align;
        }

        gcmASSERT(!(t2d->tmpSurf->address[0] & (align - 1)));
        gcmASSERT(!(t2d->tmpSurf->stride[0] & (align - 1)));
    }

    return gcvSTATUS_OK;

OnError:

    return status;
}

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS   status;
    gcsRECT     tmpRect, srcRect;
    gco2D       egn2D = t2d->runtime->engine2d;
    gctUINT     count = t2d->loop;
    gctUINT     i = 0, flag=0x1;
    gctUINT32   align, srcNum;
    gctUINT32   pixelCount;
    gctUINT64   start, end;
    gctFLOAT    pixelsPerSec = 0.0;
    gceSURF_FORMAT  format;
    gctUINT     noMulEx = 1;

    if (gcoHAL_IsFeatureAvailable(t2d->runtime->hal, gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2))
    {
        noMulEx = 0;
    }

    align = Array[frameNo].align;
    srcNum = Array[frameNo].srcNum;
    format = Array[frameNo].dstFormat;
    srcRect = Array[frameNo].srcResolution;
    tmpRect = Array[frameNo].dstResolution;

    // Create source surface
    gcmONERROR(LoadSourceSurfaceFromFile(t2d, frameNo));
    // Create tmp surface
    gcmONERROR(CreateTmpSurface(t2d, frameNo));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        t2d->tmpSurf->address, 1,
        t2d->tmpSurf->stride, 1,
        gcvLINEAR,
        format,
        gcvSURF_0_DEGREE,
        t2d->tmpSurf->width,
        t2d->tmpSurf->height));

    for (i=0; i<srcNum; i++)
    {
        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        gcmONERROR(gco2D_SetClipping(egn2D, &tmpRect));

        gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            t2d->srcSurf[i]->address,
            t2d->srcSurf[i]->validAddressNum,
            t2d->srcSurf[i]->stride,
            t2d->srcSurf[i]->validStrideNum,
            t2d->srcSurf[i]->tiling,
            t2d->srcSurf[i]->format,
            gcvSURF_0_DEGREE,
            t2d->srcSurf[i]->width,
            t2d->srcSurf[i]->height));

        gcmONERROR(gco2D_SetSource(egn2D, &srcRect));
        gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));

        if (!noMulEx)
        {
            gcmONERROR(gco2D_SetTargetRect(egn2D, &tmpRect));
        }

        flag |= 0x1 << i;
    }

    start = gcoOS_GetTicks();

    for (i=0; i<count; i++)
    {
        gcmONERROR(gco2D_MultiSourceBlit(egn2D, flag, &tmpRect, noMulEx));
    }

    gcmONERROR(gco2D_Flush(egn2D));
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    end = gcoOS_GetTicks() - start;

    pixelCount      = (tmpRect.right-tmpRect.left) * (tmpRect.bottom-tmpRect.top) * srcNum * count;
    pixelsPerSec    = (float)((float)pixelCount/(((float)(gctUINT32)end) *1000.0));

    GalOutput(GalOutputType_Perf|GalOutputType_Console,
              "%s\t%s\t%dx%d\t%s\t%dx%d\t%.3f M/s",
               Array[frameNo].itemName,
               GetFormatName(Array[frameNo].srcFormat[srcNum-1]),
               srcRect.right - srcRect.left,
               srcRect.bottom - srcRect.top,
               GetFormatName(format),
               tmpRect.right - tmpRect.left,
               tmpRect.bottom - tmpRect.top,
               pixelsPerSec);

    if (t2d->dstSurf != gcvNULL)
    {
        // Copy tmpSurf to dstSurf
        CopyTmpToDstSurf(t2d, frameNo);
    }

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));

    return gcvFALSE;
}

static void CDECL Destroy(Test2D *t2d)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT32 i;

    if ((t2d->dstSurf != gcvNULL) && (t2d->dstLgcAddr != gcvNULL))
    {
        if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dstSurf failed:%s\\n", GalStatusString(status));
        }
        t2d->dstLgcAddr = gcvNULL;
    }

    for (i = 0; i < SRCNUM; i++)
    {
        if (t2d->srcSurf[i] != gcvNULL)
        {
            GalDeleteTSurf(gcvNULL, t2d->srcSurf[i]);
            t2d->srcSurf[i] = gcvNULL;
        }
    }

    if (t2d->tmpSurf != gcvNULL)
    {
        GalDeleteTSurf(gcvNULL, t2d->tmpSurf);
        t2d->tmpSurf = gcvNULL;
    }

    // restore timeout to default time
    if (gcmIS_ERROR(gcoHAL_SetTimeOut(t2d->runtime->hal, 2000)))
        GalOutput(GalOutputType_Error | GalOutputType_Console, "Restore timeout failed:%s\n", GalStatusString(status));

    free(t2d);
}

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i;
    gctINT argc = runtime->argc;
    gctSTRING *argv = runtime->argv;

    if (!gcoHAL_IsFeatureAvailable(runtime->hal, gcvFEATURE_2D_MULTI_SOURCE_BLT) &&
        !gcoHAL_IsFeatureAvailable(runtime->hal, gcvFEATURE_2D_MULTI_SOURCE_BLT_EX) &&
        !gcoHAL_IsFeatureAvailable(runtime->hal, gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT) &&
        !gcoHAL_IsFeatureAvailable(runtime->hal, gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2))
    {
        GalOutput(GalOutputType_Result | GalOutputType_Console, "MultiSrcBlit is not supported.\n");
        runtime->notSupport = gcvTRUE;
        return gcvFALSE;
    }

    if (gcoHAL_IsFeatureAvailable(runtime->hal, gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT))
    {
        gcmONERROR(gco2D_SetStateU32(runtime->engine2d,
                                     gcv2D_STATE_MULTI_SRC_BLIT_UNIFIED_DST_RECT,
                                     gcvTRUE));
    }

    memset(t2d, 0, sizeof(Test2D));

    t2d->runtime    = runtime;
    t2d->dstSurf    = runtime->target;
    t2d->dstFormat  = runtime->format;

    t2d->loop  = 200;

    for (i = 0; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-loop"))
        {
            t2d->loop = atoi(argv[++i]);
        }
    }

    // set timeout to infinite for performance test
    gcmONERROR(gcoHAL_SetTimeOut(t2d->runtime->hal, gcvINFINITE));

    if (t2d->dstSurf != gcvNULL)
    {
        // Get dstSurf size, format and address.
        gcmONERROR(gcoSURF_GetSize(t2d->dstSurf,
                                    &t2d->dstWidth,
                                    &t2d->dstHeight,
                                    gcvNULL));

        gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                            gcvNULL,
                                            gcvNULL,
                                            &t2d->dstStride));

        gcmONERROR(gcoSURF_Lock(t2d->dstSurf,
                                  &t2d->dstPhyAddr,
                                  &t2d->dstLgcAddr));
    }

    t2d->base.frameCount = gcmCOUNTOF(Array);

    t2d->base.render        = (PGalRender) Render;
    t2d->base.destroy       = (PGalDestroy) Destroy;
    t2d->base.description   = s_CaseDescription;

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));

    return gcvFALSE;
}

GalTest * CDECL GalCreateTestObject(GalRuntime *runtime)
{
    Test2D *t2d = (Test2D *) malloc(sizeof(Test2D));

    if (!Init(t2d, runtime))
    {
        free(t2d);
        return NULL;
    }

    return &t2d->base;
}
