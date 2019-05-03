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
    "Case gal2DBenchmarkStretchBlit : 2D StretchBlit performance benchmark.\n"\
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

typedef struct Test2D
{
    GalTest         base;
    GalRuntime      *runtime;
    gctINT          count;

    // case type
    gctUINT32       loop;
    char*           srcFile;
    SrcFileFormat   srcFileFormat;


    // tmp surface for performance test destination surface
    gcoSURF         tmpSurf;
    gceSURF_FORMAT  tmpFormat;
    gctUINT         tmpWidth;
    gctUINT         tmpHeight;
    gctUINT         tmpStride[3];
    gctUINT32       tmpPhyAddr[3];
    gctPOINTER      tmpLgcAddr[3];
    gctINT          tmpStrideNum;
    gctINT          tmpAddressNum;

    // source surface
    gcoSURF         srcSurf;
    gceSURF_FORMAT  srcFormat;
    gctUINT         srcWidth;
    gctUINT         srcHeight;
    gctUINT         srcStride[3];
    gctUINT32       srcPhyAddr[3];
    gctPOINTER      srcLgcAddr[3];
    gctINT          srcStrideNum;
    gctINT          srcAddressNum;

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
    gceSURF_FORMAT      srcFormat;
    gceSURF_FORMAT      dstFormat;
    gcsRECT             srcResolution;
    gcsRECT             dstResolution;
    gctUINT32           align;
    gctFLOAT            factor;
} AttriComb;

