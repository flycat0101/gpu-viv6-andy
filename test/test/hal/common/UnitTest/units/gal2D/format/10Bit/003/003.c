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
 *  Feature:    10bit format input and output with filter blit
 *  API:        gco2D_StretchBlit gco2D_SetGenericSource/gco2D_SetGenericTarget
 *  Check:
*/
#include <galUtil.h>

static const char *sBitmapFile[] = {
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
    "resource/zero2_YUY2_640X480_Linear.vimg",
    "resource/zero2_UYVY_640X480_Linear.vimg",
    "resource/zero2_YUV420_640X480_Linear.vimg",
    "resource/Boston_YV12_640x480_Linear.vimg",
    "resource/Crew_NV12_1280x720_Linear.vimg",
    "resource/Crew_NV16_1280x720_Linear.vimg",
    "resource/Crew_NV21_1280x720_Linear.vimg",
    "resource/Crew_NV61_1280x720_Linear.vimg",
};

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormat10Bit003\n" \
"Operation: Test filter blit for RGB10bit input and output\n" \
"2D API: gco2D_StretchBlit gco2D_SetGenericSource/gco2D_SetGenericTarget\n" \
"Src: Size        [640x480]\n"\
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
        t2d->srcSurf = GalLoadDIB2Surface(t2d->runtime->hal, sourcefile);
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

gceSURF_ROTATION sRotList[] =
{
    gcvSURF_0_DEGREE,
    gcvSURF_90_DEGREE,
    gcvSURF_180_DEGREE,
    gcvSURF_270_DEGREE,
    gcvSURF_FLIP_X,
    gcvSURF_FLIP_Y,
};

gceSURF_FORMAT sFormat[] =
{
    gcvSURF_R10G10B10A2,
    gcvSURF_B10G10R10A2,
    gcvSURF_A2R10G10B10,
    gcvSURF_A2B10G10R10,
    gcvSURF_NV12_10BIT,
    gcvSURF_NV21_10BIT,
    gcvSURF_NV12_10BIT,
    gcvSURF_NV21_10BIT,
};

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsRECT srect, drect;
    gco2D egn2D = t2d->runtime->engine2d;
    T2D_SURF_PTR tempSurf = gcvNULL;
    gceSURF_ROTATION rot = sRotList[frameNo % gcmCOUNTOF(sRotList)];
    gctUINT len;
    gctBOOL hMirror, vMirror;

    gcmONERROR(ReloadSourceSurface(t2d, sBitmapFile[frameNo % gcmCOUNTOF(sBitmapFile)]));

    len = gcmMIN(t2d->srcWidth, t2d->srcHeight);

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        sFormat[frameNo / gcmCOUNTOF(sFormat)],
        gcvLINEAR,
        gcv2D_TSC_DISABLE,
        len / 2,
        len / 2,
        &tempSurf));

    srect.left = 0;
    srect.top = 0;
    srect.right = len;
    srect.bottom = len;

    drect.left = 0;
    drect.top = 0;
    drect.right = tempSurf->aWidth;
    drect.bottom = tempSurf->aHeight;

    switch (frameNo % 4)
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
        hMirror = vMirror = gcvFALSE;
        rot = gcvSURF_0_DEGREE;
    }

    gcmONERROR(gco2D_SetBitBlitMirror(egn2D, hMirror, vMirror));

    gcmONERROR(gco2D_SetClipping(egn2D, &drect));

    gcmONERROR(gco2D_SetKernelSize(egn2D, 5, 5));

    gcmONERROR(gco2D_FilterBlitEx2(egn2D,
        t2d->srcPhyAddr, t2d->srcAddressNum,
        t2d->srcStride, t2d->srcStrideNum,
        gcvLINEAR, t2d->srcFormat, gcvSURF_0_DEGREE,
        t2d->srcWidth, t2d->srcHeight,
        &srect,
        tempSurf->address, tempSurf->validAddressNum,
        tempSurf->stride, tempSurf->validStrideNum,
        tempSurf->tiling, tempSurf->format, rot,
        tempSurf->aWidth, tempSurf->aHeight,
        &drect, gcvNULL));


    srect.left = 0;
    srect.top = 0;
    srect.right = tempSurf->width;
    srect.bottom = tempSurf->height;

    drect.left = 0;
    drect.top = 0;
    drect.right = t2d->dstWidth;
    drect.bottom = t2d->dstHeight;

    gcmONERROR(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvFALSE));

    gcmONERROR(gco2D_SetClipping(egn2D, &drect));

    gcmONERROR(gco2D_FilterBlitEx2(egn2D,
        tempSurf->address, tempSurf->validAddressNum,
        tempSurf->stride, tempSurf->validStrideNum,
        tempSurf->tiling, tempSurf->format, gcvSURF_0_DEGREE,
        tempSurf->aWidth, tempSurf->aHeight, &srect,
        &t2d->dstPhyAddr, 1,
        &t2d->dstStride, 1,
        gcvLINEAR, t2d->dstFormat, gcvSURF_0_DEGREE,
        t2d->dstWidth, t2d->dstHeight,
        &drect, gcvNULL));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

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
    gcvFEATURE_2D_10BIT_OUTPUT_LINEAR,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;

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

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                    &t2d->dstWidth,
                                    &t2d->dstHeight,
                                    &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = gcmCOUNTOF(sRotList) * gcmCOUNTOF(sFormat);
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
