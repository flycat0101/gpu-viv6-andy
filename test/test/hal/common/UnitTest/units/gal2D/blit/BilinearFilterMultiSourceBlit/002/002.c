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
 *  Feature:
 *  API:        gco2D_MultiSourceBlit gco2D_SetGenericSource gco2D_SetGenericTarget
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription =
"Case gal2DBilinearFilterMultiSourceBlit002\n" \
"Operation: Bilinear Filter MultiSrcBlit stretch with different non-zero rectagnles.\n" \
"2D API: gco2D_MultiSourceBlit\n" \
"Src1: Size       [640x480]\n"\
"     Rect        [48,58,592,432]\n"\
"     Format      [INDEX8]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src2: Size       [1920x1080]\n"\
"     Rect        [108,118,1812,972]\n"\
"     Format      [UYVY]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src3: Size       [640x480]\n"\
"     Rect        [48,58,592,432]\n"\
"     Format      [RGB565]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src4: Size       [1280x720]\n"\
"     Rect        [72,82,1208,648]\n"\
"     Format      [NV12]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src5: Size       [640x480]\n"\
"     Rect        [48,58,592,432]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src6: Size       [1280x720]\n"\
"     Rect        [72,82,1208,648]\n"\
"     Format      [NV16]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src7: Size       [640x480]\n"\
"     Rect        [48,58,592,432]\n"\
"     Format      [ARGB4444]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src8: Size       [640x480]\n"\
"     Rect        [48,58,592,432]\n"\
"     Format      [YV12]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [640x640]\n"\
"     Rect        [variable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"Alphablend: [enable]\n" \
"HW feature dependency: ";

typedef struct _MultiSrc
{
    gcoSURF         srcSurf;
    gceSURF_FORMAT  srcFormat;
    gctUINT         srcWidth;
    gctUINT         srcHeight;
    gctINT          srcStride[3];
    gctINT          srcStrideNum;
    gctINT          srcAddressNum;
    gctUINT32       srcPhyAddr[3];
    gctPOINTER      srcLgcAddr[3];
} MultiSrc, *MultiSrcPTR;

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    // destination surface
    gcoSURF         dstSurf;
    gceSURF_FORMAT  dstFormat;
    gctUINT         dstWidth;
    gctUINT         dstHeight;
    gctINT          dstStride;
    gctUINT32       dstPhyAddr;
    gctPOINTER      dstLgcAddr;

    //source surface
    MultiSrc multiSrc[8];

    T2D_SURF_PTR    mediat;

    gctUINT8        srcAlpha;
    gctUINT8        dstAlpha;
} Test2D;

static gceSTATUS ReloadSourceSurface(Test2D *t2d, gctUINT SrcIndex, const char * sourcefile)
{
    gceSTATUS status;
    MultiSrcPTR curSrc = &t2d->multiSrc[SrcIndex % 8];
    gctUINT32 address[3];
    gctPOINTER memory[3];
    gctSTRING pos = gcvNULL;

    // destroy source surface
    if (curSrc->srcSurf != gcvNULL)
    {
        if (curSrc->srcLgcAddr[0])
        {
            gcmONERROR(gcoSURF_Unlock(curSrc->srcSurf, curSrc->srcLgcAddr));
            curSrc->srcLgcAddr[0] = gcvNULL;
        }

        gcmONERROR(gcoSURF_Destroy(curSrc->srcSurf));
        curSrc->srcSurf = gcvNULL;
    }

    // create source surface
    gcmONERROR(GalStrSearch(sourcefile, ".bmp", &pos));
    if (pos)
    {
        curSrc->srcSurf = GalLoadDIB2Surface(t2d->runtime->hal,
            sourcefile);
        if (curSrc->srcSurf == NULL)
        {
            gcmONERROR(gcvSTATUS_NOT_FOUND);
        }
    }
    else
    {
        gcmONERROR(GalLoadVimgToSurface(
            sourcefile, &curSrc->srcSurf));
    }

    gcmONERROR(gcoSURF_GetAlignedSize(curSrc->srcSurf,
                                        gcvNULL,
                                        gcvNULL,
                                        curSrc->srcStride));

    gcmONERROR(gcoSURF_GetSize(curSrc->srcSurf,
                                &curSrc->srcWidth,
                                &curSrc->srcHeight,
                                gcvNULL));

    gcmONERROR(gcoSURF_GetFormat(curSrc->srcSurf, gcvNULL, &curSrc->srcFormat));

    gcmONERROR(gcoSURF_Lock(curSrc->srcSurf, address, memory));

    curSrc->srcPhyAddr[0]  = address[0];
    curSrc->srcLgcAddr[0]  = memory[0];

    curSrc->srcStrideNum = curSrc->srcAddressNum = 1;

    if (GalIsYUVFormat(curSrc->srcFormat))
    {
        gcmONERROR(GalQueryUVStride(curSrc->srcFormat, curSrc->srcStride[0],
                &curSrc->srcStride[1], &curSrc->srcStride[2]));

        curSrc->srcPhyAddr[1] = address[1];
        curSrc->srcLgcAddr[1] = memory[1];

        curSrc->srcPhyAddr[2] = address[2];
        curSrc->srcLgcAddr[2] = memory[2];
        switch (curSrc->srcFormat)
        {
        case gcvSURF_YUY2:
        case gcvSURF_UYVY:
            curSrc->srcStrideNum = curSrc->srcAddressNum = 1;
            break;

        case gcvSURF_I420:
        case gcvSURF_YV12:
            curSrc->srcStrideNum = curSrc->srcAddressNum = 3;
            break;

        case gcvSURF_NV16:
        case gcvSURF_NV61:
        case gcvSURF_NV12:
        case gcvSURF_NV21:
            curSrc->srcStrideNum = curSrc->srcAddressNum = 2;
            break;

        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }
    }

    return gcvSTATUS_OK;

OnError:

    return status;
}

