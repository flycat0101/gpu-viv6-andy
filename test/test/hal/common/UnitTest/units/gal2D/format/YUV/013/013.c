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
 *  API:
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatYUV013\n" \
"Operation: Test YUV to RGB converstion -- user defined 709.\n" \
"2D API: gco2D_SetYUVColorMode gco2D_SetStateArrayI32 gco2D_FilterBlitEx2\n" \
"Src: Size        [640x480/1280x720]\n"\
"     Rect        [0,0,640,480 / 0,0,1280,720]\n"\
"     Format      [YUY2/UYVY/I420/YV12/NV12/NV21/NV16/NV61]\n"\
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
"KernelSize: [9]\n" \
"Alphablend: [disable]\n" \
"HW feature dependency: ";

static const char *sBitmapFile[] = {
    "resource/zero2_YUY2_640X480_Linear.vimg",
    "resource/zero2_UYVY_640X480_Linear.vimg",
    "resource/zero2_YUV420_640X480_Linear.vimg",
    "resource/Boston_YV12_640x480_Linear.vimg",
    "resource/Crew_NV12_1280x720_Linear.vimg",
    "resource/Crew_NV16_1280x720_Linear.vimg",
    "resource/Crew_NV21_1280x720_Linear.vimg",
    "resource/Crew_NV61_1280x720_Linear.vimg",
};

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

    // golden surface
    gcoSURF         dst2Surf;
    gctUINT32       dst2PhyAddr;
    gctPOINTER      dst2LgcAddr;

    //source surface
    gcoSURF         srcSurf;
    gceSURF_FORMAT  srcFormat;
    gctUINT         srcWidth;
    gctUINT         srcHeight;
    gctINT          srcStride[3];
    gctINT          srcStrideNum;
    gctINT          srcAddressNum;
    gctUINT32       srcPhyAddr[3];
    gctPOINTER      srcLgcAddr[3];
} Test2D;

static gceSTATUS CDECL ReloadSourceSurface(Test2D *t2d, const char * sourcefile)
{
    gceSTATUS status;

    gctUINT32 address[3];
    gctPOINTER memory[3];

    // destroy source surface
    if (t2d->srcSurf != gcvNULL)
    {
        if (t2d->srcLgcAddr[0])
        {
            gcmONERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr));
            t2d->srcLgcAddr[0] = 0;
        }

        gcmONERROR(gcoSURF_Destroy(t2d->srcSurf));
        t2d->srcSurf = gcvNULL;
    }

    // create source surface
    gcmONERROR(GalLoadVimgToSurface(
        sourcefile, &t2d->srcSurf));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->srcSurf,
                                        gcvNULL,
                                        gcvNULL,
                                        t2d->srcStride));

    gcmONERROR(gcoSURF_GetSize(t2d->srcSurf,
                                &t2d->srcWidth,
                                &t2d->srcHeight,
                                gcvNULL));

    gcmONERROR(gcoSURF_GetFormat(t2d->srcSurf, gcvNULL, &t2d->srcFormat));

    gcmONERROR(GalQueryUVStride(t2d->srcFormat, t2d->srcStride[0],
            &t2d->srcStride[1], &t2d->srcStride[2]));

    gcmONERROR(gcoSURF_Lock(t2d->srcSurf, address, memory));

    t2d->srcPhyAddr[0]  = address[0];
    t2d->srcLgcAddr[0]  = memory[0];

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
    return gcvSTATUS_OK;

OnError:
    return status;
}

