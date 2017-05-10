/****************************************************************************
*
*    Copyright 2012 - 2017 Vivante Corporation, Santa Clara, California.
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


/*
 *  Feature:    DEC Compression
 *  API:        gco2D_SetSourceTileStatus gco2D_Blit
 *  Check:
*/
#include <galUtil.h>

static const char *sSrcFile[] = {
    "resource/zero2_A1R5G5B5.bmp",
    "resource/zero2_ARGB4.bmp",
    "resource/zero2_ARGB8.bmp",
    "resource/zero2_B4G4R4X4.bmp",
    "resource/zero2_B5G5R5X1.bmp",
    "resource/zero2_B8G8R8X8.bmp",
    "resource/zero2_R4G4B4X4.bmp",
    "resource/zero2_R5G5B5X1.bmp",
    "resource/zero2_R5G6B5.bmp",
    "resource/zero2_R8G8B8X8.bmp",
    "resource/zero2_X1B5G5R5.bmp",
    "resource/zero2_X4B4G4R4.bmp",
    "resource/zero2_X8B8G8R8.bmp",
    "resource/zero2_UYVY_640X480_Linear.vimg",
    "resource/zero2_YUY2_640X480_Linear.vimg",
    "resource/Crew_NV12_1280x720_Linear.vimg",
    "resource/Crew_NV16_1280x720_Linear.vimg",
    "resource/Boston_YV12_640x480_Linear.vimg",
};

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatCompressedDEC400_001 : BitBlit uncompressed surfaces to DEC400 compressed surface with full src rotations and mirrors\n";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    //source surface
    gcoSURF           srcSurf;
    gceSURF_FORMAT    srcFormat;
    gctUINT           srcWidth;
    gctUINT           srcHeight;
    gctINT            srcStride[3];
    gctINT            srcStrideNum;
    gctINT            srcAddressNum;
    gctUINT32         srcPhyAddr[3];
    gctPOINTER        srcLgcAddr[3];

    gctBOOL           compressed;
} Test2D;

static gceSURF_ROTATION sRots[] =
{
    gcvSURF_0_DEGREE,
    gcvSURF_90_DEGREE,
    gcvSURF_180_DEGREE,
    gcvSURF_270_DEGREE,
    gcvSURF_FLIP_X,
    gcvSURF_FLIP_Y,
};

typedef struct _Comb {
    gceSURF_FORMAT  format;
    gceTILING       tiling;
    gce2D_TILE_STATUS_CONFIG tsc;
} Comb;

static Comb sCombUnCom[] =
{
    { gcvSURF_A1R5G5B5,          gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_A4R4G4B4,          gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_X1R5G5B5,          gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_X4R4G4B4,          gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_R5G6B5,            gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_X8R8G8B8,          gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_A8R8G8B8,          gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_A2R10G10B10,       gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_YUY2,              gcvSUPERTILED,           gcv2D_TSC_DISABLE },
    { gcvSURF_UYVY,              gcvSUPERTILED,           gcv2D_TSC_DISABLE },

    { gcvSURF_X8R8G8B8,          gcvTILED_4X8,            gcv2D_TSC_DISABLE },
    { gcvSURF_A8R8G8B8,          gcvTILED_4X8,            gcv2D_TSC_DISABLE },
    { gcvSURF_A2R10G10B10,       gcvTILED_4X8,            gcv2D_TSC_DISABLE },

    { gcvSURF_X8R8G8B8,          gcvTILED_8X4,            gcv2D_TSC_DISABLE },
    { gcvSURF_A8R8G8B8,          gcvTILED_8X4,            gcv2D_TSC_DISABLE },
    { gcvSURF_A2R10G10B10,       gcvTILED_8X4,            gcv2D_TSC_DISABLE },

    { gcvSURF_P010,              gcvTILED_32X4,           gcv2D_TSC_DISABLE },

    { gcvSURF_NV12,              gcvTILED_64X4,           gcv2D_TSC_DISABLE },

    { gcvSURF_A1R5G5B5,    gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_A4R4G4B4,    gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_X1R5G5B5,    gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_X4R4G4B4,    gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_R5G6B5,      gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_X8R8G8B8,    gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_A8R8G8B8,    gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_A2R10G10B10, gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_YUY2,        gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_UYVY,        gcvTILED_8X8_XMAJOR,           gcv2D_TSC_DISABLE },

    { gcvSURF_NV12,        gcvTILED_8X8_YMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_P010,        gcvTILED_8X8_YMAJOR,           gcv2D_TSC_DISABLE },
};