gcsRECT DestRect[] =
{
    {0, 0, 640, 640},
    {10, 10, 630, 630},
    {55, 32, 515, 620},
    {100, 43, 611, 597},
    {200, 200, 400, 400},
    {400, 400, 500, 500},
};

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gco2D egn2D = t2d->runtime->engine2d;
    gctINT i;
    gctINT w = t2d->mediat->width;
    gctINT h = t2d->mediat->height;
    gcsRECT srect, drect;
    gctUINT32 horFactor, verFactor, delta;

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        t2d->mediat->address,
        t2d->mediat->validAddressNum,
        t2d->mediat->stride,
        t2d->mediat->validStrideNum,
        t2d->mediat->tiling,
        t2d->mediat->format,
        gcvSURF_0_DEGREE,
        t2d->mediat->aWidth,
        t2d->mediat->aHeight));

    /* clean temp surface first*/
    drect.left = drect.top = 0;
    drect.right = t2d->mediat->width;
    drect.bottom = t2d->mediat->height;
    gcmONERROR(gco2D_SetClipping(egn2D, &drect));
    gcmONERROR(gco2D_Clear(egn2D, 1, &drect, 0, 0xCC, 0xCC, t2d->mediat->format));
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    drect = DestRect[frameNo % gcmCOUNTOF(DestRect)];
    gcmONERROR(gco2D_SetClipping(egn2D, &drect));

    for (i = 0; i < 8; i++)
    {
        MultiSrcPTR curSrc = &t2d->multiSrc[(i + frameNo) % gcmCOUNTOF(t2d->multiSrc)];

        delta = gcmMIN(curSrc->srcWidth, curSrc->srcHeight) / 10;

        srect.left = delta;
        srect.top = delta + 10;
        srect.right = curSrc->srcWidth - delta;
        srect.bottom = curSrc->srcHeight - delta;

        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        if (!i)
        {
            gcmONERROR(gco2D_DisableAlphaBlend(egn2D));
        }
        else
        {
            switch(curSrc->srcFormat)
            {
                case gcvSURF_YUY2:
                case gcvSURF_UYVY:
                case gcvSURF_NV12:
                case gcvSURF_NV16:
                case gcvSURF_I420:
                case gcvSURF_YV12:
                case gcvSURF_R5G6B5:
                    gcmONERROR(gco2D_SetSourceGlobalColorAdvanced(egn2D, t2d->srcAlpha << 24));
                    gcmONERROR(gco2D_SetTargetGlobalColorAdvanced(egn2D, t2d->dstAlpha << 24));

                    gcmONERROR(gco2D_EnableAlphaBlendAdvanced(egn2D,
                                    gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                                    gcvSURF_GLOBAL_ALPHA_ON, gcvSURF_GLOBAL_ALPHA_ON,
                                    gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT));
                    break;

                default:
                    gcmONERROR(gco2D_SetPorterDuffBlending(
                                egn2D,
                                gcvPD_SRC_OVER));
                    break;
            }
        }

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            curSrc->srcPhyAddr, curSrc->srcAddressNum,
            curSrc->srcStride, curSrc->srcStrideNum,
            gcvLINEAR,
            curSrc->srcFormat,
            gcvSURF_0_DEGREE,
            curSrc->srcWidth,
            curSrc->srcHeight));

        gcmONERROR(gco2D_SetSource(egn2D, &srect));
        gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));
    }

    gcmONERROR(gco2D_MultiSourceBlit(egn2D, 0xFF, &drect, 1));
    gcmONERROR(gco2D_Flush(egn2D));
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    t2d->dstAlpha = (t2d->dstAlpha == 0x20)?0x80:(t2d->dstAlpha - 0x20);
    t2d->srcAlpha = (t2d->srcAlpha == 0xE0)?0x80:(t2d->srcAlpha + 0x20);

    /* StretchBlit to destiantion surface. */
    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        t2d->mediat->address,
        t2d->mediat->validAddressNum,
        t2d->mediat->stride,
        t2d->mediat->validStrideNum,
        t2d->mediat->tiling,
        t2d->mediat->format,
        gcvSURF_0_DEGREE,
        t2d->mediat->aWidth,
        t2d->mediat->aHeight));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        &t2d->dstPhyAddr, 1,
        &t2d->dstStride, 1,
        gcvLINEAR,
        t2d->dstFormat,
        gcvSURF_0_DEGREE,
        t2d->dstWidth,
        t2d->dstHeight));

    srect.left = srect.top = 0;
    srect.right = t2d->mediat->width;
    srect.bottom = t2d->mediat->height;

    drect.left = drect.top = 0;
    drect.right = t2d->dstWidth;
    drect.bottom = t2d->dstHeight;

    gcmONERROR(gco2D_SetSource(egn2D, &srect));
    gcmONERROR(gco2D_SetClipping(egn2D, &drect));

    gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

    /* Calculate the stretch factors. */
    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srect.right - srect.left,
            drect.right - drect.left, &horFactor));

    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srect.bottom - srect.top,
            drect.bottom - drect.top, &verFactor));
    gcmONERROR(gco2D_SetStretchFactors(egn2D, horFactor, verFactor));

    gcmONERROR(gco2D_StretchBlit(egn2D, 1, &drect, 0xCC, 0xCC, t2d->dstFormat));
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

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

    if (t2d->mediat)
    {
        GalDeleteTSurf(gcvNULL, t2d->mediat);
    }

    if ((t2d->dstSurf != gcvNULL) && (t2d->dstLgcAddr != gcvNULL))
    {
        if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock desSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstLgcAddr = gcvNULL;
    }

    // destroy source surface
    for (i = 0; i < 8; i++)
    {
        MultiSrcPTR curSrc = &t2d->multiSrc[i];

        if (curSrc->srcSurf != gcvNULL)
        {
            if (curSrc->srcLgcAddr[0])
            {
                status = gcoSURF_Unlock(curSrc->srcSurf, curSrc->srcLgcAddr);
                if (gcmIS_ERROR(status))
                {
                    GalOutput(GalOutputType_Error | GalOutputType_Console,
                        "Unlock SrcSurface[%d] failed:%s(%d)\n", i, GalStatusString(status), status);
                }
                curSrc->srcLgcAddr[0] = gcvNULL;
            }

            status = gcoSURF_Destroy(curSrc->srcSurf);
            if (gcmIS_ERROR(status))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console,
                    "Destroy SrcSurface[%d] failed:%s(%d)\n", i, GalStatusString(status), status);
            }
        }
    }

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_MULTI_SRC_BLT_BILINEAR_FILTER,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status;
    gctINT i;
    const char *sBasicFile[] = {
        "resource/android_720x1280_icons.bmp",
        "resource/zero2_UYVY_1920x1080_Linear.vimg",
        "resource/VV_Background.bmp",
        "resource/Crew_NV12_1280x720_Linear.vimg",
        "resource/zero2_R5G6B5.bmp",
        "resource/GoneFishing2.bmp",
        "resource/zero2_YUY2_640X480_Linear.vimg",
        "resource/smooth_720p.bmp",
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

    memset(t2d, 0, sizeof(Test2D));

    t2d->runtime = runtime;

    t2d->dstSurf    = runtime->target;
    t2d->dstFormat = runtime->format;
    t2d->dstWidth = 0;
    t2d->dstHeight = 0;
    t2d->dstStride = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;
    t2d->srcAlpha = 0x80;
    t2d->dstAlpha = 0x80;

    gcmONERROR(gco2D_SetStateU32(runtime->engine2d,
                                 gcv2D_STATE_MULTI_SRC_BLIT_BILINEAR_FILTER,
                                 gcvTRUE));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    for (i = 0; i < 8; i++)
    {
        gcmONERROR(ReloadSourceSurface(t2d, i, sBasicFile[i]));
    }

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        gcvSURF_A8R8G8B8,
        gcvLINEAR,
        gcv2D_TSC_DISABLE,
        640, 640,
        &t2d->mediat));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = gcmCOUNTOF(DestRect);
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
