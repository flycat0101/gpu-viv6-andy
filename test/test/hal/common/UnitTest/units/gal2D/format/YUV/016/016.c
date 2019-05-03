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

static const char *sSrcFile[] = {
    "resource/zero2_A1R5G5B5.bmp",
    "resource/zero2_ARGB4.bmp",
    "resource/zero2_ARGB8.bmp",
    "resource/zero2_R5G6B5.bmp",
    "resource/zero2_X1B5G5R5.bmp",
    "resource/zero2_X4B4G4R4.bmp",
    "resource/zero2_X8B8G8R8.bmp",
    "resource/zero2_YUY2_640X480_Linear.vimg",
    "resource/zero2_UYVY_640X480_Linear.vimg",
    "resource/Boston_YV12_640x480_Linear.vimg",
    "resource/zero2_YUV420_640X480_Linear.vimg",
};

static gctCONST_STRING s_CaseDescription =
"Case gal2DFormatYUV016\n" \
"Operation: Test supported YUV420 output and input with bitblit.\n" \
"2D API: gco2D_FilterBlitEx2\n" \
"Src: Size        [variable]\n"\
"     Rect        [variable]\n"\
"     Format      [ARGB1555/ARGB4444/ARGB8888/RGB565/XBGR1555/XBGR4444/XBGR8888/UYVY/YUY2/NV16/NV61/YV12/YUV420]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear/tiled]\n"\
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

typedef struct Test2D {
    GalTest         base;
    GalRuntime      *runtime;

    // destination surface
    gcoSURF         dstSurf;
    gceSURF_FORMAT  dstFormat;
    gctUINT         dstWidth;
    gctUINT         dstHeight;
    gctINT          dstStride;
    gctUINT32       dstPhyAddr;
    gctPOINTER      dstLgcAddr;

    T2D_SURF_PTR    dstTemp;

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
                                        gcvNULL,
                                        gcvNULL,
                                        t2d->srcStride));

    gcmONERROR(gcoSURF_GetSize(t2d->srcSurf,
                                &t2d->srcWidth,
                                &t2d->srcHeight,
                                gcvNULL));

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

gceSURF_FORMAT sFormat[] =
{
    gcvSURF_NV12,
    gcvSURF_NV21,
    gcvSURF_YUY2,
    gcvSURF_YV12,
    gcvSURF_NV16,
};

static gceSURF_ROTATION sRots[] =
{
    gcvSURF_0_DEGREE,
    gcvSURF_90_DEGREE,
    gcvSURF_180_DEGREE,
    gcvSURF_270_DEGREE,
    gcvSURF_FLIP_X,
    gcvSURF_FLIP_Y,
};

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gcsRECT Rect, dstRect;
    gco2D egn2D = t2d->runtime->engine2d;
    gceSURF_ROTATION rot = sRots[frameNo % gcmCOUNTOF(sRots)];
    gctUINT32 len, horFactor, verFactor;

    gcmONERROR(ReloadSourceSurface(t2d, sSrcFile[frameNo%gcmCOUNTOF(sSrcFile)]));

    len = gcmMIN(t2d->srcWidth, t2d->srcHeight);

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        sFormat[frameNo/gcmCOUNTOF(sSrcFile)],
        gcvLINEAR,
        gcv2D_TSC_DISABLE,
        len,
        len,
        &t2d->dstTemp
        ));

    Rect.left   = dstRect.left   = 0;
    Rect.top    = dstRect.top    = 0;
    Rect.right  = dstRect.right  = t2d->dstTemp->width;
    Rect.bottom = dstRect.bottom = t2d->dstTemp->height;

    // set clippint rect
    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    gcmONERROR(gco2D_SetSource(egn2D, &Rect));

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        t2d->srcPhyAddr,
        t2d->srcAddressNum,
        t2d->srcStride,
        t2d->srcStrideNum,
        gcvLINEAR,
        t2d->srcFormat,
        rot,
        t2d->srcWidth,
        t2d->srcHeight));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        t2d->dstTemp->address,
        t2d->dstTemp->validAddressNum,
        t2d->dstTemp->stride,
        t2d->dstTemp->validStrideNum,
        t2d->dstTemp->tiling,
        t2d->dstTemp->format,
        gcvSURF_0_DEGREE,
        t2d->dstTemp->aWidth,
        t2d->dstTemp->aHeight));

    gcmONERROR(gco2D_Blit(egn2D, 1, &dstRect, 0xCC, 0xCC, t2d->dstTemp->format));

    /* render the medial result to dst surface. */
    Rect.left = 0;
    Rect.top = 0;
    Rect.right = t2d->dstTemp->width;
    Rect.bottom = t2d->dstTemp->height;

    dstRect.left = 0;
    dstRect.top = 0;
    dstRect.right = t2d->dstWidth;
    dstRect.bottom = t2d->dstHeight;

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        t2d->dstTemp->address,
        t2d->dstTemp->validAddressNum,
        t2d->dstTemp->stride,
        t2d->dstTemp->validStrideNum,
        t2d->dstTemp->tiling,
        t2d->dstTemp->format,
        t2d->dstTemp->rotation,
        t2d->dstTemp->aWidth,
        t2d->dstTemp->aHeight));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        &t2d->dstPhyAddr, 1,
        &t2d->dstStride, 1,
        gcvLINEAR,
        t2d->dstFormat,
        gcvSURF_0_DEGREE,
        t2d->dstWidth,
        t2d->dstHeight));

    gcmONERROR(gco2D_CalcStretchFactor(egn2D, Rect.right - Rect.left,
            dstRect.right - dstRect.left, &horFactor));
    gcmONERROR(gco2D_CalcStretchFactor(egn2D, Rect.bottom - Rect.top,
            dstRect.bottom - dstRect.top, &verFactor));
    gcmONERROR(gco2D_SetStretchFactors(egn2D, horFactor, verFactor));

    gcmONERROR(gco2D_StretchBlit(egn2D, 1, &dstRect, 0xCC, 0xCC, t2d->dstFormat));

    gcmONERROR(gco2D_Flush(egn2D));
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    if (t2d->dstTemp != gcvNULL)
    {
        if (gcmIS_ERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->dstTemp)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "DeleteT dstTemp failed:%s\n", GalStatusString(status));
        }
        t2d->dstTemp = gcvNULL;
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

    // destroy source surface
    if (t2d->srcSurf != gcvNULL)
    {
        if (t2d->srcLgcAddr[0])
        {
            if (gcmIS_ERROR(gcoSURF_Unlock(t2d->srcSurf, t2d->srcLgcAddr)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock srcSurf failed:%s\n", GalStatusString(status));
            }
            t2d->srcLgcAddr[0] = gcvNULL;
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
    gcvFEATURE_2D_YUV420_OUTPUT_LINEAR,
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

    t2d->dstSurf = runtime->target;
    t2d->dstFormat = runtime->format;
    t2d->dstWidth = 0;
    t2d->dstHeight = 0;
    t2d->dstStride = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;

    t2d->srcSurf    = gcvNULL;
    t2d->srcLgcAddr[0] = gcvNULL;
    t2d->srcFormat = gcvSURF_UNKNOWN;

    t2d->dstTemp    = gcvNULL;

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = gcmCOUNTOF(sSrcFile) * gcmCOUNTOF(sFormat);
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