gctINT sCSC[] =
{
    298, 0, 461,
    298, -55, -137,
    298, 543, 0,
    -63776, 19808, -74272
};

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
    gcsRECT srcRect;
    gco2D egn2D = t2d->runtime->engine2d;

    gcmONERROR(ReloadSourceSurface(t2d, sBitmapFile[frameNo]));

    srcRect.left = srcRect.top = 0;
    srcRect.right = t2d->srcWidth;
    srcRect.bottom = t2d->srcHeight;

    // set clippint rect
    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    gcmONERROR(gco2D_SetYUVColorMode(egn2D, gcv2D_YUV_USER_DEFINED_CLAMP));

    gcmONERROR(gco2D_SetStateArrayI32(egn2D, gcv2D_STATE_ARRAY_CSC_YUV_TO_RGB, sCSC, gcmCOUNTOF(sCSC)));

    gcmONERROR(gco2D_FilterBlitEx2(egn2D,
            t2d->srcPhyAddr, t2d->srcAddressNum,
            t2d->srcStride, t2d->srcStrideNum,
            gcvLINEAR, t2d->srcFormat, gcvSURF_0_DEGREE,
            t2d->srcWidth, t2d->srcHeight, &srcRect,
            &t2d->dstPhyAddr, 1, &t2d->dstStride, 1,
            gcvLINEAR, t2d->dstFormat, gcvSURF_0_DEGREE,
            t2d->dstWidth, t2d->dstHeight, &dstRect,
            gcvNULL));

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    memset(t2d->dst2LgcAddr, 0, t2d->dstStride * t2d->dstHeight);

    gcmONERROR(gco2D_SetYUVColorMode(egn2D, gcv2D_YUV_709));

    gcmONERROR(gco2D_FilterBlitEx2(egn2D,
            t2d->srcPhyAddr, t2d->srcAddressNum,
            t2d->srcStride, t2d->srcStrideNum,
            gcvLINEAR, t2d->srcFormat, gcvSURF_0_DEGREE,
            t2d->srcWidth, t2d->srcHeight, &srcRect,
            &t2d->dst2PhyAddr, 1, &t2d->dstStride, 1,
            gcvLINEAR, t2d->dstFormat, gcvSURF_0_DEGREE,
            t2d->dstWidth, t2d->dstHeight, &dstRect,
            gcvNULL));

    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    if (memcmp(t2d->dstLgcAddr, t2d->dst2LgcAddr, t2d->dstStride * t2d->dstHeight))
    {
        GalOutput(GalOutputType_Error | GalOutputType_Console,
            "%s(%d) warning: results do not match.\n",__FUNCTION__, __LINE__);
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
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock desSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstLgcAddr = gcvNULL;
    }

    // destroy golden surface
    if (t2d->dst2Surf != gcvNULL)
    {
        if (t2d->dst2LgcAddr)
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dst2Surf, t2d->dst2LgcAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dst2Surf failed:%s\n", GalStatusString(status));
            }
            t2d->dst2LgcAddr = 0;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->dst2Surf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy golden failed:%s\n", GalStatusString(status));
        }
    }

    // destroy source surface
    if (t2d->srcSurf != gcvNULL)
    {
        if (t2d->srcLgcAddr[0])
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock srcSurf failed:%s\n", GalStatusString(status));
            }
            t2d->srcLgcAddr[0] = 0;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->srcSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Destroy Surf failed:%s\n", GalStatusString(status));
        }
    }

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_SCALER,
    gcvFEATURE_2D_COLOR_SPACE_CONVERSION,
    gcvFEATURE_NO_USER_CSC,
    gcvFEATURE_ANDROID_ONLY,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
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

    t2d->runtime = runtime;

    t2d->dstSurf    = runtime->target;
    t2d->dstFormat = runtime->format;
    t2d->dstWidth = 0;
    t2d->dstHeight = 0;
    t2d->dstStride = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;

    t2d->srcSurf    = gcvNULL;
    t2d->srcWidth = 0;
    t2d->srcHeight = 0;
    t2d->srcLgcAddr[0] = gcvNULL;
    t2d->srcFormat = gcvSURF_UNKNOWN;

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    gcmONERROR(gcoSURF_Construct(
        gcvNULL,
        t2d->dstWidth,
        t2d->dstHeight,
        1,
        gcvSURF_BITMAP,
        t2d->dstFormat,
        gcvPOOL_DEFAULT,
        &t2d->dst2Surf));

    gcmONERROR(gcoSURF_Lock(t2d->dst2Surf, &t2d->dst2PhyAddr, &t2d->dst2LgcAddr));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.description = s_CaseDescription;
    t2d->base.frameCount = gcmCOUNTOF(sBitmapFile);

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