static Comb sCombCom[] =
{
    { gcvSURF_A1R5G5B5,          gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A4R4G4B4,          gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_X1R5G5B5,          gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_X4R4G4B4,          gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_R5G6B5,            gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_X8R8G8B8,          gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A8R8G8B8,          gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A2R10G10B10,       gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_YUY2,              gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_UYVY,              gcvSUPERTILED_128B,    gcv2D_TSC_DEC_COMPRESSED },

    { gcvSURF_X8R8G8B8,          gcvTILED_4X8,     gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A8R8G8B8,          gcvTILED_4X8,     gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A2R10G10B10,       gcvTILED_4X8,     gcv2D_TSC_DEC_COMPRESSED },

    { gcvSURF_X8R8G8B8,          gcvTILED_8X4,     gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A8R8G8B8,          gcvTILED_8X4,     gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A2R10G10B10,       gcvTILED_8X4,     gcv2D_TSC_DEC_COMPRESSED },

    { gcvSURF_P010,              gcvTILED_32X4,    gcv2D_TSC_DEC_COMPRESSED },

    { gcvSURF_NV12,              gcvTILED_64X4,    gcv2D_TSC_DEC_COMPRESSED },

    { gcvSURF_A1R5G5B5,    gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A4R4G4B4,    gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_X1R5G5B5,    gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_X4R4G4B4,    gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_R5G6B5,      gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_X8R8G8B8,    gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A8R8G8B8,    gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_A2R10G10B10, gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_YUY2,        gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },
    { gcvSURF_UYVY,        gcvTILED_8X8_XMAJOR,    gcv2D_TSC_DEC_COMPRESSED },

    { gcvSURF_NV12,        gcvTILED_8X8_YMAJOR,           gcv2D_TSC_DISABLE },
    { gcvSURF_P010,        gcvTILED_8X8_YMAJOR,           gcv2D_TSC_DISABLE },
};

static gceSTATUS ReloadSourceSurface(Test2D *t2d, const char * sourcefile)
{
    gceSTATUS status;
    gctUINT32 address[3];
    gctPOINTER memory[3];
    gctSTRING pos = gcvNULL;

    // destroy source surface
    if (t2d->srcSurf != gcvNULL)
    {
        if (t2d->srcLgcAddr[0])
        {
            gcmONERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr));
            t2d->srcLgcAddr[0] = gcvNULL;
        }

        gcmONERROR(gcoSURF_Destroy(t2d->srcSurf));
        t2d->srcSurf = gcvNULL;
    }

    // create source surface
    gcmONERROR(GalStrSearch(sourcefile, ".bmp", &pos));
    if (pos)
    {
        t2d->srcSurf = GalLoadDIB2Surface(t2d->runtime->hal,
            sourcefile);
        if (t2d->srcSurf == NULL)
        {
            gcmONERROR(gcvSTATUS_NOT_FOUND);
        }
    }
    else
    {
        gcmONERROR(GalLoadVimgToSurface(
            sourcefile, &t2d->srcSurf));
    }

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->srcSurf,
                                        &t2d->srcWidth,
                                        &t2d->srcHeight,
                                        t2d->srcStride));

    gcmONERROR(gcoSURF_GetFormat(t2d->srcSurf, gcvNULL, &t2d->srcFormat));

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf, address, memory));

    t2d->srcPhyAddr[0]  = address[0];
    t2d->srcLgcAddr[0]  = memory[0];

    t2d->srcStrideNum = t2d->srcAddressNum = 1;

    if (GalIsYUVFormat(t2d->srcFormat))
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

    return gcvSTATUS_OK;

