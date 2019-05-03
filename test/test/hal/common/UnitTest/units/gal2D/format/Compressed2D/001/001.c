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
 *  Feature:    2D Compression
 *  API:        gco2D_SetSourceTileStatus gco2D_Blit
 *  Check:
*/
#include <galUtil.h>

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
    "resource/zero2_UYVY_1920x1080_Linear.vimg",
    "resource/zero2_UYVY_640X480_Linear.vimg",
    "resource/zero2_X1B5G5R5.bmp",
    "resource/zero2_X4B4G4R4.bmp",
    "resource/zero2_X8B8G8R8.bmp",
    "resource/zero2_YUY2_640X480_Linear.vimg",
    "resource/rects_A8R8G8B8_640x640_Tile.vimg",
    "resource/rects_A8R8G8B8_640x640_Multi_Tile.vimg",
    "resource/rects_A8R8G8B8_640x640_SuperTileV3.vimg",
    "resource/rects_A8R8G8B8_640x640_Multi_SuperTileV3.vimg",
};

static gceSURF_FORMAT sFormat[] =
{
    gcvSURF_A8R8G8B8,
    gcvSURF_X8R8G8B8,
};

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatCompressed2D001\n" \
"Operation: BitBlit uncompressed surfaces to 2D compressed surface with full src rotations and mirrors.\n" \
"2D API: gco2D_SetTargetTileStatus gco2D_SetGenericSource gco2D_SetBitBlitMirror gco2D_Blit\n" \
"Src: Size        [640x480/640x640/1920x1080]\n"\
"     Rect        [200,120,440,360 / 160,160,480,480 / 690,270,1230,810]\n"\
"     Format      [ARGB1555/ARGB4444/ARGB8888/BGRX4444/BGRX5551/BGRX8888/RGBX4444/RGBX5551/RGB565/RGBX8888/UYVY/XBGR1555/XBGR4444/XBGR8888/YUY2]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear/tile/multiTile/superTileV3/multiSuperTileV3]\n"\
"     Compression [None]\n" \
"Dst: Size        [1440x960/1920x1280/3240x2160]\n"\
"     Rect        [variable]\n"\
"     Format      [ARGB8888/XRGB8888]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [2D compressed]\n" \
"Brush: [None]\n"\
"Alphablend: [disable]\n" \
"HW feature dependency: ";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;
} Test2D;

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
    gceSTATUS status = gcvSTATUS_OK;
    gco2D egn2D = t2d->runtime->engine2d;
    T2D_SURF_PTR src = gcvNULL;
    T2D_SURF_PTR surf = gcvNULL;
    T2D_SURF_PTR result = gcvNULL;
    gctINT32 len, n;
    gcsRECT srect, rect;

    // create source surface
    gcmONERROR(GalLoadFileToTSurf(
        sSrcFile[frameNo % gcmCOUNTOF(sSrcFile)], &src));

    len = (gcmMIN(src->width, src->height)) >> 1;

    srect.left = (src->width - len) / 2 ;
    srect.right = (src->width + len) / 2;
    srect.top = (src->height - len) / 2;
    srect.bottom = (src->height + len) / 2;

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        sFormat[(frameNo / gcmCOUNTOF(sSrcFile)) % gcmCOUNTOF(sFormat)],
        gcvLINEAR,
        gcv2D_TSC_2D_COMPRESSED,
        len * 6,
        len * 4,
        &surf));

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        t2d->runtime->format,
        gcvLINEAR,
        gcv2D_TSC_DISABLE,
        surf->width,
        surf->height,
        &result));

    /* render the central part of src to surf0. */
    if (src->superTileVersion != -1)
    {
        gco2D_SetStateU32(egn2D, gcv2D_STATE_SUPER_TILE_VERSION,
            src->superTileVersion);
    }

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        src->tileStatusConfig,
        src->tileStatusFormat,
        src->tileStatusClear,
        src->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        surf->address,
        surf->validAddressNum,
        surf->stride,
        surf->validStrideNum,
        surf->tiling,
        surf->format,
        surf->rotation,
        surf->width,
        surf->height));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        surf->tileStatusConfig,
        surf->format,
        0,
        surf->tileStatusAddress
        ));

    for (n = 0; n < 24; ++n)
    {
        gctINT xx = n % 6;
        gctINT yy = n / 6;

        rect = srect;
        gcmONERROR(gcsRECT_Rotate(
            &rect,
            src->rotation,
            sRots[xx],
            src->width,
            src->height
            ));

        gcmONERROR(gco2D_SetSource(egn2D, &rect));

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            src->address, src->validAddressNum,
            src->stride, src->validStrideNum,
            src->tiling, src->format,
            sRots[xx],
            src->width, src->height));

        gcmONERROR(gco2D_SetBitBlitMirror(
            egn2D,
            yy & 1 ? gcvTRUE : gcvFALSE,
            yy & 2 ? gcvTRUE : gcvFALSE
            ));

        rect.left = xx * len;
        rect.right = rect.left + len;
        rect.top = yy * len;
        rect.bottom = rect.top + len;

        gcmONERROR(gco2D_SetClipping(egn2D, &rect));

        gcmONERROR(gco2D_Blit(egn2D, 1, &rect, 0xCC, 0xCC, surf->format));
    }
    if (!t2d->runtime->noSaveTargetNew)
    {
        char name[200];

        gcmONERROR(gcoHAL_Commit(gcvNULL, gcvTRUE));
        sprintf(name, "gal2DFormatCompressed2D001_intermediate_%03d.bmp", frameNo);
        GalSaveTSurfToDIB(surf, name);
    }
    /* Uncompress surf to result. */
    rect.left = rect.top = 0;
    rect.right = surf->width;
    rect.bottom = surf->height;

    gcmONERROR(gco2D_SetSource(egn2D, &rect));

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        surf->address,
        surf->validAddressNum,
        surf->stride,
        surf->validStrideNum,
        surf->tiling,
        surf->format,
        gcvSURF_0_DEGREE,
        surf->width,
        surf->height));

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        surf->tileStatusConfig,
        surf->tileStatusFormat,
        0,
        surf->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        result->address,
        result->validAddressNum,
        result->stride,
        result->validStrideNum,
        result->tiling,
        result->format,
        result->rotation,
        result->width,
        result->height));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        result->tileStatusConfig,
        result->format,
        0,
        result->tileStatusAddress
        ));

    gcmONERROR(gco2D_SetClipping(egn2D, &rect));

    gcmONERROR(gco2D_SetBitBlitMirror(
        egn2D, gcvFALSE, gcvFALSE
        ));

    gcmONERROR(gco2D_Blit(egn2D, 1, &rect, 0xCC, 0xCC, result->format));

    gcmONERROR(gco2D_SetTargetTileStatus(
                egn2D,
                gcv2D_TSC_DISABLE,
                gcvSURF_UNKNOWN,
                0,
                ~0U
                ));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    if (!t2d->runtime->noSaveTargetNew && t2d->runtime->saveFullName)
    {
        GalSaveTSurfToDIB(result, t2d->runtime->saveFullName);
    }

OnError:

    if (result)
    {
        GalDeleteTSurf(gcvNULL, result);
    }

    if (surf)
    {
        GalDeleteTSurf(gcvNULL, surf);
    }

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
    gcvFEATURE_2D_COMPRESSION,
    gcvFEATURE_2D_SUPER_TILE_VERSION,
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
    runtime->saveTarget = gcvFALSE;
    runtime->cleanTarget = gcvFALSE;

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
