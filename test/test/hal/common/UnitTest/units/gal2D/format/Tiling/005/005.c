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
 *  API:        gco2D_FilterBlitEx2
 *  Check:
 */
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatTiling005\n" \
"Operation: Test minor tile input/output format.\n" \
"2D API: gco2D_SetGenericSource gco2D_SetGenericTarget\n" \
"Src: Size        [640x480]\n"\
"     Rect        [0,0,640,480]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Dst: Size        [640x480]\n"\
"     Rect        [0,0,640,480]\n"\
"     Format      [XRGB4444/ARGB4444/RGBX4444/RGBA4444/XBGR4444/ABGR4444/BGRX4444/BGRA4444/XRGB1555/ARGB1555/RGBX5551/RGBA5551/XBGR1555/ABGR1555/BGRX5551/BGRA5551/XRGB8888/ARGB8888/RGBX8888/RGBA8888/XBGR8888/ABGR8888/BGRX8888/BGRA8888/RGB565/BGR565/YUY2/UYVY]\n"\
"     Rotation    [0]\n"\
"     Tile        [minorTile]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"Alphablend: [disable]\n" \
"HW feature dependency: ";

gceSURF_FORMAT TestFormat[] =
{
    gcvSURF_X4R4G4B4,
    gcvSURF_A4R4G4B4,
    gcvSURF_R4G4B4X4,
    gcvSURF_R4G4B4A4,
    gcvSURF_X4B4G4R4,
    gcvSURF_A4B4G4R4,
    gcvSURF_B4G4R4X4,
    gcvSURF_B4G4R4A4,
    gcvSURF_X1R5G5B5,
    gcvSURF_A1R5G5B5,
    gcvSURF_R5G5B5X1,
    gcvSURF_R5G5B5A1,
    gcvSURF_B5G5R5X1,
    gcvSURF_B5G5R5A1,
    gcvSURF_X1B5G5R5,
    gcvSURF_A1B5G5R5,
    gcvSURF_X8R8G8B8,
    gcvSURF_A8R8G8B8,
    gcvSURF_R8G8B8X8,
    gcvSURF_R8G8B8A8,
    gcvSURF_B8G8R8X8,
    gcvSURF_B8G8R8A8,
    gcvSURF_X8B8G8R8,
    gcvSURF_A8B8G8R8,
    gcvSURF_R5G6B5,
    gcvSURF_B5G6R5,
    gcvSURF_YUY2,
    gcvSURF_UYVY,
};

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    T2D_SURF_PTR    dstTemp;

    // destination surface
    gcoSURF            dstSurf;
    gceSURF_FORMAT    dstFormat;
    gctUINT            dstWidth;
    gctUINT            dstHeight;
    gctINT            dstStride;
    gctUINT32        dstPhyAddr;
    gctPOINTER        dstLgcAddr;

    //source surface
    gcoSURF            srcSurf;
    gceSURF_FORMAT    srcFormat;
    gctUINT            srcWidth;
    gctUINT            srcHeight;
    gctINT            srcStride[3];
    gctINT          srcStrideNum;
    gctINT          srcAddressNum;
    gctUINT32        srcPhyAddr[3];
    gctPOINTER        srcLgcAddr[3];
} Test2D;

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsRECT srcRect;
    gcsRECT destRect = {0, 0, t2d->dstTemp->width, t2d->dstTemp->height};
    gco2D egn2D = t2d->runtime->engine2d;

    /* clean temp surface. */
    memset(t2d->dstTemp->logical[0], 0, t2d->dstTemp->vNode.size);
    t2d->dstTemp->format = TestFormat[frameNo];

    srcRect.left = 0;
    srcRect.top = 0;
    srcRect.right = t2d->srcWidth;
    srcRect.bottom = t2d->srcHeight;

    gcmONERROR(gco2D_SetSource(egn2D, &srcRect));

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        t2d->srcPhyAddr, t2d->srcAddressNum,
        t2d->srcStride, t2d->srcStrideNum,
        gcvLINEAR,
        t2d->srcFormat,
        0,
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
        t2d->dstTemp->rotation,
        t2d->dstTemp->width,
        t2d->dstTemp->height));

    gcmONERROR(gco2D_SetClipping(egn2D, &destRect));

    gcmONERROR(gco2D_Blit(egn2D, 1, &destRect, 0xCC, 0xCC, t2d->dstTemp->format));

    /* render the medial result to dst surface. */
    srcRect = destRect;

    gcmONERROR(gco2D_SetSource(egn2D, &srcRect));

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        t2d->dstTemp->address,
        t2d->dstTemp->validAddressNum,
        t2d->dstTemp->stride,
        t2d->dstTemp->validStrideNum,
        t2d->dstTemp->tiling,
        t2d->dstTemp->format,
        t2d->dstTemp->rotation,
        t2d->dstTemp->width,
        t2d->dstTemp->height));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        &t2d->dstPhyAddr, 1,
        &t2d->dstStride, 1,
        gcvLINEAR,
        t2d->dstFormat,
        gcvSURF_0_DEGREE,
        t2d->dstWidth,
        t2d->dstHeight));

    destRect.left = 0;
    destRect.top = 0;
    destRect.right = t2d->dstWidth;
    destRect.bottom = t2d->dstHeight;

    gcmONERROR(gco2D_SetClipping(egn2D, &destRect));

    gcmONERROR(gco2D_SetStretchRectFactors(egn2D, &srcRect, &destRect));

    gcmONERROR(gco2D_StretchBlit(egn2D, 1, &destRect, 0xCC, 0xCC, t2d->dstFormat));

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

    if ((t2d->dstSurf != gcvNULL) && (t2d->dstLgcAddr != gcvNULL))
    {
        if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console,
                "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
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
            GalOutput(GalOutputType_Error | GalOutputType_Console,
                "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
        }
            t2d->srcLgcAddr[0] = gcvNULL;
        }

        if (gcmIS_ERROR(gcoSURF_Destroy(t2d->srcSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console,
                "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
        }
    }

    if (t2d->dstTemp != gcvNULL)
    {
        if (gcmIS_ERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->dstTemp)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console,
                "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
        }
        t2d->dstTemp = gcvNULL;
    }

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_MINOR_TILING,
    gcvFEATURE_2D_TILING,
    gcvFEATURE_ANDROID_ONLY,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status;
    gctUINT32 address[3];
    gctPOINTER memory[3];

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

    t2d->dstTemp = gcvNULL;
    t2d->srcSurf = gcvNULL;

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal, gcvSURF_A8R8G8B8, gcvMINORTILED, gcv2D_TSC_DISABLE, 640, 480, &t2d->dstTemp));

    // create source surface
    t2d->srcSurf = GalLoadDIB2Surface(t2d->runtime->hal,
                   "resource/zero2_ARGB8.bmp");
    if (t2d->srcSurf == NULL)
    {
        gcmONERROR(gcvSTATUS_NOT_FOUND);
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

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = gcmCOUNTOF(TestFormat);
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
