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


/*
 *  Feature:    multi, super, tiled input
 *  API:        gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_MultiSourceBlit
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatCompressedTPC015\n" \
"Operation: multi-src blit/rotation with alpha: all TPC compressed and argb & yuv to uncompressed and argb.\n" \
"2D API: gco2D_SetSourceTileStatus gco2D_SetTargetTileStatus gco2D_SetGenericSource gco2D_SetBitBlitMirror gco2D_MultiSourceBlit\n" \
"Src1: Size       [640x640]\n"\
"     Rect        [40,40,200,360]\n"\
"     Format      [ABGR8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [TPC compressed]\n" \
"Src2: Size       [640x640]\n"\
"     Rect        [40,40,200,360]\n"\
"     Format      [XBGR8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [TPC compressed]\n" \
"Src3: Size       [640x640]\n"\
"     Rect        [40,40,200,360]\n"\
"     Format      [ABGR4444]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [TPC compressed]\n" \
"Src4: Size       [640x640]\n"\
"     Rect        [40,40,200,360]\n"\
"     Format      [BGR565]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [TPC compressed]\n" \
"Src5: Size       [640x640]\n"\
"     Rect        [40,40,200,360]\n"\
"     Format      [ABGR1555]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [TPC compressed]\n" \
"Src6: Size       [640x640]\n"\
"     Rect        [40,40,200,360]\n"\
"     Format      [RG16]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [TPC compressed]\n" \
"Src7: Size       [640x640]\n"\
"     Rect        [40,40,200,360]\n"\
"     Format      [R8]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [TPC compressed]\n" \
"Src8: Size       [640x640]\n"\
"     Rect        [40,40,200,360]\n"\
"     Format      [ABGR8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [TPC compressed]\n" \
"Dst: Size        [640x640]\n"\
"     Rect        [variable]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: Type   [SolidBrush]\n"\
"       Format [ARGB8888]\n"\
"       Offset [0]\n" \
"Alphablend: [disable/enable]\n" \
"HW feature dependency: ";

#define INT2BOOL(i,d) ((i)%(d) == 0)?gcvFALSE:gcvTRUE

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    //source surface
    T2D_SURF_PTR        surf[10];

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

static gcsRECT sDstRect[] =
{
    {0,     0, 320, 320},
    {160,   0, 320, 320},
    {320,   0, 480, 320},
    {480,   0, 640, 320},
    {0,   320, 320, 640},
    {160, 320, 320, 640},
    {320, 320, 480, 640},
    {480, 320, 640, 640},
};

static gceSURF_FORMAT sFormat[] =
{
    gcvSURF_A8B8G8R8,
    gcvSURF_X8B8G8R8,
    gcvSURF_A4B4G4R4,
    gcvSURF_B5G6R5,
    gcvSURF_A1B5G5R5,
    gcvSURF_RG16,
    gcvSURF_R8,
};

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gco2D egn2D = t2d->runtime->engine2d;
    gcsRECT clipRect, srect;
    gctINT i;

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal, gcvSURF_A8R8G8B8, gcvLINEAR, gcv2D_TSC_DISABLE,
        640, 640, &t2d->surf[8]));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        t2d->surf[8]->address,
        t2d->surf[8]->validAddressNum,
        t2d->surf[8]->stride,
        t2d->surf[8]->validStrideNum,
        t2d->surf[8]->tiling,
        t2d->surf[8]->format,
        t2d->surf[8]->rotation,
        t2d->surf[8]->aWidth,
        t2d->surf[8]->aHeight));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        t2d->surf[8]->tileStatusConfig,
        t2d->surf[8]->format,
        gcvSURF_0_DEGREE,
        t2d->surf[8]->tileStatusAddress
        ));

    srect.left = srect.top = 40;
    srect.right = srect.left + 160;
    srect.bottom = srect.top + 320;

    clipRect.left = clipRect.top = 0;
    clipRect.right = 6400;
    clipRect.bottom = 6400;

    // Compressed to Uncompressed
    for (i = 0; i < 8; i++)
    {
        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        gcmONERROR(gco2D_SetClipping(egn2D, &clipRect));

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            t2d->surf[i]->address, t2d->surf[i]->validAddressNum,
            t2d->surf[i]->stride, t2d->surf[i]->validStrideNum,
            t2d->surf[i]->tiling, t2d->surf[i]->format,
            sRots[(frameNo + i) % 6],
            t2d->surf[i]->aWidth, t2d->surf[i]->aHeight));

        gcmONERROR(gco2D_SetSourceTileStatus(
            egn2D,
            t2d->surf[i]->tileStatusConfig,
            t2d->surf[i]->tileStatusFormat,
            t2d->surf[i]->tileStatusClear,
            t2d->surf[i]->tileStatusAddress
            ));

        gcmONERROR(gco2D_SetSource(egn2D, &srect));

        gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));

        gcmONERROR(gco2D_SetBitBlitMirror(egn2D, INT2BOOL((frameNo + i), 2),INT2BOOL((frameNo + i), 4)));

        gcmONERROR(gco2D_SetPorterDuffBlending(
                egn2D,
                gcvPD_SRC_OVER));

        gcmONERROR(gco2D_SetTargetRect(egn2D, &sDstRect[i]));
    }

    gcmONERROR(gco2D_MultiSourceBlit(egn2D, 0xFF, gcvNULL, 0));

    gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

    gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvFALSE));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));


    if (t2d->runtime->saveFullName)
    {
        GalSaveTSurfToDIB(t2d->surf[8], t2d->runtime->saveFullName);
    }

    gcmONERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->surf[8]));

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console,
    "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
    return gcvFALSE;
}

static void CDECL Destroy(Test2D *t2d)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i;

    // destroy source surface
    for (i = 0; i < 8; ++i)
    {
        if (t2d->surf[i] != gcvNULL)
        {
            if (gcmIS_ERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->surf[i])))
           {
               GalOutput(GalOutputType_Error | GalOutputType_Console,
               "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
           }
            t2d->surf[i] = gcvNULL;
        }
    }

    free(t2d);
}

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

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_TPC_COMPRESSION,
    gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status;
    gctINT i;
    gcsRECT tmpRect, clipRect;
    gctSTRING sourcefile[] =
    {
        "resource/zero2_YUY2_640X480_Linear.vimg",
        "resource/Crew_NV12_1280x720_Linear.vimg",
        "resource/Crew_NV61_1280x720_Linear.vimg",
        "resource/Boston_YV12_640x480_Linear.vimg",
        "resource/index8_argb.bmp",
        "resource/zero2_R5G6B5.bmp",
        "resource/source.bmp",
        "resource/zero2_A1R5G5B5.bmp",
    };

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

    t2d->runtime = runtime;
    runtime->saveTarget = gcvFALSE;
    runtime->cleanTarget = gcvFALSE;

    // create source surface
    for (i = 0; i < 8; ++i)
    {
        gcsRECT srect, rect;

        gcmONERROR(ReloadSourceSurface(
            t2d, sourcefile[i]));

        gcmONERROR(GalCreateTSurf(
            t2d->runtime->hal,
            sFormat[i % gcmCOUNTOF(sFormat)],
            gcvLINEAR,
            gcv2D_TSC_TPC_COMPRESSED_V10,
            640, 640, &t2d->surf[i]));

        gcmONERROR(gco2D_SetGenericTarget(
            t2d->runtime->engine2d,
            t2d->surf[i]->address,
            t2d->surf[i]->validAddressNum,
            t2d->surf[i]->stride,
            t2d->surf[i]->validStrideNum,
            t2d->surf[i]->tiling,
            t2d->surf[i]->format,
            t2d->surf[i]->rotation,
            t2d->surf[i]->aWidth,
            t2d->surf[i]->aHeight));

        gcmONERROR(gco2D_SetTargetTileStatus(
            t2d->runtime->engine2d,
            t2d->surf[i]->tileStatusConfig,
            t2d->surf[i]->format,
            gcvSURF_0_DEGREE,
            t2d->surf[i]->tileStatusAddress
            ));

        gcmONERROR(gco2D_SetGenericSource(
            t2d->runtime->engine2d,
            t2d->srcPhyAddr, t2d->srcAddressNum,
            t2d->srcStride, t2d->srcStrideNum,
            gcvLINEAR, t2d->srcFormat,
            gcvSURF_0_DEGREE,
            t2d->srcWidth, t2d->srcHeight));

        gcmONERROR(gco2D_SetSourceTileStatus(
            t2d->runtime->engine2d,
            gcv2D_TSC_DISABLE,
            gcvSURF_UNKNOWN,
            0,
            ~0U
            ));

        if (t2d->surf[i]->tileStatusConfig != gcv2D_TSC_DISABLE)
        {
            // Need to clear TPC header buffer.
            tmpRect.left = tmpRect.top = 0;
            tmpRect.right = t2d->surf[i]->aWidth;
            tmpRect.bottom = t2d->surf[i]->aHeight;

            clipRect.left = clipRect.top = 0;
            clipRect.right = 6400;
            clipRect.bottom = 6400;

            gcmONERROR(gco2D_SetClipping(t2d->runtime->engine2d, &clipRect));

            gcmONERROR(gco2D_LoadSolidBrush(
                t2d->runtime->engine2d, t2d->surf[i]->format, gcvTRUE, COLOR_ARGB8(0, 0, 0, 0), 0));
            gcmONERROR(gco2D_Clear(
                t2d->runtime->engine2d, 1, &tmpRect, COLOR_ARGB8(0, 0, 0, 0), 0xF0, 0xF0, t2d->surf[i]->format));
            gcmONERROR(gcoHAL_Commit(gcvNULL, gcvTRUE));
        }

        srect.left = 0;
        srect.top  = 0;
        srect.right  = t2d->srcWidth;
        srect.bottom = t2d->srcHeight;

        rect.left = 0;
        rect.top  = 0;
        rect.right  = t2d->surf[i]->aWidth;
        rect.bottom = t2d->surf[i]->aHeight;

        gcmONERROR(gco2D_SetClipping(t2d->runtime->engine2d, &rect));

        gcmONERROR(gco2D_SetKernelSize(t2d->runtime->engine2d, 3, 3));

        gcmONERROR(gco2D_FilterBlitEx2(t2d->runtime->engine2d,
            t2d->srcPhyAddr, t2d->srcAddressNum,
            t2d->srcStride, t2d->srcStrideNum,
            gcvLINEAR, t2d->srcFormat, gcvSURF_0_DEGREE, t2d->srcWidth, t2d->srcHeight, &srect,
            t2d->surf[i]->address, t2d->surf[i]->validAddressNum,
            t2d->surf[i]->stride, t2d->surf[i]->validStrideNum,
            t2d->surf[i]->tiling, t2d->surf[i]->format, t2d->surf[i]->rotation,
            t2d->surf[i]->aWidth, t2d->surf[i]->aHeight,
            &rect, gcvNULL));


        gcmONERROR(gcoHAL_Commit(gcvNULL, gcvTRUE));

        if (t2d->srcSurf != gcvNULL)
        {
            if (t2d->srcLgcAddr[0])
            {
                if (gcmIS_ERROR((gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr))))
                {
                    GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock srcSurf failed:%s\n", GalStatusString(status));
                }
                t2d->srcLgcAddr[0] = gcvNULL;
            }

            if (gcmIS_ERROR((gcoSURF_Destroy(t2d->srcSurf))))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy srcSurf failed:%s\n", GalStatusString(status));
            }

            t2d->srcSurf = gcvNULL;
        }
    }

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = 8;
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

    memset(t2d, 0, sizeof(Test2D));

    if (!Init(t2d, runtime)) {
        free(t2d);
        return NULL;
    }

    return &t2d->base;
}