static AttriComb Array[] =
{
    {
     "stretchblit_RGB565_480p",
     gcvSURF_R5G6B5, gcvSURF_R5G6B5,
     {0, 0, 320, 240}, {0, 0, 0, 0},
     64, 2.0
    },
    {
     "stretchblit_RGB565_720p",
     gcvSURF_R5G6B5, gcvSURF_R5G6B5,
     {0, 0, 640, 360}, {0, 0, 0, 0},
     64, 2.0
    },
    {
     "stretchblit_RGB565_1080p",
     gcvSURF_R5G6B5, gcvSURF_R5G6B5,
     {0, 0, 480, 270}, {0, 0, 0, 0},
     64, 4.0
    },
    {
     "stretchblit_RGB565_1080p_same_size",
     gcvSURF_R5G6B5, gcvSURF_R5G6B5,
     {0, 0, 1920, 1080}, {0, 0, 1920, 1080},
     64, 0
    },

    {
     "stretchblit_ARGB8888_480p",
     gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8,
     {0, 0, 320, 240}, {0, 0, 0, 0},
     64, 2.0
    },
    {
     "stretchblit_ARGB8888_720p",
     gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8,
     {0, 0, 640, 360}, {0, 0, 0, 0},
     64, 2.0
    },
    {
     "stretchblit_ARGB8888_1080p",
     gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8,
     {0, 0, 480, 270}, {0, 0, 0, 0},
     64, 4.0
    },
    {
     "stretchblit_ARGB8888_1080p_same_size",
     gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8,
     {0, 0, 1920, 1080}, {0, 0, 1920, 1080},
     64, 0
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
                                    t2d->tmpPhyAddr[0], t2d->tmpStride[0],
                                    format, gcvSURF_0_DEGREE, t2d->tmpWidth,
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

gctBOOL GetSourceFileName(Test2D *t2d, gceSURF_FORMAT format)
{
    gctUINT pos  = 0;

    switch (format)
    {
        case gcvSURF_A8R8G8B8:
            pos = 0;
            t2d->srcFileFormat = BMP;
            break;

        case gcvSURF_R5G6B5:
            pos = 1;
            t2d->srcFileFormat = BMP;
            break;

        case gcvSURF_I420:
            pos = 2;
            t2d->srcFileFormat = YUV;
            break;

        case gcvSURF_YV12:
            pos = 3;
            t2d->srcFileFormat = YUV;
            break;

        case gcvSURF_NV12:
            pos = 4;
            t2d->srcFileFormat = YUV;
            break;

        case gcvSURF_NV21:
            pos = 5;
            t2d->srcFileFormat = YUV;
            break;

        case gcvSURF_NV16:
            pos = 6;
            t2d->srcFileFormat = YUV;
            break;

        case gcvSURF_NV61:
            pos = 7;
            t2d->srcFileFormat = YUV;
            break;

        case gcvSURF_UYVY:
            pos = 8;
            t2d->srcFileFormat = YUV;
            break;

        default:
            return gcvFALSE;
    }

    t2d->srcFile = sSourceFile[pos];

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
    gctUINT32   address[3];
    gctPOINTER  memory[3];
    gctBOOL     bResult = gcvFALSE;
    gctUINT32   align = Array[frameNo].align;

    bResult = GetSourceFileName(t2d, Array[frameNo].srcFormat);
    if (!bResult)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        return gcvFALSE;
    }

    if (t2d->srcSurf != gcvNULL)
    {
        if (t2d->srcLgcAddr[0] != gcvNULL)
        {
            gcmONERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr));
            t2d->srcLgcAddr[0] = gcvNULL;
        }
        gcmONERROR(gcoSURF_Destroy(t2d->srcSurf));
        t2d->srcSurf = gcvNULL;
    }

    // create source surface
    if (t2d->srcFileFormat == YUV)
    {
        gcmONERROR(GalLoadVimgToSurface(t2d->srcFile, &t2d->srcSurf));
        GalOutput(GalOutputType_Log,
                "LoadSourceSurface: load YUV file\n");
    }
    else if (t2d->srcFileFormat == BMP)
    {
        t2d->srcSurf = GalLoadDIB2Surface(t2d->runtime->hal, t2d->srcFile);
        if (!t2d->srcSurf)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            return gcvFALSE;
        }
    }
    else
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        return gcvFALSE;
    }

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->srcSurf,
                                      gcvNULL,
                                      gcvNULL,
                                      &t2d->srcStride[0]));

    gcmONERROR(gcoSURF_GetSize(t2d->srcSurf,
                               &t2d->srcWidth,
                               &t2d->srcHeight,
                               gcvNULL));

    gcmONERROR(gcoSURF_GetFormat(t2d->srcSurf, gcvNULL, &t2d->srcFormat));

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf, address, memory));
    t2d->srcPhyAddr[0]  = address[0];
    t2d->srcLgcAddr[0]  = memory[0];
    t2d->srcStrideNum   = 1;
    t2d->srcAddressNum  = 1;

    if (t2d->srcFileFormat == YUV)
    {
        gcmONERROR(GalQueryUVStride(t2d->srcFormat, t2d->srcStride[0],
                &t2d->srcStride[1], &t2d->srcStride[2]));

        t2d->srcPhyAddr[1] = address[1];
        t2d->srcLgcAddr[1] = memory[1];

        t2d->srcPhyAddr[2] = address[2];
        t2d->srcLgcAddr[2] = memory[2];

        switch (t2d->srcFormat)
        {
        case gcvSURF_YUY2:
        case gcvSURF_UYVY:
            t2d->srcStrideNum = t2d->srcAddressNum = 1;
            break;

        case gcvSURF_I420:
        case gcvSURF_YV12:
            t2d->srcStrideNum = t2d->srcAddressNum = 3;
            break;

        case gcvSURF_NV16:
        case gcvSURF_NV12:
        case gcvSURF_NV61:
        case gcvSURF_NV21:
            t2d->srcStrideNum = t2d->srcAddressNum = 2;
            break;

        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }

    if (align && t2d->srcAddressNum == 1)
    {
        //first align to 64 bytes
        t2d->srcPhyAddr[0] = (t2d->srcPhyAddr[0] + 63) & ~63;
        if (align != 64)
        {
            t2d->srcPhyAddr[0] = t2d->srcPhyAddr[0] + align;
        }

        gcmASSERT(!(t2d->srcPhyAddr[0] & (align - 1)));
        gcmASSERT(!(t2d->srcStride[0] & (align - 1)));
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
    gctUINT32       address[3];
    gctPOINTER      memory[3];

    align = Array[frameNo].align;
    width = Array[frameNo].dstResolution.right - Array[frameNo].dstResolution.left;
    height = Array[frameNo].dstResolution.bottom - Array[frameNo].dstResolution.top;
    format = Array[frameNo].dstFormat;

    if (t2d->tmpSurf != gcvNULL)
    {
        if (t2d->tmpLgcAddr[0])
        {
            gcmONERROR(gcoSURF_Unlock(t2d->tmpSurf, t2d->tmpLgcAddr));
            t2d->tmpLgcAddr[0] = 0;
        }

        gcmONERROR(gcoSURF_Destroy(t2d->tmpSurf));
        t2d->tmpSurf = gcvNULL;
    }

    // Create tmp surface1
    gcmONERROR(gcoSURF_Construct(t2d->runtime->hal,
                                 width > height ? width : height,
                                 height > width ? height : width,
                                 1,
                                 gcvSURF_BITMAP,
                                 format,
                                 gcvPOOL_DEFAULT,
                                 &t2d->tmpSurf));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->tmpSurf,
                                      gcvNULL,
                                      gcvNULL,
                                      &t2d->tmpStride[0]));

    gcmONERROR(gcoSURF_GetSize(t2d->tmpSurf,
                               &t2d->tmpWidth,
                               &t2d->tmpHeight,
                               gcvNULL));

    gcmONERROR(gcoSURF_Lock(t2d->tmpSurf, address, memory));

    t2d->tmpPhyAddr[0]  = address[0];
    t2d->tmpLgcAddr[0]  = memory[0];
    t2d->tmpStrideNum   = 1;
    t2d->tmpAddressNum  = 1;

    if (format == gcvSURF_UYVY)
    {
        gcmONERROR(GalQueryUVStride(format, t2d->tmpStride[0],
                &t2d->tmpStride[1], &t2d->tmpStride[2]));

        t2d->tmpPhyAddr[1] = address[1];
        t2d->tmpLgcAddr[1] = memory[1];

        t2d->tmpPhyAddr[2] = address[2];
        t2d->tmpLgcAddr[2] = memory[2];

        switch (format)
        {
        case gcvSURF_YUY2:
        case gcvSURF_UYVY:
            t2d->tmpStrideNum = t2d->tmpAddressNum = 1;
            break;

        case gcvSURF_I420:
        case gcvSURF_YV12:
            t2d->tmpStrideNum = t2d->tmpAddressNum = 3;
            break;

        case gcvSURF_NV16:
        case gcvSURF_NV12:
        case gcvSURF_NV61:
        case gcvSURF_NV21:
            t2d->tmpStrideNum = t2d->tmpAddressNum = 2;
            break;

        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }

    if (align && t2d->tmpAddressNum == 1)
    {
        //first align to 64 bytes
        t2d->tmpPhyAddr[0] = (t2d->tmpPhyAddr[0] + 63) & ~63;
        if (align != 64)
        {
            t2d->tmpPhyAddr[0] = t2d->tmpPhyAddr[0] + align;
        }

        gcmASSERT(!(t2d->tmpPhyAddr[0] & (align - 1)));
        gcmASSERT(!(t2d->tmpStride[0] & (align - 1)));
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
    gctUINT     i = 0;
    gctUINT32   align;
    gctUINT32   pixelCount;
    gctUINT64   start, end;
    gctFLOAT    pixelsPerSec = 0.0, factor;
    gceSURF_FORMAT  format;
    gctUINT32   horFactor, verFactor;

    factor = Array[frameNo].factor;
    align = Array[frameNo].align;
    format = Array[frameNo].dstFormat;
    srcRect = Array[frameNo].srcResolution;

    if (factor == 0)
    {
        tmpRect = Array[frameNo].dstResolution;
    }
    else
    {
        tmpRect.left = srcRect.left;
        tmpRect.top = srcRect.top;
        tmpRect.right =
            tmpRect.left + (gctFLOAT)(srcRect.right - srcRect.left) * factor;
        tmpRect.bottom =
            tmpRect.top + (gctFLOAT)(srcRect.bottom - srcRect.top) * factor;
    }
    Array[frameNo].dstResolution = tmpRect;

    // Create source surface
    gcmONERROR(LoadSourceSurfaceFromFile(t2d, frameNo));
    // Create tmp surface
    gcmONERROR(CreateTmpSurface(t2d, frameNo));

    gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

    gcmONERROR(gco2D_SetClipping(egn2D, &tmpRect));

    /* Calculate the stretch factors. */
    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srcRect.right - srcRect.left,
        tmpRect.right - tmpRect.left, &horFactor));

    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srcRect.bottom - srcRect.top,
        tmpRect.bottom - tmpRect.top, &verFactor));

    /* Program the stretch factors. */
    gcmONERROR(gco2D_SetStretchFactors(egn2D, horFactor, verFactor));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        t2d->tmpPhyAddr,
        t2d->tmpAddressNum,
        t2d->tmpStride,
        t2d->tmpStrideNum,
        gcvLINEAR,
        format,
        gcvSURF_0_DEGREE,
        t2d->tmpWidth,
        t2d->tmpHeight));

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        t2d->srcPhyAddr,
        t2d->srcAddressNum,
        t2d->srcStride,
        t2d->srcStrideNum,
        gcvLINEAR,
        t2d->srcFormat,
        gcvSURF_0_DEGREE,
        t2d->srcWidth,
        t2d->srcHeight));

    start = gcoOS_GetTicks();

    gcmONERROR(gco2D_SetSource(egn2D, &srcRect));

    for (i=0; i<count; i++)
    {
        gcmONERROR(gco2D_StretchBlit(egn2D,
                                     1,
                                     &tmpRect,
                                     0xCC,
                                     0xCC,
                                     format));
    }
    gcmONERROR(gco2D_Flush(egn2D));
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    end = gcoOS_GetTicks() - start;

    pixelCount      = (tmpRect.right-tmpRect.left) * (tmpRect.bottom-tmpRect.top) * count;
    pixelsPerSec    = (float)((float)pixelCount/(((float)(gctUINT32)end) *1000.0));

    GalOutput(GalOutputType_Perf|GalOutputType_Console,
              "%s\t%s\t%dx%d\t%s\t%dx%d\t%.3f M/s",
               Array[frameNo].itemName,
               GetFormatName(Array[frameNo].srcFormat),
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

    if ((t2d->dstSurf != gcvNULL) && (t2d->dstLgcAddr != gcvNULL))
    {
        if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dstSurf failed:%s\\n", GalStatusString(status));
        }
        t2d->dstLgcAddr = gcvNULL;
    }

    // destroy source surface
    if (t2d->srcSurf != gcvNULL)
    {
        if (t2d->srcLgcAddr[0] != gcvNULL)
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock srcSurf failed:%s\\n", GalStatusString(status));
            }
            t2d->srcLgcAddr[0] = gcvNULL;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->srcSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\\n", GalStatusString(status));
        }
    }

    // destroy source surface
    if (t2d->tmpSurf != gcvNULL)
    {
        if (t2d->tmpLgcAddr[0] != gcvNULL)
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->tmpSurf, t2d->tmpLgcAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock tmpSurf failed:%s\\n", GalStatusString(status));
            }
            t2d->tmpLgcAddr[0] = gcvNULL;
        }
        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->tmpSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\\n", GalStatusString(status));
        }
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
