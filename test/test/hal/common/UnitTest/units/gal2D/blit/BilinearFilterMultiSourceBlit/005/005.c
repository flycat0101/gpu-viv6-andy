/****************************************************************************
*
*    Copyright 2012 - 2015 Vivante Corporation, Santa Clara, California.
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
 *  API:        gco2D_MultiSourceBlit gco2D_SetGenericSource gco2D_SetGenericTarget
 *  Check:
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DBilinearFilterMultiSourceBlit005\n" \
"Operation: Bilinear filter single source with point blit mode and compare with stretch blit.\n" \
"2D API: gco2D_MultiSourceBlit gco2D_SetGenericSource gco2D_SetGenericTarget\n" \
"Src: Size        [640x480/640x640/1920x1080]\n"\
"     Rect        [200,120,440,360 / 160,160,480,480 / 690,270,1230,810]\n"\
"     Format      [ARGB1555/ARGB4444/ARGB8888/BGRX4444/BGRX5551/BGRX8888/RGBX4444/RGBX5551/RGB565/RGBX8888/UYVY/XBGR1555/XBGR4444/XBGR8888/YUY2]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear/tile/multiTile/superTileV3/multiSuperTileV3]\n"\
"     Compression [None]\n" \
"Dst: Size        [configurable]\n"\
"     Format      [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: [None]\n"\
"Alphablend: [disable]\n" \
"HW feature dependency: ";

static const char *sSrcFile[] = {
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
    "resource/zero2_UYVY_1920x1080_Linear.vimg",
    "resource/zero2_UYVY_640X480_Linear.vimg",
    "resource/zero2_X8B8G8R8.bmp",
    "resource/zero2_YUY2_640X480_Linear.vimg",
    "resource/rects_A8R8G8B8_640x640_Tile.vimg",
    "resource/rects_A8R8G8B8_640x640_Multi_Tile.vimg",
    "resource/rects_A8R8G8B8_640x640_SuperTileV3.vimg",
    "resource/rects_A8R8G8B8_640x640_Multi_SuperTileV3.vimg",
};


typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    // destination surface
    gcoSURF         dstSurf;
    gceSURF_FORMAT  dstFormat;
    gctUINT         dstWidth;
    gctUINT         dstHeight;
    gctUINT         dstStride;
    gctUINT32       dstPhyAddr;
    gctPOINTER      dstLgcAddr;

    // tmp surface
    gcoSURF         tmpSurf;
    gctUINT         tmpWidth;
    gctUINT         tmpHeight;
    gctUINT         tmpStride;
    gctUINT32       tmpPhyAddr;
    gctPOINTER      tmpLgcAddr;
} Test2D;

gceSURF_ROTATION sRotList[] =
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
    gceSTATUS status = gcvSTATUS_OK;
    gco2D egn2D = t2d->runtime->engine2d;
    gceSURF_ROTATION drot = sRotList[frameNo % gcmCOUNTOF(sRotList)];
    gceSURF_ROTATION srot = sRotList[(frameNo / gcmCOUNTOF(sRotList)) % gcmCOUNTOF(sRotList)];
    T2D_SURF_PTR src = gcvNULL;
    gctINT32 len;
    gcsRECT srect, drect;
    gctINT w = t2d->dstWidth;
    gctINT h = t2d->dstHeight;
    gctUINT32 horFactor, verFactor;
    gctBOOL hMirror, vMirror;

    // create source surface
    gcmONERROR(GalLoadFileToTSurf(
        sSrcFile[frameNo % gcmCOUNTOF(sSrcFile)], &src));

    len = (gcmMIN(src->width, src->height)) >> 1;

    if (srot == gcvSURF_90_DEGREE || srot == gcvSURF_270_DEGREE)
    {
        srect.left = (src->height - len) / 2 ;
        srect.right = (src->height + len) / 2;
        srect.top = (src->width - len) / 2;
        srect.bottom = (src->width + len) / 2;
    }
    else
    {
        srect.left = (src->width - len) / 2 ;
        srect.right = (src->width + len) / 2;
        srect.top = (src->height - len) / 2;
        srect.bottom = (src->height + len) / 2;
    }

    if (drot == gcvSURF_90_DEGREE || drot == gcvSURF_270_DEGREE)
    {
        gctINT t = w;
        w = h;
        h = t;
    }

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

    drect.left   = 0;
    drect.top    = 0;
    drect.right  = drect.left + w;
    drect.bottom = drect.top + h;

    /* render the central part of src to surf0. */
    if (src->superTileVersion != -1)
    {
        gco2D_SetStateU32(egn2D, gcv2D_STATE_SUPER_TILE_VERSION,
            src->superTileVersion);
    }

    gcmONERROR(gco2D_SetCurrentSourceIndex(egn2D, 0));

    gcmONERROR(gco2D_SetClipping(egn2D, &drect));
    gcmONERROR(gco2D_SetSource(egn2D, &srect));

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        src->address, src->validAddressNum,
        src->stride, src->validStrideNum,
        src->tiling, src->format,
        srot,
        src->width, src->height));

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        src->tileStatusConfig,
        src->tileStatusFormat,
        src->tileStatusClear,
        src->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        &t2d->dstPhyAddr, 1,
        &t2d->dstStride, 1,
        gcvLINEAR,
        t2d->dstFormat,
        drot,
        t2d->dstWidth,
        t2d->dstHeight));

    gcmONERROR(gco2D_SetBitBlitMirror(egn2D, hMirror, vMirror));

    gcmONERROR(gco2D_MultiSourceBlit(egn2D, 0x1, &drect, 1));

    gcmONERROR(gco2D_Flush(egn2D));
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    /* Do the orignal stretch blit */
    gcmONERROR(gco2D_SetSourceTileStatus(
                egn2D,
                gcv2D_TSC_DISABLE,
                gcvSURF_UNKNOWN,
                0,
                ~0U
                ));

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        src->address, src->validAddressNum,
        src->stride, src->validStrideNum,
        src->tiling, src->format,
        srot,
        src->width, src->height));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        &t2d->tmpPhyAddr, 1,
        &t2d->tmpStride, 1,
        gcvLINEAR,
        t2d->dstFormat,
        drot,
        t2d->tmpWidth,
        t2d->tmpHeight));

    gcmONERROR(gco2D_SetGdiStretchMode(egn2D, gcvTRUE));

    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srect.right - srect.left,
            drect.right - drect.left, &horFactor));

    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srect.bottom - srect.top,
            drect.bottom - drect.top, &verFactor));

    gcmONERROR(gco2D_SetStretchFactors(egn2D, horFactor, verFactor));

    gcmONERROR(gco2D_StretchBlit(egn2D, 1, &drect, 0xCC, 0xCC, t2d->dstFormat));
    gcmONERROR(gco2D_Flush(egn2D));
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    /* Compare the result of stretch blit and bilinear filter with no stretch. */
    if (memcmp(t2d->dstLgcAddr, t2d->tmpLgcAddr, t2d->dstStride * t2d->dstHeight))
    {
        GalOutput(GalOutputType_Error | GalOutputType_Console,
            "%s(%d) warning: results do not match.\n",__FUNCTION__, __LINE__);
    }

OnError:

    if (src)
    {
        GalDeleteTSurf(gcvNULL, src);
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
    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_MULTI_SRC_BLT_BILINEAR_FILTER,
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

    gcmONERROR(gco2D_SetStateU32(runtime->engine2d,
                                 gcv2D_STATE_MULTI_SRC_BLIT_BILINEAR_FILTER,
                                 gcvFALSE));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    gcmONERROR(gcoSURF_Construct(runtime->hal,
                                 t2d->dstWidth,
                                 t2d->dstHeight,
                                 1,
                                 gcvSURF_BITMAP,
                                 t2d->dstFormat,
                                 gcvPOOL_DEFAULT,
                                 &t2d->tmpSurf));

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->tmpSurf,
                                      &t2d->tmpWidth,
                                      &t2d->tmpHeight,
                                      &t2d->tmpStride));

    gcmONERROR(gcoSURF_Lock(t2d->tmpSurf, &t2d->tmpPhyAddr, &t2d->tmpLgcAddr));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = gcmCOUNTOF(sSrcFile) * gcmCOUNTOF(sRotList);
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
