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
 *  Feature:    Test fast clear feature for bitblit
 *  API:        gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_Blit
 *  Check:
*/
#include <galUtil.h>

static const char *sSrcFile[] = {
     "resource/fc/texture2_FC_320_240_A8R8G8B8_SuperTile.vimg",
     "resource/fc/texture2_FC_320_240_X8R8G8B8_SuperTile.vimg",
     "resource/fc/texture2_FC_320_240_A8R8G8B8_SuperTile_Y.vimg",
     "resource/fc/texture2_FC_320_240_X8R8G8B8_SuperTile_Y.vimg",
     "resource/fc/texture3_FC_320_240_A8R8G8B8_SuperTile.vimg",
     "resource/fc/texture3_FC_320_240_X8R8G8B8_SuperTile.vimg",
     "resource/fc/texture3_FC_320_240_A8R8G8B8_SuperTile_Y.vimg",
     "resource/fc/texture3_FC_320_240_X8R8G8B8_SuperTile_Y.vimg",
     "resource/fc/texture4_FC_320_240_A8R8G8B8_SuperTile.vimg",
     "resource/fc/texture4_FC_320_240_X8R8G8B8_SuperTile.vimg",
     "resource/fc/texture4_FC_320_240_A8R8G8B8_SuperTile_Y.vimg",
     "resource/fc/texture4_FC_320_240_X8R8G8B8_SuperTile_Y.vimg",
     "resource/fc/texture5_FC_640_480_A8R8G8B8_SuperTile.vimg",
     "resource/fc/texture5_FC_640_480_X8R8G8B8_SuperTile.vimg",
     "resource/fc/texture5_FC_640_480_A8R8G8B8_SuperTile_Y.vimg",
     "resource/fc/texture5_FC_640_480_X8R8G8B8_SuperTile_Y.vimg",
     "resource/fc/texture7_FC_640_480_A8R8G8B8_SuperTile.vimg",
     "resource/fc/texture7_FC_640_480_X8R8G8B8_SuperTile.vimg",
     "resource/fc/texture7_FC_640_480_A8R8G8B8_SuperTile_Y.vimg",
     "resource/fc/texture7_FC_640_480_X8R8G8B8_SuperTile_Y.vimg",
     "resource/fc/texture3_FC_320_240_A4R4G4B4_SuperTile.vimg",
     "resource/fc/texture2_FC_320_240_X4R4G4B4_SuperTile.vimg",
     "resource/fc/texture4_FC_320_240_X1R5G5B5_SuperTile.vimg",
     "resource/fc/texture5_FC_640_480_A1R5G5B5_SuperTile.vimg",
     "resource/fc/texture7_FC_640_480_R5G6B5_SuperTile.vimg",
     "resource/fc/texture5_FC_640_480_R5G6B5_SuperTile.vimg",
     "resource/fc/texture4_FC_320_240_R5G6B5_SuperTile.vimg",
     "resource/fc/texture3_FC_320_240_R5G6B5_SuperTile.vimg",
     "resource/fc/texture2_FC_320_240_R5G6B5_SuperTile.vimg",
     "resource/fc/texture2_FC_320_240_R5G6B5_SuperTile_Y.vimg",
     "resource/fc/texture3_FC_320_240_R5G6B5_SuperTile_Y.vimg",
     "resource/fc/texture4_FC_320_240_R5G6B5_SuperTile_Y.vimg",
     "resource/fc/texture5_FC_640_480_R5G6B5_SuperTile_Y.vimg",
     "resource/fc/texture7_FC_640_480_R5G6B5_SuperTile_Y.vimg",
};

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatTileStatus017\n" \
"Operation: Test Fast clear surface.\n" \
"2D API: gco2D_SetGenericSource gco2D_SetGenericTarget gco2D_Blit\n" \
"Src: Size        [320x240/640x480]\n"\
"     Rect        [320x240/640x480]\n"\
"     Format      [ARGB4444/ARGB8888/RGB565/XRGB8888/A1R5G5B5]\n"\
"     Rotation    [0]\n"\
"     Tile        [SuperTile/SuperTileYMajore]\n"\
"     Compression [None]\n" \
"Dst: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"Alphablend: [disable]\n" \
"HW feature dependency:gcvFEATURE_2D_FAST_CLEAR ,gcvFEATURE_2D_SUPER_TILE_VERSION ,gcvFEATURE_2D_MAJOR_SUPER_TILE ";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;
    T2D_SURF_PTR    dstTemp;
    /*destination surface*/
    gcoSURF         dstSurf;
    gceSURF_FORMAT  dstFormat;
    gctUINT         dstWidth;
    gctUINT         dstHeight;
    gctINT          dstStride;
    gctUINT32       dstPhyAddr;
    gctPOINTER      dstLgcAddr;
    /*source surface*/
    T2D_SURF_PTR    srcSurf;
} Test2D;

static gceSTATUS CDECL ReloadSourceSurface(Test2D *t2d, const char * sourcefile)
{
    gceSTATUS status = gcvSTATUS_OK;

    /*destroy source surface*/
    if (t2d->srcSurf != gcvNULL)
    {
        gcmONERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->srcSurf));
        t2d->srcSurf = gcvNULL;
    }
    /* create source surface*/
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

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal, t2d->dstFormat, gcvLINEAR, gcv2D_TSC_DISABLE,t2d->srcSurf->width,t2d->srcSurf->height, &t2d->dstTemp));

    gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, 0));
    gcmONERROR(gco2D_SetSource(egn2D, &Rect));

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        t2d->srcSurf->address, t2d->srcSurf->validAddressNum,
        t2d->srcSurf->stride, t2d->srcSurf->validStrideNum,
        t2d->srcSurf->tiling, t2d->srcSurf->format, t2d->srcSurf->rotation,
        t2d->srcSurf->width, t2d->srcSurf->height));

    gco2D_SetStateU32(egn2D, gcv2D_STATE_SUPER_TILE_VERSION, t2d->srcSurf->superTileVersion);

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        t2d->srcSurf->tileStatusConfig,
        t2d->srcSurf->tileStatusFormat,
        t2d->srcSurf->tileStatusClear,
        t2d->srcSurf->tileStatusAddress
        ));

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

    gcmONERROR(gco2D_SetClipping(egn2D, &Rect));

    gcmONERROR(gco2D_Blit(egn2D, 1, &Rect, 0xCC, 0xCC, t2d->dstTemp->format));

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        gcv2D_TSC_DISABLE,
        gcvSURF_UNKNOWN,
        0,
        ~0U
        ));

    /* render the medial result to dst surface. */
    gcmONERROR(gco2D_SetSource(egn2D, &Rect));

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

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    gcmONERROR(gco2D_SetStretchRectFactors(egn2D, &Rect, &dstRect));

    gcmONERROR(gco2D_StretchBlit(egn2D, 1, &dstRect, 0xCC, 0xCC, t2d->dstFormat));

    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    if (t2d->dstTemp != gcvNULL)
    {
        if (gcmIS_ERROR(GalDeleteTSurf(t2d->runtime->hal, t2d->dstTemp)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console,
                "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
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
    gcvFEATURE_2D_FAST_CLEAR,
    gcvFEATURE_2D_SUPER_TILE_VERSION,
    gcvFEATURE_2D_MAJOR_SUPER_TILE,
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

    t2d->dstTemp = gcvNULL;
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
