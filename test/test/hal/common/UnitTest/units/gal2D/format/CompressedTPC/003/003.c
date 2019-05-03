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
"Case gal2DFormatCompressedTPC003\n" \
"Operation: BitBlit TPC compressed surface to uncompressed surface with full rotations and mirrors.\n" \
"2D API: gco2D_SetSourceTileStatus gco2D_SetGenericSource gco2D_SetBitBlitMirror gco2D_Blit\n" \
"Src: Size        [640x640]\n"\
"     Rect        [160,160,480,480]\n"\
"     Format      [ABGR8888/XBGR8888/ABGR4444/BGR565/ABGR1555/RG16/R8]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [TPC compressed]\n" \
"Dst: Size        [1920x1280]\n"\
"     Rect        [variable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0/90/180/270/FlipX/FlipY]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: Type   [SolidBrush]\n"\
"       Format [ARGB8888]\n"\
"       Offset [0]\n" \
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
    gcvSURF_A8B8G8R8,
    gcvSURF_X8B8G8R8,
    gcvSURF_A4B4G4R4,
    gcvSURF_B5G6R5,
    gcvSURF_A1B5G5R5,
    gcvSURF_RG16,
    gcvSURF_R8,
};

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gceSTATUS status = gcvSTATUS_OK;
    gco2D egn2D = t2d->runtime->engine2d;
    T2D_SURF_PTR src = gcvNULL;
    T2D_SURF_PTR surf = gcvNULL;
    T2D_SURF_PTR result = gcvNULL, result2 = gcvNULL;
    gctINT32 len, n;
    gctUINT32 horFactor, verFactor;
    gcsRECT srect, rect, tmpRect, clipRect;

    // create source surface
    gcmONERROR(GalLoadFileToTSurf(
        "resource/rects_640x640_A8R8G8B8.bmp",
        &src));

    egn2D = t2d->runtime->engine2d;

    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        sFormat[(frameNo / gcmCOUNTOF(sRots)) % gcmCOUNTOF(sFormat)],
        gcvLINEAR,
        gcv2D_TSC_TPC_COMPRESSED_V10,
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
        surf->aWidth,
        surf->aHeight));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        surf->tileStatusConfig,
        surf->format,
        0,
        surf->tileStatusAddress
        ));

    {
        // Need to clear TPC header buffer.
        tmpRect.left = tmpRect.top = 0;
        tmpRect.right = surf->aWidth;
        tmpRect.bottom = surf->aHeight;

        clipRect.left = clipRect.top = 0;
        clipRect.right = 6400;
        clipRect.bottom = 6400;

        gcmONERROR(gco2D_SetClipping(egn2D, &clipRect));

        gcmONERROR(gco2D_LoadSolidBrush(egn2D, surf->format, gcvTRUE, COLOR_ARGB8(0, 0, 0, 0), 0));
        gcmONERROR(gco2D_Clear(egn2D, 1, &tmpRect, COLOR_ARGB8(0, 0, 0, 0), 0xF0, 0xF0, surf->format));
        gcmONERROR(gcoHAL_Commit(gcvNULL, gcvTRUE));
    }

    gcmONERROR(gco2D_SetClipping(egn2D, &rect));

    gcmONERROR(gco2D_Blit(egn2D, 1, &rect, 0xCC, 0xCC, surf->format));


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
            surf->aWidth,
            surf->aHeight));

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

    ////
    gcmONERROR(GalCreateTSurf(
        t2d->runtime->hal,
        gcvSURF_R5G6B5,
        gcvLINEAR,
        gcv2D_TSC_DISABLE,
        640, 480,
        &result2));

    srect.left = srect.top = 0;
    srect.right = result->width;
    srect.bottom = result->height;

    rect.left = rect.top = 0;
    rect.right = result2->width;
    rect.bottom = result2->height;

    gcmONERROR(gco2D_SetGenericSource(
        egn2D,
        result->address,
        result->validAddressNum,
        result->stride,
        result->validStrideNum,
        result->tiling,
        result->format,
        gcvSURF_0_DEGREE,
        result->aWidth,
        result->aHeight));

    gcmONERROR(gco2D_SetSourceTileStatus(
        egn2D,
        gcv2D_TSC_DISABLE,
        gcvSURF_UNKNOWN,
        0,
        ~0U
        ));

    gcmONERROR(gco2D_SetTargetTileStatus(
        egn2D,
        gcv2D_TSC_DISABLE,
        gcvSURF_UNKNOWN,
        0,
        ~0U
        ));

    gcmONERROR(gco2D_SetGenericTarget(
        egn2D,
        result2->address,
        result2->validAddressNum,
        result2->stride,
        result2->validStrideNum,
        result2->tiling,
        result2->format,
        gcvSURF_0_DEGREE,
        result2->width,
        result2->height));

    gcmONERROR(gco2D_SetSource(egn2D, &srect));
    gcmONERROR(gco2D_SetClipping(egn2D, &rect));

    /* Calculate the stretch factors. */
    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srect.right - srect.left,
            rect.right - rect.left, &horFactor));

    gcmONERROR(gco2D_CalcStretchFactor(egn2D, srect.bottom - srect.top,
            rect.bottom - rect.top, &verFactor));
    gcmONERROR(gco2D_SetStretchFactors(egn2D, horFactor, verFactor));

    gcmONERROR(gco2D_StretchBlit(egn2D, 1, &rect, 0xCC, 0xCC, result2->format));
    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    if (t2d->runtime->saveFullName)
    {
        GalSaveTSurfToDIB(result2, t2d->runtime->saveFullName);
    }

    if (result2)
    {
        GalDeleteTSurf(gcvNULL, result2);
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
    gcvFEATURE_TPC_COMPRESSION,
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
