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
 *  Feature:    Test fast clear feature for multisource blit
 *  API:        gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_MultiSourceBlit
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription =
"Case gal2DFormatTilestatus018\n" \
"Operation: Test for Fast Clear with multisource blit.\n" \
"2D API: gco2D_MultiSourceBlit\n" \
"Src1: Size       [640x480]\n"\
"     Rect        [160x120]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [SuperTile]\n"\
"     Compression [None]\n" \
"Src2: Size       [320x240]\n"\
"     Rect        [160x120]\n"\
"     Format      [XRGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [SuperTile]\n"\
"     Compression [None]\n" \
"Src3: Size       [320x240]\n"\
"     Rect        [160x120]\n"\
"     Format      [ARGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [SuperTile YMajor]\n"\
"     Compression [None]\n" \
"Src4: Size       [320x240]\n"\
"     Rect        [160x120]\n"\
"     Format      [XRGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [SuperTile]\n"\
"     Compression [None]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"Alphablend: [disable]\n" \
"HW feature dependency: gcvFEATURE_2D_SUPER_TILE_VERSION,gcvFEATURE_2D_MULTI_SOURCE_BLT_EX, gcvFEATURE_2D_FAST_CLEAR,gcvFEATURE_2D_MAJOR_SUPER_TILE";

static const char *sSrcFile[] = {
     "resource/fc/texture7_FC_640_480_A8R8G8B8_SuperTile.vimg",
     "resource/fc/texture3_FC_320_240_X8R8G8B8_SuperTile.vimg",
     "resource/fc/texture2_FC_320_240_A8R8G8B8_SuperTile_Y.vimg",
     "resource/fc/texture4_FC_320_240_A8R8G8B8_SuperTile.vimg",
};


#define MAX_SRC 8

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;
    gcoSURF     dstSurf;
    gceSURF_FORMAT  dstFormat;
    gctUINT         dstWidth;
    gctUINT         dstHeight;
    gctINT          dstStride;
    gctUINT32       dstPhyAddr;
    gctPOINTER      dstLgcAddr;

    /* destination temp surface */
    gcoSURF         dstTempSurf;
    gceSURF_FORMAT  dstTempFormat;
    gctUINT         dstTempWidth;
    gctUINT         dstTempHeight;
    gctINT          dstTempStride;
    gctUINT32       dstTempPhyAddr;
    gctPOINTER      dstTempLgcAddr;

    /*source surface*/
    T2D_SURF_PTR    srcSurf[MAX_SRC];

} Test2D;

static gceSTATUS ReloadSourceSurface(Test2D *t2d, gctUINT SrcIndex, const char * sourcefile)
{

    gceSTATUS status = gcvSTATUS_OK;

    /* destroy source surface*/
    if (t2d->srcSurf[SrcIndex] != gcvNULL)
    {
        gcmONERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->srcSurf[SrcIndex]));
        t2d->srcSurf[SrcIndex] = gcvNULL;
    }

    /* create source surface*/
    gcmONERROR(GalLoadVimgToTSurf(
        sourcefile, &t2d->srcSurf[SrcIndex]));

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

    /* clear dest temp with grey color*/
    gcmONERROR(Gal2DCleanSurface(t2d->runtime->hal, t2d->dstTempSurf, COLOR_ARGB8(0x00, 0x80, 0x80, 0x80)));

    hOffset = 160 / t2d->base.frameCount * frameNo;
    vOffset = 120 / t2d->base.frameCount * frameNo;

    for (i = 0; i < gcmCOUNTOF(sSrcFile); i++)
    {

        T2D_SURF_PTR    curSurf = t2d->srcSurf[i];
        gcsRECT srcRect = {0,0,curSurf->width,curSurf->height};
        gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, i));

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            curSurf->address, curSurf->validAddressNum,
            curSurf->stride, curSurf->validStrideNum,
            curSurf->tiling,
            curSurf->format,
            curSurf->rotation,
            curSurf->width,
            curSurf->height));

        gco2D_SetStateU32(egn2D, gcv2D_STATE_SUPER_TILE_VERSION, curSurf->superTileVersion);

        gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        curSurf->tileStatusConfig,
        curSurf->tileStatusFormat,
        curSurf->tileStatusClear,
        curSurf->tileStatusAddress
        ));

        switch (i % 4)
        {
        case 0:
            srcRect.left = 160 - hOffset;
            srcRect.top  = 120 - vOffset;
            break;

        case 1:
            srcRect.left = hOffset;
            srcRect.top  = 120 - vOffset;
            break;

        case 2:
            srcRect.left = hOffset;
            srcRect.top  = vOffset;
            break;

        case 3:
            srcRect.left = 160 - hOffset;
            srcRect.top  = vOffset;
            break;
        }

        srcRect.right = srcRect.left + 160;
        srcRect.bottom = srcRect.top + 120;
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
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

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

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        gcv2D_TSC_DISABLE,
        gcvSURF_UNKNOWN,
        0,
        ~0U
        ));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        &t2d->dstPhyAddr, 1,
        &t2d->dstStride, 1,
        gcvLINEAR,
        t2d->dstFormat,
        gcvSURF_0_DEGREE,
        t2d->dstWidth,
        t2d->dstHeight));

    dstRect.top = 0;
    dstRect.left = 0;
    dstRect.right = t2d->dstWidth;
    dstRect.bottom = t2d->dstHeight;
    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

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

    /* destroy source surface */
    for (i = 0; i < MAX_SRC; i++)
    {
        T2D_SURF_PTR    curSurf = t2d->srcSurf[i];
        if (curSurf != gcvNULL)
        {
            if (gcmIS_ERROR(GalDeleteTSurf(t2d->runtime->hal, curSurf)))
            {
                GalOutput(GalOutputType_Error | GalOutputType_Console,
                    "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));

            }
            curSurf = gcvNULL;
        }
    }
    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_MULTI_SOURCE_BLT_EX,
    gcvFEATURE_2D_FAST_CLEAR,
    gcvFEATURE_2D_SUPER_TILE_VERSION,
    gcvFEATURE_2D_MAJOR_SUPER_TILE,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{

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

    for (i = 0; i < gcmCOUNTOF(sSrcFile); i++)
    {
        gcmONERROR(ReloadSourceSurface(t2d, i, sSrcFile[i]));
    }

    gcmONERROR(gcoSURF_Construct(
        t2d->runtime->hal,
        320,
        240,
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
    t2d->base.frameCount = 1;
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