OnError:
    return status;
}

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status = gcvSTATUS_OK;
    gco2D egn2D = t2d->runtime->engine2d;
    T2D_SURF_PTR surf = gcvNULL;
    T2D_SURF_PTR result = gcvNULL, result2 = gcvNULL;
    gctINT32 len, n, index;
    gctUINT32 horFactor, verFactor;
    gcsRECT srect, rect;

    // create source surface
    gcmONERROR(ReloadSourceSurface(t2d,
        sSrcFile[frameNo % gcmCOUNTOF(sSrcFile)]));

    len = (gcmMIN(t2d->srcWidth, t2d->srcHeight)) >> 1;
    if (len > 320) len = 320;

    index = frameNo % gcmCOUNTOF(sCombCom);

    if (t2d->compressed)
    {
        if ((sCombCom[index].format == gcvSURF_P010 && sCombCom[index].tiling == gcvTILED_32X4) ||
            (sCombCom[index].format == gcvSURF_NV12 && sCombCom[index].tiling == gcvTILED_64X4))
        {
            len = 256;
        }
    }
    else
    {
        if ((sCombUnCom[index].format == gcvSURF_P010 && sCombUnCom[index].tiling == gcvTILED_32X4) ||
            (sCombUnCom[index].format == gcvSURF_NV12 && sCombUnCom[index].tiling == gcvTILED_64X4))
        {
            len = 256;
        }
    }

    srect.left = (t2d->srcWidth - len) / 2 ;
    srect.right = (t2d->srcWidth + len) / 2;
    srect.top = (t2d->srcHeight - len) / 2;
    srect.bottom = (t2d->srcHeight + len) / 2;

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        t2d->compressed ?
        sCombCom[index].format :
        sCombUnCom[index].format,
        t2d->compressed ?
        sCombCom[index].tiling :
        sCombUnCom[index].tiling,
        t2d->compressed ?
        sCombCom[index].tsc :
        sCombUnCom[index].tsc,
        len * 6,
        len * 4,
        &surf));

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        t2d->runtime->format,
        gcvLINEAR,
        gcv2D_TSC_DISABLE,
        surf->width,
        surf->height,
        &result));

    gcmONERROR(gco2D_SetStateU32(egn2D, gcv2D_STATE_XRGB_ENABLE, gcvTRUE));

    /* render the central part of src to surf0. */
    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        surf->address,
        surf->validAddressNum,
        surf->stride,
        surf->validStrideNum,
        surf->tiling,
        surf->format,
        surf->rotation,
        surf->aWidth,
        surf->aHeight));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        surf->tileStatusConfig,
        surf->format,
        0,
        surf->tileStatusAddress
        ));

    if (surf->tileStatusNodeEx[0].address != 0)
    {
        gcmONERROR(gco2D_SetStateArrayU32(
            egn2D,
            gcv2D_STATE_ARRAY_YUV_DST_TILE_STATUS_ADDR,
            &surf->tileStatusNodeEx[0].address,
            1
            ));
    }

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        gcv2D_TSC_DISABLE,
        gcvSURF_UNKNOWN,
        0,
        ~0U
        ));

    for (n = 0; n < 24; ++n)
    {
        gceSURF_ROTATION rot;
        gctBOOL horMirror, verMirror;
        gctINT xx = n % 6;
        gctINT yy = n / 6;

        rect = srect;

        rot = sRots[xx];
        horMirror = yy & 1 ? gcvTRUE : gcvFALSE;
        verMirror = yy & 2 ? gcvTRUE : gcvFALSE;

        if (surf->format == gcvSURF_YUY2 ||
            surf->format == gcvSURF_UYVY)
        {
            rot = horMirror = verMirror = gcvFALSE;
        }

        gcmONERROR(gcsRECT_Rotate(
            &rect,
            gcvSURF_0_DEGREE,
            rot,
            t2d->srcWidth,
            t2d->srcHeight
            ));

        gcmONERROR(gco2D_SetSource(egn2D, &rect));

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            t2d->srcPhyAddr, t2d->srcAddressNum,
            t2d->srcStride, t2d->srcStrideNum,
            gcvLINEAR, t2d->srcFormat,
            rot,
            t2d->srcWidth, t2d->srcHeight));

        gcmONERROR(gco2D_SetBitBlitMirror(
            egn2D,
            horMirror,
            verMirror
            ));

        rect.left = xx * len;
        rect.right = rect.left + len;
        rect.top = yy * len;
        rect.bottom = rect.top + len;

        gcmONERROR(gco2D_SetClipping(egn2D, &rect));

        gcmONERROR(gco2D_Blit(egn2D, 1, &rect, 0xCC, 0xCC, surf->format));
    }


    /* Uncompress surf to result. */
    rect.left = rect.top = 0;
    rect.right = surf->width;
    rect.bottom = surf->height;

    gcmONERROR(gco2D_SetSource(egn2D, &rect));

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        surf->address,
        surf->validAddressNum,
        surf->stride,
        surf->validStrideNum,
        surf->tiling,
        surf->format,
        gcvSURF_0_DEGREE,
        surf->aWidth,
        surf->aHeight));

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        surf->tileStatusConfig,
        surf->tileStatusFormat,
        0,
        surf->tileStatusAddress
        ));

    if (surf->tileStatusNodeEx[0].address != 0)
    {
        gcmONERROR(gco2D_SetStateArrayU32(
            egn2D,
            gcv2D_STATE_ARRAY_YUV_SRC_TILE_STATUS_ADDR,
            &surf->tileStatusNodeEx[0].address,
            1
            ));
    }

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        result->address,
        result->validAddressNum,
        result->stride,
        result->validStrideNum,
        result->tiling,
        result->format,
        result->rotation,
        result->width,
        result->height));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        result->tileStatusConfig,
        result->format,
        0,
        result->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetClipping(egn2D, &rect));

    gcmONERROR(gco2D_SetBitBlitMirror(
        egn2D, gcvFALSE, gcvFALSE
        ));

    gcmONERROR(gco2D_Blit(egn2D, 1, &rect, 0xCC, 0xCC, result->format));

    gcmONERROR(gco2D_SetTargetTileStatus(
                egn2D,
                gcv2D_TSC_DISABLE,
                gcvSURF_UNKNOWN,
                0,
                ~0U
                ));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    ////
    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        gcvSURF_R5G6B5,
        gcvLINEAR,
        gcv2D_TSC_DISABLE,
        480, 320,
        &result2));

    srect.left = srect.top = 0;
    srect.right = result->width;
    srect.bottom = result->height;

    rect.left = rect.top = 0;
    rect.right = result2->width;
    rect.bottom = result2->height;

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        result->address,
        result->validAddressNum,
        result->stride,
        result->validStrideNum,
        result->tiling,
        result->format,
        gcvSURF_0_DEGREE,
        result->aWidth,
        result->aHeight));

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        gcv2D_TSC_DISABLE,
        gcvSURF_UNKNOWN,
        0,
        ~0U
        ));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        gcv2D_TSC_DISABLE,
        gcvSURF_UNKNOWN,
        0,
        ~0U
        ));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        result2->address,
        result2->validAddressNum,
        result2->stride,
        result2->validStrideNum,
        result2->tiling,
        result2->format,
        gcvSURF_0_DEGREE,
        result2->width,
        result2->height));

    gcmONERROR(gco2D_SetSource(egn2D, &srect));
    gcmONERROR(gco2D_SetClipping(egn2D, &rect));

    /* Calculate the stretch factors. */
    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srect.right - srect.left,
            rect.right - rect.left, &horFactor));

    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srect.bottom - srect.top,
            rect.bottom - rect.top, &verFactor));
    gcmONERROR(gco2D_SetStretchFactors(egn2D, horFactor, verFactor));

    gcmONERROR(gco2D_StretchBlit(egn2D, 1, &rect, 0xCC, 0xCC, result2->format));
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    if (t2d->runtime->saveFullName)
    {
        GalSaveTSurfToDIB(result2, t2d->runtime->saveFullName);
    }

    if (result2)
    {
        GalDeleteTSurf(gcvNULL, result2);
    }

