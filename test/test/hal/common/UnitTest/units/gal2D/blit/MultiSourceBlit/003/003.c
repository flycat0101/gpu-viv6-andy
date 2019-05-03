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
 *  API:        gco2D_SetGenericSource gco2D_SetGenericTarget
 *  Check:
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription =
"Case gal2DMultiSourceBlit003\n" \
"Operation: Basic test for 4 sources compositing on hardware with ANDROID_ONLY feature.\n" \
"2D API: gco2D_MultiSourceBlit\n" \
"Src1: Size       [400x400]\n"\
"     Rect        [change according to frameNo]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src2: Size       [640x480]\n"\
"     Rect        [change according to frameNo]\n"\
"     Format      [UYVY]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src3: Size       [640x480]\n"\
"     Rect        [change according to frameNo]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Src4: Size       [640x480]\n"\
"     Rect        [change according to frameNo]\n"\
"     Format      [YV12]\n"\
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

typedef struct _MultiSrc
{
    gcoSURF            srcSurf;
    gceSURF_FORMAT    srcFormat;
    gctUINT            srcWidth;
    gctUINT            srcHeight;
    gctINT            srcStride[3];
    gctINT          srcStrideNum;
    gctINT          srcAddressNum;
    gctUINT32        srcPhyAddr[3];
    gctPOINTER        srcLgcAddr[3];
} MultiSrc, *MultiSrcPTR;

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    // destination surface
    gcoSURF            dstSurf;
    gceSURF_FORMAT    dstFormat;
    gctUINT            dstWidth;
    gctUINT            dstHeight;
    gctINT            dstStride;
    gctUINT32        dstPhyAddr;
    gctPOINTER        dstLgcAddr;

    // destination temp surface
    gcoSURF            dstTempSurf;
    gceSURF_FORMAT    dstTempFormat;
    gctUINT            dstTempWidth;
    gctUINT            dstTempHeight;
    gctINT            dstTempStride;
    gctUINT32        dstTempPhyAddr;
    gctPOINTER        dstTempLgcAddr;

    //source surface
    MultiSrc multiSrc[4];

} Test2D;

