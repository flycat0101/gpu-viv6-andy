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

static gctCONST_STRING s_CaseDescription = \
"Case gal2DFormatCompressed2D003\n" \
"Operation: BitBlit 2D compressed surface to uncompressed surface with full rotations and mirrors.\n" \
"2D API: gco2D_SetSourceTileStatus gco2D_SetGenericSource gco2D_SetBitBlitMirror gco2D_Blit\n" \
"Src: Size        [640x640]\n"\
"     Rect        [160,160,480,480]\n"\
"     Format      [ARGB8888/XRGB8888]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [2D compressed]\n" \
"Dst: Size        [1920x1280]\n"\
"     Rect        [variable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
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

static gceSURF_FORMAT sFormat[] =
{
    gcvSURF_A8R8G8B8,
    gcvSURF_X8R8G8B8,
};

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status = gcvSTATUS_OK;
    gco2D egn2D = t2d->runtime->engine2d;
    T2D_SURF_PTR src = gcvNULL;
    T2D_SURF_PTR surf = gcvNULL;
    T2D_SURF_PTR result = gcvNULL;
    gctINT32 len, n;
    gcsRECT rect;

    // create source surface
    gcmONERROR(GalLoadFileToTSurf(
        "resource/rects_640x640_A8R8G8B8.bmp",
        &src));

    egn2D = t2d->runtime->engine2d;

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        sFormat[(frameNo / gcmCOUNTOF(sRots)) % gcmCOUNTOF(sFormat)],
        gcvLINEAR,
        gcv2D_TSC_2D_COMPRESSED,
        src->width,
        src->height,
        &surf));

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        t2d->runtime->format,
        gcvLINEAR,
        gcv2D_TSC_DISABLE,
        surf->width * 3,
        surf->height * 2,
        &result));

    /* compress the src to surf. */
    rect.left = 0;
    rect.right = src->width;
    rect.top = 0;
    rect.bottom = src->height;
    gcmONERROR(gco2D_SetSource(egn2D, &rect));

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        src->address, src->validAddressNum,
        src->stride, src->validStrideNum,
        src->tiling, src->format,
        src->rotation,
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

    gcmONERROR(gco2D_SetClipping(egn2D, &rect));

    gcmONERROR(gco2D_Blit(egn2D, 1, &rect, 0xCC, 0xCC, surf->format));

    if (!t2d->runtime->noSaveTargetNew)
    {
        char name[200];

        gcmONERROR(gcoHAL_Commit(gcvNULL, gcvTRUE));
        sprintf(name, "gal2DFormatCompressed2D003_intermediate_%03d.bmp", frameNo);
        GalSaveTSurfToDIB(surf, name);
    }

    len = (gcmMIN(surf->width, surf->height)) >> 1;

    /* Uncompress surf to result. */
    rect.left = (surf->width - len) / 2 ;
    rect.right = (surf->width + len) / 2;
    rect.top = (surf->height - len) / 2;
    rect.bottom = (surf->height + len) / 2;

    gcmONERROR(gco2D_SetSource(egn2D, &rect));

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
        sRots[frameNo % gcmCOUNTOF(sRots)],
        result->width,
        result->height));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        result->tileStatusConfig,
        result->format,
        0,
        result->tileStatusAddress
        ));

    for (n = 0; n < 24; ++n)
    {
        gctINT xx = n % 6;
        gctINT yy = n / 6;

        gcmONERROR(gco2D_SetGenericSource(
            egn2D,
            surf->address,
            surf->validAddressNum,
            surf->stride,
            surf->validStrideNum,
            surf->tiling,
            surf->format,
            sRots[xx],
            surf->width,
            surf->height));

        gcmONERROR(gco2D_SetBitBlitMirror(
            egn2D,
            yy & 1 ? gcvTRUE : gcvFALSE,
            yy & 2 ? gcvTRUE : gcvFALSE
            ));

        rect.left = xx * len;
        rect.right = rect.left + len;
        rect.top = yy * len;
        rect.bottom = rect.top + len;
        gcmONERROR(gcsRECT_Rotate(
            &rect,
            result->rotation,
            sRots[frameNo % gcmCOUNTOF(sRots)],
            result->width,
            result->height
            ));
        gcmONERROR(gco2D_SetClipping(egn2D, &rect));

        gcmONERROR(gco2D_Blit(egn2D, 1, &rect, 0xCC, 0xCC, result->format));
    }

    gcmONERROR(gco2D_SetBitBlitMirror(
            egn2D,
            gcvFALSE,
            gcvFALSE
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
    t2d->base.frameCount = gcmCOUNTOF(sRots) * gcmCOUNTOF(sFormat);
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