OnError:

    if (result)
    {
        GalDeleteTSurf(gcvNULL, result);
    }

    if (surf)
    {
        GalDeleteTSurf(gcvNULL, surf);
    }

    if (t2d->srcSurf != gcvNULL)
    {
        if (t2d->srcLgcAddr[0])
        {
            if (gcmIS_ERROR((gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr))))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dstSurf failed:%s\n", GalStatusString(status));
            }
            t2d->srcLgcAddr[0] = gcvNULL;
        }

        if (gcmIS_ERROR((gcoSURF_Destroy(t2d->srcSurf))))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dstSurf failed:%s\n", GalStatusString(status));
        }

        t2d->srcSurf = gcvNULL;
    }

    if (status != gcvSTATUS_OK)
    {
        GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));

        return gcvFALSE;
    }
    else
    {
        return gcvTRUE;
    }
}

static void CDECL Destroy(Test2D *t2d)
{
    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_DEC400_COMPRESSION,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 argc = runtime->argc;
    gctSTRING *argv = runtime->argv;
    gctUINT32 k, listLen = sizeof(FeatureList)/sizeof(gctINT);
    gctBOOL featureStatus;
    char featureName[FEATURE_NAME_LEN], featureMsg[FEATURE_MSG_LEN];

    runtime->wholeDescription = (char*)malloc(FEATURE_NAME_LEN * listLen + strlen(s_CaseDescription) + 1);

    if (runtime->wholeDescription == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    memcpy(runtime->wholeDescription, s_CaseDescription, strlen(s_CaseDescription) + 1);

    for(k = 0; k < listLen; k++)
    {
        gcmONERROR(GalQueryFeatureStr(FeatureList[k], featureName, featureMsg, &featureStatus));
        if (gcoHAL_IsFeatureAvailable(runtime->hal, FeatureList[k]) == featureStatus)
        {
            GalOutput(GalOutputType_Result | GalOutputType_Console, "%s is not supported.\n", featureMsg);
            runtime->notSupport = gcvTRUE;
        }
        strncat(runtime->wholeDescription, featureName, k==listLen-1 ? strlen(featureName)+1:strlen(featureName));
    }

    if (runtime->notSupport)
        return gcvFALSE;

    t2d->compressed = gcvTRUE;
    for (k = 0; k < argc; ++k)
    {
        if (!strcmp(argv[k], "-compressed"))
        {
            t2d->compressed = gcvTRUE;
        }
    }

    t2d->runtime = runtime;
    runtime->saveTarget = gcvFALSE;
    runtime->cleanTarget = gcvFALSE;
    t2d->srcSurf = gcvNULL;

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = gcmCOUNTOF(sCombCom);
    t2d->base.description = s_CaseDescription;

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
    return gcvFALSE;
}

GalTest * CDECL GalCreateTestObject(GalRuntime *runtime)
{
    Test2D *t2d = (Test2D *)malloc(sizeof(Test2D));

    if (!Init(t2d, runtime)) {
        free(t2d);
        return NULL;
    }

    return &t2d->base;
}
