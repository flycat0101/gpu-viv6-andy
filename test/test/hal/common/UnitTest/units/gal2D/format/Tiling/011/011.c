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

static const char *sSrcFile[] = {
    "resource/SuperTileV2_640x320_X8R8G8B8.vimg",
    "resource/SuperTileV3_640x320_X8R8G8B8.vimg",
    "resource/img_texture2_supertile.vimg",
    "resource/img_texture2_multisupertile.vimg",
};

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatTiling011\n" \
"Operation: Test super tile versions with StretchBlit on the hardware with ANDROID_ONLY feature.\n" \
"2D API: gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_SetStateU32\n" \
"Src: Size        [640x320]\n"\
"     Rect        [configurable]\n"\
"     Format      [XRGB8888/ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [superTileV1/superTileV2/superTileV3/superTile3D/multiSuperTile3D]\n"\
"     Compression [None]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [tile]\n"\
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
    gco2D     egn2D       = t2d->runtime->engine2d;
    gctUINT32 horFactor, verFactor;
    gcsRECT   srcRect;
    gcsRECT   dstRect     = {0, 0, t2d->dstWidth, t2d->dstHeight};
    gctINT    count       = frameNo % 5 + 1;
    gctINT32  DeltaWidth  = count * t2d->dstWidth / 10;
    gctINT32  DeltaHeight = count * t2d->dstHeight / 10;

    if ((frameNo % 5) == 0)
    {
        gcmONERROR(ReloadSourceSurface(t2d, sSrcFile[frameNo / 5]));
    }

    srcRect.left   = 0;
    srcRect.top    = 0;
    srcRect.right  = min(t2d->dstWidth, t2d->srcSurf->width);
    srcRect.bottom = min(t2d->dstHeight, t2d->srcSurf->height);

    // set source
    gcmONERROR(gco2D_SetGenericSource(egn2D, t2d->srcSurf->address, t2d->srcSurf->validAddressNum,
        t2d->srcSurf->stride, t2d->srcSurf->validStrideNum,
        t2d->srcSurf->tiling, t2d->srcSurf->format, t2d->srcSurf->rotation,
        t2d->srcSurf->width, t2d->srcSurf->height));

    gcmONERROR(gco2D_SetSource(egn2D, &srcRect));

    gco2D_SetStateU32(egn2D, gcv2D_STATE_SUPER_TILE_VERSION, t2d->srcSurf->superTileVersion);

    // set dst and clippint rect
    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        &t2d->dstPhyAddr, 1,
        &t2d->dstStride, 1,
        gcvLINEAR,
        t2d->dstFormat,
        gcvSURF_0_DEGREE,
        t2d->dstWidth,
        t2d->dstHeight));

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    // dst rect
        dstRect.left   = t2d->dstWidth / 2 - DeltaWidth;
        dstRect.right  = t2d->dstWidth / 2 + DeltaWidth;
        dstRect.top    = t2d->dstHeight / 2 - DeltaHeight;
        dstRect.bottom = t2d->dstHeight / 2 + DeltaHeight;

    /* Calculate the stretch factors. */
    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srcRect.right - srcRect.left,
        dstRect.right - dstRect.left, &horFactor));

    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srcRect.bottom - srcRect.top,
        dstRect.bottom - dstRect.top, &verFactor));

    /* Program the stretch factors. */
    gcmONERROR(gco2D_SetStretchFactors(egn2D, horFactor, verFactor));

    gcmONERROR(gco2D_StretchBlit(egn2D, 1, &dstRect, 0xCC, 0xCC, t2d->dstFormat));

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
    gceSTATUS status   = gcvSTATUS_OK;
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
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock srcSurf failed:%s\n", GalStatusString(status));
        }
        t2d->srcSurf = gcvNULL;
    }

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_SUPER_TILE_VERSION,
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

    t2d->runtime = runtime;

    t2d->dstSurf    = runtime->target;
    t2d->dstFormat  = runtime->format;
    t2d->dstWidth   = 0;
    t2d->dstHeight  = 0;
    t2d->dstStride  = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;

    t2d->srcSurf    = gcvNULL;

    // dst with dst surf
    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->base.render      = (PGalRender)Render;
    t2d->base.destroy     = (PGalDestroy)Destroy;
    t2d->base.frameCount  = gcmCOUNTOF(sSrcFile) * 5;
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
