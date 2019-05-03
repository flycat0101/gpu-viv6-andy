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

static const char **sSrcImages = gcvNULL;

static const char *sSrcFilev3[] = {
    "resource/img_texture2_multitile.vimg",
    "resource/rects_A8R8G8B8_640x640_Multi_Tile.vimg",
    "resource/rects_A8R8G8B8_640x640_SuperTileV3.vimg",
    "resource/rects_A8R8G8B8_640x640_Multi_SuperTileV3.vimg",
};

static const char *sSrcFilev2[] = {
    "resource/img_texture2_multitile.vimg",
    "resource/rects_A8R8G8B8_640x640_Multi_Tile.vimg",
    "resource/rects_A8R8G8B8_640x640_SuperTileV2.vimg",
    "resource/rects_A8R8G8B8_640x640_Multi_SuperTileV2.vimg",
};

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatTiling003\n" \
"Operation: Test multi tiled, super tiled, multi super tiled input and tiled output for FilterBlit.\n" \
"2D API: gco2D_FilterBlitEx2\n" \
"Src: Size        [640x640]\n"\
"     Rect        [configurable]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [tile/multiTile/multiTile3D/superTile/multiSuperTile]\n"\
"     Compression [None]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [YUY2/UYVY]\n"\
"     Rotation    [0]\n"\
"     Tile        [tile]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"KernelSize: [3/5]\n" \
"Alphablend: [disable]\n" \
"HW feature dependency: gcvFEATURE_2D_SUPER_TILE_V2 gcvFEATURE_2D_SUPER_TILE_V3 ";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    T2D_SURF_PTR    dstTemp;

    // destination surface
    gcoSURF         dstSurf;
    gceSURF_FORMAT  dstFormat;
    gctUINT         dstWidth;
    gctUINT         dstHeight;
    gctINT          dstStride;
    gctUINT32       dstPhyAddr;
    gctPOINTER      dstLgcAddr;

    //source surface
    T2D_SURF_PTR    srcSurf;

} Test2D;

static gceSTATUS CDECL ReloadSourceSurface(Test2D *t2d, const char * sourcefile)
{
    gceSTATUS status;

    // destroy source surface
    if (t2d->srcSurf != gcvNULL)
    {
        gcmONERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->srcSurf));
        t2d->srcSurf = gcvNULL;
    }

    // create source surface
    gcmONERROR(GalLoadVimgToTSurf(
        sourcefile, &t2d->srcSurf));

    return gcvSTATUS_OK;


OnError:
    return status;
}

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status;
    gcsRECT srcRect;
    gcsRECT dstRect = {0, 0, t2d->dstTemp->width, t2d->dstTemp->height};
    gco2D egn2D = t2d->runtime->engine2d;

    gcmONERROR(ReloadSourceSurface(t2d, sSrcImages[frameNo]));

    srcRect.left = 0;
    srcRect.top = 0;
    srcRect.right = t2d->srcSurf->width;
    srcRect.bottom = t2d->srcSurf->height;

    // set clippint rect
    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    // set kernel size
    gcmONERROR(gco2D_SetKernelSize(egn2D, 3, 5));

    t2d->dstTemp->format = frameNo&1 ? gcvSURF_YUY2 : gcvSURF_UYVY;

    /* Linear source to tiled temp. */
    gcmONERROR(gco2D_FilterBlitEx2(egn2D,
        t2d->srcSurf->address, t2d->srcSurf->validAddressNum,
        t2d->srcSurf->stride, t2d->srcSurf->validStrideNum,
        t2d->srcSurf->tiling, t2d->srcSurf->format, t2d->srcSurf->rotation,
        t2d->srcSurf->width, t2d->srcSurf->height, &srcRect,
        t2d->dstTemp->address, t2d->dstTemp->validAddressNum,
        t2d->dstTemp->stride, t2d->dstTemp->validStrideNum,
        t2d->dstTemp->tiling, t2d->dstTemp->format, t2d->dstTemp->rotation,
        t2d->dstTemp->width, t2d->dstTemp->height,
        &dstRect, &dstRect));

    /* Tiled temp to linear dest. */
    srcRect = dstRect;

    dstRect.left = 0;
    dstRect.top = 0;
    dstRect.right = t2d->dstWidth;
    dstRect.bottom = t2d->dstHeight;

    // set clippint rect
    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    // set kernel size
    gcmONERROR(gco2D_SetKernelSize(egn2D, 5, 3));

    gcmONERROR(gco2D_FilterBlitEx2(egn2D,
        t2d->dstTemp->address, t2d->dstTemp->validAddressNum,
        t2d->dstTemp->stride, t2d->dstTemp->validStrideNum,
        t2d->dstTemp->tiling, t2d->dstTemp->format, t2d->dstTemp->rotation,
        t2d->dstTemp->width, t2d->dstTemp->height, &srcRect,
        &t2d->dstPhyAddr, 1, &t2d->dstStride, 1, gcvLINEAR, t2d->dstFormat, gcvSURF_0_DEGREE,
        t2d->dstWidth, t2d->dstHeight,
        &dstRect, &dstRect));

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
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock desSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstLgcAddr = gcvNULL;
    }

    // destroy source surface
    if (t2d->srcSurf != gcvNULL)
    {
        if (gcmIS_ERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->srcSurf)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console,
            "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
        }
        t2d->srcSurf = gcvNULL;
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
    gcvFEATURE_2D_OPF_YUV_OUTPUT,
    gcvFEATURE_2D_TILING,
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

    if (runtime->ChipModel >= gcv600 ||
        ((runtime->ChipModel == gcv300 || runtime->ChipModel == gcv200) &&
         runtime->ChipRevision >= 0x4650 && runtime->ChipRevision < 0x5000))
    {
        GalOutput(GalOutputType_Result | GalOutputType_Console, "Multi super tile is not supported.\n");
        runtime->notSupport = gcvTRUE;
    }

    if (runtime->notSupport)
        return gcvFALSE;

    if (gcoHAL_IsFeatureAvailable(runtime->hal, gcvFEATURE_2D_SUPER_TILE_V2) == gcvTRUE)
    {
        sSrcImages = sSrcFilev2;
    }
    else if (gcoHAL_IsFeatureAvailable(runtime->hal, gcvFEATURE_2D_SUPER_TILE_V3) == gcvTRUE)
    {
        sSrcImages = sSrcFilev3;
    }
    else
    {
        GalOutput(GalOutputType_Result | GalOutputType_Console, "2D super tiling is not supported.\n");
        runtime->notSupport = gcvTRUE;
        return gcvFALSE;
    }

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
        t2d->runtime->hal, t2d->dstFormat, gcvLINEAR, gcv2D_TSC_DISABLE, 300, 300, &t2d->dstTemp));

    t2d->dstTemp->tiling = gcvLINEAR;

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = 4;
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