static gceSTATUS ReloadSourceSurface(Test2D *t2d, gctUINT SrcIndex, const char * sourcefile)
{
    gceSTATUS status;
    MultiSrcPTR curSrc = &t2d->multiSrc[SrcIndex%4];
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

        if (gcmIS_ERROR(gcoSURF_Destroy(curSrc->srcSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
        }
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
        case gcvSURF_NV12:
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

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gcsRECT Rect = {0, 0, t2d->dstTempWidth, t2d->dstTempHeight};
    gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
    gco2D egn2D = t2d->runtime->engine2d;
    gctINT i, vOffset,hOffset;

    // clear dest temp with grey color
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->dstTempSurf, COLOR_ARGB8(0x00, 0x80, 0x80, 0x80)));

    hOffset = 320 / t2d->base.frameCount * frameNo;
    vOffset = 240 / t2d->base.frameCount * frameNo;

    for (i = 0; i < 4; i++)
    {
        gcsRECT srcRect;

        MultiSrcPTR curSrc = &t2d->multiSrc[i];

        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            curSrc->srcPhyAddr, curSrc->srcAddressNum,
            curSrc->srcStride, curSrc->srcStrideNum,
            gcvLINEAR,
            curSrc->srcFormat,
            gcvSURF_0_DEGREE,
            curSrc->srcWidth,
            curSrc->srcHeight));

        switch (i % 4)
        {
        case 0:
            srcRect.left = 320 - hOffset;
            srcRect.top  = 240 - vOffset;
            break;

        case 1:
            srcRect.left = hOffset;
            srcRect.top  = 240 - vOffset;
            break;

        case 2:
            srcRect.left = hOffset;
            srcRect.top  = vOffset;
            break;

        case 3:
            srcRect.left = 320 - hOffset;
            srcRect.top  = vOffset;
            break;
        }

        srcRect.right = srcRect.left + 320;
        srcRect.bottom = srcRect.top + 240;

        gcmONERROR(gco2D_SetSource(egn2D, &srcRect));

        gcmONERROR(gco2D_SetROP(egn2D, 0xCC, 0xCC));
    }

    gcmONERROR(gco2D_SetClipping(egn2D, &Rect));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        &t2d->dstTempPhyAddr, 1,
        &t2d->dstTempStride, 1,
        gcvLINEAR,
        t2d->dstTempFormat,
        gcvSURF_0_DEGREE,
        t2d->dstTempWidth,
        t2d->dstTempHeight));

    gcmONERROR(gco2D_MultiSourceBlit(egn2D, 0xF, &Rect, 1));

    gcmONERROR(gco2D_Flush(egn2D));

    /* draw result to display surface. */
    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        &t2d->dstTempPhyAddr, 1,
        &t2d->dstTempStride, 1,
        gcvLINEAR,
        t2d->dstTempFormat,
        gcvSURF_0_DEGREE,
        t2d->dstTempWidth,
        t2d->dstTempHeight));

    gcmONERROR(gco2D_SetSource(egn2D, &Rect));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        &t2d->dstPhyAddr, 1,
        &t2d->dstStride, 1,
        gcvLINEAR,
        t2d->dstFormat,
        gcvSURF_0_DEGREE,
        t2d->dstWidth,
        t2d->dstHeight));

    gcmONERROR(gco2D_SetStretchRectFactors(
        egn2D,
        &Rect,
        &dstRect
        ));

    gcmONERROR(gco2D_StretchBlit(
        egn2D,
        1,
        &dstRect,
        0xCC, 0xCC,
        t2d->dstFormat
        ));

    gcmONERROR(gco2D_Flush(egn2D));

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

    if ((t2d->dstSurf != gcvNULL) && (t2d->dstLgcAddr != gcvNULL))
    {
        if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dstSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstLgcAddr = gcvNULL;
    }

    if (t2d->dstTempSurf != gcvNULL)
    {
        if (t2d->dstTempLgcAddr != gcvNULL)
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstTempSurf, t2d->dstTempLgcAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dstTempSurf failed:%s\n", GalStatusString(status));
            }
            t2d->dstTempLgcAddr = gcvNULL;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->dstTempSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
        }
    }

    // destroy source surface
    for (i = 0; i < 4; i++)
    {
        MultiSrcPTR curSrc = &t2d->multiSrc[i];

        if (curSrc->srcSurf != gcvNULL)
        {
            if (curSrc->srcLgcAddr[0])
            {
                if (gcmIS_ERROR(gcoSURF_Unlock(curSrc->srcSurf, curSrc->srcLgcAddr)))
                {
                    GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock srcSurf failed:%s\n", GalStatusString(status));
                }
                curSrc->srcLgcAddr[0] = gcvNULL;
            }

            if (gcmIS_ERROR(gcoSURF_Destroy(curSrc->srcSurf)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
            }
        }
    }

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_MULTI_SOURCE_BLT,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    const char *sBasicFile[] = {
        "resource/VV_Background.bmp",
        "resource/zero2_UYVY_640X480_Linear.vimg",
        "resource/GoneFishing2.bmp",
        "resource/Boston_YV12_640x480_Linear.vimg",
        };
    gctINT i;
    gceSTATUS status;

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

    t2d->dstTempSurf    = gcvNULL;
    t2d->dstTempFormat = runtime->format;

    for (i = 0; i < 4; i++)
    {
        gcmONERROR(ReloadSourceSurface(t2d, i, sBasicFile[i]));
    }

    gcmONERROR(gcoSURF_Construct(
        t2d->runtime->hal,
        640,
        480,
        1,
        gcvSURF_BITMAP,
        t2d->dstTempFormat,
        gcvPOOL_DEFAULT,
        &t2d->dstTempSurf));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstTempSurf,
                                        &t2d->dstTempWidth,
                                        &t2d->dstTempHeight,
                                        &t2d->dstTempStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstTempSurf, &t2d->dstTempPhyAddr, &t2d->dstTempLgcAddr));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = 10;
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
