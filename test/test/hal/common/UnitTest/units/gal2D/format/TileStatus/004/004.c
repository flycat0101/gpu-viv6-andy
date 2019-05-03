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
 *  API:        gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_Blit
 *  Check:
*/
#include <galUtil.h>

static const char *sSrcFile[] = {
     "resource/texture2_cc0_A4R4G4B4.vimg",
     "resource/texture2_cc0_A8R8G8B8.vimg",
     "resource/texture2_cc0_R5G6B5.vimg",
     "resource/texture2_cc0_X8R8G8B8.vimg",
     "resource/texture2_cc1_A4R4G4B4.vimg",
     "resource/texture2_cc1_A8R8G8B8.vimg",
     "resource/texture2_cc1_R5G6B5.vimg",
     "resource/texture2_cc1_X8R8G8B8.vimg",
     "resource/texture5_cc0_A8R8G8B8.vimg",
     "resource/texture5_cc0_R5G6B5.vimg",
     "resource/texture5_cc1_A8R8G8B8.vimg",
     "resource/texture5_cc1_R5G6B5.vimg",
};

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatTileStatus004\n" \
"Operation: Convert FC surface to linear by TwoPass FilterBlit.\n" \
"2D API: gco2D_FilterBlitEx2\n" \
"Src: Size        [320x240/640x480]\n"\
"     Rect        [0,0,320,240 / 0,0,640,480]\n"\
"     Format      [ARGB4444/ARGB8888/RGB565/XRGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [multiSuperTile]\n"\
"     Compression [None]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"KernelSize: [9]\n"\
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
    gcsRECT Rect, dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
    gco2D egn2D = t2d->runtime->engine2d;

    gcmONERROR(ReloadSourceSurface(t2d, sSrcFile[frameNo]));

    Rect.left = 0;
    Rect.top = 0;
    Rect.right = t2d->srcSurf->width;
    Rect.bottom = t2d->srcSurf->height;

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        t2d->srcSurf->tileStatusConfig,
        t2d->srcSurf->tileStatusFormat,
        t2d->srcSurf->tileStatusClear,
        t2d->srcSurf->tileStatusAddress
        ));

    gco2D_SetStateU32(egn2D, gcv2D_STATE_SUPER_TILE_VERSION, t2d->srcSurf->superTileVersion);

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    gcmONERROR(gco2D_SetKernelSize(egn2D, 9, 9));

    gcmONERROR(gco2D_FilterBlitEx2(egn2D,
        t2d->srcSurf->address, t2d->srcSurf->validAddressNum,
        t2d->srcSurf->stride, t2d->srcSurf->validStrideNum,
        t2d->srcSurf->tiling, t2d->srcSurf->format, t2d->srcSurf->rotation,
        t2d->srcSurf->width, t2d->srcSurf->height, &Rect,
        &t2d->dstPhyAddr, 1,
        &t2d->dstStride, 1,
        gcvLINEAR, t2d->dstFormat, gcvSURF_0_DEGREE,
        t2d->dstWidth, t2d->dstHeight,
        &dstRect, gcvNULL));

    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    /* Disable the source tile status. */
    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        gcv2D_TSC_DISABLE,
        gcvSURF_UNKNOWN,
        0,
        ~0U
        ));

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

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_SCALER,
    gcvFEATURE_2D_FC_SOURCE,
    gcvFEATURE_2D_TILING,
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

    t2d->srcSurf = gcvNULL;

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = gcmCOUNTOF(sSrcFile);
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
