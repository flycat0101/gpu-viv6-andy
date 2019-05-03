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
 *  Feature:    10bit format input and output with stretch blit
 *  API:        gco2D_StretchBlit gco2D_SetGenericSource/gco2D_SetGenericTarget
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormat10Bit004\n" \
"Operation: Test multisrc blit for RGB10bit input and output\n" \
"2D API: gco2D_StretchBlit gco2D_SetGenericSource/gco2D_SetGenericTarget\n" \
"Src: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [ARGB2101010/ABGR2101010/RGBA1010102/RGBA1010102]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"Alphablend: [disable]\n" \
"HW feature dependency: ";

#define MAX_SRC 8

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
    MultiSrc multiSrc[MAX_SRC];
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

static gce2D_NATURE_ROTATION sRots[] =
{
    gcvNR_0_DEGREE,
    gcvNR_LEFT_90_DEGREE,
    gcvNR_RIGHT_90_DEGREE,
    gcvNR_180_DEGREE,
    gcvNR_FLIP_X,
    gcvNR_FLIP_Y,
};

gceSURF_FORMAT sFormat[] =
{
    gcvSURF_R10G10B10A2,
    gcvSURF_B10G10R10A2,
    gcvSURF_A2R10G10B10,
    gcvSURF_A2B10G10R10,
    gcvSURF_NV12_10BIT,
    gcvSURF_NV21_10BIT,
};

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gco2D egn2D = t2d->runtime->engine2d;
    gctINT i;
    T2D_SURF_PTR tempSurf = gcvNULL;
    gcsRECT sRect, dRect;
    gctBOOL hMirror, vMirror;
    gce2D_NATURE_ROTATION trot;
    gceSURF_ROTATION srot, drot;

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        sFormat[frameNo % gcmCOUNTOF(sFormat)],
        gcvLINEAR,
        gcv2D_TSC_DISABLE,
        400,
        400,
        &tempSurf));

    dRect.left   = 0;
    dRect.top    = 0;
    dRect.right  = 400;
    dRect.bottom = 400;

    sRect = dRect;

    trot = sRots[frameNo % gcmCOUNTOF(sRots)];

    if (tempSurf->format == gcvSURF_NV12_10BIT ||
        tempSurf->format == gcvSURF_NV21_10BIT ||
        tempSurf->format == gcvSURF_NV16_10BIT ||
        tempSurf->format == gcvSURF_NV61_10BIT)
    {
        trot = gcvNR_0_DEGREE;
    }

    gcmONERROR(gco2D_NatureRotateTranslation(
        gcvFALSE,
        trot,
        0, 0,
        tempSurf->width, tempSurf->height,
        &sRect, &dRect,
        &srot, &drot));

    gcmONERROR(gco2D_SetClipping(egn2D, &dRect));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        tempSurf->address,
        tempSurf->validAddressNum,
        tempSurf->stride,
        tempSurf->validStrideNum,
        tempSurf->tiling,
        tempSurf->format,
        drot,
        tempSurf->aWidth,
        tempSurf->aHeight));

    /* clean temp surface first*/
    gcmONERROR(gco2D_Clear(egn2D, 1, &dRect, 0, 0xCC, 0xCC, tempSurf->format));

    for (i = 0; i < 8; i++)
    {
        MultiSrcPTR curSrc = &t2d->multiSrc[i % gcmCOUNTOF(t2d->multiSrc)];
        trot = sRots[(i + frameNo) % gcmCOUNTOF(sRots)];

        switch ((i+frameNo) % 4)
        {
            case 0:
                hMirror = vMirror = gcvFALSE;
                break;

            case 1:
                hMirror = gcvTRUE;
                vMirror = gcvFALSE;
                break;

            case 2:
                hMirror = gcvFALSE;
                vMirror = gcvTRUE;
                break;

            case 3:
                hMirror = vMirror = gcvTRUE;
                break;
        }

        if (tempSurf->format == gcvSURF_NV12_10BIT ||
            tempSurf->format == gcvSURF_NV21_10BIT ||
            tempSurf->format == gcvSURF_NV16_10BIT ||
            tempSurf->format == gcvSURF_NV61_10BIT)
        {
            trot = gcvNR_0_DEGREE;
            hMirror = vMirror = gcvFALSE;
        }

        gcmONERROR(gco2D_NatureRotateTranslation(
            gcvFALSE,
            trot,
            curSrc->srcWidth, curSrc->srcHeight,
            tempSurf->width, tempSurf->height,
            &sRect, &dRect,
            &srot, &drot));

        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        gcmONERROR(gco2D_SetBitBlitMirror(egn2D, hMirror, vMirror));

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
                case gcvSURF_NV21:
                case gcvSURF_NV16:
                case gcvSURF_NV61:
                case gcvSURF_I420:
                case gcvSURF_YV12:
                        gcmONERROR(gco2D_SetSourceGlobalColorAdvanced(egn2D, 0x80 << 24));
                        gcmONERROR(gco2D_SetTargetGlobalColorAdvanced(egn2D, 0x80 << 24));

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
            srot,
            curSrc->srcWidth,
            curSrc->srcHeight));

        gcmONERROR(gco2D_SetSource(egn2D, &sRect));
        gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));
    }

    gcmONERROR(gco2D_MultiSourceBlit(egn2D, 0xFF, &dRect, 1));



    /* decompress the medial result to dst surface. */
    sRect.left = 0;
    sRect.top = 0;
    sRect.right = tempSurf->width,
    sRect.bottom = tempSurf->height;

    gcmONERROR(gco2D_SetSource(egn2D, &sRect));

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        tempSurf->address,
        tempSurf->validAddressNum,
        tempSurf->stride,
        tempSurf->validStrideNum,
        tempSurf->tiling,
        tempSurf->format,
        tempSurf->rotation,
        tempSurf->aWidth,
        tempSurf->aHeight));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        &t2d->dstPhyAddr, 1,
        &t2d->dstStride, 1,
        gcvLINEAR,
        t2d->dstFormat,
        gcvSURF_0_DEGREE,
        t2d->dstWidth,
        t2d->dstHeight));

    gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvFALSE));
    gcmONERROR(gco2D_DisableAlphaBlend(egn2D));

    dRect.left = 0;
    dRect.top = 0;
    dRect.right = t2d->dstWidth;
    dRect.bottom = t2d->dstHeight;

    gcmONERROR(gco2D_SetStretchRectFactors(egn2D, &sRect, &dRect));

    gcmONERROR(gco2D_SetClipping(egn2D, &dRect));

    gcmONERROR(gco2D_StretchBlit(egn2D, 1, &dRect, 0xCC, 0xCC, t2d->dstFormat));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));
    gcmONERROR(gco2D_Flush(egn2D));

OnError:
    if (tempSurf)
    {
        GalDeleteTSurf(gcvNULL, tempSurf);
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
    gceSTATUS status = gcvSTATUS_OK;

    gctUINT8 SrcIndex = 0;
    for(SrcIndex = 0; SrcIndex < MAX_SRC; SrcIndex ++)
    {
        MultiSrcPTR curSrc = &t2d->multiSrc[SrcIndex];
        // destroy source surface
        if (curSrc->srcSurf != gcvNULL)
        {
            if (curSrc->srcLgcAddr[0])
            {
                if(gcmIS_ERROR(gcoSURF_Unlock(curSrc->srcSurf, curSrc->srcLgcAddr)))
                {
                    GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock srcSurf[%d] failed:%s\n",SrcIndex, GalStatusString(status));
                }
                curSrc->srcLgcAddr[0] = gcvNULL;
            }
            if(gcmIS_ERROR(gcoSURF_Destroy(curSrc->srcSurf)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy srcSurf[%d] failed:%s\n",SrcIndex, GalStatusString(status));
            }
            curSrc->srcSurf = gcvNULL;
        }
    }

    if ((t2d->dstSurf != gcvNULL) && (t2d->dstLgcAddr != gcvNULL))
    {
        if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock desSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstLgcAddr = gcvNULL;
    }

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_10BIT_OUTPUT_LINEAR,
    gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i;
    const char *sBasicFile[] = {
        "resource/smooth_YUY2_592X400_Linear.vimg",
        "resource/VV_Background.bmp",
        "resource/Crew_NV12_1280x720_Linear.vimg",
        "resource/zero2_ARGB4.bmp",
        "resource/source_YUV420_400x400_Linear.vimg",
        "resource/android_720x1280_icons.bmp",
        "resource/Perf25_UYVY_1920x1920_Linear.vimg",
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

    memset(t2d, 0, sizeof(Test2D));

    t2d->runtime = runtime;

    t2d->dstSurf = runtime->target;
    t2d->dstFormat = runtime->format;
    t2d->dstWidth = 0;
    t2d->dstHeight = 0;
    t2d->dstStride = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;

    for (i = 0; i < gcmCOUNTOF(sBasicFile); i++)
    {
        gcmONERROR(ReloadSourceSurface(t2d, i, sBasicFile[i]));
    }

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                    &t2d->dstWidth,
                                    &t2d->dstHeight,
                                    &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = gcmCOUNTOF(sFormat);
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
